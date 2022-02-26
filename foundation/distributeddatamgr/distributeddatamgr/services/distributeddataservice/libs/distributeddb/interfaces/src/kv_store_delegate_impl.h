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

#ifndef KV_STORE_DELEGATE_IMPL_H
#define KV_STORE_DELEGATE_IMPL_H

#ifndef OMIT_MULTI_VER
#include <string>
#include <functional>
#include <map>

#include "types.h"
#include "ikvdb_connection.h"
#include "ikvdb_factory.h"
#include "kv_store_delegate.h"

namespace DistributedDB {
class KvStoreDelegateImpl final : public KvStoreDelegate {
public:
    KvStoreDelegateImpl(IKvDBConnection *conn, const std::string &storeId);
    ~KvStoreDelegateImpl();

    DISABLE_COPY_ASSIGN_MOVE(KvStoreDelegateImpl);

    // Used to Put a k-v pair to the kvstore.
    // Return OK if the operation is successful.
    DBStatus Put(const Key &key, const Value &value) override;

    // Used to Put a vector<Entry> contains k-v pairs to the kvstore.
    // Return OK if the operation is successful.
    DBStatus PutBatch(const std::vector<Entry> &entries) override;

    // Delete a record with the given key.
    // Return OK if the operation is successful.
    DBStatus Delete(const Key &key) override;

    // Batch delete records with the given keys.
    // Return OK if the operation is successful.
    DBStatus DeleteBatch(const std::vector<Key> &keys) override;

    // Delete all record of th kvstore.
    // Return OK if the operation is successful.
    DBStatus Clear() override;

    // Return a storeId of the KvStore instance
    std::string GetStoreId() const override;

    // Get a snapshot of the kvstore. the observer is used to notify data changed, it can be null.
    // return value is DBStatus and KvStoreSnapshotDelegate*, these values will be passed to the callback.
    void GetKvStoreSnapshot(KvStoreObserver *observer,
        const std::function<void(DBStatus, KvStoreSnapshotDelegate *)> &callback) override;

    // Release a snapshot, it will return OK if the operation is successful.
    DBStatus ReleaseKvStoreSnapshot(KvStoreSnapshotDelegate *&snapshotDelegate) override;

    // Register a data change observer
    DBStatus RegisterObserver(KvStoreObserver *observer) override;

    // Unregister a data change observer
    DBStatus UnRegisterObserver(const KvStoreObserver *observer) override;

    // Start a transaction
    DBStatus StartTransaction() override;

    // Commit a transaction
    DBStatus Commit() override;

    // Rollback a transaction
    DBStatus Rollback() override;

    // Used to set the resolution policy for conflicts.
    // Return OK if operation is successful.
    DBStatus SetConflictResolutionPolicy(ResolutionPolicyType type, const ConflictResolution &resolution) override;

    // Rekey the database.
    DBStatus Rekey(const CipherPassword &password) override;

    // Empty passwords represent non-encrypted files.
    // Export existing database files to a specified database file in the specified directory.
    DBStatus Export(const std::string &filePath, const CipherPassword &passwd) override;

    // Import the existing database files to the specified database file in the specified directory.
    DBStatus Import(const std::string &filePath, const CipherPassword &passwd) override;

    // Set release flag, KvStoreManagerDelegate will set when release the kvstore
    void SetReleaseFlag(bool flag);

    // Close the KvStoreDelegateImpl
    DBStatus Close();

    // Special pragma interface, see PragmaCmd and PragmaData,
    DBStatus Pragma(PragmaCmd cmd, PragmaData &paramData) override;

private:
    IKvDBConnection *conn_;
    std::string storeId_;
    bool releaseFlag_;
    std::mutex observerMapLock_;
    std::map<const KvStoreObserver*, const KvDBObserverHandle*> observerMap_;
};
} // namespace DistributedDB

#endif // KV_STORE_DELEGATE_IMPL_H
#endif