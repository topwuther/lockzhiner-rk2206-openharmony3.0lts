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

#ifndef WIFI_C_UTILS_H_
#define WIFI_C_UTILS_H_

#include <string>
#include "native_c/wifi_device_config.h"
#include "native_c/wifi_error_code.h"
#include "securec.h"
#include "wifi_errcode.h"

namespace OHOS {
namespace Wifi {
#ifndef CHECK_PTR_RETURN
#define CHECK_PTR_RETURN(ptr, retValue)             \
    if ((ptr) == nullptr) {                         \
        WIFI_LOGE("Error: the ptr is null!");       \
        return retValue;                            \
    }
#endif

WifiErrorCode GetCErrorCode(ErrCode errCode);
errno_t MacStrToArray(const std::string& strMac, unsigned char mac[WIFI_MAC_LEN]);
std::string MacArrayToStr(const unsigned char mac[WIFI_MAC_LEN]);
bool IsMacArrayEmpty(const unsigned char mac[WIFI_MAC_LEN]);
}  // namespace Wifi
}  // namespace OHOS

#endif
