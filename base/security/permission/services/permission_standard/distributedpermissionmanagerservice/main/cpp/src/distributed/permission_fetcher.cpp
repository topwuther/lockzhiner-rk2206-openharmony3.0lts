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

#include "permission_fetcher.h"

#include "app_info_fetcher.h"
#include "permission_log.h"
#include "distributed_data_validator.h"
#include "permission_def_parcel.h"
#include "permission/permission.h"
#include "permission_kit.h"

namespace OHOS {
namespace Security {
namespace Permission {
namespace {
static constexpr OHOS::HiviewDFX::HiLogLabel LABEL = {LOG_CORE, SECURITY_DOMAIN_PERMISSION, "PermissionFetcher"};
}
PermissionFetcher::PermissionFetcher(
    sptr<AppExecFwk::IBundleMgr> iBundleManager, sptr<Permission::IPermissionManager> iPermissionManager)
    : iBundleManager_(iBundleManager), iPermissionManager_(iPermissionManager)
{}

int32_t PermissionFetcher::GetPermissions(int32_t uid, UidBundleBo &info)
{

    std::vector<std::string> bundleNames_;
    AppExecFwk::BundleInfo bundleInfo_;

    if (!DistributedDataValidator::IsUidValid(uid)) {
        PERMISSION_LOG_ERROR(LABEL, "invalid uid: %{public}d", uid);
        return Constant::FAILURE;
    }

    if (!iBundleManager_->GetBundlesForUid(uid, bundleNames_)) {
        PERMISSION_LOG_ERROR(LABEL, "getPermissions cannot get bundleName. uid: %{public}d", uid);
        return Constant::FAILURE;
    }

    if (bundleNames_.size() <= 0) {
        PERMISSION_LOG_ERROR(LABEL, "getPermissions cannot get bundleName. uid: %{public}d", uid);
        return Constant::FAILURE;
    }

    for (auto bundleName_ : bundleNames_) {

        PERMISSION_LOG_DEBUG(LABEL, "bundleName_ = %{public}s", bundleName_.c_str());

        if (!iBundleManager_->GetBundleInfo(bundleName_, AppExecFwk::BundleFlag::GET_BUNDLE_DEFAULT, bundleInfo_)) {
            PERMISSION_LOG_ERROR(
                LABEL, "getPermissions cannot get BundleInfo. bundleName: %{public}s", bundleName_.c_str());
            return Constant::CANNOT_GET_PACKAGE_FOR_UID;
        }

        appAttribute_ = AppInfoFetcher::GetInstance().GetAppAttribute(uid, bundleInfo_);
        if (appAttribute_ < 0) {
            PERMISSION_LOG_ERROR(LABEL, "getPermissions cannot get appAttribute. uid: %{public}d", uid);
            return Constant::FAILURE;
        }

        BundlePermissionsDto bundle = CreateBundlePermissionsInfo(uid, bundleInfo_, bundleName_);
        info.uid = uid;
        info.appAttribute = appAttribute_;
        info.DEFAULT_SIZE = 10;
        info.MIN_UID_PACKAGES_NUM = 1;
        info.bundles.push_back(bundle);
    }

    return Constant::SUCCESS;
}

BundlePermissionsDto PermissionFetcher::CreateBundlePermissionsInfo(
    int32_t uid, AppExecFwk::BundleInfo &bundleInfo, std::string &bundleName)
{
    PERMISSION_LOG_DEBUG(
        LABEL, "CreateBundlePermissionsInfo start: uid = %{public}d, bundleName = %{public}s", uid, bundleName.c_str());

    BundlePermissionsDto bundle;

    std::string appType_ = iBundleManager_->GetAppType(bundleName);

    PERMISSION_LOG_INFO(
        LABEL, "createBundlePermissionsInfo : {uid = %{public}d, appType = %{public}s }", uid, appType_.c_str());

    std::vector<PermissionDto> permissions;
    std::vector<std::string> reqPermissions = bundleInfo.reqPermissions;

    for (auto reqPermission : reqPermissions) {

        PERMISSION_LOG_DEBUG(LABEL, "Permission start: %{public}s ", reqPermission.c_str());

        PermissionDto permission;

        PermissionDefParcel permissionDefParcel;
        PermissionDef permissionDefResult = permissionDefParcel.permissionDef;

        if (Permission::PermissionKit::GetDefPermission(reqPermission, permissionDefResult) == 0) {
            permission.name = permissionDefResult.permissionName;
            permission.grantMode = permissionDefResult.grantMode;
            permission.status = 1;

            int32_t defInfo = 0;
            int16_t *pDefInfo = (int16_t *)&defInfo;
            *pDefInfo = permissionDefResult.grantMode;
            pDefInfo++;
            *pDefInfo = permissionDefResult.availableScope;
            permission.defInfo = defInfo;
        }

        if (Constant::UseByLocalApp(reqPermission)) {
            PERMISSION_LOG_DEBUG(LABEL, "filterPermission UseByLocalApp : %{public}s ", reqPermission.c_str());
            continue;
        }

        if (APP_TYPE_OTHER == appType_ && GrantMode::USER_GRANT == permission.grantMode) {
            PERMISSION_LOG_DEBUG(LABEL, "filterPermission OTHER USER_GRANT : %{public}s ", reqPermission.c_str());
            continue;
        }
        if (GrantMode::SYSTEM_GRANT == permission.grantMode) {
            PERMISSION_LOG_DEBUG(LABEL, "createBundlePermissionsInfo: push: %{public}s ", reqPermission.c_str());
            permission.type = Constant::SYSTEM_PERMISSION_TYPE;
            permissions.push_back(permission);
            continue;
        }
        if (iBundleManager_->CheckPermission(bundleInfo.name, reqPermission) == 0) {
            PERMISSION_LOG_DEBUG(LABEL, "createBundlePermissionsInfo: push: %{public}s ", reqPermission.c_str());
            permission.type = Constant::APP_PERMISSION_TYPE;
            permissions.push_back(permission);
            continue;
        }
    }

    bundle.permissions = permissions;
    bundle.name = bundleInfo.name;
    bundle.bundleLabel = bundleInfo.label;
    SignDto signDto;
    signDto.sha256 = bundleInfo.applicationInfo.signatureKey;
    bundle.sign.push_back(signDto);
    bundle.appId = bundleInfo.appId;

    return bundle;
}

int32_t PermissionFetcher::GetRegrantedPermissions(UidBundleBo &pInfo, UidBundleBo &rInfo)
{

    if (BaseRemoteCommand::IsValid(pInfo)) {
        PERMISSION_LOG_ERROR(LABEL, "getRegrantedPermissions: UidBundleBo is not valid. ");
        return Constant::FAILURE;
    }

    if (pInfo.appAttribute == Constant::INITIAL_APP_ATTRIBUTE) {
        PERMISSION_LOG_WARN(LABEL, "getRegrantedPermissions: old version, deal with non-third-party.");
        return Constant::FAILURE;
    }

    rInfo.uid = pInfo.uid;
    rInfo.appAttribute = pInfo.appAttribute;
    rInfo.DEFAULT_SIZE = 10;
    rInfo.MIN_UID_PACKAGES_NUM = 1;

    for (auto bundle : pInfo.bundles) {
        rInfo.bundles.push_back(MergeBundlePermissionsInfo(bundle));
    }

    return Constant::SUCCESS;
}

/**
 * Merges uid packages' permissions to the first package, remaining package info is keep.
 * Prepares app defined permissions for regranting.
 *
 * @param pInfo BundlePermissionsDto.
 * @return Merged BundlePermissionsDto.
 */
BundlePermissionsDto PermissionFetcher::MergeBundlePermissionsInfo(BundlePermissionsDto &pInfo)
{

    BundlePermissionsDto rInfo;
    std::string appType_ = iBundleManager_->GetAppType(pInfo.name);

    PERMISSION_LOG_DEBUG(LABEL,
        "MergeBundlePermissionsInfo : {name = %{public}s, appType = %{public}s }",
        pInfo.name.c_str(),
        appType_.c_str());

    std::vector<PermissionDto> tempPermissions;
    for (auto permission : pInfo.permissions) {

        PERMISSION_LOG_DEBUG(LABEL, "MergePermission : {permission = %{public}s }", permission.name.c_str());

        auto it = std::find_if(tempPermissions.begin(),
            tempPermissions.end(),
            [&permission](const auto &exisPermission) { return exisPermission.name == permission.name; });

        if (it != tempPermissions.end()) {
            continue;
        }
        tempPermissions.push_back(permission);

        if (APP_TYPE_OTHER == appType_ && GrantMode::USER_GRANT == permission.grantMode) {
            PERMISSION_LOG_DEBUG(LABEL,
                "filterPermission : APP_TYPE_OTHER and USER_GRANT {permission = %{public}s }",
                permission.name.c_str());
            continue;
        }
        rInfo.permissions.push_back(permission);
    }
    rInfo.name = pInfo.name;
    rInfo.bundleLabel = pInfo.bundleLabel;
    rInfo.sign = pInfo.sign;
    rInfo.appId = pInfo.appId;
    return rInfo;
}

}  // namespace Permission
}  // namespace Security
}  // namespace OHOS