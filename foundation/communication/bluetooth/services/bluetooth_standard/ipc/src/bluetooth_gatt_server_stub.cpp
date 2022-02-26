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

#include "bluetooth_gatt_server_stub.h"
#include "bluetooth_log.h"
#include "ipc_types.h"
#include "string_ex.h"

namespace OHOS {
namespace Bluetooth {
BluetoothGattServerStub::BluetoothGattServerStub()
{
    HILOGD("%{public}s start.", __func__);
    memberFuncMap_[static_cast<uint32_t>(BluetoothGattServerStub::Code::GATT_SERVER_ADD_SERVICE)] =
        &BluetoothGattServerStub::AddServiceInner;
    memberFuncMap_[static_cast<uint32_t>(BluetoothGattServerStub::Code::GATT_SERVER_CLEAR_SERVICES)] =
        &BluetoothGattServerStub::ClearServicesInner;
    memberFuncMap_[static_cast<uint32_t>(BluetoothGattServerStub::Code::GATT_SERVER_CANCEL_CONNECTION)] =
        &BluetoothGattServerStub::CancelConnectionInner;
    memberFuncMap_[static_cast<uint32_t>(BluetoothGattServerStub::Code::GATT_SERVER_REGISTER)] =
        &BluetoothGattServerStub::RegisterApplicationInner;
    memberFuncMap_[static_cast<uint32_t>(BluetoothGattServerStub::Code::GATT_SERVER_DEREGISTER)] =
        &BluetoothGattServerStub::DeregisterApplicationInner;
    memberFuncMap_[static_cast<uint32_t>(BluetoothGattServerStub::Code::GATT_SERVER_NOTIFY_CLIENT)] =
        &BluetoothGattServerStub::NotifyClientInner;
    memberFuncMap_[static_cast<uint32_t>(BluetoothGattServerStub::Code::GATT_SERVER_REMOVE_SERVICE)] =
        &BluetoothGattServerStub::RemoveServiceInner;
    memberFuncMap_[static_cast<uint32_t>(BluetoothGattServerStub::Code::GATT_SERVER_RESPOND_CHARACTERISTIC_READ)] =
        &BluetoothGattServerStub::RespondCharacteristicReadInner;
    memberFuncMap_[static_cast<uint32_t>(BluetoothGattServerStub::Code::GATT_SERVER_RESPOND_CHARACTERISTIC_WRITE)] =
        &BluetoothGattServerStub::RespondCharacteristicWriteInner;
    memberFuncMap_[static_cast<uint32_t>(BluetoothGattServerStub::Code::GATT_SERVER_RESPOND_DESCRIPTOR_READ)] =
        &BluetoothGattServerStub::RespondDescriptorReadInner;
    memberFuncMap_[static_cast<uint32_t>(BluetoothGattServerStub::Code::GATT_SERVER_RESPOND_DESCRIPTOR_WRITE)] =
        &BluetoothGattServerStub::RespondDescriptorWriteInner;
}

BluetoothGattServerStub::~BluetoothGattServerStub()
{
    HILOGD("%{public}s start.", __func__);
    memberFuncMap_.clear();
}
int BluetoothGattServerStub::OnRemoteRequest(
    uint32_t code, MessageParcel &data, MessageParcel &reply, MessageOption &option)
{
    HILOGD("BluetoothGattServerStub::OnRemoteRequest, cmd = %{public}d, flags= %{public}d", code, option.GetFlags());
    std::u16string descriptor = BluetoothGattServerStub::GetDescriptor();
    std::u16string remoteDescriptor = data.ReadInterfaceToken();
    if (descriptor != remoteDescriptor) {
        HILOGI("local descriptor is not equal to remote");
        return ERR_INVALID_STATE;
    }

    auto itFunc = memberFuncMap_.find(code);
    if (itFunc != memberFuncMap_.end()) {
        auto memberFunc = itFunc->second;
        if (memberFunc != nullptr) {
            return (this->*memberFunc)(data, reply);
        }
    }
    HILOGW("BluetoothGattServerStub::OnRemoteRequest, default case, need check.");
    return IPCObjectStub::OnRemoteRequest(code, data, reply, option);
};
ErrCode BluetoothGattServerStub::AddServiceInner(MessageParcel &data, MessageParcel &reply)
{
    int32_t appID = data.ReadInt32();
    BluetoothGattService *services = data.ReadParcelable<BluetoothGattService>();
    int result = AddService(appID, services);
    bool ret = reply.WriteInt32(result);
    if (!ret) {
        HILOGE("BluetoothGattServerStub: reply writing failed in: %{public}s.", __func__);
        return ERR_INVALID_VALUE;
    }
    return NO_ERROR;
}
ErrCode BluetoothGattServerStub::ClearServicesInner(MessageParcel &data, MessageParcel &reply)
{
    int appId = data.ReadInt32();
    ClearServices(appId);
    return NO_ERROR;
}

ErrCode BluetoothGattServerStub::CancelConnectionInner(MessageParcel &data, MessageParcel &reply)
{
    const BluetoothGattDevice *device = data.ReadParcelable<BluetoothGattDevice>();
    CancelConnection(*device);
    delete device;

    return NO_ERROR;
}
ErrCode BluetoothGattServerStub::RegisterApplicationInner(MessageParcel &data, MessageParcel &reply)
{
    sptr<IRemoteObject> remote = data.ReadRemoteObject();
    const sptr<IBluetoothGattServerCallback> callback = OHOS::iface_cast<IBluetoothGattServerCallback>(remote);
    int result = RegisterApplication(callback);
    bool ret = reply.WriteInt32(result);
    if (!ret) {
        HILOGE("BluetoothGattServerStub: reply writing failed in: %{public}s.", __func__);
        return ERR_INVALID_VALUE;
    }
    return NO_ERROR;
}
ErrCode BluetoothGattServerStub::DeregisterApplicationInner(MessageParcel &data, MessageParcel &reply)
{
    int appId = data.ReadInt32();
    int result = DeregisterApplication(appId);
    bool ret = reply.WriteInt32(result);
    if (!ret) {
        HILOGE("BluetoothGattServerStub: reply writing failed in: %{public}s.", __func__);
        return ERR_INVALID_VALUE;
    }
    return NO_ERROR;
}
ErrCode BluetoothGattServerStub::NotifyClientInner(MessageParcel &data, MessageParcel &reply)
{
    const BluetoothGattDevice *device = data.ReadParcelable<BluetoothGattDevice>();
    BluetoothGattCharacteristic *characteristic = data.ReadParcelable<BluetoothGattCharacteristic>();
    bool needConfirm = data.ReadBool();
    int result = NotifyClient(*device, characteristic, needConfirm);
    delete device;
    delete characteristic;
    bool ret = reply.WriteInt32(result);
    if (!ret) {
        HILOGE("BluetoothGattServerStub: reply writing failed in: %{public}s.", __func__);
        return ERR_INVALID_VALUE;
    }
    return NO_ERROR;
}
ErrCode BluetoothGattServerStub::RemoveServiceInner(MessageParcel &data, MessageParcel &reply)
{
    int appId = data.ReadInt32();
    const BluetoothGattService *services = data.ReadParcelable<BluetoothGattService>();
    int result = RemoveService(appId, *services);
    delete services;
    bool ret = reply.WriteInt32(result);
    if (!ret) {
        HILOGE("BluetoothGattServerStub: reply writing failed in: %{public}s.", __func__);
        return ERR_INVALID_VALUE;
    }
    return NO_ERROR;
}
ErrCode BluetoothGattServerStub::RespondCharacteristicReadInner(MessageParcel &data, MessageParcel &reply)
{
    const BluetoothGattDevice *device = data.ReadParcelable<BluetoothGattDevice>();
    BluetoothGattCharacteristic *characteristic = data.ReadParcelable<BluetoothGattCharacteristic>();
    int ret1 = data.ReadInt32();
    int result = RespondCharacteristicRead(*device, characteristic, ret1);
    delete device;
    delete characteristic;
    bool ret2 = reply.WriteInt32(result);
    if (!ret2) {
        HILOGE("BluetoothGattServerStub: reply writing failed in: %{public}s.", __func__);
        return ERR_INVALID_VALUE;
    }
    return NO_ERROR;
}
ErrCode BluetoothGattServerStub::RespondCharacteristicWriteInner(MessageParcel &data, MessageParcel &reply)
{
    const BluetoothGattDevice *device = data.ReadParcelable<BluetoothGattDevice>();
    const BluetoothGattCharacteristic *characteristic = data.ReadParcelable<BluetoothGattCharacteristic>();
    int ret1 = data.ReadInt32();
    int result = RespondCharacteristicWrite(*device, *characteristic, ret1);
    delete device;
    delete characteristic;
    bool ret2 = reply.WriteInt32(result);
    if (!ret2) {
        HILOGE("BluetoothGattServerStub: reply writing failed in: %{public}s.", __func__);
        return ERR_INVALID_VALUE;
    }
    return NO_ERROR;
}
ErrCode BluetoothGattServerStub::RespondDescriptorReadInner(MessageParcel &data, MessageParcel &reply)
{
    const BluetoothGattDevice *device = data.ReadParcelable<BluetoothGattDevice>();
    BluetoothGattDescriptor *descriptor = data.ReadParcelable<BluetoothGattDescriptor>();
    int ret1 = data.ReadInt32();
    int result = RespondDescriptorRead(*device, descriptor, ret1);
    delete device;
    delete descriptor;
    bool ret2 = reply.WriteInt32(result);
    if (!ret2) {
        HILOGE("BluetoothGattServerStub: reply writing failed in: %{public}s.", __func__);
        return ERR_INVALID_VALUE;
    }
    return NO_ERROR;
}
ErrCode BluetoothGattServerStub::RespondDescriptorWriteInner(MessageParcel &data, MessageParcel &reply)
{
    const BluetoothGattDevice *device = data.ReadParcelable<BluetoothGattDevice>();
    BluetoothGattDescriptor *descriptor = data.ReadParcelable<BluetoothGattDescriptor>();
    int ret1 = data.ReadInt32();
    int result = RespondDescriptorWrite(*device, *descriptor, ret1);
    delete device;
    delete descriptor;
    bool ret2 = reply.WriteInt32(result);
    if (!ret2) {
        HILOGE("BluetoothGattServerStub: reply writing failed in: %{public}s.", __func__);
        return ERR_INVALID_VALUE;
    }
    return NO_ERROR;
}
}  // namespace Bluetooth
}  // namespace OHOS