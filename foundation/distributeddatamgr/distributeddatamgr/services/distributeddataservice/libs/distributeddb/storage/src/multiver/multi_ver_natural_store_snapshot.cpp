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

#ifndef OMIT_MULTI_VER
#include "multi_ver_natural_store_snapshot.h"

#include "db_constant.h"
#include "db_errno.h"
#include "log_print.h"
#include "multi_ver_storage_executor.h"

namespace DistributedDB {
MultiVerNaturalStoreSnapshot::MultiVerNaturalStoreSnapshot(StorageExecutor *handle)
    : databaseHandle_(handle)
{}

MultiVerNaturalStoreSnapshot::~MultiVerNaturalStoreSnapshot()
{
    databaseHandle_ = nullptr;
}

int MultiVerNaturalStoreSnapshot::Get(const Key &key, Value &value) const
{
    if (databaseHandle_ == nullptr) {
        return -E_INVALID_DB;
    }
    if (key.empty() || key.size() > DBConstant::MAX_KEY_SIZE) {
        LOGE("[MultiSnapshot] Invalid key[%zu]", key.size());
        return -E_INVALID_ARGS;
    }
    return static_cast<MultiVerStorageExecutor *>(databaseHandle_)->Get(key, value);
}

int MultiVerNaturalStoreSnapshot::GetEntries(const Key &keyPrefix, std::vector<Entry> &entries) const
{
    if (databaseHandle_ == nullptr) {
        return -E_INVALID_DB;
    }
    if (keyPrefix.size() > DBConstant::MAX_KEY_SIZE) {
        LOGE("[MultiSnapshot] Invalid prefix[%zu]", keyPrefix.size());
        return -E_INVALID_ARGS;
    }
    return static_cast<MultiVerStorageExecutor *>(databaseHandle_)->GetEntries(keyPrefix, entries);
}

void MultiVerNaturalStoreSnapshot::Close()
{
    if (databaseHandle_ != nullptr) {
        static_cast<MultiVerStorageExecutor *>(databaseHandle_)->Close();
        databaseHandle_ = nullptr;
    }
}
} // namespace DistributedDB
#endif
