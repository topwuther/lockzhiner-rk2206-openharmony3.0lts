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

#ifndef MULTI_VER_NATURAL_STORE_SNAPSHOT_H
#define MULTI_VER_NATURAL_STORE_SNAPSHOT_H

#ifndef OMIT_MULTI_VER
#include "ikvdb_snapshot.h"
#include "storage_executor.h"

namespace DistributedDB {
class MultiVerNaturalStoreSnapshot : public IKvDBSnapshot {
public:
    explicit MultiVerNaturalStoreSnapshot(StorageExecutor *handle);
    ~MultiVerNaturalStoreSnapshot() override;

    DISABLE_COPY_ASSIGN_MOVE(MultiVerNaturalStoreSnapshot);

    // Get the value according the key in the snapshot
    int Get(const Key &key, Value &value) const override;

    // Get the data according the prefix key in the snapshot
    int GetEntries(const Key &keyPrefix, std::vector<Entry> &entries) const override;

    void Close();

private:
    StorageExecutor *databaseHandle_;
};
} // namespace DistributedDB

#endif  // MULTI_VER_NATURAL_STORE_SNAPSHOT_H
#endif