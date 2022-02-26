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

#ifndef SYNCER_PROXY_H
#define SYNCER_PROXY_H

#include <functional>
#include <mutex>
#include <map>
#include <memory>

#include "isyncer.h"
#include "ikvdb_sync_interface.h"

namespace DistributedDB {
class SyncerProxy : public ISyncer {
public:
    SyncerProxy();
    ~SyncerProxy() {};

    // Init the Syncer modules
    int Initialize(IKvDBSyncInterface *syncInterface) override;

    // Close the syncer
    int Close() override;

    // Sync function.
    // param devices: The device id list.
    // param mode: Sync mode, see SyncMode.
    // param onComplete: The syncer finish callback. set by caller
    // param onFinalize: will be callback when this Sync Operation finalized.
    // return a Sync id. It will return a positive value if failed,
    int Sync(const std::vector<std::string> &devices, int mode,
        const std::function<void(const std::map<std::string, int> &)> &onComplete,
        const std::function<void(void)> &onFinalize, bool wait) override;

    // Remove the operation, with the given syncId, used to clean resource if sync finished or failed.
    int RemoveSyncOperation(int syncId) override;

    // Get The current virtual timestamp
    uint64_t GetTimeStamp() override;

    // Enable auto sync function
    void EnableAutoSync(bool enable) override;

    // delete specified device's watermark
    int EraseDeviceWaterMark(const std::string &deviceId, bool isNeedHash) override;

    // Local data changed callback
    void LocalDataChanged(int notifyEvent) override;

    // Get manual sync queue size
    int GetQueuedSyncSize(int *queuedSyncSize) const override;

    // Set manual sync queue limit
    int SetQueuedSyncLimit(const int *queuedSyncLimit) override;

    // Get manual sync queue limit
    int GetQueuedSyncLimit(int *queuedSyncLimit) const override;

    // Disable add new manual sync, for rekey
    int DisableManualSync(void) override;

    // Enable add new manual sync, for rekey
    int EnableManualSync(void) override;

    // Get local deviceId, is hashed
    int GetLocalIdentity(std::string &outTarget) const override;

    // Set stale data wipe policy
    int SetStaleDataWipePolicy(WipePolicy policy) override;

private:
    std::mutex syncerLock_;
    std::shared_ptr<ISyncer> syncer_;
};
} // namespace DistributedDB

#endif  // SYNCER_PROXY_H
