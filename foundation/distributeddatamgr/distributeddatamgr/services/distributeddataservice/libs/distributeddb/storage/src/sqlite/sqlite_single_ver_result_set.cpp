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

#include "sqlite_single_ver_result_set.h"
#include <algorithm>
#include "log_print.h"
#include "db_errno.h"
#include "sqlite_single_ver_forward_cursor.h"
#include "sqlite_single_ver_natural_store.h"
#include "sqlite_single_ver_storage_executor.h"

namespace DistributedDB {
namespace {
    const int64_t MEM_WINDOW_SIZE = 0xFFFFFFFF; // 4G for max
    const double MEM_WINDOW_SCALE = 0.5; // set default window size to 2G
    const double DEFAULT_WINDOW_SCALE = 1; // For non-mem db
    const int64_t WINDOW_SIZE_MB_UNIT = 1024 * 1024; // 1024 is scale
}

SQLiteSingleVerResultSet::SQLiteSingleVerResultSet(SQLiteSingleVerNaturalStore *kvDB, const Key &keyPrefix,
    const Option& option) : option_(option), type_(ResultSetType::KEYPREFIX), keyPrefix_(keyPrefix), kvDB_(kvDB) {}

SQLiteSingleVerResultSet::SQLiteSingleVerResultSet(SQLiteSingleVerNaturalStore *kvDB, const QueryObject &queryObj,
    const Option& option) : option_(option), type_(ResultSetType::QUERY), queryObj_(queryObj), kvDB_(kvDB) {}

SQLiteSingleVerResultSet::~SQLiteSingleVerResultSet()
{
    isOpen_ = false;
    count_ = 0;
    position_ = INIT_POSTION;
    kvDB_ = nullptr;
    window_ = nullptr;
    rawCursor_ = nullptr;
    handle_ = nullptr;
    cacheStartPosition_ = INIT_POSTION;
}

// The user get KvStoreResultSet after Open function called, so no need mutex during open procedure
int SQLiteSingleVerResultSet::Open(bool isMemDb)
{
    if (isOpen_) {
        return E_OK;
    }
    if (kvDB_ == nullptr) { // Unlikely
        return -E_INVALID_ARGS;
    }
    if (option_.cacheMode == ResultSetCacheMode::CACHE_FULL_ENTRY) {
        return OpenForCacheFullEntryMode(isMemDb);
    } else {
        return OpenForCacheEntryIdMode();
    }
}

int SQLiteSingleVerResultSet::OpenForCacheFullEntryMode(bool isMemDb)
{
    if (type_ == ResultSetType::KEYPREFIX) {
        rawCursor_ = new (std::nothrow) SQLiteSingleVerForwardCursor(kvDB_, keyPrefix_);
    } else {
        rawCursor_ = new (std::nothrow) SQLiteSingleVerForwardCursor(kvDB_, queryObj_);
    }
    if (rawCursor_ == nullptr) {
        LOGE("[SqlSinResSet][OpenForEntry] OOM When Create ForwardCursor.");
        return E_OUT_OF_MEMORY;
    }
    window_ = new (std::nothrow) ResultEntriesWindow();
    if (window_ == nullptr) {
        LOGE("[SqlSinResSet][OpenForEntry] OOM When Create EntryWindow.");
        delete rawCursor_;
        rawCursor_ = nullptr;
        return -E_OUT_OF_MEMORY;
    }
    // cacheMaxSize is within [1,16]
    int64_t windowSize = isMemDb ? MEM_WINDOW_SIZE : (option_.cacheMaxSize * WINDOW_SIZE_MB_UNIT);
    double scale = isMemDb ? MEM_WINDOW_SCALE : DEFAULT_WINDOW_SCALE;
    int errCode = window_->Init(rawCursor_, windowSize, scale);
    if (errCode != E_OK) {
        LOGE("[SqlSinResSet][OpenForEntry] EntryWindow Init Fail, ErrCode=%d.", errCode);
        delete window_;
        window_ = nullptr;
        delete rawCursor_;
        rawCursor_ = nullptr;
        return errCode;
    }
    count_ = window_->GetTotalCount();
    isOpen_ = true;
    LOGD("[SqlSinResSet][OpenForEntry] Type=%d, CacheMaxSize=%d(MB), Count=%d, IsMem=%d.", type_,
        option_.cacheMaxSize, count_, isMemDb);
    return E_OK;
}

int SQLiteSingleVerResultSet::OpenForCacheEntryIdMode()
{
    int errCode = E_OK;
    handle_ = kvDB_->GetHandle(false, errCode);
    if (handle_ == nullptr) {
        LOGE("[SqlSinResSet][OpenForRowId] Get handle fail, errCode=%d.", errCode);
        return errCode;
    }
    // cacheMaxSize is within [1,16], rowId is of type int64_t
    uint32_t cacheLimit = option_.cacheMaxSize * (WINDOW_SIZE_MB_UNIT / sizeof(int64_t));
    if (type_ == ResultSetType::KEYPREFIX) {
        errCode = handle_->OpenResultSetForCacheRowIdMode(keyPrefix_, cachedRowIds_, cacheLimit, count_);
    } else {
        errCode = handle_->OpenResultSetForCacheRowIdMode(queryObj_, cachedRowIds_, cacheLimit, count_);
    }
    if (errCode != E_OK) {
        LOGE("[SqlSinResSet][OpenForRowId] Open ResultSet fail, errCode=%d.", errCode);
        kvDB_->ReleaseHandle(handle_);
        cachedRowIds_.clear();
        return errCode;
    }
    // If no result, then nothing is cached, so the cacheStartPosition_ is still INIT_POSTION
    if (count_ != 0) {
        cacheStartPosition_ = 0;
    }
    isOpen_ = true;
    LOGD("[SqlSinResSet][OpenForRowId] Type=%d, CacheMaxSize=%d(MB), Count=%d, Cached=%zu.", type_,
        option_.cacheMaxSize, count_, cachedRowIds_.size());
    return E_OK;
}

int SQLiteSingleVerResultSet::GetCount() const
{
    // count_ never changed after ResultSet opened
    return count_;
}

int SQLiteSingleVerResultSet::GetPosition() const
{
    std::lock_guard<std::mutex> lockGuard(mutex_);
    return position_;
}

int SQLiteSingleVerResultSet::MoveTo(int position) const
{
    std::lock_guard<std::mutex> lockGuard(mutex_);
    if (!isOpen_) {
        return -E_RESULT_SET_STATUS_INVALID;
    }
    if (count_ == 0) {
        position_ = (position >= 0) ? 0 : INIT_POSTION;
        LOGW("[SqlSinResSet][MoveTo] Empty ResultSet.");
        return -E_RESULT_SET_EMPTY;
    }
    if (position < 0) {
        position_ = INIT_POSTION;
        LOGW("[SqlSinResSet][MoveTo] Target Position=%d invalid.", position);
        return -E_INVALID_ARGS;
    }
    if (position >= count_) {
        position_ = count_;
        LOGW("[SqlSinResSet][MoveTo] Target Position=%d Exceed Count=%d.", position, count_);
        return -E_INVALID_ARGS;
    }
    if (position_ == position) {
        return E_OK;
    }
    if (option_.cacheMode == ResultSetCacheMode::CACHE_FULL_ENTRY) {
        return MoveToForCacheFullEntryMode(position);
    } else {
        return MoveToForCacheEntryIdMode(position);
    }
}

int SQLiteSingleVerResultSet::MoveToForCacheFullEntryMode(int position) const
{
    if (window_->MoveToPosition(position)) {
        position_ = position;
        return E_OK;
    }
    position_ = INIT_POSTION;
    LOGE("[SqlSinResSet][MoveForEntry] Move to position=%d fail.", position);
    return -E_UNEXPECTED_DATA;
}

int SQLiteSingleVerResultSet::MoveToForCacheEntryIdMode(int position) const
{
    // The parameter position now is in [0, count_) with this resultSet not empty
    // cacheEndPosition is just after cachedRowIds_, the cached range is [cacheStartPosition_, cacheEndPosition)
    int cacheEndPosition = cacheStartPosition_ + cachedRowIds_.size();
    if (position >= cacheStartPosition_ && position < cacheEndPosition) {
        // Already in the cachedRowId range, Just move position
        position_ = position;
        return E_OK;
    }
    // Not in the cachedRowId range, but valid position, we should reload the cachedRowIds to contain this position
    int newCacheStartPos = position;
    // cacheMaxSize is within [1,16], rowId is of type int64_t
    uint32_t cacheLimit = option_.cacheMaxSize * (WINDOW_SIZE_MB_UNIT / sizeof(int64_t));
    if (position > cacheStartPosition_) {
        // Move Forward
        int newCacheEndPos = newCacheStartPos + cacheLimit;
        if (newCacheEndPos > count_) {
            // Since startPos in [0, count_), So the right in (0, cacheLimit), So position still in range
            newCacheStartPos -= (newCacheEndPos - count_);
        }
    } else {
        // Move Backward
        newCacheStartPos -= (cacheLimit - 1); // Attention, subtract by 1 to ensure position still in range
    }
    newCacheStartPos = std::max(newCacheStartPos, 0); // Adjust to at least 0 if less then 0
    // Clear rowId cache to accept new rowIds
    cachedRowIds_.clear();
    int errCode;
    if (type_ == ResultSetType::KEYPREFIX) {
        errCode = handle_->ReloadResultSetForCacheRowIdMode(keyPrefix_, cachedRowIds_, cacheLimit, newCacheStartPos);
    } else {
        errCode = handle_->ReloadResultSetForCacheRowIdMode(queryObj_, cachedRowIds_, cacheLimit, newCacheStartPos);
    }
    if (errCode != E_OK) {
        LOGE("[SqlSinResSet][MoveForRowid] Move to position=%d, Reload fail, errCode=%d.", position, errCode);
        // What else shall we do if error happened ?
        cachedRowIds_.clear();
        cacheStartPosition_ = INIT_POSTION;
        position_ = INIT_POSTION; // Reset Position As MoveForEntry Do
        return -E_UNEXPECTED_DATA;
    }
    LOGD("[SqlSinResSet][MoveForRowid] Reload: position=%d, cacheStartPos=%d, cached=%zu, count=%d.",
        position, newCacheStartPos, cachedRowIds_.size(), count_);
    // Everything OK
    position_ = position;
    cacheStartPosition_ = newCacheStartPos;
    return E_OK;
}

int SQLiteSingleVerResultSet::GetEntry(Entry &entry) const
{
    std::lock_guard<std::mutex> lockGuard(mutex_);
    if (!isOpen_ || count_ == 0) {
        return -E_NO_SUCH_ENTRY;
    }
    if (position_ > INIT_POSTION && position_ < count_) {
        // If position_ in the valid range, it can be guaranteed that everything is ok without errors
        if (option_.cacheMode == ResultSetCacheMode::CACHE_FULL_ENTRY) {
            return window_->GetEntry(entry);
        } else {
            // It can be guaranteed position_ in the range [cacheStartPosition_, cacheEndPosition)
            // For CodeDex false alarm, we still do the check which is not necessary
            int cacheIndex = position_ - cacheStartPosition_;
            if (cacheIndex < 0 || cacheIndex >= static_cast<int>(cachedRowIds_.size())) { // Not Possible
                LOGE("[SqlSinResSet][GetEntry] Internal Error: Position=%d, CacheStartPos=%d, cached=%zu.", position_,
                    cacheStartPosition_, cachedRowIds_.size());
                return -E_INTERNAL_ERROR;
            }
            int errCode = handle_->GetEntryByRowId(cachedRowIds_[cacheIndex], entry);
            if (errCode != E_OK) {
                LOGE("[SqlSinResSet][GetEntry] GetEntryByRowId fail, errCode=%d.", errCode);
                return errCode;
            }
            return E_OK;
        }
    }
    return -E_NO_SUCH_ENTRY;
}

void SQLiteSingleVerResultSet::Close()
{
    std::lock_guard<std::mutex> lockGuard(mutex_);
    if (!isOpen_) {
        return;
    }
    if (option_.cacheMode == ResultSetCacheMode::CACHE_FULL_ENTRY) {
        CloseForCacheFullEntryMode();
    } else {
        CloseForCacheEntryIdMode();
    }
    isOpen_ = false;
    count_ = 0;
    position_ = INIT_POSTION;
    LOGD("[SqlSinResSet][Close] Done, Type=%d, Mode=%d.", type_, option_.cacheMode);
}

void SQLiteSingleVerResultSet::CloseForCacheFullEntryMode()
{
    // Attention! Must Delete EntryWindow First(will call ForwardCursor::Close), then delete ForwardCursor.
    // ForwardCursor::Close will call Executor::CloseResultSet(Reset the statement and rollback transaction)
    delete window_; // It is defined behavior to delete even a nullptr
    window_ = nullptr;
    // Attention! Delete ForwardCursor Later.
    delete rawCursor_; // It is defined behavior to delete even a nullptr
    rawCursor_ = nullptr;
}

void SQLiteSingleVerResultSet::CloseForCacheEntryIdMode()
{
    cacheStartPosition_ = INIT_POSTION;
    cachedRowIds_.clear();
    // In Fact : handle_ and kvDB_ is guaranteed to be not nullptr
    if (handle_ != nullptr) {
        handle_->CloseResultSet();
        kvDB_->ReleaseHandle(handle_);
    }
}
} // namespace DistributedDB
