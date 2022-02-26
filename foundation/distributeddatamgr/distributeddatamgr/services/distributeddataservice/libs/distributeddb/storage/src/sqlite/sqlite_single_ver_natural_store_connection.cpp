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

#include "sqlite_single_ver_natural_store_connection.h"

#include "db_constant.h"
#include "db_errno.h"
#include "log_print.h"
#include "kvdb_pragma.h"
#include "sqlite_single_ver_natural_store.h"
#include "kvdb_observer_handle.h"
#include "types.h"
#include "db_common.h"
#include "sqlite_single_ver_result_set.h"
#include "sqlite_single_ver_forward_cursor.h"

namespace DistributedDB {
namespace {
    enum {
        LOCAL_OPR_NONE = 0,
        LOCAL_OPR_DEL = 1,
        LOCAL_OPR_PUT = 2
    };
    const uint32_t MAX_AUTO_LIFE_CYCLE = 1800000; // half an hour.
    const uint32_t MIN_AUTO_LIFE_CYCLE = 5000; // 5s.
}

SQLiteSingleVerNaturalStoreConnection::SQLiteSingleVerNaturalStoreConnection(SQLiteSingleVerNaturalStore *kvDB)
    : SyncAbleKvDBConnection(kvDB),
      cacheMaxSizeForNewResultSet_(DEFAULT_RESULT_SET_CACHE_MAX_SIZE),
      conflictType_(0),
      transactionEntrySize_(0),
      currentMaxTimeStamp_(0),
      committedData_(nullptr),
      localCommittedData_(nullptr),
      transactionExeFlag_(false),
      conflictListener_(nullptr),
      writeHandle_(nullptr)
{}

SQLiteSingleVerNaturalStoreConnection::~SQLiteSingleVerNaturalStoreConnection()
{
    if (conflictListener_ != nullptr) {
        conflictListener_->Drop(true);
        conflictListener_ = nullptr;
    }
}

inline bool SQLiteSingleVerNaturalStoreConnection::IsFileAccessControlled() const
{
    return RuntimeContext::GetInstance()->IsAccessControlled() &&
           kvDB_->GetMyProperties().GetSecLabel() > SecurityLabel::S2;
}

int SQLiteSingleVerNaturalStoreConnection::CheckReadDataControlled() const
{
    SQLiteSingleVerNaturalStore *naturalStore = GetDB<SQLiteSingleVerNaturalStore>();
    if (naturalStore == nullptr) {
        LOGE("[SingleVerConnection] natural store is nullptr in CheckReadDataControlled.");
        return E_OK;
    }
    return naturalStore->CheckReadDataControlled();
}

int SQLiteSingleVerNaturalStoreConnection::Get(const IOption &option, const Key &key, Value &value) const
{
    if (key.size() > DBConstant::MAX_KEY_SIZE || key.empty()) {
        return -E_INVALID_ARGS;
    }

    SingleVerDataType dataType;
    if (option.dataType == IOption::LOCAL_DATA) {
        dataType = SingleVerDataType::LOCAL_TYPE;
    } else if (option.dataType == IOption::SYNC_DATA) {
        dataType = SingleVerDataType::SYNC_TYPE;
    } else {
        return -E_NOT_SUPPORT;
    }

    int errCode = CheckReadDataControlled();
    if (errCode != E_OK) {
        LOGE("[Get] Existed cache database can not read data, errCode = [%d]!", errCode);
        return errCode;
    }

    {
        // need to check if the transaction started
        std::lock_guard<std::mutex> lock(transactionMutex_);
        if (writeHandle_ != nullptr) {
            LOGD("Transaction started already.");
            TimeStamp recordTimeStamp;
            return writeHandle_->GetKvData(dataType, key, value, recordTimeStamp);
        }
    }

    SQLiteSingleVerStorageExecutor *handle = GetExecutor(false, errCode);
    if (handle == nullptr) {
        return errCode;
    }

    TimeStamp timeStamp;
    errCode = handle->GetKvData(dataType, key, value, timeStamp);
    ReleaseExecutor(handle);
    return errCode;
}

int SQLiteSingleVerNaturalStoreConnection::Put(const IOption &option, const Key &key, const Value &value)
{
    std::vector<Entry> entries;
    Entry entry{key, value};
    entries.emplace_back(std::move(entry));

    return PutBatch(option, entries);
}

int SQLiteSingleVerNaturalStoreConnection::Delete(const IOption &option, const Key &key)
{
    std::vector<Key> keys;
    keys.push_back(key);

    return DeleteBatch(option, keys);
}

int SQLiteSingleVerNaturalStoreConnection::Clear(const IOption &option)
{
    return -E_NOT_SUPPORT;
}

int SQLiteSingleVerNaturalStoreConnection::GetEntries(const IOption &option, const Key &keyPrefix,
    std::vector<Entry> &entries) const
{
    if (keyPrefix.size() > DBConstant::MAX_KEY_SIZE) {
        return -E_INVALID_ARGS;
    }

    SingleVerDataType type;
    if (option.dataType == IOption::LOCAL_DATA) {
        type = SingleVerDataType::LOCAL_TYPE;
    } else if (option.dataType == IOption::SYNC_DATA) {
        type = SingleVerDataType::SYNC_TYPE;
    } else {
        return -E_INVALID_ARGS;
    }

    int errCode = CheckReadDataControlled();
    if (errCode != E_OK) {
        LOGE("[GetEntries] Existed cache database can not read data, errCode = [%d]!", errCode);
        return errCode;
    }

    {
        std::lock_guard<std::mutex> lock(transactionMutex_);
        if (writeHandle_ != nullptr) {
            LOGD("Transaction started already.");
            return writeHandle_->GetEntries(type, keyPrefix, entries);
        }
    }

    SQLiteSingleVerStorageExecutor *handle = GetExecutor(false, errCode);
    if (handle == nullptr) {
        LOGE("[Connection]::[GetEntries] Get executor failed, errCode = [%d]", errCode);
        return errCode;
    }

    errCode = handle->GetEntries(type, keyPrefix, entries);
    ReleaseExecutor(handle);
    return errCode;
}

int SQLiteSingleVerNaturalStoreConnection::GetEntries(const IOption &option, const Query &query,
    std::vector<Entry> &entries) const
{
    if (option.dataType != IOption::SYNC_DATA) {
        return -E_NOT_SUPPORT;
    }
    SQLiteSingleVerNaturalStore *naturalStore = GetDB<SQLiteSingleVerNaturalStore>();
    if (naturalStore == nullptr) {
        return -E_INVALID_DB;
    }
    int errCode = CheckReadDataControlled();
    if (errCode != E_OK) {
        LOGE("[GetEntries] Existed cache database can not read data, errCode = [%d]!", errCode);
        return errCode;
    }
    QueryObject queryObj(query);
    // In readOnly mode, forbidden all schema related query
    if (CheckWritePermission() == E_OK) {
        const SchemaObject &schemaObjRef = naturalStore->GetSchemaObjectConstRef();
        queryObj.SetSchema(schemaObjRef);
    }
    {
        std::lock_guard<std::mutex> lock(transactionMutex_);
        if (writeHandle_ != nullptr) {
            LOGD("Transaction started already.");
            return writeHandle_->GetEntries(queryObj, entries);
        }
    }

    SQLiteSingleVerStorageExecutor *handle = GetExecutor(false, errCode);
    if (handle == nullptr) {
        return errCode;
    }

    errCode = handle->GetEntries(queryObj, entries);
    ReleaseExecutor(handle);
    return errCode;
}

int SQLiteSingleVerNaturalStoreConnection::GetCount(const IOption &option, const Query &query, int &count) const
{
    if (option.dataType != IOption::SYNC_DATA) {
        return -E_NOT_SUPPORT;
    }
    int errCode = CheckReadDataControlled();
    if (errCode != E_OK) {
        LOGE("[GetCount] Existed cache database can not read data, errCode = [%d]!", errCode);
        return errCode;
    }

    SQLiteSingleVerNaturalStore *naturalStore = GetDB<SQLiteSingleVerNaturalStore>();
    if (naturalStore == nullptr) {
        return -E_INVALID_DB;
    }
    QueryObject queryObj(query);
    // In readOnly mode, forbidden all schema related query
    if (CheckWritePermission() == E_OK) {
        const SchemaObject &schemaObjRef = naturalStore->GetSchemaObjectConstRef();
        queryObj.SetSchema(schemaObjRef);
    }
    {
        std::lock_guard<std::mutex> lock(transactionMutex_);
        if (writeHandle_ != nullptr) {
            LOGD("Transaction started already.");
            return writeHandle_->GetCount(queryObj, count);
        }
    }

    SQLiteSingleVerStorageExecutor *handle = GetExecutor(false, errCode);
    if (handle == nullptr) {
        return errCode;
    }
    errCode = handle->GetCount(queryObj, count);
    ReleaseExecutor(handle);
    return errCode;
}

int SQLiteSingleVerNaturalStoreConnection::PutBatch(const IOption &option, const std::vector<Entry> &entries)
{
    LOGD("[PutBatch] entries size is : %zu, dataType : %d", entries.size(), option.dataType);
    if (option.dataType == IOption::LOCAL_DATA) {
        int retCode = CheckLocalEntriesValid(entries);
        if (retCode != E_OK) {
            return retCode;
        }
        return PutBatchInner(option, entries);
    }

    if (option.dataType == IOption::SYNC_DATA) {
        int errCode = CheckSyncEntriesValid(entries);
        if (errCode != E_OK) {
            return errCode;
        }
        return PutBatchInner(option, entries);
    }

    return -E_NOT_SUPPORT;
}

int SQLiteSingleVerNaturalStoreConnection::DeleteBatch(const IOption &option, const std::vector<Key> &keys)
{
    LOGD("[DeleteBatch] keys size is : %zu, dataType : %d", keys.size(), option.dataType);
    if (option.dataType == IOption::LOCAL_DATA) {
        int retCode = CheckLocalKeysValid(keys);
        if (retCode != E_OK) {
            return retCode;
        }
        return DeleteBatchInner(option, keys);
    }

    if (option.dataType == IOption::SYNC_DATA) {
        int errCode = CheckSyncKeysValid(keys);
        if (errCode != E_OK) {
            return errCode;
        }
        return DeleteBatchInner(option, keys);
    }

    return -E_NOT_SUPPORT;
}

int SQLiteSingleVerNaturalStoreConnection::GetSnapshot(IKvDBSnapshot *&snapshot) const
{
    return -E_NOT_SUPPORT;
}

void SQLiteSingleVerNaturalStoreConnection::ReleaseSnapshot(IKvDBSnapshot *&snapshot)
{}

int SQLiteSingleVerNaturalStoreConnection::StartTransaction()
{
    std::lock_guard<std::mutex> lock(transactionMutex_);
    if (writeHandle_ != nullptr) {
        LOGD("Transaction started already.");
        return -E_TRANSACT_STATE;
    }

    int errCode = StartTransactionInner();
    if (errCode == E_OK) {
        transactionExeFlag_.store(true);
    }
    return errCode;
}

int SQLiteSingleVerNaturalStoreConnection::Commit()
{
    std::lock_guard<std::mutex> lock(transactionMutex_);
    if (writeHandle_ == nullptr) {
        LOGE("single version database is null or the transaction has not been started");
        return -E_INVALID_DB;
    }

    int errCode = CommitInner();
    if (errCode == E_OK) {
        transactionExeFlag_.store(false);
    }
    return errCode;
}

int SQLiteSingleVerNaturalStoreConnection::RollBack()
{
    std::lock_guard<std::mutex> lock(transactionMutex_);
    if (writeHandle_ == nullptr) {
        LOGE("Invalid handle for rollback or the transaction has not been started.");
        return -E_INVALID_DB;
    }

    int errCode = RollbackInner();
    if (errCode == E_OK) {
        transactionExeFlag_.store(false);
    }
    return errCode;
}

bool SQLiteSingleVerNaturalStoreConnection::IsTransactionStarted() const
{
    return transactionExeFlag_.load();
}

int SQLiteSingleVerNaturalStoreConnection::Pragma(int cmd, void *parameter)
{
    int errCode = E_OK;
    switch (cmd) {
        case PRAGMA_RM_DEVICE_DATA: {
            SQLiteSingleVerNaturalStore *naturalStore = GetDB<SQLiteSingleVerNaturalStore>();
            if (naturalStore == nullptr) {
                return -E_INVALID_DB;
            }
            auto deviceName = static_cast<std::string *>(parameter);
            errCode = naturalStore->RemoveDeviceData(*deviceName, false, false);
            break;
        }
        case PRAGMA_GET_IDENTIFIER_OF_DEVICE: {
            if (parameter == nullptr) {
                return -E_INVALID_ARGS;
            }
            if (static_cast<PragmaDeviceIdentifier *>(parameter)->deviceID == "") {
                return -E_INVALID_ARGS;
            }
            static_cast<PragmaDeviceIdentifier *>(parameter)->deviceIdentifier =
                DBCommon::TransferHashString(static_cast<PragmaDeviceIdentifier *>(parameter)->deviceID);
            break;
        }
        case PRAGMA_GET_DEVICE_IDENTIFIER_OF_ENTRY:
            return GetDeviceIdentifier(static_cast<PragmaEntryDeviceIdentifier *>(parameter));
        case PRAGMA_PUBLISH_LOCAL:
            return PragmaPublish(parameter);
        case PRAGMA_UNPUBLISH_SYNC:
            errCode = PragmaUnpublish(parameter);
            break;
        case PRAGMA_SET_AUTO_LIFE_CYCLE:
            return PragmaSetAutoLifeCycle(static_cast<uint32_t *>(parameter));
        case PRAGMA_RESULT_SET_CACHE_MODE:
            return PragmaResultSetCacheMode(parameter);
        case PRAGMA_RESULT_SET_CACHE_MAX_SIZE:
            return PragmaResultSetCacheMaxSize(parameter);
        case PRAGMA_TRIGGER_TO_MIGRATE_DATA:
            return PragmaTriggerToMigrateData(*static_cast<SecurityOption *>(parameter));
        default:
            // Call Pragma() of super class.
            errCode = SyncAbleKvDBConnection::Pragma(cmd, parameter);
            break;
    }

    return errCode;
}

int SQLiteSingleVerNaturalStoreConnection::TranslateObserverModeToEventTypes(unsigned mode,
    std::list<int> &eventTypes) const
{
    int errCode = E_OK;
    switch (mode) {
        case static_cast<unsigned>(SQLITE_GENERAL_NS_PUT_EVENT):
            eventTypes.push_back(SQLITE_GENERAL_NS_PUT_EVENT);
            break;
        case static_cast<unsigned>(SQLITE_GENERAL_NS_SYNC_EVENT):
            eventTypes.push_back(SQLITE_GENERAL_NS_SYNC_EVENT);
            break;
        case (static_cast<unsigned>(SQLITE_GENERAL_NS_PUT_EVENT)
            | static_cast<unsigned>(SQLITE_GENERAL_NS_SYNC_EVENT)):
            eventTypes.push_back(SQLITE_GENERAL_NS_PUT_EVENT);
            eventTypes.push_back(SQLITE_GENERAL_NS_SYNC_EVENT);
            break;
        case static_cast<unsigned>(SQLITE_GENERAL_NS_LOCAL_PUT_EVENT):
            eventTypes.push_back(SQLITE_GENERAL_NS_LOCAL_PUT_EVENT);
            break;
        default:
            errCode = -E_NOT_SUPPORT;
            break;
    }
    return errCode;
}

void SQLiteSingleVerNaturalStoreConnection::ClearConflictNotifierCount()
{
    uint32_t conflictType = static_cast<unsigned>(conflictType_);
    if ((conflictType & static_cast<unsigned>(SQLITE_GENERAL_NS_FOREIGN_KEY_ONLY)) != 0) {
        (void)kvDB_->UnregisterFunction(CONFLICT_SINGLE_VERSION_NS_FOREIGN_KEY_ONLY);
    }
    if ((conflictType & static_cast<unsigned>(SQLITE_GENERAL_NS_FOREIGN_KEY_ORIG)) != 0) {
        (void)kvDB_->UnregisterFunction(CONFLICT_SINGLE_VERSION_NS_FOREIGN_KEY_ORIG);
    }
    if ((conflictType & static_cast<unsigned>(SQLITE_GENERAL_NS_NATIVE_ALL)) != 0) {
        (void)kvDB_->UnregisterFunction(CONFLICT_SINGLE_VERSION_NS_NATIVE_ALL);
    }
    return;
}

void SQLiteSingleVerNaturalStoreConnection::ResetConflictNotifierCount(int target)
{
    // Clear the old conflict type function.
    ClearConflictNotifierCount();

    LOGD("Conflict type:%d to %d", conflictType_, target);
    // Add the new conflict type function.
    AddConflictNotifierCount(target);
    conflictType_ = target;
}

void SQLiteSingleVerNaturalStoreConnection::AddConflictNotifierCount(int target)
{
    LOGD("Conflict type:%u vs %u", conflictType_, target);
    // Add the new conflict type function.
    uint32_t targetTemp = static_cast<uint32_t>(target);
    if ((targetTemp & static_cast<uint32_t>(SQLITE_GENERAL_NS_FOREIGN_KEY_ONLY)) != 0) {
        (void)kvDB_->RegisterFunction(CONFLICT_SINGLE_VERSION_NS_FOREIGN_KEY_ONLY);
    }
    if ((targetTemp & static_cast<uint32_t>(SQLITE_GENERAL_NS_FOREIGN_KEY_ORIG)) != 0) {
        (void)kvDB_->RegisterFunction(CONFLICT_SINGLE_VERSION_NS_FOREIGN_KEY_ORIG);
    }
    if ((targetTemp & static_cast<uint32_t>(SQLITE_GENERAL_NS_NATIVE_ALL)) != 0) {
        (void)kvDB_->RegisterFunction(CONFLICT_SINGLE_VERSION_NS_NATIVE_ALL);
    }
}

int SQLiteSingleVerNaturalStoreConnection::SetConflictNotifier(int conflictType,
    const KvDBConflictAction &action)
{
    std::lock_guard<std::mutex> lock(conflictMutex_);
    if (!action && conflictListener_ == nullptr) {
        return -E_INVALID_ARGS;
    }
    if (kvDB_ == nullptr) {
        return -E_INVALID_DB;
    }

    // prevent the rekey operation.
    if (isExclusive_.load()) {
        return -E_BUSY;
    }

    int targetType = 0;
    NotificationChain::Listener *listener = nullptr;
    if (action) {
        int errCode = E_OK;
        Key key;
        listener = RegisterSpecialListener(SQLITE_GENERAL_CONFLICT_EVENT, key, action, true, errCode);
        if (listener == nullptr) {
            LOGE("Register Conflict listener failed:'%d'.", errCode);
            return errCode;
        }
        targetType = conflictType;
    }

    ResetConflictNotifierCount(targetType);
    // drop the old listener.
    if (conflictListener_ != nullptr) {
        conflictListener_->Drop(true);
    }
    conflictListener_ = listener;
    return E_OK;
}

int SQLiteSingleVerNaturalStoreConnection::Rekey(const CipherPassword &passwd)
{
    if (IsFileAccessControlled()) {
        LOGE("Forbid Rekey when screen locked and security label [%d]!", kvDB_->GetMyProperties().GetSecLabel());
        return -E_NOT_SUPPORT;
    }
    std::lock_guard<std::mutex> lock(rekeyMutex_);
    int errCode = CheckMonoStatus(OperatePerm::REKEY_MONOPOLIZE_PERM);
    if (errCode != E_OK) {
        return errCode;
    }
    LOGI("Begin rekey operation");
    errCode = kvDB_->Rekey(passwd);
    GenericKvDBConnection::ResetExclusiveStatus();
    kvDB_->ReEnableConnection(OperatePerm::REKEY_MONOPOLIZE_PERM);
    EnableManualSync();
    LOGI("End rekey operation errCode = [%d]", errCode);
    return errCode;
}

int SQLiteSingleVerNaturalStoreConnection::Export(const std::string &filePath, const CipherPassword &passwd)
{
    if (kvDB_ == nullptr) {
        return -E_INVALID_DB;
    }

    if (IsFileAccessControlled()) {
        LOGE("Forbid Export when screen locked and security label [%d] file lock state [%d]",
            kvDB_->GetMyProperties().GetSecLabel(), RuntimeContext::GetInstance()->IsAccessControlled());
        return -E_NOT_SUPPORT;
    } // Avoid abnormal branch handling without affecting the business
    return kvDB_->Export(filePath, passwd);
}

int SQLiteSingleVerNaturalStoreConnection::Import(const std::string &filePath, const CipherPassword &passwd)
{
    if (IsFileAccessControlled()) {
        LOGE("Forbid Import when screen locked and security label [%d]!", kvDB_->GetMyProperties().GetSecLabel());
        return -E_NOT_SUPPORT;
    }

    std::lock_guard<std::mutex> lock(importMutex_);
    int errCode = CheckMonoStatus(OperatePerm::IMPORT_MONOPOLIZE_PERM);
    if (errCode != E_OK) {
        return errCode;
    }
    errCode = kvDB_->Import(filePath, passwd);
    GenericKvDBConnection::ResetExclusiveStatus();
    kvDB_->ReEnableConnection(OperatePerm::IMPORT_MONOPOLIZE_PERM);
    EnableManualSync();
    return errCode;
}

int SQLiteSingleVerNaturalStoreConnection::GetResultSet(const IOption &option, const Key &keyPrefix,
    IKvDBResultSet *&resultSet) const
{
    // need to check if the transaction started
    if (transactionExeFlag_.load()) {
        LOGD("Transaction started already.");
        return -E_BUSY;
    }

    // maximum of result set size is 4
    std::lock_guard<std::mutex> lock(kvDbResultSetsMutex_);
    if (kvDbResultSets_.size() >= MAX_RESULT_SET_SIZE) {
        LOGE("Over max result set size");
        return -E_MAX_LIMITS;
    }

    int errCode = CheckReadDataControlled();
    if (errCode != E_OK) {
        LOGE("[GetResultSet][keyPrefix] Existed cache database can not read data, errCode = [%d]!", errCode);
        return errCode;
    }

    SQLiteSingleVerNaturalStore *naturalStore = GetDB<SQLiteSingleVerNaturalStore>();
    if (naturalStore == nullptr) {
        return -E_INVALID_DB;
    }
    bool isMemDb = naturalStore->GetMyProperties().GetBoolProp(KvDBProperties::MEMORY_MODE, false);
    resultSet = new (std::nothrow) SQLiteSingleVerResultSet(naturalStore, keyPrefix,
        SQLiteSingleVerResultSet::Option{cacheModeForNewResultSet_.load(), cacheMaxSizeForNewResultSet_.load()});
    if (resultSet == nullptr) {
        LOGE("Create single version result set failed.");
        return -E_OUT_OF_MEMORY;
    }
    errCode = resultSet->Open(isMemDb);
    if (errCode != E_OK) {
        delete resultSet;
        resultSet = nullptr;
        LOGE("Open result set failed.");
        return errCode;
    }
    kvDbResultSets_.insert(resultSet);
    return E_OK;
}

int SQLiteSingleVerNaturalStoreConnection::GetResultSet(const IOption &option, const Query &query,
    IKvDBResultSet *&resultSet) const
{
    // need to check if the transaction started
    if (transactionExeFlag_.load()) {
        LOGD("Transaction started already.");
        return -E_BUSY;
    }

    // maximum of result set size is 4
    std::lock_guard<std::mutex> lock(kvDbResultSetsMutex_);
    if (kvDbResultSets_.size() >= MAX_RESULT_SET_SIZE) {
        LOGE("Over max result set size");
        return -E_MAX_LIMITS;
    }

    int errCode = CheckReadDataControlled();
    if (errCode != E_OK) {
        LOGE("[GetResultSet][query] Existed cache database can not read data, errCode = [%d]!", errCode);
        return errCode;
    }

    SQLiteSingleVerNaturalStore *naturalStore = GetDB<SQLiteSingleVerNaturalStore>(); // Guarantee not nullptr
    QueryObject queryObj(query);
    // In readOnly mode, forbidden all schema related query
    if (CheckWritePermission() == E_OK) {
        const SchemaObject &schemaObjRef = naturalStore->GetSchemaObjectConstRef();
        queryObj.SetSchema(schemaObjRef);
    }
    bool isMemDb = naturalStore->GetMyProperties().GetBoolProp(KvDBProperties::MEMORY_MODE, false);
    resultSet = new (std::nothrow) SQLiteSingleVerResultSet(naturalStore, queryObj,
        SQLiteSingleVerResultSet::Option{cacheModeForNewResultSet_.load(), cacheMaxSizeForNewResultSet_.load()});
    if (resultSet == nullptr) {
        LOGE("Create single version result set failed.");
        return -E_OUT_OF_MEMORY;
    }
    errCode = resultSet->Open(isMemDb);
    if (errCode != E_OK) {
        delete resultSet;
        resultSet = nullptr;
        LOGE("Open result set failed.");
        return errCode;
    }
    kvDbResultSets_.insert(resultSet);
    return E_OK;
}

void SQLiteSingleVerNaturalStoreConnection::ReleaseResultSet(IKvDBResultSet *&resultSet)
{
    std::lock_guard<std::mutex> lock(kvDbResultSetsMutex_);
    if (resultSet == nullptr) {
        return;
    }
    resultSet->Close();
    kvDbResultSets_.erase(resultSet);
    delete resultSet;
    resultSet = nullptr;
    return;
}

int SQLiteSingleVerNaturalStoreConnection::RegisterLifeCycleCallback(const DatabaseLifeCycleNotifier &notifier)
{
    if (kvDB_ == nullptr) {
        return -E_INVALID_DB;
    }
    return static_cast<SQLiteSingleVerNaturalStore *>(kvDB_)->RegisterLifeCycleCallback(notifier);
}

int SQLiteSingleVerNaturalStoreConnection::PreClose()
{
    // check if result set closed
    {
        std::lock_guard<std::mutex> kvDbResultLock(kvDbResultSetsMutex_);
        if (kvDbResultSets_.size() > 0) {
            LOGE("The connection have [%zu] active result set, can not close.", kvDbResultSets_.size());
            return -E_BUSY;
        }
    }

    // check if transaction closed
    std::lock_guard<std::mutex> transactionLock(transactionMutex_);
    if (writeHandle_ != nullptr) {
        LOGW("Transaction started, need to rollback before close.");
        int errCode = RollbackInner();
        if (errCode != E_OK) {
            LOGE("Rollback transaction failed, %d.", errCode);
        }
        ReleaseExecutor(writeHandle_);
    }

    // Clear the conflict type function.
    {
        std::lock_guard<std::mutex> lock(conflictMutex_);
        ClearConflictNotifierCount();
        conflictType_ = 0;
    }
    return E_OK;
}

int SQLiteSingleVerNaturalStoreConnection::CheckMonoStatus(OperatePerm perm)
{
    // 1. Get the connection number
    if (kvDB_ == nullptr) {
        return -E_INVALID_DB;
    }
    int errCode = DisableManualSync();
    if (errCode != E_OK) {
        LOGE("In manual sync");
        return -E_BUSY;
    }

    // 2. check the result set number
    {
        std::lock_guard<std::mutex> kvDbResultLock(kvDbResultSetsMutex_);
        if (kvDbResultSets_.size() > 0) {
            LOGE("Active result set exist.");
            EnableManualSync();
            return -E_BUSY;
        }
    }
    // 1. Get the connection number, and get the right to do the rekey operation.
    errCode = kvDB_->TryToDisableConnection(perm);
    if (errCode != E_OK) {
        // If precheck failed, it means that there are more than one connection.
        // No need reset the condition for the scene.
        LOGE("More than one connection");
        EnableManualSync();
        return errCode;
    }
    // 2. Check the observer list.
    errCode = GenericKvDBConnection::PreCheckExclusiveStatus();
    if (errCode != E_OK) {
        kvDB_->ReEnableConnection(perm);
        LOGE("Observer prevents.");
        EnableManualSync();
        return errCode;
    }

    // 3. Check the conflict notifier.
    {
        std::lock_guard<std::mutex> conflictLock(conflictMutex_);
        if (conflictListener_ != nullptr) {
            errCode = -E_BUSY;
            GenericKvDBConnection::ResetExclusiveStatus();
            kvDB_->ReEnableConnection(perm);
            LOGE("Conflict notifier prevents");
            EnableManualSync();
            return errCode;
        }
    }
    return E_OK;
}

int SQLiteSingleVerNaturalStoreConnection::GetDeviceIdentifier(PragmaEntryDeviceIdentifier *identifier)
{
    if (identifier == nullptr) {
        return -E_INVALID_ARGS;
    }

    if (identifier->key.empty() || identifier->key.size() > DBConstant::MAX_VALUE_SIZE) {
        return -E_INVALID_ARGS;
    }

    int errCode = E_OK;
    SQLiteSingleVerStorageExecutor *handle = GetExecutor(false, errCode);
    if (handle == nullptr) {
        return errCode;
    }

    errCode = handle->GetDeviceIdentifier(identifier);
    ReleaseExecutor(handle);
    return errCode;
}

int SQLiteSingleVerNaturalStoreConnection::PutBatchInner(const IOption &option, const std::vector<Entry> &entries)
{
    std::lock_guard<std::mutex> lock(transactionMutex_);
    bool isAuto = false;
    int errCode = E_OK;

    if (writeHandle_ == nullptr) {
        isAuto = true;
        errCode = StartTransactionInner();
        if (errCode != E_OK) {
            return errCode;
        }
    }

    if ((transactionEntrySize_ + entries.size()) > DBConstant::MAX_TRANSACTION_ENTRY_SIZE) {
        return -E_MAX_LIMITS;
    }

    if (option.dataType == IOption::SYNC_DATA) {
        errCode = SaveSyncEntries(entries);
    } else {
        errCode = SaveLocalEntries(entries);
    }
    if (errCode == E_OK) {
        transactionEntrySize_ += entries.size();
    }

    if (isAuto) {
        if (errCode == E_OK) {
            errCode = CommitInner();
        } else {
            int innerCode = RollbackInner();
            errCode = (innerCode != E_OK) ? innerCode : errCode;
        }
    }
    return errCode;
}

int SQLiteSingleVerNaturalStoreConnection::DeleteBatchInner(const IOption &option, const std::vector<Key> &keys)
{
    std::lock_guard<std::mutex> lock(transactionMutex_);
    bool isAuto = false;
    int errCode = E_OK;

    if (writeHandle_ == nullptr) {
        isAuto = true;
        errCode = StartTransactionInner();
        if (errCode != E_OK) {
            return errCode;
        }
    }

    if ((transactionEntrySize_ + keys.size()) > DBConstant::MAX_TRANSACTION_ENTRY_SIZE) {
        return -E_MAX_LIMITS;
    }

    if (option.dataType == IOption::SYNC_DATA) {
        errCode = DeleteSyncEntries(keys);
    } else {
        errCode = DeleteLocalEntries(keys);
    }
    if (errCode == E_OK) {
        transactionEntrySize_ += keys.size();
    }

    if (isAuto) {
        if (errCode == E_OK) {
            errCode = CommitInner();
        } else {
            int innerCode = RollbackInner();
            errCode = (innerCode != E_OK) ? innerCode : errCode;
        }
    }
    return errCode;
}

int SQLiteSingleVerNaturalStoreConnection::SaveSyncEntries(const std::vector<Entry> &entries)
{
    int errCode = E_OK;
    for (const auto &entry : entries) {
        errCode = SaveEntry(entry, false);
        if (errCode != E_OK) {
            break;
        }
    }
    return errCode;
}

int SQLiteSingleVerNaturalStoreConnection::SaveLocalEntries(const std::vector<Entry> &entries)
{
    int errCode = E_OK;
    for (const auto &entry : entries) {
        errCode = SaveLocalEntry(entry, false);
        if (errCode != E_OK) {
            break;
        }
    }
    return errCode;
}

int SQLiteSingleVerNaturalStoreConnection::DeleteSyncEntries(const std::vector<Key> &keys)
{
    int errCode = E_OK;
    for (const auto &key : keys) {
        Entry entry;
        DBCommon::CalcValueHash(key, entry.key);
        errCode = SaveEntry(entry, true);
        if ((errCode != E_OK) && (errCode != -E_NOT_FOUND)) {
            LOGE("[DeleteSyncEntries] Delete data err:%d", errCode);
            break;
        }
    }
    return (errCode == -E_NOT_FOUND) ? E_OK : errCode;
}

int SQLiteSingleVerNaturalStoreConnection::DeleteLocalEntries(const std::vector<Key> &keys)
{
    int errCode = E_OK;
    for (const auto &key : keys) {
        Entry entry = {key, Value()};
        errCode = SaveLocalEntry(entry, true);
        if ((errCode != E_OK) && (errCode != -E_NOT_FOUND)) {
            LOGE("[DeleteLocalEntries] Delete data err:%d", errCode);
            break;
        }
    }
    return (errCode == -E_NOT_FOUND) ? E_OK : errCode;
}

// This function currently only be called in local procedure to change sync_data table, do not use in sync procedure.
// It will check and amend value when need if it is a schema database. return error if some value disagree with the
// schema. But in sync procedure, we just neglect the value that disagree with schema.
int SQLiteSingleVerNaturalStoreConnection::SaveEntry(const Entry &entry, bool isDelete, TimeStamp timeStamp)
{
    SQLiteSingleVerNaturalStore *naturalStore = GetDB<SQLiteSingleVerNaturalStore>();
    if (naturalStore == nullptr) {
        return -E_INVALID_DB;
    }

    DataItem dataItem;
    dataItem.key = entry.key;
    dataItem.value = entry.value;
    dataItem.flag = DataItem::LOCAL_FLAG;
    if (isDelete) {
        dataItem.flag |= DataItem::DELETE_FLAG;
    } else {
        int errCode = CheckAmendValueContentForLocalProcedure(dataItem.value, dataItem.value);
        if (errCode != E_OK) {
            LOGE("[SqlSinCon][SaveEntry] CheckAmendValue fail, errCode=%d.", errCode);
            return errCode;
        }
    }

    dataItem.timeStamp = naturalStore->GetCurrentTimeStamp();
    if (currentMaxTimeStamp_ > dataItem.timeStamp) {
        dataItem.timeStamp = currentMaxTimeStamp_;
    }

    if (timeStamp != 0) {
        dataItem.writeTimeStamp = timeStamp;
    } else {
        dataItem.writeTimeStamp = dataItem.timeStamp;
    }

    if (IsExtendedCacheDBMode()) {
        uint64_t recordVersion = naturalStore->GetCacheRecordVersion();
        return SaveEntryInCacheMode(dataItem, recordVersion);
    } else {
        return SaveEntryNormally(dataItem);
    }
}

int SQLiteSingleVerNaturalStoreConnection::SaveLocalEntry(const Entry &entry, bool isDelete)
{
    SQLiteSingleVerNaturalStore *naturalStore = GetDB<SQLiteSingleVerNaturalStore>();
    if (naturalStore == nullptr) {
        return -E_INVALID_DB;
    }

    LocalDataItem dataItem;
    dataItem.key = entry.key;
    dataItem.value = entry.value;
    (void)DBCommon::CalcValueHash(entry.key, dataItem.hashKey);
    if (isDelete) {
        dataItem.flag = DataItem::DELETE_FLAG;
    }
    dataItem.timeStamp = naturalStore->GetCurrentTimeStamp();
    LOGD("TimeStamp is %llu", dataItem.timeStamp);

    if (IsCacheDBMode()) {
        return SaveLocalItemInCacheMode(dataItem);
    } else {
        return SaveLocalItem(dataItem);
    }
}

int SQLiteSingleVerNaturalStoreConnection::SaveLocalItem(const LocalDataItem &dataItem) const
{
    int errCode = E_OK;
    if ((dataItem.flag & DataItem::DELETE_FLAG) == 0) {
        errCode = writeHandle_->PutKvData(SingleVerDataType::LOCAL_TYPE, dataItem.key, dataItem.value,
            dataItem.timeStamp, localCommittedData_);
    } else {
        Value value;
        TimeStamp localTimeStamp = 0;
        errCode = writeHandle_->DeleteLocalKvData(dataItem.key, localCommittedData_, value, localTimeStamp);
    }
    return errCode;
}

int SQLiteSingleVerNaturalStoreConnection::SaveLocalItemInCacheMode(const LocalDataItem &dataItem) const
{
    SQLiteSingleVerNaturalStore *naturalStore = GetDB<SQLiteSingleVerNaturalStore>();
    if (naturalStore == nullptr) {
        return -E_INVALID_DB;
    }

    int errCode = writeHandle_->PutLocalDataToCacheDB(dataItem);
    if (errCode != E_OK) {
        LOGE("[PutLocalEntries] Put local data to cacheDB err:%d", errCode);
    }
    return errCode;
}

int SQLiteSingleVerNaturalStoreConnection::SaveEntryNormally(DataItem &dataItem)
{
    int errCode = writeHandle_->PrepareForSavingData(SingleVerDataType::SYNC_TYPE);
    if (errCode != E_OK) {
        LOGE("Prepare the saving sync data failed:%d", errCode);
        return errCode;
    }

    TimeStamp maxTimestamp = 0;
    DeviceInfo deviceInfo = {true, ""};
    errCode = writeHandle_->SaveSyncDataItem(dataItem, deviceInfo, maxTimestamp, committedData_);
    if (errCode == E_OK) {
        if (maxTimestamp > currentMaxTimeStamp_) {
            currentMaxTimeStamp_ = maxTimestamp;
        }
    } else {
        LOGE("Save entry failed, err:%d", errCode);
    }
    return errCode;
}

int SQLiteSingleVerNaturalStoreConnection::SaveEntryInCacheMode(DataItem &dataItem, uint64_t recordVersion)
{
    int errCode = writeHandle_->PrepareForSavingCacheData(SingleVerDataType::SYNC_TYPE);
    if (errCode != E_OK) {
        LOGE("Prepare the saving sync data failed:%d", errCode);
        return errCode;
    }

    TimeStamp maxTimestamp = 0;
    DeviceInfo deviceInfo = {true, ""};
    errCode = writeHandle_->SaveSyncDataItemInCacheMode(dataItem, deviceInfo, maxTimestamp, recordVersion);
    if (errCode == E_OK) {
        if (maxTimestamp > currentMaxTimeStamp_) {
            currentMaxTimeStamp_ = maxTimestamp;
        }
    } else {
        LOGE("Save entry failed, err:%d", errCode);
    }
    return errCode;
}

int SQLiteSingleVerNaturalStoreConnection::CheckDataStatus(const Key &key, const Value &value, bool isDelete) const
{
    SQLiteSingleVerNaturalStore *naturalStore = GetDB<SQLiteSingleVerNaturalStore>();
    if (naturalStore == nullptr) {
        return -E_INVALID_DB;
    }

    return naturalStore->CheckDataStatus(key, value, isDelete);
}

int SQLiteSingleVerNaturalStoreConnection::CheckWritePermission() const
{
    SQLiteSingleVerNaturalStore *naturalStore = GetDB<SQLiteSingleVerNaturalStore>();
    if (naturalStore == nullptr) {
        return -E_INVALID_DB;
    }

    if (!naturalStore->CheckWritePermission()) {
        return -E_READ_ONLY;
    }
    return E_OK;
}

int SQLiteSingleVerNaturalStoreConnection::CheckSyncEntriesValid(const std::vector<Entry> &entries) const
{
    if (entries.size() > DBConstant::MAX_BATCH_SIZE) {
        return -E_INVALID_ARGS;
    }

    SQLiteSingleVerNaturalStore *naturalStore = GetDB<SQLiteSingleVerNaturalStore>();
    if (naturalStore == nullptr) {
        return -E_INVALID_DB;
    }

    if (!naturalStore->CheckWritePermission()) {
        return -E_READ_ONLY;
    }

    for (const auto &entry : entries) {
        int errCode = naturalStore->CheckDataStatus(entry.key, entry.value, false);
        if (errCode != E_OK) {
            return errCode;
        }
    }
    return E_OK;
}

int SQLiteSingleVerNaturalStoreConnection::CheckSyncKeysValid(const std::vector<Key> &keys) const
{
    if (keys.size() > DBConstant::MAX_BATCH_SIZE) {
        return -E_INVALID_ARGS;
    }

    SQLiteSingleVerNaturalStore *naturalStore = GetDB<SQLiteSingleVerNaturalStore>();
    if (naturalStore == nullptr) {
        return -E_INVALID_DB;
    }

    if (!naturalStore->CheckWritePermission()) {
        return -E_READ_ONLY;
    }

    for (const auto &key : keys) {
        int errCode = naturalStore->CheckDataStatus(key, {}, true);
        if (errCode != E_OK) {
            return errCode;
        }
    }
    return E_OK;
}

int SQLiteSingleVerNaturalStoreConnection::CheckLocalEntriesValid(const std::vector<Entry> &entries) const
{
    if (entries.size() > DBConstant::MAX_BATCH_SIZE) {
        return -E_INVALID_ARGS;
    }

    GenericKvDB *naturalStore = GetDB<GenericKvDB>();
    if (naturalStore == nullptr) {
        return -E_INVALID_DB;
    }

    if (!naturalStore->GenericKvDB::CheckWritePermission()) {
        return -E_READ_ONLY;
    }

    for (const auto &entry : entries) {
        int errCode = naturalStore->GenericKvDB::CheckDataStatus(entry.key, entry.value, false);
        if (errCode != E_OK) {
            return errCode;
        }
    }
    return E_OK;
}

int SQLiteSingleVerNaturalStoreConnection::CheckLocalKeysValid(const std::vector<Key> &keys) const
{
    if (keys.size() > DBConstant::MAX_BATCH_SIZE) {
        return -E_INVALID_ARGS;
    }

    GenericKvDB *naturalStore = GetDB<GenericKvDB>();
    if (naturalStore == nullptr) {
        return -E_INVALID_DB;
    }

    if (!naturalStore->GenericKvDB::CheckWritePermission()) {
        return -E_READ_ONLY;
    }

    for (const auto &key : keys) {
        int errCode = naturalStore->GenericKvDB::CheckDataStatus(key, {}, true);
        if (errCode != E_OK) {
            return errCode;
        }
    }
    return E_OK;
}

void SQLiteSingleVerNaturalStoreConnection::CommitAndReleaseNotifyData(
    SingleVerNaturalStoreCommitNotifyData *&committedData, bool isNeedCommit, int eventType)
{
    SQLiteSingleVerNaturalStore *naturalStore = GetDB<SQLiteSingleVerNaturalStore>();
    if ((naturalStore != nullptr) && (committedData != nullptr)) {
        if (isNeedCommit) {
            if (!committedData->IsChangedDataEmpty()) {
                naturalStore->CommitNotify(eventType, committedData);
            }
            if (!committedData->IsConflictedDataEmpty()) {
                naturalStore->CommitNotify(SQLITE_GENERAL_CONFLICT_EVENT, committedData);
            }
        }
    }
    ReleaseCommitData(committedData);
}

int SQLiteSingleVerNaturalStoreConnection::StartTransactionInner()
{
    if (IsExtendedCacheDBMode()) {
        return StartTransactionInCacheMode();
    } else {
        return StartTransactionNormally();
    }
}

int SQLiteSingleVerNaturalStoreConnection::StartTransactionInCacheMode()
{
    int errCode = E_OK;
    SQLiteSingleVerStorageExecutor *handle = GetExecutor(true, errCode);
    if (handle == nullptr) {
        return errCode;
    }
    errCode = handle->StartTransaction(TransactType::DEFERRED);
    if (errCode != E_OK) {
        ReleaseExecutor(handle);
        return errCode;
    }

    LOGD("[SingleVerConnection] Start transaction finish in cache mode.");
    writeHandle_ = handle;
    transactionEntrySize_ = 0;
    return E_OK;
}

int SQLiteSingleVerNaturalStoreConnection::StartTransactionNormally()
{
    int errCode = E_OK;
    SQLiteSingleVerStorageExecutor *handle = GetExecutor(true, errCode);
    if (handle == nullptr) {
        return errCode;
    }

    if (committedData_ == nullptr) {
        committedData_ = new (std::nothrow) SingleVerNaturalStoreCommitNotifyData;
        if (committedData_ == nullptr) {
            ReleaseExecutor(handle);
            return -E_OUT_OF_MEMORY;
        }
        InitConflictNotifiedFlag();
    }
    if (localCommittedData_ == nullptr) {
        localCommittedData_ = new (std::nothrow) SingleVerNaturalStoreCommitNotifyData;
        if (localCommittedData_ == nullptr) {
            ReleaseExecutor(handle);
            ReleaseCommitData(committedData_);
            return -E_OUT_OF_MEMORY;
        }
    }
    errCode = handle->StartTransaction(TransactType::DEFERRED);
    if (errCode != E_OK) {
        ReleaseExecutor(handle);
        ReleaseCommitData(committedData_);
        ReleaseCommitData(localCommittedData_);
        return errCode;
    }

    LOGD("[SingleVerConnection] Start transaction finish.");
    writeHandle_ = handle;
    transactionEntrySize_ = 0;
    return E_OK;
}

void SQLiteSingleVerNaturalStoreConnection::InitConflictNotifiedFlag()
{
    unsigned int conflictFlag = 0;
    if (kvDB_->GetRegisterFunctionCount(CONFLICT_SINGLE_VERSION_NS_FOREIGN_KEY_ONLY) != 0) {
        conflictFlag |= static_cast<unsigned>(SQLITE_GENERAL_NS_FOREIGN_KEY_ONLY);
    }
    if (kvDB_->GetRegisterFunctionCount(CONFLICT_SINGLE_VERSION_NS_FOREIGN_KEY_ORIG) != 0) {
        conflictFlag |= static_cast<unsigned>(SQLITE_GENERAL_NS_FOREIGN_KEY_ORIG);
    }
    if (kvDB_->GetRegisterFunctionCount(CONFLICT_SINGLE_VERSION_NS_NATIVE_ALL) != 0) {
        conflictFlag |= static_cast<unsigned>(SQLITE_GENERAL_NS_NATIVE_ALL);
    }
    LOGD("[SingleVer][InitConflictNotifiedFlag] conflictFlag Flag: %u", conflictFlag);
    committedData_->SetConflictedNotifiedFlag(static_cast<int>(conflictFlag));
}

int SQLiteSingleVerNaturalStoreConnection::CommitInner()
{
    bool isCacheOrMigrating = IsExtendedCacheDBMode();

    int errCode = writeHandle_->Commit();
    ReleaseExecutor(writeHandle_);
    LOGD("Commit transaction finish.");
    transactionEntrySize_ = 0;

    if (!isCacheOrMigrating) {
        CommitAndReleaseNotifyData(committedData_, true, SQLITE_GENERAL_NS_PUT_EVENT);
        CommitAndReleaseNotifyData(localCommittedData_, true, SQLITE_GENERAL_NS_LOCAL_PUT_EVENT);
    }
    SQLiteSingleVerNaturalStore *naturalStore = GetDB<SQLiteSingleVerNaturalStore>();
    if (naturalStore == nullptr) {
        return -E_INVALID_DB;
    }
    naturalStore->SetMaxTimeStamp(currentMaxTimeStamp_);

    if (isCacheOrMigrating) {
        naturalStore->IncreaseCacheRecordVersion();
    }
    return errCode;
}

int SQLiteSingleVerNaturalStoreConnection::RollbackInner()
{
    int errCode = writeHandle_->Rollback();
    LOGD("Rollback transaction finish.");
    transactionEntrySize_ = 0;
    currentMaxTimeStamp_ = 0;
    if (!IsExtendedCacheDBMode()) {
        ReleaseCommitData(committedData_);
        ReleaseCommitData(localCommittedData_);
    }
    ReleaseExecutor(writeHandle_);
    return errCode;
}

SQLiteSingleVerStorageExecutor *SQLiteSingleVerNaturalStoreConnection::GetExecutor(bool isWrite, int &errCode) const
{
    SQLiteSingleVerNaturalStore *naturalStore = GetDB<SQLiteSingleVerNaturalStore>();
    if (naturalStore == nullptr) {
        errCode = -E_NOT_INIT;
        LOGE("[SingleVerConnection] Kvstore is null, get executor failed! errCode = [%d]", errCode);
        return nullptr;
    }
    return naturalStore->GetHandle(isWrite, errCode);
}

bool SQLiteSingleVerNaturalStoreConnection::IsCacheDBMode() const
{
    SQLiteSingleVerNaturalStore *naturalStore = GetDB<SQLiteSingleVerNaturalStore>();
    if (naturalStore == nullptr) {
        LOGE("[SingleVerConnection] natural store is null in IsCacheDBMode.");
        return false;
    }
    return naturalStore->IsCacheDBMode();
}

bool SQLiteSingleVerNaturalStoreConnection::IsExtendedCacheDBMode() const
{
    SQLiteSingleVerNaturalStore *naturalStore = GetDB<SQLiteSingleVerNaturalStore>();
    if (naturalStore == nullptr) {
        LOGE("[SingleVerConnection] natural store is nullptr in IsExtendedCacheDBMode.");
        return false;
    }
    return naturalStore->IsExtendedCacheDBMode();
}

void SQLiteSingleVerNaturalStoreConnection::ReleaseExecutor(SQLiteSingleVerStorageExecutor *&executor) const
{
    SQLiteSingleVerNaturalStore *naturalStore = GetDB<SQLiteSingleVerNaturalStore>();
    if (naturalStore != nullptr) {
        naturalStore->ReleaseHandle(executor);
    }
}

int SQLiteSingleVerNaturalStoreConnection::PublishLocal(const Key &key, bool deleteLocal, bool updateTimestamp,
    const KvStoreNbPublishAction &onConflict)
{
    int errCode = CheckWritePermission();
    if (errCode != E_OK) {
        return errCode;
    }

    bool isNeedCallback = (onConflict != nullptr);
    SingleVerRecord localRecord;
    localRecord.key = key;
    SingleVerRecord syncRecord;
    {
        if (IsTransactionStarted()) {
            return -E_NOT_SUPPORT;
        }
        std::lock_guard<std::mutex> lock(transactionMutex_);
        errCode = StartTransactionInner();
        if (errCode != E_OK) {
            return errCode;
        }

        SingleVerNaturalStoreCommitNotifyData *localCommittedData = nullptr;
        if (deleteLocal) {
            localCommittedData = new (std::nothrow) SingleVerNaturalStoreCommitNotifyData;
            if (localCommittedData == nullptr) {
                errCode = -E_OUT_OF_MEMORY;
            }
        }
        if (errCode == E_OK) {
            errCode = PublishInner(localCommittedData, updateTimestamp, localRecord, syncRecord, isNeedCallback);
        }

        if (errCode != E_OK || isNeedCallback) {
            int innerCode = RollbackInner();
            errCode = (innerCode != E_OK) ? innerCode : errCode;
        } else {
            errCode = CommitInner();
            if (errCode == E_OK) {
                CommitAndReleaseNotifyData(localCommittedData, true, SQLITE_GENERAL_NS_LOCAL_PUT_EVENT);
            }
        }
        ReleaseCommitData(localCommittedData);
    }

    // need to release the handle lock before callback invoked
    if (isNeedCallback) {
        return PublishLocalCallback(updateTimestamp, localRecord, syncRecord, onConflict);
    }

    return errCode;
}

int SQLiteSingleVerNaturalStoreConnection::PublishLocalCallback(bool updateTimestamp,
    const SingleVerRecord &localRecord, const SingleVerRecord &syncRecord, const KvStoreNbPublishAction &onConflict)
{
    bool isLocalLastest = updateTimestamp ? true : (localRecord.timeStamp > syncRecord.writeTimeStamp);
    if ((syncRecord.flag & DataItem::DELETE_FLAG) == DataItem::DELETE_FLAG) {
        onConflict({localRecord.key, localRecord.value}, nullptr, isLocalLastest);
    } else {
        Entry syncEntry = {syncRecord.key, syncRecord.value};
        onConflict({localRecord.key, localRecord.value}, &syncEntry, isLocalLastest);
    }
    return E_OK;
}

int SQLiteSingleVerNaturalStoreConnection::PublishInner(SingleVerNaturalStoreCommitNotifyData *committedData,
    bool updateTimestamp, SingleVerRecord &localRecord, SingleVerRecord &syncRecord, bool &isNeedCallback)
{
    Key hashKey;
    int errCode = DBCommon::CalcValueHash(localRecord.key, hashKey);
    if (errCode != E_OK) {
        return errCode;
    }

    if (committedData != nullptr) {
        errCode = writeHandle_->DeleteLocalKvData(localRecord.key, committedData, localRecord.value,
            localRecord.timeStamp);
        if (errCode != E_OK) {
            LOGE("Delete local kv data err:%d", errCode);
            return errCode;
        }
    } else {
        if (!writeHandle_->CheckIfKeyExisted(localRecord.key, true, localRecord.value, localRecord.timeStamp)) {
            LOGE("Record not found.");
            return -E_NOT_FOUND;
        }
    }

    // begin to insert entry to sync table
    errCode = CheckDataStatus(localRecord.key, localRecord.value, false);
    if (errCode != E_OK) {
        return errCode;
    }

    errCode = writeHandle_->GetKvDataByHashKey(hashKey, syncRecord);
    if (errCode == E_OK) { // has conflict record
        if (isNeedCallback) {
            return errCode;
        }
        // fix conflict with LAST_WIN policy
        if (updateTimestamp) { // local win
            errCode = SaveEntry({localRecord.key, localRecord.value}, false);
        } else {
            if (localRecord.timeStamp <= syncRecord.writeTimeStamp) { // sync win
                errCode = -E_STALE;
            } else {
                errCode = SaveEntry({localRecord.key, localRecord.value}, false, localRecord.timeStamp);
            }
        }
    } else {
        isNeedCallback = false;
        if (errCode == -E_NOT_FOUND) {
            errCode = SaveEntry({localRecord.key, localRecord.value}, false, localRecord.timeStamp);
        }
    }
    return errCode;
}

int SQLiteSingleVerNaturalStoreConnection::UnpublishToLocal(const Key &key, bool deletePublic, bool updateTimestamp)
{
    int errCode = CheckWritePermission();
    if (errCode != E_OK) {
        return errCode;
    }

    if (IsTransactionStarted()) {
        return -E_NOT_SUPPORT;
    }

    std::lock_guard<std::mutex> lock(transactionMutex_);

    errCode = StartTransactionInner();
    if (errCode != E_OK) {
        return errCode;
    }

    Key hashKey;
    int innerErrCode = E_OK;
    SingleVerRecord syncRecord;
    SingleVerNaturalStoreCommitNotifyData *localCommittedData = nullptr;
    errCode = DBCommon::CalcValueHash(key, hashKey);
    if (errCode != E_OK) {
        goto END;
    }

    errCode = writeHandle_->GetKvDataByHashKey(hashKey, syncRecord);
    if (errCode != E_OK) {
        goto END;
    }

    syncRecord.key = key;
    errCode = UnpublishInner(localCommittedData, syncRecord, updateTimestamp, innerErrCode);
    if (errCode != E_OK) {
        goto END;
    }

    if (deletePublic && (syncRecord.flag & DataItem::DELETE_FLAG) != DataItem::DELETE_FLAG) {
        errCode = SaveEntry({hashKey, {}}, true);
        if (errCode != E_OK) {
            goto END;
        }
    }

END:
    // finalize
    if (errCode != E_OK) {
        int rollbackRet = RollbackInner();
        errCode = (rollbackRet != E_OK) ? rollbackRet : errCode;
    } else {
        errCode = CommitInner();
        if (errCode == E_OK) {
            CommitAndReleaseNotifyData(localCommittedData, true, SQLITE_GENERAL_NS_LOCAL_PUT_EVENT);
        }
    }
    ReleaseCommitData(localCommittedData);

    return (errCode == E_OK) ? innerErrCode : errCode;
}

int SQLiteSingleVerNaturalStoreConnection::UnpublishInner(SingleVerNaturalStoreCommitNotifyData *&committedData,
    const SingleVerRecord &syncRecord, bool updateTimestamp, int &innerErrCode)
{
    int errCode = E_OK;
    int localOperation = LOCAL_OPR_NONE;
    SingleVerRecord localRecord;

    innerErrCode = -E_LOCAL_DEFEAT;
    if (writeHandle_->CheckIfKeyExisted(syncRecord.key, true, localRecord.value, localRecord.timeStamp)) {
        if ((syncRecord.flag & DataItem::DELETE_FLAG) == DataItem::DELETE_FLAG) {
            if (updateTimestamp || localRecord.timeStamp <= syncRecord.writeTimeStamp) { // sync win
                innerErrCode = -E_LOCAL_DELETED;
                localOperation = LOCAL_OPR_DEL;
            }
        } else if (updateTimestamp || localRecord.timeStamp <= syncRecord.writeTimeStamp) { // sync win
            innerErrCode = -E_LOCAL_COVERED;
            localOperation = LOCAL_OPR_PUT;
        }
    } else { // no conflict entry in local
        innerErrCode = E_OK;
        if ((syncRecord.flag & DataItem::DELETE_FLAG) != DataItem::DELETE_FLAG) {
            localOperation = LOCAL_OPR_PUT;
        }
    }

    if (localOperation != LOCAL_OPR_NONE) {
        errCode = UnpublishOper(committedData, syncRecord, updateTimestamp, localOperation);
    }

    return errCode;
}

int SQLiteSingleVerNaturalStoreConnection::UnpublishOper(SingleVerNaturalStoreCommitNotifyData *&committedData,
    const SingleVerRecord &syncRecord, bool updateTimestamp, int operType)
{
    committedData = new (std::nothrow) SingleVerNaturalStoreCommitNotifyData;
    if (committedData == nullptr) {
        return -E_OUT_OF_MEMORY;
    }

    int errCode = E_OK;
    if (operType == LOCAL_OPR_PUT) {
        SQLiteSingleVerNaturalStore *naturalStore = GetDB<SQLiteSingleVerNaturalStore>();
        if (naturalStore == nullptr) {
            return -E_INVALID_DB;
        }

        errCode = CheckDataStatus(syncRecord.key, syncRecord.value, false);
        if (errCode != E_OK) {
            return errCode;
        }

        TimeStamp time = updateTimestamp ? naturalStore->GetCurrentTimeStamp() : syncRecord.writeTimeStamp;
        errCode = writeHandle_->PutKvData(SingleVerDataType::LOCAL_TYPE, syncRecord.key, syncRecord.value, time,
            committedData);
    } else if (operType == LOCAL_OPR_DEL) {
        TimeStamp localTimeStamp = 0;
        Value value;
        errCode = writeHandle_->DeleteLocalKvData(syncRecord.key, committedData, value, localTimeStamp);
    }

    return errCode;
}

void SQLiteSingleVerNaturalStoreConnection::ReleaseCommitData(SingleVerNaturalStoreCommitNotifyData *&committedData)
{
    if (committedData != nullptr) {
        committedData->DecObjRef(committedData);
        committedData = nullptr;
    }
}

int SQLiteSingleVerNaturalStoreConnection::PragmaPublish(void *parameter)
{
    PragmaPublishInfo *info = static_cast<PragmaPublishInfo *>(parameter);
    if (info == nullptr) {
        return -E_INVALID_ARGS;
    }
    if (IsExtendedCacheDBMode()) {
        int err = IsCacheDBMode() ? -E_EKEYREVOKED : -E_BUSY;
        LOGE("[PragmaPublish]Existed cache database can not read data, errCode = [%d]!", err);
        return err;
    }
    return PublishLocal(info->key, info->deleteLocal, info->updateTimestamp, info->action);
}

int SQLiteSingleVerNaturalStoreConnection::PragmaUnpublish(void *parameter)
{
    PragmaUnpublishInfo *info = static_cast<PragmaUnpublishInfo *>(parameter);
    if (info == nullptr) {
        return -E_INVALID_ARGS;
    }
    if (IsExtendedCacheDBMode()) {
        int err = IsCacheDBMode() ? -E_EKEYREVOKED : -E_BUSY;
        LOGE("[PragmaUnpublish]Existed cache database can not read data, errCode = [%d]!", err);
        return err;
    }
    return UnpublishToLocal(info->key, info->isDeleteSync, info->isUpdateTime);
}

int SQLiteSingleVerNaturalStoreConnection::PragmaSetAutoLifeCycle(const uint32_t *lifeTime)
{
    if (lifeTime == nullptr || *lifeTime > MAX_AUTO_LIFE_CYCLE || *lifeTime < MIN_AUTO_LIFE_CYCLE) {
        return -E_INVALID_ARGS;
    }
    if (kvDB_ == nullptr) {
        return -E_INVALID_DB;
    }
    return static_cast<SQLiteSingleVerNaturalStore *>(kvDB_)->SetAutoLifeCycleTime(*lifeTime);
}

int SQLiteSingleVerNaturalStoreConnection::PragmaResultSetCacheMode(PragmaData inMode)
{
    if (inMode == nullptr) {
        return -E_INVALID_ARGS;
    }
    auto mode = *(static_cast<ResultSetCacheMode *>(inMode));
    if (mode != ResultSetCacheMode::CACHE_FULL_ENTRY && mode != ResultSetCacheMode::CACHE_ENTRY_ID_ONLY) {
        return -E_INVALID_ARGS;
    }
    cacheModeForNewResultSet_.store(mode);
    return E_OK;
}

int SQLiteSingleVerNaturalStoreConnection::PragmaResultSetCacheMaxSize(PragmaData inSize)
{
    if (inSize == nullptr) {
        return -E_INVALID_ARGS;
    }
    int size = *(static_cast<int *>(inSize));
    if (size < RESULT_SET_CACHE_MAX_SIZE_MIN || size > RESULT_SET_CACHE_MAX_SIZE_MAX) {
        return -E_INVALID_ARGS;
    }
    cacheMaxSizeForNewResultSet_.store(size);
    return E_OK;
}

// use for getkvstore migrating cache data
int SQLiteSingleVerNaturalStoreConnection::PragmaTriggerToMigrateData(const SecurityOption &secOption) const
{
    if ((secOption.securityLabel != S3) || (secOption.securityFlag != SECE)) {
        LOGD("Only S3 SECE data need migrate data!");
        return E_OK;
    }

    LOGI("Begin trigger the migration data while open the database!");
    SQLiteSingleVerNaturalStore *naturalStore = GetDB<SQLiteSingleVerNaturalStore>();
    if (naturalStore == nullptr) {
        return -E_INVALID_CONNECTION;
    }
    return naturalStore->TriggerToMigrateData();
}

int SQLiteSingleVerNaturalStoreConnection::CheckAmendValueContentForLocalProcedure(const Value &oriValue,
    Value &amendValue) const
{
    SQLiteSingleVerNaturalStore *naturalStore = GetDB<SQLiteSingleVerNaturalStore>();
    if (naturalStore == nullptr) { // Not Likely
        return -E_INVALID_DB;
    }
    bool useAmendValue = false;
    return naturalStore->CheckValueAndAmendIfNeed(ValueSource::FROM_LOCAL, oriValue, amendValue, useAmendValue);
}

DEFINE_OBJECT_TAG_FACILITIES(SQLiteSingleVerNaturalStoreConnection)
}