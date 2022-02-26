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

#ifndef KVSTORE_H
#define KVSTORE_H

#include "kvstore_observer.h"
#include "kvstore_snapshot.h"
#include "types.h"

namespace OHOS {
namespace DistributedKv {
class KvStore {
public:
    KVSTORE_API KvStore() = default;

    // forbidden copy constructor.
    KvStore(const KvStore &) = delete;
    KvStore &operator=(const KvStore &) = delete;

    KVSTORE_API virtual ~KvStore()
    {}

    // Get kvstore name of this kvstore instance.
    KVSTORE_API virtual StoreId GetStoreId() const = 0;

    // Creates a snapshot of the kvstore, allowing the client app to read a
    // consistent data of the content of the kvstore.
    //
    // If observer is provided, it will receive notifications for changes of the
    // kvstore newer than the resulting snapshot.
    // Parameters:
    // observer: observer for subscribe.
    // callback: including status and KvStoreSnapshot instance returned by this call.
    KVSTORE_API
    virtual void GetKvStoreSnapshot(std::shared_ptr<KvStoreObserver> observer,
                                    std::function<void(Status, std::unique_ptr<KvStoreSnapshot>)> callback) const = 0;

    // Release snapshot created by calling GetKvStoreSnapshot.
    KVSTORE_API virtual Status ReleaseKvStoreSnapshot(std::unique_ptr<KvStoreSnapshot> kvStoreSnapshotPtr) = 0;

    // Mutation operations.
    // Key level operations.
    // Mutations are bundled together into atomic commits. If a transaction is in
    // progress, the list of mutations bundled together is tied to the current
    // transaction. If no transaction is in progress, mutations will be a unique transaction.
    // Put one entry with key-value into kvstore,
    // key length should not be greater than 256, and can not be empty.
    // value size should be less than IPC transport limit, and can not be empty.
    KVSTORE_API virtual Status Put(const Key &key, const Value &value) = 0;

    // see Put, PutBatch put a list of entries to kvstore,
    // all entries will be put in a transaction,
    // if entries contains invalid entry, PutBatch will all fail.
    // entries's size should be less than 128 and memory size must be less than IPC transport limit.
    KVSTORE_API virtual Status PutBatch(const std::vector<Entry> &entries) = 0;

    // delete one entry in the kvstore,
    // delete non-exist key still return KEY NOT FOUND error,
    // key length should not be greater than 256, and can not be empty.
    KVSTORE_API virtual Status Delete(const Key &key) = 0;

    // delete a list of entries in the kvstore,
    // delete key not exist still return success,
    // key length should not be greater than 256, and can not be empty.
    // if keys contains invaid key, all delete will fail.
    // keys memory size should not be greater than IPC transport limit, and can not be empty.
    KVSTORE_API virtual Status DeleteBatch(const std::vector<Key> &keys) = 0;

    // clear all entries in the kvstore.
    // after this call, IsClear function in ChangeNotification in subscription return true.
    KVSTORE_API virtual Status Clear() = 0;

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

    // subscribe kvstore to watch data change in the kvstore,
    // OnChange in he observer will be called when data changed, with all the changed contents.
    // client is responsible for free observer after and only after call UnSubscribeKvStore.
    // otherwise, codes in sdk may use a freed memory and cause unexpected result.
    // Parameters:
    // subscribeType: strategy for this subscribe, default right now.
    // observer: callback client provided, client must implement KvStoreObserver and override OnChange function, when
    // data changed in store, OnChange will called in Observer.
    KVSTORE_API
    virtual Status SubscribeKvStore(SubscribeType subscribeType, std::shared_ptr<KvStoreObserver> observer) = 0;

    // unSubscribe kvstore to un-watch data change in the kvstore,
    // after this call, no message will be received even data change in the kvstore.
    // client is responsible for free observer after and only after call UnSubscribeKvStore.
    // otherwise, codes in sdk may use a freed memory and cause unexpected result.
    // Parameters:
    // subscribeType: strategy for this subscribe, default right now.
    // observer: callback client provided in SubscribeKvStore.
    KVSTORE_API
    virtual Status UnSubscribeKvStore(SubscribeType subscribeType, std::shared_ptr<KvStoreObserver> observer) = 0;
};
}  // namespace DistributedKv
}  // namespace OHOS
#endif  // KVSTORE_H
