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

#ifndef SQLITE_SINGLE_VER_FORWARD_CURSOR_H
#define SQLITE_SINGLE_VER_FORWARD_CURSOR_H

#include <mutex>

#include "macro_utils.h"
#include "ikvdb_raw_cursor.h"
#include "sqlite_single_ver_storage_executor.h"
#include "sqlite_single_ver_natural_store.h"

namespace DistributedDB {
class SQLiteSingleVerForwardCursor : public IKvDBRawCursor {
public:
    SQLiteSingleVerForwardCursor(SQLiteSingleVerNaturalStore *kvDB, const Key &keyPrefix);
    SQLiteSingleVerForwardCursor(SQLiteSingleVerNaturalStore *kvDB, const QueryObject &queryObj);
    ~SQLiteSingleVerForwardCursor() override;

    // Delete the copy and assign constructors
    DISABLE_COPY_ASSIGN_MOVE(SQLiteSingleVerForwardCursor);

    // Open the raw cursor.
    int Open() override;

    // Close the raw cursor before invoking operator delete.
    void Close() override;

    // Reload all data.
    int Reload() override;

    // Get total entries count.
    int GetCount() const override;

    // Get next entry, return errCode if it fails.
    int GetNext(Entry &entry, bool isCopy) const override;

private:
    SQLiteSingleVerNaturalStore *kvDB_;
    Key keyPrefix_;
    QueryObject queryObj_;
    mutable SQLiteSingleVerStorageExecutor *handle_;
    int count_;
    mutable bool isOpen_;
    bool isQueryMode_;
    mutable std::mutex isOpenMutex_;
};
} // namespace DistributedDB

#endif // SQLITE_SINGLE_VER_FORWARD_CURSOR_H
