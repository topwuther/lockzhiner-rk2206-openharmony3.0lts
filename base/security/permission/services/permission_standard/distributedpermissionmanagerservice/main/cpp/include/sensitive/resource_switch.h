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

#ifndef RESOURCE_SWITCH_H
#define RESOURCE_SWITCH_H

#include <map>
#include "sensitive_resource_switch_setting.h"

namespace OHOS {
namespace Security {
namespace Permission {
class ResourceSwitch {
public:
    static ResourceSwitch &GetInstance();

    void Initialize();

    static void OnLocalChange(std::initializer_list<std::string> sensitiveResources);

    /**
     * Called when device online. This will notify core module to update cache, and remote to sync settings.
     *
     * @param deviceId The remote side device id.
     */
    void OnRemoteOnline(std::string deviceId);

    /**
     * Called when device offline. This will notify core module to clear cache.
     *
     * @param deviceId The remote side device id.
     */
    void OnRemoteOffline(std::string deviceId);

    /**
     * Call when remote device settings changed, start by profile.
     *
     * @param deviceId Device id for the remote device. The local device udid is not valid.
     */
    void OnRemoteChange(std::string deviceId);

    /**
     * Get local sensitive resource switch.
     *
     * @param permission Sensitive resource permission.
     * @return 0 if permission invalid, not sensitive resource or switch off(denied). 1 switch on(allow).
     */
    int GetLocalSensitiveResourceSwitch(std::string permission);

    /**
     * Get remote sensitive resource switch.
     *
     * @param deviceId The remote side device id, Device udid not null.
     * @param permission Sensitive resource permission.
     * @return 0 if permission invalid, not sensitive resource or switch off. 1 switch on.
     */
    int GetRemoteSensitiveResourceSwitch(std::string deviceId, std::string permission);

private:
    ResourceSwitch()
    {}
};
}  // namespace Permission
}  // namespace Security
}  // namespace OHOS

#endif  // RESOURCE_SWITCH_H