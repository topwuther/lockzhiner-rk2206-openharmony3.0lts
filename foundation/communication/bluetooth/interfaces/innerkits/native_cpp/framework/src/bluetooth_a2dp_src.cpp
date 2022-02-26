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

#include "bluetooth_a2dp_src.h"
#include "bluetooth_a2dp_src_proxy.h"
#include "bluetooth_a2dp_src_observer_stub.h"
#include "bluetooth_host_proxy.h"
#include "bluetooth_observer_list.h"
#include "raw_address.h"

#include "bluetooth_log.h"
#include "iservice_registry.h"
#include "system_ability_definition.h"

namespace OHOS {
namespace Bluetooth {
using namespace bluetooth;

class BluetoothA2dpSourceObserverImp final : public BluetoothA2dpSrcObserverStub {
public:
    BluetoothA2dpSourceObserverImp() = default;
    ~BluetoothA2dpSourceObserverImp() = default;

    void Register(std::shared_ptr<BluetoothA2dpSrcObserver> &observer)
    {
        observers_.Register(observer);
    }

    void Deregister(std::shared_ptr<BluetoothA2dpSrcObserver> &observer)
    {
        observers_.Deregister(observer);
    }

    void OnPlayingStateChanged(const RawAddress &device, int playingState, int error) override
    {
        observers_.ForEach([device, playingState, error](std::shared_ptr<BluetoothA2dpSrcObserver> observer) {
            observer->OnPlayingStateChanged(BluetoothRemoteDevice(device.GetAddress(), 0), playingState, error);
        });
    }

private:
    BluetoothObserverList<BluetoothA2dpSrcObserver> observers_;
};

struct BluetoothA2dpSrc::impl {
    impl()
    {
        HILOGI("BluetoothA2dpSrc::impl:impl() starts");
        sptr<ISystemAbilityManager> samgr = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
        sptr<IRemoteObject> hostRemote = samgr->GetSystemAbility(BLUETOOTH_HOST_SYS_ABILITY_ID);

        if (!hostRemote) {
            HILOGI("BluetoothA2dpSrc::impl:impl() failed: no hostRemote");
            return;
        }

        sptr<IBluetoothHost> hostProxy = iface_cast<IBluetoothHost>(hostRemote);
        sptr<IRemoteObject> remote = hostProxy->GetProfile(PROFILE_A2DP_SRC);

        if (!remote) {
            HILOGI("BluetoothA2dpSrc::impl:impl() failed: no remote");
            return;
        }
        HILOGI("BluetoothA2dpSrc::impl:impl() remote obtained");

        proxy_ = iface_cast<IBluetoothA2dpSrc>(remote);
        observerImp_ = new BluetoothA2dpSourceObserverImp();

        proxy_->RegisterObserver(observerImp_);
    }
    ~impl()
    {}

    sptr<IBluetoothA2dpSrc> proxy_;
    sptr<BluetoothA2dpSourceObserverImp> observerImp_;
};

BluetoothA2dpSrc BluetoothA2dpSrc::a2dpSrcProfile_;
BluetoothA2dpSrc::BluetoothA2dpSrc() : pimpl(std::make_unique<impl>())
{}
BluetoothA2dpSrc::~BluetoothA2dpSrc()
{}

BluetoothA2dpSrc &BluetoothA2dpSrc::GetProfile()
{
    return a2dpSrcProfile_;
}

int BluetoothA2dpSrc::GetDeviceState(const BluetoothRemoteDevice &device) const
{
    return pimpl->proxy_->GetDeviceState(RawAddress(device.GetDeviceAddr()));
}

void BluetoothA2dpSrc::RegisterObserver(std::shared_ptr<BluetoothA2dpSrcObserver> &observer)
{
    pimpl->observerImp_->Register(observer);
}

void BluetoothA2dpSrc::DeregisterObserver(std::shared_ptr<BluetoothA2dpSrcObserver> &observer)
{
    pimpl->observerImp_->Deregister(observer);
}
}  // namespace Bluetooth
}  // namespace OHOS