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

#define LOG_TAG "AppKvStoreConflictDataImpl"

#include "app_kvstore_conflict_data_impl.h"
#include "log_print.h"

namespace OHOS {
namespace AppDistributedKv {
AppKvStoreConflictDataImpl::AppKvStoreConflictDataImpl(const KvStoreConflictEntry &kvStoreConflictEntry)
    : kvStoreConflictEntry_(kvStoreConflictEntry)
{
    ZLOGI("constructor");
}

AppKvStoreConflictDataImpl::~AppKvStoreConflictDataImpl()
{
    ZLOGI("destructor");
}

AppKvStoreConflictPolicyType AppKvStoreConflictDataImpl::GetType() const
{
    return static_cast<AppKvStoreConflictPolicyType>(kvStoreConflictEntry_.type);
}

void AppKvStoreConflictDataImpl::GetKey(Key &key) const
{
    key = kvStoreConflictEntry_.key;
}

Status AppKvStoreConflictDataImpl::GetValue(ConflictValueType type, Value &value) const
{
    if (type == ConflictValueType::OLD_VALUE) {
        if (!kvStoreConflictEntry_.oldData.isDeleted) {
            value = kvStoreConflictEntry_.oldData.value;
        }
        return kvStoreConflictEntry_.oldData.status;
    } else {
        if (!kvStoreConflictEntry_.newData.isDeleted) {
            value = kvStoreConflictEntry_.newData.value;
        }
        return kvStoreConflictEntry_.newData.status;
    }
}

bool AppKvStoreConflictDataImpl::IsDeleted(ConflictValueType type) const
{
    if (type == ConflictValueType::OLD_VALUE) {
        return kvStoreConflictEntry_.oldData.isDeleted;
    }
    return kvStoreConflictEntry_.newData.isDeleted;
}

bool AppKvStoreConflictDataImpl::IsNative(ConflictValueType type) const
{
    if (type == ConflictValueType::OLD_VALUE) {
        return kvStoreConflictEntry_.oldData.isLocal;
    } else {
        return kvStoreConflictEntry_.newData.isLocal;
    }
}
}  // namespace AppDistributedKv
}  // namespace OHOS
