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

#include "syncer_proxy.h"

#include "syncer_factory.h"
#include "db_errno.h"
#include "log_print.h"

namespace DistributedDB {
SyncerProxy::SyncerProxy()
    : syncer_(nullptr)
{
}

int SyncerProxy::Initialize(IKvDBSyncInterface *syncInterface)
{
    if (syncInterface == nullptr) {
        return -E_INVALID_ARGS;
    }

    int interfaceType = syncInterface->GetInterfaceType();
    {
        std::lock_guard<std::mutex> lock(syncerLock_);
        if (syncer_ == nullptr) {
            syncer_ = SyncerFactory::GetSyncer(interfaceType);
        }
    }
    if (syncer_ == nullptr) {
        LOGF("syncer create failed! invalid interface type %d", interfaceType);
        return -E_OUT_OF_MEMORY;
    }

    return syncer_->Initialize(syncInterface);
}

int SyncerProxy::Close()
{
    if (syncer_ == nullptr) {
        return -E_NOT_INIT;
    }
    return syncer_->Close();
}

int SyncerProxy::Sync(const std::vector<std::string> &devices, int mode,
    const std::function<void(const std::map<std::string, int> &)> &onComplete,
    const std::function<void(void)> &onFinalize, bool wait = false)
{
    if (syncer_ == nullptr) {
        return -E_NOT_INIT;
    }
    return syncer_->Sync(devices, mode, onComplete, onFinalize, wait);
}

int SyncerProxy::RemoveSyncOperation(int syncId)
{
    if (syncer_ == nullptr) {
        return -E_NOT_INIT;
    }
    return syncer_->RemoveSyncOperation(syncId);
}

uint64_t SyncerProxy::GetTimeStamp()
{
    if (syncer_ == nullptr) {
        return SyncerFactory::GetSyncer(IKvDBSyncInterface::SYNC_SVD)->GetTimeStamp();
    }
    return syncer_->GetTimeStamp();
}

void SyncerProxy::EnableAutoSync(bool enable)
{
    if (syncer_ == nullptr) {
        return;
    }
    syncer_->EnableAutoSync(enable);
}

int SyncerProxy::EraseDeviceWaterMark(const std::string &deviceId, bool isNeedHash)
{
    if (syncer_ == nullptr) {
        return SyncerFactory::GetSyncer(IKvDBSyncInterface::SYNC_SVD)->EraseDeviceWaterMark(deviceId, isNeedHash);
    }
    return syncer_->EraseDeviceWaterMark(deviceId, isNeedHash);
}

void SyncerProxy::LocalDataChanged(int notifyEvent)
{
    if (syncer_ == nullptr) {
        return;
    }
    syncer_->LocalDataChanged(notifyEvent);
}

int SyncerProxy::GetQueuedSyncSize(int *queuedSyncSize) const
{
    if (syncer_ == nullptr) {
        return -E_NOT_INIT;
    }
    return syncer_->GetQueuedSyncSize(queuedSyncSize);
}

int SyncerProxy::SetQueuedSyncLimit(const int *queuedSyncLimit)
{
    if (syncer_ == nullptr) {
        return -E_NOT_INIT;
    }
    return syncer_->SetQueuedSyncLimit(queuedSyncLimit);
}

int SyncerProxy::GetQueuedSyncLimit(int *queuedSyncLimit) const
{
    if (syncer_ == nullptr) {
        return -E_NOT_INIT;
    }
    return syncer_->GetQueuedSyncLimit(queuedSyncLimit);
}

int SyncerProxy::DisableManualSync(void)
{
    if (syncer_ == nullptr) {
        return -E_NOT_INIT;
    }
    return syncer_->DisableManualSync();
}

int SyncerProxy::EnableManualSync(void)
{
    if (syncer_ == nullptr) {
        return -E_NOT_INIT;
    }
    return syncer_->EnableManualSync();
}

int SyncerProxy::GetLocalIdentity(std::string &outTarget) const
{
    if (syncer_ == nullptr) {
        return -E_NOT_INIT;
    }
    return syncer_->GetLocalIdentity(outTarget);
}

int SyncerProxy::SetStaleDataWipePolicy(WipePolicy policy)
{
    if (syncer_ == nullptr) {
        return -E_NOT_INIT;
    }
    return syncer_->SetStaleDataWipePolicy(policy);
}
} // namespace DistributedDB
