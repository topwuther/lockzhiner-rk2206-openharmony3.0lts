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
#include "sqlite_multi_ver_data_storage.h"

#include <climits>
#include <string>
#include <vector>
#include <algorithm>

#include "db_constant.h"
#include "db_types.h"
#include "log_print.h"
#include "sqlite_utils.h"
#include "multi_ver_kv_entry.h"
#include "multi_ver_value_object.h"
#include "value_hash_calc.h"
#include "db_common.h"
#include "multi_ver_natural_store.h"
#include "platform_specific.h"

namespace DistributedDB {
namespace {
    const std::string CREATE_TABLE_SQL =
        "CREATE TABLE IF NOT EXISTS version_data(key BLOB, value BLOB, oper_flag INTEGER, version INTEGER, " \
        "timestamp INTEGER, ori_timestamp INTEGER, hash_key BLOB, " \
        "PRIMARY key(hash_key, version));" \
        "CREATE INDEX IF NOT EXISTS version_index ON version_data (version);" \
        "CREATE INDEX IF NOT EXISTS flag_index ON version_data (oper_flag);";

    const std::size_t MAX_READ_CONNECT_NUM = 16;
}

SQLiteMultiVerDataStorage::SQLiteMultiVerDataStorage()
    : writeTransaction_(nullptr),
      writeTransactionUsed_(false)
{}

SQLiteMultiVerDataStorage::~SQLiteMultiVerDataStorage()
{
    writeTransaction_ = nullptr;
}

int SQLiteMultiVerDataStorage::CheckVersion(const Property &property, bool &isDbExist) const
{
    int dbVer = 0;
    int errCode = GetVersion(property, dbVer, isDbExist);
    if (errCode != E_OK) {
        LOGE("[DataStorage][CheckVer] GetVersion failed, errCode=%d.", errCode);
        return errCode;
    }
    if (!isDbExist) {
        return E_OK;
    }
    LOGD("[DataStorage][CheckVer] DbVersion=%d, CurVersion=%d.", dbVer, MULTI_VER_DATA_STORAGE_VERSION_CURRENT);
    if (dbVer > MULTI_VER_DATA_STORAGE_VERSION_CURRENT) {
        LOGE("[DataStorage][CheckVer] Version Not Support!");
        return -E_VERSION_NOT_SUPPORT;
    }
    return E_OK;
}

int SQLiteMultiVerDataStorage::GetVersion(const Property &property, int &version, bool &isDbExisted) const
{
    std::string uri = property.path + "/" + property.identifierName + "/" + DBConstant::MULTI_SUB_DIR + "/" +
        DBConstant::MULTI_VER_DATA_STORE + DBConstant::SQLITE_DB_EXTENSION;
    isDbExisted = OS::CheckPathExistence(uri);
    if (isDbExisted) {
        std::vector<std::string> tableVect;
        OpenDbProperties option = {uri, property.isNeedCreate, false, tableVect, property.cipherType, property.passwd};
        return SQLiteUtils::GetVersion(option, version);
    }
    return E_OK;
}

int SQLiteMultiVerDataStorage::Open(const Property &property)
{
    // only set the property para or create the database and the table?
    // whether create the transactions.
    property_ = property;
    uri_ = property.path + "/" + property_.identifierName + "/" + DBConstant::MULTI_SUB_DIR + "/" +
        DBConstant::MULTI_VER_DATA_STORE + DBConstant::SQLITE_DB_EXTENSION;
    std::vector<std::string> tableVect;
    tableVect.push_back(CREATE_TABLE_SQL);

    OpenDbProperties option = {uri_, property.isNeedCreate, false, tableVect, property.cipherType, property.passwd};
    sqlite3 *db = nullptr;
    int errCode = SQLiteUtils::OpenDatabase(option, db);
    if (errCode != E_OK) {
        LOGE("Open the multi ver data store error:%d", errCode);
        goto END;
    }

    // Version had been check before open and currently no upgrade to do
    errCode = SQLiteUtils::SetUserVer(option, MULTI_VER_DATA_STORAGE_VERSION_CURRENT);
    if (errCode != E_OK) {
        LOGE("Init the version multi ver store error:%d", errCode);
    }

END:
    if (db != nullptr) {
        (void)sqlite3_close_v2(db);
        db = nullptr;
    }

    return errCode;
}

// start one write transaction
// do the transaction initialization and call the start transaction;
int SQLiteMultiVerDataStorage::StartWrite(KvDataType dataType, IKvDBMultiVerTransaction *&transaction)
{
    (void)dataType;
    std::unique_lock<std::mutex> lock(transactionMutex_);
    // if same thread. return nullptr.
    if (std::this_thread::get_id() == writeHolderId_) {
        transaction = nullptr;
        return -E_BUSY;
    }

    if (writeTransaction_ != nullptr) {
        writeCondition_.wait(lock, [&] {
            return !writeTransactionUsed_;
        });

        writeTransactionUsed_ = true;
        writeHolderId_ = std::this_thread::get_id();
        transaction = writeTransaction_;
        return E_OK;
    }

    transaction = new (std::nothrow) SQLiteMultiVerTransaction();
    if (transaction == nullptr) {
        LOGE("Failed to create the SQLite write transaction");
        return -E_OUT_OF_MEMORY;
    }

    // initialize the transaction.
    int errCode = static_cast<SQLiteMultiVerTransaction *>(transaction)->Initialize(uri_, false,
        property_.cipherType, property_.passwd);
    if (errCode != E_OK) {
        LOGE("Init write transaction failed:%d", errCode);
        delete transaction;
        transaction = nullptr;
        return errCode;
    }

    writeTransaction_ = static_cast<SQLiteMultiVerTransaction *>(transaction);
    writeTransactionUsed_ = true;
    writeHolderId_ = std::this_thread::get_id();
    return E_OK;
}

// do the first step of commit record.
// commit the transaction, and avoid other operation reading the new data.
int SQLiteMultiVerDataStorage::CommitWritePhaseOne(IKvDBMultiVerTransaction *transaction,
    const UpdateVerTimeStamp &multiVerTimeStamp)
{
    if (transaction == nullptr) {
        LOGE("Invalid transaction!");
        return -E_INVALID_DB;
    }
    // Get versionInfo from transaction.
    // Call the commit of the sqlite.
    Version versionInfo = transaction->GetVersion();

    if (multiVerTimeStamp.isNeedUpdate) {
        (void)transaction->UpdateTimestampByVersion(versionInfo, multiVerTimeStamp.timestamp);
    }

    int errCode = transaction->CommitTransaction();
    if (errCode != E_OK) {
        auto sqliteTransaction = static_cast<SQLiteMultiVerTransaction *>(transaction);
        if (transaction != nullptr) {
            (void)sqliteTransaction->Reset(property_.cipherType, property_.passwd);
        }
        LOGE("SQLite commit the transaction failed:%d", errCode);
    }
    return errCode;
}

// when the commit history update failed, need delete the commit
int SQLiteMultiVerDataStorage::RollbackWritePhaseOne(IKvDBMultiVerTransaction *transaction,
    const Version &versionInfo)
{
    if (transaction == nullptr) {
        LOGE("Invalid transaction!");
        return -E_INVALID_DB;
    }

    SQLiteMultiVerTransaction *sqliteTransaction = static_cast<SQLiteMultiVerTransaction *>(transaction);
    sqliteTransaction->StartTransaction();
    int errCode = sqliteTransaction->ClearEntriesByVersion(versionInfo);
    if (errCode == E_OK) {
        sqliteTransaction->CommitTransaction();
    } else {
        sqliteTransaction->RollBackTransaction();
    }

    return errCode;
}

// Rollback the write transaction.
int SQLiteMultiVerDataStorage::RollbackWrite(IKvDBMultiVerTransaction *transaction)
{
    if (transaction == nullptr) {
        LOGE("Invalid transaction!");
        return -E_INVALID_DB;
    }
    // call the rollback of the sqlite.
    int errCode = static_cast<SQLiteMultiVerTransaction *>(transaction)->RollBackTransaction();
    if (errCode != E_OK) {
        (void)static_cast<SQLiteMultiVerTransaction *>(transaction)->Reset(property_.cipherType, property_.passwd);
        LOGE("SQLite rollback failed:%d", errCode);
    }
    return errCode;
}

// should update the flag indicated that other operating could read the new record.
void SQLiteMultiVerDataStorage::CommitWritePhaseTwo(IKvDBMultiVerTransaction *transaction)
{
    // just change the head version?
}

// Get one start transaction.
IKvDBMultiVerTransaction *SQLiteMultiVerDataStorage::StartRead(KvDataType dataType,
    const Version &versionInfo, int &errCode)
{
    (void)dataType;
    std::unique_lock<std::mutex> lock(transactionMutex_);
    for (auto &iter : readTransactions_) {
        if (iter.second) {
            iter.second = false;
            (iter.first)->SetVersion(versionInfo);
            errCode = E_OK;
            return iter.first;
        }
    }
    // need wait.
    if (readTransactions_.size() > MAX_READ_CONNECT_NUM) {
        LOGE("Over the max transaction num");
        errCode = -E_BUSY;
        return nullptr;
    }

    IKvDBMultiVerTransaction *transaction = new (std::nothrow) SQLiteMultiVerTransaction;
    if (transaction == nullptr) {
        errCode = -E_OUT_OF_MEMORY;
        return nullptr;
    }
    errCode = static_cast<SQLiteMultiVerTransaction *>(transaction)->Initialize(uri_,
        true, property_.cipherType, property_.passwd);
    if (errCode != E_OK) {
        delete transaction;
        transaction = nullptr;
        return nullptr;
    }

    transaction->SetVersion(versionInfo);
    readTransactions_.insert(std::make_pair(transaction, false));
    return transaction;
}

// Release the transaction created.
void SQLiteMultiVerDataStorage::ReleaseTransaction(IKvDBMultiVerTransaction *transaction)
{
    // whether need manage the transaction.
    std::unique_lock<std::mutex> lock(transactionMutex_);
    if (transaction == nullptr) {
        LOGE("Invalid transaction!");
        return;
    }

    if (transaction == writeTransaction_) {
        static_cast<SQLiteMultiVerTransaction *>(writeTransaction_)->ResetVersion();
        writeTransactionUsed_ = false;
        writeHolderId_ = std::thread::id();
        writeCondition_.notify_all();
        return;
    }

    auto iter = readTransactions_.find(transaction);
    if (iter != readTransactions_.end()) {
        static_cast<SQLiteMultiVerTransaction *>(iter->first)->ResetVersion();
        iter->second = true;
    }
    return;
}

void SQLiteMultiVerDataStorage::Close()
{
    std::lock_guard<std::mutex> lock(transactionMutex_);
    // close all the transaction?
    for (auto iter = readTransactions_.begin(); iter != readTransactions_.end(); iter++) {
        if (iter->first != nullptr) {
            delete iter->first;
        }
    }
    readTransactions_.clear();

    if (writeTransaction_ != nullptr) {
        delete writeTransaction_;
        writeTransaction_ = nullptr;
    }
}

int SQLiteMultiVerDataStorage::RunRekeyLogic(CipherType type, const CipherPassword &passwd)
{
    (void)type;
    // openDatabase to get the sqlite3 pointer
    std::vector<std::string> tableVect;
    tableVect.push_back(CREATE_TABLE_SQL);
    sqlite3 *db = nullptr;
    OpenDbProperties option = {uri_, property_.isNeedCreate, false, tableVect, property_.cipherType, property_.passwd};
    int errCode = SQLiteUtils::OpenDatabase(option, db);
    if (errCode != E_OK) {
        LOGE("Open db error:%d", errCode);
        return errCode;
    }

    // execute rekey
    errCode = SQLiteUtils::Rekey(db, passwd);
    if (errCode != E_OK) {
        LOGE("multi ver data rekey failed:%d", errCode);
    }
    // close db
    (void)sqlite3_close_v2(db);
    db = nullptr;

    return errCode;
}

int SQLiteMultiVerDataStorage::RunExportLogic(CipherType type, const CipherPassword &passwd, const std::string &dbDir)
{
    // openDatabase to get the sqlite3 pointer
    std::vector<std::string> tableVect;
    sqlite3 *db = nullptr;
    OpenDbProperties option = {uri_, true, false, tableVect, property_.cipherType, property_.passwd};
    int errCode = SQLiteUtils::OpenDatabase(option, db);
    if (errCode != E_OK) {
        LOGE("Open db error:%d", errCode);
        return errCode;
    }

    // execute export
    std::string newDbName = dbDir + "/" + DBConstant::MULTI_VER_DATA_STORE + DBConstant::SQLITE_DB_EXTENSION;
    errCode = SQLiteUtils::ExportDatabase(db, type, passwd, newDbName);
    if (errCode != E_OK) {
        LOGE("multi ver data export failed:%d", errCode);
    }
    // close db
    (void)sqlite3_close_v2(db);
    db = nullptr;

    return errCode;
}

int SQLiteMultiVerDataStorage::BackupCurrentDatabase(const Property &property, const std::string &dir)
{
    std::string currentDb = property.path + "/" + property.identifierName + "/" + DBConstant::MULTI_SUB_DIR + "/" +
        DBConstant::MULTI_VER_DATA_STORE + DBConstant::SQLITE_DB_EXTENSION;
    std::string dstDb = dir + "/" + DBConstant::MULTI_VER_DATA_STORE + DBConstant::SQLITE_DB_EXTENSION;
    int errCode = DBCommon::CopyFile(currentDb, dstDb);
    if (errCode != E_OK) {
        LOGE("Copy the local current db error:%d", errCode);
    }
    return errCode;
}

int SQLiteMultiVerDataStorage::ImportDatabase(const Property &property, const std::string &dir,
    const CipherPassword &passwd)
{
    std::string currentDb = property.path + "/" + property.identifierName + "/" + DBConstant::MULTI_SUB_DIR + "/" +
        DBConstant::MULTI_VER_DATA_STORE + DBConstant::SQLITE_DB_EXTENSION;
    std::string srcDb = dir + "/" + DBConstant::MULTI_VER_DATA_STORE + DBConstant::SQLITE_DB_EXTENSION;
    int errCode = SQLiteUtils::ExportDatabase(srcDb, property.cipherType, passwd, currentDb, property.passwd);
    if (errCode != E_OK) {
        LOGE("import the multi ver data db error:%d", errCode);
    }
    return E_OK;
}
} // namespace DistributedDB
#endif