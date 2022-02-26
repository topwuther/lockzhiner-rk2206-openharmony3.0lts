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

#ifndef KVSTORE_SYNC_MANAGER_H
#define KVSTORE_SYNC_MANAGER_H

#include <map>
#include <list>
#include "types.h"
#include "kv_scheduler.h"
#include "kv_store_nb_delegate.h"

namespace OHOS {
namespace DistributedKv {
class KvStoreSyncManager {
public:
    static constexpr uint32_t SYNC_DEFAULT_DELAY_MS = 1000;
    static constexpr uint32_t SYNC_MIN_DELAY_MS = 100;
    static constexpr uint32_t SYNC_MAX_DELAY_MS = 1000 * 3600 * 24;  // 24hours
    static constexpr uint32_t SYNC_RETRY_MAX_COUNT = 3;
    static KvStoreSyncManager *GetInstance()
    {
        static KvStoreSyncManager syncManager;
        return &syncManager;
    }
    using TimePoint = std::chrono::system_clock::time_point;
    using SyncEnd = std::function<void(const std::map<std::string, DistributedDB::DBStatus> &)>;
    using SyncFunc = std::function<Status(const SyncEnd &)>;

    struct KvSyncOperation {
        uintptr_t syncId{ 0 };
        uint32_t opSeq{ 0 };
        uint32_t delayMs{ 0 };
        SyncFunc syncFunc{};
        SyncEnd  syncEnd{};
        TimePoint beginTime{};
    };
    using OpPred = std::function<bool(KvSyncOperation &)>;
    Status AddSyncOperation(uintptr_t syncId, uint32_t delayMs, const SyncFunc &syncFunc,
        const SyncEnd &syncEnd);
    Status RemoveSyncOperation(uintptr_t syncId);
private:
    KvStoreSyncManager();
    ~KvStoreSyncManager();

    uint32_t GetExpireTimeRange(uint32_t delayMs) const;
    uint32_t DoRemoveSyncingOp(OpPred pred, std::list<KvSyncOperation> &syncingOps);
    Status RemoveSyncingOp(uint32_t opSeq, std::list<KvSyncOperation> &syncingOps);
    void AddTimer(const TimePoint &expireTime);
    bool GetTimeoutSyncOps(const TimePoint &time, std::list<KvSyncOperation> &syncOps);
    void DoCheckSyncingTimeout(std::list<KvSyncOperation> &syncingOps);
    void Schedule(const TimePoint &expireTime);

    static constexpr uint32_t SYNCING_TIMEOUT_MS = 5000;
    static constexpr uint32_t REALTIME_PRIOR_SYNCING_MS = 300;
    static constexpr uint32_t DELAY_TIME_RANGE_DIVISOR = 4;

    mutable std::mutex syncOpsMutex_{};
    std::list<KvSyncOperation> realtimeSyncingOps_{};
    std::list<KvSyncOperation> delaySyncingOps_{};
    std::multimap<TimePoint, KvSyncOperation> scheduleSyncOps_{};

    KvScheduler syncScheduler_{};
    TimePoint nextScheduleTime_{};
    std::atomic_uint32_t syncOpSeq_{ 0 };
};
}  // namespace DistributedKv
}  // namespace OHOS

#endif  // KVSTORE_SYNC_MANAGER_H
