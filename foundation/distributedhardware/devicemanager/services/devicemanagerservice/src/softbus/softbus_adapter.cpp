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

#include "softbus_adapter.h"

#include <cstdlib>
#include <string>
#include <unistd.h>
#include <map>
#include <vector>
#include <memory>

#include <securec.h>

#include "softbus_bus_center.h"

#include "dm_device_info.h"
#include "dm_subscribe_info.h"

#include "anonymous_string.h"
#include "device_manager_errno.h"
#include "device_manager_log.h"
#include "softbus_session.h"
#include "system_ability_definition.h"

#include "ipc_server_listener_adapter.h"

namespace OHOS {
namespace DistributedHardware {
namespace {
const std::string DEVICE_MANAGER_PACKAGE_NAME = "ohos.distributedhardware.devicemanager";
const int32_t CHECK_INTERVAL = 100000; // 100ms
const uint32_t SUBSCRIBE_ID_PREFIX_LEN = 16;
const int32_t SUBSCRIBE_ID_MASK = 0x0000FFFF;
const int32_t DISCOVER_DEVICEINFO_MAX_SIZE = 20;
}
std::map<std::string, std::vector<std::shared_ptr<SoftbusAdapter::SubscribeInfoAdapter>>>
    SoftbusAdapter::subscribeInfos_;
std::map<std::string, std::shared_ptr<DeviceInfo>> SoftbusAdapter::discoverDeviceInfoMap_;
std::vector<std::shared_ptr<DeviceInfo>> SoftbusAdapter::discoverDeviceInfoVector_;
uint16_t SoftbusAdapter::subscribeIdPrefix = 0;
std::mutex SoftbusAdapter::lock_;
INodeStateCb SoftbusAdapter::softbusNodeStateCb_ = {
    .events = EVENT_NODE_STATE_ONLINE | EVENT_NODE_STATE_OFFLINE | EVENT_NODE_STATE_INFO_CHANGED,
    .onNodeOnline = OnSoftBusDeviceOnline,
    .onNodeOffline = OnSoftbusDeviceOffline,
    .onNodeBasicInfoChanged = OnSoftbusDeviceInfoChanged
};
IDiscoveryCallback SoftbusAdapter::softbusDiscoverCallback_ = {
    .OnDeviceFound = OnSoftbusDeviceFound,
    .OnDiscoverFailed = OnSoftbusDiscoverFailed,
    .OnDiscoverySuccess = OnSoftbusDiscoverySuccess
};
IPublishCallback SoftbusAdapter::servicePublishCallback_ = {
    .OnPublishSuccess = PublishServiceCallBack::OnPublishSuccess,
    .OnPublishFail    = PublishServiceCallBack::OnPublishFail
};

void SoftbusAdapter::RemoveDiscoverDeviceInfo(const std::string deviceId)
{
    discoverDeviceInfoMap_.erase(deviceId);
    auto iter = discoverDeviceInfoVector_.begin();
    while (iter != discoverDeviceInfoVector_.end()) {
        if (strcmp(iter->get()->devId, deviceId.c_str()) == 0) {
            iter = discoverDeviceInfoVector_.erase(iter);
        } else {
            ++iter;
        }
    }
}

void SoftbusAdapter::OnSoftBusDeviceOnline(NodeBasicInfo *info)
{
    if (info == nullptr) {
        DMLOG(DM_LOG_ERROR, "OnSoftBusDeviceOnline NodeBasicInfo is nullptr");
        return;
    }
    DmDeviceInfo dmDeviceInfo;

    NodeBasicInfoCopyToDmDevice(dmDeviceInfo, *info);
    IpcServerListenerAdapter::GetInstance().OnDeviceStateChange(DmDeviceState::DEVICE_STATE_ONLINE, dmDeviceInfo);

    uint8_t udid[UDID_BUF_LEN] = {0};
    int32_t ret = GetNodeKeyInfo(DEVICE_MANAGER_PACKAGE_NAME.c_str(), info->networkId,
        NodeDeivceInfoKey::NODE_KEY_UDID, udid, sizeof(udid));
    if (ret != DEVICEMANAGER_OK) {
        DMLOG(DM_LOG_ERROR, "GetNodeKeyInfo failed");
        return;
    }
    std::string deviceId = (char *)udid;
    DMLOG(DM_LOG_INFO, "device online, deviceId: %s", GetAnonyString(deviceId).c_str());
    RemoveDiscoverDeviceInfo(deviceId);
}

void SoftbusAdapter::OnSoftbusDeviceOffline(NodeBasicInfo *info)
{
    if (info == nullptr) {
        DMLOG(DM_LOG_ERROR, "OnSoftbusDeviceOffline NodeBasicInfo is nullptr");
        return;
    }
    DmDeviceInfo dmDeviceInfo;

    NodeBasicInfoCopyToDmDevice(dmDeviceInfo, *info);
    IpcServerListenerAdapter::GetInstance().OnDeviceStateChange(DmDeviceState::DEVICE_STATE_OFFLINE, dmDeviceInfo);
}

void SoftbusAdapter::OnSoftbusDeviceInfoChanged(NodeBasicInfoType type, NodeBasicInfo *info)
{
    DMLOG(DM_LOG_INFO, "OnSoftbusDeviceInfoChanged.");
    // currently do nothing
    (void)type;
    (void)info;
}

void SoftbusAdapter::SaveDiscoverDeviceInfo(const DeviceInfo *deviceInfo)
{
    std::shared_ptr<DeviceInfo> info = std::make_shared<DeviceInfo>();
    DeviceInfo *infoPtr = info.get();
    if (memcpy_s(infoPtr, sizeof(DeviceInfo), deviceInfo, sizeof(DeviceInfo)) != 0) {
        DMLOG(DM_LOG_ERROR, "SoftbusAdapter::SaveDiscoverDeviceInfo failed.");
        return;
    }

    std::string deviceId = deviceInfo->devId;
    discoverDeviceInfoMap_[deviceId] = info;
    discoverDeviceInfoVector_.push_back(info);

    // Remove the earliest element when reached the max size
    if (discoverDeviceInfoVector_.size() == DISCOVER_DEVICEINFO_MAX_SIZE) {
        auto iter = discoverDeviceInfoVector_.begin();
        std::string delDevId = iter->get()->devId;
        discoverDeviceInfoMap_.erase(delDevId);
        discoverDeviceInfoVector_.erase(iter);
    }
}

void SoftbusAdapter::OnSoftbusDeviceFound(const DeviceInfo *device)
{
    if (device == nullptr) {
        DMLOG(DM_LOG_ERROR, "deviceinfo is null");
        return;
    }

    std::string deviceId = device->devId;
    DMLOG(DM_LOG_INFO, "SoftbusAdapter::OnSoftbusDeviceFound device %s found.", GetAnonyString(deviceId).c_str());
    if (IsDeviceOnLine(deviceId)) {
        return;
    }

    SaveDiscoverDeviceInfo(device);
    for (auto iter = subscribeInfos_.begin(); iter != subscribeInfos_.end(); ++iter) {
        auto subInfovector = iter->second;
        for (auto vectorIter = subInfovector.begin(); vectorIter != subInfovector.end(); ++vectorIter) {
            auto info = vectorIter->get();
            DMLOG(DM_LOG_INFO, "subscribe info capability:%s.", info->info.capability);
            if (strcmp(DM_CAPABILITY_OSD, info->info.capability) != 0) {
                DMLOG(DM_LOG_ERROR, "subscribe info capability invalid.");
            }
            uint16_t originId = (uint16_t)(((uint32_t)info->info.subscribeId) & SUBSCRIBE_ID_MASK);
            std::string strPkgName = iter->first;
            DmDeviceInfo dmDeviceInfo;

            DeviceInfoCopyToDmDevice(dmDeviceInfo, *device);
            IpcServerListenerAdapter::GetInstance().OnDeviceFound(strPkgName, originId, dmDeviceInfo);
        }
    }
}

bool SoftbusAdapter::GetpkgNameBySubscribeId(int32_t adapterId, std::string &pkgName)
{
    for (auto iter = subscribeInfos_.begin(); iter != subscribeInfos_.end(); ++iter) {
        std::vector<std::shared_ptr<SubscribeInfoAdapter>> &subinfoVector = iter->second;
        auto vectorIter = subinfoVector.begin();
        for (; vectorIter != subinfoVector.end(); ++vectorIter) {
            if (vectorIter->get()->info.subscribeId == adapterId) {
                pkgName = iter->first;
                return true;
            }
        }
    }
    return false;
}

void SoftbusAdapter::OnSoftbusDiscoverFailed(int32_t subscribeId, DiscoveryFailReason failReason)
{
    DMLOG(DM_LOG_INFO, "In, subscribeId %d, failReason %d", subscribeId, (int32_t)failReason);
    std::string pkgName;
    if (!GetpkgNameBySubscribeId(subscribeId, pkgName)) {
        DMLOG(DM_LOG_ERROR, "OnSoftbusDiscoverFailed: pkgName not found");
        return;
    }

    uint16_t originId = (uint16_t)(((uint32_t)subscribeId) & SUBSCRIBE_ID_MASK);
    IpcServerListenerAdapter::GetInstance().OnDiscoverFailed(pkgName, originId, failReason);
}

void SoftbusAdapter::OnSoftbusDiscoverySuccess(int32_t subscribeId)
{
    DMLOG(DM_LOG_INFO, "In, subscribeId %d", subscribeId);
    std::string pkgName;
    if (!GetpkgNameBySubscribeId(subscribeId, pkgName)) {
        DMLOG(DM_LOG_ERROR, "OnSoftbusDiscoverySuccess: pkgName not found");
        return;
    }
    uint16_t originId = (uint16_t)(((uint32_t)subscribeId) & SUBSCRIBE_ID_MASK);
    IpcServerListenerAdapter::GetInstance().OnDiscoverySuccess(pkgName, originId);
}

bool SoftbusAdapter::GetsubscribeIdAdapter(std::string &pkgName, int16_t originId, int32_t &adapterId)
{
    DMLOG(DM_LOG_INFO, "GetsubscribeIdAdapter in, pkgName: %s, originId:%d", pkgName.c_str(),
        (int32_t)originId);
    auto iter = subscribeInfos_.find(pkgName);
    if (iter == subscribeInfos_.end()) {
        DMLOG(DM_LOG_ERROR, "subscribeInfo not find for pkgName: %s", pkgName.c_str());
        return false;
    }

    std::vector<std::shared_ptr<SubscribeInfoAdapter>> &subinfoVector = iter->second;
    auto vectorIter = subinfoVector.begin();
    for (; vectorIter != subinfoVector.end(); ++vectorIter) {
        if (vectorIter->get()->subscribeIdOrigin == originId) {
            DMLOG(DM_LOG_ERROR, "find adapterId:%d for pkgName: %s, originId:%d",
                vectorIter->get()->info.subscribeId, pkgName.c_str(), (int32_t)originId);
            adapterId = vectorIter->get()->info.subscribeId;
            return true;
        }
    }
    DMLOG(DM_LOG_ERROR, "subscribe not find. pkgName: %s, originId:%d", pkgName.c_str(), (int32_t)originId);
    return false;
}

int32_t SoftbusAdapter::Init()
{
    int32_t ret;
    int32_t retryTimes = 0;
    do {
        ret = RegNodeDeviceStateCb(DEVICE_MANAGER_PACKAGE_NAME.c_str(), &softbusNodeStateCb_);
        if (ret != DEVICEMANAGER_OK) {
            ++retryTimes;
            DMLOG(DM_LOG_ERROR, "RegNodeDeviceStateCb failed with ret %d, retryTimes %d", ret, retryTimes);
            usleep(CHECK_INTERVAL);
        }
    } while (ret != DEVICEMANAGER_OK);
    DMLOG(DM_LOG_INFO, "RegNodeDeviceStateCb success.");
    SoftbusSession::GetInstance().Start();

    PublishInfo dmPublishInfo;
    dmPublishInfo.publishId = DISTRIBUTED_HARDWARE_DEVICEMANAGER_SA_ID;
    dmPublishInfo.mode = DiscoverMode::DISCOVER_MODE_ACTIVE;
    dmPublishInfo.medium = ExchanageMedium::AUTO;
    dmPublishInfo.freq = ExchangeFreq::HIGH;
    dmPublishInfo.capability = DM_CAPABILITY_OSD;
    dmPublishInfo.capabilityData = nullptr;
    dmPublishInfo.dataLen = 0;
    ret = PublishService(DEVICE_MANAGER_PACKAGE_NAME.c_str(), &dmPublishInfo, &servicePublishCallback_);
    DMLOG(DM_LOG_INFO, "service publish result is : %d", ret);
    return ret;
}

int32_t SoftbusAdapter::GetTrustDevices(const std::string &pkgName, NodeBasicInfo **info, int32_t *infoNum)
{
    DMLOG(DM_LOG_INFO, "DM_GetSoftbusTrustDevices start, pkgName: %s", pkgName.c_str());
    int32_t ret = GetAllNodeDeviceInfo(DEVICE_MANAGER_PACKAGE_NAME.c_str(), info, infoNum);
    if (ret != 0) {
        DMLOG(DM_LOG_ERROR, "GetAllNodeDeviceInfo failed with ret %d", ret);
        return ret;
    }
    DMLOG(DM_LOG_INFO, "success, pkgName: %s, deviceCount %d", pkgName.c_str(), *infoNum);
    return DEVICEMANAGER_OK;
}

int32_t SoftbusAdapter::StartDiscovery(std::string &pkgName, SubscribeInfo *info)
{
    std::shared_ptr<SubscribeInfoAdapter> subinfo = nullptr;
    if (subscribeInfos_.find(pkgName) == subscribeInfos_.end()) {
        subscribeInfos_[pkgName] = {};
    }

    auto iter = subscribeInfos_.find(pkgName);
    std::vector<std::shared_ptr<SubscribeInfoAdapter>> &subinfoVector = iter->second;
    auto vectorIter = subinfoVector.begin();
    for (; vectorIter != subinfoVector.end(); ++vectorIter) {
        if (vectorIter->get()->subscribeIdOrigin == info->subscribeId) {
            subinfo = *vectorIter;
            break;
        }
    }
    if (subinfo == nullptr) {
        std::lock_guard<std::mutex> autoLock(lock_);
        subinfo = std::make_shared<SubscribeInfoAdapter>();
        subinfo->subscribeIdOrigin = info->subscribeId;
        subinfo->subscribeIdPrefix = subscribeIdPrefix++;
        subinfo->info = *info;

        uint32_t uSubscribeId = static_cast<uint32_t>(info->subscribeId);
        uSubscribeId = (subinfo->subscribeIdPrefix << SUBSCRIBE_ID_PREFIX_LEN) | uSubscribeId;
        subinfo->info.subscribeId = static_cast<int32_t>(uSubscribeId);
    }
    if (vectorIter == subinfoVector.end()) {
        subinfoVector.push_back(subinfo);
    }
    DMLOG(DM_LOG_INFO, "StartDiscovery, pkgName: %s, subscribeId %d, prefix %d, origin %d",
        pkgName.c_str(), subinfo->info.subscribeId, subinfo->subscribeIdPrefix, subinfo->subscribeIdOrigin);
    DMLOG(DM_LOG_INFO, "Capability: %s", subinfo->info.capability);
    int32_t ret = ::StartDiscovery(DEVICE_MANAGER_PACKAGE_NAME.c_str(), &subinfo->info, &softbusDiscoverCallback_);
    if (ret != 0) {
        DMLOG(DM_LOG_ERROR, "StartDiscovery failed with ret %d.", ret);
        return DEVICEMANAGER_DISCOVERY_FAILED;
    }
    return DEVICEMANAGER_OK;
}

int32_t SoftbusAdapter::StopDiscovery(std::string &pkgName, uint16_t subscribeId)
{
    int32_t subscribeIdAdapter = -1;
    if (!GetsubscribeIdAdapter(pkgName, subscribeId, subscribeIdAdapter)) {
        DMLOG(DM_LOG_ERROR, "StopDiscovery failed, subscribeId not match");
        return DEVICEMANAGER_FAILED;
    }

    DMLOG(DM_LOG_INFO, "StopDiscovery begin, pkgName: %s, subscribeId:%d, subscribeIdAdapter:%d",
        pkgName.c_str(), (int32_t)subscribeId, subscribeIdAdapter);
    int32_t ret = ::StopDiscovery(DEVICE_MANAGER_PACKAGE_NAME.c_str(), subscribeIdAdapter);
    if (ret != 0) {
        DMLOG(DM_LOG_ERROR, "StopDiscovery failed with ret %d", ret);
        return ret;
    }

    auto iter = subscribeInfos_.find(pkgName);
    auto subinfoVector = iter->second;
    auto vectorIter = subinfoVector.begin();
    while (vectorIter != subinfoVector.end()) {
        if (vectorIter->get()->subscribeIdOrigin == subscribeId) {
            vectorIter = subinfoVector.erase(vectorIter);
            break;
        } else {
            ++vectorIter;
        }
    }
    if (subinfoVector.empty()) {
        subscribeInfos_.erase(pkgName);
    }
    DMLOG(DM_LOG_INFO, "DM_StopSoftbusDiscovery completed, pkgName: %s", pkgName.c_str());
    return DEVICEMANAGER_OK;
}

bool SoftbusAdapter::IsDeviceOnLine(std::string &deviceId)
{
    NodeBasicInfo *info = nullptr;
    int32_t infoNum = 0;

    if (GetTrustDevices(DEVICE_MANAGER_PACKAGE_NAME.c_str(), &info, &infoNum) != DEVICEMANAGER_OK) {
        DMLOG(DM_LOG_ERROR, "DM_IsDeviceOnLine DM_GetSoftbusTrustDevices failed");
        return false;
    }

    bool bDeviceOnline = false;
    for (int32_t i = 0; i < infoNum; ++i) {
        NodeBasicInfo *nodeBasicInfo = info + i;
        if (nodeBasicInfo == nullptr) {
            DMLOG(DM_LOG_ERROR, "nodeBasicInfo is empty for index %d, infoNum %d.", i, infoNum);
            continue;
        }
        std::string networkId = nodeBasicInfo->networkId;
        if (networkId == deviceId) {
            DMLOG(DM_LOG_INFO, "DM_IsDeviceOnLine device %s online", GetAnonyString(deviceId).c_str());
            bDeviceOnline = true;
            break;
        }
        uint8_t udid[UDID_BUF_LEN] = {0};
        int32_t ret = GetNodeKeyInfo(DEVICE_MANAGER_PACKAGE_NAME.c_str(), networkId.c_str(),
            NodeDeivceInfoKey::NODE_KEY_UDID, udid, sizeof(udid));
        if (ret != DEVICEMANAGER_OK) {
            DMLOG(DM_LOG_ERROR, "DM_IsDeviceOnLine GetNodeKeyInfo failed");
            break;
        }

        if (strcmp((char *)udid, deviceId.c_str()) == 0) {
            DMLOG(DM_LOG_INFO, "DM_IsDeviceOnLine devccie %s online", GetAnonyString(deviceId).c_str());
            bDeviceOnline = true;
            break;
        }
    }
    FreeNodeInfo(info);
    return bDeviceOnline;
}

int32_t SoftbusAdapter::GetConnectionIpAddr(std::string deviceId, std::string &ipAddr)
{
    auto iter = discoverDeviceInfoMap_.find(deviceId);
    if (iter == discoverDeviceInfoMap_.end()) {
        DMLOG(DM_LOG_ERROR, "deviceInfo not found by deviceId %s", GetAnonyString(deviceId).c_str());
        return DEVICEMANAGER_FAILED;
    }

    DeviceInfo *deviceInfo = iter->second.get();
    if (deviceInfo->addrNum <= 0 || deviceInfo->addrNum >= CONNECTION_ADDR_MAX) {
        DMLOG(DM_LOG_ERROR, "deviceInfo addrNum not valid, addrNum %d", deviceInfo->addrNum);
        return DEVICEMANAGER_FAILED;
    }

    for (uint32_t i = 0; i < deviceInfo->addrNum; ++i) {
        // currently, only support CONNECT_ADDR_WLAN
        if (deviceInfo->addr[i].type != ConnectionAddrType::CONNECTION_ADDR_WLAN &&
            deviceInfo->addr[i].type != ConnectionAddrType::CONNECTION_ADDR_ETH) {
            continue;
        }
        ipAddr = deviceInfo->addr[i].info.ip.ip;
        DMLOG(DM_LOG_INFO, "DM_GetConnectionIpAddr get ip ok.");
        return DEVICEMANAGER_OK;
    }
    DMLOG(DM_LOG_ERROR, "failed to get ipAddr for deviceId %s", GetAnonyString(deviceId).c_str());
    return DEVICEMANAGER_FAILED;
}

// eth >> wlan >> ble >> br
ConnectionAddr *SoftbusAdapter::GetConnectAddrByType(DeviceInfo *deviceInfo, ConnectionAddrType type)
{
    if (deviceInfo == nullptr) {
        return nullptr;
    }
    for (uint32_t i = 0; i < deviceInfo->addrNum; ++i) {
        if (deviceInfo->addr[i].type == type) {
            return &deviceInfo->addr[i];
        }
    }
    return nullptr;
}

ConnectionAddr *SoftbusAdapter::GetConnectAddr(std::string deviceId)
{
    auto iter = discoverDeviceInfoMap_.find(deviceId);
    if (iter == discoverDeviceInfoMap_.end()) {
        DMLOG(DM_LOG_ERROR, "deviceInfo not found by deviceId %s", GetAnonyString(deviceId).c_str());
        return nullptr;
    }

    DeviceInfo *deviceInfo = iter->second.get();
    if (deviceInfo->addrNum <= 0 || deviceInfo->addrNum >= CONNECTION_ADDR_MAX) {
        DMLOG(DM_LOG_ERROR, "deviceInfo addrNum not valid, addrNum %d", deviceInfo->addrNum);
        return nullptr;
    }

    ConnectionAddr *addr = nullptr;
    addr = GetConnectAddrByType(deviceInfo, ConnectionAddrType::CONNECTION_ADDR_ETH);
    if (addr != nullptr) {
        DMLOG(DM_LOG_INFO, "get ETH ConnectionAddr for deviceId %s", GetAnonyString(deviceId).c_str());
        return addr;
    }

    addr = GetConnectAddrByType(deviceInfo, ConnectionAddrType::CONNECTION_ADDR_WLAN);
    if (addr != nullptr) {
        DMLOG(DM_LOG_INFO, "get WLAN ConnectionAddr for deviceId %s", GetAnonyString(deviceId).c_str());
        return addr;
    }

    addr = GetConnectAddrByType(deviceInfo, ConnectionAddrType::CONNECTION_ADDR_BLE);
    if (addr != nullptr) {
        DMLOG(DM_LOG_INFO, "get BLE ConnectionAddr for deviceId %s", GetAnonyString(deviceId).c_str());
        return addr;
    }

    addr = GetConnectAddrByType(deviceInfo, ConnectionAddrType::CONNECTION_ADDR_BR);
    if (addr != nullptr) {
        DMLOG(DM_LOG_INFO, "get BR ConnectionAddr for deviceId %s", GetAnonyString(deviceId).c_str());
        return addr;
    }

    DMLOG(DM_LOG_ERROR, "failed to get ConnectionAddr for deviceId %s", GetAnonyString(deviceId).c_str());
    return nullptr;
}

void SoftbusAdapter::NodeBasicInfoCopyToDmDevice(DmDeviceInfo &dmDeviceInfo, NodeBasicInfo &nodeBasicInfo)
{
    (void)memset_s(&dmDeviceInfo, sizeof(DmDeviceInfo), 0, sizeof(DmDeviceInfo));
    if (memcpy_s(dmDeviceInfo.deviceId, sizeof(dmDeviceInfo.deviceId), nodeBasicInfo.networkId,
        std::min(sizeof(dmDeviceInfo.deviceId), sizeof(nodeBasicInfo.networkId))) != DEVICEMANAGER_OK) {
        DMLOG(DM_LOG_ERROR, "memcpy failed");
    }
    if (memcpy_s(dmDeviceInfo.deviceName, sizeof(dmDeviceInfo.deviceName), nodeBasicInfo.deviceName,
        std::min(sizeof(dmDeviceInfo.deviceName), sizeof(nodeBasicInfo.deviceName))) != DEVICEMANAGER_OK) {
        DMLOG(DM_LOG_ERROR, "memcpy failed");
    }
    dmDeviceInfo.deviceTypeId = (DMDeviceType)nodeBasicInfo.deviceTypeId;
}

void SoftbusAdapter::DeviceInfoCopyToDmDevice(DmDeviceInfo &dmDeviceInfo, const DeviceInfo &deviceInfo)
{
    (void)memset_s(&dmDeviceInfo, sizeof(DmDeviceInfo), 0, sizeof(DmDeviceInfo));
    if (memcpy_s(dmDeviceInfo.deviceId, sizeof(dmDeviceInfo.deviceId), deviceInfo.devId,
        std::min(sizeof(dmDeviceInfo.deviceId), sizeof(deviceInfo.devId))) != DEVICEMANAGER_OK) {
        DMLOG(DM_LOG_ERROR, "memcpy failed");
    }
    if (memcpy_s(dmDeviceInfo.deviceName, sizeof(dmDeviceInfo.deviceName), deviceInfo.devName,
        std::min(sizeof(dmDeviceInfo.deviceName), sizeof(deviceInfo.devName))) != DEVICEMANAGER_OK) {
        DMLOG(DM_LOG_ERROR, "memcpy failed");
    }
    dmDeviceInfo.deviceTypeId = (DMDeviceType)deviceInfo.devType;
}

void PublishServiceCallBack::OnPublishSuccess(int32_t publishId)
{
    DMLOG(DM_LOG_INFO, "service publish succeed, publishId: %d", publishId);
}

void PublishServiceCallBack::OnPublishFail(int32_t publishId, PublishFailReason reason)
{
    DMLOG(DM_LOG_INFO, "service publish failed, publishId: %d, reason: %d", publishId, reason);
}
} // namespace DistributedHardware
} // namespace OHOS