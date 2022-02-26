/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "communicator_aggregator.h"
#include <new>
#include <sstream>
#include <utility>
#include <functional>
#include "hash.h"
#include "log_print.h"
#include "db_common.h"
#include "communicator.h"
#include "endian_convert.h"
#include "protocol_proto.h"
#include "communicator_linker.h"

namespace DistributedDB {
namespace {
inline std::string GetThreadId()
{
    std::stringstream stream;
    stream << std::this_thread::get_id();
    return stream.str();
}
}

std::atomic<bool> CommunicatorAggregator::isCommunicatorNotFoundFeedbackEnable_{true};

CommunicatorAggregator::CommunicatorAggregator()
    : shutdown_(false),
      incFrameId_(0),
      localSourceId_(0)
{
}

CommunicatorAggregator::~CommunicatorAggregator()
{
    scheduler_.Finalize(); // Clear residual frame dumped by linker after CommunicatorAggregator finalize
    adapterHandle_ = nullptr;
    commLinker_ = nullptr;
}

int CommunicatorAggregator::Initialize(IAdapter *inAdapter)
{
    if (inAdapter == nullptr) {
        return -E_INVALID_ARGS;
    }
    adapterHandle_ = inAdapter;

    combiner_.Initialize();
    retainer_.Initialize();
    scheduler_.Initialize();

    int errCode;
    commLinker_ = new (std::nothrow) CommunicatorLinker(this);
    if (commLinker_ == nullptr) {
        errCode = -E_OUT_OF_MEMORY;
        goto ROLL_BACK;
    }
    commLinker_->Initialize();

    errCode = RegCallbackToAdapter();
    if (errCode != E_OK) {
        goto ROLL_BACK;
    }

    errCode = adapterHandle_->StartAdapter();
    if (errCode != E_OK) {
        LOGE("[CommAggr][Init] Start Adapter Fail, errCode=%d.", errCode);
        goto ROLL_BACK;
    }
    GenerateLocalSourceId();

    shutdown_ = false;
    exclusiveThread_ = std::thread(&CommunicatorAggregator::SendDataRoutine, this);
    return E_OK;
ROLL_BACK:
    UnRegCallbackFromAdapter();
    if (commLinker_ != nullptr) {
        RefObject::DecObjRef(commLinker_); // Refcount of linker is 1 when created, here to unref linker
        commLinker_ = nullptr;
    }
    // Scheduler do not need to do finalize in this roll_back
    retainer_.Finalize();
    combiner_.Finalize();
    return errCode;
}

void CommunicatorAggregator::Finalize()
{
    shutdown_ = true;
    retryCv_.notify_all();
    {
        std::lock_guard<std::mutex> wakingLockGuard(wakingMutex_);
        wakingSignal_ = true;
        wakingCv_.notify_one();
    }
    exclusiveThread_.join(); // Waiting thread to thoroughly quit
    LOGI("[CommAggr][Final] Sub Thread Exit.");
    scheduler_.Finalize(); // scheduler_ must finalize here to make space for linker to dump residual frame

    adapterHandle_->StopAdapter();
    UnRegCallbackFromAdapter();
    std::this_thread::sleep_for(std::chrono::milliseconds(100)); // Wait 100 ms to make sure all callback thread quit

    // No callback now and later, so combiner, retainer and linker can finalize or delete safely
    RefObject::DecObjRef(commLinker_); // Refcount of linker is 1 when created, here to unref linker
    commLinker_ = nullptr;
    retainer_.Finalize();
    combiner_.Finalize();
}

ICommunicator *CommunicatorAggregator::AllocCommunicator(uint64_t commLabel, int &outErrorNo)
{
    uint64_t netOrderLabel = HostToNet(commLabel);
    uint8_t *eachByte = reinterpret_cast<uint8_t *>(&netOrderLabel);
    std::vector<uint8_t> realLabel(COMM_LABEL_LENGTH, 0);
    for (int i = 0; i < static_cast<int>(sizeof(uint64_t)); i++) {
        realLabel[i] = eachByte[i];
    }
    return AllocCommunicator(realLabel, outErrorNo);
}

ICommunicator *CommunicatorAggregator::AllocCommunicator(const std::vector<uint8_t> &commLabel, int &outErrorNo)
{
    std::lock_guard<std::mutex> commMapLockGuard(commMapMutex_);
    LOGI("[CommAggr][Alloc] Label=%s.", VEC_TO_STR(commLabel));
    if (commLabel.size() != COMM_LABEL_LENGTH) {
        outErrorNo = -E_INVALID_ARGS;
        return nullptr;
    }

    if (commMap_.count(commLabel) != 0) {
        outErrorNo = -E_ALREADY_ALLOC;
        return nullptr;
    }

    Communicator *commPtr = new (std::nothrow) Communicator(this, commLabel);
    if (commPtr == nullptr) {
        outErrorNo = -E_OUT_OF_MEMORY;
        return nullptr;
    }
    commMap_[commLabel] = {commPtr, false}; // Communicator is not activated when allocated
    return commPtr;
}

void CommunicatorAggregator::ReleaseCommunicator(ICommunicator *inCommunicator)
{
    if (inCommunicator == nullptr) {
        return;
    }
    Communicator *commPtr = static_cast<Communicator *>(inCommunicator);
    LabelType commLabel = commPtr->GetCommunicatorLabel();
    LOGI("[CommAggr][Release] Label=%s.", VEC_TO_STR(commLabel));

    std::lock_guard<std::mutex> commMapLockGuard(commMapMutex_);
    if (commMap_.count(commLabel) == 0) {
        LOGE("[CommAggr][Release] Not Found.");
        return;
    }
    commMap_.erase(commLabel);
    RefObject::DecObjRef(commPtr); // Refcount of Communicator is 1 when created, here to unref Communicator

    int errCode = commLinker_->DecreaseLocalLabel(commLabel);
    if (errCode != E_OK) {
        LOGE("[CommAggr][Release] DecreaseLocalLabel Fail, Just Log, errCode=%d.", errCode);
    }
}

int CommunicatorAggregator::RegCommunicatorLackCallback(const CommunicatorLackCallback &onCommLack,
    const Finalizer &inOper)
{
    std::lock_guard<std::mutex> onCommLackLockGuard(onCommLackMutex_);
    return RegCallBack(onCommLack, onCommLackHandle_, inOper, onCommLackFinalizer_);
}

int CommunicatorAggregator::RegOnConnectCallback(const OnConnectCallback &onConnect, const Finalizer &inOper)
{
    std::lock_guard<std::mutex> onConnectLockGuard(onConnectMutex_);
    int errCode = RegCallBack(onConnect, onConnectHandle_, inOper, onConnectFinalizer_);
    if (onConnect && errCode == E_OK) {
        // Register action and success
        std::set<std::string> onlineTargets = commLinker_->GetOnlineRemoteTarget();
        for (auto &entry : onlineTargets) {
            LOGI("[CommAggr][RegConnect] Online target=%s{private}.", entry.c_str());
            onConnectHandle_(entry, true);
        }
    }
    return errCode;
}

uint32_t CommunicatorAggregator::GetCommunicatorAggregatorMtuSize() const
{
    return adapterHandle_->GetMtuSize() - ProtocolProto::GetLengthBeforeSerializedData();
}

uint32_t CommunicatorAggregator::GetCommunicatorAggregatorMtuSize(const std::string &target) const
{
    return adapterHandle_->GetMtuSize(target) - ProtocolProto::GetLengthBeforeSerializedData();
}

int CommunicatorAggregator::GetLocalIdentity(std::string &outTarget) const
{
    return adapterHandle_->GetLocalIdentity(outTarget);
}

void CommunicatorAggregator::ActivateCommunicator(const LabelType &commLabel)
{
    std::lock_guard<std::mutex> commMapLockGuard(commMapMutex_);
    LOGI("[CommAggr][Activate] Label=%s.", VEC_TO_STR(commLabel));
    if (commMap_.count(commLabel) == 0) {
        LOGW("[CommAggr][Activate] Communicator of this label not allocated.");
        return;
    }
    if (commMap_.at(commLabel).second) {
        LOGW("[CommAggr][Activate] Communicator of this label had been activated.");
        return;
    }
    commMap_.at(commLabel).second = true; // Mark this communicator as activated

    // IncreaseLocalLabel below and DecreaseLocalLabel in ReleaseCommunicator should all be protected by commMapMutex_
    // To avoid disordering probably caused by concurrent call to ActivateCommunicator and ReleaseCommunicator
    std::set<std::string> onlineTargets;
    int errCode = commLinker_->IncreaseLocalLabel(commLabel, onlineTargets);
    if (errCode != E_OK) {
        LOGE("[CommAggr][Activate] IncreaseLocalLabel Fail, Just Log, errCode=%d.", errCode);
        // Do not return here
    }
    for (auto &entry : onlineTargets) {
        LOGI("[CommAggr][Activate] Already Online Target=%s{private}.", entry.c_str());
        commMap_.at(commLabel).first->OnConnectChange(entry, true);
    }
    // Do Redeliver, the communicator is responsible to deal with the frame
    std::list<FrameInfo> framesToRedeliver = retainer_.FetchFramesForSpecificCommunicator(commLabel);
    for (auto &entry : framesToRedeliver) {
        commMap_.at(commLabel).first->OnBufferReceive(entry.srcTarget, entry.buffer);
    }
}

namespace {
void DoOnSendEndByTaskIfNeed(const OnSendEnd &onEnd, int result)
{
    if (onEnd) {
        TaskAction onSendEndTask = [onEnd, result]() {
            LOGD("[CommAggr][SendEndTask] Before On Send End.");
            onEnd(result);
            LOGD("[CommAggr][SendEndTask] After On Send End.");
        };
        int errCode = RuntimeContext::GetInstance()->ScheduleTask(onSendEndTask);
        if (errCode != E_OK) {
            LOGE("[CommAggr][SendEndTask] ScheduleTask failed, errCode = %d.", errCode);
        }
    }
}
}

int CommunicatorAggregator::CreateSendTask(const std::string &dstTarget, SerialBuffer *inBuff,
    FrameType inType, const TaskConfig &inConfig, const OnSendEnd &onEnd)
{
    if (inBuff == nullptr) {
        return -E_INVALID_ARGS;
    }
    LOGI("[CommAggr][Create] Enter, thread=%s, target=%s{private}, type=%d, nonBlock=%d, timeout=%u, prio=%d.",
        GetThreadId().c_str(), dstTarget.c_str(), static_cast<int>(inType), inConfig.nonBlock, inConfig.timeout,
        static_cast<int>(inConfig.prio));

    if (!ReGenerateLocalSourceIdIfNeed()) {
        delete inBuff;
        inBuff = nullptr;
        DoOnSendEndByTaskIfNeed(onEnd, -E_PERIPHERAL_INTERFACE_FAIL);
        LOGE("[CommAggr][Create] Exit ok but discard since localSourceId zero, thread=%s.", GetThreadId().c_str());
        return E_OK; // Returns E_OK here to indicate this buffer was accepted though discard immediately
    }
    PhyHeaderInfo info{localSourceId_, incFrameId_.fetch_add(1, std::memory_order_seq_cst), inType};
    int errCode = ProtocolProto::SetPhyHeader(inBuff, info);
    if (errCode != E_OK) {
        LOGE("[CommAggr][Create] Set phyHeader fail, thread=%s, errCode=%d", GetThreadId().c_str(), errCode);
        return errCode;
    }

    SendTask task{inBuff, dstTarget, onEnd};
    if (inConfig.nonBlock) {
        errCode = scheduler_.AddSendTaskIntoSchedule(task, inConfig.prio);
    } else {
        errCode = RetryUntilTimeout(task, inConfig.timeout, inConfig.prio);
    }
    if (errCode != E_OK) {
        LOGW("[CommAggr][Create] Exit failed, thread=%s, errCode=%d", GetThreadId().c_str(), errCode);
        return errCode;
    }

    std::lock_guard<std::mutex> wakingLockGuard(wakingMutex_);
    wakingSignal_ = true;
    wakingCv_.notify_one();
    LOGI("[CommAggr][Create] Exit ok, thread=%s, frameId=%u", GetThreadId().c_str(), info.frameId); // Delete In Future
    return E_OK;
}

void CommunicatorAggregator::EnableCommunicatorNotFoundFeedback(bool isEnable)
{
    isCommunicatorNotFoundFeedbackEnable_ = isEnable;
}

int CommunicatorAggregator::GetRemoteCommunicatorVersion(const std::string &target, uint16_t &outVersion) const
{
    std::lock_guard<std::mutex> versionMapLockGuard(versionMapMutex_);
    auto pair = versionMap_.find(target);
    if (pair == versionMap_.end()) {
        return -E_NOT_FOUND;
    }
    outVersion = pair->second;
    return E_OK;
}

void CommunicatorAggregator::SendDataRoutine()
{
    while (!shutdown_) {
        if (scheduler_.GetNoDelayTaskCount() == 0) {
            std::unique_lock<std::mutex> wakingUniqueLock(wakingMutex_);
            LOGI("[CommAggr][Routine] Send done and sleep."); // Delete In Future
            wakingCv_.wait(wakingUniqueLock, [this] { return this->wakingSignal_; });
            LOGI("[CommAggr][Routine] Send continue."); // Delete In Future
            wakingSignal_ = false;
            continue;
        }

        SendTask taskToSend;
        int errCode = scheduler_.ScheduleOutSendTask(taskToSend);
        if (errCode != E_OK) {
            continue; // Not possible to happen
        }

        std::vector<std::vector<uint8_t>> piecePackets;
        errCode = ProtocolProto::SplitFrameIntoPacketsIfNeed(taskToSend.buffer,
            adapterHandle_->GetMtuSize(taskToSend.dstTarget), piecePackets);
        if (errCode != E_OK) {
            LOGE("[CommAggr][Routine] Split frame fail, errCode=%d.", errCode);
            TaskFinalizer(taskToSend, errCode);
            continue;
        }

        std::vector<std::pair<const uint8_t *, uint32_t>> eachPacket;
        if (piecePackets.size() == 0) {
            // Case that no need to split a frame, just use original buffer as a packet
            eachPacket.push_back(taskToSend.buffer->GetReadOnlyBytesForEntireBuffer());
        } else {
            for (auto &entry : piecePackets) {
                eachPacket.push_back(std::make_pair<const uint8_t *, uint32_t>(&(entry[0]), entry.size()));
            }
        }

        SendPacketsAndDisposeTask(taskToSend, eachPacket);
    }
}

void CommunicatorAggregator::SendPacketsAndDisposeTask(const SendTask &inTask,
    const std::vector<std::pair<const uint8_t *, uint32_t>> &eachPacket)
{
    bool taskNeedFinalize = true;
    int errCode = E_OK;
    for (auto &entry : eachPacket) {
        LOGI("[CommAggr][SendPackets] DoSendBytes, dstTarget=%s{private}, length=%u.", inTask.dstTarget.c_str(),
            entry.second);
        ProtocolProto::DisplayPacketInformation(entry.first, entry.second); // For debug, delete in the future
        errCode = adapterHandle_->SendBytes(inTask.dstTarget, entry.first, entry.second);
        if (errCode == -E_WAIT_RETRY) {
            LOGE("[CommAggr][SendPackets] SendBytes temporally fail.");
            scheduler_.DelayTaskByTarget(inTask.dstTarget);
            taskNeedFinalize = false;
            break;
        } else if (errCode != E_OK) {
            LOGE("[CommAggr][SendPackets] SendBytes totally fail, errCode=%d.", errCode);
            break;
        }
    }
    if (taskNeedFinalize) {
        TaskFinalizer(inTask, errCode);
    }
}

int CommunicatorAggregator::RetryUntilTimeout(SendTask &inTask, uint32_t timeout, Priority inPrio)
{
    int errCode = scheduler_.AddSendTaskIntoSchedule(inTask, inPrio);
    if (errCode != E_OK) {
        bool notTimeout = true;
        auto retryFunc = [this, inPrio, &inTask]()->bool{
            if (this->shutdown_) {
                delete inTask.buffer;
                inTask.buffer = nullptr;
                return true;
            }
            int retCode = scheduler_.AddSendTaskIntoSchedule(inTask, inPrio);
            if (retCode != E_OK) {
                return false;
            }
            return true;
        };

        if (timeout == 0) { // Unlimited retry
            std::unique_lock<std::mutex> retryUniqueLock(retryMutex_);
            retryCv_.wait(retryUniqueLock, retryFunc);
        } else {
            std::unique_lock<std::mutex> retryUniqueLock(retryMutex_);
            notTimeout = retryCv_.wait_for(retryUniqueLock, std::chrono::milliseconds(timeout), retryFunc);
        }

        if (shutdown_) {
            return E_OK;
        }
        if (!notTimeout) {
            return -E_TIMEOUT;
        }
    }
    return E_OK;
}

void CommunicatorAggregator::TaskFinalizer(const SendTask &inTask, int result)
{
    // Call the OnSendEnd if need
    if (inTask.onEnd) {
        LOGD("[CommAggr][TaskFinal] On Send End.");
        inTask.onEnd(result);
    }
    // Finalize the task that just scheduled
    int errCode = scheduler_.FinalizeLastScheduleTask();
    // Notify Sendable To All Communicator If Need
    if (errCode == -E_CONTAINER_FULL_TO_NOTFULL) {
        retryCv_.notify_all();
    }
    if (errCode == -E_CONTAINER_NOTEMPTY_TO_EMPTY) {
        NotifySendableToAllCommunicator();
    }
}

void CommunicatorAggregator::NotifySendableToAllCommunicator()
{
    std::lock_guard<std::mutex> commMapLockGuard(commMapMutex_);
    for (auto &entry : commMap_) {
        // Ignore nonactivated communicator
        if (entry.second.second) {
            entry.second.first->OnSendAvailable();
        }
    }
}

void CommunicatorAggregator::OnBytesReceive(const std::string &srcTarget, const uint8_t *bytes, uint32_t length)
{
    ProtocolProto::DisplayPacketInformation(bytes, length); // For debug, delete in the future
    ParseResult packetResult;
    int errCode = ProtocolProto::CheckAndParsePacket(srcTarget, bytes, length, packetResult);
    if (errCode != E_OK) {
        LOGE("[CommAggr][Receive] Parse packet fail, errCode=%d.", errCode);
        if (errCode == -E_VERSION_NOT_SUPPORT) {
            TriggerVersionNegotiation(srcTarget);
        }
        return;
    }

    // Update version of remote target
    SetRemoteCommunicatorVersion(srcTarget, packetResult.GetDbVersion());
    if (packetResult.GetFrameTypeInfo() == FrameType::EMPTY) { // Empty frame will never be fragmented
        LOGI("[CommAggr][Receive] Empty frame, just ignore in this version of distributeddb.");
        return;
    }

    if (packetResult.IsFragment()) {
        OnFragmentReceive(srcTarget, bytes, length, packetResult);
    } else if (packetResult.GetFrameTypeInfo() != FrameType::APPLICATION_MESSAGE) {
        errCode = OnCommLayerFrameReceive(srcTarget, packetResult);
        if (errCode != E_OK) {
            LOGE("[CommAggr][Receive] CommLayer receive fail, errCode=%d.", errCode);
        }
    } else {
        errCode = OnAppLayerFrameReceive(srcTarget, bytes, length, packetResult);
        if (errCode != E_OK) {
            LOGE("[CommAggr][Receive] AppLayer receive fail, errCode=%d.", errCode);
        }
    }
}

void CommunicatorAggregator::OnTargetChange(const std::string &target, bool isConnect)
{
    if (target.empty()) {
        LOGE("[CommAggr][OnTarget] Target empty string.");
        return;
    }
    // For process level target change
    {
        std::lock_guard<std::mutex> onConnectLockGuard(onConnectMutex_);
        if (onConnectHandle_) {
            onConnectHandle_(target, isConnect);
            LOGI("[CommAggr][OnTarget] On Connect End."); // Log in case callback block this thread
        } else {
            LOGI("[CommAggr][OnTarget] ConnectHandle invalid currently.");
        }
    }
    // For communicator level target change
    std::set<LabelType> relatedLabels;
    if (isConnect) {
        int errCode = commLinker_->TargetOnline(target, relatedLabels);
        if (errCode != E_OK) {
            LOGE("[CommAggr][OnTarget] TargetOnline fail, target=%s{private}, errCode=%d.", target.c_str(), errCode);
        }
    } else {
        int errCode = commLinker_->TargetOffline(target, relatedLabels);
        if (errCode != E_OK) {
            LOGE("[CommAggr][OnTarget] TargetOffline fail, target=%s{private}, errCode=%d.", target.c_str(), errCode);
        }
    }
    // All related communicator online or offline this target, no matter TargetOnline or TargetOffline fail or not
    std::lock_guard<std::mutex> commMapLockGuard(commMapMutex_);
    for (auto &entry : commMap_) {
        // Ignore nonactivated communicator
        if (relatedLabels.count(entry.first) != 0 && entry.second.second) {
            entry.second.first->OnConnectChange(target, isConnect);
        }
    }
}

void CommunicatorAggregator::OnSendable(const std::string &target)
{
    int errCode = scheduler_.NoDelayTaskByTarget(target);
    if (errCode != E_OK) {
        LOGE("[CommAggr][Sendable] NoDelay target=%s{private} fail, errCode=%d.", target.c_str(), errCode);
        return;
    }
    std::lock_guard<std::mutex> wakingLockGuard(wakingMutex_);
    wakingSignal_ = true;
    wakingCv_.notify_one();
}

void CommunicatorAggregator::OnFragmentReceive(const std::string &srcTarget, const uint8_t *bytes, uint32_t length,
    const ParseResult &inResult)
{
    int errorNo = E_OK;
    ParseResult frameResult;
    SerialBuffer *frameBuffer = combiner_.AssembleFrameFragment(bytes, length, inResult, frameResult, errorNo);
    if (errorNo != E_OK) {
        LOGE("[CommAggr][Receive] Combine fail, errCode=%d.", errorNo);
        return;
    }
    if (frameBuffer == nullptr) {
        LOGW("[CommAggr][Receive] Combine undone.");
        return;
    }

    int errCode = ProtocolProto::CheckAndParseFrame(frameBuffer, frameResult);
    if (errCode != E_OK) {
        LOGE("[CommAggr][Receive] Parse frame fail, errCode=%d.", errCode);
        delete frameBuffer;
        frameBuffer = nullptr;
        if (errCode == -E_VERSION_NOT_SUPPORT) {
            TriggerVersionNegotiation(srcTarget);
        }
        return;
    }

    if (frameResult.GetFrameTypeInfo() != FrameType::APPLICATION_MESSAGE) {
        errCode = OnCommLayerFrameReceive(srcTarget, frameResult);
        if (errCode != E_OK) {
            LOGE("[CommAggr][Receive] CommLayer receive fail after combination, errCode=%d.", errCode);
        }
        delete frameBuffer;
        frameBuffer = nullptr;
    } else {
        errCode = OnAppLayerFrameReceive(srcTarget, frameBuffer, frameResult);
        if (errCode != E_OK) {
            LOGE("[CommAggr][Receive] AppLayer receive fail after combination, errCode=%d.", errCode);
        }
    }
}

int CommunicatorAggregator::OnCommLayerFrameReceive(const std::string &srcTarget, const ParseResult &inResult)
{
    if (inResult.GetFrameTypeInfo() == FrameType::COMMUNICATION_LABEL_EXCHANGE_ACK) {
        int errCode = commLinker_->ReceiveLabelExchangeAck(srcTarget, inResult.GetLabelExchangeDistinctValue(),
            inResult.GetLabelExchangeSequenceId());
        if (errCode != E_OK) {
            LOGE("[CommAggr][CommReceive] Receive LabelExchangeAck Fail.");
            return errCode;
        }
    } else {
        std::map<LabelType, bool> changedLabels;
        int errCode = commLinker_->ReceiveLabelExchange(srcTarget, inResult.GetLatestCommLabels(),
            inResult.GetLabelExchangeDistinctValue(), inResult.GetLabelExchangeSequenceId(), changedLabels);
        if (errCode != E_OK) {
            LOGE("[CommAggr][CommReceive] Receive LabelExchange Fail.");
            return errCode;
        }
        if (!commLinker_->IsRemoteTargetOnline(srcTarget)) {
            LOGW("[CommAggr][CommReceive] Receive LabelExchange from offline target=%s{private}.", srcTarget.c_str());
            for (const auto &entry : changedLabels) {
                LOGW("[CommAggr][CommReceive] REMEMBER: label=%s, inOnline=%d.", VEC_TO_STR(entry.first), entry.second);
            }
            return E_OK;
        }
        // Do target change notify
        std::lock_guard<std::mutex> commMapLockGuard(commMapMutex_);
        for (auto &entry : changedLabels) {
            // Ignore nonactivated communicator
            if (commMap_.count(entry.first) != 0 && commMap_.at(entry.first).second) {
                LOGI("[CommAggr][CommReceive] label=%s, srcTarget=%s{private}, isOnline=%d.",
                    VEC_TO_STR(entry.first), srcTarget.c_str(), entry.second);
                commMap_.at(entry.first).first->OnConnectChange(srcTarget, entry.second);
            }
        }
    }
    return E_OK;
}

int CommunicatorAggregator::OnAppLayerFrameReceive(const std::string &srcTarget, const uint8_t *bytes,
    uint32_t length, const ParseResult &inResult)
{
    SerialBuffer *buffer = new (std::nothrow) SerialBuffer();
    if (buffer == nullptr) {
        LOGE("[CommAggr][AppReceive] New SerialBuffer fail.");
        return -E_OUT_OF_MEMORY;
    }
    int errCode = buffer->SetExternalBuff(bytes, length - inResult.GetPaddingLen(),
        ProtocolProto::GetAppLayerFrameHeaderLength());
    if (errCode != E_OK) {
        LOGE("[CommAggr][AppReceive] SetExternalBuff fail, errCode=%d.", errCode);
        delete buffer;
        buffer = nullptr;
        return -E_INTERNAL_ERROR;
    }
    return OnAppLayerFrameReceive(srcTarget, buffer, inResult);
}

// In early time, we cover "OnAppLayerFrameReceive" totally by commMapMutex_, then search communicator, if not found,
// we call onCommLackHandle_ if exist to ask whether to retain this frame or not, if the answer is yes we retain this
// frame, otherwise we discard this frame and send out CommunicatorNotFound feedback.
// We design so(especially cover this function totally by commMapMutex_) to avoid current situation described below
// 1:This func find that target communicator not allocated or activated, so decide to retain this frame.
// 2:Thread switch out, the target communicator is allocated and activated, previous retained frame is fetched out.
// 3:Thread switch back, this frame is then retained into the retainer, no chance to be fetched out.
// In conclusion: the decision to retain a frame and the action to retain a frame should not be separated.
// Otherwise, at the action time, the retain decision may be obsolete and wrong.
// #### BUT #### since onCommLackHandle_ callback is go beyond DistributedDB and there is the risk that the final upper
// user may do something such as GetKvStore(we can prevent them to so) which could result in calling AllocCommunicator
// in the same callback thread finally causing DeadLock on commMapMutex_.
// #### SO #### we have to make a change described below
// 1:Search communicator under commMapMutex_, if found then deliver frame to that communicator and end.
// 2:Call onCommLackHandle_ if exist to ask whether to retain this frame or not, without commMapMutex_.
// Note: during this period, commMap_ maybe changed, and communicator not found before may exist now.
// 3:Search communicator under commMapMutex_ again, if found then deliver frame to that communicator and end.
// 4:If still not found, retain this frame if need or otherwise send CommunicatorNotFound feedback.
int CommunicatorAggregator::OnAppLayerFrameReceive(const std::string &srcTarget, SerialBuffer *&inFrameBuffer,
    const ParseResult &inResult)
{
    LabelType toLabel = inResult.GetCommLabel();
    {
        std::lock_guard<std::mutex> commMapLockGuard(commMapMutex_);
        int errCode = TryDeliverAppLayerFrameToCommunicatorNoMutex(srcTarget, inFrameBuffer, toLabel);
        if (errCode == E_OK) { // Attention: Here is equal to E_OK
            return E_OK;
        }
    }
    LOGI("[CommAggr][AppReceive] Communicator of %s not found or nonactivated.", VEC_TO_STR(toLabel));
    int errCode = -E_NOT_FOUND;
    {
        std::lock_guard<std::mutex> onCommLackLockGuard(onCommLackMutex_);
        if (onCommLackHandle_) {
            errCode = onCommLackHandle_(toLabel);
            LOGI("[CommAggr][AppReceive] On CommLack End."); // Log in case callback block this thread
        } else {
            LOGI("[CommAggr][AppReceive] CommLackHandle invalid currently.");
        }
    }
    // Here we have to lock commMapMutex_ and search communicator again.
    std::lock_guard<std::mutex> commMapLockGuard(commMapMutex_);
    int errCodeAgain = TryDeliverAppLayerFrameToCommunicatorNoMutex(srcTarget, inFrameBuffer, toLabel);
    if (errCodeAgain == E_OK) { // Attention: Here is equal to E_OK.
        LOGI("[CommAggr][AppReceive] Communicator of %s found after try again(rare case).", VEC_TO_STR(toLabel));
        return E_OK;
    }
    // Here, communicator is still not found, retain or discard according to the result of onCommLackHandle_
    if (errCode != E_OK) {
        TryToFeedbackWhenCommunicatorNotFound(srcTarget, toLabel, inFrameBuffer);
        delete inFrameBuffer;
        inFrameBuffer = nullptr;
        return errCode; // The caller will display errCode in log
    }
    // Do Retention, the retainer is responsible to deal with the frame
    retainer_.RetainFrame(FrameInfo{inFrameBuffer, srcTarget, toLabel, inResult.GetFrameId()});
    inFrameBuffer = nullptr;
    return E_OK;
}

int CommunicatorAggregator::TryDeliverAppLayerFrameToCommunicatorNoMutex(const std::string &srcTarget,
    SerialBuffer *&inFrameBuffer, const LabelType &toLabel)
{
    // Ignore nonactivated communicator, which is regarded as inexistent
    if (commMap_.count(toLabel) != 0 && commMap_.at(toLabel).second) {
        commMap_.at(toLabel).first->OnBufferReceive(srcTarget, inFrameBuffer);
        // Frame handed over to communicator who is responsible to delete it. The frame is deleted here after return.
        inFrameBuffer = nullptr;
        return E_OK;
    }
    return -E_NOT_FOUND;
}

int CommunicatorAggregator::RegCallbackToAdapter()
{
    RefObject::IncObjRef(this); // Reference to be hold by adapter
    int errCode = adapterHandle_->RegBytesReceiveCallback(
        std::bind(&CommunicatorAggregator::OnBytesReceive, this, std::placeholders::_1, std::placeholders::_2,
            std::placeholders::_3),
        [this]() { RefObject::DecObjRef(this); });
    if (errCode != E_OK) {
        RefObject::DecObjRef(this); // Rollback in case reg failed
        return errCode;
    }

    RefObject::IncObjRef(this); // Reference to be hold by adapter
    errCode = adapterHandle_->RegTargetChangeCallback(
        std::bind(&CommunicatorAggregator::OnTargetChange, this, std::placeholders::_1, std::placeholders::_2),
        [this]() { RefObject::DecObjRef(this); });
    if (errCode != E_OK) {
        RefObject::DecObjRef(this); // Rollback in case reg failed
        return errCode;
    }

    RefObject::IncObjRef(this); // Reference to be hold by adapter
    errCode = adapterHandle_->RegSendableCallback(
        std::bind(&CommunicatorAggregator::OnSendable, this, std::placeholders::_1),
        [this]() { RefObject::DecObjRef(this); });
    if (errCode != E_OK) {
        RefObject::DecObjRef(this); // Rollback in case reg failed
        return errCode;
    }

    return E_OK;
}

void CommunicatorAggregator::UnRegCallbackFromAdapter()
{
    adapterHandle_->RegBytesReceiveCallback(nullptr, nullptr);
    adapterHandle_->RegTargetChangeCallback(nullptr, nullptr);
    adapterHandle_->RegSendableCallback(nullptr, nullptr);
}

void CommunicatorAggregator::GenerateLocalSourceId()
{
    std::string identity;
    adapterHandle_->GetLocalIdentity(identity);
    // When GetLocalIdentity fail, the identity be an empty string, the localSourceId be zero, need regenerate
    // The localSourceId is std::atomic<uint64_t>, so there is no concurrency risk
    uint64_t identityHash = Hash::HashFunc(identity);
    localSourceId_ = identityHash;
    LOGI("[CommAggr][GenSrcId] identity=%s{private}, localSourceId=%llu.", identity.c_str(), ULL(identityHash));
}

bool CommunicatorAggregator::ReGenerateLocalSourceIdIfNeed()
{
    // If localSourceId is zero, pre-generate must have used an empty identity, re-fetch the identity and generate.
    // The localSourceId is std::atomic<uint64_t>, so there is no concurrency risk, no need lockguard here.
    if (localSourceId_ == 0) {
        GenerateLocalSourceId();
        return (localSourceId_ != 0);
    }
    return true;
}

void CommunicatorAggregator::TriggerVersionNegotiation(const std::string &dstTarget)
{
    LOGI("[CommAggr][TrigVer] Do version negotiate with target=%s{private}.", dstTarget.c_str());
    int errCode = E_OK;
    SerialBuffer *buffer = ProtocolProto::BuildEmptyFrameForVersionNegotiate(errCode);
    if (errCode != E_OK) {
        LOGE("[CommAggr][TrigVer] Build empty frame fail, errCode=%d", errCode);
        return;
    }

    TaskConfig config{true, 0, Priority::HIGH};
    errCode = CreateSendTask(dstTarget, buffer, FrameType::EMPTY, config);
    if (errCode != E_OK) {
        LOGE("[CommAggr][TrigVer] Send empty frame fail, errCode=%d", errCode);
        // if send fails, free buffer, otherwise buffer will be taked over by SendTaskScheduler
        delete buffer;
        buffer = nullptr;
    }
}

void CommunicatorAggregator::TryToFeedbackWhenCommunicatorNotFound(const std::string &dstTarget,
    const LabelType &dstLabel, const SerialBuffer *inOriFrame)
{
    if (!isCommunicatorNotFoundFeedbackEnable_ || dstTarget.empty() || inOriFrame == nullptr) {
        return;
    }
    int errCode = E_OK;
    Message *message = ProtocolProto::ToMessage(inOriFrame, errCode, true);
    if (message == nullptr) {
        if (errCode == -E_VERSION_NOT_SUPPORT) {
            TriggerVersionNegotiation(dstTarget);
        }
        return;
    }
    // Message is release in TriggerCommunicatorNotFoundFeedback
    TriggerCommunicatorNotFoundFeedback(dstTarget, dstLabel, message);
}

void CommunicatorAggregator::TriggerCommunicatorNotFoundFeedback(const std::string &dstTarget,
    const LabelType &dstLabel, Message* &oriMsg)
{
    if (oriMsg == nullptr || oriMsg->GetMessageType() != TYPE_REQUEST) {
        LOGI("[CommAggr][TrigNotFound] Do nothing for message with type not request.");
        // Do not have to do feedback if the message is not a request type message
        delete oriMsg;
        oriMsg = nullptr;
        return;
    }

    LOGI("[CommAggr][TrigNotFound] Do communicator not found feedback with target=%s{private}.", dstTarget.c_str());
    oriMsg->SetMessageType(TYPE_RESPONSE);
    oriMsg->SetErrorNo(E_FEEDBACK_COMMUNICATOR_NOT_FOUND);

    int errCode = E_OK;
    SerialBuffer *buffer = ProtocolProto::BuildFeedbackMessageFrame(oriMsg, dstLabel, errCode);
    delete oriMsg;
    oriMsg = nullptr;
    if (errCode != E_OK) {
        LOGE("[CommAggr][TrigNotFound] Build communicator not found feedback frame fail, errCode=%d", errCode);
        return;
    }

    TaskConfig config{true, 0, Priority::HIGH};
    errCode = CreateSendTask(dstTarget, buffer, FrameType::APPLICATION_MESSAGE, config);
    if (errCode != E_OK) {
        LOGE("[CommAggr][TrigNotFound] Send communicator not found feedback frame fail, errCode=%d", errCode);
        // if send fails, free buffer, otherwise buffer will be taked over by CreateSendTask
        delete buffer;
        buffer = nullptr;
    }
}

void CommunicatorAggregator::SetRemoteCommunicatorVersion(const std::string &target, uint16_t version)
{
    std::lock_guard<std::mutex> versionMapLockGuard(versionMapMutex_);
    versionMap_[target] = version;
}

DEFINE_OBJECT_TAG_FACILITIES(CommunicatorAggregator)
} // namespace DistributedDB
