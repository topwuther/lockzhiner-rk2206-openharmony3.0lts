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

#include "db_errno.h"
#include "db_constant.h"
#include "log_print.h"
#include "distributeddb_data_generate_unit_test.h"
#include "distributeddb_tools_unit_test.h"

using namespace testing::ext;
using namespace DistributedDB;
using namespace DistributedDBUnitTest;
using namespace std;

namespace {
    string g_testDir;
    KvStoreConfig g_config;
    KvStoreDelegateManager g_mgr(APP_ID, USER_ID);
    DBStatus g_kvDelegateStatus = INVALID_ARGS;
    KvStoreNbDelegate *g_kvNbDelegatePtr = nullptr;

    const int OBSERVER_SLEEP_TIME = 100;
    const int BATCH_PRESET_SIZE_TEST = 10;
    const int DIVIDE_BATCH_PRESET_SIZE = 5;

    const Key KEY1{'k', 'e', 'y', '1'};
    const Key KEY2{'k', 'e', 'y', '2'};
    const Value VALUE1{'v', 'a', 'l', 'u', 'e', '1'};
    const Value VALUE2{'v', 'a', 'l', 'u', 'e', '2'};

    // the type of g_kvNbDelegateCallback is function<void(DBStatus, KvStoreDelegate*)>
    auto g_kvNbDelegateCallback = bind(&DistributedDBToolsUnitTest::KvStoreNbDelegateCallback, placeholders::_1,
        placeholders::_2, std::ref(g_kvDelegateStatus), std::ref(g_kvNbDelegatePtr));
}

class DistributedDBInterfacesTransactionOptimizationTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
};

void DistributedDBInterfacesTransactionOptimizationTest::SetUpTestCase(void)
{
    DistributedDBToolsUnitTest::TestDirInit(g_testDir);
    g_config.dataDir = g_testDir;
    g_mgr.SetKvStoreConfig(g_config);
}

void DistributedDBInterfacesTransactionOptimizationTest::TearDownTestCase(void)
{
    if (DistributedDBToolsUnitTest::RemoveTestDbFiles(g_testDir) != 0) {
        LOGE("rm test db files error!");
    }
}

void DistributedDBInterfacesTransactionOptimizationTest::SetUp(void)
{
    g_kvDelegateStatus = INVALID_ARGS;
    g_kvNbDelegatePtr = nullptr;
}

void DistributedDBInterfacesTransactionOptimizationTest::TearDown(void)
{
    if (g_kvNbDelegatePtr != nullptr) {
        g_mgr.CloseKvStore(g_kvNbDelegatePtr);
        g_kvNbDelegatePtr = nullptr;
    }
}

/**
  * @tc.name: BatchOperationsOfSyncAndLocal001
  * @tc.desc: Verify the batch put and query functions of the sync and local data in the same transaction.
  * @tc.type: FUNC
  * @tc.require: AR000EPAS8
  * @tc.author: changguicai
  */
HWTEST_F(DistributedDBInterfacesTransactionOptimizationTest, SyncAndLocalBatchOperations001, TestSize.Level1)
{
    /**
     * @tc.steps:step1. Get the nb delegate.
     * @tc.expected: step1. Get results OK and non-null delegate.
     */
    std::string storeId("SyncAndLocalBatchOperations001");
    KvStoreNbDelegate::Option option = {true, false, false};
    g_mgr.GetKvStore(storeId, option, g_kvNbDelegateCallback);
    ASSERT_TRUE(g_kvNbDelegatePtr != nullptr);
    EXPECT_TRUE(g_kvDelegateStatus == OK);

    /**
     * @tc.steps:step2. Starting a transaction.
     * @tc.expected: step2. The transaction is started successfully.
     */
    EXPECT_EQ(g_kvNbDelegatePtr->StartTransaction(), OK);

    vector<Entry> entries;
    vector<Key> keys;
    DistributedDBUnitTest::GenerateRecords(BATCH_PRESET_SIZE_TEST, entries, keys);
    EXPECT_TRUE(entries.size() == BATCH_PRESET_SIZE_TEST);

    vector<Entry> localEntrys;
    vector<Key> localKeys;
    DistributedDBUnitTest::GenerateRecords(DIVIDE_BATCH_PRESET_SIZE, localEntrys, localKeys);
    EXPECT_TRUE(localEntrys.size() == DIVIDE_BATCH_PRESET_SIZE);

    /**
     * @tc.steps:step3. Put batch data.
     * @tc.expected: step3. Returns OK.
     */
    EXPECT_EQ(g_kvNbDelegatePtr->PutBatch(entries), OK);
    EXPECT_EQ(g_kvNbDelegatePtr->PutLocalBatch(localEntrys), OK);

    Key keyPrefix;
    std::vector<Entry> getSyncEntries;
    EXPECT_EQ(g_kvNbDelegatePtr->GetEntries(keyPrefix, getSyncEntries), OK);
    EXPECT_TRUE(DistributedDBToolsUnitTest::IsEntriesEqual(entries, getSyncEntries, true));

    std::vector<Entry> getLocalEntries;
    EXPECT_EQ(g_kvNbDelegatePtr->GetLocalEntries(keyPrefix, getLocalEntries), OK);
    EXPECT_TRUE(DistributedDBToolsUnitTest::IsEntriesEqual(localEntrys, getLocalEntries, true));

    /**
     * @tc.steps:step4. Commit a transaction.
     * @tc.expected: step4. Transaction submitted successfully.
     */
    EXPECT_EQ(g_kvNbDelegatePtr->Commit(), OK);

    /**
     * @tc.steps:step5. GetEntries after the transaction is submitted.
     * @tc.expected: step5. GetEntries return OK and the geted data is correct.
     */
    getSyncEntries.clear();
    EXPECT_EQ(g_kvNbDelegatePtr->GetEntries(keyPrefix, getSyncEntries), OK);
    EXPECT_TRUE(DistributedDBToolsUnitTest::IsEntriesEqual(entries, getSyncEntries, true));

    getLocalEntries.clear();
    EXPECT_EQ(g_kvNbDelegatePtr->GetLocalEntries(keyPrefix, getLocalEntries), OK);
    EXPECT_TRUE(DistributedDBToolsUnitTest::IsEntriesEqual(localEntrys, getLocalEntries, true));

    /**
     * @tc.steps:step6. Close the kv store.
     * @tc.expected: step6. Results OK and delete successfully.
     */
    EXPECT_EQ(g_mgr.CloseKvStore(g_kvNbDelegatePtr), OK);
    EXPECT_EQ(g_mgr.DeleteKvStore(storeId), OK);
    g_kvNbDelegatePtr = nullptr;
}

/**
  * @tc.name: SyncAndLocalSingleOperations001
  * @tc.desc: Verify the single put and query functions of the sync and local data in the same transaction.
  * @tc.type: FUNC
  * @tc.require: AR000EPAS8
  * @tc.author: changguicai
  */
HWTEST_F(DistributedDBInterfacesTransactionOptimizationTest, SyncAndLocalSingleOperations001, TestSize.Level1)
{
    /**
     * @tc.steps:step1. Get the nb delegate.
     * @tc.expected: step1. Get results OK and non-null delegate.
     */
    std::string storeId("SyncAndLocalSingleOperations001");
    KvStoreNbDelegate::Option option = {true, false, false};
    g_mgr.GetKvStore(storeId, option, g_kvNbDelegateCallback);
    ASSERT_TRUE(g_kvNbDelegatePtr != nullptr);
    EXPECT_TRUE(g_kvDelegateStatus == OK);

    /**
     * @tc.steps:step2. Starting a transaction.
     * @tc.expected: step2. The transaction is started successfully.
     */
    EXPECT_EQ(g_kvNbDelegatePtr->StartTransaction(), OK);

    /**
     * @tc.steps:step3. Put and Get single data.
     * @tc.expected: step3. Returns OK.
     */
    EXPECT_EQ(g_kvNbDelegatePtr->Put(KEY1, VALUE1), OK);
    EXPECT_EQ(g_kvNbDelegatePtr->PutLocal(KEY2, VALUE2), OK);

    Value getSyncValue;
    EXPECT_EQ(g_kvNbDelegatePtr->Get(KEY1, getSyncValue), OK);
    EXPECT_TRUE(DistributedDBToolsUnitTest::IsValueEqual(VALUE1, getSyncValue));

    Value getLocalValue;
    EXPECT_EQ(g_kvNbDelegatePtr->GetLocal(KEY2, getLocalValue), OK);
    EXPECT_TRUE(DistributedDBToolsUnitTest::IsValueEqual(VALUE2, getLocalValue));

    /**
     * @tc.steps:step4. Commit a transaction.
     * @tc.expected: step4. Transaction submitted successfully.
     */
    EXPECT_EQ(g_kvNbDelegatePtr->Commit(), OK);

    /**
     * @tc.steps:step5. Get after the transaction is submitted.
     * @tc.expected: step5. Get return OK and the geted data is correct.
     */
    getSyncValue.clear();
    EXPECT_EQ(g_kvNbDelegatePtr->Get(KEY1, getSyncValue), OK);
    EXPECT_TRUE(DistributedDBToolsUnitTest::IsValueEqual(VALUE1, getSyncValue));

    getLocalValue.clear();
    EXPECT_EQ(g_kvNbDelegatePtr->GetLocal(KEY2, getLocalValue), OK);
    EXPECT_TRUE(DistributedDBToolsUnitTest::IsValueEqual(VALUE2, getLocalValue));

    /**
     * @tc.steps:step6. Close the kv store.
     * @tc.expected: step6. Results OK and delete successfully.
     */
    EXPECT_EQ(g_mgr.CloseKvStore(g_kvNbDelegatePtr), OK);
    EXPECT_EQ(g_mgr.DeleteKvStore(storeId), OK);
    g_kvNbDelegatePtr = nullptr;
}

/**
  * @tc.name: DeleteInTransaction001
  * @tc.desc: Verify that the sync and local functions can be deleted in the same transaction.
  * @tc.type: FUNC
  * @tc.require: AR000EPAS8
  * @tc.author: changguicai
  */
HWTEST_F(DistributedDBInterfacesTransactionOptimizationTest, DeleteInTransaction001, TestSize.Level1)
{
    /**
     * @tc.steps:step1. Get the nb delegate.
     * @tc.expected: step1. Get results OK and non-null delegate.
     */
    std::string storeId("DeleteInTransaction001");
    KvStoreNbDelegate::Option option = {true, false, false};
    g_mgr.GetKvStore(storeId, option, g_kvNbDelegateCallback);
    ASSERT_TRUE(g_kvNbDelegatePtr != nullptr);
    EXPECT_TRUE(g_kvDelegateStatus == OK);

    /**
     * @tc.steps:step2. Starting a transaction.
     * @tc.expected: step2. The transaction is started successfully.
     */
    EXPECT_EQ(g_kvNbDelegatePtr->StartTransaction(), OK);

    /**
     * @tc.steps:step3. Put and Get single data.
     * @tc.expected: step3. Returns OK.
     */
    EXPECT_EQ(g_kvNbDelegatePtr->Put(KEY1, VALUE1), OK);
    EXPECT_EQ(g_kvNbDelegatePtr->PutLocal(KEY2, VALUE2), OK);

    /**
     * @tc.steps:step4 Delete before the transaction is submitted.
     * @tc.expected: step4. Delete return OK.
     */
    EXPECT_EQ(g_kvNbDelegatePtr->Delete(KEY1), OK);
    EXPECT_EQ(g_kvNbDelegatePtr->DeleteLocal(KEY2), OK);

    /**
     * @tc.steps:step5 Commit a transaction.
     * @tc.expected: step5 Transaction submitted successfully.
     */
    EXPECT_EQ(g_kvNbDelegatePtr->Commit(), OK);

    /**
     * @tc.steps:step6 Get after the transaction is submitted.
     * @tc.expected: step6 Get return NOT_FOUND and the geted data is correct.
     */
    Value getSyncValue;
    EXPECT_EQ(g_kvNbDelegatePtr->Get(KEY1, getSyncValue), NOT_FOUND);
    Value getLocalValue;
    EXPECT_EQ(g_kvNbDelegatePtr->GetLocal(KEY2, getLocalValue), NOT_FOUND);

    /**
     * @tc.steps:step7 Close the kv store.
     * @tc.expected: step7 Results OK and delete successfully.
     */
    EXPECT_EQ(g_mgr.CloseKvStore(g_kvNbDelegatePtr), OK);
    EXPECT_EQ(g_mgr.DeleteKvStore(storeId), OK);
    g_kvNbDelegatePtr = nullptr;
}

/**
  * @tc.name: DeleteBatchInTransaction001
  * @tc.desc: Local data does not check readOnly.
  * @tc.type: FUNC
  * @tc.require: AR000EPAS8
  * @tc.author: changguicai
  */
HWTEST_F(DistributedDBInterfacesTransactionOptimizationTest, DeleteBatchInTransaction001, TestSize.Level1)
{
    /**
     * @tc.steps:step1. Get the nb delegate.
     * @tc.expected: step1. Get results OK and non-null delegate.
     */
    std::string storeId("DeleteBatchInTransaction001");
    KvStoreNbDelegate::Option option = {true, false, false};
    g_mgr.GetKvStore(storeId, option, g_kvNbDelegateCallback);
    ASSERT_TRUE(g_kvNbDelegatePtr != nullptr);
    EXPECT_TRUE(g_kvDelegateStatus == OK);

    /**
     * @tc.steps:step2. Starting a Transaction.
     * @tc.expected: step2. The transaction is started successfully.
     */
    EXPECT_EQ(g_kvNbDelegatePtr->StartTransaction(), OK);

    vector<Entry> entries;
    vector<Key> keys;
    DistributedDBUnitTest::GenerateRecords(BATCH_PRESET_SIZE_TEST, entries, keys);
    EXPECT_TRUE(entries.size() == BATCH_PRESET_SIZE_TEST);

    vector<Entry> localEntrys;
    vector<Key> localKeys;
    DistributedDBUnitTest::GenerateRecords(DIVIDE_BATCH_PRESET_SIZE, localEntrys, localKeys);
    EXPECT_TRUE(localEntrys.size() == DIVIDE_BATCH_PRESET_SIZE);

    /**
     * @tc.steps:step3. Put batch data.
     * @tc.expected: step3. Returns OK.
     */
    EXPECT_EQ(g_kvNbDelegatePtr->PutBatch(entries), OK);
    EXPECT_EQ(g_kvNbDelegatePtr->PutLocalBatch(localEntrys), OK);

    /**
     * @tc.steps:step4 DeleteBatch before the transaction is submitted.
     * @tc.expected: step4. Delete return OK.
     */
    EXPECT_EQ(g_kvNbDelegatePtr->DeleteBatch(keys), OK);
    EXPECT_EQ(g_kvNbDelegatePtr->DeleteLocalBatch(localKeys), OK);

    /**
     * @tc.steps:step5 Commit a transaction.
     * @tc.expected: step5 Transaction submitted successfully.
     */
    EXPECT_EQ(g_kvNbDelegatePtr->Commit(), OK);

    /**
     * @tc.steps:step6 GetEntries after the transaction is submitted.
     * @tc.expected: step6 GetEntries return NOT_FOUND and the geted data is correct.
     */
    Key keyPrefix;
    std::vector<Entry> getSyncEntries;
    EXPECT_EQ(g_kvNbDelegatePtr->GetEntries(keyPrefix, getSyncEntries), NOT_FOUND);
    std::vector<Entry> getLocalEntries;
    EXPECT_EQ(g_kvNbDelegatePtr->GetLocalEntries(keyPrefix, getLocalEntries), NOT_FOUND);

    /**
     * @tc.steps:step7 Close the kv store.
     * @tc.expected: step7 Results OK and delete successfully.
     */
    EXPECT_EQ(g_mgr.CloseKvStore(g_kvNbDelegatePtr), OK);
    EXPECT_EQ(g_mgr.DeleteKvStore(storeId), OK);
    g_kvNbDelegatePtr = nullptr;
}

/**
  * @tc.name: SyncAndLocalObserver001
  * @tc.desc: Verify the observer functions of the sync and local data in the same transaction.
  * @tc.type: FUNC
  * @tc.require: AR000EPAS8
  * @tc.author: changguicai
  */
HWTEST_F(DistributedDBInterfacesTransactionOptimizationTest, SyncAndLocalObserver001, TestSize.Level1)
{
    /**
     * @tc.steps:step1. Get the nb delegate.
     * @tc.expected: step1. Get results OK and non-null delegate.
     */
    std::string storeId("SyncAndLocalObserver001");
    KvStoreNbDelegate::Option option = {true, false, false};
    g_mgr.GetKvStore(storeId, option, g_kvNbDelegateCallback);
    ASSERT_TRUE(g_kvNbDelegatePtr != nullptr);
    EXPECT_TRUE(g_kvDelegateStatus == OK);

    KvStoreObserverUnitTest *syncObserver = new (std::nothrow) KvStoreObserverUnitTest;
    ASSERT_TRUE(syncObserver != nullptr);
    KvStoreObserverUnitTest *localObserver = new (std::nothrow) KvStoreObserverUnitTest;
    ASSERT_TRUE(localObserver != nullptr);

    /**
     * @tc.steps:step2. Register the non-null observer for the special key.
     * @tc.expected: step2. Register results OK.
     */
    Key key;
    EXPECT_EQ(g_kvNbDelegatePtr->RegisterObserver(key, OBSERVER_CHANGES_NATIVE, syncObserver), OK);
    EXPECT_EQ(g_kvNbDelegatePtr->RegisterObserver(key, OBSERVER_CHANGES_LOCAL_ONLY, localObserver), OK);

    /**
     * @tc.steps:step3. Starting a Transaction.
     * @tc.expected: step3. The transaction is started successfully.
     */
    EXPECT_EQ(g_kvNbDelegatePtr->StartTransaction(), OK);

    /**
     * @tc.steps:step4. Put batch data.
     * @tc.expected: step4. Returns OK.
     */
    vector<Key> syncKeys;
    vector<Entry> syncEntries;
    DistributedDBUnitTest::GenerateRecords(BATCH_PRESET_SIZE_TEST, syncEntries, syncKeys);
    EXPECT_TRUE(syncEntries.size() == BATCH_PRESET_SIZE_TEST);
    EXPECT_EQ(g_kvNbDelegatePtr->PutBatch(syncEntries), OK);

    vector<Key> localKeys;
    vector<Entry> localEntries;
    DistributedDBUnitTest::GenerateRecords(DIVIDE_BATCH_PRESET_SIZE, localEntries, localKeys);
    EXPECT_TRUE(localEntries.size() == DIVIDE_BATCH_PRESET_SIZE);
    EXPECT_EQ(g_kvNbDelegatePtr->PutLocalBatch(localEntries), OK);

    /**
     * @tc.steps:step5. Commit a transaction.
     * @tc.expected: step5. Transaction submitted successfully.
     */
    EXPECT_EQ(g_kvNbDelegatePtr->Commit(), OK);

    /**
     * @tc.steps:step6. Check changed data.
     * @tc.expected: step6. The inserted data is the same as the written data.
     */
    std::this_thread::sleep_for(std::chrono::milliseconds(OBSERVER_SLEEP_TIME));
    EXPECT_TRUE(DistributedDBToolsUnitTest::CheckObserverResult(syncEntries, syncObserver->GetEntriesInserted()));
    EXPECT_TRUE(DistributedDBToolsUnitTest::CheckObserverResult(localEntries, localObserver->GetEntriesInserted()));

    /**
     * @tc.steps:step7. UnRegister the observer.
     * @tc.expected: step7. Returns OK.
     */
    EXPECT_EQ(g_kvNbDelegatePtr->UnRegisterObserver(syncObserver), OK);
    delete syncObserver;
    syncObserver = nullptr;
    EXPECT_EQ(g_kvNbDelegatePtr->UnRegisterObserver(localObserver), OK);
    delete localObserver;
    localObserver = nullptr;

    /**
     * @tc.steps:step8. Close the kv store.
     * @tc.expected: step8. Results OK and delete successfully.
     */
    EXPECT_EQ(g_mgr.CloseKvStore(g_kvNbDelegatePtr), OK);
    EXPECT_EQ(g_mgr.DeleteKvStore(storeId), OK);
    g_kvNbDelegatePtr = nullptr;
}

/**
  * @tc.name: OnlyDeleteInTransaction001
  * @tc.desc: Verify the observer functions of delete operation in the transaction.
  * @tc.type: FUNC
  * @tc.require: AR000EPAS8
  * @tc.author: changguicai
  */
HWTEST_F(DistributedDBInterfacesTransactionOptimizationTest, OnlyDeleteInTransaction001, TestSize.Level1)
{
    /**
     * @tc.steps:step1. Get the nb delegate.
     * @tc.expected: step1. Get results OK and non-null delegate.
     */
    std::string storeId("OnlyDeleteInTransaction001");
    KvStoreNbDelegate::Option option = {true, false, false};
    g_mgr.GetKvStore(storeId, option, g_kvNbDelegateCallback);
    ASSERT_TRUE(g_kvNbDelegatePtr != nullptr);
    EXPECT_TRUE(g_kvDelegateStatus == OK);

    /**
     * @tc.steps:step2. Put batch data.
     * @tc.expected: step2. Returns OK.
     */
    vector<Key> syncKeys;
    vector<Entry> syncEntries;
    DistributedDBUnitTest::GenerateRecords(BATCH_PRESET_SIZE_TEST, syncEntries, syncKeys);
    EXPECT_TRUE(syncEntries.size() == BATCH_PRESET_SIZE_TEST);
    EXPECT_EQ(g_kvNbDelegatePtr->PutBatch(syncEntries), OK);

    vector<Key> localKeys;
    vector<Entry> localEntries;
    DistributedDBUnitTest::GenerateRecords(DIVIDE_BATCH_PRESET_SIZE, localEntries, localKeys);
    EXPECT_TRUE(localEntries.size() == DIVIDE_BATCH_PRESET_SIZE);
    EXPECT_EQ(g_kvNbDelegatePtr->PutLocalBatch(localEntries), OK);

    KvStoreObserverUnitTest *syncObserver = new (std::nothrow) KvStoreObserverUnitTest;
    ASSERT_TRUE(syncObserver != nullptr);
    KvStoreObserverUnitTest *localObserver = new (std::nothrow) KvStoreObserverUnitTest;
    ASSERT_TRUE(localObserver != nullptr);

    /**
     * @tc.steps:step3. Register the non-null observer for the special key.
     * @tc.expected: step3. Register results OK.
     */
    Key key;
    EXPECT_EQ(g_kvNbDelegatePtr->RegisterObserver(key, OBSERVER_CHANGES_NATIVE, syncObserver), OK);
    EXPECT_EQ(g_kvNbDelegatePtr->RegisterObserver(key, OBSERVER_CHANGES_LOCAL_ONLY, localObserver), OK);

    /**
     * @tc.steps:step4. Starting a Transaction.
     * @tc.expected: step4. The transaction is started successfully.
     */
    EXPECT_EQ(g_kvNbDelegatePtr->StartTransaction(), OK);

    /**
     * @tc.steps:step5 DeleteBatch before the transaction is submitted.
     * @tc.expected: step5. Delete return OK.
     */
    EXPECT_EQ(g_kvNbDelegatePtr->DeleteBatch(syncKeys), OK);
    EXPECT_EQ(g_kvNbDelegatePtr->DeleteLocalBatch(localKeys), OK);

    /**
     * @tc.steps:step6. Commit a transaction.
     * @tc.expected: step6. Transaction submitted successfully.
     */
    EXPECT_EQ(g_kvNbDelegatePtr->Commit(), OK);

    /**
     * @tc.steps:step7. Check changed data.
     * @tc.expected: step7. The inserted data is the same as the written data.
     */
    std::this_thread::sleep_for(std::chrono::milliseconds(OBSERVER_SLEEP_TIME));
    EXPECT_TRUE(DistributedDBToolsUnitTest::CheckObserverResult(syncEntries, syncObserver->GetEntriesDeleted()));
    EXPECT_TRUE(DistributedDBToolsUnitTest::CheckObserverResult(localEntries, localObserver->GetEntriesDeleted()));

    /**
     * @tc.steps:step8. UnRegister the observer.
     * @tc.expected: step8. Returns OK.
     */
    EXPECT_EQ(g_kvNbDelegatePtr->UnRegisterObserver(syncObserver), OK);
    delete syncObserver;
    syncObserver = nullptr;
    EXPECT_EQ(g_kvNbDelegatePtr->UnRegisterObserver(localObserver), OK);
    delete localObserver;
    localObserver = nullptr;

    /**
     * @tc.steps:step9. Close the kv store.
     * @tc.expected: step9. Results OK and delete successfully.
     */
    EXPECT_EQ(g_mgr.CloseKvStore(g_kvNbDelegatePtr), OK);
    EXPECT_EQ(g_mgr.DeleteKvStore(storeId), OK);
    g_kvNbDelegatePtr = nullptr;
}

/**
  * @tc.name: SyncAndLocalObserver002
  * @tc.desc: Verify the observer functions of the sync and local data in the same transaction.
  * @tc.type: FUNC
  * @tc.require: AR000EPAS8
  * @tc.author: changguicai
  */
HWTEST_F(DistributedDBInterfacesTransactionOptimizationTest, SyncAndLocalObserver002, TestSize.Level1)
{
    /**
     * @tc.steps:step1. Get the nb delegate.
     * @tc.expected: step1. Get results OK and non-null delegate.
     */
    std::string storeId("SyncAndLocalObserver002");
    KvStoreNbDelegate::Option option = {true, false, false};
    g_mgr.GetKvStore(storeId, option, g_kvNbDelegateCallback);
    ASSERT_TRUE(g_kvNbDelegatePtr != nullptr);
    EXPECT_TRUE(g_kvDelegateStatus == OK);

    KvStoreObserverUnitTest *syncObserver = new (std::nothrow) KvStoreObserverUnitTest;
    ASSERT_TRUE(syncObserver != nullptr);
    KvStoreObserverUnitTest *localObserver = new (std::nothrow) KvStoreObserverUnitTest;
    ASSERT_TRUE(localObserver != nullptr);
    /**
     * @tc.steps:step2. Register the non-null observer for the special key.
     * @tc.expected: step2. Register results OK.
     */
    Key key;
    EXPECT_EQ(g_kvNbDelegatePtr->RegisterObserver(key, OBSERVER_CHANGES_NATIVE, syncObserver), OK);
    EXPECT_EQ(g_kvNbDelegatePtr->RegisterObserver(key, OBSERVER_CHANGES_LOCAL_ONLY, localObserver), OK);

    /**
     * @tc.steps:step3. Starting a Transaction.
     * @tc.expected: step3. The transaction is started successfully.
     */
    EXPECT_EQ(g_kvNbDelegatePtr->StartTransaction(), OK);

    /**
     * @tc.steps:step4. Put data.
     * @tc.expected: step4. Returns OK.
     */
    EXPECT_EQ(g_kvNbDelegatePtr->Put(KEY1, VALUE1), OK);
    EXPECT_EQ(g_kvNbDelegatePtr->PutLocal(KEY2, VALUE2), OK);

    /**
     * @tc.steps:step5. Commit a transaction.
     * @tc.expected: step5. Transaction submitted successfully.
     */
    EXPECT_EQ(g_kvNbDelegatePtr->Commit(), OK);

    std::vector<DistributedDB::Entry> syncEntries;
    Entry syncEntry{KEY1, VALUE1};
    syncEntries.emplace_back(syncEntry);
    std::vector<DistributedDB::Entry> localEntries;
    Entry localEntry{KEY2, VALUE2};
    localEntries.emplace_back(localEntry);

    /**
     * @tc.steps:step6. Check changed data.
     * @tc.expected: step6. The inserted data is the same as the written data.
     */
    std::this_thread::sleep_for(std::chrono::milliseconds(OBSERVER_SLEEP_TIME));
    EXPECT_TRUE(DistributedDBToolsUnitTest::CheckObserverResult(syncEntries, syncObserver->GetEntriesInserted()));
    EXPECT_TRUE(DistributedDBToolsUnitTest::CheckObserverResult(localEntries, localObserver->GetEntriesInserted()));

    /**
     * @tc.steps:step7. UnRegister the observer.
     * @tc.expected: step7. Returns OK.
     */
    EXPECT_EQ(g_kvNbDelegatePtr->UnRegisterObserver(syncObserver), OK);
    delete syncObserver;
    syncObserver = nullptr;
    EXPECT_EQ(g_kvNbDelegatePtr->UnRegisterObserver(localObserver), OK);
    delete localObserver;
    localObserver = nullptr;

    /**
     * @tc.steps:step8. Close the kv store.
     * @tc.expected: step8. Results OK and delete successfully.
     */
    EXPECT_EQ(g_mgr.CloseKvStore(g_kvNbDelegatePtr), OK);
    EXPECT_EQ(g_mgr.DeleteKvStore(storeId), OK);
    g_kvNbDelegatePtr = nullptr;
}

/**
  * @tc.name: PutRollback001
  * @tc.desc: Verify that a transaction can be rolled back after data is put.
  * @tc.type: FUNC
  * @tc.require: AR000EPAS8
  * @tc.author: changguicai
  */
HWTEST_F(DistributedDBInterfacesTransactionOptimizationTest, PutRollback001, TestSize.Level1)
{
    /**
     * @tc.steps:step1. Get the nb delegate.
     * @tc.expected: step1. Get results OK and non-null delegate.
     */
    std::string storeId("PutRollback001");
    KvStoreNbDelegate::Option option = {true, false, false};
    g_mgr.GetKvStore(storeId, option, g_kvNbDelegateCallback);
    ASSERT_TRUE(g_kvNbDelegatePtr != nullptr);
    EXPECT_TRUE(g_kvDelegateStatus == OK);

    KvStoreObserverUnitTest *syncObserver = new (std::nothrow) KvStoreObserverUnitTest;
    ASSERT_TRUE(syncObserver != nullptr);
    KvStoreObserverUnitTest *localObserver = new (std::nothrow) KvStoreObserverUnitTest;
    ASSERT_TRUE(localObserver != nullptr);
    /**
     * @tc.steps:step2. Register the non-null observer for the special key.
     * @tc.expected: step2. Register results OK.
     */
    Key key;
    EXPECT_EQ(g_kvNbDelegatePtr->RegisterObserver(key, OBSERVER_CHANGES_NATIVE, syncObserver), OK);
    EXPECT_EQ(g_kvNbDelegatePtr->RegisterObserver(key, OBSERVER_CHANGES_LOCAL_ONLY, localObserver), OK);

    /**
     * @tc.steps:step3. Starting a Transaction.
     * @tc.expected: step3. The transaction is started successfully.
     */
    EXPECT_EQ(g_kvNbDelegatePtr->StartTransaction(), OK);

    /**
     * @tc.steps:step3. Put data.
     * @tc.expected: step3. Returns OK.
     */
    EXPECT_EQ(g_kvNbDelegatePtr->Put(KEY1, VALUE1), OK);
    EXPECT_EQ(g_kvNbDelegatePtr->PutLocal(KEY2, VALUE2), OK);

    /**
     * @tc.steps:step3. Transaction rollback.
     * @tc.expected: step3. Returns OK.
     */
    EXPECT_EQ(g_kvNbDelegatePtr->Rollback(), OK);

    /**
     * @tc.steps:step4. After the rollback, query the database and observe the changed data.
     * @tc.expected: step4. Get return NOT_FOUND. The changed data is empty.
     */
    Value value;
    EXPECT_EQ(g_kvNbDelegatePtr->Get(KEY1, value), NOT_FOUND);
    EXPECT_EQ(g_kvNbDelegatePtr->GetLocal(KEY2, value), NOT_FOUND);

    std::vector<DistributedDB::Entry> empty;
    std::this_thread::sleep_for(std::chrono::milliseconds(OBSERVER_SLEEP_TIME));
    EXPECT_TRUE(DistributedDBToolsUnitTest::CheckObserverResult(empty, syncObserver->GetEntriesInserted()));
    EXPECT_TRUE(DistributedDBToolsUnitTest::CheckObserverResult(empty, localObserver->GetEntriesInserted()));

    /**
     * @tc.steps:step5. UnRegister the observer.
     * @tc.expected: step5. Returns OK.
     */
    EXPECT_EQ(g_kvNbDelegatePtr->UnRegisterObserver(syncObserver), OK);
    delete syncObserver;
    syncObserver = nullptr;
    EXPECT_EQ(g_kvNbDelegatePtr->UnRegisterObserver(localObserver), OK);
    delete localObserver;
    localObserver = nullptr;

    /**
     * @tc.steps:step6. Close the kv store.
     * @tc.expected: step6. Results OK and delete successfully.
     */
    EXPECT_EQ(g_mgr.CloseKvStore(g_kvNbDelegatePtr), OK);
    EXPECT_EQ(g_mgr.DeleteKvStore(storeId), OK);
    g_kvNbDelegatePtr = nullptr;
}

/**
  * @tc.name: PutBatchRollback001
  * @tc.desc: Verify that a transaction can be rolled back after data is put.
  * @tc.type: FUNC
  * @tc.require: AR000EPAS8
  * @tc.author: changguicai
  */
HWTEST_F(DistributedDBInterfacesTransactionOptimizationTest, PutBatchRollback001, TestSize.Level1)
{
    /**
     * @tc.steps:step1. Get the nb delegate.
     * @tc.expected: step1. Get results OK and non-null delegate.
     */
    std::string storeId("OptimizeObserver008");
    KvStoreNbDelegate::Option option = {true, false, false};
    g_mgr.GetKvStore(storeId, option, g_kvNbDelegateCallback);
    ASSERT_TRUE(g_kvNbDelegatePtr != nullptr);
    EXPECT_TRUE(g_kvDelegateStatus == OK);

    KvStoreObserverUnitTest *syncObserver = new (std::nothrow) KvStoreObserverUnitTest;
    ASSERT_TRUE(syncObserver != nullptr);
    KvStoreObserverUnitTest *localObserver = new (std::nothrow) KvStoreObserverUnitTest;
    ASSERT_TRUE(localObserver != nullptr);
    /**
     * @tc.steps:step2. Register the non-null observer for the special key.
     * @tc.expected: step2. Register results OK.
     */
    Key key;
    EXPECT_EQ(g_kvNbDelegatePtr->RegisterObserver(key, OBSERVER_CHANGES_NATIVE, syncObserver), OK);
    EXPECT_EQ(g_kvNbDelegatePtr->RegisterObserver(key, OBSERVER_CHANGES_LOCAL_ONLY, localObserver), OK);

    /**
     * @tc.steps:step3. Starting a Transaction.
     * @tc.expected: step3. The transaction is started successfully.
     */
    EXPECT_EQ(g_kvNbDelegatePtr->StartTransaction(), OK);

    /**
     * @tc.steps:step3. Put batch data.
     * @tc.expected: step3. Returns OK.
     */
    std::vector<Key> syncKeys;
    std::vector<Entry> syncEntries;
    DistributedDBUnitTest::GenerateRecords(BATCH_PRESET_SIZE_TEST, syncEntries, syncKeys);
    EXPECT_TRUE(syncEntries.size() == BATCH_PRESET_SIZE_TEST);
    EXPECT_EQ(g_kvNbDelegatePtr->PutBatch(syncEntries), OK);

    std::vector<Key> localKeys;
    std::vector<Entry> localEntries;
    DistributedDBUnitTest::GenerateRecords(DIVIDE_BATCH_PRESET_SIZE, localEntries, localKeys);
    EXPECT_TRUE(localEntries.size() == DIVIDE_BATCH_PRESET_SIZE);
    EXPECT_EQ(g_kvNbDelegatePtr->PutLocalBatch(localEntries), OK);

    /**
     * @tc.steps:step3. Transaction rollback.
     * @tc.expected: step3. Returns OK.
     */
    EXPECT_EQ(g_kvNbDelegatePtr->Rollback(), OK);

    /**
     * @tc.steps:step4. After the rollback, query the database and observe the changed data.
     * @tc.expected: step4. Get return NOT_FOUND. The changed data is empty.
     */
    Key keyPrefix;
    std::vector<Entry> entries;
    EXPECT_EQ(g_kvNbDelegatePtr->GetEntries(keyPrefix, entries), NOT_FOUND);
    EXPECT_EQ(g_kvNbDelegatePtr->GetLocalEntries(keyPrefix, entries), NOT_FOUND);

    std::this_thread::sleep_for(std::chrono::milliseconds(OBSERVER_SLEEP_TIME));
    EXPECT_TRUE(DistributedDBToolsUnitTest::CheckObserverResult(entries, syncObserver->GetEntriesInserted()));
    EXPECT_TRUE(DistributedDBToolsUnitTest::CheckObserverResult(entries, localObserver->GetEntriesInserted()));

    /**
     * @tc.steps:step5. UnRegister the observer.
     * @tc.expected: step5. Returns OK.
     */
    EXPECT_EQ(g_kvNbDelegatePtr->UnRegisterObserver(syncObserver), OK);
    delete syncObserver;
    syncObserver = nullptr;
    EXPECT_EQ(g_kvNbDelegatePtr->UnRegisterObserver(localObserver), OK);
    delete localObserver;
    localObserver = nullptr;

    /**
     * @tc.steps:step6. Close the kv store.
     * @tc.expected: step6. Results OK and delete successfully.
     */
    EXPECT_EQ(g_mgr.CloseKvStore(g_kvNbDelegatePtr), OK);
    EXPECT_EQ(g_mgr.DeleteKvStore(storeId), OK);
    g_kvNbDelegatePtr = nullptr;
}

