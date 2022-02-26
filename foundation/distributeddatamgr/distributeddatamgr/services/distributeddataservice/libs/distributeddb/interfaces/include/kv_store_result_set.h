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

#ifndef KV_STORE_RESULT_SET_H
#define KV_STORE_RESULT_SET_H

#include "types.h"

namespace DistributedDB {
class KvStoreResultSet {
public:
    DB_API virtual ~KvStoreResultSet() {};

    // Returns the count of rows in the result set.
    DB_API virtual int GetCount() const = 0;

    // Returns the current read position of the result set.
    DB_API virtual int GetPosition() const = 0;

    // Move the read position to the first row, return false if the result set is empty.
    DB_API virtual bool MoveToFirst() = 0;

    // Move the read position to the last row, return false if the result set is empty.
    DB_API virtual bool MoveToLast() = 0;

    // Move the read position to the next row, return false if the result set is empty
    // or the read position is already past the last entry in the result set.
    DB_API virtual bool MoveToNext() = 0;

    // Move the read position to the previous row, return false if the result set is empty
    // or the read position is already before the first entry in the result set.
    DB_API virtual bool MoveToPrevious() = 0;

    // Move the read position by a relative amount from the current position.
    DB_API virtual bool Move(int offset) = 0;

    // Move the read position to an absolute position value.
    DB_API virtual bool MoveToPosition(int position) = 0;

    // Returns whether the read position is pointing to the first row.
    DB_API virtual bool IsFirst() const = 0;

    // Returns whether the read position is pointing to the last row.
    DB_API virtual bool IsLast() const = 0;

    // Returns whether the read position is before the first row.
    DB_API virtual bool IsBeforeFirst() const = 0;

    // Returns whether the read position is after the last row
    DB_API virtual bool IsAfterLast() const = 0;

    // Get a key-value entry.
    DB_API virtual DBStatus GetEntry(Entry &entry) const = 0;
};
} // namespace DistributedDB

#endif // KV_STORE_RESULT_SET_H