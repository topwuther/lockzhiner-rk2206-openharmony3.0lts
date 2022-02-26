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

#define LOG_TAG "DeviceKvStoreImpl"

#include "device_change_listener_impl.h"
#include "kvstore_utils.h"
#include "log_print.h"

using namespace OHOS::AppDistributedKv;
namespace OHOS::DistributedKv {
DeviceChangeListenerImpl::DeviceChangeListenerImpl(std::map<IRemoteObject *,
    sptr<IDeviceStatusChangeListener>> &observers) : observers_(observers)
{}

void DeviceChangeListenerImpl::OnDeviceChanged(const AppDistributedKv::DeviceInfo &info,
                                               const AppDistributedKv::DeviceChangeType &type) const
{
    DeviceChangeType deviceType = type == AppDistributedKv::DeviceChangeType::DEVICE_ONLINE ?
            DeviceChangeType::DEVICE_ONLINE : DeviceChangeType::DEVICE_OFFLINE;
    auto nodeid = KvStoreUtils::GetProviderInstance().ToNodeId(info.deviceId);
    ZLOGD("networkid:%s", nodeid.c_str());
    ZLOGD("uuid:%s", KvStoreUtils::ToBeAnonymous(info.deviceId).c_str());
    for (auto const &observer : observers_) {
        observer.second->OnChange({nodeid, info.deviceName, info.deviceType}, deviceType);
    }
}
AppDistributedKv::ChangeLevelType DeviceChangeListenerImpl::GetChangeLevelType() const
{
    return AppDistributedKv::ChangeLevelType::MIN;
}
}
