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

#ifndef KVSTORE_IMPL_H
#define KVSTORE_IMPL_H

#include <set>
#include <map>
#include <memory>
#include <shared_mutex>
#include "ikvstore.h"
#include "ikvstore_observer.h"
#include "ikvstore_snapshot.h"
#include "kv_store_delegate.h"
#include "kv_store_delegate_manager.h"
#include "kvstore_observer_impl.h"
#include "kvstore_snapshot_impl.h"
#include "types.h"
#include "inner_types.h"

namespace OHOS {
namespace DistributedKv {
#define IMPORT_DATABASE(bundleName) (Import(bundleName) ? Status::RECOVER_SUCCESS : Status::RECOVER_FAILED)

struct KvStoreObserverImplPtrCompare {
    bool operator()(const KvStoreObserverImpl *lhs, const KvStoreObserverImpl *rhs) const
    {
        if (lhs == rhs || rhs == nullptr) {
            return false;
        }
        if (lhs == nullptr) {
            return true;
        }
        return lhs->GetKvStoreObserverProxy()->AsObject().GetRefPtr() <
               rhs->GetKvStoreObserverProxy()->AsObject().GetRefPtr();
    }
};

class KvStoreImpl : public KvStoreImplStub {
public:
    KvStoreImpl(const Options &options, const std::string &deviceAccountId,
                const std::string &bundleName, const std::string &storeId,
                const std::string &appDirectory, DistributedDB::KvStoreDelegate *kvStoreDelegate);

    void GetKvStoreSnapshot(sptr<IKvStoreObserver> observer,
                            std::function<void(Status, sptr<IKvStoreSnapshotImpl>)> callback) override;

    Status ReleaseKvStoreSnapshot(sptr<IKvStoreSnapshotImpl> iKvStoreSnapshot) override;

    Status Put(const Key &key, const Value &value) override;

    Status PutBatch(const std::vector<Entry> &entries) override;

    Status Delete(const Key &key) override;

    Status DeleteBatch(const std::vector<Key> &keys) override;

    Status Clear() override;

    Status StartTransaction() override;

    Status Commit() override;

    Status Rollback() override;

    /* subscribe kv store */
    Status SubscribeKvStore(const SubscribeType subscribeType, sptr<IKvStoreObserver> observer) override;

    /* unsubscribe kv store */
    Status UnSubscribeKvStore(const SubscribeType subscribeType, sptr<IKvStoreObserver> observer) override;

    virtual const std::string GetStorePath();

    virtual ~KvStoreImpl();

    InnerStatus Close(DistributedDB::KvStoreDelegateManager *kvStoreDelegateManager);

    Status ForceClose(DistributedDB::KvStoreDelegateManager *kvStoreDelegateManager);

    Status MigrateKvStore(const std::string &harmonyAccountId,
                          const std::string &kvStoreDataDir,
                          DistributedDB::KvStoreDelegateManager *oldDelegateMgr,
                          DistributedDB::KvStoreDelegateManager *&newDelegateMgr);

    void IncreaseOpenCount();

    Status ReKey(const std::vector<uint8_t> &key);

    bool Import(const std::string &bundleName) const;
private:
    Status RebuildKvStoreObserver(DistributedDB::KvStoreDelegate *kvStoreDelegate);

    Status RebuildKvStoreSnapshot(DistributedDB::KvStoreDelegate *kvStoreDelegate);

    // kvstore options
    const Options options_;

    // device account id
    std::string deviceAccountId_;

    // appId get from PMS.
    const std::string bundleName_;

    // kvstore name.
    const std::string storeId_;

    // kvstore absolute path in distributeddatamgr.
    const std::string storePath_;

    // distributeddb is responsible for free kvStoreDelegate_,
    // by calling CloseKvStore in KvStoreAppManager,
    // can not free it in KvStoreImpl's destructor.
    mutable std::shared_mutex storeDelegateMutex_{};
    DistributedDB::KvStoreDelegate *kvStoreDelegate_;
    std::mutex storeObserverMutex_;
    std::set<KvStoreObserverImpl *, KvStoreObserverImplPtrCompare> observerSet_;
    std::mutex storeSnapshotMutex_;
    std::map<KvStoreSnapshotImpl *, sptr<IKvStoreSnapshotImpl>> snapshotMap_;
    int openCount_;
};
}  // namespace DistributedKv
}  // namespace OHOS
#endif  // KVSTORE_IMPL_H
