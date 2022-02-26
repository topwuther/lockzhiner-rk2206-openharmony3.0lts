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

#ifndef KV_STORE_NB_CONFLICT_DATA_H
#define KV_STORE_NB_CONFLICT_DATA_H

#include "types.h"

namespace DistributedDB {
enum KvStoreNbConflictType {
    CONFLICT_FOREIGN_KEY_ONLY = 0x01, // sync conflict for same origin dev
    CONFIICT_FOREIGN_KEY_ONLY = CONFLICT_FOREIGN_KEY_ONLY, // sync conflict for same origin dev(compatible for mistake)
    CONFLICT_FOREIGN_KEY_ORIG = 0x02, // sync conflict for different origin dev
    CONFLICT_NATIVE_ALL = 0x0c,       // native conflict.
};

class KvStoreNbConflictData {
public:
    enum class ValueType {
        OLD_VALUE = 0,
        NEW_VALUE,
    };

    DB_API virtual ~KvStoreNbConflictData() {};

    DB_API virtual KvStoreNbConflictType GetType() const = 0;

    DB_API virtual void GetKey(Key &key) const = 0;

    DB_API virtual DBStatus GetValue(ValueType type, Value &value) const = 0;

    DB_API virtual bool IsDeleted(ValueType type) const = 0;

    DB_API virtual bool IsNative(ValueType type) const = 0;
};
} // namespace DistributedDB

#endif // KV_STORE_NB_CONFLICT_DATA_H
