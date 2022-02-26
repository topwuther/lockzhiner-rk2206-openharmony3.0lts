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

#include "sqlite_local_kvdb_snapshot.h"
#include "sqlite_local_kvdb_connection.h"

namespace DistributedDB {
SQLiteLocalKvDBSnapshot::SQLiteLocalKvDBSnapshot(IKvDBConnection *connect)
    : connect_(connect)
{}

SQLiteLocalKvDBSnapshot::~SQLiteLocalKvDBSnapshot()
{
    connect_ = nullptr;
}

int SQLiteLocalKvDBSnapshot::Get(const Key &key, Value &value) const
{
    if (connect_ == nullptr) {
        return -E_INVALID_DB;
    }
    IOption option;
    return connect_->Get(option, key, value);
}

int SQLiteLocalKvDBSnapshot::GetEntries(const Key &keyPrefix, std::vector<Entry> &entries) const
{
    if (connect_ == nullptr) {
        return -E_INVALID_DB;
    }
    IOption option;
    return connect_->GetEntries(option, keyPrefix, entries);
}

void SQLiteLocalKvDBSnapshot::Close()
{
    if (connect_ != nullptr) {
        connect_->Close();
        connect_ = nullptr;
    }
}
} // namespace DistributedDB
