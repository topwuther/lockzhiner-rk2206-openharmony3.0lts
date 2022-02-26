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

#ifndef APP_DISTRIBUTED_KVSTORE_APP_TYPES_H
#define APP_DISTRIBUTED_KVSTORE_APP_TYPES_H

#include <errors.h>
#include <cstdint>
#include <string>
#include <vector>
#include "app_blob.h"
#include "visibility.h"

namespace OHOS {
namespace AppDistributedKv {
// Key set by the client, which can be any non-empty byte array with a length of less than 256 bytes.
using Key = OHOS::AppDistributedKv::AppBlob;

// Value set by client, which can be any byte array.
using Value = OHOS::AppDistributedKv::AppBlob;

// User ID from the user account
struct UserId {
    std::string userId;
};

// App ID from the BMS
struct AppId {
    std::string appId;
};

struct PipeInfo {
    std::string pipeId;
    std::string userId;
};

struct DeviceInfo {
    std::string deviceId;
    std::string deviceName;
    std::string deviceType;
};

enum class MessageType {
    DEFAULT = 0,
    FILE = 1,
};

struct MessageInfo {
    MessageType msgType;
};

enum class DeviceChangeType {
    DEVICE_OFFLINE = 0,
    DEVICE_ONLINE = 1,
};

struct DeviceId {
    std::string deviceId;
};

// app_distributed_data_manager using sub error code 0
constexpr ErrCode APP_DISTRIBUTEDDATAMGR_ERR_OFFSET = ErrCodeOffset(SUBSYS_DISTRIBUTEDDATAMNG, 0);

enum class Status {
    SUCCESS = ERR_OK,
    ERROR = APP_DISTRIBUTEDDATAMGR_ERR_OFFSET,
    INVALID_ARGUMENT = APP_DISTRIBUTEDDATAMGR_ERR_OFFSET + 1,
    ILLEGAL_STATE = APP_DISTRIBUTEDDATAMGR_ERR_OFFSET + 2,
    STORE_NOT_OPEN = APP_DISTRIBUTEDDATAMGR_ERR_OFFSET + 3,
    STORE_NOT_FOUND = APP_DISTRIBUTEDDATAMGR_ERR_OFFSET + 4,
    STORE_ALREADY_SUBSCRIBE = APP_DISTRIBUTEDDATAMGR_ERR_OFFSET + 5,
    STORE_NOT_SUBSCRIBE = APP_DISTRIBUTEDDATAMGR_ERR_OFFSET + 6,
    KEY_NOT_FOUND = APP_DISTRIBUTEDDATAMGR_ERR_OFFSET + 7,
    DB_ERROR = APP_DISTRIBUTEDDATAMGR_ERR_OFFSET + 8,
    DEVICE_NOT_FOUND = APP_DISTRIBUTEDDATAMGR_ERR_OFFSET + 9,
    NETWORK_ERROR = APP_DISTRIBUTEDDATAMGR_ERR_OFFSET + 10,
    NO_DEVICE_CONNECTED = APP_DISTRIBUTEDDATAMGR_ERR_OFFSET + 11,
    PERMISSION_DENIED = APP_DISTRIBUTEDDATAMGR_ERR_OFFSET + 12,
    TIME_OUT = APP_DISTRIBUTEDDATAMGR_ERR_OFFSET + 13,
    REPEATED_REGISTER = APP_DISTRIBUTEDDATAMGR_ERR_OFFSET + 14,
    CREATE_SESSION_ERROR = APP_DISTRIBUTEDDATAMGR_ERR_OFFSET + 15,
    SECURITY_LEVEL_ERROR = APP_DISTRIBUTEDDATAMGR_ERR_OFFSET + 32,
};

enum class SubscribeType {
    DEFAULT = 0, // reserved value, to be deleted
    OBSERVER_CHANGES_NATIVE = 1, // native changes of syncable KvStore
    OBSERVER_CHANGES_FOREIGN = 2, // synced data changes from remote devices
    OBSERVER_CHANGES_ALL = 3, // both native changes and synced data changes
};

struct Entry {
    Key key;
    Value value;
};

enum class SyncMode {
    PULL,
    PUSH,
    PUSH_PULL,
};

enum ConflictResolvePolicy {
    LAST_WIN = 0,
    DEVICE_COLLABORATION,
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

struct Options {
    bool createIfMissing = false;
    bool encrypt = false;
    bool persistant = false;
    int conflictResolvePolicy = LAST_WIN;
    int securityLevel = SecurityLevel::NO_LABEL;
};

struct WriteOptions {
    bool local;
};

struct ReadOptions {
    bool local;
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

enum class AppKvStoreConflictPolicyType {
    CONFIICT_FOREIGN_KEY_ONLY = 0x01,
    CONFLICT_FOREIGN_KEY_ORIG = 0x02,
    CONFLICT_NATIVE_ALL = 0x0c,
};
}  // namespace AppDistributedKv
}  // namespace OHOS

#endif  // APP_DISTRIBUTED_KVSTORE_TYPES_H
