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

#include "kv_store_changed_data_impl.h"

namespace DistributedDB {
KvStoreChangedDataImpl::~KvStoreChangedDataImpl()
{
    observerData_ = nullptr;
}

const std::list<Entry> &KvStoreChangedDataImpl::GetEntriesInserted() const
{
    std::lock_guard<std::mutex> lock(mutex_);
    if (insertedEntries_.empty() && observerData_ != nullptr) {
        int errCode;
        insertedEntries_ = observerData_->GetInsertedEntries(errCode);
    }

    return insertedEntries_;
}

const std::list<Entry> &KvStoreChangedDataImpl::GetEntriesUpdated() const
{
    std::lock_guard<std::mutex> lock(mutex_);
    if (updatedEntries_.empty() && observerData_ != nullptr) {
        int errCode;
        updatedEntries_ = observerData_->GetUpdatedEntries(errCode);
    }

    return updatedEntries_;
}

const std::list<Entry> &KvStoreChangedDataImpl::GetEntriesDeleted() const
{
    std::lock_guard<std::mutex> lock(mutex_);
    if (deletedEntries_.empty() && observerData_ != nullptr) {
        int errCode;
        deletedEntries_ = observerData_->GetDeletedEntries(errCode);
    }

    return deletedEntries_;
}

bool KvStoreChangedDataImpl::IsCleared() const
{
    if (observerData_ != nullptr) {
        return observerData_->IsCleared();
    }

    return false;
}
} // namespace DistributedDB

