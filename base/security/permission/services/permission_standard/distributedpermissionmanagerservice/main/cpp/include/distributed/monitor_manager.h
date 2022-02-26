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

#ifndef MONITOR_MANAGER_H
#define MONITOR_MANAGER_H

#include <string>
#include <map>
#include <memory>
#include <functional>
#include <mutex>

#include "permission_log.h"

namespace OHOS {
namespace Security {
namespace Permission {

class MonitorManager {

public:
    using OnPermissionChangedCallback = std::function<void(int uid, const std::string &packageName)>;
    using OnDeviceOnlineCallback = std::function<void(const std::string &deviceId, const std::string &deviceNetworkId)>;
    using OnDeviceOfflineCallback =
        std::function<void(const std::string &deviceId, const std::string &deviceNetworkId)>;

public:
    virtual ~MonitorManager();
    static MonitorManager &GetInstance();

    /**
     * @brief Add a listener for permission changes. Maybe caused by package changes, runtime permission granted or
     * denied, and appops changes.
     *
     * @param moduleName Name of the registrant, better class name.
     * @param callback callback for permission changed event.
     */
    void AddOnPermissionChangedListener(
        const std::string &moduleName, const std::shared_ptr<OnPermissionChangedCallback> &callback);

    /**
     * @brief Remove a listener for permission changes by module name.
     *
     * @param moduleName Name of the registrant, better class name.
     */
    void RemoveOnPermissionChangedListener(const std::string &moduleName);

    /**
     * @brief Notify all subscribers in {@link MonitorManager} that permissions have changed. Actually, it is
     * responsible for dispatch the callback message from BMS for package changes.
     *
     * @param uid The application uid with a change.
     * @param packageName The packageName with a change.
     */
    void OnPermissionChange(const int uid, const std::string &packageName);

    /**
     * Add a listener for device status changes.
     *
     * @param moduleName Name of the registrant, better class name.
     * @param onlineCallback callback for device online event.
     * @param offlineCallback callback for device offline event.
     */
    void AddOnDeviceStatusChangedListener(const std::string &moduleName,
        const std::shared_ptr<OnDeviceOnlineCallback> &onlineCallback,
        const std::shared_ptr<OnDeviceOnlineCallback> &offlineCallback);

    /**
     * Remove a listener for device status changes.
     *
     * @param moduleName Name of module has been registered before.
     */
    void RemoveOnDeviceStatusChangedListener(const std::string &moduleName);

    /**
     * Notify all subscribers in {@link MonitorManager} that device online.
     *
     * @param deviceId The deviceId which online.
     * @param deviceNetworkId The networkId.
     */
    void OnDeviceOnline(const std::string &deviceId, const std::string &deviceNetworkId);

    /**
     * Notify all subscribers in {@link MonitorManager} that device offline.
     *
     * @param deviceId The deviceId which offline.
     * @param deviceNetworkId The networkId.
     */
    void OnDeviceOffline(const std::string &deviceId, const std::string &deviceNetworkId);

private:
    MonitorManager();

    // permission changed callback map, kv is <moduleName, callback>.
    std::map<std::string, std::shared_ptr<OnPermissionChangedCallback>> permissionChangedListeners_;
    // mutex for permission listener map
    std::mutex permissionMutex_;

    // device online callback map, kv is <moduleName, callback>.
    std::map<std::string, std::shared_ptr<OnDeviceOnlineCallback>> deviceOnlineListeners_;
    // device offline callback map, kv is <moduleName, callback>.
    std::map<std::string, std::shared_ptr<OnDeviceOfflineCallback>> deviceOfflineListeners_;
    // mutex for device listener maps
    std::mutex deviceMutex_;
};

}  // namespace Permission
}  // namespace Security
}  // namespace OHOS
#endif  // MONITOR_MANAGER_H