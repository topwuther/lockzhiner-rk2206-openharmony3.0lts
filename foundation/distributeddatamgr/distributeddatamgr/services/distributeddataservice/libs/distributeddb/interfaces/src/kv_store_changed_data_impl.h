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

#ifndef KV_STORE_CHANGED_DATA_IMPL_H
#define KV_STORE_CHANGED_DATA_IMPL_H

#include <mutex>

#include "kv_store_changed_data.h"
#include "kvdb_commit_notify_data.h"

namespace DistributedDB {
class KvStoreChangedDataImpl : public KvStoreChangedData {
public:
    explicit KvStoreChangedDataImpl(const KvDBCommitNotifyData *observerData) : observerData_(observerData) {}
    virtual ~KvStoreChangedDataImpl();

    DISABLE_COPY_ASSIGN_MOVE(KvStoreChangedDataImpl);

    const std::list<Entry> &GetEntriesInserted() const override;

    const std::list<Entry> &GetEntriesUpdated() const override;

    const std::list<Entry> &GetEntriesDeleted() const override;

    bool IsCleared() const override;

private:
    const KvDBCommitNotifyData *observerData_;
    mutable std::mutex mutex_;
    mutable std::list<Entry> insertedEntries_;
    mutable std::list<Entry> updatedEntries_;
    mutable std::list<Entry> deletedEntries_;
};
} // namespace DistributedDB

#endif // KV_STORE_CHANGED_DATA_IMPL_H

