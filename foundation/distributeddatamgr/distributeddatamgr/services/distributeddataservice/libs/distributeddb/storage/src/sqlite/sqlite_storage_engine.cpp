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

#include "sqlite_storage_engine.h"

#include "db_errno.h"
#include "log_print.h"
#include "sqlite_storage_executor.h"
#include "sqlite_utils.h"
#include "runtime_context.h"

namespace DistributedDB {
SQLiteStorageEngine::SQLiteStorageEngine()
{}

SQLiteStorageEngine::~SQLiteStorageEngine()
{}

int SQLiteStorageEngine::InitSQLiteStorageEngine(const StorageEngineAttr &poolSize, const OpenDbProperties &option,
    const std::string &identifier)
{
    if (StorageEngine::CheckEngineAttr(poolSize)) {
        LOGE("Invalid storage engine attributes!");
        return -E_INVALID_ARGS;
    }
    engineAttr_ = poolSize;
    option_ = option;
    identifier_ = identifier;
    if (GetEngineState() == EngineState::CACHEDB) {
        LOGI("Is alive! not need to create executor, only fix option.");
        return E_OK;
    }
    int errCode = Init();
    if (errCode != E_OK) {
        LOGI("Storage engine init fail! errCode = [%d]", errCode);
    }
    return errCode;
}

StorageExecutor *SQLiteStorageEngine::NewSQLiteStorageExecutor(sqlite3 *dbHandle, bool isWrite, bool isMemDb)
{
    return new (std::nothrow) SQLiteStorageExecutor(dbHandle, isWrite, isMemDb);
}

int SQLiteStorageEngine::Upgrade(sqlite3 *db)
{
    // SQLiteSingleVerStorageEngine override this function to do table structure and even content upgrade
    // SQLiteLocalStorageEngine is used by SQLiteLocalKvDB, and SQLiteLocalKvDB is used as LocalStore, CommitStorage,
    //      ValueSliceStorage, MetaDataStorage, all of them will not change the table structure,
    //      so the version of these dbFile only represent for the content version.
    // SQLiteLocalStorageEngine do not override this function so content upgrade can only be done by the Storage
    //      who use the SQLiteLocalKvDB.
    // MultiVerStorageEngine do not inherit SQLiteStorageEngine.
    return E_OK;
}

int SQLiteStorageEngine::CreateNewExecutor(bool isWrite, StorageExecutor *&handle)
{
    sqlite3 *dbHandle = nullptr;
    int errCode = SQLiteUtils::OpenDatabase(option_, dbHandle);
    if (errCode != E_OK) {
        return errCode;
    }
    bool isMemDb = option_.isMemDb;
    if (!isUpdated_) {
        errCode = Upgrade(dbHandle);
        if (errCode != E_OK) {
            (void)sqlite3_close_v2(dbHandle);
            dbHandle = nullptr;
            return errCode;
        }
        isUpdated_ = true;
    }

    handle = NewSQLiteStorageExecutor(dbHandle, isWrite, isMemDb);
    if (handle == nullptr) {
        LOGE("New SQLiteStorageExecutor[%d] for the pool failed.", isWrite);
        (void)sqlite3_close_v2(dbHandle);
        dbHandle = nullptr;
        return -E_OUT_OF_MEMORY;
    }
    return E_OK;
}

int SQLiteStorageEngine::ReInit()
{
    return E_OK;
}

bool SQLiteStorageEngine::IsNeedTobeReleased() const
{
    EngineState engineState = GetEngineState();
    return ((engineState == MAINDB) || (engineState == INVALID));
}

const std::string &SQLiteStorageEngine::GetIdentifier() const
{
    return identifier_;
}

EngineState SQLiteStorageEngine::GetEngineState() const
{
    return engineState_;
}

void SQLiteStorageEngine::SetEngineState(EngineState state)
{
    LOGD("[SQLiteStorageEngine::SetEngineState] Engine State : [%d]", state);
    engineState_ = state; // Current usage logically can guarante no concurrency
}

const OpenDbProperties &SQLiteStorageEngine::GetOpenOption() const
{
    return option_;
}

bool SQLiteStorageEngine::IsNeedMigrate() const
{
    return false;
}

int SQLiteStorageEngine::ExecuteMigrate()
{
    return -E_NOT_SUPPORT;
}

void SQLiteStorageEngine::IncreaseCacheRecordVersion()
{
    return;
}

uint64_t SQLiteStorageEngine::GetCacheRecordVersion() const
{
    return 0;
}

uint64_t SQLiteStorageEngine::GetAndIncreaseCacheRecordVersion()
{
    return 0;
}

bool SQLiteStorageEngine::IsEngineCorrupted() const
{
    return false;
}

void SQLiteStorageEngine::ClearEnginePasswd()
{
    option_.passwd.Clear();
}

int SQLiteStorageEngine::CheckEngineOption(const KvDBProperties &kvDBProp) const
{
    SecurityOption securityOpt;
    if (RuntimeContext::GetInstance()->IsProcessSystemApiAdapterValid()) {
        securityOpt.securityLabel = kvDBProp.GetSecLabel();
        securityOpt.securityFlag = kvDBProp.GetSecFlag();
    }
    std::string schemaStr = kvDBProp.GetSchema().ToSchemaString();
    int conflictReslovePolicy = kvDBProp.GetIntProp(KvDBProperties::CONFLICT_RESOLVE_POLICY, DEFAULT_LAST_WIN);
    bool createDirByStoreIdOnly = kvDBProp.GetBoolProp(KvDBProperties::CREATE_DIR_BY_STORE_ID_ONLY, false);

    if (kvDBProp.GetSchemaConstRef().IsSchemaValid() == option_.schema.empty()) {
        // If both true, newSchema not empty, oriEngineSchema empty, not same
        // If both false, newSchema empty, oriEngineSchema not empty, not same
        LOGE("Engine and kvdb schema only one not empty! kvdb schema is [%d]", option_.schema.empty());
        return -E_SCHEMA_MISMATCH;
    }
    // Here both schema empty or not empty, be the same
    if (kvDBProp.GetSchemaConstRef().IsSchemaValid()) {
        int errCode = kvDBProp.GetSchemaConstRef().CompareAgainstSchemaString(option_.schema);
        if (errCode != -E_SCHEMA_EQUAL_EXACTLY) {
            LOGE("Engine and kvdb schema mismatch!");
            return -E_SCHEMA_MISMATCH;
        }
    }

    bool isMemDb = kvDBProp.GetBoolProp(KvDBProperties::MEMORY_MODE, false);
    // Here both schema empty or "not empty and equal exactly", this is ok as required
    if (isMemDb == false &&
        option_.createDirByStoreIdOnly == createDirByStoreIdOnly &&
        option_.securityOpt == securityOpt &&
        option_.conflictReslovePolicy == conflictReslovePolicy) {
        return E_OK;
    }
    return -E_INVALID_ARGS;
}
}
