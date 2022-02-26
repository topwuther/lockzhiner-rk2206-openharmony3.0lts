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

#ifndef DEVICE_STATUS_CHANGE_LISTENER_H
#define DEVICE_STATUS_CHANGE_LISTENER_H

#include "types.h"

namespace OHOS {
namespace DistributedKv {
class DeviceStatusChangeListener {
public:
    KVSTORE_API virtual ~DeviceStatusChangeListener() {};
    KVSTORE_API virtual void OnDeviceChanged(const DeviceInfo &info, const DeviceChangeType &type) const = 0;
    KVSTORE_API virtual DeviceFilterStrategy GetFilterStrategy() const = 0;
};
}  // namespace DistributedKv
}  // namespace OHOS

#endif  // DEVICE_STATUS_CHANGE_LISTENER_H
