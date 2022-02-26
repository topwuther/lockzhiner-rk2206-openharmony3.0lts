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

#include "sqlite_single_ver_storage_executor.h"

#include <algorithm>

#include "log_print.h"
#include "db_constant.h"
#include "db_common.h"
#include "db_errno.h"
#include "parcel.h"
#include "runtime_context.h"
#include "sqlite_single_ver_storage_executor_sql.h"

namespace DistributedDB {
namespace {
    void InitCommitNotifyDataKeyStatus(SingleVerNaturalStoreCommitNotifyData *committedData, const Key &hashKey,
        const DataOperStatus &dataStatus)
    {
        if (committedData == nullptr) {
            return;
        }

        ExistStatus existedStatus = ExistStatus::NONE;
        if (dataStatus.preStatus == DataStatus::DELETED) {
            existedStatus = ExistStatus::DELETED;
        } else if (dataStatus.preStatus == DataStatus::EXISTED) {
            existedStatus = ExistStatus::EXIST;
        }

        committedData->InitKeyPropRecord(hashKey, existedStatus);
    }
}

SQLiteSingleVerStorageExecutor::SQLiteSingleVerStorageExecutor(sqlite3 *dbHandle, bool writable, bool isMemDb)
    : SQLiteStorageExecutor(dbHandle, writable, isMemDb),
      getSyncStatement_(nullptr),
      getResultRowIdStatement_(nullptr),
      getResultEntryStatement_(nullptr),
      isTransactionOpen_(false),
      attachMetaMode_(false),
      executorState_(ExecutorState::INVALID),
      maxTimeStampInMainDB_(0),
      migrateTimeOffset_(0),
      isSyncMigrating_(false),
      conflictResolvePolicy_(DEFAULT_LAST_WIN)
{}

SQLiteSingleVerStorageExecutor::SQLiteSingleVerStorageExecutor(sqlite3 *dbHandle, bool writable, bool isMemDb,
    ExecutorState executorState)
    : SQLiteStorageExecutor(dbHandle, writable, isMemDb),
      getSyncStatement_(nullptr),
      getResultRowIdStatement_(nullptr),
      getResultEntryStatement_(nullptr),
      isTransactionOpen_(false),
      attachMetaMode_(false),
      executorState_(executorState),
      maxTimeStampInMainDB_(0),
      migrateTimeOffset_(0),
      isSyncMigrating_(false),
      conflictResolvePolicy_(DEFAULT_LAST_WIN)
{}

SQLiteSingleVerStorageExecutor::~SQLiteSingleVerStorageExecutor()
{
    if (isTransactionOpen_) {
        Rollback();
    }
    FinalizeAllStatements();
}

int SQLiteSingleVerStorageExecutor::GetKvData(SingleVerDataType type, const Key &key, Value &value,
    TimeStamp &timeStamp) const
{
    std::string sql;
    if (type == SingleVerDataType::LOCAL_TYPE) {
        sql = SELECT_LOCAL_VALUE_TIMESTAMP_SQL;
    } else if (type == SingleVerDataType::SYNC_TYPE) {
        sql = SELECT_SYNC_VALUE_WTIMESTAMP_SQL;
    } else if (type == SingleVerDataType::META_TYPE) {
        if (attachMetaMode_ == true) {
            sql = SELECT_ATTACH_META_VALUE_SQL;
        } else {
            sql = SELECT_META_VALUE_SQL;
        }
    } else {
        return -E_INVALID_ARGS;
    }

    sqlite3_stmt *statement = nullptr;
    int errCode = SQLiteUtils::GetStatement(dbHandle_, sql, statement);
    if (errCode != E_OK) {
        goto END;
    }

    errCode = SQLiteUtils::BindBlobToStatement(statement, 1, key, false); // first arg.
    if (errCode != E_OK) {
        goto END;
    }

    errCode = SQLiteUtils::StepWithRetry(statement, isMemDb_);
    if (errCode == SQLiteUtils::MapSQLiteErrno(SQLITE_DONE)) {
        errCode = -E_NOT_FOUND;
        goto END;
    } else if (errCode != SQLiteUtils::MapSQLiteErrno(SQLITE_ROW)) {
        goto END;
    }

    errCode = SQLiteUtils::GetColumnBlobValue(statement, 0, value); // only one result.

    // get timestamp
    if (type == SingleVerDataType::LOCAL_TYPE) {
        timeStamp = static_cast<TimeStamp>(sqlite3_column_int64(statement, GET_KV_RES_LOCAL_TIME_INDEX));
        LOGD("[SingleVerExe][GetKv] Timestamp from local is %llu", timeStamp);
    } else if (type == SingleVerDataType::SYNC_TYPE) {
        timeStamp = static_cast<TimeStamp>(sqlite3_column_int64(statement, GET_KV_RES_SYNC_TIME_INDEX));
        LOGD("[SingleVerExe][GetKv] Timestamp from sync is %llu", timeStamp);
    }

END:
    SQLiteUtils::ResetStatement(statement, true, errCode);
    return CheckCorruptedStatus(errCode);
}

int SQLiteSingleVerStorageExecutor::BindPutKvData(sqlite3_stmt *statement, const Key &key, const Value &value,
    TimeStamp timestamp, SingleVerDataType type)
{
    int errCode = SQLiteUtils::BindBlobToStatement(statement, BIND_KV_KEY_INDEX, key, false);
    if (errCode != E_OK) {
        LOGE("[SingleVerExe][BindPutKv]Bind key error:%d", errCode);
        return errCode;
    }

    errCode = SQLiteUtils::BindBlobToStatement(statement, BIND_KV_VAL_INDEX, value, true);
    if (errCode != E_OK) {
        LOGE("[SingleVerExe][BindPutKv]Bind value error:%d", errCode);
        return errCode;
    }

    if (type == SingleVerDataType::LOCAL_TYPE) {
        Key hashKey;
        errCode = DBCommon::CalcValueHash(key, hashKey);
        if (errCode != E_OK) {
            return errCode;
        }

        errCode = SQLiteUtils::BindBlobToStatement(statement, BIND_LOCAL_HASH_KEY_INDEX, hashKey, false);
        if (errCode != E_OK) {
            LOGE("[SingleVerExe][BindPutKv]Bind hash key error:%d", errCode);
            return errCode;
        }

        errCode = SQLiteUtils::BindInt64ToStatement(statement, BIND_LOCAL_TIMESTAMP_INDEX, timestamp);
        if (errCode != E_OK) {
            LOGE("[SingleVerExe][BindPutKv]Bind timestamp error:%d", errCode);
            return errCode;
        }
    }
    return E_OK;
}

int SQLiteSingleVerStorageExecutor::GetKvDataByHashKey(const Key &hashKey, SingleVerRecord &result) const
{
    sqlite3_stmt *statement = nullptr;
    std::vector<uint8_t> devVect;
    std::vector<uint8_t> origDevVect;
    int errCode = SQLiteUtils::GetStatement(dbHandle_, SELECT_SYNC_HASH_SQL, statement);
    if (errCode != E_OK) {
        goto END;
    }

    errCode = SQLiteUtils::BindBlobToStatement(statement, 1, hashKey, false); // bind the first arg hashkey.
    if (errCode != E_OK) {
        goto END;
    }

    errCode = SQLiteUtils::StepWithRetry(statement, isMemDb_);
    if (errCode == SQLiteUtils::MapSQLiteErrno(SQLITE_ROW)) {
        result.hashKey = hashKey;
        result.timeStamp = static_cast<TimeStamp>(sqlite3_column_int64(statement, SYNC_RES_TIME_INDEX));
        result.writeTimeStamp = static_cast<TimeStamp>(sqlite3_column_int64(statement, SYNC_RES_W_TIME_INDEX));
        result.flag = static_cast<uint64_t>(sqlite3_column_int64(statement, SYNC_RES_FLAG_INDEX));
        // get key
        errCode = SQLiteUtils::GetColumnBlobValue(statement, SYNC_RES_KEY_INDEX, result.key);
        if (errCode != E_OK) {
            goto END;
        }
        // get value
        errCode = SQLiteUtils::GetColumnBlobValue(statement, SYNC_RES_VAL_INDEX, result.value);
        if (errCode != E_OK) {
            goto END;
        }
        // get device
        errCode = SQLiteUtils::GetColumnBlobValue(statement, SYNC_RES_DEVICE_INDEX, devVect);
        if (errCode != E_OK) {
            goto END;
        }
        result.device = std::string(devVect.begin(), devVect.end());
        // get original device
        errCode = SQLiteUtils::GetColumnBlobValue(statement, SYNC_RES_ORI_DEV_INDEX, origDevVect);
        if (errCode != E_OK) {
            goto END;
        }
        result.origDevice = std::string(origDevVect.begin(), origDevVect.end());
    } else if (errCode == SQLiteUtils::MapSQLiteErrno(SQLITE_DONE)) {
        errCode = -E_NOT_FOUND;
        goto END;
    }

END:
    SQLiteUtils::ResetStatement(statement, true, errCode);
    return CheckCorruptedStatus(errCode);
}

int SQLiteSingleVerStorageExecutor::SaveKvData(SingleVerDataType type, const Key &key, const Value &value,
    TimeStamp timestamp)
{
    sqlite3_stmt *statement = nullptr;
    std::string sql;
    if (type == SingleVerDataType::LOCAL_TYPE) {
        sql = (executorState_ == ExecutorState::CACHE_ATTACH_MAIN ? INSERT_LOCAL_SQL_FROM_CACHEHANDLE :
            INSERT_LOCAL_SQL);
    } else {
        sql = (attachMetaMode_ ? INSERT_ATTACH_META_SQL : INSERT_META_SQL);
    }
    int errCode = SQLiteUtils::GetStatement(dbHandle_, sql, statement);
    if (errCode != E_OK) {
        goto ERROR;
    }

    errCode = BindPutKvData(statement, key, value, timestamp, type);
    if (errCode != E_OK) {
        goto ERROR;
    }

    errCode = SQLiteUtils::StepWithRetry(statement, isMemDb_);
    if (errCode == SQLiteUtils::MapSQLiteErrno(SQLITE_DONE)) {
        errCode = E_OK;
    }

ERROR:
    SQLiteUtils::ResetStatement(statement, true, errCode);
    return CheckCorruptedStatus(errCode);
}

int SQLiteSingleVerStorageExecutor::PutKvData(SingleVerDataType type, const Key &key, const Value &value,
    TimeStamp timestamp, SingleVerNaturalStoreCommitNotifyData *committedData)
{
    if (type != SingleVerDataType::LOCAL_TYPE && type != SingleVerDataType::META_TYPE) {
        return -E_INVALID_ARGS;
    }
    // committedData is only for local data, not for meta data.
    bool isLocal = (SingleVerDataType::LOCAL_TYPE == type);
    TimeStamp localTimeStamp = 0;
    Value readValue;
    bool isExisted = CheckIfKeyExisted(key, isLocal, readValue, localTimeStamp);
    if (isLocal && committedData != nullptr) {
        ExistStatus existedStatus = isExisted ? ExistStatus::EXIST : ExistStatus::NONE;
        Key hashKey;
        int innerErrCode = DBCommon::CalcValueHash(key, hashKey);
        if (innerErrCode != E_OK) {
            return innerErrCode;
        }
        committedData->InitKeyPropRecord(hashKey, existedStatus);
    }
    int errCode = SaveKvData(type, key, value, timestamp);
    if (errCode != E_OK) {
        return errCode;
    }

    if (isLocal && committedData != nullptr) {
        Entry entry = {key, value};
        committedData->InsertCommittedData(std::move(entry), (isExisted ? DataType::UPDATE : DataType::INSERT), true);
    }
    return E_OK;
}

int SQLiteSingleVerStorageExecutor::GetEntries(SingleVerDataType type, const Key &keyPrefix,
    std::vector<Entry> &entries) const
{
    if ((type != SingleVerDataType::LOCAL_TYPE) && (type != SingleVerDataType::SYNC_TYPE)) {
        return -E_INVALID_ARGS;
    }

    std::string sql = (type == SingleVerDataType::SYNC_TYPE) ? SELECT_SYNC_PREFIX_SQL : SELECT_LOCAL_PREFIX_SQL;
    sqlite3_stmt *statement = nullptr;
    int errCode = SQLiteUtils::GetStatement(dbHandle_, sql, statement);
    if (errCode != E_OK) {
        goto END;
    }

    // bind the prefix key for the first and second args.
    errCode = SQLiteUtils::BindPrefixKey(statement, 1, keyPrefix); // first argument is key
    if (errCode != E_OK) {
        goto END;
    }

    errCode = StepForResultEntries(statement, entries);

END:
    SQLiteUtils::ResetStatement(statement, true, errCode);
    return CheckCorruptedStatus(errCode);
}

int SQLiteSingleVerStorageExecutor::GetEntries(QueryObject &queryObj, std::vector<Entry> &entries) const
{
    std::string sql;
    int errCode = queryObj.GetQuerySql(sql);
    if (errCode != E_OK) {
        return errCode;
    }
    sqlite3_stmt *statement = nullptr;
    errCode = queryObj.GetQuerySqlStatement(dbHandle_, sql, statement);
    if (errCode != E_OK) {
        goto END;
    }
    errCode = StepForResultEntries(statement, entries);
END:
    SQLiteUtils::ResetStatement(statement, true, errCode);
    return CheckCorruptedStatus(errCode);
}

int SQLiteSingleVerStorageExecutor::GetCount(QueryObject &queryObj, int &count) const
{
    if (dbHandle_ == nullptr) {
        return -E_INVALID_DB;
    }

    std::string countSql;
    int errCode = queryObj.GetCountQuerySql(countSql);
    if (errCode != E_OK) {
        return errCode;
    }

    if (!queryObj.IsCountValid()) {
        LOGE("GetCount no need limit or orderby");
        return -E_INVALID_QUERY_FORMAT;
    }

    sqlite3_stmt *countStatement = nullptr;
    // get statement for count
    errCode = queryObj.GetQuerySqlStatement(dbHandle_, countSql, countStatement);
    if (errCode != E_OK) {
        LOGE("Get count bind statement error:%d", errCode);
        goto END;
    }
    // get count value
    errCode = SQLiteUtils::StepWithRetry(countStatement, isMemDb_);
    if (errCode == SQLiteUtils::MapSQLiteErrno(SQLITE_ROW)) {
        uint64_t readCount = static_cast<uint64_t>(sqlite3_column_int64(countStatement, 0));
        if (readCount > INT32_MAX) {
            LOGW("total count is beyond the max count");
            count = 0;
            errCode = -E_UNEXPECTED_DATA;
        } else {
            count = static_cast<int>(readCount);
            errCode = E_OK;
        }
        LOGD("Entry count in this result set is %d", count);
    } else {
        errCode = -E_UNEXPECTED_DATA;
    }

END:
    SQLiteUtils::ResetStatement(countStatement, true, errCode);
    return CheckCorruptedStatus(errCode);
}

void SQLiteSingleVerStorageExecutor::InitCurrentMaxStamp(TimeStamp &maxStamp)
{
    if (dbHandle_ == nullptr) {
        return;
    }
    std::string sql = ((executorState_ == ExecutorState::CACHE_ATTACH_MAIN) ?
        SELECT_MAX_TIMESTAMP_SQL_FROM_CACHEHANDLE : SELECT_MAX_TIMESTAMP_SQL);
    sqlite3_stmt *statement = nullptr;
    int errCode = SQLiteUtils::GetStatement(dbHandle_, sql, statement);
    if (errCode != E_OK) {
        goto ERROR;
    }

    errCode = SQLiteUtils::StepWithRetry(statement, isMemDb_);
    if (errCode == SQLiteUtils::MapSQLiteErrno(SQLITE_ROW)) {
        maxStamp = static_cast<uint64_t>(sqlite3_column_int64(statement, 0)); // get the first column
        LOGD("Max time stamp is %llu", maxStamp);
    }

ERROR:
    SQLiteUtils::ResetStatement(statement, true, errCode);
}

int SQLiteSingleVerStorageExecutor::PrepareForSyncDataByTime(TimeStamp begin, TimeStamp end,
    sqlite3_stmt *&statement) const
{
    if (dbHandle_ == nullptr) {
        return -E_INVALID_DB;
    }
    int errCode = SQLiteUtils::GetStatement(dbHandle_, SELECT_SYNC_ENTRIES_SQL, statement);
    if (errCode != E_OK) {
        LOGE("Prepare the sync entries statement error:%d", errCode);
        return errCode;
    }

    errCode = SQLiteUtils::BindInt64ToStatement(statement, BIND_BEGIN_STAMP_INDEX, begin);
    if (errCode != E_OK) {
        goto ERROR;
    }

    errCode = SQLiteUtils::BindInt64ToStatement(statement, BIND_END_STAMP_INDEX, end);
    if (errCode != E_OK) {
        goto ERROR;
    }

ERROR:
    if (errCode != E_OK) {
        LOGE("Bind the timestamp for getting sync data error:%d", errCode);
        SQLiteUtils::ResetStatement(statement, true, errCode);
    }

    return CheckCorruptedStatus(errCode);
}

void SQLiteSingleVerStorageExecutor::ReleaseContinueStatement()
{
    if (getSyncStatement_ != nullptr) {
        int errCode = E_OK;
        SQLiteUtils::ResetStatement(getSyncStatement_, true, errCode);
        if (errCode == -E_INVALID_PASSWD_OR_CORRUPTED_DB) {
            SetCorruptedStatus();
        }
    }
}

int SQLiteSingleVerStorageExecutor::GetSyncDataByTimestamp(std::vector<DataItem> &dataItems, size_t appenedLength,
    TimeStamp begin, TimeStamp end, const DataSizeSpecInfo &dataSizeInfo) const
{
    size_t dataTotalSize = 0;
    sqlite3_stmt *statement = nullptr;
    int errCode = PrepareForSyncDataByTime(begin, end, statement);
    if (errCode != E_OK) {
        return errCode;
    }

    dataItems.clear();
    do {
        DataItem dataItem;
        errCode = SQLiteUtils::StepWithRetry(statement, isMemDb_);
        if (errCode == SQLiteUtils::MapSQLiteErrno(SQLITE_ROW)) {
            errCode = GetDataItemForSync(statement, dataItem);
        } else {
            if (errCode == SQLiteUtils::MapSQLiteErrno(SQLITE_DONE)) {
                LOGD("Get sync data finished, number: %zu, size: %zu", dataTotalSize, dataItems.size());
                errCode = -E_FINISHED;
            } else {
                LOGE("Get sync data error:%d", errCode);
            }
            break;
        }

        // If dataTotalSize value is bigger than blockSize value , reserve the surplus data item.
        dataTotalSize += GetDataItemSerialSize(dataItem, appenedLength);
        if ((dataTotalSize > dataSizeInfo.blockSize && !dataItems.empty()) ||
            dataItems.size() >= dataSizeInfo.packetSize) {
            errCode = -E_UNFINISHED;
            break;
        } else {
            dataItems.push_back(std::move(dataItem));
        }
    } while (true);

    SQLiteUtils::ResetStatement(statement, true, errCode);
    return CheckCorruptedStatus(errCode);
}

int SQLiteSingleVerStorageExecutor::OpenResultSet(const Key &keyPrefix, int &count)
{
    sqlite3_stmt *countStatement = nullptr;
    if (InitResultSet(keyPrefix, countStatement) != E_OK) {
        LOGE("Initialize result set stat failed.");
        return -E_INVALID_DB;
    }

    int errCode = StartTransaction(TransactType::DEFERRED);
    if (errCode != E_OK) {
        goto END;
    }

    // get count value
    errCode = SQLiteUtils::StepWithRetry(countStatement, isMemDb_);
    if (errCode == SQLiteUtils::MapSQLiteErrno(SQLITE_ROW)) {
        uint64_t readCount = static_cast<uint64_t>(sqlite3_column_int64(countStatement, 0));
        if (readCount > INT32_MAX) {
            LOGW("total count is beyond the max count");
            count = 0;
            errCode = -E_UNEXPECTED_DATA;
        } else {
            count = static_cast<int>(readCount);
            errCode = E_OK;
        }
        LOGD("Entry count in this result set is %d", count);
    } else {
        errCode = -E_UNEXPECTED_DATA;
    }

END:
    SQLiteUtils::ResetStatement(countStatement, true, errCode);
    if (errCode != E_OK) {
        CloseResultSet();
    }
    return CheckCorruptedStatus(errCode);
}

int SQLiteSingleVerStorageExecutor::OpenResultSet(QueryObject &queryObj, int &count)
{
    sqlite3_stmt *countStatement = nullptr;
    int errCode = InitResultSet(queryObj, countStatement);
    if (errCode != E_OK) {
        LOGE("Initialize result set stat failed.");
        return errCode;
    }

    errCode = StartTransaction(TransactType::DEFERRED);
    if (errCode != E_OK) {
        goto END;
    }

    // get count value
    errCode = SQLiteUtils::StepWithRetry(countStatement, isMemDb_);
    if (errCode == SQLiteUtils::MapSQLiteErrno(SQLITE_ROW)) {
        uint64_t readCount = static_cast<uint64_t>(sqlite3_column_int64(countStatement, 0));
        if (queryObj.HasLimit()) {
            int limit = 0;
            int offset = 0;
            queryObj.GetLimitVal(limit, offset);
            offset = (offset < 0) ? 0 : offset;
            limit = (limit < 0) ? 0 : limit;
            if (readCount <= static_cast<uint64_t>(offset)) {
                readCount = 0;
            } else {
                readCount = std::min(readCount - offset, static_cast<uint64_t>(limit));
            }
        }

        if (readCount > INT32_MAX) {
            LOGW("total count is beyond the max count");
            count = 0;
            errCode = -E_UNEXPECTED_DATA;
        } else {
            count = static_cast<int>(readCount);
            errCode = E_OK;
        }
        LOGD("Entry count in this result set is %d", count);
    } else {
        errCode = -E_UNEXPECTED_DATA;
    }

END:
    SQLiteUtils::ResetStatement(countStatement, true, errCode);
    if (errCode != E_OK) {
        CloseResultSet();
    }
    return CheckCorruptedStatus(errCode);
}

int SQLiteSingleVerStorageExecutor::OpenResultSetForCacheRowIdMode(const Key &keyPrefix,
    std::vector<int64_t> &rowIdCache, uint32_t cacheLimit, int &count)
{
    if (dbHandle_ == nullptr) {
        return -E_INVALID_DB;
    }
    int errCode = SQLiteUtils::GetStatement(dbHandle_, SELECT_SYNC_ROWID_PREFIX_SQL, getResultRowIdStatement_);
    if (errCode != E_OK) {
        LOGE("[SqlSinExe][OpenResSetRowId][PrefixKey] Get rowId stmt fail, errCode=%d", errCode);
        return CheckCorruptedStatus(errCode);
    }
    errCode = SQLiteUtils::BindPrefixKey(getResultRowIdStatement_, 1, keyPrefix); // first argument index is 1
    if (errCode != E_OK) {
        LOGE("[SqlSinExe][OpenResSetRowId][PrefixKey] Bind rowid stmt fail, errCode=%d", errCode);
        SQLiteUtils::ResetStatement(getResultRowIdStatement_, true, errCode);
        return CheckCorruptedStatus(errCode);
    }
    errCode = OpenResultSetForCacheRowIdModeCommon(rowIdCache, cacheLimit, count);
    if (errCode != E_OK) {
        SQLiteUtils::ResetStatement(getResultRowIdStatement_, true, errCode);
    }
    return errCode;
}

int SQLiteSingleVerStorageExecutor::OpenResultSetForCacheRowIdMode(QueryObject &queryObj,
    std::vector<int64_t> &rowIdCache, uint32_t cacheLimit, int &count)
{
    if (dbHandle_ == nullptr) {
        return -E_INVALID_DB;
    }
    int errCode = E_OK;
    if (!queryObj.IsValid(errCode)) {
        LOGE("[SqlSinExe][OpenResSetRowId][Query] Not Valid, errCode=%d", errCode);
        return errCode;
    }
    std::string selectSql;
    errCode = queryObj.GetQuerySql(selectSql, true); // only rowid sql
    if (errCode != E_OK) {
        LOGE("[SqlSinExe][OpenResSetRowId][Query] Get SQL fail, errCode=%d", errCode);
        return errCode;
    }
    errCode = queryObj.GetQuerySqlStatement(dbHandle_, selectSql, getResultRowIdStatement_);
    if (errCode != E_OK) {
        LOGE("[SqlSinExe][OpenResSetRowId][Query] Get Stmt fail, errCode=%d", errCode);
        // The GetQuerySqlStatement does not self rollback(BAD...), so we have to reset the stmt here.
        SQLiteUtils::ResetStatement(getResultRowIdStatement_, true, errCode);
        return errCode;
    }
    errCode = OpenResultSetForCacheRowIdModeCommon(rowIdCache, cacheLimit, count);
    if (errCode != E_OK) {
        SQLiteUtils::ResetStatement(getResultRowIdStatement_, true, errCode);
    }
    return errCode;
}

int SQLiteSingleVerStorageExecutor::ReloadResultSet(const Key &keyPrefix)
{
    int errCode = E_OK;
    SQLiteUtils::ResetStatement(getResultRowIdStatement_, false, errCode);
    if (errCode != E_OK) {
        LOGE("Reset result set rowid statement of keyPrefix error:%d", errCode);
        SQLiteUtils::ResetStatement(getResultRowIdStatement_, true, errCode);
        errCode = SQLiteUtils::GetStatement(dbHandle_, SELECT_SYNC_ROWID_PREFIX_SQL, getResultRowIdStatement_);
        if (errCode != E_OK) {
            LOGE("Reset result set rowid statement of keyPrefix error:%d", errCode);
            return CheckCorruptedStatus(errCode);
        }
    }

    // No need to reset getResultEntryStatement_. Because the binding of it will be cleared in each get operation
    errCode = SQLiteUtils::BindPrefixKey(getResultRowIdStatement_, 1, keyPrefix); // first argument is key
    if (errCode != E_OK) {
        LOGE("Rebind result set rowid statement of keyPrefix error:%d", errCode);
        return CheckCorruptedStatus(errCode);
    }
    return E_OK;
}

int SQLiteSingleVerStorageExecutor::ReloadResultSet(QueryObject &queryObj)
{
    int errCode = E_OK;
    bool isValid = queryObj.IsValid(errCode);
    if (!isValid) {
        return errCode;
    }
    std::string sql;
    errCode = queryObj.GetQuerySql(sql, true); // only rowid sql
    if (errCode != E_OK) {
        return errCode;
    }

    SQLiteUtils::ResetStatement(getResultRowIdStatement_, false, errCode);
    if (errCode != E_OK) {
        LOGE("Reset result set rowid statement of query error:%d", errCode);
        // Finish current statement and remade one
        SQLiteUtils::ResetStatement(getResultRowIdStatement_, true, errCode);
        errCode = SQLiteUtils::GetStatement(dbHandle_, sql, getResultRowIdStatement_);
        if (errCode != E_OK) {
            LOGE("Reget result set rowid statement of query error:%d", errCode);
            return CheckCorruptedStatus(errCode);
        }
    }

    // No need to reset getResultEntryStatement_. Because the binding of it will be cleared in each get operation
    // GetQuerySqlStatement will not alter getResultRowIdStatement_ if it is not null
    errCode = queryObj.GetQuerySqlStatement(dbHandle_, sql, getResultRowIdStatement_);
    if (errCode != E_OK) {
        LOGE("Rebind result set rowid statement of query error:%d", errCode);
        return CheckCorruptedStatus(errCode);
    }
    return E_OK;
}

int SQLiteSingleVerStorageExecutor::ReloadResultSetForCacheRowIdMode(const Key &keyPrefix,
    std::vector<int64_t> &rowIdCache, uint32_t cacheLimit, uint32_t cacheStartPos)
{
    int errCode = ReloadResultSet(keyPrefix); // Reuse this function(A convenience)
    if (errCode != E_OK) {
        return errCode;
    }
    int count = 0; // Ignored
    errCode = ResultSetLoadRowIdCache(rowIdCache, cacheLimit, cacheStartPos, count);
    if (errCode != E_OK) {
        LOGE("[SqlSinExe][ReloadResSet][KeyPrefix] Load fail, errCode=%d", errCode);
        // We can just return, no need to reset the statement
        return errCode;
    }
    return E_OK;
}

int SQLiteSingleVerStorageExecutor::ReloadResultSetForCacheRowIdMode(QueryObject &queryObj,
    std::vector<int64_t> &rowIdCache, uint32_t cacheLimit, uint32_t cacheStartPos)
{
    int errCode = ReloadResultSet(queryObj); // Reuse this function(A convenience)
    if (errCode != E_OK) {
        return errCode;
    }
    int count = 0; // Ignored
    errCode = ResultSetLoadRowIdCache(rowIdCache, cacheLimit, cacheStartPos, count);
    if (errCode != E_OK) {
        LOGE("[SqlSinExe][ReloadResSet][Query] Load fail, errCode=%d", errCode);
        // We can just return, no need to reset the statement
        return errCode;
    }
    return E_OK;
}

int SQLiteSingleVerStorageExecutor::GetNextEntryFromResultSet(Key &key, Value &value, bool isCopy)
{
    if (getResultRowIdStatement_ == nullptr || getResultEntryStatement_ == nullptr) {
        return -E_RESULT_SET_STATUS_INVALID;
    }

    int errCode = SQLiteUtils::StepWithRetry(getResultRowIdStatement_, isMemDb_);
    if (errCode == SQLiteUtils::MapSQLiteErrno(SQLITE_ROW)) {
        if (!isCopy) {
            return E_OK;
        }
        int64_t rowId = sqlite3_column_int64(getResultRowIdStatement_, 0);
        errCode = E_OK;
        SQLiteUtils::ResetStatement(getResultEntryStatement_, false, errCode);
        if (errCode != E_OK) {
            LOGE("[SqlSinExe][GetNext] Reset result set entry statement fail, errCode=%d.", errCode);
            return CheckCorruptedStatus(errCode);
        }

        SQLiteUtils::BindInt64ToStatement(getResultEntryStatement_, 1, rowId);
        errCode = SQLiteUtils::StepWithRetry(getResultEntryStatement_, isMemDb_);
        if (errCode == SQLiteUtils::MapSQLiteErrno(SQLITE_ROW)) {
            errCode = SQLiteUtils::GetColumnBlobValue(getResultEntryStatement_, 0, key);
            if (errCode != E_OK) {
                LOGE("[SqlSinExe][GetNext] Get key failed:%d", errCode);
                return CheckCorruptedStatus(errCode);
            }
            errCode = SQLiteUtils::GetColumnBlobValue(getResultEntryStatement_, 1, value);
            if (errCode != E_OK) {
                LOGE("[SqlSinExe][GetNext] Get value failed:%d", errCode);
                return CheckCorruptedStatus(errCode);
            }
            return E_OK;
        } else {
            return -E_UNEXPECTED_DATA;
        }
    }
    if (errCode == SQLiteUtils::MapSQLiteErrno(SQLITE_DONE)) {
        return -E_FINISHED;
    }

    LOGE("SQLite step failed:%d", errCode);
    return CheckCorruptedStatus(errCode);
}

int SQLiteSingleVerStorageExecutor::GetEntryByRowId(int64_t rowId, Entry &entry)
{
    if (getResultEntryStatement_ == nullptr) {
        return -E_RESULT_SET_STATUS_INVALID;
    }
    int errCode = E_OK;
    SQLiteUtils::ResetStatement(getResultEntryStatement_, false, errCode);
    if (errCode != E_OK) {
        LOGE("[SqlSinExe][GetEntryByRowid] Reset result set entry statement fail, errCode=%d.", errCode);
        return CheckCorruptedStatus(errCode);
    }
    SQLiteUtils::BindInt64ToStatement(getResultEntryStatement_, 1, rowId);
    errCode = SQLiteUtils::StepWithRetry(getResultEntryStatement_, isMemDb_);
    if (errCode == SQLiteUtils::MapSQLiteErrno(SQLITE_ROW)) {
        errCode = SQLiteUtils::GetColumnBlobValue(getResultEntryStatement_, 0, entry.key);
        if (errCode != E_OK) {
            LOGE("[SqlSinExe][GetEntryByRowid] Get key failed, errCode=%d.", errCode);
            return CheckCorruptedStatus(errCode);
        }
        errCode = SQLiteUtils::GetColumnBlobValue(getResultEntryStatement_, 1, entry.value);
        if (errCode != E_OK) {
            LOGE("[SqlSinExe][GetEntryByRowid] Get value failed, errCode=%d.", errCode);
            return CheckCorruptedStatus(errCode);
        }
        return E_OK;
    } else {
        LOGE("[SqlSinExe][GetEntryByRowid] Step failed, errCode=%d.", errCode);
        return -E_UNEXPECTED_DATA;
    }
}

void SQLiteSingleVerStorageExecutor::CloseResultSet()
{
    int errCode = E_OK;
    SQLiteUtils::ResetStatement(getResultRowIdStatement_, true, errCode);
    if (errCode == -E_INVALID_PASSWD_OR_CORRUPTED_DB) {
        SetCorruptedStatus();
    }
    SQLiteUtils::ResetStatement(getResultEntryStatement_, true, errCode);
    if (errCode == -E_INVALID_PASSWD_OR_CORRUPTED_DB) {
        SetCorruptedStatus();
    }
    if (isTransactionOpen_) {
        SQLiteUtils::RollbackTransaction(dbHandle_);
        isTransactionOpen_ = false;
    }
}

int SQLiteSingleVerStorageExecutor::StartTransaction(TransactType type)
{
    if (dbHandle_ == nullptr) {
        LOGE("Begin transaction failed, dbHandle is null.");
        return -E_INVALID_DB;
    }
    int errCode = SQLiteUtils::BeginTransaction(dbHandle_, type);
    if (errCode == E_OK) {
        isTransactionOpen_ = true;
    } else {
        LOGE("Begin transaction failed, errCode = %d", errCode);
    }
    return CheckCorruptedStatus(errCode);
}

int SQLiteSingleVerStorageExecutor::Commit()
{
    if (dbHandle_ == nullptr) {
        return -E_INVALID_DB;
    }
    int errCode = SQLiteUtils::CommitTransaction(dbHandle_);
    if (errCode == E_OK) {
        isTransactionOpen_ = false;
    }
    return CheckCorruptedStatus(errCode);
}

int SQLiteSingleVerStorageExecutor::Rollback()
{
    if (dbHandle_ == nullptr) {
        return -E_INVALID_DB;
    }
    int errCode = SQLiteUtils::RollbackTransaction(dbHandle_);
    if (errCode != E_OK) {
        LOGE("sqlite single ver storage executor rollback fail! errCode = [%d]", errCode);
        return CheckCorruptedStatus(errCode);
    }
    isTransactionOpen_ = false;
    return E_OK;
}

bool SQLiteSingleVerStorageExecutor::CheckIfKeyExisted(const Key &key, bool isLocal,
    Value &value, TimeStamp &timeStamp) const
{
    // not local value, no need to get the value.
    if (!isLocal) {
        return false;
    }

    int errCode = GetKvData(SingleVerDataType::LOCAL_TYPE, key, value, timeStamp);
    if (errCode != E_OK) {
        return false;
    }
    return true;
}

int SQLiteSingleVerStorageExecutor::GetDeviceIdentifier(PragmaEntryDeviceIdentifier *identifier)
{
    if (identifier == nullptr) {
        return -E_INVALID_ARGS;
    }

    if (dbHandle_ == nullptr) {
        return -E_INVALID_DB;
    }

    sqlite3_stmt *statement = nullptr;
    int errCode = SQLiteUtils::GetStatement(dbHandle_, SELECT_ENTRY_DEVICE, statement);
    if (errCode != E_OK) {
        return errCode;
    }

    int keyIndex = identifier->origDevice ? BIND_ORI_DEVICE_ID : BIND_PRE_DEVICE_ID;
    errCode = SQLiteUtils::BindBlobToStatement(statement, BIND_KV_KEY_INDEX, identifier->key, false);
    if (errCode != E_OK) {
        goto END;
    }

    errCode = SQLiteUtils::StepWithRetry(statement, isMemDb_);
    if (errCode == SQLiteUtils::MapSQLiteErrno(SQLITE_ROW)) {
        std::vector<uint8_t> deviceId;
        errCode = SQLiteUtils::GetColumnBlobValue(statement, keyIndex, deviceId);
        identifier->deviceIdentifier.assign(deviceId.begin(), deviceId.end());
    } else if (errCode == SQLiteUtils::MapSQLiteErrno(SQLITE_DONE)) {
        errCode = -E_NOT_FOUND;
    }

END:
    SQLiteUtils::ResetStatement(statement, true, errCode);
    return CheckCorruptedStatus(errCode);
}

void SQLiteSingleVerStorageExecutor::PutIntoCommittedData(const DataItem &itemPut, const DataItem &itemGet,
    const DataOperStatus &status, const Key &hashKey,
    SingleVerNaturalStoreCommitNotifyData *committedData)
{
    if (committedData == nullptr) {
        return;
    }

    Entry entry;
    int errCode;
    if (!status.isDeleted) {
        entry.key = itemPut.key;
        entry.value = itemPut.value;
        DataType dataType = (status.preStatus == DataStatus::EXISTED) ? DataType::UPDATE : DataType::INSERT;
        errCode = committedData->InsertCommittedData(std::move(entry), dataType, true);
    } else {
        entry.key = itemGet.key;
        entry.value = itemGet.value;
        errCode = committedData->InsertCommittedData(std::move(entry), DataType::DELETE, true);
    }

    if (errCode != E_OK) {
        LOGE("[SingleVerExe][PutCommitData]Insert failed:%d", errCode);
    }
}

int SQLiteSingleVerStorageExecutor::PrepareForSavingData(const std::string &readSql, const std::string &writeSql,
    SaveRecordStatements &statements) const
{
    int errCode = SQLiteUtils::GetStatement(dbHandle_, readSql, statements.queryStatement);
    if (errCode != E_OK) {
        return errCode;
    }

    errCode = SQLiteUtils::GetStatement(dbHandle_, writeSql, statements.putStatement);
    if (errCode != E_OK) {
        SQLiteUtils::ResetStatement(statements.queryStatement, true, errCode);
    }
    return errCode;
}

int SQLiteSingleVerStorageExecutor::PrepareForSavingData(SingleVerDataType type)
{
    int errCode = -E_NOT_SUPPORT;
    if (type == SingleVerDataType::LOCAL_TYPE) {
        errCode = PrepareForSavingData(SELECT_LOCAL_HASH_SQL, INSERT_LOCAL_SQL, saveLocalStatements_);
    } else if (type == SingleVerDataType::SYNC_TYPE) {
        errCode = PrepareForSavingData(SELECT_SYNC_HASH_SQL, INSERT_SYNC_SQL, saveSyncStatements_);
    }
    return CheckCorruptedStatus(errCode);
}

int SQLiteSingleVerStorageExecutor::ResetForSavingData(SingleVerDataType type)
{
    int errCode = E_OK;
    if (type == SingleVerDataType::LOCAL_TYPE) {
        SQLiteUtils::ResetStatement(saveLocalStatements_.putStatement, false, errCode);
        SQLiteUtils::ResetStatement(saveLocalStatements_.queryStatement, false, errCode);
    } else if (type == SingleVerDataType::SYNC_TYPE) {
        SQLiteUtils::ResetStatement(saveSyncStatements_.putStatement, false, errCode);
        SQLiteUtils::ResetStatement(saveSyncStatements_.queryStatement, false, errCode);
    }
    return CheckCorruptedStatus(errCode);
}

std::string SQLiteSingleVerStorageExecutor::GetOriginDevName(const DataItem &dataItem,
    const std::string &origDevGet)
{
    if (((dataItem.flag & DataItem::LOCAL_FLAG) != 0) && dataItem.origDev.empty()) {
        return origDevGet;
    }
    return dataItem.origDev;
}

int SQLiteSingleVerStorageExecutor::SaveSyncDataToDatabase(const DataItem &dataItem, const Key &hashKey,
    const std::string &origDev, const std::string &deviceName)
{
    auto statement = saveSyncStatements_.putStatement;
    if (statement == nullptr) {
        return -E_INVALID_ARGS;
    }

    std::string devName = DBCommon::TransferHashString(deviceName);
    int errCode = BindSavedSyncData(statement, dataItem, hashKey, {origDev, devName});
    if (errCode != E_OK) {
        return errCode;
    }

    errCode = SQLiteUtils::StepWithRetry(statement, isMemDb_);
    if (errCode == SQLiteUtils::MapSQLiteErrno(SQLITE_DONE)) {
        errCode = E_OK;
    }
    return errCode;
}

DataOperStatus SQLiteSingleVerStorageExecutor::JudgeSyncSaveType(DataItem &dataItem,
    const DataItem &itemGet, const std::string &devName, bool isHashKeyExisted)
{
    DataOperStatus status;
    status.isDeleted = ((dataItem.flag & DataItem::DELETE_FLAG) != 0);
    if (isHashKeyExisted) {
        if ((itemGet.flag & DataItem::DELETE_FLAG) != 0) {
            status.preStatus = DataStatus::DELETED;
        } else {
            status.preStatus = DataStatus::EXISTED;
        }
        std::string deviceName = DBCommon::TransferHashString(devName);
        if (itemGet.writeTimeStamp >= dataItem.writeTimeStamp) {
            if ((!deviceName.empty()) && (itemGet.dev == deviceName)) {
                LOGI("Force overwrite the data:%llu vs %llu", itemGet.writeTimeStamp, dataItem.writeTimeStamp);
                status.isDefeated = false;
                dataItem.writeTimeStamp = itemGet.writeTimeStamp + 1;
                dataItem.timeStamp = itemGet.timeStamp;
            } else {
                status.isDefeated = true;
            }
        }
    }
    return status;
}

int SQLiteSingleVerStorageExecutor::GetSyncDataItemExt(const DataItem &dataItem, DataItem &itemGet,
    const DataOperStatus &dataStatus) const
{
    if (dataStatus.preStatus != DataStatus::EXISTED) {
        return E_OK;
    }
    auto statement = isSyncMigrating_ ? migrateSyncStatements_.queryStatement : saveSyncStatements_.queryStatement;
    // only deleted item need origin value.
    int errCode = SQLiteUtils::GetColumnBlobValue(statement, SYNC_RES_KEY_INDEX, itemGet.key);
    if (errCode != E_OK) {
        return errCode;
    }

    errCode = SQLiteUtils::GetColumnBlobValue(statement, SYNC_RES_VAL_INDEX, itemGet.value);
    if (errCode != E_OK) {
        LOGE("Get column value data failed:%d", errCode);
    }

    return errCode;
}

int SQLiteSingleVerStorageExecutor::ResetSaveSyncStatements(int errCode)
{
    SQLiteUtils::ResetStatement(saveSyncStatements_.putStatement, false, errCode);
    SQLiteUtils::ResetStatement(saveSyncStatements_.queryStatement, false, errCode);
    return CheckCorruptedStatus(errCode);
}

namespace {
    inline bool IsNeedIgnoredData(const DataItem &itemPut, const DataItem &itemGet,
        const DeviceInfo &devInfo, bool isHashKeyExisted, int policy)
    {
        // deny the data synced from other dev which the origin dev is current or the existed value is current dev data.
        return (((itemGet.origDev.empty() && isHashKeyExisted) || itemPut.origDev.empty()) &&
            (!devInfo.isLocal && policy == DENY_OTHER_DEV_AMEND_CUR_DEV_DATA));
    }
}

int SQLiteSingleVerStorageExecutor::PrepareForNotifyConflictAndObserver(DataItem &dataItem,
    const DeviceInfo &deviceInfo, NotifyConflictAndObserverData &notify)
{
    // Check sava data existed info
    int errCode = GetSyncDataItemPre(dataItem, notify.getData, notify.hashKey);
    if (errCode != E_OK && errCode != -E_NOT_FOUND) {
        LOGD("[SingleVerExe][PrepareForNotifyConflictAndObserver] failed:%d", errCode);
        if (isSyncMigrating_) {
            ResetForMigrateCacheData();
            return errCode;
        }
        return ResetSaveSyncStatements(errCode);
    }

    bool isHashKeyExisted = (errCode != -E_NOT_FOUND);
    if (IsNeedIgnoredData(dataItem, notify.getData, deviceInfo, isHashKeyExisted, conflictResolvePolicy_)) {
        LOGD("[SingleVerExe] Ignore the sync data.");
        if (isSyncMigrating_) {
            ResetForMigrateCacheData();
            return -E_IGNOR_DATA;
        }
        return ResetSaveSyncStatements(-E_IGNOR_DATA);
    }

    notify.dataStatus = JudgeSyncSaveType(dataItem, notify.getData, deviceInfo.deviceName, isHashKeyExisted);
    InitCommitNotifyDataKeyStatus(notify.committedData, notify.hashKey, notify.dataStatus);

    // Nonexistent data, but deleted by local.
    if ((notify.dataStatus.preStatus == DataStatus::DELETED || notify.dataStatus.preStatus == DataStatus::NOEXISTED) &&
        (dataItem.flag & DataItem::DELETE_FLAG) != 0 &&
        (dataItem.flag & DataItem::LOCAL_FLAG) != 0) {
        // For delete item in cacheDB, which not in mainDB. Cannot notify, but this is not error.
        errCode = -E_NOT_FOUND;
        LOGD("Nonexistent data, but deleted by local");
        if (isSyncMigrating_) {
            ResetForMigrateCacheData();
            return errCode;
        }
        return ResetSaveSyncStatements(errCode);
    }

    // get key and value from ori database
    errCode = GetSyncDataItemExt(dataItem, notify.getData, notify.dataStatus);
    if (errCode != E_OK) {
        LOGD("GetSyncDataItemExt failed:%d", errCode);
        if (isSyncMigrating_) {
            ResetForMigrateCacheData();
            return errCode;
        }
        return ResetSaveSyncStatements(errCode);
    }

    return E_OK;
}

int SQLiteSingleVerStorageExecutor::SaveSyncDataItem(DataItem &dataItem, const DeviceInfo &deviceInfo,
    TimeStamp &maxStamp, SingleVerNaturalStoreCommitNotifyData *committedData)
{
    NotifyConflictAndObserverData notify = {
        .committedData = committedData
    };

    int errCode = PrepareForNotifyConflictAndObserver(dataItem, deviceInfo, notify);
    if (errCode != E_OK) {
        if (errCode == -E_IGNOR_DATA) {
            errCode = E_OK;
        }
        return errCode;
    }

    PutConflictData(dataItem, notify.getData, deviceInfo, notify.dataStatus, committedData);
    if (notify.dataStatus.isDefeated == true) {
        LOGD("Data status is defeated:%d", errCode);
        return ResetSaveSyncStatements(errCode);
    }

    std::string origDev = GetOriginDevName(dataItem, notify.getData.origDev);
    errCode = SaveSyncDataToDatabase(dataItem, notify.hashKey, origDev, deviceInfo.deviceName);
    if (errCode == E_OK) {
        PutIntoCommittedData(dataItem, notify.getData, notify.dataStatus, notify.hashKey, committedData);
        maxStamp = std::max(dataItem.timeStamp, maxStamp);
    } else {
        LOGE("Save sync data to db failed:%d", errCode);
    }
    return ResetSaveSyncStatements(errCode);
}

int SQLiteSingleVerStorageExecutor::GetAllMetaKeys(std::vector<Key> &keys) const
{
    sqlite3_stmt *statement = nullptr;
    int errCode = -E_BASE;
    if (attachMetaMode_ == true) {
        errCode = SQLiteUtils::GetStatement(dbHandle_, SELECT_ATTACH_ALL_META_KEYS, statement);
    } else {
        errCode = SQLiteUtils::GetStatement(dbHandle_, SELECT_ALL_META_KEYS, statement);
    }
    if (errCode != E_OK) {
        LOGE("[SingleVerExe][GetAllKey] Get statement failed:%d", errCode);
        return errCode;
    }

    errCode = GetAllKeys(statement, keys);
    SQLiteUtils::ResetStatement(statement, true, errCode);
    return errCode;
}

int SQLiteSingleVerStorageExecutor::GetAllSyncedEntries(const std::string &deviceName,
    std::vector<Entry> &entries) const
{
    sqlite3_stmt *statement = nullptr;
    std::string sql = (executorState_ == ExecutorState::CACHE_ATTACH_MAIN ?
        SELECT_ALL_SYNC_ENTRIES_BY_DEV_FROM_CACHEHANDLE : SELECT_ALL_SYNC_ENTRIES_BY_DEV);
    int errCode = SQLiteUtils::GetStatement(dbHandle_, sql, statement);
    if (errCode != E_OK) {
        LOGE("Get all entries statement failed:%d", errCode);
        return errCode;
    }

    // When removing device data in cache mode, key is "remove", value is deviceID's hashstring.
    // Therefore no need to transfer hashstring when migrating.
    std::string devName = isSyncMigrating_ ? deviceName : DBCommon::TransferHashString(deviceName);
    std::vector<uint8_t> devVect(devName.begin(), devName.end());
    errCode = SQLiteUtils::BindBlobToStatement(statement, 1, devVect, true); // bind the 1st to device.
    if (errCode != E_OK) {
        LOGE("Failed to bind the synced device for all entries:%d", errCode);
        goto ERROR;
    }

    errCode = GetAllEntries(statement, entries);
ERROR:
    SQLiteUtils::ResetStatement(statement, true, errCode);
    return errCode;
}

int SQLiteSingleVerStorageExecutor::GetAllEntries(sqlite3_stmt *statement, std::vector<Entry> &entries) const
{
    if (statement == nullptr) {
        return -E_INVALID_DB;
    }
    int errCode;
    do {
        errCode = SQLiteUtils::StepWithRetry(statement, isMemDb_);
        if (errCode == SQLiteUtils::MapSQLiteErrno(SQLITE_ROW)) {
            Entry entry;
            errCode = SQLiteUtils::GetColumnBlobValue(statement, 0, entry.key); // No.0 is the key
            if (errCode != E_OK) {
                break;
            }
            errCode = SQLiteUtils::GetColumnBlobValue(statement, 1, entry.value); // No.1 is the value
            if (errCode != E_OK) {
                break;
            }

            entries.push_back(std::move(entry));
        } else if (errCode == SQLiteUtils::MapSQLiteErrno(SQLITE_DONE)) {
            errCode = E_OK;
            break;
        } else {
            LOGE("SQLite step for all entries failed:%d", errCode);
            break;
        }
    } while (true);

    return errCode;
}

int SQLiteSingleVerStorageExecutor::GetAllKeys(sqlite3_stmt *statement, std::vector<Key> &keys) const
{
    if (statement == nullptr) {
        return -E_INVALID_DB;
    }
    int errCode;
    do {
        errCode = SQLiteUtils::StepWithRetry(statement, isMemDb_);
        if (errCode == SQLiteUtils::MapSQLiteErrno(SQLITE_ROW)) {
            Key key;
            errCode = SQLiteUtils::GetColumnBlobValue(statement, 0, key);
            if (errCode != E_OK) {
                break;
            }

            keys.push_back(std::move(key));
        } else if (errCode == SQLiteUtils::MapSQLiteErrno(SQLITE_DONE)) {
            errCode = E_OK;
            break;
        } else {
            LOGE("SQLite step for getting all keys failed:%d", errCode);
            break;
        }
    } while (true);

    return errCode;
}

int SQLiteSingleVerStorageExecutor::BindSavedSyncData(sqlite3_stmt *statement, const DataItem &dataItem,
    const Key &hashKey, const SyncDataDevices &devices, int beginIndex)
{
    int errCode = SQLiteUtils::BindBlobToStatement(statement, beginIndex + BIND_SYNC_HASH_KEY_INDEX, hashKey, false);
    if (errCode != E_OK) {
        LOGE("Bind saved sync data hash key failed:%d", errCode);
        return errCode;
    }

    // if delete flag is set, just use the hash key instead of the key
    if ((dataItem.flag & DataItem::DELETE_FLAG) == DataItem::DELETE_FLAG) {
        errCode = SQLiteUtils::MapSQLiteErrno(sqlite3_bind_zeroblob(statement, beginIndex + BIND_SYNC_KEY_INDEX, -1));
    } else {
        errCode = SQLiteUtils::BindBlobToStatement(statement, beginIndex + BIND_SYNC_KEY_INDEX, dataItem.key, false);
    }

    if (errCode != E_OK) {
        LOGE("Bind saved sync data key failed:%d", errCode);
        return errCode;
    }

    errCode = SQLiteUtils::BindBlobToStatement(statement, beginIndex + BIND_SYNC_VAL_INDEX, dataItem.value, true);
    if (errCode != E_OK) {
        LOGE("Bind saved sync data value failed:%d", errCode);
        return errCode;
    }

    errCode = SQLiteUtils::BindInt64ToStatement(statement, beginIndex + BIND_SYNC_STAMP_INDEX, dataItem.timeStamp);
    if (errCode != E_OK) {
        LOGE("Bind saved sync data stamp failed:%d", errCode);
        return errCode;
    }

    errCode = SQLiteUtils::BindInt64ToStatement(statement, beginIndex + BIND_SYNC_W_TIME_INDEX,
        dataItem.writeTimeStamp);
    LOGD("Write timestamp:%llu timestamp:%llu, %llu", dataItem.writeTimeStamp, dataItem.timeStamp, dataItem.flag);
    if (errCode != E_OK) {
        LOGE("Bind saved sync data write stamp failed:%d", errCode);
        return errCode;
    }

    return BindDevForSavedSyncData(statement, dataItem, devices.origDev, devices.dev);
}

void SQLiteSingleVerStorageExecutor::PutConflictData(const DataItem &itemPut, const DataItem &itemGet,
    const DeviceInfo &deviceInfo, const DataOperStatus &dataStatus,
    SingleVerNaturalStoreCommitNotifyData *commitData)
{
    if (commitData == nullptr) {
        return;
    }

    bool conflictNotifyMatch = commitData->IsConflictedNotifyMatched(itemPut, itemGet);
    if (!conflictNotifyMatch) {
        return;
    }

    if (dataStatus.preStatus == DataStatus::NOEXISTED ||
        ((dataStatus.preStatus == DataStatus::DELETED) && (dataStatus.isDeleted == true))) {
        return;
    }

    Key origKey;
    if ((itemPut.flag & DataItem::DELETE_FLAG) == DataItem::DELETE_FLAG) {
        origKey = itemGet.key;
    } else {
        origKey = itemPut.key;
    }

    // insert db original entry
    std::vector<uint8_t> getDevVect(itemGet.dev.begin(), itemGet.dev.end());
    DataItemInfo orgItemInfo = {itemGet, true, getDevVect};
    orgItemInfo.dataItem.key = origKey;
    commitData->InsertConflictedItem(orgItemInfo, true);

    // insert conflict entry
    std::string putDeviceName = DBCommon::TransferHashString(deviceInfo.deviceName);
    std::vector<uint8_t> putDevVect(putDeviceName.begin(), putDeviceName.end());

    DataItemInfo newItemInfo = {itemPut, deviceInfo.isLocal, putDevVect};
    newItemInfo.dataItem.key = origKey;
    commitData->InsertConflictedItem(newItemInfo, false);
}

int SQLiteSingleVerStorageExecutor::Reset()
{
    if (isTransactionOpen_) {
        Rollback();
    }

    int errCode = ResetForSavingData(SingleVerDataType::SYNC_TYPE);
    if (errCode != E_OK) {
        LOGE("Finalize the sync resources for saving sync data failed: %d", errCode);
    }

    errCode = ResetForSavingData(SingleVerDataType::LOCAL_TYPE);
    if (errCode != E_OK) {
        LOGE("Finalize the local resources for saving sync data failed: %d", errCode);
    }
    return SQLiteStorageExecutor::Reset();
}

int SQLiteSingleVerStorageExecutor::GetSyncDataItemPre(const DataItem &itemPut, DataItem &itemGet,
    Key &hashKey) const
{
    if (isSyncMigrating_) {
        hashKey = itemPut.hashKey;
    } else if ((itemPut.flag & DataItem::DELETE_FLAG) == DataItem::DELETE_FLAG) {
        hashKey = itemPut.key;
    } else {
        int errCode = DBCommon::CalcValueHash(itemPut.key, hashKey);
        if (errCode != E_OK) {
            return errCode;
        }
    }

    return GetSyncDataPreByHashKey(hashKey, itemGet);
}

int SQLiteSingleVerStorageExecutor::GetSyncDataPreByHashKey(const Key &hashKey, DataItem &itemGet) const
{
    auto statement = isSyncMigrating_ ? migrateSyncStatements_.queryStatement : saveSyncStatements_.queryStatement;
    int errCode = SQLiteUtils::BindBlobToStatement(statement, 1, hashKey, false); // 1st arg.
    if (errCode != E_OK) {
        return errCode;
    }

    errCode = SQLiteUtils::StepWithRetry(statement, isMemDb_);
    if (errCode == SQLiteUtils::MapSQLiteErrno(SQLITE_DONE)) { // no find the key
        errCode = -E_NOT_FOUND;
    } else if (errCode == SQLiteUtils::MapSQLiteErrno(SQLITE_ROW)) {
        itemGet.timeStamp = static_cast<TimeStamp>(sqlite3_column_int64(statement, SYNC_RES_TIME_INDEX));
        itemGet.writeTimeStamp = static_cast<TimeStamp>(sqlite3_column_int64(statement, SYNC_RES_W_TIME_INDEX));
        itemGet.flag = static_cast<uint64_t>(sqlite3_column_int64(statement, SYNC_RES_FLAG_INDEX));
        errCode = SQLiteUtils::GetColumnBlobValue(statement, SYNC_RES_KEY_INDEX, itemGet.key);
        if (errCode != E_OK) {
            return errCode;
        }
        std::vector<uint8_t> devVect;
        errCode = SQLiteUtils::GetColumnBlobValue(statement, SYNC_RES_DEVICE_INDEX, devVect);
        if (errCode != E_OK) {
            return errCode;
        }

        std::vector<uint8_t> origDevVect;
        errCode = SQLiteUtils::GetColumnBlobValue(statement, SYNC_RES_ORI_DEV_INDEX, origDevVect);
        if (errCode != E_OK) {
            return errCode;
        }
        itemGet.dev.assign(devVect.begin(), devVect.end());
        itemGet.origDev.assign(origDevVect.begin(), origDevVect.end());
    }
    return errCode;
}

int SQLiteSingleVerStorageExecutor::DeleteLocalDataInner(SingleVerNaturalStoreCommitNotifyData *committedData,
    const Key &key, const Value &value)
{
    if (committedData != nullptr) {
        Key hashKey;
        int innerErrCode = DBCommon::CalcValueHash(key, hashKey);
        if (innerErrCode != E_OK) {
            return innerErrCode;
        }
        committedData->InitKeyPropRecord(hashKey, ExistStatus::EXIST);
    }

    std::string sql = DELETE_LOCAL_SQL;
    if (executorState_ == ExecutorState::CACHE_ATTACH_MAIN)  {
        sql = DELETE_LOCAL_SQL_FROM_CACHEHANDLE;
    }
    sqlite3_stmt *statement = nullptr;
    int errCode = SQLiteUtils::GetStatement(dbHandle_, sql, statement);
    if (errCode != E_OK) {
        goto ERROR;
    }

    errCode = SQLiteUtils::BindBlobToStatement(statement, 1, key, false);
    if (errCode != E_OK) {
        LOGE("Bind the key error(%d) when delete kv data.", errCode);
        goto ERROR;
    }

    errCode = SQLiteUtils::StepWithRetry(statement, isMemDb_);
    if (errCode == SQLiteUtils::MapSQLiteErrno(SQLITE_DONE)) {
        if (sqlite3_changes(dbHandle_) > 0) {
            if (committedData != nullptr) {
                Entry entry = {key, value};
                committedData->InsertCommittedData(std::move(entry), DataType::DELETE, true);
            } else {
                LOGE("DeleteLocalKvData failed to do commit notify because of OOM.");
            }
            errCode = E_OK;
        }
    }

ERROR:
    SQLiteUtils::ResetStatement(statement, true, errCode);
    return CheckCorruptedStatus(errCode);
}

int SQLiteSingleVerStorageExecutor::DeleteLocalKvData(const Key &key,
    SingleVerNaturalStoreCommitNotifyData *committedData, Value &value, TimeStamp &timeStamp)
{
    int errCode = GetKvData(SingleVerDataType::LOCAL_TYPE, key, value, timeStamp);
    if (errCode != E_OK) {
        return CheckCorruptedStatus(errCode);
    }

    return DeleteLocalDataInner(committedData, key, value);
}

int SQLiteSingleVerStorageExecutor::RemoveDeviceData(const std::string &deviceName)
{
    // Transfer the device name.
    std::string devName = DBCommon::TransferHashString(deviceName);
    sqlite3_stmt *statement = nullptr;
    std::vector<uint8_t> devVect(devName.begin(), devName.end());

    int errCode = SQLiteUtils::GetStatement(dbHandle_, REMOVE_DEV_DATA_SQL, statement);
    if (errCode != E_OK) {
        goto ERROR;
    }

    errCode = SQLiteUtils::BindBlobToStatement(statement, 1, devVect, true); // only one arg.
    if (errCode != E_OK) {
        LOGE("Failed to bind the removed device:%d", errCode);
        goto ERROR;
    }

    errCode = SQLiteUtils::StepWithRetry(statement, isMemDb_);
    if (errCode != SQLiteUtils::MapSQLiteErrno(SQLITE_DONE)) {
        LOGE("Failed to execute rm the device synced data:%d", errCode);
    } else {
        errCode = E_OK;
    }

ERROR:
    SQLiteUtils::ResetStatement(statement, true, errCode);
    return CheckCorruptedStatus(errCode);
}

int SQLiteSingleVerStorageExecutor::StepForResultEntries(sqlite3_stmt *statement, std::vector<Entry> &entries) const
{
    entries.clear();
    entries.shrink_to_fit();
    Entry entry;
    int errCode = E_OK;
    do {
        errCode = SQLiteUtils::StepWithRetry(statement, isMemDb_);
        if (errCode == SQLiteUtils::MapSQLiteErrno(SQLITE_ROW)) {
            errCode = SQLiteUtils::GetColumnBlobValue(statement, 0, entry.key);
            if (errCode != E_OK) {
                return errCode;
            }

            errCode = SQLiteUtils::GetColumnBlobValue(statement, 1, entry.value);
            if (errCode != E_OK) {
                return errCode;
            }

            entries.push_back(std::move(entry));
        } else if (errCode == SQLiteUtils::MapSQLiteErrno(SQLITE_DONE)) {
            errCode = E_OK;
            break;
        } else {
            LOGE("SQLite step failed:%d", errCode);
            return errCode;
        }
    } while (true);

    // if select no result, return the -E_NOT_FOUND.
    if (entries.empty()) {
        errCode = -E_NOT_FOUND;
    }

    return errCode;
}

int SQLiteSingleVerStorageExecutor::BindDevForSavedSyncData(sqlite3_stmt *statement, const DataItem &dataItem,
    const std::string &origDev, const std::string &deviceName, int beginIndex)
{
    int errCode = SQLiteUtils::BindInt64ToStatement(statement, beginIndex + BIND_SYNC_FLAG_INDEX,
        static_cast<int64_t>(dataItem.flag));
    if (errCode != E_OK) {
        LOGE("Bind saved sync data flag failed:%d", errCode);
        return errCode;
    }

    std::vector<uint8_t> devVect(deviceName.begin(), deviceName.end());
    errCode = SQLiteUtils::BindBlobToStatement(statement, beginIndex + BIND_SYNC_DEV_INDEX, devVect, true);
    if (errCode != E_OK) {
        LOGE("Bind dev for sync data failed:%d", errCode);
        return errCode;
    }

    std::vector<uint8_t> origDevVect(origDev.begin(), origDev.end());
    errCode = SQLiteUtils::BindBlobToStatement(statement, beginIndex + BIND_SYNC_ORI_DEV_INDEX, origDevVect, true);
    if (errCode != E_OK) {
        LOGE("Bind orig dev for sync data failed:%d", errCode);
    }

    return errCode;
}

size_t SQLiteSingleVerStorageExecutor::GetDataItemSerialSize(const DataItem &item, size_t appendLen)
{
    // timestamp and local flag: 3 * uint64_t, version(uint32_t), key, value, origin dev and the padding size.
    // the size would not be very large.
    static const size_t maxOrigDevLength = 40;
    size_t devLength = std::max(maxOrigDevLength, item.origDev.size());
    size_t dataSize = (Parcel::GetUInt64Len() * 3 + Parcel::GetUInt32Len() + Parcel::GetVectorCharLen(item.key) +
        Parcel::GetVectorCharLen(item.value) + devLength + appendLen);

    return dataSize;
}

int SQLiteSingleVerStorageExecutor::GetDataItemForSync(sqlite3_stmt *statement, DataItem &dataItem)
{
    dataItem.timeStamp = static_cast<uint64_t>(sqlite3_column_int64(statement, SYNC_RES_TIME_INDEX));
    dataItem.writeTimeStamp = static_cast<uint64_t>(sqlite3_column_int64(statement, SYNC_RES_W_TIME_INDEX));
    dataItem.flag = static_cast<uint64_t>(sqlite3_column_int64(statement, SYNC_RES_FLAG_INDEX));
    dataItem.flag &= (~DataItem::LOCAL_FLAG);
    std::vector<uint8_t> devVect;
    int errCode = SQLiteUtils::GetColumnBlobValue(statement, SYNC_RES_ORI_DEV_INDEX, devVect);
    if (errCode != E_OK) {
        return errCode;
    }

    dataItem.origDev = std::string(devVect.begin(), devVect.end());
    int keyIndex = SYNC_RES_KEY_INDEX;
    // If the data has been deleted, just use the hash key for sync.
    if ((dataItem.flag & DataItem::DELETE_FLAG) == DataItem::DELETE_FLAG) {
        keyIndex = SYNC_RES_HASH_KEY_INDEX;
    }

    errCode = SQLiteUtils::GetColumnBlobValue(statement, keyIndex, dataItem.key);
    if (errCode != E_OK) {
        return errCode;
    }

    return SQLiteUtils::GetColumnBlobValue(statement, SYNC_RES_VAL_INDEX, dataItem.value);
}

int SQLiteSingleVerStorageExecutor::InitResultSet(const Key &keyPrefix, sqlite3_stmt *&countStmt)
{
    if (dbHandle_ == nullptr) {
        return -E_INVALID_DB;
    }
    // bind statement for count
    int errCode = SQLiteUtils::GetStatement(dbHandle_, SELECT_COUNT_SYNC_PREFIX_SQL, countStmt);
    if (errCode != E_OK) {
        LOGE("Get count statement for resultset error:%d", errCode);
        return errCode;
    }

    errCode = SQLiteUtils::BindPrefixKey(countStmt, 1, keyPrefix); // first argument is key
    if (errCode != E_OK) {
        LOGE("Bind count key error:%d", errCode);
        goto ERROR;
    }
    // bind statement for result set
    errCode = SQLiteUtils::GetStatement(dbHandle_, SELECT_SYNC_ROWID_PREFIX_SQL, getResultRowIdStatement_);
    if (errCode != E_OK) {
        LOGE("Get result set rowid statement error:%d", errCode);
        goto ERROR;
    }

    errCode = SQLiteUtils::GetStatement(dbHandle_, SELECT_SYNC_DATA_BY_ROWID_SQL, getResultEntryStatement_);
    if (errCode != E_OK) {
        LOGE("Get result set entry statement error:%d", errCode);
        goto ERROR;
    }

    errCode = SQLiteUtils::BindPrefixKey(getResultRowIdStatement_, 1, keyPrefix); // first argument is key
    if (errCode != E_OK) {
        LOGE("Bind result set rowid statement error:%d", errCode);
        goto ERROR;
    }
    return E_OK;

ERROR:
    SQLiteUtils::ResetStatement(countStmt, true, errCode);
    SQLiteUtils::ResetStatement(getResultRowIdStatement_, true, errCode);
    SQLiteUtils::ResetStatement(getResultEntryStatement_, true, errCode);
    return CheckCorruptedStatus(errCode);
}

int SQLiteSingleVerStorageExecutor::InitResultSetCount(QueryObject &queryObj, sqlite3_stmt *&countStmt)
{
    if (dbHandle_ == nullptr) {
        return -E_INVALID_DB;
    }

    std::string countSql;
    int errCode = queryObj.GetCountQuerySql(countSql);
    if (errCode != E_OK) {
        return errCode;
    }

    errCode = queryObj.GetCountSqlStatement(dbHandle_, countSql, countStmt);
    if (errCode != E_OK) {
        LOGE("Get count bind statement error:%d", errCode);
        SQLiteUtils::ResetStatement(countStmt, true, errCode);
        return errCode;
    }
    return errCode;
}

int SQLiteSingleVerStorageExecutor::InitResultSetContent(QueryObject &queryObj)
{
    std::string selectSql;
    int errCode = queryObj.GetQuerySql(selectSql, true); // only rowid sql
    if (errCode != E_OK) {
        return errCode;
    }
    // bind statement for result set
    errCode = queryObj.GetQuerySqlStatement(dbHandle_, selectSql, getResultRowIdStatement_);
    if (errCode != E_OK) {
        LOGE("[SqlSinExe][InitResSetContent] Bind result set rowid statement of query error:%d", errCode);
        SQLiteUtils::ResetStatement(getResultRowIdStatement_, true, errCode);
        return errCode;
    }
    errCode = SQLiteUtils::GetStatement(dbHandle_, SELECT_SYNC_DATA_BY_ROWID_SQL, getResultEntryStatement_);
    if (errCode != E_OK) {
        LOGE("[SqlSinExe][InitResSetContent] Get result set entry statement of query error:%d", errCode);
        return CheckCorruptedStatus(errCode);
    }
    return errCode;
}

int SQLiteSingleVerStorageExecutor::InitResultSet(QueryObject &queryObj, sqlite3_stmt *&countStmt)
{
    if (dbHandle_ == nullptr) {
        return -E_INVALID_DB;
    }
    int errCode = E_OK;
    bool isValid = queryObj.IsValid(errCode);
    if (!isValid) {
        return errCode;
    }

    errCode = InitResultSetCount(queryObj, countStmt);
    if (errCode != E_OK) {
        return CheckCorruptedStatus(errCode);
    }

    errCode = InitResultSetContent(queryObj);
    if (errCode != E_OK) {
        SQLiteUtils::ResetStatement(countStmt, true, errCode);
    }
    return CheckCorruptedStatus(errCode);
}

int SQLiteSingleVerStorageExecutor::UpdateLocalDataTimestamp(TimeStamp timestamp)
{
    const std::string updateSql = "UPDATE local_data SET timestamp=";
    std::string sql = updateSql + std::to_string(timestamp) + " WHERE timestamp=0;";
    int errCode = SQLiteUtils::ExecuteRawSQL(dbHandle_, sql);
    return CheckCorruptedStatus(errCode);
}

void SQLiteSingleVerStorageExecutor::SetAttachMetaMode(bool attachMetaMode)
{
    attachMetaMode_ = attachMetaMode;
}

int SQLiteSingleVerStorageExecutor::GetOneRawDataItem(sqlite3_stmt *statement, DataItem &dataItem,
    uint64_t &verInCurCacheDb, bool isCacheDb) const
{
    int errCode = SQLiteUtils::GetColumnBlobValue(statement, SYNC_RES_KEY_INDEX, dataItem.key);
    if (errCode != E_OK) {
        return errCode;
    }

    errCode = SQLiteUtils::GetColumnBlobValue(statement, SYNC_RES_VAL_INDEX, dataItem.value);
    if (errCode != E_OK) {
        return errCode;
    }

    dataItem.timeStamp = static_cast<uint64_t>(sqlite3_column_int64(statement, SYNC_RES_TIME_INDEX));
    dataItem.flag = static_cast<uint64_t>(sqlite3_column_int64(statement, SYNC_RES_FLAG_INDEX));

    std::vector<uint8_t> devVect;
    errCode = SQLiteUtils::GetColumnBlobValue(statement, SYNC_RES_DEVICE_INDEX, devVect);
    if (errCode != E_OK) {
        return errCode;
    }
    dataItem.dev  = std::string(devVect.begin(), devVect.end());

    devVect.clear();
    errCode = SQLiteUtils::GetColumnBlobValue(statement, SYNC_RES_ORI_DEV_INDEX, devVect);
    if (errCode != E_OK) {
        return errCode;
    }
    dataItem.origDev = std::string(devVect.begin(), devVect.end());

    errCode = SQLiteUtils::GetColumnBlobValue(statement, SYNC_RES_HASH_KEY_INDEX, dataItem.hashKey);
    if (errCode != E_OK) {
        return errCode;
    }
    dataItem.writeTimeStamp = static_cast<uint64_t>(sqlite3_column_int64(statement, SYNC_RES_W_TIME_INDEX));
    if (errCode != E_OK) {
        return errCode;
    }
    if (!isCacheDb) {
        return E_OK;
    }
    verInCurCacheDb = static_cast<uint64_t>(sqlite3_column_int64(statement, SYNC_RES_VERSION_INDEX));

    return E_OK;
}

int SQLiteSingleVerStorageExecutor::GetAllDataItems(sqlite3_stmt *statement, std::vector<DataItem> &dataItems,
    uint64_t &verInCurCacheDb, bool isCacheDb) const
{
    dataItems.clear();
    dataItems.shrink_to_fit();
    DataItem dataItem;
    int errCode;
    do {
        errCode = SQLiteUtils::StepWithRetry(statement, isMemDb_);
        if (errCode == SQLiteUtils::MapSQLiteErrno(SQLITE_ROW)) {
            errCode = GetOneRawDataItem(statement, dataItem, verInCurCacheDb, isCacheDb);
            if (errCode != E_OK) {
                return errCode;
            }
            dataItems.push_back(std::move(dataItem));
        } else if (errCode == SQLiteUtils::MapSQLiteErrno(SQLITE_DONE)) {
            errCode = E_OK;
            break;
        } else {
            LOGE("SQLite step failed:%d", errCode);
            break;
        }
    } while (true);

    return CheckCorruptedStatus(errCode);
}

int SQLiteSingleVerStorageExecutor::OpenResultSetForCacheRowIdModeCommon(std::vector<int64_t> &rowIdCache,
    uint32_t cacheLimit, int &count)
{
    int errCode = SQLiteUtils::GetStatement(dbHandle_, SELECT_SYNC_DATA_BY_ROWID_SQL, getResultEntryStatement_);
    if (errCode != E_OK) {
        LOGE("[SqlSinExe][OpenResSetRowId][Common] Get entry stmt fail, errCode=%d", errCode);
        return CheckCorruptedStatus(errCode);
    }
    errCode = StartTransaction(TransactType::DEFERRED);
    if (errCode != E_OK) {
        SQLiteUtils::ResetStatement(getResultEntryStatement_, true, errCode);
        return CheckCorruptedStatus(errCode);
    }
    // Now Ready To Execute
    errCode = ResultSetLoadRowIdCache(rowIdCache, cacheLimit, 0, count);
    if (errCode != E_OK) {
        SQLiteUtils::ResetStatement(getResultEntryStatement_, true, errCode);
        Rollback();
        return CheckCorruptedStatus(errCode);
    }
    // Consider finalize getResultRowIdStatement_ here if count equal to size of rowIdCache.
    return E_OK;
}

int SQLiteSingleVerStorageExecutor::ResultSetLoadRowIdCache(std::vector<int64_t> &rowIdCache, uint32_t cacheLimit,
    uint32_t cacheStartPos, int &count)
{
    rowIdCache.clear();
    count = 0;
    while (true) {
        int errCode = SQLiteUtils::StepWithRetry(getResultRowIdStatement_, isMemDb_);
        if (errCode == SQLiteUtils::MapSQLiteErrno(SQLITE_ROW)) {
            if (count >= static_cast<int>(cacheStartPos) && rowIdCache.size() < cacheLimit) {
                // If we can start cache, and, if we can still cache
                int64_t rowid = sqlite3_column_int64(getResultRowIdStatement_, 0);
                rowIdCache.push_back(rowid);
            }
            // Always increase the count
            count++;
        } else if (errCode == SQLiteUtils::MapSQLiteErrno(SQLITE_DONE)) {
            break;
        } else {
            LOGE("[SqlSinExe][ResSetLoadCache] Step fail, errCode=%d", errCode);
            rowIdCache.clear();
            count = 0;
            return CheckCorruptedStatus(errCode);
        }
    }
    return E_OK;
}

void SQLiteSingleVerStorageExecutor::FinalizeAllStatements()
{
    int errCode = E_OK;
    SQLiteUtils::ResetStatement(saveLocalStatements_.putStatement, true, errCode);
    if (errCode != E_OK) {
        LOGE("Finalize saveLocal put statements failed, error: %d", errCode);
    }

    SQLiteUtils::ResetStatement(saveLocalStatements_.queryStatement, true, errCode);
    if (errCode != E_OK) {
        LOGE("Finalize saveLocal query statement failed, error: %d", errCode);
    }

    SQLiteUtils::ResetStatement(saveSyncStatements_.putStatement, true, errCode);
    if (errCode != E_OK) {
        LOGE("Finalize saveSync put statement failed, error: %d", errCode);
    }

    SQLiteUtils::ResetStatement(saveSyncStatements_.queryStatement, true, errCode);
    if (errCode != E_OK) {
        LOGE("Finalize saveSync query statement failed, error: %d", errCode);
    }

    SQLiteUtils::ResetStatement(getResultRowIdStatement_, true, errCode);
    if (errCode != E_OK) {
        LOGE("Finalize getResultRowIdStatement_ failed, error: %d", errCode);
    }

    SQLiteUtils::ResetStatement(getResultEntryStatement_, true, errCode);
    if (errCode != E_OK) {
        LOGE("Finalize getResultEntryStatement_ failed, error: %d", errCode);
    }

    SQLiteUtils::ResetStatement(migrateSyncStatements_.putStatement, true, errCode);
    if (errCode != E_OK) {
        LOGE("Finalize migrateSync put statements failed, error: %d", errCode);
    }

    SQLiteUtils::ResetStatement(migrateSyncStatements_.queryStatement, true, errCode);
    if (errCode != E_OK) {
        LOGE("Finalize migrateSync query statement failed, error: %d", errCode);
    }

    ReleaseContinueStatement();
}

void SQLiteSingleVerStorageExecutor::SetConflictResolvePolicy(int policy)
{
    if (policy == DENY_OTHER_DEV_AMEND_CUR_DEV_DATA || policy == DEFAULT_LAST_WIN) {
        conflictResolvePolicy_ = policy;
    }
}
} // namespace DistributedDB
