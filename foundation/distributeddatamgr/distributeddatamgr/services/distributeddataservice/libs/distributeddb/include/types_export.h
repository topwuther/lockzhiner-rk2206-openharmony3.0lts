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

#ifndef DISTRIBUTEDDB_TYPES_EXPORT_H
#define DISTRIBUTEDDB_TYPES_EXPORT_H

#include <cstdint>
#include <vector>
#include <climits>
#include <string>
#include <functional>

namespace DistributedDB {
#ifdef _WIN32
    #ifdef DB_DLL_EXPORT
        #define DB_API __declspec(dllexport)
    #else
        #define DB_API
    #endif
#else
    #define DB_API __attribute__ ((visibility ("default")))
#endif

#define DB_SYMBOL DB_API

using Key = std::vector<uint8_t>;
using Value = std::vector<uint8_t>;

struct Entry {
    Key key;
    Value value;
};

enum class CipherType {
    DEFAULT,
    AES_256_GCM, // AES-256-GCM
};

class CipherPassword final {
public:
    enum ErrorCode {
        OK = 0,
        OVERSIZE,
        INVALID_INPUT,
        SECUREC_ERROR,
    };

    DB_API CipherPassword();
    DB_API ~CipherPassword();

    DB_API bool operator==(const CipherPassword &input) const;
    DB_API bool operator!=(const CipherPassword &input) const;

    DB_API size_t GetSize() const;
    DB_API const uint8_t *GetData() const;
    DB_API int SetValue(const uint8_t *inputData, size_t inputSize);
    DB_API int Clear();

private:
    static const size_t MAX_PASSWORD_SIZE = 128;
    uint8_t data_[MAX_PASSWORD_SIZE] = {UCHAR_MAX};
    size_t size_ = 0;
};

using PragmaData = void *;

struct PragmaEntryDeviceIdentifier {
    Key key;
    bool origDevice = true;
    std::string deviceIdentifier;
};

using KvStoreNbPublishAction = std::function<void (const Entry &local, const Entry *sync, bool isLocalLastest)>;

struct PragmaDeviceIdentifier {
    std::string deviceID;
    std::string deviceIdentifier;
};

enum WipePolicy {
    RETAIN_STALE_DATA = 1, // remote stale data will be retained in syncing when remote db rebuiled.
    WIPE_STALE_DATA // remote stale data will be wiped when in syncing remote db rebuiled.
};

// We don't parse, read or modify the array type, so there are not a corresponding array value
// The leaf object is empty, an internal object always composed by other type values.
struct FieldValue {
    union {
        bool boolValue;
        int32_t integerValue;
        int64_t longValue = 0;
        double doubleValue;
    };
    std::string stringValue;
};

enum PermissionCheckFlag {
    CHECK_FLAG_SEND = 1, // send
    CHECK_FLAG_RECEIVE = 2, // receive
};

using PermissionCheckCallback = std::function<bool (const std::string &userId, const std::string &appId,
    const std::string &storeId, uint8_t flag)>;

using PermissionCheckCallbackV2 = std::function<bool (const std::string &userId, const std::string &appId,
    const std::string &storeId, const std::string &deviceId, uint8_t flag)>;

enum AutoLaunchStatus {
    WRITE_OPENED = 1,
    WRITE_CLOSED = 2,
    INVALID_PARAM = 3, // AutoLaunchRequestCallback, if param check failed
};

using AutoLaunchNotifier = std::function<void (const std::string &userId,
    const std::string &appId, const std::string &storeId, AutoLaunchStatus status)>;

enum SecurityLabel : int {
    INVALID_SEC_LABEL = -1,
    NOT_SET = 0,
    S0,
    S1,
    S2,
    S3,
    S4
};

// security flag type
enum SecurityFlag : int {
    INVALID_SEC_FLAG = -1,
    ECE = 0,
    SECE
};

struct SecurityOption {
    int securityLabel = 0; // the securityLabel is the class of data sensitive, see enum SecurityLabel
    int securityFlag = 0;  // the securityFlag is the encryption method of the file only used for S3 like 0:ECE, 1:SECE
    bool operator==(const SecurityOption &rhs) const
    {
        return securityLabel == rhs.securityLabel && securityFlag == rhs.securityFlag;
    }
};
} // namespace DistributedDB

enum class ResultSetCacheMode : int {
    CACHE_FULL_ENTRY = 0,       // Ordinary mode efficient when sequential access, the default mode
    CACHE_ENTRY_ID_ONLY = 1,    // Special mode efficient when random access
};

struct RemotePushNotifyInfo {
    std::string deviceId;
};

using RemotePushFinishedNotifier = std::function<void (const RemotePushNotifyInfo &info)>;
using RemotePushFinisheNotifier = RemotePushFinishedNotifier; // To correct spelling errors in the previous version
#endif // DISTRIBUTEDDB_TYPES_EXPORT_H
