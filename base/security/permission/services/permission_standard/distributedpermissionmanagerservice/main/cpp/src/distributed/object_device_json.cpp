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

#include "nlohmann/json.hpp"

#include "app_log_wrapper.h"
#include "object_device_json.h"

namespace OHOS {
namespace Security {
namespace Permission {
namespace {
const std::string DEVICE_INFO_DEVICE_ID = "deviceId_";
const std::string DEVICE_INFO_UID = "uid_";
const std::string DEVICE_INFO_GRABTED_PERMISSION = "grantedPermission_";
}  // namespace

std::string &ObjectDeviceJson::ToJsonString(std::string &jsonString) const
{
    nlohmann::json jsonObject;
    jsonObject[DEVICE_INFO_DEVICE_ID] = deviceId_;
    jsonObject[DEVICE_INFO_UID] = uid_;
    jsonObject[DEVICE_INFO_GRABTED_PERMISSION] = grantedPermission_;
    jsonString = jsonObject.dump();
    return jsonString;
}
bool ObjectDeviceJson::FromJsonString(const std::string &jsonString)
{
    nlohmann::json jsonObject = nlohmann::json::parse(jsonString);
    if (jsonObject.is_discarded()) {
        return false;
    }
    if (jsonObject.find(DEVICE_INFO_DEVICE_ID) != jsonObject.end()) {
        deviceId_ = jsonObject.at(DEVICE_INFO_DEVICE_ID).get<std::string>();
    }
    if (jsonObject.find(DEVICE_INFO_UID) != jsonObject.end()) {
        uid_ = jsonObject.at(DEVICE_INFO_UID).get<std::string>();
    }
    if (jsonObject.find(DEVICE_INFO_GRABTED_PERMISSION) != jsonObject.end()) {
        grantedPermission_ = jsonObject.at(DEVICE_INFO_GRABTED_PERMISSION).get<std::string>();
    }
    return true;
}
}  // namespace Permission
}  // namespace Security
}  // namespace OHOS