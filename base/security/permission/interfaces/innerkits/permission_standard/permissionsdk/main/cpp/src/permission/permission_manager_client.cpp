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

#include "permission_manager_client.h"

#include "permission.h"
#include "permission_log.h"

#include "iservice_registry.h"

namespace OHOS {
namespace Security {
namespace Permission {
namespace {
static constexpr OHOS::HiviewDFX::HiLogLabel LABEL = {LOG_CORE, SECURITY_DOMAIN_PERMISSION, "PermissionManagerClient"};
} // namespace

PermissionManagerClient& PermissionManagerClient::GetInstance()
{
    static PermissionManagerClient instance;
    return instance;
}

PermissionManagerClient::PermissionManagerClient()
{}

PermissionManagerClient::~PermissionManagerClient()
{}

int PermissionManagerClient::VerifyPermission(
    const std::string& bundleName, const std::string& permissionName, int userId) const
{
    PERMISSION_LOG_DEBUG(LABEL, "%{public}s: called!", __func__);
    auto proxy = GetProxy();
    if (proxy == nullptr) {
        PERMISSION_LOG_ERROR(LABEL, "%{public}s: proxy is null", __func__);
        return PERMISSION_NOT_GRANTED;
    }
    return proxy->VerifyPermission(bundleName, permissionName, userId);
}

bool PermissionManagerClient::CanRequestPermission(
    const std::string& bundleName, const std::string& permissionName, int userId) const
{
    PERMISSION_LOG_DEBUG(LABEL, "%{public}s: called!", __func__);
    auto proxy = GetProxy();
    if (proxy == nullptr) {
        PERMISSION_LOG_ERROR(LABEL, "%{public}s: proxy is null", __func__);
        return false;
    }
    return proxy->CanRequestPermission(bundleName, permissionName, userId);
}

int PermissionManagerClient::GrantUserGrantedPermission(
    const std::string& bundleName, const std::string& permissionName, int userId) const
{
    PERMISSION_LOG_DEBUG(LABEL, "%{public}s: called!", __func__);
    auto proxy = GetProxy();
    if (proxy == nullptr) {
        PERMISSION_LOG_ERROR(LABEL, "%{public}s: proxy is null", __func__);
        return RET_FAILED;
    }
    return proxy->GrantUserGrantedPermission(bundleName, permissionName, userId);
}

int PermissionManagerClient::GrantSystemGrantedPermission(
    const std::string& bundleName, const std::string& permissionName) const
{
    PERMISSION_LOG_DEBUG(LABEL, "%{public}s: called!", __func__);
    auto proxy = GetProxy();
    if (proxy == nullptr) {
        PERMISSION_LOG_ERROR(LABEL, "%{public}s: proxy is null", __func__);
        return RET_FAILED;
    }
    return proxy->GrantSystemGrantedPermission(bundleName, permissionName);
}

int PermissionManagerClient::RevokeUserGrantedPermission(
    const std::string& bundleName, const std::string& permissionName, int userId) const
{
    PERMISSION_LOG_DEBUG(LABEL, "%{public}s: called!", __func__);
    auto proxy = GetProxy();
    if (proxy == nullptr) {
        PERMISSION_LOG_ERROR(LABEL, "%{public}s: proxy is null", __func__);
        return RET_FAILED;
    }
    return proxy->RevokeUserGrantedPermission(bundleName, permissionName, userId);
}

int PermissionManagerClient::RevokeSystemGrantedPermission(
    const std::string& bundleName, const std::string& permissionName) const
{
    PERMISSION_LOG_DEBUG(LABEL, "%{public}s: called!", __func__);
    auto proxy = GetProxy();
    if (proxy == nullptr) {
        PERMISSION_LOG_ERROR(LABEL, "%{public}s: proxy is null", __func__);
        return RET_FAILED;
    }
    return proxy->RevokeSystemGrantedPermission(bundleName, permissionName);
}

int PermissionManagerClient::AddUserGrantedReqPermissions(
    const std::string& bundleName, const std::vector<std::string>& permList, int userId) const
{
    PERMISSION_LOG_DEBUG(LABEL, "%{public}s: called!", __func__);
    auto proxy = GetProxy();
    if (proxy == nullptr) {
        PERMISSION_LOG_ERROR(LABEL, "%{public}s: proxy is null", __func__);
        return RET_FAILED;
    }
    return proxy->AddUserGrantedReqPermissions(bundleName, permList, userId);
}

int PermissionManagerClient::AddSystemGrantedReqPermissions(
    const std::string& bundleName, const std::vector<std::string>& permList) const
{
    PERMISSION_LOG_DEBUG(LABEL, "%{public}s: called!", __func__);
    auto proxy = GetProxy();
    if (proxy == nullptr) {
        PERMISSION_LOG_ERROR(LABEL, "%{public}s: proxy is null", __func__);
        return RET_FAILED;
    }
    return proxy->AddSystemGrantedReqPermissions(bundleName, permList);
}

int PermissionManagerClient::RemoveUserGrantedReqPermissions(const std::string& bundleName, int userId) const
{
    PERMISSION_LOG_DEBUG(LABEL, "%{public}s: called!", __func__);
    auto proxy = GetProxy();
    if (proxy == nullptr) {
        PERMISSION_LOG_ERROR(LABEL, "%{public}s: proxy is null", __func__);
        return RET_FAILED;
    }
    return proxy->RemoveUserGrantedReqPermissions(bundleName, userId);
}

int PermissionManagerClient::RemoveSystemGrantedReqPermissions(const std::string& bundleName) const
{
    PERMISSION_LOG_DEBUG(LABEL, "%{public}s: called!", __func__);
    auto proxy = GetProxy();
    if (proxy == nullptr) {
        PERMISSION_LOG_ERROR(LABEL, "%{public}s: proxy is null", __func__);
        return RET_FAILED;
    }
    return proxy->RemoveSystemGrantedReqPermissions(bundleName);
}

int PermissionManagerClient::AddDefPermissions(const std::vector<PermissionDef>& permDefList) const
{
    PERMISSION_LOG_DEBUG(LABEL, "%{public}s: called!", __func__);
    auto proxy = GetProxy();
    if (proxy == nullptr) {
        PERMISSION_LOG_ERROR(LABEL, "%{public}s: proxy is null", __func__);
        return RET_FAILED;
    }
    std::vector<PermissionDefParcel> parcelList;
    for (auto permDef : permDefList) {
        PermissionDefParcel parcel;
        parcel.permissionDef = permDef;
        parcelList.emplace_back(parcel);
    }
    PERMISSION_LOG_INFO(LABEL, "%{public}s called, permList size: %{public}d", __func__, (int) parcelList.size());
    return proxy->AddDefPermissions(parcelList);
}

int PermissionManagerClient::RemoveDefPermissions(const std::string& bundleName) const
{
    PERMISSION_LOG_DEBUG(LABEL, "%{public}s: called!", __func__);
    auto proxy = GetProxy();
    if (proxy == nullptr) {
        PERMISSION_LOG_ERROR(LABEL, "%{public}s: proxy is null", __func__);
        return RET_FAILED;
    }
    return proxy->RemoveDefPermissions(bundleName);
}

int PermissionManagerClient::GetDefPermission(
    const std::string& permissionName, PermissionDef& permissionDefResult) const
{
    PERMISSION_LOG_DEBUG(LABEL, "%{public}s: called!", __func__);
    auto proxy = GetProxy();
    if (proxy == nullptr) {
        PERMISSION_LOG_ERROR(LABEL, "%{public}s: proxy is null", __func__);
        return RET_FAILED;
    }
    PermissionDefParcel permissionDefParcel;
    int result = proxy->GetDefPermission(permissionName, permissionDefParcel);
    permissionDefResult = permissionDefParcel.permissionDef;
    return result;
}

sptr<IPermissionManager> PermissionManagerClient::GetProxy() const
{
    auto sam = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    if (sam == nullptr) {
        PERMISSION_LOG_DEBUG(LABEL, "%{public}s: GetSystemAbilityManager is null", __func__);
        return nullptr;
    }
    auto permissionSa = sam->GetSystemAbility(IPermissionManager::SA_ID_PERMISSION_MANAGER_SERVICE);
    if (permissionSa == nullptr) {
        PERMISSION_LOG_DEBUG(LABEL, "%{public}s: GetSystemAbility %{public}d is null", __func__,
            IPermissionManager::SA_ID_PERMISSION_MANAGER_SERVICE);
        return nullptr;
    }

    auto proxy = iface_cast<IPermissionManager>(permissionSa);
    if (proxy == nullptr) {
        PERMISSION_LOG_DEBUG(LABEL, "%{public}s: iface_cast get null", __func__);
        return nullptr;
    }
    return proxy;
}
} // namespace Permission
} // namespace Security
} // namespace OHOS