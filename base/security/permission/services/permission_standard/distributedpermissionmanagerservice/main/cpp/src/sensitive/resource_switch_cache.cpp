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

#include "resource_switch_cache.h"
#include "ipc_skeleton.h"
#include "permission_log.h"
#include "constant.h"
using namespace OHOS::HiviewDFX;

namespace OHOS {
namespace Security {
namespace Permission {
static constexpr HiLogLabel LABEL = {LOG_CORE, SECURITY_DOMAIN_PERMISSION, "ResourceSwitchCache"};
ResourceSwitchCache &ResourceSwitchCache::GetInstance()
{
    static ResourceSwitchCache m_instance;
    return m_instance;
}

bool ResourceSwitchCache::GetSwitchStatus(std::string sensitiveResource)
{
    return GetSwitchStatus(IPCSkeleton::GetLocalDeviceID(), sensitiveResource);
}

bool ResourceSwitchCache::GetSwitchStatus(std::string deviceId, std::string sensitiveResource)
{
    bool isSwitchAllow = false;
    auto iter = cache_.find(deviceId);
    if (iter != cache_.end()) {
        if (iter->second != nullptr) {
            isSwitchAllow = iter->second->GetSwitchStatus(sensitiveResource);
        } else {
            PERMISSION_LOG_ERROR(
                LABEL, "setting of deviceid[%{public}s] in cache is null", Constant::EncryptDevId(deviceId).c_str());
        }
    }

    PERMISSION_LOG_DEBUG(LABEL,
        "Get switch %{public}s for %{public}s result %{public}s",
        sensitiveResource.c_str(),
        Constant::EncryptDevId(deviceId).c_str(),
        (isSwitchAllow ? "allowed." : "denied."));

    return isSwitchAllow;
}

void ResourceSwitchCache::SetSwitchStatus(std::string sensitiveResource, bool isSwitchAllow)
{
    SetSwitchStatus(IPCSkeleton::GetLocalDeviceID(), sensitiveResource, isSwitchAllow);
}

void ResourceSwitchCache::SetSwitchStatus(std::string deviceId, std::string sensitiveResource, bool isSwitchAllow)
{
    auto iter = cache_.find(deviceId);
    if (iter != cache_.end()) {
        if (iter->second != nullptr) {
            PERMISSION_LOG_ERROR(LABEL,
                "SetSwitchStatus deviceId[%{public}s] resource[%{public}s], status[%{public}d]",
                Constant::EncryptDevId(deviceId).c_str(),
                sensitiveResource.c_str(),
                isSwitchAllow);
            iter->second->SetSwitchStatus(sensitiveResource, isSwitchAllow);
        }
    } else {
        PERMISSION_LOG_ERROR(LABEL,
            "SetSwitchStatus deviceId[%{public}s] does not exist in cache",
            Constant::EncryptDevId(deviceId).c_str());
    }
}

std::shared_ptr<SensitiveResourceSwitchSetting> ResourceSwitchCache::GetSwitchSetting(std::string deviceId)
{
    auto iter = cache_.find(deviceId);
    if (iter != cache_.end()) {
        return iter->second;
    } else {
        PERMISSION_LOG_WARN(
            LABEL, "deviceid[%{public}s] does not exist in cache", Constant::EncryptDevId(deviceId).c_str());
        return nullptr;
    }
}

void ResourceSwitchCache::UpdateSwitchSetting(std::shared_ptr<SensitiveResourceSwitchSetting> SwitchSetting)
{
    std::lock_guard<std::recursive_mutex> guard(sensitiveResourceSwitchSettingMutex_);
    if (SwitchSetting == nullptr) {
        PERMISSION_LOG_ERROR(LABEL, "UpdateSwitchSetting SwitchSetting is null");
        return;
    }
    auto iter = cache_.find(SwitchSetting->GetDeviceId());
    if (iter != cache_.end()) {
        PERMISSION_LOG_DEBUG(
            LABEL, "find device id [%{public}s]", Constant::EncryptDevId(SwitchSetting->GetDeviceId()).c_str());
        cache_[SwitchSetting->GetDeviceId()].reset();
        cache_[SwitchSetting->GetDeviceId()] = SwitchSetting;
    } else {
        PERMISSION_LOG_DEBUG(
            LABEL, "didn't find device id [%{public}s]", Constant::EncryptDevId(SwitchSetting->GetDeviceId()).c_str());
        cache_.insert(std::pair<std::string, std::shared_ptr<SensitiveResourceSwitchSetting>>(
            SwitchSetting->GetDeviceId(), SwitchSetting));
    }
    PERMISSION_LOG_DEBUG(LABEL,
        "device id [%{public}s], setting[%{public}s]",
        Constant::EncryptDevId(SwitchSetting->GetDeviceId()).c_str(),
        cache_[SwitchSetting->GetDeviceId()]->ToString().c_str());
}

void ResourceSwitchCache::RemoveSwitchSetting(std::string deviceId)
{
    std::lock_guard<std::recursive_mutex> guard(sensitiveResourceSwitchSettingMutex_);
    auto iter = cache_.find(deviceId);
    if (iter != cache_.end()) {
        PERMISSION_LOG_DEBUG(LABEL,
            "device id [%{public}s], setting[%{public}s]",
            Constant::EncryptDevId(deviceId).c_str(),
            cache_[deviceId]->ToString().c_str());
        iter->second.reset();
        cache_.erase(deviceId);
    }
}
}  // namespace Permission
}  // namespace Security
}  // namespace OHOS