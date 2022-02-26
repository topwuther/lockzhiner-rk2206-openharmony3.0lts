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

#ifndef DEVICE_MANAGER_H
#define DEVICE_MANAGER_H

#include <set>
#include <mutex>

#include "icommunicator.h"

namespace DistributedDB {
class DeviceManager final {
public:
    DeviceManager();
    ~DeviceManager();

    DISABLE_COPY_ASSIGN_MOVE(DeviceManager);

    static int RegisterTransformFunc();

    // Calculate the length of message.
    static uint32_t CalculateLen();

    // Initialize the DeviceManager.
    int Initialize(ICommunicator *communicator, const std::function<void(std::string)> &callback);

    // Set The Device online Callback.
    void RegDeviceOnLineCallBack(const std::function<void(std::string)> &callback);

    // Set The Device offline Callback.
    void RegDeviceOffLineCallBack(const std::function<void(std::string)> &callback);

    // The Device connect message callback, registered to the ICommunicator
    void OnDeviceConnectCallback(const std::string &targetDev, bool isConnect);

    // Get The online devices list.
    void GetOnlineDevices(std::vector<std::string> &devices) const;

    // Send a BroadCast to all online device.
    int SendBroadCast(uint32_t msgId);

    // Determine if the device is online.
    bool IsDeviceOnline(const std::string &deviceId) const;

private:

    // Send a local data changed broadcast.
    int SendLocalDataChanged();

    std::set<std::string> devices_;
    std::function<void(std::string)> onlineCallback_;
    std::function<void(std::string)> offlineCallback_;
    ICommunicator *communicator_;
    mutable std::mutex devicesLock_;
};
} // namespace DistributedDB

#endif // DEVICE_MANAGER_H
