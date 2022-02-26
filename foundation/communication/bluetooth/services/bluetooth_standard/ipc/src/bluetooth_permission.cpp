/*
 * Copyright (C) 2021 Huawei Device Co., Ltd.
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

#include "bluetooth_permission.h"

#include <string>
#include "ipc_skeleton.h"

namespace OHOS {
namespace Bluetooth {
bool PermissionUtil::CheckCallerPermission(const string &permissionName)
{
    int auth = Security::Permission::PermissionKit::CheckCallingPermission(permissionName);
    return auth == Security::Permission::PermissionKit::GRANTED;
}

bool PermissionUtil::CheckCallerPermission(const string &permissionName, pid_t pid, uid_t uid)
{
    string appIdInfo = Security::Permission::AppIdInfoHelper::CreateAppIdInfo(pid, uid);
    int auth = Security::Permission::PermissionKit::CheckPermission(permissionName, appIdInfo);
    return auth == Security::Permission::PermissionKit::GRANTED;
}

bool PermissionUtil::CheckPermissionByPidUid(const string &permissionName)
{
    string appIdInfo = FindAppIdInfo();
    int auth = Security::Permission::PermissionKit::CheckPermission(permissionName, appIdInfo);
    return auth == Security::Permission::PermissionKit::GRANTED;
}

string PermissionUtil::FindAppIdInfo()
{
    pid_t pid = IPCSkeleton::GetCallingPid();
    uid_t uid = IPCSkeleton::GetCallingUid();
    return Security::Permission::AppIdInfoHelper::CreateAppIdInfo(pid, uid);
}
}  // namespace Bluetooth
}  // namespace OHOS