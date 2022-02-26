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

#ifndef KV_STORE_SNAPSHOT_DELEGATE_H
#define KV_STORE_SNAPSHOT_DELEGATE_H

#include <functional>
#include <vector>

#include "types.h"

namespace DistributedDB {
class KvStoreSnapshotDelegate {
public:
    DB_API virtual ~KvStoreSnapshotDelegate() {}

    // Get a value from the snapshot with the given key.
    // The return value is DBStatus and Value, these values will be passed to the callback.
    DB_API virtual void Get(const Key &key, const std::function<void(DBStatus, const Value &)> &callback) const = 0;

    // Get entries from the snapshot  which keys start with keyPrefix.
    // The return value is DBStatus and Value, these values will be passed to the callback.
    DB_API virtual void GetEntries(const Key &keyPrefix,
        const std::function<void(DBStatus, const std::vector<Entry> &)> &callback) const = 0;
};
} // namespace DistributedDB

#endif // KV_STORE_SNAPSHOT_DELEGATE_H