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

#ifndef MULTI_VER_NATURAL_STORE_COMMIT_NOTIFY_DATA_H
#define MULTI_VER_NATURAL_STORE_COMMIT_NOTIFY_DATA_H

#ifndef OMIT_MULTI_VER
#include <mutex>

#include "kvdb_commit_notify_filterable_data.h"
#include "multi_ver_natural_store.h"

namespace DistributedDB {
class MultiVerNaturalStoreCommitNotifyData final : public KvDBCommitNotifyFilterAbleData {
public:
    MultiVerNaturalStoreCommitNotifyData(MultiVerNaturalStore *db, const CommitID &startCommitID,
        const CommitID &endCommitID, Version curVersion);
    ~MultiVerNaturalStoreCommitNotifyData();
    DISABLE_COPY_ASSIGN_MOVE(MultiVerNaturalStoreCommitNotifyData);

    const std::list<Entry> GetInsertedEntries(int &errCode) const override;

    const std::list<Entry> GetUpdatedEntries(int &errCode) const override;

    const std::list<Entry> GetDeletedEntries(int &errCode) const override;

    bool IsCleared() const override;

    bool IsChangedDataEmpty() const override;

private:
    int FillInnerData() const;

    DECLARE_OBJECT_TAG(MultiVerNaturalStoreCommitNotifyData);

    mutable MultiVerNaturalStore *db_;
    CommitID startCommitID_;
    CommitID endCommitID_;
    mutable MultiVerDiffData diffData_;
    mutable bool isFilled_;
    mutable std::mutex fillMutex_;
    Version version_;
};
} // namespace DistributedDB

#endif // MULTI_VER_NATURAL_STORE_COMMIT_NOTIFY_DATA_H
#endif