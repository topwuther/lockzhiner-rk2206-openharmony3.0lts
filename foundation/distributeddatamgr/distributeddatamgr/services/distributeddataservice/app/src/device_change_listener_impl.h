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

#ifndef DEV_DEVICE_CHANGE_LISTENER_IMPL_H
#define DEV_DEVICE_CHANGE_LISTENER_IMPL_H

#include <map>
#include "app_device_status_change_listener.h"
#include "idevice_status_change_listener.h"

namespace OHOS::DistributedKv {
class DeviceChangeListenerImpl : public AppDistributedKv::AppDeviceStatusChangeListener {
public:
    explicit DeviceChangeListenerImpl(std::map<IRemoteObject *,
                                      sptr<IDeviceStatusChangeListener>> &observers);
    void OnDeviceChanged(const AppDistributedKv::DeviceInfo &info,
                         const AppDistributedKv::DeviceChangeType &type) const override;
    AppDistributedKv::ChangeLevelType GetChangeLevelType() const override;
    ~DeviceChangeListenerImpl() {}
private:
    std::map<IRemoteObject *, sptr<IDeviceStatusChangeListener>> &observers_;
};
}
#endif // DEV_DEVICE_CHANGE_LISTENER_IMPL_H
