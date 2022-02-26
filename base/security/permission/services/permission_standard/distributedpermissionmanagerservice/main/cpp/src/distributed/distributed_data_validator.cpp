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

#include "distributed_data_validator.h"

namespace OHOS {
namespace Security {
namespace Permission {
bool DistributedDataValidator::IsBundleNameValid(const std::string &bundleName)
{
    return !bundleName.empty() && (bundleName.length() <= MAX_LENGTH);
}

bool DistributedDataValidator::IsPermissionNameValid(const std::string &permissionName)
{
    return !permissionName.empty() && (permissionName.length() <= MAX_LENGTH);
}

bool DistributedDataValidator::IsUserIdValid(const int userId)
{
    return userId >= 0;
}
/**
 * Check device id is valid or not
 *
 * @param deviceId The deviceId.
 * @return True if deviceId is not empty and deviceId's length is less than or equal to 64.
 */
bool DistributedDataValidator::IsDeviceIdValid(const std::string &deviceId)
{
    return !deviceId.empty() && (deviceId.length() <= MAX_DEVICE_ID_LENGTH);
}

/**
 * Check uid is valid or not
 *
 * @param uid The uid.
 * @return True if uid greater or equal to 0.
 */
bool DistributedDataValidator::IsUidValid(const int uid)
{
    return uid >= 0;
}

/**
 * Check package name is valid or not
 *
 * @param packageName The package name.
 * @return True if package is not empty.
 */
bool DistributedDataValidator::IsPackageNameValid(const std::string &packageName)
{
    return !packageName.empty();
}

/**
 * Check duid is valid or not. A valid duid should be in range of 12,600,000 ~ 12,619,999. The special duid 0 and
 * 1000 will pass.
 *
 * @param duid The distribution uid.
 * @return true if userId is 126, and appId is less than or equal to 19999.
 */
bool DistributedDataValidator::IsDuidValid(const int duid)
{
    if (duid == ROOT_UID || duid == SYSTEM_UID) {
        return true;
    }
    const int userId = duid / PER_USER_RANGE;
    const int appId = duid % PER_USER_RANGE;

    if (userId != DISTRIBUTED_USER_ID) {
        return false;
    }
    return (appId >= FIRST_APPLICATION_UID) && (appId <= LAST_APPLICATION_UID);
}

/**
 * Check duid is valid or not. A valid duid should be in range of 12,600,000 ~ 12,619,999. The special duid 0 and
 * 1000 will not pass.
 *
 * @param duid The distribution uid.
 * @return true if userId is 126, and appId is less than or equal to 19999.
 */
bool DistributedDataValidator::IsDuidValidAndNotSpecial(const int32_t duid)
{
    const int userId = duid / PER_USER_RANGE;
    const int appId = duid % PER_USER_RANGE;
    if (userId != DISTRIBUTED_USER_ID) {
        return false;
    }
    return (appId >= FIRST_APPLICATION_UID) && (appId <= LAST_APPLICATION_UID);
}

/**
 * Check the version is valid or not.
 *
 * @param version The version.
 * @return true if version is greater than 0.
 */
bool DistributedDataValidator::IsCommandVersionValid(const int version)
{
    return version > 0;
}

/**
 * Check whether the current ruid is a root process or a system process.
 *
 * @param ruid The Application id.
 * @return True if is root or system when user id is 0, false otherwise.
 */
bool DistributedDataValidator::IsSpecRuid(const int ruid)
{
    return (ruid % PER_USER_RANGE == ROOT_UID) || (ruid % PER_USER_RANGE == SYSTEM_UID);
}

/**
 * Method to judge whether a uid is a distributed uid or not
 *
 * @param uid a identification code
 * @return the result whether a uid is a distributed uid or not
 */
bool DistributedDataValidator::IsDuid(int uid)
{
    int userId = uid / PER_USER_RANGE;
    return userId == DUSER_ID;
}

bool DistributedDataValidator::IsSpecDuid(const int32_t duid)
{
    return (duid % PER_USER_RANGE == ROOT_UID) || (duid % PER_USER_RANGE == SYSTEM_UID);
}
}  // namespace Permission
}  // namespace Security
}  // namespace OHOS