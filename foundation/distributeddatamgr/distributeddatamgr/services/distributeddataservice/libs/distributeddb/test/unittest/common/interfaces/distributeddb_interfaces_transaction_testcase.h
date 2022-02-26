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

#ifndef DISTRIBUTEDDB_INTERFACES_TRANSACTION_TESTCASE_H
#define DISTRIBUTEDDB_INTERFACES_TRANSACTION_TESTCASE_H

#include <gtest/gtest.h>
#include "distributeddb_data_generate_unit_test.h"
#include "distributeddb_tools_unit_test.h"

class DistributedDBInterfacesTransactionTestCase final {
public:
    DistributedDBInterfacesTransactionTestCase() {};
    ~DistributedDBInterfacesTransactionTestCase() {};

    static void StartTransaction001(DistributedDB::KvStoreDelegate *&kvDelegatePtr);

    static void StartTransaction002(DistributedDB::KvStoreDelegate *&kvDelegatePtr);

    static void StartTransaction003(DistributedDB::KvStoreDelegate *&kvDelegatePtr);

    static void StartTransaction004(DistributedDB::KvStoreDelegate *&kvDelegatePtr, const std::string &storeId,
        bool localOnly, DistributedDB::KvStoreDelegateManager &mgr,
        DistributedDB::KvStoreSnapshotDelegate *&snapshotDelegatePtr);

    static void StartTransaction005(DistributedDB::KvStoreDelegate *&kvDelegatePtr, const std::string &storeId,
        bool localOnly, DistributedDB::KvStoreDelegateManager &mgr);

    static void Commit001(DistributedDB::KvStoreDelegate *&kvDelegatePtr);

    static void Commit002(DistributedDB::KvStoreDelegate *&kvDelegatePtr);

    static void Commit003(DistributedDB::KvStoreDelegate *&kvDelegatePtr,
        DistributedDB::KvStoreSnapshotDelegate *&snapshotDelegatePtr);

    static void Commit004(DistributedDB::KvStoreDelegate *&kvDelegatePtr,
        DistributedDB::KvStoreSnapshotDelegate *&snapshotDelegatePtr);

    static void Commit005(DistributedDB::KvStoreDelegate *&kvDelegatePtr,
        DistributedDB::KvStoreSnapshotDelegate *&snapshotDelegatePtr);

    static void Commit006(DistributedDB::KvStoreDelegate *&kvDelegatePtr,
        DistributedDB::KvStoreSnapshotDelegate *&snapshotDelegatePtr);

    static void Commit007(DistributedDB::KvStoreDelegate *&kvDelegatePtr,
        DistributedDB::KvStoreSnapshotDelegate *&snapshotDelegatePtr);

    static void Commit008(DistributedDB::KvStoreDelegate *&kvDelegatePtr,
        DistributedDB::KvStoreSnapshotDelegate *&snapshotDelegatePtr);

    static void RollBack001(DistributedDB::KvStoreDelegate *&kvDelegatePtr);

    static void RollBack002(DistributedDB::KvStoreDelegate *&kvDelegatePtr);

    static void RollBack003(DistributedDB::KvStoreDelegate *&kvDelegatePtr,
        DistributedDB::KvStoreSnapshotDelegate *&snapshotDelegatePtr);

    static void RollBack004(DistributedDB::KvStoreDelegate *&kvDelegatePtr,
        DistributedDB::KvStoreSnapshotDelegate *&snapshotDelegatePtr);

    static void RollBack005(DistributedDB::KvStoreDelegate *&kvDelegatePtr,
        DistributedDB::KvStoreSnapshotDelegate *&snapshotDelegatePtr);

    static void RollBack006(DistributedDB::KvStoreDelegate *&kvDelegatePtr,
        DistributedDB::KvStoreSnapshotDelegate *&snapshotDelegatePtr);

    static void RollBack007(DistributedDB::KvStoreDelegate *&kvDelegatePtr,
        DistributedDB::KvStoreSnapshotDelegate *&snapshotDelegatePtr);

    static void RollBack008(DistributedDB::KvStoreDelegate *&kvDelegatePtr,
        DistributedDB::KvStoreSnapshotDelegate *&snapshotDelegatePtr);
};
#endif