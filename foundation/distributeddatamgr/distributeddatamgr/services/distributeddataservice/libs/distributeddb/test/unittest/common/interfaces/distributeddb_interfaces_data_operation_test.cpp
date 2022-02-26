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

using namespace testing::ext;
using namespace DistributedDB;
using namespace DistributedDBUnitTest;
using namespace std;

namespace {
    string g_testDir;
    const bool LOCAL_ONLY = true;
    const string STORE_ID = STORE_ID_LOCAL;

    KvStoreDelegateManager g_mgr(APP_ID, USER_ID);
    KvStoreConfig g_config;
    // define the g_kvDelegateCallback, used to get some information when open a kv store.
    DBStatus g_kvDelegateStatusForQuery = INVALID_ARGS;
    KvStoreDelegate *g_kvDelegatePtrForQuery = nullptr;
    // the type of g_kvDelegateCallback is function<void(DBStatus, KvStoreDelegate*)>
    auto g_kvDelegateCallbackForQuery = bind(&DistributedDBToolsUnitTest::KvStoreDelegateCallback, placeholders::_1,
        placeholders::_2, std::ref(g_kvDelegateStatusForQuery), std::ref(g_kvDelegatePtrForQuery));
    KvStoreNbDelegate *g_kvNbDelegatePtrForQuery = nullptr;
    auto g_kvNbDelegateCallbackForQuery = bind(&DistributedDBToolsUnitTest::KvStoreNbDelegateCallback,
        placeholders::_1, placeholders::_2, std::ref(g_kvDelegateStatusForQuery), std::ref(g_kvNbDelegatePtrForQuery));

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

    const string SCHEMA_STRING = "{\"SCHEMA_VERSION\":\"1.0\","
                                    "\"SCHEMA_MODE\":\"STRICT\","
                                    "\"SCHEMA_DEFINE\":{"
                                        "\"field_name1\":\"BOOL\","
                                        "\"field_name2\":\"BOOL\","
                                        "\"field_name3\":\"INTEGER, NOT NULL\","
                                        "\"field_name4\":\"LONG, DEFAULT 100\","
                                        "\"field_name5\":\"DOUBLE, NOT NULL, DEFAULT 3.14\","
                                        "\"field_name6\":\"STRING, NOT NULL, DEFAULT '3.1415'\","
                                        "\"field_name7\":\"LONG, DEFAULT 100\","
                                        "\"field_name8\":\"LONG, DEFAULT 100\","
                                        "\"field_name9\":\"LONG, DEFAULT 100\","
                                        "\"field_name10\":\"LONG, DEFAULT 100\""
                                    "},"
                                    "\"SCHEMA_INDEXES\":[\"$.field_name1\", \"$.field_name2\"]}";

    void GetSnapshotUnitTest()
    {
        g_kvDelegatePtr->GetKvStoreSnapshot(nullptr, g_snapshotDelegateCallback);
        EXPECT_TRUE(g_snapshotDelegateStatus == OK);
        ASSERT_TRUE(g_snapshotDelegatePtr != nullptr);
    }
#ifndef OMIT_JSON
    const int CIRCLE_COUNT = 3;
    void PutValidEntries1()
    {
        std::string validData = "{\"field_name1\":true,";
        validData += "\"field_name2\":true,";
        validData += "\"field_name3\":10,";
        validData += "\"field_name4\":20,";
        validData += "\"field_name5\":3.14,";
        validData += "\"field_name6\":\"3.1415\",";
        validData += "\"field_name7\":100,";
        validData += "\"field_name8\":100,";
        validData += "\"field_name9\":100,";
        validData += "\"field_name10\":100}";

        Value value(validData.begin(), validData.end());
        Key key;
        DistributedDBToolsUnitTest::GetRandomKeyValue(key);
        EXPECT_EQ(g_kvNbDelegatePtrForQuery->Put(key, value), OK);
    }
    void PutValidEntries2()
    {
        std::string validData = "{\"field_name1\":true,"
                                "\"field_name2\":false,"
                                "\"field_name3\":10,"
                                "\"field_name4\":20,"
                                "\"field_name5\":3.14,"
                                "\"field_name6\":\"3.1415\","
                                "\"field_name7\":100,"
                                "\"field_name8\":100,"
                                "\"field_name9\":100,"
                                "\"field_name10\":100}";
        Value value(validData.begin(), validData.end());
        Key key;
        DistributedDBToolsUnitTest::GetRandomKeyValue(key);
        EXPECT_EQ(g_kvNbDelegatePtrForQuery->Put(key, value), OK);
    }
    void PutValidEntries3()
    {
        std::string validData = "{\"field_name1\":true,"
                                "\"field_name2\":true,"
                                "\"field_name3\":20,"
                                "\"field_name4\":20,"
                                "\"field_name5\":3.14,"
                                "\"field_name6\":\"3.1415\","
                                "\"field_name7\":100,"
                                "\"field_name8\":100,"
                                "\"field_name9\":100,"
                                "\"field_name10\":100}";
        Value value(validData.begin(), validData.end());
        Key key;
        DistributedDBToolsUnitTest::GetRandomKeyValue(key);
        EXPECT_EQ(g_kvNbDelegatePtrForQuery->Put(key, value), OK);
    }
    void PutValidEntries4()
    {
        std::string validData = "{\"field_name1\":true,"
                                "\"field_name2\":true,"
                                "\"field_name3\":20,"
                                "\"field_name4\":2,"
                                "\"field_name5\":3.14,"
                                "\"field_name6\":\"3.1415\","
                                "\"field_name7\":100,"
                                "\"field_name8\":100,"
                                "\"field_name9\":100,"
                                "\"field_name10\":100}";
        Value value(validData.begin(), validData.end());
        Key key;
        DistributedDBToolsUnitTest::GetRandomKeyValue(key);
        EXPECT_EQ(g_kvNbDelegatePtrForQuery->Put(key, value), OK);
    }
    void PutValidEntries5()
    {
        std::string validData = "{\"field_name1\":true,"
                                "\"field_name2\":true,"
                                "\"field_name3\":20,"
                                "\"field_name4\":2,"
                                "\"field_name5\":3.15,"
                                "\"field_name6\":\"3.1415\","
                                "\"field_name7\":1001,"
                                "\"field_name8\":100,"
                                "\"field_name9\":100,"
                                "\"field_name10\":100}";
        Value value(validData.begin(), validData.end());
        Key key;
        DistributedDBToolsUnitTest::GetRandomKeyValue(key);
        EXPECT_EQ(g_kvNbDelegatePtrForQuery->Put(key, value), OK);
    }
    void PutValidEntries6()
    {
        std::string validData = "{\"field_name1\":true,"
                                "\"field_name2\":true,"
                                "\"field_name3\":20,"
                                "\"field_name4\":2,"
                                "\"field_name5\":3.15,"
                                "\"field_name6\":\"4.141\","
                                "\"field_name7\":1002,"
                                "\"field_name8\":100,"
                                "\"field_name9\":100,"
                                "\"field_name10\":100}";
        Value value(validData.begin(), validData.end());
        Key key;
        DistributedDBToolsUnitTest::GetRandomKeyValue(key);
        EXPECT_EQ(g_kvNbDelegatePtrForQuery->Put(key, value), OK);
    }
    void PutValidEntries7()
    {
        std::string validData = "{\"field_name1\":true,"
                                "\"field_name2\":true,"
                                "\"field_name3\":20,"
                                "\"field_name4\":2,"
                                "\"field_name5\":3.15,"
                                "\"field_name6\":\"4.141\","
                                "\"field_name7\":100,"
                                "\"field_name8\":200,"
                                "\"field_name9\":100,"
                                "\"field_name10\":100}";
        Value value(validData.begin(), validData.end());
        Key key;
        DistributedDBToolsUnitTest::GetRandomKeyValue(key);
        EXPECT_EQ(g_kvNbDelegatePtrForQuery->Put(key, value), OK);
    }
    void PutValidEntries8()
    {
        std::string validData = "{\"field_name1\":true,"
                                "\"field_name2\":true,"
                                "\"field_name3\":20,"
                                "\"field_name4\":2,"
                                "\"field_name5\":3.15,"
                                "\"field_name6\":\"4.141\","
                                "\"field_name7\":1009,"
                                "\"field_name8\":200,"
                                "\"field_name9\":100,"
                                "\"field_name10\":500}";
        Value value(validData.begin(), validData.end());
        Key key;
        DistributedDBToolsUnitTest::GetRandomKeyValue(key);
        EXPECT_EQ(g_kvNbDelegatePtrForQuery->Put(key, value), OK);
    }

    void PutValidEntries()
    {
        for (int i = 0; i < CIRCLE_COUNT; i++) {
            PutValidEntries1();
            PutValidEntries2();
            PutValidEntries3();
            PutValidEntries4();
            PutValidEntries5();
            PutValidEntries6();
            PutValidEntries7();
            PutValidEntries8();
        }
    }

    const std::string SCHEMA_DEFINE2 = "{\"SCHEMA_VERSION\":\"1.0\","
                                "\"SCHEMA_MODE\":\"STRICT\","
                                "\"SCHEMA_DEFINE\":{"
                                    "\"field_name1\":\"INTEGER\","
                                    "\"field_name2\":\"DOUBLE\""
                                "},"
                                "\"SCHEMA_INDEXES\":[\"$.field_name1\", \"$.field_name2\"]}";

    void PresetDataForPreifxAndOrderBy001()
    {
        std::string validData = "{\"field_name1\":1, \"field_name2\":1}";
        Value value(validData.begin(), validData.end());
        EXPECT_EQ(g_kvNbDelegatePtrForQuery->Put(KEY_1, value), OK);
        validData = "{\"field_name1\":1, \"field_name2\":2}";
        Value value2(validData.begin(), validData.end());
        EXPECT_EQ(g_kvNbDelegatePtrForQuery->Put(KEY_2, value2), OK);
        validData = "{\"field_name1\":2, \"field_name2\":3}";
        Value value3(validData.begin(), validData.end());
        EXPECT_EQ(g_kvNbDelegatePtrForQuery->Put(KEY_3, value3), OK);
        validData = "{\"field_name1\":2, \"field_name2\":4}";
        Value value4(validData.begin(), validData.end());
        EXPECT_EQ(g_kvNbDelegatePtrForQuery->Put(KEY_4, value4), OK);
        validData = "{\"field_name1\":3, \"field_name2\":5}";
        Value value5(validData.begin(), validData.end());
        EXPECT_EQ(g_kvNbDelegatePtrForQuery->Put(KEY_5, value5), OK);
    }
#endif
    void TestSnapshotCreateAndRelease()
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
#ifndef OMIT_JSON
    vector<Entry> PreDataForQueryByPreFixKey()
    {
        vector<Entry> res;
        for (int i = 0; i < 5; i++) { // rand num 5 for test
            Key key = DistributedDBToolsUnitTest::GetRandPrefixKey({'a', 'b'}, 1024); // rand num 1024 for test
            std::string validData = "{\"field_name1\":null, \"field_name2\":" + std::to_string(rand()) + "}";
            Value value(validData.begin(), validData.end());
            res.push_back({key, value});
        }

        for (int i = 0; i < 5; i++) { // rand num 5 for test
            Key key = DistributedDBToolsUnitTest::GetRandPrefixKey({'a', 'c'}, 1024); // rand num 1024 for test
            std::string validData = "{\"field_name1\":null, \"field_name2\":" + std::to_string(rand()) + "}";
            Value value(validData.begin(), validData.end());
            res.push_back({key, value});
        }
        return res;
    }

    static void PreDataForGroupTest()
    {
        std::string validData = "{\"field_name1\":1, \"field_name2\":1}";
        Value value(validData.begin(), validData.end());
        EXPECT_EQ(g_kvNbDelegatePtrForQuery->Put(KEY_1, value), OK);
        validData = "{\"field_name1\":2, \"field_name2\":2}";
        Value value2(validData.begin(), validData.end());
        EXPECT_EQ(g_kvNbDelegatePtrForQuery->Put(KEY_2, value2), OK);
        validData = "{\"field_name1\":3, \"field_name2\":3}";
        Value value3(validData.begin(), validData.end());
        EXPECT_EQ(g_kvNbDelegatePtrForQuery->Put(KEY_3, value3), OK);
        validData = "{\"field_name1\":4, \"field_name2\":4}";
        Value value4(validData.begin(), validData.end());
        EXPECT_EQ(g_kvNbDelegatePtrForQuery->Put(KEY_4, value4), OK);
        validData = "{\"field_name1\":5, \"field_name2\":5}";
        Value value5(validData.begin(), validData.end());
        EXPECT_EQ(g_kvNbDelegatePtrForQuery->Put(KEY_5, value5), OK);
    }
#endif
}

class DistributedDBInterfacesDataOperationTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
};

void DistributedDBInterfacesDataOperationTest::SetUpTestCase(void)
{
    DistributedDBToolsUnitTest::TestDirInit(g_testDir);
    g_config.dataDir = g_testDir;
    g_mgr.SetKvStoreConfig(g_config);
}

void DistributedDBInterfacesDataOperationTest::TearDownTestCase(void)
{
    if (DistributedDBToolsUnitTest::RemoveTestDbFiles(g_testDir) != 0) {
        LOGE("rm test db files error!");
    }
}

void DistributedDBInterfacesDataOperationTest::SetUp(void)
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

void DistributedDBInterfacesDataOperationTest::TearDown(void)
{
    if (g_kvDelegatePtr != nullptr && g_snapshotDelegatePtr != nullptr) {
        EXPECT_TRUE(g_kvDelegatePtr->ReleaseKvStoreSnapshot(g_snapshotDelegatePtr) == OK);
        g_snapshotDelegatePtr = nullptr;
    }

    if (g_kvDelegatePtr != nullptr) {
        EXPECT_EQ(g_mgr.CloseKvStore(g_kvDelegatePtr), OK);
        g_kvDelegatePtr = nullptr;
    }
}

/**
  * @tc.name: Put001
  * @tc.desc: Put a data(non-empty key, non-empty value) into an exist distributed db
  * @tc.type: FUNC
  * @tc.require: AR000CQDTM AR000CQS3Q
  * @tc.author: huangnaigu
  */
HWTEST_F(DistributedDBInterfacesDataOperationTest, Put001, TestSize.Level1)
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
HWTEST_F(DistributedDBInterfacesDataOperationTest, Put002, TestSize.Level1)
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
  * @tc.require: AR000CQDTM AR000CQS3Q
  * @tc.author: huangnaigu
  */
HWTEST_F(DistributedDBInterfacesDataOperationTest, Put003, TestSize.Level1)
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
  * @tc.desc: Put data into the local database
  * @tc.type: FUNC
  * @tc.require: AR000CQDVD AR000CQS3Q
  * @tc.author: huangnaigu
  */
HWTEST_F(DistributedDBInterfacesDataOperationTest, Put004, TestSize.Level1)
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
  * @tc.require: AR000C6TRV AR000CQDTM
  * @tc.author: huangnaigu
  */
HWTEST_F(DistributedDBInterfacesDataOperationTest, Clear001, TestSize.Level1)
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
    GetSnapshotUnitTest();
    g_snapshotDelegatePtr->Get(keyTmp, g_valueCallback);
    EXPECT_EQ(g_valueStatus, NOT_FOUND);
}

/**
  * @tc.name: PutBatch001
  * @tc.desc: Putbatch data into the local database
  * @tc.type: FUNC
  * @tc.require: AR000BVDFE AR000CQDVC
  * @tc.author: huangnaigu
  */
HWTEST_F(DistributedDBInterfacesDataOperationTest, PutBatch001, TestSize.Level1)
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
  * @tc.desc: PutBatch modified data into the local database
  * @tc.type: FUNC
  * @tc.require: AR000BVDFE AR000CQDVC
  * @tc.author: huangnaigu
  */
HWTEST_F(DistributedDBInterfacesDataOperationTest, PutBatch002, TestSize.Level1)
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
  * @tc.desc: Delete existed data from the local database
  * @tc.type: FUNC
  * @tc.require: AR000C6TRV AR000CQDTM
  * @tc.author: huangnaigu
  */
HWTEST_F(DistributedDBInterfacesDataOperationTest, Delete001, TestSize.Level1)
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
  * @tc.desc: Delete non-existed data from the local database
  * @tc.type: FUNC
  * @tc.require: AR000C6TRV AR000CQDTM
  * @tc.author: huangnaigu
  */
HWTEST_F(DistributedDBInterfacesDataOperationTest, Delete002, TestSize.Level1)
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
    EXPECT_TRUE(g_kvDelegatePtr->Delete(keyTmp) == OK);
}

/**
  * @tc.name: DeleteBatch001
  * @tc.desc: Delete the existed batch data from the local database
  * @tc.type: FUNC
  * @tc.require: AR000C6TRV AR000CQDTM
  * @tc.author: huangnaigu
  */
HWTEST_F(DistributedDBInterfacesDataOperationTest, DeleteBatch001, TestSize.Level1)
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
    g_snapshotDelegatePtr->Get(keyTmp, g_valueCallback);
    EXPECT_TRUE(g_valueStatus == INVALID_ARGS);
}

/**
  * @tc.name: DeleteBatch002
  * @tc.desc: Delete the non-existed batch data from the local database
  * @tc.type: FUNC
  * @tc.require: AR000C6TRV AR000CQDTM
  * @tc.author: huangnaigu
  */
HWTEST_F(DistributedDBInterfacesDataOperationTest, DeleteBatch002, TestSize.Level1)
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
  * @tc.require: AR000C6TUQ AR000CQDV3
  * @tc.author: huangnaigu
  */
HWTEST_F(DistributedDBInterfacesDataOperationTest, GetEntries001, TestSize.Level1)
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
  * @tc.require: AR000C6TUQ AR000CQDV3
  * @tc.author: huangnaigu
  */
HWTEST_F(DistributedDBInterfacesDataOperationTest, GetEntries002, TestSize.Level1)
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
  * @tc.require: AR000C6TUQ AR000CQDV3
  * @tc.author: huangnaigu
  */
HWTEST_F(DistributedDBInterfacesDataOperationTest, GetEntries003, TestSize.Level1)
{
    /**
     * @tc.steps: step1. Put batch data into the database.
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

    /**
     * @tc.steps: step2. Get all the data from the database using the empty prefix key.
     * @tc.expected: step2. GetEntries results OK and the entries size is the put batch data size.
     */
    Key keyPrefix;
    unsigned long matchSize = 10;
    GetSnapshotUnitTest();
    g_snapshotDelegatePtr->GetEntries(keyPrefix, g_entryVectorCallback);
    ASSERT_TRUE(g_matchSize == matchSize);
    EXPECT_TRUE(g_entryVectorStatus == OK);
}

/**
  * @tc.name: GetSnapshot001
  * @tc.desc: Get observer is empty, whether you get the snapshot.
  * @tc.type: FUNC
  * @tc.require: AR000BVRNF AR000CQDTI
  * @tc.author: wangbingquan
  */
HWTEST_F(DistributedDBInterfacesDataOperationTest, GetSnapshot001, TestSize.Level1)
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
HWTEST_F(DistributedDBInterfacesDataOperationTest, GetSnapshot002, TestSize.Level1)
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
HWTEST_F(DistributedDBInterfacesDataOperationTest, ReleaseSnapshot001, TestSize.Level1)
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
HWTEST_F(DistributedDBInterfacesDataOperationTest, ReleaseSnapshot002, TestSize.Level1)
{
    TestSnapshotCreateAndRelease();
}

/**
  * @tc.name: SetConflictResolutionPolicySuccessTest001
  * @tc.desc: Verify SetConflictResolutionPolicy() return OK with valid input.
  * @tc.type: FUNC
  * @tc.require: AR000CQE12
  * @tc.author: wumin
  */
HWTEST_F(DistributedDBInterfacesDataOperationTest, SetConflictResolutionPolicySuccessTest001, TestSize.Level1)
{
    /**
     * @tc.steps: step1. get g_kvDelegatePtr pointer
     * @tc.expected: step1. g_kvDelegatePtr is not nullptr
     */
    ASSERT_TRUE(g_kvDelegatePtr != nullptr);

    /**
     * @tc.steps: step2. invoke SetConflictResolutionPolicy() method by g_kvDelegatePtr
     * @tc.expected: step2. SetConflictResolutionPolicy() return OK
     */
    EXPECT_EQ(g_kvDelegatePtr->SetConflictResolutionPolicy(AUTO_LAST_WIN, nullptr), OK);
}

/**
  * @tc.name: SetConflictResolutionPolicyFailedTest001
  * @tc.desc: Verify SetConflictResolutionPolicy() return INVALID_ARGS with invalid input.
  * @tc.type: FUNC
  * @tc.require: AR000CQE12
  * @tc.author: wumin
  */
HWTEST_F(DistributedDBInterfacesDataOperationTest, SetConflictResolutionPolicyFailedTest001, TestSize.Level1)
{
    /**
     * @tc.steps: step1. get g_kvDelegatePtr pointer
     * @tc.expected: step1. g_kvDelegatePtr is not nullptr
     */
    ASSERT_TRUE(g_kvDelegatePtr != nullptr);

    /**
     * @tc.steps: step2. invoke SetConflictResolutionPolicy() method by g_kvDelegatePtr
     * @tc.expected: step2. SetConflictResolutionPolicy() return not OK
     */
    EXPECT_NE(g_kvDelegatePtr->SetConflictResolutionPolicy(CUSTOMER_RESOLUTION, nullptr), OK);
}
#ifndef OMIT_JSON
/**
  * @tc.name: GetEntriesWithQuery001
  * @tc.desc: check query_format.
  * @tc.type: FUNC
  * @tc.require: AR000DR9K7
  * @tc.author: weifeng
  */
HWTEST_F(DistributedDBInterfacesDataOperationTest, GetEntriesWithQuery001, TestSize.Level1)
{
    /**
     * @tc.steps: step1. get a non-schema store, Getentries with query
     * @tc.expected: step1. get store ok, Getentries return OK
     */
    KvStoreNbDelegate::Option option = {true, false, false};
    g_mgr.GetKvStore("GetEntriesWithQuery001_001", option, g_kvNbDelegateCallbackForQuery);
    ASSERT_TRUE(g_kvNbDelegatePtrForQuery != nullptr);
    EXPECT_TRUE(g_kvDelegateStatusForQuery == OK);
    Query query = Query::Select();
    std::vector<Entry> entries;
    DBStatus ret = g_kvNbDelegatePtrForQuery->GetEntries(query, entries);
    EXPECT_EQ(ret, NOT_FOUND);
    EXPECT_EQ(g_mgr.CloseKvStore(g_kvNbDelegatePtrForQuery), OK);
    EXPECT_TRUE(g_mgr.DeleteKvStore("GetEntriesWithQuery001_001") == OK);
    /**
     * @tc.steps: step2. get a schema store, Getentries with empty query
     * @tc.expected: step2. get store ok, Getentries return OK
     */
    option.schema = SCHEMA_STRING;
    g_mgr.GetKvStore("GetEntriesWithQuery001_002", option, g_kvNbDelegateCallbackForQuery);
    ASSERT_TRUE(g_kvNbDelegatePtrForQuery != nullptr);
    EXPECT_TRUE(g_kvDelegateStatusForQuery == OK);
    ret = g_kvNbDelegatePtrForQuery->GetEntries(query, entries);
    EXPECT_TRUE(ret == NOT_FOUND);
    /**
     * @tc.steps: step3. Getentries by query, query undefined field
     * @tc.expected: step3. Getentries return INVALID_QUERY_FIELD
     */
    query = query.EqualTo("$.field_name200", 10);
    ret = g_kvNbDelegatePtrForQuery->GetEntries(query, entries);
    EXPECT_EQ(ret, INVALID_QUERY_FIELD);
    /**
     * @tc.steps: step4. Getentries by query, query has invalid linker;
     * @tc.expected: step4. Getentries return INVALID_QUERY_FORMAT
     */
    Query invalidQuery1 = Query::Select().EqualTo("$.field_name1", true).And().Or();
    ret = g_kvNbDelegatePtrForQuery->GetEntries(invalidQuery1, entries);
    EXPECT_TRUE(ret == INVALID_QUERY_FORMAT);
    /**
     * @tc.steps: step5. Getentries by query, query has invalid linker;
     * @tc.expected: step5. Getentries return INVALID_QUERY_FORMAT
     */
    Query invalidQuery2 = Query::Select().And();
    ret = g_kvNbDelegatePtrForQuery->GetEntries(invalidQuery2, entries);
    EXPECT_TRUE(ret == INVALID_QUERY_FORMAT);
    /**
     * @tc.steps: step6. Getentries by query, query has invalid limit;
     * @tc.expected: step6. Getentries return INVALID_QUERY_FORMAT
     */
    Query invalidQuery3 = Query::Select().Limit(10, 0).EqualTo("$.field_name1", true);
    ret = g_kvNbDelegatePtrForQuery->GetEntries(invalidQuery3, entries);
    EXPECT_TRUE(ret == INVALID_QUERY_FORMAT);
    /**
     * @tc.steps: step7. Getentries by query, query has invalid orderby;
     * @tc.expected: step7. Getentries return INVALID_QUERY_FORMAT
     */
    Query invalidQuery4 = Query::Select().OrderBy("$.field_name1", true).EqualTo("field_name1", true);
    ret = g_kvNbDelegatePtrForQuery->GetEntries(invalidQuery4, entries);
    EXPECT_TRUE(ret == INVALID_QUERY_FORMAT);
    /**
     * @tc.steps: step8. Getentries by query, query has invalid orderby and limit;
     * @tc.expected: step8. Getentries return INVALID_QUERY_FORMAT
     */
    Query invalidQuery5 = Query::Select().Limit(10, 0).OrderBy("$.field_name1", true);
    ret = g_kvNbDelegatePtrForQuery->GetEntries(invalidQuery5, entries);
    EXPECT_TRUE(ret == INVALID_QUERY_FORMAT);
    /**
     * @tc.steps: step9. Getentries by query, query has invalid field type;
     * @tc.expected: step9. Getentries return OK
     */
    std::string queryType = "true";
    Query invalidQuery6 = Query::Select().EqualTo("$.field_name1", queryType);
    ret = g_kvNbDelegatePtrForQuery->GetEntries(invalidQuery6, entries);
    EXPECT_TRUE(ret == NOT_FOUND);

    Query invalidQuery7 = Query::Select().EqualTo("$.field_name1", queryType).IsNull("$.field_name1");
    ret = g_kvNbDelegatePtrForQuery->GetEntries(invalidQuery7, entries);
    EXPECT_TRUE(ret == INVALID_QUERY_FORMAT);

    EXPECT_EQ(g_mgr.CloseKvStore(g_kvNbDelegatePtrForQuery), OK);
    EXPECT_TRUE(g_mgr.DeleteKvStore("GetEntriesWithQuery001_002") == OK);
}

/**
  * @tc.name: GetEntriesWithQuery002
  * @tc.desc: GetEntries(const Query &query, std::vector<Entry> &entries) interface test.
  * @tc.type: FUNC
  * @tc.require: AR000DR9K7
  * @tc.author: weifeng
  */
HWTEST_F(DistributedDBInterfacesDataOperationTest, GetEntriesWithQuery002, TestSize.Level1)
{
    KvStoreNbDelegate::Option option = {true, false, false};
    option.schema = SCHEMA_STRING;
    g_mgr.GetKvStore("GetEntriesWithQuery002_002", option, g_kvNbDelegateCallbackForQuery);
    ASSERT_TRUE(g_kvNbDelegatePtrForQuery != nullptr);
    EXPECT_TRUE(g_kvDelegateStatusForQuery == OK);
    PutValidEntries();
    /**
     * @tc.steps: step1. Getentries by query, query is empty;
     * @tc.expected: step1. Getentries return OK, entries size == totalSize;
     */
    Query query = Query::Select();
    std::vector<Entry> entries;
    int ret = g_kvNbDelegatePtrForQuery->GetEntries(query, entries);
    EXPECT_EQ(ret, OK);
    EXPECT_EQ(entries.size(), 24ul);
    /**
     * @tc.steps: step2. Getentries by query, query is full-set;
     * @tc.expected: step2. Getentries return OK, ;
     */
    std::vector<int> inCondition = {1, 10, 100, 200};
    Query fullQuery = Query::Select().EqualTo("$.field_name1", true).And().NotEqualTo("$.field_name2", false).
        And().GreaterThanOrEqualTo("$.field_name3", 10).And().LessThan("$.field_name4", 10).And().
        Like("$.field_name6", "4%").And().In("$.field_name7", inCondition).OrderBy("$.field_name9").Limit(20);
    ret = g_kvNbDelegatePtrForQuery->GetEntries(fullQuery, entries);
    EXPECT_EQ(ret, OK);
    EXPECT_EQ(entries.size(), 3ul);
    EXPECT_EQ(g_mgr.CloseKvStore(g_kvNbDelegatePtrForQuery), OK);
    EXPECT_TRUE(g_mgr.DeleteKvStore("GetEntriesWithQuery002_002") == OK);
}

/**
  * @tc.name: GetEntriesWithQuery003
  * @tc.desc: GetEntries(const Query &query, std::vector<Entry> &entries) interface test.
  * @tc.type: FUNC
  * @tc.require: AR000DR9K7
  * @tc.author: weifeng
  */
HWTEST_F(DistributedDBInterfacesDataOperationTest, GetEntriesWithQuery003, TestSize.Level1)
{
    KvStoreNbDelegate::Option option = {true, false, false};
    option.schema = SCHEMA_STRING;
    g_mgr.GetKvStore("GetEntriesWithQuery003", option, g_kvNbDelegateCallbackForQuery);
    ASSERT_TRUE(g_kvNbDelegatePtrForQuery != nullptr);
    EXPECT_TRUE(g_kvDelegateStatusForQuery == OK);
    PutValidEntries();
    /**
     * @tc.steps: step1. Getentries by query, query is empty;
     * @tc.expected: step1. Getentries return OK, entries size == totalSize;
     */
    Query query = Query::Select();
    int count = 0;
    int ret = g_kvNbDelegatePtrForQuery->GetCount(query, count);
    EXPECT_TRUE(ret == OK);
    EXPECT_TRUE(count == 24);
    /**
     * @tc.steps: step2. Getentries by query, query is full-set;
     * @tc.expected: step2. Getentries return OK, ;
     */
    std::vector<int> inCondition = {1, 10, 100, 200};
    Query fullQuery = Query::Select().EqualTo("$.field_name1", true).And().NotEqualTo("$.field_name2", false).
        And().GreaterThan("$.field_name3", 10).And().LessThan("$.field_name4", 10).And().Like("$.field_name6", "4%").
        And().In("$.field_name7", inCondition);
    ret = g_kvNbDelegatePtrForQuery->GetCount(fullQuery, count);
    EXPECT_TRUE(ret == OK);
    EXPECT_TRUE(count == 3);
    EXPECT_EQ(g_mgr.CloseKvStore(g_kvNbDelegatePtrForQuery), OK);
    EXPECT_TRUE(g_mgr.DeleteKvStore("GetEntriesWithQuery003") == OK);
}

/**
  * @tc.name: GetEntriesWithQuery004
  * @tc.desc: GetEntries(const Query &query, std::vector<Entry> &entries) interface test.
  * @tc.type: FUNC
  * @tc.require: AR000DR9K7
  * @tc.author: weifeng
  */
HWTEST_F(DistributedDBInterfacesDataOperationTest, GetEntriesWithQuery004, TestSize.Level1)
{
    KvStoreNbDelegate::Option option = {true, false, false};
    option.schema = SCHEMA_STRING;
    g_mgr.GetKvStore("GetEntriesWithQuery004", option, g_kvNbDelegateCallbackForQuery);
    ASSERT_TRUE(g_kvNbDelegatePtrForQuery != nullptr);
    EXPECT_TRUE(g_kvDelegateStatusForQuery == OK);
    PutValidEntries();
    /**
     * @tc.steps: step1. Getentries by query, query is empty;
     * @tc.expected: step1. Getentries return OK, entries size == totalSize;
     */
    Query query = Query::Select();
    KvStoreResultSet *resultSet = nullptr;
    int ret = g_kvNbDelegatePtrForQuery->GetEntries(query, resultSet);
    ASSERT_TRUE(resultSet != nullptr);
    EXPECT_TRUE(ret == OK);
    EXPECT_EQ(resultSet->GetCount(), 24);
    EXPECT_EQ(resultSet->GetPosition(), -1);
    EXPECT_TRUE(!resultSet->IsFirst());
    EXPECT_TRUE(resultSet->MoveToFirst());
    EXPECT_TRUE(resultSet->IsFirst());
    EXPECT_EQ(g_kvNbDelegatePtrForQuery->CloseResultSet(resultSet), OK);
    EXPECT_TRUE(resultSet == nullptr);
    /**
     * @tc.steps: step2. Getentries by query, query is full-set;
     * @tc.expected: step2. Getentries return OK, ;
     */
    std::vector<int> inCondition = {1, 10, 100, 200};
    Query fullQuery = Query::Select().EqualTo("$.field_name1", true).And().NotEqualTo("$.field_name2", false).
        And().GreaterThan("$.field_name3", 10).And().LessThan("$.field_name4", 10).And().Like("$.field_name6", "4%").
        And().In("$.field_name7", inCondition).OrderBy("$.field_name9").Limit(20, 1);
    ret = g_kvNbDelegatePtrForQuery->GetEntries(fullQuery, resultSet);
    ASSERT_TRUE(resultSet != nullptr);
    EXPECT_EQ(ret, OK);
    EXPECT_EQ(resultSet->GetCount(), 2);
    EXPECT_EQ(g_kvNbDelegatePtrForQuery->CloseResultSet(resultSet), OK);
    EXPECT_TRUE(resultSet == nullptr);
    EXPECT_EQ(g_mgr.CloseKvStore(g_kvNbDelegatePtrForQuery), OK);
    EXPECT_TRUE(g_mgr.DeleteKvStore("GetEntriesWithQuery004") == OK);
}

/**
  * @tc.name: QueryIsNotNull001
  * @tc.desc: IsNotNull interface normal function
  * @tc.type: FUNC
  * @tc.require: AR000EPARK
  * @tc.author: sunpeng
  */
HWTEST_F(DistributedDBInterfacesDataOperationTest, QueryIsNotNull001, TestSize.Level1)
{
    KvStoreNbDelegate::Option option = {true, false, false};
    option.schema = SCHEMA_DEFINE2;
    g_mgr.GetKvStore("QueryIsNotNull001", option, g_kvNbDelegateCallbackForQuery);
    ASSERT_TRUE(g_kvNbDelegatePtrForQuery != nullptr);
    EXPECT_TRUE(g_kvDelegateStatusForQuery == OK);
    std::string validData = "{\"field_name1\":null, \"field_name2\":1}";
    Value value(validData.begin(), validData.end());
    EXPECT_EQ(g_kvNbDelegatePtrForQuery->Put(KEY_1, value), OK);
    validData = "{\"field_name1\":2, \"field_name2\":2}";
    Value value2(validData.begin(), validData.end());
    EXPECT_EQ(g_kvNbDelegatePtrForQuery->Put(KEY_2, value2), OK);
    /**
     * @tc.steps: step1. Get Query object by IsNotNull
     */
    Query query = Query::Select().IsNotNull("$.field_name1");
    /**
     * @tc.steps: step2. Use GetEntries get KV
     * @tc.expected: step2. Getentries return OK, Get K1V1;
     */
    std::vector<Entry> entries;
    int errCode = g_kvNbDelegatePtrForQuery->GetEntries(query, entries);
    EXPECT_EQ(errCode, OK);
    EXPECT_EQ(entries.size(), 1ul);
    EXPECT_EQ(entries[0].key, KEY_2);
    EXPECT_EQ(entries[0].value, value2);
    /**
     * @tc.steps: step3. Use GetCount to get number of item
     * @tc.expected: step3. Get count = 1;
     */
    int count = -1;
    g_kvNbDelegatePtrForQuery->GetCount(query, count);
    EXPECT_EQ(count, 1);
    /**
     * @tc.steps: step4. Use GetEntries to get resultSet
     * @tc.expected: step4. Getentries return OK, Get K1V1;
     */
    KvStoreResultSet *resultSet = nullptr;
    errCode = g_kvNbDelegatePtrForQuery->GetEntries(query, resultSet);
    ASSERT_TRUE(resultSet != nullptr);
    EXPECT_EQ(errCode, OK);
    EXPECT_EQ(resultSet->GetCount(), 1);
    EXPECT_EQ(g_kvNbDelegatePtrForQuery->CloseResultSet(resultSet), OK);
    EXPECT_TRUE(resultSet == nullptr);
    EXPECT_EQ(g_mgr.CloseKvStore(g_kvNbDelegatePtrForQuery), OK);
    EXPECT_TRUE(g_mgr.DeleteKvStore("QueryIsNotNull001") == OK);
}

/**
  * @tc.name: QueryPreFixKey001
  * @tc.desc: Normal function of query by prefix key
  * @tc.type: FUNC
  * @tc.require: AR000EPARK
  * @tc.author: sunpeng
  */
HWTEST_F(DistributedDBInterfacesDataOperationTest, QueryPreFixKey001, TestSize.Level1)
{
    KvStoreNbDelegate::Option option = {true, false, false};
    option.schema = SCHEMA_DEFINE2;
    g_mgr.GetKvStore("QueryPreFixKey001", option, g_kvNbDelegateCallbackForQuery);
    ASSERT_TRUE(g_kvNbDelegatePtrForQuery != nullptr);
    EXPECT_TRUE(g_kvDelegateStatusForQuery == OK);

    vector<Entry> entries = PreDataForQueryByPreFixKey();
    EXPECT_EQ(g_kvNbDelegatePtrForQuery->PutBatch(entries), OK);

    /**
     * @tc.steps: step1. Get Query object by PrefixKey ac
     */
    Query query = Query::Select().PrefixKey({'a', 'c'});

    /**
     * @tc.steps: step2. Use GetEnties to get same key prefix ac
     * @tc.expected: step2. Get count = 5, Key6~10
     */
    std::vector<Entry> entriesRes;
    int errCode = g_kvNbDelegatePtrForQuery->GetEntries(query, entriesRes);
    EXPECT_EQ(errCode, OK);
    EXPECT_EQ(entriesRes.size(), 5ul);
    for (size_t i = 0; i < entriesRes.size(); i++) {
        EXPECT_EQ(entriesRes[i].key.front(), 'a');
        EXPECT_EQ(entriesRes[i].key[1], 'c');
    }
    /**
     * @tc.steps: step3. Use GetCount to get number of item of this query object
     * @tc.expected: step3. Get count = 5
     */
    int count = -1;
    g_kvNbDelegatePtrForQuery->GetCount(query, count);
    EXPECT_EQ(count, 5);
    /**
     * @tc.steps: step4. Use GetEnties to get same key prefix ac of resultSet
     * @tc.expected: step4. Get resultSet of key6~10
     */
    KvStoreResultSet *resultSet = nullptr;
    errCode = g_kvNbDelegatePtrForQuery->GetEntries(query, resultSet);
    ASSERT_TRUE(resultSet != nullptr);
    EXPECT_EQ(errCode, OK);
    EXPECT_EQ(resultSet->GetCount(), 5);
    EXPECT_EQ(g_kvNbDelegatePtrForQuery->CloseResultSet(resultSet), OK);
    EXPECT_TRUE(resultSet == nullptr);

    /**
     * @tc.steps: step5. Get Query object by null PrefixKey
     */
    Query query1 = Query::Select().PrefixKey({});
    errCode = g_kvNbDelegatePtrForQuery->GetEntries(query1, entriesRes);
    /**
     * @tc.steps: step6. Use GetEnties and GetCount to null key prefix query object
     * @tc.expected: step6. Get all KV from database
     */
    EXPECT_EQ(errCode, OK);
    EXPECT_EQ(entriesRes.size(), 10ul);
    EXPECT_TRUE(DistributedDBToolsUnitTest::IsEntriesEqual(entries, entriesRes, true));

    errCode = g_kvNbDelegatePtrForQuery->GetEntries(query1, resultSet);
    ASSERT_TRUE(resultSet != nullptr);
    EXPECT_EQ(errCode, OK);
    EXPECT_EQ(resultSet->GetCount(), 10);
    EXPECT_EQ(g_kvNbDelegatePtrForQuery->CloseResultSet(resultSet), OK);
    EXPECT_TRUE(resultSet == nullptr);

    Query query2 = Query::Select().PrefixKey(Key(1025, 'a')); // 1025 over max key length 1 byte
    errCode = g_kvNbDelegatePtrForQuery->GetEntries(query2, entriesRes);
    EXPECT_EQ(errCode, INVALID_ARGS);

    EXPECT_EQ(g_mgr.CloseKvStore(g_kvNbDelegatePtrForQuery), OK);
    EXPECT_TRUE(g_mgr.DeleteKvStore("QueryPreFixKey001") == OK);
}

/**
  * @tc.name: QueryPreFixKey003
  * @tc.desc: For special key prefix combination condition of query
  * @tc.type: FUNC
  * @tc.require: AR000EPARK
  * @tc.author: sunpeng
  */
HWTEST_F(DistributedDBInterfacesDataOperationTest, QueryPreFixKey003, TestSize.Level1)
{
    KvStoreNbDelegate::Option option = {true, false, false};
    option.schema = SCHEMA_DEFINE2;
    g_mgr.GetKvStore("QueryPreFixKey003", option, g_kvNbDelegateCallbackForQuery);
    ASSERT_TRUE(g_kvNbDelegatePtrForQuery != nullptr);
    EXPECT_TRUE(g_kvDelegateStatusForQuery == OK);

    vector<Entry> entries = PreDataForQueryByPreFixKey();
    EXPECT_EQ(g_kvNbDelegatePtrForQuery->PutBatch(entries), OK);

    /**
     * @tc.steps: step1. Get Query object by double PrefixKey
     */
    Query query = Query::Select().PrefixKey({'a', 'c'}).PrefixKey({});
    std::vector<Entry> entriesRes;
    /**
     * @tc.steps: step2. Use GetEnties for double prefixkey query object
     * @tc.expected: step2. return INVALID_QUERY_FORMAT
     */
    int errCode = g_kvNbDelegatePtrForQuery->GetEntries(query, entriesRes);
    EXPECT_EQ(errCode, INVALID_QUERY_FORMAT);
    /**
     * @tc.steps: step3. Use GetEnties for double prefixkey query object to get resultSet
     * @tc.expected: step3. return INVALID_QUERY_FORMAT
     */
    KvStoreResultSet *resultSet = nullptr;
    errCode = g_kvNbDelegatePtrForQuery->GetEntries(query, resultSet);
    EXPECT_EQ(errCode, INVALID_QUERY_FORMAT);
    EXPECT_TRUE(resultSet == nullptr);
    /**
     * @tc.steps: step4. Get Query object by PrefixKey and orderBy
     */
    Query query1 = Query::Select().PrefixKey({'a', 'b'}).OrderBy("$.field_name1");
    /**
     * @tc.steps: step3. Use GetEnties and GetCount for this query object
     * @tc.expected: step3. Can get content by GetEntries, but GetCount can not use for query object include orderBy
     */
    errCode = g_kvNbDelegatePtrForQuery->GetEntries(query1, entriesRes);
    EXPECT_EQ(entriesRes.size(), 5ul);
    int count = -1;
    errCode = g_kvNbDelegatePtrForQuery->GetCount(query1, count);
    EXPECT_EQ(errCode, INVALID_QUERY_FORMAT);

    errCode = g_kvNbDelegatePtrForQuery->GetEntries(query1, resultSet);
    ASSERT_TRUE(resultSet != nullptr);
    EXPECT_EQ(errCode, OK);
    EXPECT_EQ(resultSet->GetCount(), 5);
    EXPECT_EQ(g_kvNbDelegatePtrForQuery->CloseResultSet(resultSet), OK);
    EXPECT_TRUE(resultSet == nullptr);

    EXPECT_EQ(g_mgr.CloseKvStore(g_kvNbDelegatePtrForQuery), OK);
    EXPECT_TRUE(g_mgr.DeleteKvStore("QueryPreFixKey003") == OK);
}

/**
  * @tc.name: QueryPreFixKey004
  * @tc.desc: Query a prefix that does not exist
  * @tc.type: FUNC
  * @tc.require: AR000EPARK
  * @tc.author: sunpeng
  */
HWTEST_F(DistributedDBInterfacesDataOperationTest, QueryPreFixKey004, TestSize.Level1)
{
    KvStoreNbDelegate::Option option = {true, false, false};
    option.schema = SCHEMA_DEFINE2;
    g_mgr.GetKvStore("QueryPreFixKey004", option, g_kvNbDelegateCallbackForQuery);
    ASSERT_TRUE(g_kvNbDelegatePtrForQuery != nullptr);
    EXPECT_TRUE(g_kvDelegateStatusForQuery == OK);

    vector<Entry> entries = PreDataForQueryByPreFixKey();
    EXPECT_EQ(g_kvNbDelegatePtrForQuery->PutBatch(entries), OK);
    /**
     * @tc.steps: step1. Get Query object by PrefixKey that does not exist
     */
    Query query = Query::Select().PrefixKey({'c'});
    /**
     * @tc.steps: step2. Use GetEnties and GetCount to get result
     * @tc.expected: step2. Return NOT_FOUND, get result OK, number of KV is 0
     */
    std::vector<Entry> entriesRes;
    int errCode = g_kvNbDelegatePtrForQuery->GetEntries(query, entriesRes);
    EXPECT_EQ(errCode, NOT_FOUND);

    int count = -1;
    g_kvNbDelegatePtrForQuery->GetCount(query, count);
    EXPECT_EQ(count, 0);

    KvStoreResultSet *resultSet = nullptr;
    errCode = g_kvNbDelegatePtrForQuery->GetEntries(query, resultSet);
    ASSERT_TRUE(resultSet != nullptr);
    EXPECT_EQ(errCode, OK);
    EXPECT_EQ(resultSet->GetCount(), 0);
    EXPECT_EQ(g_kvNbDelegatePtrForQuery->CloseResultSet(resultSet), OK);
    EXPECT_TRUE(resultSet == nullptr);

    EXPECT_EQ(g_mgr.CloseKvStore(g_kvNbDelegatePtrForQuery), OK);
    EXPECT_TRUE(g_mgr.DeleteKvStore("QueryPreFixKey004") == OK);
}

/**
  * @tc.name: QueryGroup001
  * @tc.desc: Query group nomal ability to change operation priority
  * @tc.type: FUNC
  * @tc.require: AR000EPARK
  * @tc.author: sunpeng
  */
HWTEST_F(DistributedDBInterfacesDataOperationTest, QueryGroup001, TestSize.Level1)
{
    KvStoreNbDelegate::Option option = {true, false, false};
    option.schema = SCHEMA_DEFINE2;
    g_mgr.GetKvStore("QueryGroup001", option, g_kvNbDelegateCallbackForQuery);
    ASSERT_TRUE(g_kvNbDelegatePtrForQuery != nullptr);
    EXPECT_TRUE(g_kvDelegateStatusForQuery == OK);

    PreDataForGroupTest();

    /**
     * @tc.steps: step1. Get Query object：
     * query:  <4 and =4 or >1
     * query1: (<4 and =4) or >1
     * query2: <4 and (=4 or >1)
     */
    Query query = Query::Select().LessThan("$.field_name1", 4).And().EqualTo("$.field_name1", 4).
        Or().GreaterThan("$.field_name1", 1);
    Query query1 = Query::Select().BeginGroup().LessThan("$.field_name1", 4).And().
        EqualTo("$.field_name1", 4).EndGroup().Or().GreaterThan("$.field_name1", 1);
    Query query2 = Query::Select().LessThan("$.field_name1", 4).And().BeginGroup().
        EqualTo("$.field_name1", 4).Or().GreaterThan("$.field_name1", 1).EndGroup();

    /**
     * @tc.steps: step2. Use GetEnties to get different result
     * @tc.expected: step2. Result:
     * query:  count = 4
     * query1: count = 4
     * query2: count = 2
     */
    std::vector<Entry> entries;
    int errCode = g_kvNbDelegatePtrForQuery->GetEntries(query, entries);
    EXPECT_EQ(errCode, OK);
    EXPECT_EQ(entries.size(), 4ul);
    int count = -1;
    g_kvNbDelegatePtrForQuery->GetCount(query, count);
    EXPECT_EQ(count, 4);

    errCode = g_kvNbDelegatePtrForQuery->GetEntries(query2, entries);
    EXPECT_EQ(errCode, OK);

    EXPECT_EQ(entries.size(), 2ul);
    g_kvNbDelegatePtrForQuery->GetCount(query2, count);
    EXPECT_EQ(count, 2);
    KvStoreResultSet *resultSet = nullptr;
    errCode = g_kvNbDelegatePtrForQuery->GetEntries(query2, resultSet);
    ASSERT_TRUE(resultSet != nullptr);
    EXPECT_EQ(errCode, OK);
    EXPECT_EQ(resultSet->GetCount(), 2);
    EXPECT_EQ(g_kvNbDelegatePtrForQuery->CloseResultSet(resultSet), OK);
    EXPECT_TRUE(resultSet == nullptr);

    errCode = g_kvNbDelegatePtrForQuery->GetEntries(query1, entries);
    EXPECT_EQ(errCode, OK);
    EXPECT_EQ(entries.size(), 4ul);
    g_kvNbDelegatePtrForQuery->GetCount(query1, count);
    EXPECT_EQ(count, 4);

    errCode = g_kvNbDelegatePtrForQuery->GetEntries(query1, resultSet);
    ASSERT_TRUE(resultSet != nullptr);
    EXPECT_EQ(errCode, OK);
    EXPECT_EQ(resultSet->GetCount(), 4);
    EXPECT_EQ(g_kvNbDelegatePtrForQuery->CloseResultSet(resultSet), OK);
    EXPECT_TRUE(resultSet == nullptr);

    EXPECT_EQ(g_mgr.CloseKvStore(g_kvNbDelegatePtrForQuery), OK);
    EXPECT_TRUE(g_mgr.DeleteKvStore("QueryGroup001") == OK);
}

/**
  * @tc.name: QueryGroup002
  * @tc.desc: Test for illegal Group query object
  * @tc.type: FUNC
  * @tc.require: AR000EPARK
  * @tc.author: sunpeng
  */
HWTEST_F(DistributedDBInterfacesDataOperationTest, QueryGroup002, TestSize.Level1)
{
    KvStoreNbDelegate::Option option = {true, false, false};
    option.schema = SCHEMA_DEFINE2;
    g_mgr.GetKvStore("QueryGroup002", option, g_kvNbDelegateCallbackForQuery);
    ASSERT_TRUE(g_kvNbDelegatePtrForQuery != nullptr);
    EXPECT_TRUE(g_kvDelegateStatusForQuery == OK);

    PreDataForGroupTest();

    /**
     * @tc.steps: step1. Get Query object：
     * query:  (<4 and (=4 or） >1)
     * query1: (<4 and =4 or >1
     * query2: <4 and =4) or >1
     * query3:  )<4 and =4( or >1
     * query4:  <4 (and = 4 or >1)
     */
    Query query = Query::Select().BeginGroup().LessThan("$.field_name1", 4).And().BeginGroup().
        EqualTo("$.field_name1", 4).Or().EndGroup().GreaterThan("$.field_name1", 1).EndGroup();
    Query query1 = Query::Select().BeginGroup().LessThan("$.field_name1", 4).And().
        EqualTo("$.field_name1", 4).Or().GreaterThan("$.field_name1", 1);
    Query query2 = Query::Select().LessThan("$.field_name1", 4).And().
        EqualTo("$.field_name1", 4).EndGroup().Or().GreaterThan("$.field_name1", 1);
    Query query3 = Query::Select().EndGroup().LessThan("$.field_name1", 4).And().
        EqualTo("$.field_name1", 4).BeginGroup().Or().GreaterThan("$.field_name1", 1);
    Query query4 = Query::Select().LessThan("$.field_name1", 4).BeginGroup().And().
        EqualTo("$.field_name1", 4).Or().GreaterThan("$.field_name1", 1).EndGroup();

    /**
     * @tc.steps: step2. Use GetEnties and GetCount to get result
     * @tc.expected: step2. All query object is illegal, reeturn INVALID_QUERY_FORMAT
     */
    std::vector<Entry> entries;
    int errCode = g_kvNbDelegatePtrForQuery->GetEntries(query, entries);
    EXPECT_EQ(errCode, INVALID_QUERY_FORMAT);

    errCode = g_kvNbDelegatePtrForQuery->GetEntries(query1, entries);
    EXPECT_EQ(errCode, INVALID_QUERY_FORMAT);
    errCode = g_kvNbDelegatePtrForQuery->GetEntries(query2, entries);
    EXPECT_EQ(errCode, INVALID_QUERY_FORMAT);
    errCode = g_kvNbDelegatePtrForQuery->GetEntries(query3, entries);
    EXPECT_EQ(errCode, INVALID_QUERY_FORMAT);
    errCode = g_kvNbDelegatePtrForQuery->GetEntries(query4, entries);
    EXPECT_EQ(errCode, INVALID_QUERY_FORMAT);

    KvStoreResultSet *resultSet = nullptr;
    errCode = g_kvNbDelegatePtrForQuery->GetEntries(query, resultSet);
    EXPECT_EQ(errCode, INVALID_QUERY_FORMAT);
    EXPECT_TRUE(resultSet == nullptr);

    errCode = g_kvNbDelegatePtrForQuery->GetEntries(query1, resultSet);
    EXPECT_EQ(errCode, INVALID_QUERY_FORMAT);
    EXPECT_TRUE(resultSet == nullptr);

    errCode = g_kvNbDelegatePtrForQuery->GetEntries(query2, resultSet);
    EXPECT_EQ(errCode, INVALID_QUERY_FORMAT);
    EXPECT_TRUE(resultSet == nullptr);

    errCode = g_kvNbDelegatePtrForQuery->GetEntries(query3, resultSet);
    EXPECT_EQ(errCode, INVALID_QUERY_FORMAT);
    EXPECT_TRUE(resultSet == nullptr);

    errCode = g_kvNbDelegatePtrForQuery->GetEntries(query4, resultSet);
    EXPECT_EQ(errCode, INVALID_QUERY_FORMAT);
    EXPECT_TRUE(resultSet == nullptr);

    EXPECT_EQ(g_mgr.CloseKvStore(g_kvNbDelegatePtrForQuery), OK);
    EXPECT_TRUE(g_mgr.DeleteKvStore("QueryGroup002") == OK);
}

/**
  * @tc.name: QueryGroup003
  * @tc.desc: Query expressions containing nested parentheses
  * @tc.type: FUNC
  * @tc.require: AR000EPARK
  * @tc.author: sunpeng
  */
HWTEST_F(DistributedDBInterfacesDataOperationTest, QueryGroup003, TestSize.Level1)
{
    KvStoreNbDelegate::Option option = {true, false, false};
    option.schema = SCHEMA_DEFINE2;
    g_mgr.GetKvStore("QueryGroup003", option, g_kvNbDelegateCallbackForQuery);
    ASSERT_TRUE(g_kvNbDelegatePtrForQuery != nullptr);
    EXPECT_TRUE(g_kvDelegateStatusForQuery == OK);

    PreDataForGroupTest();

    /**
     * @tc.steps: step1. Get Query object for  (<=5 and (=4 or >1) and <3)
     */
    Query query = Query::Select().BeginGroup().LessThan("$.field_name1", 5).And().BeginGroup().
        EqualTo("$.field_name1", 4).Or().GreaterThan("$.field_name1", 1).EndGroup().And().
        LessThan("$.field_name1", 3).EndGroup();

    /**
     * @tc.steps: step2. Use GetEnties and GetCount to get result
     * @tc.expected: step2. reeturn OK, count = 1
     */
    std::vector<Entry> entries;
    int errCode = g_kvNbDelegatePtrForQuery->GetEntries(query, entries);
    EXPECT_EQ(errCode, OK);
    EXPECT_EQ(entries.size(), 1ul);
    int count = -1;
    g_kvNbDelegatePtrForQuery->GetCount(query, count);
    EXPECT_EQ(count, 1);

    KvStoreResultSet *resultSet = nullptr;
    errCode = g_kvNbDelegatePtrForQuery->GetEntries(query, resultSet);
    ASSERT_TRUE(resultSet != nullptr);
    EXPECT_EQ(errCode, OK);
    EXPECT_EQ(resultSet->GetCount(), 1);
    EXPECT_EQ(g_kvNbDelegatePtrForQuery->CloseResultSet(resultSet), OK);
    EXPECT_TRUE(resultSet == nullptr);

    EXPECT_EQ(g_mgr.CloseKvStore(g_kvNbDelegatePtrForQuery), OK);
    EXPECT_TRUE(g_mgr.DeleteKvStore("QueryGroup003") == OK);
}

/**
  * @tc.name: multiOrderBy001
  * @tc.desc: Test multiple orderby conditions together to query
  * @tc.type: FUNC
  * @tc.require: AR000DR9K7
  * @tc.author: sunpeng
  */
HWTEST_F(DistributedDBInterfacesDataOperationTest, multiOrderBy001, TestSize.Level1)
{
    KvStoreNbDelegate::Option option = {true, false, false};
    option.schema = SCHEMA_DEFINE2;
    g_mgr.GetKvStore("multiOrderBy001", option, g_kvNbDelegateCallbackForQuery);
    ASSERT_TRUE(g_kvNbDelegatePtrForQuery != nullptr);
    EXPECT_TRUE(g_kvDelegateStatusForQuery == OK);

    PreDataForGroupTest();

    Query query = Query::Select().PrefixKey({}).OrderBy("$.field_name1").OrderBy("$.field_name1");
    std::vector<Entry> entries;
    int errCode = g_kvNbDelegatePtrForQuery->GetEntries(query, entries);
    EXPECT_EQ(errCode, OK);

    errCode = g_kvNbDelegatePtrForQuery->GetEntries(query.Limit(2, 2), entries);
    EXPECT_EQ(errCode, OK);

    Query query1 = Query::Select().PrefixKey({}).Limit(2, 2).OrderBy("$.field_name1").OrderBy("$.field_name1");
    errCode = g_kvNbDelegatePtrForQuery->GetEntries(query1, entries);
    EXPECT_EQ(errCode, INVALID_QUERY_FORMAT);

    Query query2 = Query::Select().PrefixKey({}).OrderBy("$.field_name1").Limit(2, 2).OrderBy("$.field_name1");
    KvStoreResultSet *resultSet = nullptr;
    errCode = g_kvNbDelegatePtrForQuery->GetEntries(query2, resultSet);
    EXPECT_EQ(errCode, INVALID_QUERY_FORMAT);

    Query query3 = Query::Select().PrefixKey({}).OrderBy("$.field_name1").
        OrderBy("$.field_name1").OrderBy("$.field_name1");
    errCode = g_kvNbDelegatePtrForQuery->GetEntries(query3, resultSet);
    EXPECT_EQ(errCode, OK);
    EXPECT_EQ(g_kvNbDelegatePtrForQuery->CloseResultSet(resultSet), OK);
    EXPECT_TRUE(resultSet == nullptr);

    EXPECT_EQ(g_mgr.CloseKvStore(g_kvNbDelegatePtrForQuery), OK);
    EXPECT_TRUE(g_mgr.DeleteKvStore("multiOrderBy001") == OK);
}

/**
  * @tc.name: multiOrderBy001
  * @tc.desc: For multiple order query.
  * @tc.type: FUNC
  * @tc.require: AR000EPARK
  * @tc.author: sunpeng
  */
HWTEST_F(DistributedDBInterfacesDataOperationTest, PreifxAndOrderBy001, TestSize.Level1)
{
    KvStoreNbDelegate::Option option = {true, false, false};
    option.schema = SCHEMA_DEFINE2;
    g_mgr.GetKvStore("PreifxAndOrderBy001", option, g_kvNbDelegateCallbackForQuery);
    ASSERT_TRUE(g_kvNbDelegatePtrForQuery != nullptr);
    EXPECT_TRUE(g_kvDelegateStatusForQuery == OK);

    PresetDataForPreifxAndOrderBy001();

    Query query = Query::Select().PrefixKey({}).OrderBy("$.field_name1", false);
    std::vector<Entry> entriesRes;
    int errCode = g_kvNbDelegatePtrForQuery->GetEntries(query, entriesRes);
    EXPECT_EQ(errCode, OK);
    ASSERT_EQ(entriesRes.size(), 5ul);
    EXPECT_EQ(entriesRes[0].key, KEY_5);
    EXPECT_EQ(entriesRes[1].key, KEY_3);
    EXPECT_EQ(entriesRes[2].key, KEY_4);
    EXPECT_EQ(entriesRes[3].key, KEY_1);
    EXPECT_EQ(entriesRes[4].key, KEY_2);

    Query query1 = Query::Select().OrderBy("$.field_name1", false);
    errCode = g_kvNbDelegatePtrForQuery->GetEntries(query1, entriesRes);
    ASSERT_EQ(entriesRes.size(), 5ul);
    EXPECT_EQ(entriesRes[0].key, KEY_5);
    EXPECT_EQ(entriesRes[1].key, KEY_4);
    EXPECT_EQ(entriesRes[2].key, KEY_3);
    EXPECT_EQ(entriesRes[3].key, KEY_2);
    EXPECT_EQ(entriesRes[4].key, KEY_1);

    Query query2 = Query::Select().PrefixKey({}).OrderBy("$.field_name1", false).OrderBy("$.field_name2", false);
    errCode = g_kvNbDelegatePtrForQuery->GetEntries(query2, entriesRes);
    ASSERT_EQ(entriesRes.size(), 5ul);
    EXPECT_EQ(entriesRes[0].key, KEY_5);
    EXPECT_EQ(entriesRes[1].key, KEY_4);
    EXPECT_EQ(entriesRes[2].key, KEY_3);
    EXPECT_EQ(entriesRes[3].key, KEY_2);
    EXPECT_EQ(entriesRes[4].key, KEY_1);

    EXPECT_EQ(g_mgr.CloseKvStore(g_kvNbDelegatePtrForQuery), OK);
    EXPECT_TRUE(g_mgr.DeleteKvStore("PreifxAndOrderBy001") == OK);
}

/**
  * @tc.name: PrefixAndOther001
  * @tc.desc: Combination of prefix query and logical filtering
  * @tc.type: FUNC
  * @tc.require: AR000EPARK
  * @tc.author: sunpeng
  */
HWTEST_F(DistributedDBInterfacesDataOperationTest, PrefixAndOther001, TestSize.Level1)
{
    KvStoreNbDelegate::Option option = {true, false, false};
    option.schema = SCHEMA_DEFINE2;
    g_mgr.GetKvStore("PrefixAndOther001", option, g_kvNbDelegateCallbackForQuery);
    ASSERT_TRUE(g_kvNbDelegatePtrForQuery != nullptr);
    EXPECT_TRUE(g_kvDelegateStatusForQuery == OK);

    PresetDataForPreifxAndOrderBy001();

    std::vector<Entry> entriesRes;
    Query query1 = Query::Select().EqualTo("$.field_name1", 1).PrefixKey({});
    int errCode = g_kvNbDelegatePtrForQuery->GetEntries(query1, entriesRes);
    EXPECT_EQ(errCode, OK);
    query1 = Query::Select().PrefixKey({}).EqualTo("$.field_name1", 1);
    errCode = g_kvNbDelegatePtrForQuery->GetEntries(query1, entriesRes);
    EXPECT_EQ(errCode, OK);

    query1 = Query::Select().EqualTo("$.field_name1", 1).PrefixKey({}).And().EqualTo("$.field_name1", 1);
    errCode = g_kvNbDelegatePtrForQuery->GetEntries(query1, entriesRes);
    EXPECT_EQ(errCode, OK);

    query1 = Query::Select().EqualTo("$.field_name1", 1).PrefixKey({}).And().EqualTo("$.field_name1", 2);
    errCode = g_kvNbDelegatePtrForQuery->GetEntries(query1, entriesRes);
    EXPECT_EQ(errCode, NOT_FOUND);

    EXPECT_EQ(g_mgr.CloseKvStore(g_kvNbDelegatePtrForQuery), OK);
    EXPECT_TRUE(g_mgr.DeleteKvStore("PrefixAndOther001") == OK);
}
#endif