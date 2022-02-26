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

#ifndef APP_KV_STORE_H
#define APP_KV_STORE_H

#include <map>
#include <functional>
#include "app_kvstore_conflict_data.h"
#include "app_kvstore_observer.h"
#include "app_types.h"
#include "app_kvstore_result_set.h"

namespace OHOS {
namespace AppDistributedKv {
// This is a public interface. Implementation of this class is in AppKvStoreImpl.
// This class provides put, delete, search, sync and subscribe functions of a key-value store.
class AppKvStore {
public:
    KVSTORE_API virtual ~AppKvStore()
    {}

    // Get id of this AppKvStore.
    KVSTORE_API virtual const std::string &GetStoreId() = 0;

    // Write a pair of key and value to this store. Set write option to local if you do not this entry sync to other
    // devices.
    // Parameters:
    //     options: mark this is a local entry or not.
    //     key: key of this entry. Should be less than 256 bytes. key will be trimmed before store.
    //     value: value of this entry. Should be less than (1024 * 1024) bytes.
    // Return:
    //     Status of this put operation.
    KVSTORE_API virtual Status Put(const WriteOptions &options, const Key &key, const Value &value) = 0;

    // Delete an entry by its key. Set write option to local if you want this delete to be a local change.
    // Parameters:
    //     options: mark this delete is a local change or not.
    //     key: key of the entry to be deleted.
    // Return:
    //     Status of this delete operation.
    KVSTORE_API virtual Status Delete(const WriteOptions &options, const Key &key) = 0;

    // Get value from AppKvStore by its key. Set options->local to true if you want to get from local kvstore.
    // Parameters:
    //     options: mark we get from local store or remote store. options->batch is a reserved parameter and should
    //              always be false.
    //     key: key of this entry.
    //     value: value will be returned in this parameter.
    // Return:
    //     Status of this get operation.
    KVSTORE_API virtual Status Get(const ReadOptions &options, const Key &key, Value &value) = 0;

    // Get all entries in this store which key start with prefixKey. This function will always get from synced store.
    // Parameters:
    //     prefixkey: the prefix to be searched.
    //     entries: entries will be returned in this parameter.
    // Return:
    //     Status of this GetEntries operation.
    KVSTORE_API virtual Status GetEntries(const Key &prefixKey, std::vector<Entry> &entries) = 0;

    // Get all entries in this store which key start with prefixKey. This function will always get from synced store.
    // Parameters:
    //     prefixkey: the prefix to be searched.
    //     resultSet: resultSet will be returned in this parameter.
    // Return:
    //     Status of this GetEntries operation.
    KVSTORE_API virtual Status GetEntries(const Key &prefixKey, AppKvStoreResultSet *&resultSet) = 0;

    // Close the result set returned by GetEntries().
    // Parameters:
    //     resultSet: resultSet will be returned in this parameter.
    // Return:
    //     Status of this GetEntries operation.
    KVSTORE_API virtual Status CloseResultSet(AppKvStoreResultSet *&resultSet) = 0;

    // Sync store with other devices. This is an asynchronous method,
    // sync will fail if there is a syncing operation in progress.
    // Parameters:
    //     deviceIdList: device list to sync.
    //     mode: mode can be set to SyncMode::PUSH, SyncMode::PULL and SyncMode::PUTH_PULL. PUSH_PULL will firstly
    //           push all not-local store to listed devices, then pull these stores back.
    //     callback: return <device-id, sync-result> map to caller.
    // Return:
    //     Status of this Sync operation.
    KVSTORE_API virtual Status Sync(const std::vector<std::string> &deviceIdList, const SyncMode &mode,
                                    const std::function<void(const std::map<std::string, Status> &)> &callback) = 0;

    // Register change of this kvstore to a client-defined observer. observer->OnChange method will be called when store
    // changes. One observer can subscribe more than one AppKvStore.
    // Parameters:
    //     options: mark this is a local entry or not.
    //     subscribeType: OBSERVER_CHANGES_NATIVE means native changes of syncable kv store,
    //                  : OBSERVER_CHANGES_FOREIGN means synced data changes from remote devices,
    //                  : OBSERVER_CHANGES_ALL means both native changes and synced data changes.
    //     observer: observer to subscribe changes.
    // Return:
    //     Status of this subscribe operation.
    KVSTORE_API virtual Status SubscribeKvStore(const ReadOptions &options, const SubscribeType &subscribeType,
                                                AppKvStoreObserver *observer) = 0;

    // Unregister a kvstore to an observer.
    // Parameters:
    //     options: mark this is a local entry or not.
    //     subscribeType: reserved parameter. Current is always SubscribeType::DEFAULT.
    //     observer: observer to unsubscribe this store.
    // Return:
    //     Status of this unsubscribe operation.
    KVSTORE_API virtual Status UnSubscribeKvStore(const ReadOptions &options, const SubscribeType &subscribeType,
                                                  AppKvStoreObserver *observer) = 0;

    // Remove the device data synced from remote.
    // Parameters:
    //     device: device id.
    // Return:
    //     Status of this remove operation.
    KVSTORE_API virtual Status RemoveDeviceData(const std::string &device) = 0;

    // Set policy of conflict resolution.
    // Parameters:
    //     conflictType: include CONFLICT_FOREIGN_KEY_ONLY  CONFLICT_FOREIGN_KEY_ORIG  CONFLICT_NATIVE_ALL.
    //     callback: conflict resolution callback.
    // Return:
    //     Status of Setting policy operation.
    KVSTORE_API
    virtual Status SetConflictResolutionPolicy(AppKvStoreConflictPolicyType conflictType,
                                               std::function<void(const AppKvStoreConflictData &data)> callback) = 0;

    // Export current data store to ${filePath} using ${passwd}
    // Parameters:
    //     filePath: directory which store will be saved.
    //     passwd: can be null, which means dont cipher.
    // Return:
    //     Status of this operation.
    KVSTORE_API virtual Status Export(const std::string &filePath, const std::vector<uint8_t> &passwd) = 0;

    // Import current data store to ${filePath} using ${passwd}
    // Parameters:
    //     filePath: directory from which will recovery.
    //     passwd: can be null, which means dont cipher.
    // Return:
    //     Status of this operation.
    KVSTORE_API virtual Status Import(const std::string &filePath, const std::vector<uint8_t> &passwd) = 0;

    // get security level.
    // Parameters:
    //     securityLevel: the security level.
    // Return:
    //     Status of this operation.
    KVSTORE_API virtual Status GetSecurityLevel(SecurityLevel &securityLevel) const = 0;
};
}  // namespace AppDistributedKv
}  // namespace OHOS

#endif  // APP_KV_STORE_H
