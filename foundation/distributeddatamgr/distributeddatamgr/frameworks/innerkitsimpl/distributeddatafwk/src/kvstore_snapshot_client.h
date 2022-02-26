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

#ifndef KVSTORE_SNAPSHOT_CLIENT_H
#define KVSTORE_SNAPSHOT_CLIENT_H

#include "ikvstore_snapshot.h"
#include "kvstore_service_death_notifier.h"
#include "kvstore_snapshot.h"
#include "types.h"

namespace OHOS {
namespace DistributedKv {
class KvStoreSnapshotClient final : public KvStoreSnapshot {
public:
    KvStoreSnapshotClient();

    explicit KvStoreSnapshotClient(sptr<IKvStoreSnapshotImpl> kvStoreSnapshotProxy);

    ~KvStoreSnapshotClient();

    void GetEntries(const Key &prefixKey, const Key &nextKey,
                    std::function<void(Status, std::vector<Entry> &, const Key &)> callback) override;

    void GetEntries(const Key &prefixKey, std::function<void(Status, std::vector<Entry> &)> callback) override;

    void GetKeys(const Key &prefixKey, const Key &nextKey,
                 std::function<void(Status, std::vector<Key> &, const Key &)> callback) override;

    void GetKeys(const Key &prefixKey, std::function<void(Status, std::vector<Key> &)> callback) override;

    Status Get(const Key &key, Value &value) override;

    sptr<IKvStoreSnapshotImpl> GetkvStoreSnapshotProxy();
private:
    // use shared_ptr here to free pointer when reference count is 0.
    sptr<IKvStoreSnapshotImpl> kvStoreSnapshotProxy_;
};
}  // namespace DistributedKv
}  // namespace OHOS
#endif  // KVSTORE_SNAPSHOT_CLIENT_H
