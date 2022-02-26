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

#ifndef SQLITE_KVDB_HANDLE_POOL_H
#define SQLITE_KVDB_HANDLE_POOL_H

#include <vector>

#include "macro_utils.h"
#include "storage_engine.h"
#include "sqlite_utils.h"

namespace DistributedDB {
class SQLiteStorageEngine : public StorageEngine {
public:
    SQLiteStorageEngine();
    ~SQLiteStorageEngine() override;

    // Delete the copy and assign constructors
    DISABLE_COPY_ASSIGN_MOVE(SQLiteStorageEngine);

    int InitSQLiteStorageEngine(const StorageEngineAttr &poolSize, const OpenDbProperties &option,
        const std::string &identifier = std::string());

    bool IsNeedTobeReleased() const override;

    const std::string &GetIdentifier() const override;

    EngineState GetEngineState() const override;

    int ExecuteMigrate() override;

    void SetEngineState(EngineState state) override;

    const OpenDbProperties &GetOpenOption() const;

    virtual void IncreaseCacheRecordVersion();
    virtual uint64_t GetCacheRecordVersion() const;
    virtual uint64_t GetAndIncreaseCacheRecordVersion();

    virtual bool IsEngineCorrupted() const;

    void ClearEnginePasswd() override;

    int CheckEngineOption(const KvDBProperties &kvdbOption) const override;

protected:

    virtual int Upgrade(sqlite3 *db);

    virtual StorageExecutor *NewSQLiteStorageExecutor(sqlite3 *dbHandle, bool isWrite, bool isMemDb) = 0;

    int CreateNewExecutor(bool isWrite, StorageExecutor *&handle) override;

    virtual int ReInit();

    bool IsNeedMigrate() const override;

    OpenDbProperties option_;
};
} // namespace DistributedDB
#endif // SQLITE_DB_HANDLE_H
