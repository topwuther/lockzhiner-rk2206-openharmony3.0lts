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

#ifndef OHOS_DEVICE_MANAGER_CONSTANTS_H
#define OHOS_DEVICE_MANAGER_CONSTANTS_H

#include <cstdint>
#include <string>

namespace OHOS {
namespace DistributedHardware {
    const std::string TARGET_PKG_NAME_KEY = "targetPkgName";
    const std::string HOST_PKG_NAME_KEY = "hostPackageName";

    const int32_t LOG_MAX_LEN = 512;
    const int32_t MIN_PIN_TOKEN = 10000000;
    const int32_t MAX_PIN_TOKEN = 90000000;
    const int32_t MIN_PIN_CODE = 100000;
    const int32_t MAX_PIN_CODE = 999999;
    const int32_t TOKEN_LEN = 9;

    const int32_t FAIL = -1;
    const int32_t SUCCESS = 0;
    const int32_t ERROR_INPUT_PARA_EMPTY = 2000;
    const int32_t ERROR_INPUT_PARA_INVALID = 2001;
    const int32_t ERROR_TARGET_PKG_NAME_NULL = 2002;
    const int32_t ERROR_PKG_NAME_NOT_ON_DISCOVERY = 2003;
    const int32_t ERROR_START_REMOTE_DM = 2006;
    const int32_t ERROR_OPEN_CHANNEL_FAIL = 2008;
    const int32_t ERROR_SEND_COMMAND_FAIL = 2009;
    const int32_t ERROR_CHANNEL_BROKEN = 2010;
    const int32_t ERROR_DUPLICATE_REQUEST = 2011;
    const int32_t ERROR_TIME_OUT = 2012;
    const int32_t ERROR_REQUEST_CANCEL = 2013;
    const int32_t ERROR_USER_REJECT = 2015;
    const int32_t ERROR_USER_BUSY = 2016;
    const int32_t ERROR_API_NOT_SUPPORT = 2018;
    const int32_t ERROR_SESSION_NOT_EXIT = 2019;
    const int32_t ERROR_CREAT_GROUP_FAIL = 2020;
    const int32_t ERROR_JOIN_GROUP_FAIL = 2021;
    const int32_t ERROR_GET_LOCAL_DEVICE_INFO_FAIL = 2022;
    const int32_t ERROR_CHECK_AUTH_FAIL = 2023;
    const int32_t ERROR_NETWORK_UNAVAILABLE = 2028;
    const int32_t ERROR_DEVICE_INFO_NULL = 2030;
    const int32_t ERROR_APP_NAME_NULL = 2031;
    const int32_t ERROR_APP_DESCRIPTION_INVALID = 2032;
    const int32_t ERROR_APP_ICON_INVALID = 2033;
    const int32_t ERROR_APP_THUMBNAIL_INVALID = 2034;
    const int32_t ERROR_FA_START_FAIL = 2100;

    const int32_t ENCRYPT_TAG_LEN = 32;

    const std::string TAG_REQUESTER = "REQUESTER";
    const std::string TAG_TOKEN = "TOKEN";
    const std::string TAG_HOST = "HOST";
    const std::string TAG_TARGET = "TARGET";
    const std::string TAG_VISIBILITY = "VISIBILITY";
    const std::string TAG_GROUPIDS = "GROUPIDLIST";
    const std::string TAG_REPLY = "REPLY";
    const std::string TAG_NET_ID = "NETID";
    const std::string TAG_GROUP_ID = "GROUPID";
    const std::string TAG_GROUP_NAME = "GROUPNAME";
    const std::string TAG_REQUEST_ID = "REQUESTID";
    const std::string TAG_DEVICE_ID = "DEVICEID";
    const std::string TAG_DEVICE_TYPE = "DEVICETYPE";
    const std::string TAG_APP_NAME = "APPNAME";
    const std::string TAG_APP_DESCRIPTION = "APPDESC";
    const std::string TAG_APP_ICON = "APPICON";
    const std::string TAG_APP_THUMBNAIL = "APPTHUM";
    const std::string TAG_INDEX = "INDEX";
    const std::string TAG_SLICE_NUM = "SLICE";
    const std::string TAG_THUMBNAIL_SIZE = "THUMSIZE";
    const std::string TAG_AUTH_TYPE = "AUTHTYPE";

    const std::string TAG_VER = "ITF_VER";
    const std::string TAG_TYPE = "MSG_TYPE";
    const std::string DM_ITF_VER = "1.0";
    const std::string TAG = "DM_MSG_CODEC";

    // GroupConstants
    const int32_t GROUP_VISIBILITY_IS_PUBLIC = -1;
    const int32_t GROUP_VISIBILITY_IS_PRIVATE = 0;
    const std::string REQUEST_AUTH_ADD_ID = "REQUEST_AUTH_ADD_ID";
    const std::string INVITATE_AUTH_ADD_ID = "INVITE_AUTH_ADD_ID";
    const int32_t PIN_CODE_INVALID = -1;

    // AuthConstants
    const int32_t AUTH_SESSION_SIDE_SERVER = 0;
    const int32_t AUTH_SESSION_SIDE_CLIENT = 1;
    const int32_t SESSION_TYPE_IS_DEVICE_AUTH = 0;
    const int32_t SESSION_TYPE_IS_APP_AUTH = 1;
    const int32_t SESSION_REPLY_UNKNOWN = -1;
    const int32_t SESSION_REPLY_ACCEPT = 0;
    const int32_t SESSION_REPLY_CANCEL = 1;
    const int32_t SESSION_REPLY_TIMEOUT = 2;
    const int32_t SESSION_REPLY_AUTH_CONFIRM_TIMEOUT = 2;
    const int32_t SESSION_REPLY_CANCEL_PINCODE_DISPLAY = 3;
    const int32_t SESSION_REPLY_CANCEL_PINCODE_INPUT = 4;
    const int32_t SESSION_REPLY_CREAT_GROUP_FAILED = 7;
    const int32_t REQUEST_OPERATION = 0;
    const int32_t QR_OPERATION = 1;
    const int32_t PIN_OPERATION = 2;
    const int32_t START_ACTIVITY_DIALOG = 1;
    const int32_t START_ACTIVITY_QR = 2;
    const int32_t START_ACTIVITY_FINISH = 3;
    const int32_t START_ACTIVITY_PIN = 4;
    const int32_t AUTH_STATUS_SKIP = 3;
    const int32_t DEFAULT_PIN_CODE = 0;
    const int32_t DEFAULT_PIN_TOKEN = 0;
    const std::string DEVICE_NAME_KEY = "deviceName";
    const std::string DISPLAY_TIME_KEY = "displayTime";
    const std::string QR_BITMAP_KEY = "qrBitMap";
    const std::string PIN_CODE_KEY = "pinCode";
    const std::string START_ACTIVITY_KEY = "startActivityType";
    const std::string AUTH_TYPE = "authType";
    const std::string TOKEN = "token";
    const std::string PIN_TOKEN = "pinToken";
    const int32_t MSG_TYPE_TO_REMOTE = 0;
    const int32_t MSG_TYPE_TO_CLOSED_CHANNEL = 1;
    const std::string APP_NAME_KEY = "appName";
    const std::string APP_DESCRIPTION_KEY = "appDescription";
    const std::string APP_ICON_KEY = "appIcon";
    const std::string APP_THUMBNAIL_KEY = "appThumbnail";
    const std::string CANCEL_DISPLAY_KEY = "cancelPinCodeDisplay";
    const int32_t SLICE_NUM_INVALID = -1;
    const int32_t AUTH_TYPE_QR = 0;
    const int32_t AUTH_TYPE_PIN = 1;
    const std::string DISPLAY_OWNER = "displayOwner";
    const int32_t DISPLAY_OWNER_SYSTEM = 0;
    const int32_t DISPLAY_OWNER_OTHER = 1;

    // DmService constants
    const int32_t MSG_MAX_SIZE = 45 * 1024;
    const int32_t DES_SLICE_MAX_NUM = 5;
    const int32_t ABILITY_ON = 1;
    const int32_t ABILITY_INVALID = -1;
    const int32_t DES_MAX_LEN = 256;
    const int32_t ICON_MAX_LEN = 32 * 1024;
    const int32_t THUMB_MAX_LEN = 153 * 1024;

    const int32_t DEVICE_UUID_LENGTH = 65;
    const int32_t GROUP_TYPE_IDENTICAL_ACCOUNT_GROUP = 1;
    const int32_t GROUP_TYPE_PEER_TO_PEER_GROUP = 256;

    const int32_t GROUP_VISIBILITY_PUBLIC = -1;

    const int32_t BUSINESS_FA_MIRGRATION = 0;
    const int32_t BUSINESS_RESOURCE_ACCESS = 1;

    // Base64 Constants
    const int32_t BASE64_BYTE_LEN_3 = 3;
    const int32_t BASE64_BYTE_LEN_4 = 4;
}
}
#endif
