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

#include "permission_re_granter.h"

namespace OHOS {
namespace Security {
namespace Permission {
namespace {
static constexpr OHOS::HiviewDFX::HiLogLabel LABEL = {LOG_CORE, SECURITY_DOMAIN_PERMISSION, "PermissionReGranter"};
}
void PermissionReGranter::ReGrantDuidPermissions(UidBundleBo& uidBundlePermInfo)
{
    if (BaseRemoteCommand::IsValid(uidBundlePermInfo)) {
        PERMISSION_LOG_ERROR(LABEL, "ReGrantDuidPermissions:No remoteSensitivePermission need to reGrand");
        return;
    }

    PERMISSION_LOG_DEBUG(LABEL, "begin");

    for (auto item = uidBundlePermInfo.bundles.at(0).permissions.begin();
         item != uidBundlePermInfo.bundles.at(0).permissions.end(); item++) {
        PERMISSION_LOG_DEBUG(LABEL, "loop, status: %{public}d, grant mode: %{public}d", item->status, item->grantMode);
        PermissionDefParcel permInfo;

        GetPermissionInfoNoThrow(item->name, permInfo);

        if (item->grantMode == SYSTEM_GRANT && permInfo.permissionDef.availableScope != AVAILABLE_SCOPE_RESTRICTED) {
            SetStatusGranted(true, *item);
            PERMISSION_LOG_ERROR(LABEL,
                "ReGrantDuidPermissions: remote permission granted due to normal protect level");
            continue;
        }

        if (item->grantMode == USER_GRANT && permInfo.permissionDef.availableScope != AVAILABLE_SCOPE_RESTRICTED) {
            SetStatusGranted(IsRemoteGranted(*item), *item);
            PERMISSION_LOG_ERROR(LABEL,
                "ReGrantDuidPermissions: remote permission set granted due to remote granted status");
            continue;
        }

        if (item->grantMode == SYSTEM_GRANT && permInfo.permissionDef.availableScope == AVAILABLE_SCOPE_RESTRICTED) {
            bool isSignaturePermissionGranted = VerifySignatruePermission(permInfo, uidBundlePermInfo);
            SetStatusGranted(isSignaturePermissionGranted, *item);
            PERMISSION_LOG_ERROR(LABEL, "ReGrantDuidPermissions: remote permission set granted");
            continue;
        }

        SetStatusGranted(false, *item);
        PERMISSION_LOG_ERROR(LABEL,
            "ReGrantDuidPermissions: remote permission set no granted due to unknown availableScope");
    }

    PERMISSION_LOG_DEBUG(LABEL, "end");
}

void PermissionReGranter::GetPermissionInfoNoThrow(const std::string& permission, PermissionDefParcel& permInfo)
{
    std::unique_ptr<ExternalDeps> externalDeps = std::make_unique<ExternalDeps>();
    if (externalDeps == nullptr) {
        PERMISSION_LOG_INFO(LABEL, "getPermissionInfoNoThrow::externalDeps is nullptr.");
        return;
    }
    sptr<Permission::IPermissionManager> iPermissionManager_;
    iPermissionManager_ = externalDeps->GetPermissionManager(iPermissionManager_);
    iPermissionManager_->GetDefPermission(permission, permInfo);
}

bool PermissionReGranter::VerifySignatruePermission(const PermissionDefParcel& permInfo,
    const UidBundleBo& uidBundlePermInfo)
{
    sptr<AppExecFwk::IBundleMgr> iBundleManager_;
    std::shared_ptr<ExternalDeps> externalDeps = std::make_shared<ExternalDeps>();
    iBundleManager_ = externalDeps->GetBundleManager(iBundleManager_);

    AppExecFwk::BundleInfo bundleInfo;
    int result = iBundleManager_->GetBundleInfo(permInfo.permissionDef.bundleName,
        AppExecFwk::BundleFlag::GET_BUNDLE_DEFAULT, bundleInfo);
    if (!result) {
        PERMISSION_LOG_INFO(LABEL, "%{public}s cannot get bundleInfo by bundleName %{public}s", __func__,
            permInfo.permissionDef.bundleName.c_str());
        return false;
    }

    if (bundleInfo.defPermissions.size() != uidBundlePermInfo.remoteSensitivePermission.size()) {
        return false;
    }

    std::set<std::string>::iterator permName = uidBundlePermInfo.remoteSensitivePermission.begin();
    while (permName != uidBundlePermInfo.remoteSensitivePermission.end()) {
        if (find(bundleInfo.defPermissions.begin(), bundleInfo.defPermissions.end(), *permName) ==
            bundleInfo.defPermissions.end()) {
            return false;
        }
    }

    return true;
}

bool PermissionReGranter::IsGranted(const PermissionDto& permission)
{
    return (permission.status & FLAG_PERMISSION_STATUS_DISTRIBUTED_GRANTED) != 0;
}

bool PermissionReGranter::IsRemoteGranted(const PermissionDto& permission)
{
    return (permission.status & FLAG_PERMISSION_STATUS_REMOTE_GRANTED) != 0;
}

void PermissionReGranter::SetStatusGranted(const bool isGranted, PermissionDto& permission)
{
    PERMISSION_LOG_DEBUG(LABEL, "begin, isGranted: %{public}d, status: %{public}d", isGranted, permission.status);
    if (isGranted) {
        permission.status = (permission.status | FLAG_PERMISSION_STATUS_DISTRIBUTED_GRANTED) &
            (~FLAG_PERMISSION_STATUS_DISTRIBUTED_DENIED);
    } else {
        permission.status = (permission.status | FLAG_PERMISSION_STATUS_DISTRIBUTED_DENIED) &
            (~FLAG_PERMISSION_STATUS_DISTRIBUTED_GRANTED);
    }
    PERMISSION_LOG_DEBUG(LABEL, "end, isGranted: %{public}d, status: %{public}d", isGranted, permission.status);
}

void PermissionReGranter::SetFlagRemoteGranted(const bool isRemoteGranted, PermissionDto& permission)
{
    if (isRemoteGranted) {
        permission.status |= FLAG_PERMISSION_STATUS_REMOTE_GRANTED;
    } else {
        permission.status &= (~FLAG_PERMISSION_STATUS_REMOTE_GRANTED);
    }
}
} // namespace Permission
} // namespace Security
} // namespace OHOS