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

#include "sqlite_local_kvdb.h"

#include <algorithm>

#include "db_constant.h"
#include "db_common.h"
#include "log_print.h"
#include "platform_specific.h"
#include "package_file.h"
#include "kvdb_utils.h"
#include "local_database_oper.h"
#include "sqlite_local_kvdb_connection.h"
#include "sqlite_local_storage_engine.h"

namespace DistributedDB {
namespace {
    const std::string CREATE_SQL =
        "CREATE TABLE IF NOT EXISTS data(key BLOB PRIMARY key, value BLOB);";
}

SQLiteLocalKvDB::SQLiteLocalKvDB()
    : storageEngine_(nullptr)
{}

SQLiteLocalKvDB::~SQLiteLocalKvDB()
{
    if (storageEngine_ != nullptr) {
        delete storageEngine_;
        storageEngine_ = nullptr;
    }
}

int SQLiteLocalKvDB::Open(const KvDBProperties &kvDBProp)
{
    int databaseType = kvDBProp.GetIntProp(KvDBProperties::DATABASE_TYPE, KvDBProperties::LOCAL_TYPE);
    if (databaseType == KvDBProperties::LOCAL_TYPE) {
        std::unique_ptr<LocalDatabaseOper> operation = std::make_unique<LocalDatabaseOper>(this, nullptr);
        (void)operation->ClearExportedTempFiles(kvDBProp);
        int errCode = operation->RekeyRecover(kvDBProp);
        if (errCode != E_OK) {
            LOGE("Recover for open db failed in local db:%d", errCode);
            return errCode;
        }

        errCode = operation->ClearImportTempFile(kvDBProp);
        if (errCode != E_OK) {
            LOGE("Recover for open db failed in multi version:%d", errCode);
            return errCode;
        }
    }

    bool createIfNecessary = kvDBProp.GetBoolProp(KvDBProperties::CREATE_IF_NECESSARY, true);
    std::string subDir = KvDBProperties::GetStoreSubDirectory(databaseType);
    std::string dataDir = kvDBProp.GetStringProp(KvDBProperties::DATA_DIR, "");
    std::string identifierDir = kvDBProp.GetStringProp(KvDBProperties::IDENTIFIER_DIR, "");
    int errCode = DBCommon::CreateStoreDirectory(dataDir, identifierDir, subDir, createIfNecessary);
    if (errCode != E_OK) {
        LOGE("Create directory for local database failed:%d", errCode);
        return errCode;
    }

    errCode = InitStorageEngine(kvDBProp);
    if (errCode != E_OK) {
        return errCode;
    }
    MyProp() = kvDBProp;
    return E_OK;
}

GenericKvDBConnection *SQLiteLocalKvDB::NewConnection(int &errCode)
{
    auto connection = new (std::nothrow) SQLiteLocalKvDBConnection(this);
    if (connection == nullptr) {
        errCode = -E_OUT_OF_MEMORY;
        return nullptr;
    }

    errCode = E_OK;
    return connection;
}

void SQLiteLocalKvDB::Close() {}

int SQLiteLocalKvDB::Rekey(const CipherPassword &passwd)
{
    if (storageEngine_ == nullptr) {
        return -E_INVALID_DB;
    }

    std::unique_ptr<LocalDatabaseOper> operation = std::make_unique<LocalDatabaseOper>(this, storageEngine_);
    return operation->Rekey(passwd);
}

int SQLiteLocalKvDB::Export(const std::string &filePath, const CipherPassword &passwd)
{
    int errCode = E_OK;
    // Exclusively write resources
    SQLiteLocalStorageExecutor *handle = GetHandle(true, errCode);
    if (handle == nullptr) {
        return errCode;
    }
    std::string devId = "local";

    std::unique_ptr<LocalDatabaseOper> operation = std::make_unique<LocalDatabaseOper>(this, storageEngine_);
    operation->SetLocalDevId(DBCommon::TransferHashString(devId));
    errCode = operation->Export(filePath, passwd);

    ReleaseHandle(handle);
    return errCode;
}

int SQLiteLocalKvDB::Import(const std::string &filePath, const CipherPassword &passwd)
{
    if (storageEngine_ == nullptr) {
        return -E_INVALID_DB;
    }

    int errCode = storageEngine_->TryToDisable(true, OperatePerm::IMPORT_MONOPOLIZE_PERM);
    if (errCode != E_OK) {
        LOGE("Failed to disable the database");
        return errCode;
    }

    // Need to monopolize the entire process
    std::unique_ptr<LocalDatabaseOper> operation = std::make_unique<LocalDatabaseOper>(this, storageEngine_);
    errCode = operation->Import(filePath, passwd);
    // restore the storage engine and the syncer.
    storageEngine_->Enable(OperatePerm::IMPORT_MONOPOLIZE_PERM);
    return errCode;
}

int SQLiteLocalKvDB::InitDatabaseContext(const KvDBProperties &kvDBProp)
{
    return InitStorageEngine(kvDBProp);
}

void SQLiteLocalKvDB::EnableAutonomicUpgrade()
{
    isAutonomicUpgradeEnable_ = true;
}

int SQLiteLocalKvDB::RunExportLogic(CipherType type, const CipherPassword &passwd, const std::string &newDbName)
{
    OpenDbProperties option;
    InitDataBaseOption(MyProp(), option);
    option.createIfNecessary = true;
    sqlite3 *db = nullptr;
    int errCode = SQLiteUtils::OpenDatabase(option, db);
    if (errCode != E_OK) {
        LOGE("Open db for export error:%d", errCode);
        return errCode;
    }

    errCode = SQLiteUtils::ExportDatabase(db, type, passwd, newDbName);
    if (errCode != E_OK) {
        goto END;
    }

END:
    (void)sqlite3_close_v2(db);
    db = nullptr;
    return errCode;
}

int SQLiteLocalKvDB::RunRekeyLogic(CipherType type, const CipherPassword &passwd)
{
    OpenDbProperties option;
    InitDataBaseOption(MyProp(), option);
    option.createIfNecessary = true;
    sqlite3 *db = nullptr;
    int errCode = SQLiteUtils::OpenDatabase(option, db);
    if (errCode != E_OK) {
        LOGE("Open db for rekey error:%d", errCode);
        return errCode;
    }

    errCode = SQLiteUtils::Rekey(db, passwd);
    if (errCode != E_OK) {
        (void)sqlite3_close_v2(db);
        db = nullptr;
        return errCode;
    }
    (void)sqlite3_close_v2(db);
    db = nullptr;
    MyProp().SetPassword(option.cipherType, passwd);
    if (storageEngine_ != nullptr) {
        storageEngine_->Release();
    }

    return InitStorageEngine(MyProp());
}

SQLiteLocalStorageExecutor *SQLiteLocalKvDB::GetHandle(bool isWrite, int &errCode, OperatePerm perm) const
{
    if (storageEngine_ == nullptr) {
        errCode = -E_INVALID_DB;
        return nullptr;
    }

    return static_cast<SQLiteLocalStorageExecutor *>(storageEngine_->FindExecutor(isWrite, perm, errCode));
}

int SQLiteLocalKvDB::GetVersion(const KvDBProperties &kvDBProp, int &version, bool &isDbExisted) const
{
    OpenDbProperties option;
    InitDataBaseOption(kvDBProp, option);
    isDbExisted = OS::CheckPathExistence(option.uri);

    int errCode = E_OK;
    if (isDbExisted) {
        errCode = SQLiteUtils::GetVersion(option, version);
    }
    return errCode;
}

int SQLiteLocalKvDB::SetVersion(const KvDBProperties &kvDBProp, int version)
{
    OpenDbProperties option;
    InitDataBaseOption(kvDBProp, option);
    bool isDbExisted = OS::CheckPathExistence(option.uri);
    if (!isDbExisted) {
        return -E_NOT_FOUND;
    }
    return SQLiteUtils::SetUserVer(option, version);
}

const KvDBProperties &SQLiteLocalKvDB::GetDbProperties() const
{
    return GetMyProperties();
}

KvDBProperties &SQLiteLocalKvDB::GetDbPropertyForUpdate()
{
    return MyProp();
}

void SQLiteLocalKvDB::ReleaseHandle(SQLiteLocalStorageExecutor *&handle) const
{
    if (storageEngine_ != nullptr) {
        bool isCorrupted = handle->GetCorruptedStatus();
        StorageExecutor *databaseHandle = handle;
        storageEngine_->Recycle(databaseHandle);
        handle = nullptr;
        if (isCorrupted) {
            CorruptNotify();
        }
    }
}

int SQLiteLocalKvDB::InitStorageEngine(const KvDBProperties &kvDBProp)
{
    if (storageEngine_ == nullptr) {
        // Create HandlePool
        storageEngine_ = new (std::nothrow) SQLiteLocalStorageEngine();
        if (storageEngine_ == nullptr) {
            LOGE("Create local sqlite storage engine OOM");
            return -E_OUT_OF_MEMORY;
        }
    }

    OpenDbProperties option;
    InitDataBaseOption(kvDBProp, option);
    StorageEngineAttr poolSize = {0, 1, 0, 4}; // 1 write 4 read at most.
    int errCode = storageEngine_->InitSQLiteStorageEngine(poolSize, option);
    if (errCode != E_OK) {
        goto END;
    }

    // We don't have to do version check here if the SQLiteLocalKvDB does not work as LocalStore.
    // The isAutonomicUpgradeEnable_ true indicate that it work as LocalStore. Do version check in three case:
    // Open the database, which call Open then InitStorageEngine.
    // Import the database, which call InitDatabaseContext then InitStorageEngine.
    // Rekey the database, which call RunRekeyLogic then InitStorageEngine. (This case is not necessary in fact)
    errCode = CheckVersionAndUpgradeIfNeed(option);
END:
    if (errCode != E_OK) {
        LOGE("Init sqlite handler pool failed:%d", errCode);
        // Transform the errCode.
    }
    return errCode;
}

void SQLiteLocalKvDB::InitDataBaseOption(const KvDBProperties &kvDBProp, OpenDbProperties &option) const
{
    std::string dataDir = kvDBProp.GetStringProp(KvDBProperties::DATA_DIR, "");
    std::string identifierDir = kvDBProp.GetStringProp(KvDBProperties::IDENTIFIER_DIR, "");
    std::string dbName = kvDBProp.GetStringProp(KvDBProperties::FILE_NAME, DBConstant::LOCAL_DATABASE_NAME);
    int databaseType = kvDBProp.GetIntProp(KvDBProperties::DATABASE_TYPE, KvDBProperties::LOCAL_TYPE);
    bool createIfNecessary = kvDBProp.GetBoolProp(KvDBProperties::CREATE_IF_NECESSARY, true);
    std::string subDir = KvDBProperties::GetStoreSubDirectory(databaseType);
    // Table name "data" should not be changed in the future, otherwise when an older software open a newer database
    // with table of other name, we will create an second table as result which is not expected.
    std::vector<std::string> createTableSqls = {CREATE_SQL};
    CipherType cipherType;
    CipherPassword passwd;
    kvDBProp.GetPassword(cipherType, passwd);
    std::string uri = dataDir + "/" + identifierDir + "/" + subDir + "/" + dbName + DBConstant::SQLITE_DB_EXTENSION;
    option = {uri, createIfNecessary, false, createTableSqls, cipherType, passwd};
}

int SQLiteLocalKvDB::BackupCurrentDatabase(const KvDBProperties &properties, const std::string &dir)
{
    std::string baseDir;
    int errCode = GetWorkDir(properties, baseDir);
    if (errCode != E_OK) {
        LOGE("[SqlLocalDb][Backup] GetWorkDir fail, errCode=%d.", errCode);
        return errCode;
    }
    std::string dbName = properties.GetStringProp(KvDBProperties::FILE_NAME, DBConstant::LOCAL_DATABASE_NAME);
    int databaseType = properties.GetIntProp(KvDBProperties::DATABASE_TYPE, KvDBProperties::LOCAL_TYPE);
    std::string subDir = KvDBProperties::GetStoreSubDirectory(databaseType);
    std::string currentDb = baseDir + "/" + subDir + "/" + dbName + DBConstant::SQLITE_DB_EXTENSION;
    std::string dstDb = dir + "/" + dbName + DBConstant::SQLITE_DB_EXTENSION;
    errCode = DBCommon::CopyFile(currentDb, dstDb);
    if (errCode != E_OK) {
        LOGE("Copy the local current db error:%d", errCode);
    }
    return errCode;
}

int SQLiteLocalKvDB::ImportDatabase(const KvDBProperties &properties, const std::string &dir,
    const CipherPassword &passwd)
{
    std::string baseDir;
    int errCode = GetWorkDir(properties, baseDir);
    if (errCode != E_OK) {
        return errCode;
    }
    std::string dbName = properties.GetStringProp(KvDBProperties::FILE_NAME, DBConstant::LOCAL_DATABASE_NAME);
    int databaseType = properties.GetIntProp(KvDBProperties::DATABASE_TYPE, KvDBProperties::LOCAL_TYPE);
    std::string subDir = KvDBProperties::GetStoreSubDirectory(databaseType);
    std::string dstDb = baseDir + "/" + subDir + "/" + dbName + DBConstant::SQLITE_DB_EXTENSION;
    std::string currentDb = dir + "/" + dbName + DBConstant::SQLITE_DB_EXTENSION;
    CipherType cipherType;
    CipherPassword dstPasswd;
    properties.GetPassword(cipherType, dstPasswd);
    return SQLiteUtils::ExportDatabase(currentDb, cipherType, passwd, dstDb, dstPasswd);
}

int SQLiteLocalKvDB::RemoveKvDB(const KvDBProperties &properties)
{
    // Only care the data directory and the db name.
    std::string storeOnlyDir;
    std::string storeDir;
    GenericKvDB::GetStoreDirectory(properties, KvDBProperties::LOCAL_TYPE, storeDir, storeOnlyDir);
    int dbType = properties.GetIntProp(KvDBProperties::DATABASE_TYPE, KvDBProperties::LOCAL_TYPE);
    return KvDBUtils::RemoveKvDB(storeDir, storeOnlyDir, KvDBProperties::GetStoreSubDirectory(dbType));
}

int SQLiteLocalKvDB::GetKvDBSize(const KvDBProperties &properties, uint64_t &size) const
{
    std::string storeOnlyDir;
    std::string storeDir;
    GenericKvDB::GetStoreDirectory(properties, KvDBProperties::LOCAL_TYPE, storeDir, storeOnlyDir);
    int dbType = properties.GetIntProp(KvDBProperties::DATABASE_TYPE, KvDBProperties::LOCAL_TYPE);
    return KvDBUtils::GetKvDbSize(storeDir, storeOnlyDir, KvDBProperties::GetStoreSubDirectory(dbType), size);
}

int SQLiteLocalKvDB::CheckVersionAndUpgradeIfNeed(const OpenDbProperties &openProp)
{
    if (!isAutonomicUpgradeEnable_) {
        return E_OK;
    }
    int dbVersion = 0;
    int errCode = SQLiteUtils::GetVersion(openProp, dbVersion);
    if (errCode != E_OK) {
        LOGE("[SqlLocalDb][CheckUpgrade] GetVersion fail, errCode=%d.", errCode);
        return errCode;
    }
    LOGD("[SqlLocalDb][CheckUpgrade] DbFile Version=%d, CurVersion=%d.", dbVersion, LOCAL_STORE_VERSION_CURRENT);
    if (dbVersion > LOCAL_STORE_VERSION_CURRENT) {
        return -E_VERSION_NOT_SUPPORT;
    }
    // For version equal or less LOCAL_STORE_VERSION_CURRENT except zero, we can do nothing currently
    if (dbVersion != 0) {
        return E_OK;
    }
    errCode = SQLiteUtils::SetUserVer(openProp, LOCAL_STORE_VERSION_CURRENT);
    if (errCode != E_OK) {
        LOGE("[SqlLocalDb][CheckUpgrade] SetUserVer fail, errCode=%d.", errCode);
        return errCode;
    }
    return E_OK;
}

DEFINE_OBJECT_TAG_FACILITIES(SQLiteLocalKvDB)
} // namespace DistributedDB
