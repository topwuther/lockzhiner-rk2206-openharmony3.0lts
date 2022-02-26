/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
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

#ifndef OMIT_MULTI_VER
#include "generic_multi_ver_kv_entry.h"

#include "multi_ver_value_object.h"
#include "parcel.h"

namespace DistributedDB {
GenericMultiVerKvEntry::GenericMultiVerKvEntry()
    : operFlag_(0),
      timestamp_(0),
      oriTimestamp_(0)
{}

GenericMultiVerKvEntry::~GenericMultiVerKvEntry()
{}

int GenericMultiVerKvEntry::GetSerialData(std::vector<uint8_t> &data) const
{
    std::vector<uint8_t> valueObjectSerial;
    int errCode = valueObject_.GetSerialData(valueObjectSerial);
    if (errCode != E_OK) {
        return errCode;
    }

    size_t totalLength = Parcel::GetVectorCharLen(key_) + Parcel::GetVectorCharLen(valueObjectSerial) +
        Parcel::GetUInt64Len() * 3; // 3 means number of 3 uint64_t parameters: operFlag, timestamp and oriTimestamp.

    data.resize(totalLength);
    Parcel parcel(data.data(), data.size());

    errCode = parcel.WriteVectorChar(key_);
    if (errCode != E_OK) {
        return errCode;
    }

    errCode = parcel.WriteVectorChar(valueObjectSerial);
    if (errCode != E_OK) {
        return errCode;
    }

    errCode = parcel.WriteUInt64(operFlag_);
    if (errCode != E_OK) {
        return errCode;
    }

    errCode = parcel.WriteUInt64(timestamp_);
    if (errCode != E_OK) {
        return errCode;
    }

    errCode = parcel.WriteUInt64(oriTimestamp_);
    if (errCode != E_OK) {
        return errCode;
    }

    return E_OK;
}

int GenericMultiVerKvEntry::GetValueHash(std::vector<ValueSliceHash> &valueHashes) const
{
    return valueObject_.GetValueHash(valueHashes);
}

int GenericMultiVerKvEntry::DeSerialData(const std::vector<uint8_t> &data)
{
    Parcel parcel(const_cast<uint8_t *>(data.data()), data.size());
    parcel.ReadVectorChar(key_);
    std::vector<uint8_t> objectData;
    parcel.ReadVectorChar(objectData);
    parcel.ReadUInt64(operFlag_);
    parcel.ReadUInt64(timestamp_);
    parcel.ReadUInt64(oriTimestamp_);
    if (parcel.IsError()) {
        return -E_PARSE_FAIL;
    }
    return valueObject_.DeSerialData(objectData);
}

int GenericMultiVerKvEntry::SetKey(const Key &key)
{
    key_ = key;
    return E_OK;
}

int GenericMultiVerKvEntry::GetKey(Key &key) const
{
    key = key_;
    return E_OK;
}

int GenericMultiVerKvEntry::SetValue(const Value &value)
{
    return valueObject_.DeSerialData(value);
}

int GenericMultiVerKvEntry::GetValue(Value &value)const
{
    return valueObject_.GetSerialData(value);
}

void GenericMultiVerKvEntry::SetOperFlag(uint64_t flag)
{
    operFlag_ = flag;
}

void GenericMultiVerKvEntry::GetOperFlag(uint64_t &flag) const
{
    flag = operFlag_;
}

uint32_t GenericMultiVerKvEntry::GetDataLength() const
{
    return valueObject_.GetDataLength();
}

void GenericMultiVerKvEntry::SetDataLength(uint32_t length)
{
    valueObject_.SetDataLength(length);
}

void GenericMultiVerKvEntry::SetTimestamp(uint64_t timestamp)
{
    timestamp_ = timestamp;
}

void GenericMultiVerKvEntry::GetTimestamp(uint64_t &timestamp) const
{
    timestamp = timestamp_;
}

void GenericMultiVerKvEntry::SetOriTimestamp(uint64_t oriTimestamp)
{
    oriTimestamp_ = oriTimestamp;
}

void GenericMultiVerKvEntry::GetOriTimestamp(uint64_t &oriTimestamp) const
{
    oriTimestamp = oriTimestamp_;
}
} // namespace DistributedDB
#endif
