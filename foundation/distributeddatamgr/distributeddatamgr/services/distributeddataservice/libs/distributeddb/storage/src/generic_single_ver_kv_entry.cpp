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

#include "generic_single_ver_kv_entry.h"

#include <algorithm>
#include "db_errno.h"
#include "parcel.h"
#include "version.h"

namespace DistributedDB {
namespace {
    enum OperType {
        SERIALIZE,
        DESERIALIZE,
        CAL_LEN,
    };
} // namespace

GenericSingleVerKvEntry::GenericSingleVerKvEntry()
{
}

GenericSingleVerKvEntry::~GenericSingleVerKvEntry()
{
}

std::string GenericSingleVerKvEntry::GetOrigDevice() const
{
    return dataItem_.origDev;
}

void GenericSingleVerKvEntry::SetOrigDevice(const std::string &dev)
{
    dataItem_.origDev = dev;
}

TimeStamp GenericSingleVerKvEntry::GetTimestamp() const
{
    return dataItem_.timeStamp;
}

void GenericSingleVerKvEntry::SetTimestamp(TimeStamp time)
{
    dataItem_.timeStamp = time;
}

TimeStamp GenericSingleVerKvEntry::GetWriteTimestamp() const
{
    return dataItem_.writeTimeStamp;
}

void GenericSingleVerKvEntry::SetWriteTimestamp(TimeStamp time)
{
    dataItem_.writeTimeStamp = time;
}

void GenericSingleVerKvEntry::SetEntryData(DataItem &&dataItem)
{
    dataItem_ = dataItem;
}

void GenericSingleVerKvEntry::GetKey(Key &key) const
{
    key = dataItem_.key;
}

void GenericSingleVerKvEntry::GetValue(Value &value) const
{
    value = dataItem_.value;
}

uint64_t GenericSingleVerKvEntry::GetFlag() const
{
    return dataItem_.flag;
}

// this func should do compatible
int GenericSingleVerKvEntry::SerializeData(Parcel &parcel, uint32_t targetVersion)
{
    uint64_t len = 0;
    int errCode = parcel.WriteUInt32(targetVersion);
    if (errCode != E_OK) {
        return errCode;
    }
    errCode = AdaptToVersion(SERIALIZE, targetVersion, parcel, len);
    if (errCode != E_OK) {
        return errCode;
    }
    return errCode;
}

int GenericSingleVerKvEntry::SerializeDatas(const std::vector<SingleVerKvEntry *> &kvEntries, Parcel &parcel,
    uint32_t targetVersion)
{
    LOGD("GenericSingleVerKvEntry::SerialDatas targetVersion:%d", targetVersion);
    uint32_t size = kvEntries.size();
    int errCode = parcel.WriteUInt32(size);
    if (errCode != E_OK) {
        return errCode;
    }
    parcel.EightByteAlign();
    for (const auto &kvEntry : kvEntries) {
        if (kvEntry == nullptr) {
            continue;
        }
        errCode = kvEntry->SerializeData(parcel, targetVersion);
        if (errCode != E_OK) {
            return errCode;
        }
    }
    return errCode;
}

// this func should do compatible
uint32_t GenericSingleVerKvEntry::CalculateLen(uint32_t targetVersion)
{
    uint64_t len = 0;
    int errCode = AdaptToVersion(CAL_LEN, targetVersion, len);
    if ((len > INT32_MAX) || (errCode != E_OK)) {
        return 0;
    }
    return len;
}

uint32_t GenericSingleVerKvEntry::CalculateLens(const std::vector<SingleVerKvEntry *> &kvEntries,
    uint32_t targetVersion)
{
    LOGD("GenericSingleVerKvEntry::CalculateLen targetVersion:%d", targetVersion);
    uint64_t len = 0;
    len += Parcel::GetUInt32Len();
    len = BYTE_8_ALIGN(len);
    for (const auto &kvEntry : kvEntries) {
        if (kvEntry == nullptr) {
            continue;
        }
        len += kvEntry->CalculateLen(targetVersion);
        if (len > INT32_MAX) {
            return 0;
        }
    }
    return len;
}

// this func should do compatible
int GenericSingleVerKvEntry::DeSerializeData(Parcel &parcel)
{
    uint32_t version = VERSION_INVALID;
    uint64_t len = parcel.ReadUInt32(version);
    if (parcel.IsError()) {
        return 0;
    }
    int errCode = AdaptToVersion(DESERIALIZE, version, parcel, len);
    if (errCode != E_OK) {
        len = 0;
    }
    return len;
}

int GenericSingleVerKvEntry::DeSerializeDatas(std::vector<SingleVerKvEntry *> &kvEntries, Parcel &parcel)
{
    uint64_t len = 0;
    uint32_t size = 0;
    len += parcel.ReadUInt32(size);
    parcel.EightByteAlign();
    len = BYTE_8_ALIGN(len);
    for (uint32_t i = 0; i < size; i++) {
        auto kvEntry = new (std::nothrow) GenericSingleVerKvEntry();
        if (kvEntry == nullptr) {
            LOGE("Create kvEntry failed.");
            len = 0;
            goto END;
        }
        len += kvEntry->DeSerializeData(parcel);
        kvEntries.push_back(kvEntry);
        if (len > INT32_MAX) {
            len = 0;
            goto END;
        }
    }
END:
    if (len == 0) {
        for (auto &kvEntry : kvEntries) {
            delete kvEntry;
            kvEntry = nullptr;
        }
    }
    return len;
}

int GenericSingleVerKvEntry::AdaptToVersion(int operType, uint32_t targetVersion, Parcel &parcel, uint64_t &datalen)
{
    if (targetVersion < SOFTWARE_VERSION_EARLIEST || targetVersion > SOFTWARE_VERSION_CURRENT) {
        return -E_VERSION_NOT_SUPPORT;
    }
    int errCode = E_OK;
    switch (operType) {
        case SERIALIZE:
            errCode = SerializeDataByVersion(targetVersion, parcel);
            break;
        case DESERIALIZE:
            errCode = DeSerializeByVersion(targetVersion, parcel, datalen);
            break;
        default:
            LOGE("Unknown upgrade serialize oper!");
            return -E_UPGRADE_FAILED;
    }
    return errCode;
}

int GenericSingleVerKvEntry::AdaptToVersion(int operType, uint32_t targetVersion, uint64_t &datalen)
{
    if (targetVersion < SOFTWARE_VERSION_EARLIEST || targetVersion > SOFTWARE_VERSION_CURRENT) {
        return -E_VERSION_NOT_SUPPORT;
    }

    if (operType == CAL_LEN) {
        return CalLenByVersion(targetVersion, datalen);
    } else {
        LOGE("Unknown upgrade serialize oper!");
        return -E_UPGRADE_FAILED;
    }
}

int GenericSingleVerKvEntry::SerializeDataByFirstVersion(Parcel &parcel) const
{
    int errCode = parcel.WriteVectorChar(dataItem_.key);
    if (errCode != E_OK) {
        return errCode;
    }
    errCode = parcel.WriteVectorChar(dataItem_.value);
    if (errCode != E_OK) {
        return errCode;
    }
    errCode = parcel.WriteUInt64(dataItem_.timeStamp);
    if (errCode != E_OK) {
        return errCode;
    }
    errCode = parcel.WriteUInt64(dataItem_.flag);
    if (errCode != E_OK) {
        return errCode;
    }

    return parcel.WriteString(dataItem_.origDev);
}

int GenericSingleVerKvEntry::SerializeDataByLaterVersion(Parcel &parcel) const
{
    TimeStamp writeTimeStamp = dataItem_.writeTimeStamp;
    if (writeTimeStamp == 0) {
        writeTimeStamp = dataItem_.timeStamp;
    }

    return parcel.WriteUInt64(writeTimeStamp);
}

int GenericSingleVerKvEntry::SerializeDataByVersion(uint32_t targetVersion, Parcel &parcel) const
{
    int errCode = SerializeDataByFirstVersion(parcel);
    if (targetVersion == SOFTWARE_VERSION_EARLIEST || errCode != E_OK) {
        return errCode;
    }
    return SerializeDataByLaterVersion(parcel);
}

void GenericSingleVerKvEntry::CalLenByFirstVersion(uint64_t &len) const
{
    len += Parcel::GetUInt32Len();
    len += Parcel::GetVectorCharLen(dataItem_.key);
    len += Parcel::GetVectorCharLen(dataItem_.value);
    len += Parcel::GetUInt64Len();
    len += Parcel::GetUInt64Len();
    len += Parcel::GetStringLen(dataItem_.origDev);
}

void GenericSingleVerKvEntry::CalLenByLaterVersion(uint64_t &len) const
{
    len += Parcel::GetUInt64Len();
}

int GenericSingleVerKvEntry::CalLenByVersion(uint32_t targetVersion, uint64_t &len) const
{
    CalLenByFirstVersion(len);
    if (targetVersion == SOFTWARE_VERSION_EARLIEST) {
        return E_OK;
    }
    CalLenByLaterVersion(len);
    return E_OK;
}

void GenericSingleVerKvEntry::DeSerializeByFirstVersion(uint64_t &len, Parcel &parcel)
{
    len += parcel.ReadVectorChar(dataItem_.key);
    len += parcel.ReadVectorChar(dataItem_.value);
    len += parcel.ReadUInt64(dataItem_.timeStamp);
    len += parcel.ReadUInt64(dataItem_.flag);
    len += parcel.ReadString(dataItem_.origDev);
    dataItem_.writeTimeStamp = dataItem_.timeStamp;
}

void GenericSingleVerKvEntry::DeSerializeByLaterVersion(uint64_t &len, Parcel &parcel)
{
    len += parcel.ReadUInt64(dataItem_.writeTimeStamp);
}

int GenericSingleVerKvEntry::DeSerializeByVersion(uint32_t targetVersion, Parcel &parcel, uint64_t &len)
{
    DeSerializeByFirstVersion(len, parcel);
    if (targetVersion == SOFTWARE_VERSION_EARLIEST) {
        return E_OK;
    }
    DeSerializeByLaterVersion(len, parcel);
    return E_OK;
}
} // namespace DistributedDB
