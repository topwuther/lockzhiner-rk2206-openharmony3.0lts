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

#include "resource_switch_remote.h"
#include "ipc_skeleton.h"
#include "../common/constant.h"
#include "permission_log.h"
using namespace OHOS::HiviewDFX;

namespace OHOS {
namespace Security {
namespace Permission {
static const HiLogLabel LABEL = {LOG_CORE, SECURITY_DOMAIN_PERMISSION, "ResourceSwitchRemote"};
ResourceSwitchRemote &ResourceSwitchRemote::GetInstance()
{
    static ResourceSwitchRemote m_instance;
    return m_instance;
}

void ResourceSwitchRemote::SubscribeRemoteChangeListener()
{
    if (hasRegistered_) {
        PERMISSION_LOG_INFO(LABEL, "has been registed.");
        return;
    }

    PERMISSION_LOG_INFO(LABEL, "Subscribe remote change listener done.");
}

std::shared_ptr<SensitiveResourceSwitchSetting> ResourceSwitchRemote::PullSwitchSetting(
    std::string deviceId, bool isNetworkId)
{
    PERMISSION_LOG_INFO(LABEL,
        "Pull sensitive resource switches start with deviceId = [%{public}s] isNetworkId = [%{public}s]",
        Constant::EncryptDevId(deviceId).c_str(),
        (isNetworkId ? "true" : "false"));

    std::map<std::string, int32_t> profileMap;
    std::shared_ptr<SensitiveResourceSwitchSetting> remoteSwitchSetting = ConvertToSwitchSetting(deviceId, profileMap);
    PERMISSION_LOG_INFO(
        LABEL, "Pull sensitive resource switches setting %{public}s", remoteSwitchSetting->ToString().c_str());
    return remoteSwitchSetting;
}

void ResourceSwitchRemote::PushSwitchSetting(std::shared_ptr<SensitiveResourceSwitchSetting> switchSetting)
{
    PERMISSION_LOG_DEBUG(LABEL,
        "Push sensitive resource switches called with switchSetting = [%{public}s]",
        switchSetting->ToString().c_str());
    PERMISSION_LOG_DEBUG(LABEL, "Push sensitive resource switches asyncExecute done.");
}

void ResourceSwitchRemote::SyncSwitchSetting()
{
    PERMISSION_LOG_DEBUG(LABEL, "syncSwitchSetting() called");

    PERMISSION_LOG_DEBUG(LABEL, "syncSwitchSetting() done");
}

std::shared_ptr<SensitiveResourceSwitchSetting> ResourceSwitchRemote::ConvertToSwitchSetting(
    std::string deviceId, std::map<std::string, int32_t> profileMap)
{
    std::shared_ptr<SensitiveResourceSwitchSetting> remoteSwitchSetting =
        SensitiveResourceSwitchSetting::CreateBuilder()->DeviceId(deviceId).Build();

    if (profileMap.size() == 0) {
        PERMISSION_LOG_WARN(LABEL, "ConvertToSwitchSetting profileMap.size is 0");
        return remoteSwitchSetting;
    }

    std::string resourceValue;
    bool ispermitted = false;
    for (auto &iter : profileMap) {
        resourceValue = ConvertToSensitiveResourceIfPossible(iter);
        ispermitted = IsPermittedFromProfile(iter);
        remoteSwitchSetting->SetSwitchStatus(resourceValue, ispermitted);
    }

    return remoteSwitchSetting;
}

std::string ResourceSwitchRemote::ConvertToSensitiveResourceIfPossible(std::pair<std::string, int32_t> mapEntry)
{
    if (mapEntry.first == Constant::PROFILE_KEY_CAMERA) {
        return Constant::CAMERA;
    } else if (mapEntry.first == Constant::PROFILE_KEY_HEALTH_SENSOR) {
        return Constant::READ_HEALTH_DATA;
    } else if (mapEntry.first == Constant::PROFILE_KEY_LOCATION) {
        return Constant::LOCATION;
    } else if (mapEntry.first == Constant::PROFILE_KEY_MICROPHONE) {
        return Constant::MICROPHONE;
    } else {
        return "";
    }
}

bool ResourceSwitchRemote::IsPermittedFromProfile(std::pair<std::string, int32_t> mapEntry)
{
    if (mapEntry.second == Constant::PERMISSION_GRANTED_STATUS) {
        return true;
    } else {
        return false;
    }
}
}  // namespace Permission
}  // namespace Security
}  // namespace OHOS