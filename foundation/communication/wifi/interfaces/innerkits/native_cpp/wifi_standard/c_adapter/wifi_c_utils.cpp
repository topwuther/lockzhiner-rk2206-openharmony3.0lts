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

#include "wifi_c_utils.h"
#include <map>
#include <sstream>

namespace OHOS {
namespace Wifi {
static std::map<ErrCode, WifiErrorCode> g_ErrCodeMap = {
    {WIFI_OPT_SUCCESS, WIFI_SUCCESS},
    {WIFI_OPT_FAILED, ERROR_WIFI_UNKNOWN},
    {WIFI_OPT_NOT_SUPPORTED, ERROR_WIFI_NOT_SUPPORTED},
    {WIFI_OPT_INVALID_PARAM, ERROR_WIFI_INVALID_ARGS},
    {WIFI_OPT_FORBID_AIRPLANE, ERROR_WIFI_NOT_AVAILABLE},
    {WIFI_OPT_FORBID_POWSAVING, ERROR_WIFI_NOT_AVAILABLE},
    {WIFI_OPT_PERMISSION_DENIED, ERROR_WIFI_UNKNOWN},
    {WIFI_OPT_OPEN_FAIL_WHEN_CLOSING, ERROR_WIFI_BUSY},
    {WIFI_OPT_OPEN_SUCC_WHEN_OPENED, ERROR_WIFI_BUSY},
    {WIFI_OPT_CLOSE_FAIL_WHEN_OPENING, ERROR_WIFI_BUSY},
    {WIFI_OPT_CLOSE_SUCC_WHEN_CLOSED, ERROR_WIFI_BUSY},
    {WIFI_OPT_STA_NOT_OPENED, ERROR_WIFI_NOT_STARTED},
    {WIFI_OPT_SCAN_NOT_OPENED, ERROR_WIFI_NOT_STARTED},
    {WIFI_OPT_AP_NOT_OPENED, ERROR_WIFI_NOT_STARTED},
    {WIFI_OPT_INVALID_CONFIG, ERROR_WIFI_UNKNOWN}
};

WifiErrorCode GetCErrorCode(ErrCode errCode)
{
    std::map<ErrCode, WifiErrorCode>::const_iterator iter = g_ErrCodeMap.find(errCode);
    return iter == g_ErrCodeMap.end() ? ERROR_WIFI_UNKNOWN : iter->second;
}

static unsigned char ConvertStrChar(char ch)
{
    constexpr int numDiffForHexAlphabet = 10;
    if (ch >= '0' && ch <= '9') {
        return (ch - '0');
    }
    if (ch >= 'A' && ch <= 'F') {
        return (ch - 'A' + numDiffForHexAlphabet);
    }
    if (ch >= 'a' && ch <= 'f') {
        return (ch - 'a' + numDiffForHexAlphabet);
    }
    return 0;
}

errno_t MacStrToArray(const std::string& strMac, unsigned char mac[WIFI_MAC_LEN])
{
    constexpr int strMacLen = 18;
    char tempArray[strMacLen] = { 0 };
    errno_t ret = memcpy_s(tempArray, strMacLen, strMac.c_str(), strMac.size() + 1);
    if (ret != EOK) {
        return ret;
    }

    int idx = 0;
    constexpr int bitWidth = 4;
    char *ptr = nullptr;
    char *p = strtok_s(tempArray, ":", &ptr);
    while (p != nullptr) {
        mac[idx++] = (ConvertStrChar(*p) << bitWidth) | ConvertStrChar(*(p + 1));
        p = strtok_s(nullptr, ":", &ptr);
    }
    return EOK;
}

static char ConvertArrayChar(unsigned char ch)
{
    constexpr int maxDecNum = 9;
    constexpr int numDiffForHexAlphabet = 10;
    if (ch >= 0 && ch <= maxDecNum) {
        return '0' + ch;
    }
    if (ch >= 0xa && ch <= 0xf) {
        return ch + 'a' - numDiffForHexAlphabet;
    }
    return '0';
}

std::string MacArrayToStr(const unsigned char mac[WIFI_MAC_LEN])
{
    constexpr int bitWidth = 4;
    constexpr int noColonBit = 5;
    std::stringstream ss;
    for (int i = 0; i != WIFI_MAC_LEN; ++i) {
        ss << ConvertArrayChar(mac[i] >> bitWidth) << ConvertArrayChar(mac[i] & 0xf);
        if (i != noColonBit) {
            ss << ":";
        }
    }
    return ss.str();
}

bool IsMacArrayEmpty(const unsigned char mac[WIFI_MAC_LEN])
{
    for (int i = 0; i != WIFI_MAC_LEN; ++i) {
        if (mac[i] != 0) {
            return false;
        }
    }
    return true;
}
}  // namespace Wifi
}  // namespace OHOS
