/*
 * Copyright (C) 2021 Huawei Device Co., Ltd.
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

#ifndef OHOS_BLUETOOTH_STANDARD_A2DP_SRC_INTERFACE_H
#define OHOS_BLUETOOTH_STANDARD_A2DP_SRC_INTERFACE_H

#include <string>

#include "iremote_broker.h"
#include "i_bluetooth_a2dp_src_observer.h"
#include "raw_address.h"

namespace OHOS {
namespace Bluetooth {
using namespace bluetooth;
class IBluetoothA2dpSrc : public OHOS::IRemoteBroker {
public:
    DECLARE_INTERFACE_DESCRIPTOR(u"ohos.ipc.IBluetoothA2dpSrc");

    enum Code {
        BT_GET_DEVICE_STATE = 0,
        BT_REGISTER_OBSERVER,
    };

    virtual int GetDeviceState(const RawAddress &device) = 0;
    virtual void RegisterObserver(const sptr<IBluetoothA2dpSrcObserver> &observer) = 0;
};
}  // namespace Bluetooth
}  // namespace OHOS
#endif  // OHOS_BLUETOOTH_STANDARD_A2DP_INTERFACE_H