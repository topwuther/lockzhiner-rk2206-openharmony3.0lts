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

#include "sqlite_local_kvdb_connection.h"

#include <cstring>

#include "log_print.h"
#include "db_constant.h"
#include "sqlite_utils.h"
#include "sqlite_local_kvdb.h"
#include "sqlite_local_kvdb_snapshot.h"
#include "kvdb_commit_notify_filterable_data.h"
#include "sqlite_local_storage_executor.h"

namespace DistributedDB {
SQLiteLocalKvDBConnection::SQLiteLocalKvDBConnection(SQLiteLocalKvDB *kvDB)
    : GenericKvDBConnection(kvDB),
      writeHandle_(nullptr)
{}

SQLiteLocalKvDBConnection::~SQLiteLocalKvDBConnection()
{}

int SQLiteLocalKvDBConnection::Get(const IOption &option, const Key &key, Value &value) const
{
    if (kvDB_ == nullptr) {
        return -E_INVALID_DB;
    }
    if (key.empty() || key.size() > DBConstant::MAX_KEY_SIZE) {
        return -E_INVALID_ARGS;
    }
    {
        std::lock_guard<std::mutex> lock(transactionMutex_);
        if (writeHandle_ != nullptr) {
            return writeHandle_->Get(key, value);
        }
    }
    int errCode = E_OK;
    SQLiteLocalStorageExecutor *handle = GetDB<SQLiteLocalKvDB>()->GetHandle(false, errCode);
    if (handle == nullptr) {
        return errCode;
    }

    errCode = handle->Get(key, value);
    GetDB<SQLiteLocalKvDB>()->ReleaseHandle(handle);
    return errCode;
}

int SQLiteLocalKvDBConnection::Put(const IOption &option, const Key &key, const Value &value)
{
    int errCode = CheckDataStatus(key, value, false);
    if (errCode != E_OK) {
        return errCode;
    }
    std::lock_guard<std::mutex> lock(transactionMutex_);
    bool isAuto = false;
    errCode = StartTransactionInner(isAuto);
    if (errCode != E_OK) {
        LOGE("StartTransaction failed when Put error:%d", errCode);
        return errCode;
    }

    errCode = writeHandle_->Put(key, value);
    if (errCode != E_OK) {
        if (isAuto) {
            int errCodeRollBack = RollBackInner();
            LOGI("Put failed,need rollback! errCode:[%d]", errCodeRollBack);
        }
        return errCode;
    }
    if (isAuto) {
        errCode = CommitInner();
        if (errCode != E_OK) {
            LOGE("CommitTransaction failed when Put error:%d", errCode);
            return errCode;
        }
    }

    return errCode;
}

int SQLiteLocalKvDBConnection::Delete(const IOption &option, const Key &key)
{
    int errCode = CheckDataStatus(key, {}, true);
    if (errCode != E_OK) {
        return errCode;
    }
    std::lock_guard<std::mutex> lock(transactionMutex_);
    bool isAuto = false;
    errCode = StartTransactionInner(isAuto);
    if (errCode != E_OK) {
        LOGE("StartTransaction failed when Delete error:%d", errCode);
        return errCode;
    }

    errCode = writeHandle_->Delete(key);
    if (errCode != E_OK) {
        if (isAuto) {
            int errCodeRollBack = RollBackInner();
            LOGI("Delete failed, need rollback! errcode:[%d]", errCodeRollBack);
        }
        return errCode;
    }

    if (isAuto) {
        errCode = CommitInner();
        if (errCode != E_OK) {
            LOGE("CommitInner failed while delete:%d", errCode);
            return errCode;
        }
    }
    return E_OK;
}

int SQLiteLocalKvDBConnection::Clear(const IOption &option)
{
    std::lock_guard<std::mutex> lock(transactionMutex_);
    bool isAuto = false;
    int errCode = StartTransactionInner(isAuto);
    if (errCode != E_OK) {
        LOGE("StartTransaction failed when Clear error:%d", errCode);
        return errCode;
    }

    errCode = writeHandle_->Clear();
    if (errCode != E_OK) {
        if (isAuto) {
            int errCodeRollBack = RollBackInner();
            LOGI("Clear failed, need rollback! RollBack result is [%d]", errCodeRollBack);
        }
        return errCode;
    }

    if (isAuto) {
        errCode = CommitInner();
        if (errCode != E_OK) {
            LOGE("CommitInner failed when Clear error:%d", errCode);
            return errCode;
        }
    }

    return E_OK;
}

int SQLiteLocalKvDBConnection::GetEntries(const IOption &option, const Key &keyPrefix,
    std::vector<Entry> &entries) const
{
    if (kvDB_ == nullptr) {
        return -E_INVALID_DB;
    }
    if (keyPrefix.size() > DBConstant::MAX_KEY_SIZE) {
        return -E_INVALID_ARGS;
    }
    {
        std::lock_guard<std::mutex> lock(transactionMutex_);
        if (writeHandle_ != nullptr) {
            return writeHandle_->GetEntries(keyPrefix, entries);
        }
    }
    int errCode = E_OK;
    SQLiteLocalStorageExecutor *handle = GetDB<SQLiteLocalKvDB>()->GetHandle(false, errCode);
    if (handle == nullptr) {
        return errCode;
    }
    errCode = handle->GetEntries(keyPrefix, entries);
    GetDB<SQLiteLocalKvDB>()->ReleaseHandle(handle);
    return errCode;
}

int SQLiteLocalKvDBConnection::PutBatch(const IOption &option, const std::vector<Entry> &entries)
{
    if (entries.empty() || entries.size() > DBConstant::MAX_BATCH_SIZE) {
        return -E_INVALID_ARGS;
    }
    for (const auto &item : entries) {
        if (CheckDataStatus(item.key, item.value, false) != E_OK) {
            return -E_INVALID_ARGS;
        }
    }

    bool isAuto = false;
    std::lock_guard<std::mutex> lock(transactionMutex_);
    int errCode = StartTransactionInner(isAuto);
    if (errCode != E_OK) {
        LOGE("StartTransaction failed when PutBatch error:%d", errCode);
        return errCode;
    }

    for (const auto &entry : entries) {
        // first argument is key and second argument is value.
        errCode = writeHandle_->Put(entry.key, entry.value);
        if (errCode != E_OK) {
            if (isAuto) {
                int errCodeRollBack = RollBackInner();
                LOGI("PutBatch failed,need rollback! RollBack result is %d", errCodeRollBack);
            }
            return errCode;
        }
    }

    if (isAuto) {
        errCode = CommitInner();
        if (errCode != E_OK) {
            LOGE("CommitTransaction failed when PutBatch error:%d", errCode);
            return errCode;
        }
    }

    return E_OK;
}

int SQLiteLocalKvDBConnection::DeleteBatch(const IOption &option, const std::vector<Key> &keys)
{
    if (keys.empty() || keys.size() > DBConstant::MAX_BATCH_SIZE) {
        LOGE("[Local]DeleteBatch size[%zu]!", keys.size());
        return -E_INVALID_ARGS;
    }
    for (const auto &item : keys) {
        if (item.empty() || item.size() > DBConstant::MAX_KEY_SIZE) {
            return -E_INVALID_ARGS;
        }
    }

    bool isAuto = false;
    std::lock_guard<std::mutex> lock(transactionMutex_);
    int errCode = StartTransactionInner(isAuto);
    if (errCode != E_OK) {
        LOGE("StartTransaction failed when DeleteBatch error:%d", errCode);
        return errCode;
    }

    errCode = writeHandle_->DeleteBatch(keys);

    if (isAuto) {
        if (errCode == E_OK) {
            errCode = CommitInner();
            if (errCode != E_OK) {
                LOGE("CommitTransaction failed when DeleteBatch error:%d", errCode);
                return errCode;
            }
        } else {
            int errCodeRollBack = RollBackInner();
            LOGI("DeleteBatchm need rollback! RollBack result is [%d]", errCodeRollBack);
            return errCode;
        }
    }

    return errCode;
}

// when GetSnapshot successfully, you must delete snapshot by ReleaseSnapshot
int SQLiteLocalKvDBConnection::GetSnapshot(IKvDBSnapshot *&snapshot) const
{
    if (kvDB_ == nullptr) {
        snapshot = nullptr;
        return -E_INVALID_DB;
    }

    int errCode = E_OK;
    IKvDBConnection *newConnect = kvDB_->GetDBConnection(errCode);
    if (errCode != E_OK) {
        LOGE("failed to get the new connection");
        return errCode;
    }

    SQLiteLocalKvDBSnapshot *dbSnapshot = new (std::nothrow) SQLiteLocalKvDBSnapshot(newConnect);
    if (dbSnapshot == nullptr) {
        newConnect->Close();
        delete newConnect;
        return -E_OUT_OF_MEMORY;
    }

    snapshot = dbSnapshot;
    {
        std::lock_guard<std::mutex> lock(snapshotMutex_);
        snapshots_.insert(dbSnapshot);
    }

    return E_OK;
}

void SQLiteLocalKvDBConnection::ReleaseSnapshot(IKvDBSnapshot *&snapshot)
{
    if (snapshot != nullptr && kvDB_ != nullptr) {
        std::lock_guard<std::mutex> lock(snapshotMutex_);
        SQLiteLocalKvDBSnapshot *sqliteSnapshot = static_cast<SQLiteLocalKvDBSnapshot *>(snapshot);
        sqliteSnapshot->Close();
        snapshots_.erase(snapshot);
        delete snapshot;
        snapshot = nullptr;
    }
}

int SQLiteLocalKvDBConnection::StartTransaction()
{
    std::lock_guard<std::mutex> lock(transactionMutex_);
    if (writeHandle_ != nullptr) {
        return -E_TRANSACT_STATE;
    }
    bool isAuto = false;
    return StartTransactionInner(isAuto);
}

int SQLiteLocalKvDBConnection::Commit()
{
    std::lock_guard<std::mutex> lock(transactionMutex_);
    return CommitInner();
}

int SQLiteLocalKvDBConnection::RollBack()
{
    std::lock_guard<std::mutex> lock(transactionMutex_);
    return RollBackInner();
}

bool SQLiteLocalKvDBConnection::IsTransactionStarted() const
{
    std::lock_guard<std::mutex> lock(transactionMutex_);
    if (writeHandle_ != nullptr) {
        return true;
    }
    return false;
}

int SQLiteLocalKvDBConnection::PreClose()
{
    {
        std::lock_guard<std::mutex> snapshotLock(snapshotMutex_);
        if (snapshots_.size() != 0) {
            LOGE("Close failed, the connection have unreleased snapshot.");
            return -E_BUSY;
        }
    }
    std::lock_guard<std::mutex> transactionLock(transactionMutex_);
    if (writeHandle_ != nullptr) {
        writeHandle_->RollBack();
        GetDB<SQLiteLocalKvDB>()->ReleaseHandle(writeHandle_);
    }
    return E_OK;
}

int SQLiteLocalKvDBConnection::TranslateObserverModeToEventTypes(unsigned mode,
    std::list<int> &eventTypes) const
{
    return E_OK;
}

int SQLiteLocalKvDBConnection::StartTransactionInner(bool &isAuto)
{
    // if the transaction has been started, writeHandle wouldn't be nullptr.
    if (writeHandle_ != nullptr) {
        return E_OK;
    }

    if (kvDB_ == nullptr) {
        LOGE("local database is null");
        return -E_INVALID_DB;
    }

    int errCode = E_OK;
    SQLiteLocalStorageExecutor *handle = GetDB<SQLiteLocalKvDB>()->GetHandle(true, errCode);
    if (handle == nullptr) {
        return errCode;
    }

    errCode = handle->StartTransaction();
    if (errCode != E_OK) {
        GetDB<SQLiteLocalKvDB>()->ReleaseHandle(handle);
        return errCode;
    }
    writeHandle_ = handle;
    // only the transaction has not been started before, set the flag to true.
    // the manual operation would ignore the flag.
    isAuto = true;
    return E_OK;
}

int SQLiteLocalKvDBConnection::CommitInner()
{
    if (writeHandle_ == nullptr) {
        LOGE("local database is null or the transaction has not been started");
        return -E_INVALID_DB;
    }

    int errCode = writeHandle_->Commit();
    if (kvDB_ == nullptr) {
        return -E_INVALID_DB;
    }
    GetDB<SQLiteLocalKvDB>()->ReleaseHandle(writeHandle_);
    return errCode;
}

int SQLiteLocalKvDBConnection::RollBackInner()
{
    if (writeHandle_ == nullptr) {
        LOGE("Invalid handle for rollback or the transaction has not been started.");
        return -E_INVALID_DB;
    }

    int errCode = writeHandle_->RollBack();
    if (kvDB_ == nullptr) {
        return -E_INVALID_DB;
    }
    GetDB<SQLiteLocalKvDB>()->ReleaseHandle(writeHandle_);
    return errCode;
}

int SQLiteLocalKvDBConnection::Rekey(const CipherPassword &passwd)
{
    if (kvDB_ == nullptr) {
        return -E_INVALID_DB;
    }
    std::lock_guard<std::mutex> lock(transactionMutex_);
    // return BUSY if in transaction
    if (writeHandle_ != nullptr) {
        LOGE("Transaction exists for rekey failed");
        return -E_BUSY;
    }
    // Check the connection number.
    int errCode = kvDB_->TryToDisableConnection(OperatePerm::REKEY_MONOPOLIZE_PERM);
    if (errCode != E_OK) {
        return errCode;
    }

    // Check the observer.
    errCode = GenericKvDBConnection::PreCheckExclusiveStatus();
    if (errCode != E_OK) {
        kvDB_->ReEnableConnection(OperatePerm::REKEY_MONOPOLIZE_PERM);
        return errCode;
    }
    // If only have one connection, just have the transactionMutex_;
    // It means there would not be another operation on this connection.
    errCode = kvDB_->Rekey(passwd);

    GenericKvDBConnection::ResetExclusiveStatus();
    kvDB_->ReEnableConnection(OperatePerm::REKEY_MONOPOLIZE_PERM);
    return errCode;
}

int SQLiteLocalKvDBConnection::Export(const std::string &filePath, const CipherPassword &passwd)
{
    if (kvDB_ == nullptr) {
        return -E_INVALID_DB;
    }
    return kvDB_->Export(filePath, passwd);
}

int SQLiteLocalKvDBConnection::Import(const std::string &filePath, const CipherPassword &passwd)
{
    if (kvDB_ == nullptr) {
        return -E_INVALID_DB;
    }
    {
        std::lock_guard<std::mutex> lock(transactionMutex_);
        // return BUSY if in transaction
        if (writeHandle_ != nullptr) {
            LOGE("Transaction exists for rekey failed");
            return -E_BUSY;
        }
    }
    std::lock_guard<std::mutex> importLock(importMutex_);
    int errCode = kvDB_->TryToDisableConnection(OperatePerm::IMPORT_MONOPOLIZE_PERM);
    if (errCode != E_OK) {
        return errCode;
    }

    errCode = GenericKvDBConnection::PreCheckExclusiveStatus();
    if (errCode != E_OK) {
        kvDB_->ReEnableConnection(OperatePerm::IMPORT_MONOPOLIZE_PERM);
        return errCode;
    }
    errCode = kvDB_->Import(filePath, passwd);
    GenericKvDBConnection::ResetExclusiveStatus();
    kvDB_->ReEnableConnection(OperatePerm::IMPORT_MONOPOLIZE_PERM);
    return errCode;
}

int SQLiteLocalKvDBConnection::CheckDataStatus(const Key &key, const Value &value, bool isDeleted) const
{
    if (kvDB_ == nullptr) {
        return -E_INVALID_DB;
    }
    return static_cast<SQLiteLocalKvDB *>(kvDB_)->CheckDataStatus(key, value, isDeleted);
}
} // namespace DistributedDB
