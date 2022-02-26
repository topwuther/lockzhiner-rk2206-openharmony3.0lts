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
#ifndef DEVICE_INFO_REPOSITORY_H
#define DEVICE_INFO_REPOSITORY_H

#include <map>
#include <string>
#include <vector>
#include <mutex>

#include "constant.h"
#include "device_info.h"

namespace OHOS {
namespace Security {
namespace Permission {
class DeviceInfoRepository {
public:
    static DeviceInfoRepository &GetInstance();

    std::vector<DeviceInfo> ListDeviceInfo();

    bool FindDeviceInfo(const std::string &nodeId, DeviceIdType type, DeviceInfo &deviceInfo);

    void DeleteAllDeviceInfoExceptOne(const DeviceInfo deviceInfo);

    void SaveDeviceInfo(const DeviceInfo deviceInfo);

    void SaveDeviceInfo(const DeviceId deviceId, const std::string &deviceName, const std::string &deviceType);

    void SaveDeviceInfo(const std::string &networkId, const std::string &universallyUniqueId,
        const std::string &uniqueDisabilityId, const std::string &deviceName, const std::string &deviceType);

    void DeleteDeviceInfo(const std::string &nodeId, const DeviceIdType type);

    void Clear();

private:
    bool FindDeviceIdByNodeIdLocked(const std::string &nodeId, const DeviceIdType type, DeviceId &deviceId) const;

    bool FindDeviceInfoByDeviceIdLocked(const DeviceId deviceId, DeviceInfo &deviceInfo) const;

    bool FindDeviceIdByNetworkIdLocked(const std::string &networkId, DeviceId &deviceId) const;

    bool FindDeviceIdByUniversallyUniqueIdLocked(const std::string &universallyUniqueId, DeviceId &deviceId) const;

    bool FindDeviceIdByUniqueDisabilityIdLocked(const std::string &uniqueDisabilityId, DeviceId &deviceId) const;

    void DeleteDeviceInfoByDeviceIdLocked(const DeviceId deviceId);

    std::map<std::string, DeviceId> deviceIdMapByNetworkId_;

    std::map<std::string, DeviceId> deviceIdMapByUniversallyUniqueId_;

    std::map<std::string, DeviceId> deviceIdMapByUniqueDisabilityId_;

    std::map<std::string, DeviceInfo> deviceInfoMap_;

    std::recursive_mutex stackLock_;
};
}  // namespace Permission
}  // namespace Security
}  // namespace OHOS
#endif  // DEVICE_INFO_REPOSITORY_H