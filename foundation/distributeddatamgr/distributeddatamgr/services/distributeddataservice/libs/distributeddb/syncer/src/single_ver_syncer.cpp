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

#include "single_ver_syncer.h"

#include <functional>
#include <mutex>
#include <map>

#include "ikvdb_sync_interface.h"
#include "meta_data.h"
#include "log_print.h"
#include "sqlite_single_ver_natural_store.h"
#include "single_ver_sync_engine.h"

namespace DistributedDB {
SingleVerSyncer::SingleVerSyncer()
    : autoSyncEnable_(false)
{
}

SingleVerSyncer::~SingleVerSyncer()
{
}

void SingleVerSyncer::EnableAutoSync(bool enable)
{
    LOGI("[Syncer] EnableAutoSync enable = %d, Label=%s", enable, label_.c_str());
    if (autoSyncEnable_ == enable) {
        return;
    }

    autoSyncEnable_ = enable;
    if (!enable) {
        return;
    }

    if (!initialized_) {
        LOGE("[Syncer] Syncer has not Init");
        return;
    }

    std::vector<std::string> devices;
    GetOnlineDevices(devices);
    if (devices.empty()) {
        LOGI("[Syncer] EnableAutoSync no online devices");
        return;
    }
    int syncId = Sync(devices, SyncOperation::AUTO_PUSH, nullptr, nullptr, false);
    if (syncId < MIN_VALID_SYNC_ID) {
        LOGE("[Syncer] sync start by EnableAutoSync failed err %d", syncId);
    }
}

int SingleVerSyncer::EraseDeviceWaterMark(const std::string &deviceId, bool isNeedHash)
{
    if (metadata_ == nullptr) {
        return -E_NOT_INIT;
    }
    return metadata_->EraseDeviceWaterMark(deviceId, isNeedHash);
}

// Local data changed callback
void SingleVerSyncer::LocalDataChanged(int notifyEvent)
{
    if (!initialized_) {
        LOGE("[Syncer] Syncer has not Init");
        return;
    }

    if (!autoSyncEnable_) {
        LOGD("[Syncer] autoSync no enable");
        return;
    }

    if (notifyEvent != SQLITE_GENERAL_FINISH_MIGRATE_EVENT &&
        notifyEvent != SQLITE_GENERAL_NS_PUT_EVENT) {
        LOGD("[Syncer] ignore event:%d", notifyEvent);
        return;
    }

    std::vector<std::string> devices;
    GetOnlineDevices(devices);
    if (devices.empty()) {
        LOGI("[Syncer] LocalDataChanged no online devices, Label=%s", label_.c_str());
        return;
    }

    int syncId = Sync(devices, SyncOperation::AUTO_PUSH, nullptr, nullptr, false);
    if (syncId < MIN_VALID_SYNC_ID) {
        LOGE("[Syncer] sync start by RemoteDataChanged failed err %d", syncId);
    }
    return;
}

// Remote data changed callback
void SingleVerSyncer::RemoteDataChanged(const std::string &device)
{
    LOGI("[SingleVerSyncer] device online dev %s{private}", device.c_str());
    if (autoSyncEnable_) {
        RefObject::IncObjRef(syncEngine_);
        int retCode = RuntimeContext::GetInstance()->ScheduleTask([this, device] {
            std::vector<std::string> devices;
            devices.push_back(device);
            int syncId = Sync(devices, SyncOperation::AUTO_PUSH, nullptr, nullptr, false);
            if (syncId < MIN_VALID_SYNC_ID) {
                LOGE("[SingleVerSyncer] sync start by RemoteDataChanged failed err %d", syncId);
            }
            RefObject::DecObjRef(syncEngine_);
        });
        if (retCode != E_OK) {
            LOGE("[AutoLaunch] RemoteDataChanged triggler sync retCode:%d", retCode);
            RefObject::DecObjRef(syncEngine_);
        }
    }
}

ISyncEngine *SingleVerSyncer::CreateSyncEngine()
{
    return new (std::nothrow) SingleVerSyncEngine();
}

int SingleVerSyncer::SetStaleDataWipePolicy(WipePolicy policy)
{
    std::lock_guard<std::mutex> lock(syncerLock_);
    if (closing_) {
        LOGE("[Syncer] Syncer is closing, return!");
        return -E_BUSY;
    }
    if (syncEngine_ == nullptr) {
        return -E_NOT_INIT;
    }
    int errCode = E_OK;
    switch (policy) {
        case RETAIN_STALE_DATA:
            static_cast<SingleVerSyncEngine *>(syncEngine_)->EnableClearRemoteStaleData(false);
            break;
        case WIPE_STALE_DATA:
            static_cast<SingleVerSyncEngine *>(syncEngine_)->EnableClearRemoteStaleData(true);
            break;
        default:
            errCode = -E_NOT_SUPPORT;
            break;
    }
    return errCode;
}
} // namespace DistributedDB
