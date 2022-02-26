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
#include "bluetooth_gatt_descriptor.h"
#include "bluetooth_log.h"

namespace OHOS {
namespace Bluetooth {
bool BluetoothGattDescriptor::Marshalling(Parcel &parcel) const
{
    if (!parcel.WriteUint16(handle_)) {
        return false;
    }
    if (!parcel.WriteInt32(permissions_)) {
        return false;
    }
    if (!parcel.WriteUint32(length_)) {
        return false;
    }
    for (size_t i = 0; i < length_; i++) {
        if (!parcel.WriteUint8(value_[i])) {
            return false;
        }
    }
    BluetoothUuid *bt_uuid = new BluetoothUuid(uuid_);
    if (!parcel.WriteParcelable(bt_uuid)) {
        return false;
    }
    return true;
}

BluetoothGattDescriptor *BluetoothGattDescriptor::Unmarshalling(Parcel &parcel)
{
    bool noError = true;
    uint16_t handle;
    if (!parcel.ReadUint16(handle)) {
        noError = false;
    }
    int permissions;
    if (!parcel.ReadInt32(permissions)) {
        noError = false;
    }
    size_t length;
    if (!parcel.ReadUint32(length)) {
        noError = false;
    }
    uint8_t value_v[length];
    for (size_t i = 0; i < length; i++) {
        if (!parcel.ReadUint8(value_v[i])) {
            noError = false;
        }
    }
    BluetoothUuid *bt_uuid = parcel.ReadParcelable<BluetoothUuid>();
    if (!bt_uuid) {
        return nullptr;
    }
    BluetoothGattDescriptor *descriptor =
        new BluetoothGattDescriptor(Descriptor((bluetooth::Uuid)(*bt_uuid), handle, permissions, value_v, length));
    if (!noError) {
        delete descriptor;
        descriptor = nullptr;
    }
    return descriptor;
}

bool BluetoothGattDescriptor::writeToParcel(Parcel &parcel)
{
    return Marshalling(parcel);
}

bool BluetoothGattDescriptor::readFromParcel(Parcel &parcel)
{
    bool noError = true;
    uint16_t handle;
    if (parcel.ReadUint16(handle)) {
        handle_ = handle;
    } else {
        noError = false;
    }
    int permissions;
    if (parcel.ReadInt32(permissions)) {
        permissions_ = permissions;
    } else {
        noError = false;
    }
    int len;
    if (parcel.ReadInt32(len)) {
        for (int i = 0; i < len; i++) {
            if (parcel.ReadUint8(value_[i])) {
                i += 0;
            } else {
                noError = false;
            }
        }
    } else {
        noError = false;
    }
    size_t length;
    if (parcel.ReadUint32(length)) {
        length_ = length;
    } else {
        noError = false;
    }
    uint32_t uuid;
    if (parcel.ReadUint32(uuid)) {
        uuid_ = bluetooth::Uuid::ConvertFrom32Bits(uuid);
    } else {
        noError = false;
    }
    return noError;
}
}  // namespace Bluetooth
}  // namespace OHOS
