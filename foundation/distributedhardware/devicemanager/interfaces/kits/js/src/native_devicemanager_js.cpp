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

#include "native_devicemanager_js.h"

#include <securec.h>
#include "nlohmann/json.hpp"

#include "device_manager.h"
#include "device_manager_log.h"
#include "constants.h"

using namespace OHOS::DistributedHardware;

namespace {
#define GET_PARAMS(env, info, num)      \
    size_t argc = num;                  \
    napi_value argv[num] = { nullptr }; \
    napi_value thisVar = nullptr;       \
    void *data = nullptr;               \
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, &thisVar, &data))

const std::string DM_NAPI_EVENT_DEVICE_STATE_CHANGE = "deviceStateChange";
const std::string DM_NAPI_EVENT_DEVICE_FOUND = "deviceFound";
const std::string DM_NAPI_EVENT_DEVICE_DISCOVER_FAIL = "discoverFail";
const std::string DM_NAPI_EVENT_DMFA_CALLBACK = "dmFaCallback";
const std::string DM_NAPI_EVENT_DEVICE_SERVICE_DIE = "serviceDie";

const std::string DEVICE_MANAGER_NAPI_CLASS_NAME = "DeviceManager";

const int32_t DM_NAPI_ARGS_ONE = 1;
const int32_t DM_NAPI_ARGS_TWO = 2;
const int32_t DM_NAPI_ARGS_THREE = 3;
const int32_t DM_NAPI_SUB_ID_MAX = 65535;

const int32_t DM_AUTH_TYPE_PINCODE = 1;
const int32_t DM_AUTH_DIRECTION_CLIENT = 1;

const int32_t DM_NAPI_SUBSCRIBE_CAPABILITY_DDMP = 0;
const int32_t DM_NAPI_SUBSCRIBE_CAPABILITY_OSD = 1;

std::map<std::string, DeviceManagerNapi *> g_deviceManagerMap;
std::map<std::string, std::shared_ptr<DmNapiInitCallback>> g_initCallbackMap;
std::map<std::string, std::shared_ptr<DmNapiDeviceStateCallback>> g_deviceStateCallbackMap;
std::map<std::string, std::shared_ptr<DmNapiDiscoverCallback>> g_discoverCallbackMap;
std::map<std::string, std::shared_ptr<DmNapiAuthenticateCallback>> g_authCallbackMap;
std::map<std::string, std::shared_ptr<DmNapiCheckAuthCallback>> g_checkAuthCallbackMap;
std::map<std::string, std::shared_ptr<DmNapiDeviceManagerFaCallback>> g_dmfaCallbackMap;
}

napi_ref DeviceManagerNapi::sConstructor_ = nullptr;
AuthAsyncCallbackInfo DeviceManagerNapi::authAsyncCallbackInfo_;
AuthAsyncCallbackInfo DeviceManagerNapi::verifyAsyncCallbackInfo_;

void DmNapiInitCallback::OnRemoteDied()
{
    DeviceManagerNapi *deviceManagerNapi = DeviceManagerNapi::GetDeviceManagerNapi(bundleName_);
    if (deviceManagerNapi == nullptr) {
        DMLOG(DM_LOG_ERROR, "OnRemoteDied, deviceManagerNapi not find for bunderName %s", bundleName_.c_str());
        return;
    }
    deviceManagerNapi->OnEvent("serviceDie", 0, nullptr);
}

void DmNapiDeviceStateCallback::OnDeviceOnline(const DmDeviceInfo &deviceInfo)
{
    DeviceManagerNapi *deviceManagerNapi = DeviceManagerNapi::GetDeviceManagerNapi(bundleName_);
    if (deviceManagerNapi == nullptr) {
        DMLOG(DM_LOG_ERROR, "OnDeviceOnline, deviceManagerNapi not find for bunderName %s", bundleName_.c_str());
        return;
    }
    deviceManagerNapi->OnDeviceStateChange(DmNapiDevStateChangeAction::ONLINE, deviceInfo);
}

void DmNapiDeviceStateCallback::OnDeviceReady(const DmDeviceInfo &deviceInfo)
{
    DeviceManagerNapi *deviceManagerNapi = DeviceManagerNapi::GetDeviceManagerNapi(bundleName_);
    if (deviceManagerNapi == nullptr) {
        DMLOG(DM_LOG_ERROR, "OnDeviceOnline, deviceManagerNapi not find for bunderName %s", bundleName_.c_str());
        return;
    }
    deviceManagerNapi->OnDeviceStateChange(DmNapiDevStateChangeAction::READY, deviceInfo);
}

void DmNapiDeviceStateCallback::OnDeviceOffline(const DmDeviceInfo &deviceInfo)
{
    DeviceManagerNapi *deviceManagerNapi = DeviceManagerNapi::GetDeviceManagerNapi(bundleName_);
    if (deviceManagerNapi == nullptr) {
        DMLOG(DM_LOG_ERROR, "OnDeviceOffline, deviceManagerNapi not find for bunderName %s", bundleName_.c_str());
        return;
    }
    deviceManagerNapi->OnDeviceStateChange(DmNapiDevStateChangeAction::OFFLINE, deviceInfo);
}

void DmNapiDeviceStateCallback::OnDeviceChanged(const DmDeviceInfo &deviceInfo)
{
    DeviceManagerNapi *deviceManagerNapi = DeviceManagerNapi::GetDeviceManagerNapi(bundleName_);
    if (deviceManagerNapi == nullptr) {
        DMLOG(DM_LOG_ERROR, "OnDeviceChanged, deviceManagerNapi not find for bunderName %s", bundleName_.c_str());
        return;
    }
    deviceManagerNapi->OnDeviceStateChange(DmNapiDevStateChangeAction::CHANGE, deviceInfo);
}

void DmNapiDiscoverCallback::OnDeviceFound(uint16_t subscribeId,  const DmDeviceInfo &deviceInfo)
{
    DeviceManagerNapi *deviceManagerNapi = DeviceManagerNapi::GetDeviceManagerNapi(bundleName_);
    if (deviceManagerNapi == nullptr) {
        DMLOG(DM_LOG_ERROR, "OnDeviceFound, deviceManagerNapi not find for bunderName %s", bundleName_.c_str());
        return;
    }

    DMLOG(DM_LOG_INFO, "OnDeviceFound for %s, subscribeId %d", bundleName_.c_str(), (int32_t)subscribeId);
    deviceManagerNapi->OnDeviceFound(subscribeId, deviceInfo);
}

void DmNapiDiscoverCallback::OnDiscoverFailed(uint16_t subscribeId, int32_t failedReason)
{
    DeviceManagerNapi *deviceManagerNapi = DeviceManagerNapi::GetDeviceManagerNapi(bundleName_);
    if (deviceManagerNapi == nullptr) {
        DMLOG(DM_LOG_ERROR, "OnDiscoverFailed, deviceManagerNapi not find for bunderName %s", bundleName_.c_str());
        return;
    }

    deviceManagerNapi->OnDiscoverFailed(subscribeId, failedReason);
}

void DmNapiDiscoverCallback::OnDiscoverySuccess(uint16_t subscribeId)
{
    DeviceManagerNapi *deviceManagerNapi = DeviceManagerNapi::GetDeviceManagerNapi(bundleName_);
    if (deviceManagerNapi == nullptr) {
        DMLOG(DM_LOG_ERROR, "OnDiscoverySuccess, deviceManagerNapi not find for bunderName %s", bundleName_.c_str());
        return;
    }
    DMLOG(DM_LOG_INFO, "DiscoverySuccess for %s, subscribeId %d", bundleName_.c_str(), (int32_t)subscribeId);
}

void DmNapiDiscoverCallback::IncreaseRefCount()
{
    refCount_++;
}

void DmNapiDiscoverCallback::DecreaseRefCount()
{
    refCount_--;
}

int32_t DmNapiDiscoverCallback::GetRefCount()
{
    return refCount_;
}

void DmNapiAuthenticateCallback::OnAuthResult(std::string &deviceId, int32_t pinToken, int32_t status, int32_t reason)
{
    DeviceManagerNapi *deviceManagerNapi = DeviceManagerNapi::GetDeviceManagerNapi(bundleName_);
    if (deviceManagerNapi == nullptr) {
        DMLOG(DM_LOG_ERROR, "OnAuthResult, deviceManagerNapi not find for bunderName %s", bundleName_.c_str());
        return;
    }
    deviceManagerNapi->OnAuthResult(deviceId, pinToken, status, reason);
}

void DmNapiCheckAuthCallback::OnCheckAuthResult(std::string &deviceId, int32_t resultCode, int32_t flag)
{
    DeviceManagerNapi *deviceManagerNapi = DeviceManagerNapi::GetDeviceManagerNapi(bundleName_);
    if (deviceManagerNapi == nullptr) {
        DMLOG(DM_LOG_ERROR, "OnCheckAuthResult, deviceManagerNapi not find for bunderName %s", bundleName_.c_str());
        return;
    }
    deviceManagerNapi->OnVerifyResult(deviceId, resultCode, flag);
}

void DmNapiDeviceManagerFaCallback::OnCall(std::string &paramJson)
{
    DeviceManagerNapi *deviceManagerNapi = DeviceManagerNapi::GetDeviceManagerNapi(bundleName_);
    if (deviceManagerNapi == nullptr) {
        DMLOG(DM_LOG_ERROR, "OnCall, deviceManagerNapi not find for bunderName %s", bundleName_.c_str());
        return;
    }
    deviceManagerNapi->OnDmfaCall(paramJson);
}

DeviceManagerNapi::DeviceManagerNapi(napi_env env, napi_value thisVar) : DmNativeEvent(env, thisVar)
{
    env_ = env;
    wrapper_ = nullptr;
}

DeviceManagerNapi::~DeviceManagerNapi()
{
    if (wrapper_ != nullptr) {
        napi_delete_reference(env_, wrapper_);
    }
}

DeviceManagerNapi *DeviceManagerNapi::GetDeviceManagerNapi(std::string &buldleName)
{
    auto iter = g_deviceManagerMap.find(buldleName);
    if (iter == g_deviceManagerMap.end()) {
        return nullptr;
    }
    return iter->second;
}

void DeviceManagerNapi::OnDeviceStateChange(DmNapiDevStateChangeAction action, const DmDeviceInfo &deviceInfo)
{
    napi_value result = nullptr;
    napi_create_object(env_, &result);
    SetValueInt32(env_, "action", (int)action, result);

    napi_value device = nullptr;
    napi_create_object(env_, &device);
    SetValueUtf8String(env_, "deviceId", deviceInfo.deviceId, device);
    SetValueUtf8String(env_, "deviceName", deviceInfo.deviceName, device);
    SetValueInt32(env_, "deviceType", (int)deviceInfo.deviceTypeId, device);

    napi_set_named_property(env_, result, "device", device);
    OnEvent("deviceStateChange", DM_NAPI_ARGS_ONE, &result);
}

void DeviceManagerNapi::OnDeviceFound(uint16_t subscribeId, const DmDeviceInfo &deviceInfo)
{
    DMLOG(DM_LOG_INFO, "OnDeviceFound for subscribeId %d", (int32_t)subscribeId);
    napi_value result = nullptr;
    napi_create_object(env_, &result);
    SetValueInt32(env_, "subscribeId", (int)subscribeId, result);

    napi_value device = nullptr;
    napi_create_object(env_, &device);
    SetValueUtf8String(env_, "deviceId", deviceInfo.deviceId, device);
    SetValueUtf8String(env_, "deviceName", deviceInfo.deviceName, device);
    SetValueInt32(env_, "deviceType", (int)deviceInfo.deviceTypeId, device);

    napi_set_named_property(env_, result, "device", device);
    OnEvent("deviceFound", DM_NAPI_ARGS_ONE, &result);
}

void DeviceManagerNapi::OnDiscoverFailed(uint16_t subscribeId, int32_t failedReason)
{
    DMLOG(DM_LOG_INFO, "OnDiscoverFailed for subscribeId %d", (int32_t)subscribeId);
    napi_value result = nullptr;
    napi_create_object(env_, &result);
    SetValueInt32(env_, "subscribeId", (int)subscribeId, result);
    SetValueInt32(env_, "reason", (int)failedReason, result);
    OnEvent("discoverFail", DM_NAPI_ARGS_ONE, &result);
}

void DeviceManagerNapi::OnDmfaCall(const std::string &paramJson)
{
    DMLOG(DM_LOG_INFO, "OnCall for paramJson");
    napi_value result = nullptr;
    napi_create_object(env_, &result);
    SetValueUtf8String(env_, "param", paramJson, result);
    OnEvent("dmFaCallback", DM_NAPI_ARGS_ONE, &result);
}

void DeviceManagerNapi::OnAuthResult(const std::string &deviceId, int32_t pinToken, int32_t status, int32_t reason)
{
    DMLOG(DM_LOG_INFO, "OnAuthResult for status: %d, reason: %d", status, reason);
    napi_value thisVar = nullptr;
    napi_get_reference_value(env_, thisVarRef_, &thisVar);
    napi_value result[DM_NAPI_ARGS_TWO] = { 0 };

    if (status == 0) {
        DMLOG(DM_LOG_INFO, "OnAuthResult success");
        napi_get_undefined(env_, &result[0]);
        napi_create_object(env_, &result[1]);
        SetValueUtf8String(env_, "deviceId", deviceId, result[1]);
        if (authAsyncCallbackInfo_.authType == DM_AUTH_TYPE_PINCODE) {
            SetValueInt32(env_, "pinTone", pinToken, result[1]);
        }
    } else {
        DMLOG(DM_LOG_INFO, "OnAuthResult failed");
        napi_create_object(env_, &result[0]);
        SetValueInt32(env_, "code", status, result[0]);
        SetValueInt32(env_, "reason", reason, result[0]);
        napi_get_undefined(env_, &result[1]);
    }

    napi_value callResult = nullptr;
    napi_value handler = nullptr;
    napi_get_reference_value(env_, authAsyncCallbackInfo_.callback, &handler);
    if (handler != nullptr) {
        napi_call_function(env_, nullptr, handler, DM_NAPI_ARGS_TWO, &result[0], &callResult);
        napi_delete_reference(env_, verifyAsyncCallbackInfo_.callback);
    } else {
        DMLOG(DM_LOG_ERROR, "handler is nullptr");
    }
    g_authCallbackMap.erase(bundleName_);
}

void DeviceManagerNapi::OnVerifyResult(const std::string &deviceId, int32_t resultCode, int32_t flag)
{
    DMLOG(DM_LOG_INFO, "OnVerifyResult for resultCode: %d, flag: %d", resultCode, flag);
    napi_value thisVar = nullptr;
    napi_get_reference_value(env_, thisVarRef_, &thisVar);
    napi_value result[DM_NAPI_ARGS_TWO] = { 0 };
    if (resultCode == 0) {
        napi_get_undefined(env_, &result[0]);
        napi_create_object(env_, &result[1]);
        SetValueUtf8String(env_, "deviceId", deviceId, result[1]);
        SetValueInt32(env_, "level", flag, result[1]);
    } else {
        napi_create_object(env_, &result[0]);
        SetValueInt32(env_, "code", resultCode, result[0]);
        napi_get_undefined(env_, &result[1]);
    }

    napi_value callResult = nullptr;
    napi_value handler = nullptr;
    napi_get_reference_value(env_, verifyAsyncCallbackInfo_.callback, &handler);
    if (handler != nullptr) {
        napi_call_function(env_, nullptr, handler, DM_NAPI_ARGS_TWO, &result[0], &callResult);
        napi_delete_reference(env_, verifyAsyncCallbackInfo_.callback);
    } else {
        DMLOG(DM_LOG_ERROR, "handler is nullptr");
    }
    g_checkAuthCallbackMap.erase(bundleName_);
}

void DeviceManagerNapi::SetValueUtf8String(const napi_env &env, const std::string &fieldStr, const std::string &str,
    napi_value &result)
{
    napi_value value = nullptr;
    napi_create_string_utf8(env, str.c_str(), NAPI_AUTO_LENGTH, &value);
    napi_set_named_property(env, result, fieldStr.c_str(), value);
}

void DeviceManagerNapi::SetValueInt32(const napi_env &env, const std::string &fieldStr, const int32_t intValue,
    napi_value &result)
{
    napi_value value = nullptr;
    napi_create_int32(env, intValue, &value);
    napi_set_named_property(env, result, fieldStr.c_str(), value);
}

void DeviceManagerNapi::DeviceInfoToJsArray(const napi_env &env,
    const std::vector<DmDeviceInfo> &vecDevInfo,
    const int32_t idx, napi_value &arrayResult)
{
    napi_value result = nullptr;
    napi_create_object(env, &result);

    SetValueUtf8String(env, "deviceId", vecDevInfo[idx].deviceId, result);
    SetValueUtf8String(env, "deviceName", vecDevInfo[idx].deviceName, result);
    SetValueInt32(env, "deviceType", (int)vecDevInfo[idx].deviceTypeId, result);

    napi_status status = napi_set_element(env, arrayResult, idx, result);
    if (status != napi_ok) {
        DMLOG(DM_LOG_ERROR, "DmDeviceInfo To JsArray set element error: %d", status);
    }
}

void DeviceManagerNapi::DmAuthParamToJsAuthParamy(const napi_env &env,
    const DmAuthParam &authParam, napi_value &paramResult)
{
    DMLOG(DM_LOG_INFO, "DmAuthParamToJsAuthParamy in");
    SetValueInt32(env, "authType", authParam.authType, paramResult);

    napi_value extraInfo = nullptr;
    napi_create_object(env, &extraInfo);
    SetValueInt32(env, "direction", authParam.direction, extraInfo);
    SetValueInt32(env, "pinToken", authParam.pinToken, extraInfo);
    if (authParam.direction == DM_AUTH_DIRECTION_CLIENT) {
        napi_set_named_property(env, paramResult, "extraInfo", extraInfo);
        return;
    }

    SetValueUtf8String(env, "packageName", authParam.packageName, extraInfo);
    SetValueUtf8String(env, "appName", authParam.appName, extraInfo);
    SetValueUtf8String(env, "appDescription", authParam.appDescription, extraInfo);
    SetValueInt32(env, "business", authParam.business, extraInfo);
    SetValueInt32(env, "pinCode", authParam.pincode, extraInfo);
    napi_set_named_property(env, paramResult, "extraInfo", extraInfo);

    size_t appIconLen = (size_t)authParam.imageinfo.GetAppIconLen();
    if (appIconLen > 0) {
        void *appIcon = nullptr;
        napi_value appIconBuffer = nullptr;
        napi_create_arraybuffer(env, appIconLen, &appIcon, &appIconBuffer);
        if (appIcon != nullptr &&
            memcpy_s(appIcon, appIconLen, reinterpret_cast<const void*>(authParam.imageinfo.GetAppIcon()),
                appIconLen) == 0) {
            napi_value appIconArray = nullptr;
            napi_create_typedarray(env, napi_uint8_array, appIconLen, appIconBuffer, 0, &appIconArray);
            napi_set_named_property(env, paramResult, "appIcon", appIconArray);
        }
    }

    size_t appThumbnailLen = (size_t)authParam.imageinfo.GetAppThumbnailLen();
    if (appThumbnailLen > 0) {
        void *appThumbnail = nullptr;
        napi_value appThumbnailBuffer = nullptr;
        napi_create_arraybuffer(env, appThumbnailLen, &appThumbnail, &appThumbnailBuffer);
        if (appThumbnail != nullptr &&
            memcpy_s(appThumbnail, appThumbnailLen,
                reinterpret_cast<const void*>(authParam.imageinfo.GetAppThumbnail()), appThumbnailLen) == 0) {
            napi_value appThumbnailArray = nullptr;
            napi_create_typedarray(env, napi_uint8_array, appThumbnailLen, appThumbnailBuffer, 0, &appThumbnailArray);
            napi_set_named_property(env, paramResult, "appThumbnail", appThumbnailArray);
        }
    }
}

void DeviceManagerNapi::JsObjectToString(const napi_env &env, const napi_value &object,
    const std::string &fieldStr, char *dest, const int32_t destLen)
{
    bool hasProperty = false;
    NAPI_CALL_RETURN_VOID(env, napi_has_named_property(env, object, fieldStr.c_str(), &hasProperty));
    if (hasProperty) {
        napi_value field = nullptr;
        napi_valuetype valueType = napi_undefined;

        napi_get_named_property(env, object, fieldStr.c_str(), &field);
        NAPI_CALL_RETURN_VOID(env, napi_typeof(env, field, &valueType));
        NAPI_ASSERT_RETURN_VOID(env, valueType == napi_string, "Wrong argument type. String expected.");
        size_t result = 0;
        NAPI_CALL_RETURN_VOID(env, napi_get_value_string_utf8(env, field, dest, destLen, &result));
    } else {
        DMLOG(DM_LOG_ERROR, "devicemanager napi js to str no property: %s", fieldStr.c_str());
    }
}

std::string DeviceManagerNapi::JsObjectToString(const napi_env &env, const napi_value &param)
{
    DMLOG(DM_LOG_INFO, "JsObjectToString in.");
    size_t size = 0;
    if (napi_get_value_string_utf8(env, param, nullptr, 0, &size) != napi_ok) {
        return "";
    }
    if (size == 0) {
        return "";
    }
    char *buf = new (std::nothrow) char[size + 1];
    if (buf == nullptr) {
        return "";
    }
    memset_s(buf, (size + 1), 0, (size + 1));
    bool rev = napi_get_value_string_utf8(env, param, buf, size + 1, &size) == napi_ok;

    std::string value;
    if (rev) {
        value = buf;
    } else {
        value = "";
    }
    delete[] buf;
    buf = nullptr;
    return value;
}

void DeviceManagerNapi::JsObjectToInt(const napi_env &env, const napi_value &object,
    const std::string &fieldStr, int32_t &fieldRef)
{
    bool hasProperty = false;
    NAPI_CALL_RETURN_VOID(env, napi_has_named_property(env, object, fieldStr.c_str(), &hasProperty));
    if (hasProperty) {
        napi_value field = nullptr;
        napi_valuetype valueType = napi_undefined;

        napi_get_named_property(env, object, fieldStr.c_str(), &field);
        NAPI_CALL_RETURN_VOID(env, napi_typeof(env, field, &valueType));
        NAPI_ASSERT_RETURN_VOID(env, valueType == napi_number, "Wrong argument type. Number expected.");
        napi_get_value_int32(env, field, &fieldRef);
    } else {
        DMLOG(DM_LOG_ERROR, "devicemanager napi js to int no property: %s", fieldStr.c_str());
    }
}

void DeviceManagerNapi::JsObjectToBool(const napi_env &env, const napi_value &object,
    const std::string &fieldStr, bool &fieldRef)
{
    bool hasProperty = false;
    NAPI_CALL_RETURN_VOID(env, napi_has_named_property(env, object, fieldStr.c_str(), &hasProperty));
    if (hasProperty) {
        napi_value field = nullptr;
        napi_valuetype valueType = napi_undefined;

        napi_get_named_property(env, object, fieldStr.c_str(), &field);
        NAPI_CALL_RETURN_VOID(env, napi_typeof(env, field, &valueType));
        NAPI_ASSERT_RETURN_VOID(env, valueType == napi_boolean, "Wrong argument type. Bool expected.");
        napi_get_value_bool(env, field, &fieldRef);
    } else {
        DMLOG(DM_LOG_ERROR, "devicemanager napi js to bool no property: %s", fieldStr.c_str());
    }
}

int32_t DeviceManagerNapi::JsToDmSubscribeInfo(const napi_env &env, const napi_value &object,
    DmSubscribeInfo &info)
{
    int32_t subscribeId = -1;
    JsObjectToInt(env, object, "subscribeId", subscribeId);
    if (subscribeId < 0 || subscribeId > DM_NAPI_SUB_ID_MAX) {
        DMLOG(DM_LOG_ERROR, "DeviceManagerNapi::JsToDmSubscribeInfo, subscribeId error, subscribeId: %d ", subscribeId);
        return -1;
    }

    info.subscribeId = (uint16_t)subscribeId;

    int32_t mode = -1;
    JsObjectToInt(env, object, "mode", mode);
    info.mode = (DmDiscoverMode)mode;

    int32_t medium = -1;
    JsObjectToInt(env, object, "medium", medium);
    info.medium = (DmExchangeMedium)medium;

    int32_t freq = -1;
    JsObjectToInt(env, object, "freq", freq);
    info.freq = (DmExchangeFreq)freq;

    JsObjectToBool(env, object, "isSameAccount", info.isSameAccount);
    JsObjectToBool(env, object, "isWakeRemote", info.isWakeRemote);

    int32_t capability = -1;
    JsObjectToInt(env, object, "capability", capability);
    if (capability == DM_NAPI_SUBSCRIBE_CAPABILITY_DDMP || capability == DM_NAPI_SUBSCRIBE_CAPABILITY_OSD) {
        (void)strncpy_s(info.capability, sizeof(info.capability), DM_CAPABILITY_OSD, strlen(DM_CAPABILITY_OSD));
    }
    return 0;
}

void DeviceManagerNapi::JsToDmDeviceInfo(const napi_env &env, const napi_value &object,
    DmDeviceInfo &info)
{
    JsObjectToString(env, object, "deviceId", info.deviceId, sizeof(info.deviceId));
    JsObjectToString(env, object, "deviceName", info.deviceName, sizeof(info.deviceName));
    int32_t deviceType = -1;
    JsObjectToInt(env, object, "deviceType", deviceType);
    info.deviceTypeId = (DMDeviceType)deviceType;
}

void DeviceManagerNapi::JsToDmAppImageInfoAndDmExtra(const napi_env &env, const napi_value &object,
    DmAppImageInfo& appImageInfo, std::string &extra, int32_t &authType)
{
    DMLOG(DM_LOG_INFO, "JsToDmAppImageInfoAndDmExtra in.");
    int32_t authTypeTemp = -1;
    JsObjectToInt(env, object, "authType", authTypeTemp);
    authType = authTypeTemp;

    uint8_t *appIconBufferPtr = nullptr;
    int32_t appIconBufferLen = 0;
    JsToDmBuffer(env, object, "appIcon", &appIconBufferPtr, appIconBufferLen);

    uint8_t *appThumbnailBufferPtr = nullptr;
    int32_t appThumbnailBufferLen = 0;
    JsToDmBuffer(env, object, "appThumbnail", &appThumbnailBufferPtr, appThumbnailBufferLen);

    appImageInfo.Reset(appIconBufferPtr, appIconBufferLen, appThumbnailBufferPtr, appThumbnailBufferLen);
    if (appIconBufferPtr != nullptr) {
        free(appIconBufferPtr);
        appIconBufferPtr = nullptr;
    }
    if (appThumbnailBufferPtr != nullptr) {
        free(appThumbnailBufferPtr);
        appThumbnailBufferPtr = nullptr;
    }

    nlohmann::json jsonObj;
    jsonObj[AUTH_TYPE] = authType;
    std::string extraInfo = "extraInfo";

    JsToJsonObject(env, object, "extraInfo", jsonObj);
    extra = jsonObj.dump();
    DMLOG(DM_LOG_INFO, "appIconLen %d, appThumbnailLen %d", appIconBufferLen, appThumbnailBufferLen);
}

void DeviceManagerNapi::JsToDmBuffer(const napi_env &env, const napi_value &object,
    const std::string &fieldStr, uint8_t **bufferPtr, int32_t &bufferLen)
{
    DMLOG(DM_LOG_INFO, "JsToDmBuffer in.");
    bool hasProperty = false;
    NAPI_CALL_RETURN_VOID(env, napi_has_named_property(env, object, fieldStr.c_str(), &hasProperty));
    if (!hasProperty) {
        DMLOG(DM_LOG_ERROR, "devicemanager napi js to str no property: %s", fieldStr.c_str());
        return;
    }

    napi_value field = nullptr;
    napi_get_named_property(env, object, fieldStr.c_str(), &field);
    napi_typedarray_type type = napi_uint8_array;
    size_t length = 0;
    napi_value buffer = nullptr;
    size_t offset = 0;
    uint8_t *data = nullptr;
    napi_get_typedarray_info(env, field, &type,
        &length, reinterpret_cast<void **>(&data), &buffer, &offset);
    if (type != napi_uint8_array || length == 0 || data == nullptr) {
        DMLOG(DM_LOG_ERROR, "Invaild AppIconInfo");
        return;
    }
    *bufferPtr = (uint8_t*)calloc(sizeof(uint8_t), length);
    if (*bufferPtr == nullptr) {
        DMLOG(DM_LOG_ERROR, "low memory, calloc return nullptr, length is %d,  filed %s", length, fieldStr.c_str());
        return;
    }
    if (memcpy_s(*bufferPtr, length, data, length) != 0) {
        DMLOG(DM_LOG_ERROR, "memcpy_s failed, filed %s", fieldStr.c_str());
        free(*bufferPtr);
        *bufferPtr = nullptr;
        return;
    }
    bufferLen = length;
}

void DeviceManagerNapi::JsToJsonObject(const napi_env &env, const napi_value &object,
    const std::string &fieldStr, nlohmann::json &jsonObj)
{
    DMLOG(DM_LOG_INFO, "JsToJsonObject in.");
    bool hasProperty = false;
    NAPI_CALL_RETURN_VOID(env, napi_has_named_property(env, object, fieldStr.c_str(), &hasProperty));
    if (!hasProperty) {
        DMLOG(DM_LOG_ERROR, "devicemanager napi js to str no property: %s", fieldStr.c_str());
        return;
    }

    napi_value jsonField = nullptr;
    napi_get_named_property(env, object, fieldStr.c_str(), &jsonField);
    napi_valuetype jsValueType = napi_undefined;
    napi_value jsProNameList = nullptr;
    uint32_t jsProCount = 0;
    napi_get_property_names(env, jsonField, &jsProNameList);
    napi_get_array_length(env, jsProNameList, &jsProCount);
    DMLOG(DM_LOG_INFO, "Property size=%d.", jsProCount);

    napi_value jsProName = nullptr;
    napi_value jsProValue = nullptr;
    for (uint32_t index = 0; index < jsProCount; index++) {
        napi_get_element(env, jsProNameList, index, &jsProName);
        std::string strProName = JsObjectToString(env, jsProName);
        napi_get_named_property(env, jsonField, strProName.c_str(), &jsProValue);
        napi_typeof(env, jsProValue, &jsValueType);
        switch (jsValueType) {
            case napi_string: {
                std::string natValue = JsObjectToString(env, jsProValue);
                DMLOG(DM_LOG_INFO, "Property name=%s, string, value=%s", strProName.c_str(), natValue.c_str());
                jsonObj[strProName] = natValue;
                break;
            }
            case napi_boolean: {
                bool elementValue = false;
                napi_get_value_bool(env, jsProValue, &elementValue);
                DMLOG(DM_LOG_INFO, "Property name=%s, boolean, value=%d.", strProName.c_str(), elementValue);
                jsonObj[strProName] = elementValue;
                break;
            }
            case napi_number: {
                int32_t elementValue = 0;
                if (napi_get_value_int32(env, jsProValue, &elementValue) != napi_ok) {
                    DMLOG(DM_LOG_ERROR, "Property name=%s, Property int32_t parse error", strProName.c_str());
                } else {
                    jsonObj[strProName] = elementValue;
                    DMLOG(DM_LOG_INFO, "Property name=%s, number, value=%d.", strProName.c_str(), elementValue);
                }
                break;
            }
            default: {
                DMLOG(DM_LOG_ERROR, "Property name=%s, value type not support.", strProName.c_str());
                break;
            }
        }
    }
}

void DeviceManagerNapi::JsToDmAuthInfo(const napi_env &env, const napi_value &object, std::string &extra)
{
    DMLOG(DM_LOG_INFO, "%s called.", __func__);
    int32_t authType = -1;
    int32_t token = -1;

    JsObjectToInt(env, object, "authType", authType);
    JsObjectToInt(env, object, "token", token);

    nlohmann::json jsonObj;
    jsonObj[AUTH_TYPE] = authType;
    if (authType == DM_AUTH_TYPE_PINCODE) {
        jsonObj[PIN_TOKEN] = token;
    } else {
        jsonObj[TOKEN] = token;
    }
    JsToJsonObject(env, object, "extraInfo", jsonObj);
    extra = jsonObj.dump();
}

void DeviceManagerNapi::CreateDmCallback(std::string &bundleName, std::string &eventType)
{
    DMLOG(DM_LOG_ERROR, "CreateDmCallback for bunderName %s eventType %s", bundleName.c_str(), eventType.c_str());
    if (eventType == DM_NAPI_EVENT_DEVICE_STATE_CHANGE) {
        auto iter = g_deviceStateCallbackMap.find(bundleName);
        if (iter == g_deviceStateCallbackMap.end()) {
            auto callback = std::make_shared<DmNapiDeviceStateCallback>(bundleName);
            std::string extra = "";
            int32_t ret = DeviceManager::GetInstance().RegisterDevStateCallback(bundleName, extra, callback);
            if (ret != 0) {
                DMLOG(DM_LOG_ERROR, "RegisterDevStateCallback failed for bunderName %s", bundleName.c_str());
                return;
            }
            g_deviceStateCallbackMap[bundleName] = callback;
        }
        return;
    }

    if (eventType == DM_NAPI_EVENT_DEVICE_FOUND || eventType == DM_NAPI_EVENT_DEVICE_DISCOVER_FAIL) {
        std::shared_ptr<DmNapiDiscoverCallback> discoverCallback = nullptr;
        auto iter = g_discoverCallbackMap.find(bundleName);
        if (iter == g_discoverCallbackMap.end()) {
            auto callback = std::make_shared<DmNapiDiscoverCallback>(bundleName);
            g_discoverCallbackMap[bundleName] = callback;
            discoverCallback = callback;
        } else {
            discoverCallback = iter->second;
        }

        discoverCallback->IncreaseRefCount();
        return;
    }

    if (eventType == DM_NAPI_EVENT_DMFA_CALLBACK) {
        auto iter = g_dmfaCallbackMap.find(bundleName);
        if (iter == g_dmfaCallbackMap.end()) {
            auto callback = std::make_shared<DmNapiDeviceManagerFaCallback>(bundleName);
            int32_t ret = DeviceManager::GetInstance().RegisterDeviceManagerFaCallback(bundleName, callback);
            if (ret != 0) {
                DMLOG(DM_LOG_ERROR, "RegisterDeviceManagerFaCallback failed for bunderName %s", bundleName.c_str());
                return;
            }
            g_dmfaCallbackMap[bundleName] = callback;
        }
        return;
    }
}

void DeviceManagerNapi::ReleaseDmCallback(std::string &bundleName, std::string &eventType)
{
    if (eventType == DM_NAPI_EVENT_DEVICE_STATE_CHANGE) {
        auto iter = g_deviceStateCallbackMap.find(bundleName);
        if (iter == g_deviceStateCallbackMap.end()) {
            DMLOG(DM_LOG_ERROR, "ReleaseDmCallback: cannot find stateCallback for bunderName %s", bundleName.c_str());
            return;
        }
        int32_t ret = DeviceManager::GetInstance().UnRegisterDevStateCallback(bundleName);
        if (ret != 0) {
            DMLOG(DM_LOG_ERROR, "RegisterDevStateCallback failed for bunderName %s", bundleName.c_str());
            return;
        }
        g_deviceStateCallbackMap.erase(bundleName);
        return;
    }

    if (eventType == DM_NAPI_EVENT_DEVICE_FOUND || eventType == DM_NAPI_EVENT_DEVICE_DISCOVER_FAIL) {
        std::shared_ptr<DmNapiDiscoverCallback> discoverCallback = nullptr;
        auto iter = g_discoverCallbackMap.find(bundleName);
        if (iter == g_discoverCallbackMap.end()) {
            return;
        }

        discoverCallback = iter->second;
        discoverCallback->DecreaseRefCount();
        if (discoverCallback->GetRefCount() == 0) {
            g_discoverCallbackMap.erase(bundleName);
        }
        return;
    }

    if (eventType == DM_NAPI_EVENT_DMFA_CALLBACK) {
        auto iter = g_dmfaCallbackMap.find(bundleName);
        if (iter == g_dmfaCallbackMap.end()) {
            DMLOG(DM_LOG_ERROR, "cannot find dmFaCallback for bunderName %s", bundleName.c_str());
            return;
        }
        int32_t ret = DeviceManager::GetInstance().UnRegisterDeviceManagerFaCallback(bundleName);
        if (ret != 0) {
            DMLOG(DM_LOG_ERROR, "RegisterDevStateCallback failed for bunderName %s", bundleName.c_str());
            return;
        }
        g_dmfaCallbackMap.erase(bundleName);
        return;
    }
}

napi_value DeviceManagerNapi::SetUserOperationSync(napi_env env, napi_callback_info info)
{
    DMLOG(DM_LOG_INFO, "SetUserOperationSync in");
    GET_PARAMS(env, info, DM_NAPI_ARGS_ONE);
    napi_valuetype valueType = napi_undefined;
    napi_typeof(env, argv[0], &valueType);
    NAPI_ASSERT(env, valueType == napi_number, "Wrong argument type. Object expected.");

    int32_t action = 0;
    napi_get_value_int32(env, argv[0], &action);
    DeviceManagerNapi *deviceManagerWrapper = nullptr;
    napi_unwrap(env, thisVar, reinterpret_cast<void **>(&deviceManagerWrapper));
    int32_t ret = DeviceManager::GetInstance().SetUserOperation(deviceManagerWrapper->bundleName_, action);
    if (ret != 0) {
        DMLOG(DM_LOG_ERROR, "SetUserOperation for bunderName %s failed, ret %d",
            deviceManagerWrapper->bundleName_.c_str(), ret);
    }
    napi_value result = nullptr;
    napi_get_undefined(env, &result);
    return result;
}

napi_value DeviceManagerNapi::GetAuthenticationParamSync(napi_env env, napi_callback_info info)
{
    DMLOG(DM_LOG_INFO, "GetAuthenticationParamSync in");
    size_t argc = 0;
    napi_value thisVar = nullptr;
    napi_value resultParam = nullptr;

    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, nullptr, &thisVar, nullptr));
    NAPI_ASSERT(env, argc == 0, "Wrong number of arguments");

    DeviceManagerNapi *deviceManagerWrapper = nullptr;
    napi_unwrap(env, thisVar, reinterpret_cast<void **>(&deviceManagerWrapper));

    DmAuthParam authParam;
    int32_t ret = DeviceManager::GetInstance().GetAuthenticationParam(deviceManagerWrapper->bundleName_, authParam);
    if (ret != 0) {
        DMLOG(DM_LOG_ERROR, "GetAuthenticationParam for %s failed, ret %d",
            deviceManagerWrapper->bundleName_.c_str(), ret);
        napi_get_undefined(env, &resultParam);
        return resultParam;
    }

    napi_create_object(env, &resultParam);
    DmAuthParamToJsAuthParamy(env, authParam, resultParam);
    return resultParam;
}

napi_value DeviceManagerNapi::GetTrustedDeviceListSync(napi_env env, napi_callback_info info)
{
    DMLOG(DM_LOG_INFO, "GetTrustedDeviceList in");
    size_t argc = 0;
    napi_value thisVar = nullptr;
    napi_value array = nullptr;

    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, nullptr, &thisVar, nullptr));
    NAPI_ASSERT(env, argc == 0, "Wrong number of arguments");

    DeviceManagerNapi *deviceManagerWrapper = nullptr;
    napi_unwrap(env, thisVar, reinterpret_cast<void **>(&deviceManagerWrapper));
    std::string extra = "";
    std::vector<DmDeviceInfo> devList;
    int32_t ret = DeviceManager::GetInstance().GetTrustedDeviceList(deviceManagerWrapper->bundleName_, extra, devList);
    if (ret != 0) {
        DMLOG(DM_LOG_ERROR, "GetTrustedDeviceList for bunderName %s failed, ret %d",
            deviceManagerWrapper->bundleName_.c_str(), ret);
        return array;
    }

    if (devList.size() > 0) {
        bool isArray = false;
        napi_create_array(env, &array);
        napi_is_array(env, array, &isArray);
        if (isArray == false) {
            DMLOG(DM_LOG_ERROR, "napi_create_array fail");
        }

        for (size_t i = 0; i != devList.size(); ++i) {
            DeviceInfoToJsArray(env, devList, i, array);
        }
    } else {
        DMLOG(DM_LOG_ERROR, "devList is null");
    }

    return array;
}

napi_value DeviceManagerNapi::StartDeviceDiscoverSync(napi_env env, napi_callback_info info)
{
    DMLOG(DM_LOG_INFO, "StartDeviceDiscoverSync in");
    GET_PARAMS(env, info, DM_NAPI_ARGS_ONE);
    napi_value result = nullptr;
    napi_valuetype valueType = napi_undefined;
    napi_typeof(env, argv[0], &valueType);
    NAPI_ASSERT(env, valueType == napi_object, "Wrong argument type. Object expected.");

    DeviceManagerNapi *deviceManagerWrapper = nullptr;
    napi_unwrap(env, thisVar, reinterpret_cast<void **>(&deviceManagerWrapper));

    std::shared_ptr<DmNapiDiscoverCallback> discoverCallback = nullptr;
    auto iter = g_discoverCallbackMap.find(deviceManagerWrapper->bundleName_);
    if (iter == g_discoverCallbackMap.end()) {
        discoverCallback = std::make_shared<DmNapiDiscoverCallback>(deviceManagerWrapper->bundleName_);
        g_discoverCallbackMap[deviceManagerWrapper->bundleName_] = discoverCallback;
    } else {
        discoverCallback = iter->second;
    }
    DmSubscribeInfo subInfo;
    int32_t res = JsToDmSubscribeInfo(env, argv[0], subInfo);
    NAPI_ASSERT(env, res == 0, "Wrong subscribeId ");

    int32_t ret = DeviceManager::GetInstance().StartDeviceDiscovery(deviceManagerWrapper->bundleName_,
        subInfo, discoverCallback);
    if (ret != 0) {
        DMLOG(DM_LOG_ERROR, "StartDeviceDiscovery for bunderName %s failed, ret %d",
            deviceManagerWrapper->bundleName_.c_str(), ret);
        return result;
    }

    napi_get_undefined(env, &result);
    return result;
}

napi_value DeviceManagerNapi::StopDeviceDiscoverSync(napi_env env, napi_callback_info info)
{
    DMLOG(DM_LOG_INFO, "StopDeviceDiscoverSync in");
    GET_PARAMS(env, info, DM_NAPI_ARGS_ONE);
    napi_value result = nullptr;
    napi_valuetype valueType = napi_undefined;
    napi_typeof(env, argv[0], &valueType);
    NAPI_ASSERT(env, valueType == napi_number, "Wrong argument type. Object expected.");

    int32_t subscribeId = 0;
    napi_get_value_int32(env, argv[0], &subscribeId);
    NAPI_ASSERT(env, subscribeId <= DM_NAPI_SUB_ID_MAX, "Wrong argument. subscribeId Too Big.");

    DeviceManagerNapi *deviceManagerWrapper = nullptr;
    napi_unwrap(env, thisVar, reinterpret_cast<void **>(&deviceManagerWrapper));
    int32_t ret = DeviceManager::GetInstance().StopDeviceDiscovery(deviceManagerWrapper->bundleName_,
        (int16_t)subscribeId);
    if (ret != 0) {
        DMLOG(DM_LOG_ERROR, "StopDeviceDiscovery for bunderName %s failed, ret %d",
            deviceManagerWrapper->bundleName_.c_str(), ret);
        return result;
    }

    napi_get_undefined(env, &result);
    return result;
}

napi_value DeviceManagerNapi::AuthenticateDevice(napi_env env, napi_callback_info info)
{
    const int32_t PARAM_INDEX_ONE = 1;
    const int32_t PARAM_INDEX_TWO = 2;
    DMLOG(DM_LOG_INFO, "AuthenticateDevice in");
    GET_PARAMS(env, info, DM_NAPI_ARGS_THREE);
    napi_value result = nullptr;
    napi_valuetype deviceInfoType = napi_undefined;
    napi_typeof(env, argv[0], &deviceInfoType);
    NAPI_ASSERT(env, deviceInfoType == napi_object, "Wrong argument type. Object expected.");

    napi_valuetype authparamType = napi_undefined;
    napi_typeof(env, argv[PARAM_INDEX_ONE], &authparamType);
    NAPI_ASSERT(env, authparamType == napi_object, "Wrong argument type. Object expected.");

    napi_valuetype eventHandleType = napi_undefined;
    napi_typeof(env, argv[PARAM_INDEX_TWO], &eventHandleType);
    NAPI_ASSERT(env, eventHandleType == napi_function, "Wrong argument type. Function expected.");

    authAsyncCallbackInfo_.env = env;
    napi_create_reference(env, argv[PARAM_INDEX_TWO], 1, &authAsyncCallbackInfo_.callback);

    DeviceManagerNapi *deviceManagerWrapper = nullptr;
    napi_unwrap(env, thisVar, reinterpret_cast<void **>(&deviceManagerWrapper));

    std::shared_ptr<DmNapiAuthenticateCallback> authCallback = nullptr;
    auto iter = g_authCallbackMap.find(deviceManagerWrapper->bundleName_);
    if (iter == g_authCallbackMap.end()) {
        authCallback = std::make_shared<DmNapiAuthenticateCallback>(deviceManagerWrapper->bundleName_);
        g_authCallbackMap[deviceManagerWrapper->bundleName_] = authCallback;
    } else {
        authCallback = iter->second;
    }
    DmDeviceInfo deviceInfo;
    JsToDmDeviceInfo(env, argv[0], deviceInfo);

    DmAppImageInfo appImageInfo(nullptr, 0, nullptr, 0);
    std::string extra;
    JsToDmAppImageInfoAndDmExtra(env, argv[PARAM_INDEX_ONE], appImageInfo, extra, authAsyncCallbackInfo_.authType);

    int32_t ret = DeviceManager::GetInstance().AuthenticateDevice(deviceManagerWrapper->bundleName_, deviceInfo,
        appImageInfo, extra, authCallback);
    if (ret != 0) {
        DMLOG(DM_LOG_ERROR, "AuthenticateDevice for bunderName %s failed, ret %d",
            deviceManagerWrapper->bundleName_.c_str(), ret);
    }

    napi_get_undefined(env, &result);
    return result;
}

napi_value DeviceManagerNapi::VerifyAuthInfo(napi_env env, napi_callback_info info)
{
    DMLOG(DM_LOG_INFO, "VerifyAuthInfo in");
    GET_PARAMS(env, info, DM_NAPI_ARGS_TWO);
    napi_value result = nullptr;
    napi_valuetype valueType = napi_undefined;
    napi_typeof(env, argv[0], &valueType);
    NAPI_ASSERT(env, valueType == napi_object, "Wrong argument type. Object expected.");

    napi_valuetype eventHandleType = napi_undefined;
    napi_typeof(env, argv[1], &eventHandleType);
    NAPI_ASSERT(env, eventHandleType == napi_function, "Wrong argument type. Object expected.");

    verifyAsyncCallbackInfo_.env = env;
    napi_create_reference(env, argv[1], 1, &verifyAsyncCallbackInfo_.callback);

    DeviceManagerNapi *deviceManagerWrapper = nullptr;
    napi_unwrap(env, thisVar, reinterpret_cast<void **>(&deviceManagerWrapper));

    std::shared_ptr<DmNapiCheckAuthCallback> verifyCallback = nullptr;
    auto iter = g_checkAuthCallbackMap.find(deviceManagerWrapper->bundleName_);
    if (iter == g_checkAuthCallbackMap.end()) {
        verifyCallback = std::make_shared<DmNapiCheckAuthCallback>(deviceManagerWrapper->bundleName_);
        g_checkAuthCallbackMap[deviceManagerWrapper->bundleName_] = verifyCallback;
    } else {
        verifyCallback = iter->second;
    }
    std::string authParam;
    JsToDmAuthInfo(env, argv[0], authParam);

    int32_t ret = DeviceManager::GetInstance().CheckAuthentication(deviceManagerWrapper->bundleName_,
        authParam, verifyCallback);
    if (ret != 0) {
        DMLOG(DM_LOG_ERROR, "VerifyAuthInfo for bunderName %s failed, ret %d",
            deviceManagerWrapper->bundleName_.c_str(), ret);
    }

    napi_get_undefined(env, &result);
    return result;
}

napi_value DeviceManagerNapi::JsOn(napi_env env, napi_callback_info info)
{
    DMLOG(DM_LOG_INFO, "JsOn in");
    GET_PARAMS(env, info, DM_NAPI_ARGS_TWO);
    NAPI_ASSERT(env, argc >= DM_NAPI_ARGS_TWO, "Wrong number of arguments, required 2");

    napi_valuetype eventValueType = napi_undefined;
    napi_typeof(env, argv[0], &eventValueType);
    NAPI_ASSERT(env, eventValueType == napi_string, "type mismatch for parameter 1");

    napi_valuetype eventHandleType = napi_undefined;
    napi_typeof(env, argv[1], &eventHandleType);
    NAPI_ASSERT(env, eventHandleType == napi_function, "type mismatch for parameter 2");

    size_t typeLen = 0;
    napi_get_value_string_utf8(env, argv[0], nullptr, 0, &typeLen);

    NAPI_ASSERT(env, typeLen > 0, "typeLen == 0");
    NAPI_ASSERT(env, typeLen < DM_NAPI_BUF_LENGTH, "typeLen >= MAXLEN");
    char type[DM_NAPI_BUF_LENGTH] = {0};
    napi_get_value_string_utf8(env, argv[0], type, typeLen + 1, &typeLen);

    std::string eventType = type;
    DeviceManagerNapi *deviceManagerWrapper = nullptr;
    napi_unwrap(env, thisVar, reinterpret_cast<void **>(&deviceManagerWrapper));

    DMLOG(DM_LOG_INFO, "JsOn for bunderName %s, eventType %s ", deviceManagerWrapper->bundleName_.c_str(),
        eventType.c_str());
    deviceManagerWrapper->On(eventType, argv[1]);
    CreateDmCallback(deviceManagerWrapper->bundleName_, eventType);

    napi_value result = nullptr;
    napi_get_undefined(env, &result);
    return result;
}

napi_value DeviceManagerNapi::JsOff(napi_env env, napi_callback_info info)
{
    DMLOG(DM_LOG_INFO, "JsOff in");
    GET_PARAMS(env, info, DM_NAPI_ARGS_TWO);
    size_t requireArgc = 1;
    NAPI_ASSERT(env, argc >= requireArgc, "Wrong number of arguments, required 1");

    napi_valuetype eventValueType = napi_undefined;
    napi_typeof(env, argv[0], &eventValueType);
    NAPI_ASSERT(env, eventValueType == napi_string, "type mismatch for parameter 1");

    if (argc > requireArgc) {
        napi_valuetype eventHandleType = napi_undefined;
        napi_typeof(env, argv[1], &eventHandleType);
        NAPI_ASSERT(env, eventValueType == napi_function, "type mismatch for parameter 2");
    }

    size_t typeLen = 0;
    napi_get_value_string_utf8(env, argv[0], nullptr, 0, &typeLen);

    NAPI_ASSERT(env, typeLen > 0, "typeLen == 0");
    NAPI_ASSERT(env, typeLen < DM_NAPI_BUF_LENGTH, "typeLen >= MAXLEN");
    char type[DM_NAPI_BUF_LENGTH] = {0};
    napi_get_value_string_utf8(env, argv[0], type, typeLen + 1, &typeLen);

    std::string eventType = type;
    DeviceManagerNapi *deviceManagerWrapper = nullptr;
    napi_unwrap(env, thisVar, reinterpret_cast<void **>(&deviceManagerWrapper));

    DMLOG(DM_LOG_INFO, "JsOff for bunderName %s, eventType %s ", deviceManagerWrapper->bundleName_.c_str(),
        eventType.c_str());
    deviceManagerWrapper->Off(eventType);
    ReleaseDmCallback(deviceManagerWrapper->bundleName_, eventType);

    napi_value result = nullptr;
    napi_get_undefined(env, &result);
    return result;
}

napi_value DeviceManagerNapi::ReleaseDeviceManager(napi_env env, napi_callback_info info)
{
    DMLOG(DM_LOG_INFO, "ReleaseDeviceManager in");
    size_t argc = 0;
    napi_value thisVar = nullptr;
    napi_value result = nullptr;

    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, nullptr, &thisVar, nullptr));
    NAPI_ASSERT(env, argc == 0, "Wrong number of arguments");

    DeviceManagerNapi *deviceManagerWrapper = nullptr;
    napi_unwrap(env, thisVar, reinterpret_cast<void **>(&deviceManagerWrapper));
    DMLOG(DM_LOG_INFO, "ReleaseDeviceManager for bunderName %s", deviceManagerWrapper->bundleName_.c_str());
    int32_t ret = DeviceManager::GetInstance().UnInitDeviceManager(deviceManagerWrapper->bundleName_);
    if (ret != 0) {
        DMLOG(DM_LOG_ERROR, "ReleaseDeviceManager for bunderName %s failed, ret %d",
            deviceManagerWrapper->bundleName_.c_str(), ret);
        napi_create_uint32(env, ret, &result);
        return result;
    }

    g_deviceManagerMap.erase(deviceManagerWrapper->bundleName_);
    g_initCallbackMap.erase(deviceManagerWrapper->bundleName_);
    g_deviceStateCallbackMap.erase(deviceManagerWrapper->bundleName_);
    g_discoverCallbackMap.erase(deviceManagerWrapper->bundleName_);
    g_authCallbackMap.erase(deviceManagerWrapper->bundleName_);
    g_checkAuthCallbackMap.erase(deviceManagerWrapper->bundleName_);
    napi_get_undefined(env, &result);
    return result;
}

void DeviceManagerNapi::HandleCreateDmCallBack(const napi_env &env, AsyncCallbackInfo *asCallbackInfo)
{
    napi_value resourceName;
    napi_create_string_latin1(env, "createDeviceManagerCallback", NAPI_AUTO_LENGTH, &resourceName);
    napi_create_async_work(
        env, nullptr, resourceName,
        [](napi_env env, void *data) {
            (void)env;
            AsyncCallbackInfo *asCallbackInfo = (AsyncCallbackInfo *)data;
            std::string bundleName = std::string(asCallbackInfo->bundleName);
            std::shared_ptr<DmNapiInitCallback> initCallback = std::make_shared<DmNapiInitCallback>(bundleName);
            if (DeviceManager::GetInstance().InitDeviceManager(bundleName, initCallback) != 0) {
                DMLOG(DM_LOG_ERROR, "InitDeviceManager for bunderName %s failed", bundleName.c_str());
                return;
            }
            g_initCallbackMap[bundleName] = initCallback;
            asCallbackInfo->status = 0;
        },
        [](napi_env env, napi_status status, void *data) {
            (void)status;
            AsyncCallbackInfo *asCallbackInfo = (AsyncCallbackInfo *)data;
            napi_value result[DM_NAPI_ARGS_TWO] = { 0 };
            napi_value ctor = nullptr;
            napi_value argv = nullptr;
            napi_get_reference_value(env, sConstructor_, &ctor);
            napi_create_string_utf8(env, asCallbackInfo->bundleName, NAPI_AUTO_LENGTH, &argv);
            napi_status ret = napi_new_instance(env, ctor, DM_NAPI_ARGS_ONE, &argv, &result[1]);
            if (ret != napi_ok) {
                DMLOG(DM_LOG_ERROR, "Create DeviceManagerNapi for bunderName %s failed", asCallbackInfo->bundleName);
                asCallbackInfo->status = -1;
            }
            if (asCallbackInfo->status == 0) {
                DMLOG(DM_LOG_INFO, "InitDeviceManager for bunderName %s success", asCallbackInfo->bundleName);
                napi_get_undefined(env, &result[0]);
                napi_value callback = nullptr;
                napi_value callResult = nullptr;
                napi_get_reference_value(env, asCallbackInfo->callback, &callback);
                napi_call_function(env, nullptr, callback, DM_NAPI_ARGS_TWO, &result[0], &callResult);
                napi_delete_reference(env, asCallbackInfo->callback);
            } else {
                DMLOG(DM_LOG_INFO, "InitDeviceManager for bunderName %s failed", asCallbackInfo->bundleName);
                napi_value message = nullptr;
                napi_create_object(env, &result[0]);
                napi_create_int32(env, asCallbackInfo->status, &message);
                napi_set_named_property(env, result[0], "code", message);
                napi_get_undefined(env, &result[1]);
            }
            napi_delete_async_work(env, asCallbackInfo->asyncWork);
            delete asCallbackInfo;
        }, (void *)asCallbackInfo, &asCallbackInfo->asyncWork);
    napi_queue_async_work(env, asCallbackInfo->asyncWork);
}

napi_value DeviceManagerNapi::CreateDeviceManager(napi_env env, napi_callback_info info)
{
    DMLOG(DM_LOG_INFO, "CreateDeviceManager in");
    GET_PARAMS(env, info, DM_NAPI_ARGS_TWO);
    NAPI_ASSERT(env, argc >= DM_NAPI_ARGS_TWO, "Wrong number of arguments, required 2");

    napi_valuetype bundleNameValueType = napi_undefined;
    napi_typeof(env, argv[0], &bundleNameValueType);
    NAPI_ASSERT(env, bundleNameValueType == napi_string, "type mismatch for parameter 0");

    napi_valuetype funcValueType = napi_undefined;
    napi_typeof(env, argv[1], &funcValueType);
    NAPI_ASSERT(env, funcValueType == napi_function, "type mismatch for parameter 1");

    auto *asCallbackInfo = new AsyncCallbackInfo();
    asCallbackInfo->env = env;
    napi_get_value_string_utf8(env, argv[0], asCallbackInfo->bundleName, DM_NAPI_BUF_LENGTH - 1,
        &asCallbackInfo->bundleNameLen);
    napi_create_reference(env, argv[1], 1, &asCallbackInfo->callback);

    HandleCreateDmCallBack(env, asCallbackInfo);

    napi_value result = nullptr;
    napi_get_undefined(env, &result);
    return result;
}

napi_value DeviceManagerNapi::Constructor(napi_env env, napi_callback_info info)
{
    DMLOG(DM_LOG_INFO, "DeviceManagerNapi Constructor in");
    GET_PARAMS(env, info, DM_NAPI_ARGS_ONE);
    NAPI_ASSERT(env, argc >= DM_NAPI_ARGS_ONE, "Wrong number of arguments, required 1");

    napi_valuetype valueType = napi_undefined;
    napi_typeof(env, argv[0], &valueType);
    NAPI_ASSERT(env, valueType == napi_string, "type mismatch for parameter 1");

    char bundleName[DM_NAPI_BUF_LENGTH] = { 0 };
    size_t typeLen = 0;
    napi_get_value_string_utf8(env, argv[0], bundleName, sizeof(bundleName), &typeLen);

    DMLOG(DM_LOG_INFO, "create DeviceManagerNapi for packageName:%s", bundleName);
    DeviceManagerNapi *obj = new DeviceManagerNapi(env, thisVar);
    obj->bundleName_ = std::string(bundleName);
    g_deviceManagerMap[obj->bundleName_] = obj;
    napi_wrap(env, thisVar, reinterpret_cast<void *>(obj),
        [](napi_env env, void *data, void *hint) {
            (void)env;
            (void)hint;
            DeviceManagerNapi *deviceManager = (DeviceManagerNapi *)data;
            delete deviceManager;
        },
        nullptr, &(obj->wrapper_));
    return thisVar;
}

napi_value DeviceManagerNapi::Init(napi_env env, napi_value exports)
{
    napi_value dmClass = nullptr;
    napi_property_descriptor dmProperties[] = {
        DECLARE_NAPI_FUNCTION("release", ReleaseDeviceManager),
        DECLARE_NAPI_FUNCTION("getTrustedDeviceListSync", GetTrustedDeviceListSync),
        DECLARE_NAPI_FUNCTION("startDeviceDiscovery", StartDeviceDiscoverSync),
        DECLARE_NAPI_FUNCTION("stopDeviceDiscovery", StopDeviceDiscoverSync),
        DECLARE_NAPI_FUNCTION("authenticateDevice", AuthenticateDevice),
        DECLARE_NAPI_FUNCTION("verifyAuthInfo", VerifyAuthInfo),
        DECLARE_NAPI_FUNCTION("setUserOperation", SetUserOperationSync),
        DECLARE_NAPI_FUNCTION("getAuthenticationParam", GetAuthenticationParamSync),
        DECLARE_NAPI_FUNCTION("on", JsOn),
        DECLARE_NAPI_FUNCTION("off", JsOff)
        };

    napi_property_descriptor static_prop[] = {
        DECLARE_NAPI_STATIC_FUNCTION("createDeviceManager", CreateDeviceManager),
    };

    DMLOG(DM_LOG_DEBUG, "DeviceManagerNapi::Init() is called!");
    NAPI_CALL(env,
        napi_define_class(env, DEVICE_MANAGER_NAPI_CLASS_NAME.c_str(), NAPI_AUTO_LENGTH, Constructor, nullptr,
            sizeof(dmProperties) / sizeof(dmProperties[0]), dmProperties, &dmClass));
    NAPI_CALL(env, napi_create_reference(env, dmClass, 1, &sConstructor_));
    NAPI_CALL(env, napi_set_named_property(env, exports, DEVICE_MANAGER_NAPI_CLASS_NAME.c_str(), dmClass));
    NAPI_CALL(env, napi_define_properties(env, exports, sizeof(static_prop) / sizeof(static_prop[0]), static_prop));
    DMLOG(DM_LOG_INFO, "All props and functions are configured..");
    return exports;
}

/*
 * Function registering all props and functions of ohos.distributedhardware
 */
static napi_value Export(napi_env env, napi_value exports)
{
    DMLOG(DM_LOG_INFO, "Export() is called!");
    DeviceManagerNapi::Init(env, exports);
    return exports;
}

/*
 * module define
 */
static napi_module g_dmModule = {
    .nm_version = 1,
    .nm_flags = 0,
    .nm_filename = nullptr,
    .nm_register_func = Export,
    .nm_modname = "distributedhardware.devicemanager",
    .nm_priv = ((void *)0),
    .reserved = {0}
    };

/*
 * module register
 */
extern "C" __attribute__((constructor)) void RegisterModule(void)
{
    DMLOG(DM_LOG_INFO, "RegisterModule() is called!");
    napi_module_register(&g_dmModule);
}
