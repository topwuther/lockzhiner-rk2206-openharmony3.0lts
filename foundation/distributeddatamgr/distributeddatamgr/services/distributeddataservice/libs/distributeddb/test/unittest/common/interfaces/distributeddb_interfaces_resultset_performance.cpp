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
#include <thread>
#include "types.h"
#include "kv_store_delegate_manager.h"
#include "kv_store_nb_delegate.h"
#include "distributeddb_tools_unit_test.h"
#include "log_print.h"

using namespace testing::ext;
using namespace DistributedDB;
using namespace DistributedDBUnitTest;
using namespace std;

namespace {
    // define some variables to init a KvStoreDelegateManager object.
    KvStoreDelegateManager g_mgr("app0", "user0");
    string g_testDir;
    KvStoreConfig g_config;
    Key g_keyPrefix = {'A', 'B', 'C'};

    const int BASE_NUMBER = 100000;
    const int INSERT_NUMBER = 10000;
    const int ENTRY_VALUE_SIZE = 3000;
    const int BATCH_ENTRY_NUMBER = 100;

    DBStatus g_kvDelegateStatus = INVALID_ARGS;
    KvStoreNbDelegate *g_kvNbDelegatePtr = nullptr;
    KvStoreDelegate *g_kvDelegatePtr = nullptr;

    void KvStoreNbDelegateCallback(DBStatus statusSrc, KvStoreNbDelegate* kvStoreSrc,
        DBStatus* statusDst, KvStoreNbDelegate** kvStoreDst)
    {
        *statusDst = statusSrc;
        *kvStoreDst = kvStoreSrc;
    }

    // the type of g_kvNbDelegateCallback is function<void(DBStatus, KvStoreDelegate*)>
    auto g_kvNbDelegateCallback = bind(&KvStoreNbDelegateCallback, placeholders::_1,
        placeholders::_2, &g_kvDelegateStatus, &g_kvNbDelegatePtr);

    void InitResultSet()
    {
        Key testKey;
        Value testValue;
        for (int i = BASE_NUMBER; i < BASE_NUMBER + INSERT_NUMBER; i++) {
            testKey.clear();
            testValue.clear();
            testKey = g_keyPrefix;
            std::string strIndex = std::to_string(i);
            testKey.insert(testKey.end(), strIndex.begin(), strIndex.end());

            DistributedDBToolsUnitTest::GetRandomKeyValue(testValue, ENTRY_VALUE_SIZE);
            if ((i % BATCH_ENTRY_NUMBER) == 0) {
                g_kvNbDelegatePtr->StartTransaction();
            }
            EXPECT_EQ(g_kvNbDelegatePtr->Put(testKey, testValue), OK);
            if (((i + 1) % BATCH_ENTRY_NUMBER) == 0) {
                g_kvNbDelegatePtr->Commit();
            }
        }

        std::this_thread::sleep_for(std::chrono::seconds(2)); // sleep 2 s for the cache.
    }
}
class DistributedDBInterfacesNBResultsetPerfTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
};

void DistributedDBInterfacesNBResultsetPerfTest::SetUpTestCase(void)
{
    DistributedDBToolsUnitTest::TestDirInit(g_testDir);
    g_config.dataDir = g_testDir;
    g_mgr.SetKvStoreConfig(g_config);
}

void DistributedDBInterfacesNBResultsetPerfTest::TearDownTestCase(void)
{
}

void DistributedDBInterfacesNBResultsetPerfTest::SetUp(void)
{
    g_kvDelegateStatus = INVALID_ARGS;
    g_kvNbDelegatePtr = nullptr;
    g_kvDelegatePtr = nullptr;
}

void DistributedDBInterfacesNBResultsetPerfTest::TearDown(void)
{
    if (g_kvDelegatePtr != nullptr) {
        g_mgr.CloseKvStore(g_kvNbDelegatePtr);
        g_kvNbDelegatePtr = nullptr;
    }
}

/**
  * @tc.name: ResultSetPerfTest001
  * @tc.desc: Test the NbDelegate for result set function.
  * @tc.type: FUNC
  * @tc.require: AR000D08KT
  * @tc.author: wangbingquan
  */
HWTEST_F(DistributedDBInterfacesNBResultsetPerfTest, ResultSetPerfTest001, TestSize.Level4)
{
    /**
     * @tc.steps: step1. initialize result set.
     * @tc.expected: step1. Success.
     */
    KvStoreNbDelegate::Option option = {true, false, false};
    g_mgr.GetKvStore("resultset_perf_test", option, g_kvNbDelegateCallback);
    ASSERT_TRUE(g_kvNbDelegatePtr != nullptr);
    EXPECT_TRUE(g_kvDelegateStatus == OK);
    InitResultSet();

    /**
     * @tc.steps: step2. get entries using result set.
     * @tc.expected: step2. Success.
     */
    LOGI("######## Before get resultset");
    KvStoreResultSet *readResultSet = nullptr;
    Key keyGet = g_keyPrefix;
    keyGet.push_back('1');

    int offset = 5000; // offset 5000
    LOGI("######## Query resultSet");
    Query query = Query::Select().PrefixKey(keyGet).Limit(128, offset); // limit 128
    EXPECT_EQ(g_kvNbDelegatePtr->GetEntries(query, readResultSet), OK);
    ASSERT_TRUE(readResultSet != nullptr);
    LOGI("######## After get resultset");
    int totalCount = readResultSet->GetCount();
    EXPECT_EQ(totalCount, 128); // limit 128
    LOGI("######## After get count:%d", totalCount);

    readResultSet->MoveToPosition(0);
    LOGI("######## After move to next");
    EXPECT_EQ(g_kvNbDelegatePtr->CloseResultSet(readResultSet), OK);
    EXPECT_TRUE(readResultSet == nullptr);

    std::this_thread::sleep_for(std::chrono::seconds(5)); // sleep 5 s
    LOGI("######## Plain resultSet");
    EXPECT_EQ(g_kvNbDelegatePtr->GetEntries(keyGet, readResultSet), OK);
    ASSERT_TRUE(readResultSet != nullptr);
    LOGI("######## After get resultset");
    totalCount = readResultSet->GetCount();
    EXPECT_EQ(totalCount, INSERT_NUMBER);
    LOGI("######## After get count:%d", totalCount);

    readResultSet->MoveToPosition(offset);
    LOGI("######## After move to next");
    EXPECT_EQ(g_kvNbDelegatePtr->CloseResultSet(readResultSet), OK);
    EXPECT_TRUE(readResultSet == nullptr);

    EXPECT_EQ(g_mgr.CloseKvStore(g_kvNbDelegatePtr), OK);
    EXPECT_EQ(g_mgr.DeleteKvStore("resultset_perf_test"), OK);
    g_kvNbDelegatePtr = nullptr;
}