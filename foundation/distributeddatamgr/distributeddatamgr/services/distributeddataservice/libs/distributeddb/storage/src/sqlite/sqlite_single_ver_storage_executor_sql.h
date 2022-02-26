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

#ifndef SQLITE_SINGLE_VER_STORAGE_EXECUTOR_SQL_H
#define SQLITE_SINGLE_VER_STORAGE_EXECUTOR_SQL_H

#include <string>

#include "types_export.h"

namespace DistributedDB {
    //  cache.sync_data is design for migrating action after process restart
    const std::string INSERT_LOCAL_SQL =
        "INSERT OR REPLACE INTO local_data VALUES(?,?,?,?);";
    const std::string INSERT_LOCAL_SQL_FROM_CACHEHANDLE =
        "INSERT OR REPLACE INTO maindb.local_data VALUES(?,?,?,?);";

    const std::string INSERT_CACHE_LOCAL_SQL =
        "INSERT OR REPLACE INTO local_data VALUES(?,?,?,?,?);";

    const std::string INSERT_META_SQL =
        "INSERT OR REPLACE INTO meta_data VALUES(?,?);";

    const std::string INSERT_ATTACH_META_SQL =
        "INSERT OR REPLACE INTO meta.meta_data VALUES(?,?);";

    const std::string INSERT_SYNC_SQL =
        "INSERT OR REPLACE INTO sync_data VALUES(?,?,?,?,?,?,?,?);";

    const std::string INSERT_CACHE_SYNC_SQL =
        "INSERT OR REPLACE INTO sync_data VALUES(?,?,?,?,?,?,?,?,?);";
    const std::string INSERT_CACHE_SYNC_SQL_FROM_MAINHANDLE =
        "INSERT OR REPLACE INTO cache.sync_data VALUES(?,?,?,?,?,?,?,?,?);";

    const std::string DELETE_LOCAL_SQL =
        "DELETE FROM local_data WHERE key=?;";
    const std::string DELETE_LOCAL_SQL_FROM_CACHEHANDLE =
        "DELETE FROM maindb.local_data WHERE key=?;";

    const std::string SELECT_ALL_META_KEYS =
        "SELECT key FROM meta_data;";

    const std::string SELECT_ATTACH_ALL_META_KEYS =
        "SELECT key FROM meta.meta_data;";

    const std::string SELECT_ALL_SYNC_ENTRIES_BY_DEV =
        "SELECT key, value FROM sync_data WHERE device=? AND (flag&0x03=0);";

    const std::string SELECT_ALL_SYNC_ENTRIES_BY_DEV_FROM_CACHEHANDLE =
        "SELECT key, value FROM maindb.sync_data WHERE device=? AND (flag&0x03=0);";

    const std::string SELECT_LOCAL_VALUE_TIMESTAMP_SQL =
        "SELECT value, timestamp FROM local_data WHERE key=?;";

    const std::string SELECT_SYNC_SQL =
        "SELECT * FROM sync_data WHERE key=?;";

    const std::string SELECT_SYNC_VALUE_WTIMESTAMP_SQL =
        "SELECT value, w_timestamp FROM sync_data WHERE key=?;";

    const std::string SELECT_SYNC_HASH_SQL =
        "SELECT * FROM sync_data WHERE hash_key=?;";

    const std::string SELECT_CACHE_SYNC_HASH_SQL =
        "SELECT * FROM sync_data WHERE hash_key=? AND version=?;";
    const std::string SELECT_CACHE_SYNC_HASH_SQL_FROM_MAINHANDLE =
        "SELECT * FROM cache.sync_data WHERE hash_key=? AND version=?;";

    const std::string SELECT_LOCAL_HASH_SQL =
        "SELECT * FROM local_data WHERE hash_key=?;";

    const std::string SELECT_CACHE_LOCAL_HASH_SQL =
        "SELECT * FROM local_data WHERE hash_key=?;";

    const std::string SELECT_META_VALUE_SQL =
        "SELECT value FROM meta_data WHERE key=?;";

    const std::string SELECT_ATTACH_META_VALUE_SQL =
        "SELECT value FROM meta.meta_data WHERE key=?;";

    const std::string SELECT_MAX_TIMESTAMP_SQL =
        "SELECT MAX(timestamp) FROM sync_data;";
    const std::string SELECT_MAX_TIMESTAMP_SQL_FROM_CACHEHANDLE =
        "SELECT MAX(timestamp) FROM maindb.sync_data;";

    const std::string SELECT_NATIVE_MIN_TIMESTAMP_IN_CACHE_SYNC_DATA_SQL =
        "SELECT MIN(timestamp) FROM sync_data WHERE flag&0x02=0x02;";
    const std::string SELECT_NATIVE_MIN_TIMESTAMP_IN_CACHE_SYNC_DATA_SQL_FROM_MAINHANDLE =
        "SELECT MIN(timestamp) FROM cache.sync_data WHERE flag&0x02=0x02;";

    const std::string SELECT_SYNC_ENTRIES_SQL =
        "SELECT * FROM sync_data WHERE timestamp >= ? AND timestamp < ? AND (flag&0x02=0x02) ORDER BY timestamp ASC;";

    const std::string SELECT_SYNC_PREFIX_SQL =
        "SELECT key, value FROM sync_data WHERE key>=? AND key<=? AND (flag&0x01=0) ORDER BY key ASC;";

    const std::string SELECT_SYNC_ROWID_PREFIX_SQL =
        "SELECT rowid FROM sync_data WHERE key>=? AND key<=? AND (flag&0x01=0) ORDER BY key ASC;";

    const std::string SELECT_SYNC_DATA_BY_ROWID_SQL =
        "SELECT key, value FROM sync_data WHERE rowid=?;";

    const std::string SELECT_LOCAL_PREFIX_SQL =
        "SELECT key, value FROM local_data WHERE key>=? AND key<=? ORDER BY key ASC;";

    const std::string SELECT_COUNT_SYNC_PREFIX_SQL =
        "SELECT count(key) FROM sync_data WHERE key>=? AND key<=? AND (flag&0x01=0);";

    const std::string REMOVE_DEV_DATA_SQL =
        "DELETE FROM sync_data WHERE device=? AND (flag&0x02=0);";
    const std::string REMOVE_DEV_DATA_SQL_FROM_CACHEHANDLE =
        "DELETE FROM maindb.sync_data WHERE device=? AND (flag&0x02=0);";

    const std::string SELECT_ENTRY_DEVICE =
        "SELECT ori_device, device FROM sync_data WHERE key=?;";

    // sql for migrating data
    const std::string MIGRATE_LOCAL_SQL_FROM_CACHEHANDLE =
        "INSERT OR REPLACE INTO maindb.local_data select key, value, timestamp, hash_key from main.local_data;";
    const std::string MIGRATE_LOCAL_SQL_FROM_MAINHANDLE =
        "INSERT OR REPLACE INTO main.local_data select key, value, timestamp, hash_key from cache.local_data;";

    const std::string MIGRATE_VACUUM_LOCAL_SQL_FROM_CACHEHANDLE =
        "DELETE FROM maindb.local_data where hash_key in (select hash_key FROM maindb.local_data where key is null);";
    const std::string MIGRATE_VACUUM_LOCAL_SQL_FROM_MAINHANDLE =
        "DELETE FROM main.local_data where hash_key in (select hash_key FROM main.local_data where key is null);";

    // version is index, order by better than MIN()
    const std::string MIGRATE_SELECT_MIN_VER_CACHEDATA_FROM_CACHEHANDLE =
        "SELECT * FROM sync_data where version = (select version from sync_data order by version limit 1);";
    const std::string MIGRATE_SELECT_MIN_VER_CACHEDATA_FROM_MAINHANDLE =
        "SELECT * FROM cache.sync_data where version = (select version from cache.sync_data order by version limit 1);";

    const std::string GET_MAX_VER_CACHEDATA_FROM_CACHEHANDLE =
        "select version from sync_data order by version DESC limit 1;";
    const std::string GET_MAX_VER_CACHEDATA_FROM_MAINHANDLE =
        "select version from cache.sync_data order by version DESC limit 1;";

    const std::string MIGRATE_PUT_DATA_TO_MAINDB_FROM_CACHEHANDLE =
        "INSERT OR REPLACE INTO maindb.sync_data VALUES(?,?,?,?,?,?,?,?);";
    const std::string MIGRATE_PUT_DATA_TO_MAINDB_FROM_MAINHANDLE =
        "INSERT OR REPLACE INTO sync_data VALUES(?,?,?,?,?,?,?,?);";

    const std::string MIGRATE_DEL_DATA_BY_VERSION_FROM_CACHEHANDLE =
        "DELETE FROM sync_data WHERE version=?;";
    const std::string MIGRATE_DEL_DATA_BY_VERSION_FROM_MAINHANDLE =
        "DELETE FROM cache.sync_data WHERE version=?;";

    const std::string SELECT_MAIN_SYNC_HASH_SQL_FROM_CACHEHANDLE = "SELECT * FROM maindb.sync_data WHERE hash_key=?;";

    const int BIND_KV_KEY_INDEX = 1;
    const int BIND_KV_VAL_INDEX = 2;
    const int BIND_LOCAL_TIMESTAMP_INDEX = 3;
    const int BIND_LOCAL_HASH_KEY_INDEX = 4;

    // binding index just for the get sync data sql
    const int BIND_BEGIN_STAMP_INDEX = 1;
    const int BIND_END_STAMP_INDEX = 2;

    // mainDB
    const int BIND_SYNC_KEY_INDEX = 1;
    const int BIND_SYNC_VAL_INDEX = 2;
    const int BIND_SYNC_STAMP_INDEX = 3;
    const int BIND_SYNC_FLAG_INDEX = 4;
    const int BIND_SYNC_DEV_INDEX = 5;
    const int BIND_SYNC_ORI_DEV_INDEX = 6;
    const int BIND_SYNC_HASH_KEY_INDEX = 7;
    const int BIND_SYNC_W_TIME_INDEX = 8;

    // cacheDB
    const int BIND_CACHE_LOCAL_KEY_INDEX = 1;
    const int BIND_CACHE_LOCAL_VAL_INDEX = 2;
    const int BIND_CACHE_LOCAL_TIMESTAMP_INDEX = 3;
    const int BIND_CACHE_LOCAL_HASH_KEY_INDEX = 4;
    const int BIND_CACHE_LOCAL_FLAG_INDEX = 5;

    const int BIND_CACHE_SYNC_KEY_INDEX = 1;
    const int BIND_CACHE_SYNC_VAL_INDEX = 2;
    const int BIND_CACHE_SYNC_STAMP_INDEX = 3;
    const int BIND_CACHE_SYNC_FLAG_INDEX = 4;
    const int BIND_CACHE_SYNC_DEV_INDEX = 5;
    const int BIND_CACHE_SYNC_ORI_DEV_INDEX = 6;
    const int BIND_CACHE_SYNC_HASH_KEY_INDEX = 7;
    const int BIND_CACHE_SYNC_W_TIME_INDEX = 8;
    const int BIND_CACHE_SYNC_VERSION_INDEX = 9;

    // select result index for the item for sync database
    const int SYNC_RES_KEY_INDEX = 0;
    const int SYNC_RES_VAL_INDEX = 1;
    const int SYNC_RES_TIME_INDEX = 2;
    const int SYNC_RES_FLAG_INDEX = 3;
    const int SYNC_RES_DEVICE_INDEX = 4;
    const int SYNC_RES_ORI_DEV_INDEX = 5;
    const int SYNC_RES_HASH_KEY_INDEX = 6;
    const int SYNC_RES_W_TIME_INDEX = 7;
    const int SYNC_RES_VERSION_INDEX = 8; // Available in cacheDB.

    // get kv data Response index
    const int GET_KV_RES_LOCAL_TIME_INDEX = 1;
    const int GET_KV_RES_SYNC_TIME_INDEX = 1;

    const int BIND_ORI_DEVICE_ID = 0;
    const int BIND_PRE_DEVICE_ID = 1;

    const Key REMOVE_DEVICE_DATA_KEY = {'r', 'e', 'm', 'o', 'v', 'e'};
} // namespace DistributedDB

#endif // SQLITE_SINGLE_VER_STORAGE_EXECUTOR_SQL_H
