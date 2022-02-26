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
#include "sensitive_resource_switch_setting.h"
#include "../common/constant.h"
#include "ipc_skeleton.h"

namespace OHOS {
namespace Security {
namespace Permission {
SensitiveResourceSwitchSetting::SensitiveResourceSwitchSetting(Builder &builder)
{
    this->deviceId_ = builder.deviceId_;
    sensitiveResourceSwitchStatus_.insert(std::pair<std::string, bool>(Constant::CAMERA, builder.isCameraAllow_));
    sensitiveResourceSwitchStatus_.insert(
        std::pair<std::string, bool>(Constant::READ_HEALTH_DATA, builder.isHealthSensorAllow_));
    sensitiveResourceSwitchStatus_.insert(std::pair<std::string, bool>(Constant::LOCATION, builder.isLocationAllow_));
    sensitiveResourceSwitchStatus_.insert(
        std::pair<std::string, bool>(Constant::MICROPHONE, builder.isMicrophoneAllow_));
}

std::shared_ptr<SensitiveResourceSwitchSetting> SensitiveResourceSwitchSetting::Copy(
    SensitiveResourceSwitchSetting &another)
{
    std::shared_ptr<Builder> builder = SensitiveResourceSwitchSetting::CreateBuilder();
    if (builder) {
        return builder->DeviceId(another.deviceId_)
            .Camera(another.GetSwitchStatus(Constant::CAMERA))
            .HealthSensor(another.GetSwitchStatus(Constant::READ_HEALTH_DATA))
            .Location(another.GetSwitchStatus(Constant::LOCATION))
            .Microphone(another.GetSwitchStatus(Constant::MICROPHONE))
            .Build();
    } else {
        return nullptr;
    }
}

std::shared_ptr<Builder> SensitiveResourceSwitchSetting::CreateBuilder()
{
    return std::make_shared<Builder>();
}

bool SensitiveResourceSwitchSetting::GetSwitchStatus(std::string sensitiveResource)
{
    std::lock_guard<std::recursive_mutex> guard(sensitiveResourceSwitchStatusMutex_);
    auto iter = sensitiveResourceSwitchStatus_.find(sensitiveResource);
    if (iter != sensitiveResourceSwitchStatus_.end()) {
        return iter->second;
    } else {
        return false;
    }
}

void SensitiveResourceSwitchSetting::SetSwitchStatus(std::string sensitiveResource, bool isSwitchAllow)
{
    std::lock_guard<std::recursive_mutex> guard(sensitiveResourceSwitchStatusMutex_);
    sensitiveResourceSwitchStatus_[sensitiveResource] = isSwitchAllow;
}

std::string SensitiveResourceSwitchSetting::ToString()
{
    std::string logInfo = "SensitiveResourceSwitchSetting{deviceId=" + this->deviceId_ + ",";
    std::map<std::string, bool>::iterator iter;
    for (iter = sensitiveResourceSwitchStatus_.begin(); iter != sensitiveResourceSwitchStatus_.end(); iter++) {
        logInfo += "sensitiveResourceSwitchStatus_[" + iter->first;
        logInfo += "] = " + std::to_string(static_cast<int32_t>(iter->second)) + ",";
    }
    logInfo += "}";
    return logInfo;
}

Builder &Builder::DeviceId(std::string deviceId)
{
    this->deviceId_ = deviceId;
    return *this;
}

Builder &Builder::LocalDevice()
{
    this->deviceId_ = IPCSkeleton::GetLocalDeviceID();
    return *this;
}
Builder &Builder::Camera(bool isCameraAllow)
{
    this->isCameraAllow_ = isCameraAllow;
    return *this;
}

Builder &Builder::HealthSensor(bool isHealthSensorAllow)
{
    this->isHealthSensorAllow_ = isHealthSensorAllow;
    return *this;
}

Builder &Builder::Location(bool isLocationAllow)
{
    this->isLocationAllow_ = isLocationAllow;
    return *this;
}

Builder &Builder::Microphone(bool isMicrophoneAllow)
{
    this->isMicrophoneAllow_ = isMicrophoneAllow;
    return *this;
}

std::shared_ptr<SensitiveResourceSwitchSetting> Builder::Build()
{
    return std::make_shared<SensitiveResourceSwitchSetting>(*this);
}
}  // namespace Permission
}  // namespace Security
}  // namespace OHOS