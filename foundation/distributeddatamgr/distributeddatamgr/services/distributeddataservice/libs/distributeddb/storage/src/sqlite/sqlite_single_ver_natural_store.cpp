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

#include "sqlite_single_ver_natural_store.h"

#include <algorithm>
#include <thread>
#include <chrono>

#include "db_common.h"
#include "db_constant.h"
#include "sqlite_utils.h"
#include "storage_engine_manager.h"
#include "sqlite_single_ver_storage_engine.h"
#include "sqlite_single_ver_natural_store_connection.h"
#include "db_errno.h"
#include "log_print.h"
#include "value_hash_calc.h"
#include "hash.h"
#include "platform_specific.h"
#include "db_constant.h"
#include "package_file.h"
#include "generic_single_ver_kv_entry.h"
#include "schema_object.h"
#include "kvdb_utils.h"
#include "single_ver_database_oper.h"

namespace DistributedDB {
#define CHECK_STORAGE_ENGINE do { \
    if (storageEngine_ == nullptr) { \
        return -E_INVALID_DB; \
    } \
} while (0)

namespace {
    constexpr int MAX_SYNC_BLOCK_SIZE = 31457280; // 30MB
    constexpr int DEF_LIFE_CYCLE_TIME = 60000; // 60s
    constexpr int WAIT_DELEGATE_CALLBACK_TIME = 100;

    // Currently this function only suitable to be call from sync in insert_record_from_sync procedure
    // Take attention if future coder attempt to call it in other situation procedure
    int SaveSyncItems(SQLiteSingleVerStorageExecutor *handle, std::vector<DataItem> &dataItems,
        const DeviceInfo &deviceInfo, TimeStamp &maxTimestamp, SingleVerNaturalStoreCommitNotifyData *commitData)
    {
        int errCode = handle->StartTransaction(TransactType::IMMEDIATE);
        if (errCode != E_OK) {
            return errCode;
        }

        int innerCode;
        errCode = handle->PrepareForSavingData(SingleVerDataType::SYNC_TYPE);
        if (errCode != E_OK) {
            goto END;
        }

        for (auto &item : dataItems) {
            if (item.neglect) { // Do not save this record if it is neglected
                continue;
            }
            errCode = handle->SaveSyncDataItem(item, deviceInfo, maxTimestamp, commitData);
            if (errCode != E_OK && errCode != -E_NOT_FOUND) {
                break;
            }
        }

        if (errCode == -E_NOT_FOUND) {
            errCode = E_OK;
        }

        innerCode = handle->ResetForSavingData(SingleVerDataType::SYNC_TYPE);
        if (innerCode != E_OK) {
            errCode = innerCode;
        }
    END:
        if (errCode == E_OK) {
            errCode = handle->Commit();
        } else {
            (void)handle->Rollback(); // Keep the error code of the first scene
        }
        return errCode;
    }

    void ProcessContinueToken(int &errCode, TimeStamp end, const std::vector<DataItem> &dataItems,
        ContinueTokenStruct *&token)
    {
        if (token == nullptr) {
            return;
        }
        if (!dataItems.empty()) {
            TimeStamp timestamp = dataItems.back().timeStamp;
            if (timestamp > INT64_MAX - 1) {
                token->SetBeginTimeStamp(INT64_MAX);
            } else {
                token->SetBeginTimeStamp(timestamp + 1); // Add 1 for the timestamp.
            }
            token->SetEndTimeStamp(end);
        } else {
            delete token;
            token = nullptr;
            errCode = -E_INTERNAL_ERROR;
        }
    }

    void UpdateSecProperties(KvDBProperties &properties, bool isReadOnly, const SchemaObject &savedSchemaObj,
        const SQLiteSingleVerStorageEngine *engine)
    {
        if (isReadOnly) {
            properties.SetSchema(savedSchemaObj);
            properties.SetBoolProp(KvDBProperties::FIRST_OPEN_IS_READ_ONLY, true);
        }
        // Update the security option from the storage engine for that
        // we will not update the security label and flag for the existed database.
        // So the security label and flag are from the existed database.
        if (engine == nullptr) {
            return;
        }
        properties.SetIntProp(KvDBProperties::SECURITY_LABEL, engine->GetSecurityOption().securityLabel);
        properties.SetIntProp(KvDBProperties::SECURITY_FLAG, engine->GetSecurityOption().securityFlag);
    }
}

SQLiteSingleVerNaturalStore::SQLiteSingleVerNaturalStore()
    : currentMaxTimeStamp_(0),
      storageEngine_(nullptr),
      notificationEventsRegistered_(false),
      notificationConflictEventsRegistered_(false),
      isInitialized_(false),
      isReadOnly_(false),
      lifeCycleNotifier_(nullptr),
      lifeTimerId_(0),
      autoLifeTime_(DEF_LIFE_CYCLE_TIME)
{}

SQLiteSingleVerNaturalStore::~SQLiteSingleVerNaturalStore()
{
    ReleaseResources();
}

std::string SQLiteSingleVerNaturalStore::GetDatabasePath(const KvDBProperties &kvDBProp)
{
    std::string dataDir = kvDBProp.GetStringProp(KvDBProperties::DATA_DIR, "");
    std::string identifierDir = kvDBProp.GetStringProp(KvDBProperties::IDENTIFIER_DIR, "");
    std::string filePath = dataDir + "/" + identifierDir + "/" + DBConstant::SINGLE_SUB_DIR + "/" +
        DBConstant::MAINDB_DIR + "/" + DBConstant::SINGLE_VER_DATA_STORE + DBConstant::SQLITE_DB_EXTENSION;
    return filePath;
}

std::string SQLiteSingleVerNaturalStore::GetSubDirPath(const KvDBProperties &kvDBProp)
{
    std::string dataDir = kvDBProp.GetStringProp(KvDBProperties::DATA_DIR, "");
    std::string identifierDir = kvDBProp.GetStringProp(KvDBProperties::IDENTIFIER_DIR, "");
    std::string dirPath = dataDir + "/" + identifierDir + "/" + DBConstant::SINGLE_SUB_DIR;
    return dirPath;
}

int SQLiteSingleVerNaturalStore::SetUserVer(const KvDBProperties &kvDBProp, int version)
{
    OpenDbProperties properties;
    properties.uri = GetDatabasePath(kvDBProp);
    bool isEncryptedDb = kvDBProp.GetBoolProp(KvDBProperties::ENCRYPTED_MODE, false);
    if (isEncryptedDb) {
        kvDBProp.GetPassword(properties.cipherType, properties.passwd);
    }

    int errCode = SQLiteUtils::SetUserVer(properties, version);
    if (errCode != E_OK) {
        LOGE("Recover for open db failed in single version:%d", errCode);
    }
    return errCode;
}

int SQLiteSingleVerNaturalStore::InitDatabaseContext(const KvDBProperties &kvDBProp, bool isNeedUpdateSecOpt)
{
    int errCode = InitStorageEngine(kvDBProp, isNeedUpdateSecOpt);
    if (errCode != E_OK) {
        return errCode;
    }
    InitCurrentMaxStamp();
    return errCode;
}

int SQLiteSingleVerNaturalStore::RegisterLifeCycleCallback(const DatabaseLifeCycleNotifier &notifier)
{
    std::lock_guard<std::mutex> lock(lifeCycleMutex_);
    int errCode;
    if (!notifier) {
        if (lifeTimerId_ == 0) {
            return E_OK;
        }
        errCode = StopLifeCycleTimer();
        if (errCode != E_OK) {
            LOGE("Stop the life cycle timer failed:%d", errCode);
        }
        return E_OK;
    }

    if (notifier && lifeTimerId_ != 0) {
        errCode = StopLifeCycleTimer();
        if (errCode != E_OK) {
            LOGE("Stop the life cycle timer failed:%d", errCode);
        }
    }
    errCode = StartLifeCycleTimer(notifier);
    if (errCode != E_OK) {
        LOGE("Register life cycle timer failed:%d", errCode);
    }
    return errCode;
}

int SQLiteSingleVerNaturalStore::SetAutoLifeCycleTime(uint32_t time)
{
    std::lock_guard<std::mutex> lock(lifeCycleMutex_);
    if (lifeTimerId_ == 0) {
        autoLifeTime_ = time;
    } else {
        auto runtimeCxt = RuntimeContext::GetInstance();
        if (runtimeCxt == nullptr) {
            return -E_INVALID_ARGS;
        }
        LOGI("[SingleVer] Set life cycle to %u", time);
        int errCode = runtimeCxt->ModifyTimer(lifeTimerId_, time);
        if (errCode != E_OK) {
            return errCode;
        }
        autoLifeTime_ = time;
    }
    return E_OK;
}

int SQLiteSingleVerNaturalStore::GetSecurityOption(SecurityOption &option) const
{
    bool isMemDb = GetDbProperties().GetBoolProp(KvDBProperties::MEMORY_MODE, false);
    if (isMemDb) {
        LOGI("[GetSecurityOption] MemDb, no need to get security option");
        option = SecurityOption();
        return E_OK;
    }

    option.securityLabel = GetDbProperties().GetSecLabel();
    option.securityFlag = GetDbProperties().GetSecFlag();

    return E_OK;
}

bool SQLiteSingleVerNaturalStore::IsReadable() const
{
    return true;
}

namespace {
inline bool OriValueCanBeUse(int errCode)
{
    return (errCode == -E_VALUE_MATCH);
}

inline bool AmendValueShouldBeUse(int errCode)
{
    return (errCode == -E_VALUE_MATCH_AMENDED);
}

inline bool ValueIsSomehowWrong(int errCode)
{
    if (errCode == -E_VALUE_MISMATCH_FEILD_COUNT ||
        errCode == -E_VALUE_MISMATCH_FEILD_TYPE ||
        errCode == -E_VALUE_MISMATCH_CONSTRAINT) {
        return true;
    }
    return false;
}
}

int SQLiteSingleVerNaturalStore::CheckValueAndAmendIfNeed(ValueSource sourceType, const Value &oriValue,
    Value &amendValue, bool &useAmendValue) const
{
    // oriValue size may already be checked previously, but check here const little
    if (oriValue.size() > DBConstant::MAX_VALUE_SIZE) {
        return -E_INVALID_ARGS;
    }
    const SchemaObject &schemaObjRef = MyProp().GetSchemaConstRef();
    if (!schemaObjRef.IsSchemaValid()) {
        // Not a schema database, do not need to check more
        return E_OK;
    }
    if (schemaObjRef.GetSchemaType() == SchemaType::JSON) {
        ValueObject valueObj;
        int errCode = valueObj.Parse(oriValue.data(), oriValue.data() + oriValue.size(), schemaObjRef.GetSkipSize());
        if (errCode != E_OK) {
            return -E_INVALID_FORMAT;
        }
        errCode = schemaObjRef.CheckValueAndAmendIfNeed(sourceType, valueObj);
        if (OriValueCanBeUse(errCode)) {
            useAmendValue = false;
            return E_OK;
        }
        if (AmendValueShouldBeUse(errCode)) {
            std::string amended = valueObj.ToString();
            if (amended.size() > DBConstant::MAX_VALUE_SIZE) {
                LOGE("[SqlSinStore][CheckAmendValue] ValueSize=%zu exceed limit after amend.", amended.size());
                return -E_INVALID_FORMAT;
            }
            amendValue.clear();
            amendValue.assign(amended.begin(), amended.end());
            useAmendValue = true;
            return E_OK;
        }
        if (ValueIsSomehowWrong(errCode)) {
            return errCode;
        }
    } else {
        int errCode = schemaObjRef.VerifyValue(sourceType, oriValue);
        if (errCode == E_OK) {
            useAmendValue = false;
            return E_OK;
        }
    }
    // Any unexpected wrong
    return -E_INVALID_FORMAT;
}

int SQLiteSingleVerNaturalStore::ClearIncompleteDatabase(const KvDBProperties &kvDBPro) const
{
    std::string dbSubDir = SQLiteSingleVerNaturalStore::GetSubDirPath(kvDBPro);
    if (OS::CheckPathExistence(dbSubDir + DBConstant::PATH_POSTFIX_DB_INCOMPLETE)) {
        int errCode = DBCommon::RemoveAllFilesOfDirectory(dbSubDir);
        if (errCode != E_OK) {
            LOGE("Remove the incomplete database dir failed!");
            return -E_REMOVE_FILE;
        }
    }

    return E_OK;
}

int SQLiteSingleVerNaturalStore::CheckDatabaseRecovery(const KvDBProperties &kvDBProp)
{
    std::unique_ptr<SingleVerDatabaseOper> operation = std::make_unique<SingleVerDatabaseOper>(this, nullptr);
    (void)operation->ClearExportedTempFiles(kvDBProp);
    int errCode = operation->RekeyRecover(kvDBProp);
    if (errCode != E_OK) {
        LOGE("Recover from rekey failed in single version:%d", errCode);
        return errCode;
    }

    errCode = operation->ClearImportTempFile(kvDBProp);
    if (errCode != E_OK) {
        LOGE("Clear imported temp db failed in single version:%d", errCode);
        return errCode;
    }

    // Currently, Design for the consistency of directory and file setting secOption
    errCode = ClearIncompleteDatabase(kvDBProp);
    if (errCode != E_OK) {
        LOGE("Clear incomplete database failed in single version:%d", errCode);
        return errCode;
    }
    const std::string dataDir = kvDBProp.GetStringProp(KvDBProperties::DATA_DIR, "");
    const std::string identifierDir = kvDBProp.GetStringProp(KvDBProperties::IDENTIFIER_DIR, "");
    bool isCreate = kvDBProp.GetBoolProp(KvDBProperties::CREATE_IF_NECESSARY, true);
    bool isMemoryDb = kvDBProp.GetBoolProp(KvDBProperties::MEMORY_MODE, false);
    if (!isMemoryDb) {
        errCode = DBCommon::CreateStoreDirectory(dataDir, identifierDir, DBConstant::SINGLE_SUB_DIR, isCreate);
        if (errCode != E_OK) {
            LOGE("Create single version natural store directory failed:%d", errCode);
        }
    }
    return errCode;
}

int SQLiteSingleVerNaturalStore::Open(const KvDBProperties &kvDBProp)
{
    std::lock_guard<std::mutex> lock(initialMutex_);
    if (isInitialized_) {
        return E_OK; // avoid the reopen operation.
    }

    int errCode = CheckDatabaseRecovery(kvDBProp);
    if (errCode != E_OK) {
        return errCode;
    }

    bool isReadOnly = false;
    SchemaObject savedSchemaObj;
    storageEngine_ =
        static_cast<SQLiteSingleVerStorageEngine *>(StorageEngineManager::GetStorageEngine(kvDBProp, errCode));
    if (errCode != E_OK) {
        goto ERROR;
    }

    if (storageEngine_->IsEngineCorrupted()) {
        errCode = -E_INVALID_PASSWD_OR_CORRUPTED_DB;
        LOGE("[SqlSinStore][Open] database engine is corrupted, not need continue to open! errCode = [%d]", errCode);
        goto ERROR;
    }

    errCode = InitDatabaseContext(kvDBProp);
    if (errCode != E_OK) {
        LOGE("[SqlSinStore][Open] Init database context fail! errCode = [%d]", errCode);
        goto ERROR;
    }
    errCode = RegisterNotification();
    if (errCode != E_OK) {
        LOGE("Register notification failed:%d", errCode);
        goto ERROR;
    }
    // Here, the dbfile is created or opened, and upgrade of table structure has done.
    // More, Upgrade of schema is also done in upgrader call in InitDatabaseContext, schema in dbfile updated if need.
    // If inputSchema is empty, upgrader do nothing of schema, isReadOnly will be true if dbfile contain schema before.
    // In this case, we should load the savedSchema for checking value from sync which not restricted by readOnly.
    // If inputSchema not empty, isReadOnly will not be true, we should do nothing more.
    errCode = DecideReadOnlyBaseOnSchema(kvDBProp, isReadOnly, savedSchemaObj);
    if (errCode != E_OK) {
        LOGE("[SqlSinStore][Open] DecideReadOnlyBaseOnSchema failed=%d", errCode);
        goto ERROR;
    }
    // Set KvDBProperties and set Schema
    MyProp() = kvDBProp;
    UpdateSecProperties(MyProp(), isReadOnly, savedSchemaObj, storageEngine_);

    StartSyncer();
    InitialLocalDataTimestamp();
    isInitialized_ = true;
    isReadOnly_ = isReadOnly;
    return E_OK;
ERROR:
    ReleaseResources();
    return errCode;
}

void SQLiteSingleVerNaturalStore::Close()
{
    SyncAbleKvDB::Close();
    ReleaseResources();
}

GenericKvDBConnection *SQLiteSingleVerNaturalStore::NewConnection(int &errCode)
{
    SQLiteSingleVerNaturalStoreConnection *connection = new (std::nothrow) SQLiteSingleVerNaturalStoreConnection(this);
    if (connection == nullptr) {
        errCode = -E_OUT_OF_MEMORY;
        return nullptr;
    }
    errCode = E_OK;
    return connection;
}

// Get interface type of this kvdb.
int SQLiteSingleVerNaturalStore::GetInterfaceType() const
{
    return SYNC_SVD;
}

// Get the interface ref-count, in order to access asynchronously.
void SQLiteSingleVerNaturalStore::IncRefCount()
{
    IncObjRef(this);
}

// Drop the interface ref-count.
void SQLiteSingleVerNaturalStore::DecRefCount()
{
    DecObjRef(this);
}

// Get the identifier of this kvdb.
std::vector<uint8_t> SQLiteSingleVerNaturalStore::GetIdentifier() const
{
    std::string identifier = MyProp().GetStringProp(KvDBProperties::IDENTIFIER_DATA, "");
    std::vector<uint8_t> identifierVect(identifier.begin(), identifier.end());
    return identifierVect;
}

// Get interface for syncer.
IKvDBSyncInterface *SQLiteSingleVerNaturalStore::GetSyncInterface()
{
    return this;
}

int SQLiteSingleVerNaturalStore::GetMetaData(const Key &key, Value &value) const
{
    CHECK_STORAGE_ENGINE;
    if (key.size() > DBConstant::MAX_KEY_SIZE) {
        return -E_INVALID_ARGS;
    }

    int errCode = E_OK;
    auto handle = GetHandle(true, errCode);
    if (handle == nullptr) {
        return errCode;
    }

    TimeStamp timeStamp;
    errCode = handle->GetKvData(SingleVerDataType::META_TYPE, key, value, timeStamp);
    ReleaseHandle(handle);
    HeartBeatForLifeCycle();
    return errCode;
}

int SQLiteSingleVerNaturalStore::PutMetaData(const Key &key, const Value &value)
{
    int errCode = SQLiteSingleVerNaturalStore::CheckDataStatus(key, value, false);
    if (errCode != E_OK) {
        return errCode;
    }

    SQLiteSingleVerStorageExecutor *handle = GetHandle(true, errCode);
    if (handle == nullptr) {
        return errCode;
    }

    errCode = handle->PutKvData(SingleVerDataType::META_TYPE, key, value, 0, nullptr); // meta doesn't need time.
    if (errCode != E_OK) {
        LOGE("Put kv data err:%d", errCode);
    }

    HeartBeatForLifeCycle();
    ReleaseHandle(handle);
    return errCode;
}

int SQLiteSingleVerNaturalStore::GetAllMetaKeys(std::vector<Key> &keys) const
{
    CHECK_STORAGE_ENGINE;
    int errCode = E_OK;
    SQLiteSingleVerStorageExecutor *handle = GetHandle(true, errCode);
    if (handle == nullptr) {
        return errCode;
    }

    errCode = handle->GetAllMetaKeys(keys);
    ReleaseHandle(handle);
    return errCode;
}

void SQLiteSingleVerNaturalStore::CommitAndReleaseNotifyData(SingleVerNaturalStoreCommitNotifyData *&committedData,
    bool isNeedCommit, int eventType)
{
    if (isNeedCommit) {
        if (committedData != nullptr) {
            if (!committedData->IsChangedDataEmpty()) {
                CommitNotify(eventType, committedData);
            }
            if (!committedData->IsConflictedDataEmpty()) {
                CommitNotify(SQLITE_GENERAL_CONFLICT_EVENT, committedData);
            }
        }
    }

    if (committedData != nullptr) {
        committedData->DecObjRef(committedData);
        committedData = nullptr;
    }
}

int SQLiteSingleVerNaturalStore::GetSyncData(TimeStamp begin, TimeStamp end, std::vector<SingleVerKvEntry *> &entries,
    ContinueToken &continueStmtToken, const DataSizeSpecInfo &dataSizeInfo) const
{
    int errCode = CheckReadDataControlled();
    if (errCode != E_OK) {
        LOGE("[GetSyncData] Existed cache database can not read data, errCode = [%d]!", errCode);
        return errCode;
    }

    std::vector<DataItem> dataItems;
    errCode = GetSyncData(begin, end, dataItems, continueStmtToken, dataSizeInfo);
    if (errCode != E_OK && errCode != -E_UNFINISHED) {
        LOGE("GetSyncData errCode:%d", errCode);
        goto ERROR;
    }

    for (auto &item : dataItems) {
        GenericSingleVerKvEntry *entry = new (std::nothrow) GenericSingleVerKvEntry();
        if (entry == nullptr) {
            errCode = -E_OUT_OF_MEMORY;
            LOGE("GetSyncData errCode:%d", errCode);
            goto ERROR;
        }
        entry->SetEntryData(std::move(item));
        entries.push_back(entry);
    }

ERROR:
    if (errCode != E_OK && errCode != -E_UNFINISHED) {
        for (auto &itemEntry : entries) {
            delete itemEntry;
            itemEntry = nullptr;
        }
        entries.clear();
    }
    HeartBeatForLifeCycle();
    return errCode;
}

int SQLiteSingleVerNaturalStore::GetSyncData(TimeStamp begin, TimeStamp end, std::vector<DataItem> &dataItems,
    ContinueToken &continueStmtToken, const DataSizeSpecInfo &dataSizeInfo) const
{
    if (begin >= end || dataSizeInfo.blockSize > MAX_SYNC_BLOCK_SIZE) {
        return -E_INVALID_ARGS;
    }

    auto token = new (std::nothrow) ContinueTokenStruct;
    if (token == nullptr) {
        LOGE("[SQLiteSingleVerNaturalStore][NewToken] Bad alloc.");
        return -E_OUT_OF_MEMORY;
    }

    int errCode = E_OK;
    SQLiteSingleVerStorageExecutor *handle = GetHandle(false, errCode);
    if (handle == nullptr) {
        goto ERROR;
    }

    errCode = handle->GetSyncDataByTimestamp(dataItems, GetAppendedLen(), begin, end, dataSizeInfo);
    if (errCode == -E_FINISHED) {
        errCode = E_OK;
    }

ERROR:
    if (errCode != -E_UNFINISHED) {
        if (errCode != E_OK) {
            dataItems.clear();
        }
        delete token;
        token = nullptr;
    } else {
        ProcessContinueToken(errCode, end, dataItems, token);
    }

    if (handle != nullptr) {
        ReleaseHandle(handle);
    }
    continueStmtToken = static_cast<ContinueToken>(token);
    return errCode;
}

int SQLiteSingleVerNaturalStore::GetSyncDataNext(std::vector<SingleVerKvEntry *> &entries,
    ContinueToken &continueStmtToken, const DataSizeSpecInfo &dataSizeInfo) const
{
    int errCode = CheckReadDataControlled();
    if (errCode != E_OK) {
        LOGE("[GetSyncDataNext] Existed cache database can not read data, errCode = [%d]!", errCode);
        return errCode;
    }

    std::vector<DataItem> dataItems;
    errCode = GetSyncDataNext(dataItems, continueStmtToken, dataSizeInfo);
    if (errCode != E_OK && errCode != -E_UNFINISHED) {
        LOGE("GetSyncDataNext errCode:%d", errCode);
        goto ERROR;
    }

    for (auto &item : dataItems) {
        GenericSingleVerKvEntry *entry = new (std::nothrow) GenericSingleVerKvEntry();
        if (entry == nullptr) {
            errCode = -E_OUT_OF_MEMORY;
            LOGE("GetSyncDataNext errCode:%d", errCode);
            goto ERROR;
        }
        entry->SetEntryData(std::move(item));
        entries.push_back(entry);
    }

ERROR:
    if (errCode != E_OK && errCode != -E_UNFINISHED) {
        for (auto &itemEntry : entries) {
            delete itemEntry;
            itemEntry = nullptr;
        }
        entries.clear();
    }
    return errCode;
}

int SQLiteSingleVerNaturalStore::GetSyncDataNext(std::vector<DataItem> &dataItems, ContinueToken &continueStmtToken,
    const DataSizeSpecInfo &dataSizeInfo) const
{
    if (dataSizeInfo.blockSize > MAX_SYNC_BLOCK_SIZE) {
        return -E_INVALID_ARGS;
    }

    auto token = static_cast<ContinueTokenStruct *>(continueStmtToken);
    if (token == nullptr || !(token->CheckValid())) {
        LOGE("[SingleVerNaturalStore][GetSyncDataNext] invalid continue token.");
        return -E_INVALID_ARGS;
    }

    int errCode = E_OK;
    SQLiteSingleVerStorageExecutor *handle = GetHandle(false, errCode);
    if (handle == nullptr) {
        ReleaseContinueToken(continueStmtToken);
        return errCode;
    }

    errCode = handle->GetSyncDataByTimestamp(dataItems, GetAppendedLen(), token->GetBeginTimeStamp(),
        token->GetEndTimeStamp(), dataSizeInfo);
    if (errCode == -E_UNFINISHED) {
        // update the begin timestamp of the next fetch.
        if (!dataItems.empty()) {
            TimeStamp timestamp = dataItems.back().timeStamp;
            if (timestamp >= INT64_MAX) {
                token->SetBeginTimeStamp(INT64_MAX);
            } else {
                token->SetBeginTimeStamp(timestamp + 1); // Add 1 for the next fetch.
            }
            continueStmtToken = static_cast<ContinueToken>(token);
        } else {
            ReleaseContinueToken(continueStmtToken);
            errCode = -E_INTERNAL_ERROR;
        }
    } else {
        ReleaseContinueToken(continueStmtToken);
        if (errCode == -E_FINISHED) {
            errCode = E_OK;
        }
    }
    ReleaseHandle(handle);
    return errCode;
}

void SQLiteSingleVerNaturalStore::ReleaseContinueToken(ContinueToken &continueStmtToken) const
{
    auto token = static_cast<ContinueTokenStruct *>(continueStmtToken);
    if (token == nullptr || !(token->CheckValid())) {
        LOGE("[SQLiteSingleVerNaturalStore][ReleaseContinueToken] Input is not a continue token.");
        return;
    }
    delete token;
    continueStmtToken = nullptr;
    return;
}

int SQLiteSingleVerNaturalStore::PutSyncData(const std::vector<SingleVerKvEntry *> &entries,
    const std::string &deviceName)
{
    std::vector<DataItem> dataItems;
    for (auto itemEntry : entries) {
        GenericSingleVerKvEntry *entry = static_cast<GenericSingleVerKvEntry *>(itemEntry);
        if (entry != nullptr) {
            DataItem item;
            item.origDev = entry->GetOrigDevice();
            item.flag = entry->GetFlag();
            item.timeStamp = entry->GetTimestamp();
            item.writeTimeStamp = entry->GetWriteTimestamp();
            entry->GetKey(item.key);
            entry->GetValue(item.value);
            dataItems.push_back(item);
        }
    }
    HeartBeatForLifeCycle();
    return PutSyncData(dataItems, deviceName);
}

int SQLiteSingleVerNaturalStore::PutSyncData(std::vector<DataItem> &dataItems, const std::string &deviceName)
{
    if (deviceName.length() > DBConstant::MAX_DEV_LENGTH) {
        LOGW("Device length is invalid for sync put");
        return -E_INVALID_ARGS;
    }
    DeviceInfo deviceInfo = {false, deviceName};
    if (deviceName.empty()) {
        deviceInfo.deviceName = "Unknown";
    }

    int errCode = SaveSyncDataItems(dataItems, deviceInfo, true); // Currently true to check value content
    if (errCode != E_OK) {
        LOGE("PutSyncData errCode:%d", errCode);
    }

    return errCode;
}

void SQLiteSingleVerNaturalStore::ReleaseKvEntry(const SingleVerKvEntry *entry)
{
    if (entry != nullptr) {
        delete entry;
        entry = nullptr;
    }
}

void SQLiteSingleVerNaturalStore::GetMaxTimeStamp(TimeStamp &stamp) const
{
    std::lock_guard<std::mutex> lock(maxTimeStampMutex_);
    stamp = currentMaxTimeStamp_;
}

int SQLiteSingleVerNaturalStore::SetMaxTimeStamp(TimeStamp timestamp)
{
    std::lock_guard<std::mutex> lock(maxTimeStampMutex_);
    if (timestamp > currentMaxTimeStamp_) {
        currentMaxTimeStamp_ = timestamp;
    }
    return E_OK;
}

// In sync procedure, call this function
int SQLiteSingleVerNaturalStore::RemoveDeviceData(const std::string &deviceName, bool isNeedNotify)
{
    LOGI("[RemoveDeviceData] %s{private} rebuild, clear historydata", deviceName.c_str());
    return RemoveDeviceData(deviceName, isNeedNotify, true);
}

// In local procedure, call this function
int SQLiteSingleVerNaturalStore::RemoveDeviceData(const std::string &deviceName, bool isNeedNotify, bool isInSync)
{
    if (deviceName.empty() || deviceName.length() > DBConstant::MAX_DEV_LENGTH) {
        return -E_INVALID_ARGS;
    }
    if (!isInSync && !CheckWritePermission()) {
        return -E_NOT_PERMIT;
    }
    // Call the syncer module to erase the water mark.
    int errCode = EraseDeviceWaterMark(deviceName);
    if (errCode != E_OK) {
        LOGE("[SingleVerNStore] erase water mark failed:%d", errCode);
        return errCode;
    }

    if (IsExtendedCacheDBMode()) {
        errCode = RemoveDeviceDataInCacheMode(deviceName, isNeedNotify);
    } else {
        errCode = RemoveDeviceDataNormally(deviceName, isNeedNotify);
    }
    if (errCode != E_OK) {
        LOGE("[SingleVerNStore] RemoveDeviceData failed:%d", errCode);
    }

    return errCode;
}

int SQLiteSingleVerNaturalStore::RemoveDeviceDataInCacheMode(const std::string &deviceName, bool isNeedNotify)
{
    int errCode = E_OK;
    SQLiteSingleVerStorageExecutor *handle = GetHandle(true, errCode);
    if (handle == nullptr) {
        LOGE("[SingleVerNStore] RemoveDeviceData get handle failed:%d", errCode);
        return errCode;
    }
    uint64_t recordVersion = GetAndIncreaseCacheRecordVersion();
    LOGI("Remove device data in cache mode isNeedNotify : %d, recordVersion : %d", isNeedNotify, recordVersion);
    errCode = handle->RemoveDeviceDataInCacheMode(deviceName, isNeedNotify, recordVersion);
    if (errCode != E_OK) {
        LOGE("[SingleVerNStore] RemoveDeviceDataInCacheMode failed:%d", errCode);
    }
    ReleaseHandle(handle);
    return errCode;
}

int SQLiteSingleVerNaturalStore::RemoveDeviceDataNormally(const std::string &deviceName, bool isNeedNotify)
{
    int errCode = E_OK;
    SQLiteSingleVerStorageExecutor *handle = GetHandle(true, errCode);
    if (handle == nullptr) {
        LOGE("[SingleVerNStore] RemoveDeviceData get handle failed:%d", errCode);
        return errCode;
    }

    std::vector<Entry> entries;
    if (isNeedNotify) {
        handle->GetAllSyncedEntries(deviceName, entries);
    }

    LOGI("Remove device data:%d", isNeedNotify);
    errCode = handle->RemoveDeviceData(deviceName);
    if (errCode == E_OK && isNeedNotify) {
        NotifyRemovedData(entries);
    }
    ReleaseHandle(handle);
    return errCode;
}

void SQLiteSingleVerNaturalStore::NotifyRemovedData(std::vector<Entry> &entries)
{
    if (entries.empty() || entries.size() > MAX_TOTAL_NOTIFY_ITEM_SIZE) {
        return;
    }

    size_t index = 0;
    size_t totalSize = 0;
    SingleVerNaturalStoreCommitNotifyData *notifyData = nullptr;
    while (index < entries.size()) {
        if (notifyData == nullptr) {
            notifyData = new (std::nothrow) SingleVerNaturalStoreCommitNotifyData;
            if (notifyData == nullptr) {
                LOGE("Failed to do commit sync removing because of OOM");
                break;
            }
        }

        // ignore the invalid key.
        if (entries[index].key.size() > DBConstant::MAX_KEY_SIZE ||
            entries[index].value.size() > DBConstant::MAX_VALUE_SIZE) {
            index++;
            continue;
        }

        if ((entries[index].key.size() + entries[index].value.size() + totalSize) > MAX_TOTAL_NOTIFY_DATA_SIZE) {
            CommitAndReleaseNotifyData(notifyData, true, SQLITE_GENERAL_NS_SYNC_EVENT);
            totalSize = 0;
            notifyData = nullptr;
            continue;
        }

        totalSize += (entries[index].key.size() + entries[index].value.size());
        notifyData->InsertCommittedData(std::move(entries[index]), DataType::DELETE, false);
        index++;
    }
    if (notifyData != nullptr) {
        CommitAndReleaseNotifyData(notifyData, true, SQLITE_GENERAL_NS_SYNC_EVENT);
    }
}

SQLiteSingleVerStorageExecutor *SQLiteSingleVerNaturalStore::GetHandle(bool isWrite, int &errCode,
    OperatePerm perm) const
{
    if (storageEngine_ == nullptr) {
        errCode = -E_INVALID_DB;
        return nullptr;
    }
    // Use for check database corrupted in Asynchronous task, like cache data migrate to main database
    if (storageEngine_->IsEngineCorrupted()) {
        CorruptNotify();
        errCode = -E_INVALID_PASSWD_OR_CORRUPTED_DB;
        LOGI("Handle is corrupted can not to get! errCode = [%d]", errCode);
        return nullptr;
    }
    return static_cast<SQLiteSingleVerStorageExecutor *>(storageEngine_->FindExecutor(isWrite, perm, errCode));
}

void SQLiteSingleVerNaturalStore::ReleaseHandle(SQLiteSingleVerStorageExecutor *&handle) const
{
    if (handle == nullptr) {
        return;
    }

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

int SQLiteSingleVerNaturalStore::RegisterNotification()
{
    int errCode = RegisterNotificationEventType(SQLITE_GENERAL_NS_LOCAL_PUT_EVENT);
    if (errCode != E_OK) {
        LOGE("Register single version local event failed:%d!", errCode);
        return errCode;
    }

    errCode = RegisterNotificationEventType(SQLITE_GENERAL_NS_PUT_EVENT);
    if (errCode != E_OK) {
        LOGE("Register single version put event failed:%d!", errCode);
        UnRegisterNotificationEventType(SQLITE_GENERAL_NS_LOCAL_PUT_EVENT);
        return errCode;
    }

    errCode = RegisterNotificationEventType(SQLITE_GENERAL_NS_SYNC_EVENT);
    if (errCode != E_OK) {
        LOGE("Register single version sync event failed:%d!", errCode);
        UnRegisterNotificationEventType(SQLITE_GENERAL_NS_PUT_EVENT);
        UnRegisterNotificationEventType(SQLITE_GENERAL_NS_LOCAL_PUT_EVENT);
        return errCode;
    }

    errCode = RegisterNotificationEventType(SQLITE_GENERAL_CONFLICT_EVENT);
    if (errCode != E_OK) {
        LOGE("Register single version sync event failed:%d!", errCode);
        UnRegisterNotificationEventType(SQLITE_GENERAL_NS_SYNC_EVENT);
        UnRegisterNotificationEventType(SQLITE_GENERAL_NS_PUT_EVENT);
        UnRegisterNotificationEventType(SQLITE_GENERAL_NS_LOCAL_PUT_EVENT);
        return errCode;
    }

    notificationEventsRegistered_ = true;
    notificationConflictEventsRegistered_ = true;
    return E_OK;
}

void SQLiteSingleVerNaturalStore::ReleaseResources()
{
    if (notificationEventsRegistered_) {
        UnRegisterNotificationEventType(static_cast<EventType>(SQLITE_GENERAL_NS_SYNC_EVENT));
        UnRegisterNotificationEventType(static_cast<EventType>(SQLITE_GENERAL_NS_PUT_EVENT));
        UnRegisterNotificationEventType(static_cast<EventType>(SQLITE_GENERAL_NS_LOCAL_PUT_EVENT));
        notificationEventsRegistered_ = false;
    }

    if (notificationConflictEventsRegistered_) {
        UnRegisterNotificationEventType(static_cast<EventType>(SQLITE_GENERAL_CONFLICT_EVENT));
        notificationConflictEventsRegistered_ = false;
    }
    {
        std::lock_guard<std::mutex> lock(syncerMutex_);
        if (storageEngine_ != nullptr) {
            storageEngine_->ClearEnginePasswd();
            (void)StorageEngineManager::ReleaseStorageEngine(storageEngine_);
            storageEngine_ = nullptr;
        }
    }

    isInitialized_ = false;
}

void SQLiteSingleVerNaturalStore::InitCurrentMaxStamp()
{
    if (storageEngine_ == nullptr) {
        return;
    }
    int errCode = E_OK;
    SQLiteSingleVerStorageExecutor *handle = GetHandle(true, errCode);
    if (handle == nullptr) {
        return;
    }

    handle->InitCurrentMaxStamp(currentMaxTimeStamp_);
    LOGD("Init max timestamp:%llu", currentMaxTimeStamp_);
    ReleaseHandle(handle);
}

void SQLiteSingleVerNaturalStore::InitConflictNotifiedFlag(SingleVerNaturalStoreCommitNotifyData *committedData)
{
    unsigned int conflictFlag = 0;
    if (GetRegisterFunctionCount(CONFLICT_SINGLE_VERSION_NS_FOREIGN_KEY_ONLY) != 0) {
        conflictFlag |= static_cast<unsigned>(SQLITE_GENERAL_NS_FOREIGN_KEY_ONLY);
    }
    if (GetRegisterFunctionCount(CONFLICT_SINGLE_VERSION_NS_FOREIGN_KEY_ORIG) != 0) {
        conflictFlag |= static_cast<unsigned>(SQLITE_GENERAL_NS_FOREIGN_KEY_ORIG);
    }
    if (GetRegisterFunctionCount(CONFLICT_SINGLE_VERSION_NS_NATIVE_ALL) != 0) {
        conflictFlag |= static_cast<unsigned>(SQLITE_GENERAL_NS_NATIVE_ALL);
    }
    committedData->SetConflictedNotifiedFlag(static_cast<int>(conflictFlag));
}

// Currently this function only suitable to be call from sync in insert_record_from_sync procedure
// Take attention if future coder attempt to call it in other situation procedure
int SQLiteSingleVerNaturalStore::SaveSyncDataItems(std::vector<DataItem> &dataItems, const DeviceInfo &deviceInfo,
    bool checkValueContent)
{
    // Sync procedure does not care readOnly Flag
    CHECK_STORAGE_ENGINE;
    int errCode = E_OK;
    for (const auto &item : dataItems) {
        // Check only the key and value size
        errCode = CheckDataStatus(item.key, item.value, (item.flag & DataItem::DELETE_FLAG) != 0);
        if (errCode != E_OK) {
            return errCode;
        }
    }
    if (checkValueContent) {
        CheckAmendValueContentForSyncProcedure(dataItems);
    }

    if (IsExtendedCacheDBMode()) {
        errCode = SaveSyncDataToCacheDB(dataItems, deviceInfo);
    } else {
        errCode = SaveSyncDataToMain(dataItems, deviceInfo);
    }
    if (errCode != E_OK) {
        LOGE("[SingleVerNStore] SaveSyncDataItems failed:%d", errCode);
    }
    return errCode;
}

int SQLiteSingleVerNaturalStore::SaveSyncDataToMain(std::vector<DataItem> &dataItems, const DeviceInfo &deviceInfo)
{
    int errCode = E_OK;
    LOGD("[SQLiteSingleVerNaturalStore::SaveSyncData] Get write handle.");
    SQLiteSingleVerStorageExecutor *handle = GetHandle(true, errCode);
    if (handle == nullptr) {
        return errCode;
    }

    auto *committedData = new (std::nothrow) SingleVerNaturalStoreCommitNotifyData;
    if (committedData == nullptr) {
        LOGE("[SingleVerNStore] Failed to alloc single version notify data");
        ReleaseHandle(handle);
        return -E_OUT_OF_MEMORY;
    }

    InitConflictNotifiedFlag(committedData);
    TimeStamp maxTimestamp = 0;
    bool isNeedCommit = false;
    errCode = SaveSyncItems(handle, dataItems, deviceInfo, maxTimestamp, committedData);
    if (errCode == E_OK) {
        isNeedCommit = true;
        (void)SetMaxTimeStamp(maxTimestamp);
    }

    CommitAndReleaseNotifyData(committedData, isNeedCommit, SQLITE_GENERAL_NS_SYNC_EVENT);
    ReleaseHandle(handle);
    return errCode;
}

int SQLiteSingleVerNaturalStore::SaveSyncDataToCacheDB(std::vector<DataItem> &dataItems, const DeviceInfo &deviceInfo)
{
    int errCode = E_OK;
    SQLiteSingleVerStorageExecutor *handle = GetHandle(true, errCode);
    if (handle == nullptr) {
        return errCode;
    }

    TimeStamp maxTimestamp = 0;
    errCode = SaveSyncItemsInCacheMode(handle, dataItems, deviceInfo, maxTimestamp);
    if (errCode != E_OK) {
        LOGE("[SingleVerNStore] Failed to save sync data in cache mode, err : %d", errCode);
    } else {
        (void)SetMaxTimeStamp(maxTimestamp);
    }
    ReleaseHandle(handle);
    return errCode;
}

TimeStamp SQLiteSingleVerNaturalStore::GetCurrentTimeStamp()
{
    return GetTimeStamp();
}

int SQLiteSingleVerNaturalStore::InitStorageEngine(const KvDBProperties &kvDBProp, bool isNeedUpdateSecOpt)
{
    OpenDbProperties option;
    InitDataBaseOption(kvDBProp, option);

    bool isMemoryMode = kvDBProp.GetBoolProp(KvDBProperties::MEMORY_MODE, false);
    StorageEngineAttr poolSize = {1, 1, 1, 16}; // at most 1 write 16 read.
    if (isMemoryMode) {
        poolSize.minWriteNum = 1; // keep at least one connection.
    }

    storageEngine_->SetNotifiedCallback(
        [&](int eventType, KvDBCommitNotifyFilterAbleData *committedData) {
            if (eventType == SQLITE_GENERAL_FINISH_MIGRATE_EVENT) {
                return this->TriggerSync(eventType);
            }
            auto commitData = static_cast<SingleVerNaturalStoreCommitNotifyData *>(committedData);
            this->CommitAndReleaseNotifyData(commitData, true, eventType);
        }
    );

    std::string identifier = kvDBProp.GetStringProp(KvDBProperties::IDENTIFIER_DATA, "");
    storageEngine_->SetNeedUpdateSecOption(isNeedUpdateSecOpt);
    int errCode = storageEngine_->InitSQLiteStorageEngine(poolSize, option, identifier);
    if (errCode != E_OK) {
        LOGE("Init the sqlite storage engine failed:%d", errCode);
    }
    return errCode;
}

int SQLiteSingleVerNaturalStore::Rekey(const CipherPassword &passwd)
{
    // Check the storage engine and try to disable the engine.
    if (storageEngine_ == nullptr) {
        return -E_INVALID_DB;
    }

    std::unique_ptr<SingleVerDatabaseOper> operation;

    // stop the syncer;
    int errCode = storageEngine_->TryToDisable(false, OperatePerm::REKEY_MONOPOLIZE_PERM);
    if (errCode != E_OK) {
        return errCode;
    }
    LOGI("Stop the syncer for rekey");
    StopSyncer();
    std::this_thread::sleep_for(std::chrono::milliseconds(5));  // wait for 5 ms
    errCode = storageEngine_->TryToDisable(true, OperatePerm::REKEY_MONOPOLIZE_PERM);
    if (errCode != E_OK) {
        LOGE("[Rekey] Failed to disable the database: %d", errCode);
        goto END;
    }

    if (storageEngine_->GetEngineState() != EngineState::MAINDB) {
        LOGE("Rekey is not supported while cache exists! state = [%d]", storageEngine_->GetEngineState());
        errCode = (storageEngine_->GetEngineState() == EngineState::CACHEDB) ? -E_NOT_SUPPORT : -E_BUSY;
        goto END;
    }

    operation = std::make_unique<SingleVerDatabaseOper>(this, storageEngine_);
    LOGI("Operation rekey");
    errCode = operation->Rekey(passwd);
END:
    // Only maindb state have existed handle, if rekey fail other state will create error cache db
    // Abort can forbid get new handle, requesting handle will return BUSY and nullptr handle
    if (errCode != -E_FORBID_CACHEDB) {
        storageEngine_->Enable(OperatePerm::REKEY_MONOPOLIZE_PERM);
    } else {
        storageEngine_->Abort(OperatePerm::REKEY_MONOPOLIZE_PERM);
        errCode = E_OK;
    }
    StartSyncer();
    return errCode;
}

int SQLiteSingleVerNaturalStore::Export(const std::string &filePath, const CipherPassword &passwd)
{
    CHECK_STORAGE_ENGINE;
    if (MyProp().GetBoolProp(KvDBProperties::MEMORY_MODE, false)) {
        return -E_NOT_SUPPORT;
    }

    // Exclusively write resources
    std::string localDev;
    int errCode = GetLocalIdentity(localDev);
    if (errCode != E_OK) {
        LOGE("Get local dev id err:%d", errCode);
        localDev.resize(0);
    }

    // The write handle is applied to prevent writing data during the export process.
    SQLiteSingleVerStorageExecutor *handle = GetHandle(true, errCode, OperatePerm::NORMAL_PERM);
    if (handle == nullptr) {
        return errCode;
    }

    // forbid migrate by hold write handle not release
    if (storageEngine_->GetEngineState() != EngineState::MAINDB) {
        LOGE("Not support export when cacheDB existed! state = [%d]", storageEngine_->GetEngineState());
        errCode = (storageEngine_->GetEngineState() == EngineState::CACHEDB) ? -E_NOT_SUPPORT : -E_BUSY;
        ReleaseHandle(handle);
        return errCode;
    }

    std::unique_ptr<SingleVerDatabaseOper> operation = std::make_unique<SingleVerDatabaseOper>(this, storageEngine_);
    operation->SetLocalDevId(localDev);
    LOGI("Begin export the kv store");
    errCode = operation->Export(filePath, passwd);

    ReleaseHandle(handle);
    return errCode;
}

int SQLiteSingleVerNaturalStore::Import(const std::string &filePath, const CipherPassword &passwd)
{
    CHECK_STORAGE_ENGINE;
    if (MyProp().GetBoolProp(KvDBProperties::MEMORY_MODE, false)) {
        return -E_NOT_SUPPORT;
    }

    std::string localDev;
    int errCode = GetLocalIdentity(localDev);
    if (errCode != E_OK) {
        LOGE("Failed to GetLocalIdentity!");
        localDev.resize(0);
    }

    // stop the syncer;
    errCode = storageEngine_->TryToDisable(false, OperatePerm::IMPORT_MONOPOLIZE_PERM);
    if (errCode != E_OK) {
        return errCode;
    }
    StopSyncer();
    std::this_thread::sleep_for(std::chrono::milliseconds(5)); // wait for 5 ms
    std::unique_ptr<SingleVerDatabaseOper> operation;

    errCode = storageEngine_->TryToDisable(true, OperatePerm::IMPORT_MONOPOLIZE_PERM);
    if (errCode != E_OK) {
        LOGE("[Import] Failed to disable the database: %d", errCode);
        goto END;
    }

    if (storageEngine_->GetEngineState() != EngineState::MAINDB) {
        LOGE("Not support import when cacheDB existed! state = [%d]", storageEngine_->GetEngineState());
        errCode = (storageEngine_->GetEngineState() == EngineState::CACHEDB) ? -E_NOT_SUPPORT : -E_BUSY;
        goto END;
    }

    operation = std::make_unique<SingleVerDatabaseOper>(this, storageEngine_);
    operation->SetLocalDevId(localDev);
    errCode = operation->Import(filePath, passwd);
END:
    // restore the storage engine and the syncer.
    storageEngine_->Enable(OperatePerm::IMPORT_MONOPOLIZE_PERM);
    StartSyncer();
    return errCode;
}

bool SQLiteSingleVerNaturalStore::CheckWritePermission() const
{
    return !isReadOnly_;
}

SchemaObject SQLiteSingleVerNaturalStore::GetSchemaInfo() const
{
    return MyProp().GetSchemaConstRef();
}

SchemaObject SQLiteSingleVerNaturalStore::GetSchemaObject() const
{
    return MyProp().GetSchema();
}

const SchemaObject &SQLiteSingleVerNaturalStore::GetSchemaObjectConstRef() const
{
    return MyProp().GetSchemaConstRef();
}

bool SQLiteSingleVerNaturalStore::CheckCompatible(const std::string &schema) const
{
    const SchemaObject &localSchema = MyProp().GetSchemaConstRef();
    if (!localSchema.IsSchemaValid() || schema.empty()) {
        // If at least one of local or remote is normal-kvdb, then allow sync
        LOGI("IsLocalSchemaDb=%d, IsRemoteSchemaDb=%d.", localSchema.IsSchemaValid(), !schema.empty());
        return true;
    }
    // Here both are schema-db, check their compatibility mutually
    SchemaObject remoteSchema;
    int errCode = remoteSchema.ParseFromSchemaString(schema);
    if (errCode != E_OK) {
        // Consider: if the parse errCode is SchemaVersionNotSupport, we can consider allow sync if schemaType equal.
        LOGE("Parse remote schema fail, errCode=%d.", errCode);
        return false;
    }
    // First, Compare remoteSchema based on localSchema
    errCode = localSchema.CompareAgainstSchemaObject(remoteSchema);
    if (errCode != -E_SCHEMA_UNEQUAL_INCOMPATIBLE) {
        LOGI("Remote(Maybe newer) compatible based on local, result=%d.", errCode);
        return true;
    }
    // Second, Compare localSchema based on remoteSchema
    errCode = remoteSchema.CompareAgainstSchemaObject(localSchema);
    if (errCode != -E_SCHEMA_UNEQUAL_INCOMPATIBLE) {
        LOGI("Local(Newer) compatible based on remote, result=%d.", errCode);
        return true;
    }
    LOGE("Local incompatible with remote mutually.");
    return false;
}

void SQLiteSingleVerNaturalStore::InitDataBaseOption(const KvDBProperties &kvDBProp, OpenDbProperties &option)
{
    std::string uri = GetDatabasePath(kvDBProp);
    bool isMemoryDb = kvDBProp.GetBoolProp(KvDBProperties::MEMORY_MODE, false);
    if (isMemoryDb) {
        std::string identifierDir = kvDBProp.GetStringProp(KvDBProperties::IDENTIFIER_DIR, "");
        uri = identifierDir + DBConstant::SQLITE_MEMDB_IDENTIFY;
        LOGD("Begin create memory natural store database");
    }
    std::string subDir = GetSubDirPath(kvDBProp);
    CipherType cipherType;
    CipherPassword passwd;
    kvDBProp.GetPassword(cipherType, passwd);
    std::string schemaStr = kvDBProp.GetSchema().ToSchemaString();

    bool isCreateNecessary = kvDBProp.GetBoolProp(KvDBProperties::CREATE_IF_NECESSARY, true);
    std::vector<std::string> createTableSqls;

    SecurityOption securityOpt;
    if (RuntimeContext::GetInstance()->IsProcessSystemApiAdapterValid()) {
        securityOpt.securityLabel = kvDBProp.GetSecLabel();
        securityOpt.securityFlag = kvDBProp.GetSecFlag();
    }

    option = {uri, isCreateNecessary, isMemoryDb, createTableSqls, cipherType, passwd, schemaStr,
        subDir, securityOpt};
    option.conflictReslovePolicy = kvDBProp.GetIntProp(KvDBProperties::CONFLICT_RESOLVE_POLICY, DEFAULT_LAST_WIN);
    option.createDirByStoreIdOnly = kvDBProp.GetBoolProp(KvDBProperties::CREATE_DIR_BY_STORE_ID_ONLY, false);
}

int SQLiteSingleVerNaturalStore::TransObserverTypeToRegisterFunctionType(
    int observerType, RegisterFuncType &type) const
{
    switch (observerType) {
        case static_cast<uint32_t>(SQLITE_GENERAL_NS_PUT_EVENT):
            type = OBSERVER_SINGLE_VERSION_NS_PUT_EVENT;
            return E_OK;
        case static_cast<uint32_t>(SQLITE_GENERAL_NS_SYNC_EVENT):
            type = OBSERVER_SINGLE_VERSION_NS_SYNC_EVENT;
            return E_OK;
        case static_cast<uint32_t>(SQLITE_GENERAL_NS_LOCAL_PUT_EVENT):
            type = OBSERVER_SINGLE_VERSION_NS_LOCAL_EVENT;
            return E_OK;
        case static_cast<uint32_t>(SQLITE_GENERAL_CONFLICT_EVENT):
            type = OBSERVER_SINGLE_VERSION_NS_CONFLICT_EVENT;
            return E_OK;
        default:
            return -E_NOT_SUPPORT;
    }
}

int SQLiteSingleVerNaturalStore::TransConflictTypeToRegisterFunctionType(
    int conflictType, RegisterFuncType &type) const
{
    switch (conflictType) {
        case static_cast<int>(SQLITE_GENERAL_NS_FOREIGN_KEY_ONLY):
            type = CONFLICT_SINGLE_VERSION_NS_FOREIGN_KEY_ONLY;
            return E_OK;
        case static_cast<int>(SQLITE_GENERAL_NS_FOREIGN_KEY_ORIG):
            type = CONFLICT_SINGLE_VERSION_NS_FOREIGN_KEY_ORIG;
            return E_OK;
        case static_cast<int>(SQLITE_GENERAL_NS_NATIVE_ALL):
            type = CONFLICT_SINGLE_VERSION_NS_NATIVE_ALL;
            return E_OK;
        default:
            return -E_NOT_SUPPORT;
    }
}

int SQLiteSingleVerNaturalStore::GetSchema(SchemaObject &schema) const
{
    int errCode = E_OK;
    auto handle = GetHandle(true, errCode); // Only open kvdb use, no competition for write handle
    if (handle == nullptr) {
        return errCode;
    }

    TimeStamp timeStamp;
    std::string schemaKey = DBConstant::SCHEMA_KEY;
    Key key(schemaKey.begin(), schemaKey.end());
    Value value;
    errCode = handle->GetKvData(SingleVerDataType::META_TYPE, key, value, timeStamp);
    if (errCode == E_OK) {
        std::string schemaValue(value.begin(), value.end());
        errCode = schema.ParseFromSchemaString(schemaValue);
    } else {
        LOGI("[SqlSinStore][GetSchema] Get schema from db failed or no schema=%d.", errCode);
    }
    ReleaseHandle(handle);
    return errCode;
}

int SQLiteSingleVerNaturalStore::DecideReadOnlyBaseOnSchema(const KvDBProperties &kvDBProp, bool &isReadOnly,
    SchemaObject &savedSchemaObj) const
{
    // Check whether it is a memory db
    if (kvDBProp.GetBoolProp(KvDBProperties::MEMORY_MODE, false)) {
        isReadOnly = false;
        return E_OK;
    }
    SchemaObject inputSchemaObj = kvDBProp.GetSchema();
    if (!inputSchemaObj.IsSchemaValid()) {
        int errCode = GetSchema(savedSchemaObj);
        if (errCode != E_OK && errCode != -E_NOT_FOUND) {
            LOGE("[SqlSinStore][DecideReadOnly] GetSchema fail=%d.", errCode);
            return errCode;
        }
        if (savedSchemaObj.IsSchemaValid()) {
            isReadOnly = true;
            return E_OK;
        }
    }
    // An valid schema will not lead to readonly
    isReadOnly = false;
    return E_OK;
}

void SQLiteSingleVerNaturalStore::InitialLocalDataTimestamp()
{
    TimeStamp timeStamp = GetCurrentTimeStamp();

    int errCode = E_OK;
    auto handle = GetHandle(true, errCode);
    if (handle == nullptr) {
        return;
    }

    errCode = handle->UpdateLocalDataTimestamp(timeStamp);
    if (errCode != E_OK) {
        LOGE("Update the timestamp for local data failed:%d", errCode);
    }
    ReleaseHandle(handle);
}

const KvDBProperties &SQLiteSingleVerNaturalStore::GetDbProperties() const
{
    return GetMyProperties();
}

int SQLiteSingleVerNaturalStore::RemoveKvDB(const KvDBProperties &properties)
{
    // To avoid leakage, the engine resources are forced to be released
    const std::string identifier = properties.GetStringProp(KvDBProperties::IDENTIFIER_DATA, "");
    (void)StorageEngineManager::ForceReleaseStorageEngine(identifier);

    // Only care the data directory and the db name.
    std::string storeOnlyDir;
    std::string storeDir;
    GenericKvDB::GetStoreDirectory(properties, KvDBProperties::SINGLE_VER_TYPE, storeDir, storeOnlyDir);

    std::string currentDir = storeDir + DBConstant::MAINDB_DIR + "/";
    std::string currentOnlyDir = storeOnlyDir + DBConstant::MAINDB_DIR + "/";
    int removeMainErrCode = KvDBUtils::RemoveKvDB(currentDir, currentOnlyDir, DBConstant::SINGLE_VER_DATA_STORE);
    if (removeMainErrCode != -E_NOT_FOUND && removeMainErrCode != E_OK) {
        return removeMainErrCode;
    }
    currentDir = storeDir + DBConstant::METADB_DIR + "/";
    currentOnlyDir = storeOnlyDir + DBConstant::METADB_DIR + "/";
    int removeMetaErrCode = KvDBUtils::RemoveKvDB(currentDir, currentOnlyDir, DBConstant::SINGLE_VER_META_STORE);
    if (removeMetaErrCode != -E_NOT_FOUND && removeMetaErrCode != E_OK) {
        return removeMetaErrCode;
    }
    currentDir = storeDir + DBConstant::CACHEDB_DIR + "/";
    currentOnlyDir = storeOnlyDir + DBConstant::CACHEDB_DIR + "/";
    int removeCacheErrCode = KvDBUtils::RemoveKvDB(currentDir, currentOnlyDir, DBConstant::SINGLE_VER_CACHE_STORE);
    if (removeCacheErrCode != -E_NOT_FOUND && removeCacheErrCode != E_OK) {
        return removeCacheErrCode;
    }

    // Signed numbers can not use bit operations
    if (removeMainErrCode == -E_NOT_FOUND && removeMetaErrCode == -E_NOT_FOUND && removeCacheErrCode == -E_NOT_FOUND) {
        return -E_NOT_FOUND;
    }

    int errCode = DBCommon::RemoveAllFilesOfDirectory(storeDir, true);
    if (errCode != E_OK) {
        return errCode;
    }
    errCode = DBCommon::RemoveAllFilesOfDirectory(storeOnlyDir, true);
    if (errCode != E_OK) {
        return errCode;
    }
    return errCode;
}

int SQLiteSingleVerNaturalStore::GetKvDBSize(const KvDBProperties &properties, uint64_t &size) const
{
    std::string storeOnlyIdentDir;
    std::string storeIdentDir;
    GenericKvDB::GetStoreDirectory(properties, KvDBProperties::SINGLE_VER_TYPE, storeIdentDir, storeOnlyIdentDir);
    std::vector<std::pair<std::string, std::string>> dbDir {{DBConstant::MAINDB_DIR, DBConstant::SINGLE_VER_DATA_STORE},
        {DBConstant::METADB_DIR, DBConstant::SINGLE_VER_META_STORE},
        {DBConstant::CACHEDB_DIR, DBConstant::SINGLE_VER_CACHE_STORE}};
    int errCode = -E_NOT_FOUND;
    for (const auto &item : dbDir) {
        std::string storeDir = storeIdentDir + item.first;
        std::string storeOnlyDir = storeOnlyIdentDir + item.first;
        int err = KvDBUtils::GetKvDbSize(storeDir, storeOnlyDir, item.second, size);
        if (err != -E_NOT_FOUND && err != E_OK) {
            return err;
        }
        if (err == E_OK) {
            errCode = E_OK;
        }
    }
    return errCode;
}

KvDBProperties &SQLiteSingleVerNaturalStore::GetDbPropertyForUpdate()
{
    return MyProp();
}

void SQLiteSingleVerNaturalStore::HeartBeatForLifeCycle() const
{
    std::lock_guard<std::mutex> lock(lifeCycleMutex_);
    int errCode = ResetLifeCycleTimer();
    if (errCode != E_OK) {
        LOGE("Heart beat for life cycle failed:%d", errCode);
    }
}

int SQLiteSingleVerNaturalStore::StartLifeCycleTimer(const DatabaseLifeCycleNotifier &notifier) const
{
    auto runtimeCxt = RuntimeContext::GetInstance();
    if (runtimeCxt == nullptr) {
        return -E_INVALID_ARGS;
    }
    RefObject::IncObjRef(this);
    TimerId timerId = 0;
    int errCode = runtimeCxt->SetTimer(autoLifeTime_,
        [this](TimerId id) -> int {
            std::lock_guard<std::mutex> lock(lifeCycleMutex_);
            if (lifeCycleNotifier_) {
                auto identifier = GetMyProperties().GetStringProp(KvDBProperties::IDENTIFIER_DATA, "");
                lifeCycleNotifier_(identifier);
            }
            return 0;
        },
        [this]() {
            int errCode = RuntimeContext::GetInstance()->ScheduleTask([this]() {
                RefObject::DecObjRef(this);
            });
            if (errCode != E_OK) {
                LOGE("SQLiteSingleVerNaturalStore timer finalizer ScheduleTask, errCode %d", errCode);
            }
        },
        timerId);
    if (errCode != E_OK) {
        lifeTimerId_ = 0;
        LOGE("SetTimer failed:%d", errCode);
        RefObject::DecObjRef(this);
        return errCode;
    }

    lifeCycleNotifier_ = notifier;
    lifeTimerId_ = timerId;
    return E_OK;
}

int SQLiteSingleVerNaturalStore::ResetLifeCycleTimer() const
{
    if (lifeTimerId_ == 0) {
        return E_OK;
    }
    auto lifeNotifier = lifeCycleNotifier_;
    lifeCycleNotifier_ = nullptr;
    int errCode = StopLifeCycleTimer();
    if (errCode != E_OK) {
        LOGE("[Reset timer]Stop the life cycle timer failed:%d", errCode);
    }
    return StartLifeCycleTimer(lifeNotifier);
}

int SQLiteSingleVerNaturalStore::StopLifeCycleTimer() const
{
    auto runtimeCxt = RuntimeContext::GetInstance();
    if (runtimeCxt == nullptr) {
        return -E_INVALID_ARGS;
    }
    if (lifeTimerId_ != 0) {
        TimerId timerId = lifeTimerId_;
        lifeTimerId_ = 0;
        runtimeCxt->RemoveTimer(timerId, false);
    }
    return E_OK;
}

bool SQLiteSingleVerNaturalStore::IsDataMigrating() const
{
    if (storageEngine_ != nullptr) {
        EngineState state = storageEngine_->GetEngineState();
        if (state == EngineState::MIGRATING || state == EngineState::ENGINE_BUSY) {
            return true;
        }
    }
    return false;
}

void SQLiteSingleVerNaturalStore::SetConnectionFlag(bool isExisted) const
{
    if (storageEngine_ != nullptr) {
        storageEngine_->SetConnectionFlag(isExisted);
    }
}

int SQLiteSingleVerNaturalStore::TriggerToMigrateData() const
{
    RefObject::IncObjRef(this);
    int errCode = RuntimeContext::GetInstance()->ScheduleTask(
        std::bind(&SQLiteSingleVerNaturalStore::AsyncDataMigration, this));
    if (errCode != E_OK) {
        RefObject::DecObjRef(this);
        LOGE("[SingleVerNStore] Trigger to migrate data failed : %d.", errCode);
    }
    return errCode;
}

bool SQLiteSingleVerNaturalStore::IsCacheDBMode() const
{
    if (storageEngine_ == nullptr) {
        LOGE("[SingleVerNStore] IsCacheDBMode storage engine is invalid.");
        return false;
    }
    EngineState engineState = storageEngine_->GetEngineState();
    return (engineState == CACHEDB);
}

bool SQLiteSingleVerNaturalStore::IsExtendedCacheDBMode() const
{
    if (storageEngine_ == nullptr) {
        LOGE("[SingleVerNStore] storage engine is invalid.");
        return false;
    }
    EngineState engineState = storageEngine_->GetEngineState();
    return (engineState == CACHEDB || engineState == MIGRATING || engineState == ATTACHING);
}

int SQLiteSingleVerNaturalStore::CheckReadDataControlled() const
{
    if (IsExtendedCacheDBMode()) {
        int err = IsCacheDBMode() ? -E_EKEYREVOKED : -E_BUSY;
        LOGE("Existed cache database can not read data, errCode = [%d]!", err);
        return err;
    }
    return E_OK;
}

void SQLiteSingleVerNaturalStore::IncreaseCacheRecordVersion() const
{
    if (storageEngine_ == nullptr) {
        LOGE("[SingleVerNStore] Increase cache version storage engine is invalid.");
        return;
    }
    storageEngine_->IncreaseCacheRecordVersion();
}

uint64_t SQLiteSingleVerNaturalStore::GetCacheRecordVersion() const
{
    if (storageEngine_ == nullptr) {
        LOGE("[SingleVerNStore] Get cache version storage engine is invalid.");
        return 0;
    }
    return storageEngine_->GetCacheRecordVersion();
}

uint64_t SQLiteSingleVerNaturalStore::GetAndIncreaseCacheRecordVersion() const
{
    if (storageEngine_ == nullptr) {
        LOGE("[SingleVerNStore] Get cache version storage engine is invalid.");
        return 0;
    }
    return storageEngine_->GetAndIncreaseCacheRecordVersion();
}

void SQLiteSingleVerNaturalStore::AsyncDataMigration() const
{
    // Delay a little time to ensure the completion of the delegate callback
    std::this_thread::sleep_for(std::chrono::milliseconds(WAIT_DELEGATE_CALLBACK_TIME));
    bool isLocked = RuntimeContext::GetInstance()->IsAccessControlled();
    if (!isLocked) {
        LOGI("Begin to migrate cache data to manDb asynchronously!");
        (void)StorageEngineManager::ExecuteMigration(storageEngine_);
    }

    RefObject::DecObjRef(this);
}

void SQLiteSingleVerNaturalStore::CheckAmendValueContentForSyncProcedure(std::vector<DataItem> &dataItems) const
{
    const SchemaObject &schemaObjRef = MyProp().GetSchemaConstRef();
    if (!schemaObjRef.IsSchemaValid()) {
        // Not a schema database, do not need to check more
        return;
    }
    uint32_t deleteCount = 0;
    uint32_t amendCount = 0;
    uint32_t neglectCount = 0;
    for (auto &eachItem : dataItems) {
        if ((eachItem.flag & DataItem::DELETE_FLAG) != 0) {
            // Delete record not concerned
            deleteCount++;
            continue;
        }
        bool useAmendValue = false;
        int errCode = CheckValueAndAmendIfNeed(ValueSource::FROM_SYNC, eachItem.value, eachItem.value, useAmendValue);
        if (errCode != E_OK) {
            eachItem.neglect = true;
            neglectCount++;
            continue;
        }
        if (useAmendValue) {
            amendCount++;
        }
    }
    LOGI("[SqlSinStore][CheckAmendForSync] OriCount=%zu, DeleteCount=%u, AmendCount=%u, NeglectCount=%u",
        dataItems.size(), deleteCount, amendCount, neglectCount);
}

int SQLiteSingleVerNaturalStore::SaveSyncItemsInCacheMode(SQLiteSingleVerStorageExecutor *handle,
    std::vector<DataItem> &dataItems, const DeviceInfo &deviceInfo, TimeStamp &maxTimestamp) const
{
    int errCode = handle->StartTransaction(TransactType::IMMEDIATE);
    if (errCode != E_OK) {
        return errCode;
    }

    int innerCode;
    const uint64_t recordVersion = GetCacheRecordVersion();
    errCode = handle->PrepareForSavingCacheData(SingleVerDataType::SYNC_TYPE);
    if (errCode != E_OK) {
        goto END;
    }

    for (auto &item : dataItems) {
        errCode = handle->SaveSyncDataItemInCacheMode(item, deviceInfo, maxTimestamp, recordVersion);
        if (errCode != E_OK && errCode != -E_NOT_FOUND) {
            break;
        }
    }

    if (errCode == -E_NOT_FOUND) {
        errCode = E_OK;
    }

    innerCode = handle->ResetForSavingCacheData(SingleVerDataType::SYNC_TYPE);
    if (innerCode != E_OK) {
        errCode = innerCode;
    }
END:
    if (errCode == E_OK) {
        errCode = handle->Commit();
        storageEngine_->IncreaseCacheRecordVersion();
    } else {
        (void)handle->Rollback(); // Keep the error code of the first scene
    }
    return errCode;
}

void SQLiteSingleVerNaturalStore::NotifyRemotePushFinished(const std::string &targetId) const
{
    std::string identifier = DBCommon::VectorToHexString(GetIdentifier());
    LOGI("label:%s sourceTarget: %s{private} push finished", identifier.c_str(), targetId.c_str());
    NotifyRemotePushFinishedInner(targetId);
}

DEFINE_OBJECT_TAG_FACILITIES(SQLiteSingleVerNaturalStore)
}
