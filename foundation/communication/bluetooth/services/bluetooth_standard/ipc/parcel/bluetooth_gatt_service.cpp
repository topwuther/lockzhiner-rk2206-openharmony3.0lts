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
#include "bluetooth_gatt_service.h"
#include "bluetooth_log.h"
#include "parcel_bt_uuid.h"

namespace OHOS {
namespace Bluetooth {
bool BluetoothGattService::Marshalling(Parcel &parcel) const
{
    if (!parcel.WriteBool(isPrimary_)) {
        return false;
    }
    if (!parcel.WriteUint16(handle_)) {
        return false;
    }
    if (!parcel.WriteUint16(startHandle_)) {
        return false;
    }
    if (!parcel.WriteUint16(endHandle_)) {
        return false;
    }
    BluetoothUuid *bt_uuid = new BluetoothUuid(uuid_);
    if (!parcel.WriteParcelable(bt_uuid)) {
        return false;
    }
    int ser_num = includeServices_.size();
    if (!parcel.WriteInt32(ser_num)) {
        return false;
    }
    if (ser_num != 0) {
        if (!ServiceIteratorWrite(includeServices_, parcel, ser_num)) {
            return false;
        }
    }
    int cha_num = characteristics_.size();
    if (!parcel.WriteInt32(cha_num)) {
        return false;
    }
    int des_num, l;
    for (int j = 0; j < cha_num; j++) {
        if (!parcel.WriteUint16(characteristics_[j].handle_)) {
            return false;
        }
        if (!parcel.WriteUint16(characteristics_[j].endHandle_)) {
            return false;
        }
        if (!parcel.WriteUint16(characteristics_[j].valueHandle_)) {
            return false;
        }
        if (!parcel.WriteInt32(characteristics_[j].properties_)) {
            return false;
        }
        if (!parcel.WriteInt32(characteristics_[j].permissions_)) {
            return false;
        }
        if (!parcel.WriteUint32(characteristics_[j].length_)) {
            return false;
        }
        for (size_t k = 0; k < characteristics_[j].length_; k++) {
            if (!parcel.WriteUint8(characteristics_[j].value_[k])) {
                return false;
            }
        }
        BluetoothUuid *ch_bt_uuid = new BluetoothUuid(characteristics_[j].uuid_);
        if (!parcel.WriteParcelable(ch_bt_uuid)) {
            return false;
        }
        des_num = characteristics_[j].descriptors_.size();
        if (!parcel.WriteInt32(des_num)) {
            return false;
        }
        for (l = 0; l < des_num; l++) {
            if (!parcel.WriteUint16(characteristics_[j].descriptors_[l].handle_)) {
                return false;
            }
            if (!parcel.WriteInt32(characteristics_[j].descriptors_[l].permissions_)) {
                return false;
            }
            if (!parcel.WriteUint32(characteristics_[j].descriptors_[l].length_)) {
                return false;
            }
            for (size_t m = 0; m < characteristics_[j].descriptors_[l].length_; m++) {
                if (!parcel.WriteUint8(characteristics_[j].descriptors_[l].value_[m])) {
                    return false;
                }
            }
            BluetoothUuid *ch_de_bt_uuid = new BluetoothUuid(characteristics_[j].descriptors_[l].uuid_);
            if (!parcel.WriteParcelable(ch_de_bt_uuid)) {
                return false;
            }
        }
    }
    return true;
}

BluetoothGattService *BluetoothGattService::Unmarshalling(Parcel &parcel)
{
    BluetoothGattService *gattservice = new BluetoothGattService();

    bool noError = true;
    bool isPrimary;
    if (parcel.ReadBool(isPrimary)) {
        gattservice->isPrimary_ = isPrimary;
    } else {
        noError = false;
    }
    uint16_t handle;
    if (parcel.ReadUint16(handle)) {
        gattservice->handle_ = handle;
    } else {
        noError = false;
    }
    uint16_t startHandle;
    if (parcel.ReadUint16(startHandle)) {
        gattservice->startHandle_ = startHandle;
    } else {
        noError = false;
    }
    uint16_t endHandle;
    if (parcel.ReadUint16(endHandle)) {
        gattservice->endHandle_ = endHandle;
    } else {
        noError = false;
    }
    BluetoothUuid *bt_uuid = parcel.ReadParcelable<BluetoothUuid>();
    if (bt_uuid) {
        gattservice->uuid_ = (bluetooth::Uuid)(*bt_uuid);
    } else {
        return nullptr;
    }
    int ser_num;
    if (parcel.ReadInt32(ser_num)) {
        if (ser_num != 0) {
            if (!ServiceIteratorRead(gattservice->includeServices_, parcel, ser_num)) {
                noError = false;
            }
        }
    } else {
        noError = false;
    }
    int cha_num;
    if (parcel.ReadInt32(cha_num)) {
        for (int z = 0; z < cha_num; z++) {
            uint16_t cha_handle;
            if (!parcel.ReadUint16(cha_handle)) {
                noError = false;
            }
            uint16_t cha_endHandle;
            if (!parcel.ReadUint16(cha_endHandle)) {
                noError = false;
            }
            uint16_t cha_valueHandle;
            if (!parcel.ReadUint16(cha_valueHandle)) {
                noError = false;
            }
            int cha_properties;
            if (!parcel.ReadInt32(cha_properties)) {
                noError = false;
            }
            int cha_permissions;
            if (!parcel.ReadInt32(cha_permissions)) {
                noError = false;
            }
            size_t cha_length;
            if (!parcel.ReadUint32(cha_length)) {
                noError = false;
            }
            uint8_t value_v[cha_length];
            for (size_t cha_i = 0; cha_i < cha_length; cha_i++) {
                if (!parcel.ReadUint8(value_v[cha_i])) {
                    noError = false;
                }
            }
            BluetoothUuid *ch_bt_uuid = parcel.ReadParcelable<BluetoothUuid>();
            if (!ch_bt_uuid) {
                delete gattservice;
                return nullptr;
            }
            bluetooth::Characteristic characteristic = bluetooth::Characteristic(
                (bluetooth::Uuid)(*ch_bt_uuid), cha_handle, cha_properties, cha_permissions, value_v, cha_length);
            characteristic.endHandle_ = cha_endHandle;
            characteristic.valueHandle_ = cha_valueHandle;
            int des_num;
            if (parcel.ReadInt32(des_num)) {
                uint16_t des_handle;
                int des_permissions;
                size_t des_k;
                size_t des_length;
                for (int des_j = 0; des_j < des_num; des_j++) {
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
                    for (des_k = 0; des_k < des_length; des_k++) {
                        if (!parcel.ReadUint8(des_value[des_k])) {
                            noError = false;
                        }
                    }
                    BluetoothUuid *ch_de_bt_uuid = parcel.ReadParcelable<BluetoothUuid>();
                    if (!ch_de_bt_uuid) {
                        delete gattservice;
                        return nullptr;
                    }
                    bluetooth::Descriptor desc = bluetooth::Descriptor(
                        (bluetooth::Uuid)(*ch_de_bt_uuid), des_handle, des_permissions, des_value, des_length);
                    characteristic.descriptors_.push_back(desc);
                }
            } else {
                noError = false;
            }
            gattservice->characteristics_.push_back(characteristic);
        }
    } else {
        noError = false;
    }
    if (!noError) {
        delete gattservice;
        gattservice = nullptr;
    }
    return gattservice;
}

bool BluetoothGattService::writeToParcel(Parcel &parcel)
{
    return Marshalling(parcel);
}

bool BluetoothGattService::readFromParcel(Parcel &parcel)
{
    bool noError = true;
    bool isPrimary;
    if (parcel.ReadBool(isPrimary)) {
        isPrimary_ = isPrimary;
    } else {
        noError = false;
    }
    uint16_t handle;
    if (parcel.ReadUint16(handle)) {
        handle_ = handle;
    } else {
        noError = false;
    }
    uint16_t startHandle;
    if (parcel.ReadUint16(startHandle)) {
        startHandle_ = startHandle;
    } else {
        noError = false;
    }
    uint16_t endHandle;
    if (parcel.ReadUint16(endHandle)) {
        endHandle_ = endHandle;
    } else {
        noError = false;
    }
    uint32_t uuid;
    if (parcel.ReadUint32(uuid)) {
        uuid_ = bluetooth::Uuid::ConvertFrom32Bits(uuid);
    } else {
        noError = false;
    }
    int ser_num;
    if (parcel.ReadInt32(ser_num)) {
        if (ser_num != 0) {
            if (ServiceIteratorRead(includeServices_, parcel, ser_num)) {
                ser_num += 0;
            } else {
                noError = false;
            }
        }
    } else {
        noError = false;
    }
    int cha_num;
    if (parcel.ReadInt32(cha_num)) {
        for (int z = 0; z < cha_num; z++) {
            uint16_t cha_handle;
            if (parcel.ReadUint16(cha_handle)) {
                characteristics_[z].handle_ = cha_handle;
            } else {
                noError = false;
            }
            uint16_t cha_endHandle;
            if (parcel.ReadUint16(cha_endHandle)) {
                characteristics_[z].endHandle_ = cha_endHandle;
            } else {
                noError = false;
            }
            uint16_t cha_valueHandle;
            if (parcel.ReadUint16(cha_valueHandle)) {
                characteristics_[z].valueHandle_ = cha_valueHandle;
            } else {
                noError = false;
            }
            int cha_properties;
            if (parcel.ReadInt32(cha_properties)) {
                characteristics_[z].properties_ = cha_properties;
            } else {
                noError = false;
            }
            int cha_permissions;
            if (parcel.ReadInt32(cha_permissions)) {
                characteristics_[z].permissions_ = cha_permissions;
            } else {
                noError = false;
            }
            int cha_len;
            if (parcel.ReadInt32(cha_len)) {
                for (int cha_i = 0; cha_i < cha_len; cha_i++) {
                    if (parcel.ReadUint8(characteristics_[z].value_[cha_i])) {
                        cha_i += 0;
                    } else {
                        noError = false;
                    }
                }
            } else {
                noError = false;
            }
            size_t cha_length;
            if (parcel.ReadUint32(cha_length)) {
                characteristics_[z].length_ = cha_length;
            } else {
                noError = false;
            }
            uint32_t cha_uuid;
            if (parcel.ReadUint32(cha_uuid)) {
                characteristics_[z].uuid_ = bluetooth::Uuid::ConvertFrom32Bits(cha_uuid);
            } else {
                noError = false;
            }
            int des_num;
            if (parcel.ReadInt32(des_num)) {
                uint16_t des_handle;
                int des_permissions;
                int des_len, des_k;
                std::unique_ptr<uint8_t[]> des_value;
                size_t des_length;
                uint32_t des_uuid;
                for (int des_j = 0; des_j < des_num; des_j++) {
                    if (parcel.ReadUint16(des_handle)) {
                        des_j += 0;
                    } else {
                        noError = false;
                    }
                    if (parcel.ReadInt32(des_permissions)) {
                        des_j += 0;
                    } else {
                        noError = false;
                    }
                    if (parcel.ReadInt32(des_len)) {
                        for (des_k = 0; des_k < des_len; des_k++) {
                            if (parcel.ReadUint8(des_value[des_k])) {
                                des_k += 0;
                            } else {
                                noError = false;
                            }
                        }
                    } else {
                        noError = false;
                    }
                    if (parcel.ReadUint32(des_length)) {
                        des_j += 0;
                    } else {
                        noError = false;
                    }
                    if (parcel.ReadUint32(des_uuid)) {
                        des_j += 0;
                    } else {
                        noError = false;
                    }
                    uint8_t *par_value = des_value.get();
                    bluetooth::Descriptor desc = bluetooth::Descriptor(bluetooth::Uuid::ConvertFrom32Bits(des_uuid),
                        des_handle,
                        des_permissions,
                        par_value,
                        des_length);
                    characteristics_[z].descriptors_.push_back(desc);
                }
            } else {
                noError = false;
            }
        }
    } else {
        noError = false;
    }
    return noError;
}

bool BluetoothGattService::ServiceIteratorWrite(const std::vector<Service> &service, Parcel &parcel, int length) const
{
    for (int i = 0; i < length; i++) {
        if (!parcel.WriteBool(service[i].isPrimary_)) {
            return false;
        }
        if (!parcel.WriteUint16(service[i].handle_)) {
            return false;
        }
        if (!parcel.WriteUint16(service[i].startHandle_)) {
            return false;
        }
        if (!parcel.WriteUint16(service[i].endHandle_)) {
            return false;
        }
        BluetoothUuid *bt_uuid = new BluetoothUuid(service[i].uuid_);
        if (!parcel.WriteParcelable(bt_uuid)) {
            return false;
        }
        int ser_num = service[i].includeServices_.size();
        if (!parcel.WriteInt32(ser_num)) {
            return false;
        }
        if (ser_num != 0) {
            ServiceIteratorWrite(service[i].includeServices_, parcel, ser_num);
        }
        int cha_num = service[i].characteristics_.size();
        if (!parcel.WriteInt32(cha_num)) {
            return false;
        }
        int des_num, l;
        for (int j = 0; j < cha_num; j++) {
            if (!parcel.WriteUint16(service[i].characteristics_[j].handle_)) {
                return false;
            }
            if (!parcel.WriteUint16(service[i].characteristics_[j].endHandle_)) {
                return false;
            }
            if (!parcel.WriteUint16(service[i].characteristics_[j].valueHandle_)) {
                return false;
            }
            if (!parcel.WriteInt32(service[i].characteristics_[j].properties_)) {
                return false;
            }
            if (!parcel.WriteInt32(service[i].characteristics_[j].permissions_)) {
                return false;
            }
            if (!parcel.WriteUint32(service[i].characteristics_[j].length_)) {
                return false;
            }
            for (size_t k = 0; k < service[i].characteristics_[j].length_; k++) {
                if (!parcel.WriteUint8(service[i].characteristics_[j].value_[k])) {
                    return false;
                }
            }
            BluetoothUuid *ch_bt_uuid = new BluetoothUuid(service[i].characteristics_[j].uuid_);
            if (!parcel.WriteParcelable(ch_bt_uuid)) {
                return false;
            }
            des_num = service[i].characteristics_[j].descriptors_.size();
            if (!parcel.WriteInt32(des_num)) {
                return false;
            }
            for (l = 0; l < des_num; l++) {
                if (!parcel.WriteUint16(service[i].characteristics_[j].descriptors_[l].handle_)) {
                    return false;
                }
                if (!parcel.WriteInt32(service[i].characteristics_[j].descriptors_[l].permissions_)) {
                    return false;
                }
                if (!parcel.WriteUint32(service[i].characteristics_[j].descriptors_[l].length_)) {
                    return false;
                }
                for (size_t m = 0; m < service[i].characteristics_[j].descriptors_[l].length_; m++) {
                    if (!parcel.WriteUint8(service[i].characteristics_[j].descriptors_[l].value_[m])) {
                        return false;
                    }
                }
                BluetoothUuid *ch_de_bt_uuid = new BluetoothUuid(service[i].characteristics_[j].descriptors_[l].uuid_);
                if (!parcel.WriteParcelable(ch_de_bt_uuid)) {
                    return false;
                }
            }
        }
    }
    return true;
}

bool BluetoothGattService::ServiceIteratorRead(std::vector<Service> &service, Parcel &parcel, int length)
{
    HILOGI("BluetoothGattService::ServiceIteratorRead: start");
    bool noError = true;
    HILOGI("BluetoothGattService::service = %{public}d", service.size());
    for (int i = 0; i < length; i++) {
        bool isPrimary;
        if (!parcel.ReadBool(isPrimary)) {
            HILOGI("BluetoothGattService::isPrimary: start");
            noError = false;
        }
        uint16_t handle;
        if (!parcel.ReadUint16(handle)) {
            HILOGI("BluetoothGattService::handle: start");
            noError = false;
        }
        uint16_t startHandle;
        if (!parcel.ReadUint16(startHandle)) {
            HILOGI("BluetoothGattService::startHandle: start");
            noError = false;
        }
        uint16_t endHandle;
        if (!parcel.ReadUint16(endHandle)) {
            HILOGI("BluetoothGattService::endHandle: start");
            noError = false;
        }
        BluetoothUuid *bt_uuid = parcel.ReadParcelable<BluetoothUuid>();
        if (!bt_uuid) {
            noError = false;
            return noError;
        }
        bluetooth::Service serv = bluetooth::Service((bluetooth::Uuid)(*bt_uuid), handle, startHandle, endHandle);
        HILOGI("BluetoothGattService::new service: start");
        service.push_back(serv);
        HILOGI("BluetoothGattService::new service: end");
        HILOGI("BluetoothGattService::service = %{public}d", service.size());
        service[i].isPrimary_ = isPrimary;
        int ser_num;
        if (parcel.ReadInt32(ser_num)) {
            HILOGI("ServiceIteratorRead :ser_num = %{public}d ", ser_num);
            HILOGI("ServiceIteratorRead :start");
            if (ser_num != 0) {
                if (!ServiceIteratorRead(service[i].includeServices_, parcel, ser_num)) {
                    noError = false;
                }
            }
            HILOGI("ServiceIteratorRead :end");
        } else {
            noError = false;
        }
        int cha_num;
        if (parcel.ReadInt32(cha_num)) {
            for (int z = 0; z < cha_num; z++) {
                uint16_t cha_handle;
                if (!parcel.ReadUint16(cha_handle)) {
                    noError = false;
                }
                uint16_t cha_endHandle;
                if (!parcel.ReadUint16(cha_endHandle)) {
                    noError = false;
                }
                uint16_t cha_valueHandle;
                if (!parcel.ReadUint16(cha_valueHandle)) {
                    noError = false;
                }
                int cha_properties;
                if (!parcel.ReadInt32(cha_properties)) {
                    noError = false;
                }
                int cha_permissions;
                if (!parcel.ReadInt32(cha_permissions)) {
                    noError = false;
                }
                size_t cha_length;
                if (!parcel.ReadUint32(cha_length)) {
                    noError = false;
                }
                uint8_t value_v[cha_length];
                for (size_t cha_i = 0; cha_i < cha_length; cha_i++) {
                    if (!parcel.ReadUint8(value_v[cha_i])) {
                        noError = false;
                    }
                }
                BluetoothUuid *ch_bt_uuid = parcel.ReadParcelable<BluetoothUuid>();
                if (!ch_bt_uuid) {
                    noError = false;
                    return noError;
                }
                bluetooth::Characteristic characteristic = bluetooth::Characteristic(
                    (bluetooth::Uuid)(*ch_bt_uuid), cha_handle, cha_properties, cha_permissions, value_v, cha_length);
                characteristic.endHandle_ = cha_endHandle;
                characteristic.valueHandle_ = cha_valueHandle;
                int des_num;
                if (parcel.ReadInt32(des_num)) {
                    uint16_t des_handle;
                    int des_permissions;
                    size_t des_k;
                    size_t des_length;
                    for (int des_j = 0; des_j < des_num; des_j++) {
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
                        for (des_k = 0; des_k < des_length; des_k++) {
                            if (!parcel.ReadUint8(des_value[des_k])) {
                                noError = false;
                            }
                        }
                        BluetoothUuid *ch_de_bt_uuid = parcel.ReadParcelable<BluetoothUuid>();
                        if (!ch_de_bt_uuid) {
                            noError = false;
                            return noError;
                        }
                        bluetooth::Descriptor desc = bluetooth::Descriptor(
                            (bluetooth::Uuid)(*ch_de_bt_uuid), des_handle, des_permissions, des_value, des_length);
                        characteristic.descriptors_.push_back(desc);
                    }
                } else {
                    noError = false;
                }
                service[i].characteristics_.push_back(characteristic);
            }
        } else {
            noError = false;
        }
    }
    return noError;
}
}  // namespace Bluetooth
}  // namespace OHOS
