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

#define LOG_TAG "AppKvStoreResultSetImpl"

#include "app_kvstore_result_set_impl.h"
#include "app_types.h"

namespace OHOS {
namespace AppDistributedKv {
const int AppKvStoreResultSetImpl::INIT_POSTION = -1;

AppKvStoreResultSetImpl::AppKvStoreResultSetImpl(DistributedDB::KvStoreResultSet *resultSet,
                                                 DistributedDB::KvStoreNbDelegate *delegate)
    : kvStoreResultSet_(resultSet), nbDelegate_(delegate)
{
}

AppKvStoreResultSetImpl::~AppKvStoreResultSetImpl()
{
}

// Returns the count of rows in the result set.
int AppKvStoreResultSetImpl::GetCount() const
{
    return (kvStoreResultSet_ == nullptr) ? 0 : kvStoreResultSet_->GetCount();
}

// Returns the current read position of the result set.
int AppKvStoreResultSetImpl::GetPosition() const
{
    return (kvStoreResultSet_ == nullptr) ? INIT_POSTION : kvStoreResultSet_->GetPosition();
}

// Move the read position to the first row, return false if the result set is empty.
bool AppKvStoreResultSetImpl::MoveToFirst()
{
    return (kvStoreResultSet_ == nullptr) ? false : kvStoreResultSet_->MoveToFirst();
}

// Move the read position to the last row, return false if the result set is empty.
bool AppKvStoreResultSetImpl::MoveToLast()
{
    return (kvStoreResultSet_ == nullptr) ? false : kvStoreResultSet_->MoveToLast();
}

// Move the read position to the next row,
// return false if the result set is empty or the read position is already past the last entry in the result set.
bool AppKvStoreResultSetImpl::MoveToNext()
{
    return (kvStoreResultSet_ == nullptr) ? false : kvStoreResultSet_->MoveToNext();
}

// Move the read position to the previous row,
// return false if the result set is empty or the read position is already before the first entry in the result set.
bool AppKvStoreResultSetImpl::MoveToPrevious()
{
    return (kvStoreResultSet_ == nullptr) ? false : kvStoreResultSet_->MoveToPrevious();
}

// Move the read position by a relative amount from the current position.
bool AppKvStoreResultSetImpl::Move(int offset)
{
    return (kvStoreResultSet_ == nullptr) ? false : kvStoreResultSet_->Move(offset);
}

// Move the read position to an absolute position value.
bool AppKvStoreResultSetImpl::MoveToPosition(int position)
{
    return (kvStoreResultSet_ == nullptr) ? false : kvStoreResultSet_->MoveToPosition(position);
}

// Returns whether the read position is pointing to the first row.
bool AppKvStoreResultSetImpl::IsFirst() const
{
    return (kvStoreResultSet_ == nullptr) ? false : kvStoreResultSet_->IsFirst();
}

// Returns whether the read position is pointing to the last row.
bool AppKvStoreResultSetImpl::IsLast() const
{
    return (kvStoreResultSet_ == nullptr) ? false : kvStoreResultSet_->IsLast();
}

// Returns whether the read position is before the first row.
bool AppKvStoreResultSetImpl::IsBeforeFirst() const
{
    return (kvStoreResultSet_ == nullptr) ? false : kvStoreResultSet_->IsBeforeFirst();
}

// Returns whether the read position is after the last row
bool AppKvStoreResultSetImpl::IsAfterLast() const
{
    return (kvStoreResultSet_ == nullptr) ? false : kvStoreResultSet_->IsAfterLast();
}

// Get a key-value entry.
Status AppKvStoreResultSetImpl::GetEntry(Entry &entry) const
{
    if (kvStoreResultSet_ == nullptr) {
        return Status::ERROR;
    }
    if (GetCount() == 0) {
        return Status::KEY_NOT_FOUND;
    }
    DistributedDB::Entry dbEntry;
    DistributedDB::DBStatus dbStatus = kvStoreResultSet_->GetEntry(dbEntry);
    if (dbStatus == DistributedDB::DBStatus::OK) {
        Key tmpKey(dbEntry.key);
        Value tmpValue(dbEntry.value);
        entry.key = tmpKey;
        entry.value = tmpValue;
        return Status::SUCCESS;
    }
    return Status::KEY_NOT_FOUND;
}

Status AppKvStoreResultSetImpl::Close()
{
    if (kvStoreResultSet_ == nullptr) {
        return Status::SUCCESS;
    }

    if (nbDelegate_ == nullptr) {
        return Status::INVALID_ARGUMENT;
    }

    auto result = nbDelegate_->CloseResultSet(kvStoreResultSet_);
    if (result == DistributedDB::DBStatus::OK) {
        return Status::SUCCESS;
    }
    kvStoreResultSet_ = nullptr;
    return Status::ERROR;
}
}  // namespace AppDistributedKv
}  // namespace OHOS
