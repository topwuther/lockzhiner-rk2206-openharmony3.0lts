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
#ifndef DEVICE_INFO_MANAGER_H
#define DEVICE_INFO_MANAGER_H

#include <string>

#include "device_info_repository.h"
#include "distributed_data_validator.h"
#include "permission_log.h"
#include "parameter.h"
#include "ipc_skeleton.h"
#include "soft_bus_manager.h"

namespace OHOS {
namespace Security {
namespace Permission {
class DeviceInfoManager {
public:
    static DeviceInfoManager &GetInstance();

    /**
     * Get device info by device id.
     *
     * @param nodeId Device id.
     * @param deviceIdType Device id type {@link DeviceIdType}
     * @return Optional deviceInfo
     */
    bool GetDeviceInfo(const std::string &nodeId, DeviceIdType deviceIdType, DeviceInfo &deviceInfo) const;

    /**
     * Check device info exist. Online and local device info will be here.
     *
     * @param nodeId Device id.
     * @param deviceIdType Device id type {@link DeviceIdType}
     * @return True for exist, false otherwise.
     */
    bool ExistDeviceInfo(const std::string &nodeId, DeviceIdType deviceIdType) const;

    /**
     * Add device info with device ids and device properties.
     *
     * @param networkId Device networkId.
     * @param universallyUniqueId Device uuid.
     * @param uniqueDisabilityId Device udid.
     * @param deviceName Device name.
     * @param deviceType Device type.
     */
    void AddDeviceInfo(const std::string &networkId, const std::string &universallyUniqueId,
        const std::string &uniqueDisabilityId, const std::string &deviceName, const std::string &deviceType);

    /**
     * Remote all device info.
     */
    void RemoveAllRemoteDeviceInfo();

    /**
     * Remove one device info.
     *
     * @param nodeId Device id.
     */
    void RemoveRemoteDeviceInfo(const std::string &nodeId, DeviceIdType deviceIdType);

    /**
     * Convert nodeId to deviceId(UUID) if possible.
     *
     * @param nodeId which is considered as indefinite id, maybe deviceId(UUID) or networkId.
     * @return The deviceId if local or device online, otherwise return empty string.
     */
    std::string ConvertToUniversallyUniqueIdOrFetch(const std::string &nodeId) const;

    /**
     * Convert nodeId to deviceId(UDID) if possible.
     *
     * @param nodeId which is considered as indefinite id, maybe deviceId(UDID) or networkId.
     * @return The deviceId if local or device online, otherwise return empty string.
     */
    std::string ConvertToUniqueDisabilityIdOrFetch(const std::string &nodeId) const;

    /**
     * Check nodeId is uuid or not.
     *
     * @param nodeId Node id.
     * @return True if node id is uuid. False otherwise.
     */
    bool IsDeviceUniversallyUniqueId(const std::string &nodeId) const;
};
}  // namespace Permission
}  // namespace Security
}  // namespace OHOS
#endif  // PERMISSION_BASE_SERVICE_H