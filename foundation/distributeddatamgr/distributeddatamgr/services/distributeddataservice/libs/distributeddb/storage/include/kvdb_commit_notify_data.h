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

#ifndef KVDB_COMMIT_NOTIFY_DATA_H
#define KVDB_COMMIT_NOTIFY_DATA_H

#include <list>

#include "db_types.h"
#include "ref_object.h"
#include "macro_utils.h"
#include "kvdb_conflict_entry.h"

namespace DistributedDB {
// Data from local commit or syncer commit.
class KvDBCommitNotifyData : public RefObject {
public:
    KvDBCommitNotifyData() = default;
    virtual ~KvDBCommitNotifyData() {}
    DISABLE_COPY_ASSIGN_MOVE(KvDBCommitNotifyData);

    // get the new inserted entries.
    virtual const std::list<Entry> GetInsertedEntries(int &errCode) const = 0;

    // get the new updated entries.
    virtual const std::list<Entry> GetUpdatedEntries(int &errCode) const = 0;

    // get the new deleted entries.
    virtual const std::list<Entry> GetDeletedEntries(int &errCode) const = 0;

    // get all conflict entries when commit.
    virtual const std::list<KvDBConflictEntry> GetCommitConflicts(int &errCode) const = 0;

    // database is cleared by user in the commit.
    virtual bool IsCleared() const = 0;

    // test if the inserted/updated/deleted data is empty or not.
    virtual bool IsChangedDataEmpty() const = 0;

    // test if the conflict data is empty or not.
    virtual bool IsConflictedDataEmpty() const = 0;
};
} // namespace DistributedDB

#endif // KVDB_COMMIT_NOTIFY_DATA_H
