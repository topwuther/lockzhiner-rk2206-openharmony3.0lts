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

#include "permission_manager_service.h"

#include "permission.h"
#include "permission_definition_manager.h"
#include "permission_state_manager.h"
#include "permission_log.h"

namespace OHOS {
namespace Security {
namespace Permission {
namespace {
static constexpr OHOS::HiviewDFX::HiLogLabel LABEL = {LOG_CORE, SECURITY_DOMAIN_PERMISSION, "PermissionManagerService"};
}

const bool REGISTER_RESULT =
    SystemAbility::MakeAndRegisterAbility(DelayedSingleton<PermissionManagerService>::GetInstance().get());

PermissionManagerService::PermissionManagerService()
    : SystemAbility(SA_ID_PERMISSION_MANAGER_SERVICE, true), state_(ServiceRunningState::STATE_NOT_START)
{
    PERMISSION_LOG_INFO(LABEL, "PermissionManagerService()");
}

PermissionManagerService::~PermissionManagerService()
{
    PERMISSION_LOG_INFO(LABEL, "~PermissionManagerService()");
}

void PermissionManagerService::OnStart()
{
    if (state_ == ServiceRunningState::STATE_RUNNING) {
        PERMISSION_LOG_INFO(LABEL, "PermissionManagerService has already started!");
        return;
    }
    PERMISSION_LOG_INFO(LABEL, "PermissionManagerService is starting");
    if (!Initialize()) {
        PERMISSION_LOG_ERROR(LABEL, "Failed to initialize");
        return;
    }
    state_ = ServiceRunningState::STATE_RUNNING;
    bool ret = Publish(DelayedSingleton<PermissionManagerService>::GetInstance().get());
    if (!ret) {
        PERMISSION_LOG_ERROR(LABEL, "Failed to publish service!");
        return;
    }
    PERMISSION_LOG_INFO(LABEL, "Congratulations, PermissionManagerService start successfully!");
}

void PermissionManagerService::OnStop()
{
    PERMISSION_LOG_INFO(LABEL, "stop service");
    state_ = ServiceRunningState::STATE_NOT_START;
}

int PermissionManagerService::VerifyPermission(
    const std::string& bundleName, const std::string& permissionName, int userId)
{
    PERMISSION_LOG_INFO(LABEL,
        "%{public}s called, packageName: %{public}s, permissionName: %{public}s, userId: %{public}d", __func__,
        bundleName.c_str(), permissionName.c_str(), userId);
    return PermissionStateManager::GetInstance().VerifyPermission(bundleName, permissionName, userId);
}

bool PermissionManagerService::CanRequestPermission(
    const std::string& bundleName, const std::string& permissionName, int userId)
{
    PERMISSION_LOG_INFO(LABEL,
        "%{public}s called, bundleName: %{public}s, permissionName: %{public}s, userId: %{public}d", __func__,
        bundleName.c_str(), permissionName.c_str(), userId);
    return PermissionStateManager::GetInstance().CanRequestPermission(bundleName, permissionName, userId);
}

int PermissionManagerService::GrantUserGrantedPermission(
    const std::string& bundleName, const std::string& permissionName, int userId)
{
    PERMISSION_LOG_INFO(LABEL,
        "%{public}s called, bundleName: %{public}s, permissionName: %{public}s, userId: %{public}d", __func__,
        bundleName.c_str(), permissionName.c_str(), userId);
    PermissionStateManager::GetInstance().GrantUserGrantedPermission(bundleName, permissionName, userId);
    return RET_SUCCESS;
}

int PermissionManagerService::GrantSystemGrantedPermission(
    const std::string& bundleName, const std::string& permissionName)
{
    PERMISSION_LOG_INFO(LABEL, "%{public}s called, bundleName: %{public}s, permissionName: %{public}s", __func__,
        bundleName.c_str(), permissionName.c_str());
    PermissionStateManager::GetInstance().GrantSystemGrantedPermission(bundleName, permissionName);
    return RET_SUCCESS;
}

int PermissionManagerService::RevokeUserGrantedPermission(
    const std::string& bundleName, const std::string& permissionName, int userId)
{
    PERMISSION_LOG_INFO(LABEL,
        "%{public}s called, bundleName: %{public}s, permissionName: %{public}s, userId: %{public}d", __func__,
        bundleName.c_str(), permissionName.c_str(), userId);
    PermissionStateManager::GetInstance().RevokeUserGrantedPermission(bundleName, permissionName, userId);
    return RET_SUCCESS;
}

int PermissionManagerService::RevokeSystemGrantedPermission(
    const std::string& bundleName, const std::string& permissionName)
{
    PERMISSION_LOG_INFO(LABEL, "%{public}s called, bundleName: %{public}s, permissionName: %{public}s", __func__,
        bundleName.c_str(), permissionName.c_str());
    PermissionStateManager::GetInstance().RevokeSystemGrantedPermission(bundleName, permissionName);
    return RET_SUCCESS;
}

int PermissionManagerService::AddUserGrantedReqPermissions(
    const std::string& bundleName, const std::vector<std::string>& permList, int userId)
{
    PERMISSION_LOG_INFO(LABEL,
        "%{public}s called, bundleName: %{public}s, permList size: %{public}d, userId: %{public}d", __func__,
        bundleName.c_str(), (int) permList.size(), userId);
    PermissionStateManager::GetInstance().AddUserGrantedReqPermissions(bundleName, permList, userId);
    return RET_SUCCESS;
}

int PermissionManagerService::AddSystemGrantedReqPermissions(
    const std::string& bundleName, const std::vector<std::string>& permList)
{
    PERMISSION_LOG_INFO(LABEL, "%{public}s called, bundleName: %{public}s, permList size: %{public}d", __func__,
        bundleName.c_str(), (int) permList.size());
    PermissionStateManager::GetInstance().AddSystemGrantedReqPermissions(bundleName, permList);
    return RET_SUCCESS;
}

int PermissionManagerService::RemoveUserGrantedReqPermissions(const std::string& bundleName, int userId)
{
    PERMISSION_LOG_INFO(
        LABEL, "%{public}s called, bundleName: %{public}s, userId: %{public}d", __func__, bundleName.c_str(), userId);
    PermissionStateManager::GetInstance().RemoveUserGrantedReqPermissions(bundleName, userId);
    return RET_SUCCESS;
}

int PermissionManagerService::RemoveSystemGrantedReqPermissions(const std::string& bundleName)
{
    PERMISSION_LOG_INFO(LABEL, "%{public}s called, bundleName: %{public}s", __func__, bundleName.c_str());
    PermissionStateManager::GetInstance().RemoveSystemGrantedReqPermissions(bundleName);
    return RET_SUCCESS;
}

int PermissionManagerService::AddDefPermissions(const std::vector<PermissionDefParcel>& permDefList)
{
    PERMISSION_LOG_INFO(LABEL, "%{public}s called, permList size: %{public}d", __func__, (int) permDefList.size());
    PermissionDefinitionManager::GetInstance().AddDefPermissions(permDefList);
    return RET_SUCCESS;
}

int PermissionManagerService::RemoveDefPermissions(const std::string& bundleName)
{
    PERMISSION_LOG_INFO(LABEL, "%{public}s called, bundleName: %{public}s", __func__, bundleName.c_str());
    PermissionDefinitionManager::GetInstance().RemoveDefPermissions(bundleName);
    return RET_SUCCESS;
}

int PermissionManagerService::GetDefPermission(
    const std::string& permissionName, PermissionDefParcel& permissionDefResult)
{
    PERMISSION_LOG_INFO(LABEL, "%{public}s called, bundleName: %{public}s", __func__, permissionName.c_str());
    return PermissionDefinitionManager::GetInstance().GetDefPermission(permissionName, permissionDefResult);
}

bool PermissionManagerService::Initialize() const
{
    PermissionDefinitionManager::GetInstance().Init();
    PermissionStateManager::GetInstance().Init();
    return true;
}
} // namespace Permission
} // namespace Security
} // namespace OHOS