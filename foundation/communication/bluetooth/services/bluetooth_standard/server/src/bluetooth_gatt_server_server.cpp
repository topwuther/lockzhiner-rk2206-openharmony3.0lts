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

#include "bluetooth_gatt_server_server.h"
#include "bluetooth_log.h"
#include "gatt_data.h"
#include "i_bluetooth_gatt_server.h"
#include "interface_adapter_manager.h"
#include "interface_profile_gatt_server.h"
#include "interface_profile_manager.h"
#include "interface_adapter_manager.h"
#include "hilog/log.h"

namespace OHOS {
namespace Bluetooth {
struct BluetoothGattServerServer::impl {
    class GattServerCallbackImpl;
    class SystemStateObserver;

    bluetooth::IProfileGattServer *serverService_;
    std::unique_ptr<SystemStateObserver> systemStateObserver_;
    std::list<std::unique_ptr<GattServerCallbackImpl>> callbacks_;
    std::mutex registerMutex_;

    impl();
    ~impl();

    bluetooth::IProfileGattServer *GetServicePtr()
    {
        if (bluetooth::IProfileManager::GetInstance() == nullptr) {
            return nullptr;
        }
        return static_cast<bluetooth::IProfileGattServer *>(
            bluetooth::IProfileManager::GetInstance()->GetProfileService(bluetooth::PROFILE_NAME_GATT_SERVER));
    }
};
class BluetoothGattServerServer::impl::SystemStateObserver : public bluetooth::ISystemStateObserver {
public:
    SystemStateObserver(BluetoothGattServerServer::impl *impl) : impl_(impl){};
    ~SystemStateObserver() override = default;

    void OnSystemStateChange(const bluetooth::BTSystemState state) override
    {
        std::lock_guard<std::mutex> lck(impl_->registerMutex_);
        switch (state) {
            case bluetooth::BTSystemState::ON:
                impl_->serverService_ = impl_->GetServicePtr();
                break;
            case bluetooth::BTSystemState::OFF:
                impl_->serverService_ = nullptr;
                break;
            default:
                break;
        }
    }

private:
    BluetoothGattServerServer::impl *impl_;
};

class BluetoothGattServerServer::impl::GattServerCallbackImpl : public bluetooth::IGattServerCallback {
public:
    void OnCharacteristicReadRequest(
        const bluetooth::GattDevice &device, const bluetooth::Characteristic &characteristic) override
    {
        callback_->OnCharacteristicReadRequest(
            (BluetoothGattDevice)device, (BluetoothGattCharacteristic)characteristic);
    }
    void OnCharacteristicReadByUuidRequest(
        const bluetooth::GattDevice &device, const bluetooth::Characteristic &characteristic) override
    {}
    void OnCharacteristicWriteRequest(const bluetooth::GattDevice &device,
        const bluetooth::Characteristic &characteristic, bool needRespones) override
    {
        callback_->OnCharacteristicWriteRequest(
            (BluetoothGattDevice)device, (BluetoothGattCharacteristic)characteristic, needRespones);
    }
    void OnDescriptorReadRequest(const bluetooth::GattDevice &device, const bluetooth::Descriptor &descriptor) override
    {
        callback_->OnDescriptorReadRequest((BluetoothGattDevice)device, (BluetoothGattDescriptor)descriptor);
    }
    void OnDescriptorWriteRequest(const bluetooth::GattDevice &device, const bluetooth::Descriptor &descriptor) override
    {
        callback_->OnDescriptorWriteRequest((BluetoothGattDevice)device, (BluetoothGattDescriptor)descriptor);
    }
    void OnNotifyConfirm(
        const bluetooth::GattDevice &device, const bluetooth::Characteristic &characteristic, int result) override
    {
        callback_->OnNotifyConfirm((BluetoothGattDevice)device, (BluetoothGattCharacteristic)characteristic, result);
    }
    void OnConnectionStateChanged(const bluetooth::GattDevice &device, int ret, int state) override
    {
        callback_->OnConnectionStateChanged((BluetoothGattDevice)device, ret, state);
    }
    void OnMtuChanged(const bluetooth::GattDevice &device, int mtu) override
    {
        callback_->OnMtuChanged((BluetoothGattDevice)device, mtu);
    }
    void OnAddService(int ret, const bluetooth::Service &services) override
    {
        callback_->OnAddService(ret, (BluetoothGattService)services);
    }
    void OnServiceChanged(const bluetooth::Service &services) override
    {}
    void OnConnectionParameterChanged(
        const bluetooth::GattDevice &device, int interval, int latency, int timeout, int status) override
    {
        callback_->OnConnectionParameterChanged((BluetoothGattDevice)device, interval, latency, timeout, status);
    }

    sptr<IBluetoothGattServerCallback> GetCallback()
    {
        return callback_;
    }
    GattServerCallbackImpl(const sptr<IBluetoothGattServerCallback> &callback, BluetoothGattServerServer &owner);
    ~GattServerCallbackImpl()
    {
        callback_ = nullptr;
        deathRecipient_ = nullptr;
    };

private:
    class GattServerCallbackDeathRecipient : public IRemoteObject::DeathRecipient {
    public:
        GattServerCallbackDeathRecipient(
            const sptr<IBluetoothGattServerCallback> &callback, BluetoothGattServerServer &owner);

        sptr<IBluetoothGattServerCallback> Getcallback() const
        {
            return callback_;
        };

        void OnRemoteDied(const wptr<IRemoteObject> &remote) override;

    private:
        sptr<IBluetoothGattServerCallback> callback_;
        BluetoothGattServerServer &owner_;
    };

    sptr<IBluetoothGattServerCallback> callback_;
    sptr<GattServerCallbackDeathRecipient> deathRecipient_;
};
BluetoothGattServerServer::impl::GattServerCallbackImpl::GattServerCallbackImpl(
    const sptr<IBluetoothGattServerCallback> &callback, BluetoothGattServerServer &owner)
    : callback_(callback), deathRecipient_(new GattServerCallbackDeathRecipient(callback, owner))
{}

BluetoothGattServerServer::impl::GattServerCallbackImpl::GattServerCallbackDeathRecipient::
GattServerCallbackDeathRecipient(const sptr<IBluetoothGattServerCallback> &callback, BluetoothGattServerServer &owner)
    : callback_(callback), owner_(owner)
{}

void BluetoothGattServerServer::impl::GattServerCallbackImpl::GattServerCallbackDeathRecipient::OnRemoteDied(
    const wptr<IRemoteObject> &remote)
{
    for (auto it = owner_.pimpl->callbacks_.begin(); it != owner_.pimpl->callbacks_.end(); ++it) {
        if ((*it)->GetCallback() == iface_cast<IBluetoothGattServerCallback>(remote.promote())) {
            *it = nullptr;
            owner_.pimpl->callbacks_.erase(it);
            return;
        }
    }
}
BluetoothGattServerServer::impl::impl() : serverService_(nullptr), systemStateObserver_(new SystemStateObserver(this))
{
    bluetooth::IAdapterManager::GetInstance()->RegisterSystemStateObserver(*systemStateObserver_);
}

BluetoothGattServerServer::impl::~impl()
{
    bluetooth::IAdapterManager::GetInstance()->DeregisterSystemStateObserver(*systemStateObserver_);
}

int BluetoothGattServerServer::AddService(int32_t appId, BluetoothGattService *services)
{
    std::lock_guard<std::mutex> lck(pimpl->registerMutex_);
    if (!pimpl->serverService_) {
        return bluetooth::GattStatus::REQUEST_NOT_SUPPORT;
    }
    bluetooth::Service svc = (bluetooth::Service)*services;
    
    return pimpl->serverService_->AddService(appId, svc);
}

void BluetoothGattServerServer::ClearServices(int appId)
{
    std::lock_guard<std::mutex> lck(pimpl->registerMutex_);
    if (!pimpl->serverService_) {
        return;
    }
    pimpl->serverService_->ClearServices(appId);
}

void BluetoothGattServerServer::CancelConnection(const BluetoothGattDevice &device)
{
    std::lock_guard<std::mutex> lck(pimpl->registerMutex_);
    if (!pimpl->serverService_) {
        return;
    }
    pimpl->serverService_->CancelConnection((bluetooth::GattDevice)device);
}

int BluetoothGattServerServer::NotifyClient(
    const BluetoothGattDevice &device, BluetoothGattCharacteristic *characteristic, bool needConfirm)
{
    std::lock_guard<std::mutex> lck(pimpl->registerMutex_);
    if (!pimpl->serverService_) {
        return bluetooth::GattStatus::REQUEST_NOT_SUPPORT;
    }
    bluetooth::Characteristic character(characteristic->handle_);
    character.length_ = characteristic->length_;
    character.value_ = std::move(characteristic->value_);
    characteristic->length_ = 0;

    return pimpl->serverService_->NotifyClient((bluetooth::GattDevice)device, character, needConfirm);
}

int BluetoothGattServerServer::RemoveService(int32_t appId, const BluetoothGattService &services)
{
    std::lock_guard<std::mutex> lck(pimpl->registerMutex_);
    if (!pimpl->serverService_) {
        return bluetooth::GattStatus::REQUEST_NOT_SUPPORT;
    }

    return pimpl->serverService_->RemoveService(appId, (bluetooth::Service)services);
}

int BluetoothGattServerServer::RespondCharacteristicRead(
    const BluetoothGattDevice &device, BluetoothGattCharacteristic *characteristic, int32_t ret)
{
    std::lock_guard<std::mutex> lck(pimpl->registerMutex_);
    if (!pimpl->serverService_) {
        return bluetooth::GattStatus::REQUEST_NOT_SUPPORT;
    }
    bluetooth::Characteristic character(characteristic->handle_);
    character.length_ = characteristic->length_;
    character.value_ = std::move(characteristic->value_);
    characteristic->length_ = 0;

    return pimpl->serverService_->RespondCharacteristicRead((bluetooth::GattDevice)device, character, ret);
}

int BluetoothGattServerServer::RespondCharacteristicWrite(
    const BluetoothGattDevice &device, const BluetoothGattCharacteristic &characteristic, int32_t ret)
{
    std::lock_guard<std::mutex> lck(pimpl->registerMutex_);
    if (!pimpl->serverService_) {
        return bluetooth::GattStatus::REQUEST_NOT_SUPPORT;
    }

    return pimpl->serverService_->RespondCharacteristicWrite(
        (bluetooth::GattDevice)device, (bluetooth::Characteristic)characteristic, ret);
}

int BluetoothGattServerServer::RespondDescriptorRead(
    const BluetoothGattDevice &device, BluetoothGattDescriptor *descriptor, int32_t ret)
{
    std::lock_guard<std::mutex> lck(pimpl->registerMutex_);
    if (!pimpl->serverService_) {
        return bluetooth::GattStatus::REQUEST_NOT_SUPPORT;
    }
    bluetooth::Descriptor desc(descriptor->handle_);
    desc.length_ = descriptor->length_;
    desc.value_ = std::move(descriptor->value_);
    descriptor->length_ = 0;

    return pimpl->serverService_->RespondDescriptorRead((bluetooth::GattDevice)device, desc, ret);
}

int BluetoothGattServerServer::RespondDescriptorWrite(
    const BluetoothGattDevice &device, const BluetoothGattDescriptor &descriptor, int32_t ret)
{
    std::lock_guard<std::mutex> lck(pimpl->registerMutex_);
    if (!pimpl->serverService_) {
        return bluetooth::GattStatus::REQUEST_NOT_SUPPORT;
    }

    return pimpl->serverService_->RespondDescriptorWrite(
        (bluetooth::GattDevice)device, (bluetooth::Descriptor)descriptor, ret);
}

int BluetoothGattServerServer::RegisterApplication(const sptr<IBluetoothGattServerCallback> &callback)
{
    std::lock_guard<std::mutex> lck(pimpl->registerMutex_);
    pimpl->serverService_ = pimpl->GetServicePtr();
    if (!pimpl->serverService_) {
        return bluetooth::GattStatus::REQUEST_NOT_SUPPORT;
    }

    auto it = pimpl->callbacks_.emplace(
        pimpl->callbacks_.begin(), std::make_unique<impl::GattServerCallbackImpl>(callback, *this));

    return pimpl->serverService_->RegisterApplication(*it->get());
}

int BluetoothGattServerServer::DeregisterApplication(int32_t appId)
{
    std::lock_guard<std::mutex> lck(pimpl->registerMutex_);
    if (!pimpl->serverService_) {
        return bluetooth::GattStatus::REQUEST_NOT_SUPPORT;
    }

    return pimpl->serverService_->DeregisterApplication(appId);
}
BluetoothGattServerServer::BluetoothGattServerServer() : pimpl(new impl())
{
    HILOGI("Bluetooth Gatt Server Server Created!");
}

BluetoothGattServerServer::~BluetoothGattServerServer()
{}
}  // namespace Bluetooth
}  // namespace OHOS