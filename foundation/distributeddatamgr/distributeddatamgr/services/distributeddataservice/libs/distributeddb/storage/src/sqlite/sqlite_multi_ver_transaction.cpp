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
#include "sqlite_multi_ver_transaction.h"

#include <climits>
#include <string>
#include <vector>
#include <algorithm>

#include "securec.h"

#include "db_common.h"
#include "db_constant.h"
#include "db_types.h"
#include "log_print.h"
#include "sqlite_utils.h"
#include "multi_ver_kv_entry.h"
#include "multi_ver_value_object.h"
#include "value_hash_calc.h"
#include "time_helper.h"

namespace DistributedDB {
const std::string SQLiteMultiVerTransaction::CREATE_TABLE_SQL =
    "CREATE TABLE IF NOT EXISTS version_data(key BLOB, value BLOB, oper_flag INTEGER, version INTEGER, " \
    "timestamp INTEGER, ori_timestamp INTEGER, hash_key BLOB, " \
    "PRIMARY key(hash_key, version));" \
    "CREATE INDEX IF NOT EXISTS version_index ON version_data (version);" \
    "CREATE INDEX IF NOT EXISTS flag_index ON version_data (oper_flag);";

const std::string SQLiteMultiVerTransaction::SELECT_ONE_SQL =
    "SELECT oper_flag, key, value FROM version_data WHERE hash_key=? AND (timestamp>? OR (timestamp=? AND rowid>=?)) " \
    "AND version<=? AND (oper_flag&0x08=0x08) ORDER BY version DESC LIMIT 1;";
const std::string SQLiteMultiVerTransaction::SELECT_BY_HASHKEY_VER_SQL =
    "SELECT oper_flag, value FROM version_data WHERE hash_key=? AND version=? ";
const std::string SQLiteMultiVerTransaction::SELECT_BATCH_SQL =
    "SELECT oper_flag, key, value, version FROM version_data WHERE key>=? AND key<=?" \
    "AND (timestamp>? OR (timestamp=? AND rowid>=?)) AND version<=? AND (oper_flag&0x08=0x08) " \
    "ORDER BY key ASC, version DESC;";
// select the data whose hash key is same to the current data.
const std::string SQLiteMultiVerTransaction::SELECT_HASH_ENTRY_SQL =
    "SELECT oper_flag FROM version_data WHERE hash_key=? AND version>? AND version<=? AND (oper_flag&0x08=0x08) " \
    "ORDER BY version DESC LIMIT 1;";
const std::string SQLiteMultiVerTransaction::SELECT_ONE_VER_RAW_SQL =
    "SELECT key, value, oper_flag, timestamp, ori_timestamp, hash_key FROM version_data " \
    "WHERE version=? ORDER BY rowid ASC;";
const std::string SQLiteMultiVerTransaction::INSERT_SQL =
    "INSERT OR REPLACE INTO version_data VALUES(?,?,?,?,?,?,?);";
const std::string SQLiteMultiVerTransaction::DELETE_VER_SQL =
    "DELETE FROM version_data WHERE version=?;";
const std::string SQLiteMultiVerTransaction::DELETE_BY_VER_HASHKEY_SQL =
    "DELETE FROM version_data WHERE version=? and hash_key=?;";
const std::string SQLiteMultiVerTransaction::SELECT_PRE_PUT_VER_DATA_SQL =
    "SELECT value FROM version_data WHERE version=? AND timestamp<=?;";
const std::string SQLiteMultiVerTransaction::DELETE_PRE_PUT_VER_DATA_SQL =
    "DELETE FROM version_data WHERE version=? AND timestamp<=?;";

const std::string SQLiteMultiVerTransaction::SELECT_ONE_BY_KEY_TIMESTAMP_SQL =
    "SELECT timestamp, ori_timestamp, version, value FROM version_data WHERE hash_key=? AND (oper_flag&0x08=0x08) " \
    "ORDER BY version DESC LIMIT 1;";

const std::string SQLiteMultiVerTransaction::SELECT_LATEST_CLEAR_ID =
    "SELECT rowid, timestamp FROM version_data WHERE (oper_flag&0x07=0x03) AND (oper_flag&0x08=0x08) " \
    "ORDER BY rowid DESC LIMIT 1;"; // clear flag and the local flag.

const std::string SQLiteMultiVerTransaction::SELECT_MAX_LOCAL_VERSION =
    "SELECT MAX(version) FROM version_data WHERE (oper_flag&0x08=0x08);";

const std::string SQLiteMultiVerTransaction::SELECT_MAX_VERSION =
    "SELECT MAX(version) FROM version_data;";

const std::string SQLiteMultiVerTransaction::SELECT_MAX_TIMESTAMP =
    "SELECT MAX(timestamp) FROM version_data;";

const std::string SQLiteMultiVerTransaction::UPDATE_VERSION_TIMESTAMP =
    "UPDATE version_data SET timestamp=? WHERE version=?;";

const std::string SQLiteMultiVerTransaction::SELECT_OVERWRITTEN_CLEAR_TYPE =
    "SELECT hash_key, oper_flag, version FROM version_data WHERE version <? AND (oper_flag&0x08=0x08) " \
    "AND timestamp < (SELECT timestamp FROM version_data WHERE version =? AND (oper_flag&0x07=0x03) );";

const std::string SQLiteMultiVerTransaction::SELECT_OVERWRITTEN_NO_CLEAR_TYPE =
    "SELECT hash_key, oper_flag, version FROM version_data WHERE version <? AND (oper_flag&0x08=0x08) " \
    "AND hash_key =?;";

namespace {
    // just for the insert sql argument index.
    const int BIND_INSERT_KEY_INDEX = 1;
    const int BIND_INSERT_VAL_INDEX = 2;
    const int BIND_INSERT_OPER_FLG_INDEX = 3;
    const int BIND_INSERT_VER_INDEX = 4;
    const int BIND_INSERT_TIME_INDEX = 5;
    const int BIND_INSERT_ORI_TIME_INDEX = 6;
    const int BIND_INSERT_HASH_KEY_INDEX = 7;

    // just for query entries.
    const int STEP_SUCCESS = 0; // result OK, and put the data into the result set.
    const int STEP_CONTINUE = 1; // the current key has been put into the result set, discard this one.
    const int STEP_NEXTKEY = 2; // the current key has been deleted or abandoned.
    const int STEP_ERROR = 3; // errors occur while executing the query.

    const uint64_t NO_TIMESTAMP = 0;
}

SQLiteMultiVerTransaction::SQLiteMultiVerTransaction()
    : clearId_(0),
      clearTime_(0),
      currentMaxTimestamp_(NO_TIMESTAMP),
      version_(0),
      db_(nullptr),
      isReadOnly_(false),
      isDataChanged_(false)
{}

SQLiteMultiVerTransaction::~SQLiteMultiVerTransaction()
{
    if (db_ != nullptr) {
        (void)sqlite3_close_v2(db_);
        db_ = nullptr;
    }
}

int SQLiteMultiVerTransaction::Initialize(const std::string &uri,
    bool isReadOnly, CipherType type, const CipherPassword &passwd)
{
    std::vector<std::string> tableVect;
    tableVect.push_back(CREATE_TABLE_SQL);
    OpenDbProperties option = {uri, true, false, tableVect, type, passwd};
    int errCode = SQLiteUtils::OpenDatabase(option, db_);
    if (errCode != E_OK) {
        LOGE("Init db error:%d", errCode);
        return errCode;
    }

    uri_ = uri;
    isReadOnly_ = isReadOnly;
    return E_OK;
}

void SQLiteMultiVerTransaction::SetVersion(const Version &versionInfo)
{
    version_ = versionInfo;
}

int SQLiteMultiVerTransaction::Put(const Key &key, const Value &value)
{
    // for only read transaction, not support for writing.
    if (isReadOnly_) {
        return -E_NOT_SUPPORT;
    }

    uint64_t flag = ADD_FLAG | LOCAL_FLAG;
    MultiVerEntryAuxData data = {flag, NO_TIMESTAMP, NO_TIMESTAMP};
    return AddRecord(key, value, data);
}

int SQLiteMultiVerTransaction::Delete(const Key &key)
{
    if (key.empty() || key.size() > DBConstant::MAX_VALUE_SIZE) {
        return -E_INVALID_ARGS;
    }
    Value valueRead;
    int errCode = Get(key, valueRead);
    if (errCode != E_OK) {
        return errCode;
    }

    valueRead.clear();
    MultiVerValueObject valueObject;
    errCode = valueObject.SetValue(valueRead);
    if (errCode != E_OK) {
        return errCode;
    }

    Value value;
    errCode = valueObject.GetSerialData(value);
    if (errCode != E_OK) {
        return errCode;
    }

    Key hashKey;
    errCode = DBCommon::CalcValueHash(key, hashKey);
    if (errCode != E_OK) {
        return errCode;
    }

    MultiVerEntryAuxData data = {DEL_FLAG | LOCAL_FLAG, NO_TIMESTAMP, NO_TIMESTAMP};
    return AddRecord(hashKey, value, data);
}

int SQLiteMultiVerTransaction::Clear()
{
    if (isReadOnly_) {
        return -E_NOT_SUPPORT;
    }

    Key key = {'c', 'l', 'e', 'a', 'r'};
    Value emptyValue;
    MultiVerValueObject valueObject;
    int errCode = valueObject.SetValue(emptyValue);
    if (errCode != E_OK) {
        return errCode;
    }

    Value value;
    errCode = valueObject.GetSerialData(value);
    if (errCode != E_OK) {
        return errCode;
    }

    MultiVerEntryAuxData data = {LOCAL_FLAG | CLEAR_FLAG, NO_TIMESTAMP, NO_TIMESTAMP};
    errCode = AddRecord(key, value, data);

    clearId_ = 0;
    GetClearId();
    return errCode;
}

int SQLiteMultiVerTransaction::Get(const Key &key, Value &value) const
{
    sqlite3_stmt *statement = nullptr;
    std::lock_guard<std::mutex> lock(readMutex_);
    int errCode = SQLiteUtils::GetStatement(db_, SELECT_ONE_SQL, statement);
    if (errCode != E_OK) {
        return errCode;
    }

    GetClearId(); // query the clear rowid, and only concern the later entry.
    Key readKey;
    Key hashKey;
    errCode = DBCommon::CalcValueHash(key, hashKey);
    if (errCode != E_OK) {
        goto END;
    }
    errCode = GetKeyAndValueByHashKey(statement, hashKey, readKey, value, false);
END:
    SQLiteUtils::ResetStatement(statement, true, errCode);
    return errCode;
}

int SQLiteMultiVerTransaction::GetValueForTrimSlice(const Key &hashKey, const Version version, Value &value) const
{
    sqlite3_stmt *statement = nullptr;
    std::lock_guard<std::mutex> lock(readMutex_);
    int errCode = SQLiteUtils::GetStatement(db_, SELECT_BY_HASHKEY_VER_SQL, statement);
    if (errCode != E_OK) {
        return errCode;
    }

    uint64_t operFlag;
    errCode = SQLiteUtils::BindBlobToStatement(statement, 1, hashKey, false); // bind the 1st para for hash key
    if (errCode != E_OK) {
        goto END;
    }

    errCode = sqlite3_bind_int64(statement, 2, version); // bind the 2nd para for version
    if (errCode != SQLITE_OK) {
        LOGE("Bind the clear id for query error:%d", errCode);
        goto END;
    }

    errCode = SQLiteUtils::StepWithRetry(statement);
    if (errCode == SQLiteUtils::MapSQLiteErrno(SQLITE_DONE)) {
        errCode = -E_NOT_FOUND;
        goto END;
    } else if (errCode != SQLiteUtils::MapSQLiteErrno(SQLITE_ROW)) {
        goto END;
    }

    errCode = E_OK;
    operFlag = static_cast<uint64_t>(sqlite3_column_int64(statement, 0));
    // only the added data should be operated.
    if ((OPERATE_MASK & operFlag) == ADD_FLAG) {
        errCode = SQLiteUtils::GetColumnBlobValue(statement, 1, value); // Get the value.
    }

END:
    SQLiteUtils::ResetStatement(statement, true, errCode);
    return errCode;
}

int SQLiteMultiVerTransaction::GetEntries(const Key &keyPrefix, std::vector<Entry> &entries) const
{
    GetEntriesStatements statements;
    std::lock_guard<std::mutex> lock(readMutex_);
    int errCode = PrepareForGetEntries(keyPrefix, statements);
    if (errCode != E_OK) {
        return errCode;
    }

    Entry entry;
    Key lastKey;
    int stepResult;
    int innerCode;

    entries.clear();
    entries.shrink_to_fit();
    do {
        errCode = SQLiteUtils::StepWithRetry(statements.getEntriesStatement);
        if (errCode == SQLiteUtils::MapSQLiteErrno(SQLITE_ROW)) {
            stepResult = GetOneEntry(statements, lastKey, entry, errCode);
            SQLiteUtils::ResetStatement(statements.hashFilterStatement, false, errCode);
            if (stepResult == STEP_SUCCESS) {
                lastKey = entry.key;
                entries.push_back(std::move(entry));
            } else if (stepResult == STEP_NEXTKEY) { // this key would be dispatched
                lastKey = entry.key;
            } else if (stepResult == STEP_CONTINUE) {
                continue;
            } else {
                goto END;
            }
        } else if (errCode == SQLiteUtils::MapSQLiteErrno(SQLITE_DONE)) {
            break;
        } else {
            LOGE("SQLite step failed:%d", errCode);
            goto END;
        }
    } while (true);

    // if select no result, return -E_NOT_FOUND.
    if (entries.empty()) {
        errCode = -E_NOT_FOUND;
    } else {
        errCode = E_OK;
    }
END:
    innerCode = ReleaseGetEntriesStatements(statements);
    if (innerCode != E_OK) {
        errCode = innerCode;
    }
    return errCode;
}

int SQLiteMultiVerTransaction::CheckToSaveRecord(const MultiVerKvEntry *entry, bool &isNeedSave,
    std::vector<Value> &values)
{
    Value disVal;
    int errCode = CheckIfNeedSaveRecord(entry, isNeedSave, disVal);
    if (errCode != E_OK) {
        return errCode;
    }

    if (!isNeedSave) {
        static_cast<const GenericMultiVerKvEntry *>(entry)->GetValue(disVal);
        return E_OK;
    }
    // Should erase the data inserted before the clear operation.
    uint64_t operFlag = 0;
    uint64_t timestamp = 0;
    (static_cast<const GenericMultiVerKvEntry *>(entry))->GetOperFlag(operFlag);
    entry->GetTimestamp(timestamp);
    if ((operFlag & OPERATE_MASK) == CLEAR_FLAG && version_ != 0) {
        LOGD("Erase one version:%llu ", version_);
        errCode = GetPrePutValues(version_, timestamp, values);
        if (errCode != E_OK) {
            return errCode;
        }
        errCode = RemovePrePutEntries(version_, timestamp);
        if (errCode != E_OK) {
            LOGE("Delete version data before clear oper failed:%d", errCode);
            return errCode;
        }
        clearId_ = 0; // Clear the clear id.
    }

    return E_OK;
}

int SQLiteMultiVerTransaction::PutBatch(const std::vector<Entry> &entries)
{
    for (auto iter = entries.begin(); iter != entries.end(); iter++) {
        int errCode = Put(iter->key, iter->value);
        if (errCode != E_OK) {
            LOGE("put failed:%d!", errCode);
            return errCode;
        }
    }
    return E_OK;
}

int SQLiteMultiVerTransaction::PutBatch(const std::vector<MultiVerKvEntry *> &entries, bool isLocal,
    std::vector<Value> &values)
{
    for (const auto &item : entries) {
        if (item == nullptr) {
            continue;
        }

        auto entry = static_cast<GenericMultiVerKvEntry *>(item);
        MultiVerEntryAuxData data;
        entry->GetOperFlag(data.operFlag);
        entry->GetTimestamp(data.timestamp);
        entry->GetOriTimestamp(data.oriTimestamp);
        data.operFlag &= OPERATE_MASK;

        // isLocal means that the entries need merge.
        if (isLocal) {
            data.operFlag |= LOCAL_FLAG; // set to local
        }

        bool isNeedSave = false;
        int errCode = CheckToSaveRecord(item, isNeedSave, values);
        if (errCode != E_OK) {
            return errCode;
        }
        // already add to the values.
        if (!isNeedSave) {
            continue;
        }

        Key key;
        Value value;
        (void)entry->GetKey(key);
        errCode = entry->GetValue(value);
        if (errCode != E_OK) {
            return errCode;
        }

        values.push_back(value);
        errCode = AddRecord(key, value, data);
        if (errCode != E_OK) {
            LOGE("Put batch data failed:%d", errCode);
            return errCode;
        }
    }
    return E_OK;
}

int SQLiteMultiVerTransaction::GetDiffEntries(const Version &begin, const Version &end, MultiVerDiffData &data) const
{
    sqlite3_stmt *statement = nullptr;
    int errCode = SQLiteUtils::GetStatement(db_, SELECT_ONE_VER_RAW_SQL, statement);
    if (errCode != E_OK) {
        LOGE("Fail to get the version raw data statement:%d", errCode);
        return errCode;
    }

    Value value;
    std::vector<MultiVerEntryData> savedEntries;
    errCode = GetRawDataByVersion(statement, end, savedEntries); // Get all the data of the end version.
    if (errCode != E_OK) {
        LOGE("Get raw data for diff version failed:%d", errCode);
        goto ERROR;
    }

    for (auto &item : savedEntries) {
        if ((item.auxData.operFlag & OPERATE_MASK) == CLEAR_FLAG) {
            data.Reset();
            data.isCleared = true;
            continue;
        }
        value.clear();
        if (begin == 0) { // no begin version, means no value
            errCode = -E_NOT_FOUND;
        } else {
            // Need get the origin key of the deleted data.
            if ((item.auxData.operFlag & OPERATE_MASK) == ADD_FLAG) {
                errCode = Get(item.key, value);
            } else {
                errCode = GetOriginKeyValueByHash(item, value);
            }
        }

        if (errCode == E_OK || errCode == -E_NOT_FOUND) {
            ClassifyDiffEntries(errCode, (item.auxData.operFlag & OPERATE_MASK), value, item, data);
            errCode = E_OK;
        } else {
            break;
        }
    }

ERROR:
    if (errCode != E_OK) {
        data.Reset();
    }

    SQLiteUtils::ResetStatement(statement, true, errCode);
    return errCode;
}

int SQLiteMultiVerTransaction::GetMaxVersion(MultiVerDataType type, Version &maxVersion) const
{
    std::string sql = SELECT_MAX_VERSION;
    if (type == MultiVerDataType::NATIVE_TYPE) {
        sql = SELECT_MAX_LOCAL_VERSION;
    }
    sqlite3_stmt *statement = nullptr;
    int errCode = SQLiteUtils::GetStatement(db_, sql, statement);
    if (errCode != E_OK) {
        return errCode;
    }

    // Step for getting the latest version
    errCode = SQLiteUtils::StepWithRetry(statement);
    if (errCode != SQLiteUtils::MapSQLiteErrno(SQLITE_ROW)) {
        if (errCode == SQLiteUtils::MapSQLiteErrno(SQLITE_DONE)) {
            LOGI("Initial the new max local version");
            maxVersion = 0;
            errCode = E_OK;
        } else {
            LOGE("Execute max version failed:%d", errCode);
        }
    } else {
        maxVersion = static_cast<Version>(sqlite3_column_int64(statement, 0)); // only select the first result.
        errCode = E_OK;
    }

    SQLiteUtils::ResetStatement(statement, true, errCode);
    return errCode;
}

int SQLiteMultiVerTransaction::ClearEntriesByVersion(const Version &versionInfo)
{
    // consider to get the statement.
    sqlite3_stmt *statement = nullptr;
    int errCode = SQLiteUtils::GetStatement(db_, DELETE_VER_SQL, statement);
    if (errCode != E_OK) {
        LOGE("Get delete version statement error:%d", errCode);
        return errCode;
    }

    // bind the version info.
    errCode = sqlite3_bind_int64(statement, 1, versionInfo); // bind the first argument;
    if (errCode != SQLITE_OK) {
        LOGE("bind the delete version statement error:%d", errCode);
        errCode = SQLiteUtils::MapSQLiteErrno(errCode);
        goto END;
    }

    // Step for getting the latest version
    errCode = SQLiteUtils::StepWithRetry(statement);
    if (errCode != SQLiteUtils::MapSQLiteErrno(SQLITE_DONE)) {
        LOGE("Delete records error:%d", errCode);
    } else {
        errCode = E_OK;
    }

END:
    SQLiteUtils::ResetStatement(statement, true, errCode);
    return errCode;
}

int SQLiteMultiVerTransaction::GetPrePutValues(const Version &versionInfo, TimeStamp timestamp,
    std::vector<Value> &values) const
{
    sqlite3_stmt *statement = nullptr;
    int errCode = SQLiteUtils::GetStatement(db_, SELECT_PRE_PUT_VER_DATA_SQL, statement);
    if (errCode != E_OK) {
        LOGE("get delete version statement for clear error:%d", errCode);
        return errCode;
    }

    // bind the versioninfo
    errCode = sqlite3_bind_int64(statement, 1, versionInfo); // bind the first argument;
    if (errCode != SQLITE_OK) {
        LOGE("bind the delete version statement for clear error:%d", errCode);
        errCode = SQLiteUtils::MapSQLiteErrno(errCode);
        goto ERROR;
    }

    // bind the clear timestamp
    errCode = sqlite3_bind_int64(statement, 2, timestamp); // bind the second argument for timestamp;
    if (errCode != SQLITE_OK) {
        LOGE("bind the clear timestamp for delete ver data error:%d", errCode);
        errCode = SQLiteUtils::MapSQLiteErrno(errCode);
        goto ERROR;
    }

    do {
        errCode = SQLiteUtils::StepWithRetry(statement);
        if (errCode == SQLiteUtils::MapSQLiteErrno(SQLITE_ROW)) {
            Value value;
            errCode = SQLiteUtils::GetColumnBlobValue(statement, 0, value); // get the 1st for value.
            if (errCode != E_OK) {
                goto ERROR;
            }
            values.push_back(std::move(value));
        } else if (errCode == SQLiteUtils::MapSQLiteErrno(SQLITE_DONE)) {
            errCode = E_OK;
            goto ERROR;
        } else {
            goto ERROR;
        }
    } while (true);

ERROR:
    SQLiteUtils::ResetStatement(statement, true, errCode);
    return errCode;
}

int SQLiteMultiVerTransaction::RemovePrePutEntries(const Version &versionInfo, TimeStamp timestamp)
{
    sqlite3_stmt *statement = nullptr;
    int errCode = SQLiteUtils::GetStatement(db_, DELETE_PRE_PUT_VER_DATA_SQL, statement);
    if (errCode != E_OK) {
        LOGE("get delete version statement for clear error:%d", errCode);
        return errCode;
    }

    // bind the versioninfo
    errCode = sqlite3_bind_int64(statement, 1, versionInfo); // bind the first argument;
    if (errCode != SQLITE_OK) {
        LOGE("bind the delete version statement for clear error:%d", errCode);
        errCode = SQLiteUtils::MapSQLiteErrno(errCode);
        goto ERROR;
    }

    // bind the clear timestamp
    errCode = sqlite3_bind_int64(statement, 2, timestamp); // bind the second argument for timestamp;
    if (errCode != SQLITE_OK) {
        LOGE("bind the clear timestamp for delete ver data error:%d", errCode);
        errCode = SQLiteUtils::MapSQLiteErrno(errCode);
        goto ERROR;
    }

    // Step for getting the latest version
    errCode = SQLiteUtils::StepWithRetry(statement);
    if (errCode != SQLiteUtils::MapSQLiteErrno(SQLITE_DONE)) {
        LOGE("Delete records for clear error:%d", errCode);
    } else {
        errCode = E_OK;
    }

ERROR:
    SQLiteUtils::ResetStatement(statement, true, errCode);
    return errCode;
}

int SQLiteMultiVerTransaction::StartTransaction()
{
    return SQLiteUtils::BeginTransaction(db_);
}

int SQLiteMultiVerTransaction::RollBackTransaction()
{
    return SQLiteUtils::RollbackTransaction(db_);
}

int SQLiteMultiVerTransaction::CommitTransaction()
{
    return SQLiteUtils::CommitTransaction(db_);
}

int SQLiteMultiVerTransaction::GetEntriesByVersion(Version version, std::list<MultiVerTrimedVersionData> &data) const
{
    std::lock_guard<std::mutex> lock(readMutex_);
    sqlite3_stmt *statement = nullptr;
    int errCode = SQLiteUtils::GetStatement(db_, SELECT_ONE_VER_RAW_SQL, statement);
    if (errCode != E_OK) {
        return errCode;
    }

    std::vector<MultiVerEntryData> savedEntries;
    errCode = GetRawDataByVersion(statement, version, savedEntries);
    if (errCode != E_OK) {
        LOGE("get raw data failed:%d", errCode);
        goto ERROR;
    }

    for (auto &item : savedEntries) {
        MultiVerTrimedVersionData versionData;
        versionData.operFlag = item.auxData.operFlag;
        if ((versionData.operFlag & OPERATE_MASK) == ADD_FLAG) {
            (void)DBCommon::CalcValueHash(item.key, versionData.key);
        } else {
            versionData.key = item.key;
        }
        versionData.version = version;
        data.push_front(versionData);
    }

ERROR:
    SQLiteUtils::ResetStatement(statement, true, errCode);
    return errCode;
}

int SQLiteMultiVerTransaction::GetEntriesByVersion(const Version &versionInfo,
    std::vector<MultiVerKvEntry *> &entries) const
{
    std::lock_guard<std::mutex> lock(readMutex_);
    sqlite3_stmt *statement = nullptr;
    int errCode = SQLiteUtils::GetStatement(db_, SELECT_ONE_VER_RAW_SQL, statement);
    if (errCode != E_OK) {
        return errCode;
    }

    std::vector<MultiVerEntryData> savedEntries;
    errCode = GetRawDataByVersion(statement, versionInfo, savedEntries);
    if (errCode != E_OK) {
        LOGE("get raw data failed:%d", errCode);
        goto ERROR;
    }

    for (auto &item : savedEntries) {
        GenericMultiVerKvEntry *entry = new (std::nothrow) GenericMultiVerKvEntry;
        if (entry == nullptr) {
            errCode = -E_OUT_OF_MEMORY;
            break;
        }
        entry->SetOperFlag(item.auxData.operFlag);
        entry->SetKey(item.key);
        entry->SetValue(item.value);
        entry->SetTimestamp(item.auxData.timestamp);
        entry->SetOriTimestamp(item.auxData.oriTimestamp);
        entries.push_back(entry);
    }

ERROR:
    if (errCode != E_OK) {
        for (auto &entry : entries) {
            delete entry;
            entry = nullptr;
        }

        entries.clear();
        entries.shrink_to_fit();
    }

    SQLiteUtils::ResetStatement(statement, true, errCode);
    return errCode;
}

TimeStamp SQLiteMultiVerTransaction::GetCurrentMaxTimestamp() const
{
    // consider to get the statement.
    sqlite3_stmt *statement = nullptr;
    int errCode = SQLiteUtils::GetStatement(db_, SELECT_MAX_TIMESTAMP, statement);
    if (errCode != E_OK) {
        LOGE("Get current max timestamp statement error:%d", errCode);
        return 0;
    }
    TimeStamp timestamp = 0;
    // Step for getting the latest version
    errCode = SQLiteUtils::StepWithRetry(statement);
    if (errCode != SQLiteUtils::MapSQLiteErrno(SQLITE_ROW)) {
        if (errCode == SQLiteUtils::MapSQLiteErrno(SQLITE_DONE)) {
            LOGI("Initial the current max timestamp");
        }
    } else {
        timestamp = static_cast<TimeStamp>(sqlite3_column_int64(statement, 0)); // the first result.
    }
    SQLiteUtils::ResetStatement(statement, true, errCode);
    return timestamp;
}

int SQLiteMultiVerTransaction::UpdateTimestampByVersion(const Version &version,
    TimeStamp stamp) const
{
    if (isReadOnly_) {
        return -E_NOT_SUPPORT;
    }

    sqlite3_stmt *statement = nullptr;
    int errCode = SQLiteUtils::GetStatement(db_, UPDATE_VERSION_TIMESTAMP, statement);
    if (errCode != E_OK) {
        LOGE("Get update timestamp statement error:%d", errCode);
        return errCode;
    }

    // bind the timestamp
    errCode = sqlite3_bind_int64(statement, 1, static_cast<int64_t>(stamp)); // bind the 1st for timestamp;
    if (errCode != SQLITE_OK) {
        LOGE("bind the updated timestamp error:%d", errCode);
        errCode = SQLiteUtils::MapSQLiteErrno(errCode);
        goto END;
    }

    // bind the versioninfo
    errCode = sqlite3_bind_int64(statement, 2, static_cast<int64_t>(version)); // bind the 2nd for version;
    if (errCode != SQLITE_OK) {
        LOGE("bind the updated version error:%d", errCode);
        errCode = SQLiteUtils::MapSQLiteErrno(errCode);
        goto END;
    }

    // Step for getting the latest version
    errCode = SQLiteUtils::StepWithRetry(statement);
    if (errCode == SQLiteUtils::MapSQLiteErrno(SQLITE_DONE)) {
        errCode = E_OK;
        currentMaxTimestamp_ = (stamp > currentMaxTimestamp_) ? stamp : currentMaxTimestamp_;
        LOGD("Update the timestamp of version:%llu - %llu", version, stamp);
    } else {
        LOGE("Failed to update the timestamp of the version:%d", errCode);
    }

END:
    SQLiteUtils::ResetStatement(statement, true, errCode);
    return errCode;
}

bool SQLiteMultiVerTransaction::IsDataChanged() const
{
    if (isReadOnly_) {
        return false;
    }

    return isDataChanged_;
}

void SQLiteMultiVerTransaction::ResetVersion()
{
    if (db_ != nullptr) {
        sqlite3_db_release_memory(db_);
    }

    version_ = 0;
    clearId_ = 0;
    isDataChanged_ = false;
}

int SQLiteMultiVerTransaction::Reset(CipherType type, const CipherPassword &passwd)
{
    std::lock_guard<std::mutex> lock(resetMutex_);
    std::vector<std::string> tableVect = {CREATE_TABLE_SQL};
    OpenDbProperties option = {uri_, true, false, tableVect, type, passwd};
    sqlite3 *newConnection = nullptr;
    int errCode = SQLiteUtils::OpenDatabase(option, newConnection);
    if (errCode != E_OK) {
        LOGE("Reset the transaction error:%d", errCode);
        return errCode;
    }
    if (db_ != nullptr) {
        (void)sqlite3_close_v2(db_);
    }
    db_ = newConnection;
    return E_OK;
}

Version SQLiteMultiVerTransaction::GetVersion() const
{
    return version_;
}

int SQLiteMultiVerTransaction::GetOverwrittenClearTypeEntries(Version clearVersion,
    std::list<MultiVerTrimedVersionData> &data) const
{
    sqlite3_stmt *statement = nullptr;
    std::lock_guard<std::mutex> lock(readMutex_);
    int errCode = SQLiteUtils::GetStatement(db_, SELECT_OVERWRITTEN_CLEAR_TYPE, statement);
    if (errCode != E_OK) {
        return errCode;
    }

    errCode = sqlite3_bind_int64(statement, 1, clearVersion); // bind the 1st for the clear version
    if (errCode != SQLITE_OK) {
        LOGE("Bind the clear id for query error:%d", errCode);
        goto END;
    }

    errCode = sqlite3_bind_int64(statement, 2, clearVersion); // bind the 2nd for the clear version to get timestamp
    if (errCode != SQLITE_OK) {
        LOGE("Bind the clear id for query error:%d", errCode);
        goto END;
    }

    do {
        errCode = SQLiteUtils::StepWithRetry(statement);
        if (errCode == SQLiteUtils::MapSQLiteErrno(SQLITE_ROW)) {
            uint64_t operFlag = static_cast<uint64_t>(sqlite3_column_int64(statement, 1)); // get the 2nd for opr

            MultiVerTrimedVersionData trimedVerData;
            errCode = SQLiteUtils::GetColumnBlobValue(statement, 0, trimedVerData.key); // get the 1st for key.
            if (errCode != E_OK) {
                goto END;
            }
            trimedVerData.operFlag = operFlag & OPERATE_MASK;
            trimedVerData.version = static_cast<uint64_t>(sqlite3_column_int64(statement, 2)); // get the 3rd for ver
            data.push_front(trimedVerData);
        } else if (errCode == SQLiteUtils::MapSQLiteErrno(SQLITE_DONE)) {
            errCode = E_OK;
            goto END;
        } else {
            goto END;
        }
    } while (true);
END:
    SQLiteUtils::ResetStatement(statement, true, errCode);
    return errCode;
}

int SQLiteMultiVerTransaction::GetOverwrittenNonClearTypeEntries(Version version, const Key &hashKey,
    std::list<MultiVerTrimedVersionData> &data) const
{
    sqlite3_stmt *statement = nullptr;
    std::lock_guard<std::mutex> lock(readMutex_);
    int errCode = SQLiteUtils::GetStatement(db_, SELECT_OVERWRITTEN_NO_CLEAR_TYPE, statement);
    if (errCode != E_OK) {
        return errCode;
    }

    errCode = sqlite3_bind_int64(statement, 1, version); // bind the 1st for the version
    if (errCode != SQLITE_OK) {
        LOGE("Bind the clear id for query error:%d", errCode);
        goto END;
    }

    errCode = SQLiteUtils::BindBlobToStatement(statement, 2, hashKey, false); // 2nd argument is hashKey
    if (errCode != E_OK) {
        goto END;
    }

    do {
        errCode = SQLiteUtils::StepWithRetry(statement);
        if (errCode == SQLiteUtils::MapSQLiteErrno(SQLITE_ROW)) {
            uint64_t operFlag = static_cast<uint64_t>(sqlite3_column_int64(statement, 1)); // 2nd for oper flag.
            MultiVerTrimedVersionData trimedVerData;
            errCode = SQLiteUtils::GetColumnBlobValue(statement, 0, trimedVerData.key); // column result is key.
            if (errCode != E_OK) {
                goto END;
            }

            trimedVerData.operFlag = operFlag & OPERATE_MASK;  // get the meta flag
            trimedVerData.version = static_cast<uint64_t>(sqlite3_column_int64(statement, 2));   // get the 3rd for ver
            data.push_front(trimedVerData);
        } else if (errCode == SQLiteUtils::MapSQLiteErrno(SQLITE_DONE)) {
            errCode = E_OK;
            goto END;
        } else {
            goto END;
        }
    } while (true);

END:
    SQLiteUtils::ResetStatement(statement, true, errCode);
    return errCode;
}

int SQLiteMultiVerTransaction::DeleteEntriesByHashKey(Version version, const Key &hashKey)
{
    // consider to get the statement.
    sqlite3_stmt *statement = nullptr;
    int errCode = SQLiteUtils::GetStatement(db_, DELETE_BY_VER_HASHKEY_SQL, statement);
    if (errCode != E_OK) {
        LOGE("Get delete version statement error:%d", errCode);
        return errCode;
    }

    // bind the version info.
    errCode = sqlite3_bind_int64(statement, 1, version); // bind the first argument;
    if (errCode != SQLITE_OK) {
        LOGE("bind the delete version statement error:%d", errCode);
        errCode = SQLiteUtils::MapSQLiteErrno(errCode);
        goto END;
    }

    errCode = SQLiteUtils::BindBlobToStatement(statement, 2, hashKey, false); // 2nd argument is hashKey
    if (errCode != E_OK) {
        goto END;
    }

    // Step for getting the latest version
    errCode = SQLiteUtils::StepWithRetry(statement);
    if (errCode != SQLiteUtils::MapSQLiteErrno(SQLITE_DONE)) {
        LOGE("Delete records error:%d", errCode);
    } else {
        errCode = E_OK;
    }

END:
    SQLiteUtils::ResetStatement(statement, true, errCode);
    return errCode;
}

int SQLiteMultiVerTransaction::GetRawMultiVerEntry(sqlite3_stmt *statement, MultiVerEntryData &keyEntry)
{
    int errCode = SQLiteUtils::GetColumnBlobValue(statement, 1, keyEntry.value);
    if (errCode != E_OK) {
        return errCode;
    }

    uint64_t flag = static_cast<uint64_t>(sqlite3_column_int64(statement, 2)); // oper flag index
    keyEntry.auxData.operFlag = flag & OPERATE_MASK; // remove the local flag.

    keyEntry.auxData.timestamp = static_cast<uint64_t>(sqlite3_column_int64(statement, 3)); // timestamp index
    keyEntry.auxData.oriTimestamp = static_cast<uint64_t>(sqlite3_column_int64(statement, 4)); // ori timestamp index

    // if the data is deleted data, just use the hash key.
    if ((flag & OPERATE_MASK) != ADD_FLAG) {
        errCode = SQLiteUtils::GetColumnBlobValue(statement, 5, keyEntry.key); // the hash key index.
        if (errCode != E_OK) {
            return errCode;
        }
    } else {
        errCode = SQLiteUtils::GetColumnBlobValue(statement, 0, keyEntry.key); // the key index.
        if (errCode != E_OK) {
            return errCode;
        }
    }
    if (keyEntry.key.empty()) {
        return -E_INVALID_DATA;
    }
    return E_OK;
}

int SQLiteMultiVerTransaction::GetRawDataByVersion(sqlite3_stmt *&statement,
    const Version &version, std::vector<MultiVerEntryData> &entries)
{
    // Bind the version
    int errCode = sqlite3_bind_int64(statement, 1, static_cast<int64_t>(version)); // only one parameter.
    if (errCode != SQLITE_OK) {
        LOGE("Bind the ver for getting raw ver data error:%d", errCode);
        return SQLiteUtils::MapSQLiteErrno(errCode);
    }

    do {
        errCode = SQLiteUtils::StepWithRetry(statement);
        if (errCode == SQLiteUtils::MapSQLiteErrno(SQLITE_ROW)) {
            MultiVerEntryData entry;
            errCode = GetRawMultiVerEntry(statement, entry);
            if (errCode == E_OK) {
                entries.push_back(std::move(entry));
            } else {
                break;
            }
        } else if (errCode == SQLiteUtils::MapSQLiteErrno(SQLITE_DONE)) {
            // if select no result, return the E_OK.
            errCode = E_OK;
            break;
        } else {
            LOGE("SQLite step failed:%d", errCode);
            break;
        }
    } while (true);

    SQLiteUtils::ResetStatement(statement, false, errCode);
    return errCode;
}

int SQLiteMultiVerTransaction::GetDiffOperator(int errCode, uint64_t flag)
{
    int oper = EntryOperator::FAIL;
    if (errCode == -E_NOT_FOUND) {
        if (flag == ADD_FLAG) {
            oper = EntryOperator::INSERT;
        }
    } else if (errCode == E_OK) {
        if (flag == DEL_FLAG) {
            oper = EntryOperator::DELETE;
        } else if (flag == ADD_FLAG) {
            oper = EntryOperator::UPDATE;
        }
    }

    return oper;
}

int SQLiteMultiVerTransaction::AddRecord(const Key &key, const Value &value,
    const MultiVerEntryAuxData &data)
{
    if (isReadOnly_) {
        return -E_NOT_SUPPORT;
    }
    sqlite3_stmt *statement = nullptr;
    int errCode = SQLiteUtils::GetStatement(db_, INSERT_SQL, statement);
    if (errCode != E_OK) {
        return errCode;
    }

    // If the record has timestamp, it means the record is foreign.
    MultiVerEntryAuxData dataCopy = data;
    if (data.timestamp == NO_TIMESTAMP) {
        if (currentMaxTimestamp_ == NO_TIMESTAMP) {
            currentMaxTimestamp_ = std::max(GetCurrentMaxTimestamp(), currentMaxTimestamp_);
        }
        dataCopy.timestamp = currentMaxTimestamp_++;
        if ((dataCopy.operFlag & LOCAL_FLAG) != 0) {
            dataCopy.oriTimestamp = currentMaxTimestamp_;
            LOGD("Origin timestamp:%llu", currentMaxTimestamp_);
        }
    }

    errCode = BindAddRecordArgs(statement, key, value, dataCopy);
    if (errCode != E_OK) {
        goto END;
    }

    // Step for put the result.
    errCode = SQLiteUtils::StepWithRetry(statement);
    if (errCode == SQLiteUtils::MapSQLiteErrno(SQLITE_DONE)) {
        currentMaxTimestamp_ = (dataCopy.timestamp > currentMaxTimestamp_) ? dataCopy.timestamp : currentMaxTimestamp_;
        errCode = E_OK;
        isDataChanged_ = true;
    } else {
        LOGE("SQLite step error: %d", errCode);
        errCode = SQLiteUtils::MapSQLiteErrno(errCode);
    }

END:
    SQLiteUtils::ResetStatement(statement, true, errCode);
    return errCode;
}

void SQLiteMultiVerTransaction::ClassifyDiffEntries(int errCode, uint64_t flag,
    const Value &value, MultiVerEntryData &item, MultiVerDiffData &data) const
{
    int oper = GetDiffOperator(errCode, flag);
    Entry entry;
    entry.key.swap(item.key);
    if (oper == EntryOperator::DELETE) {
        if (value.empty()) {
            MultiVerValueObject valueObject;
            valueObject.SetValue(value);
            Value newValue;
            int returnCode = valueObject.GetSerialData(newValue);
            if (returnCode != E_OK) {
                entry.value.clear();
            } else {
                entry.value.swap(newValue);
            }
        } else {
            entry.value = value;
        }
        data.deleted.push_back(std::move(entry));
    } else if (oper == EntryOperator::INSERT) {
        entry.value.swap(item.value);
        data.inserted.push_back(std::move(entry));
    } else if (oper == EntryOperator::UPDATE) {
        entry.value.swap(item.value);
        data.updated.push_back(std::move(entry));
    }
}

void SQLiteMultiVerTransaction::GetClearId() const
{
    if (clearId_ > 0) { // only changes at the begin or after clear operation.
        return;
    }

    // consider to get the statement.
    sqlite3_stmt *statement = nullptr;
    int errCode = SQLiteUtils::GetStatement(db_, SELECT_LATEST_CLEAR_ID, statement);
    if (errCode != E_OK) {
        LOGE("Get latest clear id error:%d", errCode);
        clearId_ = 1;
        clearTime_ = 0;
        return;
    }

    // Step for getting the latest version
    errCode = SQLiteUtils::StepWithRetry(statement);
    if (errCode != SQLiteUtils::MapSQLiteErrno(SQLITE_ROW)) {
        if (errCode == SQLiteUtils::MapSQLiteErrno(SQLITE_DONE)) {
            LOGI("Initial the new version for clear");
        }
        clearId_ = 1;
        clearTime_ = 0;
    } else {
        clearId_ = sqlite3_column_int64(statement, 0); // Get the max rowid from the 1st column.
        clearTime_ = sqlite3_column_int64(statement, 1); // Get the max timestamp from the 2nd column.
    }
    SQLiteUtils::ResetStatement(statement, true, errCode);
}

int SQLiteMultiVerTransaction::BindClearIdAndVersion(sqlite3_stmt *statement, int index) const
{
    int errCode = sqlite3_bind_int64(statement, index, clearTime_); // bind the 1st for the clear time
    if (errCode != SQLITE_OK) {
        LOGE("Bind the clear id for query error:%d", errCode);
        goto END;
    }

    // bind the next argument for the clear time in the same transact
    errCode = sqlite3_bind_int64(statement, index + 1, clearTime_);
    if (errCode != SQLITE_OK) {
        LOGE("Bind the clear id for query error:%d", errCode);
        goto END;
    }

    errCode = sqlite3_bind_int64(statement, index + 2, clearId_); // combination using with the clear time.
    if (errCode != SQLITE_OK) {
        LOGE("Bind the clear id for query error:%d", errCode);
        goto END;
    }

    errCode = sqlite3_bind_int64(statement, index + 3, version_); // version is after the clear rowid.
    if (errCode != SQLITE_OK) {
        LOGE("Bind the version for query error:%d", errCode);
        goto END;
    }
END:
    return SQLiteUtils::MapSQLiteErrno(errCode);
}

int SQLiteMultiVerTransaction::BindQueryEntryArgs(sqlite3_stmt *statement,
    const Key &key) const
{
    int errCode = SQLiteUtils::BindBlobToStatement(statement, 1, key, false); // first argument is key
    if (errCode != E_OK) {
        return errCode;
    }

    return BindClearIdAndVersion(statement, 2); // the third argument is clear id.
}

int SQLiteMultiVerTransaction::BindQueryEntriesArgs(sqlite3_stmt *statement,
    const Key &key) const
{
    // bind the prefix key for the first and second args.
    int errCode = SQLiteUtils::BindPrefixKey(statement, 1, key); // first argument is key
    if (errCode != E_OK) {
        return errCode;
    }

    return BindClearIdAndVersion(statement, 3); // the third argument is clear id.
}

int SQLiteMultiVerTransaction::BindAddRecordKeysToStatement(sqlite3_stmt *statement, const Key &key,
    const MultiVerEntryAuxData &data)
{
    if ((data.operFlag & OPERATE_MASK) != ADD_FLAG) {
        Key emptyKey;
        int errCode = SQLiteUtils::BindBlobToStatement(statement, BIND_INSERT_KEY_INDEX, emptyKey, true);
        if (errCode != E_OK) {
            return errCode;
        }

        errCode = SQLiteUtils::BindBlobToStatement(statement, BIND_INSERT_HASH_KEY_INDEX, key, false);
        if (errCode != E_OK) {
            return errCode;
        }
        return errCode;
    }
    int errCode = SQLiteUtils::BindBlobToStatement(statement, BIND_INSERT_KEY_INDEX, key, false);
    if (errCode != E_OK) {
        return errCode;
    }
    Key hashKey;
    errCode = DBCommon::CalcValueHash(key, hashKey);
    if (errCode != E_OK) {
        return errCode;
    }

    errCode = SQLiteUtils::BindBlobToStatement(statement, BIND_INSERT_HASH_KEY_INDEX, hashKey, false);
    if (errCode != E_OK) {
        return errCode;
    }
    return errCode;
}

int SQLiteMultiVerTransaction::BindAddRecordArgs(sqlite3_stmt *statement,
    const Key &key, const Value &value, const MultiVerEntryAuxData &data) const
{
    int errCode = BindAddRecordKeysToStatement(statement, key, data);
    if (errCode != E_OK) {
        LOGE("Failed to bind the keys:%d", errCode);
        return errCode;
    }

    errCode = SQLiteUtils::BindBlobToStatement(statement, BIND_INSERT_VAL_INDEX, value, true);
    if (errCode != E_OK) {
        return errCode;
    }

    errCode = sqlite3_bind_int64(statement, BIND_INSERT_OPER_FLG_INDEX, static_cast<int64_t>(data.operFlag));
    if (errCode != SQLITE_OK) {
        goto END;
    }

    errCode = sqlite3_bind_int64(statement, BIND_INSERT_VER_INDEX, static_cast<int64_t>(version_));
    if (errCode != SQLITE_OK) {
        goto END;
    }

    errCode = sqlite3_bind_int64(statement, BIND_INSERT_TIME_INDEX, static_cast<int64_t>(data.timestamp));
    if (errCode != SQLITE_OK) {
        goto END;
    }

    errCode = sqlite3_bind_int64(statement, BIND_INSERT_ORI_TIME_INDEX, static_cast<int64_t>(data.oriTimestamp));
    if (errCode != SQLITE_OK) {
        goto END;
    }

END:
    if (errCode != SQLITE_OK) {
        LOGE("Failed to bind the value:%d", errCode);
    }
    return SQLiteUtils::MapSQLiteErrno(errCode);
}

int SQLiteMultiVerTransaction::GetOneEntry(const GetEntriesStatements &statements,
    const Key &lastKey, Entry &entry, int &errCode) const
{
    // SQL: "select oper_flag, key, value, version from data;"
    errCode = SQLiteUtils::GetColumnBlobValue(statements.getEntriesStatement, 1, entry.key); // 2th is key
    if (errCode != E_OK) {
        return STEP_ERROR;
    }

    // if equal to the last key, just step to the next one.
    if (lastKey == entry.key) {
        entry.key.clear();
        return STEP_CONTINUE;
    }
    uint64_t flag = static_cast<uint64_t>(sqlite3_column_int64(statements.getEntriesStatement, 0)); // 1th is flag
    if ((flag & OPERATE_MASK) != ADD_FLAG) {
        return STEP_NEXTKEY;
    }

    errCode = SQLiteUtils::GetColumnBlobValue(statements.getEntriesStatement, 2, entry.value); // 3rd is value
    if (errCode != E_OK) {
        return STEP_ERROR;
    }

    Version curVer = static_cast<uint64_t>(sqlite3_column_int64(statements.getEntriesStatement, 3)); // 4th  is ver
    // select the version that is greater than the curEntryVer;
    Key hashKey;
    errCode = DBCommon::CalcValueHash(entry.key, hashKey);
    if (errCode != E_OK) {
        return STEP_ERROR;
    }
    errCode = SQLiteUtils::BindBlobToStatement(statements.hashFilterStatement, 1, hashKey, false);
    if (errCode != E_OK) {
        return STEP_ERROR;
    }

    errCode = sqlite3_bind_int64(statements.hashFilterStatement, 2, static_cast<int64_t>(curVer));
    if (errCode != E_OK) {
        return STEP_ERROR;
    }
    errCode = sqlite3_bind_int64(statements.hashFilterStatement, 3, static_cast<int64_t>(version_));
    if (errCode != E_OK) {
        return STEP_ERROR;
    }
    errCode = SQLiteUtils::StepWithRetry(statements.hashFilterStatement);
    if (errCode == SQLiteUtils::MapSQLiteErrno(SQLITE_ROW)) {
        return STEP_NEXTKEY;
    } else if (errCode == SQLiteUtils::MapSQLiteErrno(SQLITE_DONE)) {
        return STEP_SUCCESS;
    } else {
        LOGE("Filter the entries hash key error:%d", errCode);
        return STEP_ERROR;
    }
}

bool SQLiteMultiVerTransaction::IsRecordCleared(const TimeStamp timestamp) const
{
    GetClearId();
    if (clearTime_ < 0) {
        return true;
    }
    if (timestamp <= static_cast<uint64_t>(clearTime_)) {
        return true;
    }
    return false;
}

int SQLiteMultiVerTransaction::CheckIfNeedSaveRecord(sqlite3_stmt *statement, const MultiVerKvEntry *multiVerKvEntry,
    bool &isNeedSave, Value &origVal) const
{
    // Bind the input args for sql
    int errCode;
    Key key;
    Value value;
    uint64_t operFlag = 0;
    static_cast<const GenericMultiVerKvEntry *>(multiVerKvEntry)->GetKey(key);
    static_cast<const GenericMultiVerKvEntry *>(multiVerKvEntry)->GetOperFlag(operFlag);
    static_cast<const GenericMultiVerKvEntry *>(multiVerKvEntry)->GetValue(value);
    if ((operFlag & OPERATE_MASK) == ADD_FLAG) {
        Key hashKey;
        errCode = DBCommon::CalcValueHash(key, hashKey);
        if (errCode != E_OK) {
            return errCode;
        }
        errCode = SQLiteUtils::BindBlobToStatement(statement, 1, hashKey, false); // key is the first arg
    } else {
        errCode = SQLiteUtils::BindBlobToStatement(statement, 1, key, false); // key is the first arg
    }

    if (errCode != E_OK) {
        return errCode;
    }

    // ori_stamp should diff from timstamp
    errCode = SQLiteUtils::StepWithRetry(statement);
    if (errCode == SQLiteUtils::MapSQLiteErrno(SQLITE_DONE)) {
        errCode = E_OK;
        isNeedSave = true;
    } else if (errCode == SQLiteUtils::MapSQLiteErrno(SQLITE_ROW)) {
        auto readTime = static_cast<TimeStamp>(sqlite3_column_int64(statement, 0)); // the first for time
        auto readOriTime = static_cast<TimeStamp>(sqlite3_column_int64(statement, 1)); // the second for orig time.
        auto readVersion = static_cast<Version>(sqlite3_column_int64(statement, 2)); // the third for version.
        errCode = SQLiteUtils::GetColumnBlobValue(statement, 3, origVal); // the fourth for origin value.
        if (errCode != E_OK) {
            return errCode;
        }
        TimeStamp timestamp = NO_TIMESTAMP;
        static_cast<const GenericMultiVerKvEntry *>(multiVerKvEntry)->GetTimestamp(timestamp);
        TimeStamp oriTimestamp = NO_TIMESTAMP;
        static_cast<const GenericMultiVerKvEntry *>(multiVerKvEntry)->GetOriTimestamp(oriTimestamp);
        // Only the latest origin time  is same or the reading time is bigger than putting time.
        isNeedSave = ((readTime < timestamp) && (readOriTime != oriTimestamp || value != origVal));
        LOGD("Timestamp :%llu vs %llu, %llu vs %llu, readVersion:%llu, version:%llu, %d",
            readOriTime, oriTimestamp, readTime, timestamp, readVersion, version_, isNeedSave);
        // if the version of the data to be saved is same to the original, you should notify the caller.
        if (readVersion != version_) {
            origVal.resize(0);
        }
    } else {
        LOGE("Check if need store sync entry failed:%d", errCode);
    }

    return errCode;
}

int SQLiteMultiVerTransaction::CheckIfNeedSaveRecord(const MultiVerKvEntry *multiVerKvEntry, bool &isNeedSave,
    Value &value) const
{
    auto entry = static_cast<const GenericMultiVerKvEntry *>(multiVerKvEntry);
    TimeStamp timestamp = NO_TIMESTAMP;
    entry->GetTimestamp(timestamp);
    if (IsRecordCleared(timestamp)) {
        isNeedSave = false;
        entry->GetValue(value);
        return E_OK;
    }

    sqlite3_stmt *statement = nullptr;
    int errCode = SQLiteUtils::GetStatement(db_, SELECT_ONE_BY_KEY_TIMESTAMP_SQL, statement);
    if (errCode != E_OK) {
        return errCode;
    }

    errCode = CheckIfNeedSaveRecord(statement, entry, isNeedSave, value);
    SQLiteUtils::ResetStatement(statement, true, errCode);
    return errCode;
}

int SQLiteMultiVerTransaction::PrepareForGetEntries(const Key &keyPrefix, GetEntriesStatements &statements) const
{
    int innerCode;
    int errCode = SQLiteUtils::GetStatement(db_, SELECT_BATCH_SQL, statements.getEntriesStatement);
    if (errCode != E_OK) {
        goto END;
    }
    errCode = SQLiteUtils::GetStatement(db_, SELECT_HASH_ENTRY_SQL, statements.hashFilterStatement);
    if (errCode != E_OK) {
        goto END;
    }

    GetClearId(); // for read data.
    errCode = BindQueryEntriesArgs(statements.getEntriesStatement, keyPrefix);
    if (errCode != E_OK) {
        goto END;
    }
    return E_OK;
END:
    innerCode = ReleaseGetEntriesStatements(statements);
    if (errCode == E_OK) {
        errCode = innerCode;
    }
    return errCode;
}

int SQLiteMultiVerTransaction::ReleaseGetEntriesStatements(GetEntriesStatements &statements) const
{
    int errCode = E_OK;
    SQLiteUtils::ResetStatement(statements.getEntriesStatement, true, errCode);
    SQLiteUtils::ResetStatement(statements.hashFilterStatement, true, errCode);
    return errCode;
}

int SQLiteMultiVerTransaction::GetKeyAndValueByHashKey(sqlite3_stmt *statement, const Key &hashKey,
    Key &key, Value &value, bool isNeedReadKey) const
{
    int errCode = BindQueryEntryArgs(statement, hashKey);
    if (errCode != E_OK) {
        return errCode;
    }

    errCode = SQLiteUtils::StepWithRetry(statement);
    if (errCode == SQLiteUtils::MapSQLiteErrno(SQLITE_DONE)) {
        return -E_NOT_FOUND;
    } else if (errCode != SQLiteUtils::MapSQLiteErrno(SQLITE_ROW)) {
        return errCode;
    }

    uint64_t flag = static_cast<uint64_t>(sqlite3_column_int64(statement, 0)); // get the flag
    if ((flag & OPERATE_MASK) != ADD_FLAG) { // if not add or replace,
        return -E_NOT_FOUND;
    }
    if (isNeedReadKey) {
        errCode = SQLiteUtils::GetColumnBlobValue(statement, 1, key); // 2nd column result is key.
        if (errCode != E_OK) {
            return errCode;
        }
    }

    return SQLiteUtils::GetColumnBlobValue(statement, 2, value); // 3rd column result is value.
}

int SQLiteMultiVerTransaction::GetOriginKeyValueByHash(MultiVerEntryData &item, Value &value) const
{
    sqlite3_stmt *statement = nullptr;
    int errCode = SQLiteUtils::GetStatement(db_, SELECT_ONE_SQL, statement);
    if (errCode != E_OK) {
        return errCode;
    }
    Key origKey;
    errCode = GetKeyAndValueByHashKey(statement, item.key, origKey, value, true);
    if (errCode != E_OK) {
        goto END;
    }
    item.key = origKey;
END:
    SQLiteUtils::ResetStatement(statement, true, errCode);
    return errCode;
}
} // namespace DistributedDB
#endif