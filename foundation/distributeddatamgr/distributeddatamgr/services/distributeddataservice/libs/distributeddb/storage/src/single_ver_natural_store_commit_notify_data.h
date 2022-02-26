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

#ifndef SINGLE_VER_NATURAL_STORE_COMMIT_NOTIFY_DATA_H
#define SINGLE_VER_NATURAL_STORE_COMMIT_NOTIFY_DATA_H

#include "kvdb_commit_notify_filterable_data.h"

namespace DistributedDB {
enum class DataType {
    NONE,
    INSERT,
    UPDATE,
    DELETE,
};

enum class ExistStatus {
    NONE, // key never exist in db
    DELETED, // key deleted but exist before
    EXIST, // key exist
};
constexpr size_t MAX_TOTAL_NOTIFY_ITEM_SIZE = 1048576; // 1MB
constexpr size_t MAX_TOTAL_NOTIFY_DATA_SIZE = 4195328; // 4MB + 1KB

struct DataItemInfo {
    DataItem dataItem;
    bool isLocal = false;
    std::vector<uint8_t> deviceName;
};

class SingleVerNaturalStoreCommitNotifyData final : public KvDBCommitNotifyFilterAbleData {
public:
    SingleVerNaturalStoreCommitNotifyData();
    ~SingleVerNaturalStoreCommitNotifyData() {}
    DISABLE_COPY_ASSIGN_MOVE(SingleVerNaturalStoreCommitNotifyData);

    const std::list<Entry> GetInsertedEntries(int &errCode) const override;

    const std::list<Entry> GetUpdatedEntries(int &errCode) const override;

    const std::list<Entry> GetDeletedEntries(int &errCode) const override;

    const std::list<KvDBConflictEntry> GetCommitConflicts(int &errCode) const override;

    void SetFilterKey(const Key &key) override;

    bool IsChangedDataEmpty() const override;

    bool IsConflictedDataEmpty() const override;

    int InsertCommittedData(const Entry &entry, DataType dataType, bool needMerge = false);

    int InsertConflictedItem(const DataItemInfo &itemInfo, bool isOriginal = false);

    void InitKeyPropRecord(const Key &key, ExistStatus status);

    void SetConflictedNotifiedFlag(int conflictedFlag);

    int GetConflictedNotifiedFlag() const;

    bool IsConflictedNotifyMatched(const DataItem &itemPut, const DataItem &itemGet) const;

private:

    struct ItemProp {
        ExistStatus existStatus = ExistStatus::NONE; // indicator if the key exist in db before this transaction
        DataType latestType = DataType::NONE; // indicator the latest operation type for this key
    };

    int InsertEntry(DataType dataType, const Entry &entry);

    static const std::list<Entry> FilterEntriesByKey(const std::list<Entry> &entries,
        const Key &filterKey, int &errCode);

    void DeleteEntry(const Key &key, std::list<Entry> &entries) const;

    void DeleteEntryByKey(const Key &key, DataType type);

    void PutIntoConflictData(const DataItemInfo &orgItemInfo, const DataItemInfo &newItemInfo);

    void DeleteConflictEntry(const Key &key);

    bool IsKeyPropSet(const Key &key) const;

    DECLARE_OBJECT_TAG(SingleVerNaturalStoreCommitNotifyData);

    static const int SINGLE_VER_CONFLICT_FOREIGN_KEY_ONLY = 0x01; // sync conflict for same origin dev
    static const int SINGLE_VER_CONFLICT_FOREIGN_KEY_ORIG = 0x02; // sync conflict for different origin dev
    static const int SINGLE_VER_CONFLICT_NATIVE_ALL = 0x0c;       // native conflict.

    std::list<Entry> insertedEntries_;
    std::list<Entry> updatedEntries_;
    std::list<Entry> deletedEntries_;
    std::list<KvDBConflictEntry> conflictedEntries_;
    Key keyFilter_;
    std::map<Key, ItemProp> keyPropRecord_; // hash key mapping to item property
    std::map<Key, DataItemInfo> orgDataItem_;
    int conflictedFlag_; // the conflict notifier type composition, 0 means no conflict notifier.
};
} // namespace DistributedDB

#endif // SINGLE_VER_NATURAL_STORE_COMMIT_NOTIFY_DATA_H