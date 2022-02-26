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

#include "permission_definition_cache.h"

#include "permission.h"
#include "data_translator.h"
#include "permission_log.h"

namespace OHOS {
namespace Security {
namespace Permission {
namespace {
static constexpr OHOS::HiviewDFX::HiLogLabel LABEL = {
    LOG_CORE, SECURITY_DOMAIN_PERMISSION, "PermissionDefinitionCache"
};
}

PermissionDefinitionCache& PermissionDefinitionCache::GetInstance()
{
    static PermissionDefinitionCache instance;
    return instance;
}

PermissionDefinitionCache::PermissionDefinitionCache()
{}

PermissionDefinitionCache::~PermissionDefinitionCache()
{}

bool PermissionDefinitionCache::Insert(const PermissionDef& info)
{
    Utils::UniqueWriteGuard<Utils::RWLock> cacheGuard(this->cacheLock_);
    auto it = permissionDefinitionMap_.find(info.permissionName);
    if (it != permissionDefinitionMap_.end()) {
        PERMISSION_LOG_WARN(LABEL, "%{public}s: info for permission: %{public}s has been insert, please check!",
            __func__, info.permissionName.c_str());
        return false;
    }
    permissionDefinitionMap_[info.permissionName] = info;
    return true;
}

void PermissionDefinitionCache::DeleteByBundleName(const std::string& bundleName)
{
    Utils::UniqueWriteGuard<Utils::RWLock> cacheGuard(this->cacheLock_);
    auto it = permissionDefinitionMap_.begin();
    while (it != permissionDefinitionMap_.end()) {
        if (bundleName == it->second.bundleName) {
            permissionDefinitionMap_.erase(it++);
        } else {
            ++it;
        }
    }
}

int PermissionDefinitionCache::FindByPermissionName(const std::string& permissionName, PermissionDef& info)
{
    Utils::UniqueReadGuard<Utils::RWLock> cacheGuard(this->cacheLock_);
    auto it = permissionDefinitionMap_.find(permissionName);
    if (it == permissionDefinitionMap_.end()) {
        PERMISSION_LOG_DEBUG(LABEL, "%{public}s: can not find definition info for permission: %{public}s", __func__,
            permissionName.c_str());
        return RET_FAILED;
    }
    info = it->second;
    return RET_SUCCESS;
}

bool PermissionDefinitionCache::IsSystemGrantedPermission(const std::string& permissionName)
{
    Utils::UniqueReadGuard<Utils::RWLock> cacheGuard(this->cacheLock_);
    return IsGrantedModeEqualInner(permissionName, SYSTEM_GRANT);
}

bool PermissionDefinitionCache::IsUserGrantedPermission(const std::string& permissionName)
{
    Utils::UniqueReadGuard<Utils::RWLock> cacheGuard(this->cacheLock_);
    return IsGrantedModeEqualInner(permissionName, USER_GRANT);
}

std::string PermissionDefinitionCache::ToString()
{
    Utils::UniqueReadGuard<Utils::RWLock> cacheGuard(this->cacheLock_);
    std::string infos = R"({"Cache": "Default", "DefPermissions": [)";
    for (const auto& request : permissionDefinitionMap_) {
        PermissionDef permissionDef = request.second;
        infos.append(DataTranslator::ToString(request.second));
        infos.append(", ");
    }
    infos = infos + "]}";
    return infos;
}

bool PermissionDefinitionCache::IsGrantedModeEqualInner(const std::string& permissionName, int grantMode) const
{
    auto it = permissionDefinitionMap_.find(permissionName);
    if (it == permissionDefinitionMap_.end()) {
        return false;
    }
    return it->second.grantMode == grantMode;
}

bool PermissionDefinitionCache::HasDefinition(const std::string& permissionName)
{
    Utils::UniqueReadGuard<Utils::RWLock> cacheGuard(this->cacheLock_);
    return permissionDefinitionMap_.count(permissionName) == 1;
}

void PermissionDefinitionCache::QueryCurrentCache(std::vector<GenericValues>& valueList)
{
    Utils::UniqueReadGuard<Utils::RWLock> cacheGuard(this->cacheLock_);
    for (auto it = permissionDefinitionMap_.begin(); it != permissionDefinitionMap_.end(); ++it) {
        GenericValues genericValues;
        DataTranslator::TranslationIntoGenericValues(it->second, genericValues);
        valueList.emplace_back(genericValues);
    }
}
} // namespace Permission
} // namespace Security
} // namespace OHOS
