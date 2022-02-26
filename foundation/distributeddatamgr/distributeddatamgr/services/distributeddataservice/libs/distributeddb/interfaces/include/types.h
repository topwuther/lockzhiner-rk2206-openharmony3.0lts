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

#ifndef KV_STORE_TYPE_H
#define KV_STORE_TYPE_H

#include <functional>
#include <string>
#include <vector>

#include "types_export.h"

namespace DistributedDB {
enum DBStatus {
    DB_ERROR = -1,
    OK = 0,
    BUSY,
    NOT_FOUND,
    INVALID_ARGS,
    TIME_OUT,
    NOT_SUPPORT,
    INVALID_PASSWD_OR_CORRUPTED_DB,
    OVER_MAX_LIMITS,
    INVALID_FILE,
    NO_PERMISSION,
    FILE_ALREADY_EXISTED,
    SCHEMA_MISMATCH,
    INVALID_SCHEMA,
    READ_ONLY,
    INVALID_VALUE_FIELDS, // invalid put value for json schema.
    INVALID_FIELD_TYPE, // invalid put value field type for json schema.
    CONSTRAIN_VIOLATION, // invalid put value constrain for json schema.
    INVALID_FORMAT, // invalid put value format for json schema.
    STALE, // new record is staler compared to the same key exist in db
    LOCAL_DELETED, // local data is deleted by the unpublish.
    LOCAL_DEFEAT, // local data defeat the sync data while unpublish.
    LOCAL_COVERED, // local data is coverd by the sync data while unpublish.
    INVALID_QUERY_FORMAT,
    INVALID_QUERY_FIELD,
    PERMISSION_CHECK_FORBID_SYNC, // permission check result , forbid sync.
    ALREADY_SET, // already set.
    COMM_FAILURE, // communicator may get some error.
    EKEYREVOKED_ERROR, // EKEYREVOKED error when operating db file
    SECURITY_OPTION_CHECK_ERROR, // such as remote device's SecurityOption not equal to local
    SCHEMA_VIOLATE_VALUE, // Values already exist in dbFile do not match new schema
};

struct KvStoreConfig {
    std::string dataDir;
};

enum PragmaCmd {
    AUTO_SYNC = 1,
    SYNC_DEVICES = 2,
    RM_DEVICE_DATA = 3, // remove the device data synced from remote by device name
    PERFORMANCE_ANALYSIS_GET_REPORT,
    PERFORMANCE_ANALYSIS_OPEN,
    PERFORMANCE_ANALYSIS_CLOSE,
    PERFORMANCE_ANALYSIS_SET_REPORTFILENAME,
    GET_IDENTIFIER_OF_DEVICE,
    GET_DEVICE_IDENTIFIER_OF_ENTRY,
    GET_QUEUED_SYNC_SIZE,
    SET_QUEUED_SYNC_LIMIT,
    GET_QUEUED_SYNC_LIMIT,
    SET_WIPE_POLICY,  // set the policy of wipe remote stale data
    RESULT_SET_CACHE_MODE, // Accept ResultSetCacheMode Type As PragmaData
    RESULT_SET_CACHE_MAX_SIZE, // Allowed Int Type Range [1,16], Unit MB
};

enum ResolutionPolicyType {
    AUTO_LAST_WIN = 0,      // resolve conflicts by timestamp(default value)
    CUSTOMER_RESOLUTION = 1 // resolve conflicts by user
};

using KvStoreCorruptionHandler = std::function<void (const std::string &appId, const std::string &userId,
                                                     const std::string &storeId)>;
} // namespace DistributedDB

#endif // KV_STORE_TYPE_H
