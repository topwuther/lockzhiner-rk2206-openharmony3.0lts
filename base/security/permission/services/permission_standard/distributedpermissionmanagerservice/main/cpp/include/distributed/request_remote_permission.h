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
#ifndef REQUEST_REMOTE_PERMISSION_H
#define REQUEST_REMOTE_PERMISSION_H
#include <string>
#include <map>
#include <numeric>
#include <sstream>
#include <algorithm>
#include "request_permissions_info.h"
#include "resource_switch.h"
#include "on_request_permissions_result.h"
#include "distributed_data_validator.h"
#include "device_info_manager.h"
#include "subject_device_permission_manager.h"
#include "object_device_permission_manager.h"
#include "user_handle_manager.h"
#include "remote_command_factory.h"
#include "remote_command_manager.h"
#include "bundlemgr/bundle_mgr_interface.h"
#include "permission_log.h"
#include "refbase.h"
#include "external_deps.h"

namespace OHOS {
namespace Security {
namespace Permission {
class RequestRemotePermission {
public:
    /**
     * Get the Single Instance of RequestRemotePermission
     *
     * @return Single Instance of RequestRemotePermission
     */
    static RequestRemotePermission &GetInstance();

    /**
     * Check whether this app allows you to dynamically apply for specified permissions from a specified device.
     *
     * @param permission The permission name.
     * @param deviceId Remote device udid, not null.
     * @param pid The Caller Pid
     * @param uid The Caller Uid
     * @return true can request permission, false cannot request permission.
     */
    bool CanRequestPermissionFromRemote(
        const std::string &permission, const std::string &deviceId, const int32_t pid, const int32_t uid);

    /**
     * Dynamically applies for permissions from a specified device (the application result is returned asynchronously).
     *
     * @param permissions The permission names will request from remote.
     * @param callback The callback of the request operation.
     * @param nodeId The remote device udid.
     * @param bundleName The package name that request the permissions.
     * @param reasonResId The reason that request the permissions.
     */
    void RequestPermissionsFromRemote(const std::vector<std::string> permissions,
        const sptr<OnRequestPermissionsResult> &callback, const std::string &nodeId, const std::string &bundleName,
        int32_t reasonResId);
    /**
     * This function will call when the request is success return
     *
     * @param requestId The id of the request
     */
    void RequestResultForResult(const std::string &requestId);
    /**
     * Grants cross-device applications the permission to access sensitive resources on the local device.
     *
     * @param permission The permission name.
     * @param deviceId The remote device nodeId, not null.
     * @param ruid The remote app uid.
     */
    void GrantSensitivePermissionToRemoteApp(const std::string &permission, const std::string &deviceId, int32_t ruid);

private:
    RequestRemotePermission()
    {}
    bool CheckRequestResultForResult(std::shared_ptr<RequestPermissionsInfo> request);
    // bool CheckThirdAppMore(const std::string &permission, const std::string &deviceId, const int32_t uid);
    /**
     * When the input parameters is invalid, callback and fill the result with the same error
     *
     * @param permissions Input permissions
     * @param nodeId Input node Id
     * @param callback Input callback for process the request result
     * @param errorCode The error when check if can request
     */
    void RequestResultForError(const std::vector<std::string> permissions, const std::string &nodeId,
        const sptr<OnRequestPermissionsResult> &callback, const int32_t errorCode);
    /**
     * This function will call when the request is timeout, and not return the result
     *
     * @param requestId The id of the request
     */
    void RequestResultForTimeout(const std::string &requestId);
    /**
     * This function will call when the request is cancel by user on Subject device
     *
     * @param requestId The id of the request
     */
    void RequestResultForCancel(const std::string &requestId);

    /**
     * This function will call when the request is cancel by user on Subject device
     *
     * @param requestId The id of the request
     */

    bool HaveSensitivePermissionToRequest(
        const std::string deviceId, const int32_t pid, const int32_t uid, const std::vector<std::string> permissions);

    bool CheckThirdAppMore(const std::string deviceId, const std::string permission);

private:
    // static constexpr char *TAG = "RequestRemotePermission";
    /**
     * The request UI time out, unit: second
     */
    static constexpr long REQUEST_DELAY = 60000;

    /**
     * The requestId -> RequestPermissionsInfo Map to record the current requests
     */
    std::map<std::string, std::shared_ptr<RequestPermissionsInfo>> requestInfos_;

    /**
     * The deviceId -> deviceId Map to record the current requests
     */
    std::map<std::string, std::string> requestingDevices_;
    sptr<AppExecFwk::IBundleMgr> iBundleManager_;
};
}  // namespace Permission
}  // namespace Security
}  // namespace OHOS
#endif