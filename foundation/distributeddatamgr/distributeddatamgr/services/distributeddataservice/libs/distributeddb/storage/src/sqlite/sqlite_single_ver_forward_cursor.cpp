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

#include "sqlite_single_ver_forward_cursor.h"

#include "db_errno.h"
#include "log_print.h"

namespace DistributedDB {
SQLiteSingleVerForwardCursor::SQLiteSingleVerForwardCursor(SQLiteSingleVerNaturalStore *kvDB, const Key &keyPrefix)
    : kvDB_(kvDB),
      keyPrefix_(keyPrefix),
      handle_(nullptr),
      count_(0),
      isOpen_(false),
      isQueryMode_(false)
{}

SQLiteSingleVerForwardCursor::SQLiteSingleVerForwardCursor(SQLiteSingleVerNaturalStore *kvDB,
    const QueryObject &queryObj)
    : kvDB_(kvDB),
      queryObj_(queryObj),
      handle_(nullptr),
      count_(0),
      isOpen_(false),
      isQueryMode_(true)
{}

SQLiteSingleVerForwardCursor::~SQLiteSingleVerForwardCursor()
{
    kvDB_ = nullptr;
    keyPrefix_.clear();
    handle_ = nullptr;
    count_ = 0;
}

int SQLiteSingleVerForwardCursor::Open()
{
    std::lock_guard<std::mutex> lock(isOpenMutex_);
    if (isOpen_) {
        return E_OK;
    }
    int errCode = E_OK;
    handle_ = kvDB_->GetHandle(false, errCode);
    if (handle_ == nullptr) {
        LOGE("Get handle failed.");
        return errCode;
    }

    if (isQueryMode_) {
        errCode = handle_->OpenResultSet(queryObj_, count_);
    } else {
        errCode = handle_->OpenResultSet(keyPrefix_, count_);
    }
    if (errCode == E_OK) {
        if (count_ == 0) {
            handle_->CloseResultSet();
            kvDB_->ReleaseHandle(handle_);
        }
        isOpen_ = true;
    } else {
        handle_->CloseResultSet();
        kvDB_->ReleaseHandle(handle_);
        LOGE("Handle open result set failed, errCode: %d", errCode);
    }

    return errCode;
}

void SQLiteSingleVerForwardCursor::Close()
{
    std::lock_guard<std::mutex> lock(isOpenMutex_);
    if (!isOpen_) {
        return;
    }
    if (handle_ != nullptr) {
        handle_->CloseResultSet();
        kvDB_->ReleaseHandle(handle_);
    }
    count_ = 0;
    isOpen_ = false;
}

int SQLiteSingleVerForwardCursor::Reload()
{
    std::lock_guard<std::mutex> lock(isOpenMutex_);
    if (!isOpen_) {
        return -E_RESULT_SET_STATUS_INVALID;
    }
    if (count_ == 0) {
        return E_OK;
    }
    int errCode = E_OK;
    if (isQueryMode_) {
        errCode = handle_->ReloadResultSet(queryObj_);
    } else {
        errCode = handle_->ReloadResultSet(keyPrefix_);
    }
    if (errCode != E_OK) {
        handle_->CloseResultSet();
        kvDB_->ReleaseHandle(handle_);
        isOpen_ = false;
    }
    return errCode;
}

int SQLiteSingleVerForwardCursor::GetCount() const
{
    std::lock_guard<std::mutex> lock(isOpenMutex_);
    if (!isOpen_) {
        return 0;
    }
    return count_;
}

int SQLiteSingleVerForwardCursor::GetNext(Entry &entry, bool isCopy) const
{
    std::lock_guard<std::mutex> lock(isOpenMutex_);
    if (!isOpen_) {
        return -E_RESULT_SET_STATUS_INVALID;
    }
    if (count_ == 0) {
        return -E_RESULT_SET_EMPTY;
    }
    int errCode = handle_->GetNextEntryFromResultSet(entry.key, entry.value, isCopy);
    if (errCode != E_OK && errCode != -E_FINISHED) {
        handle_->CloseResultSet();
        kvDB_->ReleaseHandle(handle_);
        isOpen_ = false;
    }
    return errCode;
}
} // namespace DistributedDB
