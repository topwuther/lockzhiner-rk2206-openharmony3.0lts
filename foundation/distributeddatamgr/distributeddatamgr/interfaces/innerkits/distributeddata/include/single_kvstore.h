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

#ifndef SINGLE_KV_STORE_H
#define SINGLE_KV_STORE_H

#include <map>
#include "kvstore.h"
#include "kvstore_observer.h"
#include "kvstore_result_set.h"
#include "kvstore_sync_callback.h"
#include "types.h"
#include "data_query.h"

namespace OHOS {
namespace DistributedKv {
// This is a public interface. Implementation of this class is in AppKvStoreImpl.
// This class provides put, delete, search, sync and subscribe functions of a key-value store.
class SingleKvStore {
public:
    KVSTORE_API SingleKvStore() = default;

    KVSTORE_API virtual ~SingleKvStore()
    {}

    // Get all entries in this store which key start with prefixKey.
    // Parameters:
    //     perfixkey: the prefix to be searched.
    //     entries: entries will be returned in this parameter.
    // Return:
    //     Status of this GetEntries operation.
    KVSTORE_API virtual Status GetEntries(const Key &prefixKey, std::vector<Entry> &entries) const = 0;

    // Get all entries in this store by query.
    // Parameters:
    //     query: the query string.
    //     entries: entries will be returned in this parameter.
    // Return:
    //     Status of this GetEntries operation.
    virtual Status GetEntriesWithQuery(const std::string &query, std::vector<Entry> &entries) const = 0;

    // Get all entries in this store by query.
    // Parameters:
    //     query: the query object.
    //     entries: entries will be returned in this parameter.
    // Return:
    //     Status of this GetEntries operation.
    KVSTORE_API virtual Status GetEntriesWithQuery(const DataQuery &query, std::vector<Entry> &entries) const = 0;

    // Get ResultSet in this store which key start with prefixKey.
    // Parameters:
    //     perfixkey: the prefix to be searched.
    //     resultSet: resultSet will be returned in this parameter.
    // Return:
    //     Status of this GetResultSet operation.
    KVSTORE_API virtual void GetResultSet(const Key &prefixKey,
            std::function<void(Status, std::unique_ptr<KvStoreResultSet>)> callback) const = 0;

    // Get ResultSet in this store by Query.
    // Parameters:
    //     query: the query string.
    //     resultSet: resultSet will be returned in this parameter.
    // Return:
    //     Status of this GetResultSet operation.
    virtual void GetResultSetWithQuery(const std::string &query,
                 std::function<void(Status, std::unique_ptr<KvStoreResultSet>)> callback) const = 0;

    // Get ResultSet in this store by Query.
    // Parameters:
    //     query: the query object.
    //     resultSet: resultSet will be returned in this parameter.
    // Return:
    //     Status of this GetResultSet operation.
    KVSTORE_API virtual void GetResultSetWithQuery(const DataQuery &query,
                std::function<void(Status, std::unique_ptr<KvStoreResultSet>)> callback) const = 0;

    // Close the ResultSet returned by GetResultSet.
    // Parameters:
    //     resultSet: resultSet will be returned in this parameter.
    // Return:
    //     Status of this CloseResultSet operation.
    KVSTORE_API virtual Status CloseResultSet(std::unique_ptr<KvStoreResultSet> resultSet) = 0;

    // Get the number of result by query.
    // Parameters:
    //     query: the query string.
    //     result: result will be returned in this parameter.
    // Return:
    //     Status of this CloseResultSet operation.
    virtual Status GetCountWithQuery(const std::string &query, int &result) const = 0;

    // Get the number of result by query.
    // Parameters:
    //     query: the query object.
    //     result: result will be returned in this parameter.
    // Return:
    //     Status of this CloseResultSet operation.
    KVSTORE_API virtual Status GetCountWithQuery(const DataQuery &query, int &result) const = 0;

    // Sync store with other devices. This is an asynchronous method,
    // sync will fail if there is a syncing operation in progress.
    // Parameters:
    //     deviceIdList: device list to sync.
    //     mode: mode can be set to SyncMode::PUSH, SyncMode::PULL and SyncMode::PUTH_PULL. PUSH_PULL will firstly
    //           push all not-local store to listed devices, then pull these stores back.
    //     allowedDelayMs: allowed delay milli-second to sync. default value is 0 for compatibility.
    // Return:
    //     Status of this Sync operation.
    KVSTORE_API virtual Status Sync(const std::vector<std::string> &deviceIdList, const SyncMode &mode,
                                    uint32_t allowedDelayMs = 0) = 0;

    // Remove the device data synced from remote.
    // Parameters:
    //     device: device id.
    // Return:
    //     Status of this remove operation.
    KVSTORE_API virtual Status RemoveDeviceData(const std::string &device) = 0;

    // Get id of this SingleKvStore.
    KVSTORE_API virtual StoreId GetStoreId() const = 0;

    // Delete an entry by its key.
    // Parameters:
    //     key: key of the entry to be deleted.
    // Return:
    //     Status of this delete operation.
    KVSTORE_API virtual Status Delete(const Key &key) = 0;

    // Write a pair of key and value to this store.
    // Parameters:
    //     key: key of this entry. Should be less than 1024 bytes. key will be trimmed before store.
    //     value: value of this entry. Should be less than (4 * 1024 * 1024) bytes.
    // Return:
    //     Status of this put operation.
    KVSTORE_API virtual Status Put(const Key &key, const Value &value) = 0;

    // Get value from AppKvStore by its key.
    // Parameters:
    //     key: key of this entry.
    //     value: value will be returned in this parameter.
    // Return:
    //     Status of this get operation.
    KVSTORE_API virtual Status Get(const Key &key, Value &value) = 0;

    // subscribe change of this kvstore to a client-defined observer. observer->OnChange method will be called when store
    // changes.
    // Parameters:
    //     subscribeType: SUBSCRIBE_TYPE_LOCAL means local changes of syncable kv store,
    //                  : SUBSCRIBE_TYPE_REMOTE means synced data changes from remote devices,
    //                  : SUBSCRIBE_TYPE_ALL means both local changes and synced data changes.
    //     observer: observer to subscribe changes.
    // Return:
    //     Status of this subscribe operation.
    KVSTORE_API
    virtual Status SubscribeKvStore(SubscribeType subscribeType, std::shared_ptr<KvStoreObserver> observer) = 0;

    // un-subscribe change of this kvstore to a client-defined observer.
    // Parameters:
    //     subscribeType: SUBSCRIBE_TYPE_LOCAL means local changes of syncable kv store,
    //                  : SUBSCRIBE_TYPE_REMOTE means synced data changes from remote devices,
    //                  : SUBSCRIBE_TYPE_ALL means both local changes and synced data changes.
    //     observer: observer to subscribe changes.
    // Return:
    //     Status of this subscribe operation.
    KVSTORE_API
    virtual Status UnSubscribeKvStore(SubscribeType subscribeType, std::shared_ptr<KvStoreObserver> observer) = 0;

    // register message for sync operation.
    // Parameters:
    //     callback: callback to register.
    // Return:
    //     Status of this register operation.
    KVSTORE_API
    virtual Status RegisterSyncCallback(std::shared_ptr<KvStoreSyncCallback> callback) = 0;

    // un-register message for sync operation.
    // Parameters:
    //     callback: callback to register.
    // Return:
    //     Status of this register operation.
    KVSTORE_API
    virtual Status UnRegisterSyncCallback() = 0;

    // see Put, PutBatch put a list of entries to kvstore,
    // all entries will be put in a transaction,
    // if entries contains invalid entry, PutBatch will all fail.
    // entries's size should be less than 128 and memory size must be less than IPC transport limit.
    KVSTORE_API virtual Status PutBatch(const std::vector<Entry> &entries) = 0;

    // delete a list of entries in the kvstore,
    // delete key not exist still return success,
    // key length should not be greater than 256, and can not be empty.
    // if keys contains invalid key, all delete will fail.
    // keys memory size should not be greater than IPC transport limit, and can not be empty.
    KVSTORE_API virtual Status DeleteBatch(const std::vector<Key> &keys) = 0;

    // start transaction.
    // all changes to this kvstore will be in a same transaction and will not change the store until Commit() or
    // Rollback() is called.
    // before this transaction is committed or rollbacked, all attemption to close this store will fail.
    KVSTORE_API virtual Status StartTransaction() = 0;

    // commit current transaction. all changes to this store will be done after calling this method.
    // any calling of this method outside a transaction will fail.
    KVSTORE_API virtual Status Commit() = 0;

    // rollback current transaction.
    // all changes to this store during this transaction will be rollback after calling this method.
    // any calling of this method outside a transaction will fail.
    KVSTORE_API virtual Status Rollback() = 0;

    // set synchronization parameters of this store.
    // Parameters:
    //     syncParam: sync policy parameter.
    // Return:
    //     Status of this operation.
    KVSTORE_API virtual Status SetSyncParam(const KvSyncParam &syncParam) = 0;

    // get synchronization parameters of this store.
    // Parameters:
    //     syncParam: sync policy parameter.
    // Return:
    //     Status of this operation.
    KVSTORE_API virtual Status GetSyncParam(KvSyncParam &syncParam) = 0;

    KVSTORE_API virtual Status SetCapabilityEnabled(bool enabled) const = 0;

    KVSTORE_API virtual Status SetCapabilityRange(const std::vector<std::string> &localLabels,
                                                  const std::vector<std::string> &remoteSupportLabels) const = 0;

    KVSTORE_API virtual Status GetSecurityLevel(SecurityLevel &securityLevel) const = 0;
protected:
    // control this store.
    // Parameters:
    //     inputParam: input parameter.
    //     output: output data, nullptr if no data is returned.
    // Return:
    //     Status of this control operation.
    KVSTORE_API virtual Status Control(KvControlCmd cmd, const KvParam &inputParam, sptr<KvParam> &output) = 0;
};
}  // namespace AppDistributedKv
}  // namespace OHOS
#endif  // SINGLE_KV_STORE_H
