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

#ifndef META_DATA_H
#define META_DATA_H

#include <mutex>
#include <vector>
#include <map>

#include "db_types.h"
#include "ikvdb_sync_interface.h"
#include "ref_object.h"

namespace DistributedDB {
struct MetaDataValue {
    TimeOffset timeOffset = 0;
    uint64_t lastUpdateTime = 0;
    uint64_t localWaterMark = 0;
    uint64_t peerWaterMark = 0;
};

class Metadata {
public:
    Metadata();
    ~Metadata();

    int Initialize(IKvDBSyncInterface *storage);

    int SaveTimeOffset(const DeviceID &deviceId, TimeOffset inValue);

    void GetTimeOffset(const DeviceID &deviceId, TimeOffset &outValue);

    void GetLocalWaterMark(const DeviceID &deviceId, uint64_t &outValue);

    int SaveLocalWaterMark(const DeviceID &deviceId, uint64_t inValue);

    void GetPeerWaterMark(const DeviceID &deviceId, uint64_t &outValue);

    int SavePeerWaterMark(const DeviceID &deviceId, uint64_t inValue, bool isNeedHash = true);

    int SaveLocalTimeOffset(TimeOffset timeOffset);

    TimeOffset GetLocalTimeOffset() const;

    int EraseDeviceWaterMark(const std::string &deviceId, bool isNeedHash = true);

    void SetLastLocalTime(TimeStamp lastLocalTime);

    TimeStamp GetLastLocalTime() const;

private:

    int SaveMetaDataValue(const DeviceID &deviceId, const MetaDataValue &inValue);

    void GetMetaDataValue(const DeviceID &deviceId, MetaDataValue &outValue, bool isNeedHash = true);

    int SerializeMetaData(const MetaDataValue &inValue, std::vector<uint8_t> &outValue);

    int DeSerializeMetaData(const std::vector<uint8_t> &inValue, MetaDataValue &outValue) const;

    int GetMetadataFromDb(const std::vector<uint8_t> &key, std::vector<uint8_t> &outValue);

    int SetMetadataToDb(const std::vector<uint8_t> &key, const std::vector<uint8_t> &inValue);

    void PutMetadataToMap(const DeviceID &deviceId, const MetaDataValue &value);

    void GetMetadataFromMap(const DeviceID &deviceId, MetaDataValue &outValue);

    int64_t StringToLong(const std::vector<uint8_t> &value);

    int GetAllMetadataKey(std::vector<std::vector<uint8_t>> &keys);

    int LoadAllMetadata();

    uint64_t GetRandTimeOffset() const;

    void GetHashDeviceId(const DeviceID &deviceId, DeviceID &hashDeviceId, bool isNeedHash = true);

    // store localTimeOffset in ram; if change, should add a lock first, change here and metadata,
    // then release lock
    std::atomic<TimeOffset> localTimeOffset_;
    std::mutex localTimeOffsetLock_;
    IKvDBSyncInterface *naturalStoragePtr_;

    // if changed, it should be locked from save-to-db to change-in-memory.save to db must be first,
    // if save to db fail, it will not be changed in memory.
    std::map<std::string, MetaDataValue> metadataMap_;
    std::mutex metadataLock_;
    std::map<DeviceID, DeviceID> deviceIdToHashDeviceIdMap_;

    // store localTimeOffset in ram, used to make timestamp increase
    mutable std::mutex lastLocalTimeLock_;
    TimeStamp lastLocalTime_;
};
}  // namespace DistributedDB
#endif
