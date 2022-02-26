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

#include "bluetooth_ble_central_manager_proxy.h"
#include "bluetooth_log.h"
#include "parcel_bt_uuid.h"

namespace OHOS {
namespace Bluetooth {
BluetoothBleCentralManagerProxy::BluetoothBleCentralManagerProxy(const sptr<IRemoteObject> &impl)
    : IRemoteProxy<IBluetoothBleCentralManager>(impl)
{}

BluetoothBleCentralManagerProxy::~BluetoothBleCentralManagerProxy()
{}

void BluetoothBleCentralManagerProxy::RegisterBleCentralManagerCallback(
    const sptr<IBluetoothBleCentralManagerCallback> &callback)
{
    MessageParcel data;
    if (!data.WriteInterfaceToken(BluetoothBleCentralManagerProxy::GetDescriptor())) {
        HILOGW("[RegisterBleCentralManagerCallback] fail: write interface token failed.");
        return;
    }

    if (!data.WriteRemoteObject(callback->AsObject())) {
        HILOGW("[RegisterBleCentralManagerCallback] fail: write callback failed.");
        return;
    }

    MessageParcel reply;
    MessageOption option = {MessageOption::TF_SYNC};
    ErrCode result = InnerTransact(BLE_REGISTER_BLE_CENTRAL_MANAGER_CALLBACK, option, data, reply);
    if (result != NO_ERROR) {
        HILOGW("[RegisterBleCentralManagerCallback] fail: transact ErrCode=%{public}d", result);
    }
}

void BluetoothBleCentralManagerProxy::DeregisterBleCentralManagerCallback(
    const sptr<IBluetoothBleCentralManagerCallback> &callback)
{
    MessageParcel data;
    if (!data.WriteInterfaceToken(BluetoothBleCentralManagerProxy::GetDescriptor())) {
        HILOGW("[DeregisterBleCentralManagerCallback] fail: write interface token failed.");
        return;
    }

    if (!data.WriteRemoteObject(callback->AsObject())) {
        HILOGW("[DeregisterBleCentralManagerCallback] fail: write callback failed.");
        return;
    }

    MessageParcel reply;
    MessageOption option = {MessageOption::TF_SYNC};
    ErrCode result = InnerTransact(BLE_DE_REGISTER_BLE_CENTRAL_MANAGER_CALLBACK, option, data, reply);
    if (result != NO_ERROR) {
        HILOGW("[DeregisterBleCentralManagerCallback] fail: transact ErrCode=%{public}d", result);
    }
}

void BluetoothBleCentralManagerProxy::StartScan()
{
    MessageParcel data;
    if (!data.WriteInterfaceToken(BluetoothBleCentralManagerProxy::GetDescriptor())) {
        HILOGW("[StartScan] fail: write interface token failed.");
        return;
    }

    MessageParcel reply;
    MessageOption option = {MessageOption::TF_SYNC};
    ErrCode result = InnerTransact(BLE_START_SCAN, option, data, reply);
    if (result != NO_ERROR) {
        HILOGW("[StartScan] fail: transact ErrCode=%{public}d", result);
    }
}

void BluetoothBleCentralManagerProxy::StartScan(const BluetoothBleScanSettings &settings)
{
    MessageParcel data;
    if (!data.WriteInterfaceToken(BluetoothBleCentralManagerProxy::GetDescriptor())) {
        HILOGW("[StartScan] fail: write interface token failed.");
        return;
    }

    if (!data.WriteParcelable(&settings)) {
        HILOGW("[StartScan] fail:write settings failed");
        return;
    }

    MessageParcel reply;
    MessageOption option = {MessageOption::TF_SYNC};
    ErrCode result = InnerTransact(BLE_START_SCAN_WITH_SETTINGS, option, data, reply);
    if (result != NO_ERROR) {
        HILOGW("[StartScan] fail: transact ErrCode=%{public}d", result);
    }
}

void BluetoothBleCentralManagerProxy::StopScan()
{
    MessageParcel data;
    if (!data.WriteInterfaceToken(BluetoothBleCentralManagerProxy::GetDescriptor())) {
        HILOGW("[StopScan] fail: write interface token failed.");
        return;
    }

    MessageParcel reply;
    MessageOption option = {MessageOption::TF_SYNC};
    ErrCode result = InnerTransact(BLE_STOP_SCAN, option, data, reply);
    if (result != NO_ERROR) {
        HILOGW("[StopScan] fail: transact ErrCode=%{public}d", result);
    }
}

ErrCode BluetoothBleCentralManagerProxy::InnerTransact(
    uint32_t code, MessageOption &flags, MessageParcel &data, MessageParcel &reply)
{
    auto remote = Remote();
    if (remote == nullptr) {
        HILOGW("[InnerTransact] fail: get Remote fail code %{public}d", code);
        return OBJECT_NULL;
    }
    int err = remote->SendRequest(code, data, reply, flags);
    switch (err) {
        case NO_ERROR: {
            return NO_ERROR;
        }
        case DEAD_OBJECT: {
            HILOGW("[InnerTransact] fail: ipcErr=%{public}d code %{public}d", err, code);
            return DEAD_OBJECT;
        }
        default: {
            HILOGW("[InnerTransact] fail: ipcErr=%{public}d code %{public}d", err, code);
            return TRANSACTION_ERR;
        }
    }
}
}  // namespace Bluetooth
}  // namespace OHOS