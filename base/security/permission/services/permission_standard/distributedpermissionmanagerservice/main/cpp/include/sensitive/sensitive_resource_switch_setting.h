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

#ifndef SENSITIVE_RESOURCE_SWITCH_SETTING_H
#define SENSITIVE_RESOURCE_SWITCH_SETTING_H

#include <string>
#include <memory>
#include <map>
#include <mutex>

namespace OHOS {
namespace Security {
namespace Permission {
class Builder;
class SensitiveResourceSwitchSetting {
public:
    SensitiveResourceSwitchSetting(Builder &builder);
    SensitiveResourceSwitchSetting()
    {}

    static std::shared_ptr<SensitiveResourceSwitchSetting> Copy(SensitiveResourceSwitchSetting &another);

    static std::shared_ptr<Builder> CreateBuilder();

    bool GetSwitchStatus(std::string sensitiveResource);

    void SetSwitchStatus(std::string sensitiveResource, bool isSwitchAllow);

    std::string ToString();

    std::string GetDeviceId()
    {
        return this->deviceId_;
    }

private:
    std::string deviceId_;
    std::map<std::string, bool> sensitiveResourceSwitchStatus_;
    std::recursive_mutex sensitiveResourceSwitchStatusMutex_;
};

class Builder {
public:
    Builder &DeviceId(std::string deviceId);

    Builder &LocalDevice();

    Builder &Camera(bool isCameraAllow);

    Builder &HealthSensor(bool isHealthSensorAllow);

    Builder &Location(bool isLocationAllow);

    Builder &Microphone(bool isMicrophoneAllow);

    std::shared_ptr<SensitiveResourceSwitchSetting> Build();

public:
    std::string deviceId_;
    bool isCameraAllow_;
    bool isHealthSensorAllow_;
    bool isLocationAllow_;
    bool isMicrophoneAllow_;
};
}  // namespace Permission
}  // namespace Security
}  // namespace OHOS

#endif  // SENSITIVE_RESOURCE_SWITCH_SETTING_H