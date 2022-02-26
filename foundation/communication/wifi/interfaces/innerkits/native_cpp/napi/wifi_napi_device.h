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

#ifndef WIFI_NAPI_DEVICE_H_
#define WIFI_NAPI_DEVICE_H_

#include "wifi_napi_utils.h"

namespace OHOS {
namespace Wifi {
napi_value EnableWifi(napi_env env, napi_callback_info info);
napi_value DisableWifi(napi_env env, napi_callback_info info);
napi_value IsWifiActive(napi_env env, napi_callback_info info);
napi_value Scan(napi_env env, napi_callback_info info);
napi_value GetScanInfos(napi_env env, napi_callback_info info);
napi_value AddDeviceConfig(napi_env env, napi_callback_info info);
napi_value ConnectToNetwork(napi_env env, napi_callback_info info);
napi_value ConnectToDevice(napi_env env, napi_callback_info info);
napi_value Disconnect(napi_env env, napi_callback_info info);
napi_value GetSignalLevel(napi_env env, napi_callback_info info);

enum class SecTypeJs {
    SEC_TYPE_INVALID = 0, /* Invalid security type */
    SEC_TYPE_OPEN = 1, /* Open */
    SEC_TYPE_WEP = 2, /* Wired Equivalent Privacy (WEP) */
    SEC_TYPE_PSK = 3, /* Pre-shared key (PSK) */
    SEC_TYPE_SAE = 4, /* Simultaneous Authentication of Equals (SAE) */
};
}  // namespace Wifi
}  // namespace OHOS

#endif
