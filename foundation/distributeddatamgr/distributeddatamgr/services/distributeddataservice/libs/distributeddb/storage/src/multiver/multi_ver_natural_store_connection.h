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

#ifndef MULTI_VER_NATURAL_STORE_CONNECTION_H
#define MULTI_VER_NATURAL_STORE_CONNECTION_H

#ifndef OMIT_MULTI_VER
#include <mutex>
#include <set>

#include "macro_utils.h"
#include "sync_able_kvdb_connection.h"
#include "multi_ver_def.h"
#include "multi_ver_kv_entry.h"
#include "multi_ver_storage_executor.h"

namespace DistributedDB {
class MultiVerNaturalStore;

enum class TransactState {
    TRANSACT_IDLE,
    TRANSACT_IN_PROGRESS,
};

class MultiVerNaturalStoreConnection : public SyncAbleKvDBConnection {
public:
    explicit MultiVerNaturalStoreConnection(MultiVerNaturalStore *kvDB);
    ~MultiVerNaturalStoreConnection() override;

    // Delete the copy and assign constructors
    DISABLE_COPY_ASSIGN_MOVE(MultiVerNaturalStoreConnection);

    // Get the value from the database
    int Get(const IOption &option, const Key &key, Value &value) const override;

    // Put the value to the database
    int Put(const IOption &option, const Key &key, const Value &value) override;

    // Delete the value from the database
    int Delete(const IOption &option, const Key &key) override;

    // Clear all the data from the database
    int Clear(const IOption &option) override;

    // Get all the data from the database
    int GetEntries(const IOption &option, const Key &keyPrefix, std::vector<Entry> &entries) const override;

    // Put the batch values to the database.
    int PutBatch(const IOption &option, const std::vector<Entry> &entries) override;

    // Put the synced data by commit.
    int PutCommitData(const MultiVerCommitNode &commit, const std::vector<MultiVerKvEntry *> &entries);

    // Delete the batch values from the database.
    int DeleteBatch(const IOption &option, const std::vector<Key> &keys) override;

    // Get the snapshot
    int GetSnapshot(IKvDBSnapshot *&snapshot) const override;

    // Release the created snapshot
    void ReleaseSnapshot(IKvDBSnapshot *&snapshot) override;

    // Start the transaction
    int StartTransaction() override;

    // Commit the transaction
    int Commit() override;

    // Roll back the transaction
    int RollBack() override;

    // Check if the transaction already started manually
    bool IsTransactionStarted() const override;

    // Called when close and delete the connection.
    int PreClose() override;

    // Parse event types(from observer mode).
    int TranslateObserverModeToEventTypes(unsigned mode, std::list<int> &eventTypes) const override;

    int Rekey(const CipherPassword &passwd) override;

    int Export(const std::string &filePath, const CipherPassword &passwd) override;

    int Import(const std::string &filePath, const CipherPassword &passwd) override;

private:
    static bool CheckDeletedKeys(const std::vector<Key> &keys);

    int CheckDataStatus(const Key &key, const Value &value, bool isDeleted) const;

    int StartTransactionInner(bool &isAuto);

    int CommitTransactionInner();

    int RollBackTransactionInner();

    int CheckTransactionState();

    MultiVerStorageExecutor *GetHandle(bool isWrite, int &errCode) const;

    DECLARE_OBJECT_TAG(MultiVerNaturalStoreConnection);

    MultiVerStorageExecutor *writeHandle_;
    mutable std::set<IKvDBSnapshot *> snapshots_;
    mutable std::mutex snapshotMutex_;
    mutable std::mutex writeMutex_;
    std::mutex rekeyMutex_;
    std::mutex importMutex_;
};
} // namespace DistributedDB

#endif  // MULTI_VER_NATURAL_STORE_CONNECTION_H
#endif