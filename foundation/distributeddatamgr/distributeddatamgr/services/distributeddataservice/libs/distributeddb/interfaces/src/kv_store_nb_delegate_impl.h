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

#ifndef KV_STORE_NB_DELEGATE_IMPL_H
#define KV_STORE_NB_DELEGATE_IMPL_H

#include <string>
#include <functional>
#include <map>
#include <mutex>

#include "types.h"
#include "db_types.h"
#include "ikvdb_connection.h"
#include "kv_store_nb_conflict_data.h"
#include "kv_store_nb_delegate.h"

namespace DistributedDB {
class KvStoreNbDelegateImpl final : public KvStoreNbDelegate {
public:
    KvStoreNbDelegateImpl(IKvDBConnection *conn, const std::string &storeId);
    ~KvStoreNbDelegateImpl() override;

    DISABLE_COPY_ASSIGN_MOVE(KvStoreNbDelegateImpl);

    // Public zone interfaces
    DBStatus Get(const Key &key, Value &value) const override;

    DBStatus GetEntries(const Key &keyPrefix, std::vector<Entry> &entries) const override;

    DBStatus GetEntries(const Key &keyPrefix, KvStoreResultSet *&resultSet) const override;

    DBStatus GetEntries(const Query &query, std::vector<Entry> &entries) const override;

    DBStatus GetEntries(const Query &query, KvStoreResultSet *&resultSet) const override;

    DBStatus GetCount(const Query &query, int &count) const override;

    DBStatus CloseResultSet(KvStoreResultSet *&resultSet) override;

    DBStatus Put(const Key &key, const Value &value) override;

    DBStatus PutBatch(const std::vector<Entry> &entries) override;

    DBStatus DeleteBatch(const std::vector<Key> &keys) override;

    DBStatus Delete(const Key &key) override;

    // Local zone interfaces
    DBStatus GetLocal(const Key &key, Value &value) const override;

    DBStatus GetLocalEntries(const Key &keyPrefix, std::vector<Entry> &entries) const override;

    DBStatus PutLocal(const Key &key, const Value &value) override;

    DBStatus DeleteLocal(const Key &key) override;

    DBStatus PublishLocal(const Key &key, bool deleteLocal, bool updateTimestamp,
        const KvStoreNbPublishOnConflict &onConflict) override;

    DBStatus UnpublishToLocal(const Key &key, bool deletePublic, bool updateTimestamp) override;

    // Observer interfaces
    DBStatus RegisterObserver(const Key &key, unsigned int mode, KvStoreObserver *observer) override;

    DBStatus UnRegisterObserver(const KvStoreObserver *observer) override;

    DBStatus RemoveDeviceData(const std::string &device) override;

    // Other interfaces
    std::string GetStoreId() const override;

    // Sync function interface, if wait set true, this function will be blocked until sync finished
    DB_API DBStatus Sync(const std::vector<std::string> &devices, SyncMode mode,
        const std::function<void(const std::map<std::string, DBStatus> &devicesMap)> &onComplete,
        bool wait) override;

    // Special pragma interface, see PragmaCmd and PragmaData,
    DB_API DBStatus Pragma(PragmaCmd cmd, PragmaData &paramData) override;

    // Set the conflict notifier for getting the specified type conflict data.
    DB_API DBStatus SetConflictNotifier(int conflictType, const KvStoreNbConflictNotifier &notifier) override;

    // Rekey the database.
    DBStatus Rekey(const CipherPassword &password) override;

    // Empty passwords represent non-encrypted files.
    // Export existing database files to a specified database file in the specified directory.
    DBStatus Export(const std::string &filePath, const CipherPassword &passwd) override;

    // Import the existing database files to the specified database file in the specified directory.
    DBStatus Import(const std::string &filePath, const CipherPassword &passwd) override;

    // Start a transaction
    DBStatus StartTransaction() override;

    // Commit a transaction
    DBStatus Commit() override;

    // Rollback a transaction
    DBStatus Rollback() override;

    DBStatus PutLocalBatch(const std::vector<Entry> &entries) override;

    DBStatus DeleteLocalBatch(const std::vector<Key> &keys) override;

    // Get the SecurityOption of this kvStore.
    DBStatus GetSecurityOption(SecurityOption &option) const override;

    DBStatus SetRemotePushFinishedNotify(const RemotePushFinishedNotifier &notifier) override;

    void SetReleaseFlag(bool flag);

    DBStatus Close();

private:
    DBStatus GetInner(const IOption &option, const Key &key, Value &value) const;
    DBStatus PutInner(const IOption &option, const Key &key, const Value &value);
    DBStatus DeleteInner(const IOption &option, const Key &key);
    DBStatus GetEntriesInner(const IOption &option, const Key &keyPrefix, std::vector<Entry> &entries) const;

    void OnSyncComplete(const std::map<std::string, int> &statuses,
        const std::function<void(const std::map<std::string, DBStatus> &devicesMap)> &onComplete) const;

    IKvDBConnection *conn_;
    std::string storeId_;
    bool releaseFlag_;
    std::mutex observerMapLock_;
    std::map<const KvStoreObserver *, const KvDBObserverHandle *> observerMap_;
};
} // namespace DistributedDB

#endif // KV_STORE_NB_DELEGATE_IMPL_H
