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
#include "multi_ver_natural_store_connection.h"

#include <vector>
#include <cstdlib>
#include <ctime>

#include "log_print.h"
#include "db_errno.h"
#include "db_constant.h"
#include "multi_ver_natural_store_snapshot.h"
#include "multi_ver_natural_store.h"

namespace DistributedDB {
MultiVerNaturalStoreConnection::MultiVerNaturalStoreConnection(MultiVerNaturalStore *kvDB)
    : SyncAbleKvDBConnection(kvDB),
      writeHandle_(nullptr)
{}

MultiVerNaturalStoreConnection::~MultiVerNaturalStoreConnection()
{
    writeHandle_ = nullptr;
}

// Get the value from the database
int MultiVerNaturalStoreConnection::Get(const IOption &option, const Key &key, Value &value) const
{
    int errCode = CheckDataStatus(key, {}, false);
    if (errCode != E_OK) {
        return errCode;
    }
    {
        // Only for the read in the write transaction
        std::lock_guard<std::mutex> lock(writeMutex_);
        if (writeHandle_ != nullptr) {
            return writeHandle_->Get(key, value);
        }
    }

    auto handle = GetHandle(false, errCode);
    if (handle == nullptr) {
        return errCode;
    }

    errCode = handle->InitCurrentReadVersion();
    if (errCode == E_OK) {
        errCode = handle->Get(key, value);
    }

    GetDB<MultiVerNaturalStore>()->ReleaseHandle(handle);
    return errCode;
}

// Put the value to the database
int MultiVerNaturalStoreConnection::Put(const IOption &option, const Key &key, const Value &value)
{
    bool isAuto = false;
    int errCode = CheckDataStatus(key, value, false);
    if (errCode != E_OK) {
        return errCode;
    }

    std::lock_guard<std::mutex> lock(writeMutex_);
    errCode = StartTransactionInner(isAuto);
    if (errCode != E_OK) {
        LOGE("Start transaction failed:%d", errCode);
        return errCode;
    }

    errCode = writeHandle_->Put(key, value);
    if (errCode != E_OK) {
        LOGE("Put value err:%d", errCode);
        if (isAuto) {
            (void)(RollBackTransactionInner());
        }
        return errCode;
    }

    if (isAuto) {
        errCode = CommitTransactionInner();
    }

    return errCode;
}

// Delete the value from the database
int MultiVerNaturalStoreConnection::Delete(const IOption &option, const Key &key)
{
    int errCode = CheckDataStatus(key, {}, true);
    if (errCode != E_OK) {
        return errCode;
    }
    bool isAuto = false;
    std::lock_guard<std::mutex> lock(writeMutex_);
    errCode = StartTransactionInner(isAuto);
    if (errCode != E_OK) {
        LOGE("start transaction to delete failed:%d", errCode);
        return errCode;
    }

    errCode = writeHandle_->Delete(key);
    if (errCode != E_OK) {
        if (isAuto) {
            int rollbackErrCode = RollBackTransactionInner();
            LOGE("Connection Delete fail, rollback(state:%d) transaction!", rollbackErrCode);
        }
        return errCode;
    }

    if (isAuto) {
        errCode = CommitTransactionInner();
    }
    return errCode;
}

// Clear all the data from the database
int MultiVerNaturalStoreConnection::Clear(const IOption &option)
{
    bool isAuto = false;
    std::lock_guard<std::mutex> lock(writeMutex_);
    int errCode = StartTransactionInner(isAuto);
    if (errCode != E_OK) {
        LOGE("start transaction to clear failed:%d", errCode);
        return errCode;
    }

    errCode = writeHandle_->Clear();
    if (errCode != E_OK) {
        if (isAuto) {
            int rollbackErrCode = RollBackTransactionInner();
            LOGD("Connection Clear, rollback(state:%d) transaction!", rollbackErrCode);
        }
        return errCode;
    }

    if (isAuto) {
        errCode = CommitTransactionInner();
    }
    return errCode;
}

// Get all the data from the database
int MultiVerNaturalStoreConnection::GetEntries(const IOption &option,
    const Key &keyPrefix, std::vector<Entry> &entries) const
{
    {
        std::lock_guard<std::mutex> lock(writeMutex_);
        if (writeHandle_ != nullptr) {
            return writeHandle_->GetEntries(keyPrefix, entries);
        }
    }

    int errCode = E_OK;
    auto handle = GetHandle(false, errCode);
    if (handle == nullptr) {
        return errCode;
    }
    errCode = handle->GetEntries(keyPrefix, entries);
    GetDB<MultiVerNaturalStore>()->ReleaseHandle(handle);
    return errCode;
}

// Put the batch values to the database.
int MultiVerNaturalStoreConnection::PutBatch(const IOption &option, const std::vector<Entry> &entries)
{
    bool isAuto = false;
    if (entries.empty() || entries.size() > DBConstant::MAX_BATCH_SIZE) {
        return -E_INVALID_ARGS;
    }
    for (const auto &item : entries) {
        if (CheckDataStatus(item.key, item.value, false) != E_OK) {
            return -E_INVALID_ARGS;
        }
    }

    std::lock_guard<std::mutex> lock(writeMutex_);

    // if the transaction is not started auto
    int errCode = StartTransactionInner(isAuto);
    if (errCode != E_OK) {
        LOGE("start transaction failed:%d", errCode);
        return errCode;
    }

    for (const auto &item : entries) {
        errCode = writeHandle_->Put(item.key, item.value);
        if (errCode != E_OK) {
            if (isAuto) {
                (void)(RollBackTransactionInner());
            }
            return errCode;
        }
    }

    if (isAuto) {
        errCode = CommitTransactionInner();
    }
    return errCode;
}

// Delete the batch values from the database.
int MultiVerNaturalStoreConnection::DeleteBatch(const IOption &option, const std::vector<Key> &keys)
{
    if (keys.empty() || keys.size() > DBConstant::MAX_BATCH_SIZE) {
        LOGE("[MultiVer]DeleteBatch size[%zu]!", keys.size());
        return -E_INVALID_ARGS;
    }
    if (!CheckDeletedKeys(keys)) {
        return -E_INVALID_ARGS;
    }
    bool isAuto = false;
    std::lock_guard<std::mutex> lock(writeMutex_);
    // if the transaction is not started auto
    int errCode = StartTransactionInner(isAuto);
    if (errCode != E_OK) {
        LOGE("Start transaction failed:%d", errCode);
        return errCode;
    }

    // delete automatic
    bool needCommit = false;
    for (const auto &item : keys) {
        errCode = writeHandle_->Delete(item);
        if (errCode == E_OK) {
            needCommit = true;
        } else if (errCode == -E_NOT_FOUND) {
            errCode = E_OK;
        } else {
            if (isAuto) {
                (void)(RollBackTransactionInner());
            }
            LOGE("Delete failed:%d", errCode);
            return errCode;
        }
    }

    if (isAuto) {
        if (needCommit) {
            errCode = CommitTransactionInner();
        } else {
            (void)(RollBackTransactionInner());
            errCode = -E_NOT_FOUND;
        }
    } else {
        errCode = needCommit ? E_OK : -E_NOT_FOUND;
    }
    return errCode;
}

int MultiVerNaturalStoreConnection::GetSnapshot(IKvDBSnapshot *&snapshot) const
{
    if (kvDB_ == nullptr) {
        return -E_INVALID_DB;
    }
    int errCode = E_OK;
    auto handle = GetHandle(false, errCode);
    if (handle == nullptr) {
        LOGE("Get the handle for snapshot failed:%d", errCode);
        return errCode;
    }

    errCode = handle->InitCurrentReadVersion();
    if (errCode != E_OK) {
        LOGE("Init the handle version for snapshot failed:%d", errCode);
        GetDB<MultiVerNaturalStore>()->ReleaseHandle(handle);
        return errCode;
    }

    snapshot = new (std::nothrow) MultiVerNaturalStoreSnapshot(handle);
    if (snapshot == nullptr) {
        GetDB<MultiVerNaturalStore>()->ReleaseHandle(handle);
        return -E_OUT_OF_MEMORY;
    }

    std::lock_guard<std::mutex> lock(snapshotMutex_);
    snapshots_.insert(snapshot);
    GetDB<MultiVerNaturalStore>()->AddVersionConstraintToList(handle->GetCurrentReadVersion());
    return E_OK;
}

// Release the created snapshot
void MultiVerNaturalStoreConnection::ReleaseSnapshot(IKvDBSnapshot *&snapshot)
{
    if (snapshot == nullptr) {
        return;
    }

    std::lock_guard<std::mutex> lock(snapshotMutex_);
    static_cast<MultiVerNaturalStoreSnapshot *>(snapshot)->Close();
    snapshots_.erase(snapshot);
    delete snapshot;
    snapshot = nullptr;
    return;
}

// Start the transaction
int MultiVerNaturalStoreConnection::StartTransaction()
{
    // Get the state of the transaction.
    std::lock_guard<std::mutex> lock(writeMutex_);
    if (writeHandle_ != nullptr) {
        LOGE("Transaction is already running");
        return -E_TRANSACT_STATE;
    }
    bool isAuto = false;
    return StartTransactionInner(isAuto);
}

// Commit the transaction
int MultiVerNaturalStoreConnection::Commit()
{
    std::lock_guard<std::mutex> lock(writeMutex_);
    return CommitTransactionInner();
}

// Roll back the transaction
int MultiVerNaturalStoreConnection::RollBack()
{
    std::lock_guard<std::mutex> lock(writeMutex_);
    return RollBackTransactionInner();
}

bool MultiVerNaturalStoreConnection::IsTransactionStarted() const
{
    std::lock_guard<std::mutex> lock(writeMutex_);
    if (writeHandle_ != nullptr) {
        return true;
    }
    return false;
}

// Close and delete the connection.
int MultiVerNaturalStoreConnection::PreClose()
{
    std::lock_guard<std::mutex> snapshotLock(snapshotMutex_);
    if (snapshots_.size() > 0) {
        LOGE("the connection have unreleased snapshot, should not close.");
        return -E_BUSY;
    }

    std::lock_guard<std::mutex> writeLock(writeMutex_);
    if (writeHandle_ != nullptr) {
        LOGE("the connection have transaction, should not close.");
        (void)(RollBackTransactionInner());
    }
    return E_OK;
}

// Commit Transaction for local change data
int MultiVerNaturalStoreConnection::CommitTransactionInner()
{
    if (kvDB_ == nullptr) {
        return -E_INVALID_DB;
    }
    // Get the state of the transaction.
    if (writeHandle_ == nullptr) {
        LOGE("Transaction has not been started.");
        return -E_TRANSACT_STATE;
    }

    int errCode = writeHandle_->CommitTransaction();
    GetDB<MultiVerNaturalStore>()->ReleaseHandle(writeHandle_);

    return errCode;
}

// If the transaction is started automatically, should roll back automatically
int MultiVerNaturalStoreConnection::RollBackTransactionInner()
{
    if (kvDB_ == nullptr) {
        return -E_INVALID_DB;
    }
    if (writeHandle_ == nullptr) {
        return -E_TRANSACT_STATE;
    }

    int errCode = writeHandle_->RollBackTransaction();
    GetDB<MultiVerNaturalStore>()->ReleaseHandle(writeHandle_);

    return errCode;
}

int MultiVerNaturalStoreConnection::StartTransactionInner(bool &isAuto)
{
    if (kvDB_ == nullptr) {
        return -E_INVALID_DB;
    }
    isAuto = false;
    if (writeHandle_ != nullptr) {
        return E_OK;
    }

    int errCode = E_OK;
    auto handle = GetHandle(true, errCode);
    if (handle == nullptr) {
        LOGE("Get write handle for transaction failed:%d", errCode);
        return errCode;
    }
    errCode = handle->StartTransaction();
    if (errCode != E_OK) {
        LOGE("Start transaction failed:%d", errCode);
        GetDB<MultiVerNaturalStore>()->ReleaseHandle(handle);
        return errCode;
    }

    writeHandle_ = handle;
    isAuto = true;

    return E_OK;
}

int MultiVerNaturalStoreConnection::TranslateObserverModeToEventTypes(unsigned mode,
    std::list<int> &eventTypes) const
{
    if (mode != NATURAL_STORE_COMMIT_EVENT) {
        return -E_NOT_SUPPORT;
    } else {
        eventTypes.push_back(NATURAL_STORE_COMMIT_EVENT);
        return E_OK;
    }
}

int MultiVerNaturalStoreConnection::Rekey(const CipherPassword &passwd)
{
    if (kvDB_ == nullptr) {
        return -E_INVALID_DB;
    }
    std::lock_guard<std::mutex> lock(rekeyMutex_);
    // Check the condition, have no more than one connection.
    int errCode = kvDB_->TryToDisableConnection(OperatePerm::REKEY_MONOPOLIZE_PERM);
    if (errCode != E_OK) {
        return errCode;
    }

    // Check the observer condition.
    errCode = GenericKvDBConnection::PreCheckExclusiveStatus();
    if (errCode != E_OK) {
        kvDB_->ReEnableConnection(OperatePerm::REKEY_MONOPOLIZE_PERM);
        return errCode;
    }

    // No need the check other
    errCode = kvDB_->Rekey(passwd);
    GenericKvDBConnection::ResetExclusiveStatus();
    kvDB_->ReEnableConnection(OperatePerm::REKEY_MONOPOLIZE_PERM);
    return errCode;
}

int MultiVerNaturalStoreConnection::Export(const std::string &filePath, const CipherPassword &passwd)
{
    if (kvDB_ == nullptr) {
        return -E_INVALID_DB;
    }
    return kvDB_->Export(filePath, passwd);
}

int MultiVerNaturalStoreConnection::Import(const std::string &filePath, const CipherPassword &passwd)
{
    if (kvDB_ == nullptr) {
        return -E_INVALID_DB;
    }
    std::lock_guard<std::mutex> lock(importMutex_);
    int errCode = kvDB_->TryToDisableConnection(OperatePerm::IMPORT_MONOPOLIZE_PERM);
    if (errCode != E_OK) {
        return errCode;
    }

    // Check the observer condition.
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

bool MultiVerNaturalStoreConnection::CheckDeletedKeys(const std::vector<Key> &keys)
{
    for (const auto &item : keys) {
        if (item.empty() || item.size() > DBConstant::MAX_KEY_SIZE) {
            return false;
        }
    }
    return true;
}

int MultiVerNaturalStoreConnection::CheckDataStatus(const Key &key, const Value &value, bool isDeleted) const
{
    if (kvDB_ == nullptr) {
        return -E_INVALID_DB;
    }
    return static_cast<MultiVerNaturalStore *>(kvDB_)->CheckDataStatus(key, value, isDeleted);
}

MultiVerStorageExecutor *MultiVerNaturalStoreConnection::GetHandle(bool isWrite, int &errCode) const
{
    MultiVerNaturalStore *multiVerNatureStore = GetDB<MultiVerNaturalStore>();
    if (multiVerNatureStore == nullptr) {
        errCode = -E_INVALID_DB;
        return nullptr;
    }

    return multiVerNatureStore->GetHandle(isWrite, errCode);
}

DEFINE_OBJECT_TAG_FACILITIES(MultiVerNaturalStoreConnection)
} // namespace DistributedDB
#endif