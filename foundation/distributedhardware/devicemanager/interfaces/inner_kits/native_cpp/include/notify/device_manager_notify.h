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

#ifndef OHOS_DEVICE_MANAGER_NOTIFY_H
#define OHOS_DEVICE_MANAGER_NOTIFY_H

#include <mutex>
#include <map>
#include <vector>
#include <string>

#include "dm_device_info.h"
#include "dm_subscribe_info.h"
#include "device_manager_callback.h"
#include "single_instance.h"

namespace OHOS {
namespace DistributedHardware {
class DeviceManagerNotify {
DECLARE_SINGLE_INSTANCE(DeviceManagerNotify);
public:
    void RegisterDeathRecipientCallback(std::string &pkgName, std::shared_ptr<DmInitCallback> dmInitCallback);
    void UnRegisterDeathRecipientCallback(std::string &pkgName);
    void RegisterDeviceStateCallback(std::string &pkgName, std::shared_ptr<DeviceStateCallback> callback);
    void UnRegisterDeviceStateCallback(std::string &pkgName);
    void RegisterDiscoverCallback(std::string &pkgName, uint16_t subscribeId,
        std::shared_ptr<DiscoverCallback> callback);
    void UnRegisterDiscoverCallback(std::string &pkgName, uint16_t subscribeId);
    void RegisterAuthenticateCallback(std::string &pkgName, std::string &deviceId,
        std::shared_ptr<AuthenticateCallback> callback);
    void UnRegisterAuthenticateCallback(std::string &pkgName, std::string &deviceId);
    void UnRegisterPackageCallback(std::string &pkgName);
    void RegisterCheckAuthenticationCallback(std::string &pkgName, std::string &authPara,
        std::shared_ptr<CheckAuthCallback> callback);
    void UnRegisterCheckAuthenticationCallback(std::string &pkgName);
    void RegisterDeviceManagerFaCallback(std::string &packageName,
    std::shared_ptr<DeviceManagerFaCallback> callback);
    void UnRegisterDeviceManagerFaCallback(std::string &pkgName);
public:
    void OnRemoteDied();
    void OnDeviceOnline(std::string &pkgName, const DmDeviceInfo &deviceInfo);
    void OnDeviceOffline(std::string &pkgName, const DmDeviceInfo &deviceInfo);
    void OnDeviceChanged(std::string &pkgName, const DmDeviceInfo &deviceInfo);
    void OnDeviceFound(std::string &pkgName, uint16_t subscribeId, const DmDeviceInfo &deviceInfo);
    void OnDiscoverFailed(std::string &pkgName, uint16_t subscribeId, int32_t failedReason);
    void OnDiscoverySuccess(std::string &pkgName, uint16_t subscribeId);
    void OnAuthResult(std::string &pkgName, std::string &deviceId, int32_t pinToken, uint32_t status,
        uint32_t reason);
    void OnCheckAuthResult(std::string &pkgName, std::string &deviceId, int32_t resultCode, int32_t flag);
    void OnFaCall(std::string &pkgName, std::string &paramJson);
private:
    std::mutex lock_;
    std::map<std::string, std::shared_ptr<DeviceStateCallback>> deviceStateCallback_;
    std::map<std::string, std::map<uint16_t, std::shared_ptr<DiscoverCallback>>> deviceDiscoverCallbacks_;
    std::map<std::string, std::map<std::string, std::shared_ptr<AuthenticateCallback>>> authenticateCallback_;
    std::map<std::string, std::shared_ptr<CheckAuthCallback>> checkauthcallback_;
    std::map<std::string, std::shared_ptr<DmInitCallback>> dmInitCallback_;
    std::map<std::string, std::shared_ptr<DeviceManagerFaCallback>> dmFaCallback_;
};
} // namespace DistributedHardware
} // namespace OHOS
#endif // OHOS_DEVICE_MANAGER_NOTIFY_H
