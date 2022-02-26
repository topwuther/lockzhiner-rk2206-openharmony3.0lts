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

#ifndef KV_STORE_DELEGATE_H
#define KV_STORE_DELEGATE_H

#include <string>
#include <functional>

#include "types.h"
#include "kv_store_observer.h"
#include "kv_store_snapshot_delegate.h"

namespace DistributedDB {
class KvStoreDelegate {
public:
    using ConflictResolution = std::function<void(void)>;

    struct Option {
        bool createIfNecessary = true;
        bool localOnly = false;
        bool isEncryptedDb = false;
        CipherType cipher = CipherType::DEFAULT;
        CipherPassword passwd;
        bool createDirByStoreIdOnly = false;
    };

    DB_API virtual ~KvStoreDelegate() {}

    // Used to Put a k-v pair to the kvstore.
    // Return OK if operation is successful.
    DB_API virtual DBStatus Put(const Key &key, const Value &value) = 0;

    // Used to Put a vector<Entry> contains k-v pairs to the kvstore.
    // Return OK if operation is successful..
    DB_API virtual DBStatus PutBatch(const std::vector<Entry> &entries) = 0;

    // Delete a record with the given key.
    // Return OK if operation is successful.
    DB_API virtual DBStatus Delete(const Key &key) = 0;

    // Batch delete records with the given keys.
    // Return OK if operation is successful.
    DB_API virtual DBStatus DeleteBatch(const std::vector<Key> &keys) = 0;

    // Delete all record of the kvstore.
    // Return OK if operation is successful.
    DB_API virtual DBStatus Clear() = 0;

    // Return a storeId of the KvStore instance
    DB_API virtual std::string GetStoreId() const = 0;

    // Get a snapshot of the kvstore. The observer is used to notify data changed, it can be null.
    // Return value is DBStatus and KvStoreSnapshotDelegate*, these values will be passed to the callback.
    DB_API virtual void GetKvStoreSnapshot(KvStoreObserver *observer,
        const std::function<void(DBStatus, KvStoreSnapshotDelegate *)> &callback) = 0;

    // Release a snapshot, it will return OK if operation is successful.
    DB_API virtual DBStatus ReleaseKvStoreSnapshot(KvStoreSnapshotDelegate *&snapshotDelegate) = 0;

    // Register a data change observer
    DB_API virtual DBStatus RegisterObserver(KvStoreObserver *observer) = 0;

    // Unregister a data change observer
    DB_API virtual DBStatus UnRegisterObserver(const KvStoreObserver *observer) = 0;

    // Start a transaction
    DB_API virtual DBStatus StartTransaction() = 0;

    // Commit a transaction
    DB_API virtual DBStatus Commit() = 0;

    // Rollback a transaction
    DB_API virtual DBStatus Rollback() = 0;

    // Used to set the resolution policy for conflicts.
    // Return OK if operation is successful.
    DB_API virtual DBStatus SetConflictResolutionPolicy(ResolutionPolicyType type,
        const ConflictResolution &resolution) = 0;

    // Used to rekey the database.
    DB_API virtual DBStatus Rekey(const CipherPassword &password) = 0;

    // Special pragma interface, see PragmaCmd and PragmaData,
    DB_API virtual DBStatus Pragma(PragmaCmd cmd, PragmaData &paramData) = 0;

    // Empty passwords represent non-encrypted files.
    // Export existing database files to a specified database file in the specified directory.
    DB_API virtual DBStatus Export(const std::string &filePath, const CipherPassword &passwd) = 0;

    // Import the existing database files to the specified database file in the specified directory.
    DB_API virtual DBStatus Import(const std::string &filePath, const CipherPassword &passwd) = 0;
};
} // namespace DistributedDB

#endif // KV_STORE_DELEGATE_H