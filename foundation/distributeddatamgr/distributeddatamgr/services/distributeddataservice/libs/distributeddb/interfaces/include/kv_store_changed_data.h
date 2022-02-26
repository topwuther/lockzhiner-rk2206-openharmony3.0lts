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

#ifndef KV_STORE_CHANGED_DATA_H
#define KV_STORE_CHANGED_DATA_H

#include <list>
#include "types.h"

namespace DistributedDB {
class KvStoreChangedData {
public:
    KvStoreChangedData() {}
    DB_API virtual ~KvStoreChangedData() {}

    // Interface for Getting the inserted, updated, delete entries.
    DB_API virtual const std::list<Entry> &GetEntriesInserted() const = 0;

    DB_API virtual const std::list<Entry> &GetEntriesUpdated() const = 0;

    DB_API virtual const std::list<Entry> &GetEntriesDeleted() const = 0;

    DB_API virtual bool IsCleared() const = 0;
};
} // namespace DistributedDB

#endif // KV_STORE_CHANGED_DATA
