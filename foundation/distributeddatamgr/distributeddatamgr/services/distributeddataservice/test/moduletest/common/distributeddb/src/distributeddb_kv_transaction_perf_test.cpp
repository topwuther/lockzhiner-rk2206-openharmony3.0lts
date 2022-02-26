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

namespace DistributeddbKvTransactionPerf {
const bool IS_LOCAL = false;
const int PUT_TIMES = 10;
const int KEY_LENGTH = 16;
const int VALUE_LENGTH = 100;

class DistributeddbKvTransactionPerfTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
};

KvStoreDelegate *g_transactionDelegate = nullptr; // the delegate used in this suit.
KvStoreDelegateManager *g_transactionManager = nullptr;
void DistributeddbKvTransactionPerfTest::SetUpTestCase(void)
{
}

void DistributeddbKvTransactionPerfTest::TearDownTestCase(void)
{
}

void DistributeddbKvTransactionPerfTest::SetUp(void)
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

void DistributeddbKvTransactionPerfTest::TearDown(void)
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
 * @tc.name: Performance 001
 * @tc.desc: Compare between inserting many records in one transaction and one record in each transaction.
 * @tc.type: Performance
 * @tc.require: SR000BUH3J
 * @tc.author: luqianfu
 */
HWTEST_F(DistributeddbKvTransactionPerfTest, Performance001, TestSize.Level3)
{
    /**
     * @tc.steps: step1.get the putDuration of putBatch(keys,values) by transaction.
     * @tc.expected: step1. the putDuration is smaller than 1ms/k.
     */
    PerformanceData performance(PUT_TIMES, KEY_LENGTH, VALUE_LENGTH, false, false, false, false, IS_LOCAL);
    double transactionInsert, singleInsert, batchInsert;

    EXPECT_TRUE(DistributedTestTools::CalculateTransactionPerformance(performance));
    transactionInsert = performance.putDuration;
    /**
     * @tc.steps: step2.get the putDuration of put(k,v) singlely.
     * @tc.expected: step2. the putDuration is smaller than 1ms/k and bigger than step1.
     */
    EXPECT_TRUE(DistributedTestTools::CalculateOpenPerformance(performance));
    EXPECT_TRUE(DistributedTestTools::CalculateInsertPerformance(performance));
    EXPECT_TRUE(DistributedTestTools::CalculateGetPutPerformance(performance));
    EXPECT_TRUE(DistributedTestTools::CalculateUpdatePerformance(performance));
    EXPECT_TRUE(DistributedTestTools::CalculateGetUpdatePerformance(performance));
    EXPECT_TRUE(DistributedTestTools::CalculateUseClearPerformance(performance));
    singleInsert = performance.putDuration;
    /**
     * @tc.steps: step3.get the putDuration of putBatch(keys,value) without transaction.
     * @tc.expected: step3. the putDuration is smaller than 1ms/k.
     */
    performance.putBatch = true;
    EXPECT_TRUE(DistributedTestTools::CalculateOpenPerformance(performance));
    EXPECT_TRUE(DistributedTestTools::CalculateInsertPerformance(performance));
    EXPECT_TRUE(DistributedTestTools::CalculateGetPutPerformance(performance));
    EXPECT_TRUE(DistributedTestTools::CalculateUpdatePerformance(performance));
    EXPECT_TRUE(DistributedTestTools::CalculateGetUpdatePerformance(performance));
    EXPECT_TRUE(DistributedTestTools::CalculateUseClearPerformance(performance));
    batchInsert = performance.putDuration;

    if (transactionInsert <= batchInsert && batchInsert <= singleInsert) {
        MST_LOG("the put performance is ok!");
    }
}

/*
 * @tc.name: Performance 002
 * @tc.desc: Compare between updating many records in one transaction and one record in each transaction.
 * @tc.type: Performance
 * @tc.require: SR000BUH3J
 * @tc.author: luqianfu
 */
HWTEST_F(DistributeddbKvTransactionPerfTest, Performance002, TestSize.Level3)
{
    /**
     * @tc.steps: step1.get the putDuration of updateBatch(keys,values) by transaction.
     * @tc.expected: step1. the putDuration is smaller than 1ms/k.
     */
    PerformanceData performance(PUT_TIMES, KEY_LENGTH, VALUE_LENGTH, false, false, false, false, IS_LOCAL);
    double transactionUpdate, singleUpdate, batchUpdate;

    EXPECT_TRUE(DistributedTestTools::CalculateTransactionPerformance(performance));
    transactionUpdate = performance.updateDuration;
    /**
     * @tc.steps: step2.get the putDuration of update(k,v) singlely.
     * @tc.expected: step2. the putDuration is smaller than 1ms/k and is bigger than step1.
     */
    EXPECT_TRUE(DistributedTestTools::CalculateOpenPerformance(performance));
    EXPECT_TRUE(DistributedTestTools::CalculateInsertPerformance(performance));
    EXPECT_TRUE(DistributedTestTools::CalculateGetPutPerformance(performance));
    EXPECT_TRUE(DistributedTestTools::CalculateUpdatePerformance(performance));
    EXPECT_TRUE(DistributedTestTools::CalculateGetUpdatePerformance(performance));
    EXPECT_TRUE(DistributedTestTools::CalculateUseClearPerformance(performance));
    singleUpdate = performance.updateDuration;
    /**
     * @tc.steps: step3.get the putDuration of putBatch(keys,value) without transaction.
     * @tc.expected: step3. the putDuration is smaller than 1ms/k.
     */
    performance.putBatch = true;
    EXPECT_TRUE(DistributedTestTools::CalculateOpenPerformance(performance));
    EXPECT_TRUE(DistributedTestTools::CalculateInsertPerformance(performance));
    EXPECT_TRUE(DistributedTestTools::CalculateGetPutPerformance(performance));
    EXPECT_TRUE(DistributedTestTools::CalculateUpdatePerformance(performance));
    EXPECT_TRUE(DistributedTestTools::CalculateGetUpdatePerformance(performance));
    EXPECT_TRUE(DistributedTestTools::CalculateUseClearPerformance(performance));
    batchUpdate = performance.updateDuration;

    if (transactionUpdate <= batchUpdate && batchUpdate <= singleUpdate) {
        MST_LOG("the update performance is ok!");
    }
}

/*
 * @tc.name: Performance 003
 * @tc.desc: Compare between deleting many records in one transaction and one record in each transaction.
 * @tc.type: Performance
 * @tc.require: SR000BUH3J
 * @tc.author: luqianfu
 */
HWTEST_F(DistributeddbKvTransactionPerfTest, Performance003, TestSize.Level3)
{
    /**
     * @tc.steps: step1.get the putDuration of deleteBatch(keys,values) by transaction.
     * @tc.expected: step1. the putDuration is smaller than 1ms/k.
     */
    PerformanceData performance(PUT_TIMES, KEY_LENGTH, VALUE_LENGTH, false, false, false, false, IS_LOCAL);
    double transactionDel, singleDel, batchDel;

    EXPECT_TRUE(DistributedTestTools::CalculateTransactionPerformance(performance));
    transactionDel = performance.deleteDuration;
    /**
     * @tc.steps: step2.get the putDuration of delete(k,v) singlely.
     * @tc.expected: step2. the putDuration is smaller than 1ms/k and is bigger than step1.
     */
    EXPECT_TRUE(DistributedTestTools::CalculateOpenPerformance(performance));
    EXPECT_TRUE(DistributedTestTools::CalculateInsertPerformance(performance));
    EXPECT_TRUE(DistributedTestTools::CalculateGetPutPerformance(performance));
    EXPECT_TRUE(DistributedTestTools::CalculateUpdatePerformance(performance));
    EXPECT_TRUE(DistributedTestTools::CalculateGetUpdatePerformance(performance));
    EXPECT_TRUE(DistributedTestTools::CalculateUseClearPerformance(performance));
    singleDel = performance.deleteDuration;
    /**
     * @tc.steps: step3.get the putDuration of deleteBatch(keys,value) without transaction.
     * @tc.expected: step3. the putDuration is smaller than 1ms/k.
     */
    performance.putBatch = true;
    EXPECT_TRUE(DistributedTestTools::CalculateOpenPerformance(performance));
    EXPECT_TRUE(DistributedTestTools::CalculateInsertPerformance(performance));
    EXPECT_TRUE(DistributedTestTools::CalculateGetPutPerformance(performance));
    EXPECT_TRUE(DistributedTestTools::CalculateUpdatePerformance(performance));
    EXPECT_TRUE(DistributedTestTools::CalculateGetUpdatePerformance(performance));
    EXPECT_TRUE(DistributedTestTools::CalculateUseClearPerformance(performance));
    batchDel = performance.deleteDuration;

    if (transactionDel <= batchDel && batchDel <= singleDel) {
        MST_LOG("the delete performance is ok!");
    }
}

/*
 * @tc.name: Performance 004
 * @tc.desc: system info of inserting many records in one transaction and one record in each transaction.
 * @tc.type: Performance
 * @tc.require: SR000BUH3J
 * @tc.author: luqianfu
 */
HWTEST_F(DistributeddbKvTransactionPerfTest, Performance004, TestSize.Level3)
{
    PerformanceData performance(PUT_TIMES, KEY_LENGTH, VALUE_LENGTH, false, false, false, true, IS_LOCAL);
    /**
     * @tc.steps: step1.get the sys info of putBatch(keys,values) by transaction.
     * @tc.expected: step1. the sys info is normal.
     */
    EXPECT_TRUE(DistributedTestTools::CalculateTransactionPerformance(performance));
    /**
     * @tc.steps: step2.get the sys info of put(k,v) singlely.
     * @tc.expected: step2. the sys info is normal.
     */
    EXPECT_TRUE(DistributedTestTools::CalculateOpenPerformance(performance));
    EXPECT_TRUE(DistributedTestTools::CalculateInsertPerformance(performance));
    EXPECT_TRUE(DistributedTestTools::CalculateGetPutPerformance(performance));
    EXPECT_TRUE(DistributedTestTools::CalculateUpdatePerformance(performance));
    EXPECT_TRUE(DistributedTestTools::CalculateGetUpdatePerformance(performance));
    EXPECT_TRUE(DistributedTestTools::CalculateUseClearPerformance(performance));
    /**
     * @tc.steps: step3.get the sys info of putBatch(keys,values) without transaction.
     * @tc.expected: step3. the sys info is normal.
     */
    performance.putBatch = true;
    EXPECT_TRUE(DistributedTestTools::CalculateOpenPerformance(performance));
    EXPECT_TRUE(DistributedTestTools::CalculateInsertPerformance(performance));
    EXPECT_TRUE(DistributedTestTools::CalculateGetPutPerformance(performance));
    EXPECT_TRUE(DistributedTestTools::CalculateUpdatePerformance(performance));
    EXPECT_TRUE(DistributedTestTools::CalculateGetUpdatePerformance(performance));
    EXPECT_TRUE(DistributedTestTools::CalculateUseClearPerformance(performance));
}

/*
 * @tc.name: Performance 005
 * @tc.desc: system info of updating many records in one transaction and one record in each transaction.
 * @tc.type: Pressure
 * @tc.require: SR000BUH3J
 * @tc.author: luqianfu
 */
HWTEST_F(DistributeddbKvTransactionPerfTest, Performance005, TestSize.Level3)
{
    PerformanceData performance(PUT_TIMES, KEY_LENGTH, VALUE_LENGTH, false, false, false, true, IS_LOCAL);

    /**
     * @tc.steps: step1.get the sys info of updateBatch(keys,values) by transaction.
     * @tc.expected: step1. the sys info is normal.
     */
    EXPECT_TRUE(DistributedTestTools::CalculateTransactionPerformance(performance));
    /**
     * @tc.steps: step2.get the sys info of update(k,v) singlely.
     * @tc.expected: step2. the sys info is normal.
     */
    EXPECT_TRUE(DistributedTestTools::CalculateOpenPerformance(performance));
    EXPECT_TRUE(DistributedTestTools::CalculateInsertPerformance(performance));
    EXPECT_TRUE(DistributedTestTools::CalculateGetPutPerformance(performance));
    EXPECT_TRUE(DistributedTestTools::CalculateUpdatePerformance(performance));
    EXPECT_TRUE(DistributedTestTools::CalculateGetUpdatePerformance(performance));
    EXPECT_TRUE(DistributedTestTools::CalculateUseClearPerformance(performance));
    /**
     * @tc.steps: step3.get the sys info of updateBatch(keys,values) without transaction.
     * @tc.expected: step3. the sys info is normal.
     */
    performance.putBatch = true;
    EXPECT_TRUE(DistributedTestTools::CalculateOpenPerformance(performance));
    EXPECT_TRUE(DistributedTestTools::CalculateInsertPerformance(performance));
    EXPECT_TRUE(DistributedTestTools::CalculateGetPutPerformance(performance));
    EXPECT_TRUE(DistributedTestTools::CalculateUpdatePerformance(performance));
    EXPECT_TRUE(DistributedTestTools::CalculateGetUpdatePerformance(performance));
    EXPECT_TRUE(DistributedTestTools::CalculateUseClearPerformance(performance));
}

/*
 * @tc.name: Performance 006
 * @tc.desc: system info of deleting many records in one transaction and one record in each transaction.
 * @tc.type: Pressure
 * @tc.require: SR000BUH3J
 * @tc.author: luqianfu
 */
HWTEST_F(DistributeddbKvTransactionPerfTest, Performance006, TestSize.Level3)
{
    PerformanceData performance(PUT_TIMES, KEY_LENGTH, VALUE_LENGTH, false, false, false, true, IS_LOCAL);

    /**
     * @tc.steps: step1.get the sys info of deleteBatch(keys,values) by transaction.
     * @tc.expected: step1. the sys info is normal.
     */
    EXPECT_TRUE(DistributedTestTools::CalculateTransactionPerformance(performance));
    /**
     * @tc.steps: step2.get the sys info of delete(k,v) singlely.
     * @tc.expected: step2. the sys info is normal.
     */
    EXPECT_TRUE(DistributedTestTools::CalculateOpenPerformance(performance));
    EXPECT_TRUE(DistributedTestTools::CalculateInsertPerformance(performance));
    EXPECT_TRUE(DistributedTestTools::CalculateGetPutPerformance(performance));
    EXPECT_TRUE(DistributedTestTools::CalculateUpdatePerformance(performance));
    EXPECT_TRUE(DistributedTestTools::CalculateGetUpdatePerformance(performance));
    EXPECT_TRUE(DistributedTestTools::CalculateUseClearPerformance(performance));
    /**
     * @tc.steps: step3.get the sys info of deleteBatch(keys,values) without transaction.
     * @tc.expected: step3. the sys info is normal.
     */
    performance.putBatch = true;
    EXPECT_TRUE(DistributedTestTools::CalculateOpenPerformance(performance));
    EXPECT_TRUE(DistributedTestTools::CalculateInsertPerformance(performance));
    EXPECT_TRUE(DistributedTestTools::CalculateGetPutPerformance(performance));
    EXPECT_TRUE(DistributedTestTools::CalculateUpdatePerformance(performance));
    EXPECT_TRUE(DistributedTestTools::CalculateGetUpdatePerformance(performance));
    EXPECT_TRUE(DistributedTestTools::CalculateUseClearPerformance(performance));
}
}