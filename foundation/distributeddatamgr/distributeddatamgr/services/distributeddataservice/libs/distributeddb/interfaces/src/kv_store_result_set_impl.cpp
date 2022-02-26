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

#include "kv_store_result_set_impl.h"

#include "db_errno.h"

namespace DistributedDB {
const int KvStoreResultSetImpl::INIT_POSTION = -1;

KvStoreResultSetImpl::KvStoreResultSetImpl(IKvDBResultSet *resultSet)
    : resultSet_(resultSet)
{
}

int KvStoreResultSetImpl::GetCount() const
{
    if (resultSet_ == nullptr) {
        return 0;
    }
    return resultSet_->GetCount();
}

int KvStoreResultSetImpl::GetPosition() const
{
    if (resultSet_ == nullptr) {
        return INIT_POSTION;
    }
    return resultSet_->GetPosition();
}

bool KvStoreResultSetImpl::Move(int offset)
{
    long long position = GetPosition();
    long long aimPos = position + offset;
    if (aimPos > INT_MAX) {
        return MoveToPosition(INT_MAX);
    }
    if (aimPos < INIT_POSTION) {
        return MoveToPosition(INIT_POSTION);
    }
    return MoveToPosition(aimPos);
}

bool KvStoreResultSetImpl::MoveToPosition(int position)
{
    if (resultSet_ == nullptr) {
        return false;
    }
    if (resultSet_->MoveTo(position) == E_OK) {
        return true;
    }
    return false;
}

bool KvStoreResultSetImpl::MoveToFirst()
{
    return MoveToPosition(0);
}

bool KvStoreResultSetImpl::MoveToLast()
{
    return MoveToPosition(GetCount() - 1);
}

bool KvStoreResultSetImpl::MoveToNext()
{
    // move 1 step forward in this result set
    return Move(1);
}

bool KvStoreResultSetImpl::MoveToPrevious()
{
    // move 1 step backward in this result set
    return Move(-1);
}

bool KvStoreResultSetImpl::IsFirst() const
{
    if (resultSet_ == nullptr) {
        return false;
    }
    int position = resultSet_->GetPosition();
    if (GetCount() == 0) {
        return false;
    }
    if (position == 0) {
        return true;
    }
    return false;
}

bool KvStoreResultSetImpl::IsLast() const
{
    if (resultSet_ == nullptr) {
        return false;
    }
    int position = resultSet_->GetPosition();
    int count = GetCount();
    if (count == 0) {
        return false;
    }
    if (position == (count - 1)) {
        return true;
    }
    return false;
}

bool KvStoreResultSetImpl::IsBeforeFirst() const
{
    if (resultSet_ == nullptr) {
        return false;
    }
    int position = resultSet_->GetPosition();

    if (GetCount() == 0) {
        return true;
    }
    if (position <= INIT_POSTION) {
        return true;
    }
    return false;
}

bool KvStoreResultSetImpl::IsAfterLast() const
{
    if (resultSet_ == nullptr) {
        return false;
    }
    int position = resultSet_->GetPosition();
    int count = GetCount();
    if (count == 0) {
        return true;
    }
    if (position >= count) {
        return true;
    }
    return false;
}

DBStatus KvStoreResultSetImpl::GetEntry(Entry &entry) const
{
    if (resultSet_ == nullptr) {
        return DB_ERROR;
    }
    if (GetCount() == 0) {
        return NOT_FOUND;
    }

    if (resultSet_->GetEntry(entry) == E_OK) {
        return OK;
    }
    return NOT_FOUND;
}

void KvStoreResultSetImpl::GetResultSet(IKvDBResultSet *&resultSet) const
{
    resultSet = resultSet_;
}
} // namespace DistributedDB
