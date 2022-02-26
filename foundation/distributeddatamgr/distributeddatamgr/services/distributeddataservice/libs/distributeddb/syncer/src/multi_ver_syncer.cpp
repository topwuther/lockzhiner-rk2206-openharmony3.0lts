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

#ifndef OMIT_MULTI_VER
#include "multi_ver_syncer.h"

#include <functional>
#include <mutex>
#include <map>

#include "multi_ver_sync_engine.h"
#include "multi_ver_kvdb_sync_interface.h"
#include "log_print.h"

namespace DistributedDB {
MultiVerSyncer::MultiVerSyncer()
    : autoSyncEnable_(true)
{
}

MultiVerSyncer::~MultiVerSyncer()
{
}

void MultiVerSyncer::EnableAutoSync(bool enable)
{
    LOGD("[Syncer] EnableAutoSync enable = %d", enable);
    if (autoSyncEnable_ == enable) {
        return;
    }

    autoSyncEnable_ = enable;
    if (!enable || (syncEngine_ == nullptr)) {
        return;
    }

    std::vector<std::string> devices;
    GetOnlineDevices(devices);
    if (devices.empty()) {
        return;
    }

    int syncId = Sync(devices, SyncOperation::AUTO_PULL, nullptr, nullptr, false);
    if (syncId < MIN_VALID_SYNC_ID) {
        LOGE("[Syncer] sync start by EnableAutoSync failed err %d", syncId);
    }
}

int MultiVerSyncer::EraseDeviceWaterMark(const std::string &deviceId, bool isNeedHash)
{
    return -E_NOT_SUPPORT;
}

void MultiVerSyncer::LocalDataChanged(int notifyEvent)
{
    if (!initialized_) {
        LOGE("[Syncer] Syncer has not Init");
        return;
    }

    if (!autoSyncEnable_) {
        return;
    }

    syncEngine_->BroadCastDataChanged();
}

void MultiVerSyncer::RemoteDataChanged(const std::string &device)
{
    LOGD("[MultiVerSyncer] Remote data changed or device online dev %s{private}", device.c_str());
    if (autoSyncEnable_) {
        std::vector<std::string> devices;
        devices.push_back(device);
        int syncId = Sync(devices, SyncOperation::AUTO_PULL, nullptr, nullptr, false);
        if (syncId < MIN_VALID_SYNC_ID) {
            LOGE("[MultiVerSyncer] sync start by RemoteDataChanged failed err %d", syncId);
        }
    }
}

ISyncEngine *MultiVerSyncer::CreateSyncEngine()
{
    return new (std::nothrow) MultiVerSyncEngine();
}

int MultiVerSyncer::AddSyncOperation(SyncOperation *operation)
{
    if (operation == nullptr) {
        return -E_INVALID_ARGS;
    }
    MultiVerKvDBSyncInterface *syncInterface = static_cast<MultiVerKvDBSyncInterface *>(syncInterface_);
    syncInterface->NotifyStartSyncOperation();
    return GenericSyncer::AddSyncOperation(operation);
}

void MultiVerSyncer::SyncOperationKillCallbackInner(int syncId)
{
    if (syncInterface_ != nullptr) {
        LOGI("[MultiVerSyncer] Operation on kill id = %d", syncId);
        MultiVerKvDBSyncInterface *syncInterface = static_cast<MultiVerKvDBSyncInterface *>(syncInterface_);
        syncInterface->NotifyFinishSyncOperation();
    }
    GenericSyncer::SyncOperationKillCallbackInner(syncId);
}

int MultiVerSyncer::SetStaleDataWipePolicy(WipePolicy policy)
{
    return -E_NOT_SUPPORT;
}
} // namespace DistributedDB
#endif