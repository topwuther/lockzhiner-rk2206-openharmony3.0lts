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

#include "sync_engine.h"

#include <functional>
#include <deque>
#include <algorithm>

#include "isync_state_machine.h"
#include "db_errno.h"
#include "log_print.h"
#include "hash.h"
#include "runtime_context.h"
#include "time_sync.h"
#ifndef OMIT_MULTI_VER
#include "commit_history_sync.h"
#include "multi_ver_data_sync.h"
#include "value_slice_sync.h"
#endif
#include "single_ver_data_sync.h"
#include "ability_sync.h"
#include "device_manager.h"
#include "db_common.h"

namespace DistributedDB {
int SyncEngine::queueCacheSize_ = 0;
int SyncEngine::maxQueueCacheSize_ = DEFAULT_CACHE_SIZE;
unsigned int SyncEngine::discardMsgNum_ = 0;
std::mutex SyncEngine::queueLock_;

SyncEngine::SyncEngine()
    : syncInterface_(nullptr),
      communicator_(nullptr),
      deviceManager_(nullptr),
      metadata_(nullptr),
      timeChangedListener_(nullptr),
      execTaskCount_(0)
{
}

SyncEngine::~SyncEngine()
{
    LOGD("[SyncEngine] ~SyncEngine!");
    if (syncInterface_ != nullptr) {
        syncInterface_->DecRefCount();
        syncInterface_ = nullptr;
    }
    if (deviceManager_ != nullptr) {
        delete deviceManager_;
        deviceManager_ = nullptr;
    }
    if (timeChangedListener_ != nullptr) {
        timeChangedListener_->Drop(true);
        timeChangedListener_ = nullptr;
    }
    communicator_ = nullptr;
    metadata_ = nullptr;
    LOGD("[SyncEngine] ~SyncEngine ok!");
}

int SyncEngine::Initialize(IKvDBSyncInterface *syncInterface, std::shared_ptr<Metadata> &metadata,
    const std::function<void(std::string)> &onRemoteDataChanged)
{
    if ((syncInterface == nullptr) || (metadata == nullptr)) {
        return -E_INVALID_ARGS;
    }
    syncInterface_ = syncInterface;
    int errCode = InitComunicator(syncInterface);
    if (errCode != E_OK) {
        LOGE("[SyncEngine] Init Communicator failed");
        // There need to set nullptr. other wise, syncInterface will be
        // DecRef in th destroy-method.
        syncInterface_ = nullptr;
        return errCode;
    }
    onRemoteDataChanged_ = onRemoteDataChanged;
    errCode = InitDeviceManager(onRemoteDataChanged);
    if (errCode != E_OK) {
        // reset ptr if initialize device manager failed
        syncInterface_ = nullptr;
        return errCode;
    }

    metadata_ = metadata;
    timeChangedListener_ = RuntimeContext::GetInstance()->RegisterTimeChangedLister(
        [this](void *changedOffset) {
            if (changedOffset == nullptr) {
                return;
            }
            TimeOffset changedTimeOffset = *(reinterpret_cast<TimeOffset *>(changedOffset)) *
                static_cast<TimeOffset>(TimeHelper::TO_100_NS);
            TimeOffset orgOffset = this->metadata_->GetLocalTimeOffset() - changedTimeOffset;
            TimeStamp currentSysTime = TimeHelper::GetSysCurrentTime();
            TimeStamp maxItemTime = 0;
            this->syncInterface_->GetMaxTimeStamp(maxItemTime);
            if ((currentSysTime + orgOffset) <= maxItemTime) {
                orgOffset = maxItemTime - currentSysTime + TimeHelper::MS_TO_100_NS; // 1ms
            }
            this->metadata_->SaveLocalTimeOffset(orgOffset);
        }, errCode);
    if (timeChangedListener_ == nullptr) {
        LOGE("[SyncEngine] Init RegisterTimeChangedLister failed");
        return errCode;
    }
    LOGI("[SyncEngine] Engine init ok");
    return E_OK;
}

int SyncEngine::Close()
{
    LOGI("[SyncEngine] SyncEngine close enter!");
    if (timeChangedListener_ != nullptr) {
        timeChangedListener_->Drop(true);
        timeChangedListener_ = nullptr;
    }
    if (communicator_ != nullptr) {
        communicator_->RegOnMessageCallback(nullptr, nullptr);
        communicator_->RegOnConnectCallback(nullptr, nullptr);
        communicator_->RegOnSendableCallback(nullptr, nullptr);
    }
    // Clear SyncContexts
    {
        std::unique_lock<std::mutex> lock(contextMapLock_);
        for (auto &iter : syncTaskContextMap_) {
            ISyncTaskContext *tempContext = iter.second;
            iter.second = nullptr;
            lock.unlock();
            RefObject::KillAndDecObjRef(tempContext);
            tempContext = nullptr;
            lock.lock();
        }
    }
    if (communicator_ != nullptr) {
        ICommunicatorAggregator *communicatorAggregator = nullptr;
        int errCode = RuntimeContext::GetInstance()->GetCommunicatorAggregator(communicatorAggregator);
        if (communicatorAggregator == nullptr) {
            LOGF("[SyncEngine] ICommunicatorAggregator get failed when fialize SyncEngine err %d", errCode);
            return errCode;
        }
        communicatorAggregator->ReleaseCommunicator(communicator_);
        communicator_ = nullptr;
    }

    std::lock_guard<std::mutex> msgLock(queueLock_);
    while (!msgQueue_.empty()) {
        Message *inMsg = msgQueue_.front();
        msgQueue_.pop_front();
        if (inMsg != nullptr) {
            queueCacheSize_ -= GetMsgSize(inMsg);
            delete inMsg;
        }
    }

    LOGI("[SyncEngine] SyncEngine closed!");
    return E_OK;
}

int SyncEngine::AddSyncOperation(SyncOperation *operation)
{
    if (operation == nullptr) {
        return -E_INVALID_ARGS;
    }

    std::vector<std::string> devices = operation->GetDevices();
    for (const auto &deviceId : devices) {
        int checkErrCode = RunPermissionCheck(deviceId, operation->GetMode());
        if (checkErrCode == E_OK) {
            operation->SetStatus(deviceId, SyncOperation::WAITING);
            int errCode = AddSyncOperForContext(deviceId, operation);
            if (errCode != E_OK) {
                operation->SetStatus(deviceId, SyncOperation::FAILED);
            }
        } else {
            operation->SetStatus(deviceId, SyncOperation::PERMISSION_CHECK_FAILED);
        }
    }
    return E_OK;
}

void SyncEngine::RemoveSyncOperation(int syncId)
{
    std::lock_guard<std::mutex> lock(contextMapLock_);
    for (auto &iter : syncTaskContextMap_) {
        ISyncTaskContext *context = iter.second;
        if (context != nullptr) {
            context->RemoveSyncOperation(syncId);
        }
    }
}

void SyncEngine::BroadCastDataChanged() const
{
    if (deviceManager_ != nullptr) {
        (void)deviceManager_->SendBroadCast(LOCAL_DATA_CHANGED);
    }
}

void SyncEngine::RegConnectCallback()
{
    if (communicator_ == nullptr) {
        LOGE("[SyncEngine][RegConnCB] communicator is not set!");
        return;
    }
    LOGD("[SyncEngine] RegOnConnectCallback");
    int errCode = communicator_->RegOnConnectCallback(
        std::bind(&DeviceManager::OnDeviceConnectCallback, deviceManager_,
            std::placeholders::_1, std::placeholders::_2), nullptr);
    if (errCode != E_OK) {
        LOGE("[SyncEngine][RegConnCB] register failed, auto sync can not use! err %d", errCode);
        return;
    }
    communicator_->Activate();
}

void SyncEngine::GetOnlineDevices(std::vector<std::string> &devices) const
{
    devices.clear();
    if (deviceManager_ != nullptr) {
        deviceManager_->GetOnlineDevices(devices);
    }
}

int SyncEngine::InitDeviceManager(const std::function<void(std::string)> &onRemoteDataChanged)
{
    deviceManager_ = new (std::nothrow) DeviceManager();
    if (deviceManager_ == nullptr) {
        LOGE("[SyncEngine] deviceManager alloc failed!");
        return -E_OUT_OF_MEMORY;
    }

    int errCode = deviceManager_->Initialize(communicator_, onRemoteDataChanged);
    if (errCode != E_OK) {
        LOGE("[SyncEngine] deviceManager init failed! err %d", errCode);
        delete deviceManager_;
        deviceManager_ = nullptr;
        return errCode;
    }
    return E_OK;
}

int SyncEngine::InitComunicator(const IKvDBSyncInterface *syncInterface)
{
    ICommunicatorAggregator *communicatorAggregator = nullptr;
    int errCode = RuntimeContext::GetInstance()->GetCommunicatorAggregator(communicatorAggregator);
    if (communicatorAggregator == nullptr) {
        LOGE("[SyncEngine] Get ICommunicatorAggregator error when init the sync engine err = %d", errCode);
        return errCode;
    }
    std::vector<uint8_t> label = syncInterface->GetIdentifier();
    communicator_ = communicatorAggregator->AllocCommunicator(label, errCode);
    if (communicator_ == nullptr) {
        LOGE("[SyncEngine] AllocCommunicator error when init the sync engine! err = %d", errCode);
        return errCode;
    }

    errCode = communicator_->RegOnMessageCallback(
        std::bind(&SyncEngine::MessageReceiveCallback, this, std::placeholders::_1, std::placeholders::_2),
        []() {});
    if (errCode != E_OK) {
        LOGE("[SyncEngine] SyncRequestCallback register failed! err = %d", errCode);
        communicatorAggregator->ReleaseCommunicator(communicator_);
        communicator_ = nullptr;
        return errCode;
    }
    label.resize(3); // only show 3 Bytes enough
    label_ = DBCommon::VectorToHexString(label);
    LOGD("[SyncEngine] RegOnConnectCallback");
    return errCode;
}

int SyncEngine::AddSyncOperForContext(const std::string &deviceId, SyncOperation *operation)
{
    int errCode = E_OK;
    ISyncTaskContext *context = nullptr;
    {
        std::lock_guard<std::mutex> lock(contextMapLock_);
        context = FindSyncTaskContext(deviceId);
        if (context == nullptr) {
            if (!IsKilled()) {
                context = GetSyncTaskContext(deviceId, errCode);
            }
            if (context == nullptr) {
                return errCode;
            }
        }
        if (context->IsKilled()) {
            return -E_OBJ_IS_KILLED;
        }
        // IncRef for SyncEngine to make sure context is valid, to avoid a big lock
        RefObject::IncObjRef(context);
    }

    errCode = context->AddSyncOperation(operation);
    RefObject::DecObjRef(context);
    return errCode;
}

void SyncEngine::MessageReceiveCallbackTask(ISyncTaskContext *context, const ICommunicator *communicator,
    Message *inMsg)
{
    std::string deviceId = context->GetDeviceId();
    if (inMsg == nullptr) {
        LOGE("[SyncEngine] MessageReceiveCallback inMsg is null!");
        goto MSG_CALLBACK_OUT;
    }

    // deal remote local data changed message
    if (inMsg->GetMessageId() == LOCAL_DATA_CHANGED) {
        if (onRemoteDataChanged_ && deviceManager_->IsDeviceOnline(deviceId)) {
            onRemoteDataChanged_(deviceId);
        } else {
            LOGE("[SyncEngine] onRemoteDataChanged is null!");
        }
    } else { // others message, it should be dealt by context
        int errCode = context->ReceiveMessageCallback(inMsg);
        if (errCode == -E_NOT_NEED_DELETE_MSG) {
            goto MSG_CALLBACK_OUT_NOT_DEL;
        }
    }

MSG_CALLBACK_OUT:
    delete inMsg;
    inMsg = nullptr;
MSG_CALLBACK_OUT_NOT_DEL:
    (void)DealMsgUtilQueueEmpty();
    {
        std::lock_guard<std::mutex> lock(queueLock_);
        execTaskCount_--;
    }
    RefObject::DecObjRef(communicator);
    RefObject::DecObjRef(context);
    communicator = nullptr;
    context = nullptr;
}

int SyncEngine::DealMsgUtilQueueEmpty()
{
    int errCode = E_OK;
    Message *inMsg = nullptr;
    {
        std::lock_guard<std::mutex> lock(queueLock_);
        if (msgQueue_.empty()) {
            return errCode;
        }
        inMsg = msgQueue_.front();
        msgQueue_.pop_front();
        queueCacheSize_ -= GetMsgSize(inMsg);
    }

    // it will deal with the first message in queue, we should increase object reference counts and sure that resources
    // could be prevented from destroying by other threads.
    ISyncTaskContext *nextContext = GetConextForMsg(inMsg->GetTarget(), errCode);
    if (errCode != E_OK) {
        delete inMsg;
        inMsg = nullptr;
        return errCode;
    }
    errCode = ScheduleDealMsg(nextContext, inMsg);
    if (errCode != E_OK) {
        RefObject::DecObjRef(nextContext);
        delete inMsg;
        inMsg = nullptr;
    }
    return errCode;
}

ISyncTaskContext *SyncEngine::GetConextForMsg(const std::string &targetDev, int &errCode)
{
    ISyncTaskContext *context = nullptr;
    {
        std::lock_guard<std::mutex> lock(contextMapLock_);
        context = FindSyncTaskContext(targetDev);
        if (context != nullptr) {
            if (context->IsKilled()) {
                errCode = -E_OBJ_IS_KILLED;
                return nullptr;
            }
        } else {
            if (IsKilled()) {
                errCode = -E_OBJ_IS_KILLED;
                return nullptr;
            }
            context = GetSyncTaskContext(targetDev, errCode);
            if (context == nullptr) {
                return nullptr;
            }
        }
        // IncRef for context to make sure context is valid, when task run another thread
        RefObject::IncObjRef(context);
    }
    return context;
}

int SyncEngine::ScheduleDealMsg(ISyncTaskContext *context, Message *inMsg)
{
    RefObject::IncObjRef(communicator_);
    {
        std::lock_guard<std::mutex> incLock(queueLock_);
        execTaskCount_++;
    }
    int errCode = RuntimeContext::GetInstance()->ScheduleTask(std::bind(&SyncEngine::MessageReceiveCallbackTask,
        this, context, communicator_, inMsg));
    if (errCode != E_OK) {
        LOGE("[SyncEngine] MessageReceiveCallbackTask Schedule failed err %d", errCode);
        RefObject::DecObjRef(communicator_);
        {
            std::lock_guard<std::mutex> decLock(queueLock_);
            execTaskCount_--;
        }
    }
    return errCode;
}

void SyncEngine::MessageReceiveCallback(const std::string &targetDev, Message *inMsg)
{
    int errCode = MessageReceiveCallbackInner(targetDev, inMsg);
    if (errCode != E_OK) {
        delete inMsg;
        inMsg = nullptr;
        LOGE("[SyncEngine] MessageReceiveCallback failed!");
    }
}

int SyncEngine::MessageReceiveCallbackInner(const std::string &targetDev, Message *inMsg)
{
    if (targetDev.empty() || inMsg == nullptr) {
        LOGE("[SyncEngine][MessageReceiveCallback] from a invalid device or inMsg is null ");
        return -E_INVALID_ARGS;
    }

    int msgSize = GetMsgSize(inMsg);
    if (msgSize <= 0) {
        LOGE("[SyncEngine] GetMsgSize makes a mistake");
        return -E_NOT_SUPPORT;
    }

    {
        std::lock_guard<std::mutex> lock(queueLock_);
        if ((queueCacheSize_ + msgSize) > maxQueueCacheSize_) {
            LOGE("[SyncEngine] The size of message queue is beyond maximum");
            discardMsgNum_++;
            return -E_BUSY;
        }

        if (execTaskCount_ >= MAX_EXEC_NUM) {
            PutMsgIntoQueue(targetDev, inMsg, msgSize);
            return E_OK;
        }
    }

    int errCode = E_OK;
    ISyncTaskContext *nextContext = GetConextForMsg(targetDev, errCode);
    if (errCode != E_OK) {
        return errCode;
    }

    LOGD("[SyncEngine] MessageReceiveCallback MSG ID = %d", inMsg->GetMessageId());
    return ScheduleDealMsg(nextContext, inMsg);
}

void SyncEngine::PutMsgIntoQueue(const std::string &targetDev, Message *inMsg, int msgSize)
{
    if (inMsg->GetMessageId() == LOCAL_DATA_CHANGED) {
        auto iter = std::find_if(msgQueue_.begin(), msgQueue_.end(),
            [&targetDev](const Message *msg) {
                return targetDev == msg->GetTarget() && msg->GetMessageId() == LOCAL_DATA_CHANGED;
            });
        if (iter != msgQueue_.end()) {
            delete inMsg;
            return;
        }
    }
    inMsg->SetTarget(targetDev);
    msgQueue_.push_back(inMsg);
    queueCacheSize_ += msgSize;
    LOGE("[SyncEngine] The quantity of executing threads is beyond maximum. msgQueueSize = %d", msgQueue_.size());
}

int SyncEngine::GetMsgSize(const Message *inMsg) const
{
    switch (inMsg->GetMessageId()) {
        case TIME_SYNC_MESSAGE:
            return TimeSync::CalculateLen(inMsg);
        case ABILITY_SYNC_MESSAGE:
            return AbilitySync::CalculateLen(inMsg);
        case DATA_SYNC_MESSAGE:
            return SingleVerDataSync::CalculateLen(inMsg);
#ifndef OMIT_MULTI_VER
        case COMMIT_HISTORY_SYNC_MESSAGE:
            return CommitHistorySync::CalculateLen(inMsg);
        case MULTI_VER_DATA_SYNC_MESSAGE:
            return MultiVerDataSync::CalculateLen(inMsg);
        case VALUE_SLICE_SYNC_MESSAGE:
            return ValueSliceSync::CalculateLen(inMsg);
#endif
        case LOCAL_DATA_CHANGED:
            return DeviceManager::CalculateLen();
        default:
            LOGE("[SyncEngine] GetMsgSize not support msgId:%u", inMsg->GetMessageId());
            return -E_NOT_SUPPORT;
    }
}

ISyncTaskContext *SyncEngine::FindSyncTaskContext(const std::string &deviceId)
{
    auto iter = syncTaskContextMap_.find(deviceId);
    if (iter != syncTaskContextMap_.end()) {
        ISyncTaskContext *context = iter->second;
        return context;
    }
    return nullptr;
}

ISyncTaskContext *SyncEngine::GetSyncTaskContext(const std::string &deviceId, int &errCode)
{
    ISyncTaskContext *context = CreateSyncTaskContext();
    if (context == nullptr) {
        errCode = -E_OUT_OF_MEMORY;
        LOGE("[SyncEngine] SyncTaskContext alloc failed, may be no memory avliad!");
        return nullptr;
    }
    errCode = context->Initialize(deviceId, syncInterface_, metadata_, communicator_);
    if (errCode != E_OK) {
        LOGE("[SyncEngine] context init failed err %d, dev %s{private}", errCode, deviceId.c_str());
        RefObject::DecObjRef(context);
        context = nullptr;
        return nullptr;
    }
    syncTaskContextMap_.insert(std::pair<std::string, ISyncTaskContext *>(deviceId, context));
    // IncRef for SyncEngine to make sure SyncEngine is valid when context access
    RefObject::IncObjRef(this);
    context->OnLastRef([this, deviceId]() {
        LOGD("[SyncEngine] SyncTaskContext for id %s{private} finalized", deviceId.c_str());
        RefObject::DecObjRef(this);
    });
    context->RegOnSyncTask(std::bind(&SyncEngine::ExecSyncTask, this, context));
    return context;
}

int SyncEngine::ExecSyncTask(ISyncTaskContext *context)
{
    if (IsKilled()) {
        return -E_OBJ_IS_KILLED;
    }

    AutoLock lockGuard(context);
    int status = context->GetTaskExecStatus();
    if ((status == SyncTaskContext::RUNNING) || context->IsKilled()) {
        return -E_NOT_SUPPORT;
    }
    context->SetTaskExecStatus(ISyncTaskContext::RUNNING);
    if (!context->IsTargetQueueEmpty()) {
        context->MoveToNextTarget();
        context->UnlockObj();
        int errCode = context->StartStateMachine();
        context->LockObj();
        if (errCode != E_OK) {
            LOGE("[SyncEngine] machine StartSync failed");
            context->SetOperationStatus(SyncOperation::FAILED);
            return errCode;
        }
    } else {
        LOGD("[SyncEngine] ExecSyncTask finished");
        context->SetTaskExecStatus(ISyncTaskContext::FINISHED);
    }
    return E_OK;
}

int SyncEngine::GetQueueCacheSize() const
{
    return queueCacheSize_;
}

unsigned int SyncEngine::GetDiscardMsgNum() const
{
    return discardMsgNum_;
}

unsigned int SyncEngine::GetMaxExecNum() const
{
    return MAX_EXEC_NUM;
}

void SyncEngine::SetMaxQueueCacheSize(int value)
{
    maxQueueCacheSize_ = value;
}

int SyncEngine::GetLocalIdentity(std::string &outTarget) const
{
    if (communicator_ == nullptr) {
        LOGE("[SyncEngine]  communicator_ is nullptr, return!");
        return -E_NOT_INIT;
    }
    std::string deviceId;
    int errCode = communicator_->GetLocalIdentity(deviceId);
    if (errCode != E_OK) {
        LOGE("[SyncEngine]  communicator_ GetLocalIdentity fail errCode:%d", errCode);
        return errCode;
    }
    outTarget = DBCommon::TransferHashString(deviceId);
    return E_OK;
}

int SyncEngine::RunPermissionCheck(const std::string &deviceId, int mode) const
{
    uint8_t flag = 0;
    if (mode == SyncOperation::PUSH) {
        flag = CHECK_FLAG_SEND;
    } else if (mode == SyncOperation::PULL) {
        flag = CHECK_FLAG_RECEIVE;
    } else if (mode == SyncOperation::PUSH_AND_PULL) {
        flag = CHECK_FLAG_SEND | CHECK_FLAG_RECEIVE;
    }

    std::string appId = syncInterface_->GetDbProperties().GetStringProp(KvDBProperties::APP_ID, "");
    std::string userId = syncInterface_->GetDbProperties().GetStringProp(KvDBProperties::USER_ID, "");
    std::string storeId = syncInterface_->GetDbProperties().GetStringProp(KvDBProperties::STORE_ID, "");
    int errCode = RuntimeContext::GetInstance()->RunPermissionCheck(userId, appId, storeId, deviceId, flag);
    if (errCode != E_OK) {
        LOGE("[SyncEngine] RunPermissionCheck not pass errCode:%d, flag:%d, %s{private} Label=%s",
            errCode, flag, deviceId.c_str(), label_.c_str());
    }
    return errCode;
}

std::string SyncEngine::GetLabel() const
{
    return label_;
}
} // namespace DistributedDB
