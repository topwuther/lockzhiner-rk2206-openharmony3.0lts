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

#include "bluetooth_a2dp_source_server.h"
#include "bluetooth_log.h"
#include "interface_profile_manager.h"
#include "interface_profile_a2dp_src.h"
#include "remote_observer_list.h"

namespace OHOS {
namespace Bluetooth {
class A2dpSrcObserverImpl final : public bluetooth::IA2dpObserver {
public:
    A2dpSrcObserverImpl() = default;
    ~A2dpSrcObserverImpl() = default;

    void Register(const sptr<IBluetoothA2dpSrcObserver> &observer)
    {
        observers_.Register(observer);
    }

    void OnPlayingStatusChaned(const RawAddress &device, int playingState, int error) override
    {
        observers_.ForEach([device, playingState, error](sptr<IBluetoothA2dpSrcObserver> observer) {
            observer->OnPlayingStateChanged(device, playingState, error);
        });
    }

private:
    RemoteObserverList<IBluetoothA2dpSrcObserver> observers_;
};

A2dpSrcObserverImpl g_A2dpSrcObserver;

void BluetoothA2dpSourceServer::Init()
{
    bluetooth::IProfileA2dpSrc::GetSrcProfile()->RegisterObserver(&g_A2dpSrcObserver);
}

void BluetoothA2dpSourceServer::Destroy()
{
    bluetooth::IProfileA2dpSrc::GetSrcProfile()->DeregisterObserver(&g_A2dpSrcObserver);
}

int BluetoothA2dpSourceServer::GetDeviceState(const RawAddress &device)
{
    IProfileA2dpSrc* a2dpSrc =
        (IProfileA2dpSrc*)IProfileManager::GetInstance()->GetProfileService(PROFILE_NAME_A2DP_SRC);

    return a2dpSrc->GetDeviceState(device);
}

void BluetoothA2dpSourceServer::RegisterObserver(const sptr<IBluetoothA2dpSrcObserver> &observer)
{
    HILOGI("BluetoothA2dpSourceServer::RegisterObserver starts");
    g_A2dpSrcObserver.Register(observer);
}
}  // namespace Bluetooth
}  // namespace OHOS