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

#ifndef RESOURCE_SWITCH_REMOTE_H
#define RESOURCE_SWITCH_REMOTE_H

#include <map>
#include "sensitive_resource_switch_setting.h"

namespace OHOS {
namespace Security {
namespace Permission {
class ResourceSwitchRemote {
public:
    static ResourceSwitchRemote &GetInstance();

    /**
     * Register remote switch change observer.
     */
    void SubscribeRemoteChangeListener();

    /**
     * Query all remote sensitive resource switch from remote side: device profile.
     *
     * @param deviceId The remote side device id.
     * @param isNetworkId Indicate that the device id is deviceNetworkId or UDID.
     * @return Sensitive resource switch status containing switch allowed or denied information.
     */
    std::shared_ptr<SensitiveResourceSwitchSetting> PullSwitchSetting(std::string deviceId, bool isNetworkId);

    /**
     * Asynchronously sync all local sensitive resource switch to remote side: device profile.
     *
     * @param switchSetting Sensitive resource switch status containing allowed or denied information.
     */
    void PushSwitchSetting(std::shared_ptr<SensitiveResourceSwitchSetting> switchSetting);

    /**
     * Just notify device profile to sync all switch settings.
     */
    void SyncSwitchSetting();

private:
    ResourceSwitchRemote() : hasRegistered_(false)
    {}

    std::shared_ptr<SensitiveResourceSwitchSetting> ConvertToSwitchSetting(
        std::string deviceId, std::map<std::string, int32_t> profileMap);

    std::string ConvertToSensitiveResourceIfPossible(std::pair<std::string, int32_t> mapEntry);

    bool IsPermittedFromProfile(std::pair<std::string, int32_t> mapEntry);

private:
    bool hasRegistered_;
};
}  // namespace Permission
}  // namespace Security
}  // namespace OHOS

#endif  // RESOURCE_SWITCH_REMOTE_H