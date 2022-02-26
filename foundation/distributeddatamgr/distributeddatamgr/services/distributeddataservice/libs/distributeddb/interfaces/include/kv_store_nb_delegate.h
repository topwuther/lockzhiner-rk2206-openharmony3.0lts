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

#ifndef KV_STORE_NB_DELEGATE_H
#define KV_STORE_NB_DELEGATE_H

#include <string>
#include <map>
#include <functional>

#include "types.h"
#include "kv_store_observer.h"
#include "kv_store_nb_conflict_data.h"
#include "kv_store_result_set.h"
#include "query.h"
#include "iprocess_system_api_adapter.h"

namespace DistributedDB {
enum ObserverMode {
    OBSERVER_CHANGES_NATIVE = 1,
    OBSERVER_CHANGES_FOREIGN = 2,
    OBSERVER_CHANGES_LOCAL_ONLY = 4,
};

enum SyncMode {
    SYNC_MODE_PUSH_ONLY,
    SYNC_MODE_PULL_ONLY,
    SYNC_MODE_PUSH_PULL,
};

enum ConflictResolvePolicy {
    LAST_WIN = 0,
    DEVICE_COLLABORATION,
};

using KvStoreNbPublishOnConflict = std::function<void (const Entry &local, const Entry *sync, bool isLocalLastest)>;
using KvStoreNbConflictNotifier = std::function<void (const KvStoreNbConflictData &data)>;

class KvStoreNbDelegate {
public:
    struct Option {
        bool createIfNecessary = true;
        bool isMemoryDb = false;
        bool isEncryptedDb = false;
        CipherType cipher = CipherType::DEFAULT;
        CipherPassword passwd;
        std::string schema = "";
        bool createDirByStoreIdOnly = false;
        SecurityOption secOption; // Add data security level parameter
        KvStoreObserver *observer = nullptr;
        Key key; // The key that needs to be subscribed on obsever, emptye means full subscription
        unsigned int mode = 0; // obsever mode
        int conflictType = 0;
        KvStoreNbConflictNotifier notifier = nullptr;
        int conflictResolvePolicy = LAST_WIN;
    };

    DB_API virtual ~KvStoreNbDelegate() {}

    // Public zone interfaces
    // Get value from the public zone of this store according to the key.
    DB_API virtual DBStatus Get(const Key &key, Value &value) const = 0;

    // Get entries from the public zone of this store by key prefix.
    // If 'keyPrefix' is empty, It would return all the entries in the zone.
    DB_API virtual DBStatus GetEntries(const Key &keyPrefix, std::vector<Entry> &entries) const = 0;

    // Get entries from the public zone of this store by key prefix.
    // If 'keyPrefix' is empty, It would return all the entries in the zone.
    DB_API virtual DBStatus GetEntries(const Key &keyPrefix, KvStoreResultSet *&resultSet) const = 0;

    // Get entries from the public zone of this store by query.
    // If 'query' is empty, It would return all the entries in the zone.
    DB_API virtual DBStatus GetEntries(const Query &query, std::vector<Entry> &entries) const = 0;

    // Get entries from the public zone of this store by query.
    // If query is empty, It would return all the entries in the zone.
    DB_API virtual DBStatus GetEntries(const Query &query, KvStoreResultSet *&resultSet) const = 0;

    // Get count from the public zone of this store those meet conditions.
    // If query is empty, It would return all the entries count in the zone.
    DB_API virtual DBStatus GetCount(const Query &query, int &count) const = 0;

    // Close the result set returned by GetEntries().
    DB_API virtual DBStatus CloseResultSet(KvStoreResultSet *&resultSet) = 0;

    // Put one key-value entry into the public zone of this store.
    DB_API virtual DBStatus Put(const Key &key, const Value &value) = 0;

    // Put a batch of entries into the public zone of this store.
    DB_API virtual DBStatus PutBatch(const std::vector<Entry> &entries) = 0;

    // Delete a batch of entries from the public zone of this store.
    DB_API virtual DBStatus DeleteBatch(const std::vector<Key> &keys) = 0;

    // Delete one key-value entry from the public zone of this store according to the key.
    DB_API virtual DBStatus Delete(const Key &key) = 0;

    // Local zone interfaces
    // Get value from the local zone of this store according to the key.
    DB_API virtual DBStatus GetLocal(const Key &key, Value &value) const = 0;

    // Get key-value entries from the local zone of this store by key prefix.
    // If keyPrefix is empty, It would return all the entries in the local zone.
    DB_API virtual DBStatus GetLocalEntries(const Key &keyPrefix, std::vector<Entry> &entries) const = 0;

    // Put one key-value entry into the local zone of this store.
    DB_API virtual DBStatus PutLocal(const Key &key, const Value &value) = 0;

    // Delete one key-value entry from the local zone of this store according to the key.
    DB_API virtual DBStatus DeleteLocal(const Key &key) = 0;

    // Migrating(local zone <-> public zone) interfaces
    // Publish a local key-value entry.
    // Migrate the entry from the local zone to public zone.
    DB_API virtual DBStatus PublishLocal(const Key &key, bool deleteLocal, bool updateTimestamp,
        const KvStoreNbPublishOnConflict &onConflict) = 0;

    // Unpublish a public key-value entry.
    // Migrate the entry from the public zone to local zone.
    DB_API virtual DBStatus UnpublishToLocal(const Key &key, bool deletePublic, bool updateTimestamp) = 0;

    // Observer interfaces
    // Register one observer which concerns the key and the changed data mode.
    // If key is empty, observer would get all the changed data of the mode.
    // There are three mode: native changes of nb syncable kv store,
    //                       synced data changes from remote devices,
    //                       local changes of local kv store.
    DB_API virtual DBStatus RegisterObserver(const Key &key, unsigned int mode, KvStoreObserver *observer) = 0;

    // UnRegister the registered observer.
    DB_API virtual DBStatus UnRegisterObserver(const KvStoreObserver *observer) = 0;

    // Remove the device data synced from remote.
    DB_API virtual DBStatus RemoveDeviceData(const std::string &device) = 0;

    // Other interfaces
    DB_API virtual std::string GetStoreId() const = 0;

    // Sync function interface, if wait set true, this function will be blocked until sync finished
    DB_API virtual DBStatus Sync(const std::vector<std::string> &devices, SyncMode mode,
        const std::function<void(const std::map<std::string, DBStatus> &devicesMap)> &onComplete,
        bool wait = false) = 0;

    // Special pragma interface, see PragmaCmd and PragmaData,
    DB_API virtual DBStatus Pragma(PragmaCmd cmd, PragmaData &paramData) = 0;

    // Set the conflict notifier for getting the specified type conflict data.
    DB_API virtual DBStatus SetConflictNotifier(int conflictType,
        const KvStoreNbConflictNotifier &notifier) = 0;

    // Used to rekey the database.
    // Warning rekey may reopen database file, file handle may lose while locked
    DB_API virtual DBStatus Rekey(const CipherPassword &password) = 0;

    // Empty passwords represent non-encrypted files.
    // Export existing database files to a specified database file in the specified directory.
    DB_API virtual DBStatus Export(const std::string &filePath, const CipherPassword &passwd) = 0;

    // Import the existing database files to the specified database file in the specified directory.
    // Warning Import may reopen database file in locked state
    DB_API virtual DBStatus Import(const std::string &filePath, const CipherPassword &passwd) = 0;

    // Start a transaction
    DB_API virtual DBStatus StartTransaction() = 0;

    // Commit a transaction
    DB_API virtual DBStatus Commit() = 0;

    // Rollback a transaction
    DB_API virtual DBStatus Rollback() = 0;

    // Put a batch of entries into the local zone of this store.
    DB_API virtual DBStatus PutLocalBatch(const std::vector<Entry> &entries) = 0;

    // Delete a batch of entries from the local zone of this store according to the keys.
    DB_API virtual DBStatus DeleteLocalBatch(const std::vector<Key> &keys) = 0;

    // Get the SecurityOption of this kvStore.
    DB_API virtual DBStatus GetSecurityOption(SecurityOption &option) const = 0;

    // Set a notify callback, it will be called when remote push or push_pull finished.
    // If Repeat set, subject to the last time.
    // If set nullptr, means unregister the notify.
    DB_API virtual DBStatus SetRemotePushFinishedNotify(const RemotePushFinishedNotifier &notifier) = 0;
};
} // namespace DistributedDB

#endif // KV_STORE_NB_DELEGATE_H
