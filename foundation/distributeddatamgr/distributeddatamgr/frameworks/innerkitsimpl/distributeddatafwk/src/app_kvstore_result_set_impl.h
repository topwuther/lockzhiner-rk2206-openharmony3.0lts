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

#ifndef APP_KV_STORE_RESULT_SET_IMPL_H
#define APP_KV_STORE_RESULT_SET_IMPL_H

#include "app_kvstore_result_set.h"
#include "kv_store_nb_delegate.h"
#include "kv_store_result_set.h"

namespace OHOS {
namespace AppDistributedKv {
class AppKvStoreResultSetImpl : public AppKvStoreResultSet {
public:
    AppKvStoreResultSetImpl(DistributedDB::KvStoreResultSet *resultSet, DistributedDB::KvStoreNbDelegate *delegate);

    ~AppKvStoreResultSetImpl();

    // Returns the count of rows in the result set.
    int GetCount() const override;

    // Returns the current read position of the result set.
    int GetPosition() const override;

    // Move the read position to the first row, return false if the result set is empty.
    bool MoveToFirst() override;

    // Move the read position to the last row, return false if the result set is empty.
    bool MoveToLast() override;

    // Move the read position to the next row,
    // return false if the result set is empty or the read position is already past the last entry in the result set.
    bool MoveToNext() override;

    // Move the read position to the previous row,
    // return false if result set is empty or the read position is already before the first entry in the result set.
    bool MoveToPrevious() override;

    // Move the read position by a relative amount from the current position.
    bool Move(int offset) override;

    // Move the read position to an absolute position value.
    bool MoveToPosition(int position) override;

    // Returns whether the read position is pointing to the first row.
    bool IsFirst() const override;

    // Returns whether the read position is pointing to the last row.
    bool IsLast() const override;

    // Returns whether the read position is before the first row.
    bool IsBeforeFirst() const override;

    // Returns whether the read position is after the last row
    bool IsAfterLast() const override;

    // Get a key-value entry.
    Status GetEntry(Entry &entry) const override;

    Status Close() override;
private:
    DistributedDB::KvStoreResultSet *kvStoreResultSet_ = nullptr;
    DistributedDB::KvStoreNbDelegate *nbDelegate_ = nullptr;
    static const int INIT_POSTION;
};
}  // namespace AppDistributedKv
}  // namespace OHOS
#endif  // APP_KV_STORE_RESULT_SET_IMPL_H