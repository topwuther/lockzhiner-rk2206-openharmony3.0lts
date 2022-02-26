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

#ifndef APP_KV_STORE_IMPL_H
#define APP_KV_STORE_IMPL_H

#include <mutex>
#include <set>
#include "app_kvstore.h"
#include "app_kvstore_conflict_data_impl.h"
#include "app_kvstore_observer.h"
#include "app_kvstore_result_set.h"
#include "app_types.h"
#include "constant.h"
#include "kv_store_delegate_manager.h"
#include "kv_store_nb_delegate.h"
#include "kvstore_observer_nb_impl.h"

namespace OHOS {
namespace AppDistributedKv {
class AppKvStoreImpl : public AppKvStore {
public:
    AppKvStoreImpl(const std::string &storeId, DistributedDB::KvStoreNbDelegate *kvStoreNbDelegate);

    AppKvStoreImpl(AppKvStoreImpl &&) = delete;
    AppKvStoreImpl &operator=(AppKvStoreImpl &&) = delete;

    // forbidden copy constructor.
    AppKvStoreImpl(const AppKvStoreImpl &) = delete;
    AppKvStoreImpl &operator=(const AppKvStoreImpl &) = delete;

    virtual ~AppKvStoreImpl();

    // Get id of this AppKvStore.
    const std::string &GetStoreId() override;

    // Write a pair of key and value to this store. Set write option to local if you do not this entry sync to other
    // devices.
    // Parameters:
    //     options: mark this is a local entry or not.
    //     key: key of this entry. Should be less than 256 bytes. key will be trimmed before store.
    //     value: value of this entry. Should be less than (1024 * 1024) bytes.
    // Return:
    //     Status of this put operation.
    Status Put(const WriteOptions &options, const Key &key, const Value &value) override;

    // Delete an entry by its key. Set write option to local if you want this delete to be a local change.
    // Parameters:
    //     options: mark this delete is a local change or not.
    //     key: key of the entry to be deleted.
    // Return:
    //     Status of this delete operation.
    Status Delete(const WriteOptions &options, const Key &key) override;

    // Get value from AppKvStore by its key. Set options->local to true if you want to get from local kvstore.
    // Parameters:
    //     options: mark we get from local store or remote store. options->batch is a reserved parameter and should
    //              always be false.
    //     key: key of this entry.
    //     value: value will be returned in this parameter.
    // Return:
    //     Status of this get operation.
    Status Get(const ReadOptions &options, const Key &key, Value &value) override;

    // Get all entries in this store which key start with prefixKey. This function will always get from synced store.
    // Parameters:
    //     prefixkey: the prefix to be searched.
    //     entries: entries will be returned in this parameter.
    // Return:
    //     Status of this GetEntries operation.
    Status GetEntries(const Key &prefixKey, std::vector<Entry> &entries) override;

    // Get all entries in this store which key start with prefixKey. This function will always get from synced store.
    // Parameters:
    //     prefixkey: the prefix to be searched.
    //     resultSet: resultSet will be returned in this parameter.
    // Return:
    //     Status of this GetEntries operation.
    Status GetEntries(const Key &prefixKey, AppKvStoreResultSet *&resultSet) override;

    // Close the result set returned by GetEntries().
    // Parameters:
    //     resultSet: resultSet will be returned in this parameter.
    // Return:
    //     Status of this GetEntries operation.
    Status CloseResultSet(AppKvStoreResultSet *&resultSet) override;

    // Sync store with other devices. This is an asynchronous method,
    // sync will fail if there is a syncing operation in progress.
    // Parameters:
    //     deviceIdList: device list to sync.
    //     mode: mode can be set to SyncMode::PUSH, SyncMode::PULL and SyncMode::PUTH_PULL. PUSH_PULL will firstly
    //           push all not-local store to listed devices, then pull these stores back.
    //     callback: return <device-id, sync-result> map to caller.
    // Return:
    //     Status of this Sync operation.
    Status Sync(const std::vector<std::string> &deviceIdList, const SyncMode &mode,
                const std::function<void(const std::map<std::string, Status> &)> &callback) override;

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
    Status SubscribeKvStore(const ReadOptions &options, const SubscribeType &subscribeType,
                            AppKvStoreObserver *observer) override;

    // Unregister a kvstore to an observer.
    // Parameters:
    //     options: mark this is a local entry or not.
    //     subscribeType: OBSERVER_CHANGES_NATIVE means native changes of syncable kv store,
    //                  : OBSERVER_CHANGES_FOREIGN means synced data changes from remote devices,
    //                  : OBSERVER_CHANGES_LOCAL_ONLY means local changes of local kv store.
    //     observer: observer to unsubscribe this store.
    // Return:
    //     Status of this unsubscribe operation.
    Status UnSubscribeKvStore(const ReadOptions &options, const SubscribeType &subscribeType,
                              AppKvStoreObserver *observer) override;

    // Close this kvstore in KvStoreDelegateManager. This method is called before this store object destruct.
    Status Close(DistributedDB::KvStoreDelegateManager *kvStoreDelegateManager);

    // Remove Devvice data when device offline.
    // Parameters:
    //     device: device id.
    // Return:
    //     Status of this remove operation.
    Status RemoveDeviceData(const std::string &device) override;

    // Set policy of conflict resolution.
    // Parameters:
    //     appConflictPolicyType: include CONFLICT_FOREIGN_KEY_ONLY  CONFLICT_FOREIGN_KEY_ORIG  CONFLICT_NATIVE_ALL.
    //     callback: conflict resolution callback.
    // Return:
    //     Status of Setting policy operation.
    Status SetConflictResolutionPolicy(AppKvStoreConflictPolicyType appConflictPolicyType,
                                       std::function<void(const AppKvStoreConflictData &data)> callback) override;

    Status Export(const std::string &filePath, const std::vector<uint8_t> &passwd) override;

    Status Import(const std::string &filePath, const std::vector<uint8_t> &passwd) override;

    Status GetSecurityLevel(SecurityLevel &securityLevel) const override;

    static Status ConvertErrorCode(DistributedDB::DBStatus status);
private:
    void FormKvStoreConflictEntry(const DistributedDB::KvStoreNbConflictData &data,
                                  KvStoreConflictEntry &kvstoreConflictEntry);
    void FormKvStoreConflictData(const DistributedDB::KvStoreNbConflictData &data,
                                 DistributedDB::KvStoreNbConflictData::ValueType type,
                                 KvStoreConflictData &kvStoreConflictData);
    // user account get from User Account System.
    std::string userId_;
    // appId get from PMS.
    std::string appId_;
    // kvstore name.
    std::string storeId_;
    std::mutex syncStatus_ {};

    // distributeddb is responsible for free kvStoreNbDelegate_,
    // (destruct will be done while calling CloseKvStore in KvStoreDelegateManager)
    // so DO NOT free it in AppKvStoreImpl's destructor.
    DistributedDB::KvStoreNbDelegate *kvStoreNbDelegate_ = nullptr;

    std::map<AppKvStoreObserver *, KvStoreObserverNbImpl *> syncedObserverMap_ {};
    std::mutex syncedObserverMapMutex_ {};
    std::map<AppKvStoreObserver *, KvStoreObserverNbImpl *> localObserverMap_ {};
    std::mutex localObserverMapMutex_ {};
};
}  // namespace AppDistributedKv
}  // namespace OHOS
#endif  // APP_KV_STORE_IMPL_H
