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

#include "bluetooth_a2dp_src_proxy.h"
#include "bluetooth_log.h"
#include "parcel_bt_uuid.h"

namespace OHOS {
namespace Bluetooth {
void BluetoothA2dpSrcProxy::RegisterObserver(const sptr<IBluetoothA2dpSrcObserver> &observer)
{
    MessageParcel data;
    if (!data.WriteInterfaceToken(BluetoothA2dpSrcProxy::GetDescriptor())) {
        HILOGE("BluetoothA2dpSrcProxy::RegisterObserver WriteInterfaceToken error");
        return;
    }
    if (!data.WriteRemoteObject(observer->AsObject())) {
        HILOGE("BluetoothA2dpSrcProxy::RegisterObserver error");
        return;
    }

    MessageParcel reply;
    MessageOption option {
        MessageOption::TF_ASYNC
    };

    int error = Remote()->SendRequest(IBluetoothA2dpSrc::Code::BT_REGISTER_OBSERVER, data, reply, option);
    if (error != NO_ERROR) {
        HILOGE("BluetoothA2dpSrcProxy::RegisterObserver done fail, error: %{public}d", error);
        return;
    }
}

int BluetoothA2dpSrcProxy::GetDeviceState(const RawAddress &device)
{
    MessageParcel data;
    if (!data.WriteInterfaceToken(BluetoothA2dpSrcProxy::GetDescriptor())) {
        HILOGE("BluetoothA2dpSrcProxy::GetDeviceState WriteInterfaceToken error");
        return -1;
    }
    if (!data.WriteString(device.GetAddress())) {
        HILOGE("BluetoothA2dpSrcProxy::GetDeviceState write device error");
        return -1;
    }

    MessageParcel reply;
    MessageOption option {
        MessageOption::TF_SYNC
    };

    int error = Remote()->SendRequest(IBluetoothA2dpSrc::Code::BT_GET_DEVICE_STATE, data, reply, option);
    if (error != NO_ERROR) {
        HILOGE("BluetoothA2dpSrcProxy::GetDeviceState done fail, error: %{public}d", error);
        return -1;
    }

    return reply.ReadInt32();
}
}  // namespace Bluetooth
}  // namespace OHOS