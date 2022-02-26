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

#include "bluetooth_a2dp_src_observer_proxy.h"
#include "bluetooth_log.h"

namespace OHOS {
namespace Bluetooth {
void BluetoothA2dpSrcObserverProxy::OnPlayingStateChanged(const RawAddress &device, int playingState, int error)
{
    MessageParcel data;
    if (!data.WriteString(device.GetAddress())) {
        HILOGE("BluetoothA2dpSrcObserverProxy::OnStateChanged write device error");
        return;
    }
    if (!data.WriteInt32(playingState)) {
        HILOGE("BluetoothA2dpSrcObserverProxy::OnPlayingStateChanged playingState error");
        return;
    }

    if (!data.WriteInt32(error)) {
        HILOGE("BluetoothA2dpSrcObserverProxy::OnPlayingStateChanged error error");
        return;
    }

    MessageParcel reply;
    MessageOption option {
        MessageOption::TF_ASYNC
    };

    int ret = Remote()->SendRequest(
        IBluetoothA2dpSrcObserver::Code::BT_A2DP_SRC_OBSERVER_PLAYING_STATE_CHANGED, data, reply, option);
    if (ret != NO_ERROR) {
        HILOGE("BluetoothA2dpSrcProxy::OnPlayingStateChanged done fail, error: %{public}d", error);
        return;
    }
}
}  // namespace Bluetooth
}  // namespace OHOS