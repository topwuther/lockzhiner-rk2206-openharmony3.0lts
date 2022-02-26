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

#include "device_manager_notify.h"

#include "device_manager_log.h"
#include "device_manager_errno.h"
#include "nlohmann/json.hpp"
#include "constants.h"
#include "device_manager.h"

namespace OHOS {
namespace DistributedHardware {
IMPLEMENT_SINGLE_INSTANCE(DeviceManagerNotify);

void DeviceManagerNotify::RegisterDeathRecipientCallback(std::string &pkgName,
    std::shared_ptr<DmInitCallback> dmInitCallback)
{
    std::lock_guard<std::mutex> autoLock(lock_);
    dmInitCallback_[pkgName] = dmInitCallback;
}

void DeviceManagerNotify::UnRegisterDeathRecipientCallback(std::string &pkgName)
{
    std::lock_guard<std::mutex> autoLock(lock_);
    dmInitCallback_.erase(pkgName);
}

void DeviceManagerNotify::RegisterDeviceStateCallback(std::string &pkgName,
    std::shared_ptr<DeviceStateCallback> callback)
{
    std::lock_guard<std::mutex> autoLock(lock_);
    deviceStateCallback_[pkgName] = callback;
}

void DeviceManagerNotify::UnRegisterDeviceStateCallback(std::string &pkgName)
{
    std::lock_guard<std::mutex> autoLock(lock_);
    deviceStateCallback_.erase(pkgName);
}

void DeviceManagerNotify::RegisterDiscoverCallback(std::string &pkgName, uint16_t subscribeId,
    std::shared_ptr<DiscoverCallback> callback)
{
    std::lock_guard<std::mutex> autoLock(lock_);
    if (deviceDiscoverCallbacks_.count(pkgName) == 0) {
        deviceDiscoverCallbacks_[pkgName] = std::map<uint16_t, std::shared_ptr<DiscoverCallback>>();
    }
    deviceDiscoverCallbacks_[pkgName][subscribeId] = callback;
}

void DeviceManagerNotify::UnRegisterDiscoverCallback(std::string &pkgName, uint16_t subscribeId)
{
    std::lock_guard<std::mutex> autoLock(lock_);
    if (deviceDiscoverCallbacks_.count(pkgName) > 0) {
        deviceDiscoverCallbacks_[pkgName].erase(subscribeId);
        if (deviceDiscoverCallbacks_[pkgName].empty()) {
            deviceDiscoverCallbacks_.erase(pkgName);
        }
    }
}

void DeviceManagerNotify::RegisterAuthenticateCallback(std::string &pkgName, std::string &deviceId,
    std::shared_ptr<AuthenticateCallback> callback)
{
    std::lock_guard<std::mutex> autoLock(lock_);
    if (authenticateCallback_.count(pkgName) == 0) {
        authenticateCallback_[pkgName] = std::map<std::string, std::shared_ptr<AuthenticateCallback>>();
    }
    authenticateCallback_[pkgName][deviceId] = callback;
}

void DeviceManagerNotify::UnRegisterAuthenticateCallback(std::string &pkgName, std::string &deviceId)
{
    std::lock_guard<std::mutex> autoLock(lock_);
    if (authenticateCallback_.count(pkgName) > 0) {
        authenticateCallback_[pkgName].erase(deviceId);
        if (authenticateCallback_[pkgName].empty()) {
            authenticateCallback_.erase(pkgName);
        }
    }
}

void DeviceManagerNotify::UnRegisterPackageCallback(std::string &pkgName)
{
    std::lock_guard<std::mutex> autoLock(lock_);
    deviceStateCallback_.erase(pkgName);
    deviceDiscoverCallbacks_.erase(pkgName);
    authenticateCallback_.erase(pkgName);
    dmInitCallback_.erase(pkgName);
}

void DeviceManagerNotify::RegisterCheckAuthenticationCallback(std::string &pkgName, std::string &authPara,
    std::shared_ptr<CheckAuthCallback> callback)
{
    std::lock_guard<std::mutex> autoLock(lock_);
    checkauthcallback_[pkgName] = callback;
}

void DeviceManagerNotify::UnRegisterCheckAuthenticationCallback(std::string &pkgName)
{
    std::lock_guard<std::mutex> autoLock(lock_);
    checkauthcallback_.erase(pkgName);
}

void DeviceManagerNotify::RegisterDeviceManagerFaCallback(std::string &packageName,
    std::shared_ptr<DeviceManagerFaCallback> callback)
{
    std::lock_guard<std::mutex> autoLock(lock_);
    dmFaCallback_[packageName] = callback;
}

void DeviceManagerNotify::UnRegisterDeviceManagerFaCallback(std::string &pkgName)
{
    std::lock_guard<std::mutex> autoLock(lock_);
    dmFaCallback_.erase(pkgName);
}


void DeviceManagerNotify::OnRemoteDied()
{
    DMLOG(DM_LOG_WARN, "DeviceManager : OnRemoteDied");
    for (auto iter : dmInitCallback_) {
        iter.second->OnRemoteDied();
    }
}

void DeviceManagerNotify::OnDeviceOnline(std::string &pkgName, const DmDeviceInfo &deviceInfo)
{
    DMLOG(DM_LOG_INFO, "DeviceManager OnDeviceOnline pkgName:%s", pkgName.c_str());
    std::lock_guard<std::mutex> autoLock(lock_);
    if (deviceStateCallback_.count(pkgName) == 0) {
        DMLOG(DM_LOG_ERROR, "DeviceManager OnDeviceOnlinecallback not register");
        return;
    }
    deviceStateCallback_[pkgName]->OnDeviceOnline(deviceInfo);
}

void DeviceManagerNotify::OnDeviceOffline(std::string &pkgName, const DmDeviceInfo &deviceInfo)
{
    DMLOG(DM_LOG_INFO, "DeviceManager OnDeviceOffline pkgName:%s", pkgName.c_str());
    std::lock_guard<std::mutex> autoLock(lock_);
    if (deviceStateCallback_.count(pkgName) == 0) {
        DMLOG(DM_LOG_ERROR, "DeviceManager OnDeviceOfflinecallback not register");
        return;
    }
    deviceStateCallback_[pkgName]->OnDeviceOffline(deviceInfo);
}

void DeviceManagerNotify::OnDeviceChanged(std::string &pkgName, const DmDeviceInfo &deviceInfo)
{
    DMLOG(DM_LOG_INFO, "DeviceManager OnDeviceChanged pkgName:%s", pkgName.c_str());
    std::lock_guard<std::mutex> autoLock(lock_);
    if (deviceStateCallback_.count(pkgName) == 0) {
        DMLOG(DM_LOG_ERROR, "DeviceManager OnDeviceChangedcallback not register");
        return;
    }
    deviceStateCallback_[pkgName]->OnDeviceChanged(deviceInfo);
}

void DeviceManagerNotify::OnDeviceFound(std::string &pkgName, uint16_t subscribeId,
    const DmDeviceInfo &deviceInfo)
{
    DMLOG(DM_LOG_INFO, "DeviceManager OnDeviceFound pkgName:%s, subscribeId:%d.", pkgName.c_str(),
        (int32_t)subscribeId);
    std::lock_guard<std::mutex> autoLock(lock_);
    if (deviceDiscoverCallbacks_.count(pkgName) == 0) {
        DMLOG(DM_LOG_ERROR, "DeviceManager OnDeviceFound: no register discoverCallback for this package");
        return;
    }
    std::map<uint16_t, std::shared_ptr<DiscoverCallback>> &discoverCallMap = deviceDiscoverCallbacks_[pkgName];
    auto iter = discoverCallMap.find(subscribeId);
    if (iter == discoverCallMap.end()) {
        DMLOG(DM_LOG_ERROR, "DeviceManager OnDeviceFound: no register discoverCallback for subscribeId %d",
            subscribeId);
        return;
    }
    iter->second->OnDeviceFound(subscribeId, deviceInfo);
}

void DeviceManagerNotify::OnDiscoverFailed(std::string &pkgName, uint16_t subscribeId, int32_t failedReason)
{
    DMLOG(DM_LOG_INFO, "DeviceManager OnDiscoverFailed pkgName:%s, subscribeId %d, reason %d",
        pkgName.c_str(), subscribeId, failedReason);
    std::lock_guard<std::mutex> autoLock(lock_);
    if (deviceDiscoverCallbacks_.count(pkgName) == 0) {
        DMLOG(DM_LOG_ERROR, "DeviceManager OnDiscoverFailed: no register discoverCallback for this package");
        return;
    }
    std::map<uint16_t, std::shared_ptr<DiscoverCallback>> &discoverCallMap = deviceDiscoverCallbacks_[pkgName];
    auto iter = discoverCallMap.find(subscribeId);
    if (iter == discoverCallMap.end()) {
        DMLOG(DM_LOG_ERROR, "DeviceManager OnDiscoverFailed: no register discoverCallback for subscribeId %d",
            subscribeId);
        return;
    }
    iter->second->OnDiscoverFailed(subscribeId, failedReason);
}

void DeviceManagerNotify::OnDiscoverySuccess(std::string &pkgName, uint16_t subscribeId)
{
    DMLOG(DM_LOG_INFO, "DeviceManager OnDiscoverySuccess pkgName:%s, subscribeId:%d.", pkgName.c_str(),
        subscribeId);
    std::lock_guard<std::mutex> autoLock(lock_);
    if (deviceDiscoverCallbacks_.count(pkgName) == 0) {
        DMLOG(DM_LOG_ERROR, "DeviceManager OnDiscoverySuccess: no register discoverCallback for this package");
        return;
    }
    std::map<uint16_t, std::shared_ptr<DiscoverCallback>> &discoverCallMap = deviceDiscoverCallbacks_[pkgName];
    auto iter = discoverCallMap.find(subscribeId);
    if (iter == discoverCallMap.end()) {
        DMLOG(DM_LOG_ERROR, "DeviceManager OnDiscoverySuccess: no register discoverCallback for subscribeId %d",
            subscribeId);
        return;
    }
    iter->second->OnDiscoverySuccess(subscribeId);
}

void DeviceManagerNotify::OnAuthResult(std::string &pkgName, std::string &deviceId, int32_t pinToken,
    uint32_t status, uint32_t reason)
{
    DMLOG(DM_LOG_INFO, "DeviceManagerNotify::OnAuthResult pkgName:%s, status:%d, reason:%d",
        pkgName.c_str(), status, reason);
    std::lock_guard<std::mutex> autoLock(lock_);
    if (authenticateCallback_.count(pkgName) == 0) {
        DMLOG(DM_LOG_ERROR, "DeviceManager OnAuthResult: no register authCallback for this package");
        return;
    }
    std::map<std::string, std::shared_ptr<AuthenticateCallback>> &authCallMap = authenticateCallback_[pkgName];
    auto iter = authCallMap.find(deviceId);
    if (iter == authCallMap.end()) {
        DMLOG(DM_LOG_ERROR, "DeviceManager OnAuthResult: no register authCallback for deviceID ");
        return;
    }
    iter->second->OnAuthResult(deviceId, pinToken, status, reason);
    authenticateCallback_[pkgName].erase(deviceId);
    if (authenticateCallback_[pkgName].empty()) {
        authenticateCallback_.erase(pkgName);
    }
}

void DeviceManagerNotify::OnCheckAuthResult(std::string &pkgName, std::string &deviceId, int32_t resultCode,
    int32_t flag)
{
    DMLOG(DM_LOG_INFO, "DeviceManagerNotify::OnCheckAuthResult pkgName:%s, resultCode:%d, flag:%d",
        pkgName.c_str(), resultCode, flag);
    std::lock_guard<std::mutex> autoLock(lock_);
    if (checkauthcallback_.count(pkgName) == 0) {
        DMLOG(DM_LOG_ERROR, "DeviceManager OnCheckAuthResult: no register authCallback for this package");
        return;
    }

    checkauthcallback_[pkgName]->OnCheckAuthResult(deviceId, resultCode, flag);
    checkauthcallback_.erase(pkgName);
}

void DeviceManagerNotify::OnFaCall(std::string &pkgName, std::string &paramJson)
{
    DMLOG(DM_LOG_INFO, "DeviceManager OnFaCallback pkgName:%s", pkgName.c_str());
    std::lock_guard<std::mutex> autoLock(lock_);
    if (dmFaCallback_.count(pkgName) == 0) {
        DMLOG(DM_LOG_ERROR, "DeviceManager DmFaCallback not register");
        return;
    }
    dmFaCallback_[pkgName]->OnCall(paramJson);
}
} // namespace DistributedHardware
} // namespace OHOS
