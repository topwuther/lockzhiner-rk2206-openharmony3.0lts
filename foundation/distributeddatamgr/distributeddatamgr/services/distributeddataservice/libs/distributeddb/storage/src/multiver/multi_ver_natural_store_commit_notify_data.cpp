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
#include "multi_ver_natural_store_commit_notify_data.h"
#include "db_errno.h"
#include "log_print.h"

namespace DistributedDB {
MultiVerNaturalStoreCommitNotifyData::MultiVerNaturalStoreCommitNotifyData(MultiVerNaturalStore *db,
    const CommitID &startCommitID, const CommitID &endCommitID, Version curVersion)
    : db_(db),
      startCommitID_(startCommitID),
      endCommitID_(endCommitID),
      isFilled_(false),
      version_(curVersion)
{}

MultiVerNaturalStoreCommitNotifyData::~MultiVerNaturalStoreCommitNotifyData()
{
    if (db_ != nullptr) {
        db_->RemoveVersionConstraintFromList(version_);
    }

    db_ = nullptr;
}

const std::list<Entry> MultiVerNaturalStoreCommitNotifyData::GetInsertedEntries(int &errCode) const
{
    errCode = FillInnerData();
    if (errCode != E_OK) {
        LOGE("Failed to fill inner data in GetInsertedEntries(), err:%d", errCode);
    }
    return diffData_.inserted;
}

const std::list<Entry> MultiVerNaturalStoreCommitNotifyData::GetUpdatedEntries(int &errCode) const
{
    errCode = FillInnerData();
    if (errCode != E_OK) {
        LOGE("Failed to fill inner data in GetUpdatedEntries(), err:%d", errCode);
    }
    return diffData_.updated;
}

const std::list<Entry> MultiVerNaturalStoreCommitNotifyData::GetDeletedEntries(int &errCode) const
{
    errCode = FillInnerData();
    if (errCode != E_OK) {
        LOGE("Failed to fill inner data in GetDeletedEntries(), err:%d", errCode);
    }
    return diffData_.deleted;
}

bool MultiVerNaturalStoreCommitNotifyData::IsCleared() const
{
    int errCode = FillInnerData();
    if (errCode != E_OK) {
        LOGE("Failed to fill inner data in IsCleared(), err:%d", errCode);
    }
    return diffData_.isCleared;
}

bool MultiVerNaturalStoreCommitNotifyData::IsChangedDataEmpty() const
{
    int errCode = FillInnerData();
    if (errCode != E_OK) {
        LOGE("Failed to fill inner data in IsEmpty(), err:%d", errCode);
    }
    return !diffData_.isCleared &&
        diffData_.inserted.empty() &&
        diffData_.updated.empty() &&
        diffData_.deleted.empty();
}

int MultiVerNaturalStoreCommitNotifyData::FillInnerData() const
{
    std::lock_guard<std::mutex> lock(fillMutex_);
    if (isFilled_) {
        return E_OK;
    }
    if (db_ == nullptr) {
        LOGE("Failed to fill inner data, db is nullptr");
        return -E_INVALID_DB;
    }

    int errCode = db_->GetDiffEntries(startCommitID_, endCommitID_, diffData_);
    if (errCode != E_OK) {
        LOGE("Failed to get diff entries when filling inner data, err:%d", errCode);
        return errCode;
    }
    isFilled_ = true;
    return E_OK;
}

DEFINE_OBJECT_TAG_FACILITIES(MultiVerNaturalStoreCommitNotifyData)
}
#endif