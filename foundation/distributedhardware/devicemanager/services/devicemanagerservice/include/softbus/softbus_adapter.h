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

#ifndef OHOS_DEVICE_MANAGER_SOFTBUS_ADAPTER_H
#define OHOS_DEVICE_MANAGER_SOFTBUS_ADAPTER_H

#include <string>
#include <mutex>
#include <map>
#include <memory>
#include "softbus_bus_center.h"
#include "discovery_service.h"
#include "dm_device_info.h"
#include "dm_subscribe_info.h"

namespace OHOS {
namespace DistributedHardware {
class PublishServiceCallBack {
public:
    static void OnPublishSuccess(int32_t publishId);
    static void OnPublishFail(int32_t publishId, PublishFailReason reason);
};

class SoftbusAdapter {
public:
    static int32_t Init();
    static int32_t GetTrustDevices(const std::string &pkgName, NodeBasicInfo **info, int32_t *infoNum);
    static int32_t StartDiscovery(std::string &pkgName, SubscribeInfo *info);
    static int32_t StopDiscovery(std::string &pkgName, uint16_t subscribeId);
    static bool IsDeviceOnLine(std::string &deviceId);
    static int32_t GetConnectionIpAddr(std::string deviceId, std::string &ipAddr);
    static ConnectionAddr *GetConnectAddr(std::string deviceId);
public:
    static void OnSoftBusDeviceOnline(NodeBasicInfo *info);
    static void OnSoftbusDeviceOffline(NodeBasicInfo *info);
    static void OnSoftbusDeviceInfoChanged(NodeBasicInfoType type, NodeBasicInfo *info);
    static void OnSoftbusDeviceFound(const DeviceInfo *device);
    static void OnSoftbusDiscoverFailed(int32_t subscribeId, DiscoveryFailReason failReason);
    static void OnSoftbusDiscoverySuccess(int32_t subscribeId);
private:
    static bool GetsubscribeIdAdapter(std::string &pkgName, int16_t originId, int32_t &adapterId);
    static bool GetpkgNameBySubscribeId(int32_t adapterId, std::string &pkgName);
    static void SaveDiscoverDeviceInfo(const DeviceInfo *deviceInfo);
    static void RemoveDiscoverDeviceInfo(const std::string deviceId);
    static void NodeBasicInfoCopyToDmDevice(DmDeviceInfo &dmDeviceInfo, NodeBasicInfo &nodeBasicInfo);
    static void DeviceInfoCopyToDmDevice(DmDeviceInfo &dmDeviceInfo, const DeviceInfo &deviceInfo);
    static ConnectionAddr *GetConnectAddrByType(DeviceInfo *deviceInfo, ConnectionAddrType type);
private:
    struct SubscribeInfoAdapter {
        SubscribeInfo info;
        uint16_t subscribeIdOrigin;
        uint16_t subscribeIdPrefix;
    };
    static std::map<std::string, std::vector<std::shared_ptr<SubscribeInfoAdapter>>> subscribeInfos_;
    static std::map<std::string, std::shared_ptr<DeviceInfo>> discoverDeviceInfoMap_;
    static std::vector<std::shared_ptr<DeviceInfo>> discoverDeviceInfoVector_;
    static uint16_t subscribeIdPrefix;
    static std::mutex lock_;
    static INodeStateCb softbusNodeStateCb_;
    static IDiscoveryCallback softbusDiscoverCallback_;
    static IPublishCallback servicePublishCallback_;
};
} // namespace DistributedHardware
} // namespace OHOS
#endif // OHOS_DEVICE_MANAGER_SOFTBUS_ADAPTER_H
