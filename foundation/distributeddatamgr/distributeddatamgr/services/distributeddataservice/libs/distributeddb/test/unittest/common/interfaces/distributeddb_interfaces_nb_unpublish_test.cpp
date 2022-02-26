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
    KvStoreObserverUnitTest *g_syncObserver = nullptr;
    KvStoreObserverUnitTest *g_localObserver = nullptr;
    Entry g_entry1;
    Entry g_entry2;

    // define the g_kvNbDelegateCallback, used to get some information when open a kv store.
    DBStatus g_kvDelegateStatus = INVALID_ARGS;
    KvStoreNbDelegate *g_kvNbDelegatePtr = nullptr;

    const int OBSERVER_SLEEP_TIME = 100;

    void KvStoreNbDelegateCallback(
        DBStatus statusSrc, KvStoreNbDelegate *kvStoreSrc, DBStatus &statusDst, KvStoreNbDelegate *&kvStoreDst)
    {
        statusDst = statusSrc;
        kvStoreDst = kvStoreSrc;
    }

    // the type of g_kvNbDelegateCallback is function<void(DBStatus, KvStoreDelegate*)>
    auto g_kvNbDelegateCallback = bind(&KvStoreNbDelegateCallback, placeholders::_1,
        placeholders::_2, std::ref(g_kvDelegateStatus), std::ref(g_kvNbDelegatePtr));

    static void CheckUnpublishNotFound()
    {
        Value valueRead;
        EXPECT_EQ(g_kvNbDelegatePtr->Get(g_entry1.key, valueRead), NOT_FOUND);
        EXPECT_EQ(g_kvNbDelegatePtr->Get(g_entry2.key, valueRead), OK);
        EXPECT_EQ(g_entry2.value, valueRead);
        EXPECT_EQ(g_kvNbDelegatePtr->GetLocal(g_entry1.key, valueRead), OK);
        EXPECT_EQ(g_entry1.value, valueRead);
        EXPECT_EQ(g_kvNbDelegatePtr->GetLocal(g_entry2.key, valueRead), NOT_FOUND);
    }

    static void RegisterObservers()
    {
        g_localObserver = new (std::nothrow) KvStoreObserverUnitTest;
        ASSERT_TRUE(g_localObserver != nullptr);
        g_syncObserver = new (std::nothrow) KvStoreObserverUnitTest;
        ASSERT_TRUE(g_syncObserver != nullptr);
        Key key;
        EXPECT_EQ(g_kvNbDelegatePtr->RegisterObserver(key, 3, g_syncObserver), OK); // sync data observer.
        EXPECT_EQ(g_kvNbDelegatePtr->RegisterObserver(key, 4, g_localObserver), OK); // local data observer.
    }
}

class DistributedDBInterfacesNBUnpublishTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
};

void DistributedDBInterfacesNBUnpublishTest::SetUpTestCase(void)
{
    DistributedDBToolsUnitTest::TestDirInit(g_testDir);
    g_config.dataDir = g_testDir;
    g_mgr.SetKvStoreConfig(g_config);
}

void DistributedDBInterfacesNBUnpublishTest::TearDownTestCase(void)
{
    if (DistributedDBToolsUnitTest::RemoveTestDbFiles(g_testDir) != 0) {
        LOGE("rm test db files error!");
    }
}

void DistributedDBInterfacesNBUnpublishTest::SetUp(void)
{
    KvStoreNbDelegate::Option option = {true, false, false};
    g_mgr.GetKvStore("unpublish_test", option, g_kvNbDelegateCallback);
    ASSERT_TRUE(g_kvNbDelegatePtr != nullptr);
    EXPECT_TRUE(g_kvDelegateStatus == OK);
    DistributedDBToolsUnitTest::GetRandomKeyValue(g_entry1.key);
    DistributedDBToolsUnitTest::GetRandomKeyValue(g_entry1.value);
    DistributedDBToolsUnitTest::GetRandomKeyValue(g_entry2.key);
    DistributedDBToolsUnitTest::GetRandomKeyValue(g_entry2.value);
}

void DistributedDBInterfacesNBUnpublishTest::TearDown(void)
{
    if (g_localObserver != nullptr) {
        if (g_kvNbDelegatePtr != nullptr) {
            g_kvNbDelegatePtr->UnRegisterObserver(g_localObserver);
        }
        delete g_localObserver;
        g_localObserver = nullptr;
    }

    if (g_syncObserver != nullptr) {
        if (g_kvNbDelegatePtr != nullptr) {
            g_kvNbDelegatePtr->UnRegisterObserver(g_syncObserver);
        }
        delete g_syncObserver;
        g_syncObserver = nullptr;
    }
    if (g_kvNbDelegatePtr != nullptr) {
        g_mgr.CloseKvStore(g_kvNbDelegatePtr);
        g_kvNbDelegatePtr = nullptr;
    }
    g_mgr.DeleteKvStore("unpublish_test");
}

/**
  * @tc.name: CombineTest001
  * @tc.desc: Test unpublish one nonexistent data.
  * @tc.type: FUNC
  * @tc.require: AR000DPTQ5
  * @tc.author: wangbingquan
  */
HWTEST_F(DistributedDBInterfacesNBUnpublishTest, SingleVerUnPublishKey001, TestSize.Level1)
{
    /**
     * @tc.steps:step1. Put [k1, v1] into the local zone, [k2, v2] into the sync zone.
     * @tc.expected: step1. Get results OK.
     */
    EXPECT_EQ(g_kvNbDelegatePtr->PutLocal(g_entry1.key, g_entry1.value), OK);
    EXPECT_EQ(g_kvNbDelegatePtr->Put(g_entry2.key, g_entry2.value), OK);

    /**
     * @tc.steps:step2. Unpublish the k1 with para of deletePublic: true, updateTimestamp: true.
     * @tc.expected: step2. Unpublish returns NOT_FOUND.
     */
    EXPECT_EQ(g_kvNbDelegatePtr->UnpublishToLocal(g_entry1.key, true, true), NOT_FOUND);

    /**
     * @tc.steps:step3. Check the data in the local and sync zone.
     * @tc.expected: step3. Both the data are not changed.
     */
    CheckUnpublishNotFound();

    /**
     * @tc.steps:step4. Unpublish the k1 with para of deletePublic: true, updateTimestamp: false.
     * @tc.expected: step4. Unpublish returns NOT_FOUND.
     */
    EXPECT_EQ(g_kvNbDelegatePtr->UnpublishToLocal(g_entry1.key, true, false), NOT_FOUND);

    /**
     * @tc.steps:step5. Check the data in the local and sync zone.
     * @tc.expected: step5. Both the data are not changed.
     */
    CheckUnpublishNotFound();

    /**
     * @tc.steps:step6. Unpublish the k1 with para of deletePublic: false, updateTimestamp: true.
     * @tc.expected: step6. Unpublish returns NOT_FOUND.
     */
    EXPECT_EQ(g_kvNbDelegatePtr->UnpublishToLocal(g_entry1.key, false, true), NOT_FOUND);

    /**
     * @tc.steps:step7. Check the data in the local and sync zone.
     * @tc.expected: step7. Both the data are not changed.
     */
    CheckUnpublishNotFound();

    /**
     * @tc.steps:step8. Unpublish the k1 with para of deletePublic: false, updateTimestamp: false.
     * @tc.expected: step8. Unpublish returns NOT_FOUND.
     */
    EXPECT_EQ(g_kvNbDelegatePtr->UnpublishToLocal(g_entry1.key, false, false), NOT_FOUND);

    /**
     * @tc.steps:step9. Check the data in the local and sync zone.
     * @tc.expected: step9. Both the data are not changed.
     */
    CheckUnpublishNotFound();
}

/**
  * @tc.name: SingleVerUnPublishKey002
  * @tc.desc: Test unpublish existent data(no conflict with the local data).
  * @tc.type: FUNC
  * @tc.require: AR000DPTQ5
  * @tc.author: wangbingquan
  */
HWTEST_F(DistributedDBInterfacesNBUnpublishTest, SingleVerUnPublishKey002, TestSize.Level1)
{
    /**
     * @tc.steps:step1. Put [k1, v1], [k2, v2] into the sync zone.
     * @tc.expected: step1. Put returns OK.
     */
    EXPECT_EQ(g_kvNbDelegatePtr->Put(g_entry1.key, g_entry1.value), OK);
    EXPECT_EQ(g_kvNbDelegatePtr->Put(g_entry2.key, g_entry2.value), OK);

    /**
     * @tc.steps:step2. Register the obsevers for the sync data and the local data.
     */
    RegisterObservers();

    /**
     * @tc.steps:step3. Unpublish the k1 with para of deletePublic: false, updateTimestamp: false.
     * @tc.expected: step3. Unpublish returns OK.
     */
    EXPECT_EQ(g_kvNbDelegatePtr->UnpublishToLocal(g_entry1.key, false, false), OK);

    /**
     * @tc.steps:step4. Get the value of k1 from the local zone.
     * @tc.expected: step4. Get returns OK, and the value is v1.
     */
    Value valueRead;
    EXPECT_EQ(g_kvNbDelegatePtr->Get(g_entry1.key, valueRead), OK);
    EXPECT_EQ(g_entry1.value, valueRead);
    EXPECT_EQ(g_kvNbDelegatePtr->Get(g_entry2.key, valueRead), OK);
    EXPECT_EQ(g_entry2.value, valueRead);
    EXPECT_EQ(g_kvNbDelegatePtr->GetLocal(g_entry1.key, valueRead), OK);
    EXPECT_EQ(g_entry1.value, valueRead);
    EXPECT_EQ(g_kvNbDelegatePtr->GetLocal(g_entry2.key, valueRead), NOT_FOUND);

    /**
     * @tc.steps:step5. Check the observer.
     * @tc.expected: step5. local observer received one inserted data.
     */
    std::this_thread::sleep_for(std::chrono::milliseconds(OBSERVER_SLEEP_TIME));
    EXPECT_EQ(g_syncObserver->GetCallCount(), 0UL);
    EXPECT_EQ(g_localObserver->GetCallCount(), 1UL);
    EXPECT_EQ(g_localObserver->GetEntriesInserted().size(), 1UL);

    g_localObserver->ResetToZero();
    g_syncObserver->ResetToZero();

    /**
     * @tc.steps:step6. Unpublish the k2 with para of deletePublic: true, updateTimestamp: false.
     * @tc.expected: step6. Unpublish returns OK.
     */
    EXPECT_EQ(g_kvNbDelegatePtr->UnpublishToLocal(g_entry2.key, true, false), OK);

    /**
     * @tc.steps:step7. Get the value of k2 from the local zone and the sync zone.
     * @tc.expected: step7. GetLocal returns OK, and the value is equal to v2. Get returns NOT_FOUND.
     */
    EXPECT_EQ(g_kvNbDelegatePtr->Get(g_entry2.key, valueRead), NOT_FOUND);
    EXPECT_EQ(g_kvNbDelegatePtr->GetLocal(g_entry2.key, valueRead), OK);
    EXPECT_EQ(g_entry2.value, valueRead);

    /**
     * @tc.steps:step8. Check the observer.
     * @tc.expected: step8. Sync observer received 1 delete data, and the local observer received one inserted data.
     */
    std::this_thread::sleep_for(std::chrono::milliseconds(OBSERVER_SLEEP_TIME));
    EXPECT_EQ(g_syncObserver->GetCallCount(), 1UL);
    EXPECT_EQ(g_localObserver->GetCallCount(), 1UL);
    EXPECT_EQ(g_localObserver->GetEntriesInserted().size(), 1UL);
    EXPECT_EQ(g_syncObserver->GetEntriesDeleted().size(), 1UL);
}

/**
  * @tc.name: SingleVerUnPublishKey003
  * @tc.desc: Test unpublish one existent data(conflict with the local data).
  * @tc.type: FUNC
  * @tc.require: AR000DPTQ5
  * @tc.author: wangbingquan
  */
HWTEST_F(DistributedDBInterfacesNBUnpublishTest, SingleVerUnPublishKey003, TestSize.Level1)
{
    Value value3;
    Value value4;
    DistributedDBToolsUnitTest::GetRandomKeyValue(value3);
    DistributedDBToolsUnitTest::GetRandomKeyValue(value4);

    /**
     * @tc.steps:step1. Put [k1, v1] into the sync zone, [k1, v3][k2, v2] into the local zone,
     *     and put the [k2, v4] into the sync zone.
     * @tc.expected: step1. Put returns OK.
     */
    EXPECT_EQ(g_kvNbDelegatePtr->Put(g_entry1.key, g_entry1.value), OK);
    std::this_thread::sleep_for(std::chrono::milliseconds(OBSERVER_SLEEP_TIME));
    EXPECT_EQ(g_kvNbDelegatePtr->PutLocal(g_entry1.key, value3), OK);
    EXPECT_EQ(g_kvNbDelegatePtr->PutLocal(g_entry2.key, g_entry2.value), OK);
    std::this_thread::sleep_for(std::chrono::milliseconds(OBSERVER_SLEEP_TIME));
    EXPECT_EQ(g_kvNbDelegatePtr->Put(g_entry2.key, value4), OK);

    /**
     * @tc.steps:step2. Register the obsevers for the sync data and the local data.
     */
    RegisterObservers();

    /**
     * @tc.steps:step3. Unpublish the k2 with para of deletePublic: false, updateTimestamp: false.
     * @tc.expected: step3. Unpublish returns LOCAL_DEFEAT.
     */
    EXPECT_EQ(g_kvNbDelegatePtr->UnpublishToLocal(g_entry1.key, false, false), LOCAL_DEFEAT);
    Value valueRead;

    /**
     * @tc.steps:step4. Check the data of k1 in the local zone and the observer changes.
     * @tc.expected: step4. Value of k1 is v3, and the observer has no change.
     */
    EXPECT_EQ(g_kvNbDelegatePtr->GetLocal(g_entry1.key, valueRead), OK);
    EXPECT_EQ(valueRead, value3);
    std::this_thread::sleep_for(std::chrono::milliseconds(OBSERVER_SLEEP_TIME));
    EXPECT_EQ(g_localObserver->GetCallCount(), 0UL);

    /**
     * @tc.steps:step5. Unpublish the k1 with para of deletePublic: false, updateTimestamp: true.
     * @tc.expected: step5. Unpublish returns LOCAL_COVERED.
     */
    EXPECT_EQ(g_kvNbDelegatePtr->UnpublishToLocal(g_entry1.key, false, true), LOCAL_COVERED);

    /**
     * @tc.steps:step6. Check the data of k1 in the local zone and the observer changes.
     * @tc.expected: step6. Value of k1 is v1, and the observer received one updated data.
     */
    EXPECT_EQ(g_kvNbDelegatePtr->GetLocal(g_entry1.key, valueRead), OK);
    EXPECT_EQ(valueRead, g_entry1.value);
    std::this_thread::sleep_for(std::chrono::milliseconds(OBSERVER_SLEEP_TIME));
    EXPECT_EQ(g_localObserver->GetCallCount(), 1UL);
    EXPECT_EQ(g_localObserver->GetEntriesUpdated().size(), 1UL);
    g_localObserver->ResetToZero();

    /**
     * @tc.steps:step7. Unpublish the k2 with para of deletePublic: false, updateTimestamp: false.
     * @tc.expected: step7. Unpublish returns LOCAL_COVERED.
     */
    EXPECT_EQ(g_kvNbDelegatePtr->UnpublishToLocal(g_entry2.key, false, false), LOCAL_COVERED);

    /**
     * @tc.steps:step8. Check the data of k2 in the local zone and the observer changes.
     * @tc.expected: step8. Value of k2 is v2, and the observer received one updated data.
     */
    EXPECT_EQ(g_kvNbDelegatePtr->GetLocal(g_entry2.key, valueRead), OK);
    EXPECT_EQ(valueRead, value4);
    std::this_thread::sleep_for(std::chrono::milliseconds(OBSERVER_SLEEP_TIME));
    EXPECT_EQ(g_localObserver->GetCallCount(), 1UL);
    EXPECT_EQ(g_localObserver->GetEntriesUpdated().size(), 1UL);
}

/**
  * @tc.name: SingleVerUnPublishKey004
  * @tc.desc: Test unpublish one deleted data(no conflict with the local data).
  * @tc.type: FUNC
  * @tc.require: AR000DPTQ5
  * @tc.author: wangbingquan
  */
HWTEST_F(DistributedDBInterfacesNBUnpublishTest, SingleVerUnPublishKey004, TestSize.Level1)
{
    /**
     * @tc.steps:step1. Put [k1, v1] into the sync zone, [k2, v2] into the local zone, delete k1 from sync zone.
     * @tc.expected: step1. Put returns OK.
     */
    EXPECT_EQ(g_kvNbDelegatePtr->Put(g_entry1.key, g_entry1.value), OK);
    std::this_thread::sleep_for(std::chrono::milliseconds(OBSERVER_SLEEP_TIME));
    EXPECT_EQ(g_kvNbDelegatePtr->PutLocal(g_entry2.key, g_entry2.value), OK);
    std::this_thread::sleep_for(std::chrono::milliseconds(OBSERVER_SLEEP_TIME));
    EXPECT_EQ(g_kvNbDelegatePtr->Delete(g_entry1.key), OK);
    RegisterObservers();

    /**
     * @tc.steps:step2. Unpublish the k1 with para of deletePublic: false, updateTimestamp: false.
     * @tc.expected: step2. Unpublish returns OK.
     */
    EXPECT_EQ(g_kvNbDelegatePtr->UnpublishToLocal(g_entry1.key, false, false), OK);

    /**
     * @tc.steps:step3. Check the observer and the data change.
     * @tc.expected: step3. Observer have no changes.
     */
    std::this_thread::sleep_for(std::chrono::milliseconds(OBSERVER_SLEEP_TIME));
    EXPECT_EQ(g_localObserver->GetCallCount(), 0UL);
    EXPECT_EQ(g_syncObserver->GetCallCount(), 0UL);
    Value valueRead;
    EXPECT_EQ(g_kvNbDelegatePtr->GetLocal(g_entry1.key, valueRead), NOT_FOUND);
    EXPECT_EQ(g_kvNbDelegatePtr->GetLocal(g_entry2.key, valueRead), OK);
    EXPECT_EQ(valueRead, g_entry2.value);
}

/**
  * @tc.name: SingleVerUnPublishKey005
  * @tc.desc: Test unpublish one existent data(conflict with the local data).
  * @tc.type: FUNC
  * @tc.require: AR000DPTQ5
  * @tc.author: wangbingquan
  */
HWTEST_F(DistributedDBInterfacesNBUnpublishTest, SingleVerUnPublishKey005, TestSize.Level1)
{
    Value value3;
    Value value4;
    DistributedDBToolsUnitTest::GetRandomKeyValue(value3);
    DistributedDBToolsUnitTest::GetRandomKeyValue(value4);

    /**
     * @tc.steps:step1. Put [k1, v1] [k2, v2]into the sync zone, and delete the k1 from sync zone.
     */
    EXPECT_EQ(g_kvNbDelegatePtr->Put(g_entry1.key, g_entry1.value), OK);
    EXPECT_EQ(g_kvNbDelegatePtr->Put(g_entry2.key, g_entry2.value), OK);
    EXPECT_EQ(g_kvNbDelegatePtr->Delete(g_entry1.key), OK);
    std::this_thread::sleep_for(std::chrono::milliseconds(OBSERVER_SLEEP_TIME));

    /**
     * @tc.steps:step2. Put [k1, v3] [k2, v4]into the local zone, and delete the k2 from sync zone.
     */
    EXPECT_EQ(g_kvNbDelegatePtr->PutLocal(g_entry1.key, value3), OK);
    EXPECT_EQ(g_kvNbDelegatePtr->PutLocal(g_entry2.key, value4), OK);
    std::this_thread::sleep_for(std::chrono::milliseconds(OBSERVER_SLEEP_TIME));
    EXPECT_EQ(g_kvNbDelegatePtr->Delete(g_entry2.key), OK);

    /**
     * @tc.steps:step2. Register the obsevers for the sync data and the local data.
     */
    RegisterObservers();

    /**
     * @tc.steps:step3. Unpublish the k1 with para of deletePublic: false, updateTimestamp: false.
     * @tc.expected: step3. Unpublish returns LOCAL_DEFEAT.
     */
    EXPECT_EQ(g_kvNbDelegatePtr->UnpublishToLocal(g_entry1.key, false, false), LOCAL_DEFEAT);

    /**
     * @tc.steps:step4. Check the value of k1 in the local zone and the observer changes.
     * @tc.expected: step4. value of k1 is still v3, and the observer has no changes.
     */
    Value valueRead;
    EXPECT_EQ(g_kvNbDelegatePtr->GetLocal(g_entry1.key, valueRead), OK);
    EXPECT_EQ(valueRead, value3);
    std::this_thread::sleep_for(std::chrono::milliseconds(OBSERVER_SLEEP_TIME));
    EXPECT_EQ(g_localObserver->GetCallCount(), 0UL);
    EXPECT_EQ(g_syncObserver->GetCallCount(), 0UL);

    /**
     * @tc.steps:step5. Unpublish the k2 with para of deletePublic: false, updateTimestamp: false.
     * @tc.expected: step5. Unpublish returns LOCAL_DELETED.
     */
    EXPECT_EQ(g_kvNbDelegatePtr->UnpublishToLocal(g_entry2.key, false, false), LOCAL_DELETED);

    /**
     * @tc.steps:step6. Check the value of k2 in the local zone and the observer changes.
     * @tc.expected: step6. value of k2 is not found, and the local observer has one deleted data.
     */
    EXPECT_EQ(g_kvNbDelegatePtr->GetLocal(g_entry2.key, valueRead), NOT_FOUND);
    std::this_thread::sleep_for(std::chrono::milliseconds(OBSERVER_SLEEP_TIME));
    EXPECT_EQ(g_localObserver->GetCallCount(), 1UL);
    EXPECT_EQ(g_localObserver->GetEntriesDeleted().size(), 1UL);
    g_localObserver->ResetToZero();

    /**
     * @tc.steps:step7. Unpublish the k1 with para of deletePublic: false, updateTimestamp: true.
     * @tc.expected: step7. Unpublish returns LOCAL_DELETED.
     */
    EXPECT_EQ(g_kvNbDelegatePtr->UnpublishToLocal(g_entry1.key, false, true), LOCAL_DELETED);

    /**
     * @tc.steps:step8. Check the value of k1 in the local zone and the observer changes.
     * @tc.expected: step8. value of k1 is not found, and the local observer has one deleted data.
     */
    EXPECT_EQ(g_kvNbDelegatePtr->GetLocal(g_entry1.key, valueRead), NOT_FOUND);
    std::this_thread::sleep_for(std::chrono::milliseconds(OBSERVER_SLEEP_TIME));
    EXPECT_EQ(g_localObserver->GetCallCount(), 1UL);
    EXPECT_EQ(g_localObserver->GetEntriesDeleted().size(), 1UL);
    g_localObserver->ResetToZero();
}
