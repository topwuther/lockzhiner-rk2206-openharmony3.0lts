/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this-> file except in compliance with the License.
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

#ifndef DISTRIBUTED_UID_ALLOCATOR_H
#define DISTRIBUTED_UID_ALLOCATOR_H

#include <map>
#include <mutex>
#include <string>

#include "constant.h"
#include "distributed_data_validator.h"

namespace OHOS {
namespace Security {
namespace Permission {
struct DistributedUidEntity {
    std::string deviceId;
    int32_t distributedUid;
    int32_t uid;
};

class DistributedUidAllocator {
public:
    static DistributedUidAllocator &GetInstance();

    /**
     * Generate a duid based on the entered device ID and application ID.
     *
     * @param deviceId The Id of the caller device.
     * @param uid The app uid of the caller application
     * @return Generated duid value. If the value is less than 0, an error occurs.
     */
    int32_t GetDuid(const std::string &deviceId, const int32_t uid);

    /**
     * Allocate distributedUid.
     *
     * @param deviceId The Id of the caller device.
     * @param uid The app uid of the caller application.
     * @return DistributedUid allocated from idle pool.
     */
    int32_t AllocateDuid(const std::string &deviceId, const int32_t uid);

    /**
     * Delete the duid information from the currently maintained information.
     *
     * @param deviceId The Id of the caller device.
     * @return If the operation succeeds, 0 is returned. Otherwise, the operation fails.
     */
    int32_t DeleteDuid(const std::string &deviceId);

    /**
     * Delete the duid information from the currently maintained information.
     *
     * @param deviceId The Id of the caller device.
     * @param uid The app uid of the caller application
     * @return If the operation succeeds, 0 is returned. Otherwise, the operation fails.
     */
    int32_t DeleteDuid(const std::string &deviceId, const int32_t uid);

    /**
     * Get device id for given distributedUid.
     *
     * @param distributedUid The distributed uid.
     * @return Optional string for device id.
     */
    std::string GetDeviceId(const int32_t distributedUid);

    /**
     * Clears the current duid list.
     */
    void Clear();

private:
    int32_t FindCurrentIdleDuidLocked();

    std::string Hash(const std::string &deviceId, const int32_t uid) const;

    std::map<std::string, DistributedUidEntity> distributedUidMapByKey_;

    std::map<int32_t, DistributedUidEntity> distributedUidMap_;

    int32_t lastDistributedUidCandidate_ = Constant::FIRST_DISTRIBUTED_UID;

    std::recursive_mutex stackLock_;
};
}  // namespace Permission
}  // namespace Security
}  // namespace OHOS
#endif  // PERMISSION_BASE_SERVICE_H