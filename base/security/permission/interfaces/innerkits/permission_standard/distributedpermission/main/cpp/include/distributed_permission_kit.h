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

#ifndef DISRIBUTED_PERMISSION_KIT_H
#define DISRIBUTED_PERMISSION_KIT_H

#include <iostream>
#include "distributed_permission_manager_client.h"
#include "app_id_info.h"
#include "constant.h"
#include "nlohmann/json.hpp"

namespace OHOS {
namespace Security {
namespace Permission {
class DistributedPermissionKit {
public:
    /*!
    @brief Allocate local Duid for across-device accessing application.

    @param[in] nodeId the Id of the caller device.
    @param[in] rUid the app uid of the caller application.

    @return Duid allocated from idle pool.
    */
    static int32_t AllocateDuid(const std::string &nodeId, const int32_t rUid);

    /*!
    @brief Get allocated Duid for target across-device accessing application.

    @param[in] nodeId nodeId the Id of the caller device.
    @param[in] rUid the app uid of the caller application.

    @return Duid already allocated.
    */
    static int32_t QueryDuid(const std::string &deviceId, int32_t rUid);

    static bool IsDuid(int32_t uid);

    class AppIdInfoHelper {
    public:
        /*!
        @brief Create the json string based on pid and uid.

        @param[in] pid The pid of caller device.
        @param[in] uid The application uid of caller device.

        @return The json string included pid and uid.
        */
        static std::string CreateAppIdInfo(pid_t pid, uid_t uid);

        /*!
        @brief Create the json string based on pid, uid and deviceID.

        @param[in] pid The pid of caller device.
        @param[in] uid The application uid of caller device.
        @param[in] deviceID The deviceID of caller device.

        @return The json string included pid, uid and deviceID.
        */
        static std::string CreateAppIdInfo(pid_t pid, uid_t uid, const std::string &deviceID);

        /*!
        @brief Create the json string based on pid, uid, deviceID and bundleName.

        @param[in] pid The pid of caller device.
        @param[in] uid The application uid of caller device.
        @param[in] deviceID The deviceID of caller device.
        @param[in] bundleName The application bundleName of caller device.

        @return The json string included pid, uid, deviceID and bundleName.
        */
        static std::string CreateAppIdInfo(
            pid_t pid, uid_t uid, const std::string &deviceID, const std::string &bundleName);

        /*!
        @brief Parse the json string based on appIdInfo, and convert to appIdInfoObj.

        @param[in] appIdInfo The json string included pid, uid, deviceID and bundleName.
        @param[in] appIdInfoObj The appIdInfo Object included pid, uid, deviceID.

        @return If the appIdInfo Object is valid, return True; Otherwise, return False.
        */
        static bool ParseAppIdInfo(const std::string &appIdInfo, Permission::AppIdInfo &appIdInfoObj);
    };

    /*!
    @brief Check if the given across-device accessing application has been granted permission with the given name.

    @param[in] duid duid to check permission.
    @param[in] permissionName permission name.

    @return Check Distributed Permission result.
    */
    static int32_t CheckDPermission(int32_t dUid, const std::string &permissionName);

    /*!
    @brief Check the given permission if granted based on appIdInfo.

    @param[in] permissionName, The specific permission name.
    @param[in] appIdInfo, The json string included pid, uid, deviceID and bundleName.

    @return Permission checked result, 0: GRANTED, -1: DENIED.
    */
    static int32_t CheckPermission(const std::string &permissionName, const std::string &appIdInfo);

    /*!
    @brief Check self permission.

    @param[in] permissionName, permission name.

    @return If the permission is granted, 0: GRANTED; otherwise, -1: DENIED.
    */
    static int32_t CheckSelfPermission(const std::string &permissionName);

    /*!
    @brief Check current process's permission.

    @param[in] permissionName, permission name.

    @return If the permission is granted, 0: GRANTED; otherwise, -1: DENIED.
    */
    static int32_t CheckCallingPermission(const std::string &permissionName);

    /*!
    @brief Check current process's or self permission.

    @param[in] permissionName, permission name.

    @return If the permission is granted, 0: GRANTED; otherwise, -1: DENIED.
    */
    static int32_t CheckCallingOrSelfPermission(const std::string &permissionName);

    /*!
    @brief Check caller's permission.

    @param[in] permissionName, permission name.

    @return If the permission is granted, 0: GRANTED; otherwise, -1: DENIED.
    */
    static int32_t CheckCallerPermission(const std::string &permissionName);

    /*!
    @brief Check if the permission is restricted.

    @param[in] permissionName, permission name.

    @return If the permission is restricted, return true; otherwise, return false.
    */
    static bool IsRestrictedPermission(const std::string &permissionName);
};
}  // namespace Permission
}  // namespace Security
}  // namespace OHOS

#endif  // DISRIBUTED_PERMISSION_KIT_H
