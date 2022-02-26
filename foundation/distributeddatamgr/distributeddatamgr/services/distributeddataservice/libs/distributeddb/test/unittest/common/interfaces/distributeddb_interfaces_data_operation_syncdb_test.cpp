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

#include <cstdlib>
#include <ctime>
#include <thread>

#include "db_common.h"
#include "db_errno.h"
#include "distributeddb_data_generate_unit_test.h"
#include "distributeddb_tools_unit_test.h"
#include "sqlite_single_ver_natural_store.h"

using namespace testing::ext;
using namespace DistributedDB;
using namespace DistributedDBUnitTest;
using namespace std;

namespace {
    string g_testDir;
    const bool LOCAL_ONLY = false;
    const string STORE_ID = STORE_ID_SYNC;
    const int OBSERVER_SLEEP_TIME = 100;

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

    // define the g_valueCallback, used to query a value object data from the kvdb.
    DBStatus g_valueStatus = INVALID_ARGS;
    Value g_value;
    // the type of g_valueCallback is function<void(DBStatus, Value)>
    auto g_valueCallback = bind(&DistributedDBToolsUnitTest::ValueCallback,
        placeholders::_1, placeholders::_2, std::ref(g_valueStatus), std::ref(g_value));

    // define the g_entryVectorCallback, used to query a vector<Entry> object data from the kvdb.
    DBStatus g_entryVectorStatus = INVALID_ARGS;
    unsigned long g_matchSize = 0;
    std::vector<Entry> g_entriesVector;
    // the type of g_entryVectorCallback is function<void(DBStatus, vector<Entry>)>
    auto g_entryVectorCallback = bind(&DistributedDBToolsUnitTest::EntryVectorCallback, placeholders::_1,
        placeholders::_2, std::ref(g_entryVectorStatus), std::ref(g_matchSize), std::ref(g_entriesVector));

    const uint32_t MAX_KEY_SIZE = 1024;
    const uint32_t MAX_VAL_SIZE = 4194304;
    const uint32_t INVALID_KEY_SIZE = 1025;

    Entry g_entryA;
    Entry g_entryB;
    Entry g_entryC;
    Entry g_entryD;

    void GetSnapshotUnitTest()
    {
        g_kvDelegatePtr->GetKvStoreSnapshot(nullptr, g_snapshotDelegateCallback);
        EXPECT_TRUE(g_snapshotDelegateStatus == OK);
        ASSERT_TRUE(g_snapshotDelegatePtr != nullptr);
    }
}

class DistributedDBInterfacesDataOperationSyncDBTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
};

void DistributedDBInterfacesDataOperationSyncDBTest::SetUpTestCase(void)
{
    DistributedDBToolsUnitTest::TestDirInit(g_testDir);
    g_config.dataDir = g_testDir;
    g_mgr.SetKvStoreConfig(g_config);
}

void DistributedDBInterfacesDataOperationSyncDBTest::TearDownTestCase(void)
{
    if (DistributedDBToolsUnitTest::RemoveTestDbFiles(g_testDir) != 0) {
        LOGE("rm test db files error!");
    }
}

void DistributedDBInterfacesDataOperationSyncDBTest::SetUp(void)
{
    // init values.
    g_valueStatus = INVALID_ARGS;
    g_value.clear();
    g_entryVectorStatus = INVALID_ARGS;
    g_matchSize = 0;

    /*
     * Here, we create STORE_ID.db before test,
     * and it will be closed in TearDown().
     */
    CipherPassword passwd;
    KvStoreDelegate::Option option = {true, LOCAL_ONLY, false, CipherType::DEFAULT, passwd};
    g_mgr.GetKvStore(STORE_ID, option, g_kvDelegateCallback);
    EXPECT_TRUE(g_kvDelegateStatus == OK);
    ASSERT_TRUE(g_kvDelegatePtr != nullptr);
}

void DistributedDBInterfacesDataOperationSyncDBTest::TearDown(void)
{
    if (g_kvDelegatePtr != nullptr && g_snapshotDelegatePtr != nullptr) {
        EXPECT_TRUE(g_kvDelegatePtr->ReleaseKvStoreSnapshot(g_snapshotDelegatePtr) == OK);
        g_snapshotDelegatePtr = nullptr;
    }

    if (g_kvDelegatePtr != nullptr) {
        EXPECT_EQ(g_mgr.CloseKvStore(g_kvDelegatePtr), OK);
        g_kvDelegatePtr = nullptr;
    }

    EXPECT_EQ(g_mgr.DeleteKvStore(STORE_ID), OK);
}

/**
  * @tc.name: Put001
  * @tc.desc: Put a data(non-empty key, non-empty value) into an exist distributed db
  * @tc.type: FUNC
  * @tc.require: AR000CQDTM AR000CQS3R
  * @tc.author: huangnaigu
  */
HWTEST_F(DistributedDBInterfacesDataOperationSyncDBTest, Put001, TestSize.Level1)
{
    /**
     * @tc.steps: step1. Put the data(non-empty key and non-empty value) into the database.
     * @tc.expected: step1. Put returns OK.
     */
    Key keyTmp;
    keyTmp.push_back(1);
    Value valueTmp;
    valueTmp.push_back('7');
    EXPECT_TRUE(g_kvDelegatePtr->Put(keyTmp, valueTmp) == OK);
    /**
     * @tc.steps: step2. Get the value according the key through the snapshot.
     * @tc.expected: step2. Get returns OK.
     */
    GetSnapshotUnitTest();
    g_snapshotDelegatePtr->Get(keyTmp, g_valueCallback);
    EXPECT_TRUE(g_valueStatus == OK);
}

/**
  * @tc.name: Put002
  * @tc.desc: Put a data(empty key) into an exist distributed db
  * @tc.type: FUNC
  * @tc.require: AR000C6TRV AR000CQDTM
  * @tc.author: huangnaigu
  */
HWTEST_F(DistributedDBInterfacesDataOperationSyncDBTest, Put002, TestSize.Level1)
{
    /**
     * @tc.steps: step1. Put the data(empty key) into the database.
     * @tc.expected: step1. Put returns INVALID_ARGS.
     */
    Key keyTmp;
    Value valueTmp;
    valueTmp.push_back('7');
    EXPECT_TRUE(g_kvDelegatePtr->Put(keyTmp, valueTmp) == INVALID_ARGS);
}

/**
  * @tc.name: Put003
  * @tc.desc: Put a data(non-empty key, empty value) into an exist distributed db
  * @tc.type: FUNC
  * @tc.require: AR000C6TRV AR000CQDTM
  * @tc.author: huangnaigu
  */
HWTEST_F(DistributedDBInterfacesDataOperationSyncDBTest, Put003, TestSize.Level1)
{
    /**
     * @tc.steps: step1. Put the data(empty value) into the database.
     * @tc.expected: step1. Put returns OK.
     */
    Key keyTmp;
    keyTmp.push_back(1);
    Value valueTmp;

    EXPECT_TRUE(g_kvDelegatePtr->Put(keyTmp, valueTmp) == OK);
}

/**
  * @tc.name: Put004
  * @tc.desc: Put data into the multiversion database
  * @tc.type: FUNC
  * @tc.require: AR000C6TRV AR000CQDTM
  * @tc.author: huangnaigu
  */
HWTEST_F(DistributedDBInterfacesDataOperationSyncDBTest, Put004, TestSize.Level1)
{
    /**
     * @tc.steps: step1. clear the database.
     */
    EXPECT_TRUE(g_kvDelegatePtr->Clear() == OK);

    Key keyTmp;
    keyTmp.push_back(1);

    Value valueTmp;
    valueTmp.push_back('7');

    /**
     * @tc.steps: step2. Put one data into the database.
     * @tc.expected: step2. Put returns OK.
     */
    Value valueTest;
    valueTest.push_back('9');
    EXPECT_TRUE(g_kvDelegatePtr->Put(keyTmp, valueTmp) == OK);

    /**
     * @tc.steps: step3. Get the data from the database.
     * @tc.expected: step3. Get returns OK and the read value is equal to the value put before.
     */
    GetSnapshotUnitTest();
    g_snapshotDelegatePtr->Get(keyTmp, g_valueCallback);
    EXPECT_TRUE(g_valueStatus == OK);
    EXPECT_TRUE(g_value.size() > 0);
    if (g_value.size() > 0) {
        EXPECT_TRUE(g_value.front() == '7');
    }

    /**
     * @tc.steps: step4. Change the value, and Put the data into the database.
     * @tc.expected: step4. Put returns OK.
     */
    EXPECT_TRUE(g_kvDelegatePtr->Put(keyTmp, valueTest) == OK);

    if (g_kvDelegatePtr != nullptr && g_snapshotDelegatePtr != nullptr) {
        EXPECT_TRUE(g_kvDelegatePtr->ReleaseKvStoreSnapshot(g_snapshotDelegatePtr) == OK);
        g_snapshotDelegatePtr = nullptr;
    }
    GetSnapshotUnitTest();
    /**
     * @tc.steps: step5. Get the data from the database.
     * @tc.expected: step5. Get returns OK and the read value is equal to the new put value.
     */
    g_snapshotDelegatePtr->Get(keyTmp, g_valueCallback);
    EXPECT_TRUE(g_valueStatus == OK);
    EXPECT_TRUE(g_value.size() > 0);
    if (g_value.size() > 0) {
        EXPECT_TRUE(g_value.front() == '9');
    }
}

/**
  * @tc.name: Clear001
  * @tc.desc: Clear the data from an exist distributed db
  * @tc.type: FUNC
  * @tc.require: AR000CQDTM AR000CQS3R
  * @tc.author: huangnaigu
  */
HWTEST_F(DistributedDBInterfacesDataOperationSyncDBTest, Clear001, TestSize.Level1)
{
    /**
     * @tc.steps: step1. Put the valid data into the database.
     */
    Key keyTmp;
    DistributedDBToolsUnitTest::GetRandomKeyValue(keyTmp);
    Value valueTmp;
    DistributedDBToolsUnitTest::GetRandomKeyValue(valueTmp);
    EXPECT_TRUE(g_kvDelegatePtr->Put(keyTmp, valueTmp) == OK);
    /**
     * @tc.steps: step2. Clear the database.
     * @tc.expected: step2. Clear returns OK.
     */
    EXPECT_TRUE(g_kvDelegatePtr->Clear() == OK);
    /**
     * @tc.steps: step3. Get the data from the database according the inserted key before clear.
     * @tc.expected: step3. Get returns NOT_FOUND.
     */
    Key key;
    GetSnapshotUnitTest();
    g_snapshotDelegatePtr->Get(keyTmp, g_valueCallback);
    EXPECT_EQ(g_valueStatus, NOT_FOUND);
}

/**
  * @tc.name: PutBatch001
  * @tc.desc: Putbatch data into the multiversion database
  * @tc.type: FUNC
  * @tc.require: AR000CQDTM AR000CQS3R
  * @tc.author: huangnaigu
  */
HWTEST_F(DistributedDBInterfacesDataOperationSyncDBTest, PutBatch001, TestSize.Level1)
{
    /**
     * @tc.steps: step1. Put the prepared data.
     */
    vector<Entry> entries;
    for (int i = 1; i < 10; i++) {
        Entry entry;
        entry.key.push_back(i);
        entry.value.push_back('8');
        entries.push_back(entry);
    }
    /**
     * @tc.steps: step2. PutBatch the prepared data.
     * @tc.expected: step2. PutBatch returns OK.
     */
    EXPECT_TRUE(g_kvDelegatePtr->PutBatch(entries) == OK);

    /**
     * @tc.steps: step3. Get the data from the database.
     * @tc.expected: step3. Get returns OK and the get value is equal to the inserted value before.
     */
    GetSnapshotUnitTest();
    for (int i = 1; i < 10; i++) {
        Key keyTmp;
        keyTmp.push_back(i);

        g_snapshotDelegatePtr->Get(keyTmp, g_valueCallback);
        EXPECT_TRUE(g_valueStatus == OK);
        EXPECT_TRUE(g_value.size() > 0);
        if (g_value.size() > 0) {
            EXPECT_TRUE(g_value.front() == '8');
        }
    }
}

/**
  * @tc.name: PutBatch002
  * @tc.desc: PutBatch modified data into the multiversion database
  * @tc.type: FUNC
  * @tc.require: AR000C6TRV AR000CQDTM
  * @tc.author: huangnaigu
  */
HWTEST_F(DistributedDBInterfacesDataOperationSyncDBTest, PutBatch002, TestSize.Level1)
{
    /**
     * @tc.steps: step1. prepare the batch data.
     */
    vector<Entry> entries;
    for (int i = 1; i < 10; i++) {
        Entry entry;
        entry.key.push_back(i);
        entry.value.push_back('2');
        entries.push_back(entry);
    }
    /**
     * @tc.steps: step2. PutBatch the prepared batch data.
     * @tc.expected: step2. PutBatch returns OK.
     */
    EXPECT_TRUE(g_kvDelegatePtr->PutBatch(entries) == OK);
    /**
     * @tc.steps: step3. Get data from the database according the inserted keys.
     * @tc.expected: step3. Get returns OK and the read value is equal to the inserted value.
     */
    GetSnapshotUnitTest();
    for (int i = 1; i < 10; i++) {
        Key keyTmp;
        keyTmp.push_back(i);

        g_snapshotDelegatePtr->Get(keyTmp, g_valueCallback);
        EXPECT_TRUE(g_valueStatus == OK);
        EXPECT_TRUE(g_value.size() > 0);
        if (g_value.size() > 0) {
            EXPECT_TRUE(g_value.front() == '2');
        }
    }
}

/**
  * @tc.name: Delete001
  * @tc.desc: Delete existed data from the multiversion database
  * @tc.type: FUNC
  * @tc.require: AR000C6TRV AR000CQDTM
  * @tc.author: huangnaigu
  */
HWTEST_F(DistributedDBInterfacesDataOperationSyncDBTest, Delete001, TestSize.Level1)
{
    /**
     * @tc.steps: step1. Put the prepared data.
     */
    Key keyTmp;
    keyTmp.push_back(1);
    Value valueTmp;
    valueTmp.push_back(3);
    EXPECT_EQ(g_kvDelegatePtr->Put(keyTmp, valueTmp), OK);
    /**
     * @tc.steps: step2. Delete the existed data from the database.
     * @tc.expected: step2. Delete returns OK.
     */
    EXPECT_EQ(g_kvDelegatePtr->Delete(keyTmp), OK);
    /**
     * @tc.steps: step3. Get the deleted data from the database.
     * @tc.expected: step3. Get returns NOT_FOUND.
     */
    GetSnapshotUnitTest();
    g_snapshotDelegatePtr->Get(keyTmp, g_valueCallback);
    EXPECT_TRUE(g_valueStatus == NOT_FOUND);
}

/**
  * @tc.name: Delete002
  * @tc.desc: Delete non-existed data from the multiversion database
  * @tc.type: FUNC
  * @tc.require: AR000C6TRV AR000CQDTM
  * @tc.author: huangnaigu
  */
HWTEST_F(DistributedDBInterfacesDataOperationSyncDBTest, Delete002, TestSize.Level1)
{
    /**
     * @tc.steps: step1. Clear the database.
     */
    EXPECT_TRUE(g_kvDelegatePtr->Clear() == OK);

    /**
     * @tc.steps: step2. Delete the non-existed data from the database.
     * @tc.expected: step2. Delete returns OK.
     */
    Key keyTmp;
    keyTmp.push_back(1);
    EXPECT_EQ(g_kvDelegatePtr->Delete(keyTmp), OK);
}

/**
  * @tc.name: DeleteBatch001
  * @tc.desc: Delete the existed batch data from the multiversion database
  * @tc.type: FUNC
  * @tc.require: AR000C6TRV AR000CQDTM
  * @tc.author: huangnaigu
  */
HWTEST_F(DistributedDBInterfacesDataOperationSyncDBTest, DeleteBatch001, TestSize.Level1)
{
    /**
     * @tc.steps: step1. Put the batch data into the database.
     */
    vector<Entry> entries;
    for (int i = 1; i < 4; i++) {
        Entry entry;
        entry.key.push_back(i);
        entry.value.push_back('2');
        entries.push_back(entry);
    }
    EXPECT_TRUE(g_kvDelegatePtr->PutBatch(entries) == OK);

    /**
     * @tc.steps: step2. Delete the batch data from the database.
     * @tc.steps: step2. DeleteBatch returns OK.
     */
    vector<Key> keys;
    for (int i = 1; i < 4; i++) {
        Key key;
        key.push_back(i);
        keys.push_back(key);
    }
    EXPECT_TRUE(g_kvDelegatePtr->DeleteBatch(keys) == OK);

    /**
     * @tc.steps: step3. Get all the data from the database.
     * @tc.steps: step3. GetEntries result NOT_FOUND.
     */
    Key keyTmp;
    GetSnapshotUnitTest();
    g_snapshotDelegatePtr->GetEntries(keyTmp, g_entryVectorCallback);
    EXPECT_EQ(g_entryVectorStatus, NOT_FOUND);
}

/**
  * @tc.name: DeleteBatch002
  * @tc.desc: Delete the non-existed batch data from the multiversion database
  * @tc.type: FUNC
  * @tc.require: AR000C6TRV AR000CQDTM
  * @tc.author: huangnaigu
  */
HWTEST_F(DistributedDBInterfacesDataOperationSyncDBTest, DeleteBatch002, TestSize.Level1)
{
    /**
     * @tc.steps: step1. clear the database.
     */
    EXPECT_TRUE(g_kvDelegatePtr->Clear() == OK);
    /**
     * @tc.steps: step2. Delete the batch non-existed data from the database.
     * @tc.expected: step2. DeleteBatch returns OK
     */
    vector<Key> keys;
    for (int i = 1; i < 10; i++) {
        Key key;
        key.push_back(i);
        keys.push_back(key);
    }
    EXPECT_TRUE(g_kvDelegatePtr->DeleteBatch(keys) == OK);
}

/**
  * @tc.name: GetEntries001
  * @tc.desc: Get the batch data from the non-empty database by the prefix key.
  * @tc.type: FUNC
  * @tc.require: AR000C6TRV AR000CQDTM
  * @tc.author: huangnaigu
  */
HWTEST_F(DistributedDBInterfacesDataOperationSyncDBTest, GetEntries001, TestSize.Level1)
{
    /**
     * @tc.steps: step1. insert batch data into the database.
     */
    vector<Entry> entries;
    for (int i = 1; i <= 10; i++) {
        Entry entry;
        for (int j = 1; j <= i; j++) {
            entry.key.push_back(j);
        }
        entry.value.push_back(i);
        entries.push_back(entry);
    }

    EXPECT_TRUE(g_kvDelegatePtr->PutBatch(entries) == OK);

    Key keyPrefix;
    for (int j = 1; j <= 5; j++) {
        keyPrefix.push_back(j);
    }
    /**
     * @tc.steps: step2. Get batch data from the database using the prefix key.
     * @tc.expected: step2. GetEntries results OK and the result entries size is the match size.
     */
    unsigned long matchSize = 6;
    GetSnapshotUnitTest();
    g_snapshotDelegatePtr->GetEntries(keyPrefix, g_entryVectorCallback);
    ASSERT_TRUE(g_matchSize == matchSize);
    EXPECT_TRUE(g_entryVectorStatus == OK);
}

/**
  * @tc.name: GetEntries002
  * @tc.desc: Get all the data(empty prefixkey) from the empty database.
  * @tc.type: FUNC
  * @tc.require: AR000C6TRV AR000CQDTM
  * @tc.author: huangnaigu
  */
HWTEST_F(DistributedDBInterfacesDataOperationSyncDBTest, GetEntries002, TestSize.Level1)
{
    /**
     * @tc.steps: step1. Get all the data from the empty database.
     * @tc.expected: step1. GetEntries results NOT_FOUND.
     */
    Key keyPrefix;
    for (int j = 1; j <= 5; j++) {
        keyPrefix.push_back('a');
    }

    unsigned long matchSize = 0;
    GetSnapshotUnitTest();
    g_snapshotDelegatePtr->GetEntries(keyPrefix, g_entryVectorCallback);
    ASSERT_TRUE(g_matchSize == matchSize);
    EXPECT_TRUE(g_entryVectorStatus == NOT_FOUND);
}

/**
  * @tc.name: GetEntries003
  * @tc.desc: Get all the data(empty prefixkey) from the database.
  * @tc.type: FUNC
  * @tc.require: AR000C6TRV AR000CQDTM
  * @tc.author: huangnaigu
  */
HWTEST_F(DistributedDBInterfacesDataOperationSyncDBTest, GetEntries003, TestSize.Level1)
{
    /**
     * @tc.steps: step1. Put batch data into the database.
     */
    vector<Entry> entries;
    const unsigned long entriesSize = 10;
    for (unsigned long i = 1; i <= entriesSize; i++) {
        Entry entry;
        for (unsigned long j = 1; j <= i; j++) {
            entry.key.push_back(j);
        }
        entry.value.push_back(i);
        entries.push_back(entry);
    }

    EXPECT_TRUE(g_kvDelegatePtr->PutBatch(entries) == OK);

    /**
     * @tc.steps: step2. Get all the data from the database using the empty prefix key.
     * @tc.expected: step2. GetEntries results OK and the entries size is the put batch data size.
     */
    Key keyPrefix;
    GetSnapshotUnitTest();
    g_snapshotDelegatePtr->GetEntries(keyPrefix, g_entryVectorCallback);
    ASSERT_EQ(g_matchSize, entriesSize);
    EXPECT_TRUE(g_entryVectorStatus == OK);
}

static void TestSnapshotCreateAndRelease()
{
    DBStatus status;
    KvStoreSnapshotDelegate *snapshot = nullptr;
    KvStoreObserver *observer = nullptr;
    auto snapshotDelegateCallback = bind(&DistributedDBToolsUnitTest::SnapshotDelegateCallback,
        placeholders::_1, placeholders::_2, std::ref(status), std::ref(snapshot));

    /**
     * @tc.steps: step1. Obtain the snapshot object snapshot through
     *  the GetKvStoreSnapshot interface of the delegate.
     * @tc.expected: step1. Returns a non-empty snapshot.
     */
    g_kvDelegatePtr->GetKvStoreSnapshot(observer, snapshotDelegateCallback);

    EXPECT_TRUE(status == OK);
    EXPECT_NE(snapshot, nullptr);

    /**
     * @tc.steps: step2. Release the obtained snapshot through
     *  the ReleaseKvStoreSnapshot interface of the delegate.
     * @tc.expected: step2. Release successfully.
     */
    EXPECT_EQ(g_kvDelegatePtr->ReleaseKvStoreSnapshot(snapshot), OK);
}

/**
  * @tc.name: GetSnapshot001
  * @tc.desc: Get observer is empty, whether you get the snapshot.
  * @tc.type: FUNC
  * @tc.require: AR000BVRNF AR000CQDTI
  * @tc.author: wangbingquan
  */
HWTEST_F(DistributedDBInterfacesDataOperationSyncDBTest, GetSnapshot001, TestSize.Level1)
{
    /**
     * @tc.steps: step1.Obtain the snapshot object whose observer is null
     *  by using the GetKvStoreSnapshot interface of the delegate.
     * @tc.expected: step1. The obtained snapshot is not empty.
     */
    TestSnapshotCreateAndRelease();
}

/**
  * @tc.name: GetSnapshot002
  * @tc.desc: Get observer is not empty, whether you get the snapshot.
  * @tc.type: FUNC
  * @tc.require: AR000BVRNF AR000CQDTI
  * @tc.author: wangbingquan
  */
HWTEST_F(DistributedDBInterfacesDataOperationSyncDBTest, GetSnapshot002, TestSize.Level1)
{
    /**
     * @tc.steps: step1.Obtain the snapshot object whose observer is null
     *  by using the GetKvStoreSnapshot interface of the delegate.
     * @tc.expected: step1. The obtained snapshot is not empty.
     */
    DBStatus status;
    KvStoreSnapshotDelegate *snapshot = nullptr;
    KvStoreObserverUnitTest *observer = new (std::nothrow) KvStoreObserverUnitTest;
    ASSERT_NE(observer, nullptr);
    auto snapshotDelegateCallback = bind(&DistributedDBToolsUnitTest::SnapshotDelegateCallback,
        placeholders::_1, placeholders::_2, std::ref(status), std::ref(snapshot));
    g_kvDelegatePtr->GetKvStoreSnapshot(observer, snapshotDelegateCallback);

    EXPECT_TRUE(status == OK);
    EXPECT_NE(snapshot, nullptr);

    /**
     * @tc.steps: step2. Release the snapshot get before.
     * @tc.expected: step2. ReleaseKvStoreSnapshot returns OK.
     */
    EXPECT_EQ(g_kvDelegatePtr->ReleaseKvStoreSnapshot(snapshot), OK);
    delete observer;
    observer = nullptr;
}

/**
  * @tc.name: ReleaseSnapshot001
  * @tc.desc: To test the function of releasing an empty snapshot.
  * @tc.type: FUNC
  * @tc.require: AR000BVRNF AR000CQDTI
  * @tc.author: wangbingquan
  */
HWTEST_F(DistributedDBInterfacesDataOperationSyncDBTest, ReleaseSnapshot001, TestSize.Level1)
{
    /**
     * @tc.steps: step1.Release the null pointer snapshot through
     *  the ReleaseKvStoreSnapshot interface of the delegate.
     * @tc.expected: step1. Return ERROR.
     */
    KvStoreSnapshotDelegate *snapshot = nullptr;
    EXPECT_EQ(g_kvDelegatePtr->ReleaseKvStoreSnapshot(snapshot), DB_ERROR);
}

/**
  * @tc.name: ReleaseSnapshot002
  * @tc.desc: Release the obtained snapshot object that is not empty.
  * @tc.type: FUNC
  * @tc.require: AR000BVRNF AR000CQDTI
  * @tc.author: wangbingquan
  */
HWTEST_F(DistributedDBInterfacesDataOperationSyncDBTest, ReleaseSnapshot002, TestSize.Level1)
{
    TestSnapshotCreateAndRelease();
}

static void TestSnapshotEntryPut()
{
    KvStoreObserverUnitTest *observer = nullptr;
    DBStatus status;
    KvStoreSnapshotDelegate *snapshotA = nullptr;
    auto snapshotDelegateCallbackA = bind(&DistributedDBToolsUnitTest::SnapshotDelegateCallback,
        placeholders::_1, placeholders::_2, std::ref(status), std::ref(snapshotA));

    /**
     * @tc.steps: step1.Release the null pointer snapshot through
     *  the ReleaseKvStoreSnapshot interface of the delegate.
     * @tc.expected: step1. Return not empty snapshot.
     */
    g_kvDelegatePtr->GetKvStoreSnapshot(observer, snapshotDelegateCallbackA);
    ASSERT_NE(snapshotA, nullptr);
    Key keyA;
    Value valueA;
    Value valueB;
    DistributedDBToolsUnitTest::GetRandomKeyValue(keyA);
    DistributedDBToolsUnitTest::GetRandomKeyValue(valueA);
    DistributedDBToolsUnitTest::GetRandomKeyValue(valueB);

    /**
     * @tc.steps: step2. Obtain the keyA data through the Get interface of the snapshotA.
     * @tc.expected: step2. Return NOT_FOUND.
     */
    snapshotA->Get(keyA, g_valueCallback);
    EXPECT_EQ(g_valueStatus, NOT_FOUND);

    /**
     * @tc.steps: step3. Insert the data of keyA and valueA through the Put interface of the delegate.
     */
    g_kvDelegatePtr->Put(keyA, valueA);
    std::this_thread::sleep_for(std::chrono::milliseconds(OBSERVER_SLEEP_TIME));

    KvStoreSnapshotDelegate *snapshotB = nullptr;
    auto snapshotDelegateCallbackB = bind(&DistributedDBToolsUnitTest::SnapshotDelegateCallback,
        placeholders::_1, placeholders::_2, std::ref(status), std::ref(snapshotB));

    /**
     * @tc.steps: step5. Obtain the snapshot object snapshotB through the GetKvStoreSnapshot
     *  interface of the delegate. Obtain the keyA data through the Get interface of the snapshotB.
     * @tc.expected: step5. Return a non-empty snapshot. The value of keyA is valueA..
     */
    g_kvDelegatePtr->GetKvStoreSnapshot(observer, snapshotDelegateCallbackB);
    ASSERT_NE(snapshotA, nullptr);

    /**
     * @tc.steps: step4. Obtain the keyA data through the Get interface of the snapshotA.
     * @tc.expected: step4. Return NOT_FOUND.
     */
    snapshotA->Get(keyA, g_valueCallback);
    EXPECT_EQ(g_valueStatus, NOT_FOUND);
    snapshotB->Get(keyA, g_valueCallback);
    EXPECT_EQ(g_valueStatus, OK);
    EXPECT_EQ(DistributedDBToolsUnitTest::IsValueEqual(g_value, valueA), true);

    /**
     * @tc.steps: step6. Insert the data of keyA and valueB through the Put interface of the delegate..
     */
    g_kvDelegatePtr->Put(keyA, valueB);
    std::this_thread::sleep_for(std::chrono::milliseconds(OBSERVER_SLEEP_TIME));
    KvStoreSnapshotDelegate *snapshotC = nullptr;
    auto snapshotDelegateCallbackC = bind(&DistributedDBToolsUnitTest::SnapshotDelegateCallback,
        placeholders::_1, placeholders::_2, std::ref(status), std::ref(snapshotC));

    /**
     * @tc.steps: step7. Obtain the snapshotC through the GetKvStoreSnapshot interface
     *  of the delegate and obtain the data of the keyA through the Get interface.
     * @tc.expected: step7. Return a non-empty snapshot. The value of keyA is valueB.
     */
    g_kvDelegatePtr->GetKvStoreSnapshot(observer, snapshotDelegateCallbackC);
    ASSERT_NE(snapshotC, nullptr);

    /**
     * @tc.steps: step8. Obtain the keyA data through the Get interface of the snapshotB.
     * @tc.expected: step8. Return OK, and the value of keyA is valueA..
     */
    snapshotB->Get(keyA, g_valueCallback);
    EXPECT_EQ(g_valueStatus, OK);
    EXPECT_EQ(DistributedDBToolsUnitTest::IsValueEqual(g_value, valueA), true);
    snapshotC->Get(keyA, g_valueCallback);
    EXPECT_EQ(g_valueStatus, OK);
    EXPECT_EQ(DistributedDBToolsUnitTest::IsValueEqual(g_value, valueB), true);

    g_kvDelegatePtr->ReleaseKvStoreSnapshot(snapshotA);
    g_kvDelegatePtr->ReleaseKvStoreSnapshot(snapshotB);
    g_kvDelegatePtr->ReleaseKvStoreSnapshot(snapshotC);
}

static void TestSnapshotEntryDelete()
{
    KvStoreObserverUnitTest *observer = nullptr;
    DBStatus status;
    Key key;
    Value value;
    DistributedDBToolsUnitTest::GetRandomKeyValue(key);
    DistributedDBToolsUnitTest::GetRandomKeyValue(value);

    g_kvDelegatePtr->Put(key, value);
    KvStoreSnapshotDelegate *snapshotA = nullptr;
    auto snapshotDelegateCallbackA = bind(&DistributedDBToolsUnitTest::SnapshotDelegateCallback,
        placeholders::_1, placeholders::_2, std::ref(status), std::ref(snapshotA));
    g_kvDelegatePtr->GetKvStoreSnapshot(observer, snapshotDelegateCallbackA);
    ASSERT_NE(snapshotA, nullptr);
    snapshotA->Get(key, g_valueCallback);
    EXPECT_EQ(g_valueStatus, OK);
    EXPECT_EQ(DistributedDBToolsUnitTest::IsValueEqual(g_value, value), true);

    /**
     * @tc.steps: step9. Delete the keyA data through
     *  the Delete interface of the delegate.
     */
    g_kvDelegatePtr->Delete(key);
    KvStoreSnapshotDelegate *snapshotB = nullptr;
    auto snapshotDelegateCallbackB = bind(&DistributedDBToolsUnitTest::SnapshotDelegateCallback,
        placeholders::_1, placeholders::_2, std::ref(status), std::ref(snapshotB));

    /**
     * @tc.steps:step10 Obtain the snapshot object snapshotB through the GetKvStoreSnapshot interface of the delegate.
     */
    g_kvDelegatePtr->GetKvStoreSnapshot(observer, snapshotDelegateCallbackB);
    ASSERT_NE(snapshotB, nullptr);

    /**
     * @tc.steps: step11. Obtain the value of keyA through the Get interface of snapshotB.
     * @tc.expected: step11. Return NOT_FOUND.
     */
    snapshotB->Get(key, g_valueCallback);
    EXPECT_EQ(g_valueStatus, NOT_FOUND);

    /**
     * @tc.steps: step12. Obtain the value of keyA through the Get interface of snapshotA.
     * @tc.expected: step12. Return OK, the value of keyA is valueB.
     */
    snapshotA->Get(key, g_valueCallback);
    EXPECT_EQ(g_valueStatus, OK);
    EXPECT_EQ(DistributedDBToolsUnitTest::IsValueEqual(g_value, value), true);

    g_kvDelegatePtr->ReleaseKvStoreSnapshot(snapshotA);
    g_kvDelegatePtr->ReleaseKvStoreSnapshot(snapshotB);
}

/**
  * @tc.name: get_snapshot_entry_001
  * @tc.desc: Obtain data from the obtained snapshot object and test the impact of the write
  *  database on the snapshot obtained after the snapshot is obtained.
  * @tc.type: FUNC
  * @tc.require: AR000BVRNH AR000CQDTJ
  * @tc.author: wangbingquan
  */
HWTEST_F(DistributedDBInterfacesDataOperationSyncDBTest, GetSnapshotEntry001, TestSize.Level1)
{
    TestSnapshotEntryPut();
    TestSnapshotEntryDelete();
}

/**
  * @tc.name: get_snapshot_entry_002
  * @tc.desc: Read the data of the invalid key from the obtained snapshot object.
  * @tc.type: FUNC
  * @tc.require: AR000BVRNH AR000CQDTJ
  * @tc.author: wangbingquan
  */
HWTEST_F(DistributedDBInterfacesDataOperationSyncDBTest, GetSnapshotEntry002, TestSize.Level1)
{
    Key key;
    Value value;
    DistributedDBToolsUnitTest::GetRandomKeyValue(key, MAX_KEY_SIZE); // max key size.
    DistributedDBToolsUnitTest::GetRandomKeyValue(value, MAX_VAL_SIZE); // max valueSize;

    /**
     * @tc.steps: step1.Insert [keyA, valueA] data through the Put interface of the delegate.
     */
    g_kvDelegatePtr->Put(key, value);
    KvStoreSnapshotDelegate *snapshot = nullptr;
    KvStoreObserverUnitTest *observer = nullptr;
    DBStatus status;
    auto snapshotDelegateCallback = bind(&DistributedDBToolsUnitTest::SnapshotDelegateCallback,
        placeholders::_1, placeholders::_2, std::ref(status), std::ref(snapshot));

    /**
     * @tc.steps: step2. Obtain the snapshot object snapshotA through
     *  the GetKvStoreSnapshot interface of the delegate.
     */
    g_kvDelegatePtr->GetKvStoreSnapshot(observer, snapshotDelegateCallback);

    snapshot->Get(key, g_valueCallback);
    ASSERT_EQ(g_valueStatus, OK);
    EXPECT_EQ(DistributedDBToolsUnitTest::IsValueEqual(g_value, value), true);

    /**
     * @tc.steps: step3. Obtain the empty key data through the Get interface of the snapshotA.
     * @tc.expected: step3. Return ERROR.
     */
    Key keyEmpty;
    snapshot->Get(keyEmpty, g_valueCallback);
    ASSERT_EQ(g_valueStatus, INVALID_ARGS);

    /**
     * @tc.steps: step4. Obtain the data whose key size exceeds 1024 through the Get interface of the snapshotA.
     * @tc.expected: step4. Return ERROR.
     */
    Key keyMax;
    DistributedDBToolsUnitTest::GetRandomKeyValue(keyMax, INVALID_KEY_SIZE); // max add one
    snapshot->Get(keyMax, g_valueCallback);
    ASSERT_EQ(g_valueStatus, INVALID_ARGS);
    g_kvDelegatePtr->ReleaseKvStoreSnapshot(snapshot);
}

static void SnapshotTestPreEntriesPutInner(KvStoreSnapshotDelegate *snapshotA, KvStoreSnapshotDelegate *&snapshotB)
{
    DistributedDBToolsUnitTest::GetRandomKeyValue(g_entryA.key);
    DistributedDBToolsUnitTest::GetRandomKeyValue(g_entryA.value);
    DistributedDBToolsUnitTest::GetRandomKeyValue(g_entryB.value);
    g_entryB.key = g_entryA.key;

    g_entryB.key.push_back(std::rand() % 0xFF); // push back random one.
    g_entryC = g_entryB;
    uint8_t tmp = (g_entryC.key[0] == 0xFF) ? 0 : 0xFF;
    g_entryC.key.insert(g_entryC.key.begin(), tmp);

    /**
     * @tc.steps: step2. Obtain the data whose keyPrefix is empty through
     *  the GetEntries interface of the snapshotA.
     * @tc.expected: step2. Return NOT_FOUND.
     */
    snapshotA->GetEntries(g_entryA.key, g_entryVectorCallback);
    EXPECT_EQ(g_entryVectorStatus, NOT_FOUND);

    /**
     * @tc.steps: step3. Obtain the data whose keyPrefix is empty through
     *  the GetEntries interface of the snapshotA.
     * @tc.expected: step3. Return NOT_FOUND.
     */
    g_kvDelegatePtr->Put(g_entryA.key, g_entryA.value);
    std::this_thread::sleep_for(std::chrono::milliseconds(OBSERVER_SLEEP_TIME));
    g_kvDelegatePtr->Put(g_entryB.key, g_entryB.value);
    std::this_thread::sleep_for(std::chrono::milliseconds(OBSERVER_SLEEP_TIME));
    g_kvDelegatePtr->Put(g_entryC.key, g_entryC.value);
    std::this_thread::sleep_for(std::chrono::milliseconds(OBSERVER_SLEEP_TIME));

    DBStatus status;
    auto snapshotDelegateCallbackB = bind(&DistributedDBToolsUnitTest::SnapshotDelegateCallback,
        placeholders::_1, placeholders::_2, std::ref(status), std::ref(snapshotB));
    /**
     * @tc.steps: step5. Obtain the snapshot object snapshotB
     *  through the GetKvStoreSnapshot interface of the delegate.
     *  Obtain the data whose keyPrefix is empty through the GetEntries interface of the snapshotB.
     * @tc.expected: step5. Return a non-empty snapshot. GetEntries Obtain data [keyA, valueA], [keyB, valueB].
     */
    g_kvDelegatePtr->GetKvStoreSnapshot(nullptr, snapshotDelegateCallbackB);
    ASSERT_NE(snapshotB, nullptr);
}

static void SnapshotTestPreEntriesPut()
{
    DBStatus status;
    KvStoreSnapshotDelegate *snapshotA = nullptr;
    auto snapshotDelegateCallbackA = bind(&DistributedDBToolsUnitTest::SnapshotDelegateCallback,
        placeholders::_1, placeholders::_2, std::ref(status), std::ref(snapshotA));

    /**
     * @tc.steps: step1. Obtain the snapshot object snapshotA through
     *  the GetKvStoreSnapshot interface of the delegate.
     * @tc.expected: step1. Returns a non-empty snapsho.
     */
    g_kvDelegatePtr->GetKvStoreSnapshot(nullptr, snapshotDelegateCallbackA);
    ASSERT_NE(snapshotA, nullptr);

    KvStoreSnapshotDelegate *snapshotB = nullptr;
    SnapshotTestPreEntriesPutInner(snapshotA, snapshotB);

    snapshotA->GetEntries(g_entryA.key, g_entryVectorCallback);
    EXPECT_EQ(g_entryVectorStatus, NOT_FOUND);
    snapshotB->GetEntries(g_entryA.key, g_entryVectorCallback);
    EXPECT_EQ(g_matchSize, 2UL);

    EXPECT_EQ(DistributedDBToolsUnitTest::IsEntryExist(g_entryA, g_entriesVector), true);
    EXPECT_EQ(DistributedDBToolsUnitTest::IsEntryExist(g_entryB, g_entriesVector), true);

    g_entryD = g_entryA;
    g_entryD.value.push_back(std::rand() % 0xFF); // random one byte.

    /**
     * @tc.steps: step6. Insert [keyA, valueC] data through the Put interface of the delegate.
     */
    g_kvDelegatePtr->Put(g_entryD.key, g_entryD.value);
    std::this_thread::sleep_for(std::chrono::milliseconds(OBSERVER_SLEEP_TIME));
    KvStoreSnapshotDelegate *snapshotC = nullptr;
    auto snapshotDelegateCallbackC = bind(&DistributedDBToolsUnitTest::SnapshotDelegateCallback,
        placeholders::_1, placeholders::_2, std::ref(status), std::ref(snapshotC));

    /**
     * @tc.steps: step7. Obtain the snapshot object snapshotC
     *  through the GetKvStoreSnapshot interface of the delegate. Obtain the data whose
     *  keyPrefix is empty through the GetEntries interface of the snapshotC.
     * @tc.expected: step5. Return a non-empty snapshot. GetEntries Obtain data [keyA, valueC], [keyB, valueB].
     */
    g_kvDelegatePtr->GetKvStoreSnapshot(nullptr, snapshotDelegateCallbackC);
    ASSERT_NE(snapshotC, nullptr);

    /**
     * @tc.steps: step8. Obtain the data whose keyPrefix is empty
     *  through the GetEntries interface of the snapshotB.
     * @tc.expected: step8. Return OK, GetEntries obtains data [keyA, valueA], [keyB, valueB]..
     */
    snapshotB->GetEntries(g_entryA.key, g_entryVectorCallback);
    EXPECT_EQ(g_entryVectorStatus, OK);
    EXPECT_EQ(DistributedDBToolsUnitTest::IsEntryExist(g_entryA, g_entriesVector), true);
    snapshotC->GetEntries(g_entryA.key, g_entryVectorCallback);
    EXPECT_EQ(g_entryVectorStatus, OK);
    EXPECT_EQ(DistributedDBToolsUnitTest::IsEntryExist(g_entryD, g_entriesVector), true);

    g_kvDelegatePtr->ReleaseKvStoreSnapshot(snapshotA);
    g_kvDelegatePtr->ReleaseKvStoreSnapshot(snapshotB);
    g_kvDelegatePtr->ReleaseKvStoreSnapshot(snapshotC);
}

static void SnapshotTestPreEntriesDelete()
{
    DistributedDBToolsUnitTest::GetRandomKeyValue(g_entryA.key);
    DistributedDBToolsUnitTest::GetRandomKeyValue(g_entryA.value);
    g_entryB.key = g_entryA.key;
    g_entryB.key.push_back(std::rand() % 0xFF);
    DistributedDBToolsUnitTest::GetRandomKeyValue(g_entryB.value);

    g_kvDelegatePtr->Put(g_entryA.key, g_entryA.value);
    g_kvDelegatePtr->Put(g_entryB.key, g_entryB.value);

    DBStatus status;
    KvStoreSnapshotDelegate *snapshotA = nullptr;
    auto snapshotDelegateCallbackA = bind(&DistributedDBToolsUnitTest::SnapshotDelegateCallback,
        placeholders::_1, placeholders::_2, std::ref(status), std::ref(snapshotA));
    g_kvDelegatePtr->GetKvStoreSnapshot(nullptr, snapshotDelegateCallbackA);
    ASSERT_NE(snapshotA, nullptr);

    snapshotA->GetEntries(g_entryA.key, g_entryVectorCallback);
    EXPECT_EQ(g_entryVectorStatus, OK);
    EXPECT_EQ(g_matchSize, 2UL); // entryA and entryB
    EXPECT_EQ(DistributedDBToolsUnitTest::IsEntryExist(g_entryA, g_entriesVector), true);
    EXPECT_EQ(DistributedDBToolsUnitTest::IsEntryExist(g_entryB, g_entriesVector), true);

    /**
     * @tc.steps: step9. Delete the keyA data through the Delete interface of the delegate.
     */
    g_kvDelegatePtr->Delete(g_entryA.key);

    KvStoreSnapshotDelegate *snapshotB = nullptr;
    auto snapshotDelegateCallbackB = bind(&DistributedDBToolsUnitTest::SnapshotDelegateCallback,
        placeholders::_1, placeholders::_2, std::ref(status), std::ref(snapshotB));

    /**
     * @tc.steps: step10. Obtain the snapshot object snapshotB
     *  through the GetKvStoreSnapshot interface of the delegate.
     */
    g_kvDelegatePtr->GetKvStoreSnapshot(nullptr, snapshotDelegateCallbackB);
    ASSERT_NE(snapshotB, nullptr);

    /**
     * @tc.steps: step11\12. Obtain the value of keyA through the Get interface of snapshotA\B.
     */
    snapshotA->GetEntries(g_entryA.key, g_entryVectorCallback);
    EXPECT_EQ(g_entryVectorStatus, OK);
    EXPECT_EQ(g_matchSize, 2UL);
    EXPECT_EQ(DistributedDBToolsUnitTest::IsEntryExist(g_entryA, g_entriesVector), true);
    EXPECT_EQ(DistributedDBToolsUnitTest::IsEntryExist(g_entryB, g_entriesVector), true);

    snapshotB->GetEntries(g_entryA.key, g_entryVectorCallback);
    EXPECT_EQ(g_matchSize, 1UL);
    EXPECT_EQ(DistributedDBToolsUnitTest::IsEntryExist(g_entryA, g_entriesVector), false);
    EXPECT_EQ(DistributedDBToolsUnitTest::IsEntryExist(g_entryB, g_entriesVector), true);

    g_kvDelegatePtr->ReleaseKvStoreSnapshot(snapshotA);
    g_kvDelegatePtr->ReleaseKvStoreSnapshot(snapshotB);
}

/**
  * @tc.name: GetSnapshotEntries001
  * @tc.desc: To test the function of obtaining full data when keyPrefix is set to null.
  * @tc.type: FUNC
  * @tc.require: AR000BVRNI AR000CQDTK
  * @tc.author: wangbingquan
  */
HWTEST_F(DistributedDBInterfacesDataOperationSyncDBTest, GetSnapshotEntries001, TestSize.Level1)
{
    std::srand(std::time(nullptr));
    SnapshotTestPreEntriesPut();
    SnapshotTestPreEntriesDelete();
}

static void SnapshotTestEmptyPreEntriesPut()
{
    DBStatus status;
    Key emptyKey;
    KvStoreSnapshotDelegate *snapshotA = nullptr;
    auto snapshotDelegateCallbackA = bind(&DistributedDBToolsUnitTest::SnapshotDelegateCallback,
        placeholders::_1, placeholders::_2, std::ref(status), std::ref(snapshotA));

    /**
     * @tc.steps: step1.Obtain the snapshot object snapshotA
     *  through the GetKvStoreSnapshot interface of the delegate.
     * @tc.expected: step1. Returns a non-empty snapshot.
     */
    g_kvDelegatePtr->GetKvStoreSnapshot(nullptr, snapshotDelegateCallbackA);
    ASSERT_NE(snapshotA, nullptr);

    DistributedDBToolsUnitTest::GetRandomKeyValue(g_entryA.key);
    DistributedDBToolsUnitTest::GetRandomKeyValue(g_entryA.value);
    DistributedDBToolsUnitTest::GetRandomKeyValue(g_entryB.key, g_entryA.key.size() + 1); // more one
    DistributedDBToolsUnitTest::GetRandomKeyValue(g_entryB.value);

    g_kvDelegatePtr->Put(g_entryA.key, g_entryA.value);
    g_kvDelegatePtr->Put(g_entryB.key, g_entryB.value);

    /**
     * @tc.steps: step2. Obtain the data whose keyPrefix is AB from the GetEntries interface of the snapshotA.
     * @tc.expected: step2. Return NOT_FOUND.
     */
    snapshotA->GetEntries(emptyKey, g_entryVectorCallback);
    EXPECT_EQ(g_entryVectorStatus, NOT_FOUND);

    KvStoreSnapshotDelegate *snapshotB = nullptr;
    auto snapshotDelegateCallbackB = bind(&DistributedDBToolsUnitTest::SnapshotDelegateCallback,
        placeholders::_1, placeholders::_2, std::ref(status), std::ref(snapshotB));
    g_kvDelegatePtr->GetKvStoreSnapshot(nullptr, snapshotDelegateCallbackB);
    ASSERT_NE(snapshotB, nullptr);
    snapshotA->GetEntries(emptyKey, g_entryVectorCallback);
    EXPECT_EQ(g_entryVectorStatus, NOT_FOUND);
    snapshotB->GetEntries(emptyKey, g_entryVectorCallback);
    EXPECT_EQ(g_matchSize, 2UL);

    EXPECT_EQ(DistributedDBToolsUnitTest::IsEntryExist(g_entryA, g_entriesVector), true);
    EXPECT_EQ(DistributedDBToolsUnitTest::IsEntryExist(g_entryB, g_entriesVector), true);

    g_entryC = g_entryA;
    g_entryC.value.push_back(std::rand() % 0xFF); // random one byte.

    /**
     * @tc.steps: step3.Insert the data of ["AB", valueA], ["AE", valueB], ["ABC", valueC],
     *  and ["CAB", valueD] through the Put interface of the delegate.
     */
    g_kvDelegatePtr->Put(g_entryC.key, g_entryC.value);
    KvStoreSnapshotDelegate *snapshotC = nullptr;
    auto snapshotDelegateCallbackC = bind(&DistributedDBToolsUnitTest::SnapshotDelegateCallback,
        placeholders::_1, placeholders::_2, std::ref(status), std::ref(snapshotC));
    g_kvDelegatePtr->GetKvStoreSnapshot(nullptr, snapshotDelegateCallbackC);
    ASSERT_NE(snapshotC, nullptr);

    /**
     * @tc.steps: step5\6\7\8. Obtain the snapshot object snapshot
     *  through the GetKvStoreSnapshot interface of the delegate.
     *  Obtain the data whose keyPrefix is "AB" through the GetEntries interface of the snapshot.
     * @tc.expected: step5\6\7\8. Get Returns a non-empty snapshot and data.
     */
    snapshotB->GetEntries(emptyKey, g_entryVectorCallback);
    EXPECT_EQ(g_entryVectorStatus, OK);
    EXPECT_EQ(DistributedDBToolsUnitTest::IsEntryExist(g_entryA, g_entriesVector), true);
    EXPECT_EQ(DistributedDBToolsUnitTest::IsEntryExist(g_entryB, g_entriesVector), true);
    snapshotC->GetEntries(emptyKey, g_entryVectorCallback);
    EXPECT_EQ(g_entryVectorStatus, OK);
    EXPECT_EQ(DistributedDBToolsUnitTest::IsEntryExist(g_entryB, g_entriesVector), true);
    EXPECT_EQ(DistributedDBToolsUnitTest::IsEntryExist(g_entryC, g_entriesVector), true);

    g_kvDelegatePtr->ReleaseKvStoreSnapshot(snapshotA);
    g_kvDelegatePtr->ReleaseKvStoreSnapshot(snapshotB);
    g_kvDelegatePtr->ReleaseKvStoreSnapshot(snapshotC);
}

static void SnapshotTestEmptyPreEntriesDelete()
{
    DBStatus status;
    Key emptyKey;
    KvStoreSnapshotDelegate *snapshotA = nullptr;
    auto snapshotDelegateCallbackA = bind(&DistributedDBToolsUnitTest::SnapshotDelegateCallback,
        placeholders::_1, placeholders::_2, std::ref(status), std::ref(snapshotA));
    g_kvDelegatePtr->GetKvStoreSnapshot(nullptr, snapshotDelegateCallbackA);

    ASSERT_NE(snapshotA, nullptr);

    /**
     * @tc.steps: step9. Delete the "AB" data through the Delete interface of the delegate.
     */
    g_kvDelegatePtr->Delete(g_entryC.key);

    /**
     * @tc.steps: step11. Obtain the data whose keyPrefix is "AB" through the GetEntries interface of the snapshot.
     * @tc.expected: step11. Return OK.get [ABC,valueC].
     */
    snapshotA->GetEntries(emptyKey, g_entryVectorCallback);
    EXPECT_EQ(DistributedDBToolsUnitTest::IsEntryExist(g_entryC, g_entriesVector), true);

    KvStoreSnapshotDelegate *snapshotB = nullptr;
    auto snapshotDelegateCallbackB = bind(&DistributedDBToolsUnitTest::SnapshotDelegateCallback,
        placeholders::_1, placeholders::_2, std::ref(status), std::ref(snapshotB));

    /**
     * @tc.steps: step10.Obtain the snapshot object snapshot through the GetKvStoreSnapshot interface of the delegate.
     */
    g_kvDelegatePtr->GetKvStoreSnapshot(nullptr, snapshotDelegateCallbackB);
    ASSERT_NE(snapshotB, nullptr);

    /**
     * @tc.steps: step12. Obtain the data whose keyPrefix is "AB" through the GetEntries interface of the snapshot.
     * @tc.expected: step12. Return OK.
     */
    snapshotB->GetEntries(emptyKey, g_entryVectorCallback);
    EXPECT_EQ(g_entryVectorStatus, OK);
    EXPECT_EQ(DistributedDBToolsUnitTest::IsEntryExist(g_entryC, g_entriesVector), false);

    g_kvDelegatePtr->ReleaseKvStoreSnapshot(snapshotA);
    g_kvDelegatePtr->ReleaseKvStoreSnapshot(snapshotB);
}

/**
  * @tc.name: GetSnapshotEntries002
  * @tc.desc: To test the function of obtaining the prefix data when keyPrefix is set to a non-empty value.
  * @tc.type: FUNC
  * @tc.require: AR000BVRNI AR000CQDTK
  * @tc.author: wangbingquan
  */
HWTEST_F(DistributedDBInterfacesDataOperationSyncDBTest, GetSnapshotEntries002, TestSize.Level1)
{
    std::srand(std::time(nullptr));
    SnapshotTestEmptyPreEntriesPut();
    SnapshotTestEmptyPreEntriesDelete();
}

/**
  * @tc.name: GetSnapshotEntries003
  * @tc.desc: To test whether data can be obtained when keyPrefix is set to an ultra-long key.
  * @tc.type: FUNC
  * @tc.require: AR000BVRNI AR000CQDTK
  * @tc.author: wangbingquan
  */
HWTEST_F(DistributedDBInterfacesDataOperationSyncDBTest, GetSnapshotEntries003, TestSize.Level1)
{
    Key key;
    Value value;
    DistributedDBToolsUnitTest::GetRandomKeyValue(key, MAX_KEY_SIZE); // max key size.
    DistributedDBToolsUnitTest::GetRandomKeyValue(value, MAX_VAL_SIZE); // max valueSize;

    g_kvDelegatePtr->Put(key, value);
    KvStoreSnapshotDelegate *snapshot = nullptr;
    KvStoreObserverUnitTest *observer = nullptr;
    DBStatus status;
    auto snapshotDelegateCallback = bind(&DistributedDBToolsUnitTest::SnapshotDelegateCallback,
        placeholders::_1, placeholders::_2, std::ref(status), std::ref(snapshot));

    /**
     * @tc.steps: step1. Obtain the snapshot object snapshot through the GetKvStoreSnapshot interface of the delegate.
     * @tc.expected: step1. Returns a non-empty snapshot.
     */
    g_kvDelegatePtr->GetKvStoreSnapshot(observer, snapshotDelegateCallback);

    snapshot->GetEntries(key, g_entryVectorCallback);
    ASSERT_EQ(g_entryVectorStatus, OK);
    EXPECT_EQ(g_matchSize, 1UL);
    Entry entry = {key, value};
    DistributedDBToolsUnitTest::IsEntryExist(entry, g_entriesVector);

    /**
     * @tc.steps: step2.Obtain the data of the key whose keyPrefix exceeds 1024 bytes
     *  through the GetEntries interface of the snapshotA.
     * @tc.expected: step2. Return ERROR.
     */
    Key keyMax;
    DistributedDBToolsUnitTest::GetRandomKeyValue(keyMax, INVALID_KEY_SIZE); // max add one
    snapshot->GetEntries(keyMax, g_entryVectorCallback);
    ASSERT_EQ(g_entryVectorStatus, INVALID_ARGS);
    g_kvDelegatePtr->ReleaseKvStoreSnapshot(snapshot);
}

/**
  * @tc.name: TestTransactionException001
  * @tc.desc: An exception occurred while the test transaction was executing.
  * @tc.type: FUNC
  * @tc.require: AR000C0F0F AR000CQDTN or SR000BVRNJ
  * @tc.author: sunpeng
  */
HWTEST_F(DistributedDBInterfacesDataOperationSyncDBTest, TransactionException001, TestSize.Level1)
{
    // pre-set: create a db for test.
    vector<Entry> entries, entries1;

    /**
      * @tc.steps: step1. Start the transaction, insert the data in bulk, end the transaction,
      * insert the data of the data of the key1, the value1, the data of the key2, the value2, the transaction.
      */
    entries.push_back(ENTRY_1);
    entries.push_back(ENTRY_2);

    EXPECT_EQ(g_kvDelegatePtr->StartTransaction(), OK);
    EXPECT_TRUE(g_kvDelegatePtr->PutBatch(entries) == OK);
    EXPECT_TRUE(g_kvDelegatePtr->Commit() == OK);

    /**
     * @tc.steps: step2. Start the transaction, insert the data in bulk, end the transaction,
     * insert the data of the data of the key3, the value3, the data of the key4, the value4, the transaction.
     */
    entries1.push_back(ENTRY_3);
    entries1.push_back(ENTRY_4);

    EXPECT_EQ(g_kvDelegatePtr->StartTransaction(), OK);
    EXPECT_TRUE(g_kvDelegatePtr->PutBatch(entries1) == OK);
    EXPECT_EQ(g_kvDelegatePtr->Commit(), OK);

    /**
     * @tc.steps: step2. Close database and Delegate,
     */
    EXPECT_EQ(g_mgr.CloseKvStore(g_kvDelegatePtr), OK);
    g_kvDelegatePtr = nullptr;

    /**
     * @tc.steps: step4. Simulated scenes where the log library is not written to complete (power-down):
     * read the commit-log database through the sqlite3 interface, delete the latest head node,
     * and set the parent node of the header node as the head node.
     */
    IKvDBFactory *factory = IKvDBFactory::GetCurrent();
    ASSERT_NE(factory, nullptr);
    if (factory == nullptr) {
        LOGE("failed to get DefaultFactory!");
        return;
    }
    int result = E_OK;
    IKvDBCommitStorage *commitStorage = factory->CreateMultiVerCommitStorage(result);
    ASSERT_EQ(result, E_OK);
    ASSERT_NE(commitStorage, nullptr);

    std::string origIdentifier = USER_ID + "-" + APP_ID + "-" + STORE_ID_SYNC;
    std::string hashIdentifier = DBCommon::TransferHashString(origIdentifier);
    std::string hexDir = DBCommon::TransferStringToHex(hashIdentifier);
    IKvDBCommitStorage::Property property = {g_testDir, hexDir, false};

    ASSERT_EQ(commitStorage->Open(property), E_OK);
    int errCode = E_OK;
    result = commitStorage->RemoveCommit(commitStorage->GetHeader(errCode));
    ASSERT_EQ(result, E_OK);
    delete commitStorage;
    commitStorage = nullptr;

    /**
     * @tc.steps: step5. Get delegate with GetKvStore, create snapshot, get data from key3 and key4.
     * @tc.expected: step5. Data on key3 and key4 could not be obtained.
     */
    CipherPassword passwd;
    KvStoreDelegate::Option option = {true, false, false, CipherType::DEFAULT, passwd};
    g_mgr.GetKvStore(STORE_ID, option, g_kvDelegateCallback);
    ASSERT_TRUE(g_kvDelegatePtr != nullptr);
    EXPECT_TRUE(g_kvDelegateStatus == OK);

    g_kvDelegatePtr->GetKvStoreSnapshot(nullptr, g_snapshotDelegateCallback);
    EXPECT_TRUE(g_snapshotDelegateStatus == OK);
    ASSERT_TRUE(g_snapshotDelegatePtr != nullptr);

    g_snapshotDelegatePtr->Get(KEY_3, g_valueCallback);
    EXPECT_EQ(g_valueStatus, NOT_FOUND);

    g_valueStatus = INVALID_ARGS;
    g_snapshotDelegatePtr->Get(KEY_4, g_valueCallback);
    EXPECT_EQ(g_valueStatus, NOT_FOUND);
}
