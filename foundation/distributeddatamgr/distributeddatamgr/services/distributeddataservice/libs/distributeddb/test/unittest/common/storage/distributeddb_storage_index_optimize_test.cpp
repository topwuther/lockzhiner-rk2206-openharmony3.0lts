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

#include "sqlite_import.h"
#include "platform_specific.h"
#include "types.h"
#include "db_common.h"
#include "db_constant.h"
#include "distributeddb_tools_unit_test.h"
#include "distributeddb_data_generate_unit_test.h"
#include "log_print.h"

using namespace testing::ext;
using namespace DistributedDB;
using namespace DistributedDBUnitTest;

namespace {
    std::string g_testDir;
    std::string g_identifier;

    KvStoreDelegateManager g_mgr(APP_ID, USER_ID);
    DBStatus g_kvNbDelegateStatus = INVALID_ARGS;
    KvStoreNbDelegate *g_kvNbDelegatePtr = nullptr;
    auto g_kvNbDelegateCallback = bind(&DistributedDBToolsUnitTest::KvStoreNbDelegateCallback,
        std::placeholders::_1, std::placeholders::_2, std::ref(g_kvNbDelegateStatus), std::ref(g_kvNbDelegatePtr));

    const std::string BASE_SCHEMA_STRING = "{\"SCHEMA_VERSION\" : \"1.0\","
        "\"SCHEMA_MODE\" : \"COMPATIBLE\","
        "\"SCHEMA_DEFINE\" : {"
           "\"name\" : \"STRING\","
           "\"id\" : \"INTEGER\","
           "\"father\" :  {"
                   "\"name\" : \"STRING\","
                   "\"id\" : \"INTEGER\""
               "},"
           "\"phone\" : \"INTEGER\""
       "},"
       "\"SCHEMA_INDEXES\" : ";

    const std::string JSON_VALUE ="{\"name\":\"Tom\","
        "\"id\":10,"
        "\"father\":{\"name\":\"Jim\", \"id\":20},"
        "\"phone\":20}";

    void GenerateSchemaString(std::string &schema, const std::string &indexString)
    {
        schema = BASE_SCHEMA_STRING + indexString + "}";
    }

    std::string GetKvStoreDirectory(const std::string &userId, const std::string &appId, const std::string &storeId)
    {
        string identifier = userId + "-" + appId + "-" + storeId;
        string hashIdentifierName = DBCommon::TransferHashString(identifier);
        string identifierName = DBCommon::TransferStringToHex(hashIdentifierName);
        string filePath = g_testDir + "/" + identifierName + "/" + DBConstant::SINGLE_SUB_DIR + "/main/";
        filePath += DBConstant::SINGLE_VER_DATA_STORE + DBConstant::SQLITE_DB_EXTENSION;
        return filePath;
    }

    bool CheckIndexFromDbFile(const::std::string &filePath, const std::string &indexName)
    {
        sqlite3 *db = nullptr;
        if (sqlite3_open_v2(filePath.c_str(), &db, SQLITE_OPEN_URI | SQLITE_OPEN_READWRITE, nullptr) != SQLITE_OK) {
            LOGD("DB open failed %s", filePath.c_str());
            if (db != nullptr) {
                (void)sqlite3_close_v2(db);
            }
            return false;
        }

        std::string querySQL = "select sql from sqlite_master where name = '" + indexName + "'";
        int errCode = sqlite3_exec(db, querySQL.c_str(), nullptr, nullptr, nullptr);
        (void)sqlite3_close_v2(db);
        if (errCode == SQLITE_OK) {
            return true;
        }
        return false;
    }
}

class DistributedDBStorageIndexOptimizeTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
};

void DistributedDBStorageIndexOptimizeTest::SetUpTestCase(void)
{
    DistributedDBToolsUnitTest::TestDirInit(g_testDir);
    std::string origIdentifier = USER_ID + "-" + APP_ID + "-" + STORE_ID_1;
    std::string identifier = DBCommon::TransferHashString(origIdentifier);
    g_identifier = DBCommon::TransferStringToHex(identifier);
    std::string dir = g_testDir + g_identifier + "/" + DBConstant::SINGLE_SUB_DIR;
    DIR *dirTmp = opendir(dir.c_str());
    if (dirTmp == nullptr) {
        OS::MakeDBDirectory(dir);
    } else {
        closedir(dirTmp);
    }

    KvStoreConfig config;
    config.dataDir = g_testDir;
    g_mgr.SetKvStoreConfig(config);
}

void DistributedDBStorageIndexOptimizeTest::TearDownTestCase(void)
{
    if (DistributedDBToolsUnitTest::RemoveTestDbFiles(g_testDir) != 0) {
        LOGE("rm test db files error!");
    }
}

void DistributedDBStorageIndexOptimizeTest::SetUp(void)
{
}

void DistributedDBStorageIndexOptimizeTest::TearDown(void)
{
}

/**
  * @tc.name: ParseAndCheckUnionIndex001
  * @tc.desc: Test the Json union index parse and check function Open function
  * @tc.type: FUNC
  * @tc.require: AR000F3OPD
  * @tc.author: xushaohua
  */
HWTEST_F(DistributedDBStorageIndexOptimizeTest, ParseAndCheckUnionIndex001, TestSize.Level1)
{
    /**
     * @tc.steps: step1. Create a correct shema string include a correct union index.
     */
    std::string schema1;
    GenerateSchemaString(schema1, "[[\"name\", \"father.name\", \"father.id\", \"id\", \"phone\"]]");

    /**
     * @tc.steps: step2. Call SchemaObject.ParseFromSchemaString to parse the string.
     * @tc.expected: step2. Expect return E_OK.
     */
    SchemaObject so1;
    EXPECT_EQ(so1.ParseFromSchemaString(schema1), E_OK);

    /**
     * @tc.steps: step3. Create a correct shema string include a single index and a union index
     */
    std::string schema2;
    GenerateSchemaString(schema2, "[[\"name\", \"father.name\", \"father.id\", \"id\", \"phone\"], \"id\"]");

    /**
     * @tc.steps: step4. Call SchemaObject.ParseFromSchemaString to parse the string.
     * @tc.expected: step4. Expect return E_OK.
     */
    SchemaObject so2;
    EXPECT_EQ(so2.ParseFromSchemaString(schema2), E_OK);

    /**
     * @tc.steps: step5. Create a shema string include a single index and a union index, and the two index has
                         the same sort column.
     */
    std::string schema3;
    GenerateSchemaString(schema3, "[[\"name\", \"father.name\", \"father.id\", \"id\", \"phone\"], \"name\"]");

    /**
     * @tc.steps: step6. Call SchemaObject.ParseFromSchemaString to parse the string.
     * @tc.expected: step6. Expect return E_SCHEMA_PARSE_FAIL.
     */
    SchemaObject so3;
    EXPECT_EQ(so3.ParseFromSchemaString(schema3), -E_SCHEMA_PARSE_FAIL);

    /**
     * @tc.steps: step7. Create a shema string include a single index with a not exist column
     */
    std::string schema4;
    GenerateSchemaString(schema4, "[[\"name\", \"father.name\", \"father.id\", \"id\", \"tel\"]]");

    /**
     * @tc.steps: step8. Call SchemaObject.ParseFromSchemaString to parse the string.
     * @tc.expected: step8. Expect return -E_SCHEMA_PARSE_FAIL.
     */
    SchemaObject so4;
    EXPECT_EQ(so4.ParseFromSchemaString(schema4), -E_SCHEMA_PARSE_FAIL);

    /**
     * @tc.steps: step9. Create a shema string include a single index with all columns not exists
     */
    std::string schema5;
    GenerateSchemaString(schema5, "[[\"name1\", \"father.name2\", \"father1.id\", \"id2\", \"tel\"]]");

    /**
     * @tc.steps: step10. Call SchemaObject.ParseFromSchemaString to parse the string.
     * @tc.expected: step10. Expect return -E_SCHEMA_PARSE_FAIL.
     */
    SchemaObject so5;
    EXPECT_EQ(so5.ParseFromSchemaString(schema5), -E_SCHEMA_PARSE_FAIL);
}

/**
  * @tc.name: UnionIndexCreatTest001
  * @tc.desc: Test the Json uoin index create function
  * @tc.type: FUNC
  * @tc.require: AR000F3OPD
  * @tc.author: xushaohua
  */
HWTEST_F(DistributedDBStorageIndexOptimizeTest, UnionIndexCreatTest001, TestSize.Level1)
{
    /**
     * @tc.steps: step1. Create a correct shema string include a correct union index.
     */
    std::string schema;
    GenerateSchemaString(schema, "[[\"name\", \"father.name\"]]");

    /**
     * @tc.steps: step2. Create a kvStore with the schema string.
     */
    KvStoreNbDelegate::Option option;
    option.schema = schema;
    g_mgr.GetKvStore(STORE_ID_1, option, g_kvNbDelegateCallback);
    ASSERT_TRUE(g_kvNbDelegatePtr != nullptr);
    g_mgr.CloseKvStore(g_kvNbDelegatePtr);
    g_kvNbDelegatePtr = nullptr;
    EXPECT_TRUE(CheckIndexFromDbFile(GetKvStoreDirectory(USER_ID, APP_ID, STORE_ID_1), "$.name"));
}

/**
  * @tc.name: SuggestIndexTest001
  * @tc.desc: Test the Suggest index verify function
  * @tc.type: FUNC
  * @tc.require: AR000F3OPE
  * @tc.author: xushaohua
  */
HWTEST_F(DistributedDBStorageIndexOptimizeTest, SuggestIndexTest001, TestSize.Level1)
{
    std::string schema;
    GenerateSchemaString(schema, "[\"name\", \"id\"]");
    SchemaObject schemaObject1;
    ASSERT_EQ(schemaObject1.ParseFromSchemaString(schema), E_OK);

    /**
     * @tc.steps: step1. Create a Query and call GreaterThan().SuggestIndex(), then check the query1
     * @tc.expected: step1. query1 is valid.
     */
    Query query1 = Query::Select().GreaterThan("id", 1).SuggestIndex("id");
    QueryObject queryObject1(query1);
    queryObject1.SetSchema(schemaObject1);
    int errCode;
    EXPECT_TRUE(queryObject1.IsValid(errCode));

    /**
     * @tc.steps: step2. Create a Query and call SuggestIndex().SuggestIndex(), then check the query2
     * @tc.expected: step2. query1 is invalid.
     */
    SchemaObject schemaObject2;
    ASSERT_EQ(schemaObject2.ParseFromSchemaString(schema), E_OK);
    Query query2 = Query::Select().SuggestIndex("id").SuggestIndex("id");
    QueryObject queryObject2(query2);
    queryObject2.SetSchema(schemaObject2);
    EXPECT_FALSE(queryObject2.IsValid(errCode));

    /**
     * @tc.steps: step3. Create a Query and call SuggestIndex().GreaterThan(), then check the query3
     * @tc.expected: step4. query3 is invalid.
     */
    SchemaObject schemaObject3;
    ASSERT_EQ(schemaObject3.ParseFromSchemaString(schema), E_OK);
    Query query3 = Query::Select().SuggestIndex("id").GreaterThan("id", 1);
    QueryObject queryObject3(query3);
    queryObject3.SetSchema(schemaObject3);
    EXPECT_FALSE(queryObject3.IsValid(errCode));
}

/**
  * @tc.name: SuggestIndexTest002
  * @tc.desc: Test the Query parse sql the SuggestIndex
  * @tc.type: FUNC
  * @tc.require: AR000F3OPE
  * @tc.author: xushaohua
  */
HWTEST_F(DistributedDBStorageIndexOptimizeTest, SuggestIndexTest002, TestSize.Level1)
{
    /**
     * @tc.steps: step1. Create a schema include index name, id, phone.
     */
    std::string schema;
    GenerateSchemaString(schema, "[\"name\", \"id\", \"phone\"]");

    /**
     * @tc.steps: step2. Create Query,call GreaterThan("id").GreaterThan("phone").SuggestIndex("id")
     */
    SchemaObject schemaObject1;
    ASSERT_EQ(schemaObject1.ParseFromSchemaString(schema), E_OK);
    Query query1 = Query::Select().GreaterThan("id", 1).And().GreaterThan("phone", 1).SuggestIndex("id");

    /**
     * @tc.steps: step3. Create QueryObject with query1 and call GetQuerySql to check the sql
     * @tc.expected: step3. the sql contains "INDEXED BY $.id".
     */
    QueryObject queryObject1(query1);
    queryObject1.SetSchema(schemaObject1);
    std::string sql1;
    ASSERT_EQ(queryObject1.GetQuerySql(sql1, false), E_OK);
    size_t pos = sql1.find("INDEXED BY '$.id'", 0);
    ASSERT_TRUE(pos != std::string::npos);

    /**
     * @tc.steps: step4. Create a kvStore with the schema string.
     */
    KvStoreNbDelegate::Option option;
    option.schema = schema;
    g_mgr.GetKvStore(STORE_ID_1, option, g_kvNbDelegateCallback);
    ASSERT_TRUE(g_kvNbDelegatePtr != nullptr);

    /**
     * @tc.steps: step5. put a valid value
     */
    Value value(JSON_VALUE.begin(), JSON_VALUE.end());
    ASSERT_EQ(g_kvNbDelegatePtr->Put(KEY_1, value), OK);
    std::vector<Entry> entries;

    /**
     * @tc.steps: step6.  GetEntries with the query1
     * @tc.expected: step6. GetEntries return OK, and the out value is the given value.
     */
    ASSERT_EQ(g_kvNbDelegatePtr->GetEntries(query1, entries), OK);
    EXPECT_TRUE(value == entries[0].value);
    EXPECT_EQ(g_mgr.CloseKvStore(g_kvNbDelegatePtr), OK);
    g_kvNbDelegatePtr = nullptr;

    /**
     * @tc.steps: step7. Create Query,call GreaterThan("id").SuggestIndex("car")
     */
    SchemaObject schemaObject3;
    ASSERT_EQ(schemaObject3.ParseFromSchemaString(schema), E_OK);
    Query query3 = Query::Select().GreaterThan("id", 1).SuggestIndex("car");

    /**
     * @tc.steps: step8. Create QueryObject with query3 and call GetQuerySql to check the sql
     * @tc.expected: step8. the sql not contains "INDEXED BY $.car".
     */
    QueryObject queryObject3(query3);
    queryObject3.SetSchema(schemaObject3);
    std::string sql3;
    ASSERT_EQ(queryObject3.GetQuerySql(sql3, false), E_OK);
    pos = sql3.find("INDEXED BY '$.car'", 0);
    ASSERT_TRUE(pos == std::string::npos);
}
#endif