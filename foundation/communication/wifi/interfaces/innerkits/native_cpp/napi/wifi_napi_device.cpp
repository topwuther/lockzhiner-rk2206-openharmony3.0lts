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
#include "wifi_logger.h"
#include "wifi_device.h"
#include "wifi_scan.h"
#include <vector>

namespace OHOS {
namespace Wifi {
DEFINE_WIFILOG_LABEL("WifiNAPIDevice");

std::unique_ptr<WifiDevice> wifiDevicePtr = WifiDevice::GetInstance(WIFI_DEVICE_ABILITY_ID);
std::unique_ptr<WifiScan> wifiScanPtr = WifiScan::GetInstance(WIFI_SCAN_ABILITY_ID);

napi_value EnableWifi(napi_env env, napi_callback_info info)
{
    size_t argc = 1;
    napi_value argv[1];
    napi_value thisVar;
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, &thisVar, NULL));

    NAPI_ASSERT(env, wifiDevicePtr != nullptr, "[NAPI] Wifi device instance is null.");
    ErrCode ret = wifiDevicePtr->EnableWifi();
    napi_value result;
    napi_get_boolean(env, ret == WIFI_OPT_SUCCESS, &result);
    return result;
}

napi_value DisableWifi(napi_env env, napi_callback_info info)
{
    size_t argc = 1;
    napi_value argv[1];
    napi_value thisVar;
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, &thisVar, NULL));

    NAPI_ASSERT(env, wifiDevicePtr != nullptr, "[NAPI] Wifi device instance is null.");
    ErrCode ret = wifiDevicePtr->DisableWifi();
    napi_value result;
    napi_get_boolean(env, ret == WIFI_OPT_SUCCESS, &result);
    return result;
}

napi_value IsWifiActive(napi_env env, napi_callback_info info)
{
    size_t argc = 1;
    napi_value argv[1];
    napi_value thisVar;
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, &thisVar, NULL));

    NAPI_ASSERT(env, wifiDevicePtr != nullptr, "[NAPI] Wifi device instance is null.");
    bool activeStatus = true;
    ErrCode ret = wifiDevicePtr->IsWifiActive(activeStatus);
    if (ret != WIFI_OPT_SUCCESS) {
        WIFI_LOGE("[Napi Device] Get wifi active status fail: %{public}d", ret);
    }

    napi_value result;
    napi_get_boolean(env, activeStatus, &result);
    return result;
}

napi_value Scan(napi_env env, napi_callback_info info)
{
    size_t argc = 1;
    napi_value argv[1];
    napi_value thisVar;
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, &thisVar, NULL));

    NAPI_ASSERT(env, wifiScanPtr != nullptr, "[NAPI] Wifi scan instance is null.");
    ErrCode ret = wifiScanPtr->Scan();

    napi_value result;
    napi_get_boolean(env, ret == WIFI_OPT_SUCCESS, &result);
    return result;
}

static SecTypeJs SecurityTypeNativeToJs(const WifiSecurity& cppSecurityType)
{
    SecTypeJs jsSecurityType = SecTypeJs::SEC_TYPE_INVALID;
    switch (cppSecurityType) {
        case WifiSecurity::OPEN:
            jsSecurityType = SecTypeJs::SEC_TYPE_OPEN;
            break;

        case WifiSecurity::WEP:
            jsSecurityType = SecTypeJs::SEC_TYPE_WEP;
            break;

        case WifiSecurity::PSK:
            jsSecurityType = SecTypeJs::SEC_TYPE_PSK;
            break;

        case WifiSecurity::SAE:
            jsSecurityType = SecTypeJs::SEC_TYPE_SAE;
            break;

        default:
            jsSecurityType = SecTypeJs::SEC_TYPE_INVALID;
            break;
    }
    return jsSecurityType;
}

static bool NativeScanInfosToJsObj(const napi_env& env, napi_value& arrayResult,
    const std::vector<WifiScanInfo>& vecScnIanfos)
{
    uint32_t idx = 0;
    for (auto& each : vecScnIanfos) {
        napi_value eachObj;
        napi_create_object(env, &eachObj);

        SetValueUtf8String(env, "ssid", each.ssid.c_str(), eachObj);
        SetValueUtf8String(env, "bssid", each.bssid.c_str(), eachObj);
        SetValueInt32(env, "securityType", static_cast<int>(SecurityTypeNativeToJs(each.securityType)), eachObj);
        SetValueInt32(env, "rssi", each.rssi, eachObj);
        SetValueInt32(env, "band", each.band, eachObj);
        SetValueInt32(env, "frequency", each.frequency, eachObj);
        SetValueInt64(env, "timestamp", each.timestamp, eachObj);

        napi_status status = napi_set_element(env, arrayResult, idx++, eachObj);
        if (status != napi_ok) {
            WIFI_LOGE("[Napi Device] wifi napi set element error: %{public}d, idx: %{public}d", status, idx - 1);
            return false;
        }
    }
    return true;
}

static bool GetWifiScanInfoList(const napi_env& env, napi_value& arrayResult)
{
    std::vector<WifiScanInfo> vecCppScanInfos;
    if (wifiScanPtr->GetScanInfoList(vecCppScanInfos) != WIFI_OPT_SUCCESS) {
        WIFI_LOGE("[Napi Device] Get Scaninf list error");
        return false;
    }

    WIFI_LOGI("[Napi Device] GetScanInfoList, size: %{public}zu", vecCppScanInfos.size());
    napi_create_array_with_length(env, vecCppScanInfos.size(), &arrayResult);
    return NativeScanInfosToJsObj(env, arrayResult, vecCppScanInfos);
}

static napi_value ScanInfoToCallBack(const napi_env& env, AsyncCallbackInfo *asCallbackInfo,
    const size_t argc, const napi_value *argv)
{
    napi_value resourceName;
    napi_create_string_latin1(env, "getScanInfos", NAPI_AUTO_LENGTH, &resourceName);

    for (size_t i = 0; i != argc; ++i) {
        napi_valuetype valuetype;
        NAPI_CALL(env, napi_typeof(env, argv[i], &valuetype));
        NAPI_ASSERT(env, valuetype == napi_function, "Wrong argument type. Function expected.");
        napi_create_reference(env, argv[i], 1, &asCallbackInfo->callback[i]);
    }

    napi_create_async_work(
        env, nullptr, resourceName,
        [](napi_env env, void* data) {
        },
        [](napi_env env, napi_status status, void* data) {
            napi_value undefine;
            napi_get_undefined(env, &undefine);
            napi_value callback;
            AsyncCallbackInfo* asCallbackInfo = (AsyncCallbackInfo *)data;
            asCallbackInfo->isSuccess = GetWifiScanInfoList(env, asCallbackInfo->result);
            if (asCallbackInfo->isSuccess) {
                napi_get_reference_value(env, asCallbackInfo->callback[0], &callback);
                napi_call_function(env, nullptr, callback, 1, &asCallbackInfo->result, &undefine);
            } else {
                if (asCallbackInfo->callback[1]) {
                    napi_get_reference_value(env, asCallbackInfo->callback[1], &callback);
                    napi_call_function(env, nullptr, callback, 1, &asCallbackInfo->result, &undefine);
                } else {
                    WIFI_LOGE("[Napi Device] get scan info callback func is null");
                    napi_throw_error(env, "error", "get scan info callback func is null");
                }
            }
            if (asCallbackInfo->callback[0] != nullptr) {
                napi_delete_reference(env, asCallbackInfo->callback[0]);
            }
            if (asCallbackInfo->callback[1] != nullptr) {
                napi_delete_reference(env, asCallbackInfo->callback[1]);
            }
            napi_delete_async_work(env, asCallbackInfo->asyncWork);
            delete asCallbackInfo;
        },
        (void *)asCallbackInfo,
        &asCallbackInfo->asyncWork);
    NAPI_CALL(env, napi_queue_async_work(env, asCallbackInfo->asyncWork));
    return UndefinedNapiValue(env);
}

static napi_value ScanInfoToPromise(const napi_env& env, AsyncCallbackInfo *asCallbackInfo, napi_value& promise)
{
    napi_value resourceName;
    napi_create_string_latin1(env, "getScanInfos", NAPI_AUTO_LENGTH, &resourceName);

    napi_deferred deferred;
    NAPI_CALL(env, napi_create_promise(env, &deferred, &promise));
    asCallbackInfo->deferred = deferred;

    napi_create_async_work(
        env,
        nullptr,
        resourceName,
        [](napi_env env, void *data) {
        },
        [](napi_env env, napi_status status, void *data) {
            AsyncCallbackInfo *asCallbackInfo = (AsyncCallbackInfo *)data;
            asCallbackInfo->isSuccess = GetWifiScanInfoList(env, asCallbackInfo->result);
            if (asCallbackInfo->isSuccess) {
                napi_resolve_deferred(asCallbackInfo->env, asCallbackInfo->deferred, asCallbackInfo->result);
            } else {
                napi_reject_deferred(asCallbackInfo->env, asCallbackInfo->deferred, asCallbackInfo->result);
            }
            napi_delete_async_work(env, asCallbackInfo->asyncWork);
            delete asCallbackInfo;
        },
        (void *)asCallbackInfo,
        &asCallbackInfo->asyncWork);
    napi_queue_async_work(env, asCallbackInfo->asyncWork);
    return UndefinedNapiValue(env);
}

napi_value GetScanInfos(napi_env env, napi_callback_info info)
{
    size_t argc = 2;
    napi_value argv[argc];
    napi_value thisVar = nullptr;
    void *data = nullptr;
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, &thisVar, &data));

    AsyncCallbackInfo *asCallbackInfo =
        new AsyncCallbackInfo{.env = env, .asyncWork = nullptr, .deferred = nullptr};

    if (argc >= 1) {
        return ScanInfoToCallBack(env, asCallbackInfo, argc, argv);
    } else {
        napi_value promise;
        ScanInfoToPromise(env, asCallbackInfo, promise);
        return promise;
    }
}

static void ConvertEncryptionMode(const SecTypeJs& securityType, std::string& keyMgmt)
{
    switch (securityType) {
        case SecTypeJs::SEC_TYPE_OPEN:
            keyMgmt = "NONE";
            break;

        case SecTypeJs::SEC_TYPE_WEP:
            keyMgmt = "WEP";
            break;

        case SecTypeJs::SEC_TYPE_PSK:
            keyMgmt = "WPA-PSK";
            break;

        case SecTypeJs::SEC_TYPE_SAE:
            keyMgmt = "SAE";
            break;

        default:
            keyMgmt = "WPA-PSK";
            break;
    }
}

static void JsObjToDeviceConfig(const napi_env& env, const napi_value& object, WifiDeviceConfig& cppConfig)
{
    JsObjectToString(env, object, "ssid", 33, cppConfig.ssid); /* 33: ssid max length is 32 + '\0' */
    JsObjectToString(env, object, "bssid", 18, cppConfig.bssid); /* 18: max bssid length for string type */
    JsObjectToString(env, object, "preSharedKey", 256, cppConfig.preSharedKey); /* 256: max length */
    JsObjectToBool(env, object, "isHiddenSsid", cppConfig.hiddenSSID);
    int type = static_cast<int>(SecTypeJs::SEC_TYPE_INVALID);
    JsObjectToInt(env, object, "securityType", type);
    ConvertEncryptionMode(SecTypeJs(type), cppConfig.keyMgmt);
}

static napi_value AddDeviceConfigImpl(const napi_env& env, AsyncCallbackInfo *asCallbackInfo)
{
    NAPI_ASSERT(env, wifiDevicePtr != nullptr, "[NAPI] Wifi device instance is null.");
    int addResult = -1;
    int retValue = 0;
    ErrCode ret = wifiDevicePtr->AddDeviceConfig(*(WifiDeviceConfig *)asCallbackInfo->obj, addResult);
    if (addResult < 0 || ret != WIFI_OPT_SUCCESS) {
        retValue = -1;
    } else {
        retValue = addResult;
    }

    napi_value result;
    napi_create_int32(env, retValue, &result);
    asCallbackInfo->isSuccess = (ret == WIFI_OPT_SUCCESS);
    asCallbackInfo->result = result;
    return UndefinedNapiValue(env);
}

static napi_value AddDeviceConfigCallBack(const napi_env& env, AsyncCallbackInfo *asCallbackInfo,
    size_t argc, napi_value *argv)
{
    napi_value resourceName;
    napi_create_string_latin1(env, "addDeviceConfig", NAPI_AUTO_LENGTH, &resourceName);

    for (size_t i = 1; i != argc; ++i) {
        napi_valuetype valuetype;
        NAPI_CALL(env, napi_typeof(env, argv[i], &valuetype));
        NAPI_ASSERT(env, valuetype == napi_function, "Wrong argument type. Function expected.");
        napi_create_reference(env, argv[i], 1, &asCallbackInfo->callback[i - 1]);
    }

    napi_create_async_work(
        env, nullptr, resourceName,
        [](napi_env env, void* data) {
        },
        [](napi_env env, napi_status status, void* data) {
            AsyncCallbackInfo* asCallbackInfo = (AsyncCallbackInfo *)data;
            AddDeviceConfigImpl(env, asCallbackInfo);
            napi_value callback;
            napi_value undefine;
            napi_get_undefined(env, &undefine);
            if (asCallbackInfo->isSuccess) {
                napi_get_reference_value(env, asCallbackInfo->callback[0], &callback);
                napi_call_function(env, nullptr, callback, 1, &asCallbackInfo->result, &undefine);
            } else {
                if (asCallbackInfo->callback[1]) {
                    napi_get_reference_value(env, asCallbackInfo->callback[1], &callback);
                    napi_call_function(env, nullptr, callback, 1, &asCallbackInfo->result, &undefine);
                } else {
                    WIFI_LOGE("[Napi Device] get scan info callback func is null");
                    napi_throw_error(env, "error", "add wifi config callback func is null");
                }
            }
            if (asCallbackInfo->callback[0] != nullptr) {
                napi_delete_reference(env, asCallbackInfo->callback[0]);
            }
            if (asCallbackInfo->callback[1] != nullptr) {
                napi_delete_reference(env, asCallbackInfo->callback[1]);
            }
            napi_delete_async_work(env, asCallbackInfo->asyncWork);
            delete (WifiDeviceConfig *)asCallbackInfo->obj;
            delete asCallbackInfo;
        },
        (void *)asCallbackInfo,
        &asCallbackInfo->asyncWork);
    NAPI_CALL(env, napi_queue_async_work(env, asCallbackInfo->asyncWork));
    return UndefinedNapiValue(env);
}

static napi_value AddDeviceConfigPromise(const napi_env& env, AsyncCallbackInfo *asCallbackInfo, napi_value& promise)
{
    napi_value resourceName;
    napi_create_string_latin1(env, "addDeviceConfig", NAPI_AUTO_LENGTH, &resourceName);

    napi_deferred deferred;
    NAPI_CALL(env, napi_create_promise(env, &deferred, &promise));
    asCallbackInfo->deferred = deferred;

    napi_create_async_work(
        env,
        nullptr,
        resourceName,
        [](napi_env env, void *data) {
        },
        [](napi_env env, napi_status status, void *data) {
            AsyncCallbackInfo *asCallbackInfo = (AsyncCallbackInfo *)data;
            AddDeviceConfigImpl(env, asCallbackInfo);
            if (asCallbackInfo->isSuccess) {
                napi_resolve_deferred(asCallbackInfo->env, asCallbackInfo->deferred, asCallbackInfo->result);
            } else {
                napi_reject_deferred(asCallbackInfo->env, asCallbackInfo->deferred, asCallbackInfo->result);
            }
            napi_delete_async_work(env, asCallbackInfo->asyncWork);
            delete (WifiDeviceConfig *)asCallbackInfo->obj;
            delete asCallbackInfo;
        },
        (void *)asCallbackInfo,
        &asCallbackInfo->asyncWork);
    napi_queue_async_work(env, asCallbackInfo->asyncWork);
    return UndefinedNapiValue(env);
}

napi_value AddDeviceConfig(napi_env env, napi_callback_info info)
{
    size_t argc = 3;
    napi_value argv[argc];
    napi_value thisVar = nullptr;
    void *data = nullptr;
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, &thisVar, &data));
    NAPI_ASSERT(env, argc >= 1, "Wrong number of arguments");

    napi_valuetype valueType;
    napi_typeof(env, argv[0], &valueType);
    NAPI_ASSERT(env, valueType == napi_object, "Wrong argument type. Object expected for parameter 1.");

    AsyncCallbackInfo *asCallbackInfo =
        new AsyncCallbackInfo{.env = env, .asyncWork = nullptr, .deferred = nullptr};

    WifiDeviceConfig *config = new WifiDeviceConfig();
    if (config == NULL) {
        delete asCallbackInfo;
        return UndefinedNapiValue(env);
    }
    JsObjToDeviceConfig(env, argv[0], *config);
    asCallbackInfo->obj = config;
    if (argc > 1) {
        return AddDeviceConfigCallBack(env, asCallbackInfo, argc, argv);
    } else {
        napi_value promise;
        AddDeviceConfigPromise(env, asCallbackInfo, promise);
        return promise;
    }
}

napi_value ConnectToNetwork(napi_env env, napi_callback_info info)
{
    size_t argc = 1;
    napi_value argv[1];
    napi_value thisVar;
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, &thisVar, NULL));
    NAPI_ASSERT(env, argc == 1, "Wrong number of arguments");

    napi_valuetype valueType;
    napi_typeof(env, argv[0], &valueType);
    NAPI_ASSERT(env, valueType == napi_number, "Wrong argument type. napi_number expected.");

    int networkId = -1;
    napi_get_value_int32(env, argv[0], &networkId);

    NAPI_ASSERT(env, wifiDevicePtr != nullptr, "[NAPI] Wifi device instance is null.");
    ErrCode ret = wifiDevicePtr->ConnectToNetwork(networkId);
    napi_value result;
    napi_get_boolean(env, ret == WIFI_OPT_SUCCESS, &result);
    return result;
}

napi_value ConnectToDevice(napi_env env, napi_callback_info info)
{
    size_t argc = 1;
    napi_value argv[1];
    napi_value thisVar;
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, &thisVar, NULL));

    napi_valuetype valueType;
    napi_typeof(env, argv[0], &valueType);
    NAPI_ASSERT(env, valueType == napi_object, "Wrong argument type. Object expected.");

    NAPI_ASSERT(env, wifiDevicePtr != nullptr, "[NAPI] Wifi device instance is null.");
    WifiDeviceConfig config;
    JsObjToDeviceConfig(env, argv[0], config);
    ErrCode ret = wifiDevicePtr->ConnectToDevice(config);

    napi_value result;
    napi_get_boolean(env, ret == WIFI_OPT_SUCCESS, &result);
    return result;
}

napi_value Disconnect(napi_env env, napi_callback_info info)
{
    size_t argc = 1;
    napi_value argv[1];
    napi_value thisVar;
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, &thisVar, NULL));

    NAPI_ASSERT(env, wifiDevicePtr != nullptr, "[NAPI] Wifi device instance is null.");
    ErrCode ret = wifiDevicePtr->Disconnect();
    napi_value result;
    napi_get_boolean(env, ret == WIFI_OPT_SUCCESS, &result);
    return result;
}

napi_value GetSignalLevel(napi_env env, napi_callback_info info)
{
    size_t argc = 2;
    napi_value argv[2];
    napi_value thisVar;
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, &thisVar, NULL));
    /* the input have 2 parameters */
    NAPI_ASSERT(env, argc == 2, "Wrong number of arguments");

    napi_valuetype type1;
    napi_valuetype type2;
    napi_typeof(env, argv[0], &type1);
    napi_typeof(env, argv[1], &type2);
    NAPI_ASSERT(env, type1 == napi_number, "Wrong argument type. napi_number expected.");
    NAPI_ASSERT(env, type2 == napi_number, "Wrong argument type. napi_number expected.");

    int rssi, band;
    napi_get_value_int32(env, argv[0], &rssi);
    napi_get_value_int32(env, argv[1], &band);

    NAPI_ASSERT(env, wifiDevicePtr != nullptr, "[NAPI] Wifi device instance is null.");
    int level = -1;
    ErrCode ret = wifiDevicePtr->GetSignalLevel(rssi, band, level);
    if (ret != WIFI_OPT_SUCCESS) {
        WIFI_LOGW("[Napi Device] Get wifi signal level fail: %{public}d", ret);
    }

    napi_value result;
    napi_create_uint32(env, level, &result);
    return result;
}
}  // namespace Wifi
}  // namespace OHOS
