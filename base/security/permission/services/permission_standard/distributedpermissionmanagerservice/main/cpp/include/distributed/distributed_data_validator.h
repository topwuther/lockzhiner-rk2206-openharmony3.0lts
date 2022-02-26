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

#ifndef DISTRIBUTED_DATA_VALIDATOR_H
#define DISTRIBUTED_DATA_VALIDATOR_H

#include <string>

namespace OHOS {
namespace Security {
namespace Permission {
class DistributedDataValidator final {
public:
    static bool IsBundleNameValid(const std::string &bundleName);

    static bool IsPermissionNameValid(const std::string &permissionName);

    static bool IsUserIdValid(const int userId);

    /**
     * Check device id is valid or not
     */
    static bool IsDeviceIdValid(const std::string &deviceId);

    /**
     * Check uid is valid or not
     */
    static bool IsUidValid(const int uid);

    /**
     * Check package name is valid or not
     */
    static bool IsPackageNameValid(const std::string &packageName);

    /**
     * Check duid is valid or not. A valid duid should be in range of 12,600,000 ~ 12,619,999. The special duid 0 and
     * 1000 will pass.
     */
    static bool IsDuidValid(const int duid);

    /**
     * Check duid is valid or not. A valid duid should be in range of 12,600,000 ~ 12,619,999. The special duid 0 and
     * 1000 will not pass.
     */
    static bool IsDuidValidAndNotSpecial(const int32_t duid);

    /**
     * Check the version is valid or not.
     */
    static bool IsCommandVersionValid(const int version);

    /**
     * Check whether the current ruid is a root process or a system process.
     */
    static bool IsSpecRuid(const int ruid);

    /**
     * Method to judge whether a uid is a distributed uid or not
     *
     * @param uid a identification code
     * @return the result whether a uid is a distributed uid or not
     */
    static bool IsDuid(const int uid);

    /**
     * Check whether the current ruid is a root process or a system process.
     */
    static bool IsSpecDuid(const int32_t duid);

    const static int MAX_LENGTH = 256;

    const static int MAX_DEVICE_ID_LENGTH = 64;

    /**
     * Privilege uid, root.
     */
    const static int ROOT_UID = 0;

    /**
     * Privilege uid, system.
     */
    const static int SYSTEM_UID = 1000;

    /**
     * Per user range.
     */
    const static int PER_USER_RANGE = 100000;

    /**
     * Distributed user id support currently.
     */
    const static int DISTRIBUTED_USER_ID = 126;

    /**
     * First of application-specific UIDs starting.
     */
    const static int FIRST_APPLICATION_UID = 10000;

    /**
     * Last of application-specific UIDs starting.
     */
    const static int LAST_APPLICATION_UID = 19999;

    /**
     * The min value of uid.
     */
    const static int DUSER_ID_OHOS = 125;

    /**
     * The max value of uid.
     */
    const static int DUSER_ID = 126;
};
}  // namespace Permission
}  // namespace Security
}  // namespace OHOS
#endif  // DATA_VALIDATOR_H
