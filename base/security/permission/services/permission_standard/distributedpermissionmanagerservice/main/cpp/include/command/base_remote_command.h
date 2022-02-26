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
#ifndef BASE_REMOTE_COMMON_H
#define BASE_REMOTE_COMMON_H

#include <set>
#include <vector>
#include <memory>
#include <iostream>
#include <algorithm>
#include "constant.h"
#include "remote_protocol.h"
#include "bundle_mgr_interface.h"
#include "i_permission_manager.h"
#include "nlohmann/json.hpp"

namespace OHOS {
namespace Security {
namespace Permission {
struct PermissionDto {
public:
    std::string name;
    std::string type;
    int32_t defInfo;
    int32_t level = 0;
    int32_t status;
    int32_t grantMode;
};

struct SignDto {
public:
    std::string sha256;
};

struct BundlePermissionsDto {
public:
    std::string name;
    std::string bundleLabel;
    std::string appId;
    std::vector<SignDto> sign;
    std::vector<PermissionDto> permissions;
};

struct UidBundleBo {
public:
    int32_t DEFAULT_SIZE;          // default 10
    int32_t MIN_UID_PACKAGES_NUM;  // default 1
    int32_t uid;
    int32_t appAttribute = Constant::INITIAL_APP_ATTRIBUTE;
    int32_t uidState = 0;
    std::set<std::string> remoteSensitivePermission;
    std::vector<BundlePermissionsDto> bundles;
};

/**
 * The base class for command. You can treat this as remote command header.
 */
class BaseRemoteCommand {
public:
    /**
     * Default construct method.
     */
    BaseRemoteCommand() = default;
    virtual ~BaseRemoteCommand() = default;

    virtual void Prepare() = 0;
    virtual void Execute() = 0;
    virtual void Finish() = 0;

    /**
     * Check input object is valid or not.
     *
     * @param uidBundleBo UidBundleBo
     * @return True if valid.
     */
    static bool IsValid(const UidBundleBo &uidBundleBo);
    /**
     * Check permission is sensitive resource permission.
     *
     * @param permission The input permission
     * @return True if it is.
     */
    static bool IsSensitiveResource(const std::string &permission);
    /**
     * Grant a sensitive permission to RUID.
     *
     * @param uidBundlePermissionsInfo UidBundleBo
     * @param permission A sensitive permission.
     */
    static void GrantSensitivePermissionToRemoteApp(UidBundleBo &uidBundleBo, const std::string &permission);
    /**
     * Check whether the sensitive permission is granted.
     *
     * @param uidBundlePermissionsInfo UidBundleBo
     * @param permission the sensitive permission to be checked.
     * @return true if the sensitive permission is granted.
     */
    static bool IsGrantedSensitivePermission(const UidBundleBo &uidBundleBo, const std::string &permissionName);

    /**
     * Get json string of the class which implemented BaseRemoteCommand.
     *
     * @return The json string.
     */
    virtual std::string ToJsonPayload() = 0;
    /**
     * RemoteProtocol to Json
     */
    nlohmann::json ToRemoteProtocolJson();
    /**
     * UidBundlePermissions to Json
     * @param bundle UidBundleBo
     * @return json
     */
    nlohmann::json ToUidBundlePermissionsJson(const UidBundleBo &bundle);
    /**
     * Json to RemoteProtocol
     * @param jsonObject
     */
    void FromRemoteProtocolJson(nlohmann::json jsonObject);
    /**
     * Json to UidBundlePermissions
     * @param bundlePermissionsJson
     * @param uidBundlePermissions
     */
    void FromUidBundlePermissionsJson(const nlohmann::json &bundlePermissionsJson, UidBundleBo &uidBundleBo);
    RemoteProtocol remoteProtocol_;

protected:
    sptr<AppExecFwk::IBundleMgr> iBundleManager_;
    sptr<Permission::IPermissionManager> iPermissionManager_;
};
}  // namespace Permission
}  // namespace Security
}  // namespace OHOS
#endif  // BASE_REMOTE_COMMON_H
