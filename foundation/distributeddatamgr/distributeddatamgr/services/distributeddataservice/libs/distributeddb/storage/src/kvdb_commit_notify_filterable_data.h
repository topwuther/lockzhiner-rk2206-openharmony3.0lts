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

#ifndef KVDB_COMMIT_NOTIFY_FILTERABLE_DATA_H
#define KVDB_COMMIT_NOTIFY_FILTERABLE_DATA_H

#include "generic_kvdb.h"
#include "kvdb_conflict_entry.h"
#include "kvdb_commit_notify_data.h"

namespace DistributedDB {
class KvDBCommitNotifyFilterAbleData : public KvDBCommitNotifyData {
public:
    KvDBCommitNotifyFilterAbleData();
    ~KvDBCommitNotifyFilterAbleData() override;
    DISABLE_COPY_ASSIGN_MOVE(KvDBCommitNotifyFilterAbleData);

    // get the new inserted entries.
    const std::list<Entry> GetInsertedEntries(int &errCode) const override;

    // get the new updated entries.
    const std::list<Entry> GetUpdatedEntries(int &errCode) const override;

    // get the new deleted entries.
    const std::list<Entry> GetDeletedEntries(int &errCode) const override;

    // get all conflict entries when commit.
    const std::list<KvDBConflictEntry> GetCommitConflicts(int &errCode) const override;

    // database is cleared by user in the commit.
    bool IsCleared() const override;

    // test if the data is empty or not after filtered.
    bool IsChangedDataEmpty() const override;

    // test if the conflict data is empty or not.
    bool IsConflictedDataEmpty() const override;

    // set the filter key.
    virtual void SetFilterKey(const Key &key);

    // set and ref the db that we belong to.
    void SetMyDb(GenericKvDB *db, uint64_t notifyID);

    // get ID of this notify.
    uint64_t GetNotifyID() const;

private:
    DECLARE_OBJECT_TAG(KvDBCommitNotifyFilterAbleData);

    GenericKvDB *genericKvDB_;
    uint64_t notifyID_;
};
} // namespace DistributedDB

#endif // KVDB_COMMIT_NOTIFY_FILTERABLE_DATA_H