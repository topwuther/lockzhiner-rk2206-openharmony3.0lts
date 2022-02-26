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

#ifndef KV_STORE_NB_CONFLICT_DATA_IMPL_H
#define KV_STORE_NB_CONFLICT_DATA_IMPL_H

#include "db_types.h"
#include "kvdb_conflict_entry.h"
#include "kv_store_nb_conflict_data.h"

namespace DistributedDB {
class KvStoreNbConflictDataImpl final : public KvStoreNbConflictData {
public:
    KvStoreNbConflictDataImpl();
    ~KvStoreNbConflictDataImpl();

    KvStoreNbConflictType GetType() const override;

    void GetKey(Key &key) const override;

    DBStatus GetValue(ValueType type, Value &value) const override;

    bool IsDeleted(ValueType type) const override;

    bool IsNative(ValueType type) const override;

    void SetConflictData(const KvDBConflictEntry &conflictData);

private:
    KvDBConflictEntry data_;
};
} // namespace DistributedDB

#endif // KV_STORE_NB_CONFLICT_DATA_IMPL_H
