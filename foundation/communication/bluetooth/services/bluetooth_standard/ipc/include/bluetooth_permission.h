/*
 * Copyright (C) 2021 Huawei Device Co., Ltd.
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

#ifndef OHOS_BLUETOOTH_STANDARD_PERMISSION_H
#define OHOS_BLUETOOTH_STANDARD_PERMISSION_H

#include <string>
#include "permission/permission_kit.h"

namespace OHOS {
namespace Bluetooth {
namespace Permission {
const static std::string BLUETOOTH_ADMIN = "ohos.permission.DISCOVER_BLUETOOTH";
const static std::string ACCESS_FINE_LOCATION = "ohos.permission.LOCATION";
const static std::string BLUETOOTH_PRIVILEGED = "ohos.permission.MANAGE_BLUETOOTH";
const static std::string BLUETOOTH = "ohos.permission.USE_BLUETOOTH";
} // namespace Permission

namespace Token {
const static std::u16string BluetoothToken = u"ohos.bluetooth.IBluetoothHost";
} // namespace Token

class Permission {
public:
    /* check caller's permission by permission name only */
    static bool CheckCallerPermission(const std::string &permissionName);

    /* check caller's permission by provided pid uid */
    static bool CheckCallerPermission(const std::string &permissionName, pid_t pid, uid_t uid);

    /* check caller's permission by finding pid uid by system */
    static bool CheckPermissionByPidUid(const std::string &permissionName);

    /* construct appIdInfo string */
    static std::string FindAppIdInfo();

    /* return value when permission check failed */
    static constexpr int32_t PERMISSION_CHECK_FAIL = 0XEAC;
};
} // namespace Bluetooth
} // namespace OHOS
#endif