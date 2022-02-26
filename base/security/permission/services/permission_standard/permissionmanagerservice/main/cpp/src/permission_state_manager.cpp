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

#include "permission_state_manager.h"

#include "permission.h"
#include "data_storage.h"
#include "permission_log.h"
#include "permission_definition_cache.h"
#include "permission_state_cache.h"
#include "data_translator.h"
#include "data_validator.h"

namespace OHOS {
namespace Security {
namespace Permission {
namespace {
static constexpr OHOS::HiviewDFX::HiLogLabel LABEL = {LOG_CORE, SECURITY_DOMAIN_PERMISSION, "PermissionStateManager"};
}
PermissionStateManager& PermissionStateManager::GetInstance()
{
    static PermissionStateManager instance;
    return instance;
}

PermissionStateManager::PermissionStateManager() : hasInited_(false)
{}

PermissionStateManager::~PermissionStateManager()
{
    if (!hasInited_) {
        return;
    }
    this->systemGrantedDataAccessWorker_.Stop();
    this->userGrantedDataAccessWorker_.Stop();
    this->hasInited_ = false;
}

void PermissionStateManager::Init()
{
    OHOS::Utils::UniqueWriteGuard<OHOS::Utils::RWLock> lk(this->rwLock_);
    if (hasInited_) {
        return;
    }
    PERMISSION_LOG_INFO(LABEL, "init begin!");
    std::vector<GenericValues> results;
    DataStorage::GetRealDataStorage().Find(DataStorage::PERMISSIONS_STAT_SYSTEM_GRANTED, results);
    for (auto value : results) {
        std::string bundleName = value.GetString(FIELD_BUNDLE_NAME);
        PERMISSION_LOG_INFO(LABEL, "Cache recover system grant %{public}s", bundleName.c_str());
        PermissionReq permissionReq;
        DataTranslator::TranslationIntoPermissionReq(value, permissionReq);
        PermissionStateCache::GetInstance().ReplaceReqPermission(
            PermissionStateCache::SYSTEM_GRANTED_USER_ID, bundleName, permissionReq);
    }

    DataStorage::GetRealDataStorage().Find(DataStorage::PERMISSIONS_STAT_USER_GRANTED, results);
    for (auto value : results) {
        std::string bundleName = value.GetString(FIELD_BUNDLE_NAME);
        int userId = value.GetInt(FIELD_USER_ID);
        PERMISSION_LOG_INFO(LABEL, "Cache recover user grant %{public}d, %{public}s", userId, bundleName.c_str());
        PermissionReq permissionReq;
        DataTranslator::TranslationIntoPermissionReq(value, permissionReq);
        PermissionStateCache::GetInstance().ReplaceReqPermission(userId, bundleName, permissionReq);
    }
    this->systemGrantedDataAccessWorker_.Start(1);
    this->userGrantedDataAccessWorker_.Start(1);
    hasInited_ = true;
    PERMISSION_LOG_INFO(LABEL, "Init success");
}

void PermissionStateManager::AddUserGrantedReqPermissions(
    const std::string& bundleName, const std::vector<std::string>& permList, int userId)
{
    if (!DataValidator::IsBundleNameValid(bundleName) || permList.empty() || !DataValidator::IsUserIdValid(userId)) {
        PERMISSION_LOG_ERROR(LABEL, "%{public}s: invalid params!", __func__);
        return;
    }

    std::vector<GenericValues> values;
    for (const auto& permission : permList) {
        if (!PermissionDefinitionCache::GetInstance().IsUserGrantedPermission(permission)) {
            PERMISSION_LOG_ERROR(LABEL,
                "%{public}s: permission: %{public}s is not user-granted permission, check please!", __func__,
                permission.c_str());
            continue;
        }
        PermissionStateCache::GetInstance().AddReqPermission(userId, bundleName, permission);
    }
    RefreshUserGrantedPersistentDataIfNeeded();
}

void PermissionStateManager::RemoveUserGrantedReqPermissions(const std::string& bundleName, int userId)
{
    if (!DataValidator::IsBundleNameValid(bundleName) || !DataValidator::IsUserIdValid(userId)) {
        PERMISSION_LOG_ERROR(LABEL, "%{public}s: invalid params!", __func__);
        return;
    }
    PermissionStateCache::GetInstance().RemoveReqPermission(userId, bundleName);
    RefreshUserGrantedPersistentDataIfNeeded();
}

void PermissionStateManager::GrantUserGrantedPermission(
    const std::string& bundleName, const std::string& permissionName, int userId)
{
    if (!DataValidator::IsBundleNameValid(bundleName) || !DataValidator::IsPermissionNameValid(permissionName) ||
        !DataValidator::IsUserIdValid(userId)) {
        PERMISSION_LOG_ERROR(LABEL, "%{public}s: invalid params!", __func__);
        return;
    }
    PermissionStateCache::GetInstance().UpdatePermissionState(userId, bundleName, permissionName, true);
    RefreshUserGrantedPersistentDataIfNeeded();
}

void PermissionStateManager::RevokeUserGrantedPermission(
    const std::string& bundleName, const std::string& permissionName, int userId)
{
    if (!DataValidator::IsBundleNameValid(bundleName) || !DataValidator::IsPermissionNameValid(permissionName) ||
        !DataValidator::IsUserIdValid(userId)) {
        PERMISSION_LOG_ERROR(LABEL, "%{public}s: invalid params!", __func__);
        return;
    }
    PermissionStateCache::GetInstance().UpdatePermissionState(userId, bundleName, permissionName, false);
    RefreshUserGrantedPersistentDataIfNeeded();
}

void PermissionStateManager::AddSystemGrantedReqPermissions(
    const std::string& bundleName, const std::vector<std::string>& permList)
{
    if (!DataValidator::IsBundleNameValid(bundleName) || permList.empty()) {
        PERMISSION_LOG_ERROR(LABEL, "%{public}s: invalid params!", __func__);
        return;
    }

    std::vector<GenericValues> values;
    for (const auto& permission : permList) {
        if (!PermissionDefinitionCache::GetInstance().IsSystemGrantedPermission(permission)) {
            PERMISSION_LOG_ERROR(LABEL,
                "%{public}s: permission: %{public}s is not system-granted permission, check please!", __func__,
                permission.c_str());
            continue;
        }
        PermissionStateCache::GetInstance().AddReqPermission(
            PermissionStateCache::SYSTEM_GRANTED_USER_ID, bundleName, permission);
    }
    RefreshSystemGrantedPersistentDataIfNeeded();
}

void PermissionStateManager::RemoveSystemGrantedReqPermissions(const std::string& bundleName)
{
    if (!DataValidator::IsBundleNameValid(bundleName)) {
        PERMISSION_LOG_ERROR(LABEL, "%{public}s: invalid params!", __func__);
        return;
    }
    PermissionStateCache::GetInstance().RemoveReqPermission(PermissionStateCache::SYSTEM_GRANTED_USER_ID, bundleName);
    RefreshSystemGrantedPersistentDataIfNeeded();
}

void PermissionStateManager::GrantSystemGrantedPermission(
    const std::string& bundleName, const std::string& permissionName)
{
    if (!DataValidator::IsBundleNameValid(bundleName) || !DataValidator::IsPermissionNameValid(permissionName)) {
        PERMISSION_LOG_ERROR(LABEL, "%{public}s: invalid params!", __func__);
        return;
    }
    PermissionStateCache::GetInstance().UpdatePermissionState(
        PermissionStateCache::SYSTEM_GRANTED_USER_ID, bundleName, permissionName, true);
    RefreshSystemGrantedPersistentDataIfNeeded();
}

void PermissionStateManager::RevokeSystemGrantedPermission(
    const std::string& bundleName, const std::string& permissionName)
{
    if (!DataValidator::IsBundleNameValid(bundleName) || !DataValidator::IsPermissionNameValid(permissionName)) {
        PERMISSION_LOG_ERROR(LABEL, "%{public}s: invalid params!", __func__);
        return;
    }
    PermissionStateCache::GetInstance().UpdatePermissionState(
        PermissionStateCache::SYSTEM_GRANTED_USER_ID, bundleName, permissionName, false);
    RefreshSystemGrantedPersistentDataIfNeeded();
}

int PermissionStateManager::VerifyPermission(
    const std::string& bundleName, const std::string& permissionName, int userId) const
{
    if (!DataValidator::IsBundleNameValid(bundleName) || !DataValidator::IsPermissionNameValid(permissionName) ||
        !DataValidator::IsUserIdValid(userId)) {
        PERMISSION_LOG_ERROR(LABEL, "%{public}s: invalid params!", __func__);
        return PERMISSION_NOT_GRANTED;
    }
    if (!PermissionDefinitionCache::GetInstance().HasDefinition(permissionName)) {
        PERMISSION_LOG_ERROR(
            LABEL, "%{public}s: no definition for permission: %{public}s!", __func__, permissionName.c_str());
        return PERMISSION_NOT_GRANTED;
    }
    bool isGranted = false;
    if (PermissionDefinitionCache::GetInstance().IsSystemGrantedPermission(permissionName)) {
        isGranted = PermissionStateCache::GetInstance().IsGranted(
            PermissionStateCache::SYSTEM_GRANTED_USER_ID, bundleName, permissionName);
    } else {
        isGranted = PermissionStateCache::GetInstance().IsGranted(userId, bundleName, permissionName);
    }
    PERMISSION_LOG_INFO(LABEL, "VerifyPermission %{public}s, %{public}s, %{public}d isGranted: %{public}d",
        bundleName.c_str(), permissionName.c_str(), userId, isGranted);
    return isGranted ? PERMISSION_GRANTED : PERMISSION_NOT_GRANTED;
}

bool PermissionStateManager::CanRequestPermission(
    const std::string& bundleName, const std::string& permissionName, int userId) const
{
    if (!DataValidator::IsBundleNameValid(bundleName) || !DataValidator::IsPermissionNameValid(permissionName) ||
        !DataValidator::IsUserIdValid(userId)) {
        PERMISSION_LOG_ERROR(LABEL, "%{public}s: invalid params!", __func__);
        return false;
    }
    return PermissionDefinitionCache::GetInstance().IsUserGrantedPermission(permissionName) &&
        !PermissionStateCache::GetInstance().IsGranted(userId, bundleName, permissionName);
}

void PermissionStateManager::RefreshSystemGrantedPersistentDataIfNeeded()
{
    if (systemGrantedDataAccessWorker_.GetCurTaskNum() > 1) {
        PERMISSION_LOG_WARN(LABEL, "%{public}s: has refresh task!", __func__);
        return;
    }
    systemGrantedDataAccessWorker_.AddTask([]() {
        std::vector<GenericValues> valueList;
        PermissionStateCache::GetInstance().QueryCurrentCache(true, valueList);
        DataStorage::GetRealDataStorage().RefreshAll(DataStorage::PERMISSIONS_STAT_SYSTEM_GRANTED, valueList);

        // Sleep for one second to avoid frequent refresh of the database.
        std::this_thread::sleep_for(std::chrono::seconds(1));
    });
}

void PermissionStateManager::RefreshUserGrantedPersistentDataIfNeeded()
{
    if (userGrantedDataAccessWorker_.GetCurTaskNum() > 1) {
        PERMISSION_LOG_WARN(LABEL, "%{public}s: has refresh task!", __func__);
        return;
    }
    userGrantedDataAccessWorker_.AddTask([]() {
        std::vector<GenericValues> valueList;
        PermissionStateCache::GetInstance().QueryCurrentCache(false, valueList);
        DataStorage::GetRealDataStorage().RefreshAll(DataStorage::PERMISSIONS_STAT_USER_GRANTED, valueList);

        // Sleep for one second to avoid frequent refresh of the database.
        std::this_thread::sleep_for(std::chrono::seconds(1));
    });
}
} // namespace Permission
} // namespace Security
} // namespace OHOS
