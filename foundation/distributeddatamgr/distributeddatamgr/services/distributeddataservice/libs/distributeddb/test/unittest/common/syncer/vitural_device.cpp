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
#include "vitural_device.h"

#include <algorithm>
#include <thread>

#include "virtual_single_ver_sync_db_Interface.h"
#include "virtual_multi_ver_sync_db_interface.h"
#include "vitural_communicator.h"
#include "vitural_communicator_aggregator.h"
#include "single_ver_sync_state_machine.h"
#include "multi_ver_sync_state_machine.h"
#include "device_manager.h"
#include "log_print.h"

namespace DistributedDB {
VituralDevice::VituralDevice(const std::string &deviceId)
    : communicateHandle_(nullptr),
      storage_(nullptr),
      metadata_(nullptr),
      deviceId_(deviceId)
{
}

VituralDevice::~VituralDevice()
{
    std::mutex cvMutex;
    std::condition_variable cv;
    bool finished = false;
    Offline();

    if (communicateHandle_ != nullptr) {
        communicateHandle_->RegOnMessageCallback(nullptr, nullptr);
        communicatorAggregator_->ReleaseCommunicator(communicateHandle_);
        communicateHandle_ = nullptr;
    }
    communicatorAggregator_ = nullptr;

    if (context_ != nullptr) {
        IKvDBSyncInterface *storage = storage_;
        context_->OnLastRef([storage, &cv, &cvMutex, &finished]() {
            if (storage != nullptr) {
                delete storage;
            }
            {
                std::lock_guard<std::mutex> lock(cvMutex);
                finished = true;
            }
            cv.notify_one();
        });
        RefObject::KillAndDecObjRef(context_);
        std::unique_lock<std::mutex> lock(cvMutex);
        cv.wait(lock, [&finished] {return finished;});
    } else {
        delete storage_;
    }
    context_ = nullptr;
    metadata_ = nullptr;
    storage_ = nullptr;
}

int VituralDevice::Initialize(VirtualCommunicatorAggregator *communicatorAggregator, IKvDBSyncInterface *syncInterface)
{
    if ((communicatorAggregator == nullptr) || (syncInterface == nullptr)) {
        return -E_INVALID_ARGS;
    }

    communicatorAggregator_ = communicatorAggregator;
    int errCode = E_OK;
    communicateHandle_ = communicatorAggregator_->AllocCommunicator(deviceId_, errCode);
    if (communicateHandle_ == nullptr) {
        return errCode;
    }

    storage_ = syncInterface;
    metadata_ = std::make_shared<Metadata>();
    if (metadata_->Initialize(storage_) != E_OK) {
        LOGE("metadata_ init failed");
        return -E_NOT_SUPPORT;
    }

    if (storage_->GetInterfaceType() == IKvDBSyncInterface::SYNC_SVD) {
        context_ = new (std::nothrow) SingleVerSyncTaskContext;
    } else {
        context_ = new (std::nothrow) MultiVerSyncTaskContext;
    }
    if (context_ == nullptr) {
        return -E_OUT_OF_MEMORY;
    }
    communicateHandle_->RegOnMessageCallback(std::bind(&VituralDevice::MessageCallback, this,
        std::placeholders::_1, std::placeholders::_2), []() {});
    context_->Initialize(remoteDeviceId_, storage_, metadata_, communicateHandle_);
    context_->SetRetryStatus(SyncTaskContext::NO_NEED_RETRY);
    context_->RegOnSyncTask(std::bind(&VituralDevice::StartResponseTask, this));
    return E_OK;
}

void VituralDevice::SetDeviceId(const std::string &deviceId)
{
    deviceId_ = deviceId;
}

std::string VituralDevice::GetDeviceId() const
{
    return deviceId_;
}

int VituralDevice::GetData(const Key &key, VirtualDataItem &item)
{
    VirtualSingleVerSyncDBInterface *syncAble = static_cast<VirtualSingleVerSyncDBInterface *>(storage_);
    return syncAble->GetSyncData(key, item);
}

int VituralDevice::GetData(const Key &key, Value &value)
{
    VirtualMultiVerSyncDBInterface *syncInterface = static_cast<VirtualMultiVerSyncDBInterface *>(storage_);
    return syncInterface->GetData(key, value);
}

int VituralDevice::PutData(const Key &key, const Value &value, const TimeStamp &time, int flag)
{
    VirtualSingleVerSyncDBInterface *syncAble = static_cast<VirtualSingleVerSyncDBInterface *>(storage_);
    LOGI("dev %s put data time %llu", deviceId_.c_str(), time);
    return syncAble->PutData(key, value, time, flag);
}

int VituralDevice::PutData(const Key &key, const Value &value)
{
    VirtualMultiVerSyncDBInterface *syncInterface = static_cast<VirtualMultiVerSyncDBInterface *>(storage_);
    return syncInterface->PutData(key, value);
}

int VituralDevice::DeleteData(const Key &key)
{
    VirtualMultiVerSyncDBInterface *syncInterface = static_cast<VirtualMultiVerSyncDBInterface *>(storage_);
    return syncInterface->DeleteData(key);
}

int VituralDevice::StartTransaction()
{
    VirtualMultiVerSyncDBInterface *syncInterface = static_cast<VirtualMultiVerSyncDBInterface *>(storage_);
    return syncInterface->StartTransaction();
}

int VituralDevice::Commit()
{
    VirtualMultiVerSyncDBInterface *syncInterface = static_cast<VirtualMultiVerSyncDBInterface *>(storage_);
    return syncInterface->Commit();
}

int VituralDevice::MessageCallback(const std::string &deviceId, Message *inMsg)
{
    if (inMsg->GetMessageId() == LOCAL_DATA_CHANGED) {
        if (onRemoteDataChanged_) {
            onRemoteDataChanged_(deviceId);
            delete inMsg;
            inMsg = nullptr;
            return E_OK;
        }
        delete inMsg;
        inMsg = nullptr;
        return -E_INVALID_ARGS;
    }

    LOGD("[VituralDevice] onMessag, src %s", deviceId.c_str());
    RefObject::IncObjRef(context_);
    RefObject::IncObjRef(communicateHandle_);
    SyncTaskContext *context = context_;
    ICommunicator *communicateHandle = communicateHandle_;
    std::thread thread([context, communicateHandle, inMsg]() {
        int errCode = context->ReceiveMessageCallback(inMsg);
        if (errCode != -E_NOT_NEED_DELETE_MSG) {
            delete inMsg;
        }
        RefObject::DecObjRef(context);
        RefObject::DecObjRef(communicateHandle);
    });
    thread.detach();
    return E_OK;
}

void VituralDevice::OnRemoteDataChanged(const std::function<void(const std::string &)> &callback)
{
    onRemoteDataChanged_ = callback;
}

void VituralDevice::Online()
{
    static_cast<VirtualCommunicator *>(communicateHandle_)->Enable();
    communicatorAggregator_->OnlineDevice(deviceId_);
}

void VituralDevice::Offline()
{
    static_cast<VirtualCommunicator *>(communicateHandle_)->Disable();
    communicatorAggregator_->OfflineDevice(deviceId_);
}

int VituralDevice::StartResponseTask()
{
    LOGD("[VituralDevice] StartResponseTask");
    RefObject::AutoLock lockGuard(context_);
    int status = context_->GetTaskExecStatus();
    if ((status == SyncTaskContext::RUNNING) || context_->IsKilled()) {
        LOGD("[VituralDevice] StartResponseTask status:%d", status);
        return -E_NOT_SUPPORT;
    }
    if (context_->IsTargetQueueEmpty()) {
        LOGD("[VituralDevice] StartResponseTask IsTargetQueueEmpty is empty");
        return E_OK;
    }
    context_->SetTaskExecStatus(ISyncTaskContext::RUNNING);
    context_->MoveToNextTarget();
    LOGI("[VituralDevice] machine StartSync");
    context_->UnlockObj();
    int errCode = context_->StartStateMachine();
    context_->LockObj();
    if (errCode != E_OK) {
        LOGE("[VituralDevice] machine StartSync failed");
        context_->SetOperationStatus(SyncOperation::FAILED);
    }
    return errCode;
}

TimeOffset VituralDevice::GetLocalTimeOffset() const
{
    return metadata_->GetLocalTimeOffset();
}

void VituralDevice::SetSaveDataDelayTime(uint64_t milliDelayTime)
{
    VirtualSingleVerSyncDBInterface *syncInterface = static_cast<VirtualSingleVerSyncDBInterface *>(storage_);
    syncInterface->SetSaveDataDelayTime(milliDelayTime);
}

int VituralDevice::Sync(SyncMode mode, bool wait)
{
    auto operation = new (std::nothrow) SyncOperation(1, {remoteDeviceId_}, mode, nullptr, wait);
    if (operation == nullptr) {
        return -E_OUT_OF_MEMORY;
    }
    operation->Initialize();
    operation->SetOnSyncFinished([operation](int id) {
        operation->NotifyIfNeed();
        RefObject::KillAndDecObjRef(operation);
    });
    context_->AddSyncOperation(operation);
    operation->WaitIfNeed();
    return E_OK;
}
} // namespace DistributedDB