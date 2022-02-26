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

#include "sqlite_storage_executor.h"

#include "db_errno.h"
#include "log_print.h"
#include "sqlite_utils.h"

namespace DistributedDB {
SQLiteStorageExecutor::SQLiteStorageExecutor(sqlite3 *dbHandle, bool writable, bool isMemDb)
    : StorageExecutor(writable),
      dbHandle_(dbHandle),
      isMemDb_(isMemDb)
{}

SQLiteStorageExecutor::~SQLiteStorageExecutor()
{
    if (dbHandle_ != nullptr) {
        (void)sqlite3_close_v2(dbHandle_);
        dbHandle_ = nullptr;
    }
}

int SQLiteStorageExecutor::Reset()
{
    if (dbHandle_ != nullptr) {
        // Set the handle to be valid, release the heap memory as much as possible.
        sqlite3_db_release_memory(dbHandle_);
        return E_OK;
    }
    return -E_INVALID_DB;
}

int SQLiteStorageExecutor::GetDbHandle(sqlite3 *&dbHandle) const
{
    if (dbHandle_ == nullptr) {
        LOGE("Can not get dbhandle from executor!");
        return -E_NOT_FOUND;
    }
    dbHandle = dbHandle_;
    return E_OK;
}
} // namespace DistributedDB
