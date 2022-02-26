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
#include "db_errno.h"
#include "log_print.h"

using namespace testing::ext;
using namespace DistributedDB;
using namespace DistributedDBUnitTest;
using namespace std;

namespace {
    const Key NULL_KEY;
    const int OBSERVER_SLEEP_TIME = 100;

    // define some variables to init a KvStoreDelegateManager object.
    KvStoreDelegateManager g_mgr(APP_ID, USER_ID);
    string g_testDir;
    KvStoreConfig g_config;

    // define the g_kvNbDelegateCallback, used to get some information when open a kv store.
    DBStatus g_kvDelegateStatus = INVALID_ARGS;
    KvStoreNbDelegate *g_kvNbDelegatePtr = nullptr;

    void KvStoreNbDelegateCallback(DBStatus statusSrc, KvStoreNbDelegate *kvStoreSrc,
        DBStatus *statusDst, KvStoreNbDelegate **kvStoreDst)
    {
        *statusDst = statusSrc;
        *kvStoreDst = kvStoreSrc;
    }

    // the type of g_kvNbDelegateCallback is function<void(DBStatus, KvStoreDelegate*)>
    auto g_kvNbDelegateCallback = bind(&KvStoreNbDelegateCallback, placeholders::_1,
        placeholders::_2, &g_kvDelegateStatus, &g_kvNbDelegatePtr);

    // define parameters for conflict callback
    bool g_isNeedInsertEntryInCallback = false;
    int g_conflictCallbackTimes = 0;
    Entry g_localEntry;
    Entry g_syncEntry;
    bool g_isLocalLastest = false;
    bool g_isSyncNull = false;
    void ResetCallbackArg(bool isLocalLastest = false)
    {
        g_conflictCallbackTimes = 0;
        g_localEntry.key.clear();
        g_localEntry.value.clear();
        g_syncEntry.key.clear();
        g_syncEntry.value.clear();
        g_isLocalLastest = isLocalLastest;
        g_isSyncNull = true;
        g_isNeedInsertEntryInCallback = false;
    }

    void ConflictCallback(const Entry &local, const Entry *sync, bool isLocalLastest)
    {
        g_conflictCallbackTimes++;
        g_localEntry = local;
        if (sync != nullptr) {
            g_syncEntry = *sync;
            g_isSyncNull = false;
        } else {
            g_isSyncNull = true;
        }
        g_isLocalLastest = isLocalLastest;
        if (g_isNeedInsertEntryInCallback) {
            ASSERT_TRUE(g_kvNbDelegatePtr != nullptr);
            EXPECT_EQ(g_kvNbDelegatePtr->Put(KEY_1, VALUE_1), OK);
        }
    }
}

class DistributedDBInterfacesNBPublishTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
};

void DistributedDBInterfacesNBPublishTest::SetUpTestCase(void)
{
    DistributedDBToolsUnitTest::TestDirInit(g_testDir);
    g_config.dataDir = g_testDir;
    g_mgr.SetKvStoreConfig(g_config);
}

void DistributedDBInterfacesNBPublishTest::TearDownTestCase(void)
{
    if (DistributedDBToolsUnitTest::RemoveTestDbFiles(g_testDir) != 0) {
        LOGE("rm test db files error!");
    }
}

void DistributedDBInterfacesNBPublishTest::SetUp(void)
{
    g_kvDelegateStatus = INVALID_ARGS;
    g_kvNbDelegatePtr = nullptr;
}

void DistributedDBInterfacesNBPublishTest::TearDown(void) {}

/**
  * @tc.name: SingleVerPublishKey001
  * @tc.desc: Publish nonexistent key
  * @tc.type: FUNC
  * @tc.require: AR000DPTQ5
  * @tc.author: wumin
  */
HWTEST_F(DistributedDBInterfacesNBPublishTest, SingleVerPublishKey001, TestSize.Level1)
{
    const KvStoreNbDelegate::Option option = {true, false};
    g_mgr.GetKvStore("distributed_nb_publish_SingleVerPublishKey001", option, g_kvNbDelegateCallback);
    ASSERT_TRUE(g_kvNbDelegatePtr != nullptr);
    EXPECT_TRUE(g_kvDelegateStatus == OK);
    EXPECT_EQ(g_kvNbDelegatePtr->PutLocal(KEY_1, VALUE_1), OK);
    EXPECT_EQ(g_kvNbDelegatePtr->Put(KEY_2, VALUE_2), OK);
    /**
     * @tc.steps:step1. PublishLocal key2.
     * @tc.expected: step1. return NOT_FOUND.
     */
    EXPECT_EQ(g_kvNbDelegatePtr->PublishLocal(KEY_2, true, true, nullptr), NOT_FOUND);
    /**
     * @tc.steps:step2. Get value of key1 and key2 both from local and sync table
     * @tc.expected: step2. value of key1 and key2 are correct
     */
    Value readValue;
    EXPECT_EQ(g_kvNbDelegatePtr->Get(KEY_1, readValue), NOT_FOUND);
    EXPECT_EQ(g_kvNbDelegatePtr->GetLocal(KEY_1, readValue), OK);
    EXPECT_EQ(readValue, VALUE_1);

    EXPECT_EQ(g_kvNbDelegatePtr->Get(KEY_2, readValue), OK);
    EXPECT_EQ(readValue, VALUE_2);
    EXPECT_EQ(g_kvNbDelegatePtr->GetLocal(KEY_2, readValue), NOT_FOUND);
    /**
     * @tc.steps:step3. PublishLocal key2.
     * @tc.expected: step3. return OK.
     */
    EXPECT_EQ(g_kvNbDelegatePtr->PublishLocal(KEY_2, true, false, nullptr), NOT_FOUND);
    /**
     * @tc.steps:step4. Get value of key1 and key2 both from local and sync table
     * @tc.expected: step4. value of key1 and key2 are correct
     */
    EXPECT_EQ(g_kvNbDelegatePtr->Get(KEY_1, readValue), NOT_FOUND);
    EXPECT_EQ(g_kvNbDelegatePtr->GetLocal(KEY_1, readValue), OK);
    EXPECT_EQ(readValue, VALUE_1);

    EXPECT_EQ(g_kvNbDelegatePtr->Get(KEY_2, readValue), OK);
    EXPECT_EQ(readValue, VALUE_2);
    EXPECT_EQ(g_kvNbDelegatePtr->GetLocal(KEY_2, readValue), NOT_FOUND);
    /**
     * @tc.steps:step5. PublishLocal key2.
     * @tc.expected: step5. return OK.
     */
    EXPECT_EQ(g_kvNbDelegatePtr->PublishLocal(KEY_2, false, true, nullptr), NOT_FOUND);
    /**
     * @tc.steps:step6. Get value of key1 and key2 both from local and sync table
     * @tc.expected: step6. value of key1 and key2 are correct
     */
    EXPECT_EQ(g_kvNbDelegatePtr->Get(KEY_1, readValue), NOT_FOUND);
    EXPECT_EQ(g_kvNbDelegatePtr->GetLocal(KEY_1, readValue), OK);
    EXPECT_EQ(readValue, VALUE_1);

    EXPECT_EQ(g_kvNbDelegatePtr->Get(KEY_2, readValue), OK);
    EXPECT_EQ(readValue, VALUE_2);
    EXPECT_EQ(g_kvNbDelegatePtr->GetLocal(KEY_2, readValue), NOT_FOUND);
    /**
     * @tc.steps:step7. PublishLocal key2.
     * @tc.expected: step7. return OK.
     */
    EXPECT_EQ(g_kvNbDelegatePtr->PublishLocal(KEY_2, false, false, nullptr), NOT_FOUND);
    /**
     * @tc.steps:step8. Get value of key1 and key2 both from local and sync table
     * @tc.expected: step8. value of key1 and key2 are correct
     */
    EXPECT_EQ(g_kvNbDelegatePtr->Get(KEY_1, readValue), NOT_FOUND);
    EXPECT_EQ(g_kvNbDelegatePtr->GetLocal(KEY_1, readValue), OK);
    EXPECT_EQ(readValue, VALUE_1);

    EXPECT_EQ(g_kvNbDelegatePtr->Get(KEY_2, readValue), OK);
    EXPECT_EQ(readValue, VALUE_2);
    EXPECT_EQ(g_kvNbDelegatePtr->GetLocal(KEY_2, readValue), NOT_FOUND);

    // finilize
    EXPECT_EQ(g_mgr.CloseKvStore(g_kvNbDelegatePtr), OK);
    EXPECT_EQ(g_mgr.DeleteKvStore("distributed_nb_publish_SingleVerPublishKey001"), OK);
    g_kvNbDelegatePtr = nullptr;
}

/**
  * @tc.name: SingleVerPublishKey002
  * @tc.desc: Publish no conflict key without deleting key from local table
  * @tc.type: FUNC
  * @tc.require: AR000DPTQ5
  * @tc.author: wumin
  */
HWTEST_F(DistributedDBInterfacesNBPublishTest, SingleVerPublishKey002, TestSize.Level1)
{
    const KvStoreNbDelegate::Option option = {true, false};
    g_mgr.GetKvStore("distributed_nb_publish_SingleVerPublishKey002", option, g_kvNbDelegateCallback);
    ASSERT_TRUE(g_kvNbDelegatePtr != nullptr);
    EXPECT_TRUE(g_kvDelegateStatus == OK);

    EXPECT_EQ(g_kvNbDelegatePtr->PutLocal(KEY_1, VALUE_1), OK);
    EXPECT_EQ(g_kvNbDelegatePtr->PutLocal(KEY_2, VALUE_2), OK);
    EXPECT_EQ(g_kvNbDelegatePtr->Put(KEY_2, VALUE_3), OK);

    KvStoreObserverUnitTest *observerLocal = new (std::nothrow) KvStoreObserverUnitTest;
    ASSERT_TRUE(observerLocal != nullptr);
    EXPECT_EQ(g_kvNbDelegatePtr->RegisterObserver(NULL_KEY, OBSERVER_CHANGES_LOCAL_ONLY, observerLocal), OK);

    KvStoreObserverUnitTest *observerSync = new (std::nothrow) KvStoreObserverUnitTest;
    ASSERT_TRUE(observerSync != nullptr);
    EXPECT_EQ(g_kvNbDelegatePtr->RegisterObserver(NULL_KEY, OBSERVER_CHANGES_NATIVE, observerSync), OK);
    /**
     * @tc.steps:step1. PublishLocal key1.
     * @tc.expected: step1. return OK.
     */
    EXPECT_EQ(g_kvNbDelegatePtr->PublishLocal(KEY_1, false, true, nullptr), OK);
    /**
     * @tc.steps:step2. Get value of key1 from local table
     * @tc.expected: step2. value of key1 is value1
     */
    Value readValue;
    EXPECT_EQ(g_kvNbDelegatePtr->GetLocal(KEY_1, readValue), OK);
    EXPECT_EQ(readValue, VALUE_1);
    /**
     * @tc.steps:step3. Get value of key2 from local table
     * @tc.expected: step3. value of key2 is value2
     */
    EXPECT_EQ(g_kvNbDelegatePtr->GetLocal(KEY_2, readValue), OK);
    EXPECT_EQ(readValue, VALUE_2);
    /**
     * @tc.steps:step4. Get value of key1 from sync table
     * @tc.expected: step4. value of key1 is value1
     */
    EXPECT_EQ(g_kvNbDelegatePtr->Get(KEY_1, readValue), OK);
    EXPECT_EQ(readValue, VALUE_1);
    /**
     * @tc.steps:step5. Get value of key2 from sync table
     * @tc.expected: step5. value of key2 is value3
     */
    EXPECT_EQ(g_kvNbDelegatePtr->Get(KEY_2, readValue), OK);
    EXPECT_EQ(readValue, VALUE_3);
    /**
     * @tc.steps:step6. Check observer data.
     * @tc.expected: step6. return OK.
     */
    std::this_thread::sleep_for(std::chrono::milliseconds(OBSERVER_SLEEP_TIME));
    vector<Entry> entriesRet = {{KEY_1, VALUE_1}};
    EXPECT_TRUE(DistributedDBToolsUnitTest::CheckObserverResult(entriesRet, observerSync->GetEntriesInserted()));
    EXPECT_EQ(observerLocal->GetCallCount(), 0UL);

    // finilize
    EXPECT_EQ(g_kvNbDelegatePtr->UnRegisterObserver(observerLocal), OK);
    EXPECT_EQ(g_kvNbDelegatePtr->UnRegisterObserver(observerSync), OK);
    delete observerLocal;
    observerLocal = nullptr;
    delete observerSync;
    observerSync = nullptr;

    EXPECT_EQ(g_mgr.CloseKvStore(g_kvNbDelegatePtr), OK);
    EXPECT_EQ(g_mgr.DeleteKvStore("distributed_nb_publish_SingleVerPublishKey002"), OK);
    g_kvNbDelegatePtr = nullptr;
}

/**
  * @tc.name: SingleVerPublishKey003
  * @tc.desc: Publish no conflict key with deleting key from local table
  * @tc.type: FUNC
  * @tc.require: AR000DPTQ5
  * @tc.author: wumin
  */
HWTEST_F(DistributedDBInterfacesNBPublishTest, SingleVerPublishKey003, TestSize.Level1)
{
    const KvStoreNbDelegate::Option option = {true, false};
    g_mgr.GetKvStore("distributed_nb_publish_SingleVerPublishKey003", option, g_kvNbDelegateCallback);
    ASSERT_TRUE(g_kvNbDelegatePtr != nullptr);
    EXPECT_TRUE(g_kvDelegateStatus == OK);

    EXPECT_EQ(g_kvNbDelegatePtr->PutLocal(KEY_1, VALUE_1), OK);
    EXPECT_EQ(g_kvNbDelegatePtr->PutLocal(KEY_2, VALUE_2), OK);
    EXPECT_EQ(g_kvNbDelegatePtr->Put(KEY_2, VALUE_3), OK);

    KvStoreObserverUnitTest *observerLocal = new (std::nothrow) KvStoreObserverUnitTest;
    ASSERT_TRUE(observerLocal != nullptr);
    EXPECT_EQ(g_kvNbDelegatePtr->RegisterObserver(NULL_KEY, OBSERVER_CHANGES_LOCAL_ONLY, observerLocal), OK);

    KvStoreObserverUnitTest *observerSync = new (std::nothrow) KvStoreObserverUnitTest;
    ASSERT_TRUE(observerSync != nullptr);
    EXPECT_EQ(g_kvNbDelegatePtr->RegisterObserver(NULL_KEY, OBSERVER_CHANGES_NATIVE, observerSync), OK);
    /**
     * @tc.steps:step1. PublishLocal key1.
     * @tc.expected: step1. return OK.
     */
    EXPECT_EQ(g_kvNbDelegatePtr->PublishLocal(KEY_1, true, true, nullptr), OK);
    /**
     * @tc.steps:step2. Get value of key1 from local table
     * @tc.expected: step2. value of key1 is value1
     */
    Value readValue;
    EXPECT_EQ(g_kvNbDelegatePtr->GetLocal(KEY_1, readValue), NOT_FOUND);
    /**
     * @tc.steps:step3. Get value of key2 from local table
     * @tc.expected: step3. value of key2 is value2
     */
    EXPECT_EQ(g_kvNbDelegatePtr->GetLocal(KEY_2, readValue), OK);
    EXPECT_EQ(readValue, VALUE_2);
    /**
     * @tc.steps:step4. Get value of key1 from sync table
     * @tc.expected: step4. value of key1 is value1
     */
    EXPECT_EQ(g_kvNbDelegatePtr->Get(KEY_1, readValue), OK);
    EXPECT_EQ(readValue, VALUE_1);
    /**
     * @tc.steps:step5. Get value of key2 from sync table
     * @tc.expected: step5. value of key2 is value3
     */
    EXPECT_EQ(g_kvNbDelegatePtr->Get(KEY_2, readValue), OK);
    EXPECT_EQ(readValue, VALUE_3);
    /**
     * @tc.steps:step6. Check observer data.
     * @tc.expected: step6. return OK.
     */
    std::this_thread::sleep_for(std::chrono::milliseconds(OBSERVER_SLEEP_TIME));
    vector<Entry> entriesRet = {{KEY_1, VALUE_1}};
    EXPECT_TRUE(DistributedDBToolsUnitTest::CheckObserverResult(entriesRet, observerSync->GetEntriesInserted()));
    EXPECT_TRUE(DistributedDBToolsUnitTest::CheckObserverResult(entriesRet, observerLocal->GetEntriesDeleted()));

    // finilize
    EXPECT_EQ(g_kvNbDelegatePtr->UnRegisterObserver(observerLocal), OK);
    EXPECT_EQ(g_kvNbDelegatePtr->UnRegisterObserver(observerSync), OK);
    delete observerLocal;
    observerLocal = nullptr;
    delete observerSync;
    observerSync = nullptr;

    EXPECT_EQ(g_mgr.CloseKvStore(g_kvNbDelegatePtr), OK);
    EXPECT_EQ(g_mgr.DeleteKvStore("distributed_nb_publish_SingleVerPublishKey003"), OK);
    g_kvNbDelegatePtr = nullptr;
}

/**
  * @tc.name: SingleVerPublishKey004
  * @tc.desc: Publish conflict key and update timestamp
  * @tc.type: FUNC
  * @tc.require: AR000DPTQ5
  * @tc.author: wumin
  */
HWTEST_F(DistributedDBInterfacesNBPublishTest, SingleVerPublishKey004, TestSize.Level1)
{
    const KvStoreNbDelegate::Option option = {true, false};
    g_mgr.GetKvStore("distributed_nb_publish_SingleVerPublishKey004", option, g_kvNbDelegateCallback);
    ASSERT_TRUE(g_kvNbDelegatePtr != nullptr);
    EXPECT_TRUE(g_kvDelegateStatus == OK);

    EXPECT_EQ(g_kvNbDelegatePtr->PutLocal(KEY_1, VALUE_1), OK);
    EXPECT_EQ(g_kvNbDelegatePtr->PutLocal(KEY_2, VALUE_2), OK);
    EXPECT_EQ(g_kvNbDelegatePtr->Put(KEY_2, VALUE_3), OK);
    EXPECT_EQ(g_kvNbDelegatePtr->Put(KEY_1, VALUE_4), OK);

    KvStoreObserverUnitTest *observerLocal = new (std::nothrow) KvStoreObserverUnitTest;
    ASSERT_TRUE(observerLocal != nullptr);
    EXPECT_EQ(g_kvNbDelegatePtr->RegisterObserver(NULL_KEY, OBSERVER_CHANGES_LOCAL_ONLY, observerLocal), OK);

    KvStoreObserverUnitTest *observerSync = new (std::nothrow) KvStoreObserverUnitTest;
    ASSERT_TRUE(observerSync != nullptr);
    EXPECT_EQ(g_kvNbDelegatePtr->RegisterObserver(NULL_KEY, OBSERVER_CHANGES_NATIVE, observerSync), OK);
    /**
     * @tc.steps:step1. PublishLocal key1.
     * @tc.expected: step1. return OK.
     */
    EXPECT_EQ(g_kvNbDelegatePtr->PublishLocal(KEY_1, false, true, nullptr), OK);
    /**
     * @tc.steps:step2. Get value of key1 from local table
     * @tc.expected: step2. value of key1 is value1
     */
    Value readValue;
    EXPECT_EQ(g_kvNbDelegatePtr->GetLocal(KEY_1, readValue), OK);
    EXPECT_EQ(readValue, VALUE_1);
    /**
     * @tc.steps:step3. Get value of key2 from local table
     * @tc.expected: step3. value of key2 is value2
     */
    EXPECT_EQ(g_kvNbDelegatePtr->GetLocal(KEY_2, readValue), OK);
    EXPECT_EQ(readValue, VALUE_2);
    /**
     * @tc.steps:step4. Get value of key1 from sync table
     * @tc.expected: step4. value of key1 is value1
     */
    EXPECT_EQ(g_kvNbDelegatePtr->Get(KEY_1, readValue), OK);
    EXPECT_EQ(readValue, VALUE_1);
    /**
     * @tc.steps:step5. Get value of key2 from sync table
     * @tc.expected: step5. value of key2 is value3
     */
    EXPECT_EQ(g_kvNbDelegatePtr->Get(KEY_2, readValue), OK);
    EXPECT_EQ(readValue, VALUE_3);
    /**
     * @tc.steps:step6. Check observer data.
     * @tc.expected: step6. return OK.
     */
    std::this_thread::sleep_for(std::chrono::milliseconds(OBSERVER_SLEEP_TIME));
    vector<Entry> entriesRet = {{KEY_1, VALUE_1}};
    EXPECT_TRUE(DistributedDBToolsUnitTest::CheckObserverResult(entriesRet, observerSync->GetEntriesUpdated()));
    EXPECT_EQ(observerLocal->GetCallCount(), 0UL);

    // finilize
    EXPECT_EQ(g_kvNbDelegatePtr->UnRegisterObserver(observerLocal), OK);
    EXPECT_EQ(g_kvNbDelegatePtr->UnRegisterObserver(observerSync), OK);
    delete observerLocal;
    observerLocal = nullptr;
    delete observerSync;
    observerSync = nullptr;

    EXPECT_EQ(g_mgr.CloseKvStore(g_kvNbDelegatePtr), OK);
    EXPECT_EQ(g_mgr.DeleteKvStore("distributed_nb_publish_SingleVerPublishKey004"), OK);
    g_kvNbDelegatePtr = nullptr;
}

/**
  * @tc.name: SingleVerPublishKey005
  * @tc.desc: Publish conflict key but do not update timestamp
  * @tc.type: FUNC
  * @tc.require: AR000DPTQ5
  * @tc.author: wumin
  */
HWTEST_F(DistributedDBInterfacesNBPublishTest, SingleVerPublishKey005, TestSize.Level1)
{
    const KvStoreNbDelegate::Option option = {true, false};
    g_mgr.GetKvStore("distributed_nb_publish_SingleVerPublishKey005", option, g_kvNbDelegateCallback);
    ASSERT_TRUE(g_kvNbDelegatePtr != nullptr);
    EXPECT_TRUE(g_kvDelegateStatus == OK);

    EXPECT_EQ(g_kvNbDelegatePtr->PutLocal(KEY_1, VALUE_1), OK);
    EXPECT_EQ(g_kvNbDelegatePtr->PutLocal(KEY_2, VALUE_2), OK);
    EXPECT_EQ(g_kvNbDelegatePtr->Put(KEY_2, VALUE_3), OK);
    EXPECT_EQ(g_kvNbDelegatePtr->Put(KEY_1, VALUE_4), OK);

    KvStoreObserverUnitTest *observerLocal = new (std::nothrow) KvStoreObserverUnitTest;
    ASSERT_TRUE(observerLocal != nullptr);
    EXPECT_EQ(g_kvNbDelegatePtr->RegisterObserver(NULL_KEY, OBSERVER_CHANGES_LOCAL_ONLY, observerLocal), OK);

    KvStoreObserverUnitTest *observerSync = new (std::nothrow) KvStoreObserverUnitTest;
    ASSERT_TRUE(observerSync != nullptr);
    EXPECT_EQ(g_kvNbDelegatePtr->RegisterObserver(NULL_KEY, OBSERVER_CHANGES_NATIVE, observerSync), OK);
    /**
     * @tc.steps:step1. PublishLocal key1.
     * @tc.expected: step1. return OK.
     */
    EXPECT_EQ(g_kvNbDelegatePtr->PublishLocal(KEY_1, true, false, nullptr), STALE);
    /**
     * @tc.steps:step2. Get value of key1 from local table
     * @tc.expected: step2. value of key1 is value1
     */
    Value readValue;
    EXPECT_EQ(g_kvNbDelegatePtr->GetLocal(KEY_1, readValue), OK);
    EXPECT_EQ(readValue, VALUE_1);
    /**
     * @tc.steps:step3. Get value of key2 from local table
     * @tc.expected: step3. value of key2 is value2
     */
    EXPECT_EQ(g_kvNbDelegatePtr->GetLocal(KEY_2, readValue), OK);
    EXPECT_EQ(readValue, VALUE_2);
    /**
     * @tc.steps:step4. Get value of key1 from sync table
     * @tc.expected: step4. value of key1 is value4
     */
    EXPECT_EQ(g_kvNbDelegatePtr->Get(KEY_1, readValue), OK);
    EXPECT_EQ(readValue, VALUE_4);
    /**
     * @tc.steps:step5. Get value of key2 from sync table
     * @tc.expected: step5. value of key2 is value3
     */
    EXPECT_EQ(g_kvNbDelegatePtr->Get(KEY_2, readValue), OK);
    EXPECT_EQ(readValue, VALUE_3);
    /**
     * @tc.steps:step6. Check observer data.
     * @tc.expected: step6. return OK.
     */
    std::this_thread::sleep_for(std::chrono::milliseconds(OBSERVER_SLEEP_TIME));
    EXPECT_EQ(observerSync->GetCallCount(), 0UL);
    EXPECT_EQ(observerLocal->GetCallCount(), 0UL);

    // finilize
    EXPECT_EQ(g_kvNbDelegatePtr->UnRegisterObserver(observerLocal), OK);
    EXPECT_EQ(g_kvNbDelegatePtr->UnRegisterObserver(observerSync), OK);
    delete observerLocal;
    observerLocal = nullptr;
    delete observerSync;
    observerSync = nullptr;

    EXPECT_EQ(g_mgr.CloseKvStore(g_kvNbDelegatePtr), OK);
    EXPECT_EQ(g_mgr.DeleteKvStore("distributed_nb_publish_SingleVerPublishKey005"), OK);
    g_kvNbDelegatePtr = nullptr;
}

/**
  * @tc.name: SingleVerPublishKey006
  * @tc.desc: Publish no conflict key and onConflict() not null
  * @tc.type: FUNC
  * @tc.require: AR000DPTQ5
  * @tc.author: wumin
  */
HWTEST_F(DistributedDBInterfacesNBPublishTest, SingleVerPublishKey006, TestSize.Level1)
{
    const KvStoreNbDelegate::Option option = {true, false};
    g_mgr.GetKvStore("distributed_nb_publish_SingleVerPublishKey006", option, g_kvNbDelegateCallback);
    ASSERT_TRUE(g_kvNbDelegatePtr != nullptr);
    EXPECT_TRUE(g_kvDelegateStatus == OK);

    EXPECT_EQ(g_kvNbDelegatePtr->PutLocal(KEY_1, VALUE_1), OK);
    /**
     * @tc.steps:step1. PublishLocal key1.
     * @tc.expected: step1. return OK.
     */
    ResetCallbackArg();
    EXPECT_EQ(g_kvNbDelegatePtr->PublishLocal(KEY_1, true, false, ConflictCallback), OK);
    Value readValue;
    EXPECT_EQ(g_kvNbDelegatePtr->GetLocal(KEY_1, readValue), NOT_FOUND);
    /**
     * @tc.steps:step2. Check onConflict callback.
     * @tc.expected: step2. return OK.
     */
    EXPECT_EQ(g_conflictCallbackTimes, 0);
    /**
     * @tc.steps:step3. Get value of key1 from sync table
     * @tc.expected: step3. value of key1 is value4
     */
    EXPECT_EQ(g_kvNbDelegatePtr->Get(KEY_1, readValue), OK);
    EXPECT_EQ(readValue, VALUE_1);

    // finilize
    EXPECT_EQ(g_mgr.CloseKvStore(g_kvNbDelegatePtr), OK);
    EXPECT_EQ(g_mgr.DeleteKvStore("distributed_nb_publish_SingleVerPublishKey006"), OK);
    g_kvNbDelegatePtr = nullptr;
}

/**
  * @tc.name: SingleVerPublishKey007
  * @tc.desc: Publish conflict key and onConflict() not null
  * @tc.type: FUNC
  * @tc.require: AR000DPTQ5
  * @tc.author: wumin
  */
HWTEST_F(DistributedDBInterfacesNBPublishTest, SingleVerPublishKey007, TestSize.Level1)
{
    const KvStoreNbDelegate::Option option = {true, false};
    g_mgr.GetKvStore("distributed_nb_publish_SingleVerPublishKey007", option, g_kvNbDelegateCallback);
    ASSERT_TRUE(g_kvNbDelegatePtr != nullptr);
    EXPECT_TRUE(g_kvDelegateStatus == OK);

    EXPECT_EQ(g_kvNbDelegatePtr->PutLocal(KEY_1, VALUE_1), OK);
    EXPECT_EQ(g_kvNbDelegatePtr->Put(KEY_1, VALUE_2), OK);
    /**
     * @tc.steps:step1. PublishLocal key1.
     * @tc.expected: step1. return OK.
     */
    ResetCallbackArg(true);
    EXPECT_EQ(g_kvNbDelegatePtr->PublishLocal(KEY_1, true, false, ConflictCallback), OK);
    Value readValue;
    EXPECT_EQ(g_kvNbDelegatePtr->GetLocal(KEY_1, readValue), OK);
    EXPECT_EQ(readValue, VALUE_1);
    /**
     * @tc.steps:step2. Check onConflict callback.
     * @tc.expected: step2. return OK.
     */
    EXPECT_EQ(g_conflictCallbackTimes, 1);
    EXPECT_EQ(g_isLocalLastest, false);
    Entry entryRet = {KEY_1, VALUE_1};
    EXPECT_TRUE(DistributedDBToolsUnitTest::IsEntryEqual(g_localEntry, entryRet));
    entryRet = {KEY_1, VALUE_2};
    EXPECT_TRUE(DistributedDBToolsUnitTest::IsEntryEqual(g_syncEntry, entryRet));
    /**
     * @tc.steps:step3. Get value of key1 from sync table
     * @tc.expected: step3. value of key1 is value4
     */
    EXPECT_EQ(g_kvNbDelegatePtr->Get(KEY_1, readValue), OK);
    EXPECT_EQ(readValue, VALUE_2);

    // finilize
    EXPECT_EQ(g_mgr.CloseKvStore(g_kvNbDelegatePtr), OK);
    EXPECT_EQ(g_mgr.DeleteKvStore("distributed_nb_publish_SingleVerPublishKey007"), OK);
    g_kvNbDelegatePtr = nullptr;
}

/**
  * @tc.name: SingleVerPublishKey008
  * @tc.desc: Publish conflict key and onConflict() not null, need update timestamp
  * @tc.type: FUNC
  * @tc.require: AR000DPTQ5
  * @tc.author: wumin
  */
HWTEST_F(DistributedDBInterfacesNBPublishTest, SingleVerPublishKey008, TestSize.Level1)
{
    const KvStoreNbDelegate::Option option = {true, false};
    g_mgr.GetKvStore("distributed_nb_publish_SingleVerPublishKey008", option, g_kvNbDelegateCallback);
    ASSERT_TRUE(g_kvNbDelegatePtr != nullptr);
    EXPECT_TRUE(g_kvDelegateStatus == OK);

    EXPECT_EQ(g_kvNbDelegatePtr->PutLocal(KEY_1, VALUE_1), OK);
    EXPECT_EQ(g_kvNbDelegatePtr->Put(KEY_1, VALUE_2), OK);
    /**
     * @tc.steps:step1. PublishLocal key1.
     * @tc.expected: step1. return OK.
     */
    ResetCallbackArg(false);
    EXPECT_EQ(g_kvNbDelegatePtr->PublishLocal(KEY_1, true, true, ConflictCallback), OK);
    Value readValue;
    EXPECT_EQ(g_kvNbDelegatePtr->GetLocal(KEY_1, readValue), OK);
    EXPECT_EQ(readValue, VALUE_1);
    /**
     * @tc.steps:step2. Check onConflict callback.
     * @tc.expected: step2. return OK.
     */
    EXPECT_EQ(g_conflictCallbackTimes, 1);
    EXPECT_EQ(g_isLocalLastest, true);
    Entry entryRet = {KEY_1, VALUE_1};
    EXPECT_TRUE(DistributedDBToolsUnitTest::IsEntryEqual(g_localEntry, entryRet));
    entryRet = {KEY_1, VALUE_2};
    EXPECT_TRUE(DistributedDBToolsUnitTest::IsEntryEqual(g_syncEntry, entryRet));
    /**
     * @tc.steps:step3. Get value of key1 from sync table
     * @tc.expected: step3. value of key1 is value4
     */
    EXPECT_EQ(g_kvNbDelegatePtr->Get(KEY_1, readValue), OK);
    EXPECT_EQ(readValue, VALUE_2);

    // finilize
    EXPECT_EQ(g_mgr.CloseKvStore(g_kvNbDelegatePtr), OK);
    EXPECT_EQ(g_mgr.DeleteKvStore("distributed_nb_publish_SingleVerPublishKey008"), OK);
    g_kvNbDelegatePtr = nullptr;
}

/**
  * @tc.name: SingleVerPublishKey009
  * @tc.desc: Publish conflict key (deleted) and onConflict() not null
  * @tc.type: FUNC
  * @tc.require: AR000DPTQ5
  * @tc.author: wumin
  */
HWTEST_F(DistributedDBInterfacesNBPublishTest, SingleVerPublishKey009, TestSize.Level1)
{
    const KvStoreNbDelegate::Option option = {true, false};
    g_mgr.GetKvStore("distributed_nb_publish_SingleVerPublishKey009", option, g_kvNbDelegateCallback);
    ASSERT_TRUE(g_kvNbDelegatePtr != nullptr);
    EXPECT_TRUE(g_kvDelegateStatus == OK);

    EXPECT_EQ(g_kvNbDelegatePtr->PutLocal(KEY_1, VALUE_1), OK);
    EXPECT_EQ(g_kvNbDelegatePtr->Put(KEY_1, VALUE_2), OK);
    EXPECT_EQ(g_kvNbDelegatePtr->Delete(KEY_1), OK);
    /**
     * @tc.steps:step1. PublishLocal key1.
     * @tc.expected: step1. return OK.
     */
    ResetCallbackArg(true);
    EXPECT_EQ(g_kvNbDelegatePtr->PublishLocal(KEY_1, true, false, ConflictCallback), OK);
    Value readValue;
    EXPECT_EQ(g_kvNbDelegatePtr->GetLocal(KEY_1, readValue), OK);
    EXPECT_EQ(readValue, VALUE_1);
    /**
     * @tc.steps:step2. Check onConflict callback.
     * @tc.expected: step2. return OK.
     */
    EXPECT_EQ(g_conflictCallbackTimes, 1);
    EXPECT_EQ(g_isLocalLastest, false);
    EXPECT_EQ(g_isSyncNull, true);
    Entry entryRet = {KEY_1, VALUE_1};
    EXPECT_TRUE(DistributedDBToolsUnitTest::IsEntryEqual(g_localEntry, entryRet));
    /**
     * @tc.steps:step3. Get value of key1 from sync table
     * @tc.expected: step3. value of key1 is value4
     */
    EXPECT_EQ(g_kvNbDelegatePtr->Get(KEY_1, readValue), NOT_FOUND);

    // finilize
    EXPECT_EQ(g_mgr.CloseKvStore(g_kvNbDelegatePtr), OK);
    EXPECT_EQ(g_mgr.DeleteKvStore("distributed_nb_publish_SingleVerPublishKey009"), OK);
    g_kvNbDelegatePtr = nullptr;
}

/**
  * @tc.name: SingleVerPublishKey010
  * @tc.desc: Publish conflict key (deleted) and onConflict() not null, need update timestamp
  * @tc.type: FUNC
  * @tc.require: AR000DPTQ5
  * @tc.author: wumin
  */
HWTEST_F(DistributedDBInterfacesNBPublishTest, SingleVerPublishKey010, TestSize.Level1)
{
    const KvStoreNbDelegate::Option option = {true, false};
    g_mgr.GetKvStore("distributed_nb_publish_SingleVerPublishKey010", option, g_kvNbDelegateCallback);
    ASSERT_TRUE(g_kvNbDelegatePtr != nullptr);
    EXPECT_TRUE(g_kvDelegateStatus == OK);

    EXPECT_EQ(g_kvNbDelegatePtr->PutLocal(KEY_1, VALUE_1), OK);
    EXPECT_EQ(g_kvNbDelegatePtr->Put(KEY_1, VALUE_2), OK);
    EXPECT_EQ(g_kvNbDelegatePtr->Delete(KEY_1), OK);
    /**
     * @tc.steps:step1. PublishLocal key1.
     * @tc.expected: step1. return OK.
     */
    ResetCallbackArg(false);
    EXPECT_EQ(g_kvNbDelegatePtr->PublishLocal(KEY_1, true, true, ConflictCallback), OK);
    Value readValue;
    EXPECT_EQ(g_kvNbDelegatePtr->GetLocal(KEY_1, readValue), OK);
    EXPECT_EQ(readValue, VALUE_1);
    /**
     * @tc.steps:step2. Check onConflict callback.
     * @tc.expected: step2. return OK.
     */
    EXPECT_EQ(g_conflictCallbackTimes, 1);
    EXPECT_EQ(g_isLocalLastest, true);
    EXPECT_EQ(g_isSyncNull, true);
    Entry entryRet = {KEY_1, VALUE_1};
    EXPECT_TRUE(DistributedDBToolsUnitTest::IsEntryEqual(g_localEntry, entryRet));
    /**
     * @tc.steps:step3. Get value of key1 from sync table
     * @tc.expected: step3. value of key1 is value4
     */
    EXPECT_EQ(g_kvNbDelegatePtr->Get(KEY_1, readValue), NOT_FOUND);

    // finilize
    EXPECT_EQ(g_mgr.CloseKvStore(g_kvNbDelegatePtr), OK);
    EXPECT_EQ(g_mgr.DeleteKvStore("distributed_nb_publish_SingleVerPublishKey010"), OK);
    g_kvNbDelegatePtr = nullptr;
}

/**
  * @tc.name: SingleVerPublishKey011
  * @tc.desc: Publish conflict key (deleted) and onConflict() not null, put(k1,v1) in callback method
  * @tc.type: FUNC
  * @tc.require: AR000DPTQ5
  * @tc.author: wumin
  */
HWTEST_F(DistributedDBInterfacesNBPublishTest, SingleVerPublishKey011, TestSize.Level1)
{
    const KvStoreNbDelegate::Option option = {true, false};
    g_mgr.GetKvStore("distributed_nb_publish_SingleVerPublishKey011", option, g_kvNbDelegateCallback);
    ASSERT_TRUE(g_kvNbDelegatePtr != nullptr);
    EXPECT_TRUE(g_kvDelegateStatus == OK);

    EXPECT_EQ(g_kvNbDelegatePtr->PutLocal(KEY_1, VALUE_1), OK);
    EXPECT_EQ(g_kvNbDelegatePtr->Put(KEY_1, VALUE_2), OK);
    EXPECT_EQ(g_kvNbDelegatePtr->Delete(KEY_1), OK);
    /**
     * @tc.steps:step1. PublishLocal key1.
     * @tc.expected: step1. return OK.
     */
    ResetCallbackArg(true);
    g_isNeedInsertEntryInCallback = true;
    EXPECT_EQ(g_kvNbDelegatePtr->PublishLocal(KEY_1, true, false, ConflictCallback), OK);
    Value readValue;
    EXPECT_EQ(g_kvNbDelegatePtr->GetLocal(KEY_1, readValue), OK);
    EXPECT_EQ(readValue, VALUE_1);
    /**
     * @tc.steps:step2. Check onConflict callback.
     * @tc.expected: step2. return OK.
     */
    EXPECT_EQ(g_conflictCallbackTimes, 1);
    EXPECT_EQ(g_isLocalLastest, false);
    EXPECT_EQ(g_isSyncNull, true);
    Entry entryRet = {KEY_1, VALUE_1};
    EXPECT_TRUE(DistributedDBToolsUnitTest::IsEntryEqual(g_localEntry, entryRet));
    /**
     * @tc.steps:step3. Get value of key1 from sync table
     * @tc.expected: step3. value of key1 is value4
     */
    EXPECT_EQ(g_kvNbDelegatePtr->Get(KEY_1, readValue), OK);
    EXPECT_EQ(readValue, VALUE_1);

    // finilize
    EXPECT_EQ(g_mgr.CloseKvStore(g_kvNbDelegatePtr), OK);
    EXPECT_EQ(g_mgr.DeleteKvStore("distributed_nb_publish_SingleVerPublishKey011"), OK);
    g_kvNbDelegatePtr = nullptr;
}
