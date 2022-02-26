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

#ifndef I_SYNCER_H
#define I_SYNCER_H

#include <functional>
#include <mutex>
#include <map>

#include "ikvdb_sync_interface.h"
#include  "types_export.h"

namespace DistributedDB {
class ISyncer {
public:
    virtual ~ISyncer() {};

    // Init the Syncer modules
    virtual int Initialize(IKvDBSyncInterface *syncInterface) = 0;

    // Close
    virtual int Close() = 0;

    // Sync function.
    // param devices: The device id list.
    // param mode: Sync mode, see SyncMode.
    // param onComplete: The syncer finish callback. set by caller
    // param onFinalize: will be callback when this Sync Operation finalized.
    // return a Sync id. It will return a positive value if failed,
    virtual int Sync(const std::vector<std::string> &devices, int mode,
        const std::function<void(const std::map<std::string, int> &)> &onComplete,
        const std::function<void(void)> &onFinalize, bool wait) = 0;

    // Remove the operation, with the given syncId, used to clean resource if sync finished or failed.
    virtual int RemoveSyncOperation(int syncId) = 0;

    // Get The current vitural timestamp
    virtual uint64_t GetTimeStamp() = 0;

    // Enable auto sync function
    virtual void EnableAutoSync(bool enable) = 0;

    // delete specified device's watermark
    virtual int EraseDeviceWaterMark(const std::string &deviceId, bool isNeedHash) = 0;

    // Local data changed callback
    virtual void LocalDataChanged(int notifyEvent) = 0;

    // Get manual sync queue size
    virtual int GetQueuedSyncSize(int *queuedSyncSize) const = 0;

    // Set manual sync queue limit
    virtual int SetQueuedSyncLimit(const int *queuedSyncLimit) = 0;

    // Get manual sync queue limit
    virtual int GetQueuedSyncLimit(int *queuedSyncLimit) const = 0;

    // Disable add new manual sync, for rekey
    virtual int DisableManualSync(void) = 0;

    // Enable add new manual sync, for rekey
    virtual int EnableManualSync(void) = 0;

    // Get local deviceId, is hashed
    virtual int GetLocalIdentity(std::string &outTarget) const = 0;

    // Set stale data wipe policy
    virtual int SetStaleDataWipePolicy(WipePolicy policy) = 0;
};
} // namespace DistributedDB

#endif  // I_SYNCER_H
