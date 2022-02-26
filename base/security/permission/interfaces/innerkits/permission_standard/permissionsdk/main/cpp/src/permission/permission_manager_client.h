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

#ifndef PERMISSION_MANAGER_CLIENT_H
#define PERMISSION_MANAGER_CLIENT_H

#include <string>

#include "i_permission_manager.h"
#include "permission/permission_def.h"

#include "nocopyable.h"

namespace OHOS {
namespace Security {
namespace Permission {
class PermissionManagerClient final {
public:
    static PermissionManagerClient& GetInstance();

    virtual ~PermissionManagerClient();

    int VerifyPermission(const std::string& bundleName, const std::string& permissionName, int userId) const;

    bool CanRequestPermission(const std::string& bundleName, const std::string& permissionName, int userId) const;

    int GrantUserGrantedPermission(const std::string& bundleName, const std::string& permissionName, int userId) const;

    int GrantSystemGrantedPermission(const std::string& bundleName, const std::string& permissionName) const;

    int RevokeUserGrantedPermission(const std::string& bundleName, const std::string& permissionName, int userId) const;

    int RevokeSystemGrantedPermission(const std::string& bundleName, const std::string& permissionName) const;

    int AddUserGrantedReqPermissions(
        const std::string& bundleName, const std::vector<std::string>& permList, int userId) const;

    int AddSystemGrantedReqPermissions(const std::string& bundleName, const std::vector<std::string>& permList) const;

    int RemoveUserGrantedReqPermissions(const std::string& bundleName, int userId) const;

    int RemoveSystemGrantedReqPermissions(const std::string& bundleName) const;

    int AddDefPermissions(const std::vector<PermissionDef>& permDefList) const;

    int RemoveDefPermissions(const std::string& bundleName) const;

    int GetDefPermission(const std::string& permissionName, PermissionDef& permissionDefResult) const;

private:
    PermissionManagerClient();

    DISALLOW_COPY_AND_MOVE(PermissionManagerClient);

    sptr<IPermissionManager> GetProxy() const;
};
} // namespace Permission
} // namespace Security
} // namespace OHOS
#endif // PERMISSION_MANAGER_CLIENT_H
