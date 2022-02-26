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

#ifndef I_KV_DB_MULTI_VER_TRANSACTION_H
#define I_KV_DB_MULTI_VER_TRANSACTION_H

#ifndef OMIT_MULTI_VER
#include "db_types.h"
#include "multi_ver_def.h"
#include "multi_ver_kv_entry.h"

namespace DistributedDB {
class IKvDBMultiVerTransaction {
public:
    virtual ~IKvDBMultiVerTransaction() {};
    virtual int Put(const Key &key, const Value &value) = 0;
    virtual int Delete(const Key &key) = 0;
    virtual int Clear() = 0;
    virtual int Get(const Key &key, Value &value) const = 0;
    virtual int GetEntries(const Key &keyPrefix, std::vector<Entry> &entries) const = 0;
    virtual int PutBatch(const std::vector<MultiVerKvEntry *> &entries, bool isLocal,
        std::vector<Value> &values) = 0;
    virtual int GetEntriesByVersion(const Version &versionInfo, std::vector<MultiVerKvEntry *> &entries) const = 0;
    virtual int GetDiffEntries(const Version &begin, const Version &end, MultiVerDiffData &data) const = 0;
    virtual int GetMaxVersion(MultiVerDataType type, Version &maxVersion) const = 0;
    virtual int ClearEntriesByVersion(const Version &versionInfo) = 0;
    virtual int StartTransaction() = 0;
    virtual int RollBackTransaction() = 0;
    virtual int CommitTransaction() = 0;
    virtual int UpdateTimestampByVersion(const Version &version, TimeStamp stamp) const = 0;
    virtual bool IsDataChanged() const = 0; // only for write transaction.
    virtual bool IsRecordCleared(const TimeStamp timestamp) const = 0;
    virtual TimeStamp GetCurrentMaxTimestamp() const = 0;
    virtual void SetVersion(const Version &versionInfo) = 0;
    virtual Version GetVersion() const = 0;
    virtual int GetEntriesByVersion(Version version, std::list<MultiVerTrimedVersionData> &data) const = 0;
    virtual int GetOverwrittenClearTypeEntries(Version clearVersion,
        std::list<MultiVerTrimedVersionData> &data) const = 0;
    virtual int GetOverwrittenNonClearTypeEntries(Version version, const Key &hashKey,
        std::list<MultiVerTrimedVersionData> &data) const = 0;
    virtual int DeleteEntriesByHashKey(Version version, const Key &hashKey) = 0;
    virtual int GetValueForTrimSlice(const Key &hashKey, const Version vision, Value &value) const = 0;
};
} // namespace DistributedDB

#endif  // I_KV_DB_MULTI_VER_TRANSACTION_H
#endif