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

#include "bluetooth_a2dp_src_stub.h"
#include "bluetooth_log.h"
#include "ipc_types.h"
#include "parcel_bt_uuid.h"
#include "raw_address.h"

namespace OHOS {
namespace Bluetooth {
using namespace bluetooth;
int32_t BluetoothA2dpSrcStub::OnRemoteRequest(
    uint32_t code, MessageParcel &data, MessageParcel &reply, MessageOption &option)
{
    HILOGD("BluetoothA2dpSrcStub: transaction of code: %{public}d is received", code);

    std::u16string descriptor = BluetoothA2dpSrcStub::GetDescriptor();
    std::u16string remoteDescriptor = data.ReadInterfaceToken();
    if (descriptor != remoteDescriptor) {
        HILOGE("BluetoothA2dpSrcStub::OnRemoteRequest, local descriptor is not equal to remote");
        return ERR_INVALID_STATE;
    }

    ErrCode errCode;
    BluetoothA2dpSrcStub::Code msgCode = static_cast<BluetoothA2dpSrcStub::Code>(code);
    switch (msgCode) {
        case BluetoothA2dpSrcStub::Code::BT_GET_DEVICE_STATE:
            errCode = GetDeviceStateInner(data, reply);
            break;
        case BluetoothA2dpSrcStub::Code::BT_REGISTER_OBSERVER:
            errCode = RegisterObserverInner(data, reply);
            break;
        default:
            HILOGW("BluetoothA2dpSrcStub: no member func supporting, applying default process");
            return IPCObjectStub::OnRemoteRequest(code, data, reply, option);
    }

    return errCode;
}

ErrCode BluetoothA2dpSrcStub::RegisterObserverInner(MessageParcel &data, MessageParcel &reply)
{
    sptr<IRemoteObject> remote = data.ReadRemoteObject();
    const sptr<IBluetoothA2dpSrcObserver> observer = OHOS::iface_cast<IBluetoothA2dpSrcObserver>(remote);
    RegisterObserver(observer);

    return NO_ERROR;
}

ErrCode BluetoothA2dpSrcStub::GetDeviceStateInner(MessageParcel &data, MessageParcel &reply)
{
    std::string addr = data.ReadString();

    int result = GetDeviceState(RawAddress(addr));

    bool ret = reply.WriteInt32(result);
    if (!ret) {
        HILOGE("BluetoothA2dpSrcStub: GetDeviceStateInner reply writing failed in: %{public}s.", __func__);
        return TRANSACTION_ERR;
    }

    return NO_ERROR;
}
}  // namespace Bluetooth
}  // namespace OHOS