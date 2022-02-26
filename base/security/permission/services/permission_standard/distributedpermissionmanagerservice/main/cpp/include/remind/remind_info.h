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
#ifndef REMIND_INFO_H
#define REMIND_INFO_H

#include <map>
#include <string>
#include <vector>
#include <mutex>
#include "permission_log.h"
#include "constant.h"

struct PermissionRemindInfo {
    int32_t pid;
    int32_t uid;
    std::string deviceId;
    int32_t notificationId;
    std::string bundleName;
    std::string bundleLabel;
    std::string deviceName;
    std::string label;
    std::vector<std::string> permissions;
    std::string appName;
};

namespace OHOS {
namespace Security {
namespace Permission {
class RemindInfo {
public:
    static RemindInfo &GetInstance();

    void DeviceDisConnection(const std::string &deviceId);

    std::vector<PermissionRemindInfo> permRemindInfos_;
};
}  // namespace Permission
}  // namespace Security
}  // namespace OHOS
#endif  // DEVICE_INFO_REPOSITORY_H