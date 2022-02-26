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

#include "single_ver_natural_store_commit_notify_data.h"
#include "db_errno.h"
#include "log_print.h"
#include "db_common.h"

namespace DistributedDB {
SingleVerNaturalStoreCommitNotifyData::SingleVerNaturalStoreCommitNotifyData() : conflictedFlag_(0) {}

const std::list<Entry> SingleVerNaturalStoreCommitNotifyData::GetInsertedEntries(int &errCode) const
{
    return FilterEntriesByKey(insertedEntries_, keyFilter_, errCode);
}

const std::list<Entry> SingleVerNaturalStoreCommitNotifyData::GetUpdatedEntries(int &errCode) const
{
    return FilterEntriesByKey(updatedEntries_, keyFilter_, errCode);
}

const std::list<Entry> SingleVerNaturalStoreCommitNotifyData::GetDeletedEntries(int &errCode) const
{
    return FilterEntriesByKey(deletedEntries_, keyFilter_, errCode);
}

const std::list<KvDBConflictEntry> SingleVerNaturalStoreCommitNotifyData::GetCommitConflicts(int &errCode) const
{
    errCode = E_OK;
    return conflictedEntries_;
}

void SingleVerNaturalStoreCommitNotifyData::SetFilterKey(const Key &key)
{
    keyFilter_ = key;
    return;
}

bool SingleVerNaturalStoreCommitNotifyData::IsChangedDataEmpty() const
{
    int errCode;
    return (!IsCleared() && GetInsertedEntries(errCode).empty() && GetUpdatedEntries(errCode).empty() &&
        GetDeletedEntries(errCode).empty());
}

bool SingleVerNaturalStoreCommitNotifyData::IsConflictedDataEmpty() const
{
    return conflictedEntries_.empty();
}

int SingleVerNaturalStoreCommitNotifyData::InsertCommittedData(const Entry &entry, DataType dataType, bool needMerge)
{
    if (!needMerge) {
        return InsertEntry(dataType, entry);
    }

    Key hashKey;
    DBCommon::CalcValueHash(entry.key, hashKey);
    // conclude the operation type
    if (!IsKeyPropSet(hashKey)) {
        return E_OK;
    }
    DataType type = DataType::NONE;
    if (keyPropRecord_[hashKey].existStatus == ExistStatus::EXIST) {
        if (dataType == DataType::INSERT || dataType == DataType::UPDATE) {
            type = DataType::UPDATE;
        } else if (dataType == DataType::DELETE) {
            type = DataType::DELETE;
        }
    } else {
        if (dataType == DataType::INSERT || dataType == DataType::UPDATE) {
            type = DataType::INSERT;
        } else if (dataType == DataType::DELETE) {
            type = DataType::NONE;
        }
    }

    // clear the old data
    DeleteEntryByKey(entry.key, keyPropRecord_[hashKey].latestType);

    // update the latest operation type value
    keyPropRecord_[hashKey].latestType = type;

    return InsertEntry(type, entry);
}

int SingleVerNaturalStoreCommitNotifyData::InsertEntry(DataType dataType, const Entry &entry)
{
    if (dataType == DataType::INSERT) {
        insertedEntries_.push_back(entry);
    } else if (dataType == DataType::UPDATE) {
        updatedEntries_.push_back(entry);
    } else if (dataType == DataType::DELETE) {
        deletedEntries_.push_back(entry);
    }
    return E_OK;
}

int SingleVerNaturalStoreCommitNotifyData::InsertConflictedItem(const DataItemInfo &itemInfo, bool isOriginal)
{
    Key hashKey;
    DBCommon::CalcValueHash(itemInfo.dataItem.key, hashKey);
    if (!IsKeyPropSet(hashKey)) {
        LOGE("key property not set.");
        return E_OK;
    }
    // key not exist in db
    if (keyPropRecord_[hashKey].existStatus == ExistStatus::NONE) {
        return E_OK;
    }

    auto iter = orgDataItem_.find(itemInfo.dataItem.key);
    if (iter == orgDataItem_.end()) {
        if (isOriginal) {
            orgDataItem_[itemInfo.dataItem.key] = itemInfo;
        }
        return E_OK;
    }
    if (!isOriginal) {
        PutIntoConflictData(iter->second, itemInfo);
    }

    return E_OK;
}

const std::list<Entry> SingleVerNaturalStoreCommitNotifyData::FilterEntriesByKey(
    const std::list<Entry> &entries, const Key &filterKey, int &errCode)
{
    errCode = E_OK;
    if (filterKey.size() == 0) {
        return entries;
    }
    std::list<Entry> filterEntries;
    for (const auto &entry : entries) {
        if (entry.key == filterKey) {
            filterEntries.push_back(entry);
        }
    }
    return filterEntries;
}

void SingleVerNaturalStoreCommitNotifyData::DeleteEntry(const Key &key, std::list<Entry> &entries) const
{
    if (entries.empty()) {
        return;
    }
    entries.remove_if([&key](const Entry &entry) {
        return entry.key == key;
    });
}

void SingleVerNaturalStoreCommitNotifyData::DeleteEntryByKey(const Key &key, DataType type)
{
    if (type == DataType::INSERT) {
        DeleteEntry(key, insertedEntries_);
    }

    if (type == DataType::UPDATE) {
        DeleteEntry(key, updatedEntries_);
    }

    if (type == DataType::DELETE) {
        DeleteEntry(key, deletedEntries_);
    }
}

void SingleVerNaturalStoreCommitNotifyData::InitKeyPropRecord(const Key &key, ExistStatus status)
{
    // check if key status set before, we can only set key status at the first time
    if (IsKeyPropSet(key)) {
        return;
    }

    keyPropRecord_[key].existStatus = status;
}

void SingleVerNaturalStoreCommitNotifyData::SetConflictedNotifiedFlag(int conflictedFlag)
{
    conflictedFlag_ = conflictedFlag;
}

int SingleVerNaturalStoreCommitNotifyData::GetConflictedNotifiedFlag() const
{
    return conflictedFlag_;
}

bool SingleVerNaturalStoreCommitNotifyData::IsConflictedNotifyMatched(const DataItem &itemPut,
    const DataItem &itemOri) const
{
    int dataConflictedType = 0;
    // Local put
    if ((itemPut.flag & DataItem::LOCAL_FLAG) != 0) {
        dataConflictedType = SINGLE_VER_CONFLICT_NATIVE_ALL;
    } else {
        // Compare the origin device of the get and put item.
        if (itemPut.origDev != itemOri.origDev) {
            dataConflictedType = SINGLE_VER_CONFLICT_FOREIGN_KEY_ORIG;
        } else {
            dataConflictedType = SINGLE_VER_CONFLICT_FOREIGN_KEY_ONLY;
        }
    }

    int conflictedFlag = GetConflictedNotifiedFlag();
    LOGD("flag bind kvdb is %d, current data conflicted flag is %d", conflictedFlag, dataConflictedType);
    return (static_cast<uint32_t>(conflictedFlag) & static_cast<uint32_t>(dataConflictedType)) != 0;
}

void SingleVerNaturalStoreCommitNotifyData::PutIntoConflictData(const DataItemInfo &orgItemInfo,
    const DataItemInfo &newItemInfo)
{
    if (orgItemInfo.dataItem.value == newItemInfo.dataItem.value &&
        orgItemInfo.dataItem.origDev == newItemInfo.dataItem.origDev &&
        orgItemInfo.dataItem.flag == newItemInfo.dataItem.flag &&
        orgItemInfo.deviceName == newItemInfo.deviceName) {
        LOGW("same data no need to put.");
        return;
    }

    KvDBConflictEntry conflictData;
    // Local put
    if (newItemInfo.isLocal) {
        conflictData.type = SingleVerNaturalStoreCommitNotifyData::SINGLE_VER_CONFLICT_NATIVE_ALL;
    } else {
        // Compare the origin device of the get and put item.
        conflictData.type = ((newItemInfo.dataItem.origDev != orgItemInfo.dataItem.origDev) ?
            SingleVerNaturalStoreCommitNotifyData::SINGLE_VER_CONFLICT_FOREIGN_KEY_ORIG :
            SingleVerNaturalStoreCommitNotifyData::SINGLE_VER_CONFLICT_FOREIGN_KEY_ONLY);
    }

    bool isDeleted = ((orgItemInfo.dataItem.flag & DataItem::DELETE_FLAG) == DataItem::DELETE_FLAG);
    conflictData.oldData = {orgItemInfo.dataItem.value, isDeleted, true};

    isDeleted = ((newItemInfo.dataItem.flag & DataItem::DELETE_FLAG) == DataItem::DELETE_FLAG);
    conflictData.newData = {newItemInfo.dataItem.value, isDeleted, newItemInfo.isLocal};

    // If the new item is deleted, just using the key of the old data item.
    // If the items are all deleted, this function should not be executed.
    conflictData.key = isDeleted ? orgItemInfo.dataItem.key : newItemInfo.dataItem.key;
    if (newItemInfo.dataItem.writeTimeStamp <= orgItemInfo.dataItem.writeTimeStamp) {
        std::swap(conflictData.newData, conflictData.oldData);
    }

    DeleteConflictEntry(conflictData.key);
    conflictedEntries_.push_back(std::move(conflictData));
}

void SingleVerNaturalStoreCommitNotifyData::DeleteConflictEntry(const Key &key)
{
    if (conflictedEntries_.empty()) {
        return;
    }
    auto iter = conflictedEntries_.begin();
    for (; iter != conflictedEntries_.end(); ++iter) {
        if (iter->key == key) {
            conflictedEntries_.erase(iter);
            return;
        }
    }
}

bool SingleVerNaturalStoreCommitNotifyData::IsKeyPropSet(const Key &key) const
{
    // check if key status set before
    return (keyPropRecord_.find(key) != keyPropRecord_.end());
}

DEFINE_OBJECT_TAG_FACILITIES(SingleVerNaturalStoreCommitNotifyData)
} // namespace DistributedDB
