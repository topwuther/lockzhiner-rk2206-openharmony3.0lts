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

#include "monitor_manager.h"
#include "distributed_permission_manager_service.h"

namespace OHOS {
namespace Security {
namespace Permission {
namespace {
static constexpr OHOS::HiviewDFX::HiLogLabel LABEL = {LOG_CORE, SECURITY_DOMAIN_PERMISSION, "MonitorManager"};
}
MonitorManager::MonitorManager()
    : permissionChangedListeners_(),
      permissionMutex_(),
      deviceOnlineListeners_(),
      deviceOfflineListeners_(),
      deviceMutex_()
{
    PERMISSION_LOG_INFO(LABEL, "MonitorManager()");
}

MonitorManager::~MonitorManager()
{
    PERMISSION_LOG_INFO(LABEL, "~MonitorManager()");
}

MonitorManager &MonitorManager::GetInstance()
{
    static MonitorManager instance;
    return instance;
}

void MonitorManager::AddOnPermissionChangedListener(
    const std::string &moduleName, const std::shared_ptr<OnPermissionChangedCallback> &listener)
{
    if (moduleName.empty() || listener == nullptr) {
        PERMISSION_LOG_INFO(LABEL, "AddOnPermissionChangedListener: params is invalid, please check.");
        return;
    }

    std::unique_lock<std::mutex> lock(permissionMutex_);
    permissionChangedListeners_.insert(
        std::pair<std::string, std::shared_ptr<OnPermissionChangedCallback>>(moduleName, listener));
}

void MonitorManager::RemoveOnPermissionChangedListener(const std::string &moduleName)
{
    if (moduleName.empty()) {
        PERMISSION_LOG_INFO(LABEL, "RemoveOnPermissionChangedListener: params is invalid, please check.");
        return;
    }

    std::unique_lock<std::mutex> lock(permissionMutex_);
    auto it = permissionChangedListeners_.find(moduleName);
    if (it != permissionChangedListeners_.end()) {
        permissionChangedListeners_.erase(it);
    }
}

void MonitorManager::OnPermissionChange(const int uid, const std::string &packageName)
{
    std::shared_ptr<DistributedPermissionEventHandler> handler =
        DelayedSingleton<DistributedPermissionManagerService>::GetInstance()->GetEventHandler();
    if (handler == nullptr) {
        PERMISSION_LOG_ERROR(LABEL, "fail to get EventHandler");
        return;
    }
    // notify all subscribers.
    for (auto it = permissionChangedListeners_.begin(); it != permissionChangedListeners_.end(); it++) {
        OnPermissionChangedCallback callback = *(it->second);
        PERMISSION_LOG_DEBUG(LABEL,
            "execute permission changed callback on name: %{public}s, function: %{public}lX",
            it->first.c_str(),
            (unsigned long)(it->second.get()));
        handler->ProxyPostTask(std::bind(callback, uid, packageName), "MonitorManager::OnPermissionChange");
    }
}

void MonitorManager::AddOnDeviceStatusChangedListener(const std::string &moduleName,
    const std::shared_ptr<OnDeviceOnlineCallback> &onlineCallback,
    const std::shared_ptr<OnDeviceOnlineCallback> &offlineCallback)
{
    if (moduleName.empty() || onlineCallback == nullptr || offlineCallback) {
        PERMISSION_LOG_INFO(LABEL, "addOnDeviceStatusChangedListener: params is invalid, please check.");
        return;
    }

    std::unique_lock<std::mutex> lock(deviceMutex_);
    deviceOnlineListeners_.insert(
        std::pair<std::string, std::shared_ptr<OnDeviceOnlineCallback>>(moduleName, onlineCallback));
    deviceOfflineListeners_.insert(
        std::pair<std::string, std::shared_ptr<OnDeviceOnlineCallback>>(moduleName, offlineCallback));
}

void MonitorManager::RemoveOnDeviceStatusChangedListener(const std::string &moduleName)
{
    if (moduleName.empty()) {
        PERMISSION_LOG_INFO(LABEL, "removeOnDeviceStatusChangedListener: moduleName must not be null");
        return;
    }

    std::unique_lock<std::mutex> lock(deviceMutex_);
    {
        auto it = deviceOnlineListeners_.find(moduleName);
        if (it != deviceOnlineListeners_.end()) {
            deviceOnlineListeners_.erase(it);
        }
    }
    {
        auto it = deviceOfflineListeners_.find(moduleName);
        if (it != deviceOfflineListeners_.end()) {
            deviceOfflineListeners_.erase(it);
        }
    }
}

void MonitorManager::OnDeviceOnline(const std::string &deviceId, const std::string &deviceNetworkId)
{
    if (!DistributedDataValidator::IsDeviceIdValid(deviceId) ||
        !DistributedDataValidator::IsDeviceIdValid(deviceNetworkId)) {
        PERMISSION_LOG_ERROR(LABEL,
            "onDeviceOnline: invalid deviceId: %{pbulic}s, deviceNetworkId:  %{pbulic}s",
            deviceId.c_str(),
            deviceNetworkId.c_str());
        return;
    }
    // notify all subscribers.
    for (auto it = deviceOnlineListeners_.begin(); it != deviceOnlineListeners_.end(); it++) {
        auto callback = *(it->second);
        callback(deviceId, deviceNetworkId);
    }
}

void MonitorManager::OnDeviceOffline(const std::string &deviceId, const std::string &deviceNetworkId)
{
    if (!DistributedDataValidator::IsDeviceIdValid(deviceId) ||
        !DistributedDataValidator::IsDeviceIdValid(deviceNetworkId)) {
        PERMISSION_LOG_ERROR(LABEL,
            "onDeviceOffline: invalid deviceId: %{pbulic}s, deviceNetworkId:  %{pbulic}s",
            deviceId.c_str(),
            deviceNetworkId.c_str());
        return;
    }
    // notify all subscribers.
    for (auto it = deviceOfflineListeners_.begin(); it != deviceOfflineListeners_.end(); it++) {
        auto callback = *(it->second);
        callback(deviceId, deviceNetworkId);
    }
}

}  // namespace Permission
}  // namespace Security
}  // namespace OHOS