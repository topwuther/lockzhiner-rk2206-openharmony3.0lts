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

#ifndef SQLITE_SINGLE_VER_NATURAL_STORE_CONNECTION_H
#define SQLITE_SINGLE_VER_NATURAL_STORE_CONNECTION_H

#include "sync_able_kvdb_connection.h"
#include "sqlite_single_ver_storage_executor.h"
#include "db_types.h"
#include "runtime_context.h"

namespace DistributedDB {
class SQLiteSingleVerNaturalStore;

class SQLiteSingleVerNaturalStoreConnection : public SyncAbleKvDBConnection {
public:
    explicit SQLiteSingleVerNaturalStoreConnection(SQLiteSingleVerNaturalStore *kvDB);
    ~SQLiteSingleVerNaturalStoreConnection() override;

    // Delete the copy and assign constructors
    DISABLE_COPY_ASSIGN_MOVE(SQLiteSingleVerNaturalStoreConnection);

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

    int GetEntries(const IOption &option, const Query &query, std::vector<Entry> &entries) const override;

    int GetCount(const IOption &option, const Query &query, int &count) const override;
    // Put the batch values to the database.
    int PutBatch(const IOption &option, const std::vector<Entry> &entries) override;

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

    // Pragma interface.
    int Pragma(int cmd, void *parameter) override;

    // Parse event types(from observer mode).
    int TranslateObserverModeToEventTypes(unsigned mode, std::list<int> &eventTypes) const override;

    // Register a conflict notifier.
    int SetConflictNotifier(int conflictType, const KvDBConflictAction &action) override;

    int Rekey(const CipherPassword &passwd) override;

    int Export(const std::string &filePath, const CipherPassword &passwd) override;

    int Import(const std::string &filePath, const CipherPassword &passwd) override;

    // Get the result set
    int GetResultSet(const IOption &option, const Key &keyPrefix, IKvDBResultSet *&resultSet) const override;

    int GetResultSet(const IOption &option, const Query &query, IKvDBResultSet *&resultSet) const override;

    // Release the result set
    void ReleaseResultSet(IKvDBResultSet *&resultSet) override;

    int RegisterLifeCycleCallback(const DatabaseLifeCycleNotifier &notifier) override;

    // Called when Close and delete the connection.
    int PreClose() override;

private:
    int CheckMonoStatus(OperatePerm perm);

    int GetDeviceIdentifier(PragmaEntryDeviceIdentifier *identifier);

    void ClearConflictNotifierCount();

    int PutBatchInner(const IOption &option, const std::vector<Entry> &entries);
    int DeleteBatchInner(const IOption &option, const std::vector<Key> &keys);

    int SaveSyncEntries(const std::vector<Entry> &entries);
    int SaveLocalEntries(const std::vector<Entry> &entries);
    int DeleteSyncEntries(const std::vector<Key> &keys);
    int DeleteLocalEntries(const std::vector<Key> &keys);

    int SaveEntry(const Entry &entry, bool isDelete, TimeStamp timeStamp = 0);

    int CheckDataStatus(const Key &key, const Value &value, bool isDelete) const;

    int CheckWritePermission() const;

    int CheckSyncEntriesValid(const std::vector<Entry> &entries) const;

    int CheckSyncKeysValid(const std::vector<Key> &keys) const;

    int CheckLocalEntriesValid(const std::vector<Entry> &entries) const;

    int CheckLocalKeysValid(const std::vector<Key> &keys) const;

    void CommitAndReleaseNotifyData(SingleVerNaturalStoreCommitNotifyData *&committedData,
        bool isNeedCommit, int eventType);

    int StartTransactionInner();

    int CommitInner();

    int RollbackInner();

    int PublishLocal(const Key &key, bool deleteLocal, bool updateTimestamp,
        const KvStoreNbPublishAction &onConflict);

    int PublishLocalCallback(bool updateTimestamp, const SingleVerRecord &localRecord,
        const SingleVerRecord &syncRecord, const KvStoreNbPublishAction &onConflict);

    int PublishInner(SingleVerNaturalStoreCommitNotifyData *committedData, bool updateTimestamp,
        SingleVerRecord &localRecord, SingleVerRecord &syncRecord, bool &isNeedCallback);

    int UnpublishToLocal(const Key &key, bool deletePublic, bool updateTimestamp);

    int UnpublishInner(SingleVerNaturalStoreCommitNotifyData *&committedData, const SingleVerRecord &syncRecord,
        bool updateTimestamp, int &innerErrCode);

    int UnpublishOper(SingleVerNaturalStoreCommitNotifyData *&committedData, const SingleVerRecord &syncRecord,
        bool updateTimestamp, int operType);

    void ReleaseCommitData(SingleVerNaturalStoreCommitNotifyData *&committedData);

    int PragmaPublish(void *parameter);

    int PragmaUnpublish(void *parameter);

    SQLiteSingleVerStorageExecutor *GetExecutor(bool isWrite, int &errCode) const;

    void ReleaseExecutor(SQLiteSingleVerStorageExecutor *&executor) const;

    int PragmaSetAutoLifeCycle(const uint32_t *lifeTime);
    void InitConflictNotifiedFlag();
    void AddConflictNotifierCount(int target);
    void ResetConflictNotifierCount(int target);

    int PragmaResultSetCacheMode(PragmaData inMode);
    int PragmaResultSetCacheMaxSize(PragmaData inSize);

    // use for getkvstore migrating cache data
    int PragmaTriggerToMigrateData(const SecurityOption &secOption) const;
    int CheckAmendValueContentForLocalProcedure(const Value &oriValue, Value &amendValue) const;

    int SaveLocalEntry(const Entry &entry, bool isDelete);
    int SaveLocalItem(const LocalDataItem &dataItem) const;
    int SaveLocalItemInCacheMode(const LocalDataItem &dataItem) const;
    int SaveEntryNormally(DataItem &dataItem);
    int SaveEntryInCacheMode(DataItem &dataItem, uint64_t recordVersion);

    int StartTransactionInCacheMode();
    int StartTransactionNormally();

    bool IsCacheDBMode() const;
    bool IsExtendedCacheDBMode() const;
    int CheckReadDataControlled() const;
    bool IsFileAccessControlled() const;

    DECLARE_OBJECT_TAG(SQLiteSingleVerNaturalStoreConnection);

    // ResultSet Related Info
    static constexpr std::size_t MAX_RESULT_SET_SIZE = 4; // Max 4 ResultSet At The Same Time
    std::atomic<ResultSetCacheMode> cacheModeForNewResultSet_{ResultSetCacheMode::CACHE_FULL_ENTRY};
    std::atomic<int> cacheMaxSizeForNewResultSet_{0}; // Will be init to default value in constructor

    int conflictType_;
    uint32_t transactionEntrySize_; // used for transaction
    TimeStamp currentMaxTimeStamp_; // used for transaction
    SingleVerNaturalStoreCommitNotifyData *committedData_; // used for transaction
    SingleVerNaturalStoreCommitNotifyData *localCommittedData_;
    std::atomic<bool> transactionExeFlag_;

    NotificationChain::Listener *conflictListener_;
    SQLiteSingleVerStorageExecutor *writeHandle_; // only existed while in transaction.
    mutable std::set<IKvDBResultSet *> kvDbResultSets_;
    std::mutex conflictMutex_;
    std::mutex rekeyMutex_;
    std::mutex importMutex_;
    mutable std::mutex kvDbResultSetsMutex_;
    mutable std::mutex transactionMutex_; // used for transaction
};
}

#endif