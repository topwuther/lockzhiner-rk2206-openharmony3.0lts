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

#include "bluetooth_host_server.h"

#include <thread>

#include "bluetooth_a2dp_source_server.h"
#include "bluetooth_ble_advertiser_server.h"
#include "bluetooth_ble_central_manager_server.h"
#include "bluetooth_gatt_client_server.h"
#include "bluetooth_gatt_server_server.h"
#include "bluetooth_log.h"
#include "bluetooth_socket_server.h"
#include "interface_adapter_manager.h"

#include "interface_adapter_ble.h"
#include "interface_adapter_classic.h"
#include "interface_profile_manager.h"
#include "raw_address.h"
#include "remote_observer_list.h"
#include "string_ex.h"
#include "system_ability_definition.h"

namespace OHOS {
namespace Bluetooth {
using namespace bluetooth;
struct BluetoothHostServer::impl {
    impl();
    ~impl();
    void Init();
    void Clear();
    /// service ptr
    IAdapterClassic *classicService_ = nullptr;
    IAdapterBle *bleService_ = nullptr;

    /// sys state observer
    class SystemStateObserver;
    std::unique_ptr<SystemStateObserver> systemStateObserver_ = nullptr;

    /// adapter state observer
    class AdapterStateObserver;
    std::unique_ptr<AdapterStateObserver> observerImp_ = nullptr;

    /// classic observer
    class AdapterClassicObserver;
    std::unique_ptr<AdapterClassicObserver> classicObserverImp_ = nullptr;

    /// classic remote device observer
    class ClassicRemoteDeviceObserver;
    std::unique_ptr<ClassicRemoteDeviceObserver> remoteObserverImp_ = nullptr;

    /// ble observer
    class AdapterBleObserver;
    std::unique_ptr<AdapterBleObserver> bleObserverImp_ = nullptr;

    /// ble remote device observer
    class BlePeripheralCallback;
    std::unique_ptr<BlePeripheralCallback> bleRemoteObserverImp_ = nullptr;

    /// user regist observers
    RemoteObserverList<IBluetoothHostObserver> observers_;
    RemoteObserverList<IBluetoothHostObserver> bleObservers_;

    /// user regist remote observers
    RemoteObserverList<IBluetoothRemoteDeviceObserver> remoteObservers_;

    /// user regist remote observers
    RemoteObserverList<IBluetoothBlePeripheralObserver> bleRemoteObservers_;

    std::map<std::string, sptr<IRemoteObject>> servers_;
    std::map<std::string, sptr<IRemoteObject>> bleServers_;

private:
    void createServers();
};

class BluetoothHostServer::impl::SystemStateObserver : public ISystemStateObserver {
public:
    SystemStateObserver(BluetoothHostServer::impl *impl) : impl_(impl){};
    ~SystemStateObserver() override = default;

    void OnSystemStateChange(const BTSystemState state) override
    {
        if (!impl_) {
            HILOGD("BluetoothHostServer::impl::SystemStateObserver::OnSystemStateChange failed: impl_ is null");
            return;
        }
        switch (state) {
            case BTSystemState::ON:
                /// update service ptr
                impl_->classicService_ = static_cast<IAdapterClassic *>(
                    IAdapterManager::GetInstance()->GetAdapter(BTTransport::ADAPTER_BREDR));
                impl_->bleService_ =
                    static_cast<IAdapterBle *>(IAdapterManager::GetInstance()->GetAdapter(BTTransport::ADAPTER_BLE));
                if (impl_->classicService_) {
                    impl_->classicService_->RegisterClassicAdapterObserver(
                        *(IAdapterClassicObserver *)impl_->classicObserverImp_.get());
                    impl_->classicService_->RegisterRemoteDeviceObserver(
                        *(IClassicRemoteDeviceObserver *)impl_->remoteObserverImp_.get());
                }
                if (impl_->bleService_) {
                    impl_->bleService_->RegisterBleAdapterObserver(
                        *(IAdapterBleObserver *)impl_->bleObserverImp_.get());
                    impl_->bleService_->RegisterBlePeripheralCallback(
                        *(IBlePeripheralCallback *)impl_->bleRemoteObserverImp_.get());
                }
                break;

            case BTSystemState::OFF:
                if (impl_->classicService_) {
                    impl_->classicService_->DeregisterClassicAdapterObserver(
                        *(IAdapterClassicObserver *)impl_->classicObserverImp_.get());
                    impl_->classicService_->DeregisterRemoteDeviceObserver(
                        *(IClassicRemoteDeviceObserver *)impl_->remoteObserverImp_.get());
                    impl_->classicService_ = nullptr;
                }
                if (impl_->bleService_) {
                    impl_->bleService_->DeregisterBleAdapterObserver(
                        *(IAdapterBleObserver *)impl_->bleObserverImp_.get());
                    impl_->bleService_->DeregisterBlePeripheralCallback(
                        *(IBlePeripheralCallback *)impl_->bleRemoteObserverImp_.get());
                    impl_->bleService_ = nullptr;
                }
                break;
            default:
                break;
        }
    }

private:
    BluetoothHostServer::impl *impl_ = nullptr;
};

class BluetoothHostServer::impl::AdapterStateObserver : public IAdapterStateObserver {
public:
    AdapterStateObserver(BluetoothHostServer::impl *impl) : impl_(impl){};
    ~AdapterStateObserver() override = default;

    void OnStateChange(const BTTransport transport, const BTStateID state) override
    {
        if (!impl_) {
            return;
        }
        if (transport == BTTransport::ADAPTER_BREDR) {
            impl_->observers_.ForEach([transport, state](sptr<IBluetoothHostObserver> observer) {
                observer->OnStateChanged(transport, state);
            });
        } else if (transport == BTTransport::ADAPTER_BLE) {
            impl_->bleObservers_.ForEach([transport, state](sptr<IBluetoothHostObserver> observer) {
                observer->OnStateChanged(transport, state);
            });
        }
    };

private:
    BluetoothHostServer::impl *impl_ = nullptr;
    BLUETOOTH_DISALLOW_COPY_AND_ASSIGN(AdapterStateObserver);
};

class BluetoothHostServer::impl::AdapterClassicObserver : public IAdapterClassicObserver {
public:
    AdapterClassicObserver(BluetoothHostServer::impl *impl) : impl_(impl){};
    ~AdapterClassicObserver() override = default;

    void OnDiscoveryStateChanged(const int32_t status) override
    {
        impl_->observers_.ForEach([status](sptr<IBluetoothHostObserver> observer) {
            observer->OnDiscoveryStateChanged(static_cast<int32_t>(status));
        });
    }

    void OnDiscoveryResult(const RawAddress &device) override
    {
        impl_->observers_.ForEach([device](IBluetoothHostObserver *observer) { observer->OnDiscoveryResult(device); });
    }

    void OnPairRequested(const BTTransport transport, const RawAddress &device) override
    {
        impl_->observers_.ForEach(
            [transport, device](IBluetoothHostObserver *observer) { observer->OnPairRequested(transport, device); });
    }

    void OnPairConfirmed(
        const BTTransport transport, const RawAddress &device, int32_t reqType, int32_t number) override
    {
        impl_->observers_.ForEach([transport, device, reqType, number](IBluetoothHostObserver *observer) {
            observer->OnPairConfirmed(transport, device, reqType, number);
        });
    }

    void OnScanModeChanged(int32_t mode) override
    {
        impl_->observers_.ForEach([mode](IBluetoothHostObserver *observer) { observer->OnScanModeChanged(mode); });
    }

    void OnDeviceNameChanged(const std::string &deviceName) override
    {
        impl_->observers_.ForEach(
            [deviceName](IBluetoothHostObserver *observer) { observer->OnDeviceNameChanged(deviceName); });
    }

    void OnDeviceAddrChanged(const std::string &address) override
    {
        impl_->observers_.ForEach(
            [address](IBluetoothHostObserver *observer) { observer->OnDeviceAddrChanged(address); });
    }

private:
    BluetoothHostServer::impl *impl_ = nullptr;
    BLUETOOTH_DISALLOW_COPY_AND_ASSIGN(AdapterClassicObserver);
};
class BluetoothHostServer::impl::ClassicRemoteDeviceObserver : public IClassicRemoteDeviceObserver {
public:
    ClassicRemoteDeviceObserver(BluetoothHostServer::impl *impl) : impl_(impl){};
    ~ClassicRemoteDeviceObserver() override = default;

    void OnPairStatusChanged(const BTTransport transport, const RawAddress &device, const int32_t status) override
    {
        impl_->remoteObservers_.ForEach([transport, device, status](IBluetoothRemoteDeviceObserver *observer) {
            observer->OnPairStatusChanged(transport, device, status);
        });
    }

    void OnRemoteUuidChanged(const RawAddress &device, const std::vector<Uuid> &uuids) override
    {
        std::vector<bluetooth::Uuid> btUuids;
        for (const auto &val : uuids) {
            btUuids.push_back(val);
        }
        impl_->remoteObservers_.ForEach([device, btUuids](IBluetoothRemoteDeviceObserver *observer) {
            observer->OnRemoteUuidChanged(device, btUuids);
        });
    }

    void OnRemoteNameChanged(const RawAddress &device, const std::string &deviceName) override
    {
        impl_->remoteObservers_.ForEach([device, deviceName](IBluetoothRemoteDeviceObserver *observer) {
            observer->OnRemoteNameChanged(device, deviceName);
        });
    }

    void OnRemoteAliasChanged(const RawAddress &device, const std::string &alias) override
    {
        impl_->remoteObservers_.ForEach([device, alias](IBluetoothRemoteDeviceObserver *observer) {
            observer->OnRemoteAliasChanged(device, alias);
        });
    }

    void OnRemoteCodChanged(const RawAddress &device, int32_t cod) override
    {
        impl_->remoteObservers_.ForEach(
            [device, cod](IBluetoothRemoteDeviceObserver *observer) { observer->OnRemoteCodChanged(device, cod); });
    }

    void OnRemoteBatteryLevelChanged(const RawAddress &device, const int32_t batteryLevel) override
    {
        impl_->remoteObservers_.ForEach([device, batteryLevel](IBluetoothRemoteDeviceObserver *observer) {
            observer->OnRemoteBatteryLevelChanged(device, batteryLevel);
        });
    }

private:
    BluetoothHostServer::impl *impl_ = nullptr;
    BLUETOOTH_DISALLOW_COPY_AND_ASSIGN(ClassicRemoteDeviceObserver);
};
class BluetoothHostServer::impl::AdapterBleObserver : public IAdapterBleObserver {
public:
    AdapterBleObserver(BluetoothHostServer::impl *impl) : impl_(impl){};
    ~AdapterBleObserver() override = default;

    void OnDiscoveryStateChanged(const int32_t status) override
    {
        impl_->bleObservers_.ForEach([status](sptr<IBluetoothHostObserver> observer) {
            observer->OnDiscoveryStateChanged(static_cast<int32_t>(status));
        });
    }

    void OnDiscoveryResult(const RawAddress &device) override
    {
        impl_->bleObservers_.ForEach(
            [device](IBluetoothHostObserver *observer) { observer->OnDiscoveryResult(device); });
    }

    void OnPairRequested(const BTTransport transport, const RawAddress &device) override
    {
        impl_->bleObservers_.ForEach(
            [transport, device](IBluetoothHostObserver *observer) { observer->OnPairRequested(transport, device); });
    }

    void OnPairConfirmed(
        const BTTransport transport, const RawAddress &device, const int32_t reqType, const int32_t number) override
    {
        impl_->bleObservers_.ForEach([transport, device, reqType, number](IBluetoothHostObserver *observer) {
            observer->OnPairConfirmed(transport, device, reqType, number);
        });
    }

    void OnScanModeChanged(const int32_t mode) override
    {
        impl_->bleObservers_.ForEach([mode](IBluetoothHostObserver *observer) { observer->OnScanModeChanged(mode); });
    }

    void OnDeviceNameChanged(const std::string deviceName) override
    {
        impl_->bleObservers_.ForEach(
            [deviceName](IBluetoothHostObserver *observer) { observer->OnDeviceNameChanged(deviceName); });
    }

    void OnDeviceAddrChanged(const std::string address) override
    {
        impl_->bleObservers_.ForEach(
            [address](IBluetoothHostObserver *observer) { observer->OnDeviceAddrChanged(address); });
    }

    void OnAdvertisingStateChanged(const int32_t state) override
    {}

private:
    BluetoothHostServer::impl *impl_ = nullptr;
    BLUETOOTH_DISALLOW_COPY_AND_ASSIGN(AdapterBleObserver);
};

class BluetoothHostServer::impl::BlePeripheralCallback : public IBlePeripheralCallback {
public:
    BlePeripheralCallback(BluetoothHostServer::impl *impl) : impl_(impl){};
    ~BlePeripheralCallback() override = default;

    void OnReadRemoteRssiEvent(const RawAddress &device, int32_t rssi, int32_t status) override
    {
        impl_->bleRemoteObservers_.ForEach([device, rssi, status](IBluetoothBlePeripheralObserver *observer) {
            observer->OnReadRemoteRssiEvent(device, rssi, status);
        });
    }

    void OnPairStatusChanged(const BTTransport transport, const RawAddress &device, int32_t status) override
    {
        impl_->bleRemoteObservers_.ForEach([transport, device, status](IBluetoothBlePeripheralObserver *observer) {
            observer->OnPairStatusChanged(transport, device, status);
        });
    }

private:
    BluetoothHostServer::impl *impl_ = nullptr;
    BLUETOOTH_DISALLOW_COPY_AND_ASSIGN(BlePeripheralCallback);
};

std::mutex BluetoothHostServer::instanceLock;
sptr<BluetoothHostServer> BluetoothHostServer::instance;

const bool REGISTER_RESULT = SystemAbility::MakeAndRegisterAbility(BluetoothHostServer::GetInstance().GetRefPtr());

BluetoothHostServer::impl::impl()
{
    HILOGI("BluetoothHostServer::impl::impl() starts");
    systemStateObserver_ = std::make_unique<SystemStateObserver>(this);
    observerImp_ = std::make_unique<AdapterStateObserver>(this);
    classicObserverImp_ = std::make_unique<AdapterClassicObserver>(this);
    remoteObserverImp_ = std::make_unique<ClassicRemoteDeviceObserver>(this);
    bleObserverImp_ = std::make_unique<AdapterBleObserver>(this);
    bleRemoteObserverImp_ = std::make_unique<BlePeripheralCallback>(this);
}

BluetoothHostServer::impl::~impl()
{
    HILOGI("BluetoothHostServer::impl::~impl() starts");
}

void BluetoothHostServer::impl::Init()
{
    HILOGI("BluetoothHostServer::impl::Init starts");

    classicService_ =
        static_cast<IAdapterClassic *>(IAdapterManager::GetInstance()->GetAdapter(BTTransport::ADAPTER_BREDR));
    bleService_ = static_cast<IAdapterBle *>(IAdapterManager::GetInstance()->GetAdapter(BTTransport::ADAPTER_BLE));

    IAdapterManager::GetInstance()->RegisterSystemStateObserver(*systemStateObserver_);

    IAdapterManager::GetInstance()->Start();
    IAdapterManager::GetInstance()->RegisterStateObserver(*observerImp_);
    if (classicService_) {
        classicService_->RegisterClassicAdapterObserver(*classicObserverImp_.get());
        classicService_->RegisterRemoteDeviceObserver(*remoteObserverImp_.get());
    }

    if (bleService_) {
        bleService_->RegisterBleAdapterObserver(*bleObserverImp_.get());
        bleService_->RegisterBlePeripheralCallback(*bleRemoteObserverImp_.get());
    }

    createServers();
}

void BluetoothHostServer::impl::Clear()
{
    /// systerm state observer
    IAdapterManager::GetInstance()->DeregisterSystemStateObserver(*systemStateObserver_);

    /// adapter state observer
    IAdapterManager::GetInstance()->Stop();
    IAdapterManager::GetInstance()->DeregisterStateObserver(*observerImp_);

    if (classicService_) {
        /// classic observer
        classicService_->DeregisterClassicAdapterObserver(*classicObserverImp_.get());
        /// classic remote observer
        classicService_->DeregisterRemoteDeviceObserver(*remoteObserverImp_.get());
    }

    if (bleService_) {
        /// ble observer
        bleService_->DeregisterBleAdapterObserver(*bleObserverImp_.get());
        /// ble remote observer
        bleService_->DeregisterBlePeripheralCallback(*bleRemoteObserverImp_.get());
    }

    if (!servers_[PROFILE_A2DP_SRC]) {
        sptr<BluetoothA2dpSourceServer> a2dpSrc =
            OHOS::iface_cast<BluetoothA2dpSourceServer>(servers_[PROFILE_A2DP_SRC]);
        a2dpSrc->Destroy();
    }
}

void BluetoothHostServer::impl::createServers()
{
    sptr<BluetoothSocketServer> socket = new BluetoothSocketServer();
    servers_[PROFILE_SPP] = socket->AsObject();

    sptr<BluetoothA2dpSourceServer> a2dpSrc = new BluetoothA2dpSourceServer();
    a2dpSrc->Init();
    servers_[PROFILE_A2DP_SRC] = a2dpSrc->AsObject();

    sptr<BluetoothGattServerServer> gattserver = new BluetoothGattServerServer();
    servers_[PROFILE_GATT_SERVER] = gattserver->AsObject();

    sptr<BluetoothGattClientServer> gattclient = new BluetoothGattClientServer();
    servers_[PROFILE_GATT_CLIENT] = gattclient->AsObject();

    sptr<BluetoothBleAdvertiserServer> bleAdvertiser = new BluetoothBleAdvertiserServer();
    bleServers_[BLE_ADVERTISER_SERVER] = bleAdvertiser->AsObject();

    sptr<BluetoothBleCentralManagerServer> bleCentralManger = new BluetoothBleCentralManagerServer();
    bleServers_[BLE_CENTRAL_MANAGER_SERVER] = bleCentralManger->AsObject();

    HILOGI("BluetoothHostServer::impl::Init servers_ constructed, size is %{public}d", servers_.size());
}

BluetoothHostServer::BluetoothHostServer() : SystemAbility(BLUETOOTH_HOST_SYS_ABILITY_ID, true)
{
    pimpl = std::make_unique<impl>();
}

BluetoothHostServer::~BluetoothHostServer()
{
    HILOGI("BluetoothHostServer: ~BluetoothHostServer called.");
}

sptr<BluetoothHostServer> BluetoothHostServer::GetInstance()
{
    std::lock_guard<std::mutex> autoLock(instanceLock);
    if (instance == nullptr) {
        sptr<BluetoothHostServer> temp = new BluetoothHostServer();
        instance = temp;
    }
    return instance;
}

void BluetoothHostServer::OnStart()
{
    HILOGD("BluetoothHostServer: starting service.");
    if (state_ == ServiceRunningState::STATE_RUNNING) {
        HILOGD("BluetoothHostServer: service is already started.");
        return;
    }

    if (!Init()) {
        HILOGE("BluetoothHostServer: initiation failed");
        OnStop();
        return;
    }

    state_ = ServiceRunningState::STATE_RUNNING;

    HILOGD("BluetoothHostServer: service has been started successfully");
    return;
}

bool BluetoothHostServer::Init()
{
    if (!registeredToService_) {
        bool ret = Publish(BluetoothHostServer::GetInstance());
        if (!ret) {
            HILOGE("BluetoothHostServer: init publish failed!");
            return false;
        }
        registeredToService_ = true;
    }
    HILOGD("BluetoothHostServer: init success");
    pimpl->Init();
    return true;
}

void BluetoothHostServer::OnStop()
{
    HILOGD("BluetoothHostServer: stopping service.");

    pimpl->Clear();
    state_ = ServiceRunningState::STATE_IDLE;
    registeredToService_ = false;
    return;
}

void BluetoothHostServer::RegisterObserver(const sptr<IBluetoothHostObserver> &observer)
{
    if (observer == nullptr) {
        HILOGE("BluetoothHostServer: RegisterObserver observer is null");
        return;
    }
    pimpl->observers_.Register(observer);
}

void BluetoothHostServer::DeregisterObserver(const sptr<IBluetoothHostObserver> &observer)
{
    if (observer == nullptr) {
        HILOGE("BluetoothHostServer: DeregisterObserver observer is null");
        return;
    }
    pimpl->observers_.Deregister(observer);
}

bool BluetoothHostServer::EnableBt()
{
    return IAdapterManager::GetInstance()->Enable(bluetooth::BTTransport::ADAPTER_BREDR);
}

bool BluetoothHostServer::DisableBt()
{
    return IAdapterManager::GetInstance()->Disable(bluetooth::BTTransport::ADAPTER_BREDR);
}

int32_t BluetoothHostServer::GetBtState()
{
    return IAdapterManager::GetInstance()->GetState(bluetooth::BTTransport::ADAPTER_BREDR);
}

sptr<IRemoteObject> BluetoothHostServer::GetProfile(const std::string &name)
{
    HILOGI("BluetoothHostServer::GetProfile seraching %{public}s ", name.c_str());
    auto it = pimpl->servers_.find(name);
    if (it != pimpl->servers_.end()) {
        HILOGI("BluetoothHostServer::GetProfile server serached %{public}s ", name.c_str());
        return pimpl->servers_[name];
    } else {
        return nullptr;
    }
}

sptr<IRemoteObject> BluetoothHostServer::GetBleRemote(const std::string &name)
{
    HILOGI("BluetoothHostServer::GetBleRemote %{public}s ", name.c_str());
    auto iter = pimpl->bleServers_.find(name);
    if (iter != pimpl->bleServers_.end()) {
        return pimpl->bleServers_[name];
    } else {
        return nullptr;
    }
}

// Fac_Res_CODE
bool BluetoothHostServer::BluetoothFactoryReset()
{
    return IAdapterManager::GetInstance()->FactoryReset();
}

// Get_Profile_CODE
int32_t BluetoothHostServer::GetDeviceType(int32_t transport, const std::string &address)
{
    RawAddress addr(address);
    if ((transport_ == BT_TRANSPORT_BREDR) && IsBtEnabled()) {
        return pimpl->classicService_->GetDeviceType(addr);
    } else if ((transport_ == BT_TRANSPORT_BREDR) && IsBleEnabled()) {
        return pimpl->bleService_->GetDeviceType(addr);
    } else {
        HILOGE("[%{public}s]: %{public}s() Parameter::transport invalid or BT current state is not enabled!",
            __FILE__,
            __FUNCTION__);
    }
    return INVALID_VALUE;
}

std::string BluetoothHostServer::GetLocalAddress()
{
    HILOGD("[%{public}s]: %{public}s(): Enter!", __FILE__, __FUNCTION__);
    if (IsBtEnabled()) {
        return pimpl->classicService_->GetLocalAddress();
    } else if (IsBleEnabled()) {
        return pimpl->bleService_->GetLocalAddress();
    } else {
        HILOGE("BluetoothHostServer: GetLocalAddress failed");
    }
    return INVALID_MAC_ADDRESS;
}

bool BluetoothHostServer::EnableBle()
{
    HILOGD("[%{public}s]: %{public}s(): Enter!", __FILE__, __FUNCTION__);
    return IAdapterManager::GetInstance()->Enable(BTTransport::ADAPTER_BLE);
}

bool BluetoothHostServer::DisableBle()
{
    HILOGD("[%{public}s]: %{public}s(): Enter!", __FILE__, __FUNCTION__);
    return IAdapterManager::GetInstance()->Disable(BTTransport::ADAPTER_BLE);
}

bool BluetoothHostServer::IsBleEnabled()
{
    HILOGD("[%{public}s]: %{public}s(): Enter!", __FILE__, __FUNCTION__);
    return IAdapterManager::GetInstance()->GetState(BTTransport::ADAPTER_BLE) == BTStateID::STATE_TURN_ON;
}

std::vector<uint32_t> BluetoothHostServer::GetProfileList()
{
    HILOGD("[%{public}s]: %{public}s(): Enter!", __FILE__, __FUNCTION__);
    return IProfileManager::GetInstance()->GetProfileServicesList();
}

int32_t BluetoothHostServer::GetMaxNumConnectedAudioDevices()
{
    HILOGD("[%{public}s]: %{public}s(): Enter!", __FILE__, __FUNCTION__);
    return IAdapterManager::GetInstance()->GetMaxNumConnectedAudioDevices();
}

int32_t BluetoothHostServer::GetBtConnectionState()
{
    HILOGD("[%{public}s]: %{public}s(): Enter!", __FILE__, __FUNCTION__);
    return (int32_t)IAdapterManager::GetInstance()->GetAdapterConnectState();
}

int32_t BluetoothHostServer::GetBtProfileConnState(uint32_t profileId)
{
    HILOGD("[%{public}s]: %{public}s(): Enter!", __FILE__, __FUNCTION__);
    return (int32_t)IProfileManager::GetInstance()->GetProfileServiceConnectState(profileId);
}

void BluetoothHostServer::GetLocalSupportedUuids(std::vector<std::string> &uuids)
{
    HILOGD("[%{public}s]: %{public}s(): Enter!", __FILE__, __FUNCTION__);
    IProfileManager::GetInstance()->GetProfileServicesSupportedUuids(uuids);
}

int32_t BluetoothHostServer::GetLocalDeviceClass()
{
    HILOGD("[%{public}s]: %{public}s(): Enter!", __FILE__, __FUNCTION__);
    if (IsBtEnabled()) {
        return pimpl->classicService_->GetLocalDeviceClass();
    } else {
        HILOGW("[%{public}s]: %{public}s() %d: BT current state is not enabled!", __FILE__, __FUNCTION__, __LINE__);
    }
    return 0;
}

bool BluetoothHostServer::SetLocalDeviceClass(const int32_t &deviceClass)
{
    HILOGD("[%{public}s]: %{public}s(): Enter!", __FILE__, __FUNCTION__);
    if (IsBtEnabled()) {
        return pimpl->classicService_->SetLocalDeviceClass(deviceClass);
    } else {
        HILOGW("[%{public}s]: %{public}s() %d: BT current state is not enabled!", __FILE__, __FUNCTION__, __LINE__);
    }
    return false;
}

std::string BluetoothHostServer::GetLocalName()
{
    HILOGD("[%{public}s]: %{public}s(): Enter!", __FILE__, __FUNCTION__);
    if (IsBtEnabled()) {
        return pimpl->classicService_->GetLocalName();
    } else if (IsBleEnabled()) {
        return pimpl->bleService_->GetLocalName();
    } else {
        HILOGW("[%{public}s]: %{public}s() %d: BT current state is not enabled!", __FILE__, __FUNCTION__, __LINE__);
    }
    return INVALID_NAME;
}

bool BluetoothHostServer::SetLocalName(const std::string &name)
{
    HILOGD("[%{public}s]: %{public}s(): Enter!", __FILE__, __FUNCTION__);
    if (IsBtEnabled()) {
        bool ret = pimpl->classicService_->SetLocalName(name);
        if (ret && (IsBleEnabled())) {
            return pimpl->bleService_->SetLocalName(name);
        } else {
            HILOGE("[%{public}s]: %{public}s() failed!", __FILE__, __FUNCTION__);
            return ret;
        }
    } else if (IsBleEnabled()) {
        return pimpl->bleService_->SetLocalName(name);
    } else {
        HILOGW("[%{public}s]: %{public}s() %d: BT current state is not enabled!", __FILE__, __FUNCTION__, __LINE__);
    }
    return false;
}

int32_t BluetoothHostServer::GetBtScanMode()
{
    HILOGD("[%{public}s]: %{public}s(): Enter!", __FILE__, __FUNCTION__);
    if (IsBtEnabled()) {
        return pimpl->classicService_->GetBtScanMode();
    } else {
        HILOGW("[%{public}s]: %{public}s() %d: BT current state is not enabled!", __FILE__, __FUNCTION__, __LINE__);
    }
    return SCAN_MODE_NONE;
}

bool BluetoothHostServer::SetBtScanMode(int32_t mode, int32_t duration)
{
    HILOGD("[%{public}s]: %{public}s(): Enter!", __FILE__, __FUNCTION__);
    if (IsBtEnabled()) {
        return pimpl->classicService_->SetBtScanMode(mode, duration);
    } else {
        HILOGW("[%{public}s]: %{public}s() %d: BT current state is not enabled!", __FILE__, __FUNCTION__, __LINE__);
    }
    return false;
}

int32_t BluetoothHostServer::GetBondableMode(int32_t transport)
{
    HILOGD("[%{public}s]: %{public}s(): Enter!", __FILE__, __FUNCTION__);
    if ((transport == BTTransport::ADAPTER_BREDR) && IsBtEnabled()) {
        return pimpl->classicService_->GetBondableMode();
    } else if ((transport == BTTransport::ADAPTER_BLE) && IsBleEnabled()) {
        return pimpl->bleService_->GetBondableMode();
    } else {
        HILOGE("[%{public}s]: %{public}s(): Parameter::transport invalid or BT current state is not enabled!",
            __FILE__,
            __FUNCTION__);
    }
    return BONDABLE_MODE_OFF;
}

bool BluetoothHostServer::SetBondableMode(int32_t transport, int32_t mode)
{
    HILOGD("[%{public}s]: %{public}s(): Enter!", __FILE__, __FUNCTION__);
    if ((transport == BTTransport::ADAPTER_BREDR) && IsBtEnabled()) {
        return pimpl->classicService_->SetBondableMode(mode);
    } else if ((transport == BTTransport::ADAPTER_BLE) && IsBleEnabled()) {
        return pimpl->bleService_->SetBondableMode(mode);
    } else {
        HILOGE("[%{public}s]: %{public}s() Parameter::transport invalid or BT current state is not enabled!",
            __FILE__,
            __FUNCTION__);
    }
    return false;
}

bool BluetoothHostServer::StartBtDiscovery()
{
    HILOGD("[%{public}s]: %{public}s(): Enter!", __FILE__, __FUNCTION__);
    if (IsBtEnabled()) {
        return pimpl->classicService_->StartBtDiscovery();
    } else {
        HILOGW("[%{public}s]: %{public}s() %d: BT current state is not enabled!", __FILE__, __FUNCTION__, __LINE__);
    }
    return false;
}

bool BluetoothHostServer::CancelBtDiscovery()
{
    HILOGD("[%{public}s]: %{public}s(): Enter!", __FILE__, __FUNCTION__);
    if (IsBtEnabled()) {
        return pimpl->classicService_->CancelBtDiscovery();
    } else {
        HILOGW("[%{public}s]: %{public}s() %d: BT current state is not enabled!", __FILE__, __FUNCTION__, __LINE__);
    }
    return false;
}

bool BluetoothHostServer::IsBtDiscovering(int32_t transport)
{
    HILOGD("[%{public}s]: %{public}s(): Enter!", __FILE__, __FUNCTION__);
    if ((transport == BTTransport::ADAPTER_BREDR) && IsBtEnabled()) {
        return pimpl->classicService_->IsBtDiscovering();
    } else if ((transport == BTTransport::ADAPTER_BLE) && IsBleEnabled()) {
        return pimpl->bleService_->IsBtDiscovering();
    } else {
        HILOGE("[%{public}s]: %{public}s() Parameter::transport invalid or BT current state is not enabled!",
            __FILE__,
            __FUNCTION__);
    }
    return false;
}

long BluetoothHostServer::GetBtDiscoveryEndMillis()
{
    HILOGD("[%{public}s]: %{public}s(): Enter!", __FILE__, __FUNCTION__);
    if (IsBtEnabled()) {
        return pimpl->classicService_->GetBtDiscoveryEndMillis();
    } else {
        HILOGW("[%{public}s]: %{public}s() %d: BT current state is not enabled!", __FILE__, __FUNCTION__, __LINE__);
    }
    return INVALID_VALUE;
}

std::vector<sptr<BluetoothRawAddress>> BluetoothHostServer::GetPairedDevices(int32_t transport)
{
    HILOGD("[%{public}s]: %{public}s(): Enter!", __FILE__, __FUNCTION__);
    std::vector<RawAddress> rawAddrVec;
    if ((transport == BTTransport::ADAPTER_BREDR) && IsBtEnabled()) {
        rawAddrVec = pimpl->classicService_->GetPairedDevices();
    } else if ((transport == BTTransport::ADAPTER_BLE) && IsBleEnabled()) {
        rawAddrVec = pimpl->bleService_->GetPairedDevices();
    } else {
        HILOGE("[%{public}s]: %{public}s() Parameter::transport invalid or BT current state is not enabled!",
            __FILE__,
            __FUNCTION__);
    }

    std::vector<sptr<BluetoothRawAddress>> rawAddrSptrVec;
    for (auto it = rawAddrVec.begin(); it != rawAddrVec.end(); ++it) {
        sptr<BluetoothRawAddress> rawAddrSptr = new BluetoothRawAddress(*it);
        if (rawAddrSptr == nullptr) {
            HILOGE("[%{public}s]: %{public}s() %d: Failed to create BluetoothRawAddress ptr!",
                __FILE__,
                __FUNCTION__,
                __LINE__);
        }
        rawAddrSptrVec.emplace_back(rawAddrSptr);
    }
    return rawAddrSptrVec;
}

bool BluetoothHostServer::RemovePair(int32_t transport, const sptr<BluetoothRawAddress> &device)
{
    HILOGD("[%{public}s]: %{public}s(): Enter!", __FILE__, __FUNCTION__);
    if ((transport == BTTransport::ADAPTER_BREDR) && IsBtEnabled()) {
        return pimpl->classicService_->RemovePair(*device);
    } else if ((transport == BTTransport::ADAPTER_BLE) && IsBleEnabled()) {
        return pimpl->bleService_->RemovePair(*device);
    } else {
        HILOGE("[%{public}s]: %{public}s() Parameter::transport invalid or BT current state is not enabled!",
            __FILE__,
            __FUNCTION__);
    }
    return false;
}

bool BluetoothHostServer::RemoveAllPairs()
{
    HILOGD("[%{public}s]: %{public}s(): Enter!", __FILE__, __FUNCTION__);
    if (BTStateID::STATE_TURN_ON != IAdapterManager::GetInstance()->GetState(BTTransport::ADAPTER_BREDR) &&
        BTStateID::STATE_TURN_ON != IAdapterManager::GetInstance()->GetState(BTTransport::ADAPTER_BLE)) {
        HILOGW("[%{public}s]: %{public}s() %d: BT current state is not enabled!", __FILE__, __FUNCTION__, __LINE__);
        return false;
    }

    bool ret = true;
    if (IsBtEnabled()) {
        ret = pimpl->classicService_->RemoveAllPairs();
        if (!ret) {
            HILOGE("BREDR RemoveAllPairs failed");
        }
    }

    if (IsBleEnabled()) {
        ret &= pimpl->bleService_->RemoveAllPairs();
        if (!ret) {
            HILOGE("BLE RemoveAllPairs failed");
        }
    }
    return ret;
}

int32_t BluetoothHostServer::GetBleMaxAdvertisingDataLength()
{
    HILOGD("[%{public}s]: %{public}s(): Enter!", __FILE__, __FUNCTION__);
    if (IsBleEnabled()) {
        return pimpl->bleService_->GetBleMaxAdvertisingDataLength();
    } else {
        HILOGW("[%{public}s]: %{public}s() %d: BT current state is not enabled!", __FILE__, __FUNCTION__, __LINE__);
    }
    return INVALID_VALUE;
}

int32_t BluetoothHostServer::GetPhonebookPermission(const std::string &address)
{
    HILOGD("[%{public}s]: %{public}s(): Enter!", __FILE__, __FUNCTION__);
    return (int32_t)IAdapterManager::GetInstance()->GetPhonebookPermission(address);
}

bool BluetoothHostServer::SetPhonebookPermission(const std::string &address, int32_t permission)
{
    HILOGD("[%{public}s]: %{public}s(): Enter!", __FILE__, __FUNCTION__);
    return IAdapterManager::GetInstance()->SetPhonebookPermission(address, (BTPermissionType)permission);
}

int32_t BluetoothHostServer::GetMessagePermission(const std::string &address)
{
    HILOGD("[%{public}s]: %{public}s(): Enter!", __FILE__, __FUNCTION__);
    return (int32_t)IAdapterManager::GetInstance()->GetMessagePermission(address);
}

bool BluetoothHostServer::SetMessagePermission(const std::string &address, int32_t permission)
{
    HILOGD("[%{public}s]: %{public}s(): Enter!", __FILE__, __FUNCTION__);
    return IAdapterManager::GetInstance()->SetMessagePermission(address, (BTPermissionType)permission);
}

int32_t BluetoothHostServer::GetPowerMode(const std::string &address)
{
    HILOGD("[%{public}s]: %{public}s(): Enter!", __FILE__, __FUNCTION__);
    return IAdapterManager::GetInstance()->GetPowerMode(address);
}

std::string BluetoothHostServer::GetDeviceName(int32_t transport, const std::string &address)
{
    HILOGD("[%{public}s]: %{public}s(): Enter!", __FILE__, __FUNCTION__);
    RawAddress addr(address);
    if ((transport == BT_TRANSPORT_BREDR) && IsBtEnabled()) {
        return pimpl->classicService_->GetDeviceName(addr);
    } else if ((transport == BT_TRANSPORT_BLE) && IsBleEnabled()) {
        return pimpl->bleService_->GetDeviceName(addr);
    } else {
        HILOGE("[%{public}s]: %{public}s() Parameter::transport invalid or BT current state is not enabled!",
            __FILE__,
            __FUNCTION__);
    }
    return INVALID_NAME;
}

std::string BluetoothHostServer::GetDeviceAlias(const std::string &address)
{
    HILOGD("[%{public}s]: %{public}s(): Enter!", __FILE__, __FUNCTION__);
    if (IsBtEnabled()) {
        RawAddress addr(address);
        return pimpl->classicService_->GetAliasName(addr);
    } else {
        HILOGE("[%{public}s]: %{public}s() BT current state is not enabled", __FILE__, __FUNCTION__);
    }
    return INVALID_NAME;
}

bool BluetoothHostServer::SetDeviceAlias(const std::string &address, const std::string &aliasName)
{
    HILOGD("[%{public}s]: %{public}s(): Enter!", __FILE__, __FUNCTION__);
    if (IsBtEnabled()) {
        RawAddress addr(address);
        return pimpl->classicService_->SetAliasName(addr, aliasName);
    } else {
        HILOGE("[%{public}s]: %{public}s() BT current state is not enabled", __FILE__, __FUNCTION__);
    }
    return false;
}

int32_t BluetoothHostServer::GetDeviceBatteryLevel(const std::string &address)
{
    HILOGD("[%{public}s]: %{public}s(): Enter!", __FILE__, __FUNCTION__);
    if (IsBtEnabled()) {
        RawAddress addr(address);
        return pimpl->classicService_->GetDeviceBatteryLevel(addr);
    } else {
        HILOGE("[%{public}s]: %{public}s() BT current state is not enabled", __FILE__, __FUNCTION__);
    }
    return INVALID_VALUE;
}

int32_t BluetoothHostServer::GetPairState(int32_t transport, const std::string &address)
{
    HILOGD("[%{public}s]: %{public}s(): Enter!", __FILE__, __FUNCTION__);
    RawAddress addr(address);
    if ((transport == BT_TRANSPORT_BREDR) && IsBtEnabled()) {
        return pimpl->classicService_->GetPairState(addr);
    } else if ((transport == BT_TRANSPORT_BLE) && IsBleEnabled()) {
        return pimpl->bleService_->GetPairState(addr);
    } else {
        HILOGE("[%{public}s]: %{public}s() Parameter::transport invalid or BT current state is not enabled!",
            __FILE__,
            __FUNCTION__);
    }
    return INVALID_VALUE;
}

bool BluetoothHostServer::StartPair(int32_t transport, const std::string &address)
{
    HILOGD("[%{public}s]: %{public}s(): Enter!", __FILE__, __FUNCTION__);
    RawAddress addr(address);
    if ((transport == BT_TRANSPORT_BREDR) && IsBtEnabled()) {
        return pimpl->classicService_->StartPair(addr);
    } else if ((transport == BT_TRANSPORT_BLE) && IsBleEnabled()) {
        return pimpl->bleService_->StartPair(addr);
    } else {
        HILOGE("[%{public}s]: %{public}s() Parameter::transport invalid or BT current state is not enabled!",
            __FILE__,
            __FUNCTION__);
    }
    return false;
}

bool BluetoothHostServer::CancelPairing(int32_t transport, const std::string &address)
{
    HILOGD("[%{public}s]: %{public}s(): Enter!", __FILE__, __FUNCTION__);
    RawAddress addr(address);
    if ((transport == BT_TRANSPORT_BREDR) && IsBtEnabled()) {
        return pimpl->classicService_->CancelPairing(addr);
    } else if ((transport == BT_TRANSPORT_BLE) && IsBleEnabled()) {
        return pimpl->bleService_->CancelPairing(addr);
    } else {
        HILOGE("[%{public}s]: %{public}s() Parameter::transport invalid or BT current state is not enabled!",
            __FILE__,
            __FUNCTION__);
    }
    return false;
}

bool BluetoothHostServer::IsBondedFromLocal(int32_t transport, const std::string &address)
{
    HILOGD("[%{public}s]: %{public}s(): Enter!", __FILE__, __FUNCTION__);
    RawAddress addr(address);
    if ((transport == BT_TRANSPORT_BREDR) && IsBtEnabled()) {
        return pimpl->classicService_->IsBondedFromLocal(addr);
    } else if ((transport == BT_TRANSPORT_BLE) && IsBleEnabled()) {
        return pimpl->bleService_->IsBondedFromLocal(addr);
    } else {
        HILOGE("[%{public}s]: %{public}s() Parameter::transport invalid or BT current state is not enabled!",
            __FILE__,
            __FUNCTION__);
    }
    return false;
}

bool BluetoothHostServer::IsAclConnected(int32_t transport, const std::string &address)
{
    HILOGD("[%{public}s]: %{public}s(): Enter!", __FILE__, __FUNCTION__);
    RawAddress addr(address);
    if ((transport == BT_TRANSPORT_BREDR) && IsBtEnabled()) {
        return pimpl->classicService_->IsAclConnected(addr);
    } else if ((transport == BT_TRANSPORT_BLE) && IsBleEnabled()) {
        return pimpl->bleService_->IsAclConnected(addr);
    } else {
        HILOGE("[%{public}s]: %{public}s() Parameter::transport invalid or BT current state is not enabled!",
            __FILE__,
            __FUNCTION__);
    }
    return false;
}

bool BluetoothHostServer::IsAclEncrypted(int32_t transport, const std::string &address)
{
    HILOGD("[%{public}s]: %{public}s(): Enter!", __FILE__, __FUNCTION__);
    RawAddress addr(address);
    if ((transport == BT_TRANSPORT_BREDR) && IsBtEnabled()) {
        return pimpl->classicService_->IsAclEncrypted(addr);
    } else if ((transport == BT_TRANSPORT_BLE) && IsBleEnabled()) {
        return pimpl->bleService_->IsAclEncrypted(addr);
    } else {
        HILOGE("[%{public}s]: %{public}s() Parameter::transport invalid or BT current state is not enabled!",
            __FILE__,
            __FUNCTION__);
    }
    return false;
}

int32_t BluetoothHostServer::GetDeviceClass(const std::string &address)
{
    HILOGD("[%{public}s]: %{public}s(): Enter!", __FILE__, __FUNCTION__);
    if (IsBtEnabled()) {
        RawAddress addr(address);
        return pimpl->classicService_->GetDeviceClass(addr);
    } else {
        HILOGE("[%{public}s]: %{public}s() BT current state is not enabled!", __FILE__, __FUNCTION__);
    }
    return INVALID_VALUE;
}

std::vector<bluetooth::Uuid> BluetoothHostServer::GetDeviceUuids(int32_t transport, const std::string &address)
{
    HILOGD("[%{public}s]: %{public}s(): Enter!", __FILE__, __FUNCTION__);
    std::vector<bluetooth::Uuid> parcelUuids;
    RawAddress addr(address);
    if ((transport == BT_TRANSPORT_BREDR) && IsBtEnabled()) {
        parcelUuids = pimpl->classicService_->GetDeviceUuids(addr);
    } else if ((transport == BT_TRANSPORT_BLE) && IsBleEnabled()) {
        parcelUuids = pimpl->bleService_->GetDeviceUuids(addr);
    } else {
        HILOGE("[%{public}s]: %{public}s() BT current state is not enabled!", __FILE__, __FUNCTION__);
    }
    return parcelUuids;
}

bool BluetoothHostServer::SetDevicePin(const std::string &address, const std::string &pin)
{
    HILOGD("[%{public}s]: %{public}s(): Enter!", __FILE__, __FUNCTION__);
    if (IsBtEnabled()) {
        RawAddress addr(address);
        return pimpl->classicService_->SetDevicePin(addr, pin);
    } else {
        HILOGE("[%{public}s]: %{public}s() BT current state is not enabled!", __FILE__, __FUNCTION__);
    }
    return false;
}

bool BluetoothHostServer::SetDevicePairingConfirmation(int32_t transport, const std::string &address, bool accept)
{
    HILOGD("[%{public}s]: %{public}s(): Enter!", __FILE__, __FUNCTION__);
    RawAddress addr(address);
    if ((transport == BT_TRANSPORT_BREDR) && IsBtEnabled()) {
        return pimpl->classicService_->SetDevicePairingConfirmation(addr, accept);
    } else if ((transport == BT_TRANSPORT_BLE) && IsBleEnabled()) {
        return pimpl->bleService_->SetDevicePairingConfirmation(addr, accept);
    } else {
        HILOGE("[%{public}s]: %{public}s() Parameter::transport invalid or BT current state is not enabled!",
            __FILE__,
            __FUNCTION__);
    }
    return false;
}

bool BluetoothHostServer::SetDevicePasskey(int32_t transport, const std::string &address, int32_t passkey, bool accept)
{
    HILOGD("[%{public}s]: %{public}s(): Enter!", __FILE__, __FUNCTION__);
    RawAddress addr(address);
    if ((transport == BT_TRANSPORT_BREDR) && IsBtEnabled()) {
        return pimpl->classicService_->SetDevicePasskey(addr, passkey, accept);
    } else if ((transport == BT_TRANSPORT_BLE) && IsBleEnabled()) {
        return pimpl->bleService_->SetDevicePasskey(addr, passkey, accept);
    } else {
        HILOGE("[%{public}s]: %{public}s() Parameter::transport invalid or BT current state is not enabled!",
            __FILE__,
            __FUNCTION__);
    }
    return false;
}

bool BluetoothHostServer::PairRequestReply(int32_t transport, const std::string &address, bool accept)
{
    HILOGD("[%{public}s]: %{public}s(): Enter!", __FILE__, __FUNCTION__);
    RawAddress addr(address);
    if ((transport == BT_TRANSPORT_BREDR) && IsBtEnabled()) {
        return pimpl->classicService_->PairRequestReply(addr, accept);
    } else if ((transport == BT_TRANSPORT_BLE) && IsBleEnabled()) {
        return pimpl->bleService_->PairRequestReply(addr, accept);
    } else {
        HILOGE("[%{public}s]: %{public}s() Parameter::transport invalid or BT current state is not enabled!",
            __FILE__,
            __FUNCTION__);
    }
    return false;
}

bool BluetoothHostServer::ReadRemoteRssiValue(const std::string &address)
{
    HILOGD("[%{public}s]: %{public}s(): Enter!", __FILE__, __FUNCTION__);
    if (IsBleEnabled()) {
        RawAddress addr(address);
        return pimpl->bleService_->ReadRemoteRssiValue(addr);
    } else {
        HILOGE("[%{public}s]: %{public}s() BT current state is not enabled!", __FILE__, __FUNCTION__);
    }
    return false;
}

void BluetoothHostServer::RegisterRemoteDeviceObserver(const sptr<IBluetoothRemoteDeviceObserver> &observer)
{
    HILOGD("[%{public}s]: %{public}s(): Enter!", __FILE__, __FUNCTION__);
    if (observer == nullptr) {
        HILOGE("[%{public}s]: %{public}s() observer is nullptr!", __FILE__, __FUNCTION__);
        return;
    }
    pimpl->remoteObservers_.Register(observer);
}

void BluetoothHostServer::DeregisterRemoteDeviceObserver(const sptr<IBluetoothRemoteDeviceObserver> &observer)
{
    HILOGD("[%{public}s]: %{public}s(): Enter!", __FILE__, __FUNCTION__);
    if (observer == nullptr) {
        HILOGE("[%{public}s]: %{public}s() observer is nullptr!", __FILE__, __FUNCTION__);
        return;
    }
    pimpl->remoteObservers_.Deregister(observer);
}

bool BluetoothHostServer::IsBtEnabled()
{
    return GetBtState() == static_cast<int32_t>(bluetooth::BTStateID::STATE_TURN_ON) ? true : false;
}

void BluetoothHostServer::RegisterBleAdapterObserver(const sptr<IBluetoothHostObserver> &observer)
{
    HILOGI("BluetoothHostServer::RegisterBleAdapterObserver start.");

    if (observer == nullptr) {
        HILOGE("[%{public}s]: %{public}s() observer is nullptr!", __FILE__, __FUNCTION__);
        return;
    }
    pimpl->bleObservers_.Register(observer);
}

void BluetoothHostServer::DeregisterBleAdapterObserver(const sptr<IBluetoothHostObserver> &observer)
{
    HILOGI("BluetoothHostServer::DeregisterBleAdapterObserver start.");

    if (observer == nullptr) {
        HILOGE("[%{public}s]: %{public}s() observer is nullptr!", __FILE__, __FUNCTION__);
        return;
    }
    pimpl->bleObservers_.Deregister(observer);
}

void BluetoothHostServer::RegisterBlePeripheralCallback(const sptr<IBluetoothBlePeripheralObserver> &observer)
{
    HILOGI("BluetoothHostServer::RegisterBlePeripheralCallback start.");

    if (observer == nullptr) {
        HILOGE("[%{public}s]: %{public}s() observer is nullptr!", __FILE__, __FUNCTION__);
        return;
    }
    pimpl->bleRemoteObservers_.Register(observer);
}

void BluetoothHostServer::DeregisterBlePeripheralCallback(const sptr<IBluetoothBlePeripheralObserver> &observer)
{
    HILOGI("BluetoothHostServer::DeregisterBlePeripheralCallback start.");

    if (observer == nullptr) {
        HILOGE("[%{public}s]: %{public}s() observer is nullptr!", __FILE__, __FUNCTION__);
        return;
    }
    pimpl->bleRemoteObservers_.Deregister(observer);
}
}  // namespace Bluetooth
}  // namespace OHOS
