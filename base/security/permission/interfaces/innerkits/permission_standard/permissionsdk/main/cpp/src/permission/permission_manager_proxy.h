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

#ifndef PERMISSION_MANAGER_PROXY_H
#define PERMISSION_MANAGER_PROXY_H

#include "i_permission_manager.h"

#include "iremote_proxy.h"

namespace OHOS {
namespace Security {
namespace Permission {
class PermissionManagerProxy : public IRemoteProxy<IPermissionManager> {
public:
    explicit PermissionManagerProxy(const sptr<IRemoteObject>& impl);
    virtual ~PermissionManagerProxy() override;

    int VerifyPermission(const std::string& packageName, const std::string& permissionName, int userId) override;

    bool CanRequestPermission(const std::string& bundleName, const std::string& permissionName, int userId) override;

    int GrantUserGrantedPermission(
        const std::string& bundleName, const std::string& permissionName, int userId) override;

    int GrantSystemGrantedPermission(const std::string& bundleName, const std::string& permissionName) override;

    int RevokeUserGrantedPermission(
        const std::string& bundleName, const std::string& permissionName, int userId) override;

    int RevokeSystemGrantedPermission(const std::string& bundleName, const std::string& permissionName) override;

    int AddUserGrantedReqPermissions(
        const std::string& bundleName, const std::vector<std::string>& permList, int userId) override;

    int AddSystemGrantedReqPermissions(
        const std::string& bundleName, const std::vector<std::string>& permList) override;

    int RemoveUserGrantedReqPermissions(const std::string& bundleName, int userId) override;

    int RemoveSystemGrantedReqPermissions(const std::string& bundleName) override;

    int AddDefPermissions(const std::vector<PermissionDefParcel>& permDefList) override;

    int RemoveDefPermissions(const std::string& bundleName) override;

    int GetDefPermission(const std::string& permissionName, PermissionDefParcel& permissionDefResult) override;

private:
    static inline BrokerDelegator<PermissionManagerProxy> delegator_;
};
} // namespace Permission
} // namespace Security
} // namespace OHOS
#endif // PERMISSION_MANAGER_PROXY_H
