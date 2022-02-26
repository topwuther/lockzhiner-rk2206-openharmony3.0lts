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

#ifndef SQLITE_SINGLE_VER_STORAGE_ENGINE_H
#define SQLITE_SINGLE_VER_STORAGE_ENGINE_H

#include "macro_utils.h"
#include "sqlite_storage_engine.h"
#include "sqlite_single_ver_storage_executor.h"

namespace DistributedDB {
enum SQLiteGeneralNSNotificationEventType {
    SQLITE_GENERAL_NS_PUT_EVENT = 0x01,
    SQLITE_GENERAL_NS_SYNC_EVENT = 0x02,
    SQLITE_GENERAL_NS_LOCAL_PUT_EVENT = 0x04,
    SQLITE_GENERAL_CONFLICT_EVENT = 0x08, // Conflict event
    SQLITE_GENERAL_FINISH_MIGRATE_EVENT = 0x10, // Only trigger sync event
};
enum SQLiteGeneralNSConflictType {
    SQLITE_GENERAL_NS_FOREIGN_KEY_ONLY = 0x01, // sync conflict for same origin dev
    SQLITE_GENERAL_NS_FOREIGN_KEY_ORIG = 0x02, // sync conflict for different origin dev
    SQLITE_GENERAL_NS_NATIVE_ALL = 0x0c, // native conflict.
};
class SQLiteSingleVerStorageEngine : public SQLiteStorageEngine {
public:
    SQLiteSingleVerStorageEngine();
    ~SQLiteSingleVerStorageEngine() override;

    // Delete the copy and assign constructors
    DISABLE_COPY_ASSIGN_MOVE(SQLiteSingleVerStorageEngine);

    void IncreaseCacheRecordVersion() override;
    uint64_t GetCacheRecordVersion() const override;
    uint64_t GetAndIncreaseCacheRecordVersion() override;

    int ExecuteMigrate() override;
    bool IsEngineCorrupted() const override;

    const SecurityOption &GetSecurityOption() const
    {
        return option_.securityOpt;
    }

    void SetNeedUpdateSecOption(bool flag)
    {
        isNeedUpdateSecOpt_ = flag;
    }

protected:
    StorageExecutor *NewSQLiteStorageExecutor(sqlite3 *dbHandle, bool isWrite, bool isMemDb) override;

    int Upgrade(sqlite3 *db) override;

    int CreateNewExecutor(bool isWrite, StorageExecutor *&handle) override;

private:
    // For executor.
    int PreCreateExecutor(bool isWrite);
    int EndCreateExecutor(bool isWrite);
    int ReInit() override;
    int ReleaseExecutor(SQLiteSingleVerStorageExecutor *&handle);
    int ReleaseHandleTransiently(SQLiteSingleVerStorageExecutor *&handle, uint64_t idleTime);

    // For migrate.
    int MigrateLocalData(SQLiteSingleVerStorageExecutor *handle) const;
    int MigrateSyncDataByVersion(SQLiteSingleVerStorageExecutor *&handle,
        NotifyMigrateSyncData &syncData, uint64_t &curMigrateVer);
    int MigrateSyncData(SQLiteSingleVerStorageExecutor *&handle, bool &isNeedTriggerSync);
    int FinishMigrateData(SQLiteSingleVerStorageExecutor *&handle, EngineState stateBeforeMigrate);
    int InitExecuteMigrate(SQLiteSingleVerStorageExecutor *handle, EngineState preMigrateState);
    void EndMigrate(SQLiteSingleVerStorageExecutor *&handle, EngineState stateBeforeMigrate, int errCode,
        bool isNeedTriggerSync);
    void ResetCacheRecordVersion();
    void SetMaxTimeStamp(TimeStamp maxTimeStamp) const;
    int EraseDeviceWaterMark(SQLiteSingleVerStorageExecutor *&handle, const std::vector<DataItem> &dataItems);

    // For db.
    int TryToOpenMainDatabase(sqlite3 *&db);
    int GetCacheDbHandle(sqlite3 *&db);
    int GetDbHandle(bool isWrite, const SecurityOption &secOpt, sqlite3 *&dbHandle);
    int AttachMetaDatabase(sqlite3 *dbHandle, const OpenDbProperties &option) const;
    int AttachMainDbAndCacheDb(SQLiteSingleVerStorageExecutor *handle, EngineState stateBeforeMigrate);
    int AttachMainDbAndCacheDb(sqlite3 *db, EngineState stateBeforeMigrate) const;
    void RegisterFunctionIfNeed(sqlite3 *dbHandle) const;
    int TryAttachMetaDb(sqlite3 *&dbHandle, bool &isAttachMeta);

    // For secOpt.
    int CreateNewDirsAndSetSecOpt() const;
    int CheckDatabaseSecOpt(const SecurityOption &secOption) const;
    int GetExistedSecOption(SecurityOption &secOption) const;

    void ClearCorruptedFlag() override;

    // For commit notify.
    void CommitAndReleaseNotifyData(SingleVerNaturalStoreCommitNotifyData *&committedData, int eventType) const;
    void InitConflictNotifiedFlag(SingleVerNaturalStoreCommitNotifyData *&committedData) const;
    void CommitNotifyForMigrateCache(NotifyMigrateSyncData &syncData) const;

    mutable std::mutex migrateLock_;
    std::atomic<uint64_t> cacheRecordVersion_;
    ExecutorState executorState_;
    bool isCorrupted_;
    bool isNeedUpdateSecOpt_; // update the option_
};
} // namespace DistributedDB

#endif // SQLITE_SINGLE_VER_STORAGE_ENGINE_H
