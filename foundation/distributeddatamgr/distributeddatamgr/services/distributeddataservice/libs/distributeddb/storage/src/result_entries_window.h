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

#ifndef RESULT_ENTRIES_WINDOW_H
#define RESULT_ENTRIES_WINDOW_H

#include "macro_utils.h"
#include "ikvdb_raw_cursor.h"

namespace DistributedDB {
class ResultEntriesWindow {
public:
    ResultEntriesWindow();
    ~ResultEntriesWindow();
    int Init(IKvDBRawCursor *rawCursor, int64_t windowSize, double scale);
    DISABLE_COPY_ASSIGN_MOVE(ResultEntriesWindow);
    int GetTotalCount() const;
    int GetCurrentPosition() const;
    bool MoveToPosition(int position);
    int GetEntry(Entry &entry) const;

private:
    void ResetWindow();
    int SetCursor(int begin, int target);
    int LoadData(int begin, int target) const;

private:
    static const int64_t MAX_WINDOW_SIZE = 0xFFFFFFFFL; // 4G - 1
    mutable IKvDBRawCursor *rawCursor_;
    int64_t windowSize_;
    int totalCount_;
    mutable std::vector<Entry> buffer_;
    int begin_;
    int currentPosition_;
};
} // namespace DistributedDB

#endif // RESULT_ENTRIES_WINDOW_H
