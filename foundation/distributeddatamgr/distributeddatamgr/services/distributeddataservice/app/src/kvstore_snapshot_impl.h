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

#ifndef KVSTORE_SNAPSHOT_IMPL_H
#define KVSTORE_SNAPSHOT_IMPL_H

#include <map>
#include <string>
#include <vector>
#include <memory>
#include <mutex>
#include <shared_mutex>
#include "kv_store_delegate.h"
#include "ikvstore_snapshot.h"
#include "kv_store_snapshot_delegate.h"
#include "types.h"
#include "kvstore_observer_impl.h"

namespace OHOS {
namespace DistributedKv {

class KvStoreSnapshotImpl : public KvStoreSnapshotImplStub {
public:
    explicit KvStoreSnapshotImpl(DistributedDB::KvStoreSnapshotDelegate *kvStoreSnapshotDelegate,
        KvStoreObserverImpl *kvStoreObserverImpl);

    virtual ~KvStoreSnapshotImpl();

    Status Get(const Key &key, Value &value) override;

    void GetEntries(const Key &prefixKey, const Key &nextKey,
                    std::function<void(Status, std::vector<Entry> &, const Key &)> callback) override;

    void GetKeys(const Key &prefixKey, const Key &nextKey,
                 std::function<void(Status, std::vector<Key> &, const Key &)> callback) override;

    Status Release(DistributedDB::KvStoreDelegate *kvStoreDelegate);

    Status MigrateKvStore(DistributedDB::KvStoreDelegate *kvStoreDelegate);

private:
    // distributeddb is responsible for free kvStoreSnapshotDelegate_,
    // by calling ReleaseKvStoreSnapshot in kvstore,
    // can not free it in KvStoreSnapshotImpl's destructor.
    mutable std::shared_mutex snapshotDelegateMutex_{};
    DistributedDB::KvStoreSnapshotDelegate *kvStoreSnapshotDelegate_;
    KvStoreObserverImpl *kvStoreObserverImpl_;
    // write amplification of each write parcel operation. currently zero.
    static constexpr unsigned int IPC_WRITE_AMPLIFICATION = 0;

    // max size of returned entries or keys. size of the key and IPC_WRITE_AMPLIFICATION of the last element is ignored.
    // IPC limit is 819200 currently. SOFT_LIMIT should be smaller than IPC limit.
    static constexpr unsigned int SOFT_LIMIT = 750000;

    // max size of batchEntries_ and batchKeys_
    static constexpr unsigned int BUFFER_SIZE = 3;

    // temporarily storage entries of an unfinished search.
    std::list<std::pair<std::string, std::list<Entry>>> batchEntries_;
    std::mutex entriesMutex_;
    void GetEntriesFromDelegateLocked(const Key &prefixKey, const Key &nextKey,
                                      std::function<void(Status, std::vector<Entry> &, const Key &)> callback);

    // temporarily storage keys of an unfinished search.
    std::list<std::pair<std::string, std::list<Key>>> batchKeys_;
    std::mutex keysMutex_;
    void GetKeysFromDelegateLocked(const Key &prefixKey, const Key &nextKey,
                                   std::function<void(Status, std::vector<Key> &, const Key &)> callback);
};

}  // namespace DistributedKv
}  // namespace OHOS

#endif  // KVSTORE_SNAPSHOT_IMPL_H
