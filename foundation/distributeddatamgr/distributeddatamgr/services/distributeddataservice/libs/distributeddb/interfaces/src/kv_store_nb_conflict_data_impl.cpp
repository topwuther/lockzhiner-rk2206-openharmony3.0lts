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

#include "kv_store_nb_conflict_data_impl.h"

namespace DistributedDB {
KvStoreNbConflictDataImpl::KvStoreNbConflictDataImpl() {}

KvStoreNbConflictDataImpl::~KvStoreNbConflictDataImpl() {}

KvStoreNbConflictType KvStoreNbConflictDataImpl::GetType() const
{
    return static_cast<KvStoreNbConflictType>(data_.type);
}

void KvStoreNbConflictDataImpl::GetKey(Key &key) const
{
    key = data_.key;
}

DBStatus KvStoreNbConflictDataImpl::GetValue(ValueType type, Value &value) const
{
    if (IsDeleted(type)) {
        return DB_ERROR;
    }

    if (type == ValueType::OLD_VALUE) {
        value = data_.oldData.value;
    } else {
        value = data_.newData.value;
    }

    return OK;
}

bool KvStoreNbConflictDataImpl::IsDeleted(ValueType type) const
{
    if (type == ValueType::OLD_VALUE) {
        return data_.oldData.isDeleted;
    } else {
        return data_.newData.isDeleted;
    }
}

bool KvStoreNbConflictDataImpl::IsNative(ValueType type) const
{
    if (type == ValueType::OLD_VALUE) {
        return data_.oldData.isLocal;
    } else {
        return data_.newData.isLocal;
    }
}

void KvStoreNbConflictDataImpl::SetConflictData(const KvDBConflictEntry &conflictData)
{
    data_ = conflictData;
}
}
