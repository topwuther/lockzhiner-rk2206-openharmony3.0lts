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

#ifndef KV_STORE_RESULT_SET_IMPL_H
#define KV_STORE_RESULT_SET_IMPL_H

#include <mutex>

#include "kv_store_result_set.h"
#include "ikvdb_result_set.h"

namespace DistributedDB {
class KvStoreResultSetImpl final : public KvStoreResultSet {
public:
    explicit KvStoreResultSetImpl(IKvDBResultSet *resultSet);
    ~KvStoreResultSetImpl() override {};

    DISABLE_COPY_ASSIGN_MOVE(KvStoreResultSetImpl);

    // Returns the numbers of rows in the result set.
    int GetCount() const override;

    // Returns the current position of the result set in the row set.
    int GetPosition() const override;

    // Move the result set to the first row, return false if the result set is empty.
    bool MoveToFirst() override;

    // Move the result set to the last row, return false if the result set is empty.
    bool MoveToLast() override;

    // Move the result set to the next row, return false if the result set is already past
    // the last entry in the result set.
    bool MoveToNext() override;

    // Move the result set to the previous row, return false if the result set is already before
    // the first entry in the result set
    bool MoveToPrevious() override;

    // Move the result set by a relative amount, forward or backward, from the current position.
    bool Move(int offset) override;

    // Move the result set to an absolute position, the valid range of value is [-1, count]
    bool MoveToPosition(int position) override;

    // Returns whether the result set is pointing to the first row.
    bool IsFirst() const override;

    // Returns whether the result set is pointing to the last row.
    bool IsLast() const override;

    // Returns whether the result set is pointing to the position before the first row.
    bool IsBeforeFirst() const override;

    // Returns whether the result set is pointing to the position after the last row
    bool IsAfterLast() const override;

    // Get a key-value entry.
    DBStatus GetEntry(Entry &entry) const override;

    // Get the result set obj
    void GetResultSet(IKvDBResultSet *&resultSet) const;

private:
    static const int INIT_POSTION;
    IKvDBResultSet * const resultSet_;
};
} // namespace DistributedDB

#endif // KV_STORE_RESULT_SET_IMPL_H