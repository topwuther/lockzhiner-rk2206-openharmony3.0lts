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

#include "kvdb_commit_notify_filterable_data.h"
#include "db_errno.h"

namespace DistributedDB {
KvDBCommitNotifyFilterAbleData::KvDBCommitNotifyFilterAbleData()
    : genericKvDB_(nullptr),
      notifyID_(0)
{}

KvDBCommitNotifyFilterAbleData::~KvDBCommitNotifyFilterAbleData()
{
    if (genericKvDB_ != nullptr) {
        genericKvDB_->DecObjRef(genericKvDB_);
        genericKvDB_ = nullptr;
    }
}

const std::list<Entry> KvDBCommitNotifyFilterAbleData::GetInsertedEntries(int &errCode) const
{
    std::list<Entry> entries;
    errCode = E_OK;
    return entries;
}

const std::list<Entry> KvDBCommitNotifyFilterAbleData::GetUpdatedEntries(int &errCode) const
{
    std::list<Entry> entries;
    errCode = E_OK;
    return entries;
}

const std::list<Entry> KvDBCommitNotifyFilterAbleData::GetDeletedEntries(int &errCode) const
{
    std::list<Entry> entries;
    errCode = E_OK;
    return entries;
}

const std::list<KvDBConflictEntry> KvDBCommitNotifyFilterAbleData::GetCommitConflicts(int &errCode) const
{
    std::list<KvDBConflictEntry> entries;
    errCode = E_OK;
    return entries;
}

bool KvDBCommitNotifyFilterAbleData::IsCleared() const
{
    return false;
}

bool KvDBCommitNotifyFilterAbleData::IsChangedDataEmpty() const
{
    return true;
}

bool KvDBCommitNotifyFilterAbleData::IsConflictedDataEmpty() const
{
    return true;
}

void KvDBCommitNotifyFilterAbleData::SetFilterKey(const Key &key)
{
    return;
}

void KvDBCommitNotifyFilterAbleData::SetMyDb(GenericKvDB *db, uint64_t notifyID)
{
    if (genericKvDB_ == db) {
        notifyID_ = notifyID;
        return;
    }
    if (genericKvDB_ != nullptr) {
        genericKvDB_->DecObjRef(genericKvDB_);
    }
    genericKvDB_ = db;
    if (genericKvDB_ != nullptr) {
        genericKvDB_->IncObjRef(genericKvDB_);
    }
    notifyID_ = notifyID;
}

uint64_t KvDBCommitNotifyFilterAbleData::GetNotifyID() const
{
    return notifyID_;
}

DEFINE_OBJECT_TAG_FACILITIES(KvDBCommitNotifyFilterAbleData)
}
