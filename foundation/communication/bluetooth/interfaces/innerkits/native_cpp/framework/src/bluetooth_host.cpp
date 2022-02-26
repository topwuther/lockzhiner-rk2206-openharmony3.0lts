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

#include "bluetooth_host.h"

#include "bluetooth_ble_peripheral_observer_stub.h"
#include "bluetooth_host_observer_stub.h"
#include "bluetooth_host_proxy.h"
#include "bluetooth_log.h"
#include "bluetooth_observer_list.h"
#include "bluetooth_remote_device_observer_stub.h"

#include "iservice_registry.h"
#include "system_ability_definition.h"
#include <memory>
#include <unistd.h>

namespace OHOS {
namespace Bluetooth {
struct BluetoothHost::impl {
    impl();
    ~impl();

    // host observer
    class BluetoothHostObserverImp;
    sptr<BluetoothHostObserverImp> observerImp_ = nullptr;
    sptr<BluetoothHostObserverImp> bleObserverImp_ = nullptr;

    // remote device observer
    class BluetoothRemoteDeviceObserverImp;
    sptr<BluetoothRemoteDeviceObserverImp> remoteObserverImp_ = nullptr;

    // remote device observer
    class BluetoothBlePeripheralCallbackImp;
    sptr<BluetoothBlePeripheralCallbackImp> bleRemoteObserverImp_ = nullptr;

    // user regist observers
    BluetoothObserverList<BluetoothHostObserver> observers_;

    // user regist remote observers
    BluetoothObserverList<BluetoothRemoteDeviceObserver> remoteObservers_;

    sptr<IBluetoothHost> proxy_ = nullptr;

private:
    void GetProxy();
};

class BluetoothHost::impl::BluetoothHostObserverImp : public BluetoothHostObserverStub {
public:
    BluetoothHostObserverImp(BluetoothHost::impl &host) : host_(host){};
    ~BluetoothHostObserverImp() override{};

    void Register(std::shared_ptr<BluetoothHostObserver> &observer)
    {
        host_.observers_.Register(observer);
    }

    void Deregister(std::shared_ptr<BluetoothHostObserver> &observer)
    {
        host_.observers_.Deregister(observer);
    }

    void OnStateChanged(int32_t transport, int32_t status) override
    {
        HILOGD("[%{public}s]: %{public}s(): Enter!", __FILE__, __FUNCTION__);
        host_.observers_.ForEach([transport, status](std::shared_ptr<BluetoothHostObserver> observer) {
            observer->OnStateChanged(transport, status);
        });
    }

    void OnDiscoveryStateChanged(int32_t status) override
    {
        HILOGD("[%{public}s]: %{public}s(): Enter!", __FILE__, __FUNCTION__);
        host_.observers_.ForEach(
            [status](std::shared_ptr<BluetoothHostObserver> observer) { observer->OnDiscoveryStateChanged(status); });
    }

    void OnDiscoveryResult(const BluetoothRawAddress &device) override
    {
        HILOGD("[%{public}s]: %{public}s(): Enter!", __FILE__, __FUNCTION__);
        BluetoothRemoteDevice remoteDevice(device.GetAddress(), BTTransport::ADAPTER_BREDR);
        host_.observers_.ForEach([remoteDevice](std::shared_ptr<BluetoothHostObserver> observer) {
            observer->OnDiscoveryResult(remoteDevice);
        });
    }

    void OnPairRequested(const int32_t transport, const BluetoothRawAddress &device) override
    {
        HILOGD("[%{public}s]: %{public}s(): Enter!", __FILE__, __FUNCTION__);
        BluetoothRemoteDevice remoteDevice(device.GetAddress(), transport);
        host_.observers_.ForEach([remoteDevice](std::shared_ptr<BluetoothHostObserver> observer) {
            observer->OnPairRequested(remoteDevice);
        });
    }

    void OnPairConfirmed(const int32_t transport, const BluetoothRawAddress &device, int reqType, int number) override
    {
        HILOGD("[%{public}s]: %{public}s(): Enter!", __FILE__, __FUNCTION__);
        BluetoothRemoteDevice remoteDevice(device.GetAddress(), transport);
        host_.observers_.ForEach([remoteDevice, reqType, number](std::shared_ptr<BluetoothHostObserver> observer) {
            observer->OnPairConfirmed(remoteDevice, reqType, number);
        });
    }

    void OnScanModeChanged(int mode) override
    {
        HILOGD("[%{public}s]: %{public}s(): Enter!", __FILE__, __FUNCTION__);
        host_.observers_.ForEach(
            [mode](std::shared_ptr<BluetoothHostObserver> observer) { observer->OnScanModeChanged(mode); });
    }

    void OnDeviceNameChanged(const std::string &deviceName) override
    {
        HILOGD("[%{public}s]: %{public}s(): Enter!", __FILE__, __FUNCTION__);
        host_.observers_.ForEach([deviceName](std::shared_ptr<BluetoothHostObserver> observer) {
            observer->OnDeviceNameChanged(deviceName);
        });
    }

    void OnDeviceAddrChanged(const std::string &address) override
    {
        HILOGD("[%{public}s]: %{public}s(): Enter!", __FILE__, __FUNCTION__);
        host_.observers_.ForEach(
            [address](std::shared_ptr<BluetoothHostObserver> observer) { observer->OnDeviceAddrChanged(address); });
    }

private:
    BluetoothHost::impl &host_;
    BLUETOOTH_DISALLOW_COPY_AND_ASSIGN(BluetoothHostObserverImp);
};

class BluetoothHost::impl::BluetoothRemoteDeviceObserverImp : public BluetoothRemoteDeviceObserverstub {
public:
    BluetoothRemoteDeviceObserverImp(BluetoothHost::impl &host) : host_(host){};
    ~BluetoothRemoteDeviceObserverImp() override = default;

    void OnPairStatusChanged(const int32_t transport, const BluetoothRawAddress &device, int32_t status) override
    {
        HILOGD("[%{public}s]: %{public}s(): Enter!", __FILE__, __FUNCTION__);
        BluetoothRemoteDevice remoteDevice(device.GetAddress(), transport);

        host_.remoteObservers_.ForEach([remoteDevice, status](std::shared_ptr<BluetoothRemoteDeviceObserver> observer) {
            observer->OnPairStatusChanged(remoteDevice, status);
        });
    }

    void OnRemoteUuidChanged(const BluetoothRawAddress &device, const std::vector<bluetooth::Uuid> uuids) override
    {
        HILOGD("[%{public}s]: %{public}s(): Enter!", __FILE__, __FUNCTION__);
        BluetoothRemoteDevice remoteDevice(device.GetAddress(), BTTransport::ADAPTER_BREDR);
        host_.remoteObservers_.ForEach([remoteDevice, uuids](std::shared_ptr<BluetoothRemoteDeviceObserver> observer) {
            std::vector<ParcelUuid> parcelUuids;
            for (auto &uuid : uuids) {
                ParcelUuid parcelUuid = UUID::ConvertFrom128Bits(uuid.ConvertTo128Bits());
                parcelUuids.push_back(parcelUuid);
                observer->OnRemoteUuidChanged(remoteDevice, parcelUuids);
            }
        });
    }

    void OnRemoteNameChanged(const BluetoothRawAddress &device, const std::string deviceName) override
    {
        HILOGD("[%{public}s]: %{public}s(): Enter!", __FILE__, __FUNCTION__);
        BluetoothRemoteDevice remoteDevice(device.GetAddress(), BTTransport::ADAPTER_BREDR);
        host_.remoteObservers_.ForEach(
            [remoteDevice, deviceName](std::shared_ptr<BluetoothRemoteDeviceObserver> observer) {
                observer->OnRemoteNameChanged(remoteDevice, deviceName);
            });
    }

    void OnRemoteAliasChanged(const BluetoothRawAddress &device, const std::string alias) override
    {
        HILOGD("[%{public}s]: %{public}s(): Enter!", __FILE__, __FUNCTION__);
        BluetoothRemoteDevice remoteDevice(device.GetAddress(), BTTransport::ADAPTER_BREDR);
        host_.remoteObservers_.ForEach([remoteDevice, alias](std::shared_ptr<BluetoothRemoteDeviceObserver> observer) {
            observer->OnRemoteAliasChanged(remoteDevice, alias);
        });
    }

    void OnRemoteCodChanged(const BluetoothRawAddress &device, int32_t cod) override
    {
        HILOGD("[%{public}s]: %{public}s(): Enter!", __FILE__, __FUNCTION__);
        BluetoothRemoteDevice remoteDevice(device.GetAddress(), BTTransport::ADAPTER_BREDR);
        BluetoothDeviceClass deviceClass(cod);
        host_.remoteObservers_.ForEach(
            [remoteDevice, deviceClass](std::shared_ptr<BluetoothRemoteDeviceObserver> observer) {
                observer->OnRemoteCodChanged(remoteDevice, deviceClass);
            });
    }

    void OnRemoteBatteryLevelChanged(const BluetoothRawAddress &device, int32_t batteryLevel) override
    {
        HILOGD("[%{public}s]: %{public}s(): Enter!", __FILE__, __FUNCTION__);
        BluetoothRemoteDevice remoteDevice(device.GetAddress(), BTTransport::ADAPTER_BREDR);
        host_.remoteObservers_.ForEach(
            [remoteDevice, batteryLevel](std::shared_ptr<BluetoothRemoteDeviceObserver> observer) {
                observer->OnRemoteBatteryLevelChanged(remoteDevice, batteryLevel);
            });
    }

private:
    BluetoothHost::impl &host_;
    BLUETOOTH_DISALLOW_COPY_AND_ASSIGN(BluetoothRemoteDeviceObserverImp);
};

class BluetoothHost::impl::BluetoothBlePeripheralCallbackImp : public BluetoothBlePeripheralObserverStub {
public:
    BluetoothBlePeripheralCallbackImp(BluetoothHost::impl &host) : host_(host){};
    ~BluetoothBlePeripheralCallbackImp() override = default;

    void OnPairStatusChanged(const int32_t transport, const BluetoothRawAddress &device, int status) override
    {
        BluetoothRemoteDevice remoteDevice(device.GetAddress(), transport);
        host_.remoteObservers_.ForEach([remoteDevice, status](std::shared_ptr<BluetoothRemoteDeviceObserver> observer) {
            observer->OnPairStatusChanged(remoteDevice, status);
        });
    }

    void OnReadRemoteRssiEvent(const BluetoothRawAddress &device, int rssi, int status) override
    {
        HILOGD("[%{public}s]: %{public}s(): Enter!", __FILE__, __FUNCTION__);
        BluetoothRemoteDevice remoteDevice(device.GetAddress(), BTTransport::ADAPTER_BREDR);
        host_.remoteObservers_.ForEach(
            [remoteDevice, rssi, status](std::shared_ptr<BluetoothRemoteDeviceObserver> observer) {
                observer->OnReadRemoteRssiEvent(remoteDevice, rssi, status);
            });
    }

private:
    BluetoothHost::impl &host_;
    BLUETOOTH_DISALLOW_COPY_AND_ASSIGN(BluetoothBlePeripheralCallbackImp);
};

BluetoothHost::impl::impl()
{
    HILOGD("BluetoothHost::impl::impl  starts");
    observerImp_ = new (std::nothrow) BluetoothHostObserverImp(*this);
    if (observerImp_ == nullptr) {
        return;
    }

    remoteObserverImp_ = new (std::nothrow) BluetoothRemoteDeviceObserverImp(*this);
    if (remoteObserverImp_ == nullptr) {
        return;
    }

    bleRemoteObserverImp_ = new (std::nothrow) BluetoothBlePeripheralCallbackImp(*this);
    if (bleRemoteObserverImp_ == nullptr) {
        return;
    }

    bleObserverImp_ = new (std::nothrow) BluetoothHostObserverImp(*this);
    if (bleObserverImp_ == nullptr) {
        return;
    }

    GetProxy();

    proxy_->RegisterObserver(observerImp_);
    proxy_->RegisterBleAdapterObserver(bleObserverImp_);
    proxy_->RegisterRemoteDeviceObserver(remoteObserverImp_);
    proxy_->RegisterBlePeripheralCallback(bleRemoteObserverImp_);
}

BluetoothHost::impl::~impl()
{
    HILOGD("BluetoothHost::impl::~impl  starts");
    proxy_->DeregisterObserver(observerImp_);
    proxy_->DeregisterBleAdapterObserver(observerImp_);
    proxy_->DeregisterRemoteDeviceObserver(remoteObserverImp_);
    proxy_->DeregisterBlePeripheralCallback(bleRemoteObserverImp_);
}

void BluetoothHost::impl::GetProxy()
{
    sptr<ISystemAbilityManager> samgr = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    if (!samgr) {
        HILOGE("BluetoothHost::impl::impl() error: no samgr");
        return;
    }

    sptr<IRemoteObject> remote = samgr->GetSystemAbility(BLUETOOTH_HOST_SYS_ABILITY_ID);
    if (!remote) {
        int try_time = 5;
        while ((!remote) && (try_time--) > 0) {
            sleep(1);
            remote = samgr->GetSystemAbility(BLUETOOTH_HOST_SYS_ABILITY_ID);
        }
    }

    if (!remote) {
        HILOGE("BluetoothHost::impl::impl() error:no remote");
        return;
    }

    proxy_ = iface_cast<IBluetoothHost>(remote);
    if (!proxy_) {
        HILOGE("BluetoothHost::impl::impl() error:no proxy");
        return;
    }
}

BluetoothHost BluetoothHost::hostAdapter_;

BluetoothHost::BluetoothHost()
{
    pimpl = std::make_unique<impl>();
    if (!pimpl) {
        HILOGE("BluetoothHost::BluetoothHost fails: no pimpl");
    }
}

BluetoothHost::~BluetoothHost()
{}

BluetoothHost &BluetoothHost::GetDefaultHost()
{
    return hostAdapter_;
}

void BluetoothHost::RegisterObserver(BluetoothHostObserver &observer)
{
    if (!pimpl) {
        HILOGE("BluetoothHost::RegisterObserver fails: no pimpl");
        return;
    }
    std::shared_ptr<BluetoothHostObserver> pointer(&observer, [](BluetoothHostObserver *) {});
    pimpl->observers_.Register(pointer);
}

void BluetoothHost::DeregisterObserver(BluetoothHostObserver &observer)
{
    if (!pimpl) {
        HILOGE("BluetoothHost::DeregisterObserver fails: no pimpl");
        return;
    }
    std::shared_ptr<BluetoothHostObserver> pointer(&observer, [](BluetoothHostObserver *) {});
    pimpl->observers_.Deregister(pointer);
}

bool BluetoothHost::EnableBt()
{
    if (!pimpl) {
        HILOGE("BluetoothHost::EnableBt fails: no pimpl");
        return false;
    }
    if (pimpl->proxy_ == nullptr) {
        HILOGE("BluetoothHost::EnableBt fails: no proxy");
        return false;
    }

    return pimpl->proxy_->EnableBt();
}

bool BluetoothHost::DisableBt()
{
    if (!pimpl) {
        HILOGE("BluetoothHost::DisableBt fails: no pimpl");
        return false;
    }
    if (pimpl->proxy_ == nullptr) {
        HILOGE("BluetoothHost::DisableBt fails: no proxy");
        return false;
    }
    return pimpl->proxy_->DisableBt();
}

int BluetoothHost::GetBtState() const
{
    if (!pimpl) {
        HILOGE("BluetoothHost::GetBtState fails: no pimpl");
        return INVALID_VALUE;
    }
    if (pimpl->proxy_ == nullptr) {
        HILOGE("BluetoothHost::GetBtState fails: no proxy");
        return INVALID_VALUE;
    }
    return pimpl->proxy_->GetBtState();
}

bool BluetoothHost::BluetoothFactoryReset()
{
    if (!pimpl) {
        HILOGE("BluetoothHost::BluetoothFactoryReset fails: no pimpl");
        return false;
    }
    if (pimpl->proxy_ == nullptr) {
        HILOGE("BluetoothHost::BluetoothFactoryReset fails: no proxy");
        return false;
    }
    return pimpl->proxy_->BluetoothFactoryReset();
}

bool BluetoothHost::IsValidBluetoothAddr(const std::string &addr)
{
    if (addr.empty() || addr.length() != ADDRESS_LENGTH) {
        return false;
    }
    for (int i = 0; i < ADDRESS_LENGTH; i++) {
        char c = addr[i];
        switch (i % ADDRESS_SEPARATOR_UNIT) {
            case 0:
            case 1:
                if ((c >= '0' && c <= '9') || (c >= 'A' && c <= 'F')) {
                    break;
                }
                return false;
            case ADDRESS_COLON_INDEX:
            default:
                if (c == ':') {
                    break;
                }
                return false;
        }
    }
    return true;
}

BluetoothRemoteDevice BluetoothHost::GetRemoteDevice(const std::string &addr, int transport) const
{
    BluetoothRemoteDevice remoteDevice(addr, transport);
    return remoteDevice;
}

bool BluetoothHost::EnableBle()
{
    HILOGD("BluetoothHost::Enable BLE starts");

    if (!pimpl) {
        HILOGE("BluetoothHost::Enable BLE fails: no pimpl");
        return false;
    }
    if (pimpl->proxy_ == nullptr) {
        HILOGE("BluetoothHost::Enable fails: no proxy");
        return false;
    }
    return pimpl->proxy_->EnableBle();
}

bool BluetoothHost::DisableBle()
{
    if (!pimpl) {
        HILOGE("BluetoothHost::DisableBle BLE fails: no pimpl");
        return false;
    }
    if (pimpl->proxy_ == nullptr) {
        HILOGE("BluetoothHost::DisableBle fails: no proxy");
        return false;
    }
    return pimpl->proxy_->DisableBle();
}

bool BluetoothHost::IsBleEnabled() const
{
    if (!pimpl) {
        HILOGE("BluetoothHost::IsBleEnabled BLE fails: no pimpl");
        return false;
    }
    if (pimpl->proxy_ == nullptr) {
        HILOGE("BluetoothHost::IsBleEnabled fails: no proxy");
        return false;
    }
    return pimpl->proxy_->IsBleEnabled();
}

std::string BluetoothHost::GetLocalAddress() const
{
    if (!pimpl) {
        HILOGE("BluetoothHost::GetLocalAddress BLE fails: no pimpl");
        return std::string();
    }
    if (pimpl->proxy_ == nullptr) {
        HILOGE("BluetoothHost::GetLocalAddress fails: no proxy");
        return std::string();
    }
    return pimpl->proxy_->GetLocalAddress();
}

std::vector<uint32_t> BluetoothHost::GetProfileList() const
{
    std::vector<uint32_t> profileList;
    if (!pimpl) {
        HILOGE("BluetoothHost::GetProfileList BLE fails: no pimpl");
        return profileList;
    }
    if (pimpl->proxy_ == nullptr) {
        HILOGE("BluetoothHost::GetProfileList fails: no proxy");
        return profileList;
    }
    profileList = pimpl->proxy_->GetProfileList();
    return profileList;
}

int BluetoothHost::GetMaxNumConnectedAudioDevices() const
{
    if (!pimpl) {
        HILOGE("BluetoothHost::GetMaxNumConnectedAudioDevices BLE fails: no pimpl");
        return INVALID_VALUE;
    }
    if (pimpl->proxy_ == nullptr) {
        HILOGE("BluetoothHost::GetMaxNumConnectedAudioDevices fails: no proxy");
        return INVALID_VALUE;
    }
    return pimpl->proxy_->GetMaxNumConnectedAudioDevices();
}

int BluetoothHost::GetBtConnectionState() const
{
    if (!pimpl) {
        HILOGE("BluetoothHost::GetBtConnectionState BLE fails: no pimpl");
        return INVALID_VALUE;
    }
    if (pimpl->proxy_ == nullptr) {
        HILOGE("BluetoothHost::GetBtConnectionState fails: no proxy");
        return INVALID_VALUE;
    }
    return pimpl->proxy_->GetBtConnectionState();
}

int BluetoothHost::GetBtProfileConnState(uint32_t profileId) const
{
    if (!pimpl) {
        HILOGE("BluetoothHost::GetBtProfileConnState BLE fails: no pimpl");
        return INVALID_VALUE;
    }
    if (pimpl->proxy_ == nullptr) {
        HILOGE("BluetoothHost::GetBtProfileConnState fails: no proxy");
        return INVALID_VALUE;
    }
    return pimpl->proxy_->GetBtProfileConnState(profileId);
}

void BluetoothHost::GetLocalSupportedUuids(std::vector<ParcelUuid> &uuids)
{
    if (!pimpl) {
        HILOGE("BluetoothHost::GetLocalSupportedUuids BLE fails: no pimpl");
        return;
    }
    if (pimpl->proxy_ == nullptr) {
        HILOGE("BluetoothHost::GetLocalSupportedUuids fails: no proxy");
        return;
    }
    std::vector<std::string> stringUuids;
    pimpl->proxy_->GetLocalSupportedUuids(stringUuids);
    for (auto uuid : stringUuids) {
        uuids.push_back(UUID::FromString(uuid));
    }
}

bool BluetoothHost::Start()
{
    /// This function only used for passthrough, so this is empty.
    return true;
}

void BluetoothHost::Stop()
{
    /// This function only used for passthrough, so this is empty.
}

BluetoothDeviceClass BluetoothHost::GetLocalDeviceClass() const
{
    if (!pimpl) {
        HILOGE("BluetoothHost::GetLocalDeviceClass BLE fails: no pimpl");
        return BluetoothDeviceClass(0);
    }
    if (pimpl->proxy_ == nullptr) {
        HILOGE("BluetoothHost::GetLocalDeviceClass fails: no proxy");
        return BluetoothDeviceClass(0);
    }
    int LocalDeviceClass = pimpl->proxy_->GetLocalDeviceClass();
    return BluetoothDeviceClass(LocalDeviceClass);
}

bool BluetoothHost::SetLocalDeviceClass(const BluetoothDeviceClass &deviceClass)
{
    if (!pimpl) {
        HILOGE("BluetoothHost::SetLocalDeviceClass BLE fails: no pimpl");
        return false;
    }
    if (pimpl->proxy_ == nullptr) {
        HILOGE("BluetoothHost::SetLocalDeviceClass fails: no proxy");
        return false;
    }
    int cod = deviceClass.GetClassOfDevice();
    return pimpl->proxy_->SetLocalDeviceClass(cod);
}

std::string BluetoothHost::GetLocalName() const
{
    if (!pimpl) {
        HILOGE("BluetoothHost::GetLocalName BLE fails: no pimpl");
        return std::string();
    }
    if (pimpl->proxy_ == nullptr) {
        HILOGE("BluetoothHost::GetLocalName fails: no proxy");
        return std::string();
    }
    return pimpl->proxy_->GetLocalName();
}

bool BluetoothHost::SetLocalName(const std::string &name)
{
    if (!pimpl) {
        HILOGE("BluetoothHost::SetLocalName BLE fails: no pimpl");
        return false;
    }
    if (pimpl->proxy_ == nullptr) {
        HILOGE("BluetoothHost::SetLocalName fails: no proxy");
        return false;
    }
    return pimpl->proxy_->SetLocalName(name);
}

int BluetoothHost::GetBtScanMode() const
{
    if (!pimpl) {
        HILOGE("BluetoothHost::GetBtScanMode BLE fails: no pimpl");
        return INVALID_VALUE;
    }
    if (pimpl->proxy_ == nullptr) {
        HILOGE("BluetoothHost::GetBtScanMode fails: no proxy");
        return INVALID_VALUE;
    }
    return pimpl->proxy_->GetBtScanMode();
}

bool BluetoothHost::SetBtScanMode(int mode, int duration)
{
    if (!pimpl) {
        HILOGE("BluetoothHost::SetBtScanMode BLE fails: no pimpl");
        return false;
    }
    if (pimpl->proxy_ == nullptr) {
        HILOGE("BluetoothHost::SetBtScanMode fails: no proxy");
        return false;
    }
    return pimpl->proxy_->SetBtScanMode(mode, duration);
}

int BluetoothHost::GetBondableMode(int transport) const
{
    if (!pimpl) {
        HILOGE("BluetoothHost::GetBondableMode BLE fails: no pimpl");
        return INVALID_VALUE;
    }
    if (pimpl->proxy_ == nullptr) {
        HILOGE("BluetoothHost::GetBondableMode fails: no proxy");
        return INVALID_VALUE;
    }
    return pimpl->proxy_->GetBondableMode(transport);
}

bool BluetoothHost::SetBondableMode(int transport, int mode)
{
    if (!pimpl) {
        HILOGE("BluetoothHost::SetBondableMode BLE fails: no pimpl");
        return false;
    }
    if (pimpl->proxy_ == nullptr) {
        HILOGE("BluetoothHost::SetBondableMode fails: no proxy");
        return false;
    }
    return pimpl->proxy_->SetBondableMode(transport, mode);
}

bool BluetoothHost::StartBtDiscovery()
{
    if (!pimpl) {
        HILOGE("BluetoothHost::StartBtDiscovery BLE fails: no pimpl");
        return false;
    }
    if (pimpl->proxy_ == nullptr) {
        HILOGE("BluetoothHost::StartBtDiscovery fails: no proxy");
        return false;
    }
    return pimpl->proxy_->StartBtDiscovery();
}

bool BluetoothHost::CancelBtDiscovery()
{
    if (!pimpl) {
        HILOGE("BluetoothHost::CancelBtDiscovery BLE fails: no pimpl");
        return false;
    }
    if (pimpl->proxy_ == nullptr) {
        HILOGE("BluetoothHost::CancelBtDiscovery fails: no proxy");
        return false;
    }
    return pimpl->proxy_->CancelBtDiscovery();
}

bool BluetoothHost::IsBtDiscovering(int transport) const
{
    if (!pimpl) {
        HILOGE("BluetoothHost::IsBtDiscovering BLE fails: no pimpl");
        return false;
    }
    if (pimpl->proxy_ == nullptr) {
        HILOGE("BluetoothHost::IsBtDiscovering fails: no proxy");
        return false;
    }
    return pimpl->proxy_->IsBtDiscovering(transport);
}

long BluetoothHost::GetBtDiscoveryEndMillis() const
{
    if (!pimpl) {
        HILOGE("BluetoothHost::GetBtDiscoveryEndMillis BLE fails: no pimpl");
        return 0;
    }
    if (pimpl->proxy_ == nullptr) {
        HILOGE("BluetoothHost::GetBtDiscoveryEndMillis fails: no proxy");
        return 0;
    }
    return pimpl->proxy_->GetBtDiscoveryEndMillis();
}

std::vector<BluetoothRemoteDevice> BluetoothHost::GetPairedDevices(int transport) const
{
    std::vector<BluetoothRemoteDevice> pairedDevices;
    if (!pimpl) {
        HILOGE("BluetoothHost::GetPairedDevices BLE fails: no pimpl");
        return pairedDevices;
    }
    if (pimpl->proxy_ == nullptr) {
        HILOGE("BluetoothHost::GetPairedDevices fails: no proxy");
        return pairedDevices;
    }
    std::vector<sptr<BluetoothRawAddress>> pairedAddr = pimpl->proxy_->GetPairedDevices(transport);

    for (auto it = pairedAddr.begin(); it != pairedAddr.end(); it++) {
        if (*it != nullptr) {
            BluetoothRemoteDevice device((*it)->GetAddress(), transport);
            pairedDevices.emplace_back(device);
        }
    }
    return pairedDevices;
}

bool BluetoothHost::RemovePair(const BluetoothRemoteDevice &device)
{
    if (!device.IsValidBluetoothRemoteDevice()) {
        HILOGE("RemovePair::Invalid remote device.");
        return false;
    }
    if (!pimpl) {
        HILOGE("BluetoothHost::RemovePair BLE fails: no pimpl");
        return false;
    }
    if (pimpl->proxy_ == nullptr) {
        HILOGE("BluetoothHost::RemovePair fails: no proxy");
        return false;
    }
    sptr<BluetoothRawAddress> rawAddrSptr = new BluetoothRawAddress(device.GetDeviceAddr());
    return pimpl->proxy_->RemovePair(device.GetTransportType(), rawAddrSptr);
}

bool BluetoothHost::RemoveAllPairs()
{
    if (!pimpl) {
        HILOGE("BluetoothHost::RemoveAllPairs BLE fails: no pimpl");
        return false;
    }
    if (pimpl->proxy_ == nullptr) {
        HILOGE("BluetoothHost::RemoveAllPairs fails: no proxy");
        return false;
    }
    return pimpl->proxy_->RemoveAllPairs();
}

void BluetoothHost::RegisterRemoteDeviceObserver(BluetoothRemoteDeviceObserver &observer)
{
    if (!pimpl) {
        HILOGE("BluetoothHost::RegisterRemoteDeviceObserver fails: no pimpl");
        return;
    }
    std::shared_ptr<BluetoothRemoteDeviceObserver> pointer(&observer, [](BluetoothRemoteDeviceObserver *) {});
    pimpl->remoteObservers_.Register(pointer);
}

void BluetoothHost::DeregisterRemoteDeviceObserver(BluetoothRemoteDeviceObserver &observer)
{
    if (!pimpl) {
        HILOGE("BluetoothHost::DeregisterRemoteDeviceObserver fails: no pimpl");
        return;
    }
    std::shared_ptr<BluetoothRemoteDeviceObserver> pointer(&observer, [](BluetoothRemoteDeviceObserver *) {});
    pimpl->remoteObservers_.Deregister(pointer);
}

int BluetoothHost::GetBleMaxAdvertisingDataLength() const
{
    if (!pimpl) {
        HILOGE("BluetoothHost::GetBleMaxAdvertisingDataLength BLE fails: no pimpl");
        return INVALID_VALUE;
    }
    if (pimpl->proxy_ == nullptr) {
        HILOGE("BluetoothHost::GetBleMaxAdvertisingDataLength fails: no proxy");
        return INVALID_VALUE;
    }
    return pimpl->proxy_->GetBleMaxAdvertisingDataLength();
}
}  // namespace Bluetooth
}  // namespace OHOS
