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

#define LOG_TAG "KvStoreResultsetImpl"

#include "kvstore_resultset_impl.h"
#include <utility>
#include "dds_trace.h"
#include "log_print.h"

namespace OHOS::DistributedKv {
constexpr int KvStoreResultSetImpl::INIT_POSTION;
KvStoreResultSetImpl::~KvStoreResultSetImpl()
{
}

KvStoreResultSetImpl::KvStoreResultSetImpl(DistributedDB::KvStoreResultSet *resultSet)
    : kvStoreResultSet_(resultSet)
{
}

KvStoreResultSetImpl::KvStoreResultSetImpl(DistributedDB::Key keyPrefix, DistributedDB::KvStoreResultSet *resultSet)
    : keyPrefix_(std::move(keyPrefix)), kvStoreResultSet_(resultSet)
{
}

int KvStoreResultSetImpl::GetCount()
{
    DdsTrace trace(std::string(LOG_TAG "::") + std::string(__FUNCTION__));
    std::shared_lock<std::shared_mutex> lock(this->mutex_);
    if (kvStoreResultSet_ == nullptr) {
        return 0;
    }
    return kvStoreResultSet_->GetCount();
}

int KvStoreResultSetImpl::GetPosition()
{
    DdsTrace trace(std::string(LOG_TAG "::") + std::string(__FUNCTION__));

    std::shared_lock<std::shared_mutex> lock(this->mutex_);
    if (kvStoreResultSet_ == nullptr) {
        return INIT_POSTION;
    }
    return kvStoreResultSet_->GetPosition();
}

bool KvStoreResultSetImpl::MoveToFirst()
{
    DdsTrace trace(std::string(LOG_TAG "::") + std::string(__FUNCTION__));

    std::shared_lock<std::shared_mutex> lock(this->mutex_);
    if (kvStoreResultSet_ == nullptr) {
        return false;
    }
    return kvStoreResultSet_->MoveToFirst();
}

bool KvStoreResultSetImpl::MoveToLast()
{
    DdsTrace trace(std::string(LOG_TAG "::") + std::string(__FUNCTION__));

    std::shared_lock<std::shared_mutex> lock(this->mutex_);
    if (kvStoreResultSet_ == nullptr) {
        return false;
    }
    return kvStoreResultSet_->MoveToLast();
}

bool KvStoreResultSetImpl::MoveToNext()
{
    DdsTrace trace(std::string(LOG_TAG "::") + std::string(__FUNCTION__));

    std::shared_lock<std::shared_mutex> lock(this->mutex_);
    if (kvStoreResultSet_ == nullptr) {
        return false;
    }
    return kvStoreResultSet_->MoveToNext();
}

bool KvStoreResultSetImpl::MoveToPrevious()
{
    DdsTrace trace(std::string(LOG_TAG "::") + std::string(__FUNCTION__));

    std::shared_lock<std::shared_mutex> lock(this->mutex_);
    if (kvStoreResultSet_ == nullptr) {
        return false;
    }
    return kvStoreResultSet_->MoveToPrevious();
}

bool KvStoreResultSetImpl::Move(int offset)
{
    DdsTrace trace(std::string(LOG_TAG "::") + std::string(__FUNCTION__));

    std::shared_lock<std::shared_mutex> lock(this->mutex_);
    if (kvStoreResultSet_ == nullptr) {
        return false;
    }
    return kvStoreResultSet_->Move(offset);
}

bool KvStoreResultSetImpl::MoveToPosition(int position)
{
    DdsTrace trace(std::string(LOG_TAG "::") + std::string(__FUNCTION__));

    std::shared_lock<std::shared_mutex> lock(this->mutex_);
    if (kvStoreResultSet_ == nullptr) {
        return false;
    }
    return kvStoreResultSet_->MoveToPosition(position);
}

bool KvStoreResultSetImpl::IsFirst()
{
    std::shared_lock<std::shared_mutex> lock(this->mutex_);
    if (kvStoreResultSet_ == nullptr) {
        return false;
    }
    return kvStoreResultSet_->IsFirst();
}

bool KvStoreResultSetImpl::IsLast()
{
    std::shared_lock<std::shared_mutex> lock(this->mutex_);
    if (kvStoreResultSet_ == nullptr) {
        return false;
    }
    return kvStoreResultSet_->IsLast();
}

bool KvStoreResultSetImpl::IsBeforeFirst()
{
    std::shared_lock<std::shared_mutex> lock(this->mutex_);
    if (kvStoreResultSet_ == nullptr) {
        return false;
    }
    return kvStoreResultSet_->IsBeforeFirst();
}

bool KvStoreResultSetImpl::IsAfterLast()
{
    std::shared_lock<std::shared_mutex> lock(this->mutex_);
    if (kvStoreResultSet_ == nullptr) {
        return false;
    }
    return kvStoreResultSet_->IsAfterLast();
}

Status KvStoreResultSetImpl::GetEntry(Entry &entry)
{
    DdsTrace trace(std::string(LOG_TAG "::") + std::string(__FUNCTION__));

    std::shared_lock<std::shared_mutex> lock(this->mutex_);
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

Status KvStoreResultSetImpl::CloseResultSet(DistributedDB::KvStoreNbDelegate *kvStoreNbDelegate)
{
    if (kvStoreNbDelegate == nullptr) {
        return Status::INVALID_ARGUMENT;
    }
    DdsTrace trace(std::string(LOG_TAG "::") + std::string(__FUNCTION__));
    std::shared_lock<std::shared_mutex> lock(this->mutex_);
    DistributedDB::DBStatus status = kvStoreNbDelegate->CloseResultSet(kvStoreResultSet_);
    if (status != DistributedDB::DBStatus::OK) {
        return Status::DB_ERROR;
    }
    kvStoreResultSet_ = nullptr;
    return Status::SUCCESS;
}

Status KvStoreResultSetImpl::MigrateKvStore(DistributedDB::KvStoreNbDelegate *kvStoreNbDelegate)
{
    if (kvStoreNbDelegate == nullptr) {
        return Status::INVALID_ARGUMENT;
    }

    std::unique_lock<std::shared_mutex> lock(this->mutex_);
    int position = GetPosition();
    DistributedDB::DBStatus dbStatus = kvStoreNbDelegate->CloseResultSet(kvStoreResultSet_);
    if (dbStatus != DistributedDB::DBStatus::OK) {
        ZLOGE("close result set failed.");
        return Status::DB_ERROR;
    }

    DistributedDB::KvStoreResultSet *dbResultSet = nullptr;
    dbStatus = kvStoreNbDelegate->GetEntries(keyPrefix_, dbResultSet);
    if (dbStatus != DistributedDB::DBStatus::OK || dbResultSet == nullptr) {
        ZLOGE("rebuild result set failed during get entries by key prefix.");
        kvStoreResultSet_ = nullptr;
        return Status::DB_ERROR;
    }
    kvStoreResultSet_ = dbResultSet;
    kvStoreResultSet_->MoveToPosition(position);

    return Status::SUCCESS;
}
} // namespace OHOS::DistributedKv