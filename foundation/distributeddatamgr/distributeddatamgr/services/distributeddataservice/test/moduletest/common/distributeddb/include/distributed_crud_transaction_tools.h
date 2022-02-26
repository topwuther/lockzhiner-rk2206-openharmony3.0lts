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
#ifndef DISTRIBUTED_DB_MODULE_CRUD_TRANSACTION_TOOLS_H
#define DISTRIBUTED_DB_MODULE_CRUD_TRANSACTION_TOOLS_H

#include "kv_store_delegate.h"
#include "kv_store_delegate_manager.h"
#include "types.h"

enum class CrudMode {
    PUT_BATCH = 0,
    UPDATE_BATCH = 1,
    DELETE_BATCH = 2,
    CLEAR = 3,
    PUT = 4,
    DELETE = 5,
};
class DistributedCrudTransactionTools final {
public:

    DistributedCrudTransactionTools(DistributedDB::KvStoreDelegate &delegate,
        CrudMode first, CrudMode second, bool preset, bool isLocal);
    ~DistributedCrudTransactionTools() {}
    bool testCrudTransaction();

    // Delete the copy and assign constructors
    DistributedCrudTransactionTools(const DistributedCrudTransactionTools &distributeDBTools) = delete;
    DistributedCrudTransactionTools& operator=(const DistributedCrudTransactionTools &distributeDBTools) = delete;
    DistributedCrudTransactionTools(DistributedCrudTransactionTools &&distributeDBTools) = delete;
    DistributedCrudTransactionTools& operator=(DistributedCrudTransactionTools &&distributeDBTools) = delete;

private:
    bool PresetValue();
    bool CheckFirst();
    bool CheckSecond();
    void Check();
    bool Action1(DistributedDB::KvStoreDelegate &delegate);
    bool Action2(DistributedDB::KvStoreDelegate &delegate);
    DistributedDB::KvStoreDelegate *storeDelegate_;
    CrudMode firstMode_;
    CrudMode secondMode_;
    bool isLocal_;
    bool needPresetData_;
    unsigned long presetCount_ = 128;
    int presetValue_ = 1;
    std::vector<DistributedDB::Entry> entriesBatch_;
    std::vector<DistributedDB::Key> allKeys_;
    bool firstComplete_ = false;
    bool secondComplete_ = false;
    bool success_ = true;
};

#endif // DISTRIBUTED_DB_MODULE_CRUD_TRANSACTION_TOOLS_H