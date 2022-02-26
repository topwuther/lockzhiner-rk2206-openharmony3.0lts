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

#include "permission_bms_manager.h"
#include "monitor_manager.h"
#include "distributed_data_validator.h"

namespace OHOS {
namespace Security {
namespace Permission {
namespace {
static constexpr OHOS::HiviewDFX::HiLogLabel LABEL = {LOG_CORE, SECURITY_DOMAIN_PERMISSION, "PermissionBmsManager"};
}
namespace {
static const std::string TAG = "PERMISSION_BMS_MANAGER";
}

PermissionBmsManager::PermissionBmsManager()
    : iBundleManager_(nullptr), iPermissionManager_(nullptr), permissionFetcher_(nullptr), appInfoFetcher_(nullptr)
{
    PERMISSION_LOG_DEBUG(LABEL, "PermissionBmsManager()");
}

PermissionBmsManager::~PermissionBmsManager()
{
    PERMISSION_LOG_DEBUG(LABEL, "~PermissionBmsManager()");
}

PermissionBmsManager &PermissionBmsManager::GetInstance()
{
    static PermissionBmsManager instance;
    return instance;
}

int PermissionBmsManager::Init()
{
    PERMISSION_LOG_DEBUG(LABEL, "Init begin");
    appInfoFetcher_ = std::make_shared<AppInfoFetcher>();
    if (appInfoFetcher_ == nullptr) {
        PERMISSION_LOG_ERROR(LABEL, "construct AppInfoFetcher failed.");
        return Constant::FAILURE;
    }
    std::unique_ptr<ExternalDeps> externalDeps = std::make_unique<ExternalDeps>();
    if (externalDeps == nullptr) {
        PERMISSION_LOG_ERROR(LABEL, "construct ExternalDeps failed.");
        return Constant::FAILURE;
    }
    iBundleManager_ = externalDeps->GetBundleManager(iBundleManager_);
    iPermissionManager_ = externalDeps->GetPermissionManager(iPermissionManager_);
    permissionFetcher_ = std::make_shared<PermissionFetcher>(iBundleManager_, iPermissionManager_);
    if (permissionFetcher_ == nullptr) {
        PERMISSION_LOG_ERROR(LABEL, "construct PermissionFetcher failed.");
        return Constant::FAILURE;
    }

    PermissionListener::Init();

    std::function<void(int uid, const std::string &packageName)> callback =
        std::bind(&AppInfoFetcher::RemoveAttributeBuffer, appInfoFetcher_, std::placeholders::_1);
    auto callbackPtr = std::make_shared<MonitorManager::OnPermissionChangedCallback>(callback);
    MonitorManager::GetInstance().AddOnPermissionChangedListener(TAG, callbackPtr);

    PERMISSION_LOG_DEBUG(LABEL, "Init end");
    return Constant::SUCCESS;
}

int32_t PermissionBmsManager::GetPermissions(int32_t uid, UidBundleBo &info)
{
    if (!DistributedDataValidator::IsUidValid(uid)) {
        PERMISSION_LOG_ERROR(LABEL, "parameter invalid. uid: %{public}d", uid);
        return Constant::FAILURE;
    }

    if (permissionFetcher_ == nullptr) {
        PERMISSION_LOG_ERROR(LABEL, "PermissionFetcher is not inited.");
        return Constant::FAILURE;
    }

    return permissionFetcher_->GetPermissions(uid, info);
}

int32_t PermissionBmsManager::GetRegrantedPermissions(UidBundleBo &pInfo, UidBundleBo &rInfo)
{
    if (permissionFetcher_ == nullptr) {
        PERMISSION_LOG_ERROR(LABEL, "PermissionFetcher is not inited.");
        return Constant::FAILURE;
    }
    return permissionFetcher_->GetRegrantedPermissions(pInfo, rInfo);
}

bool PermissionBmsManager::IsSystemSignatureUid(const int32_t &uid)
{
    if (uid < 0) {
        return false;
    }
    if (uid % Constant::PER_USER_RANGE < Constant::FIRST_APPLICATION_UID) {
        return true;
    }

    std::shared_ptr<ExternalDeps> externalDeps = std::make_shared<ExternalDeps>();
    iBundleManager_ = externalDeps->GetBundleManager(iBundleManager_);

    std::vector<std::string> bundleNames;
    if (!iBundleManager_->GetBundlesForUid(uid, bundleNames)) {
        PERMISSION_LOG_ERROR(LABEL, "getPermissions cannot get bundleName. uid: %{public}d", uid);
        return false;
    }

    for (auto bundleName : bundleNames) {
        AppExecFwk::BundleInfo bundleInfo;
        if (iBundleManager_->GetBundleInfo(bundleName, AppExecFwk::BundleFlag::GET_BUNDLE_DEFAULT, bundleInfo)) {
            PERMISSION_LOG_ERROR(
                LABEL, "getPermissions cannot get BundleInfo. bundleName: %{public}s", bundleName.c_str());
            return false;
        }

        int32_t attribute = AppInfoFetcher::GetInstance().GetAppAttribute(uid, bundleInfo);
        if (attribute < 0) {
            PERMISSION_LOG_ERROR(LABEL, "getPermissions cannot get appAttribute. uid: %{public}d", uid);
            return false;
        }
        int32_t key = static_cast<int32_t>(AppAttribute::APP_ATTRIBUTE_SIGNED_WITH_PLATFORM_KEY);

        if ((attribute & key) != 0) {
            return false;
        }
    }
    return true;
}

int PermissionBmsManager::InitSystemDefinedPermissions()
{
    if (permissionFetcher_ == nullptr) {
        PERMISSION_LOG_ERROR(LABEL, "PermissionFetcher is not inited.");
        return Constant::FAILURE;
    }

    PERMISSION_LOG_DEBUG(LABEL, "InitSystemDefinedPermissions should not used.");
    return Constant::SUCCESS;
}

void PermissionBmsManager::ReGrantDuidPermissions(UidBundleBo &uidBundlePermInfo)
{
    if (uidBundlePermInfo.remoteSensitivePermission.size() == 0) {
        PERMISSION_LOG_ERROR(LABEL, "ReGrantDuidPermissions:invalid params.");
        return;
    }

    PermissionReGranter::ReGrantDuidPermissions(uidBundlePermInfo);
}

}  // namespace Permission
}  // namespace Security
}  // namespace OHOS
