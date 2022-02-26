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

#include "bluetooth_bt_uuid.h"
#include "bluetooth_gatt_device.h"
#include "bluetooth_log.h"
#include "bluetooth_raw_address.h"

namespace OHOS {
namespace Bluetooth {
bool BluetoothGattDevice::Marshalling(Parcel &parcel) const
{
    if (!parcel.WriteBool(isEncryption_)) {
        return false;
    }
    if (!parcel.WriteUint8(transport_)) {
        return false;
    }
    if (!parcel.WriteUint8(addressType_)) {
        return false;
    }
    if (!parcel.WriteInt32(connectState_)) {
        return false;
    }
    if (!parcel.WriteString(addr_.GetAddress())) {
        return false;
    }
    return true;
}

BluetoothGattDevice *BluetoothGattDevice::Unmarshalling(Parcel &parcel)
{
    BluetoothGattDevice *gattdevice = new BluetoothGattDevice();

    bool noError = true;
    bool isEncryption;
    if (parcel.ReadBool(isEncryption)) {
        gattdevice->isEncryption_ = isEncryption;
    } else {
        noError = false;
    }
    uint8_t transport;
    if (parcel.ReadUint8(transport)) {
        gattdevice->transport_ = transport;
    } else {
        noError = false;
    }
    uint8_t addressType;
    if (parcel.ReadUint8(addressType)) {
        gattdevice->addressType_ = addressType;
    } else {
        noError = false;
    }
    int connectState;
    if (parcel.ReadInt32(connectState)) {
        gattdevice->connectState_ = connectState;
    } else {
        noError = false;
    }
    std::string address = "";
    if (parcel.ReadString(address)) {
        gattdevice->addr_ = bluetooth::RawAddress(address);
    } else {
        noError = false;
    }
    if (!noError) {
        delete gattdevice;
        gattdevice = nullptr;
    }
    return gattdevice;
}

bool BluetoothGattDevice::writeToParcel(Parcel &parcel)
{
    return Marshalling(parcel);
}

bool BluetoothGattDevice::readFromParcel(Parcel &parcel)
{
    bool noError = true;
    bool isEncryption;
    if (parcel.ReadBool(isEncryption)) {
        isEncryption_ = isEncryption;
    } else {
        noError = false;
    }
    uint8_t transport;
    if (parcel.ReadUint8(transport)) {
        transport_ = transport;
    } else {
        noError = false;
    }
    uint8_t addressType;
    if (parcel.ReadUint8(addressType)) {
        addressType_ = addressType;
    } else {
        noError = false;
    }
    int connectState;
    if (parcel.ReadInt32(connectState)) {
        connectState_ = connectState;
    } else {
        noError = false;
    }
    std::string address = "";
    if (parcel.ReadString(address)) {
        addr_ = bluetooth::RawAddress(address);
    } else {
        noError = false;
    }
    return noError;
}
}  // namespace Bluetooth
}  // namespace OHOS
