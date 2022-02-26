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
int SQLiteSingleVerStorageExecutor::PrepareForSavingCacheData(SingleVerDataType type)
{
    int errCode = -E_NOT_SUPPORT;
    if (type == SingleVerDataType::LOCAL_TYPE) {
        std::string insertLocalSql = ((executorState_ == ExecutorState::CACHE_ATTACH_MAIN) ?
            INSERT_LOCAL_SQL_FROM_CACHEHANDLE : INSERT_CACHE_LOCAL_SQL);
        errCode = PrepareForSavingData(SELECT_CACHE_LOCAL_HASH_SQL, insertLocalSql, saveLocalStatements_);
    } else if (type == SingleVerDataType::SYNC_TYPE) {
        std::string insertSyncSql = ((executorState_ == ExecutorState::MAIN_ATTACH_CACHE) ?
            INSERT_CACHE_SYNC_SQL_FROM_MAINHANDLE : INSERT_CACHE_SYNC_SQL);
        std::string selectSyncHashSql = ((executorState_ == ExecutorState::MAIN_ATTACH_CACHE) ?
            SELECT_CACHE_SYNC_HASH_SQL_FROM_MAINHANDLE : SELECT_CACHE_SYNC_HASH_SQL);
        errCode = PrepareForSavingData(selectSyncHashSql, insertSyncSql, saveSyncStatements_);
    }
    if (errCode != E_OK) {
        LOGE("Prepare to save sync cache data failed:%d", errCode);
    }
    return CheckCorruptedStatus(errCode);
}

int SQLiteSingleVerStorageExecutor::ResetForSavingCacheData(SingleVerDataType type)
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

int SQLiteSingleVerStorageExecutor::ResetForMigrateCacheData()
{
    int errCode = E_OK;
    SQLiteUtils::ResetStatement(migrateSyncStatements_.putStatement, false, errCode);
    SQLiteUtils::ResetStatement(migrateSyncStatements_.queryStatement, false, errCode);

    return CheckCorruptedStatus(errCode);
}

int SQLiteSingleVerStorageExecutor::RemoveDeviceDataInCacheMode(const std::string &deviceName,
    bool isNeedNotify, uint64_t recordVersion) const
{
    // Transfer the device name.
    std::string devName = DBCommon::TransferHashString(deviceName);
    std::vector<uint8_t> devVect(devName.begin(), devName.end());

    Key hashKey;
    int errCode = DBCommon::CalcValueHash(REMOVE_DEVICE_DATA_KEY, hashKey);
    if (errCode != E_OK) {
        return errCode;
    }

    DataItem dataItem;
    dataItem.key = REMOVE_DEVICE_DATA_KEY;
    dataItem.value = devVect;
    if (isNeedNotify) {
        dataItem.flag = DataItem::REMOVE_DEVICE_DATA_NOTIFY_FLAG;
    } else {
        dataItem.flag = DataItem::REMOVE_DEVICE_DATA_FLAG;
    }

    sqlite3_stmt *statement = nullptr;
    std::string sql = (executorState_ == ExecutorState::MAIN_ATTACH_CACHE) ?
        INSERT_CACHE_SYNC_SQL_FROM_MAINHANDLE : INSERT_CACHE_SYNC_SQL;
    errCode = SQLiteUtils::GetStatement(dbHandle_, sql, statement);
    if (errCode != E_OK) {
        goto ERROR;
    }

    errCode = BindSyncDataInCacheMode(statement, dataItem, hashKey, recordVersion);
    if (errCode != E_OK) {
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

int SQLiteSingleVerStorageExecutor::GetMinVersionCacheData(
    std::vector<DataItem> &dataItems, uint64_t &minVerIncurCacheDb) const
{
    std::string sql;
    if (executorState_ == ExecutorState::MAIN_ATTACH_CACHE) {
        sql = MIGRATE_SELECT_MIN_VER_CACHEDATA_FROM_MAINHANDLE;
    } else if (executorState_ == ExecutorState::CACHE_ATTACH_MAIN)  {
        sql = MIGRATE_SELECT_MIN_VER_CACHEDATA_FROM_CACHEHANDLE;
    } else {
        return -E_INVALID_ARGS;
    }

    sqlite3_stmt *statement = nullptr;
    int errCode = SQLiteUtils::GetStatement(dbHandle_, sql, statement);
    if (errCode != E_OK) {
        LOGE("GetStatement fail when get min version cache data! errCode = [%d]", errCode);
        goto END;
    }

    errCode = GetAllDataItems(statement, dataItems, minVerIncurCacheDb, true);
    if (errCode != E_OK) {
        LOGE("Failed to get all the data items by the min version:[%d]", errCode);
    }

END:
    SQLiteUtils::ResetStatement(statement, true, errCode);
    return CheckCorruptedStatus(errCode);
}

int SQLiteSingleVerStorageExecutor::MigrateRmDevData(const DataItem &dataItem) const
{
    if (dataItem.key != REMOVE_DEVICE_DATA_KEY) {
        LOGE("This item not means remove devices data, can not continue exe!");
        return -E_INVALID_ARGS;
    }

    std::string sql;
    if (executorState_ == ExecutorState::MAIN_ATTACH_CACHE) {
        sql = REMOVE_DEV_DATA_SQL;
    } else if (executorState_ == ExecutorState::CACHE_ATTACH_MAIN)  {
        sql = REMOVE_DEV_DATA_SQL_FROM_CACHEHANDLE;
    } else {
        return -E_INVALID_ARGS;
    }

    sqlite3_stmt *statement = nullptr;
    int errCode = SQLiteUtils::GetStatement(dbHandle_, sql, statement);
    if (errCode != E_OK) {
        LOGE("GetStatement fail when remove device data migrating-data to main! errCode = [%d]", errCode);
        return CheckCorruptedStatus(errCode);
    }

    errCode = SQLiteUtils::BindBlobToStatement(statement, 1, dataItem.value, true);
    if (errCode != E_OK) {
        LOGE("[sinverExecutor][MiRmData] Bind dev for sync data failed:%d", errCode);
        goto END;
    }

    errCode = SQLiteUtils::StepWithRetry(statement, isMemDb_);
    if (errCode == SQLiteUtils::MapSQLiteErrno(SQLITE_DONE)) {
        errCode = E_OK;
    }
END:
    SQLiteUtils::ResetStatement(statement, true, errCode);
    return CheckCorruptedStatus(errCode);
}

int SQLiteSingleVerStorageExecutor::PutMigratingDataToMain(const std::vector<DataItem> &dataItems) const
{
    std::string sql;
    if (executorState_ == ExecutorState::MAIN_ATTACH_CACHE) {
        sql = MIGRATE_PUT_DATA_TO_MAINDB_FROM_MAINHANDLE;
    } else if (executorState_ == ExecutorState::CACHE_ATTACH_MAIN)  {
        sql = MIGRATE_PUT_DATA_TO_MAINDB_FROM_CACHEHANDLE;
    } else {
        return -E_INVALID_ARGS;
    }

    sqlite3_stmt *statement = nullptr;
    int errCode = SQLiteUtils::GetStatement(dbHandle_, sql, statement);
    if (errCode != E_OK) {
        LOGE("GetStatement fail when put migrating-data to main! errCode = [%d]", errCode);
        return CheckCorruptedStatus(errCode);
    }

    for (const auto &dataItem : dataItems) {
        if ((dataItem.flag & DataItem::REMOVE_DEVICE_DATA_FLAG) == DataItem::REMOVE_DEVICE_DATA_FLAG ||
            (dataItem.flag & DataItem::REMOVE_DEVICE_DATA_NOTIFY_FLAG) == DataItem::REMOVE_DEVICE_DATA_NOTIFY_FLAG) {
            errCode = MigrateRmDevData(dataItem);
            LOGI("[PutMigratingDataToMain]Execute remove devices data! errCode = [%d]", errCode);
            if (errCode != E_OK) {
                break;
            }
            continue;
        }
        SyncDataDevices devices{ dataItem.origDev, dataItem.dev };
        errCode = BindSavedSyncData(statement, dataItem, dataItem.hashKey, devices);
        if (errCode != E_OK) {
            goto END;
        }

        errCode = SQLiteUtils::StepWithRetry(statement, isMemDb_);
        if (errCode == SQLiteUtils::MapSQLiteErrno(SQLITE_DONE)) {
            errCode = E_OK;
        } else {
            LOGD("StepWithRetry fail when put migrating-data to main!");
            break;
        }
        SQLiteUtils::ResetStatement(statement, false, errCode);
    }
END:
    SQLiteUtils::ResetStatement(statement, true, errCode);
    return CheckCorruptedStatus(errCode);
}

int SQLiteSingleVerStorageExecutor::AttachMainDbAndCacheDb(CipherType type, const CipherPassword &passwd,
    const std::string &attachDbAbsPath, EngineState engineState)
{
    std::string attachAsName;
    if (engineState == EngineState::MAINDB) {
        attachAsName = "cache";
    } else if (engineState == EngineState::CACHEDB)  {
        attachAsName = "maindb";
    } else if (engineState == EngineState::ATTACHING) {
        executorState_ = ExecutorState::MAIN_ATTACH_CACHE;
        return E_OK;
    } else {
        return -E_INVALID_ARGS;
    }

    int errCode = SQLiteUtils::AttachNewDatabase(dbHandle_, type, passwd, attachDbAbsPath, attachAsName);
    if (errCode != E_OK) {
        LOGE("handle attach to [%s] fail! errCode = [%d]", attachAsName.c_str(), errCode);
        return CheckCorruptedStatus(errCode);
    }

    if (engineState == EngineState::MAINDB) {
        executorState_ = ExecutorState::MAIN_ATTACH_CACHE;
    } else if (engineState == EngineState::CACHEDB)  {
        executorState_ = ExecutorState::CACHE_ATTACH_MAIN;
    } else {
        return -E_INVALID_ARGS;
    }
    LOGD("[singleVerExecutor][attachDb] current engineState[%d], executorState[%d]", engineState, executorState_);

    return errCode;
}

int SQLiteSingleVerStorageExecutor::GetMaxVersionIncacheDb(uint64_t &maxVersion) const
{
    sqlite3_stmt *statement = nullptr;
    std::string sql;
    if (executorState_ == ExecutorState::MAIN_ATTACH_CACHE) {
        sql = GET_MAX_VER_CACHEDATA_FROM_MAINHANDLE;
    } else if (executorState_ == ExecutorState::CACHE_ATTACH_MAIN)  {
        sql = GET_MAX_VER_CACHEDATA_FROM_CACHEHANDLE;
    } else {
        return -E_INVALID_ARGS;
    }

    int errCode = SQLiteUtils::GetStatement(dbHandle_, sql, statement);
    if (errCode != E_OK) {
        LOGE("GetStatement fail when get max version in cache db! errCode = [%d]", errCode);
        goto END;
    }

    do {
        errCode = SQLiteUtils::StepWithRetry(statement, isMemDb_);
        if (errCode == SQLiteUtils::MapSQLiteErrno(SQLITE_ROW)) {
            maxVersion = static_cast<uint64_t>(sqlite3_column_int64(statement, 0));
        } else if (errCode == SQLiteUtils::MapSQLiteErrno(SQLITE_DONE)) {
            errCode = E_OK;
            break;
        } else {
            LOGE("SQLite step failed:%d", errCode);
            break;
        }
    } while (true);

END:
    SQLiteUtils::ResetStatement(statement, true, errCode);
    return CheckCorruptedStatus(errCode);
}

int SQLiteSingleVerStorageExecutor::MigrateSyncDataByVersion(uint64_t recordVer, NotifyMigrateSyncData &syncData,
    std::vector<DataItem> &dataItems)
{
    int errCode = StartTransaction(TransactType::IMMEDIATE);
    if (errCode != E_OK) {
        return errCode;
    }

    // Init migrate data.
    errCode = InitMigrateData();
    if (errCode != E_OK) {
        LOGE("Init migrate data failed, errCode = [%d]", errCode);
        goto END;
    }

    // fix dataItem timestap for mingrate
    errCode = ProcessTimeStampForSyncDataInCacheDB(dataItems);
    if (errCode != E_OK) {
        LOGE("Chang the time stamp for migrate failed! errCode = [%d]", errCode);
        goto END;
    }

    // observer and conflictdata
    errCode = NotifyForMigrateCacheDB(dataItems, syncData);
    if (errCode != E_OK) {
        LOGE("Notify cacheDb to mainDb failed! errCode = [%d]", errCode);
        goto END;
    }

    // Put this version into main db
    errCode = PutMigratingDataToMain(dataItems);
    if (errCode != E_OK) {
        LOGE("Put migrating data to main db failed! errCode = [%d]", errCode);
        goto END;
    }

    // delete recordVersion data
    errCode = DelCacheDbDataByVersion(recordVer);
    if (errCode != E_OK) {
        LOGE("Delete the migrated data in cacheDb! errCode = [%d]", errCode);
        goto END;
    }

    errCode = Commit();
    if (errCode != E_OK) {
        LOGE("Commit data error and rollback, errCode = [%d]", errCode);
        goto END;
    }
    return E_OK;
END:
    Rollback();
    return errCode;
}

int SQLiteSingleVerStorageExecutor::DelCacheDbDataByVersion(uint64_t version) const
{
    std::string sql;
    if (executorState_ == ExecutorState::MAIN_ATTACH_CACHE) {
        sql = MIGRATE_DEL_DATA_BY_VERSION_FROM_MAINHANDLE;
    } else if (executorState_ == ExecutorState::CACHE_ATTACH_MAIN)  {
        sql = MIGRATE_DEL_DATA_BY_VERSION_FROM_CACHEHANDLE;
    } else {
        return -E_INVALID_ARGS;
    }

    sqlite3_stmt *statement = nullptr;
    int errCode = SQLiteUtils::GetStatement(dbHandle_, sql, statement);
    if (errCode != E_OK) {
        LOGE("GetStatement fail when delete cache data by version! errCode = [%d]", errCode);
        return errCode;
    }

    errCode = SQLiteUtils::BindInt64ToStatement(statement, 1, static_cast<int64_t>(version));
    if (errCode != E_OK) {
        LOGE("[SingleVerExe] Bind destDbNickName error:[%d]", errCode);
        goto END;
    }

    errCode = SQLiteUtils::StepWithRetry(statement, isMemDb_);
    if (errCode == SQLiteUtils::MapSQLiteErrno(SQLITE_DONE)) {
        errCode = E_OK;
    }

END:
    SQLiteUtils::ResetStatement(statement, true, errCode);
    return CheckCorruptedStatus(errCode);
}

int SQLiteSingleVerStorageExecutor::VacuumLocalData() const
{
    std::string sql;
    if (executorState_ == ExecutorState::MAIN_ATTACH_CACHE) {
        sql = MIGRATE_VACUUM_LOCAL_SQL_FROM_MAINHANDLE;
    } else if (executorState_ == ExecutorState::CACHE_ATTACH_MAIN)  {
        sql = MIGRATE_VACUUM_LOCAL_SQL_FROM_CACHEHANDLE;
    } else {
        return -E_INVALID_ARGS;
    }

    int errCode = SQLiteUtils::ExecuteRawSQL(dbHandle_, sql);
    if (errCode != E_OK) {
        LOGE("SQLite sync mode failed: %d", errCode);
    }

    return CheckCorruptedStatus(errCode);
}

// The local table data is only for local reading and writing, which can be sensed by itself.
// The current migration process does not provide callback subscription function.
int SQLiteSingleVerStorageExecutor::MigrateLocalData()
{
    // Nick name "main" represent current database(dbhande) in sqlite grammar
    std::string migrateLocaldataSql;
    if (executorState_ == ExecutorState::MAIN_ATTACH_CACHE) {
        migrateLocaldataSql = MIGRATE_LOCAL_SQL_FROM_MAINHANDLE;
    } else if (executorState_ == ExecutorState::CACHE_ATTACH_MAIN)  {
        migrateLocaldataSql = MIGRATE_LOCAL_SQL_FROM_CACHEHANDLE;
    } else {
        return -E_INVALID_ARGS;
    }

    int errCode = SQLiteUtils::ExecuteRawSQL(dbHandle_, migrateLocaldataSql);
    if (errCode != E_OK) {
        LOGW("Failed to migrate the local data:%d", errCode);
        return CheckCorruptedStatus(errCode);
    }

    return VacuumLocalData();
}

int SQLiteSingleVerStorageExecutor::BindSyncDataInCacheMode(sqlite3_stmt *statement,
    const DataItem &dataItem, const Key &hashKey, uint64_t recordVersion) const
{
    int errCode = BindPrimaryKeySyncDataInCacheMode(statement, hashKey, recordVersion);
    if (errCode != E_OK) {
        LOGE("Bind saved sync data primary key failed:%d", errCode);
        return errCode;
    }

    // if delete flag is set, just use the hash key instead of the key
    if ((dataItem.flag & DataItem::DELETE_FLAG) == DataItem::DELETE_FLAG) {
        errCode = SQLiteUtils::MapSQLiteErrno(sqlite3_bind_zeroblob(statement, BIND_CACHE_SYNC_KEY_INDEX, -1));
    } else {
        errCode = SQLiteUtils::BindBlobToStatement(statement, BIND_CACHE_SYNC_KEY_INDEX, dataItem.key, false);
    }

    if (errCode != E_OK) {
        LOGE("Bind saved sync data key failed:%d", errCode);
        return errCode;
    }

    errCode = SQLiteUtils::BindBlobToStatement(statement, BIND_CACHE_SYNC_VAL_INDEX, dataItem.value, true);
    if (errCode != E_OK) {
        LOGE("Bind saved sync data value failed:%d", errCode);
        return errCode;
    }

    LOGD("Write timestamp:%llu timestamp%llu, %llu, version %llu", dataItem.writeTimeStamp, dataItem.timeStamp,
        dataItem.flag, recordVersion);
    errCode = SQLiteUtils::BindInt64ToStatement(statement, BIND_CACHE_SYNC_FLAG_INDEX,
        static_cast<int64_t>(dataItem.flag));
    if (errCode != E_OK) {
        LOGE("Bind saved sync data flag failed:%d", errCode);
        return errCode;
    }
    errCode = BindTimeStampSyncDataInCacheMode(statement, dataItem);
    if (errCode != E_OK) {
        LOGE("Bind saved sync data time stamp failed:%d", errCode);
        return errCode;
    }
    return BindDevSyncDataInCacheMode(statement, dataItem.origDev, dataItem.dev);
}

int SQLiteSingleVerStorageExecutor::BindPrimaryKeySyncDataInCacheMode(
    sqlite3_stmt *statement, const Key &hashKey, uint64_t recordVersion) const
{
    int errCode = SQLiteUtils::BindBlobToStatement(statement, BIND_CACHE_SYNC_HASH_KEY_INDEX, hashKey, false);
    if (errCode != E_OK) {
        LOGE("Bind saved sync data hash key failed:%d", errCode);
        return errCode;
    }
    errCode = SQLiteUtils::BindInt64ToStatement(statement, BIND_CACHE_SYNC_VERSION_INDEX, recordVersion);
    if (errCode != E_OK) {
        LOGE("Bind saved sync data version failed:%d", errCode);
    }
    return errCode;
}

int SQLiteSingleVerStorageExecutor::BindTimeStampSyncDataInCacheMode(
    sqlite3_stmt *statement, const DataItem &dataItem) const
{
    int errCode = SQLiteUtils::BindInt64ToStatement(statement, BIND_CACHE_SYNC_STAMP_INDEX, dataItem.timeStamp);
    if (errCode != E_OK) {
        LOGE("Bind saved sync data stamp failed:%d", errCode);
        return errCode;
    }

    errCode = SQLiteUtils::BindInt64ToStatement(statement, BIND_CACHE_SYNC_W_TIME_INDEX, dataItem.writeTimeStamp);
    if (errCode != E_OK) {
        LOGE("Bind saved sync data write stamp failed:%d", errCode);
    }
    return errCode;
}

int SQLiteSingleVerStorageExecutor::BindDevSyncDataInCacheMode(sqlite3_stmt *statement,
    const std::string &origDev, const std::string &deviceName) const
{
    std::string devName = DBCommon::TransferHashString(deviceName);
    std::vector<uint8_t> devVect(devName.begin(), devName.end());
    int errCode = SQLiteUtils::BindBlobToStatement(statement, BIND_CACHE_SYNC_DEV_INDEX, devVect, true);
    if (errCode != E_OK) {
        LOGE("Bind dev for sync data failed:%d", errCode);
        return errCode;
    }

    std::vector<uint8_t> origDevVect(origDev.begin(), origDev.end());
    errCode = SQLiteUtils::BindBlobToStatement(statement, BIND_CACHE_SYNC_ORI_DEV_INDEX, origDevVect, true);
    if (errCode != E_OK) {
        LOGE("Bind orig dev for sync data failed:%d", errCode);
    }
    return errCode;
}

int SQLiteSingleVerStorageExecutor::SaveSyncDataItemInCacheMode(DataItem &dataItem,
    const DeviceInfo &deviceInfo, TimeStamp &maxStamp, uint64_t recordVersion)
{
    Key hashKey;
    if ((dataItem.flag & DataItem::DELETE_FLAG) == DataItem::DELETE_FLAG) {
        hashKey = dataItem.key;
    } else {
        int errCode = DBCommon::CalcValueHash(dataItem.key, hashKey);
        if (errCode != E_OK) {
            return errCode;
        }
    }

    std::string origDev = dataItem.origDev;
    if (((dataItem.flag & DataItem::LOCAL_FLAG) != 0) && dataItem.origDev.empty()) {
        origDev.clear();
    }
    dataItem.dev = deviceInfo.deviceName;
    dataItem.origDev = origDev;
    int errCode = SaveSyncDataToCacheDatabase(dataItem, hashKey, recordVersion);
    if (errCode == E_OK) {
        maxStamp = std::max(dataItem.timeStamp, maxStamp);
    } else {
        LOGE("Save sync data to db failed:%d", errCode);
    }
    return ResetForSavingCacheData(SingleVerDataType::SYNC_TYPE);
}

int SQLiteSingleVerStorageExecutor::SaveSyncDataToCacheDatabase(const DataItem &dataItem,
    const Key &hashKey, uint64_t recordVersion) const
{
    auto statement = saveSyncStatements_.putStatement;
    if (statement == nullptr) {
        return -E_INVALID_ARGS;
    }
    int errCode = BindSyncDataInCacheMode(statement, dataItem, hashKey, recordVersion);
    if (errCode != E_OK) {
        return errCode;
    }

    errCode = SQLiteUtils::StepWithRetry(statement, isMemDb_);
    if (errCode == SQLiteUtils::MapSQLiteErrno(SQLITE_DONE)) {
        errCode = E_OK;
    }
    return errCode;
}

int SQLiteSingleVerStorageExecutor::PutLocalDataToCacheDB(const LocalDataItem &dataItem) const
{
    sqlite3_stmt *statement = nullptr;
    int errCode = SQLiteUtils::GetStatement(dbHandle_, INSERT_CACHE_LOCAL_SQL, statement);
    if (errCode != E_OK) {
        goto ERROR;
    }

    errCode = BindLocalDataInCacheMode(statement, dataItem);
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

int SQLiteSingleVerStorageExecutor::BindLocalDataInCacheMode(sqlite3_stmt *statement,
    const LocalDataItem &dataItem) const
{
    int errCode = SQLiteUtils::BindBlobToStatement(statement,
        BIND_CACHE_LOCAL_HASH_KEY_INDEX, dataItem.hashKey, false);
    if (errCode != E_OK) {
        LOGE("[SingleVerExe][BindLocalData]Bind hash key error:%d", errCode);
        return errCode;
    }

    // if delete flag is set, just use the hash key instead of the key
    if ((dataItem.flag & DataItem::DELETE_FLAG) == DataItem::DELETE_FLAG) {
        errCode = SQLiteUtils::MapSQLiteErrno(sqlite3_bind_zeroblob(statement, BIND_CACHE_LOCAL_KEY_INDEX, -1));
    } else {
        errCode = SQLiteUtils::BindBlobToStatement(statement, BIND_CACHE_LOCAL_KEY_INDEX, dataItem.key, false);
    }

    if (errCode != E_OK) {
        LOGE("Bind saved sync data key failed:%d", errCode);
        return errCode;
    }

    errCode = SQLiteUtils::BindBlobToStatement(statement, BIND_CACHE_LOCAL_VAL_INDEX, dataItem.value, true);
    if (errCode != E_OK) {
        LOGE("[SingleVerExe][BindLocalData]Bind value error:%d", errCode);
        return errCode;
    }

    errCode = SQLiteUtils::BindInt64ToStatement(statement, BIND_CACHE_LOCAL_TIMESTAMP_INDEX, dataItem.timeStamp);
    if (errCode != E_OK) {
        LOGE("[SingleVerExe][BindLocalData]Bind timestamp error:%d", errCode);
        return errCode;
    }

    errCode = SQLiteUtils::BindInt64ToStatement(statement, BIND_CACHE_LOCAL_FLAG_INDEX,
        static_cast<int64_t>(dataItem.flag));
    if (errCode != E_OK) {
        LOGE("[SingleVerExe][BindLocalData]Bind local data flag failed:%d", errCode);
        return errCode;
    }

    return E_OK;
}

int SQLiteSingleVerStorageExecutor::PutIntoConflictAndCommitForMigrateCache(DataItem &dataItem,
    const DeviceInfo &deviceInfo, NotifyConflictAndObserverData &notify)
{
    int errCode = PrepareForNotifyConflictAndObserver(dataItem, deviceInfo, notify);
    if (errCode != E_OK) {
        errCode = (errCode == -E_NOT_FOUND ? E_OK : errCode);
        if (errCode == -E_IGNOR_DATA) {
            notify.dataStatus.isDefeated = true;
            errCode = E_OK;
        }
        return errCode;
    }

    // If delete data, the key is empty.
    if (isSyncMigrating_ && dataItem.key.empty()) {
        dataItem.key = notify.getData.key;
    }

    PutConflictData(dataItem, notify.getData, deviceInfo, notify.dataStatus, notify.committedData);
    if (notify.dataStatus.isDefeated == true) {
        LOGD("Data status is defeated:%d", errCode);
        return ResetForMigrateCacheData();
    }

    PutIntoCommittedData(dataItem, notify.getData, notify.dataStatus, notify.hashKey, notify.committedData);
    return ResetForMigrateCacheData();
}

int SQLiteSingleVerStorageExecutor::GetMinTimestampInCacheDB(TimeStamp &minStamp) const
{
    if (dbHandle_ == nullptr) {
        return E_OK;
    }
    std::string sql = ((executorState_ == ExecutorState::CACHE_ATTACH_MAIN) ?
        SELECT_NATIVE_MIN_TIMESTAMP_IN_CACHE_SYNC_DATA_SQL :
        SELECT_NATIVE_MIN_TIMESTAMP_IN_CACHE_SYNC_DATA_SQL_FROM_MAINHANDLE);
    sqlite3_stmt *statement = nullptr;
    int errCode = SQLiteUtils::GetStatement(dbHandle_, sql, statement);
    if (errCode != E_OK) {
        goto ERROR;
    }

    errCode = SQLiteUtils::StepWithRetry(statement, isMemDb_);
    if (errCode == SQLiteUtils::MapSQLiteErrno(SQLITE_ROW)) {
        minStamp = static_cast<uint64_t>(sqlite3_column_int64(statement, 0)); // get the first column
        LOGD("Min time stamp in cacheDB is %llu", minStamp);
        errCode = E_OK;
    } else {
        LOGE("GetMinTimestampInCacheDB failed, errCode = %d.", errCode);
    }

ERROR:
    SQLiteUtils::ResetStatement(statement, true, errCode);
    return errCode;
}

int SQLiteSingleVerStorageExecutor::InitMigrateTimeStampOffset()
{
    // Not first migrate, migrateTimeOffset_ has been set.
    if (migrateTimeOffset_ != 0) {
        return E_OK;
    }

    // Get min timestamp of local data in sync_data, cacheDB.
    TimeStamp minTimeInCache = 0;
    int errCode = GetMinTimestampInCacheDB(minTimeInCache);
    if (errCode != E_OK) {
        return errCode;
    }

    // There is no native data in cacheDB, cannot get accurate migrateTimeOffset_ now.
    if (minTimeInCache == 0) {
        migrateTimeOffset_ = -1;
        LOGI("Time offset during migrating is -1.");
        return E_OK;
    }

    // Get max timestamp in mainDB.
    TimeStamp maxTimeInMain = 0;
    InitCurrentMaxStamp(maxTimeInMain);

    // Get timestamp offset between mainDB and cacheDB.
    // The purpose of -1 is to ensure that the first data record in the original cacheDB is 1 greater than
    // the last data record in the original mainDB after the migration.
    migrateTimeOffset_ = minTimeInCache - maxTimeInMain - 1;
    LOGI("Min timestamp in cacheDB is %llu, max timestamp in mainDB is %llu. Time offset during migrating is %lld.",
        minTimeInCache, maxTimeInMain, migrateTimeOffset_);
    return E_OK;
}

int SQLiteSingleVerStorageExecutor::ProcessTimeStampForSyncDataInCacheDB(std::vector<DataItem> &dataItems)
{
    if (dataItems.size() == 0) {
        LOGE("[SQLiteSingleVerStorageExecutor::ProcessTimeStampForCacheDB] Invalid parameter : dataItems.");
        return -E_INVALID_ARGS;
    }

    // Get the offset between the min timestamp in dataitems and max timestamp in mainDB.
    int errCode = InitMigrateTimeStampOffset();
    if (errCode != E_OK) {
        return errCode;
    }

    // Set real timestamp for DataItem in dataItems and get the max timestamp in these dataitems.
    TimeStamp maxTimeInDataItems = 0;
    for (auto &item : dataItems) {
        item.timeStamp -= migrateTimeOffset_;
        maxTimeInDataItems = std::max(maxTimeInDataItems, item.timeStamp);
    }

    // Update max timestamp in mainDB.
    maxTimeStampInMainDB_ = maxTimeInDataItems;
    return E_OK;
}

int SQLiteSingleVerStorageExecutor::GetEntriesForNotifyRemoveDevData(const DataItem &item,
    std::vector<Entry> &entries) const
{
    // When removing device data, key is 'remove', value is device name.
    if (item.key != REMOVE_DEVICE_DATA_KEY) {
        LOGE("Invalid key. Can not notify remove device data.");
        return -E_INVALID_ARGS;
    }
    if ((item.flag & DataItem::REMOVE_DEVICE_DATA_NOTIFY_FLAG) == 0) {
        LOGI("No need to notify remove device data.");
        return E_OK;
    }
    entries.clear();
    std::string dev;
    DBCommon::VectorToString(item.value, dev);
    return GetAllSyncedEntries(dev, entries);
}

int SQLiteSingleVerStorageExecutor::NotifyForMigrateCacheDB(std::vector<DataItem> &dataItems,
    NotifyMigrateSyncData &syncData)
{
    auto &isRemote = syncData.isRemote;
    auto &isRemoveDeviceData = syncData.isRemoveDeviceData;
    auto &committedData = syncData.committedData;

    isRemote = ((dataItems[0].flag & DataItem::LOCAL_FLAG) == 0);
    isRemoveDeviceData = (dataItems[0].flag & DataItem::REMOVE_DEVICE_DATA_FLAG) != 0 ||
                         (dataItems[0].flag & DataItem::REMOVE_DEVICE_DATA_NOTIFY_FLAG) != 0;

    for (auto iter = dataItems.begin(); iter != dataItems.end();) {
        auto &item = *iter;
        // Remove device data owns one version itself.
        // Get entry here. Prepare notify data in storageEngine.
        if (isRemoveDeviceData) {
            return GetEntriesForNotifyRemoveDevData(item, syncData.entries);
        }

        // Put or delete. Prepare notify data here.
        DeviceInfo deviceInfo = {item.dev.empty(), item.dev};
        NotifyConflictAndObserverData notify;
        notify.committedData = committedData;
        int errCode = PutIntoConflictAndCommitForMigrateCache(item, deviceInfo, notify);
        if (errCode != E_OK) {
            ResetForMigrateCacheData();
            LOGE("PutIntoConflictAndCommitForMigrateCache failed, errCode = %d", errCode);
            return errCode;
        }
        errCode = ResetForMigrateCacheData();
        if (errCode != E_OK) {
            LOGE("ResetForMigrateCacheData failed, errCode = %d", errCode);
            return errCode;
        }
        // after solving conflict, the item should not be saved into mainDB
        if (notify.dataStatus.isDefeated == true) {
            iter = dataItems.erase(iter);
            continue;
        }
        iter++;
    }
    return E_OK;
}

int SQLiteSingleVerStorageExecutor::InitMigrateData()
{
    // Sync_data already in migrating. Need not to init data.
    if (isSyncMigrating_) {
        return E_OK;
    }
    ClearMigrateData();
    std::string querySQL;
    std::string putSQL;
    if (executorState_ == ExecutorState::MAIN_ATTACH_CACHE) {
        querySQL = SELECT_SYNC_HASH_SQL;
        putSQL = MIGRATE_PUT_DATA_TO_MAINDB_FROM_MAINHANDLE;
    } else if (executorState_ == ExecutorState::CACHE_ATTACH_MAIN) {
        querySQL = SELECT_MAIN_SYNC_HASH_SQL_FROM_CACHEHANDLE;
        putSQL = MIGRATE_PUT_DATA_TO_MAINDB_FROM_CACHEHANDLE;
    } else {
        LOGE("[InitMigrateData] executor in an error state[%d]!", executorState_);
        return -E_INVALID_DB;
    }
    int errCode = PrepareForSavingData(querySQL, putSQL, migrateSyncStatements_);
    if (errCode != E_OK) {
        LOGE("Prepare migrateSyncStatements_ fail, errCode = %d", errCode);
        return errCode;
    }
    isSyncMigrating_ = true;
    return errCode;
}

void SQLiteSingleVerStorageExecutor::ClearMigrateData()
{
    // Reset data.
    migrateTimeOffset_ = 0;
    maxTimeStampInMainDB_ = 0;

    // Reset statement.
    int errCode = E_OK;
    SQLiteUtils::ResetStatement(migrateSyncStatements_.putStatement, true, errCode);
    if (errCode != E_OK) {
        LOGE("Reset migrateSyncStatements_.putStatement fail, errCode = %d", errCode);
    }
    errCode = E_OK;
    SQLiteUtils::ResetStatement(migrateSyncStatements_.queryStatement, true, errCode);
    if (errCode != E_OK) {
        LOGE("Reset migrateSyncStatements_.queryStatement fail, errCode = %d", errCode);
    }

    isSyncMigrating_ = false;
}

int SQLiteSingleVerStorageExecutor::GetMaxTimeStampDuringMigrating(TimeStamp &maxTimeStamp) const
{
    if (maxTimeStampInMainDB_ == 0) {
        return -E_NOT_INIT;
    }
    maxTimeStamp = maxTimeStampInMainDB_;
    return E_OK;
}
} // namespace DistributedDB
