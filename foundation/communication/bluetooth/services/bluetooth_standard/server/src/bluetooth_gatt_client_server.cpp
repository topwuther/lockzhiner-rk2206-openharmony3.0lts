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

#include <list>
#include <mutex>
#include <thread>

#include "bluetooth_a2dp_source_server.h"
#include "bluetooth_gatt_client_server.h"
#include "bluetooth_log.h"
#include "bluetooth_socket_server.h"
#include "interface_adapter_ble.h"
#include "interface_adapter_classic.h"
#include "interface_adapter_manager.h"
#include "interface_profile_gatt_client.h"
#include "interface_profile_manager.h"
#include "string_ex.h"
#include "system_ability_definition.h"

using namespace bluetooth;
namespace OHOS {
namespace Bluetooth {
struct BluetoothGattClientServer::impl {
    class GattClientCallbackImpl;
    class SystemStateObserver;

    IProfileGattClient *clientService_;
    std::unique_ptr<SystemStateObserver> systemStateObserver_;
    std::list<std::unique_ptr<GattClientCallbackImpl>> callbacks_;
    std::mutex registerMutex_;

    impl();
    ~impl();

    IProfileGattClient *GetServicePtr()
    {
        if (IProfileManager::GetInstance() == nullptr) {
            return nullptr;
        }
        return static_cast<IProfileGattClient *>(
            IProfileManager::GetInstance()->GetProfileService(PROFILE_NAME_GATT_CLIENT));
    }
};

class BluetoothGattClientServer::impl::SystemStateObserver : public ISystemStateObserver {
public:
    SystemStateObserver(BluetoothGattClientServer::impl *impl) : impl_(impl){};
    ~SystemStateObserver() override = default;

    void OnSystemStateChange(const BTSystemState state) override
    {
        std::lock_guard<std::mutex> lck(impl_->registerMutex_);
        switch (state) {
            case BTSystemState::ON:
                impl_->clientService_ = impl_->GetServicePtr();
                break;
            case BTSystemState::OFF:
                impl_->clientService_ = nullptr;
                break;
            default:
                break;
        }
    }

private:
    BluetoothGattClientServer::impl *impl_;
};

class BluetoothGattClientServer::impl::GattClientCallbackImpl : public IGattClientCallback {
public:
    void OnConnectionStateChanged(int state, int newState) override
    {
        HILOGI("Bluetooth Gatt Client Server OnConnectionStateChanged Triggered!");
        callback_->OnConnectionStateChanged(state, newState);
    }

    void OnCharacteristicChanged(const Characteristic &characteristic) override
    {
        HILOGI("Bluetooth Gatt Client Server OnCharacteristicChanged Triggered!");
        callback_->OnCharacteristicChanged((BluetoothGattCharacteristic)characteristic);
    }

    void OnCharacteristicRead(int ret, const Characteristic &characteristic) override
    {
        HILOGI("Bluetooth Gatt Client Server OnCharacteristicRead Triggered!");
        callback_->OnCharacteristicRead(ret, (BluetoothGattCharacteristic)characteristic);
    }

    void OnCharacteristicWrite(int ret, const Characteristic &characteristic) override
    {
        HILOGI("Bluetooth Gatt Client Server OnCharacteristicWrite Triggered!");
        callback_->OnCharacteristicWrite(ret, (BluetoothGattCharacteristic)characteristic);
    }

    void OnDescriptorRead(int ret, const Descriptor &descriptor) override
    {
        HILOGI("Bluetooth Gatt Client Server OnDescriptorRead Triggered!");
        callback_->OnDescriptorRead(ret, (BluetoothGattDescriptor)descriptor);
    }

    void OnDescriptorWrite(int ret, const Descriptor &descriptor) override
    {
        HILOGI("Bluetooth Gatt Client Server OnDescriptorWrite Triggered!");
        callback_->OnDescriptorWrite(ret, (BluetoothGattDescriptor)descriptor);
    }

    void OnMtuChanged(int state, int mtu) override
    {
        HILOGI("Bluetooth Gatt Client Server OnMtuChanged Triggered!");
        callback_->OnMtuChanged(state, mtu);
    }

    void OnServicesDiscovered(int status) override
    {
        HILOGI("Bluetooth Gatt Client Server OnServicesDiscovered Triggered!");
        callback_->OnServicesDiscovered(status);
    }

    void OnConnectionParameterChanged(int interval, int latency, int timeout, int status) override
    {
        HILOGI("Bluetooth Gatt Client Server OnConnectionParameterChanged Triggered!");
        callback_->OnConnectionParameterChanged(interval, latency, timeout, status);
    }

    void OnServicesChanged(const std::vector<Service> &services) override
    {
        HILOGI("Bluetooth Gatt Client Server OnServicesChanged Triggered!");
        std::vector<BluetoothGattService> result;
        int num = services.size();
        for (int i = 0; i < num; i++) {
            result.push_back((BluetoothGattService)services[i]);
        }
        callback_->OnServicesChanged(result);
    }

    sptr<IBluetoothGattClientCallback> GetCallback()
    {
        return callback_;
    }

    GattClientCallbackImpl(const sptr<IBluetoothGattClientCallback> &callback, BluetoothGattClientServer &owner);
    ~GattClientCallbackImpl()
    {
        callback_ = nullptr;
        deathRecipient_ = nullptr;
    };

private:
    class CallbackDeathRecipient : public IRemoteObject::DeathRecipient {
    public:
        CallbackDeathRecipient(const sptr<IBluetoothGattClientCallback> &callback, BluetoothGattClientServer &owner);

        sptr<IBluetoothGattClientCallback> GetCallback() const
        {
            return callback_;
        };

        void OnRemoteDied(const wptr<IRemoteObject> &remote) override;

    private:
        sptr<IBluetoothGattClientCallback> callback_;
        BluetoothGattClientServer &owner_;
    };

    sptr<IBluetoothGattClientCallback> callback_;
    sptr<CallbackDeathRecipient> deathRecipient_;
};

BluetoothGattClientServer::impl::GattClientCallbackImpl::GattClientCallbackImpl(
    const sptr<IBluetoothGattClientCallback> &callback, BluetoothGattClientServer &owner)
    : callback_(callback), deathRecipient_(new CallbackDeathRecipient(callback, owner))
{}

BluetoothGattClientServer::impl::GattClientCallbackImpl::CallbackDeathRecipient::CallbackDeathRecipient(
    const sptr<IBluetoothGattClientCallback> &callback, BluetoothGattClientServer &owner)
    : callback_(callback), owner_(owner)
{}

void BluetoothGattClientServer::impl::GattClientCallbackImpl::CallbackDeathRecipient::OnRemoteDied(
    const wptr<IRemoteObject> &remote)
{
    for (auto it = owner_.pimpl->callbacks_.begin(); it != owner_.pimpl->callbacks_.end(); ++it) {
        if ((*it)->GetCallback() == iface_cast<IBluetoothGattClientCallback>(remote.promote())) {
            *it = nullptr;
            owner_.pimpl->callbacks_.erase(it);
            return;
        }
    }
}

BluetoothGattClientServer::impl::impl() : clientService_(nullptr), systemStateObserver_(new SystemStateObserver(this))
{
    IAdapterManager::GetInstance()->RegisterSystemStateObserver(*systemStateObserver_);
}

BluetoothGattClientServer::impl::~impl()
{
    IAdapterManager::GetInstance()->DeregisterSystemStateObserver(*systemStateObserver_);
}

BluetoothGattClientServer::BluetoothGattClientServer() : pimpl(new impl())
{
    HILOGI("Bluetooth Gatt Client Server pimpl Created!");
}

BluetoothGattClientServer::~BluetoothGattClientServer()
{}

int BluetoothGattClientServer::RegisterApplication(
    const sptr<IBluetoothGattClientCallback> &callback, const BluetoothRawAddress &addr, int32_t transport)
{
    HILOGI("Bluetooth Gatt Client Server RegisterApplication Triggered!");
    std::lock_guard<std::mutex> lck(pimpl->registerMutex_);
    pimpl->clientService_ = pimpl->GetServicePtr();
    if (pimpl->clientService_ == nullptr) {
        HILOGI("BluetoothGattClientServer: RegisterApplication request not support.");
        return bluetooth::GattStatus::REQUEST_NOT_SUPPORT;
    }
    auto it = pimpl->callbacks_.emplace(
        pimpl->callbacks_.begin(), std::make_unique<impl::GattClientCallbackImpl>(callback, *this));
    return pimpl->clientService_->RegisterApplication(*it->get(), (RawAddress)addr, transport);
}

int BluetoothGattClientServer::DeregisterApplication(int32_t appId)
{
    HILOGI("Bluetooth Gatt Client Server DeregisterApplication Triggered!");
    std::lock_guard<std::mutex> lck(pimpl->registerMutex_);
    if (pimpl->clientService_ == nullptr) {
        HILOGI("BluetoothGattClientServer: DeregisterApplication request not support.");
        return bluetooth::GattStatus::REQUEST_NOT_SUPPORT;
    }
    return pimpl->clientService_->DeregisterApplication(appId);
}

int BluetoothGattClientServer::Connect(int32_t appId, bool autoConnect)
{
    HILOGI("Bluetooth Gatt Client Server Connect Triggered!");
    std::lock_guard<std::mutex> lck(pimpl->registerMutex_);
    if (pimpl->clientService_ == nullptr) {
        HILOGI("BluetoothGattClientServer: Connect request not support.");
        return bluetooth::GattStatus::REQUEST_NOT_SUPPORT;
    }
    return pimpl->clientService_->Connect(appId, autoConnect);
}

int BluetoothGattClientServer::Disconnect(int32_t appId)
{
    HILOGI("Bluetooth Gatt Client Server Disconnect Triggered!");
    std::lock_guard<std::mutex> lck(pimpl->registerMutex_);
    if (pimpl->clientService_ == nullptr) {
        HILOGI("BluetoothGattClientServer: Disconnect request not support.");
        return bluetooth::GattStatus::REQUEST_NOT_SUPPORT;
    }
    return pimpl->clientService_->Disconnect(appId);
}

int BluetoothGattClientServer::DiscoveryServices(int32_t appId)
{
    HILOGI("Bluetooth Gatt Client Server DiscoveryServices Triggered!");
    std::lock_guard<std::mutex> lck(pimpl->registerMutex_);
    if (pimpl->clientService_ == nullptr) {
        HILOGI("BluetoothGattClientServer: DiscoveryServices request not support.");
        return bluetooth::GattStatus::REQUEST_NOT_SUPPORT;
    }
    return pimpl->clientService_->DiscoveryServices(appId);
}

int BluetoothGattClientServer::ReadCharacteristic(int32_t appId, const BluetoothGattCharacteristic &characteristic)
{
    HILOGI("Bluetooth Gatt Client Server ReadCharacteristic Triggered!");
    std::lock_guard<std::mutex> lck(pimpl->registerMutex_);
    if (pimpl->clientService_ == nullptr) {
        HILOGI("BluetoothGattClientServer: ReadCharacteristic request not support.");
        return bluetooth::GattStatus::REQUEST_NOT_SUPPORT;
    }
    return pimpl->clientService_->ReadCharacteristic(appId, (Characteristic)characteristic);
}

int BluetoothGattClientServer::WriteCharacteristic(
    int32_t appId, BluetoothGattCharacteristic *characteristic, bool withoutRespond)
{
    HILOGI("Bluetooth Gatt Client Server WriteCharacteristic Triggered!");
    Characteristic character(characteristic->handle_);
    character.length_ = characteristic->length_;
    character.value_ = std::move(characteristic->value_);
    characteristic->length_ = 0;
    std::lock_guard<std::mutex> lck(pimpl->registerMutex_);
    if (pimpl->clientService_ == nullptr) {
        HILOGI("BluetoothGattClientServer: WriteCharacteristic request not support.");
        return bluetooth::GattStatus::REQUEST_NOT_SUPPORT;
    }
    return pimpl->clientService_->WriteCharacteristic(appId, character, withoutRespond);
}
int BluetoothGattClientServer::SignedWriteCharacteristic(int32_t appId, BluetoothGattCharacteristic *characteristic)
{
    HILOGI("Bluetooth Gatt Client Server SignedWriteCharacteristic Triggered!");
    Characteristic character(characteristic->handle_);
    character.length_ = characteristic->length_;
    character.value_ = std::move(characteristic->value_);
    characteristic->length_ = 0;
    std::lock_guard<std::mutex> lck(pimpl->registerMutex_);
    if (pimpl->clientService_ == nullptr) {
        HILOGI("BluetoothGattClientServer: SignedWriteCharacteristic request not support.");
        return bluetooth::GattStatus::REQUEST_NOT_SUPPORT;
    }
    return pimpl->clientService_->SignedWriteCharacteristic(appId, character);
}

int BluetoothGattClientServer::ReadDescriptor(int32_t appId, const BluetoothGattDescriptor &descriptor)
{
    HILOGI("Bluetooth Gatt Client Server ReadDescriptor Triggered!");
    std::lock_guard<std::mutex> lck(pimpl->registerMutex_);
    if (pimpl->clientService_ == nullptr) {
        HILOGI("BluetoothGattClientServer: ReadDescriptor request not support.");
        return bluetooth::GattStatus::REQUEST_NOT_SUPPORT;
    }
    return pimpl->clientService_->ReadDescriptor(appId, (Descriptor)descriptor);
}

int BluetoothGattClientServer::WriteDescriptor(int32_t appId, BluetoothGattDescriptor *descriptor)
{
    HILOGI("Bluetooth Gatt Client Server WriteDescriptor Triggered!");
    Descriptor desc(descriptor->handle_);
    desc.length_ = descriptor->length_;
    desc.value_ = std::move(descriptor->value_);
    descriptor->length_ = 0;
    std::lock_guard<std::mutex> lck(pimpl->registerMutex_);
    if (pimpl->clientService_ == nullptr) {
        HILOGI("BluetoothGattClientServer: WriteDescriptor request not support.");
        return bluetooth::GattStatus::REQUEST_NOT_SUPPORT;
    }
    return pimpl->clientService_->WriteDescriptor(appId, desc);
}

int BluetoothGattClientServer::RequestExchangeMtu(int32_t appId, int32_t mtu)
{
    HILOGI("Bluetooth Gatt Client Server RequestExchangeMtu Triggered!");
    std::lock_guard<std::mutex> lck(pimpl->registerMutex_);
    if (pimpl->clientService_ == nullptr) {
        HILOGI("BluetoothGattClientServer: RequestExchangeMtu request not support.");
        return bluetooth::GattStatus::REQUEST_NOT_SUPPORT;
    }
    return pimpl->clientService_->RequestExchangeMtu(appId, mtu);
}

void BluetoothGattClientServer::GetAllDevice(::std::vector<BluetoothGattDevice> &device)
{
    HILOGI("Bluetooth Gatt Client Server GetAllDevice Triggered!");
    std::lock_guard<std::mutex> lck(pimpl->registerMutex_);
    if (pimpl->clientService_ == nullptr) {
        return;
    }
    for (auto &dev : pimpl->clientService_->GetAllDevice()) {
        device.push_back(dev);
    }
}

int BluetoothGattClientServer::RequestConnectionPriority(int32_t appId, int32_t connPriority)
{
    HILOGI("Bluetooth Gatt Client Server RequestConnectionPriority Triggered!");
    std::lock_guard<std::mutex> lck(pimpl->registerMutex_);
    if (pimpl->clientService_ == nullptr) {
        return 0;
    }
    return pimpl->clientService_->RequestConnectionPriority(appId, connPriority);
}

void BluetoothGattClientServer::GetServices(int32_t appId, ::std::vector<BluetoothGattService> &service)
{
    HILOGI("Bluetooth Gatt Client Server GetServices Triggered!");
    std::lock_guard<std::mutex> lck(pimpl->registerMutex_);
    if (pimpl->clientService_ == nullptr) {
        return;
    }
    for (auto &svc : pimpl->clientService_->GetServices(appId)) {
        service.push_back(svc);
    }
}
}  // namespace Bluetooth
}  // namespace OHOS