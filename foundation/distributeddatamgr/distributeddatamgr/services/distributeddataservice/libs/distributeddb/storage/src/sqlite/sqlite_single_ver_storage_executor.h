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

#ifndef SQLITE_SINGLE_VER_STORAGE_EXECUTOR_H
#define SQLITE_SINGLE_VER_STORAGE_EXECUTOR_H

#include "macro_utils.h"
#include "db_types.h"
#include "query_object.h"
#include "sqlite_utils.h"
#include "sqlite_storage_executor.h"
#include "single_ver_natural_store_commit_notify_data.h"

namespace DistributedDB {
enum class SingleVerDataType {
    META_TYPE,
    LOCAL_TYPE,
    SYNC_TYPE,
};

enum class DataStatus {
    NOEXISTED,
    DELETED,
    EXISTED,
};

enum class ExecutorState {
    INVALID = -1,
    MAINDB,
    CACHEDB,
    MAIN_ATTACH_CACHE, // After process crash and cacheDb existed
    CACHE_ATTACH_MAIN, // while cacheDb migrating to mainDb
};

struct DataOperStatus {
    DataStatus preStatus = DataStatus::NOEXISTED;
    bool isDeleted = false;
    bool isDefeated = false; // whether the put data is defeated.
};

struct SingleVerRecord {
    Key key;
    Value value;
    TimeStamp timeStamp = 0;
    uint64_t flag = 0;
    std::string device;
    std::string origDevice;
    Key hashKey;
    TimeStamp writeTimeStamp = 0;
};

struct DeviceInfo {
    bool isLocal = false;
    std::string deviceName;
};

struct LocalDataItem {
    Key key;
    Value value;
    TimeStamp timeStamp = 0;
    Key hashKey;
    uint64_t flag = 0;
};

struct NotifyConflictAndObserverData {
    SingleVerNaturalStoreCommitNotifyData *committedData = nullptr;
    DataItem getData;
    Key hashKey;
    DataOperStatus dataStatus;
};

struct NotifyMigrateSyncData {
    bool isRemote = false;
    bool isRemoveDeviceData = false;
    SingleVerNaturalStoreCommitNotifyData *committedData = nullptr;
    std::vector<Entry> entries{};
};

struct SyncDataDevices {
    std::string origDev;
    std::string dev;
};

class SQLiteSingleVerStorageExecutor : public SQLiteStorageExecutor {
public:
    SQLiteSingleVerStorageExecutor(sqlite3 *dbHandle, bool writable, bool isMemDb);
    SQLiteSingleVerStorageExecutor(sqlite3 *dbHandle, bool writable, bool isMemDb, ExecutorState executorState);
    ~SQLiteSingleVerStorageExecutor() override;

    // Delete the copy and assign constructors
    DISABLE_COPY_ASSIGN_MOVE(SQLiteSingleVerStorageExecutor);

    // Get the Kv data according the type(sync, meta, local data).
    int GetKvData(SingleVerDataType type, const Key &key, Value &value, TimeStamp &timeStamp) const;

    // Get the sync data record by hash key.
    int GetKvDataByHashKey(const Key &hashKey, SingleVerRecord &result) const;

    // Put the Kv data according the type(meta and the local data).
    int PutKvData(SingleVerDataType type, const Key &key, const Value &value,
        TimeStamp timestamp, SingleVerNaturalStoreCommitNotifyData *committedData);

    int GetEntries(SingleVerDataType type, const Key &keyPrefix, std::vector<Entry> &entries) const;

    int GetEntries(QueryObject &queryObj, std::vector<Entry> &entries) const;

    int GetCount(QueryObject &queryObj, int &count) const;

    // Get all the meta keys.
    int GetAllMetaKeys(std::vector<Key> &keys) const;

    int GetAllSyncedEntries(const std::string &deviceName, std::vector<Entry> &entries) const;

    int SaveSyncDataItem(DataItem &dataItem, const DeviceInfo &deviceInfo,
        TimeStamp &maxStamp, SingleVerNaturalStoreCommitNotifyData *committedData);

    int DeleteLocalKvData(const Key &key, SingleVerNaturalStoreCommitNotifyData *committedData, Value &value,
        TimeStamp &timeStamp);

    int RemoveDeviceData(const std::string &deviceName);

    int RemoveDeviceDataInCacheMode(const std::string &deviceName, bool isNeedNotify, uint64_t recordVersion) const;

    void InitCurrentMaxStamp(TimeStamp &maxStamp);

    void ReleaseContinueStatement();

    int GetSyncDataByTimestamp(std::vector<DataItem> &dataItems, size_t appenedLength, TimeStamp begin,
        TimeStamp end, const DataSizeSpecInfo &dataSizeInfo) const;

    int GetDeviceIdentifier(PragmaEntryDeviceIdentifier *identifier);

    int OpenResultSet(const Key &keyPrefix, int &count);

    int OpenResultSet(QueryObject &queryObj, int &count);

    int OpenResultSetForCacheRowIdMode(const Key &keyPrefix, std::vector<int64_t> &rowIdCache,
        uint32_t cacheLimit, int &count);

    int OpenResultSetForCacheRowIdMode(QueryObject &queryObj, std::vector<int64_t> &rowIdCache,
        uint32_t cacheLimit, int &count);

    int ReloadResultSet(const Key &keyPrefix);

    int ReloadResultSet(QueryObject &queryObj);

    int ReloadResultSetForCacheRowIdMode(const Key &keyPrefix, std::vector<int64_t> &rowIdCache,
        uint32_t cacheLimit, uint32_t cacheStartPos);

    int ReloadResultSetForCacheRowIdMode(QueryObject &queryObj, std::vector<int64_t> &rowIdCache,
        uint32_t cacheLimit, uint32_t cacheStartPos);

    int GetNextEntryFromResultSet(Key &key, Value &value, bool isCopy);

    int GetEntryByRowId(int64_t rowId, Entry &entry);

    void CloseResultSet();

    int StartTransaction(TransactType type);

    int Commit();

    int Rollback();

    bool CheckIfKeyExisted(const Key &key, bool isLocal, Value &value, TimeStamp &timeStamp) const;

    int PrepareForSavingData(SingleVerDataType type);

    int ResetForSavingData(SingleVerDataType type);

    int Reset() override;

    int UpdateLocalDataTimestamp(TimeStamp timestamp);

    void SetAttachMetaMode(bool attachMetaMode);

    int PutLocalDataToCacheDB(const LocalDataItem &dataItem) const;

    int SaveSyncDataItemInCacheMode(DataItem &dataItem,
        const DeviceInfo &deviceInfo, TimeStamp &maxStamp, uint64_t recordVersion);

    int PrepareForSavingCacheData(SingleVerDataType type);
    int ResetForSavingCacheData(SingleVerDataType type);

    int MigrateLocalData();

    int MigrateSyncDataByVersion(uint64_t recordVer, NotifyMigrateSyncData &syncData,
        std::vector<DataItem> &dataItems);
    int GetMinVersionCacheData(std::vector<DataItem> &dataItems, uint64_t &maxVerIncurCacheDb) const;

    int GetMaxVersionIncacheDb(uint64_t &maxVersion) const;
    int AttachMainDbAndCacheDb(CipherType type, const CipherPassword &passwd,
        const std::string &attachDbAbsPath, EngineState engineState);

    // Clear migrating data.
    void ClearMigrateData();

    // Get current max timestamp.
    int GetMaxTimeStampDuringMigrating(TimeStamp &maxTimeStamp) const;

    void SetConflictResolvePolicy(int policy);

private:
    struct SaveRecordStatements {
        sqlite3_stmt *queryStatement = nullptr;
        sqlite3_stmt *putStatement = nullptr;
    };

    void PutIntoCommittedData(const DataItem &itemPut, const DataItem &itemGet, const DataOperStatus &status,
        const Key &hashKey, SingleVerNaturalStoreCommitNotifyData *committedData);

    static int BindSavedSyncData(sqlite3_stmt *statement, const DataItem &dataItem, const Key &hashKey,
        const SyncDataDevices &devices, int beginIndex = 0);

    static int BindDevForSavedSyncData(sqlite3_stmt *statement, const DataItem &dataItem, const std::string &origDev,
        const std::string &deviceName, int beginIndex = 0);

    static int GetDataItemForSync(sqlite3_stmt *statement, DataItem &dataItem);

    static size_t GetDataItemSerialSize(const DataItem &item, size_t appendLen);

    static void PutConflictData(const DataItem &itemPut, const DataItem &itemGet, const DeviceInfo &deviceInfo,
        const DataOperStatus &dataStatus, SingleVerNaturalStoreCommitNotifyData *commitData);

    static DataOperStatus JudgeSyncSaveType(DataItem &dataItem, const DataItem &itemGet,
        const std::string &devName, bool isHashKeyExisted);

    static std::string GetOriginDevName(const DataItem &dataItem, const std::string &origDevGet);

    int GetSyncDataItemPre(const DataItem &itemPut, DataItem &itemGet, Key &hashKey) const;

    int GetSyncDataItemExt(const DataItem &dataItem, DataItem &itemGet, const DataOperStatus &dataStatus) const;

    int GetSyncDataPreByHashKey(const Key &hashKey, DataItem &itemGet) const;

    int PrepareForSyncDataByTime(TimeStamp begin, TimeStamp end, sqlite3_stmt *&statement) const;

    int StepForResultEntries(sqlite3_stmt *statement, std::vector<Entry> &entries) const;

    int InitResultSet(const Key &keyPrefix, sqlite3_stmt *&countStmt);

    int InitResultSetCount(QueryObject &queryObj, sqlite3_stmt *&countStmt);

    int InitResultSetContent(QueryObject &queryObj);

    int InitResultSet(QueryObject &queryObj, sqlite3_stmt *&countStmt);

    int GetAllKeys(sqlite3_stmt *statement, std::vector<Key> &keys) const;

    int GetAllEntries(sqlite3_stmt *statement, std::vector<Entry> &entries) const;

    int BindPutKvData(sqlite3_stmt *statement, const Key &key, const Value &value, TimeStamp timestamp,
        SingleVerDataType type);

    int SaveSyncDataToDatabase(const DataItem &dataItem, const Key &hashKey, const std::string &origDev,
        const std::string &deviceName);

    int SaveKvData(SingleVerDataType type, const Key &key, const Value &value, TimeStamp timestamp);

    int DeleteLocalDataInner(SingleVerNaturalStoreCommitNotifyData *committedData,
        const Key &key, const Value &value);

    int PrepareForSavingData(const std::string &readSql, const std::string &writeSql,
        SaveRecordStatements &statements) const;

    int OpenResultSetForCacheRowIdModeCommon(std::vector<int64_t> &rowIdCache, uint32_t cacheLimit, int &count);

    int ResultSetLoadRowIdCache(std::vector<int64_t> &rowIdCache, uint32_t cacheLimit,
        uint32_t cacheStartPos, int &count);

    void FinalizeAllStatements();
    int ResetSaveSyncStatements(int errCode);

    int BindSyncDataInCacheMode(sqlite3_stmt *statement,
        const DataItem &dataItem, const Key &hashKey, uint64_t recordVersion) const;

    int BindPrimaryKeySyncDataInCacheMode(
        sqlite3_stmt *statement, const Key &hashKey, uint64_t recordVersion) const;

    int BindTimeStampSyncDataInCacheMode(sqlite3_stmt *statement, const DataItem &dataItem) const;

    int BindDevSyncDataInCacheMode(sqlite3_stmt *statement,
        const std::string &origDev, const std::string &deviceName) const;

    int SaveSyncDataToCacheDatabase(const DataItem &dataItem, const Key &hashKey, uint64_t recordVersion) const;

    int GetOneRawDataItem(sqlite3_stmt *statement, DataItem &dataItem,
        uint64_t &verInCurCacheDb, bool isCacheDb) const;
    int GetAllDataItems(sqlite3_stmt *statement, std::vector<DataItem> &dataItems,
        uint64_t &verInCurCacheDb, bool isCacheDb) const;
    int DelCacheDbDataByVersion(uint64_t version) const;

    // use for migrating data
    int BindLocalDataInCacheMode(sqlite3_stmt *statement, const LocalDataItem &dataItem) const;
    int PutMigratingDataToMain(const std::vector<DataItem> &dataItems) const;

    // Process timestamp for syncdata in cacheDB when migrating.
    int ProcessTimeStampForSyncDataInCacheDB(std::vector<DataItem> &dataItems);
    // Get migrateTimeOffset_.
    int InitMigrateTimeStampOffset();
    // Get min timestamp of local data in sync_data, cacheDB.
    int GetMinTimestampInCacheDB(TimeStamp &minStamp) const;

    // Prepare conflict notify and commit notify data.
    int PrepareForNotifyConflictAndObserver(DataItem &dataItem, const DeviceInfo &deviceInfo,
        NotifyConflictAndObserverData &notify);
    // Put observer and conflict data into commit notify when migrating cacheDB.
    int PutIntoConflictAndCommitForMigrateCache(DataItem &dataItem, const DeviceInfo &deviceInfo,
        NotifyConflictAndObserverData &notify);
    // Notify when migrating cacheDB.
    int NotifyForMigrateCacheDB(std::vector<DataItem> &dataItems, NotifyMigrateSyncData &syncData);
    int GetEntriesForNotifyRemoveDevData(const DataItem &item, std::vector<Entry> &entries) const;

    // Reset migrateSyncStatements_.
    int ResetForMigrateCacheData();
    // Init migrating data.
    int InitMigrateData();

    int MigrateRmDevData(const DataItem &dataItem) const;
    int VacuumLocalData() const;

    sqlite3_stmt *getSyncStatement_;
    sqlite3_stmt *getResultRowIdStatement_;
    sqlite3_stmt *getResultEntryStatement_;
    SaveRecordStatements saveSyncStatements_;
    SaveRecordStatements saveLocalStatements_;
    // Used for migrating sync_data.
    SaveRecordStatements migrateSyncStatements_;
    bool isTransactionOpen_;
    bool attachMetaMode_; // true for attach meta mode
    ExecutorState executorState_;
    // Max timestamp in mainDB. Used for migrating.
    TimeStamp maxTimeStampInMainDB_;
    // The offset between min timestamp in cacheDB and max timestamp in mainDB. Used for migrating.
    TimeOffset migrateTimeOffset_;
    // Migrating sync flag. When the flag is true, mainDB and cacheDB are attached, migrateSyncStatements_ is set,
    // maxTimeStampInMainDB_ and migrateTimeOffset_ is meaningful.
    bool isSyncMigrating_;
    int conflictResolvePolicy_;
};
} // namespace DistributedDB

#endif // SQLITE_SINGLE_VER_STORAGE_EXECUTOR_H
