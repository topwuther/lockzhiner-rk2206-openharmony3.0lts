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

#ifndef GENRIC_SYNCER_H
#define GENRIC_SYNCER_H

#include <functional>
#include <mutex>
#include <map>

#include "isyncer.h"
#include "isync_engine.h"
#include "meta_data.h"
#include "time_helper.h"
#include "sync_operation.h"

namespace DistributedDB {
class GenericSyncer : public virtual ISyncer {
using DataChangedFunc = std::function<void(const std::string &device)>;

public:
    GenericSyncer();
    virtual ~GenericSyncer();

    // Init the Syncer modules
    int Initialize(IKvDBSyncInterface *syncInterface) override;

    // Close
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

protected:
    // Remote data changed callback
    virtual void RemoteDataChanged(const std::string &device) = 0;

    // Create a sync engine, if has memory error, will return nullptr.
    virtual ISyncEngine *CreateSyncEngine() = 0;

    // Add a Sync Operation, after call this function, the operation will be start
    virtual int AddSyncOperation(SyncOperation *operation);

    // Used to set to the SyncOperation Onkill
    virtual void SyncOperationKillCallbackInner(int syncId);

    // Used to set to the SyncOperation Onkill
    void SyncOperationKillCallback(int syncId);

    // Init the metadata
    int InitMetaData(IKvDBSyncInterface *syncInterface);

    // Init the TimeHelper
    int InitTimeHelper(IKvDBSyncInterface *syncInterface);

    // Init the Sync engine
    int InitSyncEngine(IKvDBSyncInterface *syncInterface);

    // Used to general a sync id, maybe it is currentSyncId++;
    // The return value is sync id.
    uint32_t GenerateSyncId();

    // Check if the mode arg is valid
    bool IsValidMode(int mode) const;

    // Check if the devices arg is valid
    bool IsValidDevices(const std::vector<std::string> &devices) const;

    // Used Clear all SyncOperations.
    void ClearSyncOperations();

    // Callback when the special sync finished.
    void OnSyncFinished(int syncId);

    void AddQueuedManualSyncSize(int mode, bool wait);

    bool IsQueuedManualSyncFull(int mode, bool wait) const;

    void SubQueuedSyncSize(void);

    void GetOnlineDevices(std::vector<std::string> &devices) const;

    static int SyncModuleInit();

    static int SyncResourceInit();

    static const int MIN_VALID_SYNC_ID;
    static std::mutex moduleInitLock_;

    // Used to general the next sync id.
    static int currentSyncId_;
    static std::mutex syncIdLock_;

    ISyncEngine *syncEngine_;
    IKvDBSyncInterface *syncInterface_;
    std::shared_ptr<TimeHelper> timeHelper_;
    std::shared_ptr<Metadata> metadata_;
    bool initialized_;
    std::mutex operationMapLock_;
    std::map<int, SyncOperation *> syncOperationMap_;
    int queuedManualSyncSize_;
    int queuedManualSyncLimit_;
    bool manualSyncEnable_;
    bool closing_;
    mutable std::mutex queuedManualSyncLock_;
    mutable std::mutex syncerLock_;
    std::string label_;
};
} // namespace DistributedDB

#endif  // GENRIC_SYNCER_H
