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
#include "bluetooth_gatt_characteristic.h"
#include "bluetooth_gatt_descriptor.h"
#include "bluetooth_log.h"

namespace OHOS {
namespace Bluetooth {
bool BluetoothGattCharacteristic::Marshalling(Parcel &parcel) const
{
    if (!parcel.WriteUint16(handle_)) {
        return false;
    }
    if (!parcel.WriteUint16(endHandle_)) {
        return false;
    }
    if (!parcel.WriteUint16(valueHandle_)) {
        return false;
    }
    if (!parcel.WriteInt32(properties_)) {
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
    int des_num = descriptors_.size();
    if (!parcel.WriteInt32(des_num)) {
        return false;
    }
    for (int j = 0; j < des_num; j++) {
        if (!parcel.WriteUint16(descriptors_[j].handle_)) {
            return false;
        }
        if (!parcel.WriteInt32(descriptors_[j].permissions_)) {
            return false;
        }
        if (!parcel.WriteUint32(descriptors_[j].length_)) {
            return false;
        }
        for (size_t k = 0; k < descriptors_[j].length_; k++) {
            if (!parcel.WriteUint8(descriptors_[j].value_[k])) {
                return false;
            }
        }
        BluetoothUuid *de_bt_uuid = new BluetoothUuid(descriptors_[j].uuid_);
        if (!parcel.WriteParcelable(de_bt_uuid)) {
            return false;
        }
    }
    return true;
}

BluetoothGattCharacteristic *BluetoothGattCharacteristic::Unmarshalling(Parcel &parcel)
{
    bool noError = true;
    uint16_t handle;
    if (!parcel.ReadUint16(handle)) {
        noError = false;
    }
    uint16_t endHandle;
    if (!parcel.ReadUint16(endHandle)) {
        noError = false;
    }
    uint16_t valueHandle;
    if (!parcel.ReadUint16(valueHandle)) {
        noError = false;
    }
    int properties;
    if (!parcel.ReadInt32(properties)) {
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
    BluetoothGattCharacteristic *characteristic = new BluetoothGattCharacteristic(
        Characteristic((bluetooth::Uuid)(*bt_uuid), handle, properties, permissions, value_v, length));
    characteristic->endHandle_ = endHandle;
    characteristic->valueHandle_ = valueHandle;
    int des_num;
    if (parcel.ReadInt32(des_num)) {
        uint16_t des_handle;
        int des_permissions;
        size_t des_length;
        for (int j = 0; j < des_num; j++) {
            if (!parcel.ReadUint16(des_handle)) {
                noError = false;
            }
            if (!parcel.ReadInt32(des_permissions)) {
                noError = false;
            }
            if (!parcel.ReadUint32(des_length)) {
                noError = false;
            }
            uint8_t des_value[des_length];
            for (size_t k = 0; k < des_length; k++) {
                if (!parcel.ReadUint8(des_value[k])) {
                    noError = false;
                }
            }
            BluetoothUuid *de_bt_uuid = parcel.ReadParcelable<BluetoothUuid>();
            if (!de_bt_uuid) {
                delete characteristic;
                return nullptr;
            }
            bluetooth::Descriptor desc = bluetooth::Descriptor(
                (bluetooth::Uuid)(*de_bt_uuid), des_handle, des_permissions, des_value, des_length);
            characteristic->descriptors_.push_back(desc);
        }
    } else {
        noError = false;
    }
    if (!noError) {
        delete characteristic;
        characteristic = nullptr;
    }
    return characteristic;
}

bool BluetoothGattCharacteristic::writeToParcel(Parcel &parcel)
{
    return Marshalling(parcel);
}

bool BluetoothGattCharacteristic::readFromParcel(Parcel &parcel)
{
    bool noError = true;
    uint16_t handle;
    if (parcel.ReadUint16(handle)) {
        handle_ = handle;
    } else {
        noError = false;
    }
    uint16_t endHandle;
    if (parcel.ReadUint16(endHandle)) {
        endHandle_ = endHandle;
    } else {
        noError = false;
    }
    uint16_t valueHandle;
    if (parcel.ReadUint16(valueHandle)) {
        valueHandle_ = valueHandle;
    } else {
        noError = false;
    }
    int properties;
    if (parcel.ReadInt32(properties)) {
        properties_ = properties;
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
    int des_num;
    if (parcel.ReadInt32(des_num)) {
        uint16_t des_handle;
        int des_permissions;
        int des_len, k;
        std::unique_ptr<uint8_t[]> des_value;
        size_t des_length;
        uint32_t des_uuid;
        for (int j = 0; j < des_num; j++) {
            if (parcel.ReadUint16(des_handle)) {
                j += 0;
            } else {
                noError = false;
            }
            if (parcel.ReadInt32(des_permissions)) {
                j += 0;
            } else {
                noError = false;
            }
            if (parcel.ReadInt32(des_len)) {
                for (k = 0; k < des_len; k++) {
                    if (parcel.ReadUint8(des_value[k])) {
                        k += 0;
                    } else {
                        noError = false;
                    }
                }
            } else {
                noError = false;
            }
            if (parcel.ReadUint32(des_length)) {
                j += 0;
            } else {
                noError = false;
            }
            if (parcel.ReadUint32(des_uuid)) {
                j += 0;
            } else {
                noError = false;
            }
            uint8_t *par_value = des_value.get();
            bluetooth::Descriptor desc = bluetooth::Descriptor(
                bluetooth::Uuid::ConvertFrom32Bits(des_uuid), des_handle, des_permissions, par_value, des_length);
            descriptors_.push_back(desc);
        }
    } else {
        noError = false;
    }
    return noError;
}
}  // namespace Bluetooth
}  // namespace OHOS
