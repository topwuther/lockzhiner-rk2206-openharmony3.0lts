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
#include "mock_permission_mgr.h"

namespace OHOS {
namespace Security {
namespace Permission {
int PermissionManagerService::VerifyPermission(
    const std::string &packageName, const std::string &permissionName, int userId)
{
    return 0;
}
bool PermissionManagerService::CanRequestPermission(
    const std::string &bundleName, const std::string &permissionName, int userId)
{
    return true;
}
int PermissionManagerService::GrantUserGrantedPermission(
    const std::string &bundleName, const std::string &permissionName, int userId)
{
    return 0;
}
int PermissionManagerService::GrantSystemGrantedPermission(
    const std::string &bundleName, const std::string &permissionName)
{
    return 0;
}
int PermissionManagerService::RevokeUserGrantedPermission(
    const std::string &bundleName, const std::string &permissionName, int userId)
{
    return 0;
}
int PermissionManagerService::RevokeSystemGrantedPermission(
    const std::string &bundleName, const std::string &permissionName)
{
    return 0;
}
int PermissionManagerService::AddUserGrantedReqPermissions(
    const std::string &bundleName, const std::vector<std::string> &permList, int userId)
{
    return 0;
}
int PermissionManagerService::AddSystemGrantedReqPermissions(
    const std::string &bundleName, const std::vector<std::string> &permList)
{
    return 0;
}
int PermissionManagerService::RemoveUserGrantedReqPermissions(const std::string &bundleName, int userId)
{
    return 0;
}
int PermissionManagerService::RemoveSystemGrantedReqPermissions(const std::string &bundleName)
{
    return 0;
}
int PermissionManagerService::AddDefPermissions(const std::vector<PermissionDefParcel> &permDefList)
{
    return 0;
}
int PermissionManagerService::RemoveDefPermissions(const std::string &bundleName)
{
    return 0;
}
int PermissionManagerService::GetDefPermission(
    const std::string &permissionName, PermissionDefParcel &permissionDefResult)
{
    if (permissionName == "") {
        return -1;
    }
    int APP_ATTRIBUTE_SIGNED_WITH_PLATFORM_KEY = 1 << 2;
    if (permissionName == "ohos.permission.READ_CONTACTS" || permissionName == "ohos.permission.WRITE_CONTACTS") {
        permissionDefResult.permissionDef.permissionName = permissionName;
        permissionDefResult.permissionDef.bundleName = permissionName + "bundleName";
        permissionDefResult.permissionDef.grantMode = 0;
        permissionDefResult.permissionDef.availableScope = APP_ATTRIBUTE_SIGNED_WITH_PLATFORM_KEY;
        permissionDefResult.permissionDef.label = permissionName + "label";
        permissionDefResult.permissionDef.labelId = 0;
        permissionDefResult.permissionDef.description = permissionName + "description";
        permissionDefResult.permissionDef.descriptionId = 0;
        return 0;
    }
    permissionDefResult.permissionDef.permissionName = permissionName;
    permissionDefResult.permissionDef.bundleName = permissionName + "bundleName";
    permissionDefResult.permissionDef.grantMode = 0;
    permissionDefResult.permissionDef.availableScope = 0;
    permissionDefResult.permissionDef.label = permissionName + "label";
    permissionDefResult.permissionDef.labelId = 0;
    permissionDefResult.permissionDef.description = permissionName + "description";
    permissionDefResult.permissionDef.descriptionId = 0;

    return 0;
}
}  // namespace Permission
}  // namespace Security
}  // namespace OHOS