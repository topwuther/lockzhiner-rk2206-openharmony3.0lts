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

#include <map>

#include "bluetooth_ble_central_manager_stub.h"
#include "bluetooth_log.h"
#include "ipc_types.h"
#include "parcel_bt_uuid.h"

namespace OHOS {
namespace Bluetooth {
const std::map<uint32_t, std::function<ErrCode(BluetoothBleCentralManagerStub *, MessageParcel &, MessageParcel &)>>
    BluetoothBleCentralManagerStub::interfaces_ = {
        {IBluetoothBleCentralManager::Code::BLE_REGISTER_BLE_CENTRAL_MANAGER_CALLBACK,
            std::bind(&BluetoothBleCentralManagerStub::RegisterBleCentralManagerCallbackInner, std::placeholders::_1,
                std::placeholders::_2, std::placeholders::_3)},
        {IBluetoothBleCentralManager::Code::BLE_DE_REGISTER_BLE_CENTRAL_MANAGER_CALLBACK,
            std::bind(&BluetoothBleCentralManagerStub::DeregisterBleCentralManagerCallbackInner, std::placeholders::_1,
                std::placeholders::_2, std::placeholders::_3)},
        {IBluetoothBleCentralManager::Code::BLE_START_SCAN,
            std::bind(&BluetoothBleCentralManagerStub::StartScanInner, std::placeholders::_1, std::placeholders::_2,
                std::placeholders::_3)},
        {IBluetoothBleCentralManager::Code::BLE_START_SCAN_WITH_SETTINGS,
            std::bind(&BluetoothBleCentralManagerStub::StartScanWithSettingsInner, std::placeholders::_1,
                std::placeholders::_2, std::placeholders::_3)},
        {IBluetoothBleCentralManager::Code::BLE_STOP_SCAN,
            std::bind(&BluetoothBleCentralManagerStub::StopScanInner, std::placeholders::_1, std::placeholders::_2,
                std::placeholders::_3)},
};

BluetoothBleCentralManagerStub::BluetoothBleCentralManagerStub()
{}

BluetoothBleCentralManagerStub::~BluetoothBleCentralManagerStub()
{}

int BluetoothBleCentralManagerStub::OnRemoteRequest(
    uint32_t code, MessageParcel &data, MessageParcel &reply, MessageOption &option)
{
    std::u16string descriptor = BluetoothBleCentralManagerStub::GetDescriptor();
    std::u16string remoteDescriptor = data.ReadInterfaceToken();
    if (descriptor != remoteDescriptor) {
        HILOGW("[OnRemoteRequest] fail: invalid interface token!");
        return OBJECT_NULL;
    }

    auto it = interfaces_.find(code);
    if (it == interfaces_.end()) {
        HILOGW("[OnRemoteRequest] fail: unknown code!");
        return IRemoteStub<IBluetoothBleCentralManager>::OnRemoteRequest(code, data, reply, option);
    }

    auto fun = it->second;
    if (fun == nullptr) {
        HILOGW("[OnRemoteRequest] fail: not find function!");
        return IRemoteStub<IBluetoothBleCentralManager>::OnRemoteRequest(code, data, reply, option);
    }

    ErrCode result = fun(this, data, reply);
    if (SUCCEEDED(result)) {
        return NO_ERROR;
    }

    HILOGW("[OnRemoteRequest] fail: Failed to call interface %{public}u, err:%{public}d", code, result);
    return result;
}

ErrCode BluetoothBleCentralManagerStub::RegisterBleCentralManagerCallbackInner(
    MessageParcel &data, MessageParcel &reply)
{
    sptr<IRemoteObject> remote = data.ReadRemoteObject();
    const sptr<IBluetoothBleCentralManagerCallback> callBack =
        OHOS::iface_cast<IBluetoothBleCentralManagerCallback>(remote);
    RegisterBleCentralManagerCallback(callBack);
    return NO_ERROR;
}

ErrCode BluetoothBleCentralManagerStub::DeregisterBleCentralManagerCallbackInner(
    MessageParcel &data, MessageParcel &reply)
{
    sptr<IRemoteObject> remote = data.ReadRemoteObject();
    const sptr<IBluetoothBleCentralManagerCallback> callBack =
        OHOS::iface_cast<IBluetoothBleCentralManagerCallback>(remote);
    DeregisterBleCentralManagerCallback(callBack);
    return NO_ERROR;
}

ErrCode BluetoothBleCentralManagerStub::StartScanInner(MessageParcel &data, MessageParcel &reply)
{
    StartScan();
    return NO_ERROR;
}

ErrCode BluetoothBleCentralManagerStub::StartScanWithSettingsInner(MessageParcel &data, MessageParcel &reply)
{
    sptr<BluetoothBleScanSettings> settings = data.ReadParcelable<BluetoothBleScanSettings>();
    if (settings == nullptr) {
        HILOGW("[StartScanWithSettingsInner] fail: read settings failed");
        return TRANSACTION_ERR;
    }

    StartScan(*settings);
    return NO_ERROR;
}

ErrCode BluetoothBleCentralManagerStub::StopScanInner(MessageParcel &data, MessageParcel &reply)
{
    StopScan();
    return NO_ERROR;
}
}  // namespace Bluetooth
}  // namespace OHOS
