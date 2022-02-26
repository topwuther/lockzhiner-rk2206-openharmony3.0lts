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

#ifndef RESOURCE_SWITCH_LOCAL_H
#define RESOURCE_SWITCH_LOCAL_H

#include <map>
#include "sensitive_resource_switch_setting.h"

namespace OHOS {
namespace Security {
namespace Permission {
typedef void (*OnChangeCallback)(std::initializer_list<std::string> sensitiveResources);
class ResourceSwitchLocal {
public:
    static ResourceSwitchLocal &GetInstance();

    /**
     * Register self secure content observer.
     *
     * @param onChangeCallback The callback listener.
     */
    void RegisterContentObserver(OnChangeCallback onChangeCallback);

    /**
     * Get all local sensitive resource switch settings from settings.
     *
     * @return Sensitive resource switch status containing allowed or denied information.
     */
    std::shared_ptr<SensitiveResourceSwitchSetting> GetSwitchSetting();

    /**
     * Get single local sensitive resource switch status from settings.
     *
     * @param sensitiveResource The query sensitive resource.
     * @return Allowed or denied information.
     */
    bool GetSwitchStatus(std::string sensitiveResource);

private:
    ResourceSwitchLocal() : hasRegistered_(false)
    {}

    static bool HasDeviceProvisioned();

private:
    bool hasRegistered_;
};
}  // namespace Permission
}  // namespace Security
}  // namespace OHOS

#endif  // RESOURCE_SWITCH_LOCAL_H