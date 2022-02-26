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

#ifndef OHOS_DEVICE_MANAGER_NATIVE_DEVICEMANAGER_JS_H
#define OHOS_DEVICE_MANAGER_NATIVE_DEVICEMANAGER_JS_H

#include <memory>
#include <string>
#include "napi/native_api.h"
#include "napi/native_node_api.h"
#include "device_manager_callback.h"
#include "dm_native_event.h"
#include "dm_device_info.h"
#include "dm_subscribe_info.h"
#include "nlohmann/json.hpp"
#include "dm_device_info.h"

#define DM_NAPI_BUF_LENGTH (256)

struct AsyncCallbackInfo {
    napi_env env = nullptr;
    napi_async_work asyncWork = nullptr;

    char bundleName[DM_NAPI_BUF_LENGTH] = {0};
    size_t bundleNameLen = 0;

    napi_ref callback = nullptr;
    int32_t status = -1;
};

struct AuthAsyncCallbackInfo {
    napi_env env = nullptr;

    char bundleName[DM_NAPI_BUF_LENGTH] = {0};
    size_t bundleNameLen = 0;

    napi_ref callback = nullptr;
    int32_t authType = -1;
};

enum DmNapiDevStateChangeAction {
    ONLINE = 0,
    READY = 1,
    OFFLINE = 2,
    CHANGE = 3
};

class DmNapiInitCallback : public OHOS::DistributedHardware::DmInitCallback {
public:
    explicit DmNapiInitCallback(std::string &bundleName) : bundleName_(bundleName) {}
    virtual ~DmNapiInitCallback() {}
    void OnRemoteDied() override;

private:
    std::string bundleName_;
};

class DmNapiDeviceStateCallback : public OHOS::DistributedHardware::DeviceStateCallback {
public:
    explicit DmNapiDeviceStateCallback(std::string &bundleName) : bundleName_(bundleName) {}
    virtual ~DmNapiDeviceStateCallback() {};
    void OnDeviceOnline(const OHOS::DistributedHardware::DmDeviceInfo &deviceInfo) override;
    void OnDeviceReady(const OHOS::DistributedHardware::DmDeviceInfo &deviceInfo) override;
    void OnDeviceOffline(const OHOS::DistributedHardware::DmDeviceInfo &deviceInfo) override;
    void OnDeviceChanged(const OHOS::DistributedHardware::DmDeviceInfo &deviceInfo) override;

private:
    std::string bundleName_;
};

class DmNapiDiscoverCallback : public OHOS::DistributedHardware::DiscoverCallback {
public:
    explicit DmNapiDiscoverCallback(std::string &bundleName) : refCount_(0), bundleName_(bundleName) {}
    virtual ~DmNapiDiscoverCallback() {};
    void OnDeviceFound(uint16_t subscribeId, const OHOS::DistributedHardware::DmDeviceInfo &deviceInfo) override;
    void OnDiscoverFailed(uint16_t subscribeId, int32_t failedReason) override;
    void OnDiscoverySuccess(uint16_t subscribeId) override;
    void IncreaseRefCount();
    void DecreaseRefCount();
    int32_t GetRefCount();

private:
    std::atomic<int32_t> refCount_;
    std::string bundleName_;
};

class DmNapiAuthenticateCallback : public OHOS::DistributedHardware::AuthenticateCallback {
public:
    explicit DmNapiAuthenticateCallback(std::string &bundleName) : bundleName_(bundleName) {}
    virtual ~DmNapiAuthenticateCallback() {};
    void OnAuthResult(std::string &deviceId, int32_t pinToken, int32_t status, int32_t reason) override;

private:
    std::string bundleName_;
};

class DmNapiCheckAuthCallback : public OHOS::DistributedHardware::CheckAuthCallback {
public:
    explicit DmNapiCheckAuthCallback(std::string &bundleName) : bundleName_(bundleName) {}
    virtual ~DmNapiCheckAuthCallback() {};
    void OnCheckAuthResult(std::string &deviceId, int32_t resultCode, int32_t flag) override;

private:
    std::string bundleName_;
};

class DmNapiDeviceManagerFaCallback : public OHOS::DistributedHardware::DeviceManagerFaCallback {
public:
    explicit DmNapiDeviceManagerFaCallback(std::string &bundleName) : bundleName_(bundleName) {}
    virtual ~DmNapiDeviceManagerFaCallback() {};
    void OnCall(std::string &paramJson) override;

private:
    std::string bundleName_;
};

class DeviceManagerNapi : public DmNativeEvent {
public:
    explicit DeviceManagerNapi(napi_env env, napi_value thisVar);
    virtual ~DeviceManagerNapi();
    static napi_value Init(napi_env env, napi_value exports);
    static napi_value Constructor(napi_env env, napi_callback_info info);
    static napi_value CreateDeviceManager(napi_env env, napi_callback_info info);
    static napi_value ReleaseDeviceManager(napi_env env, napi_callback_info info);
    static napi_value GetTrustedDeviceListSync(napi_env env, napi_callback_info info);
    static napi_value StartDeviceDiscoverSync(napi_env env, napi_callback_info info);
    static napi_value StopDeviceDiscoverSync(napi_env env, napi_callback_info info);
    static napi_value AuthenticateDevice(napi_env env, napi_callback_info info);
    static napi_value VerifyAuthInfo(napi_env env, napi_callback_info info);
    static napi_value JsOn(napi_env env, napi_callback_info info);
    static napi_value JsOff(napi_env env, napi_callback_info info);
    static napi_value SetUserOperationSync(napi_env env, napi_callback_info info);
    static napi_value GetAuthenticationParamSync(napi_env env, napi_callback_info info);
    static void HandleCreateDmCallBack(const napi_env &env, AsyncCallbackInfo *asCallbackInfo);
    static DeviceManagerNapi *GetDeviceManagerNapi(std::string &buldleName);
    static void CreateDmCallback(std::string &bundleName, std::string &eventType);
    static void ReleaseDmCallback(std::string &bundleName, std::string &eventType);
    static void DeviceInfoToJsArray(const napi_env &env,
        const std::vector<OHOS::DistributedHardware::DmDeviceInfo>& vecDevInfo,
        const int32_t idx, napi_value &arrayResult);
    static void DmAuthParamToJsAuthParam(const napi_env &env,
        const OHOS::DistributedHardware::DmAuthParam &authParam, napi_value &paramResult);
    static void SetValueInt32(const napi_env &env, const std::string &fieldStr, const int32_t intValue,
        napi_value &result);
    static void SetValueUtf8String(const napi_env &env, const std::string &fieldStr, const std::string &str,
        napi_value &result);
    static void JsObjectToString(const napi_env &env, const napi_value &object,
        const std::string &fieldStr, char *dest, const int32_t destLen);
    static void JsObjectToBool(const napi_env &env, const napi_value &object,
        const std::string &fieldStr, bool& fieldRef);
    static void JsObjectToInt(const napi_env &env, const napi_value &object, const std::string &fieldStr,
        int& fieldRef);
    static std::string JsObjectToString(const napi_env &env, const napi_value &object);
    static int32_t JsToDmSubscribeInfo(const napi_env &env, const napi_value &object,
        OHOS::DistributedHardware::DmSubscribeInfo& info);
    static void JsToDmDeviceInfo(const napi_env &env, const napi_value &object,
        OHOS::DistributedHardware::DmDeviceInfo& info);
    static void JsToDmAppImageInfoAndDmExtra(const napi_env &env, const napi_value &object,
        OHOS::DistributedHardware::DmAppImageInfo& appImageInfo, std::string &extra, int32_t &authType);
    static void JsToDmAuthInfo(const napi_env &env, const napi_value &object, std::string &extra);
    static void JsToDmBuffer(const napi_env &env, const napi_value &object, const std::string &fieldStr,
        uint8_t **bufferPtr, int32_t &bufferLen);
    static void JsToJsonObject(const napi_env &env, const napi_value &object, const std::string &fieldStr,
        nlohmann::json &jsonObj);
    static void JsToDmTokenInfo(const napi_env &env, const napi_value &object,
        const std::string &fieldStr, nlohmann::json &jsonObj);
    static void JsToDmAuthExtra(const napi_env &env, const napi_value &param, nlohmann::json &jsonObj);
    void OnDeviceStateChange(DmNapiDevStateChangeAction action,
        const OHOS::DistributedHardware::DmDeviceInfo &deviceInfo);
    void OnDeviceFound(uint16_t subscribeId, const OHOS::DistributedHardware::DmDeviceInfo &deviceInfo);
    void OnDiscoverFailed(uint16_t subscribeId, int32_t failedReason);
    void OnAuthResult(const std::string &deviceId, int32_t pinToken, int32_t status, int32_t reason);
    void OnVerifyResult(const std::string &deviceId, int32_t resultCode, int32_t flag);
    void OnDmfaCall(const std::string &paramJson);
    static void DmAuthParamToJsAuthParamy(const napi_env &env,
        const OHOS::DistributedHardware::DmAuthParam &authParam, napi_value &paramResult);

private:
    napi_env env_;
    napi_ref wrapper_;
    static napi_ref sConstructor_;
    std::string bundleName_;
    static AuthAsyncCallbackInfo authAsyncCallbackInfo_;
    static AuthAsyncCallbackInfo verifyAsyncCallbackInfo_;
};

#endif // OHOS_DEVICE_MANAGER_NATIVE_DEVICEMANAGER_JS_H
