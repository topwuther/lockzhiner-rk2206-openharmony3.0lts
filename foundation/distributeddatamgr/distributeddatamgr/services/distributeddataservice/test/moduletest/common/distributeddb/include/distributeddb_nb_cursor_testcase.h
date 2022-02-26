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
#ifndef DISTRIBUTED_NB_CURSOR_TESTCASE_H
#define DISTRIBUTED_NB_CURSOR_TESTCASE_H

#include <vector>
#include "kv_store_delegate.h"
#include "kv_store_delegate_manager.h"
#include "kv_store_result_set.h"
class DistributedNbCursorTestcase final {
public:
    DistributedNbCursorTestcase() {};
    ~DistributedNbCursorTestcase() {}
    static void ResultSetDb001(DistributedDB::KvStoreNbDelegate *delegate, bool isRowIdMode);
    static void ResultSetDb002(DistributedDB::KvStoreNbDelegate *delegate, bool isRowIdMode);
    static void ResultSetDb003(DistributedDB::KvStoreNbDelegate *delegate, bool isRowIdMode);
    static void ResultSetDb004(DistributedDB::KvStoreNbDelegate *delegate, bool isRowIdMode);
    static void ResultSetDb005(DistributedDB::KvStoreNbDelegate *delegate, bool isRowIdMode);
    static void ResultSetDb006(DistributedDB::KvStoreNbDelegate *delegate, bool isRowIdMode);
    static void ResultSetDb007(DistributedDB::KvStoreNbDelegate *delegate, bool isRowIdMode);
    static void ResultSetDb008(DistributedDB::KvStoreNbDelegate *delegate, bool isRowIdMode);
    static void ResultSetDb009(DistributedDB::KvStoreNbDelegate *delegate, bool isRowIdMode);
    static void ResultSetDb010(DistributedDB::KvStoreNbDelegate *delegate, bool isRowIdMode);
    static void ResultSetDb011(DistributedDB::KvStoreNbDelegate *delegate, bool isRowIdMode);
    static void ResultSetDb012(DistributedDB::KvStoreNbDelegate *delegate, bool isRowIdMode);
    static void ResultSetDb013(DistributedDB::KvStoreNbDelegate *delegate, bool isRowIdMode);
    static void ResultSetDb014(DistributedDB::KvStoreNbDelegate *delegate, bool isRowIdMode);
    static void ResultSetDb015(DistributedDB::KvStoreNbDelegate *delegate, bool isRowIdMode);
    static void ResultSetDb016(DistributedDB::KvStoreNbDelegate *delegate, bool isRowIdMode);
    static void ResultSetDb017(DistributedDB::KvStoreNbDelegate *delegate, bool isRowIdMode);
    static void ResultSetDb018(DistributedDB::KvStoreNbDelegate *delegate, bool isRowIdMode);
    static void ResultSetDb019(DistributedDB::KvStoreNbDelegate *delegate, bool isRowIdMode);
    static void ResultSetDb020(DistributedDB::KvStoreNbDelegate *delegate, bool isRowIdMode);
    static void ResultSetDb021(DistributedDB::KvStoreNbDelegate *delegate,
        DistributedDB::KvStoreDelegateManager *manager, bool isRowIdMode);
    static void ResultSetDb022(bool isRowIdMode);
    static void ResultSetDb023(bool isRowIdMode);
    static void ResultSetDb024(bool isRowIdMode);
    static void ResultSetDb025(DistributedDB::KvStoreNbDelegate *delegate, bool isRowIdMode);
    static void ResultSetDb026(DistributedDB::KvStoreNbDelegate *delegate, bool isRowIdMode);
    static void ResultSetDb027(bool isRowIdMode);
};
#endif // DISTRIBUTED_NB_CURSOR_TESTCASE_H