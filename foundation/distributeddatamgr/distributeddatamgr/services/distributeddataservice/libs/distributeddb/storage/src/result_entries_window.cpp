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

#include "result_entries_window.h"

#include "db_constant.h"
#include "db_errno.h"

using std::vector;
using std::move;

namespace DistributedDB {
ResultEntriesWindow::ResultEntriesWindow()
    : rawCursor_(nullptr),
      windowSize_(0),
      totalCount_(0),
      begin_(0),
      currentPosition_(0) {}

ResultEntriesWindow::~ResultEntriesWindow()
{
    if (rawCursor_ != nullptr) {
        (void)(rawCursor_->Close());
        rawCursor_ = nullptr;
    }
}

int ResultEntriesWindow::Init(IKvDBRawCursor *rawCursor, int64_t windowSize, double scale)
{
    if (rawCursor == nullptr ||
        (windowSize <= 0 || windowSize > MAX_WINDOW_SIZE) ||
        (scale <= std::numeric_limits<double>::epsilon() || scale > 1)) {
        return -E_INVALID_ARGS;
    }
    int errCode = rawCursor->Open();
    if (errCode != E_OK) {
        return errCode;
    }

    rawCursor_ = rawCursor;
    windowSize_ = static_cast<int64_t>(static_cast<double>(windowSize) * scale);
    totalCount_ = rawCursor_->GetCount();
    return E_OK;
}

int ResultEntriesWindow::GetTotalCount() const
{
    return totalCount_;
}

int ResultEntriesWindow::GetCurrentPosition() const
{
    return currentPosition_;
}

bool ResultEntriesWindow::MoveToPosition(int position)
{
    if ((rawCursor_ == nullptr && buffer_.size() == 0) || (position < 0 || position >= totalCount_)) {
        return false;
    }
    if (buffer_.size() == 0) {
        if (SetCursor(0, position) != E_OK) {
            return false;
        }
        return true;
    }
    int last = begin_ + buffer_.size() - 1;
    if (position > last) {
        buffer_.clear();
        buffer_.shrink_to_fit();
        if (SetCursor(last + 1, position) != E_OK) {
            return false;
        }
        return true;
    } else if (position < begin_) {
        if (rawCursor_ == nullptr) {
            return false;
        }
        buffer_.clear();
        buffer_.shrink_to_fit();
        if (rawCursor_->Reload() != E_OK) {
            ResetWindow();
            return false;
        }
        if (SetCursor(0, position) != E_OK) {
            return false;
        }
        return true;
    } else {
        currentPosition_ = position;
    }
    return true;
}

int ResultEntriesWindow::GetEntry(Entry &entry) const
{
    if (rawCursor_ == nullptr && buffer_.size() == 0) {
        return -E_NOT_INIT;
    }
    if (totalCount_ == 0) {
        return -E_NOT_FOUND;
    }
    if (buffer_.size() == 0) {
        int errCode = LoadData(0, currentPosition_);
        if (errCode != E_OK) {
            return errCode;
        }
    }
    entry = buffer_[currentPosition_ - begin_];
    return E_OK;
}

void ResultEntriesWindow::ResetWindow()
{
    buffer_.clear();
    buffer_.shrink_to_fit();
    if (rawCursor_ != nullptr) {
        (void)(rawCursor_->Reload());
    }
    begin_ = 0;
    currentPosition_ = 0;
    return;
}

int ResultEntriesWindow::SetCursor(int begin, int target)
{
    int errCode = LoadData(begin, target);
    if (errCode == E_OK) {
        begin_ = target;
        currentPosition_ = target;
    } else {
        ResetWindow();
    }
    return errCode;
}

int ResultEntriesWindow::LoadData(int begin, int target) const
{
    if (rawCursor_ == nullptr) {
        return -E_NOT_INIT;
    }
    if (target < begin || target >= totalCount_) {
        return -E_INVALID_ARGS;
    }
    int cursor = begin;
    int errCode = E_OK;
    for (; cursor < target; cursor++) {
        Entry next;
        errCode = rawCursor_->GetNext(next, false);
        if (errCode != E_OK) {
            return errCode;
        }
    }
    int64_t bufferSize = 0;
    for (; cursor < totalCount_ && bufferSize < windowSize_; cursor++) {
        Entry next;
        errCode = rawCursor_->GetNext(next, true);
        if (errCode != E_OK) {
            return errCode;
        }
        // filter the abnormal data.
        if (next.key.size() > DBConstant::MAX_KEY_SIZE || next.value.size() > DBConstant::MAX_VALUE_SIZE) {
            continue;
        }
        bufferSize += next.key.size() + next.value.size();
        buffer_.push_back(move(next));
    }
    if (buffer_.size() == static_cast<size_t>(totalCount_)) {
        (void)(rawCursor_->Close());
        rawCursor_ = nullptr;
    }
    return E_OK;
}
}
