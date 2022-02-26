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

#ifndef DISTRIBUTED_KVSTORE_TYPES_H
#define DISTRIBUTED_KVSTORE_TYPES_H

#include <cstdint>
#include <string>
#include <vector>
#include <errors.h>
#include "blob.h"
#include "visibility.h"

namespace OHOS {
namespace DistributedKv {

// key set by client, can be any non-empty bytes array, and less than 1024 size.
using Key = OHOS::DistributedKv::Blob;

// value set by client, can be any bytes array.
using Value = OHOS::DistributedKv::Blob;

// user identifier from user-account
struct UserId {
    std::string userId;
};

// app identifier from Bms
struct AppId {
    std::string appId;
};

// kvstore name set by client by calling GetKvStore,
// storeId len must be less or equal than 256,
// and can not be empty and all space.
struct StoreId {
    std::string storeId;
};

struct KvStoreTuple {
    std::string userId;
    std::string appId;
    std::string storeId;
};

struct AppThreadInfo {
    std::int32_t pid;
    std::int32_t uid;
};

// distributed_data_manager using sub error code 3
constexpr ErrCode DISTRIBUTEDDATAMGR_ERR_OFFSET = ErrCodeOffset(SUBSYS_DISTRIBUTEDDATAMNG, 3);
enum class Status {
    SUCCESS = ERR_OK,
    ERROR = DISTRIBUTEDDATAMGR_ERR_OFFSET,
    INVALID_ARGUMENT = DISTRIBUTEDDATAMGR_ERR_OFFSET + 1,
    ILLEGAL_STATE = DISTRIBUTEDDATAMGR_ERR_OFFSET + 2,
    SERVER_UNAVAILABLE = DISTRIBUTEDDATAMGR_ERR_OFFSET + 3,
    STORE_NOT_OPEN = DISTRIBUTEDDATAMGR_ERR_OFFSET + 4,
    STORE_NOT_FOUND = DISTRIBUTEDDATAMGR_ERR_OFFSET + 5,
    STORE_ALREADY_SUBSCRIBE = DISTRIBUTEDDATAMGR_ERR_OFFSET + 6,
    STORE_NOT_SUBSCRIBE = DISTRIBUTEDDATAMGR_ERR_OFFSET + 7,
    KEY_NOT_FOUND = DISTRIBUTEDDATAMGR_ERR_OFFSET + 8,
    DB_ERROR = DISTRIBUTEDDATAMGR_ERR_OFFSET + 9,
    NETWORK_ERROR = DISTRIBUTEDDATAMGR_ERR_OFFSET + 10,
    NO_DEVICE_CONNECTED = DISTRIBUTEDDATAMGR_ERR_OFFSET + 11,
    PERMISSION_DENIED = DISTRIBUTEDDATAMGR_ERR_OFFSET + 12,
    IPC_ERROR = DISTRIBUTEDDATAMGR_ERR_OFFSET + 13,
    CRYPT_ERROR = DISTRIBUTEDDATAMGR_ERR_OFFSET + 14,
    TIME_OUT = DISTRIBUTEDDATAMGR_ERR_OFFSET + 15,
    DEVICE_NOT_FOUND = DISTRIBUTEDDATAMGR_ERR_OFFSET + 16,
    NOT_SUPPORT = DISTRIBUTEDDATAMGR_ERR_OFFSET + 17,
    SCHEMA_MISMATCH = DISTRIBUTEDDATAMGR_ERR_OFFSET + 18,
    INVALID_SCHEMA = DISTRIBUTEDDATAMGR_ERR_OFFSET + 19,
    READ_ONLY = DISTRIBUTEDDATAMGR_ERR_OFFSET + 20,
    INVALID_VALUE_FIELDS = DISTRIBUTEDDATAMGR_ERR_OFFSET + 21,
    INVALID_FIELD_TYPE = DISTRIBUTEDDATAMGR_ERR_OFFSET + 22,
    CONSTRAIN_VIOLATION = DISTRIBUTEDDATAMGR_ERR_OFFSET + 23,
    INVALID_FORMAT = DISTRIBUTEDDATAMGR_ERR_OFFSET + 24,
    INVALID_QUERY_FORMAT = DISTRIBUTEDDATAMGR_ERR_OFFSET + 25,
    INVALID_QUERY_FIELD = DISTRIBUTEDDATAMGR_ERR_OFFSET + 26,
    SYSTEM_ACCOUNT_EVENT_PROCESSING = DISTRIBUTEDDATAMGR_ERR_OFFSET + 27,
    RECOVER_SUCCESS = DISTRIBUTEDDATAMGR_ERR_OFFSET + 28,
    RECOVER_FAILED = DISTRIBUTEDDATAMGR_ERR_OFFSET + 29,
    MIGRATION_KVSTORE_FAILED = DISTRIBUTEDDATAMGR_ERR_OFFSET + 30,
    EXCEED_MAX_ACCESS_RATE = DISTRIBUTEDDATAMGR_ERR_OFFSET + 31,
    SECURITY_LEVEL_ERROR = DISTRIBUTEDDATAMGR_ERR_OFFSET + 32,
};

enum class SubscribeType {
    DEFAULT = 0, // default let bms delete
    SUBSCRIBE_TYPE_LOCAL = 1, // local changes of syncable kv store
    SUBSCRIBE_TYPE_REMOTE = 2, // synced data changes from remote devices
    SUBSCRIBE_TYPE_ALL = 3, // both local changes and synced data changes
};

struct Entry : public virtual Parcelable {
    Key key;
    Value value;
    // Write a parcelable object to the given parcel.
    // The object position is saved into Parcel if set savePosition to
    // true, and this intends to use in kernel data transaction.
    // Returns size being written on success or zero if any error occur.
    bool Marshalling(Parcel &parcel) const override
    {
        if (!parcel.WriteParcelable(&key)) {
            return false;
        }
        if (!parcel.WriteParcelable(&value)) {
            return false;
        }
        return true;
    }

    // Read data from the given parcel into this parcelable object.
    // Returns size being read on success or zero if any error occur.
    static Entry *Unmarshalling(Parcel &parcel)
    {
        Entry *entry = new Entry;

        bool noError = true;
        sptr<Key> keyTmp = parcel.ReadParcelable<Key>();
        if (keyTmp != nullptr) {
            entry->key = *keyTmp;
        } else {
            noError = false;
        }

        sptr<Value> valueTmp = parcel.ReadParcelable<Value>();
        if (valueTmp != nullptr) {
            entry->value = *valueTmp;
        } else {
            noError = false;
        }

        if (!noError) {
            delete entry;
            entry = nullptr;
        }
        return entry;
    }

    KVSTORE_API virtual ~Entry()
    {}
};

enum class SyncPolicy {
    LOW,
    MEDIUM,
    HIGH,
    HIGHTEST,
};

enum class SyncMode {
    PULL,
    PUSH,
    PUSH_PULL,
};

enum KvStoreType : int32_t {
    DEVICE_COLLABORATION,
    SINGLE_VERSION,
    MULTI_VERSION,
    INVALID_TYPE,
};

enum SecurityLevel : int {
    NO_LABEL,
    S0,
    S1,
    S2,
    S3_EX,
    S3,
    S4,
};

enum class KvControlCmd {
    SET_SYNC_PARAM = 1,
    GET_SYNC_PARAM,
};

using KvParam = OHOS::DistributedKv::Blob;

struct KvSyncParam {
    uint32_t allowedDelayMs { 0 };
};

enum class DeviceChangeType {
    DEVICE_OFFLINE = 0,
    DEVICE_ONLINE = 1,
};

struct DeviceInfo {
    std::string deviceId;
    std::string deviceName;
    std::string deviceType;

    bool Marshalling(Parcel &data) const
    {
        data.WriteString(deviceId);
        data.WriteString(deviceName);
        data.WriteString(deviceType);
        return true;
    }

    static DeviceInfo* UnMarshalling(Parcel &data)
    {
        auto deviceInfoPtr = new (std::nothrow)DeviceInfo();
        if (deviceInfoPtr != nullptr) {
            data.ReadString(deviceInfoPtr->deviceId);
            data.ReadString(deviceInfoPtr->deviceName);
            data.ReadString(deviceInfoPtr->deviceType);
        }
        return deviceInfoPtr;
    }
};

enum class DeviceFilterStrategy {
    FILTER = 0,
    NO_FILTER = 1,
};

struct Options {
    bool createIfMissing = true;
    bool encrypt = false;
    bool persistant = false;
    bool backup = true;
    bool autoSync = true;
    int securityLevel = SecurityLevel::NO_LABEL;
    SyncPolicy syncPolicy = SyncPolicy::HIGH;
    KvStoreType kvStoreType = KvStoreType::DEVICE_COLLABORATION;
    bool syncable = true; // let bms delete first
    std::string schema = "";
    bool dataOwnership = true; // true indicates the ownership of distributed data is DEVICE, otherwise, ACCOUNT
};

template<typename T>
std::vector<uint8_t> TransferTypeToByteArray(const T &t)
{
    return std::vector<uint8_t>(reinterpret_cast<uint8_t *>(const_cast<T *>(&t)),
                                reinterpret_cast<uint8_t *>(const_cast<T *>(&t)) + sizeof(T));
}

template<typename T>
T TransferByteArrayToType(const std::vector<uint8_t> &blob)
{
    // replace assert to HILOG_FATAL when HILOG_FATAL is ok.
    if (blob.size() != sizeof(T) || blob.size() == 0) {
        constexpr int tSize = sizeof(T);
        uint8_t tContent[tSize] = { 0 };
        return *reinterpret_cast<T *>(tContent);
    }
    return *reinterpret_cast<T *>(const_cast<uint8_t *>(&blob[0]));
}
}  // namespace DistributedKv
}  // namespace OHOS
#endif  // DISTRIBUTED_KVSTORE_TYPES_H
