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

#ifndef KVSTORE_RESULTSET_IMPL_H
#define KVSTORE_RESULTSET_IMPL_H

#include <memory>
#include <mutex>
#include <shared_mutex>
#include "ikvstore_resultset.h"
#include "kv_store_result_set.h"
#include "kv_store_nb_delegate.h"

namespace OHOS::DistributedKv {
class KvStoreResultSetImpl : public KvStoreResultSetStub {
public:
    explicit KvStoreResultSetImpl(DistributedDB::KvStoreResultSet *resultSet);
    KvStoreResultSetImpl(DistributedDB::Key keyPrefix, DistributedDB::KvStoreResultSet *resultSet);
    ~KvStoreResultSetImpl() override;

    int GetCount() override;

    int GetPosition() override;

    bool MoveToFirst() override;

    bool MoveToLast() override;

    bool MoveToNext() override;

    bool MoveToPrevious() override;

    bool Move(int offset) override;

    bool MoveToPosition(int position) override;

    bool IsFirst() override;

    bool IsLast() override;

    bool IsBeforeFirst() override;

    bool IsAfterLast() override;

    Status GetEntry(Entry &entry) override;

    Status CloseResultSet(DistributedDB::KvStoreNbDelegate *kvStoreNbDelegate);

    Status MigrateKvStore(DistributedDB::KvStoreNbDelegate *kvStoreNbDelegate);
private:
    static constexpr int INIT_POSTION = -1;

    DistributedDB::Key keyPrefix_ {};
    mutable std::shared_mutex mutex_ {};
    DistributedDB::KvStoreResultSet *kvStoreResultSet_ = nullptr;
};
} // namespace OHOS::DistributedKv
#endif // KVSTORE_RESULTSET_IMPL_H
