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

#ifndef PERMISSION_INNERKITS_INCLUDE_DISRIBUTED_PERMISSION_H
#define PERMISSION_INNERKITS_INCLUDE_DISRIBUTED_PERMISSION_H

#include "base64_util.h"
#include "idistributed_permission.h"
#include "zip_util.h"

namespace OHOS {
namespace Security {
namespace Permission {
class DistributedPermissionManagerClient {
public:
    static DistributedPermissionManagerClient &GetInstance();

    int32_t AllocateDuid(const std::string &nodeId, const int32_t rUid);

    int32_t QueryDuid(const std::string &deviceId, int32_t rUid);

    int32_t CheckDPermission(int32_t dUid, const std::string &permissionName);

    int32_t CheckPermission(const std::string &permissionName, const std::string &nodeId, int32_t pid, int32_t uid);

    int32_t CheckPermission(const std::string &permissionName, const std::string &appIdInfo);

    int32_t CheckSelfPermission(const std::string &permissionName);

    int32_t CheckCallingPermission(const std::string &permissionName);

    int32_t CheckCallingOrSelfPermission(const std::string &permissionName);

    int32_t CheckCallerPermission(const std::string &permissionName);

    bool IsRestrictedPermission(const std::string &permissionName);

    int32_t VerifyPermissionFromRemote(
        const std::string &permission, const std::string &nodeId, const std::string &appIdInfo);

    int32_t VerifySelfPermissionFromRemote(const std::string &permissionName, const std::string &nodeId);

    bool CanRequestPermissionFromRemote(const std::string &permissionName, const std::string &nodeId);

    void RequestPermissionsFromRemote(const std::vector<std::string> permissions,
        const sptr<OnRequestPermissionsResult> &callback, const std::string &nodeId, const std::string &bundleName,
        int32_t reasonResId);

    void GrantSensitivePermissionToRemoteApp(
        const std::string &permissionName, const std::string &nodeId, int32_t ruid);

    int32_t RegisterUsingPermissionReminder(const sptr<OnUsingPermissionReminder> &callback);

    int32_t UnregisterUsingPermissionReminder(const sptr<OnUsingPermissionReminder> &callback);

    int32_t CheckPermissionAndStartUsing(const std::string &permissionName, const std::string &appIdInfo);

    int32_t CheckCallerPermissionAndStartUsing(const std::string &permissionName);

    void StartUsingPermission(const std::string &permName, const std::string &appIdInfo);

    void StopUsingPermission(const std::string &permName, const std::string &appIdInfo);

    void AddPermissionUsedRecord(const std::string &permissionName, const std::string &appIdInfo,
        const int32_t sucCount, const int32_t failCount);

    int32_t GetPermissionUsedRecords(const QueryPermissionUsedRequest &request, QueryPermissionUsedResult &result);

    int32_t GetPermissionUsedRecords(
        const QueryPermissionUsedRequest &request, const sptr<OnPermissionUsedRecord> &callback);

    void ResetDistributedPermissionProxy();

private:
    bool GetDistributedPermissionProxy();

private:
    std::mutex mutex_;
    sptr<IDistributedPermission> distributedPermissionProxy_;
    sptr<IRemoteObject::DeathRecipient> recipient_;
};
}  // namespace Permission
}  // namespace Security
}  // namespace OHOS

#endif  // PERMISSION_INNERKITS_INCLUDE_DISRIBUTED_PERMISSION_H