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

#include "sqlite_single_ver_storage_engine.h"

#include <memory>

#include "db_errno.h"
#include "log_print.h"
#include "db_constant.h"
#include "sqlite_single_ver_database_upgrader.h"
#include "sqlite_single_ver_natural_store.h"
#include "sqlite_single_ver_schema_database_upgrader.h"
#include "platform_specific.h"
#include "runtime_context.h"
#include "db_common.h"
#include "kvdb_manager.h"
#include "param_check_utils.h"

namespace DistributedDB {
namespace {
    const uint64_t CACHE_RECORD_DEFAULT_VERSION = 1;
    int GetPathSecurityOption(const std::string &filePath, SecurityOption &secOpt)
    {
        return RuntimeContext::GetInstance()->GetSecurityOption(filePath, secOpt);
    }

    enum class DbType {
        MAIN,
        META,
        CACHE
    };

    std::string GetDbDir(const std::string &subDir, DbType type)
    {
        static const std::map<DbType, std::string> dbDirDic {
            { DbType::MAIN, DBConstant::MAINDB_DIR },
            { DbType::META, DBConstant::METADB_DIR },
            { DbType::CACHE, DBConstant::CACHEDB_DIR },
        }; // for ensure static compilation order

        if (dbDirDic.find(type) == dbDirDic.end()) {
            return std::string();
        }
        return subDir + "/" + dbDirDic.at(type);
    }
} // namespace

SQLiteSingleVerStorageEngine::SQLiteSingleVerStorageEngine()
    : cacheRecordVersion_(CACHE_RECORD_DEFAULT_VERSION),
      executorState_(ExecutorState::INVALID),
      isCorrupted_(false),
      isNeedUpdateSecOpt_(false)
{}

SQLiteSingleVerStorageEngine::~SQLiteSingleVerStorageEngine()
{}

int SQLiteSingleVerStorageEngine::MigrateLocalData(SQLiteSingleVerStorageExecutor *handle) const
{
    return handle->MigrateLocalData();
}

int SQLiteSingleVerStorageEngine::EraseDeviceWaterMark(SQLiteSingleVerStorageExecutor *&handle,
    const std::vector<DataItem> &dataItems)
{
    int errCode = E_OK;
    for (const auto &dataItem : dataItems) {
        if ((dataItem.flag & DataItem::REMOVE_DEVICE_DATA_FLAG) == DataItem::REMOVE_DEVICE_DATA_FLAG ||
            (dataItem.flag & DataItem::REMOVE_DEVICE_DATA_NOTIFY_FLAG) == DataItem::REMOVE_DEVICE_DATA_NOTIFY_FLAG) {
            auto kvdbManager = KvDBManager::GetInstance();
            if (kvdbManager == nullptr) {
                return -E_INVALID_DB;
            }

            // sync module will use handle to fix water mark, if fix fail then migrate fail, not need hold write handle
            errCode = ReleaseExecutor(handle);
            if (errCode != E_OK) {
                LOGE("release executor for erase water mark! errCode = [%d]", errCode);
                return errCode;
            }

            auto identifier = GetIdentifier();
            auto kvdb = kvdbManager->FindKvDB(identifier);
            if (kvdb == nullptr) {
                LOGE("[SingleVerEngine::EraseWaterMark] kvdb is null.");
                return -E_INVALID_DB;
            }

            auto kvStore = static_cast<SQLiteSingleVerNaturalStore *>(kvdb);
            errCode = kvStore->EraseDeviceWaterMark(dataItem.dev, false);
            RefObject::DecObjRef(kvdb);
            if (errCode != E_OK) {
                LOGE("EraseDeviceWaterMark failed when migrating, errCode = [%d]", errCode);
                return errCode;
            }

            handle = static_cast<SQLiteSingleVerStorageExecutor *>(FindExecutor(true, OperatePerm::NORMAL_PERM,
                errCode));
            if (errCode != E_OK) {
                LOGE("Migrate sync data fail, Can not get available executor, errCode = [%d]", errCode);
                return errCode;
            }
        }
    }
    return errCode;
}

int SQLiteSingleVerStorageEngine::MigrateSyncDataByVersion(SQLiteSingleVerStorageExecutor *&handle,
    NotifyMigrateSyncData &syncData, uint64_t &curMigrateVer)
{
    if (syncData.committedData == nullptr) {
        syncData.committedData = new (std::nothrow) SingleVerNaturalStoreCommitNotifyData();
        if (syncData.committedData == nullptr) {
            LOGE("[SQLiteSingleVerStorageEngine::MigrateSyncData] committedData is null.");
            return -E_OUT_OF_MEMORY;
        }
    }
    InitConflictNotifiedFlag(syncData.committedData);

    std::vector<DataItem> dataItems;
    uint64_t minVerIncurCacheDb = 0;
    int errCode = handle->GetMinVersionCacheData(dataItems, minVerIncurCacheDb);
    if (errCode != E_OK) {
        LOGE("[MigrateSyncDataByVersion]Fail to get cur data in cache! err[%d]", errCode);
        return errCode;
    }

    if (minVerIncurCacheDb == 0) { // min version in cache db is 1
        ++curMigrateVer;
        return E_OK;
    }

    if (minVerIncurCacheDb != curMigrateVer) { // double check for latest version is migrated
        curMigrateVer = minVerIncurCacheDb;
    }

    // Call the syncer module to erase the water mark.
    errCode = EraseDeviceWaterMark(handle, dataItems);
    if (errCode != E_OK) {
        LOGE("[MigrateSyncData] Erase water mark failed:%d", errCode);
        return errCode;
    }

    // next version need process
    LOGD("MigrateVer[%llu], minVer[%llu] maxVer[%llu]", curMigrateVer, minVerIncurCacheDb, GetCacheRecordVersion());
    errCode = handle->MigrateSyncDataByVersion(curMigrateVer++, syncData, dataItems);
    if (errCode != E_OK) {
        LOGE("Migrate sync data fail and rollback, errCode = [%d]", errCode);
        return errCode;
    }

    CommitNotifyForMigrateCache(syncData);

    TimeStamp timestamp = 0;
    errCode = handle->GetMaxTimeStampDuringMigrating(timestamp);
    if (errCode == E_OK) {
        SetMaxTimeStamp(timestamp);
    }

    errCode = ReleaseHandleTransiently(handle, 2ull); // temporary release handle 2ms
    if (errCode != E_OK) {
        return errCode;
    }

    return E_OK;
}

// Temporary release handle for idleTime ms, avoid long-term blocking
int SQLiteSingleVerStorageEngine::ReleaseHandleTransiently(SQLiteSingleVerStorageExecutor *&handle, uint64_t idleTime)
{
    int errCode = ReleaseExecutor(handle);
    if (errCode != E_OK) {
        LOGE("release executor for reopen database! errCode = [%d]", errCode);
        return errCode;
    }

    std::this_thread::sleep_for(std::chrono::milliseconds(idleTime)); // Wait 2 ms to free this handle for put data
    handle = static_cast<SQLiteSingleVerStorageExecutor *>(FindExecutor(true, OperatePerm::NORMAL_PERM, errCode));
    if (errCode != E_OK) {
        LOGE("Migrate sync data fail, Can not get available executor, errCode = [%d]", errCode);
        return errCode;
    }
    return errCode;
}

int SQLiteSingleVerStorageEngine::MigrateSyncData(SQLiteSingleVerStorageExecutor *&handle, bool &isNeedTriggerSync)
{
    int errCode = E_OK;
    if (handle == nullptr) {
        handle = static_cast<SQLiteSingleVerStorageExecutor *>(FindExecutor(true, OperatePerm::NORMAL_PERM, errCode));
        if (errCode != E_OK) {
            LOGE("Migrate sync data fail, Can not get available executor, errCode = [%d]", errCode);
            return errCode;
        }
    }

    LOGD("Begin migrate sync data, need migrate version[%llu]", GetCacheRecordVersion() - 1);
    uint64_t curMigrateVer = 0; // The migration process is asynchronous and continuous
    NotifyMigrateSyncData syncData;
    // cache atomic version represents version of cacheDb input next time
    while (curMigrateVer < GetCacheRecordVersion()) {
        errCode = MigrateSyncDataByVersion(handle, syncData, curMigrateVer);
        if (errCode != E_OK) {
            LOGE("Migrate version[%llu] failed! errCode = [%d]", curMigrateVer, errCode);
            break;
        }
        if (!syncData.isRemote) {
            isNeedTriggerSync = true;
        }
    }
    if (syncData.committedData != nullptr) {
        RefObject::DecObjRef(syncData.committedData);
        syncData.committedData = nullptr;
    }
    // When finished Migrating sync data, will fix engine state
    return errCode;
}

int SQLiteSingleVerStorageEngine::AttachMainDbAndCacheDb(SQLiteSingleVerStorageExecutor *handle,
    EngineState stateBeforeMigrate)
{
    LOGD("Begin attach main db and cache db by executor!");
    // Judge the file corresponding to db by the engine status and attach it to another file
    int errCode = E_OK;
    std::string attachAbsPath;
    if (stateBeforeMigrate == EngineState::MAINDB) {
        attachAbsPath = GetDbDir(option_.subdir, DbType::CACHE) + "/" + DBConstant::SINGLE_VER_CACHE_STORE +
            DBConstant::SQLITE_DB_EXTENSION;
        errCode = handle->AttachMainDbAndCacheDb(option_.cipherType, option_.passwd, attachAbsPath, stateBeforeMigrate);
    } else if (stateBeforeMigrate == EngineState::CACHEDB) {
        attachAbsPath = GetDbDir(option_.subdir, DbType::MAIN) + "/" + DBConstant::SINGLE_VER_DATA_STORE +
        DBConstant::SQLITE_DB_EXTENSION;
        errCode = handle->AttachMainDbAndCacheDb(option_.cipherType, option_.passwd, attachAbsPath, stateBeforeMigrate);
    } else {
        return -E_NOT_SUPPORT;
    }
    if (errCode != E_OK) {
        LOGE("Attached database failed, errCode = [%d] engine state = [%d]", errCode, stateBeforeMigrate);
        return errCode;
    }

    uint64_t maxVersion = 0;
    errCode = handle->GetMaxVersionIncacheDb(maxVersion);
    if (errCode != E_OK || maxVersion < CACHE_RECORD_DEFAULT_VERSION) {
        maxVersion = CACHE_RECORD_DEFAULT_VERSION;
    }

    (void)cacheRecordVersion_.store(maxVersion + 1, std::memory_order_seq_cst);
    return errCode;
}

int SQLiteSingleVerStorageEngine::AttachMainDbAndCacheDb(sqlite3 *dbHandle, EngineState stateBeforeMigrate) const
{
    LOGD("Begin attach main db and cache db by sqlite handle!");
    // Judge the file corresponding to db by the engine status and attach it to another file
    int errCode = E_OK;
    std::string attachAbsPath;
    if (stateBeforeMigrate == EngineState::MAINDB) {
        attachAbsPath = GetDbDir(option_.subdir, DbType::CACHE) + "/" + DBConstant::SINGLE_VER_CACHE_STORE +
            DBConstant::SQLITE_DB_EXTENSION;
        errCode = SQLiteUtils::AttachNewDatabase(dbHandle, option_.cipherType, option_.passwd, attachAbsPath, "cache");
    } else if (stateBeforeMigrate == EngineState::CACHEDB) {
        attachAbsPath = GetDbDir(option_.subdir, DbType::MAIN) + "/" + DBConstant::SINGLE_VER_DATA_STORE +
            DBConstant::SQLITE_DB_EXTENSION;
        errCode = SQLiteUtils::AttachNewDatabase(dbHandle, option_.cipherType, option_.passwd, attachAbsPath, "maindb");
    } else {
        return -E_NOT_SUPPORT;
    }
    if (errCode != E_OK) {
        LOGE("Attached database failed, errCode = [%d] engine state = [%d]", errCode, stateBeforeMigrate);
        return errCode;
    }

    return errCode;
}

int SQLiteSingleVerStorageEngine::ReInit()
{
    return Init();
}

int SQLiteSingleVerStorageEngine::ReleaseExecutor(SQLiteSingleVerStorageExecutor *&handle)
{
    if (handle == nullptr) {
        return E_OK;
    }
    StorageExecutor *databaseHandle = handle;
    isCorrupted_ = isCorrupted_ || handle->GetCorruptedStatus();
    Recycle(databaseHandle);
    handle = nullptr;
    if (isCorrupted_) {
        LOGE("Database is corrupted!");
        return -E_INVALID_PASSWD_OR_CORRUPTED_DB; // Externally imperceptible, used to terminate migration
    }
    return E_OK;
}

int SQLiteSingleVerStorageEngine::FinishMigrateData(SQLiteSingleVerStorageExecutor *&handle,
    EngineState stateBeforeMigrate)
{
    LOGD("Begin to finish migrate and reinit db state!");
    int errCode;
    if (handle == nullptr) {
        return -E_INVALID_ARGS;
    }

    if (stateBeforeMigrate == EngineState::MAINDB) {
        sqlite3 *dbHandle = nullptr;
        errCode = handle->GetDbHandle(dbHandle); // use executor get sqlite3 handle to operating database
        if (errCode != E_OK) {
            LOGE("Get Db handle failed! errCode = [%d]", errCode);
            return errCode;
        }

        errCode = SQLiteUtils::ExecuteRawSQL(dbHandle, "DETACH 'cache'");
        if (errCode != E_OK) {
            LOGE("Execute the SQLite detach failed:%d", errCode);
            return errCode;
        }
        // delete cachedb
        errCode = DBCommon::RemoveAllFilesOfDirectory(GetDbDir(option_.subdir, DbType::CACHE), false);
        if (errCode != E_OK) {
            LOGE("Remove files of cache database after detach:%d", errCode);
        }

        SetEngineState(EngineState::MAINDB);
        return errCode;
    }

    errCode = ReleaseExecutor(handle);
    if (errCode != E_OK) {
        LOGE("Release executor for reopen database! errCode = [%d]", errCode);
        return errCode;
    }

    // close db for reinit this engine
    Release();

    // delete cache db
    errCode = DBCommon::RemoveAllFilesOfDirectory(GetDbDir(option_.subdir, DbType::CACHE), false);
    if (errCode != E_OK) {
        LOGE("Remove files of cache database after release current db:%d", errCode);
        return errCode;
    }

    // reInit, it will reset engine state
    errCode = ReInit();
    if (errCode != E_OK) {
        LOGE("Reinit failed when finish migrate data! please try reopen kvstore! errCode = [%d]", errCode);
        return errCode;
    }

    return E_OK;
}

int SQLiteSingleVerStorageEngine::InitExecuteMigrate(SQLiteSingleVerStorageExecutor *handle,
    EngineState preMigrateState)
{
    // after attach main and cache need change operate data sql, changing state forbid operate database
    SetEngineState(EngineState::MIGRATING);

    int errCode = E_OK;
    // check if has been attach and attach cache and main for migrate
    if (executorState_ == ExecutorState::MAINDB || executorState_ == ExecutorState::CACHEDB) {
        errCode = AttachMainDbAndCacheDb(handle, preMigrateState);
        if (errCode != E_OK) {
            LOGE("[ExeMigrate] Attach main db and cache db failed!, errCode = [%d]", errCode);
            // For lock state open db, can not attach main and cache
            return errCode;
        }
    } else if (executorState_ == ExecutorState::MAIN_ATTACH_CACHE ||
        // Has been attach, maybe ever crashed, need update version
        executorState_ == ExecutorState::CACHE_ATTACH_MAIN) {
        uint64_t maxVersion = 0;
        errCode = handle->GetMaxVersionIncacheDb(maxVersion);
        if (errCode != E_OK || maxVersion < CACHE_RECORD_DEFAULT_VERSION) {
            maxVersion = CACHE_RECORD_DEFAULT_VERSION;
        }
        (void)cacheRecordVersion_.store(maxVersion + 1, std::memory_order_seq_cst);
    } else {
        return -E_UNEXPECTED_DATA;
    }

    return errCode;
}

int SQLiteSingleVerStorageEngine::ExecuteMigrate()
{
    EngineState preState = GetEngineState();
    std::lock_guard<std::mutex> lock(migrateLock_);
    if (preState == EngineState::MIGRATING || preState == EngineState::INVALID ||
        !OS::CheckPathExistence(GetDbDir(option_.subdir, DbType::CACHE) + "/" + DBConstant::SINGLE_VER_CACHE_STORE +
        DBConstant::SQLITE_DB_EXTENSION)) {
        LOGD("[SqlSingleVerEngine] Being single ver migrating or never create db! engine state [%d]", preState);
        return E_OK;
    }

    // Get write executor for migrate
    int errCode = E_OK;
    auto handle = static_cast<SQLiteSingleVerStorageExecutor *>(FindExecutor(true, OperatePerm::NORMAL_PERM, errCode));
    if (errCode != E_OK) {
        LOGE("Migrate data fail, Can not get available executor, errCode = [%d]", errCode);
        return errCode;
    }

    bool isNeedTriggerSync = false;
    errCode = InitExecuteMigrate(handle, preState);
    if (errCode != E_OK) {
        LOGE("Init migrate data fail, errCode = [%d]", errCode);
        goto END;
    }

    LOGD("[SqlSingleVerEngine] Current enginState [%d] executorState [%d], begin to executing singleVer db migrate!",
        preState, executorState_);
    // has been attach, Mark start of migration and it can migrating data
    errCode = MigrateLocalData(handle);
    if (errCode != E_OK) {
        LOGE("Migrate local data fail, errCode = [%d]", errCode);
        goto END;
    }

    errCode = MigrateSyncData(handle, isNeedTriggerSync);
    if (errCode != E_OK) {
        LOGE("Migrate Sync data fail, errCode = [%d]", errCode);
        goto END;
    }

    SetEngineState(EngineState::ENGINE_BUSY); // temp forbid use handle and engine for detach and close executor

    // detach database and delete cachedb
    errCode = FinishMigrateData(handle, preState);
    if (errCode != E_OK) {
        LOGE("Finish migrating data fail, errCode = [%d]", errCode);
        goto END;
    }

END: // after FinishMigrateData, it will reset engine state
    // there is no need cover the errCode
    EndMigrate(handle, preState, errCode, isNeedTriggerSync);
    return errCode;
}

void SQLiteSingleVerStorageEngine::EndMigrate(SQLiteSingleVerStorageExecutor *&handle, EngineState stateBeforeMigrate,
    int errCode, bool isNeedTriggerSync)
{
    LOGD("Finish migrating data! errCode = [%d]", errCode);
    if (errCode != E_OK) {
        SetEngineState(stateBeforeMigrate);
    }
    if (handle != nullptr) {
        handle->ClearMigrateData();
    }
    errCode = ReleaseExecutor(handle);
    if (errCode != E_OK) {
        LOGE("release executor after migrating! errCode = [%d]", errCode);
    }
    // Notify max timestamp offset for SyncEngine.
    // When time change offset equals 0, SyncEngine can adjust local time offset according to max timestamp.
    RuntimeContext::GetInstance()->NotifyTimeStampChanged(0);
    if (isNeedTriggerSync) {
        commitNotifyFunc_(SQLITE_GENERAL_FINISH_MIGRATE_EVENT, nullptr);
    }
    return;
}

bool SQLiteSingleVerStorageEngine::IsEngineCorrupted() const
{
    return isCorrupted_;
}

StorageExecutor *SQLiteSingleVerStorageEngine::NewSQLiteStorageExecutor(sqlite3 *dbHandle, bool isWrite, bool isMemDb)
{
    auto executor = new (std::nothrow) SQLiteSingleVerStorageExecutor(dbHandle, isWrite, isMemDb);
    if (executor == nullptr) {
        return executor;
    }
    executor->SetConflictResolvePolicy(option_.conflictReslovePolicy);
    return executor;
}

int SQLiteSingleVerStorageEngine::TryToOpenMainDatabase(sqlite3 *&db)
{
    // Only could get the main database handle in the uninitialized and the main status.
    if (GetEngineState() != EngineState::INVALID && GetEngineState() != EngineState::MAINDB) {
        LOGE("[SQLiteSinStoreEng][GetMainHandle] Can only create new handle for state[%d]", GetEngineState());
        return -E_EKEYREVOKED;
    }

    if (!option_.isMemDb) {
        option_.uri = GetDbDir(option_.subdir, DbType::MAIN) + "/" + DBConstant::SINGLE_VER_DATA_STORE +
            DBConstant::SQLITE_DB_EXTENSION;
    }

    int errCode = SQLiteUtils::OpenDatabase(option_, db);
    if (errCode != E_OK) {
        if (errno == EKEYREVOKED) {
            LOGI("Failed to open the main database for key revoked[%d]", errCode);
            errCode = -E_EKEYREVOKED;
        }
        return errCode;
    }

    // Set the engine state to main status for that the main database is valid.
    SetEngineState(EngineState::MAINDB);

    if (OS::CheckPathExistence(GetDbDir(option_.subdir, DbType::CACHE) + "/" + DBConstant::SINGLE_VER_CACHE_STORE +
        DBConstant::SQLITE_DB_EXTENSION)) {
        // In status cacheDb crash
        errCode = AttachMainDbAndCacheDb(db, EngineState::MAINDB);
        if (errCode != E_OK) {
            LOGE("[SingleVerEngine][GetMain] Attach main db and cache db failed!, errCode = [%d]", errCode);
            executorState_ = ExecutorState::MAINDB;
            return E_OK; // not care err to return, only use for print log
        }
        executorState_ = ExecutorState::MAIN_ATTACH_CACHE;
        // cache and main existed together, can not read data, must execute migrate first
        SetEngineState(EngineState::ATTACHING);
    }

    return errCode;
}

int SQLiteSingleVerStorageEngine::GetDbHandle(bool isWrite, const SecurityOption &secOpt, sqlite3 *&dbHandle)
{
    int errCode = TryToOpenMainDatabase(dbHandle);
    LOGD("Finish to open the main database, write[%d], label[%d], flag[%d], errCode[%d]",
        isWrite, secOpt.securityLabel, secOpt.securityFlag, errCode);
    if (!(ParamCheckUtils::IsS3SECEOpt(secOpt) && errCode == -E_EKEYREVOKED)) {
        return errCode;
    }

    std::string cacheDbPath = GetDbDir(option_.subdir, DbType::CACHE) + "/" + DBConstant::SINGLE_VER_CACHE_STORE +
        DBConstant::SQLITE_DB_EXTENSION;
    if (!isWrite || GetEngineState() != EngineState::INVALID ||
        OS::CheckPathExistence(cacheDbPath)) {
        LOGI("[SQLiteSingleStorageEng][GetDbHandle]Only use for first create cache db! [%d] [%d]",
            isWrite, GetEngineState());
        return -E_EKEYREVOKED;
    }

    errCode = GetCacheDbHandle(dbHandle);
    if (errCode != E_OK) {
        LOGE("singleVerStorageEngine::GetDbHandle get cache handle fail! errCode = [%d]", errCode);
        return errCode;
    }
    SetEngineState(CACHEDB);
    executorState_ = ExecutorState::CACHEDB;

    ResetCacheRecordVersion();
    // Get handle means maindb file ekeyevoked, not need attach to
    return errCode;
}

namespace CacheDbSqls {
const std::string CREATE_CACHE_LOCAL_TABLE_SQL =
    "CREATE TABLE IF NOT EXISTS local_data(" \
        "key     BLOB   NOT NULL," \
        "value  BLOB," \
        "timestamp  INT," \
        "hash_key   BLOB   PRIMARY KEY   NOT NULL," \
        "flag  INT  NOT NULL);";

const std::string CREATE_CACHE_SYNC_TABLE_SQL =
    "CREATE TABLE IF NOT EXISTS sync_data(" \
        "key         BLOB NOT NULL," \
        "value       BLOB," \
        "timestamp   INT  NOT NULL," \
        "flag        INT  NOT NULL," \
        "device      BLOB," \
        "ori_device  BLOB," \
        "hash_key    BLOB  NOT NULL," \
        "w_timestamp INT," \
        "version     INT  NOT NULL," \
        "PRIMARY Key(version, hash_key));";
}

// Warning: Use error passwd create cache database can not check, it will create error passwd cache db,
// And make migrate data failed! This cache db will not be open correctly.
int SQLiteSingleVerStorageEngine::GetCacheDbHandle(sqlite3 *&db)
{
    option_.uri = GetDbDir(option_.subdir, DbType::CACHE) + "/" + DBConstant::SINGLE_VER_CACHE_STORE +
        DBConstant::SQLITE_DB_EXTENSION;
    // creatTable
    option_.sqls = {
        CacheDbSqls::CREATE_CACHE_LOCAL_TABLE_SQL,
        CacheDbSqls::CREATE_CACHE_SYNC_TABLE_SQL
    };

    if (!option_.createIfNecessary) {
        std::string mainDbPtah = GetDbDir(option_.subdir, DbType::MAIN) + "/" + DBConstant::SINGLE_VER_DATA_STORE +
            DBConstant::SQLITE_DB_EXTENSION;
        if (!OS::CheckPathExistence(mainDbPtah)) { // Whether to create a cacheDb is based on whether the mainDb exists
            return -E_INVALID_DB;
        }
    }

    OpenDbProperties option = option_; // copy for no change it
    option.createIfNecessary = true;
    int errCode = SQLiteUtils::OpenDatabase(option, db);
    if (errCode != E_OK) {
        LOGE("Get CacheDb handle failed, errCode = [%d], errno = [%d]", errCode, errno);
        return errCode;
    }
    return errCode;
}

int SQLiteSingleVerStorageEngine::CheckDatabaseSecOpt(const SecurityOption &secOption) const
{
    if (!(secOption == option_.securityOpt) &&
        secOption.securityLabel != SecurityLabel::NOT_SET &&
        option_.securityOpt.securityLabel != SecurityLabel::NOT_SET) {
        LOGE("SecurityOption mismatch, existed:[%d-%d] vs input:[%d-%d]", secOption.securityLabel,
            secOption.securityFlag, option_.securityOpt.securityLabel, option_.securityOpt.securityFlag);
        return -E_SECURITY_OPTION_CHECK_ERROR;
    }
    return E_OK;
}

int SQLiteSingleVerStorageEngine::CreateNewDirsAndSetSecOpt() const
{
    std::vector<std::string> dbDir {DBConstant::MAINDB_DIR, DBConstant::METADB_DIR, DBConstant::CACHEDB_DIR};
    for (const auto &item : dbDir) {
        if (OS::CheckPathExistence(option_.subdir + "/" + item)) {
            continue;
        }

        // Dir and old db file not existed, it means that the database is newly created
        // need create flag of database not incomplete
        if (!OS::CheckPathExistence(option_.subdir + DBConstant::PATH_POSTFIX_DB_INCOMPLETE) &&
            !OS::CheckPathExistence(option_.subdir + "/" + DBConstant::SINGLE_VER_DATA_STORE +
            DBConstant::SQLITE_DB_EXTENSION) &&
            OS::CreateFileByFileName(option_.subdir + DBConstant::PATH_POSTFIX_DB_INCOMPLETE) != E_OK) {
            LOGE("Fail to create the token of database incompleted! errCode = [E_SYSTEM_API_FAIL]");
            return -E_SYSTEM_API_FAIL;
        }

        if (DBCommon::CreateDirectory(option_.subdir + "/" + item) != E_OK) {
            LOGE("Create sub-directory for single ver failed, errno:%d", errno);
            return -E_SYSTEM_API_FAIL;
        }

        if (option_.securityOpt.securityLabel == NOT_SET) {
            continue;
        }

        SecurityOption option = option_.securityOpt;
        if (item == DBConstant::METADB_DIR) {
            option.securityLabel = ((option_.securityOpt.securityLabel >= SecurityLabel::S2) ?
                SecurityLabel::S2 : option_.securityOpt.securityLabel);
            option.securityFlag = SecurityFlag::ECE;
        }

        int errCode = RuntimeContext::GetInstance()->SetSecurityOption(option_.subdir + "/" + item, option);
        if (errCode != E_OK && errCode != -E_NOT_SUPPORT) {
            LOGE("Set the security option of sub-directory failed[%d]", errCode);
            return errCode;
        }
    }
    return E_OK;
}

int SQLiteSingleVerStorageEngine::GetExistedSecOption(SecurityOption &secOption) const
{
    // Check the existence of the database, include the origin database and the database in the 'main' directory.
    auto mainDbDir = GetDbDir(option_.subdir, DbType::MAIN);
    auto mainDbFilePath = mainDbDir + "/" + DBConstant::SINGLE_VER_DATA_STORE + DBConstant::SQLITE_DB_EXTENSION;
    auto origDbFilePath = option_.subdir + "/" + DBConstant::SINGLE_VER_DATA_STORE + DBConstant::SQLITE_DB_EXTENSION;
    if (!OS::CheckPathExistence(origDbFilePath) && !OS::CheckPathExistence(mainDbFilePath)) {
        secOption = option_.securityOpt;
        return E_OK;
    }

    // the main database file has high priority of the security option.
    int errCode;
    if (OS::CheckPathExistence(mainDbFilePath)) {
        errCode = GetPathSecurityOption(mainDbFilePath, secOption);
    } else {
        errCode = GetPathSecurityOption(origDbFilePath, secOption);
    }
    if (errCode != E_OK) {
        secOption = SecurityOption();
        if (errCode == -E_NOT_SUPPORT) {
            return E_OK;
        }
        LOGE("Get the security option of the existed database failed.");
    }
    return errCode;
}

void SQLiteSingleVerStorageEngine::ClearCorruptedFlag()
{
    isCorrupted_ = false;
}

int SQLiteSingleVerStorageEngine::PreCreateExecutor(bool isWrite)
{
    // Assume that create the write executor firstly and the write one we will not be released.
    // If the write one would be released in the future, should take care the pass through.
    if (!isWrite) {
        return E_OK;
    }

    if (option_.isMemDb) {
        return E_OK;
    }

    // Get the existed database secure option.
    SecurityOption existedSecOpt;
    int errCode = GetExistedSecOption(existedSecOpt);
    if (errCode != E_OK) {
        return errCode;
    }

    errCode = CheckDatabaseSecOpt(existedSecOpt);
    if (errCode != E_OK) {
        return errCode;
    }

    // Judge whether need update the security option of the engine.
    // Should update the security in the import or rekey scene(inner).
    if (!isNeedUpdateSecOpt_) {
        option_.securityOpt = existedSecOpt;
    }

    errCode = CreateNewDirsAndSetSecOpt();
    if (errCode != E_OK) {
        return errCode;
    }

    if (!isUpdated_) {
        errCode = SQLiteSingleVerDatabaseUpgrader::TransferDatabasePath(option_.subdir, option_);
        if (errCode != E_OK) {
            LOGE("[PreCreateExecutor] Transfer Db file path failed[%d].", errCode);
            return errCode;
        }
    }

    return E_OK;
}

int SQLiteSingleVerStorageEngine::EndCreateExecutor(bool isWrite)
{
    if (option_.isMemDb || !isWrite) {
        return E_OK;
    }

    int errCode = SQLiteSingleVerDatabaseUpgrader::SetSecOption(option_.subdir, option_.securityOpt,
        isNeedUpdateSecOpt_);
    if (errCode != E_OK) {
        if (errCode == -E_NOT_SUPPORT) {
            option_.securityOpt = SecurityOption();
            errCode = E_OK;
        }
        LOGE("SetSecOption failed:%d", errCode);
        return errCode;
    }

    // after setting secOption, the database file operation ends
    // database create completed, delete the token
    if (OS::CheckPathExistence(option_.subdir + DBConstant::PATH_POSTFIX_DB_INCOMPLETE) &&
        OS::RemoveFile(option_.subdir + DBConstant::PATH_POSTFIX_DB_INCOMPLETE) != E_OK) {
        LOGE("Finish to create the complete database, but delete token fail! errCode = [E_SYSTEM_API_FAIL]");
        return -E_SYSTEM_API_FAIL;
    }
    return errCode;
}

int SQLiteSingleVerStorageEngine::TryAttachMetaDb(sqlite3 *&dbHandle, bool &isAttachMeta)
{
    // attach or not depend on its true secOpt, but it's not permit while option_.secOpt different from true secOpt
    if ((!option_.isMemDb) && (ParamCheckUtils::IsS3SECEOpt(option_.securityOpt))) {
        int errCode = AttachMetaDatabase(dbHandle, option_);
        if (errCode != E_OK) {
            (void)sqlite3_close_v2(dbHandle);
            dbHandle = nullptr;
            return errCode;
        }
        isAttachMeta = true;
    }
    return E_OK;
}

int SQLiteSingleVerStorageEngine::CreateNewExecutor(bool isWrite, StorageExecutor *&handle)
{
    int errCode = PreCreateExecutor(isWrite);
    if (errCode != E_OK) {
        return errCode;
    }

    sqlite3 *dbHandle = nullptr;
    errCode = GetDbHandle(isWrite, option_.securityOpt, dbHandle);
    if (errCode != E_OK) {
        return errCode;
    }

    bool isAttachMeta = false;
    errCode = TryAttachMetaDb(dbHandle, isAttachMeta);
    if (errCode != E_OK) {
        return errCode;
    }

    RegisterFunctionIfNeed(dbHandle);
    errCode = Upgrade(dbHandle);
    if (errCode != E_OK) {
        (void)sqlite3_close_v2(dbHandle);
        dbHandle = nullptr;
        return errCode;
    }

    errCode = EndCreateExecutor(isWrite);
    if (errCode != E_OK) {
        LOGE("After create executor, set security option incomplete!");
        (void)sqlite3_close_v2(dbHandle);
        dbHandle = nullptr;
        return errCode;
    }

    handle = NewSQLiteStorageExecutor(dbHandle, isWrite, option_.isMemDb);
    if (handle == nullptr) {
        LOGE("New SQLiteStorageExecutor[%d] for the pool failed.", isWrite);
        (void)sqlite3_close_v2(dbHandle);
        dbHandle = nullptr;
        return -E_OUT_OF_MEMORY;
    }
    if (isAttachMeta == true) {
        SQLiteSingleVerStorageExecutor *singleVerHandle = static_cast<SQLiteSingleVerStorageExecutor *>(handle);
        singleVerHandle->SetAttachMetaMode(isAttachMeta);
    }
    return E_OK;
}

int SQLiteSingleVerStorageEngine::Upgrade(sqlite3 *db)
{
    if (isUpdated_ || GetEngineState() == CACHEDB) {
        LOGI("Storage engine is in cache status or has been upgraded[%d]!", isUpdated_);
        return E_OK;
    }

    std::unique_ptr<SQLiteSingleVerDatabaseUpgrader> upgrader;
    LOGD("[SqlSingleEngine][Upgrade] NewSchemaStrSize=%zu", option_.schema.size());
    if (option_.schema.empty()) {
        upgrader = std::make_unique<SQLiteSingleVerDatabaseUpgrader>(db, option_.securityOpt, option_.isMemDb);
    } else {
        SchemaObject schema;
        int errCode = schema.ParseFromSchemaString(option_.schema);
        if (errCode != E_OK) {
            LOGE("Upgrader failed while parsing the origin schema:%d", errCode);
            return errCode;
        }
        upgrader = std::make_unique<SQLiteSingleVerSchemaDatabaseUpgrader>(db, schema,
            option_.securityOpt, option_.isMemDb);
    }

    std::string mainDbDir = GetDbDir(option_.subdir, DbType::MAIN);
    std::string mainDbFilePath = mainDbDir + "/" + DBConstant::SINGLE_VER_DATA_STORE + DBConstant::SQLITE_DB_EXTENSION;
    SecurityOption secOpt = option_.securityOpt;
    int errCode = E_OK;
    if (isNeedUpdateSecOpt_) {
        errCode = GetPathSecurityOption(mainDbFilePath, secOpt);
        if (errCode != E_OK) {
            LOGI("[SingleVerStorageEngine::Upgrade] Failed to get the path security option, errCode = [%d]", errCode);
            if (errCode != -E_NOT_SUPPORT) {
                return errCode;
            }
            secOpt = SecurityOption();
        }
    }

    upgrader->SetMetaUpgrade(secOpt, option_.securityOpt, option_.subdir);
    upgrader->SetSubdir(option_.subdir);
    errCode = upgrader->Upgrade();
    if (errCode != E_OK) {
        LOGE("Single ver database upgrade failed:%d", errCode);
        return errCode;
    }

    LOGD("Finish upgrade single ver database!");
    isUpdated_ = true; // Identification to avoid repeated upgrades
    return errCode;
}

// Attention: This function should be called before "Upgrade".
// Attention: This function should be called for each executor on the sqlite3 handle that the executor binds to.
void SQLiteSingleVerStorageEngine::RegisterFunctionIfNeed(sqlite3 *dbHandle) const
{
    // This function should accept a sqlite3 handle with no perception of database classification. That is, if it is
    // not a newly created database, the meta-Table should exist and can be accessed.
    std::string schemaStr = option_.schema;
    if (schemaStr.empty()) {
        // If schema from GetKvStore::Option is empty, we have to try to load it from database. ReadOnly mode if exist;
        int errCode = SQLiteUtils::GetSchema(dbHandle, schemaStr);
        if (errCode != E_OK) {
            LOGD("[SqlSinEngine] Can't get schema from db[%d], maybe it is just created or not a schema-db.", errCode);
        }
    }
    if (!schemaStr.empty()) {
        // This must be a Schema-Database, if it is Json-Schema, the Register will do nothing and return E_OK
        int errCode = SQLiteUtils::RegisterFlatBufferFunction(dbHandle, schemaStr);
        if (errCode != E_OK) { // Not very likely
            // Just warning, if no index had been or need to be created, then put or kv-get can still use.
            LOGW("[SqlSinEngine] RegisterFlatBufferExtractFunction fail, errCode = %d", errCode);
        }
    }
}

int SQLiteSingleVerStorageEngine::AttachMetaDatabase(sqlite3 *dbHandle, const OpenDbProperties &option) const
{
    int errCode;
    LOGD("SQLiteSingleVerStorageEngine begin attach metaDb!");
    std::string metaDbPath = option.subdir + "/" + DBConstant::METADB_DIR + "/" +
        DBConstant::SINGLE_VER_META_STORE + DBConstant::SQLITE_DB_EXTENSION;
    // attach metaDb may failed while createIfNecessary is false, here need to create metaDb first.
    if (!option.createIfNecessary && !OS::CheckPathExistence(metaDbPath)) {
        errCode = SQLiteUtils::CreateMetaDatabase(metaDbPath);
        if (errCode != E_OK) {
            return errCode;
        }
    }
    CipherPassword passwd;
    errCode = SQLiteUtils::AttachNewDatabase(dbHandle, option.cipherType, passwd, metaDbPath, "meta");
    if (errCode != E_OK) {
        LOGE("AttachNewDatabase fail, errCode = %d", errCode);
    }
    return errCode;
}

void SQLiteSingleVerStorageEngine::ResetCacheRecordVersion()
{
    (void)cacheRecordVersion_.store(CACHE_RECORD_DEFAULT_VERSION, std::memory_order_seq_cst);
}

void SQLiteSingleVerStorageEngine::IncreaseCacheRecordVersion()
{
    (void)cacheRecordVersion_.fetch_add(1, std::memory_order_seq_cst);
}

uint64_t SQLiteSingleVerStorageEngine::GetAndIncreaseCacheRecordVersion()
{
    return cacheRecordVersion_.fetch_add(1, std::memory_order_seq_cst);
}

uint64_t SQLiteSingleVerStorageEngine::GetCacheRecordVersion() const
{
    return cacheRecordVersion_.load(std::memory_order_seq_cst);
}

void SQLiteSingleVerStorageEngine::CommitAndReleaseNotifyData(SingleVerNaturalStoreCommitNotifyData *&committedData,
    int eventType) const
{
    std::shared_lock<std::shared_mutex> lock(notifyMutex_);
    if (commitNotifyFunc_ == nullptr) {
        LOGE("commitNotifyFunc_ is nullptr, can't notify now.");
        RefObject::DecObjRef(committedData);
        committedData = nullptr;
        return;
    }
    commitNotifyFunc_(eventType, static_cast<KvDBCommitNotifyFilterAbleData *>(committedData));
    committedData = nullptr;
    return;
}

void SQLiteSingleVerStorageEngine::InitConflictNotifiedFlag(SingleVerNaturalStoreCommitNotifyData *&committedData) const
{
    if (committedData == nullptr) {
        LOGI("[SQLiteSingleVerStorageEngine::InitConflictNotifiedFlag] committedData is null.");
        return;
    }
    auto identifier = GetIdentifier();
    auto kvdb = KvDBManager::GetInstance()->FindKvDB(identifier);
    if (kvdb == nullptr) {
        LOGE("[SQLiteSingleVerStorageEngine::InitConflictNotifiedFlag] kvdb is null.");
        return;
    }
    unsigned int conflictFlag = 0;
    if (static_cast<GenericKvDB *>(kvdb)->GetRegisterFunctionCount(CONFLICT_SINGLE_VERSION_NS_FOREIGN_KEY_ONLY) != 0) {
        conflictFlag |= static_cast<unsigned>(SQLITE_GENERAL_NS_FOREIGN_KEY_ONLY);
    }
    if (static_cast<GenericKvDB *>(kvdb)->GetRegisterFunctionCount(CONFLICT_SINGLE_VERSION_NS_FOREIGN_KEY_ORIG) != 0) {
        conflictFlag |= static_cast<unsigned>(SQLITE_GENERAL_NS_FOREIGN_KEY_ORIG);
    }
    if (static_cast<GenericKvDB *>(kvdb)->GetRegisterFunctionCount(CONFLICT_SINGLE_VERSION_NS_NATIVE_ALL) != 0) {
        conflictFlag |= static_cast<unsigned>(SQLITE_GENERAL_NS_NATIVE_ALL);
    }
    RefObject::DecObjRef(kvdb);
    LOGD("[SQLiteSingleVerStorageEngine::InitConflictNotifiedFlag] conflictFlag Flag: %u", conflictFlag);
    committedData->SetConflictedNotifiedFlag(static_cast<int>(conflictFlag));
}

void SQLiteSingleVerStorageEngine::SetMaxTimeStamp(TimeStamp maxTimeStamp) const
{
    auto kvdbManager = KvDBManager::GetInstance();
    if (kvdbManager == nullptr) {
        return;
    }
    auto identifier = GetIdentifier();
    auto kvdb = kvdbManager->FindKvDB(identifier);
    if (kvdb == nullptr) {
        LOGE("[SQLiteSingleVerStorageEngine::SetMaxTimeStamp] kvdb is null.");
        return;
    }

    auto kvStore = static_cast<SQLiteSingleVerNaturalStore *>(kvdb);
    kvStore->SetMaxTimeStamp(maxTimeStamp);
    RefObject::DecObjRef(kvdb);
    return;
}

void SQLiteSingleVerStorageEngine::CommitNotifyForMigrateCache(NotifyMigrateSyncData &syncData) const
{
    auto &isRemote = syncData.isRemote;
    auto &isRemoveDeviceData = syncData.isRemoveDeviceData;
    auto &committedData = syncData.committedData;
    auto &entries = syncData.entries;

    // Put data. Including insert, update and delete.
    if (!isRemoveDeviceData) {
        if (committedData != nullptr) {
            int eventType = isRemote ? SQLITE_GENERAL_NS_SYNC_EVENT : SQLITE_GENERAL_NS_PUT_EVENT;
            CommitAndReleaseNotifyData(committedData, eventType);
        }
        return;
    }

    // Remove device data.
    if (entries.empty() || entries.size() > MAX_TOTAL_NOTIFY_ITEM_SIZE) {
        return;
    }
    size_t totalSize = 0;
    for (auto iter = entries.begin(); iter != entries.end();) {
        auto &entry = *iter;
        if (committedData == nullptr) {
            committedData = new (std::nothrow) SingleVerNaturalStoreCommitNotifyData();
            if (committedData == nullptr) {
                LOGE("Alloc committed notify data failed.");
                return;
            }
        }
        if (entry.key.size() > DBConstant::MAX_KEY_SIZE || entry.value.size() > DBConstant::MAX_VALUE_SIZE) {
            iter++;
            continue;
        }
        if (entry.key.size() + entry.value.size() + totalSize > MAX_TOTAL_NOTIFY_DATA_SIZE) {
            CommitAndReleaseNotifyData(committedData, SQLITE_GENERAL_NS_SYNC_EVENT);
            totalSize = 0;
            continue;
        }
        totalSize += (entry.key.size() + entry.value.size());
        committedData->InsertCommittedData(std::move(entry), DataType::DELETE, false);
        iter++;
    }
    if (committedData != nullptr) {
        CommitAndReleaseNotifyData(committedData, SQLITE_GENERAL_NS_SYNC_EVENT);
    }
    return;
}
}
