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
#include "external_deps.h"

#include "bundle_mgr_interface.h"
#include "bundle_mgr_proxy.h"
#include "iservice_registry.h"
#include "permission_manager_client.h"

#include "permission_log.h"

namespace OHOS {
namespace Security {
namespace Permission {
namespace {
static constexpr OHOS::HiviewDFX::HiLogLabel LABEL = {LOG_CORE, SECURITY_DOMAIN_PERMISSION, "EXTERNAL_DEPS_MOCK"};
const Constant::ServiceId BUNDLE_MGR_SERVICE_SYS_ABILITY_ID = (Constant::ServiceId)401;
class MockPermissionManager : public IPermissionManager {
public:
    // IRemoteBroker
    sptr<IRemoteObject> AsObject() override
    {
        PERMISSION_LOG_ERROR(LABEL, "\n************ MockPermissionManager ERROR: not implemented *************\n");
        return nullptr;
    };

    // IPermissionManager
    PermissionManagerClient *pmc_ = &PermissionManagerClient::GetInstance();

    DECLARE_INTERFACE_DESCRIPTOR(u"ohos.security.permission.IPermissionManager");

    int VerifyPermission(const std::string &bundleName, const std::string &permissionName, int userId) override
    {
        return pmc_->VerifyPermission(bundleName, permissionName, userId);
    };

    bool CanRequestPermission(const std::string &bundleName, const std::string &permissionName, int userId) override
    {
        PERMISSION_LOG_ERROR(
            LABEL, "%{public}s,%{public}s,%{public}d", bundleName.c_str(), permissionName.c_str(), userId);
        return true;
    };

    int GrantUserGrantedPermission(
        const std::string &bundleName, const std::string &permissionName, int userId) override
    {
        return pmc_->GrantUserGrantedPermission(bundleName, permissionName, userId);
    };

    int GrantSystemGrantedPermission(const std::string &bundleName, const std::string &permissionName) override
    {
        return pmc_->GrantSystemGrantedPermission(bundleName, permissionName);
    };

    int RevokeUserGrantedPermission(
        const std::string &bundleName, const std::string &permissionName, int userId) override
    {
        return pmc_->RevokeUserGrantedPermission(bundleName, permissionName, userId);
    };

    int RevokeSystemGrantedPermission(const std::string &bundleName, const std::string &permissionName) override
    {
        return pmc_->RevokeSystemGrantedPermission(bundleName, permissionName);
    };

    int AddUserGrantedReqPermissions(
        const std::string &bundleName, const std::vector<std::string> &permList, int userId) override
    {
        return pmc_->AddUserGrantedReqPermissions(bundleName, permList, userId);
    };

    int AddSystemGrantedReqPermissions(const std::string &bundleName, const std::vector<std::string> &permList) override
    {
        return pmc_->AddSystemGrantedReqPermissions(bundleName, permList);
    };

    int RemoveUserGrantedReqPermissions(const std::string &bundleName, int userId) override
    {
        return pmc_->RemoveUserGrantedReqPermissions(bundleName, userId);
    };

    int RemoveSystemGrantedReqPermissions(const std::string &bundleName) override
    {
        return pmc_->RemoveSystemGrantedReqPermissions(bundleName);
    };

    int AddDefPermissions(const std::vector<PermissionDefParcel> &permDefList) override
    {
        std::vector<PermissionDef> defs;
        for (auto it : permDefList) {
            defs.push_back(it.permissionDef);
        }
        return pmc_->AddDefPermissions(defs);
    };

    int RemoveDefPermissions(const std::string &bundleName) override
    {
        return pmc_->RemoveSystemGrantedReqPermissions(bundleName);
    };

    int GetDefPermission(const std::string &permissionName, PermissionDefParcel &permissionDefResult) override
    {
        // from mock_permission_mgr.cpp
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
    };
};
}  // namespace

sptr<AppExecFwk::IBundleMgr> ExternalDeps::GetBundleManager(sptr<AppExecFwk::IBundleMgr> iBundleManager_)
{
    return iface_cast<AppExecFwk::IBundleMgr>(GetSystemAbility(BUNDLE_MGR_SERVICE_SYS_ABILITY_ID));
}

sptr<Permission::IPermissionManager> ExternalDeps::GetPermissionManager(
    sptr<Permission::IPermissionManager> iPermissionManager_)
{
    sptr<Permission::IPermissionManager> ipm = new MockPermissionManager;
    PERMISSION_LOG_INFO(LABEL,
        "get system ability(%{public}d) proxy success.",
        Constant::ServiceId::SUBSYS_SECURITY_PERMISSION_SYS_SERVICE_ID);
    return ipm;
}

sptr<AAFwk::IAbilityManager> ExternalDeps::GetAbilityManager(sptr<AAFwk::IAbilityManager> iAbilityManager_)
{
    PERMISSION_LOG_ERROR(LABEL, "system error. not mocked");
    return nullptr;
}

sptr<IRemoteObject> ExternalDeps::GetSystemAbility(const Constant::ServiceId systemAbilityId)
{
    sptr<ISystemAbilityManager> systemAbilityManager =
        SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    if (!systemAbilityManager) {
        PERMISSION_LOG_ERROR(LABEL, "fail to get system ability mgr.");
        return nullptr;
    }
    sptr<IRemoteObject> remoteObject = systemAbilityManager->GetSystemAbility(systemAbilityId);
    if (!remoteObject) {
        PERMISSION_LOG_ERROR(LABEL, "fail to get system ability proxy.");
        return nullptr;
    }
    PERMISSION_LOG_INFO(LABEL, "get system ability(%{public}d) proxy success.", systemAbilityId);
    return remoteObject;
}
}  // namespace Permission
}  // namespace Security
}  // namespace OHOS