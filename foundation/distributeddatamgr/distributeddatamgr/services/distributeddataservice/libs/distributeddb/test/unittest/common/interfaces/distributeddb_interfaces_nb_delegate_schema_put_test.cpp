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

#ifndef OMIT_JSON
#include <gtest/gtest.h>

#include <string>
#include <functional>

#include "distributeddb_tools_unit_test.h"
#include "kv_store_delegate_manager.h"
#include "kv_store_nb_delegate.h"
#include "query.h"

using namespace testing::ext;
using namespace DistributedDB;
using namespace DistributedDBUnitTest;

namespace {
    const std::string VALID_SCHEMA_STRICT_DEFINE = "{\"SCHEMA_VERSION\":\"1.0\","
        "\"SCHEMA_MODE\":\"STRICT\","
        "\"SCHEMA_DEFINE\":{"
            "\"field_name1\":\"BOOL\","
            "\"field_name2\":\"INTEGER, NOT NULL\""
        "},"
        "\"SCHEMA_INDEXES\":[\"$.field_name1\"]}";
    const std::string VALID_SCHEMA_COMPA_DEFINE = "{\"SCHEMA_VERSION\":\"1.0\","
        "\"SCHEMA_MODE\":\"COMPATIBLE\","
        "\"SCHEMA_DEFINE\":{"
            "\"field_name1\":\"BOOL\","
            "\"field_name2\":\"INTEGER, NOT NULL\""
        "},"
        "\"SCHEMA_INDEXES\":[\"$.field_name1\"]}";
    // define some variables to init a KvStoreDelegateManager object.
    KvStoreDelegateManager g_mgr("app0", "user0");
    std::string g_testDir;
    KvStoreConfig g_config;
    std::string g_storeName = "schema_put_test";

    // define the g_kvNbDelegateCallback, used to get some information when open a kv store.
    DBStatus g_kvDelegateStatus = INVALID_ARGS;
    KvStoreNbDelegate *g_kvStore = nullptr;
    CipherPassword g_passwd;
    KvStoreNbDelegate::Option g_strictOpt = {true, false, false, CipherType::DEFAULT, g_passwd,
        VALID_SCHEMA_STRICT_DEFINE};
    KvStoreNbDelegate::Option g_compOpt = {true, false, false, CipherType::DEFAULT, g_passwd,
        VALID_SCHEMA_COMPA_DEFINE};

    void KvStoreNbDelegateCallback(
        DBStatus statusSrc, KvStoreNbDelegate* kvStoreSrc, DBStatus* statusDst, KvStoreNbDelegate** kvStoreDst)
    {
        *statusDst = statusSrc;
        *kvStoreDst = kvStoreSrc;
    }

    // the type of g_kvNbDelegateCallback is function<void(DBStatus, KvStoreDelegate*)>
    auto g_kvNbDelegateCallback = std::bind(&KvStoreNbDelegateCallback, std::placeholders::_1,
        std::placeholders::_2, &g_kvDelegateStatus, &g_kvStore);

    void CheckPutSchemaData(KvStoreNbDelegate *kvStore)
    {
        ASSERT_NE(kvStore, nullptr);
        Key key;
        DistributedDBToolsUnitTest::GetRandomKeyValue(key);
        /**
         * @tc.steps:step1. Put one data whose value has less fields than the schema(less value is not null).
         * @tc.expected: step1. return CONSTRAIN_VIOLATION.
         */
        std::string lessData = "{\"field_name1\":true}";
        Value value(lessData.begin(), lessData.end());
        EXPECT_EQ(g_kvStore->Put(key, value), CONSTRAIN_VIOLATION);

        /**
         * @tc.steps:step2. Put one data whose value has different fields with the schema(less value is not null).
         * @tc.expected: step2. return CONSTRAIN_VIOLATION.
         */
        DistributedDBToolsUnitTest::GetRandomKeyValue(key);
        std::string filedDiffData = "{\"field_name1\":true,\"field_name3\":10}";
        value.assign(filedDiffData.begin(), filedDiffData.end());
        EXPECT_EQ(kvStore->Put(key, value), CONSTRAIN_VIOLATION);

        /**
         * @tc.steps:step3. Put one data whose value has different type with the schema.
         * @tc.expected: step3. return INVALID_FIELD_TYPE.
         */
        std::string typeDiffData = "{\"field_name1\":30,\"field_name2\":10}";
        value.assign(typeDiffData.begin(), typeDiffData.end());
        EXPECT_EQ(kvStore->Put(key, value), INVALID_FIELD_TYPE);

        /**
         * @tc.steps:step4. Put one data whose value has constrain violation with the schema.
         * @tc.expected: step4. return CONSTRAIN_VIOLATION.
         */
        std::string constrainDiffData = "{\"field_name1\":false,\"field_name2\":null}";
        value.assign(constrainDiffData.begin(), constrainDiffData.end());
        EXPECT_EQ(kvStore->Put(key, value), CONSTRAIN_VIOLATION);

        /**
         * @tc.steps:step5. Put one data whose value has invalid json.
         * @tc.expected: step5. return INVALID_FORMAT.
         */
        std::string invalidJsonData = "{\"field_name1\":false,\"field_name2\":10";
        value.assign(invalidJsonData.begin(), invalidJsonData.end());
        EXPECT_EQ(kvStore->Put(key, value), INVALID_FORMAT);

        /**
         * @tc.steps:step6. Put one data whose value is empty.
         * @tc.expected: step6. return INVALID_FORMAT.
         */
        value.clear();
        EXPECT_EQ(kvStore->Put(key, value), INVALID_FORMAT);

        /**
         * @tc.steps:step7. Put one data whose value is match with the schema.
         * @tc.expected: step7. return INVALID_FORMAT.
         */
        std::string validJsonData = "{\"field_name1\":false,\"field_name2\":10}";
        value.assign(validJsonData.begin(), validJsonData.end());
        EXPECT_EQ(kvStore->Put(key, value), OK);
    }

    void CheckPutBatchSchemaData(KvStoreNbDelegate *kvStore)
    {
        ASSERT_NE(kvStore, nullptr);
        /**
         * @tc.steps:step1. Put the batch data, one data is invalid.
         * @tc.expected: step1. return INVALID_FORMAT.
         */
        Key key;
        DistributedDBToolsUnitTest::GetRandomKeyValue(key);
        std::string invalidData = "{\"field_name1\":true, \"field_name2\":null}";
        Value value(invalidData.begin(), invalidData.end());
        std::vector<Entry> entries;
        entries.push_back({key, value});

        DistributedDBToolsUnitTest::GetRandomKeyValue(key);
        std::string validData = "{\"field_name1\":true, \"field_name2\":0}";
        value.assign(validData.begin(), validData.end());
        entries.push_back({key, value});

        EXPECT_NE(kvStore->PutBatch(entries), INVALID_FORMAT);

        entries.clear();
        entries.push_back({key, value});

        /**
         * @tc.steps:step2. Put the batch data, both valid.
         * @tc.expected: step2. return OK.
         */
        DistributedDBToolsUnitTest::GetRandomKeyValue(key);
        validData = "{\"field_name1\":null, \"field_name2\":30}";
        value.assign(validData.begin(), validData.end());
        entries.push_back({key, value});

        EXPECT_EQ(kvStore->PutBatch(entries), OK);
    }
}
class DistributedDBInterfacesNBDelegateSchemaPutTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
};

void DistributedDBInterfacesNBDelegateSchemaPutTest::SetUpTestCase(void)
{
    DistributedDBToolsUnitTest::TestDirInit(g_testDir);
    g_config.dataDir = g_testDir;
    g_mgr.SetKvStoreConfig(g_config);
}

void DistributedDBInterfacesNBDelegateSchemaPutTest::TearDownTestCase(void)
{
    if (DistributedDBToolsUnitTest::RemoveTestDbFiles(g_testDir) != 0) {
        LOGE("rm test db files error!");
    }
}

void DistributedDBInterfacesNBDelegateSchemaPutTest::SetUp(void)
{}

void DistributedDBInterfacesNBDelegateSchemaPutTest::TearDown(void)
{
    EXPECT_EQ(g_mgr.CloseKvStore(g_kvStore), OK);
    EXPECT_EQ(g_mgr.DeleteKvStore(g_storeName), OK);
    g_kvStore = nullptr;
    g_kvDelegateStatus = INVALID_ARGS;
}

/**
  * @tc.name: PutValueStrictSchemaCheck001
  * @tc.desc: Check the value in the strict schema mode.
  * @tc.type: FUNC
  * @tc.require: AR000DR9K5
  * @tc.author: wangbingquan
  */
HWTEST_F(DistributedDBInterfacesNBDelegateSchemaPutTest, PutValueStrictSchemaCheck001, TestSize.Level1)
{
    g_mgr.GetKvStore(g_storeName, g_strictOpt, g_kvNbDelegateCallback);
    ASSERT_TRUE(g_kvStore != nullptr);
    EXPECT_TRUE(g_kvDelegateStatus == OK);

    /**
     * @tc.steps:step1. Put one data whose value has more fields than the schema.
     * @tc.expected: step1. return INVALID_VALUE_FIELDS.
     */
    Key key;
    DistributedDBToolsUnitTest::GetRandomKeyValue(key);
    std::string moreData = "{\"field_name1\":true,\"field_name2\":10,\"field_name3\":10}";
    Value value(moreData.begin(), moreData.end());
    EXPECT_EQ(g_kvStore->Put(key, value), INVALID_VALUE_FIELDS);
    /**
     * @tc.steps:step2. Put the data whose value is mismatch with the schema.
     * @tc.expected: step2. return not OK.
     */
    CheckPutSchemaData(g_kvStore);
    CheckPutBatchSchemaData(g_kvStore);
}

/**
  * @tc.name: PutValueReadOnlyCheck001
  * @tc.desc: Test writing the data into the no-schema kvStore which has schema originally.
  * @tc.type: FUNC
  * @tc.require: AR000DR9K5
  * @tc.author: wangbingquan
  */
HWTEST_F(DistributedDBInterfacesNBDelegateSchemaPutTest, PutValueCompaSchemaCheck001, TestSize.Level1)
{
    g_mgr.GetKvStore(g_storeName, g_compOpt, g_kvNbDelegateCallback);
    ASSERT_TRUE(g_kvStore != nullptr);
    EXPECT_TRUE(g_kvDelegateStatus == OK);
    /**
     * @tc.steps:step1. Put one data whose value has more fields than the schema.
     * @tc.expected: step1. return OK.
     */
    Key key;
    DistributedDBToolsUnitTest::GetRandomKeyValue(key);
    std::string moreData = "{\"field_name1\":true,\"field_name2\":10,\"field_name3\":10}";
    Value value(moreData.begin(), moreData.end());
    EXPECT_EQ(g_kvStore->Put(key, value), OK);
    /**
     * @tc.steps:step2. Put the data whose value is mismatch with the schema.
     * @tc.expected: step2. return not OK.
     */
    CheckPutSchemaData(g_kvStore);
    CheckPutBatchSchemaData(g_kvStore);
}

/**
  * @tc.name: PutValueReadOnlyCheck001
  * @tc.desc: Test writing the data into the no-schema kvStore which has schema originally.
  * @tc.type: FUNC
  * @tc.require: AR000DR9K5
  * @tc.author: wangbingquan
  */
HWTEST_F(DistributedDBInterfacesNBDelegateSchemaPutTest, PutValueReadOnlyCheck001, TestSize.Level1)
{
    /**
     * @tc.steps:step1. Open the kv store with valid schema, and close it.
     */
    g_mgr.GetKvStore(g_storeName, g_compOpt, g_kvNbDelegateCallback);
    ASSERT_TRUE(g_kvStore != nullptr);
    EXPECT_TRUE(g_kvDelegateStatus == OK);
    EXPECT_EQ(g_mgr.CloseKvStore(g_kvStore), OK);
    /**
     * @tc.steps:step2. Open the kv store with no schema.
     * @tc.expected: step2. return OK.
     */
    DistributedDB::KvStoreNbDelegate::Option option = g_compOpt;
    option.schema.clear();
    g_mgr.GetKvStore(g_storeName, option, g_kvNbDelegateCallback);
    ASSERT_TRUE(g_kvStore != nullptr);
    EXPECT_TRUE(g_kvDelegateStatus == OK);
    /**
     * @tc.steps:step3. Put the data.
     * @tc.expected: step3. return READ_ONLY.
     */
    Key key;
    DistributedDBToolsUnitTest::GetRandomKeyValue(key);
    std::string valueData = "{\"field_name1\":true,\"field_name2\":20}";
    Value value(valueData.begin(), valueData.end());
    EXPECT_EQ(g_kvStore->Put(key, value), READ_ONLY);
}

/**
  * @tc.name: QueryDeleted001
  * @tc.desc: Test the query in the deleted scene.
  * @tc.type: FUNC
  * @tc.require: AR000DR9K5
  * @tc.author: wangbingquan
  */
HWTEST_F(DistributedDBInterfacesNBDelegateSchemaPutTest, QueryDeleted001, TestSize.Level1)
{
    g_mgr.GetKvStore(g_storeName, g_strictOpt, g_kvNbDelegateCallback);
    ASSERT_TRUE(g_kvStore != nullptr);
    EXPECT_TRUE(g_kvDelegateStatus == OK);

    /**
     * @tc.steps:step1. Put 2 schema data.
     * @tc.expected: step1. return OK.
     */
    Key key1;
    std::string valueData = "{\"field_name1\":true,\"field_name2\":1}";
    Value value(valueData.begin(), valueData.end());
    DistributedDBToolsUnitTest::GetRandomKeyValue(key1);
    EXPECT_EQ(g_kvStore->Put(key1, value), OK);

    Key key2;
    valueData = "{\"field_name1\":true,\"field_name2\":2}";
    value.assign(valueData.begin(), valueData.end());
    DistributedDBToolsUnitTest::GetRandomKeyValue(key2);
    EXPECT_EQ(g_kvStore->Put(key2, value), OK);

    /**
     * @tc.steps:step2. Get the data through the query condition where the field value is 1.
     * @tc.expected: step2. GetEntries return OK, and the data num is 1.
     */
    std::vector<Entry> entries;
    KvStoreResultSet *resultSet = nullptr;
    Query query = Query::Select().EqualTo("$.field_name2", 1);
    EXPECT_EQ(g_kvStore->GetEntries(query, entries), OK);
    EXPECT_EQ(g_kvStore->GetEntries(query, resultSet), OK);
    ASSERT_NE(resultSet, nullptr);
    EXPECT_EQ(resultSet->GetCount(), 1);
    int count = 0;
    EXPECT_EQ(g_kvStore->GetCount(query, count), OK);
    EXPECT_EQ(count, 1);
    EXPECT_EQ(g_kvStore->CloseResultSet(resultSet), OK);

    /**
     * @tc.steps:step3. Delete the data whose field value is 1.
     */
    EXPECT_EQ(g_kvStore->Delete(key1), OK);

    /**
     * @tc.steps:step4. Get the data whose field value is 1.
     * @tc.expected: step4. GetEntries return NOT_FOUND, and the data num is 0.
     */
    EXPECT_EQ(g_kvStore->GetEntries(query, entries), NOT_FOUND);
    EXPECT_EQ(g_kvStore->GetCount(query, count), NOT_FOUND);
    EXPECT_EQ(g_kvStore->GetEntries(query, resultSet), OK);
    ASSERT_NE(resultSet, nullptr);
    EXPECT_EQ(resultSet->GetCount(), 0);
    EXPECT_EQ(g_kvStore->CloseResultSet(resultSet), OK);
}
#endif