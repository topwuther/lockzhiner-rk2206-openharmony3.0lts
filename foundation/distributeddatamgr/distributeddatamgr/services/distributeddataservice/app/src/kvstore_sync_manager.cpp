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

#define LOG_TAG "KvSyncManager"

#include "kvstore_sync_manager.h"
#include "log_print.h"

namespace OHOS {
namespace DistributedKv {
KvStoreSyncManager::KvStoreSyncManager() : syncScheduler_()
{}

KvStoreSyncManager::~KvStoreSyncManager() {}

Status KvStoreSyncManager::AddSyncOperation(uintptr_t syncId, uint32_t delayMs, const SyncFunc &syncFunc,
                                            const SyncEnd &syncEnd)
{
    if (syncId == 0 || syncFunc == nullptr) {
        return Status::INVALID_ARGUMENT;
    }
    uint32_t opSeq = ++syncOpSeq_;
    SyncEnd endFunc;
    if (syncEnd != nullptr) {
        endFunc = [opSeq, delayMs, syncEnd, this](const std::map<std::string, DistributedDB::DBStatus> &devices) {
            RemoveSyncingOp(opSeq, (delayMs == 0) ? realtimeSyncingOps_ : delaySyncingOps_);
            syncEnd(devices);
        };
    }

    auto beginTime = std::chrono::system_clock::now() + std::chrono::milliseconds(delayMs);
    KvSyncOperation syncOp{ syncId, opSeq, delayMs, syncFunc, endFunc, beginTime };
    if (delayMs == 0) {
        if (endFunc != nullptr) {
            std::lock_guard<std::mutex> lock(syncOpsMutex_);
            realtimeSyncingOps_.push_back(syncOp);
        }
        return syncFunc(endFunc);
    }

    std::lock_guard<std::mutex> lock(syncOpsMutex_);
    scheduleSyncOps_.emplace(beginTime, syncOp);
    ZLOGD("add op %u delay %u count %zu.", opSeq, delayMs, scheduleSyncOps_.size());
    if ((scheduleSyncOps_.size() == 1) ||
        (nextScheduleTime_ > beginTime + std::chrono::milliseconds(GetExpireTimeRange(delayMs)))) {
        AddTimer(beginTime);
    }
    return Status::SUCCESS;
}

uint32_t KvStoreSyncManager::GetExpireTimeRange(uint32_t delayMs) const
{
    uint32_t range = delayMs / DELAY_TIME_RANGE_DIVISOR;
    return std::max(range, SYNC_MIN_DELAY_MS >> 1);
}

Status KvStoreSyncManager::RemoveSyncOperation(uintptr_t syncId)
{
    auto pred = [syncId](const KvSyncOperation &op) -> bool { return syncId == op.syncId; };

    std::lock_guard<std::mutex> lock(syncOpsMutex_);
    uint32_t count = DoRemoveSyncingOp(pred, realtimeSyncingOps_);
    count += DoRemoveSyncingOp(pred, delaySyncingOps_);

    auto &syncOps = scheduleSyncOps_;
    for (auto it = syncOps.begin(); it != syncOps.end();) {
        if (pred(it->second)) {
            count++;
            it = syncOps.erase(it);
        } else {
            ++it;
        }
    }
    return (count > 0) ? Status::SUCCESS : Status::ERROR;
}

uint32_t KvStoreSyncManager::DoRemoveSyncingOp(OpPred pred, std::list<KvSyncOperation> &syncingOps)
{
    uint32_t count = 0;
    for (auto it = syncingOps.begin(); it != syncingOps.end();) {
        if (pred(*it)) {
            count++;
            it = syncingOps.erase(it);
        } else {
            ++it;
        }
    }
    return count;
}

Status KvStoreSyncManager::RemoveSyncingOp(uint32_t opSeq, std::list<KvSyncOperation> &syncingOps)
{
    auto pred = [opSeq](const KvSyncOperation &op) -> bool { return opSeq == op.opSeq; };

    ZLOGD("remove op %u", opSeq);
    std::lock_guard<std::mutex> lock(syncOpsMutex_);
    uint32_t count = DoRemoveSyncingOp(pred, syncingOps);
    return (count == 1) ? Status::SUCCESS : Status::ERROR;
}

void KvStoreSyncManager::AddTimer(const TimePoint &expireTime)
{
    ZLOGD("time %lld", expireTime.time_since_epoch().count());
    nextScheduleTime_ = expireTime;
    syncScheduler_.At(expireTime, [time = expireTime, this]() { Schedule(time); });
}

bool KvStoreSyncManager::GetTimeoutSyncOps(const TimePoint &currentTime, std::list<KvSyncOperation> &syncOps)
{
    std::lock_guard<std::mutex> lock(syncOpsMutex_);
    if ((!realtimeSyncingOps_.empty()) && (!scheduleSyncOps_.empty())) {
        // the last processing time is less than priorSyncingTime
        auto priorSyncingTime = std::chrono::milliseconds(REALTIME_PRIOR_SYNCING_MS);
        if (currentTime < realtimeSyncingOps_.rbegin()->beginTime + priorSyncingTime) {
            return true;
        }
    }
    for (auto it = scheduleSyncOps_.begin(); it != scheduleSyncOps_.end();) {
        const auto &expireTime = it->first;
        const auto &op = it->second;
        // currentTime is earlier than expireTime minus delayMs
        if (currentTime + std::chrono::milliseconds(GetExpireTimeRange(op.delayMs)) < expireTime) {
            break;
        }

        syncOps.push_back(op);
        if (op.syncEnd != nullptr) {
            delaySyncingOps_.push_back(op);
        }
        it = scheduleSyncOps_.erase(it);
    }
    return false;
}

void KvStoreSyncManager::DoCheckSyncingTimeout(std::list<KvSyncOperation> &syncingOps)
{
    auto syncingTimeoutPred = [](const KvSyncOperation &op) -> bool {
        return op.beginTime + std::chrono::milliseconds(SYNCING_TIMEOUT_MS) < std::chrono::system_clock::now();
    };

    uint32_t count = DoRemoveSyncingOp(syncingTimeoutPred, syncingOps);
    if (count > 0) {
        ZLOGI("remove %u syncing ops by timeout", count);
    }
}

void KvStoreSyncManager::Schedule(const TimePoint &time)
{
    ZLOGD("timeout %lld", time.time_since_epoch().count());
    std::list<KvSyncOperation> syncOps;
    bool delaySchedule = GetTimeoutSyncOps(time, syncOps);

    for (const auto &op : syncOps) {
        op.syncFunc(op.syncEnd);
    }

    std::lock_guard<std::mutex> lock(syncOpsMutex_);
    DoCheckSyncingTimeout(realtimeSyncingOps_);
    DoCheckSyncingTimeout(delaySyncingOps_);
    if (!scheduleSyncOps_.empty()) {
        auto nextTime = scheduleSyncOps_.begin()->first;
        if (delaySchedule) {
            nextTime = std::chrono::system_clock::now() + std::chrono::milliseconds(SYNC_MIN_DELAY_MS);
        }
        AddTimer(nextTime);
    }
}
}  // namespace DistributedKv
}  // namespace OHOS
