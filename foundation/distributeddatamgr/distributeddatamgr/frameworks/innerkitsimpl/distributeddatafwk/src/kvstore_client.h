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

#ifndef KVSTORE_CLIENT_H
#define KVSTORE_CLIENT_H

#include <map>
#include "ikvstore.h"
#include "kvstore.h"
#include "kvstore_service_death_notifier.h"
#include "kvstore_snapshot.h"
#include "types.h"

namespace OHOS {
namespace DistributedKv {

class KvStoreClient final : public KvStore {
public:
    explicit KvStoreClient(sptr<IKvStoreImpl> kvStoreProxy, const std::string &storeId);

    ~KvStoreClient();

    StoreId GetStoreId() const override;

    void GetKvStoreSnapshot(std::shared_ptr<KvStoreObserver> observer,
                            std::function<void(Status, std::unique_ptr<KvStoreSnapshot>)> callback) const override;

    Status ReleaseKvStoreSnapshot(std::unique_ptr<KvStoreSnapshot> kvStoreSnapshotPtr) override;

    Status Put(const Key &key, const Value &value) override;

    Status PutBatch(const std::vector<Entry> &entries) override;

    Status Delete(const Key &key) override;

    Status DeleteBatch(const std::vector<Key> &keys) override;

    Status Clear() override;

    Status StartTransaction() override;

    Status Commit() override;

    Status Rollback() override;

    Status SubscribeKvStore(SubscribeType subscribeType, std::shared_ptr<KvStoreObserver> observer) override;

    Status UnSubscribeKvStore(SubscribeType subscribeType, std::shared_ptr<KvStoreObserver> observer) override;

private:
    sptr<IKvStoreImpl> kvStoreProxy_;
    std::map<KvStoreObserver *, sptr<IKvStoreObserver>> registeredObservers_;
    std::mutex observerMapMutex_;
    std::string storeId_;
};

}  // namespace DistributedKv
}  // namespace OHOS

#endif  // KVSTORE_CLIENT_H
