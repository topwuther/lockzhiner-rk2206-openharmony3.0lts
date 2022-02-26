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

#ifndef SQLITE_LOCAL_STORAGE_ENGINE_H
#define SQLITE_LOCAL_STORAGE_ENGINE_H

#include "macro_utils.h"
#include "sqlite_storage_engine.h"

namespace DistributedDB {
class SQLiteLocalStorageEngine : public SQLiteStorageEngine {
public:
    SQLiteLocalStorageEngine();
    ~SQLiteLocalStorageEngine() override;

    // Delete the copy and assign constructors
    DISABLE_COPY_ASSIGN_MOVE(SQLiteLocalStorageEngine);

protected:
    StorageExecutor *NewSQLiteStorageExecutor(sqlite3 *dbHandle, bool isWrite, bool isMemDb) override;
};
} // namespace DistributedDB

#endif // SQLITE_DB_HANDLE_H
