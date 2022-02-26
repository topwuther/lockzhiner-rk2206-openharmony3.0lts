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

#ifndef IPROCESSCOMMUNICATOR_H
#define IPROCESSCOMMUNICATOR_H

#include <string>
#include <vector>
#include <cstdint>
#include <functional>
#include "types.h"

namespace DistributedDB {
// The DeviceInfos may contain other fields(Can only be auxiliary information) besides identifier field in the future.
struct DeviceInfos {
    std::string identifier; // An unique and fixed identifier representing a device, such as UUID.
};

// In OnDeviceChange, all field of devInfo should be valid, isOnline true for online and false for offline.
// The concept of online or offline:
// 1: Can be at the physical device level, which means the remote device can be visible and communicable by local device
// 2: Can also be at the process level, which means the same ProcessCommunicator(with same processLabel) had been
//    started on the remote device and thus visible and communicable by this local ProcessCommunicator.
using OnDeviceChange = std::function<void(const DeviceInfos &devInfo, bool isOnline)>;

// In OnDataReceive, all field of srcDevInfo should be valid
using OnDataReceive = std::function<void(const DeviceInfos &srcDevInfo, const uint8_t *data, uint32_t length)>;

// For all functions with returnType DBStatus:
// return DBStatus::OK if successful, otherwise DBStatus::DB_ERROR if anything wrong.
// Additional information of reason why failed can be present in the log by the implementation.
// For "Get" or "Is" functions, implementation should notice that concurrent call is possible.
class IProcessCommunicator {
public:
    // The distributeddb in one process can only use one ProcessCommunicator at the same time
    // The ProcessCommunicator can only Start one processLabel at the same time
    // The ProcessCommunicator can Start again after stop
    // The processLabel should not be an empty string
    virtual DBStatus Start(const std::string &processLabel) = 0;

    // The Stop should only be called after Start successfully
    virtual DBStatus Stop() = 0;

    // The register function can be called anytime regardless of whether started or stopped.
    // There will only be one callback at the same time for each function
    // If register again, the latter callback replace the former callback.
    // Register nullptr as callback to do unregister semantic.
    // For concurrency security of implementation, there should be lock between register_operation and callback_event.
    virtual DBStatus RegOnDeviceChange(const OnDeviceChange &callback) = 0;
    virtual DBStatus RegOnDataReceive(const OnDataReceive &callback) = 0;

    // The SendData function should only be called after Start successfully
    // Only the identifier field of dstDevInfo must be valid, no requirement for other field.
    virtual DBStatus SendData(const DeviceInfos &dstDevInfo, const uint8_t *data, uint32_t length) = 0;

    // The GetMtuSize function can be called anytime regardless of whether started or stopped.
    // The mtuSize should not less than 1K otherwise it will be regard as 1K.
    // For run on OHOS, there is agreement that the mtuSize should be nearly 5M.
    virtual uint32_t GetMtuSize() = 0;

    // The GetLocalDeviceInfos function should only be called after Start successfully
    // All field of returned DeviceInfos must be valid, the identifier must not be empty and changed between time.
    virtual DeviceInfos GetLocalDeviceInfos() = 0;

    // The GetRemoteOnlineDeviceInfosList function should only be called after Start successfully
    // All field of returned DeviceInfos must be valid, should not contain duplicate device or local device
    virtual std::vector<DeviceInfos> GetRemoteOnlineDeviceInfosList() = 0;

    // The IsSameProcessLabelStartedOnPeerDevice function should only be called after Start successfully
    // Only the identifier field of peerDevInfo must be valid, no requirement for other field.
    // If the peer device is offline, then return false.
    // If the peer device is online but no ProcessCommunicator with same processLabel had started on it, return false.
    // If the peer device is online and ProcessCommunicator with same processLabel had started on it, return true.
    virtual bool IsSameProcessLabelStartedOnPeerDevice(const DeviceInfos &peerDevInfo) = 0;

    virtual ~IProcessCommunicator() {};

    // For ABI compatibility reason, temporarily place this method at last and offer a fake implementation.
    // The valid mtuSize range from 1K to 5M, value beyond this range will be set to the upper or lower limit.
    virtual uint32_t GetMtuSize(const DeviceInfos &devInfo)
    {
        if (devInfo.identifier.empty()) {
            // Error case(would never happen actually) to avoid "unused-parameter" warning.
            return 0;
        }
        return GetMtuSize();
    }
};
} // namespace DistributedDB

#endif // IPROCESSCOMMUNICATOR_H
