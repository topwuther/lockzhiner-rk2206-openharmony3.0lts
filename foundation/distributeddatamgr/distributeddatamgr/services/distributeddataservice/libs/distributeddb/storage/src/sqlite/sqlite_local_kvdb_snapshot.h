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

#ifndef SQLITE_LOCAL_KV_DB_SNAP_SHOT_H
#define SQLITE_LOCAL_KV_DB_SNAP_SHOT_H

#include <string>

#include "macro_utils.h"
#include "db_errno.h"
#include "db_types.h"
#include "ikvdb_snapshot.h"
#include "sqlite_local_kvdb_connection.h"

namespace DistributedDB {
class SQLiteLocalKvDBSnapshot : public IKvDBSnapshot {
public:
    explicit SQLiteLocalKvDBSnapshot(IKvDBConnection *connect);
    ~SQLiteLocalKvDBSnapshot();

    // Delete the copy and assign constructors
    DISABLE_COPY_ASSIGN_MOVE(SQLiteLocalKvDBSnapshot);

    // Get the value of the key
    int Get(const Key &key, Value &value) const override;

    // Get the entries of the key set
    int GetEntries(const Key &keyPrefix, std::vector<Entry> &entry) const override;

    void Close();

private:
    IKvDBConnection *connect_;
};
} // namespace DistributedDB

#endif // SQLITE_LOCAL_KV_DB_SNAP_SHOT_H
