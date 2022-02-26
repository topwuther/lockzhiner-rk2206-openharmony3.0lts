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

#ifndef FAKE_KV_STORE_NB_DELEGATE_IMPL_H
#define FAKE_KV_STORE_NB_DELEGATE_IMPL_H

#include <functional>
#include <map>
#include <mutex>
#include <string>
#include <memory>
#include "kv_store_nb_delegate.h"

namespace DistributedDB {
class UtKvStoreNbDelegateImpl : public KvStoreNbDelegate {
public:
    UtKvStoreNbDelegateImpl(const std::string &storeId, const std::string &deviceId);
    ~UtKvStoreNbDelegateImpl() override;

    DBStatus Get(const Key &key, Value &value) const override;
    DBStatus GetEntries(const Key &keyPrefix, std::vector<Entry> &entries) const override;
    DBStatus GetEntries(const Key &keyPrefix, KvStoreResultSet *&resultSet) const override;
    DBStatus GetEntries(const Query &query, std::vector<Entry> &entries) const override;
    DBStatus GetEntries(const Query &query, KvStoreResultSet *&resultSet) const override;
    DBStatus GetCount(const Query &query, int &count) const override;
    DBStatus CloseResultSet(KvStoreResultSet *&resultSet) override;
    DBStatus Put(const Key &key, const Value &value) override;
    DBStatus PutBatch(const std::vector<Entry> &entries) override;
    DBStatus Delete(const Key &key) override;
    DBStatus DeleteBatch(const std::vector<Key> &keys) override;

    // Local entry interfaces
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

    // Used to rekey the database.
    DB_API DBStatus Rekey(const CipherPassword &password) override;

    DBStatus Export(const std::string &filePath, const CipherPassword &passwd) override;
    DBStatus Import(const std::string &filePath, const CipherPassword &passwd) override;

    // Start a transaction
    DBStatus StartTransaction() override;

    // Commit a transaction
    DBStatus Commit() override;

    // Rollback a transaction
    DBStatus Rollback() override;

    void SetSyncStatus(DBStatus status);
    void SetNeighbor(const std::shared_ptr<UtKvStoreNbDelegateImpl> &neighbor);
    DBStatus PutLocalBatch(const std::vector<Entry> &entries) override;

    DBStatus DeleteLocalBatch(const std::vector<Key> &keys) override;
    // Get the SecurityOption of this kvStore.
    DBStatus GetSecurityOption(SecurityOption &option) const override { return DBStatus::NOT_SUPPORT; };

    // Set a notify callback, it will be called when remote push or push_pull finished.
    // If Repeat set, subject to the last time.
    // If set nullptr, means unregister the notify.
    DBStatus SetRemotePushFinishedNotify(const RemotePushFinisheNotifier &notifier) override
    {
        return DBStatus::NOT_SUPPORT;
    };
private:
    static constexpr size_t MAX_KEY_SIZE = 1024; // 1KB
    static constexpr size_t MAX_VALUE_SIZE = 4 * 1024 * 1024; // 4MB
    struct DbValue {
        Value value{};
        bool isLocalPut{ true };
        bool isDeleted{ false };
        bool isSynced{ false };
    };

    static bool IsValidKey(const Key &key);
    static bool IsValidValue(const Value &value);
    DBStatus PutByOtherDevice(const Key &key, const Value &value);
    DBStatus DeleteByOtherDevice(const Key &key);
    DBStatus DoPut(const Key &key, const DbValue &dbValue);
    DBStatus DoDelete(const Key &key, bool isLocalDelete);

    std::string storeId_{};
    std::string deviceId_{};
    std::map<Key, DbValue> db_{};
    std::vector<const KvStoreObserver *> observerMap_{};
    std::weak_ptr<UtKvStoreNbDelegateImpl> neighbor_{};
    DBStatus syncStatus_{ OK };
};
}  // namespace DistributedDB

#endif
