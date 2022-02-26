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

#ifndef SQLITE_STORAGE_EXECUTOR_H
#define SQLITE_STORAGE_EXECUTOR_H

#include "sqlite_import.h"
#include "macro_utils.h"
#include "storage_executor.h"

namespace DistributedDB {
class SQLiteStorageExecutor : public StorageExecutor {
public:
    SQLiteStorageExecutor(sqlite3 *dbHandle, bool writable, bool isMemDb);
    ~SQLiteStorageExecutor() override;

    // Delete the copy and assign constructors
    DISABLE_COPY_ASSIGN_MOVE(SQLiteStorageExecutor);

    int Reset() override;
    int GetDbHandle(sqlite3 *&dbHandle) const;

protected:
    sqlite3 *dbHandle_;
    bool isMemDb_;
};
} // namespace DistributedDB

#endif // SQLITE_STORAGE_EXECUTOR_H
