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

#include "meta_data.h"

#include <openssl/rand.h>
#include "securec.h"
#include "db_errno.h"
#include "db_common.h"
#include "log_print.h"
#include "time_helper.h"
#include "hash.h"

namespace DistributedDB {
namespace {
    const int STR_TO_LL_BY_DEC = 10;
    // store local timeoffset;this is a special key;
    const std::string LOCALTIMEOFFSET_KEY = "localTimeOffset";
    const std::string DEVICEID_PREFIX_KEY = "deviceId";
}

Metadata::Metadata()
    : localTimeOffset_(0),
      naturalStoragePtr_(nullptr),
      lastLocalTime_(0)
{}

Metadata::~Metadata()
{
    naturalStoragePtr_ = nullptr;
    metadataMap_.clear();
}

int Metadata::Initialize(IKvDBSyncInterface* storage)
{
    naturalStoragePtr_ = storage;
    std::vector<uint8_t> key;
    std::vector<uint8_t> timeOffset;
    DBCommon::StringToVector(LOCALTIMEOFFSET_KEY, key);

    int errCode = GetMetadataFromDb(key, timeOffset);
    if (errCode == -E_NOT_FOUND) {
        uint64_t randTimeOffset = GetRandTimeOffset();
        SaveLocalTimeOffset(TimeHelper::BASE_OFFSET + randTimeOffset);
    } else {
        localTimeOffset_ = StringToLong(timeOffset);
    }
    {
        std::lock_guard<std::mutex> lockGuard(metadataLock_);
        metadataMap_.clear();
    }
    return LoadAllMetadata();
}

int Metadata::SaveTimeOffset(const DeviceID &deviceId, TimeOffset inValue)
{
    MetaDataValue metadata;
    std::lock_guard<std::mutex> lockGuard(metadataLock_);
    GetMetaDataValue(deviceId, metadata);
    metadata.timeOffset = inValue;
    metadata.lastUpdateTime = TimeHelper::GetSysCurrentTime();
    LOGD("Metadata::SaveTimeOffset = %lld dev %s{private}", inValue, deviceId.c_str());
    return SaveMetaDataValue(deviceId, metadata);
}

void Metadata::GetTimeOffset(const DeviceID &deviceId, TimeOffset &outValue)
{
    MetaDataValue metadata;
    std::lock_guard<std::mutex> lockGuard(metadataLock_);
    GetMetaDataValue(deviceId, metadata);
    outValue = metadata.timeOffset;
}

void Metadata::GetLocalWaterMark(const DeviceID &deviceId, uint64_t &outValue)
{
    MetaDataValue metadata;
    std::lock_guard<std::mutex> lockGuard(metadataLock_);
    GetMetaDataValue(deviceId, metadata);
    outValue = metadata.localWaterMark;
}

int Metadata::SaveLocalWaterMark(const DeviceID &deviceId, uint64_t inValue)
{
    MetaDataValue metadata;
    std::lock_guard<std::mutex> lockGuard(metadataLock_);
    GetMetaDataValue(deviceId, metadata);
    metadata.localWaterMark = inValue;
    LOGD("Metadata::SaveLocalWaterMark = %llu\n", inValue);
    return SaveMetaDataValue(deviceId, metadata);
}

void Metadata::GetPeerWaterMark(const DeviceID &deviceId, uint64_t &outValue)
{
    MetaDataValue metadata;
    std::lock_guard<std::mutex> lockGuard(metadataLock_);
    GetMetaDataValue(deviceId, metadata);
    outValue = metadata.peerWaterMark;
}

int Metadata::SavePeerWaterMark(const DeviceID &deviceId, uint64_t inValue, bool isNeedHash)
{
    MetaDataValue metadata;
    std::lock_guard<std::mutex> lockGuard(metadataLock_);
    GetMetaDataValue(deviceId, metadata, isNeedHash);
    metadata.peerWaterMark = inValue;
    LOGD("Metadata::SavePeerWaterMark = %llu", inValue);
    return SaveMetaDataValue(deviceId, metadata);
}

int Metadata::SaveLocalTimeOffset(TimeOffset timeOffset)
{
    std::string timeOffsetString = std::to_string(timeOffset);
    std::vector<uint8_t> timeOffsetValue(timeOffsetString.begin(), timeOffsetString.end());
    std::vector<uint8_t> localTimeOffsetValue(
        LOCALTIMEOFFSET_KEY.begin(), LOCALTIMEOFFSET_KEY.end());

    std::lock_guard<std::mutex> lockGuard(localTimeOffsetLock_);
    localTimeOffset_ = timeOffset;
    LOGD("Metadata::SaveLocalTimeOffset offset = %lld\n", timeOffset);
    int errCode = SetMetadataToDb(localTimeOffsetValue, timeOffsetValue);
    if (errCode != E_OK) {
        LOGE("Metadata::SaveLocalTimeOffset SetMetadataToDb failed errCode:%d", errCode);
    }
    return errCode;
}

TimeOffset Metadata::GetLocalTimeOffset() const
{
    return localTimeOffset_.load(std::memory_order_seq_cst);
}

int Metadata::EraseDeviceWaterMark(const std::string &deviceId, bool isNeedHash)
{
    return SavePeerWaterMark(deviceId, 0, isNeedHash);
}

void Metadata::SetLastLocalTime(TimeStamp lastLocalTime)
{
    std::lock_guard<std::mutex> lock(lastLocalTimeLock_);
    if (lastLocalTime > lastLocalTime_) {
        lastLocalTime_ = lastLocalTime;
    }
}

TimeStamp Metadata::GetLastLocalTime() const
{
    std::lock_guard<std::mutex> lock(lastLocalTimeLock_);
    return lastLocalTime_;
}

int Metadata::SaveMetaDataValue(const DeviceID &deviceId, const MetaDataValue &inValue)
{
    std::vector<uint8_t> value;
    int errCode = SerializeMetaData(inValue, value);
    if (errCode != E_OK) {
        return errCode;
    }

    DeviceID hashDeviceId;
    GetHashDeviceId(deviceId, hashDeviceId);
    std::vector<uint8_t> key;
    DBCommon::StringToVector(hashDeviceId, key);
    errCode = SetMetadataToDb(key, value);
    if (errCode != E_OK) {
        LOGE("Metadata::SetMetadataToDb failed errCode:%d\n", errCode);
        return errCode;
    }
    PutMetadataToMap(hashDeviceId, inValue);
    return E_OK;
}

void Metadata::GetMetaDataValue(const DeviceID &deviceId, MetaDataValue &outValue, bool isNeedHash)
{
    DeviceID hashDeviceId;
    GetHashDeviceId(deviceId, hashDeviceId, isNeedHash);
    GetMetadataFromMap(hashDeviceId, outValue);
}

int Metadata::SerializeMetaData(const MetaDataValue &inValue, std::vector<uint8_t> &outValue)
{
    outValue.resize(sizeof(MetaDataValue));
    errno_t err = memcpy_s(&outValue[0], outValue.size(), &inValue, sizeof(MetaDataValue));
    if (err != EOK) {
        return -E_SECUREC_ERROR;
    }
    return E_OK;
}

int Metadata::DeSerializeMetaData(const std::vector<uint8_t> &inValue, MetaDataValue &outValue) const
{
    if (inValue.empty()) {
        return -E_INVALID_ARGS;
    }

    errno_t err = memcpy_s(&outValue, sizeof(MetaDataValue), &inValue[0], inValue.size());
    if (err != EOK) {
        return -E_SECUREC_ERROR;
    }
    return E_OK;
}

int Metadata::GetMetadataFromDb(const std::vector<uint8_t> &key, std::vector<uint8_t> &outValue)
{
    if (naturalStoragePtr_ == nullptr) {
        return -E_INVALID_DB;
    }
    return naturalStoragePtr_->GetMetaData(key, outValue);
}

int Metadata::SetMetadataToDb(const std::vector<uint8_t> &key, const std::vector<uint8_t> &inValue)
{
    if (naturalStoragePtr_ == nullptr) {
        return -E_INVALID_DB;
    }
    return naturalStoragePtr_->PutMetaData(key, inValue);
}

void Metadata::PutMetadataToMap(const DeviceID &deviceId, const MetaDataValue &value)
{
    metadataMap_[deviceId] = value;
}

void Metadata::GetMetadataFromMap(const DeviceID &deviceId, MetaDataValue &outValue)
{
    outValue = metadataMap_[deviceId];
}

int64_t Metadata::StringToLong(const std::vector<uint8_t> &value)
{
    std::string valueString(value.begin(), value.end());
    int64_t longData = std::strtoll(valueString.c_str(), nullptr, STR_TO_LL_BY_DEC);
    LOGD("Metadata::StringToLong longData = %lld\n", longData);
    return longData;
}

int Metadata::GetAllMetadataKey(std::vector<std::vector<uint8_t>> &keys)
{
    if (naturalStoragePtr_ == nullptr) {
        return -E_INVALID_DB;
    }
    return naturalStoragePtr_->GetAllMetaKeys(keys);
}

int Metadata::LoadAllMetadata()
{
    std::vector<std::vector<uint8_t>> deviceIds;
    std::vector<uint8_t> value;
    int errCode = E_OK;
    GetAllMetadataKey(deviceIds);

    for (auto it = deviceIds.begin(); it != deviceIds.end(); ++it) {
        if (it->size() < DEVICEID_PREFIX_KEY.size()) {
            continue;
        }
        std::string prefixKey(it->begin(), it->begin() + DEVICEID_PREFIX_KEY.size());
        if (prefixKey != DEVICEID_PREFIX_KEY) {
            continue;
        }
        errCode = GetMetadataFromDb(*it, value);
        if (errCode != E_OK) {
            return errCode;
        }
        MetaDataValue metadata;
        std::string deviceId(it->begin(), it->end());
        errCode = DeSerializeMetaData(value, metadata);
        {
            std::lock_guard<std::mutex> lockGuard(metadataLock_);
            PutMetadataToMap(deviceId, metadata);
        }
    }
    return errCode;
}

uint64_t Metadata::GetRandTimeOffset() const
{
    const int randOffsetLength = 2; // 2 byte
    uint8_t randBytes[randOffsetLength] = { 0 };
    RAND_bytes(randBytes, randOffsetLength);

    // use a 16 bit rand data to make a rand timeoffset
    uint64_t randTimeOffset = (static_cast<uint16_t>(randBytes[1]) << 8) | randBytes[0]; // 16 bit data, 8 is offset
    randTimeOffset = randTimeOffset * 1000 * 1000 * 10; // second, 1000 is scale
    LOGD("[Metadata] GetRandTimeOffset %llu", randTimeOffset);
    return randTimeOffset;
}

void Metadata::GetHashDeviceId(const DeviceID &deviceId, DeviceID &hashDeviceId, bool isNeedHash)
{
    if (!isNeedHash) {
        hashDeviceId = deviceId;
        return;
    }
    if (deviceIdToHashDeviceIdMap_.count(deviceId) == 0) {
        hashDeviceId = DEVICEID_PREFIX_KEY + DBCommon::TransferHashString(deviceId);
        deviceIdToHashDeviceIdMap_.insert(std::pair<DeviceID, DeviceID>(deviceId, hashDeviceId));
    } else {
        hashDeviceId = deviceIdToHashDeviceIdMap_[deviceId];
    }
}
}  // namespace DistributedDB
