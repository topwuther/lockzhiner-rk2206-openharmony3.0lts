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

#include "resource_switch_local.h"
#include "ipc_skeleton.h"
#include "../common/constant.h"
#include "permission_log.h"
using namespace OHOS::HiviewDFX;

namespace OHOS {
namespace Security {
namespace Permission {
static constexpr HiLogLabel LABEL = {LOG_CORE, SECURITY_DOMAIN_PERMISSION, "ResourceSwitchLocal"};
ResourceSwitchLocal &ResourceSwitchLocal::GetInstance()
{
    static ResourceSwitchLocal m_instance;
    return m_instance;
}

bool ResourceSwitchLocal::HasDeviceProvisioned()
{
    return true;
}

void ResourceSwitchLocal::RegisterContentObserver(OnChangeCallback onChangeCallback)
{
    if (onChangeCallback == nullptr) {
        PERMISSION_LOG_ERROR(LABEL, "Register content observer with null onChangeCallback, please check.");
        return;
    }

    if (hasRegistered_) {
        PERMISSION_LOG_WARN(LABEL, "Register content observer no need to register secure content observer twice!");
        return;
    }

    if (!HasDeviceProvisioned()) {
        PERMISSION_LOG_INFO(
            LABEL, "registerContentObserver: device has not provisioned, register global content observer");
    }

    hasRegistered_ = true;
    PERMISSION_LOG_INFO(LABEL, "Register content observer done.");
}

std::shared_ptr<SensitiveResourceSwitchSetting> ResourceSwitchLocal::GetSwitchSetting()
{
    if (!hasRegistered_) {
        PERMISSION_LOG_ERROR(LABEL, "Get local resource switch entities called before init, please check");
        SensitiveResourceSwitchSetting::CreateBuilder()->Build();
    }

    return SensitiveResourceSwitchSetting::CreateBuilder()
        ->LocalDevice()
        .Camera(GetSwitchStatus(Constant::CAMERA))
        .HealthSensor(GetSwitchStatus(Constant::READ_HEALTH_DATA))
        .Location(GetSwitchStatus(Constant::LOCATION))
        .Microphone(GetSwitchStatus(Constant::MICROPHONE))
        .Build();
}

bool ResourceSwitchLocal::GetSwitchStatus(std::string sensitiveResource)
{
    if (!hasRegistered_) {
        PERMISSION_LOG_ERROR(LABEL, "Get local resource switch entity called before init, please check");
        return false;
    }

    // Default switch is true.
    bool isSwitchAllow = true;

    int switchFromSettings = Constant::RESOURCE_SWITCH_STATUS_ALLOWED;
    isSwitchAllow = (switchFromSettings == Constant::RESOURCE_SWITCH_STATUS_ALLOWED);

    PERMISSION_LOG_INFO(LABEL,
        "Switch %{public}s from settings %{public}s",
        sensitiveResource.c_str(),
        (isSwitchAllow ? "allowed." : "denied."));
    return isSwitchAllow;
}
}  // namespace Permission
}  // namespace Security
}  // namespace OHOS