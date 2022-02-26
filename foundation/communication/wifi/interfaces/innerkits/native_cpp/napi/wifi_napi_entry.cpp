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

#include "wifi_napi_device.h"
#include "wifi_napi_hotspot.h"
#include "wifi_napi_p2p.h"
#include "wifi_napi_event.h"
#include "wifi_logger.h"

namespace OHOS {
namespace Wifi {
DEFINE_WIFILOG_LABEL("WifiNAPIEntry");

/*
 * Event class initialization function
 */
static void InitEventClass(napi_env& env, napi_value& exports) {
    const char className[] = "EventListener";
    napi_property_descriptor properties[] = {
        DECLARE_NAPI_FUNCTION("on", On),
        DECLARE_NAPI_FUNCTION("off", Off),
    };

    napi_value eventListenerClass = nullptr;
    napi_define_class(env, className, sizeof(className), EventListenerConstructor, nullptr,
        sizeof(properties) / sizeof(napi_property_descriptor), properties, &eventListenerClass);
    napi_status status = napi_set_named_property(env, exports, "EventListener", eventListenerClass);
    if (status != napi_ok) {
        WIFI_LOGE("[Napi Entry] Init event class set property error.");
    }
}

/*
 * Module initialization function
 */
static napi_value Init(napi_env env, napi_value exports) {
    napi_property_descriptor desc[] = {
        DECLARE_NAPI_FUNCTION("enableWifi", EnableWifi),
        DECLARE_NAPI_FUNCTION("disableWifi", DisableWifi),
        DECLARE_NAPI_FUNCTION("isWifiActive", IsWifiActive),
        DECLARE_NAPI_FUNCTION("scan", Scan),
        DECLARE_NAPI_FUNCTION("getScanInfos", GetScanInfos),
        DECLARE_NAPI_FUNCTION("addDeviceConfig", AddDeviceConfig),
        DECLARE_NAPI_FUNCTION("connectToNetwork", ConnectToNetwork),
        DECLARE_NAPI_FUNCTION("connectToDevice", ConnectToDevice),
        DECLARE_NAPI_FUNCTION("disconnect", Disconnect),
        DECLARE_NAPI_FUNCTION("getSignalLevel", GetSignalLevel),
    };

    NAPI_CALL(env, napi_define_properties(env, exports, sizeof(desc) / sizeof(napi_property_descriptor), desc));
    InitEventClass(env, exports);
    return exports;
}

static napi_module wifiJsModule = {
    .nm_version = 1,
    .nm_flags = 0,
    .nm_filename = NULL,
    .nm_register_func = Init,
    .nm_modname = "wifi_native_js",
    .nm_priv = ((void *)0),
    .reserved = { 0 }
};

extern "C" __attribute__((constructor)) void RegisterModule(void) {
    napi_module_register(&wifiJsModule);
}
}  // namespace Wifi
}  // namespace OHOS
