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

#include "communicator_linker.h"
#include "hash.h"
#include "db_errno.h"
#include "log_print.h"
#include "protocol_proto.h"
#include "platform_specific.h"
#include "communicator_aggregator.h"

namespace DistributedDB {
namespace {
constexpr uint32_t TIME_LAPSE_FOR_WAITING_ACK = 5000; // 5s
constexpr uint32_t TIME_LAPSE_FOR_RETRY_SEND = 1000; // 1s
constexpr uint32_t RETRANSMIT_LIMIT = 20; // Currently we do at most 20 retransmission if no ack received
constexpr uint32_t RETRANSMIT_LIMIT_EQUAL_INTERVAL = 5; // First 5 retransmission will be equal interval
}

CommunicatorLinker::CommunicatorLinker(CommunicatorAggregator *inAggregator)
    : incSequenceId_(0), incAckTriggerId_(0)
{
    aggregator_ = inAggregator;
    RefObject::IncObjRef(aggregator_); // The linker rely on CommunicatorAggregator
}

CommunicatorLinker::~CommunicatorLinker()
{
    RefObject::DecObjRef(aggregator_); // The linker no longer rely on CommunicatorAggregator
    aggregator_ = nullptr;
}

void CommunicatorLinker::Initialize()
{
    uint64_t curTime = 0;
    int errCode = OS::GetCurrentSysTimeInMicrosecond(curTime);
    if (errCode != E_OK) {
        LOGW("[Linker][Init] Get systime fail, use default, errCode=%d.", errCode);
    }
    std::string curTimeStr = std::to_string(curTime);
    localDistinctValue_ = Hash::HashFunc(curTimeStr);
    LOGI("[Linker][Init] curTime=%llu, distinct=%llu.", ULL(curTime), ULL(localDistinctValue_));
}

// Create async task to send out label_exchange and waiting for label_exchange_ack.
// If waiting timeout, pass the send&wait task to overrall timing retry task.
int CommunicatorLinker::TargetOnline(const std::string &inTarget, std::set<LabelType> &outRelatedLabels)
{
    {
        std::lock_guard<std::mutex> entireInfoLockGuard(entireInfoMutex_);
        // if inTarget is offline before, use the remembered previous online labels to decide which communicator to be
        // notified online. Such handling is in case for abnormal unilateral offline, which A and B is notified online
        // mutually, then B is notified A offline and for a while B is notified A online again, but A feels no notify.
        if (remoteOnlineTarget_.count(inTarget) == 0) {
            outRelatedLabels = targetMapOnlineLabels_[inTarget];
            remoteOnlineTarget_.insert(inTarget);
        }
    }
    return TriggerLabelExchangeEvent(inTarget);
}

// Clear all labels related to this target. Let no longer waiting for ack of this target.
// The caller should notify all related communicator about this target offline.
int CommunicatorLinker::TargetOffline(const std::string &inTarget, std::set<LabelType> &outRelatedLabels)
{
    std::lock_guard<std::mutex> entireInfoLockGuard(entireInfoMutex_);
    outRelatedLabels = targetMapOnlineLabels_[inTarget];
    // Do not erase the Labels of inTarget from targetMapOnlineLabels_, remember it for using when TargetOnline
    remoteOnlineTarget_.erase(inTarget);
    // Note: The process of remote target may quit, when remote target restart,
    // the distinctValue of this remote target may be changed, and the sequenceId may start from zero
    targetDistinctValue_.erase(inTarget);
    topRecvLabelSeq_.erase(inTarget);
    return E_OK;
}

// Add local label. Create async task to send out label_exchange and waiting for label_exchange_ack.
// If waiting timeout, pass the send&wait task to overrall timing retry task.
// Find out targets for this label that is already online.
// The caller should notify communicator of this label about already online target.
int CommunicatorLinker::IncreaseLocalLabel(const LabelType &inLabel, std::set<std::string> &outOnlineTarget)
{
    std::set<std::string> totalOnlineTargets;
    {
        std::lock_guard<std::mutex> entireInfoLockGuard(entireInfoMutex_);
        localOnlineLabels_.insert(inLabel);
        totalOnlineTargets = remoteOnlineTarget_;
        for (auto &entry : targetMapOnlineLabels_) {
            if (remoteOnlineTarget_.count(entry.first) == 0) { // Ignore offline target
                continue;
            }
            if (entry.second.count(inLabel) != 0) { // This online target had opened then same Label
                outOnlineTarget.insert(entry.first);
            }
        }
    }
    bool everFail = false;
    for (auto &entry : totalOnlineTargets) {
        int errCode = TriggerLabelExchangeEvent(entry);
        if (errCode != E_OK) {
            everFail = true;
        }
    }
    return everFail ? -E_INTERNAL_ERROR : E_OK;
}

// Del local label. Create async task to send out label_exchange and waiting for label_exchange_ack.
// If waiting timeout, pass the send&wait task to overrall timing retry task.
int CommunicatorLinker::DecreaseLocalLabel(const LabelType &inLabel)
{
    std::set<std::string> totalOnlineTargets;
    {
        std::lock_guard<std::mutex> entireInfoLockGuard(entireInfoMutex_);
        localOnlineLabels_.erase(inLabel);
        totalOnlineTargets = remoteOnlineTarget_;
    }
    bool everFail = false;
    for (auto &entry : totalOnlineTargets) {
        int errCode = TriggerLabelExchangeEvent(entry);
        if (errCode != E_OK) {
            everFail = true;
        }
    }
    return everFail ? -E_INTERNAL_ERROR : E_OK;
}

// Compare the latest labels with previous Label, find out label changes.
// The caller should notify the target changes according to label changes.
// Update the online labels of this target. Send out label_exchange_ack.
int CommunicatorLinker::ReceiveLabelExchange(const std::string &inTarget, const std::set<LabelType> &inLatestLabels,
    uint64_t inDistinctValue, uint64_t inSequenceId, std::map<LabelType, bool> &outChangeLabels)
{
    {
        std::lock_guard<std::mutex> entireInfoLockGuard(entireInfoMutex_);
        DetectDistinctValueChange(inTarget, inDistinctValue);
        if (topRecvLabelSeq_.count(inTarget) == 0) {
            // Firstly receive LabelExchange from this target
            topRecvLabelSeq_[inTarget] = inSequenceId;
        } else if (inSequenceId < topRecvLabelSeq_[inTarget]) {
            // inSequenceId can be equal to topRecvLabelSeq, in this case, the ack of this sequence send to this target
            // may be lost, this target resend LabelExchange, and we should resend ack to this target
            LOGW("[Linker][RecvLabel] inSequenceId=%llu smaller than topRecvLabelSeq=%llu. Frame Ignored.",
                ULL(inSequenceId), ULL(topRecvLabelSeq_[inTarget]));
            return -E_OUT_OF_DATE;
        } else {
            // Update top sequenceId of received LabelExchange
            topRecvLabelSeq_[inTarget] = inSequenceId;
        }
        // Find out online labels by check difference
        for (auto &entry : inLatestLabels) {
            if (targetMapOnlineLabels_[inTarget].count(entry) == 0) {
                outChangeLabels[entry] = true;
            }
        }
        // Find out offline labels by check difference
        for (auto &entry : targetMapOnlineLabels_[inTarget]) {
            if (inLatestLabels.count(entry) == 0) {
                outChangeLabels[entry] = false;
            }
        }
        // Update target online labels
        targetMapOnlineLabels_[inTarget] = inLatestLabels;
    }
    // Trigger sending ack
    int errCode = TriggerLabelExchangeAckEvent(inTarget, inSequenceId);
    if (errCode != E_OK) {
        LOGE("[Linker][RecvLabel] TriggerAckEvent Fail, Just Log, errCode=%d.", errCode);
        // Do not return error here
    }
    return E_OK;
}

// Waiting finish if the ack is what linker wait by check inSequenceId
// Similarly, stop the retry task of this Target.
int CommunicatorLinker::ReceiveLabelExchangeAck(const std::string &inTarget, uint64_t inDistinctValue,
    uint64_t inSequenceId)
{
    std::lock_guard<std::mutex> entireInfoLockGuard(entireInfoMutex_);
    DetectDistinctValueChange(inTarget, inDistinctValue);
    // This two judge is for detecting case that local device process restart so incSequenceId_ restart from 0
    // The remote device may send an ack cause by previous process, which may destroy the functionality of this process
    if (waitAckSeq_.count(inTarget) == 0) {
        LOGW("[Linker][RecvAck] Not waiting any ack now, inSequenceId=%llu", ULL(inSequenceId));
        return -E_NOT_FOUND;
    }
    if (waitAckSeq_[inTarget] < inSequenceId) {
        LOGW("[Linker][RecvAck] Not waiting this ack now, inSequenceId=%llu, waitAckSeq_=%llu",
            ULL(inSequenceId), ULL(waitAckSeq_[inTarget]));
        return -E_NOT_FOUND;
    }
    // An valid ack received
    if (recvAckSeq_.count(inTarget) == 0) {
        // Firstly receive LabelExchangeAck from this target
        recvAckSeq_[inTarget] = inSequenceId;
    } else if (inSequenceId <= recvAckSeq_[inTarget]) {
        LOGW("[Linker][RecvAck] inSequenceId=%llu not greater than recvAckSeq_=%llu. Frame Ignored.",
            ULL(inSequenceId), ULL(recvAckSeq_[inTarget]));
        return -E_OUT_OF_DATE;
    } else {
        // Update top sequenceId of received LabelExchangeAck
        recvAckSeq_[inTarget] = inSequenceId;
    }
    return E_OK;
}

std::set<std::string> CommunicatorLinker::GetOnlineRemoteTarget() const
{
    std::lock_guard<std::mutex> entireInfoLockGuard(entireInfoMutex_);
    return remoteOnlineTarget_;
}

bool CommunicatorLinker::IsRemoteTargetOnline(const std::string &inTarget) const
{
    std::lock_guard<std::mutex> entireInfoLockGuard(entireInfoMutex_);
    if (remoteOnlineTarget_.count(inTarget) != 0) {
        return true;
    }
    return false;
}

// inCountDown is in millisecond
void CommunicatorLinker::SuspendByOnceTimer(const std::function<void(void)> &inAction, uint32_t inCountDown)
{
    TimerId thisTimerId = 0;
    RuntimeContext *context = RuntimeContext::GetInstance();
    int errCode = context->SetTimer(static_cast<int>(inCountDown), [inAction](TimerId inTimerId)->int{
        // Note: inAction should be captured by value (must not by reference)
        LOGI("[Linker][Suspend] Timer Due : inTimerId=%llu.", ULL(inTimerId));
        inAction();
        return -E_END_TIMER;
    }, nullptr, thisTimerId);
    if (errCode == E_OK) {
        LOGI("[Linker][Suspend] SetTimer Success : thisTimerId=%llu, wait=%u(ms).", ULL(thisTimerId), inCountDown);
    } else {
        LOGI("[Linker][Suspend] SetTimer Fail Raise Thread Instead : errCode=%d, wait=%u(ms).", errCode, inCountDown);
        std::thread timerThread([inAction, inCountDown]() {
            // Note: inAction and inCountDown should be captured by value (must not by reference)
            std::this_thread::sleep_for(std::chrono::milliseconds(inCountDown));
            inAction();
        });
        timerThread.detach();
    }
}

// This function should be called under protection of entireInfoMutex_
void CommunicatorLinker::DetectDistinctValueChange(const std::string &inTarget, uint64_t inDistinctValue)
{
    // Firstly received distinctValue from this target ever or after offline
    if (targetDistinctValue_.count(inTarget) == 0) {
        targetDistinctValue_[inTarget] = inDistinctValue;
        return;
    }

    // DistinctValue is the same as before
    if (targetDistinctValue_[inTarget] == inDistinctValue) {
        return;
    }

    // DistinctValue change detected !!! This must be caused by malfunctioning of underlayer communication component.
    LOGE("[Linker][Detect] ######## DISTINCT VALUE CHANGE DETECTED : %llu VS %llu ########",
        ULL(inDistinctValue), ULL(targetDistinctValue_[inTarget]));
    targetDistinctValue_[inTarget] = inDistinctValue;
    // The process of remote target must have undergone a quit and restart, the remote sequenceId will start from zero.
    topRecvLabelSeq_.erase(inTarget);
}

int CommunicatorLinker::TriggerLabelExchangeEvent(const std::string &toTarget)
{
    // Apply for a latest sequenceId
    uint64_t sequenceId = incSequenceId_.fetch_add(1, std::memory_order_seq_cst);
    // Get a snapshot of current online labels
    std::set<LabelType> onlineLabels;
    {
        std::lock_guard<std::mutex> entireInfoLockGuard(entireInfoMutex_);
        onlineLabels = localOnlineLabels_;
    }
    // Build LabelExchange Frame
    int error = E_OK;
    SerialBuffer *buffer = ProtocolProto::BuildLabelExchange(localDistinctValue_, sequenceId, onlineLabels, error);
    if (error != E_OK) {
        LOGE("[Linker][TriggerLabel] BuildLabel fail, error=%d", error);
        return error;
    }
    // Update waitAckSeq, Check whether new event be triggered in other thread
    {
        std::lock_guard<std::mutex> entireInfoLockGuard(entireInfoMutex_);
        if (waitAckSeq_.count(toTarget) == 0) {
            // Firstly send LabelExchange to this target
            waitAckSeq_[toTarget] = sequenceId;
        } else if (waitAckSeq_[toTarget] > sequenceId) {
            // New LabelExchangeEvent had been trigger for this target, so this event can be abort
            LOGI("[Linker][TriggerLabel] Detect newSeqId=%llu than thisSeqId=%llu be triggered for target=%s{private}",
                ULL(waitAckSeq_[toTarget]), ULL(sequenceId), toTarget.c_str());
            delete buffer;
            buffer = nullptr;
            return E_OK;
        } else {
            waitAckSeq_[toTarget] = sequenceId;
        }
    }
    // Synchronously call SendLabelExchange and hand over buffer to it
    RefObject::IncObjRef(this); // SendLabelExchange will only DecRef when total done if no need to send
    SendLabelExchange(toTarget, buffer, sequenceId, 0); // Initially retransmitCount is 0
    return E_OK;
}

int CommunicatorLinker::TriggerLabelExchangeAckEvent(const std::string &toTarget, uint64_t inSequenceId)
{
    // Build LabelExchangeAck Frame
    int error = E_OK;
    SerialBuffer *buffer = ProtocolProto::BuildLabelExchangeAck(localDistinctValue_, inSequenceId, errno);
    if (error != E_OK) {
        LOGE("[Linker][TriggerAck] BuildAck fail, error=%d", error);
        return error;
    }
    // Apply for a latest ackId and update ackTriggerId_
    uint64_t ackId;
    {
        std::lock_guard<std::mutex> entireInfoLockGuard(entireInfoMutex_);
        ackId = incAckTriggerId_.fetch_add(1, std::memory_order_seq_cst);
        ackTriggerId_[toTarget] = ackId;
    }
    // Synchronously call SendLabelExchangeAck and hand over buffer to it
    RefObject::IncObjRef(this); // SendLabelExchangeAck will only DecRef when total done if no need to send
    SendLabelExchangeAck(toTarget, buffer, inSequenceId, ackId);
    return E_OK;
}

namespace {
inline uint32_t GetDynamicTimeLapseForWaitingAck(uint32_t inRetransmitCount)
{
    if (inRetransmitCount <= RETRANSMIT_LIMIT_EQUAL_INTERVAL) {
        return TIME_LAPSE_FOR_WAITING_ACK;
    }
    uint32_t subsequentRetransmit = inRetransmitCount - RETRANSMIT_LIMIT_EQUAL_INTERVAL;
    return subsequentRetransmit * subsequentRetransmit * TIME_LAPSE_FOR_WAITING_ACK;
}
}

void CommunicatorLinker::SendLabelExchange(const std::string &toTarget, SerialBuffer *inBuff, uint64_t inSequenceId,
    uint32_t inRetransmitCount)
{
    // Check whether have the need to send
    bool noNeedToSend = ((inRetransmitCount <= RETRANSMIT_LIMIT) ? false : true);
    {
        std::lock_guard<std::mutex> entireInfoLockGuard(entireInfoMutex_);
        if (remoteOnlineTarget_.count(toTarget) == 0) {
            // Target offline
            noNeedToSend = true;
        }
        if (waitAckSeq_[toTarget] > inSequenceId) {
            // New LabelExchangeEvent had been trigger for this target, so this event can be abort
            noNeedToSend = true;
        }
        if (recvAckSeq_.count(toTarget) != 0 && recvAckSeq_[toTarget] >= inSequenceId) {
            // Ack of this sequenceId had been received or even later ack had been received
            noNeedToSend = true;
        }
        if (noNeedToSend) { // ATTENTION: This Log should be inside the protection of entireInfoLockGuard!!!
            LOGI("[Linker][SendLabel] NoNeedSend:target=%s{private}, thisSeqId=%llu, waitAckSeq=%llu, recvAckSeq=%llu,"
                "retrans=%u.", toTarget.c_str(), ULL(inSequenceId), ULL(waitAckSeq_[toTarget]),
                ULL((recvAckSeq_.count(toTarget) != 0) ? recvAckSeq_[toTarget] : ~ULL(0)), inRetransmitCount);
        } // ~0 indicate no ack ever recv
    }
    if (noNeedToSend) {
        delete inBuff;
        inBuff = nullptr;
        RefObject::DecObjRef(this); // ATTENTION: The DecObjRef should be outside entireInfoLockGuard!!!
        return;
    }

    int error = E_OK;
    SerialBuffer *cloneBuffer = inBuff->Clone(error);
    TaskConfig config{true, 0, Priority::HIGH};
    int errCode = aggregator_->CreateSendTask(toTarget, inBuff, FrameType::COMMUNICATION_LABEL_EXCHANGE, config);
    if (errCode == E_OK) {
        // Send ok, go on to wait ack, and maybe resend
        if (error == E_OK) {
            SuspendByOnceTimer([this, toTarget, cloneBuffer, inSequenceId, inRetransmitCount]() {
                // Note: toTarget and cloneBuffer and inSequenceId should be captured by value (must not by reference)
                SendLabelExchange(toTarget, cloneBuffer, inSequenceId, inRetransmitCount + 1); // Do retransmission
            }, GetDynamicTimeLapseForWaitingAck(inRetransmitCount));
        } else {
            LOGE("[Linker][SendLabel] CloneFail: target=%s{private}, SeqId=%llu.", toTarget.c_str(), ULL(inSequenceId));
        }
    } else {
        // Send fail, go on to retry send
        SuspendByOnceTimer([this, toTarget, inBuff, inSequenceId, inRetransmitCount]() {
            // Note: toTarget and inBuff and inSequenceId should be captured by value (must not by reference)
            SendLabelExchange(toTarget, inBuff, inSequenceId, inRetransmitCount); // Just do retry send
        }, TIME_LAPSE_FOR_RETRY_SEND);
        if (error == E_OK) {
            delete cloneBuffer;
            cloneBuffer = nullptr;
        }
    }
}

void CommunicatorLinker::SendLabelExchangeAck(const std::string &toTarget, SerialBuffer *inBuff,
    uint64_t inSequenceId, uint64_t inAckTriggerId)
{
    // Check whether have the need to send
    bool noNeedToSend = false;
    {
        std::lock_guard<std::mutex> entireInfoLockGuard(entireInfoMutex_);
        // Now that LabelExchange is received, LabelExchangeAck should be send no matter target online or not
        if (topRecvLabelSeq_.count(toTarget) != 0 && topRecvLabelSeq_[toTarget] > inSequenceId) {
            // topRecvLabelSeq for this target may have been erased, detect it for avoid creating an entry
            // New LabelExchange had been received for this target, so this event can be abort
            noNeedToSend = true;
        }
        if (ackTriggerId_[toTarget] > inAckTriggerId) {
            // New LabelExchangeAck had been trigger for this target, so this event can be abort
            noNeedToSend = true;
        }
        if (noNeedToSend) { // ATTENTION: This Log should be inside the protection of entireInfoLockGuard!!!
            LOGI("[Linker][SendAck] NoNeedSend:target=%s{private}, thisSeqId=%llu, topRecLabelSeq=%llu, thisAckId=%llu,"
                "ackTriggerId=%llu.", toTarget.c_str(), ULL(inSequenceId), // ~0 indacate no label ever recv
                ULL((topRecvLabelSeq_.count(toTarget) != 0) ? topRecvLabelSeq_[toTarget] : ~ULL(0)),
                ULL(inAckTriggerId), ULL(ackTriggerId_[toTarget]));
        }
    }
    if (noNeedToSend) {
        delete inBuff;
        inBuff = nullptr;
        RefObject::DecObjRef(this); // ATTENTION: The DecObjRef should be outside entireInfoLockGuard!!!
        return;
    }

    TaskConfig config{true, 0, Priority::HIGH};
    int errCode = aggregator_->CreateSendTask(toTarget, inBuff, FrameType::COMMUNICATION_LABEL_EXCHANGE_ACK, config);
    if (errCode == E_OK) {
        // Send ok, finish event
        RefObject::DecObjRef(this); // ATTENTION: The DecObjRef should be outside entireInfoLockGuard!!!
    } else {
        // Send fail, go on to retry send
        SuspendByOnceTimer([this, toTarget, inBuff, inSequenceId, inAckTriggerId]() {
            // Note: toTarget, inBuff, inSequenceId, inAckTriggerId should be captured by value (must not by reference)
            SendLabelExchangeAck(toTarget, inBuff, inSequenceId, inAckTriggerId);
        }, TIME_LAPSE_FOR_RETRY_SEND);
    }
}

DEFINE_OBJECT_TAG_FACILITIES(CommunicatorLinker)
}
