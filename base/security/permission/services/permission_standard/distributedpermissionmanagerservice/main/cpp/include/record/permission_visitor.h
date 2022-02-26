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

#ifndef PERMISSION_VISITOR_H
#define PERMISSION_VISITOR_H

#include <string>

namespace OHOS {
namespace Security {
namespace Permission {
struct PermissionVisitor {
    int32_t id = -1;
    std::string deviceId;
    std::string deviceName;
    int32_t bundleUserId = 0;
    std::string bundleName;
    std::string bundleLabel;

    PermissionVisitor() = default;

    static bool SetPermissionVisitor(const std::string &deviceId, const std::string &deviceName,
        const int32_t bundleUserId, const std::string &bundleName, const std::string &bundleLabel,
        PermissionVisitor &permissionVisitor);
};
}  // namespace Permission
}  // namespace Security
}  // namespace OHOS

#endif  // PERMISSION_VISITOR_H