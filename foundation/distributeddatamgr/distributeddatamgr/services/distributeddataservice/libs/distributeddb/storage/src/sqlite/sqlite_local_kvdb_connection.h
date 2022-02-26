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

#ifndef SQLITE_LOCAL_KV_DB_CONNECTION_H
#define SQLITE_LOCAL_KV_DB_CONNECTION_H

#include <set>
#include <mutex>

#include "db_errno.h"
#include "kvdb_properties.h"
#include "generic_kvdb_connection.h"
#include "sqlite_local_storage_executor.h"

namespace DistributedDB {
class SQLiteLocalKvDB;

class SQLiteLocalKvDBConnection : public GenericKvDBConnection {
public:
    explicit SQLiteLocalKvDBConnection(SQLiteLocalKvDB *kvDB);
    ~SQLiteLocalKvDBConnection() override;

    // Delete the copy and assign constructors
    DISABLE_COPY_ASSIGN_MOVE(SQLiteLocalKvDBConnection);

    // Get the value from the sqlite database
    int Get(const IOption &option, const Key &key, Value &value) const override;

    // Put the value to the sqlite database
    int Put(const IOption &option, const Key &key, const Value &value) override;

    // Delete the value from the sqlite database
    int Delete(const IOption &option, const Key &key) override;

    // Clear all the data from the sqlite database
    int Clear(const IOption &option) override;

    // Get all the data which have the prefix key from the sqlite database
    int GetEntries(const IOption &option, const Key &keyPrefix, std::vector<Entry> &entries) const override;

    // Put the batch data to the sqlite database
    int PutBatch(const IOption &option, const std::vector<Entry> &entries) override;

    // Delete the batch data from the sqlite database according to the key from the set
    int DeleteBatch(const IOption &option, const std::vector<Key> &keys) override;

    // Get the snapshot
    int GetSnapshot(IKvDBSnapshot *&snapshot) const override;

    // Release the snapshot
    void ReleaseSnapshot(IKvDBSnapshot *&snapshot) override;

    // Next step interface
    // Start the transaction
    int StartTransaction() override;

    // Commit the transaction
    int Commit() override;

    // Roll back the transaction
    int RollBack() override;

    // Check if the transaction already started manually
    bool IsTransactionStarted() const override;

    // Called when close the connection
    int PreClose() override;

    // Parse event types(from observer mode).
    int TranslateObserverModeToEventTypes(unsigned mode, std::list<int> &eventTypes) const override;

    int Rekey(const CipherPassword &passwd) override;

    int Export(const std::string &filePath, const CipherPassword &passwd) override;

    int Import(const std::string &filePath, const CipherPassword &passwd) override;

private:
    // Start the transaction
    int StartTransactionInner(bool &isAuto);

    // Commit the transaction
    int CommitInner();

    // Roll back the transaction
    int RollBackInner();

    int CheckDataStatus(const Key &key, const Value &value, bool isDeleted) const;

    SQLiteLocalStorageExecutor *writeHandle_; // only existed while in transaction.

    mutable std::mutex transactionMutex_;
    mutable std::set<IKvDBSnapshot *> snapshots_;
    mutable std::mutex snapshotMutex_;
    std::mutex importMutex_;
};
};  // namespace DistributedDB

#endif // SQLITE_LOCAL_KV_DB_CONNECTION_H
