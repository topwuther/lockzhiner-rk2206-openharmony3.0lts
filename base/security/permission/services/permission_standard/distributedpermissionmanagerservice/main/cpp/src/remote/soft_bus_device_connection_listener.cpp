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

#include "soft_bus_device_connection_listener.h"
#include "remote_command_manager.h"
#include "soft_bus_manager.h"
#include "device_info_manager.h"

#include "permission_bms_manager.h"

namespace OHOS {
namespace Security {
namespace Permission {
namespace {
static constexpr OHOS::HiviewDFX::HiLogLabel LABEL = {
    LOG_CORE, SECURITY_DOMAIN_PERMISSION, "SoftBusDeviceConnectionListener"};
}
SoftBusDeviceConnectionListener::SoftBusDeviceConnectionListener()
{
    PERMISSION_LOG_DEBUG(LABEL, "SoftBusDeviceConnectionListener()");
}
SoftBusDeviceConnectionListener::~SoftBusDeviceConnectionListener()
{
    PERMISSION_LOG_DEBUG(LABEL, "~SoftBusDeviceConnectionListener()");
}

void SoftBusDeviceConnectionListener::OnDeviceOnline(const DmDeviceInfo &info)
{
    std::string networkId = info.deviceId;
    std::string uuid = SoftBusManager::GetInstance().GetUniversallyUniqueIdByNodeId(networkId);
    std::string udid = SoftBusManager::GetInstance().GetUniqueDisabilityIdByNodeId(networkId);

    PERMISSION_LOG_INFO(LABEL,
        "networkId: %{public}s, uuid: %{public}s, udid: %{public}s",
        networkId.c_str(),
        uuid.c_str(),
        udid.c_str());

    if (uuid != "" && udid != "") {
        DeviceInfoManager::GetInstance().AddDeviceInfo(
            networkId, uuid, udid, info.deviceName, std::to_string(info.deviceTypeId));
        RemoteCommandManager::GetInstance().NotifyDeviceOnline(udid);
    } else {
        PERMISSION_LOG_ERROR(LABEL, "uuid or udid is empty, online failed.");
    }
    // no need to load local permissions by now.
}

void SoftBusDeviceConnectionListener::OnDeviceOffline(const DmDeviceInfo &info)
{
    std::string networkId = info.deviceId;
    std::string uuid = DeviceInfoManager::GetInstance().ConvertToUniversallyUniqueIdOrFetch(networkId);
    std::string udid = DeviceInfoManager::GetInstance().ConvertToUniqueDisabilityIdOrFetch(networkId);

    PERMISSION_LOG_INFO(LABEL,
        "networkId: %{public}s,  uuid: %{public}s, udid: %{public}s",
        networkId.c_str(),
        uuid.c_str(),
        udid.c_str());

    if (uuid != "" && udid != "") {
        RemoteCommandManager::GetInstance().NotifyDeviceOffline(uuid);
        RemoteCommandManager::GetInstance().NotifyDeviceOffline(udid);
        DeviceInfoManager::GetInstance().RemoveRemoteDeviceInfo(networkId, DeviceIdType::NETWORK_ID);
    } else {
        PERMISSION_LOG_ERROR(LABEL, "uuid or udid is empty, offline failed.");
    }
}

void SoftBusDeviceConnectionListener::OnDeviceReady(const DmDeviceInfo &info)
{
    std::string networkId = info.deviceId;
    PERMISSION_LOG_INFO(LABEL, "networkId: %{public}s", networkId.c_str());
}

void SoftBusDeviceConnectionListener::OnDeviceChanged(const DmDeviceInfo &info)
{
    std::string networkId = info.deviceId;
    PERMISSION_LOG_INFO(LABEL, "networkId: %{public}s", networkId.c_str());
}
}  // namespace Permission
}  // namespace Security
}  // namespace OHOS
