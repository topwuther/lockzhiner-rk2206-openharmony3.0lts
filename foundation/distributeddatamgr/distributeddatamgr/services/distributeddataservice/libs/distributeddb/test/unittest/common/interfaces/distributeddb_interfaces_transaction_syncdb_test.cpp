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

#include <gtest/gtest.h>

#include "distributeddb_data_generate_unit_test.h"
#include "distributeddb_tools_unit_test.h"
#include "distributeddb_interfaces_transaction_testcase.h"

using namespace testing::ext;
using namespace DistributedDB;
using namespace DistributedDBUnitTest;
using namespace std;

namespace {
    string g_testDir;
    const bool LOCAL_ONLY = false;
    const string STORE_ID = STORE_ID_SYNC;

    KvStoreDelegateManager g_mgr(APP_ID, USER_ID);
    KvStoreConfig g_config;

    // define the g_kvDelegateCallback, used to get some information when open a kv store.
    DBStatus g_kvDelegateStatus = INVALID_ARGS;
    KvStoreDelegate *g_kvDelegatePtr = nullptr;
    // the type of g_kvDelegateCallback is function<void(DBStatus, KvStoreDelegate*)>
    auto g_kvDelegateCallback = bind(&DistributedDBToolsUnitTest::KvStoreDelegateCallback, placeholders::_1,
        placeholders::_2, std::ref(g_kvDelegateStatus), std::ref(g_kvDelegatePtr));

    // define the g_snapshotDelegateCallback, used to get some information when open a kv snapshot.
    DBStatus g_snapshotDelegateStatus = INVALID_ARGS;
    KvStoreSnapshotDelegate *g_snapshotDelegatePtr = nullptr;
    // the type of g_snapshotDelegateCallback is function<void(DBStatus, KvStoreSnapshotDelegate*)>
    auto g_snapshotDelegateCallback = bind(&DistributedDBToolsUnitTest::SnapshotDelegateCallback,
        placeholders::_1, placeholders::_2, std::ref(g_snapshotDelegateStatus), std::ref(g_snapshotDelegatePtr));
}

class DistributedDBInterfacesTransactionSyncDBTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
};

void DistributedDBInterfacesTransactionSyncDBTest::SetUpTestCase(void)
{
    DistributedDBToolsUnitTest::TestDirInit(g_testDir);
    g_config.dataDir = g_testDir;
    g_mgr.SetKvStoreConfig(g_config);
}

void DistributedDBInterfacesTransactionSyncDBTest::TearDownTestCase(void)
{
    if (DistributedDBToolsUnitTest::RemoveTestDbFiles(g_testDir) != 0) {
        LOGE("rm test db files error!");
    }
}

void DistributedDBInterfacesTransactionSyncDBTest::SetUp(void)
{
    /*
     * Here, we create STORE_ID before test,
     * and it will be closed in TearDown().
     */
    KvStoreDelegate::Option option = {true, LOCAL_ONLY};
    g_mgr.GetKvStore(STORE_ID, option, g_kvDelegateCallback);
    EXPECT_TRUE(g_kvDelegateStatus == OK);
    ASSERT_TRUE(g_kvDelegatePtr != nullptr);
}

void DistributedDBInterfacesTransactionSyncDBTest::TearDown(void)
{
    if (g_kvDelegatePtr != nullptr && g_snapshotDelegatePtr != nullptr) {
        EXPECT_TRUE(g_kvDelegatePtr->ReleaseKvStoreSnapshot(g_snapshotDelegatePtr) == OK);
        g_snapshotDelegatePtr = nullptr;
    }

    if (g_kvDelegatePtr != nullptr) {
        EXPECT_EQ(g_mgr.CloseKvStore(g_kvDelegatePtr), OK);
        g_kvDelegatePtr = nullptr;
        EXPECT_EQ(g_mgr.DeleteKvStore(STORE_ID), OK);
    }
}

/**
  * @tc.name: StartTransaction001
  * @tc.desc: Test that can't call StartTransaction interface repeatedly.
  * @tc.type: FUNC
  * @tc.require: AR000BVRNK AR000CQDTO
  * @tc.author: huangnaigu
  */
HWTEST_F(DistributedDBInterfacesTransactionSyncDBTest, StartTransaction001, TestSize.Level1)
{
    /**
     * @tc.steps:step1. call StartTransaction interface the 1st time.
     * @tc.expected: step1. call succeed.
     */
    /**
     * @tc.steps:step2. call StartTransaction interface the 2nd time.
     * @tc.expected: step2. call failed and return ERROR.
     */
    DistributedDBInterfacesTransactionTestCase::StartTransaction001(g_kvDelegatePtr);
}

/**
  * @tc.name: StartTransaction002
  * @tc.desc: Test that call StartTransaction and commit interface normally.
  * @tc.type: FUNC
  * @tc.require: AR000BVRNK AR000CQDTO
  * @tc.author: huangnaigu
  */
HWTEST_F(DistributedDBInterfacesTransactionSyncDBTest, StartTransaction002, TestSize.Level1)
{
    /**
     * @tc.steps:step1. call StartTransaction interface.
     * @tc.expected: step1. call succeed.
     */
    /**
     * @tc.steps:step2. call commit interface.
     * @tc.expected: step2. call succeed.
     */
    DistributedDBInterfacesTransactionTestCase::StartTransaction002(g_kvDelegatePtr);
}

/**
  * @tc.name: StartTransaction003
  * @tc.desc: Test that call StartTransaction and rolback interface normally.
  * @tc.type: FUNC
  * @tc.require: AR000BVRNK AR000CQDTO
  * @tc.author: huangnaigu
  */
HWTEST_F(DistributedDBInterfacesTransactionSyncDBTest, StartTransaction003, TestSize.Level1)
{
    /**
     * @tc.steps:step1. call StartTransaction interface.
     * @tc.expected: step1. call succeed.
     */
    /**
     * @tc.steps:step2. call rollback interface.
     * @tc.expected: step2. call succeed.
     */
    DistributedDBInterfacesTransactionTestCase::StartTransaction003(g_kvDelegatePtr);
}

/**
  * @tc.name: StartTransaction004
  * @tc.desc: Test that call StartTransaction and rolback interface normally.
  * @tc.type: FUNC
  * @tc.require: AR000BVRNK AR000CQDTO
  * @tc.author: huangnaigu
  */
HWTEST_F(DistributedDBInterfacesTransactionSyncDBTest, StartTransaction004, TestSize.Level1)
{
    /**
     * @tc.steps:step1. call StartTransaction interface.
     * @tc.expected: step1. call succeed.
     */
    /**
     * @tc.steps:step2. put (k1, v1) to data base.
     * @tc.expected: step2. put succeed.
     */
    /**
     * @tc.steps:step3. close data base.
     * @tc.expected: step3. close succeed.
     */
    /**
     * @tc.steps:step4. use GetKvStore interface to open db.
     * @tc.expected: step4. open succeed.
     */
    /**
     * @tc.steps:step5. use snapshot interface to check the value of k1.
     * @tc.expected: step5. can't get the record of k1.
     */
    DistributedDBInterfacesTransactionTestCase::StartTransaction004(g_kvDelegatePtr, STORE_ID, LOCAL_ONLY,
        g_mgr, g_snapshotDelegatePtr);
}

/**
  * @tc.name: Commit001
  * @tc.desc: Test that can't commit Transaction before it start.
  * @tc.type: FUNC
  * @tc.require: AR000BVRNK AR000CQDTO
  * @tc.author: huangnaigu
  */
HWTEST_F(DistributedDBInterfacesTransactionSyncDBTest, Commit001, TestSize.Level1)
{
    /**
     * @tc.steps:step1. commit Transaction without start it.
     * @tc.expected: step1. commit failed and returned ERROR.
     */
    DistributedDBInterfacesTransactionTestCase::Commit001(g_kvDelegatePtr);
}

/**
  * @tc.name: Commit002
  * @tc.desc: Test that can't commit Transaction repeatedly even if it start normally.
  * @tc.type: FUNC
  * @tc.require: AR000BVRNK AR000CQDTO
  * @tc.author: huangnaigu
  */
HWTEST_F(DistributedDBInterfacesTransactionSyncDBTest, Commit002, TestSize.Level1)
{
    /**
     * @tc.steps:step1. call StartTransaction interface.
     * @tc.expected: step1. call succeed.
     */
    /**
     * @tc.steps:step2. call commit interface the 1st time.
     * @tc.expected: step2. call succeed.
     */
    /**
     * @tc.steps:step3. call commit interface the 2nd time.
     * @tc.expected: step3. call failed and returned ERROR.
     */
    DistributedDBInterfacesTransactionTestCase::Commit002(g_kvDelegatePtr);
}

/**
  * @tc.name: Commit003
  * @tc.desc: Test that can commit Transaction after put record.
  * @tc.type: FUNC
  * @tc.require: AR000BVRNK AR000CQDTO
  * @tc.author: huangnaigu
  */
HWTEST_F(DistributedDBInterfacesTransactionSyncDBTest, Commit003, TestSize.Level1)
{
    /**
     * @tc.steps:step1. call StartTransaction interface.
     * @tc.expected: step1. call succeed.
     */
    /**
     * @tc.steps:step2. put (k1, v1) to db.
     * @tc.expected: step2. put succeed.
     */
    /**
     * @tc.steps:step3. call commit interface.
     * @tc.expected: step3. call succeed.
     */
    /**
     * @tc.steps:step4. use snapshot interface to check if (k1, v1) is put succeed.
     * @tc.expected: step4. can find (k1, v1) from db.
     */
    DistributedDBInterfacesTransactionTestCase::Commit003(g_kvDelegatePtr, g_snapshotDelegatePtr);
}

/**
  * @tc.name: Commit004
  * @tc.desc: Test that can commit Transaction after update record.
  * @tc.type: FUNC
  * @tc.require: AR000BVRNK AR000CQDTO
  * @tc.author: huangnaigu
  */
HWTEST_F(DistributedDBInterfacesTransactionSyncDBTest, Commit004, TestSize.Level1)
{
    /**
     * @tc.steps:step1. call StartTransaction interface.
     * @tc.expected: step1. call succeed.
     */
    /**
     * @tc.steps:step2. update value = v2 where key = k1.
     * @tc.expected: step2. update succeed.
     */
    /**
     * @tc.steps:step3. call commit interface.
     * @tc.expected: step3. commit succeed.
     */
    /**
     * @tc.steps:step4. use snapshot interface to check if (k1, v2) is update succeed.
     * @tc.expected: step4. the value is v2 where key = k1 in the db.
     */
    DistributedDBInterfacesTransactionTestCase::Commit004(g_kvDelegatePtr, g_snapshotDelegatePtr);
}

/**
  * @tc.name: Commit005
  * @tc.desc: Test that can commit Transaction after delete record.
  * @tc.type: FUNC
  * @tc.require: AR000BVRNK AR000CQDTO
  * @tc.author: huangnaigu
  */
HWTEST_F(DistributedDBInterfacesTransactionSyncDBTest, Commit005, TestSize.Level1)
{
    /**
     * @tc.steps:step1. call StartTransaction interface.
     * @tc.expected: step1. call succeed.
     */
    /**
     * @tc.steps:step2. delete record from db where key = k1.
     * @tc.expected: step2. delete succeed.
     */
    /**
     * @tc.steps:step3. call commit interface.
     * @tc.expected: step3. commit succeed.
     */
    /**
     * @tc.steps:step4. use snapshot interface to check if (k1, v1) is delete succeed.
     * @tc.expected: step4. can't find (k1, v1) in the db.
     */
    DistributedDBInterfacesTransactionTestCase::Commit005(g_kvDelegatePtr, g_snapshotDelegatePtr);
}

/**
  * @tc.name: Commit006
  * @tc.desc: Test that can commit Transaction after clear all the records.
  * @tc.type: FUNC
  * @tc.require: AR000BVRNK AR000CQDTO
  * @tc.author: huangnaigu
  */
HWTEST_F(DistributedDBInterfacesTransactionSyncDBTest, Commit006, TestSize.Level1)
{
    /**
     * @tc.steps:step1. call StartTransaction interface.
     * @tc.expected: step1. call succeed.
     */
    /**
     * @tc.steps:step2. clear all the records from db.
     * @tc.expected: step2. clear succeed.
     */
    /**
     * @tc.steps:step3. call commit interface.
     * @tc.expected: step3. commit succeed.
     */
    /**
     * @tc.steps:step4. use snapshot interface to check if there are any data in db.
     * @tc.expected: step4. can't find any data in db.
     */
    DistributedDBInterfacesTransactionTestCase::Commit006(g_kvDelegatePtr, g_snapshotDelegatePtr);
}

/**
  * @tc.name: Commit007
  * @tc.desc: Test that can commit Transaction after delete and update db.
  * @tc.type: FUNC
  * @tc.require: AR000BVRNK AR000CQDTO
  * @tc.author: huangnaigu
  */
HWTEST_F(DistributedDBInterfacesTransactionSyncDBTest, Commit007, TestSize.Level1)
{
    /**
     * @tc.steps:step1. call StartTransaction interface.
     * @tc.expected: step1. call succeed.
     */
    /**
     * @tc.steps:step2. delete record from db where key = k1.
     * @tc.expected: step2. delete succeed.
     */
    /**
     * @tc.steps:step3. put (k2, v1) to db.
     * @tc.expected: step3. put succeed.
     */
    /**
     * @tc.steps:step4. call commit interface.
     * @tc.expected: step4. commit succeed.
     */
    /**
     * @tc.steps:step5. use snapshot interface to check the data in db.
     * @tc.expected: step5. can't find (k1, v1) but can find (k2, v1) in db.
     */
    DistributedDBInterfacesTransactionTestCase::Commit007(g_kvDelegatePtr, g_snapshotDelegatePtr);
}

/**
  * @tc.name: Commit008
  * @tc.desc: Test that can commit Transaction after clear and new add records.
  * @tc.type: FUNC
  * @tc.require: AR000BVRNK AR000CQDTO
  * @tc.author: huangnaigu
  */
HWTEST_F(DistributedDBInterfacesTransactionSyncDBTest, Commit008, TestSize.Level1)
{
    /**
     * @tc.steps:step1. call StartTransaction interface.
     * @tc.expected: step1. call succeed.
     */
    /**
     * @tc.steps:step2. clear all the records from db.
     * @tc.expected: step2. clear succeed.
     */
    /**
     * @tc.steps:step3. put (k3, v3) to db.
     * @tc.expected: step3. put succeed.
     */
    /**
     * @tc.steps:step4. call commit interface.
     * @tc.expected: step4. commit succeed.
     */
    /**
     * @tc.steps:step5. use snapshot interface to check the data in db.
     * @tc.expected: step5. can only find (k3, v3) in db.
     */
    DistributedDBInterfacesTransactionTestCase::Commit008(g_kvDelegatePtr, g_snapshotDelegatePtr);
}

/**
 * @tc.name: RollBack001
 * @tc.desc: Test if new commit records and logs generated
 *  when a transaction rollback-ed
 * @tc.type: FUNC
 * @tc.require: AR000BVRNM AR000CQDTQ
 * @tc.author: huangnaigu
 */
HWTEST_F(DistributedDBInterfacesTransactionSyncDBTest, RollBack001, TestSize.Level1)
{
    /**
     * @tc.steps:step1. Test g_kvDelegatePtr->Rollback
     * @tc.expected: step1. Return ERROR.
     */
    DistributedDBInterfacesTransactionTestCase::RollBack001(g_kvDelegatePtr);
}

/**
* @tc.name: RollBack002
* @tc.desc: rollback a transaction two times
* @tc.type: FUNC
* @tc.require: AR000BVRNM AR000CQDTQ
* @tc.author: huangnaigu
*/
HWTEST_F(DistributedDBInterfacesTransactionSyncDBTest, RollBack002, TestSize.Level1)
{
    /**
     * @tc.steps:step1. start a transaction
     * @tc.expected: step1. Return OK.
     */
    /**
     * @tc.steps:step2. rollback the transaction
     * @tc.expected: step2. Return OK.
     */
    /**
     * @tc.steps:step3. rollback the transaction the second time
     * @tc.expected: step3. Return ERROR.
     */
    DistributedDBInterfacesTransactionTestCase::RollBack002(g_kvDelegatePtr);
}

/**
 * @tc.name: RollBack003
 * @tc.desc: insert a data and rollback
 * @tc.type: FUNC
 * @tc.require: AR000BVRNM AR000CQDTQ
 * @tc.author: huangnaigu
 */
HWTEST_F(DistributedDBInterfacesTransactionSyncDBTest, RollBack003, TestSize.Level1)
{
    /**
     * @tc.steps:step1. start a transaction
     * @tc.expected: step1. Return OK.
     */
    /**
     * @tc.steps:step2. Put (k1,v1)
     * @tc.expected: step2. Return OK.
     */
    /**
     * @tc.steps:step3. rollback a transaction
     * @tc.expected: step3. Return OK.
     */
    /**
     * @tc.steps:step4. check if (k1,v1) exists
     * @tc.expected: step4. Return NOT_FOUND.
     */
    DistributedDBInterfacesTransactionTestCase::RollBack003(g_kvDelegatePtr, g_snapshotDelegatePtr);
}

/**
 * @tc.name: RollBack004
 * @tc.desc: update a data and rollback
 * @tc.type: FUNC
 * @tc.require: AR000BVRNM AR000CQDTQ
 * @tc.author: huangnaigu
 */
HWTEST_F(DistributedDBInterfacesTransactionSyncDBTest, RollBack004, TestSize.Level1)
{
    /**
     * @tc.steps:step1. Put (k1,v1)
     * @tc.expected: step1. Return OK.
     */
    /**
     * @tc.steps:step2. start a transaction
     * @tc.expected: step2. Return OK.
     */
    /**
     * @tc.steps:step3. Update (k1,v1) to (k1,v2) in the transaction
     * @tc.expected: step3. Return OK.
     */
    /**
     * @tc.steps:step4. rollback the transaction
     * @tc.expected: step4. Return OK.
     */
    /**
     * @tc.steps:step5. check the value of k1 is v1
     * @tc.expected: step5. verification is OK .
     */
    DistributedDBInterfacesTransactionTestCase::RollBack004(g_kvDelegatePtr, g_snapshotDelegatePtr);
}

/**
 * @tc.name: RollBack005
 * @tc.desc: delete a exist data and rollback
 * @tc.type: FUNC
 * @tc.require: AR000BVRNM AR000CQDTQ
 * @tc.author: huangnaigu
 */
HWTEST_F(DistributedDBInterfacesTransactionSyncDBTest, RollBack005, TestSize.Level1)
{
    /**
     * @tc.steps:step1. Put (k1,v1)
     * @tc.expected: step1. Return OK.
     */
    /**
     * @tc.steps:step2. start a transaction
     * @tc.expected: step2. Return OK.
     */
    /**
     * @tc.steps:step3. Delete (k1,v1) in the transaction
     * @tc.expected: step3. Return OK.
     */
    /**
     * @tc.steps:step4. rollback the transaction
     * @tc.expected: step4. Return OK.
     */
    /**
     * @tc.steps:step5. check the value of k1 is v1
     * @tc.expected: step5. verification is OK .
     */
    DistributedDBInterfacesTransactionTestCase::RollBack005(g_kvDelegatePtr, g_snapshotDelegatePtr);
}

/**
 * @tc.name: RollBack006
 * @tc.desc: clear db and rollback
 * @tc.type: FUNC
 * @tc.require: AR000BVRNM AR000CQDTQ
 * @tc.author: huangnaigu
 */
HWTEST_F(DistributedDBInterfacesTransactionSyncDBTest, RollBack006, TestSize.Level1)
{
    /**
     * @tc.steps:step1. PutBatch records: (k1,v1), (k2,v2)
     * @tc.expected: step1. Return OK.
     */
    /**
     * @tc.steps:step2. start a transaction
     * @tc.expected: step2. Return OK.
     */
    /**
     * @tc.steps:step3. Clear all records in the transaction
     * @tc.expected: step3. Return OK.
     */
    /**
     * @tc.steps:step4. rollback the transaction
     * @tc.expected: step4. Return OK.
     */
    /**
     * @tc.steps:step5. check if there are 2 records in the db
     * @tc.expected: step5. verification is OK .
     */
    DistributedDBInterfacesTransactionTestCase::RollBack006(g_kvDelegatePtr, g_snapshotDelegatePtr);
}

/**
 * @tc.name: RollBack007
 * @tc.desc: delete a exist data and update a data and rollback
 * @tc.type: FUNC
 * @tc.require: AR000BVRNM AR000CQDTQ
 * @tc.author: huangnaigu
 */
HWTEST_F(DistributedDBInterfacesTransactionSyncDBTest, RollBack007, TestSize.Level1)
{
    /**
     * @tc.steps:step1. PutBatch records: (k1,v1), (k2,v2)
     * @tc.expected: step1. Return OK.
     */
    /**
     * @tc.steps:step2. start a transaction
     * @tc.expected: step2. Return OK.
     */
    /**
     * @tc.steps:step3. Delete (k1,v1) in the transaction
     * @tc.expected: step3. Return OK.
     */
    /**
     * @tc.steps:step4. Update (k2,v2) to (k2,v1) in the transaction
     * @tc.expected: step4. Return OK.
     */
    /**
     * @tc.steps:step5. rollback the transaction
     * @tc.expected: step5. Return OK.
     */
    /**
     * @tc.steps:step6. check if (k1,v1),(k2,v2) exist and no more records in the db
     * @tc.expected: step6. verification is OK .
     */
    DistributedDBInterfacesTransactionTestCase::RollBack007(g_kvDelegatePtr, g_snapshotDelegatePtr);
}

/**
 * @tc.name: RollBack008
 * @tc.desc: clear db and insert a data and rollback
 * @tc.type: FUNC
 * @tc.require: AR000BVRNM AR000CQDTQ
 * @tc.author: huangnaigu
 */
HWTEST_F(DistributedDBInterfacesTransactionSyncDBTest, RollBack008, TestSize.Level1)
{
    /**
     * @tc.steps:step1. PutBatch records: (k1,v1), (k2,v2)
     * @tc.expected: step1. Return OK.
     */
    /**
     * @tc.steps:step2. start a transaction
     * @tc.expected: step2. Return OK.
     */
    /**
     * @tc.steps:step3. Clear all records in the transaction
     * @tc.expected: step3. Return OK.
     */
    /**
     * @tc.steps:step4. Put (012, ABC) in the transaction
     * @tc.expected: step4. Return OK.
     */
    /**
     * @tc.steps:step5. rollback the transaction
     * @tc.expected: step5. Return OK.
     */
    /**
     * @tc.steps:step6. check if (k1,v1),(k2,v2) exist and no more records in the db
     * @tc.expected: step6. verification is OK .
     */
    DistributedDBInterfacesTransactionTestCase::RollBack008(g_kvDelegatePtr, g_snapshotDelegatePtr);
}