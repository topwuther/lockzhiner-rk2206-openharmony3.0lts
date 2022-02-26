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
#include <random>
#include <chrono>
#include <thread>
#include <string>

#include "types.h"
#include "kv_store_delegate.h"
#include "kv_store_nb_delegate.h"
#include "kv_store_delegate_manager.h"
#include "distributed_test_tools.h"
#include "distributeddb_nb_test_tools.h"
#include "distributeddb_data_generator.h"

using namespace std;
using namespace chrono;
using namespace testing;
#if defined TESTCASES_USING_GTEST_EXT
using namespace testing::ext;
#endif
using namespace std::placeholders;
using namespace DistributedDB;
using namespace DistributedDBDataGenerator;

namespace DistributeddbNbCrud {
const unsigned int READ_RECORDS_NUM_START = 1;
const unsigned int READ_RECORDS_NUM_END = 1000;
const unsigned int WRITE_RECORDS_NUM_START = 1;
const unsigned int WRITE_RECORDS_NUM_END = 9999;
const unsigned int DELETE_RECORDS_NUM_START = 1;
const unsigned int DELETE_RECORDS_NUM_END = 1000;

const unsigned long LONG_TIME_TEST_SECONDS = 10;
const unsigned long LONG_TIME_INTERVAL_MILLSECONDS = 5;

const unsigned int RECORDS_NUM_THOUSAND = 1000;

KvStoreNbDelegate *g_kvStoreNbDelegate = nullptr;
KvStoreDelegateManager *g_manager = nullptr;

class DistributeddbNbCrudTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
private:
};

void DistributeddbNbCrudTest::SetUpTestCase(void)
{
}

void DistributeddbNbCrudTest::TearDownTestCase(void)
{
}

void DistributeddbNbCrudTest::SetUp(void)
{
    RemoveDir(NB_DIRECTOR);

    UnitTest *test = UnitTest::GetInstance();
    ASSERT_NE(test, nullptr);
    const TestInfo *testinfo = test->current_test_info();
    ASSERT_NE(testinfo, nullptr);
    string testCaseName = string(testinfo->name());
    MST_LOG("[SetUp] test case %s is start to run", testCaseName.c_str());

    g_kvStoreNbDelegate = DistributedDBNbTestTools::GetNbDelegateSuccess(g_manager, g_dbParameter1, g_option);
    ASSERT_TRUE(g_manager != nullptr && g_kvStoreNbDelegate != nullptr);
}

void DistributeddbNbCrudTest::TearDown(void)
{
    MST_LOG("TearDownTestCase after case.");
    ASSERT_NE(g_manager, nullptr);
    EXPECT_TRUE(EndCaseDeleteDB(g_manager, g_kvStoreNbDelegate, STORE_ID_1, g_option.isMemoryDb));
    RemoveDir(NB_DIRECTOR);
}

enum ReadOrWriteTag {
    READ = 0,
    WRITE = 1,
    DELETE = 2
};

struct ConcurParam {
    unsigned int threadId_;
    ReadOrWriteTag tag_;
    Entry* entryPtr_;
};
static std::mutex g_concurOperMutex;
// the thread runnnig methods
void ConcurOperThread(ConcurParam* args)
{
    auto paramsPtr = static_cast<ConcurParam *>(args);
    DBStatus status;
    Value valueResult;

    if (paramsPtr->tag_ == READ) {
        status = DistributedDBNbTestTools::Get(*g_kvStoreNbDelegate, paramsPtr->entryPtr_->key, valueResult);
        if (valueResult.size() != 0) {
            EXPECT_EQ(status, OK);
            EXPECT_TRUE(DistributedDBNbTestTools::isValueEquals(valueResult, paramsPtr->entryPtr_->value));
        }
    } else if (paramsPtr->tag_ == WRITE) {
        status = DistributedDBNbTestTools::Put(*g_kvStoreNbDelegate,
            paramsPtr->entryPtr_->key, paramsPtr->entryPtr_->value);
        ASSERT_EQ(status, DBStatus::OK);

        status = DistributedDBNbTestTools::Get(*g_kvStoreNbDelegate, paramsPtr->entryPtr_->key, valueResult);
        EXPECT_EQ(status, DBStatus::OK);
        EXPECT_TRUE(DistributedDBNbTestTools::isValueEquals(valueResult, paramsPtr->entryPtr_->value));
    } else {
        std::lock_guard<std::mutex> lock(g_concurOperMutex);
        status = DistributedDBNbTestTools::Get(*g_kvStoreNbDelegate, paramsPtr->entryPtr_->key, valueResult);
        if (valueResult.size() != 0) {
            status = DistributedDBNbTestTools::Delete(*g_kvStoreNbDelegate, paramsPtr->entryPtr_->key);
            ASSERT_EQ(status, DBStatus::OK);

            valueResult = {};
            status = DistributedDBNbTestTools::Get(*g_kvStoreNbDelegate, paramsPtr->entryPtr_->key, valueResult);
            EXPECT_EQ(status, DBStatus::NOT_FOUND);
            EXPECT_EQ(valueResult.size(), (unsigned long)0);
        } else {
            status = DistributedDBNbTestTools::Delete(*g_kvStoreNbDelegate, paramsPtr->entryPtr_->key);
            ASSERT_EQ(status, DBStatus::OK);
        }
    }
}

double NbCalculateTime(ConcurParam *&nbThreadParam, const Entry entryCurrent, const SysTime &start, SysDurTime dur)
{
    SysTime end;
    nbThreadParam->entryPtr_->key = entryCurrent.key;
    nbThreadParam->entryPtr_->value = entryCurrent.value;
    std::thread thread = std::thread(ConcurOperThread, reinterpret_cast<ConcurParam *>(nbThreadParam));
    thread.join();

    std::this_thread::sleep_for(std::chrono::duration<float, std::milli>(LONG_TIME_INTERVAL_MILLSECONDS));
    end = std::chrono::time_point_cast<std::chrono::microseconds>(std::chrono::steady_clock::now());
    dur = std::chrono::duration_cast<chrono::microseconds>(end - start);
    double operInterval = static_cast<double>(dur.count()) * chrono::microseconds::period::num
        / chrono::microseconds::period::den;
    delete nbThreadParam->entryPtr_;
    nbThreadParam->entryPtr_ = nullptr;
    delete nbThreadParam;
    nbThreadParam = nullptr;
    return operInterval;
}

/*
 * @tc.name: GetStoreid 001
 * @tc.desc: Verify that can obtain storeid normally.
 * @tc.type: FUNC
 * @tc.require: SR000CCPOI
 * @tc.author: luqianfu
 */
HWTEST_F(DistributeddbNbCrudTest, GetStoreid001, TestSize.Level0)
{
    /**
     * @tc.steps: step1. create db and get storeId normally.
     * @tc.expected: step1. success.
     */
    EXPECT_EQ(g_kvStoreNbDelegate->GetStoreId(), STORE_ID_1);
}

/*
 * @tc.name: SimpleAction 001
 * @tc.desc: test that get data from local and sync db and they can't affect each other.
 * @tc.type: FUNC
 * @tc.require: SR000CCPOI
 * @tc.author: luqianfu
 */
HWTEST_F(DistributeddbNbCrudTest, SimpleAction001, TestSize.Level0)
{
    DBStatus status = g_kvStoreNbDelegate->PutLocal(KEY_1, VALUE_1);
    EXPECT_TRUE(status == OK);
    status = g_kvStoreNbDelegate->PutLocal(KEY_2, VALUE_2);
    EXPECT_TRUE(status == OK);
    status = g_kvStoreNbDelegate->Put(KEY_1, VALUE_2);
    EXPECT_TRUE(status == OK);
    status = g_kvStoreNbDelegate->Put(KEY_2, VALUE_1);
    EXPECT_TRUE(status == OK);
    status = g_kvStoreNbDelegate->Put(KEY_3, VALUE_3);
    EXPECT_TRUE(status == OK);

    /**
     * @tc.steps: step1. get value from local db when key = KEY_1.
     * @tc.expected: step1. return value is VALUE_1.
     */
    Value valueResult;
    status = g_kvStoreNbDelegate->GetLocal(KEY_1, valueResult);
    EXPECT_EQ(status, OK);
    EXPECT_EQ(valueResult, VALUE_1);
    /**
     * @tc.steps: step2. get value from local db when key = KEY_2.
     * @tc.expected: step2. return value is VALUE_2.
     */
    status = g_kvStoreNbDelegate->GetLocal(KEY_2, valueResult);
    EXPECT_EQ(status, OK);
    EXPECT_EQ(valueResult, VALUE_2);
    /**
     * @tc.steps: step3. get value from local db when key = KEY_3.
     * @tc.expected: step3. return value is NOT_FOUND.
     */
    status = g_kvStoreNbDelegate->GetLocal(KEY_3, valueResult);
    EXPECT_EQ(status, NOT_FOUND);
    /**
     * @tc.steps: step4. get value from sync db when key = KEY_1.
     * @tc.expected: step4. return value is VALUE_2.
     */
    status = g_kvStoreNbDelegate->Get(KEY_1, valueResult);
    EXPECT_EQ(status, OK);
    EXPECT_EQ(valueResult, VALUE_2);
    /**
     * @tc.steps: step5. get value from sync db when key = KEY_2.
     * @tc.expected: step5. return value is VALUE_1.
     */
    status = g_kvStoreNbDelegate->Get(KEY_2, valueResult);
    EXPECT_EQ(status, OK);
    EXPECT_EQ(valueResult, VALUE_1);
    /**
     * @tc.steps: step6. get value from sync db when key = KEY_3.
     * @tc.expected: step6. return value is VALUE_3.
     */
    status = g_kvStoreNbDelegate->Get(KEY_3, valueResult);
    EXPECT_EQ(status, OK);
    EXPECT_EQ(valueResult, VALUE_3);
    g_kvStoreNbDelegate->Delete(KEY_1);
    g_kvStoreNbDelegate->Delete(KEY_2);
    g_kvStoreNbDelegate->Delete(KEY_3);
    g_kvStoreNbDelegate->DeleteLocal(KEY_1);
    g_kvStoreNbDelegate->DeleteLocal(KEY_2);
}

/*
 * @tc.name: SimpleAction 002
 * @tc.desc: test put data from local and sync db and they can't affect each other.
 * @tc.type: FUNC
 * @tc.require: SR000CCPOI
 * @tc.author: luqianfu
 */
HWTEST_F(DistributeddbNbCrudTest, SimpleAction002, TestSize.Level0)
{
    /**
     * @tc.steps: step1. put (KEY_1, VALUE_1) to local db.
     * @tc.expected: step1. put success.
     */
    DBStatus status = g_kvStoreNbDelegate->PutLocal(KEY_1, VALUE_1);
    EXPECT_TRUE(status == OK);
    /**
     * @tc.steps: step2. put (KEY_2, VALUE_2) to local db.
     * @tc.expected: step2. put success.
     */
    status = g_kvStoreNbDelegate->PutLocal(KEY_2, VALUE_2);
    EXPECT_TRUE(status == OK);
    /**
     * @tc.steps: step3. put (KEY_1, VALUE_2) to sync db.
     * @tc.expected: step3. put success.
     */
    status = g_kvStoreNbDelegate->Put(KEY_1, VALUE_2);
    EXPECT_TRUE(status == OK);
    /**
     * @tc.steps: step4. put (KEY_2, VALUE_1) to sync db.
     * @tc.expected: step4. put success.
     */
    status = g_kvStoreNbDelegate->Put(KEY_2, VALUE_1);
    EXPECT_TRUE(status == OK);

    /**
     * @tc.steps: step6. get data from local db where key = KEY_1.
     * @tc.expected: step6. return value is VALUE_1.
     */
    Value valueResult;
    status = g_kvStoreNbDelegate->GetLocal(KEY_1, valueResult);
    EXPECT_EQ(status, OK);
    EXPECT_EQ(valueResult, VALUE_1);
    /**
     * @tc.steps: step7. get data from local db where key = KEY_2.
     * @tc.expected: step7. return value is VALUE_2.
     */
    status = g_kvStoreNbDelegate->GetLocal(KEY_2, valueResult);
    EXPECT_EQ(status, OK);
    EXPECT_EQ(valueResult, VALUE_2);
    /**
     * @tc.steps: step8. get data from sync db where key = KEY_1.
     * @tc.expected: step8. return value is VALUE_2.
     */
    status = g_kvStoreNbDelegate->Get(KEY_1, valueResult);
    EXPECT_EQ(status, OK);
    EXPECT_EQ(valueResult, VALUE_2);
    /**
     * @tc.steps: step9. get data from sync db where key = KEY_2.
     * @tc.expected: step9. return value is VALUE_1.
     */
    status = g_kvStoreNbDelegate->Get(KEY_2, valueResult);
    EXPECT_EQ(status, OK);
    EXPECT_EQ(valueResult, VALUE_1);

    g_kvStoreNbDelegate->Delete(KEY_1);
    g_kvStoreNbDelegate->Delete(KEY_2);
    g_kvStoreNbDelegate->DeleteLocal(KEY_1);
    g_kvStoreNbDelegate->DeleteLocal(KEY_2);
}

/*
 * @tc.name: SimpleAction 003
 * @tc.desc: test that update data to db, and local and sync db can't affect each other.
 * @tc.type: FUNC
 * @tc.require: SR000CCPOI
 * @tc.author: luqianfu
 */
HWTEST_F(DistributeddbNbCrudTest, SimpleAction003, TestSize.Level0)
{
    DBStatus status = g_kvStoreNbDelegate->PutLocal(KEY_1, VALUE_1);
    EXPECT_TRUE(status == OK);
    status = g_kvStoreNbDelegate->PutLocal(KEY_2, VALUE_2);
    EXPECT_TRUE(status == OK);
    status = g_kvStoreNbDelegate->Put(KEY_1, VALUE_2);
    EXPECT_TRUE(status == OK);
    status = g_kvStoreNbDelegate->Put(KEY_2, VALUE_1);
    EXPECT_TRUE(status == OK);

    /**
     * @tc.steps: step1. update data (KEY_1, VALUE_2) to local db.
     * @tc.expected: step1.update success.
     */
    Value valueResult;
    status = g_kvStoreNbDelegate->PutLocal(KEY_1, VALUE_2);
    EXPECT_TRUE(status == OK);
    /**
     * @tc.steps: step2. update data (KEY_2, VALUE_1) to local db.
     * @tc.expected: step2.update success.
     */
    status = g_kvStoreNbDelegate->PutLocal(KEY_2, VALUE_1);
    EXPECT_TRUE(status == OK);
    /**
     * @tc.steps: step3. update data (KEY_1, VALUE_1) to sync db.
     * @tc.expected: step3.update success.
     */
    status = g_kvStoreNbDelegate->Put(KEY_1, VALUE_1);
    EXPECT_TRUE(status == OK);
    /**
     * @tc.steps: step4. update data (KEY_2, VALUE_2) to sync db.
     * @tc.expected: step4.update success.
     */
    status = g_kvStoreNbDelegate->Put(KEY_2, VALUE_2);
    EXPECT_TRUE(status == OK);
    /**
     * @tc.steps: step5. get data from local db where key = KEY_1.
     * @tc.expected: step5. return value is VALUE_2.
     */
    status = g_kvStoreNbDelegate->GetLocal(KEY_1, valueResult);
    EXPECT_EQ(status, OK);
    EXPECT_EQ(valueResult, VALUE_2);
    /**
     * @tc.steps: step6. get data from local db where key = KEY_2.
     * @tc.expected: step6. return value is VALUE_1.
     */
    status = g_kvStoreNbDelegate->GetLocal(KEY_2, valueResult);
    EXPECT_EQ(status, OK);
    EXPECT_EQ(valueResult, VALUE_1);
    /**
     * @tc.steps: step7. get data from sync db where key = KEY_1.
     * @tc.expected: step7. return value is VALUE_1.
     */
    status = g_kvStoreNbDelegate->Get(KEY_1, valueResult);
    EXPECT_EQ(status, OK);
    EXPECT_EQ(valueResult, VALUE_1);
    /**
     * @tc.steps: step8. get data from sync db where key = KEY_2.
     * @tc.expected: step8. return value is VALUE_2.
     */
    status = g_kvStoreNbDelegate->Get(KEY_2, valueResult);
    EXPECT_EQ(status, OK);
    EXPECT_EQ(valueResult, VALUE_2);

    g_kvStoreNbDelegate->Delete(KEY_1);
    g_kvStoreNbDelegate->Delete(KEY_2);
    g_kvStoreNbDelegate->DeleteLocal(KEY_1);
    g_kvStoreNbDelegate->DeleteLocal(KEY_2);
}

/*
 * @tc.name: SimpleAction 004
 * @tc.desc: test that can delete entries of db, and local and sync db can't affect each other.
 * @tc.type: FUNC
 * @tc.require: SR000CCPOI
 * @tc.author: luqianfu
 */
HWTEST_F(DistributeddbNbCrudTest, SimpleAction004, TestSize.Level0)
{
    DBStatus status = g_kvStoreNbDelegate->PutLocal(KEY_1, VALUE_1);
    EXPECT_TRUE(status == OK);
    status = g_kvStoreNbDelegate->PutLocal(KEY_2, VALUE_2);
    EXPECT_TRUE(status == OK);
    status = g_kvStoreNbDelegate->Put(KEY_1, VALUE_2);
    EXPECT_TRUE(status == OK);
    status = g_kvStoreNbDelegate->Put(KEY_2, VALUE_1);
    EXPECT_TRUE(status == OK);
    /**
     * @tc.steps: step1. delete entries from local db where key = KEY_1.
     * @tc.expected: step1. delete success.
     */
    status = g_kvStoreNbDelegate->DeleteLocal(KEY_1);
    EXPECT_TRUE(status == OK);
    /**
     * @tc.steps: step2. delete entries from sync db where key = KEY_2.
     * @tc.expected: step2. delete success.
     */
    status = g_kvStoreNbDelegate->Delete(KEY_2);
    EXPECT_TRUE(status == OK);
    /**
     * @tc.steps: step3. get data from local where key = KEY_1.
     * @tc.expected: step3. return value is NOT_FOUND.
     */
    Value valueResult;
    status = g_kvStoreNbDelegate->GetLocal(KEY_1, valueResult);
    EXPECT_EQ(status, NOT_FOUND);
    /**
     * @tc.steps: step4. get data from local where key = KEY_2.
     * @tc.expected: step4. return value is VALUE_2.
     */
    status = g_kvStoreNbDelegate->GetLocal(KEY_2, valueResult);
    EXPECT_EQ(status, OK);
    EXPECT_EQ(valueResult, VALUE_2);
    /**
     * @tc.steps: step5. get data from sync where key = KEY_1.
     * @tc.expected: step5. return value is VALUE_2.
     */
    status = g_kvStoreNbDelegate->Get(KEY_1, valueResult);
    EXPECT_EQ(status, OK);
    EXPECT_EQ(valueResult, VALUE_2);
    /**
     * @tc.steps: step6. get data from sync where key = KEY_2.
     * @tc.expected: step6. return value is NOT_FOUND.
     */
    status = g_kvStoreNbDelegate->Get(KEY_2, valueResult);
    EXPECT_EQ(status, NOT_FOUND);

    g_kvStoreNbDelegate->DeleteLocal(KEY_2);
    g_kvStoreNbDelegate->Delete(KEY_1);
}

/*
 * @tc.name: SimpleAction 005
 * @tc.desc: test that can query entries that don't exist.
 * @tc.type: FUNC
 * @tc.require: SR000CCPOI
 * @tc.author: luqianfu
 */
HWTEST_F(DistributeddbNbCrudTest, SimpleAction005, TestSize.Level0)
{
    DBStatus status = g_kvStoreNbDelegate->PutLocal(KEY_1, VALUE_1);
    EXPECT_TRUE(status == OK);
    status = g_kvStoreNbDelegate->Put(KEY_2, VALUE_1);
    EXPECT_TRUE(status == OK);

    /**
     * @tc.steps: step1. get data from local where key = KEY_2.
     * @tc.expected: step1. return value is NOT_FOUND.
     */
    Value valueResult;
    status = g_kvStoreNbDelegate->GetLocal(KEY_2, valueResult);
    EXPECT_EQ(status, NOT_FOUND);
    /**
     * @tc.steps: step2. get data from sync where key = KEY_1.
     * @tc.expected: step2. return value is NOT_FOUND.
     */
    status = g_kvStoreNbDelegate->Get(KEY_1, valueResult);
    EXPECT_EQ(status, NOT_FOUND);
    g_kvStoreNbDelegate->DeleteLocal(KEY_1);
    g_kvStoreNbDelegate->Delete(KEY_2);
}

/*
 * @tc.name: RepeatAction 001
 * @tc.desc: test that query the same entries repeatedly.
 * @tc.type: FUNC
 * @tc.require: SR000CCPOI
 * @tc.author: luqianfu
 */
HWTEST_F(DistributeddbNbCrudTest, RepeatAction001, TestSize.Level0)
{
    DBStatus status = g_kvStoreNbDelegate->PutLocal(KEY_1, VALUE_1);
    EXPECT_TRUE(status == OK);
    status = g_kvStoreNbDelegate->Put(KEY_1, VALUE_2);
    EXPECT_TRUE(status == OK);

    /**
     * @tc.steps: step1. get data from local data where key = KEY_1.
     * @tc.expected: step1. return value is VALUE_1.
     */
    Value valueResult;
    status = g_kvStoreNbDelegate->GetLocal(KEY_1, valueResult);
    EXPECT_EQ(status, OK);
    EXPECT_EQ(valueResult, VALUE_1);
    /**
     * @tc.steps: step2. get data from local data where key = KEY_1.
     * @tc.expected: step2. return value is VALUE_1.
     */
    status = g_kvStoreNbDelegate->GetLocal(KEY_1, valueResult);
    EXPECT_EQ(status, OK);
    EXPECT_EQ(valueResult, VALUE_1);
    /**
     * @tc.steps: step3. get data from sync data where key = KEY_1.
     * @tc.expected: step3. return value is VALUE_2.
     */
    status = g_kvStoreNbDelegate->Get(KEY_1, valueResult);
    EXPECT_EQ(status, OK);
    EXPECT_EQ(valueResult, VALUE_2);
    /**
     * @tc.steps: step4. get data from sync data where key = KEY_1.
     * @tc.expected: step4. return value is VALUE_2.
     */
    status = g_kvStoreNbDelegate->Get(KEY_1, valueResult);
    EXPECT_EQ(status, OK);
    EXPECT_EQ(valueResult, VALUE_2);
    g_kvStoreNbDelegate->DeleteLocal(KEY_1);
    g_kvStoreNbDelegate->Delete(KEY_1);
}

/*
 * @tc.name: RepeatAction 002
 * @tc.desc: test that put the same entries repeatedly.
 * @tc.type: FUNC
 * @tc.require: SR000CCPOI
 * @tc.author: luqianfu
 */
HWTEST_F(DistributeddbNbCrudTest, RepeatAction002, TestSize.Level0)
{
    /**
     * @tc.steps: step1. put (KEY_1, VALUE_1) to local db.
     * @tc.expected: step1. put succeed.
     */
    DBStatus status = g_kvStoreNbDelegate->PutLocal(KEY_1, VALUE_1);
    EXPECT_TRUE(status == OK);
    /**
     * @tc.steps: step2. put (KEY_1, VALUE_1) to local db again.
     * @tc.expected: step2. put succeed.
     */
    status = g_kvStoreNbDelegate->PutLocal(KEY_1, VALUE_1);
    EXPECT_TRUE(status == OK);
    /**
     * @tc.steps: step3. put (KEY_1, VALUE_2) to sync db.
     * @tc.expected: step3. put succeed.
     */
    status = g_kvStoreNbDelegate->Put(KEY_1, VALUE_2);
    EXPECT_TRUE(status == OK);
    /**
     * @tc.steps: step4. put (KEY_1, VALUE_2) to sync db.
     * @tc.expected: step4. put succeed.
     */
    status = g_kvStoreNbDelegate->Put(KEY_1, VALUE_2);
    EXPECT_TRUE(status == OK);

    /**
     * @tc.steps: step5. get data from local db where key = KEY_1.
     * @tc.expected: step5. return value is VALUE_1.
     */
    Value valueResult;
    status = g_kvStoreNbDelegate->GetLocal(KEY_1, valueResult);
    EXPECT_EQ(status, OK);
    EXPECT_EQ(valueResult, VALUE_1);
    /**
     * @tc.steps: step6. get data from sync db where key = KEY_1.
     * @tc.expected: step6. return value is VALUE_2.
     */
    status = g_kvStoreNbDelegate->Get(KEY_1, valueResult);
    EXPECT_EQ(status, OK);
    EXPECT_EQ(valueResult, VALUE_2);

    g_kvStoreNbDelegate->DeleteLocal(KEY_1);
    g_kvStoreNbDelegate->Delete(KEY_1);
}

/*
 * @tc.name: RepeatAction 003
 * @tc.desc: test that can update the same entries repeatedly.
 * @tc.type: FUNC
 * @tc.require: SR000CCPOI
 * @tc.author: luqianfu
 */
HWTEST_F(DistributeddbNbCrudTest, RepeatAction003, TestSize.Level0)
{
    DBStatus status = g_kvStoreNbDelegate->PutLocal(KEY_1, VALUE_1);
    EXPECT_TRUE(status == OK);
    status = g_kvStoreNbDelegate->Put(KEY_1, VALUE_2);
    EXPECT_TRUE(status == OK);
    /**
     * @tc.steps: step1. update (KEY_1, VALUE_2) to local db.
     * @tc.expected: step1. update success.
     */
    status = g_kvStoreNbDelegate->PutLocal(KEY_1, VALUE_2);
    EXPECT_TRUE(status == OK);
    /**
     * @tc.steps: step2. update (KEY_1, VALUE_2) to local db again.
     * @tc.expected: step2. update success.
     */
    status = g_kvStoreNbDelegate->PutLocal(KEY_1, VALUE_2);
    EXPECT_TRUE(status == OK);
    /**
     * @tc.steps: step3. update (KEY_1, VALUE_1) to sync db.
     * @tc.expected: step3. update success.
     */
    status = g_kvStoreNbDelegate->Put(KEY_1, VALUE_1);
    EXPECT_TRUE(status == OK);
    /**
     * @tc.steps: step4. update (KEY_1, VALUE_1) to sync db again.
     * @tc.expected: step4. update success.
     */
    status = g_kvStoreNbDelegate->Put(KEY_1, VALUE_1);
    EXPECT_TRUE(status == OK);

    /**
     * @tc.steps: step5. get data from local where key KEY_1.
     * @tc.expected: step5. return value is VALUE_2.
     */
    Value valueResult;
    status = g_kvStoreNbDelegate->GetLocal(KEY_1, valueResult);
    EXPECT_EQ(status, OK);
    EXPECT_EQ(valueResult, VALUE_2);
    /**
     * @tc.steps: step6. get data from sync where key KEY_1.
     * @tc.expected: step6. return value is VALUE_1.
     */
    status = g_kvStoreNbDelegate->Get(KEY_1, valueResult);
    EXPECT_EQ(status, OK);
    EXPECT_EQ(valueResult, VALUE_1);

    g_kvStoreNbDelegate->DeleteLocal(KEY_1);
    g_kvStoreNbDelegate->Delete(KEY_1);
}

/*
 * @tc.name: RepeatAction 004
 * @tc.desc: verify that it can delete the same entries repeatedly.
 * @tc.type: FUNC
 * @tc.require: SR000CCPOI
 * @tc.author: luqianfu
 */
HWTEST_F(DistributeddbNbCrudTest, RepeatAction004, TestSize.Level0)
{
    DBStatus status = g_kvStoreNbDelegate->PutLocal(KEY_1, VALUE_1);
    EXPECT_TRUE(status == OK);
    status = g_kvStoreNbDelegate->PutLocal(KEY_2, VALUE_2);
    EXPECT_TRUE(status == OK);
    status = g_kvStoreNbDelegate->Put(KEY_1, VALUE_2);
    EXPECT_TRUE(status == OK);
    status = g_kvStoreNbDelegate->Put(KEY_2, VALUE_1);
    EXPECT_TRUE(status == OK);

    /**
     * @tc.steps: step1. delete KEY_1 from local db.
     * @tc.expected: step1. delete succeed.
     */
    status = g_kvStoreNbDelegate->DeleteLocal(KEY_1);
    EXPECT_EQ(status, OK);
    /**
     * @tc.steps: step2. delete KEY_2 from sync db.
     * @tc.expected: step2. delete succeed.
     */
    status = g_kvStoreNbDelegate->Delete(KEY_2);
    EXPECT_EQ(status, OK);
    /**
     * @tc.steps: step3. delete KEY_1 from local db again.
     * @tc.expected: step3. delete failed but return OK.
     */
    status = g_kvStoreNbDelegate->DeleteLocal(KEY_1);
    EXPECT_EQ(status, OK);
    /**
     * @tc.steps: step4. delete KEY_2 from sync db again.
     * @tc.expected: step4. delete failed but return OK.
     */
    status = g_kvStoreNbDelegate->Delete(KEY_2);
    EXPECT_EQ(status, OK);
    /**
     * @tc.steps: step5. get data from local db where key = KEY_1.
     * @tc.expected: step5. return value is NOT_FOUND.
     */
    Value realValue;
    status = g_kvStoreNbDelegate->GetLocal(KEY_1, realValue);
    EXPECT_EQ(status, NOT_FOUND);
    /**
     * @tc.steps: step6. get data from local db where key = KEY_2.
     * @tc.expected: step6. return value is VALUE_2.
     */
    status = g_kvStoreNbDelegate->GetLocal(KEY_2, realValue);
    EXPECT_EQ(status, OK);
    EXPECT_EQ(realValue, VALUE_2);
    /**
     * @tc.steps: step7. get data from sync db where key = KEY_1.
     * @tc.expected: step7. return value is VALUE_2.
     */
    status = g_kvStoreNbDelegate->Get(KEY_1, realValue);
    EXPECT_EQ(status, OK);
    EXPECT_EQ(realValue, VALUE_2);
    /**
     * @tc.steps: step8. get data from sync db where key = KEY_2.
     * @tc.expected: step8. return value is NOT_FOUND.
     */
    status = g_kvStoreNbDelegate->Get(KEY_2, realValue);
    EXPECT_EQ(status, NOT_FOUND);

    g_kvStoreNbDelegate->DeleteLocal(KEY_2);
    g_kvStoreNbDelegate->Delete(KEY_1);
}

/*
 * @tc.name: RepeatAction 005
 * @tc.desc: test that delete the entries do not exist will return OK.
 * @tc.type: FUNC
 * @tc.require: SR000CCPOI
 * @tc.author: luqianfu
 */
HWTEST_F(DistributeddbNbCrudTest, RepeatAction005, TestSize.Level0)
{
    DBStatus status = g_kvStoreNbDelegate->PutLocal(KEY_1, VALUE_1);
    EXPECT_TRUE(status == OK);
    status = g_kvStoreNbDelegate->Put(KEY_2, VALUE_1);
    EXPECT_TRUE(status == OK);
    /**
     * @tc.steps: step1. delete KEY_2 from local db.
     * @tc.expected: step1. delete failed but return OK.
     */
    status = g_kvStoreNbDelegate->DeleteLocal(KEY_2);
    EXPECT_EQ(status, OK);
    /**
     * @tc.steps: step2. delete KEY_1 from sync db.
     * @tc.expected: step2. delete failed but return OK.
     */
    status = g_kvStoreNbDelegate->Delete(KEY_1);
    EXPECT_EQ(status, OK);

    /**
     * @tc.steps: step3. get data from local db where key = KEY_1.
     * @tc.expected: step3. return value is VALUE_1.
     */
    Value valueResult;
    status = g_kvStoreNbDelegate->GetLocal(KEY_1, valueResult);
    EXPECT_EQ(status, OK);
    EXPECT_EQ(valueResult, VALUE_1);
    /**
     * @tc.steps: step4. get data from local db where key = KEY_2.
     * @tc.expected: step4. return value is NOT_FOUND.
     */
    status = g_kvStoreNbDelegate->GetLocal(KEY_2, valueResult);
    EXPECT_EQ(status, NOT_FOUND);
    /**
     * @tc.steps: step5. get data from sync db where key = KEY_1.
     * @tc.expected: step5. return value is NOT_FOUND.
     */
    status = g_kvStoreNbDelegate->Get(KEY_1, valueResult);
    EXPECT_EQ(status, NOT_FOUND);
    /**
     * @tc.steps: step6. get data from sync db where key = KEY_2.
     * @tc.expected: step6. return value is VALUE_1.
     */
    status = g_kvStoreNbDelegate->Get(KEY_2, valueResult);
    EXPECT_EQ(status, OK);
    EXPECT_EQ(valueResult, VALUE_1);
    g_kvStoreNbDelegate->DeleteLocal(KEY_1);
    g_kvStoreNbDelegate->Delete(KEY_2);
}

/*
 * @tc.name: RepeatAction 006
 * @tc.desc: test that can put entries which was deleted.
 * @tc.type: FUNC
 * @tc.require: SR000CCPOI
 * @tc.author: luqianfu
 */
HWTEST_F(DistributeddbNbCrudTest, RepeatAction006, TestSize.Level0)
{
    DBStatus status = g_kvStoreNbDelegate->PutLocal(KEY_1, VALUE_1);
    EXPECT_TRUE(status == OK);
    status = g_kvStoreNbDelegate->Put(KEY_1, VALUE_2);
    EXPECT_TRUE(status == OK);
    /**
     * @tc.steps: step1. delete KEY_1 from local db.
     * @tc.expected: step1. delete succeed.
     */
    g_kvStoreNbDelegate->DeleteLocal(KEY_1);
    EXPECT_EQ(status, OK);
    /**
     * @tc.steps: step2. delete KEY_1 from sync db.
     * @tc.expected: step2. delete succeed.
     */
    g_kvStoreNbDelegate->Delete(KEY_1);
    EXPECT_EQ(status, OK);
    /**
     * @tc.steps: step3. update (KEY_1, VALUE_1) to local db.
     * @tc.expected: step3. update succeed.
     */
    Value valueResult;
    status = g_kvStoreNbDelegate->PutLocal(KEY_1, VALUE_1);
    EXPECT_TRUE(status == OK);
    /**
     * @tc.steps: step4. update (KEY_1, VALUE_2) to sync db.
     * @tc.expected: step4. update succeed.
     */
    status = g_kvStoreNbDelegate->Put(KEY_1, VALUE_2);
    EXPECT_TRUE(status == OK);
    /**
     * @tc.steps: step5. get data from local db where key = KEY_1.
     * @tc.expected: step5. return value is VALUE_1.
     */
    status = g_kvStoreNbDelegate->GetLocal(KEY_1, valueResult);
    EXPECT_EQ(status, OK);
    EXPECT_EQ(valueResult, VALUE_1);
    /**
     * @tc.steps: step6. get data from sync db where key = KEY_1.
     * @tc.expected: step6. return value is VALUE_2.
     */
    status = g_kvStoreNbDelegate->Get(KEY_1, valueResult);
    EXPECT_EQ(status, OK);
    EXPECT_EQ(valueResult, VALUE_2);
    /**
     * @tc.steps: step7. get data from local db where key = KEY_1.
     * @tc.expected: step7. return value is VALUE_2.
     */
    status = g_kvStoreNbDelegate->PutLocal(KEY_1, VALUE_2);
    EXPECT_TRUE(status == OK);
    /**
     * @tc.steps: step8. get data from sync db where key = KEY_1.
     * @tc.expected: step8. return value is VALUE_1.
     */
    status = g_kvStoreNbDelegate->Put(KEY_1, VALUE_1);
    EXPECT_TRUE(status == OK);
    /**
     * @tc.steps: step9. get data from local db where key = KEY_1.
     * @tc.expected: step9. return value is VALUE_2.
     */
    status = g_kvStoreNbDelegate->GetLocal(KEY_1, valueResult);
    EXPECT_EQ(status, OK);
    EXPECT_EQ(valueResult, VALUE_2);
    /**
     * @tc.steps: step10. get data from sync db where key = KEY_1.
     * @tc.expected: step10. return value is VALUE_1.
     */
    status = g_kvStoreNbDelegate->Get(KEY_1, valueResult);
    EXPECT_EQ(status, OK);
    EXPECT_EQ(valueResult, VALUE_1);
    g_kvStoreNbDelegate->DeleteLocal(KEY_1);
    g_kvStoreNbDelegate->Delete(KEY_1);
}

/*
 * @tc.name: Pressure 001
 * @tc.desc: verify that the get interface can check the key params' effectiveness.
 * @tc.type: FUNC
 * @tc.require: SR000CCPOI
 * @tc.author: luqianfu
 */
HWTEST_F(DistributeddbNbCrudTest, Pressure001, TestSize.Level1)
{
    DistributedDB::Key eKey1, eKey2, eKey3, eKey4;
    eKey1 = {};
    eKey2.assign(ONE_K_LONG_STRING, (uint8_t)'a');
    eKey3.assign(ONE_K_LONG_STRING + 1, (uint8_t)'b');
    eKey4 = { 'a', 'b', 'c', 'D', 'E', 'F', '2', '4', '5', 199, 1, 255, 0 };

    /**
     * @tc.steps: step1. get data from local db where key = eKey1.
     * @tc.expected: step1. get failed and return INVALID_ARGS.
     */
    Value valueResult;
    DBStatus status = g_kvStoreNbDelegate->GetLocal(eKey1, valueResult);
    EXPECT_EQ(status, INVALID_ARGS);
    /**
     * @tc.steps: step2. get data from local db where key = eKey2.
     * @tc.expected: step2. get failed and return NOT_FOUND.
     */
    status = g_kvStoreNbDelegate->GetLocal(eKey2, valueResult);
    EXPECT_EQ(status, NOT_FOUND);
    /**
     * @tc.steps: step3. get data from local db where key = eKey3.
     * @tc.expected: step3. get failed and return INVALID_ARGS.
     */
    status = g_kvStoreNbDelegate->GetLocal(eKey3, valueResult);
    EXPECT_EQ(status, INVALID_ARGS);
    /**
     * @tc.steps: step4. get data from local db where key = eKey4.
     * @tc.expected: step4. get failed and return NOT_FOUND.
     */
    status = g_kvStoreNbDelegate->GetLocal(eKey4, valueResult);
    EXPECT_EQ(status, NOT_FOUND);
    /**
     * @tc.steps: step5. get data from local db where key = eKey1.
     * @tc.expected: step5. get failed and return INVALID_ARGS.
     */
    status = g_kvStoreNbDelegate->Get(eKey1, valueResult);
    EXPECT_EQ(status, INVALID_ARGS);
    /**
     * @tc.steps: step6. get data from local db where key = eKey2.
     * @tc.expected: step6. get failed and return NOT_FOUND.
     */
    status = g_kvStoreNbDelegate->Get(eKey2, valueResult);
    EXPECT_EQ(status, NOT_FOUND);
    /**
     * @tc.steps: step7. get data from local db where key = eKey.
     * @tc.expected: step7. get failed and return INVALID_ARGS.
     */
    status = g_kvStoreNbDelegate->Get(eKey3, valueResult);
    EXPECT_EQ(status, INVALID_ARGS);
    /**
     * @tc.steps: step8. get data from local db where key = eKey4.
     * @tc.expected: step8. get failed and return NOT_FOUND.
     */
    status = g_kvStoreNbDelegate->Get(eKey4, valueResult);
    EXPECT_EQ(status, NOT_FOUND);
}
/*
 * @tc.name: Pressure 002
 * @tc.desc: verify that the put interface can check key params 'effectiveness and 4M value.
 * @tc.type: FUNC
 * @tc.require: SR000CCPOI
 * @tc.author: luqianfu
 */
HWTEST_F(DistributeddbNbCrudTest, Pressure002, TestSize.Level1)
{
    DistributedDB::Value longValue;
    longValue.assign(FOUR_M_LONG_STRING, (uint8_t)'a');

    DistributedDB::Key eKey1, eKey2, eKey3, eKey4;
    eKey1 = {};
    eKey2.assign(ONE_K_LONG_STRING, (uint8_t)'a');
    eKey3.assign(ONE_K_LONG_STRING + 1, (uint8_t)'b');
    eKey4 = { 'a', 'b', 'c', 'D', 'E', 'F', '2', '4', '5', 199, 1, 255, 0 };
    /**
     * @tc.steps: step1. put (eKey1, longValue) to local db, among which ekey1 = {}.
     * @tc.expected: step1. put failed, and return INVALID_ARGS.
     */
    DBStatus status = g_kvStoreNbDelegate->PutLocal(eKey1, longValue);
    EXPECT_TRUE(status == INVALID_ARGS);
    /**
     * @tc.steps: step2. put (eKey2, longValue) to local db, among which size of ekey2 is 1K.
     * @tc.expected: step2. put succeed.
     */
    status = g_kvStoreNbDelegate->PutLocal(eKey2, longValue);
    EXPECT_TRUE(status == OK);
    /**
     * @tc.steps: step3. put (eKey3, longValue) to local db, among which size of ekey3 is 1k + 1.
     * @tc.expected: step3. put failed, and return INVALID_ARGS.
     */
    status = g_kvStoreNbDelegate->PutLocal(eKey3, longValue);
    EXPECT_TRUE(status == INVALID_ARGS);
    /**
     * @tc.steps: step4. put (eKey4, longValue) to local db, among which ekey4 contains [a-zA-Z0-9], [\0-\255],
     *    chinese and latins, and size of ekey4 is big than 0 and less or equal than 1K.
     * @tc.expected: step4. put succeed.
     */
    status = g_kvStoreNbDelegate->PutLocal(eKey4, longValue);
    EXPECT_TRUE(status == OK);
    /**
     * @tc.steps: step5. put (eKey1, longValue) to sync db, among which ekey1 = {}.
     * @tc.expected: step5. put failed, and return INVALID_ARGS.
     */
    status = g_kvStoreNbDelegate->Put(eKey1, longValue);
    EXPECT_TRUE(status == INVALID_ARGS);
    /**
     * @tc.steps: step6. put (eKey2, longValue) to sync db, among which size of ekey2 is 1K.
     * @tc.expected: step6. put succeed.
     */
    status = g_kvStoreNbDelegate->Put(eKey2, longValue);
    EXPECT_TRUE(status == OK);
    /**
     * @tc.steps: step7. put (eKey3, longValue) to sync db, among which size of ekey3 is 1k + 1.
     * @tc.expected: step7. put failed, and return INVALID_ARGS.
     */
    status = g_kvStoreNbDelegate->Put(eKey3, longValue);
    EXPECT_TRUE(status == INVALID_ARGS);
    /**
     * @tc.steps: step8. put (eKey4, longValue) to local db, among which ekey4 contains [a-zA-Z0-9], [\0-\255],
     *    chinese and latins, and size of ekey4 is big than 0 and less or equal than 1K.
     * @tc.expected: step8. put succeed.
     */
    status = g_kvStoreNbDelegate->Put(eKey4, longValue);
    EXPECT_TRUE(status == OK);
    g_kvStoreNbDelegate->DeleteLocal(eKey1);
    g_kvStoreNbDelegate->DeleteLocal(eKey2);
    g_kvStoreNbDelegate->DeleteLocal(eKey3);
    g_kvStoreNbDelegate->DeleteLocal(eKey4);
    g_kvStoreNbDelegate->Delete(eKey1);
    g_kvStoreNbDelegate->Delete(eKey2);
    g_kvStoreNbDelegate->Delete(eKey3);
    g_kvStoreNbDelegate->Delete(eKey4);
}
/*
 * @tc.name: Pressure 003
 * @tc.desc: verify that the update interface can check key params' effectiveness and value params' effectiveness.
 * @tc.type: FUNC
 * @tc.require: SR000CCPOI
 * @tc.author: luqianfu
 */
HWTEST_F(DistributeddbNbCrudTest, Pressure003, TestSize.Level1)
{
    DistributedDB::Value alphanumValue1, alphanumValue2, alphanumValue3, alphanumValue4;
    alphanumValue1 = {};
    alphanumValue2.assign(FOUR_M_LONG_STRING, (uint8_t)'a');
    alphanumValue3.assign(FOUR_M_LONG_STRING + 1, (uint8_t)'b');
    int chr;
    for (chr = 'a'; chr <= 'z'; ++chr) {
        alphanumValue4.push_back(chr);
    }
    for (chr = 'A'; chr <= 'Z'; ++chr) {
        alphanumValue4.push_back(chr);
    }
    for (chr = '0'; chr <= '9'; ++chr) {
        alphanumValue4.push_back(chr);
    }
    alphanumValue4.push_back(1);
    alphanumValue4.push_back(0);
    alphanumValue4.push_back(255);
    alphanumValue4.push_back(9);
    EXPECT_TRUE((g_kvStoreNbDelegate->PutLocal(KEY_1, VALUE_1)) == OK);
    EXPECT_TRUE((g_kvStoreNbDelegate->PutLocal(KEY_2, VALUE_2)) == OK);
    EXPECT_TRUE((g_kvStoreNbDelegate->PutLocal(KEY_3, VALUE_3)) == OK);
    EXPECT_TRUE((g_kvStoreNbDelegate->PutLocal(KEY_4, VALUE_4)) == OK);
    EXPECT_TRUE((g_kvStoreNbDelegate->Put(KEY_1, VALUE_1)) == OK);
    EXPECT_TRUE((g_kvStoreNbDelegate->Put(KEY_2, VALUE_2)) == OK);
    EXPECT_TRUE((g_kvStoreNbDelegate->Put(KEY_3, VALUE_3)) == OK);
    EXPECT_TRUE((g_kvStoreNbDelegate->Put(KEY_4, VALUE_4)) == OK);
    /**
     * @tc.steps: step1. put (KEY_1, alphanumValue1) to local db, among which size of alphanumValue1 0.
     * @tc.expected: step1. put succeed and return OK.
     */
    EXPECT_TRUE((g_kvStoreNbDelegate->PutLocal(KEY_1, alphanumValue1)) == OK);
    /**
     * @tc.steps: step2. put (KEY_2, alphanumValue2) to local db, among which size of alphanumValue2 4M.
     * @tc.expected: step2. put succeed and return OK.
     */
    EXPECT_TRUE((g_kvStoreNbDelegate->PutLocal(KEY_2, alphanumValue2)) == OK);
    /**
     * @tc.steps: step3. put (KEY_3, alphanumValue3) to local db, among which size of alphanumValue3 (4M + 1).
     * @tc.expected: step3. put failed and return INVALID_ARGS.
     */
    EXPECT_TRUE((g_kvStoreNbDelegate->PutLocal(KEY_3, alphanumValue3)) == INVALID_ARGS);
    /**
     * @tc.steps: step4. put (KEY_4, alphanumValue4) to local db, among which alphanumValue4
     *    contains [a-zA-Z0-9], [\0-\255], chinese, latins.
     * @tc.expected: step4. put succeed and return OK.
     */
    EXPECT_TRUE((g_kvStoreNbDelegate->PutLocal(KEY_4, alphanumValue4)) == OK);
    /**
     * @tc.steps: step5. put (KEY_1, alphanumValue1) to sync db, among which size of alphanumValue1 0.
     * @tc.expected: step5. put succeed and return OK.
     */
    EXPECT_TRUE((g_kvStoreNbDelegate->Put(KEY_1, alphanumValue1)) == OK);
    /**
     * @tc.steps: step6. put (KEY_2, alphanumValue2) to sync db, among which size of alphanumValue2 4M.
     * @tc.expected: step6. put succeed and return OK.
     */
    EXPECT_TRUE((g_kvStoreNbDelegate->Put(KEY_2, alphanumValue2)) == OK);
    /**
     * @tc.steps: step7. put (KEY_3, alphanumValue3) to local db, among which size of alphanumValue3 (4M + 1).
     * @tc.expected: step7. put failed and return INVALID_ARGS.
     */
    EXPECT_TRUE((g_kvStoreNbDelegate->Put(KEY_3, alphanumValue3)) == INVALID_ARGS);
    /**
     * @tc.steps: step8. put (KEY_4, alphanumValue4) to local db, among which alphanumValue4
     *    contains [a-zA-Z0-9], [\0-\255], chinese, latins.
     * @tc.expected: step8. put succeed and return OK.
     */
    EXPECT_TRUE((g_kvStoreNbDelegate->Put(KEY_4, alphanumValue4)) == OK);
    g_kvStoreNbDelegate->DeleteLocal(KEY_1);
    g_kvStoreNbDelegate->DeleteLocal(KEY_2);
    g_kvStoreNbDelegate->DeleteLocal(KEY_3);
    g_kvStoreNbDelegate->DeleteLocal(KEY_4);
    g_kvStoreNbDelegate->Delete(KEY_1);
    g_kvStoreNbDelegate->Delete(KEY_2);
    g_kvStoreNbDelegate->Delete(KEY_3);
    g_kvStoreNbDelegate->Delete(KEY_4);
}

/*
 * @tc.name: Pressure 004
 * @tc.desc: verify that check value params' effectiveness of delete interface.
 * @tc.type: FUNC
 * @tc.require: SR000CCPOI
 * @tc.author: luqianfu
 */
HWTEST_F(DistributeddbNbCrudTest, Pressure004, TestSize.Level1)
{
    DistributedDB::Key eKey1, eKey2, eKey3, eKey4;
    eKey1 = {};
    eKey2.assign(ONE_K_LONG_STRING, (uint8_t)'a');
    eKey3.assign(ONE_K_LONG_STRING + 1, (uint8_t)'b');
    eKey4 = { 'a', 'b', 'c', 'D', 'E', 'F', '2', '4', '5', 199, 255, 0, 1 };
    DBStatus status = g_kvStoreNbDelegate->PutLocal(eKey1, VALUE_1);
    EXPECT_TRUE(status == INVALID_ARGS);
    status = g_kvStoreNbDelegate->PutLocal(eKey2, VALUE_1);
    EXPECT_TRUE(status == OK);
    status = g_kvStoreNbDelegate->PutLocal(eKey3, VALUE_1);
    EXPECT_TRUE(status == INVALID_ARGS);
    status = g_kvStoreNbDelegate->PutLocal(eKey4, VALUE_1);
    EXPECT_TRUE(status == OK);
    status = g_kvStoreNbDelegate->Put(eKey1, VALUE_2);
    EXPECT_TRUE(status == INVALID_ARGS);
    status = g_kvStoreNbDelegate->Put(eKey2, VALUE_2);
    EXPECT_TRUE(status == OK);
    status = g_kvStoreNbDelegate->Put(eKey3, VALUE_2);
    EXPECT_TRUE(status == INVALID_ARGS);
    status = g_kvStoreNbDelegate->Put(eKey4, VALUE_2);
    EXPECT_TRUE(status == OK);
    /**
     * @tc.steps: step1. delete eKey1 from local db, where key = eKey1, which eKey1 is null.
     * @tc.expected: step1. delete failed and return INVALID_ARGS.
     */
    status = g_kvStoreNbDelegate->DeleteLocal(eKey1);
    EXPECT_TRUE(status == INVALID_ARGS);
    /**
     * @tc.steps: step2. delete eKey2 from local db, where key = eKey2, which the size of eKey2 is 1K.
     * @tc.expected: step2. delete succeed and return OK.
     */
    status = g_kvStoreNbDelegate->DeleteLocal(eKey2);
    EXPECT_TRUE(status == OK);
    /**
     * @tc.steps: step3. delete eKey3 from local db, where key = eKey3, which the size of eKey3 is 1k + 1.
     * @tc.expected: step3. delete failed and return INVALID_ARGS.
     */
    status = g_kvStoreNbDelegate->DeleteLocal(eKey3);
    EXPECT_TRUE(status == INVALID_ARGS);
    /**
     * @tc.steps: step4. delete eKey4 from local db, where key = eKey4, which eKey2
     *    contains [a-zA-Z0-9], [\0-\255], chinese, latins.
     * @tc.expected: step4. delete succeed and return OK.
     */
    status = g_kvStoreNbDelegate->DeleteLocal(eKey4);
    EXPECT_TRUE(status == OK);
    /**
     * @tc.steps: step5. delete eKey1 from sync db, where key = eKey1, which eKey1 is null.
     * @tc.expected: step5. delete failed and return INVALID_ARGS.
     */
    status = g_kvStoreNbDelegate->Delete(eKey1);
    EXPECT_TRUE(status == INVALID_ARGS);
    /**
     * @tc.steps: step6. delete eKey2 from sync db, where key = eKey2, which the size of eKey2 is 1K.
     * @tc.expected: step6. delete succeed and return OK.
     */
    status = g_kvStoreNbDelegate->Delete(eKey2);
    EXPECT_TRUE(status == OK);
    /**
     * @tc.steps: step7. delete eKey3 from sync db, where key = eKey3, which the size of eKey3 is 1k + 1.
     * @tc.expected: step7. delete failed and return INVALID_ARGS.
     */
    status = g_kvStoreNbDelegate->Delete(eKey3);
    EXPECT_TRUE(status == INVALID_ARGS);
    /**
     * @tc.steps: step8. delete eKey4 from sync db, where key = eKey4, which eKey2
     *    contains [a-zA-Z0-9], [\0-\255], chinese, latins.
     * @tc.expected: step8. delete succeed and return OK.
     */
    status = g_kvStoreNbDelegate->Delete(eKey4);
    EXPECT_TRUE(status == OK);
}
/*
 * @tc.name: Pressure 005
 * @tc.desc: verify that can operate a super huge key and a super huge value.
 * @tc.type: FUNC
 * @tc.require: SR000CCPOI
 * @tc.author: luqianfu
 */
HWTEST_F(DistributeddbNbCrudTest, Pressure005, TestSize.Level2)
{
    DistributedDB::Key longKey;
    DistributedDB::Value longValue1, longValue2;
    longKey.assign(ONE_K_LONG_STRING, (uint8_t)'a');
    longValue1.assign(FOUR_M_LONG_STRING, (uint8_t)'b');
    longValue2.assign(FOUR_M_LONG_STRING, (uint8_t)'c');
    /**
     * @tc.steps: step1. put a longkey and longvalue (longKey, longValue1) to local db.
     * @tc.expected: step1. put succeed and return OK.
     */
    DBStatus status = g_kvStoreNbDelegate->PutLocal(longKey, longValue1);
    EXPECT_TRUE(status == OK);
    /**
     * @tc.steps: step2. put a longkey and longvalue (longKey, longValue2) to sync db.
     * @tc.expected: step2. put succeed and return OK.
     */
    status = g_kvStoreNbDelegate->Put(longKey, longValue2);
    EXPECT_TRUE(status == OK);
    /**
     * @tc.steps: step3. get data from local db where key = longKey.
     * @tc.expected: step3. succeed get value is longValue1.
     */
    Value valueResult;
    status = g_kvStoreNbDelegate->GetLocal(longKey, valueResult);
    EXPECT_EQ(status, OK);
    EXPECT_EQ(valueResult, longValue1);
    /**
     * @tc.steps: step4. get data from sync db where key = longKey.
     * @tc.expected: step4. succeed get value is longValue2.
     */
    status = g_kvStoreNbDelegate->Get(longKey, valueResult);
    EXPECT_EQ(status, OK);
    EXPECT_EQ(valueResult, longValue2);
    /**
     * @tc.steps: step5. put a longkey and longvalue (longKey, longValue2) to local db.
     * @tc.expected: step5. put succeed and return OK.
     */
    status = g_kvStoreNbDelegate->PutLocal(longKey, longValue2);
    EXPECT_TRUE(status == OK);
    /**
     * @tc.steps: step6. put a longkey and longvalue (longKey, longValue1) to sync db.
     * @tc.expected: step6. put succeed and return OK.
     */
    status = g_kvStoreNbDelegate->Put(longKey, longValue1);
    EXPECT_TRUE(status == OK);
    /**
     * @tc.steps: step7. get data from local db where key = longKey.
     * @tc.expected: step7. succeed get value is longValue2.
     */
    status = g_kvStoreNbDelegate->GetLocal(longKey, valueResult);
    EXPECT_EQ(status, OK);
    EXPECT_EQ(valueResult, longValue2);
    /**
     * @tc.steps: step8. get data from sync db where key = longKey.
     * @tc.expected: step8. succeed get value is longValue1.
     */
    status = g_kvStoreNbDelegate->Get(longKey, valueResult);
    EXPECT_EQ(status, OK);
    EXPECT_EQ(valueResult, longValue1);
    /**
     * @tc.steps: step9. delete data from local db where key = longKey.
     * @tc.expected: step9. delete succeed and return ok.
     */
    status = g_kvStoreNbDelegate->DeleteLocal(longKey);
    EXPECT_TRUE(status == OK);
    /**
     * @tc.steps: step10. delete data from sync db where key = longKey.
     * @tc.expected: step10. delete succeed and return ok.
     */
    status = g_kvStoreNbDelegate->Delete(longKey);
    EXPECT_TRUE(status == OK);
    /**
     * @tc.steps: step11. get data from local db where key = longKey.
     * @tc.expected: step11. get failed and return NOT_FOUND.
     */
    status = g_kvStoreNbDelegate->GetLocal(longKey, valueResult);
    EXPECT_TRUE(status == NOT_FOUND);
    /**
     * @tc.steps: step12. get data from sync db where key = longKey.
     * @tc.expected: step12. get failed and return NOT_FOUND.
     */
    status = g_kvStoreNbDelegate->Get(longKey, valueResult);
    EXPECT_TRUE(status == NOT_FOUND);
}

void PressureForBasicCrud(Key &longKey, Value &valueResult)
{
    /**
     * @tc.steps: step1. put (longKey, VALUE_1) to local db.
     * @tc.expected: step1. put succeed and return OK.
     */
    DBStatus status = g_kvStoreNbDelegate->PutLocal(longKey, VALUE_1);
    EXPECT_TRUE(status == OK);
    /**
     * @tc.steps: step2. put (longKey, VALUE_2) to sync db.
     * @tc.expected: step2. put succeed and return OK.
     */
    status = g_kvStoreNbDelegate->Put(longKey, VALUE_2);
    EXPECT_TRUE(status == OK);
    /**
     * @tc.steps: step3. get data from local db where key = longKey.
     * @tc.expected: step3. get success and value = VALUE_1.
     */
    status = g_kvStoreNbDelegate->GetLocal(longKey, valueResult);
    EXPECT_EQ(status, OK);
    EXPECT_EQ(valueResult, VALUE_1);
    /**
     * @tc.steps: step4. get data from sync db where key = longKey.
     * @tc.expected: step4. get success and value = VALUE_2.
     */
    status = g_kvStoreNbDelegate->Get(longKey, valueResult);
    EXPECT_EQ(status, OK);
    EXPECT_EQ(valueResult, VALUE_2);
    /**
     * @tc.steps: step5. delete data from local db where key = longKey.
     * @tc.expected: step5. delete succeed and return ok.
     */
    status = g_kvStoreNbDelegate->DeleteLocal(longKey);
    EXPECT_TRUE(status == OK);
    /**
     * @tc.steps: step6. delete data from sync db where key = longKey.
     * @tc.expected: step6. delete succeed and return ok.
     */
    status = g_kvStoreNbDelegate->Delete(longKey);
    EXPECT_TRUE(status == OK);
    /**
     * @tc.steps: step7. get data from local db where key = longKey.
     * @tc.expected: step7. get failed and return NOT_FOUND.
     */
    status = g_kvStoreNbDelegate->GetLocal(longKey, valueResult);
    EXPECT_EQ(status, NOT_FOUND);
    /**
     * @tc.steps: step8. get data from sync db where key = longKey.
     * @tc.expected: step8. get failed and return NOT_FOUND.
     */
    status = g_kvStoreNbDelegate->Get(longKey, valueResult);
    EXPECT_EQ(status, NOT_FOUND);
    /**
     * @tc.steps: step9. put (longKey, VALUE_1) to local db.
     * @tc.expected: step9. put succeed and return OK.
     */
    status = g_kvStoreNbDelegate->PutLocal(longKey, VALUE_1);
    EXPECT_TRUE(status == OK);
    /**
     * @tc.steps: step10. put (longKey, VALUE_2) to sync db.
     * @tc.expected: step10. put succeed and return OK.
     */
    status = g_kvStoreNbDelegate->Put(longKey, VALUE_2);
    EXPECT_TRUE(status == OK);
}

/*
 * @tc.name: Pressure 006
 * @tc.desc: verify that can crud one key-value repeately.
 * @tc.type: FUNC
 * @tc.require: SR000CCPOI
 * @tc.author: luqianfu
 */
HWTEST_F(DistributeddbNbCrudTest, Pressure006, TestSize.Level1)
{
    DistributedDB::Key longKey;
    longKey.assign(ONE_K_LONG_STRING, (uint8_t)'a');
    Value valueResult;
    PressureForBasicCrud(longKey, valueResult);
    /**
     * @tc.steps: step1. get data from local db where key = longKey.
     * @tc.expected: step1. get success and value = VALUE_1.
     */
    DBStatus status = g_kvStoreNbDelegate->GetLocal(longKey, valueResult);
    EXPECT_EQ(status, OK);
    EXPECT_EQ(valueResult, VALUE_1);
    /**
     * @tc.steps: step2. get data from sync db where key = longKey.
     * @tc.expected: step2. get success and value = VALUE_2.
     */
    status = g_kvStoreNbDelegate->Get(longKey, valueResult);
    EXPECT_EQ(status, OK);
    EXPECT_EQ(valueResult, VALUE_2);
    /**
     * @tc.steps: step3. put (longKey, VALUE_2) to local db.
     * @tc.expected: step3. put succeed and return OK.
     */
    status = g_kvStoreNbDelegate->PutLocal(longKey, VALUE_2);
    EXPECT_TRUE(status == OK);
    /**
     * @tc.steps: step4. put (longKey, VALUE_1) to sync db.
     * @tc.expected: step4. put succeed and return OK.
     */
    status = g_kvStoreNbDelegate->Put(longKey, VALUE_1);
    EXPECT_TRUE(status == OK);
    /**
     * @tc.steps: step5. get data from local db where key = longKey.
     * @tc.expected: step5. get success and value = VALUE_2.
     */
    status = g_kvStoreNbDelegate->GetLocal(longKey, valueResult);
    EXPECT_EQ(status, OK);
    EXPECT_EQ(valueResult, VALUE_2);
    /**
     * @tc.steps: step6. get data from sync db where key = longKey.
     * @tc.expected: step6. get success and value = VALUE_1.
     */
    status = g_kvStoreNbDelegate->Get(longKey, valueResult);
    EXPECT_EQ(status, OK);
    EXPECT_EQ(valueResult, VALUE_1);
    /**
     * @tc.steps: step7. delete data from local db where key = longKey.
     * @tc.expected: step7. delete succeed and return ok.
     */
    status = g_kvStoreNbDelegate->DeleteLocal(longKey);
    EXPECT_TRUE(status == OK);
    /**
     * @tc.steps: step8. delete data from sync db where key = longKey.
     * @tc.expected: step8. delete succeed and return ok.
     */
    status = g_kvStoreNbDelegate->Delete(longKey);
    EXPECT_TRUE(status == OK);
    /**
     * @tc.steps: step9. get data from local db where key = longKey.
     * @tc.expected: step9. get failed and return NOT_FOUND.
     */
    status = g_kvStoreNbDelegate->GetLocal(longKey, valueResult);
    EXPECT_EQ(status, NOT_FOUND);
    /**
     * @tc.steps: step10. get data from sync db where key = longKey.
     * @tc.expected: step10. get failed and return NOT_FOUND.
     */
    status = g_kvStoreNbDelegate->Get(longKey, valueResult);
    EXPECT_EQ(status, NOT_FOUND);
}

vector<Value> GenerateDataForBatch(vector<Key> &KeyCrud)
{
    DistributedDB::Key key1, key2, key3, key4, key5, key6, key7, key8;
    key1 = {'a', 'b', 'c'};
    key2 = {'a', 'b', 'c', 'd', 'e'};
    key3 = {'a', 'b', 'c'};
    key4 = {'a', 'b', 'c', 'd', 'a', 's', 'd'};
    key5 = {'a', 'b', 'c', 'd', 's'};
    key6 = {'b', 'c'};
    key7 = {'d', 'a', 'b'};
    key8 = {'d', 'a', 'b', 'c'};
    KeyCrud = {
        key1, key2, key3,
        key4, key5, key6,
        key7, key8
    };
    DistributedDB::Value value1, value2, value3, value4, value5, value6, value7, value8;
    value1 = {'l', 'o', 'c', 'a', 'l', 'V', '1'};
    value2 = {'l', 'o', 'c', 'a', 'l', 'V', '2'};
    value3 = {'s', 'y', 'n', 'V', '1'};
    value4 = {'s', 'y', 'n', 'V', '2'};
    value5 = {'s', 'y', 'n', 'V', '3'};
    value6 = {'s', 'y', 'n', 'V', '4'};
    value7 = {'s', 'y', 'n', 'V', '5'};
    value8 = {'s', 'y', 'n', 'V', '6'};
    vector<Value> ValueCrud = {
        value1, value2, value3,
        value4, value5, value6,
        value7, value8
    };
    return ValueCrud;
}

void PutData(vector<Key> &KeyCrud, vector<Value> &ValueCrud)
{
    ASSERT_TRUE(g_kvStoreNbDelegate != nullptr);
    DBStatus status;
    int index;
    for (index = 0; index < LOCAL_OPER_CNT; ++index) {
        status = g_kvStoreNbDelegate->PutLocal(KeyCrud[index], ValueCrud[index]);
        EXPECT_TRUE(status == OK);
    }
    for (index = LOCAL_OPER_CNT; index < LOCAL_OPER_CNT + NATIVE_OPER_CNT; ++index) {
        status = g_kvStoreNbDelegate->Put(KeyCrud[index], ValueCrud[index]);
        EXPECT_TRUE(status == OK);
    }
}

/*
 * @tc.name: Pressure 007
 * @tc.desc: verify that can get batch data use keyprefix from db.
 * @tc.type: FUNC
 * @tc.require: SR000CCPOI
 * @tc.author: luqianfu
 */
HWTEST_F(DistributeddbNbCrudTest, Pressure007, TestSize.Level0)
{
    vector<Key> KeyCrud;
    vector<Value> ValueCrud = GenerateDataForBatch(KeyCrud);
    PutData(KeyCrud, ValueCrud);
    DistributedDB::Key key9, key10;
    key9 = { 'a', 'b' };
    key10 = { 'a', 'b', 'c', 'd', 'e' };
    std::vector<Entry> entries;
    /**
     * @tc.steps: step1. get entries from db use keyprefix key9 = "ab".
     * @tc.expected: step1. get entries succeed and entries contains ({abcde}, {localV2}),
     *  "abc, syncV1"; "abcdasd, syncV2".
     */
    DBStatus status = g_kvStoreNbDelegate->GetEntries(key9, entries);
    EXPECT_TRUE(status == OK);
    int iCount = 3;
    DistributedDB::Key rKey[iCount];
    rKey[0] = KeyCrud[2];
    rKey[1] = KeyCrud[3];
    rKey[2] = KeyCrud[4];
    DistributedDB::Value rValue[iCount];
    rValue[0] = ValueCrud[2];
    rValue[1] = ValueCrud[3];
    rValue[2] = ValueCrud[4];

    for (unsigned long index = 0; index < entries.size(); index++) {
        EXPECT_EQ(entries[index].key, rKey[index]);
        EXPECT_EQ(entries[index].value, rValue[index]);
    }
    /**
     * @tc.steps: step2. get entries from db use keyprefix key10 = "abcde".
     * @tc.expected: step2. get entries failed and return NOT_FOUND
     */
    status = g_kvStoreNbDelegate->GetEntries(key10, entries);
    EXPECT_TRUE(status == NOT_FOUND);
    g_kvStoreNbDelegate->DeleteLocal(KeyCrud[0]);
    g_kvStoreNbDelegate->DeleteLocal(KeyCrud[1]);
    g_kvStoreNbDelegate->Delete(KeyCrud[2]);
    g_kvStoreNbDelegate->Delete(KeyCrud[3]);
    g_kvStoreNbDelegate->Delete(KeyCrud[4]);
    g_kvStoreNbDelegate->Delete(KeyCrud[5]);
    g_kvStoreNbDelegate->Delete(KeyCrud[6]);
    g_kvStoreNbDelegate->Delete(KeyCrud[7]);
}

/*
 * @tc.name: Pressure 008
 * @tc.desc: verify that key effectiveness of getEntries interface.
 * @tc.type: FUNC
 * @tc.require: SR000CCPOI
 * @tc.author: luqianfu
 */
HWTEST_F(DistributeddbNbCrudTest, Pressure008, TestSize.Level1)
{
    DistributedDB::Key longKey1, longKey2, longKey3;
    longKey1.assign(ONE_K_LONG_STRING, (uint8_t)'a');
    longKey2.assign(ONE_K_LONG_STRING + 1, (uint8_t)'b');
    longKey3 = { 'a', 'b', 'c', 'D', 'E', 'F', '2', '4', '5', 199, 255, 0 };
    Value value1, value3;
    DBStatus status = g_kvStoreNbDelegate->PutLocal(longKey1, value1);
    EXPECT_TRUE(status == OK);
    status = g_kvStoreNbDelegate->Put(longKey3, value3);
    EXPECT_TRUE(status == OK);
    DistributedDB::Key key9, key10;
    key9 = { 'a', 'b' };
    key10 = { 'a', 'b', 'c', 'd', 'e' };
    std::vector<Entry> entries;
    /**
     * @tc.steps: step1. get entries from db use keyPrefix = longKey1 size of which is 1K.
     * @tc.expected: step1. get entries failed and return NOT_FOUND
     */
    status = g_kvStoreNbDelegate->GetEntries(longKey1, entries);
    EXPECT_TRUE(status == NOT_FOUND);
    /**
     * @tc.steps: step2. get entries from db use keyPrefix = longKey2 size of which is 1k + 1.
     * @tc.expected: step2. get entries failed and return INVALID_ARGS
     */
    status = g_kvStoreNbDelegate->GetEntries(longKey2, entries);
    EXPECT_TRUE(status == INVALID_ARGS);
    /**
     * @tc.steps: step3. get entries from db use keyPrefix = longKey3
     *     which contains [a-zA-Z0-9], [\0-\255], chinese, latins.
     * @tc.expected: step3. get entries success and return the entries has longKey3 keyPrefix.
     */
    status = g_kvStoreNbDelegate->GetEntries(longKey3, entries);
    EXPECT_TRUE(status == OK);
    EXPECT_TRUE(entries.size() == 1);
    g_kvStoreNbDelegate->DeleteLocal(longKey1);
    g_kvStoreNbDelegate->Delete(longKey3);
}

/*
 * @tc.name: Pressure 010
 * @tc.desc: verify can't crud after db is closed.
 * @tc.type: Fault injection
 * @tc.require: SR000CCPOI
 * @tc.author: luqianfu
 */
HWTEST_F(DistributeddbNbCrudTest, Pressure010, TestSize.Level2)
{
#ifdef RUNNING_ON_SIMULATED_ENV
    KvStoreNbDelegate *kvStoreNbDelegate = nullptr;
    KvStoreDelegateManager *manager = nullptr;
    kvStoreNbDelegate = DistributedDBNbTestTools::GetNbDelegateSuccess(manager, g_dbParameter2, g_option);
    ASSERT_TRUE(manager != nullptr && kvStoreNbDelegate != nullptr);
    ASSERT_EQ(manager->CloseKvStore(kvStoreNbDelegate), OK);
    /**
     * @tc.steps: step1. put (KEY_1, VALUE_1) to local db.
     * @tc.expected: step1. put failed and return ERROR.
     */
    Value valueResult;
    DBStatus status = kvStoreNbDelegate->PutLocal(KEY_1, VALUE_1);
    EXPECT_TRUE(status == DB_ERROR);
    /**
     * @tc.steps: step2. put (KEY_1, VALUE_1) to sync db.
     * @tc.expected: step2. put failed and return ERROR.
     */
    status = kvStoreNbDelegate->Put(KEY_2, VALUE_2);
    EXPECT_TRUE(status == DB_ERROR);
    /**
     * @tc.steps: step3. delete data from local db where key = KEY_1.
     * @tc.expected: step3. delete failed and return ERROR.
     */
    status = kvStoreNbDelegate->DeleteLocal(KEY_1);
    EXPECT_TRUE(status == DB_ERROR);
    /**
     * @tc.steps: step4. delete data from sync db where key = KEY_2.
     * @tc.expected: step4. delete failed and return ERROR.
     */
    status = kvStoreNbDelegate->Delete(KEY_2);
    EXPECT_TRUE(status == DB_ERROR);
    /**
     * @tc.steps: step5. update (KEY_1, VALUE_2) to local db.
     * @tc.expected: step5. update failed and return ERROR.
     */
    status = kvStoreNbDelegate->PutLocal(KEY_1, VALUE_2);
    EXPECT_TRUE(status == DB_ERROR);
    /**
     * @tc.steps: step6. update (KEY_2, VALUE_1) to sync db.
     * @tc.expected: step6. update failed and return ERROR.
     */
    status = kvStoreNbDelegate->Put(KEY_2, VALUE_1);
    EXPECT_TRUE(status == DB_ERROR);
    /**
     * @tc.steps: step7. get data from local db where key = KEY_1.
     * @tc.expected: step7. get failed and return ERROR.
     */
    status = kvStoreNbDelegate->GetLocal(KEY_1, valueResult);
    EXPECT_EQ(status, DB_ERROR);
    /**
     * @tc.steps: step8. get data from sync db where key = KEY_2.
     * @tc.expected: step8. get failed and return ERROR.
     */
    status = kvStoreNbDelegate->Get(KEY_2, valueResult);
    EXPECT_EQ(status, DB_ERROR);
    DistributedDB::Key key9;
    key9 = { 'a', 'b' };
    std::vector<Entry> entries;
    /**
     * @tc.steps: step9. get batch data from db with keyPrefix = 'ab'.
     * @tc.expected: step9. get failed and return ERROR.
     */
    status = kvStoreNbDelegate->GetEntries(key9, entries);
    EXPECT_EQ(status, DB_ERROR);

    kvStoreNbDelegate->DeleteLocal(KEY_1);
    kvStoreNbDelegate->Delete(KEY_2);
    if (!g_option.isMemoryDb) {
        EXPECT_EQ(manager->DeleteKvStore(STORE_ID_2), OK);
    }
    kvStoreNbDelegate = nullptr;
    delete manager;
    manager = nullptr;
#endif
}

/*
 * @tc.name: Pressure 011
 * @tc.desc: verify can't crud after db is deleted.
 * @tc.type: Fault injection
 * @tc.require: SR000CCPOI
 * @tc.author: luqianfu
 */
HWTEST_F(DistributeddbNbCrudTest, Pressure011, TestSize.Level3)
{
    std::this_thread::sleep_for(std::chrono::seconds(UNIQUE_SECOND * 10));
    /**
     * @tc.steps: step1. put (KEY_1, VALUE_1) to local db.
     * @tc.expected: step1. put failed and return ERROR.
     */
    Value valueResult;
    DBStatus status = g_kvStoreNbDelegate->PutLocal(KEY_1, VALUE_1);
    EXPECT_TRUE(status == OK);
    /**
     * @tc.steps: step2. put (KEY_1, VALUE_1) to sync db.
     * @tc.expected: step2. put failed and return ERROR.
     */
    status = g_kvStoreNbDelegate->Put(KEY_2, VALUE_2);
    EXPECT_TRUE(status == OK);
    /**
     * @tc.steps: step3. delete data from local db where key = KEY_1.
     * @tc.expected: step3. delete failed and return ERROR.
     */
    status = g_kvStoreNbDelegate->DeleteLocal(KEY_1);
    EXPECT_TRUE(status == OK);
    /**
     * @tc.steps: step4. delete data from sync db where key = KEY_2.
     * @tc.expected: step4. delete failed and return ERROR.
     */
    status = g_kvStoreNbDelegate->Delete(KEY_2);
    EXPECT_TRUE(status == OK);
    /**
     * @tc.steps: step5. update (KEY_1, VALUE_2) to local db.
     * @tc.expected: step5. update failed and return ERROR.
     */
    status = g_kvStoreNbDelegate->PutLocal(KEY_1, VALUE_2);
    EXPECT_TRUE(status == OK);
    /**
     * @tc.steps: step6. update (KEY_2, VALUE_1) to sync db.
     * @tc.expected: step6. update failed and return ERROR.
     */
    status = g_kvStoreNbDelegate->Put(KEY_2, VALUE_1);
    EXPECT_TRUE(status == OK);
    /**
     * @tc.steps: step7. get data from local db where key = KEY_1.
     * @tc.expected: step7. get failed and return ERROR.
     */
    status = g_kvStoreNbDelegate->GetLocal(KEY_1, valueResult);
    EXPECT_EQ(status, OK);
    /**
     * @tc.steps: step8. get data from sync db where key = KEY_2.
     * @tc.expected: step8. get failed and return ERROR.
     */
    status = g_kvStoreNbDelegate->Get(KEY_2, valueResult);
    EXPECT_EQ(status, OK);
    DistributedDB::Key key9 = { 'a', 'b' };
    /**
     * @tc.steps: step9. get batch data from db with keyPrefix = 'ab'.
     * @tc.expected: step9. get failed and return ERROR.
     */
    std::vector<Entry> entries;
    status = g_kvStoreNbDelegate->GetEntries(key9, entries);
    EXPECT_EQ(status, NOT_FOUND);
    status = g_kvStoreNbDelegate->DeleteLocal(KEY_1);
    EXPECT_TRUE(status == OK);
    status = g_kvStoreNbDelegate->Delete(KEY_2);
    EXPECT_TRUE(status == OK);
}

void StartThreadForLongReadRead()
{
    std::random_device randDevReadKeyNo;
    std::mt19937 genRandReadKeyNo(randDevReadKeyNo());
    std::uniform_int_distribution<unsigned long> disRandReadKeyNo(READ_RECORDS_NUM_START, READ_RECORDS_NUM_END);

    unsigned long randKeyNo;
    unsigned int thrCurId = 0;

    SysTime start;
    SysDurTime dur;
    double operInterval = 0.0;
    start = std::chrono::time_point_cast<std::chrono::microseconds>(std::chrono::steady_clock::now());

    /**
     * @tc.steps: step1. start 6 thread read the db randly at the same time for a long time.
     * @tc.expected: step1. can read and read rightly and has no mistakes.
     */
    while (operInterval < static_cast<double>(LONG_TIME_TEST_SECONDS)) {
        auto threadParam = new (std::nothrow) ConcurParam;
        ASSERT_NE(threadParam, nullptr);
        threadParam->entryPtr_ = new (std::nothrow) DistributedDB::Entry;
        ASSERT_NE(threadParam->entryPtr_, nullptr);
        threadParam->threadId_ = thrCurId++;
        threadParam->tag_ = READ;
        randKeyNo = disRandReadKeyNo(genRandReadKeyNo);
        Entry entryCurrent;
        GenerateRecord(randKeyNo, entryCurrent);
        operInterval = NbCalculateTime(threadParam, entryCurrent, start, dur);
    }
}

/*
 * @tc.name: Concurrency 002
 * @tc.desc: verify can read and read Concurrency for a long time and has no mistakes.
 * @tc.type: LongTime Test
 * @tc.require: SR000CCPOI
 * @tc.author: luqianfu
 */
HWTEST_F(DistributeddbNbCrudTest, Concurrency002, TestSize.Level3)
{
    KvStoreNbDelegate *kvStoreNbDelegate = nullptr;
    KvStoreDelegateManager *manager = nullptr;
    kvStoreNbDelegate = DistributedDBNbTestTools::GetNbDelegateSuccess(manager, g_dbParameter2, g_option);
    ASSERT_TRUE(manager != nullptr && kvStoreNbDelegate != nullptr);

    vector<Entry> entriesBatch;
    vector<Key> allKeys;
    GenerateRecords(RECORDS_NUM_THOUSAND, DEFAULT_START, allKeys, entriesBatch);

    DBStatus status;
    for (unsigned int index = 0; index < RECORDS_NUM_THOUSAND; index++)
    {
        status = kvStoreNbDelegate->Put(entriesBatch[index].key, entriesBatch[index].value);
        EXPECT_EQ(status, OK);
    }

    vector<Entry> valueResult;
    kvStoreNbDelegate->GetEntries(KEY_EMPTY, valueResult);
    EXPECT_EQ(valueResult.size(), RECORDS_NUM_THOUSAND);

    StartThreadForLongReadRead();

    while (entriesBatch.size() > 0) {
        status = DistributedDBNbTestTools::Delete(*kvStoreNbDelegate, entriesBatch[0].key);
        EXPECT_EQ(status, OK);
        entriesBatch.erase(entriesBatch.begin());
    }
    MST_LOG("entriesBatch.size() = %zu", entriesBatch.size());
    EXPECT_TRUE(EndCaseDeleteDB(manager, kvStoreNbDelegate, STORE_ID_2, g_option.isMemoryDb));
}

void StartThreadForLongReadWrite(vector<Entry> &entriesBatch)
{
    std::vector<std::thread> threads;
    std::random_device randDevReadKeyNo, randDevWriteKeyNo, randDevTag;
    std::mt19937 genRandReadKeyNo(randDevReadKeyNo()), genRandWriteKeyNo(randDevWriteKeyNo()),
        genRandTag(randDevTag());
    std::uniform_int_distribution<unsigned long> disRandReadKeyNo(READ_RECORDS_NUM_START, READ_RECORDS_NUM_END);
    std::uniform_int_distribution<unsigned long> disRandWriteKeyNo(WRITE_RECORDS_NUM_START, WRITE_RECORDS_NUM_END);
    std::uniform_int_distribution<unsigned long> disRandTag(READ, WRITE);
    unsigned long randKeyNo;
    unsigned int threadCurId = 0;
    int randTag;
    SysTime start = std::chrono::time_point_cast<std::chrono::microseconds>(std::chrono::steady_clock::now());
    /**
     * @tc.steps: step1. start 6 thread read and write the db randly at the same time for a long time.
     * @tc.expected: step1. can read and write rightly and has no mistakes.
     */
    SysDurTime dur;
    double operInterval = 0.0;
    while (operInterval < static_cast<double>(LONG_TIME_TEST_SECONDS)) {
        auto threadParam = new (std::nothrow) ConcurParam;
        ASSERT_NE(threadParam, nullptr);
        threadParam->entryPtr_ = new (std::nothrow) DistributedDB::Entry;
        ASSERT_NE(threadParam->entryPtr_, nullptr);
        threadParam->threadId_ = threadCurId++;
        randTag = disRandTag(genRandTag);
        threadParam->tag_ = static_cast<ReadOrWriteTag>(randTag);
        if (randTag == READ) {
            randKeyNo = disRandReadKeyNo(genRandReadKeyNo);
        } else {
            randKeyNo = disRandWriteKeyNo(genRandWriteKeyNo);
        }
        Entry entryCurrent;
        GenerateRecord(randKeyNo, entryCurrent);
        if ((randTag == WRITE) && (randKeyNo > READ_RECORDS_NUM_END)) {
            entriesBatch.push_back(entryCurrent);
        }
        operInterval = NbCalculateTime(threadParam, entryCurrent, start, dur);
    }
}

/*
 * @tc.name: Concurrency 004
 * @tc.desc: verify can read and write Concurrency for a long time.
 * @tc.type: LONGTIME TEST
 * @tc.require: SR000CCPOI
 * @tc.author: luqianfu
 */
HWTEST_F(DistributeddbNbCrudTest, Concurrency004, TestSize.Level3)
{
    KvStoreNbDelegate *kvStoreNbDelegate = nullptr;
    KvStoreDelegateManager *manager = nullptr;
    kvStoreNbDelegate = DistributedDBNbTestTools::GetNbDelegateSuccess(manager, g_dbParameter2, g_option);
    ASSERT_TRUE(manager != nullptr && kvStoreNbDelegate != nullptr);

    vector<Entry> entriesBatch;
    vector<Key> allKeys;
    GenerateRecords(RECORDS_NUM_THOUSAND, DEFAULT_START, allKeys, entriesBatch);

    DBStatus status = DistributedDBNbTestTools::PutBatch(*kvStoreNbDelegate, entriesBatch);
    EXPECT_EQ(status, DBStatus::OK);
    vector<Entry> valueResult;
    status = DistributedDBNbTestTools::GetEntries(*kvStoreNbDelegate, KEY_SEARCH_4, valueResult);
    EXPECT_EQ(status, DBStatus::OK);
    MST_LOG("value size %zu", valueResult.size());

    StartThreadForLongReadWrite(entriesBatch);

    for (unsigned int index = 0; index < RECORDS_NUM_THOUSAND; index++) {
        status = DistributedDBNbTestTools::Delete(*kvStoreNbDelegate, entriesBatch[index].key);
        EXPECT_EQ(status, OK);
    }
    MST_LOG("entriesBatch.size() = %zu", entriesBatch.size());
    EXPECT_TRUE(EndCaseDeleteDB(manager, kvStoreNbDelegate, STORE_ID_2, g_option.isMemoryDb));
}

void JudgeInLongWriteWrite(vector<Entry> &entriesBatch, ConcurParam *threadParam, Entry &entryCurrent)
{
    bool isExist = false;
    if (threadParam->tag_ == WRITE) {
        for (auto &entry : entriesBatch) {
            if (CompareVector(entry.key, entryCurrent.key)) {
                isExist = true;
            }
        }
        if (!isExist) {
            entriesBatch.push_back(entryCurrent);
        }
    } else {
        for (unsigned long index = 0; index < entriesBatch.size(); ++index) {
            if (CompareVector(entriesBatch[index].key, entryCurrent.key)) {
                entriesBatch.erase(entriesBatch.begin() + index);
            }
        }
    }
}

void StartThreadForLongWriteWrite(vector<Entry> &entriesBatch)
{
    std::vector<std::thread> threads;
    std::random_device randDevWriteKeyNo, randDevDeleteKeyNo, randDevTag;
    std::mt19937 genRandWriteKeyNo(randDevWriteKeyNo()), genRandDeleteKeyNo(randDevDeleteKeyNo()),
        genRandTag(randDevTag());
    std::uniform_int_distribution<unsigned long> disRandWriteKeyNo(WRITE_RECORDS_NUM_START, WRITE_RECORDS_NUM_END);
    std::uniform_int_distribution<unsigned long> disRandDeleteKeyNo(DELETE_RECORDS_NUM_START, DELETE_RECORDS_NUM_END);
    std::uniform_int_distribution<unsigned long> disRandTag(WRITE, DELETE);
    unsigned long randKeyNo, randRandTag;
    unsigned int threadCurId = 0;

    SysTime start;
    SysDurTime dur;
    double operInterval = 0.0;

    /**
     * @tc.steps: step1. start 6 thread write the db randly at the same time for a long time.
     * @tc.expected: step1. can write rightly and has no mistake.
     */
    start = std::chrono::time_point_cast<std::chrono::microseconds>(std::chrono::steady_clock::now());
    while (operInterval < static_cast<double>(LONG_TIME_TEST_SECONDS)) {
        auto thParam = new (std::nothrow) ConcurParam;
        ASSERT_NE(thParam, nullptr);
        thParam->entryPtr_ = new (std::nothrow) DistributedDB::Entry;
        ASSERT_NE(thParam->entryPtr_, nullptr);
        thParam->threadId_ = threadCurId++;
        randRandTag = disRandTag(genRandTag);
        thParam->tag_ = static_cast<ReadOrWriteTag>(randRandTag);
        if (randRandTag == WRITE) {
            randKeyNo = disRandWriteKeyNo(genRandWriteKeyNo);
        } else {
            randKeyNo = disRandDeleteKeyNo(genRandDeleteKeyNo);
        }
        Entry entryCurrent;
        GenerateRecord(randKeyNo, entryCurrent);
        JudgeInLongWriteWrite(entriesBatch, thParam, entryCurrent);
        operInterval = NbCalculateTime(thParam, entryCurrent, start, dur);
    }
}

/*
 * @tc.name: Concurrency 006
 * @tc.desc: verify can write and write Concurrency for a long time.
 * @tc.type: LONGTIME TEST
 * @tc.require: SR000CCPOI
 * @tc.author: luqianfu
 */
HWTEST_F(DistributeddbNbCrudTest, Concurrency006, TestSize.Level3)
{
    KvStoreNbDelegate *delegate = nullptr;
    KvStoreDelegateManager *manager = nullptr;
    delegate = DistributedDBNbTestTools::GetNbDelegateSuccess(manager, g_dbParameter2, g_option);
    ASSERT_TRUE(manager != nullptr && delegate != nullptr);

    vector<Entry> entryBatches;
    vector<Key> allKeys;
    GenerateRecords(RECORDS_NUM_THOUSAND, DEFAULT_START, allKeys, entryBatches);

    DBStatus status = DistributedDBNbTestTools::PutBatch(*delegate, entryBatches);
    EXPECT_EQ(status, DBStatus::OK);
    vector<Entry> valueResult;
    status = DistributedDBNbTestTools::GetEntries(*delegate, KEY_SEARCH_4, valueResult);
    EXPECT_EQ(status, DBStatus::OK);
    MST_LOG("value size %zu", valueResult.size());

    StartThreadForLongWriteWrite(entryBatches);

    for (unsigned int index = 0; index < entryBatches.size(); index++) {
        status = DistributedDBNbTestTools::Delete(*delegate, entryBatches[index].key);
        EXPECT_TRUE(status == OK || status == NOT_FOUND);
    }
    MST_LOG("entryBatches.size() = %zu", entryBatches.size());
    EXPECT_TRUE(EndCaseDeleteDB(manager, delegate, STORE_ID_2, g_option.isMemoryDb));
}
}
