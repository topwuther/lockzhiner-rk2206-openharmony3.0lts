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

#ifndef FOUNDATION_EVENT_CESFWK_SERVICES_INCLUDE_COMMON_EVENT_MATCH_PERMISSION_H
#define FOUNDATION_EVENT_CESFWK_SERVICES_INCLUDE_COMMON_EVENT_MATCH_PERMISSION_H

#include <map>
#include <string>
#include <vector>

#include "singleton.h"

namespace OHOS {
namespace EventFwk {
struct Permission {
    enum permissionState {
        DEFAULT,
        AND,
        OR,
    } state;
    std::vector<std::string> names;
    Permission() : state(DEFAULT)
    {}
};
class CommonEventPermissionManager : public DelayedSingleton<CommonEventPermissionManager> {
public:
    CommonEventPermissionManager();

    ~CommonEventPermissionManager() = default;

    void Init();

    Permission GetEventPermission(std::string event);

private:
    std::multimap<std::string, Permission> multimap_;
};
}  // namespace EventFwk
}  // namespace OHOS

#endif  // FOUNDATION_EVENT_CESFWK_SERVICES_INCLUDE_COMMON_EVENT_MATCH_PERMISSION_H