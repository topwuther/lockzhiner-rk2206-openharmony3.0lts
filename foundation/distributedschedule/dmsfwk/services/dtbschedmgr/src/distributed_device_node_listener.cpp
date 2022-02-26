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

#include "distributed_device_node_listener.h"

#include "deviceManager/dms_device_info.h"
#include "dtbschedmgr_device_info_storage.h"
#include "dtbschedmgr_log.h"

namespace OHOS {
namespace DistributedSchedule {
void DistributedDeviceNodeListener::OnDeviceOnline(const NodeBasicInfo* nodeBasicInfo)
{
    if (nodeBasicInfo == nullptr) {
        HILOGE("DistributedDeviceNodeListener::OnDeviceOnline null nodeBasicInfo");
        return;
    }
    auto dmsDeviceInfo = std::make_shared<DmsDeviceInfo>(
        nodeBasicInfo->deviceName, nodeBasicInfo->deviceTypeId, nodeBasicInfo->networkId);
    DtbschedmgrDeviceInfoStorage::GetInstance().DeviceOnlineNotify(dmsDeviceInfo);
}

void DistributedDeviceNodeListener::OnDeviceOffline(const NodeBasicInfo* nodeBasicInfo)
{
    if (nodeBasicInfo == nullptr) {
        HILOGE("DistributedDeviceNodeListener::OnDeviceOffline null nodeBasicInfo");
        return;
    }
    DtbschedmgrDeviceInfoStorage::GetInstance().DeviceOfflineNotify(nodeBasicInfo->networkId);
}

void DistributedDeviceNodeListener::OnDeviceInfoChanged(const std::string& deviceId, DeviceInfoType type)
{
    DtbschedmgrDeviceInfoStorage::GetInstance().OnDeviceInfoChanged(deviceId, type);
}
} // namespace DistributedSchedule
} // namespace OHOS
