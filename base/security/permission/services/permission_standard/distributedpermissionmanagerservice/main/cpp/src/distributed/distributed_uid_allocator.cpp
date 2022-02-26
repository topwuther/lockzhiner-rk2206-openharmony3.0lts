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

#include "distributed_uid_allocator.h"

namespace OHOS {
namespace Security {
namespace Permission {
DistributedUidAllocator &DistributedUidAllocator::GetInstance()
{
    static DistributedUidAllocator instance;
    return instance;
}

int32_t DistributedUidAllocator::GetDuid(const std::string &deviceId, const int32_t uid)
{
    if (DistributedDataValidator::IsSpecRuid(uid)) {
        return Constant::DISTRIBUTED_USER_ID * Constant::PER_USER_RANGE + uid % Constant::PER_USER_RANGE;
    }
    std::lock_guard<std::recursive_mutex> guard(stackLock_);
    const std::string key = Hash(deviceId, uid);
    if (distributedUidMapByKey_.count(key) > 0) {
        return distributedUidMapByKey_.at(key).distributedUid;
    }
    return Constant::DISTRIBUTED_UID_NOT_EXIST;
}

int32_t DistributedUidAllocator::AllocateDuid(const std::string &deviceId, const int32_t uid)
{
    if (DistributedDataValidator::IsSpecRuid(uid)) {
        return Constant::DISTRIBUTED_USER_ID * Constant::PER_USER_RANGE + uid % Constant::PER_USER_RANGE;
    }
    std::lock_guard<std::recursive_mutex> guard(stackLock_);
    const std::string key = Hash(deviceId, uid);
    if (distributedUidMapByKey_.count(key) > 0) {
        return distributedUidMapByKey_.at(key).distributedUid;
    }

    const int32_t newDuid = FindCurrentIdleDuidLocked();
    if (newDuid == Constant::INVALID_ID) {
        return Constant::DISTRIBUTED_UID_NOT_EXIST;
    }

    DistributedUidEntity distributedUidEntity;
    distributedUidEntity.deviceId = deviceId;
    distributedUidEntity.distributedUid = newDuid;
    distributedUidEntity.uid = uid;

    distributedUidMapByKey_.insert(std::pair<std::string, DistributedUidEntity>(key, distributedUidEntity));
    distributedUidMap_.insert(std::pair<int32_t, DistributedUidEntity>(newDuid, distributedUidEntity));
    return newDuid;
}

int32_t DistributedUidAllocator::FindCurrentIdleDuidLocked()
{
    for (int32_t distributedUidCandidate = lastDistributedUidCandidate_;
         distributedUidCandidate <= Constant::LAST_DISTRIBUTED_UID;
         distributedUidCandidate++) {
        if (distributedUidMap_.count(distributedUidCandidate) == 0) {
            lastDistributedUidCandidate_ = (distributedUidCandidate == Constant::LAST_DISTRIBUTED_UID)
                                               ? Constant::FIRST_DISTRIBUTED_UID
                                               : (distributedUidCandidate + 1);
            return distributedUidCandidate;
        }
    }
    for (int32_t distributedUidCandidate = Constant::FIRST_DISTRIBUTED_UID;
         distributedUidCandidate <= lastDistributedUidCandidate_;
         distributedUidCandidate++) {
        if (distributedUidMap_.count(distributedUidCandidate) == 0) {
            lastDistributedUidCandidate_ = (distributedUidCandidate == Constant::LAST_DISTRIBUTED_UID)
                                               ? Constant::FIRST_DISTRIBUTED_UID
                                               : (distributedUidCandidate + 1);
            return distributedUidCandidate;
        }
    }
    return Constant::INVALID_ID;
}

int32_t DistributedUidAllocator::DeleteDuid(const std::string &deviceId, const int32_t uid)
{
    std::lock_guard<std::recursive_mutex> guard(stackLock_);
    const std::string key = Hash(deviceId, uid);
    if (distributedUidMapByKey_.count(key) > 0) {
        const int32_t distributedUid = distributedUidMapByKey_.at(key).distributedUid;
        distributedUidMapByKey_.erase(key);
        distributedUidMap_.erase(distributedUid);
        return Constant::SUCCESS;
    }
    return Constant::DISTRIBUTED_UID_NOT_EXIST;
}

int32_t DistributedUidAllocator::DeleteDuid(const std::string &deviceId)
{
    std::lock_guard<std::recursive_mutex> guard(stackLock_);
    if (distributedUidMapByKey_.empty()) {
        return Constant::DISTRIBUTED_UID_NOT_EXIST;
    }
    std::hash<std::string> hashStr;
    std::string deviceIdHash = std::to_string(hashStr(deviceId));
    for (std::map<std::string, DistributedUidEntity>::iterator it = distributedUidMapByKey_.begin();
         it != distributedUidMapByKey_.end();
         it++) {
        int32_t duid;
        // check exist
        if (it->first.find(deviceIdHash) != std::string::npos) {
            duid = it->second.distributedUid;
            distributedUidMapByKey_.erase(it->first);
            distributedUidMap_.erase(duid);
        }
    }
    return Constant::SUCCESS;
}

std::string DistributedUidAllocator::GetDeviceId(const int32_t distributedUid)
{
    std::lock_guard<std::recursive_mutex> guard(stackLock_);
    const std::string deviceId;
    if (distributedUidMap_.count(distributedUid) > 0) {
        return distributedUidMap_.at(distributedUid).deviceId;
    }
    return deviceId;
}

void DistributedUidAllocator::Clear()
{
    std::lock_guard<std::recursive_mutex> guard(stackLock_);
    distributedUidMapByKey_.clear();
    distributedUidMap_.clear();
}

std::string DistributedUidAllocator::Hash(const std::string &deviceId, const int32_t uid) const
{
    std::hash<std::string> hashStr;
    return std::to_string(hashStr(deviceId)) + "_" + std::to_string(uid);
}
}  // namespace Permission
}  // namespace Security
}  // namespace OHOS