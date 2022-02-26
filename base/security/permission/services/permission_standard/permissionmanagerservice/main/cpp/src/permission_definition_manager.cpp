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

#include "permission_definition_manager.h"

#include "permission_definition_cache.h"
#include "data_storage.h"
#include "data_translator.h"
#include "permission_log.h"
#include "permission.h"
#include "data_validator.h"

namespace OHOS {
namespace Security {
namespace Permission {
namespace {
static constexpr OHOS::HiviewDFX::HiLogLabel LABEL = {
    LOG_CORE, SECURITY_DOMAIN_PERMISSION, "PermissionDefinitionManager"
};
}

PermissionDefinitionManager& PermissionDefinitionManager::GetInstance()
{
    static PermissionDefinitionManager instance;
    return instance;
}

PermissionDefinitionManager::PermissionDefinitionManager() : hasInited_(false)
{}

PermissionDefinitionManager::~PermissionDefinitionManager()
{
    if (!hasInited_) {
        return;
    }
    this->permissionDefDataAccessWorker_.Stop();
    this->hasInited_ = false;
}

void PermissionDefinitionManager::Init()
{
    OHOS::Utils::UniqueWriteGuard<OHOS::Utils::RWLock> lk(this->rwLock_);
    if (hasInited_) {
        return;
    }
    PERMISSION_LOG_INFO(LABEL, "init begin!");
    std::vector<GenericValues> results;
    DataStorage::GetRealDataStorage().Find(DataStorage::PERMISSION_DEF, results);
    for (auto value : results) {
        PermissionDef permissionDef;
        DataTranslator::TranslationIntoPermissionDef(value, permissionDef);
        PermissionDefinitionCache::GetInstance().Insert(permissionDef);
    }
    this->permissionDefDataAccessWorker_.Start(1);
    hasInited_ = true;
    PERMISSION_LOG_INFO(LABEL, "Init success");
}

void PermissionDefinitionManager::AddDefPermissions(const std::vector<PermissionDefParcel>& permList)
{
    if (permList.empty()) {
        PERMISSION_LOG_ERROR(LABEL, "%{public}s: invalid params!", __func__);
        return;
    }
    for (const auto& permissionDefParcel : permList) {
        if (!IsPermissionDefValid(permissionDefParcel.permissionDef)) {
            PERMISSION_LOG_ERROR(LABEL, "%{public}s: invalid permission definition info: %{public}s", __func__,
                DataTranslator::ToString(permissionDefParcel.permissionDef).c_str());
            continue;
        }
        PermissionDefinitionCache::GetInstance().Insert(permissionDefParcel.permissionDef);
    }
    RefreshPersistentDataIfNeeded();
}

void PermissionDefinitionManager::RemoveDefPermissions(const std::string& bundleName)
{
    if (!DataValidator::IsBundleNameValid(bundleName)) {
        PERMISSION_LOG_ERROR(LABEL, "%{public}s: invalid params!", __func__);
        return;
    }
    PermissionDefinitionCache::GetInstance().DeleteByBundleName(bundleName);
    RefreshPersistentDataIfNeeded();
}

int PermissionDefinitionManager::GetDefPermission(
    const std::string& permissionName, PermissionDefParcel& permissionDefParcel) const
{
    if (!DataValidator::IsPermissionNameValid(permissionName)) {
        PERMISSION_LOG_ERROR(LABEL, "%{public}s: invalid params!", __func__);
        return RET_FAILED;
    }
    return PermissionDefinitionCache::GetInstance().FindByPermissionName(
        permissionName, permissionDefParcel.permissionDef);
}

bool PermissionDefinitionManager::IsGrantModeValid(const int grantMode) const
{
    return grantMode == GrantMode::SYSTEM_GRANT || grantMode == GrantMode::USER_GRANT;
}

bool PermissionDefinitionManager::IsAvailableScopeValid(const int availableScope) const
{
    return availableScope == AvailableScope::AVAILABLE_SCOPE_ALL ||
        availableScope == AvailableScope::AVAILABLE_SCOPE_RESTRICTED ||
        availableScope == AvailableScope::AVAILABLE_SCOPE_SIGNATURE;
}

bool PermissionDefinitionManager::IsPermissionDefValid(const PermissionDef& permissionDef) const
{
    if (!DataValidator::IsPermissionNameValid(permissionDef.permissionName)) {
        return false;
    }
    if (!DataValidator::IsBundleNameValid(permissionDef.bundleName)) {
        return false;
    }
    if (!IsGrantModeValid(permissionDef.grantMode)) {
        return false;
    }
    return IsAvailableScopeValid(permissionDef.availableScope);
}

void PermissionDefinitionManager::RefreshPersistentDataIfNeeded()
{
    if (permissionDefDataAccessWorker_.GetCurTaskNum() > 1) {
        PERMISSION_LOG_WARN(LABEL, "%{public}s: has refresh task!", __func__);
        return;
    }
    permissionDefDataAccessWorker_.AddTask([]() {
        std::vector<GenericValues> valueList;
        PermissionDefinitionCache::GetInstance().QueryCurrentCache(valueList);
        DataStorage::GetRealDataStorage().RefreshAll(DataStorage::PERMISSION_DEF, valueList);

        // Sleep for one second to avoid frequent refresh of the database.
        std::this_thread::sleep_for(std::chrono::seconds(1));
    });
}
} // namespace Permission
} // namespace Security
} // namespace OHOS
