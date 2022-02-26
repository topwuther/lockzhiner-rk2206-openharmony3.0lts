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

#include "bluetooth_remote_device_observer_stub.h"
#include "bluetooth_log.h"

namespace OHOS {
namespace Bluetooth {
std::map<uint32_t, ErrCode (BluetoothRemoteDeviceObserverstub::*)(MessageParcel &data, MessageParcel &reply)>
    BluetoothRemoteDeviceObserverstub::memberFuncMap_ = {
        {static_cast<uint32_t>(IBluetoothRemoteDeviceObserver::Code::BT_REMOTE_DEVICE_OBSERVER_PSIR_STATUS),
            &BluetoothRemoteDeviceObserverstub::OnPairStatusChangedInner},
        {static_cast<uint32_t>(IBluetoothRemoteDeviceObserver::Code::BT_REMOTE_DEVICE_OBSERVER_REMOTE_UUID),
            &BluetoothRemoteDeviceObserverstub::OnRemoteNameUuidChangedInner},
        {static_cast<uint32_t>(IBluetoothRemoteDeviceObserver::Code::BT_REMOTE_DEVICE_OBSERVER_REMOTE_NAME),
            &BluetoothRemoteDeviceObserverstub::OnRemoteNameChangedInner},
        {static_cast<uint32_t>(IBluetoothRemoteDeviceObserver::Code::BT_REMOTE_DEVICE_OBSERVER_ALIAS_CHANGED),
            &BluetoothRemoteDeviceObserverstub::OnRemoteAliasChangedInner},
        {static_cast<uint32_t>(IBluetoothRemoteDeviceObserver::Code::BT_REMOTE_DEVICE_OBSERVER_REMOTE_COD),
            &BluetoothRemoteDeviceObserverstub::OnRemoteCodChangedInner},
        {static_cast<uint32_t>(IBluetoothRemoteDeviceObserver::Code::BT_REMOTE_DEVICE_OBSERVER_REMOTE_BATTERY_LEVEL),
            &BluetoothRemoteDeviceObserverstub::OnRemoteBatteryLevelChangedInner},
};

BluetoothRemoteDeviceObserverstub::BluetoothRemoteDeviceObserverstub()
{
    HILOGD("%{public}s start.", __func__);
}
BluetoothRemoteDeviceObserverstub::~BluetoothRemoteDeviceObserverstub()
{
    HILOGD("%{public}s start.", __func__);
    memberFuncMap_.clear();
}

int BluetoothRemoteDeviceObserverstub::OnRemoteRequest(
    uint32_t code, MessageParcel &data, MessageParcel &reply, MessageOption &option)
{
    HILOGD("BluetoothRemoteDeviceObserverstub::OnRemoteRequest, cmd = %d, flags= %d", code, option.GetFlags());
    std::u16string descriptor = BluetoothRemoteDeviceObserverstub::GetDescriptor();
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
    HILOGW("BluetoothRemoteDeviceObserverstub::OnRemoteRequest, default case, need check.");
    return IPCObjectStub::OnRemoteRequest(code, data, reply, option);
}

ErrCode BluetoothRemoteDeviceObserverstub::OnPairStatusChangedInner(MessageParcel &data, MessageParcel &reply)
{
    int32_t transport = data.ReadInt32();
    sptr<BluetoothRawAddress> result = data.ReadParcelable<BluetoothRawAddress>();
    int32_t status = data.ReadInt32();
    OnPairStatusChanged(transport, *result, status);
    return NO_ERROR;
}

ErrCode BluetoothRemoteDeviceObserverstub::OnRemoteNameUuidChangedInner(MessageParcel &data, MessageParcel &reply)
{
    sptr<BluetoothRawAddress> result = data.ReadParcelable<BluetoothRawAddress>();
    int32_t uuidSize = data.ReadInt32();
    std::vector<bluetooth::Uuid> uuids;
    for (int i = 0; i < uuidSize; ++i) {
        bluetooth::Uuid uuid = ParcelBtUuid::ReadFromParcel(data);
        uuids.push_back(uuid);
    }
    OnRemoteUuidChanged(*result, uuids);
    return NO_ERROR;
}

ErrCode BluetoothRemoteDeviceObserverstub::OnRemoteNameChangedInner(MessageParcel &data, MessageParcel &reply)
{
    sptr<BluetoothRawAddress> result = data.ReadParcelable<BluetoothRawAddress>();
    std::string deviceName = data.ReadString();
    OnRemoteNameChanged(*result, deviceName);
    return NO_ERROR;
}

ErrCode BluetoothRemoteDeviceObserverstub::OnRemoteAliasChangedInner(MessageParcel &data, MessageParcel &reply)
{
    sptr<BluetoothRawAddress> result = data.ReadParcelable<BluetoothRawAddress>();
    std::string alias = data.ReadString();
    OnRemoteAliasChanged(*result, alias);
    return NO_ERROR;
}

ErrCode BluetoothRemoteDeviceObserverstub::OnRemoteCodChangedInner(MessageParcel &data, MessageParcel &reply)
{
    sptr<BluetoothRawAddress> result = data.ReadParcelable<BluetoothRawAddress>();
    int32_t cod = data.ReadInt32();
    OnRemoteCodChanged(*result, cod);
    return NO_ERROR;
}

ErrCode BluetoothRemoteDeviceObserverstub::OnRemoteBatteryLevelChangedInner(MessageParcel &data, MessageParcel &reply)
{
    sptr<BluetoothRawAddress> result = data.ReadParcelable<BluetoothRawAddress>();
    int32_t batteryLevel = data.ReadInt32();
    OnRemoteBatteryLevelChanged(*result, batteryLevel);
    return NO_ERROR;
}
}  // namespace Bluetooth
}  // namespace OHOS