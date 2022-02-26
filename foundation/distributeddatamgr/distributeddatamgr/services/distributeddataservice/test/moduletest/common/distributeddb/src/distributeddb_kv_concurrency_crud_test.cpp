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
#include <cstdio>
#include <string>

#include "distributeddb_data_generator.h"
#include "distributed_test_tools.h"
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

namespace DistributeddbKvConcurrencyCrud {
const bool IS_LOCAL = false;
const unsigned int READ_RECORDS_NUM_START = 1;
const unsigned int READ_RECORDS_NUM_END = 128;
const unsigned int WRITE_RECORDS_NUM_START = 1;
const unsigned int WRITE_RECORDS_NUM_END = 9999;
const unsigned long LONG_TIME_TEST_SECONDS = 10;
const unsigned long LONG_TIME_INTERVAL_MILLSECONDS = 5;

KvStoreDelegate *g_kvStoreConcurDelegate = nullptr; // the delegate used in this suit.
KvStoreDelegateManager *g_concurManager = nullptr;

enum ReadOrWriteTag {
    READ = 0,
    WRITE = 1,
    DELETE = 2
};

struct ConcurParam {
    unsigned int threadId_;
    ReadOrWriteTag tag_;
    DistributedDB::Entry* entryPtr_;
};

// the thread runnnig methods
void ConcurOperThread(ConcurParam* args)
{
    auto paramsPtr = static_cast<ConcurParam *>(args);
    DBStatus status;
    Value valueResult;
    string strKey, strValue;

    if (paramsPtr->tag_ == READ) {
        valueResult = DistributedTestTools::Get(*g_kvStoreConcurDelegate, paramsPtr->entryPtr_->key);
        Uint8VecToString(paramsPtr->entryPtr_->value, strValue);
        if (valueResult.size() != 0) {
            EXPECT_TRUE(DistributedTestTools::IsValueEquals(valueResult, paramsPtr->entryPtr_->value));
        }
    } else if (paramsPtr->tag_ == WRITE) {
        status = DistributedTestTools::Put(*g_kvStoreConcurDelegate,
            paramsPtr->entryPtr_->key, paramsPtr->entryPtr_->value);
        ASSERT_EQ(status, DBStatus::OK);
        Uint8VecToString(paramsPtr->entryPtr_->key, strKey);
        Uint8VecToString(paramsPtr->entryPtr_->value, strValue);

        valueResult = DistributedTestTools::Get(*g_kvStoreConcurDelegate, paramsPtr->entryPtr_->key);
        if (valueResult.size() != 0) {
            EXPECT_TRUE(DistributedTestTools::IsValueEquals(valueResult, paramsPtr->entryPtr_->value));
        }
    } else {
        valueResult = DistributedTestTools::Get(*g_kvStoreConcurDelegate, paramsPtr->entryPtr_->key);
        if (valueResult.size() != 0) {
            status = DistributedTestTools::Delete(*g_kvStoreConcurDelegate, paramsPtr->entryPtr_->key);
            ASSERT_TRUE(status == DBStatus::NOT_FOUND || status == OK);

            Uint8VecToString(paramsPtr->entryPtr_->key, strKey);
            Uint8VecToString(paramsPtr->entryPtr_->value, strValue);
        } else {
            status = DistributedTestTools::Delete(*g_kvStoreConcurDelegate, paramsPtr->entryPtr_->key);
            ASSERT_TRUE(status == DBStatus::NOT_FOUND || status == OK);
        }
    }
}

double KvCalculateTime(ConcurParam *&kvThreadParam, const Entry entryCurrent, const SysTime &start, SysDurTime dur)
{
    SysTime end;
    kvThreadParam->entryPtr_->key = entryCurrent.key;
    kvThreadParam->entryPtr_->value = entryCurrent.value;
    std::thread thread = std::thread(ConcurOperThread, reinterpret_cast<ConcurParam *>(kvThreadParam));
    thread.join();

    std::this_thread::sleep_for(std::chrono::duration<float, std::milli>(LONG_TIME_INTERVAL_MILLSECONDS));
    end = std::chrono::time_point_cast<std::chrono::microseconds>(std::chrono::steady_clock::now());
    dur = std::chrono::duration_cast<chrono::microseconds>(end - start);
    double operInterval = static_cast<double>(dur.count()) * chrono::microseconds::period::num
        / chrono::microseconds::period::den;
    delete kvThreadParam->entryPtr_;
    kvThreadParam->entryPtr_ = nullptr;
    delete kvThreadParam;
    kvThreadParam = nullptr;
    return operInterval;
}

class DistributeddbKvConcurrencyCrudTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
private:
};

void DistributeddbKvConcurrencyCrudTest::SetUpTestCase(void)
{
}

void DistributeddbKvConcurrencyCrudTest::TearDownTestCase(void)
{
}

void DistributeddbKvConcurrencyCrudTest::SetUp(void)
{
    MST_LOG("SetUpTest before all cases local[%d].", IS_LOCAL);
    RemoveDir(DIRECTOR);

    UnitTest *test = UnitTest::GetInstance();
    ASSERT_NE(test, nullptr);
    const TestInfo *testinfo = test->current_test_info();
    ASSERT_NE(testinfo, nullptr);
    string testCaseName = string(testinfo->name());
    MST_LOG("[SetUp] test case %s is start to run", testCaseName.c_str());

    g_kvStoreConcurDelegate = DistributedTestTools::GetDelegateSuccess(g_concurManager,
        g_kvdbParameter1, g_kvOption);
    ASSERT_TRUE(g_concurManager != nullptr && g_kvStoreConcurDelegate != nullptr);
}

void DistributeddbKvConcurrencyCrudTest::TearDown(void)
{
    MST_LOG("TearDownTest after all cases.");
    EXPECT_TRUE(g_concurManager->CloseKvStore(g_kvStoreConcurDelegate) == OK);
    g_kvStoreConcurDelegate = nullptr;
    DBStatus status = g_concurManager->DeleteKvStore(STORE_ID_1);
    EXPECT_TRUE(status == DistributedDB::DBStatus::OK) << "fail to delete exist kvdb";
    delete g_concurManager;
    g_concurManager = nullptr;
    RemoveDir(DIRECTOR);
}

/*
 * @tc.name: Read 002
 * @tc.desc: Verify that support long-term multi-threaded concurrent reading.
 * @tc.type: Long time
 * @tc.require: SR000BUH3J
 * @tc.author: luqianfu
 */
HWTEST_F(DistributeddbKvConcurrencyCrudTest, Read002, TestSize.Level3)
{
    DistributedTestTools::Clear(*g_kvStoreConcurDelegate);

    vector<Entry> entriesBatch;
    vector<Key> allKeys;
    GenerateRecords(BATCH_RECORDS, DEFAULT_START, allKeys, entriesBatch);

    /**
     * @tc.steps: step1. putBatch 128 items of (keys,values) then getEntries with keyprefix='k'.
     * @tc.expected: step1. putBatch successfully and the size of GetEntries is 128.
     */
    DBStatus status = DistributedTestTools::PutBatch(*g_kvStoreConcurDelegate, entriesBatch);
    EXPECT_TRUE(status == DBStatus::OK);
    vector<Entry> valueResult = DistributedTestTools::GetEntries(*g_kvStoreConcurDelegate, KEY_SEARCH_4);

    /**
     * @tc.steps: step2. create 6 threads and read data from db concurrently for 10s.
     * @tc.expected: step2. read successfully and has no exception.
     */
    std::random_device randDevReadKeyNo;
    std::mt19937 genRandReadKeyNo(randDevReadKeyNo());
    std::uniform_int_distribution<unsigned long> disRandReadKeyNo(READ_RECORDS_NUM_START, READ_RECORDS_NUM_END);
    unsigned long randKeyNo;
    unsigned int threadCurId = 0;

    SysTime start;
    SysDurTime dur;
    double operInterval = 0.0;

    start = std::chrono::time_point_cast<std::chrono::microseconds>(std::chrono::steady_clock::now());
    Entry entryCurrent;
    while (operInterval < static_cast<double>(LONG_TIME_TEST_SECONDS)) {
        auto threadParam = new (std::nothrow) ConcurParam;
        ASSERT_NE(threadParam, nullptr);
        threadParam->entryPtr_ = new (std::nothrow) DistributedDB::Entry;
        ASSERT_NE(threadParam->entryPtr_, nullptr);
        threadParam->threadId_ = threadCurId++;
        threadParam->tag_ = READ;
        randKeyNo = disRandReadKeyNo(genRandReadKeyNo);

        GenerateRecord(randKeyNo, entryCurrent);
        operInterval = KvCalculateTime(threadParam, entryCurrent, start, dur);
    }

    DistributedTestTools::Clear(*g_kvStoreConcurDelegate);
}

void StartRandThread()
{
    std::vector<std::thread> threads;
    std::random_device randDevReadKeyNo, randDevWriteKeyNo, randDevTag;
    std::mt19937 genRandReadKeyNo(randDevReadKeyNo()), genRandWriteKeyNo(randDevWriteKeyNo()), genRandTag(randDevTag());
    std::uniform_int_distribution<unsigned long> disRandReadKeyNo(READ_RECORDS_NUM_START, READ_RECORDS_NUM_END);
    std::uniform_int_distribution<unsigned long> disRandWriteKeyNo(WRITE_RECORDS_NUM_START, WRITE_RECORDS_NUM_END);
    std::uniform_int_distribution<int> disRandTag(READ, WRITE);
    unsigned long randKeyNo;
    unsigned int threadCurId = 0;
    int randTag;
    SysTime start;
    SysDurTime dur;
    double operInterval = 0.0;
    start = std::chrono::time_point_cast<std::chrono::microseconds>(std::chrono::steady_clock::now());
    while (operInterval < static_cast<double>(LONG_TIME_TEST_SECONDS)) {
        ConcurParam *threadParam = new (std::nothrow) ConcurParam;
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
        operInterval = KvCalculateTime(threadParam, entryCurrent, start, dur);
    }
}
/*
 * @tc.name: ReadWrite 002
 * @tc.desc: Verify that support long-term multi-threaded concurrent reading and writing.
 * @tc.type: Long time
 * @tc.require: SR000BUH3J
 * @tc.author: luqianfu
 */
HWTEST_F(DistributeddbKvConcurrencyCrudTest, ReadWrite002, TestSize.Level3)
{
    DistributedTestTools::Clear(*g_kvStoreConcurDelegate);

    vector<Entry> entriesBatch;
    vector<Key> allKeys;
    GenerateRecords(BATCH_RECORDS, DEFAULT_START, allKeys, entriesBatch);

    /**
     * @tc.steps: step1. putBatch 128 items of (keys,values) then getEntries with keyprefix='k'.
     * @tc.expected: step1. putBatch successfully and the size of GetEntries is 128.
     */
    DBStatus status = DistributedTestTools::PutBatch(*g_kvStoreConcurDelegate, entriesBatch);
    EXPECT_TRUE(status == DBStatus::OK);
    vector<Entry> valueResult = DistributedTestTools::GetEntries(*g_kvStoreConcurDelegate, KEY_SEARCH_4);
    MST_LOG("value size %zu", valueResult.size());

    /**
     * @tc.steps: step2. create 6 threads, read and write data concurrently for 10s.
     * @tc.expected: step2. read and write successfully and has no exception.
     */
    StartRandThread();

    DistributedTestTools::Clear(*g_kvStoreConcurDelegate);
}

/*
 * @tc.name: ReadWrite 004
 * @tc.desc: Verify that support multi-threaded concurrent writing.
 * @tc.type: Long time
 * @tc.require: SR000BUH3J
 * @tc.author: luqianfu
 */
HWTEST_F(DistributeddbKvConcurrencyCrudTest, ReadWrite004, TestSize.Level3)
{
    DistributedTestTools::Clear(*g_kvStoreConcurDelegate);

    vector<Entry> entriesBatch;
    vector<Key> allKeys;
    GenerateRecords(BATCH_RECORDS, DEFAULT_START, allKeys, entriesBatch);

    /**
     * @tc.steps: step1. putBatch 128 items of (keys,values) then getEntries with keyprefix='k'.
     * @tc.expected: step1. putBatch successfully and the size of GetEntries is 128.
     */
    DBStatus status = DistributedTestTools::PutBatch(*g_kvStoreConcurDelegate, entriesBatch);
    EXPECT_TRUE(status == DBStatus::OK);
    vector<Entry> valueResult = DistributedTestTools::GetEntries(*g_kvStoreConcurDelegate, KEY_SEARCH_4);
    MST_LOG("value size %zu", valueResult.size());
    /**
     * @tc.steps: step2. create 6 threads to write and create 4 threads to delete data concurrently for 10s.
     * @tc.expected: step2. write and delete successfully and has no exception.
     */
    StartRandThread();

    DistributedTestTools::Clear(*g_kvStoreConcurDelegate);
}
}
