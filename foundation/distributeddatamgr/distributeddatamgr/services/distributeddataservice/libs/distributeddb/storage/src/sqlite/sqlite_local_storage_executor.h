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

#ifndef SQLITE_LOCAL_DB_HANDLE_H
#define SQLITE_LOCAL_DB_HANDLE_H

#include "sqlite_import.h"
#include "macro_utils.h"
#include "db_types.h"
#include "sqlite_storage_executor.h"

namespace DistributedDB {
class SQLiteLocalStorageExecutor : public SQLiteStorageExecutor {
public:
    SQLiteLocalStorageExecutor(sqlite3 *dbHandle, bool writable, bool isMemDb);
    ~SQLiteLocalStorageExecutor() override;

    // Delete the copy and assign constructors
    DISABLE_COPY_ASSIGN_MOVE(SQLiteLocalStorageExecutor);

    int Get(const Key &key, Value &value) const;

    // Put the value to the sqlite database
    int Put(const Key &key, const Value &value);

    // Delete the value from the sqlite database
    int Delete(const Key &key);

    // Clear all the data from the sqlite database
    int Clear();

    // Get all the data which have the prefix key from the sqlite database
    int GetEntries(const Key &keyPrefix, std::vector<Entry> &entries) const;

    // Put the batch data to the sqlite database
    int PutBatch(const std::vector<Entry> &entries);

    // Delete the batch data from the sqlite database according to the key from the set
    int DeleteBatch(const std::vector<Key> &keys);

    // Next step interface
    // Start the transaction
    int StartTransaction();

    // Commit the transaction
    int Commit();

    // Roll back the transaction
    int RollBack();

private:
    // Put the value to the sqlite database, used by Put &PutBach
    int PutInner(const Key &key, const Value &value);

    // Delete the value from the sqlite database, used by Delete &DeleteBach
    int DeleteInner(const Key &key);

    // Start the transaction
    int StartTransactionInner(bool &isAuto);

    // Commit the transaction
    int CommitInner();

    // Roll back the transaction
    int RollBackInner();
};
} // namespace DistributedDB

#endif // SQLITE_DB_HANDLE_H
