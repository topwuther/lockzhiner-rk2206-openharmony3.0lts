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

#include "resource_switch.h"
#include "resource_switch_local.h"
#include "resource_switch_cache.h"
#include "resource_switch_remote.h"
#include "../distributed/distributed_data_validator.h"
#include "../common/constant.h"
#include "ipc_skeleton.h"
#include "distributed_permission_manager_service.h"
#include "permission_log.h"
#include "constant.h"
using namespace OHOS::HiviewDFX;
namespace OHOS {
namespace Security {
namespace Permission {
static const HiLogLabel LABEL = {LOG_CORE, SECURITY_DOMAIN_PERMISSION, "ResourceSwitch"};
ResourceSwitch &ResourceSwitch::GetInstance()
{
    static ResourceSwitch m_instance;
    return m_instance;
}

void ResourceSwitch::Initialize()
{
    PERMISSION_LOG_DEBUG(LABEL, "initialize: called");
    std::shared_ptr<DistributedPermissionEventHandler> handler =
        DelayedSingleton<DistributedPermissionManagerService>::GetInstance()->GetEventHandler();
    if (handler == nullptr) {
        PERMISSION_LOG_WARN(LABEL, "cant GetEventHandler ");
    } else {
        std::string taskName("RequestUpdateCacheAndRemoteSetting");
        auto task = []() {
            ResourceSwitchLocal::GetInstance().RegisterContentObserver(OnLocalChange);
            std::shared_ptr<SensitiveResourceSwitchSetting> localSwitchSetting =
                ResourceSwitchLocal::GetInstance().GetSwitchSetting();
            ResourceSwitchCache::GetInstance().UpdateSwitchSetting(localSwitchSetting);
            ResourceSwitchRemote::GetInstance().PushSwitchSetting(localSwitchSetting);
            ResourceSwitchRemote::GetInstance().SubscribeRemoteChangeListener();
        };
        handler->PostTask(task, taskName);
        PERMISSION_LOG_DEBUG(LABEL, " Add to TaskExecutor success");
    }
}

void ResourceSwitch::OnLocalChange(std::initializer_list<std::string> sensitiveResources)
{
    if (sensitiveResources.size() == 0) {
        PERMISSION_LOG_WARN(LABEL, "onLocalChange: null sensitiveResources, ignore.");
        return;
    }

    std::string resourceInfoString;
    for (auto iter = sensitiveResources.begin(); iter != sensitiveResources.end(); iter++) {
        if (iter == nullptr)
            continue;
        bool isSwitchAllow = ResourceSwitchLocal::GetInstance().GetSwitchStatus(*iter);
        ResourceSwitchCache::GetInstance().SetSwitchStatus(*iter, isSwitchAllow);
        resourceInfoString += *iter;
    }
    PERMISSION_LOG_DEBUG(LABEL, "onLocalChange, sensitive resource %{public}s on change.", resourceInfoString.c_str());

    std::shared_ptr<SensitiveResourceSwitchSetting> localSwitchSetting =
        ResourceSwitchCache::GetInstance().GetSwitchSetting(IPCSkeleton::GetLocalDeviceID());
    ResourceSwitchRemote::GetInstance().PushSwitchSetting(localSwitchSetting);
    ResourceSwitchRemote::GetInstance().SyncSwitchSetting();
    ResourceSwitchRemote::GetInstance().SubscribeRemoteChangeListener();
}

void ResourceSwitch::OnRemoteOnline(std::string deviceId)
{
    if (!DistributedDataValidator::IsDeviceIdValid(deviceId) || deviceId == IPCSkeleton::GetLocalDeviceID()) {
        PERMISSION_LOG_ERROR(LABEL, "onRemoteOnline: invalid deviceId or local deviceId.");
        return;
    }

    PERMISSION_LOG_DEBUG(
        LABEL, "onRemoteOnline() called with: deviceId = [%{public}s]", Constant::EncryptDevId(deviceId).c_str());
    std::shared_ptr<SensitiveResourceSwitchSetting> remoteSwitchSetting =
        ResourceSwitchRemote::GetInstance().PullSwitchSetting(deviceId, false);
    ResourceSwitchCache::GetInstance().UpdateSwitchSetting(remoteSwitchSetting);

    std::shared_ptr<SensitiveResourceSwitchSetting> localSwitchSetting =
        ResourceSwitchCache::GetInstance().GetSwitchSetting(IPCSkeleton::GetLocalDeviceID());
    ResourceSwitchRemote::GetInstance().PushSwitchSetting(localSwitchSetting);

    // In device online scenario, profile will sync data.
    ResourceSwitchRemote::GetInstance().SubscribeRemoteChangeListener();
}

void ResourceSwitch::OnRemoteOffline(std::string deviceId)
{
    if (!DistributedDataValidator::IsDeviceIdValid(deviceId) || deviceId == IPCSkeleton::GetLocalDeviceID()) {
        PERMISSION_LOG_ERROR(LABEL, "onRemoteOffline: invalid deviceId or local deviceId.");
        return;
    }

    PERMISSION_LOG_DEBUG(
        LABEL, "onRemoteOffline() called with: deviceId = [%{public}s]", Constant::EncryptDevId(deviceId).c_str());
    ResourceSwitchCache::GetInstance().RemoveSwitchSetting(deviceId);
}

void ResourceSwitch::OnRemoteChange(std::string deviceId)
{
    if (!DistributedDataValidator::IsDeviceIdValid(deviceId) || deviceId == IPCSkeleton::GetLocalDeviceID()) {
        PERMISSION_LOG_ERROR(LABEL, "onRemoteChange: invalid deviceId or local deviceId.");
        return;
    }

    PERMISSION_LOG_DEBUG(
        LABEL, "onRemoteChange() called with: deviceId = [%{public}s]", Constant::EncryptDevId(deviceId).c_str());
    std::shared_ptr<SensitiveResourceSwitchSetting> remoteSwitchSetting =
        ResourceSwitchRemote::GetInstance().PullSwitchSetting(deviceId, false);
    ResourceSwitchCache::GetInstance().UpdateSwitchSetting(remoteSwitchSetting);
}

int ResourceSwitch::GetLocalSensitiveResourceSwitch(std::string permission)
{
    if (!DistributedDataValidator::IsPermissionNameValid(permission)) {
        PERMISSION_LOG_ERROR(
            LABEL, "Get local switch with invalid permission name %{public}s denied.", permission.c_str());
        return Constant::RESOURCE_SWITCH_STATUS_DENIED;
    }

    bool isSwitchAllow = ResourceSwitchCache::GetInstance().GetSwitchStatus(permission);
    if (isSwitchAllow) {
        return Constant::RESOURCE_SWITCH_STATUS_ALLOWED;
    } else {
        return Constant::RESOURCE_SWITCH_STATUS_DENIED;
    }
}

int ResourceSwitch::GetRemoteSensitiveResourceSwitch(std::string deviceId, std::string permission)
{
    if (!DistributedDataValidator::IsDeviceIdValid(deviceId) ||
        !DistributedDataValidator::IsPermissionNameValid(permission)) {
        PERMISSION_LOG_ERROR(LABEL,
            "Get remote switch with invalid deviceId[%{public}s] or permission[%{public}s]  denied.",
            Constant::EncryptDevId(deviceId).c_str(),
            permission.c_str());
        return Constant::RESOURCE_SWITCH_STATUS_DENIED;
    }
    bool isSwitchAllow = ResourceSwitchCache::GetInstance().GetSwitchStatus(deviceId, permission);
    if (isSwitchAllow) {
        return Constant::RESOURCE_SWITCH_STATUS_ALLOWED;
    } else {
        return Constant::RESOURCE_SWITCH_STATUS_DENIED;
    }
}
}  // namespace Permission
}  // namespace Security
}  // namespace OHOS