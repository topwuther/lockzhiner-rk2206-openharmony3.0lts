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

#include "bluetooth_a2dp_src_observer_stub.h"
#include "bluetooth_log.h"
#include "raw_address.h"

namespace OHOS {
namespace Bluetooth {
using namespace bluetooth;
int32_t BluetoothA2dpSrcObserverStub::OnRemoteRequest(
    uint32_t code, MessageParcel &data, MessageParcel &reply, MessageOption &option)
{
    HILOGD("BluetoothA2dpSrcObserverStub: transaction of code: %{public}d is received", code);

    ErrCode errCode;
    BluetoothA2dpSrcObserverStub::Code msgCode = static_cast<BluetoothA2dpSrcObserverStub::Code>(code);
    switch (msgCode) {
        case BluetoothA2dpSrcObserverStub::Code::BT_A2DP_SRC_OBSERVER_PLAYING_STATE_CHANGED:
            errCode = OnPlayingStateChangedInner(data, reply);
            break;
        default:
            HILOGW("BluetoothA2dpSrcObserverStub: no member func supporting, applying default process");
            return IPCObjectStub::OnRemoteRequest(code, data, reply, option);
    }

    return errCode;
}

ErrCode BluetoothA2dpSrcObserverStub::OnPlayingStateChangedInner(MessageParcel &data, MessageParcel &reply)
{
    std::string addr = data.ReadString();
    int playingState = data.ReadInt32();
    int error = data.ReadInt32();

    OnPlayingStateChanged(RawAddress(addr), playingState, error);

    return NO_ERROR;
}
}  // namespace Bluetooth
}  // namespace OHOS