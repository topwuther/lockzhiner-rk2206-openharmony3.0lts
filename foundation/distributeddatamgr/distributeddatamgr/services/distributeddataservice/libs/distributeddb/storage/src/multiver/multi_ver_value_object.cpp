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
#include "multi_ver_value_object.h"
#include "parcel.h"

namespace DistributedDB {
namespace {
    const size_t SLICE_HASH_VALUE_SIZE = 32;
}

int MultiVerValueObject::GetValueHash(std::vector<ValueSliceHash> &valueHashes) const
{
    if (!IsHash()) {
        return E_OK;
    }

    for (size_t i = 0; i < valueHashVector_.size() / SLICE_HASH_VALUE_SIZE; i++) {
        ValueSliceHash sliceHash;
        sliceHash.assign(valueHashVector_.begin() + i * SLICE_HASH_VALUE_SIZE,
                         valueHashVector_.begin() + (i + 1) * SLICE_HASH_VALUE_SIZE);
        valueHashes.push_back(std::move(sliceHash));
    }
    return E_OK;
}

int MultiVerValueObject::SetValueHash(const std::vector<ValueSliceHash> &valueHashes)
{
    valueHashVector_.clear();
    valueHashVector_.shrink_to_fit();
    for (const auto &item : valueHashes) {
        valueHashVector_.insert(valueHashVector_.end(), item.begin(), item.end());
    }
    head_.flag = HASH_FLAG;
    return E_OK;
}

int MultiVerValueObject::GetSerialData(std::vector<uint8_t> &data) const
{
    const uint32_t serialIntNum = 4; // 4 int
    size_t totalLength = Parcel::GetIntLen() * serialIntNum + Parcel::GetVectorCharLen(valueHashVector_);
    data.resize(totalLength);
    Parcel parcel(data.data(), data.size());

    int errCode = parcel.WriteInt(head_.flag);
    if (errCode != E_OK) {
        return errCode;
    }

    errCode = parcel.WriteInt(head_.reserved);
    if (errCode != E_OK) {
        return errCode;
    }

    errCode = parcel.WriteInt(head_.hashNum);
    if (errCode != E_OK) {
        return errCode;
    }

    errCode = parcel.WriteInt(head_.length);
    if (errCode != E_OK) {
        return errCode;
    }

    errCode = parcel.WriteVectorChar(valueHashVector_);
    if (errCode != E_OK) {
        return errCode;
    }

    return E_OK;
}

int MultiVerValueObject::DeSerialData(const std::vector<uint8_t> &data)
{
    Parcel parcel(const_cast<uint8_t *>(data.data()), data.size());
    int32_t readValue = 0;
    parcel.ReadInt(readValue);
    head_.flag = static_cast<uint8_t>(readValue);
    parcel.ReadInt(readValue);
    head_.reserved = static_cast<uint8_t>(readValue);
    parcel.ReadInt(readValue);
    head_.hashNum = static_cast<uint16_t>(readValue);
    parcel.ReadInt(readValue);
    head_.length = static_cast<uint32_t>(readValue);
    parcel.ReadVectorChar(valueHashVector_);
    if (parcel.IsError()) {
        LOGE("Deserial the multi ver value object error");
        return -E_PARSE_FAIL;
    }
    if (((head_.flag & HASH_FLAG) == HASH_FLAG) && ((valueHashVector_.size() % SLICE_HASH_VALUE_SIZE) != 0)) {
        LOGE("Value hash list total size is unexpected:%zu", valueHashVector_.size());
        return -E_PARSE_FAIL;
    }
    return E_OK;
}

uint32_t MultiVerValueObject::GetDataLength() const
{
    return head_.length;
}

void MultiVerValueObject::SetDataLength(uint32_t length)
{
    head_.length = length;
}

int MultiVerValueObject::GetValue(Value &value) const
{
    if ((head_.flag & HASH_FLAG) == HASH_FLAG) {
        return -E_NOT_SUPPORT;
    }
    value.assign(valueHashVector_.begin(), valueHashVector_.end());
    return E_OK;
}

int MultiVerValueObject::SetValue(const Value &value)
{
    head_.flag = 0;
    valueHashVector_.clear();
    valueHashVector_.shrink_to_fit();
    valueHashVector_.assign(value.begin(), value.end());
    return E_OK;
}

bool MultiVerValueObject::IsHash() const
{
    return (head_.flag & HASH_FLAG) == HASH_FLAG;
}

void MultiVerValueObject::SetFlag(uint8_t flag)
{
    head_.flag = flag;
}
}
#endif
