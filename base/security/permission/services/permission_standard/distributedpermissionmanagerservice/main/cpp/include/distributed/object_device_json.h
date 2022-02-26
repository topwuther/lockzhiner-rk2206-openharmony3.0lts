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

#ifndef PERMISSION_OBJECT_DEVICE_JSON_H
#define PERMISSION_OBJECT_DEVICE_JSON_H
#include <string>
namespace OHOS {
namespace Security {
namespace Permission {
struct ObjectDeviceJson {
    std::string deviceId_;
    std::string uid_;
    std::string grantedPermission_;
    std::string &ToJsonString(std::string &jsonString) const;
    bool FromJsonString(const std::string &jsonString);
};
}  // namespace Permission
}  // namespace Security
}  // namespace OHOS
#endif  // FOUNDATION_REQ_PERMISSION_DETAIL_H
