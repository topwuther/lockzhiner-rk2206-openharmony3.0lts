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

#ifndef RESOURCE_SWITCH_CATCH_H
#define RESOURCE_SWITCH_CATCH_H

#include <map>
#include "sensitive_resource_switch_setting.h"

namespace OHOS {
namespace Security {
namespace Permission {
class ResourceSwitchCache {
public:
    static ResourceSwitchCache &GetInstance();

    bool GetSwitchStatus(std::string sensitiveResource);

    bool GetSwitchStatus(std::string deviceId, std::string sensitiveResource);

    void SetSwitchStatus(std::string sensitiveResource, bool isSwitchAllow);

    void SetSwitchStatus(std::string deviceId, std::string sensitiveResource, bool isSwitchAllow);

    std::shared_ptr<SensitiveResourceSwitchSetting> GetSwitchSetting(std::string deviceId);

    void UpdateSwitchSetting(std::shared_ptr<SensitiveResourceSwitchSetting> SwitchSetting);

    void RemoveSwitchSetting(std::string deviceId);

private:
    ResourceSwitchCache()
    {}

private:
    /**
     * Key is device id, value is current switch status map.
     */
    std::map<std::string, std::shared_ptr<SensitiveResourceSwitchSetting>> cache_;
    std::recursive_mutex sensitiveResourceSwitchSettingMutex_;
};
}  // namespace Permission
}  // namespace Security
}  // namespace OHOS

#endif  // RESOURCE_SWITCH_CATCH_H