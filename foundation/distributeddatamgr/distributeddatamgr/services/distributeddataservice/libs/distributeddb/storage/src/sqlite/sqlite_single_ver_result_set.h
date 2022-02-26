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

#ifndef SQLITE_SINGLE_VER_RESULT_SET_H
#define SQLITE_SINGLE_VER_RESULT_SET_H

#include <mutex>
#include <vector>

#include "kvdb_windowed_result_set.h"
#include "ikvdb_raw_cursor.h"
#include "query_object.h"

namespace DistributedDB {
constexpr int INIT_POSTION = -1;
constexpr int DEFAULT_RESULT_SET_CACHE_MAX_SIZE = 1; // Unit MB, default 1 MB
constexpr int RESULT_SET_CACHE_MAX_SIZE_MIN = 1;
constexpr int RESULT_SET_CACHE_MAX_SIZE_MAX = 16;
enum class ResultSetType : int {
    KEYPREFIX = 0,
    QUERY = 1,
};
// Forward declaration
class SQLiteSingleVerNaturalStore;
class SQLiteSingleVerStorageExecutor;

class SQLiteSingleVerResultSet : public KvDBWindowedResultSet {
public:
    struct Option {
        ResultSetCacheMode cacheMode = ResultSetCacheMode::CACHE_FULL_ENTRY;
        int cacheMaxSize = DEFAULT_RESULT_SET_CACHE_MAX_SIZE;
    };

    SQLiteSingleVerResultSet(SQLiteSingleVerNaturalStore *kvDB, const Key &keyPrefix, const Option& option);
    SQLiteSingleVerResultSet(SQLiteSingleVerNaturalStore *kvDB, const QueryObject &queryObj, const Option& option);
    ~SQLiteSingleVerResultSet() override;

    // Delete the copy and assign constructors
    DISABLE_COPY_ASSIGN_MOVE(SQLiteSingleVerResultSet);

    // Initialize logic
    int Open(bool isMemDb) override;

    // Get total entries count.
    // >= 0: count, < 0: errCode.
    int GetCount() const override;

    // Get current read position.
    // >= 0: position, < 0: errCode
    int GetPosition() const override;

    // Move the read position to an absolute position value.
    int MoveTo(int position) const override;

    // Get the entry of current position.
    int GetEntry(Entry &entry) const override;

    // Finalize logic
    void Close() override;
private:
    int OpenForCacheFullEntryMode(bool isMemDb);
    int OpenForCacheEntryIdMode();

    int MoveToForCacheFullEntryMode(int position) const;
    int MoveToForCacheEntryIdMode(int position) const;

    void CloseForCacheFullEntryMode();
    void CloseForCacheEntryIdMode();

    const Option option_;
    // Common Part Of Two ResultSet Mode.
    bool isOpen_ = false;
    int count_ = 0;
    mutable int position_ = INIT_POSTION; // The position in the overall result
    mutable std::mutex mutex_;
    // For KeyPrefix Type Or Query Type.
    const ResultSetType type_ = ResultSetType::KEYPREFIX;
    Key keyPrefix_;
    mutable QueryObject queryObj_; // Some QueryObject member function need to call is not a const function(BAD...)
    // Common Pointer For Use, Not Own it, Not Responsible To Release It.
    SQLiteSingleVerNaturalStore *kvDB_ = nullptr;
    // Cache Full Entry Mode Using ResultEntriesWindow and IKvDBRawCursor, Own It, Responsible To Release It.
    ResultEntriesWindow *window_ = nullptr;
    IKvDBRawCursor *rawCursor_ = nullptr;
    // Cache EntryId Mode Using StorageExecutor, Own It, Responsible To Release It.
    SQLiteSingleVerStorageExecutor *handle_ = nullptr;
    mutable std::vector<int64_t> cachedRowIds_;
    mutable int cacheStartPosition_ = INIT_POSTION; // The offset of the first cached rowid in all result rowids
};
} // namespace DistributedDB

#endif // SQLITE_SINGLE_VER_RESULT_SET_H
