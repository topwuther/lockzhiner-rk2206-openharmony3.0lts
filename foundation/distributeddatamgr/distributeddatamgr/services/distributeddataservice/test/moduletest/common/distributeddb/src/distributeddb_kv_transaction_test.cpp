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
#include <ctime>
#include <cmath>
#include <cstdlib>
#include <thread>
#include <cstdio>
#include <random>
#include <chrono>
#include <string>

#include "distributeddb_data_generator.h"
#include "distributed_test_tools.h"
#include "distributed_crud_transaction_tools.h"
#include "kv_store_delegate.h"
#include "kv_store_delegate_manager.h"
#include "types.h"

using namespace std;
using namespace testing;
#if defined TESTCASES_USING_GTEST_EXT
using namespace testing::ext;
#endif
using namespace DistributedDB;
using namespace DistributedDBDataGenerator;

namespace DistributeddbKvTransaction {
const bool IS_LOCAL = false;
const int REPEAT_TIMES = 5;
const long SLEEP_WHEN_CONSISTENCY_NOT_FINISHED = 20000;
const long SLEEP_ISOLATION_TRANSACTION = 5;
const unsigned long WAIT_FOR_CHECKING_SECONDS = 1;
const int BASIC_ACID_RUN_TIME = 1;

class DistributeddbKvTransactionTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
private:
};

KvStoreDelegate *g_transactionDelegate = nullptr; // the delegate used in this suit.
KvStoreDelegateManager *g_transactionManager = nullptr;
void DistributeddbKvTransactionTest::SetUpTestCase(void)
{
}

void DistributeddbKvTransactionTest::TearDownTestCase(void)
{
}

void DistributeddbKvTransactionTest::SetUp(void)
{
    MST_LOG("SetUpTestCase before all cases local[%d].", IS_LOCAL);
    RemoveDir(DIRECTOR);

    UnitTest *test = UnitTest::GetInstance();
    ASSERT_NE(test, nullptr);
    const TestInfo *testinfo = test->current_test_info();
    ASSERT_NE(testinfo, nullptr);
    string testCaseName = string(testinfo->name());
    MST_LOG("[SetUp] test case %s is start to run", testCaseName.c_str());

    g_transactionDelegate = DistributedTestTools::GetDelegateSuccess(g_transactionManager,
        g_kvdbParameter1, g_kvOption);
    ASSERT_TRUE(g_transactionManager != nullptr && g_transactionDelegate != nullptr);
}

void DistributeddbKvTransactionTest::TearDown(void)
{
    EXPECT_EQ(g_transactionManager->CloseKvStore(g_transactionDelegate), OK);
    g_transactionDelegate = nullptr;
    DBStatus status = g_transactionManager->DeleteKvStore(STORE_ID_1);
    EXPECT_EQ(status, DistributedDB::DBStatus::OK) << "fail to delete exist kvdb";
    delete g_transactionManager;
    g_transactionManager = nullptr;
    RemoveDir(DIRECTOR);
}

/*
 * @tc.name: BasicAction 001
 * @tc.desc: Verify that can start and commit a transaction successfully.
 * @tc.type: FUNC
 * @tc.require: SR000CQDTL
 * @tc.author: luqianfu
 */
HWTEST_F(DistributeddbKvTransactionTest, BasicAction001, TestSize.Level0)
{
    /**
     * @tc.steps: step1. start transaction .
     * @tc.expected: step1. start transaction successfully.
     */
    DBStatus statusStart = g_transactionDelegate->StartTransaction();
    EXPECT_TRUE(statusStart == DBStatus::OK);
    EXPECT_TRUE(g_transactionDelegate->Put(KEY_1, VALUE_1) == DBStatus::OK);
    EXPECT_TRUE(g_transactionDelegate->Put(KEY_2, VALUE_2) == DBStatus::OK);
    Value valueResult = DistributedTestTools::Get(*g_transactionDelegate, KEY_1);
    EXPECT_TRUE(valueResult.size() == 0);
    valueResult = DistributedTestTools::Get(*g_transactionDelegate, KEY_1);
    EXPECT_TRUE(valueResult.size() == 0);
    /**
     * @tc.steps: step2. commit transaction.
     * @tc.expected: step2. commit transaction successfully.
     */
    DBStatus statusCommit = g_transactionDelegate->Commit();
    EXPECT_TRUE(statusCommit == DBStatus::OK);
    valueResult = DistributedTestTools::Get(*g_transactionDelegate, KEY_1);
    EXPECT_TRUE(valueResult.size() != 0);
    EXPECT_TRUE(DistributedTestTools::IsValueEquals(valueResult, VALUE_1));
    valueResult = DistributedTestTools::Get(*g_transactionDelegate, KEY_2);
    EXPECT_TRUE(valueResult.size() != 0);
    EXPECT_TRUE(DistributedTestTools::IsValueEquals(valueResult, VALUE_2));
}

/*
 * @tc.name: BasicAction 002
 * @tc.desc: Verify that can start and rollback a transaction successfully.
 * @tc.type: FUNC
 * @tc.require: SR000CQDTL
 * @tc.author: luqianfu
 */
HWTEST_F(DistributeddbKvTransactionTest, BasicAction002, TestSize.Level0)
{
    /**
     * @tc.steps: step1. start transaction and Put (k1, v1) and check the record.
     * @tc.expected: step1. start transaction successfully and can't find the record in db.
     */
    DBStatus statusStart = g_transactionDelegate->StartTransaction();
    EXPECT_TRUE(statusStart == DBStatus::OK);
    EXPECT_TRUE(g_transactionDelegate->Put(KEY_1, VALUE_1) == DBStatus::OK);
    Value valueResult = DistributedTestTools::Get(*g_transactionDelegate, KEY_1);
    EXPECT_TRUE(valueResult.size() == 0);
    /**
     * @tc.steps: step2. rollback transaction and check the recordd of (k1, v1).
     * @tc.expected: step2. rollback transaction successfully and can't find the record in db still.
     */
    DBStatus statusRollback = g_transactionDelegate->Rollback();
    EXPECT_TRUE(statusRollback == DBStatus::OK);
    valueResult = DistributedTestTools::Get(*g_transactionDelegate, KEY_1);
    EXPECT_TRUE(valueResult.size() == 0);
}

/*
 * @tc.name: BasicAction 003
 * @tc.desc: Verify that can not start the same transaction repeatedly.
 * @tc.type: FUNC
 * @tc.require: SR000CQDTL
 * @tc.author: luqianfu
 */
HWTEST_F(DistributeddbKvTransactionTest, BasicAction003, TestSize.Level1)
{
    /**
     * @tc.steps: step1. start transaction.
     * @tc.expected: step1. start transaction successfully.
     */
    DBStatus status = g_transactionDelegate->StartTransaction();
    EXPECT_TRUE(status == DBStatus::OK);
    /**
     * @tc.steps: step2. start the same transaction again.
     * @tc.expected: step2. start transaction failed.
     */
    status = g_transactionDelegate->StartTransaction();
    EXPECT_EQ(status, DBStatus::DB_ERROR);

    status = g_transactionDelegate->Rollback();
    EXPECT_TRUE(status == DBStatus::OK);
}

/*
 * @tc.name: BasicAction 004
 * @tc.desc: Verify that can not commit transaction without starting it.
 * @tc.type: FUNC
 * @tc.require: SR000BUH3J
 * @tc.author: luqianfu
 */
HWTEST_F(DistributeddbKvTransactionTest, BasicAction004, TestSize.Level1)
{
    /**
     * @tc.steps: step1. start transaction then rollback.
     * @tc.expected: step1. Construct that has no transaction start.
     */
    EXPECT_EQ(DBStatus::OK, g_transactionDelegate->StartTransaction());
    EXPECT_EQ(DBStatus::OK, g_transactionDelegate->Rollback());
    /**
     * @tc.steps: step2. commit transaction directly.
     * @tc.expected: step2. Commit failed and return errcode correctly.
     */
    DBStatus status = g_transactionDelegate->Commit();
    EXPECT_EQ(status, DBStatus::DB_ERROR);
}

/*
 * @tc.name: BasicAction 005
 * @tc.desc: Verify that can not rollback transaction without starting it.
 * @tc.type: FUNC
 * @tc.require: SR000BUH3J
 * @tc.author: luqianfu
 */
HWTEST_F(DistributeddbKvTransactionTest, BasicAction005, TestSize.Level1)
{
    /**
     * @tc.steps: step1. rollback transaction which is not exist at all.
     * @tc.expected: step1. Rollback failed and return DB_ERROR.
     */
    DBStatus status = g_transactionDelegate->Rollback();
    EXPECT_EQ(status, DBStatus::DB_ERROR);
}

/*
 * @tc.name: BasicAction 006
 * @tc.desc: Verify that can not commit transaction repeatedly.
 * @tc.type: FUNC
 * @tc.require: SR000BUH3J
 * @tc.author: luqianfu
 */
HWTEST_F(DistributeddbKvTransactionTest, BasicAction006, TestSize.Level1)
{
    /**
     * @tc.steps: step1. start transaction.
     * @tc.expected: step1. start successfully.
     */
    DBStatus statusStart = g_transactionDelegate->StartTransaction();
    EXPECT_EQ(statusStart, DBStatus::OK);
    /**
     * @tc.steps: step2. commit transaction.
     * @tc.expected: step2. commit successfully.
     */
    DBStatus statusCommit = g_transactionDelegate->Commit();
    EXPECT_EQ(statusCommit, DBStatus::OK);
    /**
     * @tc.steps: step3. commit transaction again.
     * @tc.expected: step3. commit failed.
     */
    statusCommit = g_transactionDelegate->Commit();
    EXPECT_EQ(statusCommit, DBStatus::DB_ERROR);
}

/*
 * @tc.name: BasicAction 007
 * @tc.desc: Verify that can not rollback transaction after committing it.
 * @tc.type: FUNC
 * @tc.require: SR000BUH3J
 * @tc.author: luqianfu
 */
HWTEST_F(DistributeddbKvTransactionTest, BasicAction007, TestSize.Level1)
{
    /**
     * @tc.steps: step1. start transaction.
     * @tc.expected: step1. start successfully.
     */
    DBStatus statusStart = g_transactionDelegate->StartTransaction();
    EXPECT_TRUE(statusStart == DBStatus::OK);
    /**
     * @tc.steps: step2. commit transaction.
     * @tc.expected: step2. commit successfully.
     */
    DBStatus statusCommit = g_transactionDelegate->Commit();
    EXPECT_TRUE(statusCommit == DBStatus::OK);
    /**
     * @tc.steps: step3. rollback transaction.
     * @tc.expected: step3. rollback failed.
     */
    DBStatus statusRollback = g_transactionDelegate->Rollback();
    EXPECT_TRUE(statusRollback != DBStatus::OK);
}

/*
 * @tc.name: BasicAction 008
 * @tc.desc: Verify that can not commit transaction after rollabcking it.
 * @tc.type: FUNC
 * @tc.require: SR000BUH3J
 * @tc.author: luqianfu
 */
HWTEST_F(DistributeddbKvTransactionTest, BasicAction008, TestSize.Level1)
{
    /**
     * @tc.steps: step1. start transaction.
     * @tc.expected: step1. start successfully.
     */
    DBStatus statusStart = g_transactionDelegate->StartTransaction();
    EXPECT_TRUE(statusStart == DBStatus::OK);
    /**
     * @tc.steps: step2. rollback transaction.
     * @tc.expected: step2. rollback successfully.
     */
    DBStatus statusRollback = g_transactionDelegate->Rollback();
    EXPECT_TRUE(statusRollback == DBStatus::OK);
    /**
     * @tc.steps: step3. commit transaction.
     * @tc.expected: step3. commit failed.
     */
    DBStatus statusCommit = g_transactionDelegate->Commit();
    EXPECT_EQ(statusCommit, DBStatus::DB_ERROR);
}

/*
 * @tc.name: BasicAction 009
 * @tc.desc: Verify that can not rollback transaction repeatedly.
 * @tc.type: FUNC
 * @tc.require: SR000BUH3J
 * @tc.author: luqianfu
 */
HWTEST_F(DistributeddbKvTransactionTest, BasicAction009, TestSize.Level1)
{
    /**
     * @tc.steps: step1. start transaction.
     * @tc.expected: step1. start successfully.
     */
    DBStatus statusStart = g_transactionDelegate->StartTransaction();
    EXPECT_TRUE(statusStart == DBStatus::OK);
    /**
     * @tc.steps: step2. rollback transaction.
     * @tc.expected: step2. rollback successfully.
     */
    DBStatus statusRollback = g_transactionDelegate->Rollback();
    EXPECT_TRUE(statusRollback == DBStatus::OK);
    /**
     * @tc.steps: step3. rollback transaction again.
     * @tc.expected: step3. rollback failed.
     */
    statusRollback = g_transactionDelegate->Rollback();
    EXPECT_TRUE(statusRollback != DBStatus::OK);
}

/*
 * @tc.name: BasicAction 0010
 * @tc.desc: Verify that can start and commit, then start and rollback repeatedly.
 * @tc.type: FUNC
 * @tc.require: SR000BUH3J
 * @tc.author: luqianfu
 */
HWTEST_F(DistributeddbKvTransactionTest, BasicAction010, TestSize.Level1)
{
    /**
     * @tc.steps: step1. start transaction and put (k1, v1) to db and commit,
     *    then start and put (k2, v2) to db and rollback it for 5 times, and at the end check the data in db.
     * @tc.expected: step1. Operate successfully every time and (k1, v1) is in db and (k2, v2) is not.
     */
    for (int time = 0; time < REPEAT_TIMES; ++time) {
        DBStatus statusStart1 = g_transactionDelegate->StartTransaction();
        EXPECT_TRUE(g_transactionDelegate->Put(KEY_1, VALUE_1) == DBStatus::OK);
        DBStatus statusCommit = g_transactionDelegate->Commit();
        EXPECT_TRUE(statusStart1 == DBStatus::OK);
        EXPECT_TRUE(statusCommit == DBStatus::OK);

        DBStatus statusStart2 = g_transactionDelegate->StartTransaction();
        EXPECT_TRUE(g_transactionDelegate->Put(KEY_2, VALUE_2) == DBStatus::OK);
        DBStatus statusRollback = g_transactionDelegate->Rollback();
        EXPECT_TRUE(statusStart2 == DBStatus::OK);
        EXPECT_TRUE(statusRollback == DBStatus::OK);
    }
    Value valueResult = DistributedTestTools::Get(*g_transactionDelegate, KEY_1);
    EXPECT_TRUE(valueResult.size() != 0);
    EXPECT_TRUE(DistributedTestTools::IsValueEquals(valueResult, VALUE_1));
    valueResult = DistributedTestTools::Get(*g_transactionDelegate, KEY_2);
    EXPECT_TRUE(valueResult.size() == 0);
}

/*
 * @tc.name: Crud 001
 * @tc.desc: Verify that can insert and commit a transaction.
 * @tc.type: FUNC
 * @tc.require: SR000BUH3J
 * @tc.author: luqianfu
 */
HWTEST_F(DistributeddbKvTransactionTest, Crud001, TestSize.Level1)
{
    /**
     * @tc.steps: step1. start transaction.
     * @tc.expected: step1. start successfully.
     */
    DBStatus statusStart = g_transactionDelegate->StartTransaction();
    EXPECT_TRUE(statusStart == DBStatus::OK);
    /**
     * @tc.steps: step2. put(k1,v1) to db.
     * @tc.expected: step2. put successfully.
     */
    DBStatus statusPut = DistributedTestTools::Put(*g_transactionDelegate, KEY_1, VALUE_1);
    EXPECT_TRUE(statusPut == DBStatus::OK);
    /**
     * @tc.steps: step3. commit transaction and get.
     * @tc.expected: step3. commit successfully and get v1 of k1.
     */
    DBStatus statusCommit = g_transactionDelegate->Commit();
    EXPECT_TRUE(statusCommit == DBStatus::OK);
    Value value = DistributedTestTools::Get(*g_transactionDelegate, KEY_1);
    EXPECT_TRUE(DistributedTestTools::IsValueEquals(value, VALUE_1));
}

/*
 * @tc.name: Crud 002
 * @tc.desc: Verify that can update and commit a transaction.
 * @tc.type: FUNC
 * @tc.require: SR000BUH3J
 * @tc.author: luqianfu
 */
HWTEST_F(DistributeddbKvTransactionTest, Crud002, TestSize.Level1)
{
    /**
     * @tc.steps: step1.put(k1,v1) to db.
     * @tc.expected: step1. put successfully to construct exist data in db.
     */
    DBStatus statusPut = DistributedTestTools::Put(*g_transactionDelegate, KEY_1, VALUE_1);
    ASSERT_TRUE(statusPut == DBStatus::OK);
    Value value = DistributedTestTools::Get(*g_transactionDelegate, KEY_1);
    ASSERT_TRUE(DistributedTestTools::IsValueEquals(value, VALUE_1));
    /**
     * @tc.steps: step2. start transaction.
     * @tc.expected: step2. start successfully.
     */
    DBStatus statusStart = g_transactionDelegate->StartTransaction();
    EXPECT_TRUE(statusStart == DBStatus::OK);
    /**
     * @tc.steps: step3. update (k1,v1) to (k1,v2).
     * @tc.expected: step3. update successfully.
     */
    statusPut = DistributedTestTools::Put(*g_transactionDelegate, KEY_1, VALUE_2);
    EXPECT_TRUE(statusPut == DBStatus::OK);
    /**
     * @tc.steps: step4. commit transaction and get.
     * @tc.expected: step4. commit successfully and get v2 of k1.
     */
    DBStatus statusCommit = g_transactionDelegate->Commit();
    EXPECT_TRUE(statusCommit == DBStatus::OK);
    value = DistributedTestTools::Get(*g_transactionDelegate, KEY_1);
    ASSERT_TRUE(DistributedTestTools::IsValueEquals(value, VALUE_2));
}

/*
 * @tc.name: Crud 003
 * @tc.desc: Verify that can delete and commit a transaction.
 * @tc.type: FUNC
 * @tc.require: SR000BUH3J
 * @tc.author: luqianfu
 */
HWTEST_F(DistributeddbKvTransactionTest, Crud003, TestSize.Level0)
{
    /**
     * @tc.steps: step1.put(k1,v1) to db and get.
     * @tc.expected: step1. put successfully get v1 of k1.
     */
    DBStatus statusPut = DistributedTestTools::Put(*g_transactionDelegate, KEY_1, VALUE_1);
    ASSERT_TRUE(statusPut == DBStatus::OK);
    Value value = DistributedTestTools::Get(*g_transactionDelegate, KEY_1);
    ASSERT_TRUE(DistributedTestTools::IsValueEquals(value, VALUE_1));
    /**
     * @tc.steps: step2. start transaction.
     * @tc.expected: step2. start successfully.
     */
    DBStatus statusStart = g_transactionDelegate->StartTransaction();
    EXPECT_TRUE(statusStart == DBStatus::OK);
    /**
     * @tc.steps: step3. delete (k1) from db.
     * @tc.expected: step3. delete successfully.
     */
    statusPut = DistributedTestTools::Delete(*g_transactionDelegate, KEY_1);
    EXPECT_TRUE(statusPut == DBStatus::OK);
    /**
     * @tc.steps: step4. commit transaction and get.
     * @tc.expected: step4. commit successfully and get null.
     */
    DBStatus statusCommit = g_transactionDelegate->Commit();
    EXPECT_TRUE(statusCommit == DBStatus::OK);
    value = DistributedTestTools::Get(*g_transactionDelegate, KEY_1);
    EXPECT_TRUE(value.size() == 0);
}

/*
 * @tc.name: Crud 004
 * @tc.desc: Verify that can insert patch and commit a transaction.
 * @tc.type: FUNC
 * @tc.require: SR000BUH3J
 * @tc.author: luqianfu
 */
HWTEST_F(DistributeddbKvTransactionTest, Crud004, TestSize.Level1)
{
    DistributedTestTools::Clear(*g_transactionDelegate);
    vector<Entry> entries1;
    entries1.push_back(ENTRY_1);
    entries1.push_back(ENTRY_2);
    /**
     * @tc.steps: step1. start transaction.
     * @tc.expected: step1. start successfully.
     */
    DBStatus statusStart = g_transactionDelegate->StartTransaction();
    EXPECT_TRUE(statusStart == DBStatus::OK);
    /**
     * @tc.steps: step2. putBatch (k1,v1)(k2,v2) to db.
     * @tc.expected: step2. putBatch successfully.
     */
    DBStatus statusPut = DistributedTestTools::PutBatch(*g_transactionDelegate, entries1);
    EXPECT_TRUE(statusPut == DBStatus::OK);
    /**
     * @tc.steps: step3. commit transaction and get.
     * @tc.expected: step3. commit successfully and get v1 of k1, v2 of k2.
     */
    DBStatus statusCommit = g_transactionDelegate->Commit();
    EXPECT_TRUE(statusCommit == DBStatus::OK);
    Value valueResult = DistributedTestTools::Get(*g_transactionDelegate, KEY_1);
    EXPECT_TRUE(valueResult.size() != 0);
    EXPECT_TRUE(DistributedTestTools::IsValueEquals(valueResult, VALUE_1));
    valueResult = DistributedTestTools::Get(*g_transactionDelegate, KEY_2);
    EXPECT_TRUE(valueResult.size() != 0);
    EXPECT_TRUE(DistributedTestTools::IsValueEquals(valueResult, VALUE_2));
}

/*
 * @tc.name: Crud 005
 * @tc.desc: Verify that can update patch and commit a transaction.
 * @tc.type: FUNC
 * @tc.require: SR000BUH3J
 * @tc.author: luqianfu
 */
HWTEST_F(DistributeddbKvTransactionTest, Crud005, TestSize.Level1)
{
    DistributedTestTools::Clear(*g_transactionDelegate);
    vector<Entry> entries1;
    entries1.push_back(ENTRY_1);
    entries1.push_back(ENTRY_2);
    vector<Entry> entries1Up;
    entries1Up.push_back(ENTRY_1_2);
    entries1Up.push_back(ENTRY_2_3);
    /**
     * @tc.steps: step1. putBatch (k1,v1)(k2,v2) to db and get.
     * @tc.expected: step1. putBatch successfully and get v1 of k1, v2 of k2.
     */
    DBStatus status = DistributedTestTools::PutBatch(*g_transactionDelegate, entries1);
    EXPECT_TRUE(status == DBStatus::OK);
    Value valueResult = DistributedTestTools::Get(*g_transactionDelegate, KEY_1);
    EXPECT_TRUE(valueResult.size() != 0);
    EXPECT_TRUE(DistributedTestTools::IsValueEquals(valueResult, VALUE_1));
    valueResult = DistributedTestTools::Get(*g_transactionDelegate, KEY_2);
    EXPECT_TRUE(valueResult.size() != 0);
    EXPECT_TRUE(DistributedTestTools::IsValueEquals(valueResult, VALUE_2));
    /**
     * @tc.steps: step2. start transaction.
     * @tc.expected: step2. start successfully.
     */
    DBStatus statusStart = g_transactionDelegate->StartTransaction();
    EXPECT_TRUE(statusStart == DBStatus::OK);
    /**
     * @tc.steps: step3. update (k1,v1)(k2,v2) to (k1,v2)(k2,v3).
     * @tc.expected: step3. start successfully.
     */
    DBStatus statusPut = DistributedTestTools::PutBatch(*g_transactionDelegate, entries1Up);
    EXPECT_TRUE(statusPut == DBStatus::OK);
    /**
     * @tc.steps: step4. commit transaction and get.
     * @tc.expected: step4. commit successfully and get v2 of k1, v3 of k2.
     */
    DBStatus statusCommit = g_transactionDelegate->Commit();
    EXPECT_TRUE(statusCommit == DBStatus::OK);
    valueResult = DistributedTestTools::Get(*g_transactionDelegate, KEY_1);
    EXPECT_TRUE(valueResult.size() != 0);
    EXPECT_TRUE(DistributedTestTools::IsValueEquals(valueResult, VALUE_2));
    valueResult = DistributedTestTools::Get(*g_transactionDelegate, KEY_2);
    EXPECT_TRUE(valueResult.size() != 0);
    EXPECT_TRUE(DistributedTestTools::IsValueEquals(valueResult, VALUE_3));
}

/*
 * @tc.name: Crud 006
 * @tc.desc: Verify that can delete patch and commit a transaction.
 * @tc.type: FUNC
 * @tc.require: SR000BUH3J
 * @tc.author: luqianfu
 */
HWTEST_F(DistributeddbKvTransactionTest, Crud006, TestSize.Level1)
{
    DistributedTestTools::Clear(*g_transactionDelegate);
    vector<Entry> entries1;
    entries1.push_back(ENTRY_1);
    entries1.push_back(ENTRY_2);
    vector<Key> keys1;
    keys1.push_back(ENTRY_1.key);
    keys1.push_back(ENTRY_2.key);
    /**
     * @tc.steps: step1. putBatch (k1,v1)(k2,v2) to db and get.
     * @tc.expected: step1. putBatch successfully and get v1 of k1, v2 of k2.
     */
    DBStatus status = DistributedTestTools::PutBatch(*g_transactionDelegate, entries1);
    EXPECT_TRUE(status == DBStatus::OK);
    Value value = DistributedTestTools::Get(*g_transactionDelegate, KEY_1);
    EXPECT_TRUE(value.size() != 0);
    EXPECT_TRUE(DistributedTestTools::IsValueEquals(value, VALUE_1));
    value = DistributedTestTools::Get(*g_transactionDelegate, KEY_2);
    EXPECT_TRUE(value.size() != 0);
    EXPECT_TRUE(DistributedTestTools::IsValueEquals(value, VALUE_2));
    /**
     * @tc.steps: step2. start transaction.
     * @tc.expected: step2. start successfully.
     */
    DBStatus statusStart = g_transactionDelegate->StartTransaction();
    EXPECT_TRUE(statusStart == DBStatus::OK);
    /**
     * @tc.steps: step3. deleteBatch (k1)(k2).
     * @tc.expected: step3. deleteBatch successfully.
     */
    DBStatus statusPut = DistributedTestTools::DeleteBatch(*g_transactionDelegate, keys1);
    EXPECT_TRUE(statusPut == DBStatus::OK);
    /**
     * @tc.steps: step4. commit transaction and get.
     * @tc.expected: step4. commit successfully and get null of k1,k2.
     */
    DBStatus statusCommit = g_transactionDelegate->Commit();
    EXPECT_TRUE(statusCommit == DBStatus::OK);
    value = DistributedTestTools::Get(*g_transactionDelegate, KEY_1);
    EXPECT_TRUE(value.size() == 0);
    value = DistributedTestTools::Get(*g_transactionDelegate, KEY_2);
    EXPECT_TRUE(value.size() == 0);
}

/*
 * @tc.name: Crud 007
 * @tc.desc: Verify that can clear and commit a transaction.
 * @tc.type: FUNC
 * @tc.require: SR000BUH3J
 * @tc.author: luqianfu
 */
HWTEST_F(DistributeddbKvTransactionTest, Crud007, TestSize.Level1)
{
    DistributedTestTools::Clear(*g_transactionDelegate);
    vector<Entry> entries1;
    entries1.push_back(ENTRY_1);
    entries1.push_back(ENTRY_2);
    /**
     * @tc.steps: step1. putBatch (k1,v1)(k2,v2) to db.
     * @tc.expected: step1. putBatch successfully.
     */
    DBStatus status = DistributedTestTools::PutBatch(*g_transactionDelegate, entries1);
    EXPECT_TRUE(status == DBStatus::OK);
    /**
     * @tc.steps: step2. start transaction.
     * @tc.expected: step2. start successfully.
     */
    DBStatus statusStart = g_transactionDelegate->StartTransaction();
    EXPECT_TRUE(statusStart == DBStatus::OK);
    /**
     * @tc.steps: step3. clear db.
     * @tc.expected: step3. clear successfully.
     */
    DBStatus statusPut = DistributedTestTools::Clear(*g_transactionDelegate);
    EXPECT_TRUE(statusPut == DBStatus::OK);
    /**
     * @tc.steps: step4. commit transaction and get.
     * @tc.expected: step4. commit successfully and get null of k1,k2.
     */
    DBStatus statusCommit = g_transactionDelegate->Commit();
    EXPECT_TRUE(statusCommit == DBStatus::OK);
    Value valueResult = DistributedTestTools::Get(*g_transactionDelegate, KEY_1);
    EXPECT_TRUE(valueResult.size() == 0);
    valueResult = DistributedTestTools::Get(*g_transactionDelegate, KEY_2);
    EXPECT_TRUE(valueResult.size() == 0);
}

/*
 * @tc.name: Crud 008
 * @tc.desc: Verify that can insert then update and commit a transaction.
 * @tc.type: FUNC
 * @tc.require: SR000BUH3J
 * @tc.author: luqianfu
 */
HWTEST_F(DistributeddbKvTransactionTest, Crud008, TestSize.Level1)
{
    /**
     * @tc.steps: step1. start transaction.
     * @tc.expected: step1. start successfully.
     */
    DBStatus statusStart = g_transactionDelegate->StartTransaction();
    EXPECT_TRUE(statusStart == DBStatus::OK);
    /**
     * @tc.steps: step2. put (k1,v1) to db.
     * @tc.expected: step2. put successfully.
     */
    DBStatus statusPut1 = DistributedTestTools::Put(*g_transactionDelegate, KEY_1, VALUE_1);
    EXPECT_TRUE(statusPut1 == DBStatus::OK);
    /**
     * @tc.steps: step3. update (k1,v1) to (k1,v2).
     * @tc.expected: step3. update successfully.
     */
    DBStatus statusPut2 = DistributedTestTools::Put(*g_transactionDelegate, KEY_1, VALUE_2);
    EXPECT_TRUE(statusPut2 == DBStatus::OK);
    /**
     * @tc.steps: step4. commit transaction and get.
     * @tc.expected: step4. commit successfully and get v1 of k1, v2 of k2.
     */
    DBStatus statusCommit = g_transactionDelegate->Commit();
    EXPECT_TRUE(statusCommit == DBStatus::OK);
    Value value = DistributedTestTools::Get(*g_transactionDelegate, KEY_1);
    EXPECT_TRUE(DistributedTestTools::IsValueEquals(value, VALUE_2));
}

/*
 * @tc.name: Crud 009
 * @tc.desc: Verify that can complex update and commit a transaction.
 * @tc.type: FUNC
 * @tc.require: SR000BUH3J
 * @tc.author: luqianfu
 */
HWTEST_F(DistributeddbKvTransactionTest, Crud009, TestSize.Level1)
{
    DistributedTestTools::Clear(*g_transactionDelegate);
    vector<Entry> entries1, entries2;
    entries1.push_back(ENTRY_1);
    entries1.push_back(ENTRY_2);
    entries2.push_back(ENTRY_3);
    vector<Key> keys2;
    keys2.push_back(ENTRY_3.key);
    /**
     * @tc.steps: step1. start transaction.
     * @tc.expected: step1. start successfully.
     */
    DBStatus startStatus = g_transactionDelegate->StartTransaction();
    EXPECT_TRUE(startStatus == DBStatus::OK);
    /**
     * @tc.steps: step2. clear db.
     * @tc.expected: step2. clear successfully.
     */
    DBStatus statusClear = DistributedTestTools::Clear(*g_transactionDelegate);
    EXPECT_TRUE(statusClear == DBStatus::OK);
    /**
     * @tc.steps: step3. putBatch (k1,v1)(k2,v2) to db.
     * @tc.expected: step3. putBatch successfully.
     */
    DBStatus putBatchStatus1 = DistributedTestTools::PutBatch(*g_transactionDelegate, entries1);
    EXPECT_TRUE(putBatchStatus1 == DBStatus::OK);
    /**
     * @tc.steps: step4. putBatch (k3,v3) to db.
     * @tc.expected: step4. putBatch successfully.
     */
    DBStatus putBatchStatus2 = DistributedTestTools::PutBatch(*g_transactionDelegate, entries2);
    EXPECT_TRUE(putBatchStatus2 == DBStatus::OK);
    /**
     * @tc.steps: step5. deleteBatch entries2 from db.
     * @tc.expected: step5. deleteBatch successfully.
     */
    DBStatus statusDeleteBatch2 = DistributedTestTools::DeleteBatch(*g_transactionDelegate, keys2);
    EXPECT_TRUE(statusDeleteBatch2 == DBStatus::OK);
    /**
     * @tc.steps: step6. delete(k1)(k2) from db.
     * @tc.expected: step6. delete successfully.
     */
    DBStatus statusDelete1 = DistributedTestTools::Delete(*g_transactionDelegate, ENTRY_1.key);
    EXPECT_TRUE(statusDelete1 == DBStatus::OK);
    DBStatus statusDelete2 = DistributedTestTools::Delete(*g_transactionDelegate, ENTRY_2.key);
    EXPECT_TRUE(statusDelete2 == DBStatus::OK);
    /**
     * @tc.steps: step7. put(k1,v2) to db.
     * @tc.expected: step7. put successfully.
     */
    DBStatus statusPut = DistributedTestTools::Put(*g_transactionDelegate, ENTRY_1_2.key, ENTRY_1_2.value);
    EXPECT_TRUE(statusPut == DBStatus::OK);
    /**
     * @tc.steps: step8. commit transaction and get.
     * @tc.expected: step8. commit successfully and get v2 of k1, null of k2,k3.
     */
    DBStatus statusCommit = g_transactionDelegate->Commit();
    EXPECT_TRUE(statusCommit == DBStatus::OK);
    Value value = DistributedTestTools::Get(*g_transactionDelegate, KEY_1);
    EXPECT_TRUE(DistributedTestTools::IsValueEquals(value, VALUE_2));
    value = DistributedTestTools::Get(*g_transactionDelegate, KEY_2);
    EXPECT_TRUE(value.size() == 0);
    value = DistributedTestTools::Get(*g_transactionDelegate, KEY_3);
    EXPECT_TRUE(value.size() == 0);
}

/*
 * @tc.name: Pressure 001
 * @tc.desc: Verify that insert invalid key and commit a transaction will fail.
 * @tc.type: FUNC
 * @tc.require: SR000BUH3J
 * @tc.author: luqianfu
 */
HWTEST_F(DistributeddbKvTransactionTest, Pressure001, TestSize.Level1)
{
    DistributedTestTools::Clear(*g_transactionDelegate);
    vector<Entry> entries;
    entries.push_back(ENTRY_1);
    entries.push_back(ENTRY_2);
    /**
     * @tc.steps: step1. start transaction.
     * @tc.expected: step1. start successfully.
     */
    DBStatus statusStart = g_transactionDelegate->StartTransaction();
    EXPECT_TRUE(statusStart == DBStatus::OK);
    /**
     * @tc.steps: step2. putBatch (k1,v1)(k2,v2) to db.
     * @tc.expected: step2. putBatch successfully.
     */
    DBStatus statusPutBatch1 = DistributedTestTools::PutBatch(*g_transactionDelegate, entries);
    EXPECT_TRUE(statusPutBatch1 == DBStatus::OK);
    /**
     * @tc.steps: step3. put (null,null) to db.
     * @tc.expected: step3. put failed.
     */
    DBStatus statusPut = DistributedTestTools::Put(*g_transactionDelegate, KEY_EMPTY, VALUE_1);
    EXPECT_TRUE(statusPut != DBStatus::OK);
    /**
     * @tc.steps: step4. commit transaction and check the data on db.
     * @tc.expected: step4. commit successfully and there are only 2 records and equal to entries.
     */
    DBStatus statusCommit = g_transactionDelegate->Commit();
    EXPECT_TRUE(statusCommit == DBStatus::OK);

    std::vector<DistributedDB::Entry> entriesGot = DistributedTestTools::GetEntries(*g_transactionDelegate, KEY_EMPTY);
    EXPECT_EQ(entriesGot.size(), entries.size());
    EXPECT_TRUE(CompareEntriesVector(entriesGot, entries));
}

/*
 * @tc.name: Pressure 005
 * @tc.desc: Verify that support complex update and rollback a transaction.
 * @tc.type: FUNC
 * @tc.require: SR000BUH3J
 * @tc.author: luqianfu
 */
HWTEST_F(DistributeddbKvTransactionTest, Pressure005, TestSize.Level1)
{
    DistributedTestTools::Clear(*g_transactionDelegate);
    vector<Entry> entries1;
    entries1.push_back(ENTRY_1);
    entries1.push_back(ENTRY_2);
    vector<Entry> entries2;
    entries2.push_back(ENTRY_3);
    vector<Key> keys2;
    keys2.push_back(ENTRY_3.key);
    /**
     * @tc.steps: step1. start transaction.
     * @tc.expected: step1. start successfully.
     */
    DBStatus statusStart = g_transactionDelegate->StartTransaction();
    EXPECT_TRUE(statusStart == DBStatus::OK);
    /**
     * @tc.steps: step2. clear db.
     * @tc.expected: step2. clear successfully.
     */
    DBStatus statusClear = DistributedTestTools::Clear(*g_transactionDelegate);
    EXPECT_TRUE(statusClear == DBStatus::OK);
    /**
     * @tc.steps: step3. putBatch (k1,v1)(k2,v2) to db.
     * @tc.expected: step3. putBatch successfully.
     */
    DBStatus statusPutBatch1 = DistributedTestTools::PutBatch(*g_transactionDelegate, entries1);
    EXPECT_TRUE(statusPutBatch1 == DBStatus::OK);
    /**
     * @tc.steps: step4. putBatch (k3,v3) to db.
     * @tc.expected: step4. putBatch successfully.
     */
    DBStatus statusPutBatch2 = DistributedTestTools::PutBatch(*g_transactionDelegate, entries2);
    EXPECT_TRUE(statusPutBatch2 == DBStatus::OK);
    /**
     * @tc.steps: step5. deleteBatch (k3) to db.
     * @tc.expected: step5. deleteBatch successfully.
     */
    DBStatus statusDeleteBatch2 = DistributedTestTools::DeleteBatch(*g_transactionDelegate, keys2);
    EXPECT_TRUE(statusDeleteBatch2 == DBStatus::OK);
    /**
     * @tc.steps: step6. delete (k1)(k2) from db.
     * @tc.expected: step6. delete successfully.
     */
    DBStatus statusDelete1 = DistributedTestTools::Delete(*g_transactionDelegate, ENTRY_1.key);
    EXPECT_TRUE(statusDelete1 == DBStatus::OK);
    DBStatus statusDelete2 = DistributedTestTools::Delete(*g_transactionDelegate, ENTRY_2.key);
    EXPECT_TRUE(statusDelete2 == DBStatus::OK);
    /**
     * @tc.steps: step6. put (k1,v2) to db again.
     * @tc.expected: step6. put successfully.
     */
    DBStatus statusPut = DistributedTestTools::Put(*g_transactionDelegate, KEY_1, VALUE_2);
    EXPECT_TRUE(statusPut == DBStatus::OK);
    /**
     * @tc.steps: step6. rollback transaction and get.
     * @tc.expected: step6. rollback successfully and get null of k1,k2,k3.
     */
    DBStatus statusRollback = g_transactionDelegate->Rollback();
    EXPECT_TRUE(statusRollback == DBStatus::OK);
    std::vector<DistributedDB::Entry> entriesGot = DistributedTestTools::GetEntries(*g_transactionDelegate, KEY_EMPTY);
    EXPECT_TRUE(entriesGot.empty());
}

/*
 * @tc.name: Pressure 006
 * @tc.desc: Verify that insert invalid key and rollback a transaction successfully.
 * @tc.type: FUNC
 * @tc.require: SR000BUH3J
 * @tc.author: luqianfu
 */
HWTEST_F(DistributeddbKvTransactionTest, Pressure006, TestSize.Level1)
{
    DistributedTestTools::Clear(*g_transactionDelegate);
    vector<Entry> entries1;
    entries1.push_back(ENTRY_1);
    entries1.push_back(ENTRY_2);
    /**
     * @tc.steps: step1. start transaction.
     * @tc.expected: step1. start successfully.
     */
    DBStatus statusStart = g_transactionDelegate->StartTransaction();
    EXPECT_TRUE(statusStart == DBStatus::OK);
    /**
     * @tc.steps: step2. putBatch (k1,v1)(k2,v2) to db.
     * @tc.expected: step2. putBatch successfully.
     */
    DBStatus statusPutBatch1 = DistributedTestTools::PutBatch(*g_transactionDelegate, entries1);
    EXPECT_TRUE(statusPutBatch1 == DBStatus::OK);
    /**
     * @tc.steps: step3. put (k,v) that k=null to db.
     * @tc.expected: step3. put failed.
     */
    DBStatus statusPut = DistributedTestTools::Put(*g_transactionDelegate, KEY_EMPTY, VALUE_1);
    EXPECT_TRUE(statusPut != DBStatus::OK);
     /**
     * @tc.steps: step4. rollback transaction and get.
     * @tc.expected: step4. rollback successfully and get null of k1,k2.
     */
    DBStatus statusRollback = g_transactionDelegate->Rollback();
    EXPECT_TRUE(statusRollback == DBStatus::OK);
    std::vector<DistributedDB::Entry> entriesGot = DistributedTestTools::GetEntries(*g_transactionDelegate, KEY_EMPTY);
    EXPECT_TRUE(entriesGot.empty());
}

/*
 * @tc.name: Pressure 007
 * @tc.desc: Verify that start a transaction and close conn before rollback will result in transaction is invalid.
 * @tc.type: Fault injection
 * @tc.require: SR000BUH3J
 * @tc.author: luqianfu
 */
HWTEST_F(DistributeddbKvTransactionTest, Pressure007, TestSize.Level1)
{
    KvStoreDelegate *delegate = nullptr;
    KvStoreDelegateManager *manager = nullptr;
    delegate = DistributedTestTools::GetDelegateSuccess(manager, g_kvdbParameter2_1_2, g_kvOption);
    ASSERT_TRUE(manager != nullptr && delegate != nullptr);

    vector<Entry> entries;
    entries.push_back(ENTRY_1);
    entries.push_back(ENTRY_2);
    /**
     * @tc.steps: step1. start transaction.
     * @tc.expected: step1. start successfully.
     */
    DBStatus statusStart = delegate->StartTransaction();
    EXPECT_TRUE(statusStart == DBStatus::OK);
    /**
     * @tc.steps: step2. putBatch (k1,v1)(k2,v2) to db.
     * @tc.expected: step2. putBatch successfully.
     */
    DBStatus statusPutBatch = DistributedTestTools::PutBatch(*delegate, entries);
    EXPECT_TRUE(statusPutBatch == DBStatus::OK);
    /**
     * @tc.steps: step3. close db before rollback transaction.
     * @tc.expected: step3. close successfully.
     */
    DBStatus closeStatus = manager->CloseKvStore(delegate);
    EXPECT_TRUE(closeStatus == DBStatus::OK);
    delegate = nullptr;
    delete manager;
    manager = nullptr;
    /**
     * @tc.steps: step4. rollback transaction and get.
     * @tc.expected: step4. rollback failed and get null of k1,k2.
     */
    delegate = DistributedTestTools::GetDelegateSuccess(manager,
        g_kvdbParameter2_1_2, g_kvOption);
    ASSERT_TRUE(manager != nullptr && delegate != nullptr);
    DBStatus statusRollback = delegate->Rollback();
    EXPECT_TRUE(statusRollback != DBStatus::OK);
    if (delegate == nullptr) {
        MST_LOG("delegate nullptr");
    }
    Value value = DistributedTestTools::Get(*delegate, KEY_1);
    EXPECT_TRUE(value.size() == 0);
    value = DistributedTestTools::Get(*delegate, KEY_2);
    EXPECT_TRUE(value.size() == 0);

    closeStatus = manager->CloseKvStore(delegate);
    EXPECT_TRUE(closeStatus == DBStatus::OK);
    delegate = nullptr;
    EXPECT_EQ(manager->DeleteKvStore(STORE_ID_2), OK);
    delete manager;
    manager = nullptr;
}

/*
 * @tc.name: Acid 001
 * @tc.desc: Verify that single action's atomic.
 * @tc.type: FUNC
 * @tc.require: SR000BUH3J
 * @tc.author: luqianfu
 */
HWTEST_F(DistributeddbKvTransactionTest, Acid001, TestSize.Level1)
{
    DistributedTestTools::Clear(*g_transactionDelegate);
    DBStatus status = DistributedTestTools::Put(*g_transactionDelegate, KEY_1, VALUE_1);
    EXPECT_TRUE(status == DBStatus::OK);
    /**
     * @tc.steps: step1. start transaction.
     * @tc.expected: step1. start successfully.
     */
    DBStatus statusStart = g_transactionDelegate->StartTransaction();
    EXPECT_TRUE(statusStart == DBStatus::OK);
    /**
     * @tc.steps: step2. update (k1,v1) to (k2,v2).
     * @tc.expected: step2. update successfully.
     */
    DBStatus statusPut = DistributedTestTools::Put(*g_transactionDelegate, KEY_1, VALUE_2);
    EXPECT_TRUE(statusPut == DBStatus::OK);
    /**
     * @tc.steps: step3. get (k1).
     * @tc.expected: step3. get v1 of k1.
     */
    Value value = DistributedTestTools::Get(*g_transactionDelegate, KEY_1);
    EXPECT_TRUE(DistributedTestTools::IsValueEquals(value, VALUE_1));
    /**
     * @tc.steps: step4. delete (k1).
     * @tc.expected: step4. delete successfully.
     */
    DBStatus statusDelete1 = DistributedTestTools::Delete(*g_transactionDelegate, KEY_1);
    EXPECT_TRUE(statusDelete1 == DBStatus::OK);
    /**
     * @tc.steps: step5. get (k1).
     * @tc.expected: step5. get v1 of k1.
     */
    value = DistributedTestTools::Get(*g_transactionDelegate, KEY_1);
    EXPECT_TRUE(DistributedTestTools::IsValueEquals(value, VALUE_1));
    /**
     * @tc.steps: step6. put (k2,v2) to db.
     * @tc.expected: step6. put successfully.
     */
    DBStatus statusPut2 = DistributedTestTools::Put(*g_transactionDelegate, KEY_2, VALUE_2);
    EXPECT_TRUE(statusPut2 == DBStatus::OK);
    /**
     * @tc.steps: step7. get (k2).
     * @tc.expected: step7. get null of k2.
     */
    value = DistributedTestTools::Get(*g_transactionDelegate, KEY_2);
    EXPECT_TRUE(value.size() == 0);
    /**
     * @tc.steps: step7. commit transaction and get.
     * @tc.expected: step7. get null of k1, v2 of k2.
     */
    DBStatus statusCommit = g_transactionDelegate->Commit();
    EXPECT_TRUE(statusCommit == DBStatus::OK);
    value = DistributedTestTools::Get(*g_transactionDelegate, KEY_1);
    EXPECT_TRUE(value.size() == 0);
    value = DistributedTestTools::Get(*g_transactionDelegate, KEY_2);
    EXPECT_TRUE(DistributedTestTools::IsValueEquals(value, VALUE_2));
}

/*
 * @tc.name: Acid 002
 * @tc.desc: Verify that batch action's atomic.
 * @tc.type: FUNC
 * @tc.require: SR000BUH3J
 * @tc.author: luqianfu
 */
HWTEST_F(DistributeddbKvTransactionTest, Acid002, TestSize.Level1)
{
    DistributedTestTools::Clear(*g_transactionDelegate);
    vector<Entry> entries1, entries2, entries1Up;
    entries1.push_back(ENTRY_1);
    entries1.push_back(ENTRY_2);
    entries2.push_back(ENTRY_3);
    entries1Up.push_back(ENTRY_1_2);
    entries1Up.push_back(ENTRY_2_3);
    vector<Key> keys1;
    keys1.push_back(ENTRY_1.key);
    keys1.push_back(ENTRY_2.key);
    DBStatus status = DistributedTestTools::PutBatch(*g_transactionDelegate, entries1);
    EXPECT_TRUE(status == DBStatus::OK);
    /**
     * @tc.steps: step1. start transaction.
     * @tc.expected: step1. start successfully.
     */
    DBStatus statusStart = g_transactionDelegate->StartTransaction();
    EXPECT_TRUE(statusStart == DBStatus::OK);
    /**
     * @tc.steps: step2. updateBatch (k1,v2) to (k2,v3).
     * @tc.expected: step2. updateBatch successfully.
     */
    DBStatus statusPut = DistributedTestTools::PutBatch(*g_transactionDelegate, entries1Up);
    EXPECT_TRUE(statusPut == DBStatus::OK);
    /**
     * @tc.steps: step3. Get (k1)(k2).
     * @tc.expected: step3. get v1 of k1, v2 of k2.
     */
    Value value = DistributedTestTools::Get(*g_transactionDelegate, KEY_1);
    EXPECT_TRUE(DistributedTestTools::IsValueEquals(value, VALUE_1));
    value = DistributedTestTools::Get(*g_transactionDelegate, KEY_2);
    EXPECT_TRUE(DistributedTestTools::IsValueEquals(value, VALUE_2));
    /**
     * @tc.steps: step4. DeleteBatch (k1)(k2).
     * @tc.expected: step4. DeleteBatch successfully.
     */
    DBStatus statusDelete1 = DistributedTestTools::DeleteBatch(*g_transactionDelegate, keys1);
    EXPECT_TRUE(statusDelete1 == DBStatus::OK);
    /**
     * @tc.steps: step5. Get.
     * @tc.expected: step5. get v1 of k1, v2 of k2.
     */
    value = DistributedTestTools::Get(*g_transactionDelegate, KEY_1);
    EXPECT_TRUE(DistributedTestTools::IsValueEquals(value, VALUE_1));
    value = DistributedTestTools::Get(*g_transactionDelegate, KEY_2);
    EXPECT_TRUE(DistributedTestTools::IsValueEquals(value, VALUE_2));
    /**
     * @tc.steps: step6. putBatch (k3,v3) to db.
     * @tc.expected: step6. putBatch successfully.
     */
    DBStatus statusPut2 = DistributedTestTools::PutBatch(*g_transactionDelegate, entries2);
    EXPECT_TRUE(statusPut2 == DBStatus::OK);
    /**
     * @tc.steps: step7. get (k3).
     * @tc.expected: step7. get null 0f k3.
     */
    value = DistributedTestTools::Get(*g_transactionDelegate, KEY_3);
    EXPECT_TRUE(value.size() == 0);
    /**
     * @tc.steps: step8. commit transaction and get.
     * @tc.expected: step8. get null of k1,k2, v3 of k3.
     */
    DBStatus statusCommit = g_transactionDelegate->Commit();
    EXPECT_TRUE(statusCommit == DBStatus::OK);
    value = DistributedTestTools::Get(*g_transactionDelegate, KEY_1);
    EXPECT_TRUE(value.size() == 0);
    value = DistributedTestTools::Get(*g_transactionDelegate, KEY_2);
    EXPECT_TRUE(value.size() == 0);
    value = DistributedTestTools::Get(*g_transactionDelegate, KEY_3);
    EXPECT_TRUE(DistributedTestTools::IsValueEquals(value, VALUE_3));
}

int g_singleThreadComplete = 0;
bool g_singleThreadSuccess = true;
void ConsistencyCheck(Key kLeft, Key kRight)
{
    KvStoreDelegate *delegate = nullptr;
    KvStoreDelegateManager *delegateManager = nullptr;
    delegate = DistributedTestTools::GetDelegateSuccess(delegateManager,
        g_kvdbParameter1, g_kvOption);
    ASSERT_TRUE(delegateManager != nullptr && delegate != nullptr);

    while (g_singleThreadComplete < static_cast<int>(SINGLE_THREAD_NUM)) {
        KvStoreSnapshotDelegate *snapShot = DistributedTestTools::GetKvStoreSnapshot(*delegate);
        if (snapShot == nullptr) {
            MST_LOG("ConsistencyCheck get snapShot failed.");
            return;
        }
        Value left = DistributedTestTools::Get(*snapShot, kLeft);
        Value right = DistributedTestTools::Get(*snapShot, kRight);
        MST_LOG("ConsistencyCheck get sum %d,%d.", GetIntValue(left), GetIntValue(right));
        if (GetIntValue(left) + GetIntValue(right) != VALUE_SUM) {
            g_singleThreadSuccess = false;
            MST_LOG("ConsistencyCheck get sum %d,%d failed.", GetIntValue(left), GetIntValue(right));
            break;
        }
        EXPECT_EQ(delegate->ReleaseKvStoreSnapshot(snapShot), OK);
        snapShot = nullptr;
        std::this_thread::sleep_for(std::chrono::duration<float, std::milli>(FIFTY_MILI_SECONDS));
    }

    EXPECT_EQ(delegateManager->CloseKvStore(delegate), OK);
    delegate = nullptr;
    delete delegateManager;
    delegateManager = nullptr;
}

void ConsistencyBusiness(Key kLeft, Value vLeft, Key kRight, Value vRight)
{
    // wait 100 ms, let ConsistencyCheck begin
    std::this_thread::sleep_for(std::chrono::duration<float, std::milli>(HUNDRED_MILLI_SECONDS));
    KvStoreDelegate *delegate = nullptr;
    KvStoreDelegateManager *delegateManager = nullptr;
    delegate = DistributedTestTools::GetDelegateSuccess(delegateManager,
        g_kvdbParameter1, g_kvOption);
    ASSERT_TRUE(delegateManager != nullptr && delegate != nullptr);

    MST_LOG("ConsistencyBusiness put %d,%d.", GetIntValue(vLeft), GetIntValue(vRight));
    DBStatus statusStart = delegate->StartTransaction();
    std::this_thread::sleep_for(std::chrono::duration<float, std::milli>(FIFTY_MILI_SECONDS));
    DBStatus statusPutLeft = delegate->Put(kLeft, vLeft);
    std::this_thread::sleep_for(std::chrono::duration<float, std::milli>(FIFTY_MILI_SECONDS));
    DBStatus statusPutRight = delegate->Put(kRight, vRight);
    std::this_thread::sleep_for(std::chrono::duration<float, std::milli>(FIFTY_MILI_SECONDS));
    DBStatus statusCommit = delegate->Commit();
    std::this_thread::sleep_for(std::chrono::duration<float, std::milli>(FIFTY_MILI_SECONDS));
    if (statusPutLeft != DBStatus::OK || statusPutRight != DBStatus::OK ||
        statusStart != DBStatus::OK || statusCommit != DBStatus::OK) {
        MST_LOG("ConsistencyBusiness put failed.");
        g_singleThreadComplete++;
        g_singleThreadSuccess = false;
        return;
    }

    g_singleThreadComplete++;
    EXPECT_EQ(delegateManager->CloseKvStore(delegate), OK);
    delegate = nullptr;
    delete delegateManager;
    delegateManager = nullptr;
}

/*
 * @tc.name: Acid 003
 * @tc.desc: Verify that single action's consistency.
 * @tc.type: FUNC
 * @tc.require: SR000BUH3J
 * @tc.author: luqianfu
 */
HWTEST_F(DistributeddbKvTransactionTest, Acid003, TestSize.Level1)
{
    DistributedTestTools::Clear(*g_transactionDelegate);

    int baseNumer = VALUE_FIVE_HUNDRED;
    Value base = GetValueWithInt(baseNumer);

    Entry entry1, entry2;
    vector<Entry> entries1;
    entry1 = {KEY_CONS_1, base};
    entry2 = {KEY_CONS_2, base};
    entries1.push_back(entry1);
    entries1.push_back(entry2);
    /**
     * @tc.steps: step1. putBatch (k1=cons1,v1=500) (k2=cons2,v2=500).
     * @tc.expected: step1. putBatch successfully to construct exist v1+v2=1000.
     */
    DBStatus status = DistributedTestTools::PutBatch(*g_transactionDelegate, entries1);
    EXPECT_TRUE(status == DBStatus::OK);
    Value value = DistributedTestTools::Get(*g_transactionDelegate, entry1.key);
    EXPECT_EQ(GetIntValue(value), baseNumer);
    value = DistributedTestTools::Get(*g_transactionDelegate, entry2.key);
    EXPECT_EQ(GetIntValue(value), baseNumer);

    /**
     * @tc.steps: step2. start a thread to update k1=cons1's value to 400, update k2=cons2's value to 600.
     * @tc.expected: step2. run thread successfully and k1+k2=1000 is true in child thread.
     */
    std::vector<std::thread> bussinessThreads;
    bussinessThreads.push_back(std::thread(ConsistencyBusiness, KEY_CONS_1, GetValueWithInt(VALUE_CHANGE1_FIRST),
        KEY_CONS_2, GetValueWithInt(VALUE_CHANGE1_SECOND)));
    /**
     * @tc.steps: step3. start another thread to update k1=cons1's value to 700, update k2=cons2's value to 300.
     * @tc.expected: step3. run thread successfully and k1+k2=1000 is true in child thread.
     */
    bussinessThreads.push_back(std::thread(ConsistencyBusiness, KEY_CONS_1, GetValueWithInt(VALUE_CHANGE2_FIRST),
        KEY_CONS_2, GetValueWithInt(VALUE_CHANGE2_SECOND)));
    for (auto& th : bussinessThreads) {
        th.detach();
    }

    ConsistencyCheck(KEY_CONS_1, KEY_CONS_2);
    ASSERT_TRUE(g_singleThreadSuccess);
}

bool g_batchThreadSuccess = true;
bool g_batchThreadComplete = false;
void ConsistencyBatchCheck(vector<Key> *kLeft, vector<Key> *kRight, int size)
{
    KvStoreDelegate *delegate = nullptr;
    KvStoreDelegateManager *delegateManager = nullptr;
    delegate = DistributedTestTools::GetDelegateSuccess(delegateManager, g_kvdbParameter1, g_kvOption);
    ASSERT_TRUE(delegateManager != nullptr && delegate != nullptr);

    while (!g_batchThreadComplete) {
        KvStoreSnapshotDelegate *snapShot = DistributedTestTools::GetKvStoreSnapshot(*delegate);
        if (snapShot == nullptr) {
            MST_LOG("ConsistencyBatchCheck get snapShot failed.");
            return;
        }
        for (int i = 0; i < size; ++i) {
            Value left = DistributedTestTools::Get(*snapShot, kLeft->at(i));
            Value right = DistributedTestTools::Get(*snapShot, kRight->at(i));
            if (GetIntValue(left) + GetIntValue(right) != size) {
                g_batchThreadSuccess = false;
                MST_LOG("ConsistencyBatchCheck get sum %d failed.", size);
                break;
            }
        }
        EXPECT_EQ(delegate->ReleaseKvStoreSnapshot(snapShot), OK);
        snapShot = nullptr;
    }

    MST_LOG("g_batchThreadComplete.");
    EXPECT_EQ(delegateManager->CloseKvStore(delegate), OK);
    delegate = nullptr;
    delete delegateManager;
    delegateManager = nullptr;
}

void TransactionCheckConsistency(vector<Entry> &entries1, vector<Key> &allKeys1,
    vector<Entry> &entries2, vector<Key> &allKeys2)
{
    Key query1 = {'k', 'a'};
    Key query2 = {'k', 'b'};
    /**
     * @tc.steps: step1. putBatch 20 items of (keys1,values1)(key2,values2).
     * @tc.expected: step1. putBatch successfully to construct exist data in db.
     */
    DBStatus status = DistributedTestTools::PutBatch(*g_transactionDelegate, entries1);
    EXPECT_TRUE(status == DBStatus::OK);
    status = DistributedTestTools::PutBatch(*g_transactionDelegate, entries2);
    EXPECT_TRUE(status == DBStatus::OK);

    /**
     * @tc.steps: step2. start child thread to query keys1 and keys2 continuously.
     * @tc.expected: step2. if keys1.size()+keys2.size()!=20 print info.
     */
    thread readThread = thread(ConsistencyBatchCheck, &allKeys1, &allKeys2, TWENTY_RECORDS);
    readThread.detach();
    /**
     * @tc.steps: step3. start transaction.
     * @tc.expected: step3. start transaction successfully.
     */
    status = g_transactionDelegate->StartTransaction();
    EXPECT_TRUE(status == DBStatus::OK);
    /**
     * @tc.steps: step4. getEntries with keyprefix before updateBatch.
     * @tc.expected: step4. getEntries successfully that the size of them is 20.
     */
    vector<Entry> values1 = DistributedTestTools::GetEntries(*g_transactionDelegate, query1);
    vector<Entry> values2 = DistributedTestTools::GetEntries(*g_transactionDelegate, query2);
    ASSERT_EQ(static_cast<int>(values1.size()), TWENTY_RECORDS);
    ASSERT_EQ(static_cast<int>(values2.size()), TWENTY_RECORDS);
    /**
     * @tc.steps: step5. updateBatch values1+1 of keys1, values2-1 of keys2.
     * @tc.expected: step5. updateBatch successfully.
     */
    for (int i = 0; i != TWENTY_RECORDS; ++i) {
        entries1[i].value = GetValueWithInt(GetIntValue(values1[i].value) + 1);
        entries2[i].value = GetValueWithInt(GetIntValue(values2[i].value) - 1);
    }
    status = DistributedTestTools::PutBatch(*g_transactionDelegate, entries1);
    EXPECT_TRUE(status == DBStatus::OK);
    status = DistributedTestTools::PutBatch(*g_transactionDelegate, entries2);
    EXPECT_TRUE(status == DBStatus::OK);
    /**
     * @tc.steps: step6. updateBatch values1+2 of keys1, values2-2 of keys2.
     * @tc.expected: step6. updateBatch successfully.
     */
    for (int i = 0; i != TWENTY_RECORDS; ++i) {
        entries1[i].value = GetValueWithInt(GetIntValue(values1[i].value) + EVEN_NUMBER);
        entries2[i].value = GetValueWithInt(GetIntValue(values2[i].value) - EVEN_NUMBER);
    }
    status = DistributedTestTools::PutBatch(*g_transactionDelegate, entries1);
    EXPECT_TRUE(status == DBStatus::OK);
    status = DistributedTestTools::PutBatch(*g_transactionDelegate, entries2);
    EXPECT_TRUE(status == DBStatus::OK);
    /**
     * @tc.steps: step7. commit transaction and stop child thread.
     * @tc.expected: step7. operate successfully.
     */
    status = g_transactionDelegate->Commit();
    EXPECT_TRUE(status == DBStatus::OK);
    MST_LOG("after commit");
}

/*
 * @tc.name: Acid 004
 * @tc.desc: Verify that batch action's consistency.
 * @tc.type: FUNC
 * @tc.require: SR000BUH3J
 * @tc.author: luqianfu
 */
HWTEST_F(DistributeddbKvTransactionTest, Acid004, TestSize.Level2)
{
    DistributedTestTools::Clear(*g_transactionDelegate);

    uint8_t left = 'a';
    uint8_t right = 'b';
    vector<Entry> entries1;
    vector<Key> allKeys1;
    GenerateRecords(TWENTY_RECORDS, DEFAULT_START, allKeys1, entries1);
    vector<Entry> entries2;
    vector<Key> allKeys2;
    GenerateRecords(TWENTY_RECORDS, DEFAULT_START, allKeys2, entries2);
    for (int i = 0; i < TWENTY_RECORDS; ++i) {
        entries1[i].key.insert(entries1[i].key.begin() + 1, left);
        entries2[i].key.insert(entries2[i].key.begin() + 1, right);
        allKeys1[i].insert(allKeys1[i].begin() + 1, left);
        allKeys2[i].insert(allKeys2[i].begin() + 1, right);
        entries1[i].value = GetValueWithInt(i);
        entries2[i].value = GetValueWithInt(TWENTY_RECORDS - i);
    }

    TransactionCheckConsistency(entries1, allKeys1, entries2, allKeys2);

    g_batchThreadComplete = true;
    ASSERT_TRUE(g_batchThreadSuccess);

    std::this_thread::sleep_for(std::chrono::duration<float, std::milli>(MILLSECONDES_PER_SECOND));
}

vector<Key> g_holdingKeys;
std::mutex g_holdingMutex;
bool CheckKeyHolding(Key applyKey)
{
    std::unique_lock<std::mutex> lock(g_holdingMutex);
    for (auto key = g_holdingKeys.begin(); key != g_holdingKeys.end(); ++key) {
        if (CompareVector(applyKey, *key)) {
            string str;
            Uint8VecToString(*key, str);
            MST_LOG("key %s is hold.", str.c_str());
            return false;
        }
    }
    g_holdingKeys.push_back(applyKey);
    return true;
}

Key GetKey(KvStoreDelegate *&delegate, KvStoreSnapshotDelegate *&snapShot, vector<Key> *&allKeys)
{
    Key num = KEY_EMPTY;
    Value ticket;

    for (auto ticketNum = allKeys->begin(); ticketNum != allKeys->end(); ++ticketNum) {
        ticket = DistributedTestTools::Get(*snapShot, *ticketNum);
        if (GetIntValue(ticket) == 1) {
            if (!CheckKeyHolding(*ticketNum)) {
                continue;
            }
            num = *ticketNum;
            break;
        }
    }
    return num;
}

void IsolationBussiness(vector<Key> *allKeys, Key key)
{
    KvStoreDelegate *delegate = nullptr;
    KvStoreDelegateManager *delegateManager = nullptr;
    delegate = DistributedTestTools::GetDelegateSuccess(delegateManager, g_kvdbParameter1, g_kvOption);
    ASSERT_TRUE(delegateManager != nullptr && delegate != nullptr);
    bool hasTickets = true;
    while (hasTickets) {
        KvStoreSnapshotDelegate *snapShot = DistributedTestTools::GetKvStoreSnapshot(*delegate);
        if (snapShot == nullptr) {
            MST_LOG("IsolationBussiness get snapShot failed.");
            return;
        }
        Key num = GetKey(delegate, snapShot, allKeys);
        if (num == KEY_EMPTY) {
            MST_LOG("IsolationBussiness there have no tickets.");
            hasTickets = false;
            EXPECT_EQ(delegate->ReleaseKvStoreSnapshot(snapShot), OK);
            snapShot = nullptr;
            continue;
        }
        Value result = DistributedTestTools::Get(*snapShot, key);
        int resultPlus = GetIntValue(result) + 1;
        DBStatus statusStart = delegate->StartTransaction();
        std::this_thread::sleep_for(std::chrono::duration<float, std::milli>(SLEEP_ISOLATION_TRANSACTION));
        DBStatus statusGetTicket = delegate->Put(num, GetValueWithInt(0));
        DBStatus statusPut = delegate->Put(key, GetValueWithInt(resultPlus));
        std::this_thread::sleep_for(std::chrono::duration<float, std::milli>(SLEEP_ISOLATION_TRANSACTION));
        DBStatus statusCommit = delegate->Commit();
        std::this_thread::sleep_for(std::chrono::duration<float, std::milli>(SLEEP_ISOLATION_TRANSACTION));
        if (statusPut != DBStatus::OK || statusGetTicket != DBStatus::OK || statusStart != DBStatus::OK ||
            statusCommit != DBStatus::OK) {
            MST_LOG("IsolationBussiness put failed.");
            EXPECT_EQ(delegate->ReleaseKvStoreSnapshot(snapShot), OK);
            snapShot = nullptr;
            return;
        }

        EXPECT_EQ(delegate->ReleaseKvStoreSnapshot(snapShot), OK);
        snapShot = nullptr;
    }
    EXPECT_EQ(delegateManager->CloseKvStore(delegate), OK);
    delegate = nullptr;
    delete delegateManager;
    delegateManager = nullptr;
}

/*
 * @tc.name: Acid 005
 * @tc.desc: Verify that action's IsolationBussiness.
 * @tc.type: FUNC
 * @tc.require: SR000BUH3J
 * @tc.author: luqianfu
 */
HWTEST_F(DistributeddbKvTransactionTest, Acid005, TestSize.Level3)
{
    EXPECT_EQ(DistributedTestTools::Clear(*g_transactionDelegate), OK);
    /**
     * @tc.steps: step1. putBatch 20 items of (keys1,values1) where values=1.
     * @tc.expected: step1. putBatch successfully to construct exist data in db.
     */
    vector<Entry> entriesBatch;
    vector<Key> allKeys;
    GenerateRecords(TWENTY_RECORDS, DEFAULT_START, allKeys, entriesBatch);
    vector<Key> randomKeys = GetKeysFromEntries(entriesBatch, true);
    vector<Key> inversionKeys;
    inversionKeys.assign(allKeys.rbegin(), allKeys.rend());

    for (int i = 0; i < TWENTY_RECORDS; ++i) {
        entriesBatch[i].value = GetValueWithInt(1);
    }
    /**
     * @tc.steps: step2. start 3 threads to query the items of value=1, if find then item+1.
     * @tc.expected: step2. put the result to result1,2,3 of thread1,2,3.
     */
    DBStatus status = DistributedTestTools::PutBatch(*g_transactionDelegate, entriesBatch);
    EXPECT_TRUE(status == DBStatus::OK);
    status = DistributedTestTools::Put(*g_transactionDelegate, KEY_BATCH_CONS_1, GetValueWithInt(0));
    EXPECT_TRUE(status == DBStatus::OK);
    status = DistributedTestTools::Put(*g_transactionDelegate, KEY_BATCH_CONS_2, GetValueWithInt(0));
    EXPECT_TRUE(status == DBStatus::OK);
    status = DistributedTestTools::Put(*g_transactionDelegate, KEY_BATCH_CONS_3, GetValueWithInt(0));
    EXPECT_TRUE(status == DBStatus::OK);
    vector<thread> bussinessThreads;
    bussinessThreads.push_back(thread(IsolationBussiness, &allKeys, KEY_BATCH_CONS_1));
    bussinessThreads.push_back(thread(IsolationBussiness, &inversionKeys, KEY_BATCH_CONS_2));
    bussinessThreads.push_back(thread(IsolationBussiness, &randomKeys, KEY_BATCH_CONS_3));
    for (auto& th : bussinessThreads) {
        th.join();
    }
    Value result1 = DistributedTestTools::Get(*g_transactionDelegate, KEY_BATCH_CONS_1);
    Value result2 = DistributedTestTools::Get(*g_transactionDelegate, KEY_BATCH_CONS_2);
    Value result3 = DistributedTestTools::Get(*g_transactionDelegate, KEY_BATCH_CONS_3);
    /**
     * @tc.steps: step3. calculate the sum of result1,result2 and result3.
     * @tc.expected: step3. the sum is equle 20.
     */
    int sum = GetIntValue(result1) + GetIntValue(result2) + GetIntValue(result3);
    MST_LOG("sum %d", sum);
    ASSERT_EQ(sum, TWENTY_RECORDS);
    std::this_thread::sleep_for(std::chrono::duration<float, std::milli>(SLEEP_WHEN_CONSISTENCY_NOT_FINISHED));
    g_holdingKeys.clear();
}

/*
 * @tc.name: Acid 006
 * @tc.desc: Verify that action's PersistenceBussiness.
 * @tc.type: FUNC
 * @tc.require: SR000BUH3J
 * @tc.author: luqianfu
 */
HWTEST_F(DistributeddbKvTransactionTest, Acid006, TestSize.Level1)
{
    DistributedTestTools::Clear(*g_transactionDelegate);

    KvStoreDelegate *delegate = nullptr;
    KvStoreDelegateManager *delegateManager = nullptr;
    delegate = DistributedTestTools::GetDelegateSuccess(delegateManager, g_kvdbParameter1, g_kvOption);
    ASSERT_TRUE(delegateManager != nullptr && delegate != nullptr);
    vector<Entry> entries1;
    entries1.push_back(ENTRY_1);
    entries1.push_back(ENTRY_2);

    /**
     * @tc.steps: step1. start transaction.
     * @tc.expected: step1. start successfully.
     */
    DBStatus statusStart = delegate->StartTransaction();
    EXPECT_TRUE(statusStart == DBStatus::OK);
    /**
     * @tc.steps: step2. putBatch (k1,v1)(k2,v2) to db.
     * @tc.expected: step2. putBatch successfully.
     */
    DBStatus statusPut = delegate->PutBatch(entries1);
    EXPECT_TRUE(statusPut == DBStatus::OK);
    /**
     * @tc.steps: step3. commit transaction.
     * @tc.expected: step3. commit successfully.
     */
    DBStatus statusCommit = delegate->Commit();
    EXPECT_TRUE(statusCommit == DBStatus::OK);
    /**
     * @tc.steps: step4. close kv db then open a new db.
     * @tc.expected: step4. operate successfully.
     */
    DBStatus statusClose = delegateManager->CloseKvStore(delegate);
    EXPECT_TRUE(statusClose == DBStatus::OK);
    delegate = nullptr;
    KvStoreDelegate *delegateNew = nullptr;
    KvStoreDelegateManager *delegateManagerNew = nullptr;
    delegateNew = DistributedTestTools::GetDelegateSuccess(delegateManagerNew, g_kvdbParameter1, g_kvOption);
    ASSERT_TRUE(delegateManagerNew != nullptr && delegateNew != nullptr) << "fail to create exist kvdb";
    /**
     * @tc.steps: step5. get (k1)(k2).
     * @tc.expected: step5. get v1 of k1, v2 of k2.
     */
    Value valueResult = DistributedTestTools::Get(*delegateNew, KEY_1);
    EXPECT_TRUE(DistributedTestTools::IsValueEquals(valueResult, VALUE_1));
    valueResult = DistributedTestTools::Get(*delegateNew, KEY_2);
    EXPECT_TRUE(DistributedTestTools::IsValueEquals(valueResult, VALUE_2));

    statusClose = delegateManagerNew->CloseKvStore(delegateNew);
    delegateNew = nullptr;
    EXPECT_TRUE(statusClose == DBStatus::OK);
    delete delegateManager;
    delegateManager = nullptr;
    delete delegateManagerNew;
    delegateManagerNew = nullptr;
}

/*
 * @tc.name: ComplexAction 001
 * @tc.desc: Verify that can start transaction in new conn after close db.
 * @tc.type: FUNC
 * @tc.require: SR000BUH3J
 * @tc.author: luqianfu
 */
HWTEST_F(DistributeddbKvTransactionTest, ComplexAction001, TestSize.Level1)
{
    DistributedTestTools::Clear(*g_transactionDelegate);
    /**
     * @tc.steps: step1. open db then close it.
     * @tc.expected: step1. close successfully.
     */
    EXPECT_TRUE(g_transactionManager->CloseKvStore(g_transactionDelegate) == DBStatus::OK);
    g_transactionDelegate = nullptr;
    delete g_transactionManager;
    g_transactionManager = nullptr;

    /**
     * @tc.steps: step2. open db in new conn then start transaction and put (k1, v1) to db
     *    and then close the db without the transaction committed.
     * @tc.expected: step2. start the db and transaction and close the db successfully.
     */
    KvStoreDelegateManager *manager = nullptr;
    KvOption option = g_kvOption;
    option.createIfNecessary = false;
    KvStoreDelegate *delegate = DistributedTestTools::GetDelegateSuccess(manager, g_kvdbParameter1, option);
    ASSERT_TRUE(manager != nullptr && delegate != nullptr);
    EXPECT_EQ(delegate->StartTransaction(), DBStatus::OK);
    DBStatus status = DistributedTestTools::Put(*delegate, KEY_1, VALUE_1);
    EXPECT_EQ(status, DBStatus::OK);
    EXPECT_EQ(manager->CloseKvStore(delegate), DBStatus::OK);
    delegate = nullptr;
    delete manager;
    manager = nullptr;
    /**
     * @tc.steps: step3. reopen the same db again and the data on db.
     * @tc.expected: step3. can't find the data on db.
     */
    g_transactionDelegate = DistributedTestTools::GetDelegateSuccess(g_transactionManager,
        g_kvdbParameter1, g_kvOption);
    ASSERT_TRUE(g_transactionManager != nullptr && g_transactionDelegate != nullptr);
    Value value = DistributedTestTools::Get(*g_transactionDelegate, KEY_1);
    EXPECT_TRUE(value.size() == 0);
}

/*
 * @tc.name: ComplexAction 002
 * @tc.desc: Verify that can not start transaction after delete kv db.
 * @tc.type: FUNC
 * @tc.require: SR000BUH3J
 * @tc.author: luqianfu
 */
HWTEST_F(DistributeddbKvTransactionTest, ComplexAction002, TestSize.Level1)
{
    KvStoreDelegate *delegate = nullptr;
    KvStoreDelegateManager *delegateManager = nullptr;
    delegate = DistributedTestTools::GetDelegateSuccess(delegateManager,
        g_kvdbParameter2_1_1, g_kvOption);
    ASSERT_TRUE(delegateManager != nullptr && delegate != nullptr) << "fail to create exist kvdb";
    EXPECT_TRUE(delegateManager->CloseKvStore(delegate) == OK);
    delegate = nullptr;
    /**
     * @tc.steps: step1. delete kv db.
     * @tc.expected: step1. delete successfully.
     */
    DBStatus statusDelete = delegateManager->DeleteKvStore(STORE_ID_2);
    EXPECT_TRUE(statusDelete == DBStatus::OK);
    delete delegateManager;
    delegateManager = nullptr;
}

/*
 * @tc.name: ComplexAction 003
 * @tc.desc: Verify that can not delete kv db after start transaction.
 * @tc.type: FUNC
 * @tc.require: SR000BUH3J
 * @tc.author: luqianfu
 */
HWTEST_F(DistributeddbKvTransactionTest, ComplexAction003, TestSize.Level1)
{
    DistributedTestTools::Clear(*g_transactionDelegate);

    KvStoreDelegate *delegate = nullptr;
    KvStoreDelegateManager *delegateManager = nullptr;
    delegate = DistributedTestTools::GetDelegateSuccess(delegateManager, g_kvdbParameter2_1_1, g_kvOption);
    ASSERT_TRUE(delegateManager != nullptr && delegate != nullptr) << "fail to create exist kvdb";
    /**
     * @tc.steps: step1. start transaction.
     * @tc.expected: step1. start successfully.
     */
    DBStatus statusStart = delegate->StartTransaction();
    EXPECT_TRUE(statusStart == DBStatus::OK);
    /**
     * @tc.steps: step2. delete db.
     * @tc.expected: step2. delete failed.
     */
    DBStatus statusDelete = delegateManager->DeleteKvStore(STORE_ID_2);
    EXPECT_TRUE(statusDelete != DBStatus::OK);

    DBStatus statusCommit = delegate->Commit();
    EXPECT_TRUE(statusCommit == DBStatus::OK);
    DBStatus statusClose = delegateManager->CloseKvStore(delegate);
    delegate = nullptr;
    EXPECT_TRUE(statusClose == DBStatus::OK);
    delete delegateManager;
    delegateManager = nullptr;
}

/*
 * @tc.name: CommitHistory 001
 * @tc.desc: Verify that execute a transaction will generate a record.
 * @tc.type: FUNC
 * @tc.require: SR000BUH3J
 * @tc.author: luqianfu
 */
HWTEST_F(DistributeddbKvTransactionTest, CommitHistory001, TestSize.Level2)
{
    DistributedTestTools::Clear(*g_transactionDelegate);
    KvStoreDelegate *delegate = nullptr;
    KvStoreDelegateManager *delegateManager = nullptr;
    delegate = DistributedTestTools::GetDelegateSuccess(delegateManager, g_kvdbParameter1, g_kvOption);
    ASSERT_TRUE(delegateManager != nullptr && delegate != nullptr) << "fail to create exist kvdb";

    vector<Entry> entries1, entries1Up, entries2;
    entries1.push_back(ENTRY_1);
    entries1.push_back(ENTRY_2);
    entries1Up.push_back(ENTRY_1_2);
    entries1Up.push_back(ENTRY_2_3);
    entries2.push_back(ENTRY_3);
    entries2.push_back(ENTRY_2_4);
    /**
     * @tc.steps: step1. start transaction and put(k1,v1) then commit.
     * @tc.expected: step1. operate successfully.
     */
    EXPECT_TRUE(delegate->StartTransaction() == DBStatus::OK);
    EXPECT_TRUE(DistributedTestTools::Put(*delegate, KEY_1, VALUE_1) == DBStatus::OK);
    EXPECT_TRUE(delegate->Commit() == DBStatus::OK);
    /**
     * @tc.steps: step2. start transaction and update(k1,v1) to (k1,v2) then commit.
     * @tc.expected: step2. operate successfully.
     */
    EXPECT_TRUE(delegate->StartTransaction() == DBStatus::OK);
    EXPECT_TRUE(DistributedTestTools::Put(*delegate, KEY_1, VALUE_2) == DBStatus::OK);
    EXPECT_TRUE(delegate->Commit() == DBStatus::OK);
    /**
     * @tc.steps: step3. start transaction and delete(k1) then commit.
     * @tc.expected: step3. operate successfully.
     */
    EXPECT_TRUE(delegate->StartTransaction() == DBStatus::OK);
    EXPECT_TRUE(DistributedTestTools::Delete(*delegate, KEY_1) == DBStatus::OK);
    EXPECT_TRUE(delegate->Commit() == DBStatus::OK);
    /**
     * @tc.steps: step4. start transaction and putBatch(k1,v1)(k2,v2) then commit.
     * @tc.expected: step4. operate successfully.
     */
    EXPECT_TRUE(delegate->StartTransaction() == DBStatus::OK);
    EXPECT_TRUE(delegate->PutBatch(entries1) == DBStatus::OK);
    EXPECT_TRUE(delegate->Commit() == DBStatus::OK);
    /**
     * @tc.steps: step5. start transaction and putBatch(k1,v2)(k2,v3) then commit.
     * @tc.expected: step5. operate successfully.
     */
    EXPECT_TRUE(delegate->StartTransaction() == DBStatus::OK);
    EXPECT_TRUE(delegate->PutBatch(entries1Up) == DBStatus::OK);
    EXPECT_TRUE(delegate->Commit() == DBStatus::OK);
    /**
     * @tc.steps: step6. start transaction and deleteBatch(k1)(k2) putBatch (k2,v4)(k3,v3) then commit.
     * @tc.expected: step6. operate successfully.
     */
    EXPECT_TRUE(delegate->StartTransaction() == DBStatus::OK);
    EXPECT_TRUE(delegate->DeleteBatch(KEYS_1) == DBStatus::OK);
    EXPECT_TRUE(delegate->PutBatch(entries2) == DBStatus::OK);
    EXPECT_TRUE(delegate->Commit() == DBStatus::OK);
    /**
     * @tc.steps: step7. check transaction committion records by hand.
     * @tc.expected: step7. There are 6 records.
     */
    MST_LOG("please check the committed transaction records...");
    std::this_thread::sleep_for(std::chrono::duration<int>(WAIT_FOR_CHECKING_SECONDS));

    EXPECT_TRUE(delegateManager->CloseKvStore(delegate) == DBStatus::OK);
    delegate = nullptr;
    delete delegateManager;
    delegateManager = nullptr;
}

/*
 * @tc.name: CommitHistory 003
 * @tc.desc: Verify that transaction record will be null if rollback transaction.
 * @tc.type: Fault injection
 * @tc.require: SR000BUH3J
 * @tc.author: luqianfu
 */
HWTEST_F(DistributeddbKvTransactionTest, CommitHistory003, TestSize.Level2)
{
    DistributedTestTools::Clear(*g_transactionDelegate);

    KvStoreDelegate *delegate = nullptr;
    KvStoreDelegateManager *delegateManager = nullptr;
    delegate = DistributedTestTools::GetDelegateSuccess(delegateManager,
        g_kvdbParameter1, g_kvOption);
    ASSERT_TRUE(delegateManager != nullptr && delegate != nullptr) << "fail to create exist kvdb";

    /**
     * @tc.steps: step1. start transaction and put(k1,v1) then rollback.
     * @tc.expected: step1. operate successfully.
     */
    DBStatus statusStart = g_transactionDelegate->StartTransaction();
    EXPECT_TRUE(statusStart == DBStatus::OK);
    DBStatus statusPut = DistributedTestTools::Put(*g_transactionDelegate, KEY_1, VALUE_1);
    EXPECT_TRUE(statusPut == DBStatus::OK);
    DBStatus statusRollback = g_transactionDelegate->Rollback();
    EXPECT_TRUE(statusRollback == DBStatus::OK);
    /**
     * @tc.steps: step2. check submit records by hand.
     * @tc.expected: step2. record is null.
     */
    MST_LOG("please check the committed transaction records, and verify it is deleted...");
    std::this_thread::sleep_for(std::chrono::duration<int>(WAIT_FOR_CHECKING_SECONDS));

    delegateManager->CloseKvStore(delegate);
    delegate = nullptr;
    delete delegateManager;
    delegateManager = nullptr;
}

/*
 * @tc.name: BasicActionAcid 001
 * @tc.desc: Verify that transaction between put and putbatch.
 * @tc.type: Pressure
 * @tc.require: SR000BUH3J
 * @tc.author: luqianfu
 */
HWTEST_F(DistributeddbKvTransactionTest, BasicActionAcid001, TestSize.Level2)
{
    /**
     * @tc.steps: step1. start transaction and putBatch (k0-k127,1) then update v1=2 of k1.
     * @tc.expected: step1. operate successfully and get v1=2 of k1 finally.
     */
    for (int index = 0; index < BASIC_ACID_RUN_TIME; ++index) {
        DistributedCrudTransactionTools transactionTools(*g_transactionDelegate, CrudMode::PUT_BATCH, CrudMode::PUT,
            false, IS_LOCAL);
        EXPECT_TRUE(transactionTools.testCrudTransaction());
    }
}

/*
 * @tc.name: BasicActionAcid 002
 * @tc.desc: Verify that transaction between delete and deletebatch.
 * @tc.type: Pressure
 * @tc.require: SR000BUH3J
 * @tc.author: luqianfu
 */
HWTEST_F(DistributeddbKvTransactionTest, BasicActionAcid002, TestSize.Level2)
{
    /**
     * @tc.steps: step1. start transaction and deleteBatch (k0-k127,1) then delete k1.
     * @tc.expected: step1. operate successfully and no data exist in db.
     */
    for (int index = 0; index < BASIC_ACID_RUN_TIME; ++index) {
        DistributedCrudTransactionTools transactionTools(*g_transactionDelegate, CrudMode::DELETE_BATCH,
            CrudMode::DELETE, true, IS_LOCAL);
        EXPECT_TRUE(transactionTools.testCrudTransaction());
    }
}

/*
 * @tc.name: BasicActionAcid 003
 * @tc.desc: Verify that transaction between put and deletebatch.
 * @tc.type: Pressure
 * @tc.require: SR000BUH3J
 * @tc.author: luqianfu
 */
HWTEST_F(DistributeddbKvTransactionTest, BasicActionAcid003, TestSize.Level2)
{
    /**
     * @tc.steps: step1. start transaction and deleteBatch (k0-k127,1) then put (k1,v1=2).
     * @tc.expected: step1. operate successfully and get v1=2 of k1.
     */
    for (int index = 0; index < BASIC_ACID_RUN_TIME; ++index) {
        DistributedCrudTransactionTools transactionTools(*g_transactionDelegate, CrudMode::DELETE_BATCH,
            CrudMode::PUT, true, IS_LOCAL);
        EXPECT_TRUE(transactionTools.testCrudTransaction());
    }
}

/*
 * @tc.name: BasicActionAcid 004
 * @tc.desc: Verify that transaction between putbatch and delete.
 * @tc.type: Pressure
 * @tc.require: SR000BUH3J
 * @tc.author: luqianfu
 */
HWTEST_F(DistributeddbKvTransactionTest, BasicActionAcid004, TestSize.Level2)
{
    /**
     * @tc.steps: step1. start transaction and putBatch (k0-k127,1) then delete k1.
     * @tc.expected: step1. operate successfully and get null of k1 but the size of keys is 128.
     */
    for (int index = 0; index < BASIC_ACID_RUN_TIME; ++index) {
        DistributedCrudTransactionTools transactionTools(*g_transactionDelegate, CrudMode::PUT_BATCH,
            CrudMode::DELETE, false, IS_LOCAL);
        EXPECT_TRUE(transactionTools.testCrudTransaction());
    }
}

/*
 * @tc.name: BasicActionAcid 005
 * @tc.desc: Verify that transaction between put and clear.
 * @tc.type: Pressure
 * @tc.require: SR000BUH3J
 * @tc.author: luqianfu
 */
HWTEST_F(DistributeddbKvTransactionTest, BasicActionAcid005, TestSize.Level2)
{
    /**
     * @tc.steps: step1. start transaction and clear db then put (k1,v1=2).
     * @tc.expected: step1. operate successfully and get v1=2 of k1.
     */
    for (int index = 0; index < BASIC_ACID_RUN_TIME; ++index) {
        DistributedCrudTransactionTools transactionTools(*g_transactionDelegate, CrudMode::CLEAR,
            CrudMode::PUT, true, IS_LOCAL);
        EXPECT_TRUE(transactionTools.testCrudTransaction());
    }
}

/*
 * @tc.name: BasicActionAcid 006
 * @tc.desc: Verify that transaction between putbatch and clear.
 * @tc.type: Pressure
 * @tc.require: SR000BUH3J
 * @tc.author: luqianfu
 */
HWTEST_F(DistributeddbKvTransactionTest, BasicActionAcid006, TestSize.Level2)
{
    /**
     * @tc.steps: step1. start transaction and putBatch(k0-k127,values=2) then clear db.
     * @tc.expected: step1. operate successfully and get null of db.
     */
    for (int index = 0; index < BASIC_ACID_RUN_TIME; ++index) {
        DistributedCrudTransactionTools transactionTools(*g_transactionDelegate, CrudMode::PUT_BATCH,
            CrudMode::CLEAR, true, IS_LOCAL);
        EXPECT_TRUE(transactionTools.testCrudTransaction());
    }
}

/*
 * @tc.name: BasicActionAcid 007
 * @tc.desc: Verify that transaction between deletebatch and putbatch.
 * @tc.type: Pressure
 * @tc.require: SR000BUH3J
 * @tc.author: luqianfu
 */
HWTEST_F(DistributeddbKvTransactionTest, BasicActionAcid007, TestSize.Level2)
{
    /**
     * @tc.steps: step1. start transaction and deleteBatch(keys,values) then putBatch(k0-k127,v=2).
     * @tc.expected: step1. operate successfully and the size getEntries is 128,values=2.
     */
    for (int index = 0; index < BASIC_ACID_RUN_TIME; ++index) {
        DistributedCrudTransactionTools transactionTools(*g_transactionDelegate,
            CrudMode::DELETE_BATCH, CrudMode::PUT_BATCH, true, IS_LOCAL);
        EXPECT_TRUE(transactionTools.testCrudTransaction());
    }
}

/*
 * @tc.name: BasicActionAcid 008
 * @tc.desc: Verify that transaction between deletebatch and clear.
 * @tc.type: Pressure
 * @tc.require: SR000BUH3J
 * @tc.author: luqianfu
 */
HWTEST_F(DistributeddbKvTransactionTest, BasicActionAcid008, TestSize.Level2)
{
    /**
     * @tc.steps: step1. start transaction and clear then deleteBatch(k0-k127,v=2).
     * @tc.expected: step1. operate successfully and no data exist in db finally.
     */
    for (int index = 0; index < BASIC_ACID_RUN_TIME; ++index) {
        DistributedCrudTransactionTools transactionTools(*g_transactionDelegate, CrudMode::DELETE_BATCH,
            CrudMode::CLEAR, true, IS_LOCAL);
        EXPECT_TRUE(transactionTools.testCrudTransaction());
    }
}
}