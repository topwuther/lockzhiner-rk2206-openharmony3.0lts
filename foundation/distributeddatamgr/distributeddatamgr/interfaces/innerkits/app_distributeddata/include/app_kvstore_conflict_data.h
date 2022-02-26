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

#ifndef APP_KVSTOR_CONFLICT_DATA_H
#define APP_KVSTOR_CONFLICT_DATA_H

#include <time.h>
#include <list>
#include "app_types.h"

namespace OHOS {
namespace AppDistributedKv {
class AppKvStoreConflictData {
public:
    enum class ConflictValueType {
        OLD_VALUE = 0,
        NEW_VALUE,
    };

    KVSTORE_API virtual ~AppKvStoreConflictData() = default;

    KVSTORE_API virtual AppKvStoreConflictPolicyType GetType() const = 0;

    KVSTORE_API virtual void GetKey(Key &key) const = 0;

    KVSTORE_API virtual Status GetValue(ConflictValueType type, Value &value) const = 0;

    KVSTORE_API virtual bool IsDeleted(ConflictValueType type) const = 0;

    KVSTORE_API virtual bool IsNative(ConflictValueType type) const = 0;
};
}  // namespace AppDistributedKv
}  // namespace OHOS

#endif  // APP_KVSTOR_CONFLICT_DATA_H
