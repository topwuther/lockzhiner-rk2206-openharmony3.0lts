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
#include "constant.h"

namespace OHOS {
namespace Security {
namespace Permission {

const std::string Constant::SYSTEM_PERMISSION_TYPE = "system";
const std::string Constant::APP_PERMISSION_TYPE = "app";

const std::string Constant::LOCATION = "ohos.permission.LOCATION";
const std::string Constant::LOCATION_IN_BACKGROUND = "ohos.permission.LOCATION_IN_BACKGROUND";
const std::string Constant::CAMERA = "ohos.permission.CAMERA";
const std::string Constant::MICROPHONE = "ohos.permission.MICROPHONE";
const std::string Constant::READ_CALENDAR = "ohos.permission.READ_CALENDAR";
const std::string Constant::WRITE_CALENDAR = "ohos.permission.WRITE_CALENDAR";
const std::string Constant::ACTIVITY_MOTION = "ohos.permission.ACTIVITY_MOTION";
const std::string Constant::READ_HEALTH_DATA = "ohos.permission.READ_HEALTH_DATA";
const std::string Constant::DISTRIBUTED_DATASYNC = "ohos.permission.DISTRIBUTED_DATASYNC";
const std::string Constant::DISTRIBUTED_DATA = "ohos.permission.DISTRIBUTED_DATA";
const std::string Constant::MEDIA_LOCATION = "ohos.permission.MEDIA_LOCATION";
const std::string Constant::READ_MEDIA = "ohos.permission.READ_MEDIA";
const std::string Constant::WRITE_MEDIA = "ohos.permission.WRITE_MEDIA";

const std::string Constant::COMMAND_RESULT_SUCCESS = "success";
const std::string Constant::COMMAND_RESULT_FAILED = "execute command failed";
const std::string Constant::COMMAND_PERMISSIONS_COUNT_FAILED = "permissions exceed MAX_UID_PERMISSIONS_COUNT";
const std::string Constant::COMMAND_GET_PERMISSIONS_FAILED = "failed to get permissions";
const std::string Constant::COMMAND_GET_REGRANTED_PERMISSIONS_FAILED = "failed to get regranted permissions";

const std::map<std::string, int32_t> Constant::NAME_TO_OP_CODE_MAP = {
    std::map<std::string, int32_t>::value_type("ohos.permission.LOCATION_IN_BACKGROUND", -1),
    std::map<std::string, int32_t>::value_type("ohos.permission.LOCATION", 1),
    std::map<std::string, int32_t>::value_type("ohos.permission.READ_CONTACTS", 4),
    std::map<std::string, int32_t>::value_type("ohos.permission.WRITE_CONTACTS", 5),
    std::map<std::string, int32_t>::value_type("ohos.permission.READ_CALL_LOG", 6),
    std::map<std::string, int32_t>::value_type("ohos.permission.WRITE_CALL_LOG", 7),
    std::map<std::string, int32_t>::value_type("ohos.permission.READ_CALENDAR", 8),
    std::map<std::string, int32_t>::value_type("ohos.permission.WRITE_CALENDAR", 9),
    std::map<std::string, int32_t>::value_type("ohos.permission.PLACE_CALL", 13),
    std::map<std::string, int32_t>::value_type("ohos.permission.READ_MESSAGES", 14),
    std::map<std::string, int32_t>::value_type("ohos.permission.RECEIVE_SMS", 16),
    std::map<std::string, int32_t>::value_type("ohos.permission.RECEIVE_MMS", 18),
    std::map<std::string, int32_t>::value_type("ohos.permission.RECEIVE_WAP_MESSAGES", 19),
    std::map<std::string, int32_t>::value_type("ohos.permission.SEND_MESSAGES", 20),
    std::map<std::string, int32_t>::value_type("ohos.permission.CAMERA", 26),
    std::map<std::string, int32_t>::value_type("ohos.permission.MICROPHONE", 27),
    std::map<std::string, int32_t>::value_type("ohos.permission.GET_TELEPHONY_STATE", 51),
    std::map<std::string, int32_t>::value_type("ohos.permission.MANAGE_VOICEMAIL", 52),
    std::map<std::string, int32_t>::value_type("ohos.permission.READ_HEALTH_DATA", 56),
    std::map<std::string, int32_t>::value_type("ohos.permission.READ_USER_STORAGE", 59),
    std::map<std::string, int32_t>::value_type("ohos.permission.WRITE_USER_STORAGE", 60),
    std::map<std::string, int32_t>::value_type("ohos.permission.ANSWER_CALL", 69),
    std::map<std::string, int32_t>::value_type("ohos.permission.ACTIVITY_MOTION", 79),
    std::map<std::string, int32_t>::value_type("ohos.permission.DISTRIBUTED_VIRTUALDEVICE", 200),
    // temp add
    std::map<std::string, int32_t>::value_type("ohos.permission.MEDIA_LOCATION", 33212),
    std::map<std::string, int32_t>::value_type("ohos.permission.READ_MEDIA", 33333),
    std::map<std::string, int32_t>::value_type("ohos.permission.WRITE_MEDIA", 44444),
    std::map<std::string, int32_t>::value_type("ohos.permission.DISTRIBUTED_DATA", 55555),
    std::map<std::string, int32_t>::value_type("ohos.permission.DISTRIBUTED_DATASYNC", 66666),
};

bool Constant::PermissionNameToOrFromOpCode(std::string &permissionName, int32_t &opCode)
{
    if (permissionName.empty()) {
        auto ite = std::find_if(NAME_TO_OP_CODE_MAP.begin(),
            NAME_TO_OP_CODE_MAP.end(),
            [opCode](const std::map<std::string, int32_t>::value_type &pair) { return opCode == pair.second; });
        if (ite != NAME_TO_OP_CODE_MAP.end()) {
            permissionName = ite->first;
            return true;
        }
        return false;
    } else {
        if (NAME_TO_OP_CODE_MAP.count(permissionName) > 0) {
            opCode = NAME_TO_OP_CODE_MAP.at(permissionName);
            return true;
        }
        return false;
    }
}

const std::vector<std::string> Constant::USE_BY_LOCAL_APP_LIST = {
    "ohos.permission.LOCATION_IN_BACKGROUND",
    "ohos.permission.LOCATION",
};

bool Constant::UseByLocalApp(std::string &permissionName)
{
    auto it = std::find_if(USE_BY_LOCAL_APP_LIST.begin(),
        USE_BY_LOCAL_APP_LIST.end(),
        [&permissionName](const auto &useByLocalApp) { return permissionName == useByLocalApp; });
    if (it != USE_BY_LOCAL_APP_LIST.end()) {
        return true;
    }
    return false;
}

std::string Constant::EncryptDevId(std::string deviceId)
{
    std::string result = deviceId;
    if (deviceId.size() >= ENCRYPTLEN) {
        result.replace(ENCRYPTBEGIN, ENCRYPTEND, "****");
    } else {
        result.replace(ENCRYPTBEGIN, result.size() - 1, "****");
    }
    return result;
}

std::string Constant::GetLocalDeviceId()
{
    return "1004";
}

const std::string Constant::PROFILE_KEY_CAMERA = "cameraSwitch";
const std::string Constant::PROFILE_KEY_MICROPHONE = "microphoneSwitch";
const std::string Constant::PROFILE_KEY_LOCATION = "locationSwitch";
const std::string Constant::PROFILE_KEY_HEALTH_SENSOR = "healthSensorSwitch";

const int32_t Constant::RESOURCE_SWITCH_STATUS_DENIED = 0;
const int32_t Constant::RESOURCE_SWITCH_STATUS_ALLOWED = 1;

const std::vector<std::string> Constant::PERMISSION_RECORDS_GETTING_LIST = {"ohos.permission.READ_CONTACTS",
    "ohos.permission.GET_TELEPHONY_STATE",
    "ohos.permission.LOCATION",
    "ohos.permission.MICROPHONE",
    "ohos.permission.CAMERA",
    "ohos.permission.WRITE_CALENDAR",
    "ohos.permission.READ_CALENDAR",
    "ohos.permission.MEDIA_LOCATION",
    "ohos.permission.READ_MEDIA",
    "ohos.permission.WRITE_MEDIA",
    "ohos.permission.ACTIVITY_MOTION",
    "ohos.permission.READ_HEALTH_DATA",
    "ohos.permission.DISTRIBUTED_DATA"};

}  // namespace Permission
}  // namespace Security
}  // namespace OHOS