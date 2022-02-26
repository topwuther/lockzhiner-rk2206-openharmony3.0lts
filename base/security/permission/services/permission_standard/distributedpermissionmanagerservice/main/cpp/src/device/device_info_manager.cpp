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

#include "device_info_manager.h"

namespace OHOS {
namespace Security {
namespace Permission {
namespace {
static constexpr OHOS::HiviewDFX::HiLogLabel LABEL = {LOG_CORE, SECURITY_DOMAIN_PERMISSION, "DeviceInfoManager"};
}
DeviceInfoManager &DeviceInfoManager::GetInstance()
{
    static DeviceInfoManager instance;
    return instance;
}

bool DeviceInfoManager::GetDeviceInfo(
    const std::string &nodeId, DeviceIdType deviceIdType, DeviceInfo &deviceInfo) const
{
    return DeviceInfoRepository::GetInstance().FindDeviceInfo(nodeId, deviceIdType, deviceInfo);
}

bool DeviceInfoManager::ExistDeviceInfo(const std::string &nodeId, DeviceIdType deviceIdType) const
{
    DeviceInfo deviceInfo;
    return DeviceInfoRepository::GetInstance().FindDeviceInfo(nodeId, deviceIdType, deviceInfo);
}

void DeviceInfoManager::AddDeviceInfo(const std::string &networkId, const std::string &universallyUniqueId,
    const std::string &uniqueDisabilityId, const std::string &deviceName, const std::string &deviceType)
{
    if (!DistributedDataValidator::IsDeviceIdValid(networkId) ||
        !DistributedDataValidator::IsDeviceIdValid(universallyUniqueId) ||
        !DistributedDataValidator::IsDeviceIdValid(uniqueDisabilityId) || deviceName.empty() || deviceType.empty()) {
        PERMISSION_LOG_ERROR(LABEL, "addDeviceInfo: input param is invalid");
    }
    DeviceInfoRepository::GetInstance().SaveDeviceInfo(
        networkId, universallyUniqueId, uniqueDisabilityId, deviceName, deviceType);
}

void DeviceInfoManager::RemoveAllRemoteDeviceInfo()
{
    char deviceIdCharArray[Constant::DEVICE_UUID_LENGTH] = {0};
    GetDevUdid(deviceIdCharArray, Constant::DEVICE_UUID_LENGTH);
    DeviceInfo localDeviceInfoOpt;
    if (DeviceInfoRepository::GetInstance().FindDeviceInfo(
            deviceIdCharArray, DeviceIdType::UNIQUE_DISABILITY_ID, localDeviceInfoOpt)) {
        DeviceInfoRepository::GetInstance().DeleteAllDeviceInfoExceptOne(localDeviceInfoOpt);
    }
}

void DeviceInfoManager::RemoveRemoteDeviceInfo(const std::string &nodeId, DeviceIdType deviceIdType)
{
    if (!DistributedDataValidator::IsDeviceIdValid(nodeId)) {
        PERMISSION_LOG_ERROR(LABEL, "removeDeviceInfoByNetworkId: nodeId is invalid");
    } else {
        DeviceInfo deviceInfo;
        char deviceIdCharArray[Constant::DEVICE_UUID_LENGTH] = {0};
        GetDevUdid(deviceIdCharArray, Constant::DEVICE_UUID_LENGTH);
        if (DeviceInfoRepository::GetInstance().FindDeviceInfo(nodeId, deviceIdType, deviceInfo)) {
            if (deviceInfo.deviceId.uniqueDisabilityId != deviceIdCharArray) {
                DeviceInfoRepository::GetInstance().DeleteDeviceInfo(nodeId, deviceIdType);
            }
        }
    }
}

std::string DeviceInfoManager::ConvertToUniversallyUniqueIdOrFetch(const std::string &nodeId) const
{
    std::string result;
    if (!DistributedDataValidator::IsDeviceIdValid(nodeId)) {
        PERMISSION_LOG_ERROR(LABEL, "ConvertToUniversallyUniqueIdOrFetch: nodeId is invalid.");
        return result;
    }
    DeviceInfo deviceInfo;
    if (DeviceInfoRepository::GetInstance().FindDeviceInfo(nodeId, DeviceIdType::UNKNOWN, deviceInfo)) {
        std::string universallyUniqueId = deviceInfo.deviceId.universallyUniqueId;
        if (universallyUniqueId.empty()) {
            std::string udid = SoftBusManager::GetInstance().GetUniversallyUniqueIdByNodeId(nodeId);
            if (!udid.empty()) {
                result = udid;
            }
        } else {
            result = universallyUniqueId;
        }
    }
    return result;
}

std::string DeviceInfoManager::ConvertToUniqueDisabilityIdOrFetch(const std::string &nodeId) const
{
    std::string result;
    if (!DistributedDataValidator::IsDeviceIdValid(nodeId)) {
        PERMISSION_LOG_ERROR(LABEL, "ConvertToUniqueDisabilityIdOrFetch: nodeId is invalid.");
        return result;
    }
    DeviceInfo deviceInfo;
    if (DeviceInfoRepository::GetInstance().FindDeviceInfo(nodeId, DeviceIdType::UNKNOWN, deviceInfo)) {
        std::string uniqueDisabilityId = deviceInfo.deviceId.uniqueDisabilityId;
        if (uniqueDisabilityId.empty()) {
            std::string udid = SoftBusManager::GetInstance().GetUniqueDisabilityIdByNodeId(nodeId);
            if (!udid.empty()) {
                result = udid;
            } else {
                PERMISSION_LOG_DEBUG(LABEL,
                    "FindDeviceInfo succeed, udid and local udid is empty, nodeId(%{public}s)",
                    Constant::EncryptDevId(nodeId).c_str());
            }
        } else {
            PERMISSION_LOG_DEBUG(LABEL,
                "FindDeviceInfo succeed, udid is empty, nodeId(%{public}s) ",
                Constant::EncryptDevId(nodeId).c_str());
            result = uniqueDisabilityId;
        }
    } else {
        PERMISSION_LOG_DEBUG(
            LABEL, "FindDeviceInfo failed, nodeId(%{public}s)", Constant::EncryptDevId(nodeId).c_str());
        auto list = DeviceInfoRepository::GetInstance().ListDeviceInfo();
        auto iter = list.begin();
        for (; iter != list.end(); iter++) {
            DeviceInfo info = (*iter);
            PERMISSION_LOG_DEBUG(
                LABEL, ">>> DistributedPermissionDuidTransformTest device name: %{public}s", info.deviceName.c_str());
            PERMISSION_LOG_DEBUG(
                LABEL, ">>> DistributedPermissionDuidTransformTest device type: %{public}s", info.deviceType.c_str());
            PERMISSION_LOG_DEBUG(LABEL,
                ">>> DistributedPermissionDuidTransformTest device network id: %{public}s",
                Constant::EncryptDevId(info.deviceId.networkId).c_str());
            PERMISSION_LOG_DEBUG(LABEL,
                ">>> DistributedPermissionDuidTransformTest device udid: %{public}s",
                Constant::EncryptDevId(info.deviceId.uniqueDisabilityId).c_str());
            PERMISSION_LOG_DEBUG(LABEL,
                ">>> DistributedPermissionDuidTransformTest device uuid: %{public}s",
                Constant::EncryptDevId(info.deviceId.universallyUniqueId).c_str());
        }
    }
    return result;
}

bool DeviceInfoManager::IsDeviceUniversallyUniqueId(const std::string &nodeId) const
{
    if (!DistributedDataValidator::IsDeviceIdValid(nodeId)) {
        PERMISSION_LOG_ERROR(LABEL, "IsDeviceUniversallyUniqueId: nodeId is invalid");
        return false;
    }
    DeviceInfo deviceInfo;
    if (DeviceInfoRepository::GetInstance().FindDeviceInfo(nodeId, DeviceIdType::UNIVERSALLY_UNIQUE_ID, deviceInfo)) {
        return deviceInfo.deviceId.universallyUniqueId == nodeId;
    }
    return false;
}
}  // namespace Permission
}  // namespace Security
}  // namespace OHOS