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
#include <ctime>
#include <cmath>
#include <random>
#include <chrono>
#include <thread>
#include <mutex>
#include <string>
#include <condition_variable>

#include "types.h"
#include "kv_store_delegate.h"
#include "kv_store_nb_delegate.h"
#include "kv_store_delegate_manager.h"
#include "distributed_test_tools.h"
#include "distributeddb_nb_test_tools.h"
#include "distributeddb_data_generator.h"
#include "distributeddb_schema_test_tools.h"

using namespace std;
using namespace chrono;
using namespace testing;
#if defined TESTCASES_USING_GTEST_EXT
using namespace testing::ext;
#endif
using namespace std::placeholders;
using namespace DistributedDB;
using namespace DistributedDBDataGenerator;

namespace DistributeddbNbSchemaDb {
KvStoreNbDelegate *g_nbSchemaDelegate = nullptr;
KvStoreDelegateManager *g_manager = nullptr;

// invalid filed number: 0 or 257
void GetInvalidFieldNumber(vector<string> &defineResult)
{
    defineResult.push_back(INVALID_DEFINE_1);
    LongDefine param;
    param.recordNum = RECORDNUM;
    param.recordSize = RECORDSIZE;
    param.prefix = 'f';
    string invalidField;
    GetLongSchemaDefine(param, invalidField);
    defineResult.push_back(invalidField);
}

void GetInvalidDefault(vector<string> &defineResult)
{
    LongDefine param;
    param.recordNum = ONE_RECORD;
    param.recordSize = OVER_MAXSIZE;
    param.prefix = 'k';
    string invalidField;
    GetLongSchemaDefine(param, invalidField);
    defineResult.push_back(invalidField);
    // the level of field is over 4
    defineResult.push_back(INVALID_DEFINE_2);
}

vector<string> GetInvalidDefine(SchemaDefine &validDefine, SchemaDefine &invalidDefine)
{
    string define;
    string invalidField(KEY_SIXTYFOUR_BYTE + 1, 'a');
    define = define + "\"" + invalidField + "\"";
    invalidDefine.field.push_back(define);
    vector<string> defineResult;
    // invalid field name
    for (const auto &iter : invalidDefine.field) {
        define.clear();
        define = define + "{" + iter + ":" + "\"" + validDefine.type.at(0) + "," + validDefine.notnull.at(0) +
            "," + validDefine.defaultValue.at(0) + "\"" + "}";
        defineResult.push_back(define);
    }
    // invalid filed number: 0 or 257
    GetInvalidFieldNumber(defineResult);
    // invalid field attributes: type
    for (const auto &iter : invalidDefine.type) {
        define.clear();
        define = define + "{" + validDefine.field.at(0) + ":" + "\"" + iter + "," + validDefine.notnull.at(0) +
             "\"" + "}";
        defineResult.push_back(define);
    }
    // invalid field attributes: not_null
    for (const auto &iter : invalidDefine.notnull) {
        define.clear();
        define = define + "{" + validDefine.field.at(0) + ":" + "\"" + validDefine.type.at(0) + "," + iter +
            "," + validDefine.defaultValue.at(0) + "\"" + "}";
        defineResult.push_back(define);
    }
    // invalid field attributes: DEFAULT x
    for (unsigned int index1 = 0; index1 < validDefine.type.size(); index1++) {
        for (unsigned int index2 = 0; index2 < invalidDefine.defaultValue.size(); index2++) {
            define.clear();
            if (index1 == 0 && ((index2 == INDEX_FIFY) || (index2 == INDEX_SIX) || (index2 == INDEX_NINE) ||
                (index2 == INDEX_TEN))) {
                continue;
            } else {
                define = define + "{" + validDefine.field.at(0) + ":" + "\"" + validDefine.type[index1] + "," +
                    validDefine.notnull.at(0) + "," + invalidDefine.defaultValue[index2] + "\"" + "}";
                defineResult.push_back(define);
            }
        }
    }
    // invalid field attributes: mix of type,not_null,DEFAULT x and DEFAULT x over the range of INTEGER/LONG/DOUBLE
    for (const auto &iter : INVALID_ATTRIBUTES) {
        define.clear();
        define = define + "{" + validDefine.field.at(0) + ":" + iter + "}";
        defineResult.push_back(define);
    }
    // DEFAULT x over 4K is invalid
    GetInvalidDefault(defineResult);
    return defineResult;
}

class DistributeddbNbSchemaDbTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
private:
};

void DistributeddbNbSchemaDbTest::SetUpTestCase(void)
{
}

void DistributeddbNbSchemaDbTest::TearDownTestCase(void)
{
}

void DistributeddbNbSchemaDbTest::SetUp(void)
{
    RemoveDir(NB_DIRECTOR);

    UnitTest *test = UnitTest::GetInstance();
    ASSERT_NE(test, nullptr);
    const TestInfo *testinfo = test->current_test_info();
    ASSERT_NE(testinfo, nullptr);
    string testCaseName = string(testinfo->name());
    MST_LOG("[SetUp] test case %s is start to run", testCaseName.c_str());
}

void DistributeddbNbSchemaDbTest::TearDown(void)
{
    MST_LOG("TearDownTestCase after case.");
    RemoveDir(NB_DIRECTOR);
}

/**
 * @tc.name: BuildSchemaDb 001
 * @tc.desc: Verify that single-ver db can support create schema db if the schema is valid.
 * @tc.type: FUNC
 * @tc.require: SR000DR9JO
 * @tc.author: fengxiaoyun
 */
HWTEST_F(DistributeddbNbSchemaDbTest, BuildSchemaDb001, TestSize.Level1)
{
    KvStoreNbDelegate *delegate = nullptr;
    KvStoreDelegateManager *manager = nullptr;
    /**
     * @tc.steps: step1. create db with valid schema.
     * @tc.expected: step1. create successfully.
     */
    Schema validSchema;
    validSchema.version = VALID_VERSION;
    validSchema.mode = VALID_MODE;
    validSchema.define = VALID_DEFINE;
    validSchema.index = VALID_INDEX;
    vector<string> validSchemaStr = GetValidSchema(validSchema, true);
    Option option = g_option;
    option.isMemoryDb = false;
    int count = 0;
    for (const auto &iter : validSchemaStr) {
        MST_LOG("######## ######## ######## BuildSchemaDb001 : SubCase[%d] BEGIN ######## ######## ########", count);
        option.schema = iter;
        delegate = DistributedDBNbTestTools::GetNbDelegateSuccess(manager, g_dbParameter2, option);
        ASSERT_TRUE(manager != nullptr && delegate != nullptr);
        EXPECT_TRUE(EndCaseDeleteDB(manager, delegate, STORE_ID_2, option.isMemoryDb));
        MST_LOG("######## ######## ######## BuildSchemaDb001 : SubCase[%d] END ######## ######## ########", count++);
    }
}

/**
 * @tc.name: BuildSchemaDb 002
 * @tc.desc: Verify that single-ver db can't create schema db if the schema is invalid.
 * @tc.type: FUNC
 * @tc.require: SR000DR9JO
 * @tc.author: fengxiaoyun
 */
HWTEST_F(DistributeddbNbSchemaDbTest, BuildSchemaDb002, TestSize.Level1)
{
    KvStoreNbDelegate *delegate = nullptr;
    KvStoreDelegateManager *manager = nullptr;
    Schema validSchema, invalidSchema;
    validSchema.version = VALID_VERSION;
    validSchema.mode = VALID_MODE;
    validSchema.define = VALID_DEFINE;
    validSchema.index = VALID_INDEX;

    SchemaDefine invalidDefine, validDefine;
    invalidDefine.field = INVALID_DEFINE_FIELD;
    invalidDefine.type = INVALID_TYPE;
    invalidDefine.notnull = INVALID_NOTNULL;
    invalidDefine.defaultValue = INVALID_DEFAULT;
    validDefine.field = VALID_DEFINE_FIELD;
    validDefine.type = VALID_TYPE;
    validDefine.notnull = VALID_NOTNULL;
    validDefine.defaultValue = VALID_DEFAULT;
    vector<string> defineRes = GetInvalidDefine(validDefine, invalidDefine);
    invalidSchema.version = INVALID_VERSION;
    invalidSchema.mode = INVALID_MODE;
    invalidSchema.define = defineRes;
    invalidSchema.index = INALID_INDEX;
    map<int, vector<string>> schemaRes1 = GetInvalidSchema(invalidSchema, validSchema, false);
    map<int, vector<string>> schemaRes2 = GetInvalidSchema(invalidSchema, validSchema, true);

    /**
     * @tc.steps: step1. create db with invalid schema without SCHEMA_INDEX.
     * @tc.expected: step1. create failed and return INVALID_SCHEMA.
     */
    Option option = g_option;
    option.isMemoryDb = false;
    DBStatus status;
    int count = 0;
    for (int index = 0; index < SCHEMA_INDEX; index++) {
        for (const auto &iter : schemaRes1[index]) {
            option.schema = iter;
            MST_LOG("######## ######## BuildSchemaDb002 : SubCase[%d] BEGIN ######## ######## ########", count);
            delegate = DistributedDBNbTestTools::GetNbDelegateStatus(manager, status, g_dbParameter3, option);
            MST_LOG("The invalid field is %d", index);
            EXPECT_EQ(delegate, nullptr);
            EXPECT_EQ(status, INVALID_SCHEMA);
            MST_LOG("######## ######## BuildSchemaDb002 : SubCase[%d] END ######## ######## ########", count++);
        }
    }
    /**
     * @tc.steps: step2. create db with invalid schema with invalid SCHEMA_INDEX.
     * @tc.expected: step2. create failed and return INVALID_SCHEMA.
     */
    for (int index = 0; index < SCHEMA_INDEX; index++) {
        for (const auto &iter : schemaRes2[index]) {
            option.schema = iter;
            delegate = DistributedDBNbTestTools::GetNbDelegateStatus(manager, status, g_dbParameter2, option);
            EXPECT_EQ(delegate, nullptr);
            EXPECT_EQ(status, INVALID_SCHEMA);
        }
    }
}

/**
 * @tc.name: BuildSchemaDb 003
 * @tc.desc: Verify that single-ver memory db can't support create schema db.
 * @tc.type: FUNC
 * @tc.require: SR000DR9JO
 * @tc.author: fengxiaoyun
 */
HWTEST_F(DistributeddbNbSchemaDbTest, BuildSchemaDb003, TestSize.Level0)
{
    KvStoreNbDelegate *delegate = nullptr;
    KvStoreDelegateManager *manager = nullptr;
    /**
     * @tc.steps: step1. create memore db with valid schema.
     * @tc.expected: step1. create failed and return NOT_SUPPORT.
     */
    Schema validSchema;
    validSchema.version = VALID_VERSION;
    validSchema.mode = VALID_MODE;
    validSchema.define = VALID_DEFINE;
    validSchema.index = VALID_INDEX;
    vector<string> validSchemaStr = GetValidSchema(validSchema, true);
    Option option = g_option;
    option.isMemoryDb = true;
    DBStatus status;
    for (const auto &iter : validSchemaStr) {
        option.schema = iter;
        delegate = DistributedDBNbTestTools::GetNbDelegateStatus(manager, status, g_dbParameter2, option);
        EXPECT_EQ(delegate, nullptr);
        EXPECT_EQ(status, NOT_SUPPORT);
    }
}

/**
 * @tc.name: OpenSchemaDb 001
 * @tc.desc: Verify that can open with schema when the kvstore is not schema db.
 * @tc.type: FUNC
 * @tc.require: SR000DR9JO
 * @tc.author: fengxiaoyun
 */
HWTEST_F(DistributeddbNbSchemaDbTest, OpenSchemaDb001, TestSize.Level1)
{
    KvStoreNbDelegate *delegate = nullptr;
    KvStoreDelegateManager *manager = nullptr;
    Option option = g_option;
    option.isMemoryDb = false;
    delegate = DistributedDBNbTestTools::GetNbDelegateSuccess(manager, g_dbParameter2, option);
    ASSERT_TRUE(manager != nullptr && delegate != nullptr);
    EXPECT_TRUE(manager->CloseKvStore(delegate) == OK);
    delegate = nullptr;

    /**
     * @tc.steps: step1. reopen db with schema of STRICT mode and put(k1,value).
     * @tc.expected: step1. reopen and put successfully.
     */
    LongDefine param;
    param.recordNum = ONE_RECORD;
    param.recordSize = RECORDSIZE;
    param.prefix = 'k';
    string invalidField;
    GetLongSchemaDefine(param, invalidField);
    option.schema = SpliceToSchema(VALID_VERSION_1, VALID_MODE_1, invalidField, VALID_INDEX_1);
    option.createIfNecessary = false;
    delegate = DistributedDBNbTestTools::GetNbDelegateSuccess(manager, g_dbParameter2, option);
    ASSERT_TRUE(manager != nullptr && delegate != nullptr);
    Value valueSchema(invalidField.begin(), invalidField.end());
    EXPECT_EQ(DistributedDBNbTestTools::Put(*delegate, KEY_1, valueSchema), OK);

    /**
     * @tc.steps: step2. Get(k1) and delete(k1).
     * @tc.expected: step2. Get(k1)=value and delete successfully.
     */
    Value valueResult;
    EXPECT_EQ(DistributedDBNbTestTools::Get(*delegate, KEY_1, valueResult), OK);
    EXPECT_EQ(valueResult, valueSchema);
    EXPECT_EQ(DistributedDBNbTestTools::Delete(*delegate, KEY_1), OK);
    EXPECT_TRUE(manager->CloseKvStore(delegate) == OK);
    delegate = nullptr;

    /**
     * @tc.steps: step3. reopen db with no schema and put(k1,v1),delete(k1),Get(k1).
     * @tc.expected: step3. reopen successfully and put,delete return READ_ONLY,Get return NOT_FOUND.
     */
    option.schema.clear();
    delegate = DistributedDBNbTestTools::GetNbDelegateSuccess(manager, g_dbParameter2, option);
    ASSERT_TRUE(manager != nullptr && delegate != nullptr);
    EXPECT_EQ(DistributedDBNbTestTools::Put(*delegate, KEY_1, valueSchema), READ_ONLY);
    EXPECT_EQ(DistributedDBNbTestTools::Delete(*delegate, KEY_1), READ_ONLY);
    EXPECT_EQ(DistributedDBNbTestTools::Get(*delegate, KEY_1, valueResult), NOT_FOUND);
    EXPECT_TRUE(EndCaseDeleteDB(manager, delegate, STORE_ID_2, option.isMemoryDb));
}

/**
 * @tc.name: OpenSchemaDb 002
 * @tc.desc: Verify that single-ver db can't reopen schema db if the schema is invalid when the kvstore is
 *     not a schema db.
 * @tc.type: FUNC
 * @tc.require: SR000DR9JO
 * @tc.author: fengxiaoyun
 */
HWTEST_F(DistributeddbNbSchemaDbTest, OpenSchemaDb002, TestSize.Level1)
{
    KvStoreNbDelegate *delegate = nullptr;
    KvStoreDelegateManager *manager = nullptr;
    Option option = g_option;
    option.isMemoryDb = false;
    delegate = DistributedDBNbTestTools::GetNbDelegateSuccess(manager, g_dbParameter2, option);
    ASSERT_TRUE(manager != nullptr && delegate != nullptr);
    EXPECT_TRUE(manager->CloseKvStore(delegate) == OK);

    Schema validSchema, invalidSchema;
    validSchema.version = VALID_VERSION;
    validSchema.mode = VALID_MODE;
    validSchema.define = VALID_DEFINE;
    validSchema.index = VALID_INDEX;

    SchemaDefine invalidDefine, validDefine;
    invalidDefine.field = INVALID_DEFINE_FIELD;
    invalidDefine.type = INVALID_TYPE;
    invalidDefine.notnull = INVALID_NOTNULL;
    invalidDefine.defaultValue = INVALID_DEFAULT;
    validDefine.field = VALID_DEFINE_FIELD;
    validDefine.type = VALID_TYPE;
    validDefine.notnull = VALID_NOTNULL;
    validDefine.defaultValue = VALID_DEFAULT;
    vector<string> defineRes = GetInvalidDefine(validDefine, invalidDefine);
    invalidSchema.version = INVALID_VERSION;
    invalidSchema.mode = INVALID_MODE;
    invalidSchema.define = defineRes;
    invalidSchema.index = INALID_INDEX;
    map<int, vector<string>> schemaRes1 = GetInvalidSchema(invalidSchema, validSchema, false);
    map<int, vector<string>> schemaRes2 = GetInvalidSchema(invalidSchema, validSchema, true);

    /**
     * @tc.steps: step1. reopen db with invalid schema without SCHEMA_INDEX.
     * @tc.expected: step1. reopen failed and return INVALID_SCHEMA.
     */
    option.createIfNecessary = false;
    DBStatus status;
    for (int index = 0; index < SCHEMA_INDEX; index++) {
        for (const auto &iter : schemaRes1[index]) {
            option.schema = iter;
            delegate = DistributedDBNbTestTools::GetNbDelegateStatus(manager, status, g_dbParameter2, option);
            MST_LOG("The invalid field is %d", index);
            EXPECT_EQ(delegate, nullptr);
            EXPECT_EQ(status, INVALID_SCHEMA);
        }
    }
    /**
     * @tc.steps: step2. reopen db with invalid schema with invalid SCHEMA_INDEX.
     * @tc.expected: step2. reopen failed and return INVALID_SCHEMA.
     */
    for (int index = 0; index < SCHEMA_INDEX; index++) {
        for (const auto &iter : schemaRes2[index]) {
            option.schema = iter;
            delegate = DistributedDBNbTestTools::GetNbDelegateStatus(manager, status, g_dbParameter2, option);
            EXPECT_EQ(delegate, nullptr);
            EXPECT_EQ(status, INVALID_SCHEMA);
        }
    }
}

/**
 * @tc.name: OpenSchemaDb 003
 * @tc.desc: Verify that can open with no schema when the kvstore is schema db.
 * @tc.type: FUNC
 * @tc.require: SR000DR9JO
 * @tc.author: fengxiaoyun
 */
HWTEST_F(DistributeddbNbSchemaDbTest, OpenSchemaDb003, TestSize.Level1)
{
    KvStoreNbDelegate *delegate = nullptr;
    KvStoreDelegateManager *manager = nullptr;
    Option option = g_option;
    option.isMemoryDb = false;
    LongDefine param;
    param.recordNum = ONE_RECORD;
    param.recordSize = RECORDSIZE;
    param.prefix = 'k';
    string validField;
    GetLongSchemaDefine(param, validField);
    option.schema = SpliceToSchema(VALID_VERSION_1, VALID_MODE_1, validField, VALID_INDEX_1);
    delegate = DistributedDBNbTestTools::GetNbDelegateSuccess(manager, g_dbParameter2, option);
    ASSERT_TRUE(manager != nullptr && delegate != nullptr);
    Value valueSchema(validField.begin(), validField.end());
    EXPECT_EQ(DistributedDBNbTestTools::Put(*delegate, KEY_1, valueSchema), OK);
    EXPECT_TRUE(manager->CloseKvStore(delegate) == OK);
    /**
     * @tc.steps: step1. reopen db with no schema.
     * @tc.expected: step1. reopen successfully.
     */
    option.createIfNecessary = false;
    option.schema.clear();
    delegate = DistributedDBNbTestTools::GetNbDelegateSuccess(manager, g_dbParameter2, option);
    ASSERT_TRUE(manager != nullptr && delegate != nullptr);
    /**
     * @tc.steps: step2. Get(k1) and put(k1,v1), delete(k1).
     * @tc.expected: step2. Get(k1)=value, put and delete failed.
     */
    Value valueResult;
    EXPECT_EQ(DistributedDBNbTestTools::Get(*delegate, KEY_1, valueResult), OK);
    EXPECT_EQ(valueResult, valueSchema);
    EXPECT_EQ(DistributedDBNbTestTools::Put(*delegate, KEY_1, VALUE_1), READ_ONLY);
    EXPECT_EQ(DistributedDBNbTestTools::Delete(*delegate, KEY_1), READ_ONLY);
    EXPECT_TRUE(EndCaseDeleteDB(manager, delegate, STORE_ID_2, option.isMemoryDb));
}

/**
 * @tc.name: OpenSchemaDb 004
 * @tc.desc: Verify that can open with schema when the kvstore is schema db even if the order of same
 *     level is different.
 * @tc.type: FUNC
 * @tc.require: SR000DR9JO
 * @tc.author: fengxiaoyun
 */
HWTEST_F(DistributeddbNbSchemaDbTest, OpenSchemaDb004, TestSize.Level1)
{
    KvStoreNbDelegate *delegate = nullptr;
    KvStoreDelegateManager *manager = nullptr;
    Option option = g_option;
    option.isMemoryDb = false;
    LongDefine param;
    param.recordNum = ONE_RECORD;
    param.recordSize = RECORDSIZE;
    param.prefix = 'k';
    string validField;
    GetLongSchemaDefine(param, validField);
    option.schema = SpliceToSchema(VALID_VERSION_1, VALID_MODE_1, validField, VALID_INDEX_1);
    delegate = DistributedDBNbTestTools::GetNbDelegateSuccess(manager, g_dbParameter2, option);
    ASSERT_TRUE(manager != nullptr && delegate != nullptr);
    Value valueSchema(validField.begin(), validField.end());
    EXPECT_EQ(DistributedDBNbTestTools::Put(*delegate, KEY_1, valueSchema), OK);
    EXPECT_TRUE(manager->CloseKvStore(delegate) == OK);
    /**
     * @tc.steps: step1. reopen db with schema that the order of schema is different with kvstore.
     * @tc.expected: step1. reopen successfully.
     */
    option.createIfNecessary = false;
    option.schema.clear();
    option.schema = option.schema + "{" + "\"SCHEMA_DEFINE\"" + ":"  + validField + "," +
        "\"SCHEMA_VERSION\"" + ":" + "\"" + VALID_VERSION_1 + "\"" + "," +
        "\"SCHEMA_INDEXES\"" + ":" + VALID_INDEX_1 + "," +
        "\"SCHEMA_MODE\"" + ":" + "\"" + VALID_MODE_1 + "\"" + "}";
    MST_LOG("schema is %s", option.schema.c_str());
    delegate = DistributedDBNbTestTools::GetNbDelegateSuccess(manager, g_dbParameter2, option);
    ASSERT_TRUE(manager != nullptr && delegate != nullptr);
    /**
     * @tc.steps: step2. Get(k1) and put(k1,v1), delete(k1).
     * @tc.expected: step2. Get(k1)=value1, put and delete successfully.
     */
    Value valueResult;
    EXPECT_EQ(DistributedDBNbTestTools::Get(*delegate, KEY_1, valueResult), OK);
    EXPECT_EQ(valueResult, valueSchema);
    string value2 = "{\"field0\":\"fxy\"}";
    Value valueSchema2(value2.begin(), value2.end());
    EXPECT_EQ(DistributedDBNbTestTools::Put(*delegate, KEY_1, valueSchema2), OK);
    EXPECT_EQ(DistributedDBNbTestTools::Delete(*delegate, KEY_1), OK);
    EXPECT_TRUE(EndCaseDeleteDB(manager, delegate, STORE_ID_2, option.isMemoryDb));
}

/**
 * @tc.name: OpenSchemaDb 005
 * @tc.desc: Verify that can't open with schema when the schema is mismatch with the original kvstore schema.
 * @tc.type: FUNC
 * @tc.require: SR000DR9JO
 * @tc.author: fengxiaoyun
 */
HWTEST_F(DistributeddbNbSchemaDbTest, OpenSchemaDb005, TestSize.Level1)
{
    KvStoreNbDelegate *delegate = nullptr;
    KvStoreDelegateManager *manager = nullptr;
    Option option = g_option;
    option.isMemoryDb = false;
    LongDefine param;
    param.recordNum = ONE_RECORD;
    param.recordSize = RECORDSIZE;
    param.prefix = 'k';
    string validField;
    GetLongSchemaDefine(param, validField);
    option.schema = SpliceToSchema(VALID_VERSION_1, VALID_MODE_2, validField, VALID_INDEX_1);
    delegate = DistributedDBNbTestTools::GetNbDelegateSuccess(manager, g_dbParameter2, option);
    ASSERT_TRUE(manager != nullptr && delegate != nullptr);
    EXPECT_TRUE(manager->CloseKvStore(delegate) == OK);
    /**
     * @tc.steps: step1. reopen db with schema which mode is different from the original kvstore schema.
     * @tc.expected: step1. reopen failed and return SCHEMA_MISMATCH.
     */
    option.createIfNecessary = false;
    option.schema.clear();
    DBStatus status;
    option.schema = SpliceToSchema(VALID_VERSION_1, VALID_MODE_1, validField, VALID_INDEX_1);
    delegate = DistributedDBNbTestTools::GetNbDelegateStatus(manager, status, g_dbParameter2, option);
    EXPECT_EQ(delegate, nullptr);
    EXPECT_EQ(status, SCHEMA_MISMATCH);
    /**
     * @tc.steps: step2. reopen db with schema which define fields is more than the original kvstore schema.
     * @tc.expected: step2. reopen failed and return SCHEMA_MISMATCH.(extra field has not null but no default)
     */
    string validField2;
    param.recordNum = TWO_RECORD;
    GetLongSchemaDefine(param, validField2);
    std::string toBeErased = ",DEFAULT 'kkk2'";
    auto iter = std::search(validField2.begin(), validField2.end(), toBeErased.begin(), toBeErased.end());
    validField2.erase(iter, iter + toBeErased.size());
    option.schema.clear();
    option.schema = SpliceToSchema(VALID_VERSION_1, VALID_MODE_2, validField2, VALID_INDEX_1);
    delegate = DistributedDBNbTestTools::GetNbDelegateStatus(manager, status, g_dbParameter2, option);
    EXPECT_EQ(delegate, nullptr);
    EXPECT_EQ(status, SCHEMA_MISMATCH);

    /**
     * @tc.steps: step3. reopen db with schema which SCHEMA_INDEX is not null.
     * @tc.expected: step3. reopen failed and return SCHEMA_MISMATCH.(extra field has not null but no default)
     */
    option.schema.erase(option.schema.size() - 3, 3); // erase 3 chars that starting at 3 st last.
    option.schema.append("[\"$.field0\"]}");
    MST_LOG("option.schema is %s", option.schema.c_str());
    delegate = DistributedDBNbTestTools::GetNbDelegateStatus(manager, status, g_dbParameter2, option);
    EXPECT_EQ(delegate, nullptr);
    EXPECT_EQ(status, SCHEMA_MISMATCH);

    /**
     * @tc.steps: step4. reopen db with schema which schema_index is absent.
     * @tc.expected: step4. reopen successfully.
     */
    option.schema.clear();
    option.schema = SpliceToSchema(VALID_VERSION_1, VALID_MODE_2, validField, VALID_INDEX_1);
    option.schema.erase(option.schema.size() - 21, 20); // erase 20 char that starting at 21 st last.
    MST_LOG("schema is %s", option.schema.c_str());
    delegate = DistributedDBNbTestTools::GetNbDelegateSuccess(manager, g_dbParameter2, option);
    ASSERT_TRUE(manager != nullptr && delegate != nullptr);
    EXPECT_TRUE(EndCaseDeleteDB(manager, delegate, STORE_ID_2, option.isMemoryDb));
}

void ReopenDBWithDiffSchema(string &schemaDefine, Option &option,
    KvStoreDelegateManager *&manager, KvStoreNbDelegate *&delegate)
{
    option.schema.clear();
    DBStatus status;
    option.schema = SpliceToSchema(VALID_VERSION_1, VALID_MODE_1, schemaDefine, VALID_INDEX_1);
    delegate = DistributedDBNbTestTools::GetNbDelegateStatus(manager, status, g_dbParameter2, option);
    EXPECT_TRUE(manager == nullptr && delegate == nullptr);
    EXPECT_EQ(status, SCHEMA_MISMATCH);
}

/**
 * @tc.name: OpenSchemaDb 006
 * @tc.desc: Verify that can't open with schema when the schema_define attributes are mismatch with the
 *     original kvstore schema.
 * @tc.type: FUNC
 * @tc.require: SR000DR9JO
 * @tc.author: fengxiaoyun
 */
HWTEST_F(DistributeddbNbSchemaDbTest, OpenSchemaDb006, TestSize.Level1)
{
    KvStoreNbDelegate *delegate = nullptr;
    KvStoreDelegateManager *manager = nullptr;
    Option option = g_option;
    option.isMemoryDb = false;
    string schemaDefine1 = "{\"field0\":\"INTEGER,NOT NULL,DEFAULT 10\",\"field1\":[]}";
    option.schema = SpliceToSchema(VALID_VERSION_1, VALID_MODE_1, schemaDefine1, VALID_INDEX_1);
    delegate = DistributedDBNbTestTools::GetNbDelegateSuccess(manager, g_dbParameter2, option);
    ASSERT_TRUE(manager != nullptr && delegate != nullptr);
    EXPECT_TRUE(manager->CloseKvStore(delegate) == OK);
    delegate = nullptr;
    /**
     * @tc.steps: step1. reopen db with schema which schema_define's type is different from the original kvstore schema.
     * @tc.expected: step1. reopen failed and return SCHEMA_MISMATCH.
     */
    option.createIfNecessary = false;
    string schemaDefine2 = "{\"field0\":\"LONG,NOT NULL,DEFAULT 10\",\"field1\":[]}";
    ReopenDBWithDiffSchema(schemaDefine2, option, manager, delegate);
    /**
     * @tc.steps: step2. reopen db with schema which schema_define's not_null is different from the
     *     original kvstore schema.
     * @tc.expected: step2. reopen failed and return SCHEMA_MISMATCH.
     */
    string schemaDefine3 = "{\"field0\":\"INTEGER,DEFAULT 10\",\"field1\":[]}";
    ReopenDBWithDiffSchema(schemaDefine3, option, manager, delegate);
    /**
     * @tc.steps: step3. reopen db with schema which schema_define's default x is different from the
     *     original kvstore schema.
     * @tc.expected: step3. reopen failed and return SCHEMA_MISMATCH.
     */
    string schemaDefine4 = "{\"field0\":\"INTEGER,NOT NULL,DEFAULT 11\",\"field1\":[]}";
    ReopenDBWithDiffSchema(schemaDefine4, option, manager, delegate);
    string schemaDefine5 = "{\"field0\":\"INTEGER,NOT NULL\",\"field1\":[]}";
    ReopenDBWithDiffSchema(schemaDefine5, option, manager, delegate);
    /**
     * @tc.steps: step4. reopen db with schema which schema_define's Json Array is different from the
     *     original kvstore schema.
     * @tc.expected: step4. reopen failed and return SCHEMA_MISMATCH.
     */
    string schemaDefine6 = "{\"field0\":\"INTEGER,NOT NULL,DEFAULT 10\",\"field1\":{}}";
    ReopenDBWithDiffSchema(schemaDefine6, option, manager, delegate);
    KvStoreDelegateManager *manager1 = new (std::nothrow) KvStoreDelegateManager(APP_ID_2, USER_ID_2);
    ASSERT_NE(manager1, nullptr);
    EXPECT_EQ(manager1->SetKvStoreConfig({ .dataDir = NB_DIRECTOR }), OK);
    EXPECT_EQ(manager1->DeleteKvStore(STORE_ID_2), OK);
    delete manager1;
    manager1 = nullptr;
}

/**
 * @tc.name: OpenSchemaDb 007
 * @tc.desc: Verify that open new db conn must be the same as first time(common) when the original db is common db.
 * @tc.type: FUNC
 * @tc.require: SR000DR9JO
 * @tc.author: fengxiaoyun
 */
HWTEST_F(DistributeddbNbSchemaDbTest, OpenSchemaDb007, TestSize.Level1)
{
    map<string, KvStoreNbDelegate *> delegate;
    KvStoreDelegateManager *manager = nullptr;
    Option option = g_option;
    option.isMemoryDb = false;
    delegate["OriNormalDB"] = nullptr;
    delegate["OriNormalDB"] = DistributedDBNbTestTools::GetNbDelegateSuccess(manager, g_dbParameter2, option);
    ASSERT_TRUE(manager != nullptr && delegate["OriNormalDB"] != nullptr);
    EXPECT_EQ(DistributedDBNbTestTools::Put(*delegate["OriNormalDB"], KEY_1, VALUE_1), OK);
    EXPECT_TRUE(manager->CloseKvStore(delegate["OriNormalDB"]) == OK);

    /**
     * @tc.steps: step1. get delegate1 without schema.
     * @tc.expected: step1. get successfully.
     */
    option.createIfNecessary = false;
    delegate["OpenNormalDB"] = nullptr;
    delegate["OpenNormalDB"] = DistributedDBNbTestTools::GetNbDelegateSuccess(manager, g_dbParameter2, option);
    ASSERT_TRUE(manager != nullptr && delegate["OpenNormalDB"] != nullptr);

    /**
     * @tc.steps: step2.  get delegate2 with schema.
     * @tc.expected: step2. get failed and return SCHEMA_MISMATCH.
     */
    string validField = "{\"field0\":\"INTEGER,DEFAULT 10\",\"field1\":[]}";
    option.schema = SpliceToSchema(VALID_VERSION_1, VALID_MODE_1, validField, VALID_INDEX_1);
    DBStatus status;
    delegate["OpenSchemaDB"] = nullptr;
    delegate["OpenSchemaDB"] = DistributedDBNbTestTools::GetNbDelegateStatus(manager, status, g_dbParameter2, option);
    EXPECT_EQ(delegate["OpenSchemaDB"], nullptr);
    EXPECT_EQ(status, SCHEMA_MISMATCH);

    /**
     * @tc.steps: step3. get delegate3 without schema.
     * @tc.expected: step3. get successfully.
     */
    option.schema.clear();
    delegate["OpenNormalDB2"] = nullptr;
    delegate["OpenNormalDB2"] = DistributedDBNbTestTools::GetNbDelegateSuccess(manager, g_dbParameter2, option);
    ASSERT_TRUE(manager != nullptr && delegate["OpenNormalDB2"] != nullptr);

    /**
     * @tc.steps: step4. put(k1,v2),Get(k1),delete(k1) with delegate3.
     * @tc.expected: step4. put and delete successfully, Get(k1)=v2,.
     */
    EXPECT_EQ(DistributedDBNbTestTools::Put(*delegate["OpenNormalDB2"], KEY_1, VALUE_2), OK);
    Value valueResult;
    EXPECT_EQ(DistributedDBNbTestTools::Get(*delegate["OpenNormalDB2"], KEY_1, valueResult), OK);
    EXPECT_EQ(valueResult, VALUE_2);
    EXPECT_EQ(DistributedDBNbTestTools::Delete(*delegate["OpenNormalDB2"], KEY_1), OK);
    EXPECT_TRUE(manager->CloseKvStore(delegate["OpenNormalDB"]) == OK);
    EXPECT_TRUE(EndCaseDeleteDB(manager, delegate["OpenNormalDB2"], STORE_ID_2, option.isMemoryDb));
}

/**
 * @tc.name: OpenSchemaDb 008
 * @tc.desc: Verify that open new db conn must be the same as first time(common) when the original db is schema db.
 * @tc.type: FUNC
 * @tc.require: SR000DR9JO
 * @tc.author: fengxiaoyun
 */
HWTEST_F(DistributeddbNbSchemaDbTest, OpenSchemaDb008, TestSize.Level1)
{
    map<string, KvStoreNbDelegate *> delegate;
    KvStoreDelegateManager *manager = nullptr;
    Option option = g_option;
    option.isMemoryDb = false;
    string validField = "{\"field0\":\"INTEGER,NOT NULL,DEFAULT 10\",\"field1\":[]}";
    option.schema = SpliceToSchema(VALID_VERSION_1, VALID_MODE_1, validField, VALID_INDEX_1);
    delegate["OriSchemaDB"] = nullptr;
    delegate["OriSchemaDB"] = DistributedDBNbTestTools::GetNbDelegateSuccess(manager, g_dbParameter2, option);
    ASSERT_TRUE(manager != nullptr && delegate["OriSchemaDB"] != nullptr);
    string valueStr = "{\"field0\":15,\"field1\":[20]}";
    Value valueSchema(valueStr.begin(), valueStr.end());
    EXPECT_EQ(DistributedDBNbTestTools::Put(*delegate["OriSchemaDB"], KEY_1, valueSchema), OK);
    EXPECT_TRUE(manager->CloseKvStore(delegate["OriSchemaDB"]) == OK);

    /**
     * @tc.steps: step1. get delegate1 without schema.
     * @tc.expected: step1. get successfully.
     */
    option.createIfNecessary = false;
    option.schema.clear();
    delegate["OpenNormalDB"] = nullptr;
    delegate["OpenNormalDB"] = DistributedDBNbTestTools::GetNbDelegateSuccess(manager, g_dbParameter2, option);
    ASSERT_TRUE(manager != nullptr && delegate["OpenNormalDB"] != nullptr);

    /**
     * @tc.steps: step2.  get delegate2 with schema.
     * @tc.expected: step2. get failed and return SCHEMA_MISMATCH.
     */
    DBStatus status;
    option.schema = SpliceToSchema(VALID_VERSION_1, VALID_MODE_1, validField, VALID_INDEX_1);
    delegate["OpenSchemaDB"] = nullptr;
    delegate["OpenSchemaDB"] = DistributedDBNbTestTools::GetNbDelegateStatus(manager, status, g_dbParameter2, option);
    EXPECT_EQ(delegate["OpenSchemaDB"], nullptr);
    EXPECT_EQ(status, SCHEMA_MISMATCH);

    /**
     * @tc.steps: step3. get delegate3 without schema.
     * @tc.expected: step3. get successfully.
     */
    option.schema.clear();
    delegate["OpenNormalDB2"] = nullptr;
    delegate["OpenNormalDB2"] = DistributedDBNbTestTools::GetNbDelegateSuccess(manager, g_dbParameter2, option);
    ASSERT_TRUE(manager != nullptr && delegate["OpenNormalDB2"] != nullptr);

    /**
     * @tc.steps: step4. put(k1,v2),Get(k1),delete(k1) with delegate3.
     * @tc.expected: step4. put and delete successfully, Get(k1)=v2,.
     */
    EXPECT_EQ(DistributedDBNbTestTools::Put(*delegate["OpenNormalDB2"], KEY_1, VALUE_2), READ_ONLY);
    Value valueResult;
    EXPECT_EQ(DistributedDBNbTestTools::Get(*delegate["OpenNormalDB2"], KEY_1, valueResult), OK);
    EXPECT_EQ(valueResult, valueSchema);
    EXPECT_EQ(DistributedDBNbTestTools::Delete(*delegate["OpenNormalDB2"], KEY_1), READ_ONLY);
    EXPECT_TRUE(manager->CloseKvStore(delegate["OpenNormalDB"]) == OK);
    EXPECT_TRUE(EndCaseDeleteDB(manager, delegate["OpenNormalDB2"], STORE_ID_2, option.isMemoryDb));
}

/**
 * @tc.name: OpenSchemaDb 009
 * @tc.desc: Verify that open new db conn must be the same as first time(schema) when the original db is common db.
 * @tc.type: FUNC
 * @tc.require: SR000DR9JO
 * @tc.author: fengxiaoyun
 */
HWTEST_F(DistributeddbNbSchemaDbTest, OpenSchemaDb009, TestSize.Level1)
{
    map<string, KvStoreNbDelegate *> delegate;
    KvStoreDelegateManager *manager = nullptr;
    Option option = g_option;
    option.isMemoryDb = false;
    delegate["OriNormalDB"] = nullptr;
    delegate["OriNormalDB"] = DistributedDBNbTestTools::GetNbDelegateSuccess(manager, g_dbParameter2, option);
    ASSERT_TRUE(manager != nullptr && delegate["OriNormalDB"] != nullptr);
    EXPECT_TRUE(manager->CloseKvStore(delegate["OriNormalDB"]) == OK);

    /**
     * @tc.steps: step1. get delegate1 with schema.
     * @tc.expected: step1. get successfully.
     */
    option.createIfNecessary = false;
    string validField = "{\"field0\":\"INTEGER,NOT NULL,DEFAULT 10\",\"field1\":[]}";
    option.schema = SpliceToSchema(VALID_VERSION_1, VALID_MODE_1, validField, VALID_INDEX_1);
    delegate["FirstOpenSchema"] = nullptr;
    delegate["FirstOpenSchema"] = DistributedDBNbTestTools::GetNbDelegateSuccess(manager, g_dbParameter2, option);
    ASSERT_TRUE(manager != nullptr && delegate["FirstOpenSchema"] != nullptr);

    /**
     * @tc.steps: step2.  get delegate2 without schema.
     * @tc.expected: step2. get failed and return SCHEMA_MISMATCH.
     */
    option.schema.clear();
    DBStatus status;
    delegate["OpenNormal"] = nullptr;
    delegate["OpenNormal"] = DistributedDBNbTestTools::GetNbDelegateStatus(manager, status, g_dbParameter2, option);
    EXPECT_EQ(delegate["OpenNormal"], nullptr);
    EXPECT_EQ(status, SCHEMA_MISMATCH);

    /**
     * @tc.steps: step3. get delegate3 with schema2.
     * @tc.expected: step3. get failed and return SCHEMA_MISMATCH.
     */
    option.schema.clear();
    string validField1 = "{\"field0\":\"INTEGER,NOT NULL,DEFAULT 10\"}";
    option.schema = SpliceToSchema(VALID_VERSION_1, VALID_MODE_1, validField1, VALID_INDEX_1);
    delegate["OpenDiffSchema"] = nullptr;
    delegate["OpenDiffSchema"] = DistributedDBNbTestTools::GetNbDelegateStatus(manager, status, g_dbParameter2, option);
    EXPECT_EQ(delegate["OpenDiffSchema"], nullptr);
    EXPECT_EQ(status, SCHEMA_MISMATCH);

    /**
     * @tc.steps: step4. get delegate4 with schema.
     * @tc.expected: step4.get successfully.
     */
    option.schema.clear();
    option.schema = SpliceToSchema(VALID_VERSION_1, VALID_MODE_1, validField, VALID_INDEX_1);
    delegate["OpenSameSchema"] = nullptr;
    delegate["OpenSameSchema"] = DistributedDBNbTestTools::GetNbDelegateSuccess(manager, g_dbParameter2, option);
    ASSERT_TRUE(manager != nullptr && delegate["OpenSameSchema"] != nullptr);
    // close all opened delegates.
    EXPECT_TRUE(manager->CloseKvStore(delegate["OpenSameSchema"]) == OK);
    EXPECT_TRUE(manager->CloseKvStore(delegate["FirstOpenSchema"]) == OK);
    delegate.clear();

    /**
     * @tc.steps: step5. get delegate5 without schema after close all delegates that were opened.
     * @tc.expected: step4.get successfully.
     */
    option.schema.clear();
    delegate["ReopenNormal"] = nullptr;
    delegate["ReopenNormal"] = DistributedDBNbTestTools::GetNbDelegateSuccess(manager, g_dbParameter2, option);
    ASSERT_TRUE(manager != nullptr && delegate["ReopenNormal"] != nullptr);
    EXPECT_TRUE(EndCaseDeleteDB(manager, delegate["ReopenNormal"], STORE_ID_2, option.isMemoryDb));
}

/**
 * @tc.name: OpenSchemaDb 010
 * @tc.desc: Verify that open new db conn must be the same as first time(schema) when the original db is schema db.
 * @tc.type: FUNC
 * @tc.require: SR000DR9JO
 * @tc.author: fengxiaoyun
 */
HWTEST_F(DistributeddbNbSchemaDbTest, OpenSchemaDb010, TestSize.Level1)
{
    std::map<std::string, KvStoreNbDelegate *> delegateGroup;
    KvStoreDelegateManager *manager = nullptr;
    Option option = g_option;
    option.isMemoryDb = false;
    string validField = "{\"field0\":\"INTEGER,NOT NULL,DEFAULT 10\",\"field1\":[]}";
    option.schema = SpliceToSchema(VALID_VERSION_1, VALID_MODE_1, validField, VALID_INDEX_1);
    delegateGroup["OriSchemaDB"] = nullptr;
    delegateGroup["OriSchemaDB"] = DistributedDBNbTestTools::GetNbDelegateSuccess(manager, g_dbParameter2, option);
    ASSERT_TRUE(manager != nullptr && delegateGroup["OriSchemaDB"] != nullptr);
    EXPECT_TRUE(manager->CloseKvStore(delegateGroup["OriSchemaDB"]) == OK);
    delegateGroup["OriSchemaDB"] = nullptr;
    delete manager;
    manager = nullptr;

    /**
     * @tc.steps: step1. get delegate1 with schema.
     * @tc.expected: step1. get successfully.
     */
    option.createIfNecessary = false;
    delegateGroup["OriSchemaDB"] = DistributedDBNbTestTools::GetNbDelegateSuccess(manager, g_dbParameter2, option);
    ASSERT_TRUE(manager != nullptr && delegateGroup["OriSchemaDB"] != nullptr);

    /**
     * @tc.steps: step2.  get delegate2 without schema.
     * @tc.expected: step2. get failed and return SCHEMA_MISMATCH.
     */
    delegateGroup["NormalDB"] = nullptr;
    option.schema.clear();
    DBStatus status;
    delegateGroup["NormalDB"] = DistributedDBNbTestTools::GetNbDelegateStatus(manager, status,
        g_dbParameter2, option);
    EXPECT_EQ(delegateGroup["NormalDB"], nullptr);
    EXPECT_EQ(status, SCHEMA_MISMATCH);

    /**
     * @tc.steps: step3. get delegate3 with schema2.
     * @tc.expected: step3. get failed and return SCHEMA_MISMATCH.
     */
    delegateGroup["DiffSchemaDB"] = nullptr;
    option.schema.clear();
    string validField1 = "{\"field0\":\"INTEGER,NOT NULL,DEFAULT 10\"}";
    option.schema = SpliceToSchema(VALID_VERSION_1, VALID_MODE_1, validField1, VALID_INDEX_1);
    delegateGroup["DiffSchemaDB"] = DistributedDBNbTestTools::GetNbDelegateStatus(manager, status,
        g_dbParameter2, option);
    EXPECT_EQ(delegateGroup["DiffSchemaDB"], nullptr);
    EXPECT_EQ(status, SCHEMA_MISMATCH);

    /**
     * @tc.steps: step4. get delegate4 with schema.
     * @tc.expected: step4.get successfully.
     */
    delegateGroup["SameSchemaDB"] = nullptr;
    option.schema.clear();
    option.schema = SpliceToSchema(VALID_VERSION_1, VALID_MODE_1, validField, VALID_INDEX_1);
    delegateGroup["SameSchemaDB"] = DistributedDBNbTestTools::GetNbDelegateSuccess(manager, g_dbParameter2, option);
    ASSERT_TRUE(manager != nullptr && delegateGroup["SameSchemaDB"] != nullptr);
    // Close all opened delegate
    EXPECT_TRUE(manager->CloseKvStore(delegateGroup["OriSchemaDB"]) == OK);
    EXPECT_TRUE(manager->CloseKvStore(delegateGroup["SameSchemaDB"]) == OK);
    delegateGroup.clear();

    /**
     * @tc.steps: step5. get delegate5 without schema after close all delegates that were opened.
     * @tc.expected: step4.get successfully.
     */
    delegateGroup["NormalDB"] = nullptr;
    option.schema.clear();
    delegateGroup["NormalDB"] = DistributedDBNbTestTools::GetNbDelegateSuccess(manager, g_dbParameter2, option);
    ASSERT_TRUE(manager != nullptr && delegateGroup["NormalDB"] != nullptr);
    EXPECT_TRUE(EndCaseDeleteDB(manager, delegateGroup["NormalDB"], STORE_ID_2, option.isMemoryDb));
}

/**
 * @tc.name: SchemaPut 001
 * @tc.desc: Verify that PUT value will return OK if the format is the same as define of schema.
 * @tc.type: FUNC
 * @tc.require: SR000DR9JO
 * @tc.author: fengxiaoyun
 */
HWTEST_F(DistributeddbNbSchemaDbTest, SchemaPut001, TestSize.Level0)
{
    KvStoreNbDelegate *delegate = nullptr;
    KvStoreDelegateManager *manager = nullptr;
    Option option = g_option;
    option.isMemoryDb = false;
    option.schema = SpliceToSchema(VALID_VERSION_1, VALID_MODE_1, VALID_DEFINE_1, VALID_INDEX_2);
    delegate = DistributedDBNbTestTools::GetNbDelegateSuccess(manager, g_dbParameter2, option);
    ASSERT_TRUE(manager != nullptr && delegate != nullptr);

    /**
     * @tc.steps: step1. put(k1,value1) that value1's format is the same as define of schema.
     * @tc.expected: step1. put successfully.
     */
    string valueRes1;
    valueRes1 = valueRes1 + "{" + VALUE_MATCH_1 + "," + VALUE_MATCH_2 + "}";
    Value valueSchema1(valueRes1.begin(), valueRes1.end());
    EXPECT_EQ(DistributedDBNbTestTools::Put(*delegate, KEY_1, valueSchema1), OK);

    /**
     * @tc.steps: step2. put(k2,value2) that value2's format is the same as define of schema except there exist an
     *     repeat field in the same level.
     * @tc.expected: step2. put successfully.
     */
    string valueRes2;
    valueRes2 = valueRes2 + "{" + VALUE_MATCH_1 + "," + VALUE_MATCH_2 + "," + "\"field17\": 20000000" + "}";
    Value valueSchema2(valueRes2.begin(), valueRes2.end());
    EXPECT_EQ(DistributedDBNbTestTools::Put(*delegate, KEY_2, valueSchema2), OK);

    /**
     * @tc.steps: step3. put(k3,value3) that value3's format is the same as define of schema except the order of
     *    fields in the same level is being upset.
     * @tc.expected: step3. put successfully.
     */
    string valueRes3;
    valueRes3 = valueRes3 + "{" + VALUE_MATCH_2 + "," + VALUE_MATCH_1 + "}";
    Value valueSchema3(valueRes3.begin(), valueRes3.end());
    EXPECT_EQ(DistributedDBNbTestTools::Put(*delegate, KEY_3, valueSchema3), OK);

    /**
     * @tc.steps: step4. Get(k1,k2,k3).
     * @tc.expected: step4. Get(k1)=value1, Get(k2)=value2, Get(k3)=value3.
     */
    Value valueResult;
    EXPECT_EQ(DistributedDBNbTestTools::Get(*delegate, KEY_1, valueResult), OK);
    EXPECT_EQ(valueResult, valueSchema1);
    EXPECT_EQ(DistributedDBNbTestTools::Get(*delegate, KEY_2, valueResult), OK);
    EXPECT_EQ(valueResult, valueSchema2);
    EXPECT_EQ(DistributedDBNbTestTools::Get(*delegate, KEY_3, valueResult), OK);
    EXPECT_EQ(valueResult, valueSchema3);
    EXPECT_TRUE(EndCaseDeleteDB(manager, delegate, STORE_ID_2, option.isMemoryDb));
}

/**
 * @tc.name: SchemaPut 002
 * @tc.desc: Verify that PUT value will return INVALID_VALUE_FIELDS if the format is different from define of schema
 *     when SCHEMA_MODE = STRICT.
 * @tc.type: FUNC
 * @tc.require: SR000DR9JO
 * @tc.author: fengxiaoyun
 */
HWTEST_F(DistributeddbNbSchemaDbTest, SchemaPut002, TestSize.Level0)
{
    KvStoreNbDelegate *delegate = nullptr;
    KvStoreDelegateManager *manager = nullptr;
    Option option = g_option;
    option.isMemoryDb = false;
    option.schema = SpliceToSchema(VALID_VERSION_1, VALID_MODE_1, VALID_DEFINE_1, VALID_INDEX_1);
    delegate = DistributedDBNbTestTools::GetNbDelegateSuccess(manager, g_dbParameter2, option);
    ASSERT_TRUE(manager != nullptr && delegate != nullptr);

    /**
     * @tc.steps: step1. put(k1,value1) that value1's field is less than define of schema.
     * @tc.expected: step1. put ok(less field no notnull).
     */
    string valueMatch = VALUE_MATCH_2;
    valueMatch.erase(valueMatch.size() - 9, 9); // erase 9 chars starting at 9 st last.
    string valueRes1;
    valueRes1 = valueRes1 + "{" + VALUE_MATCH_1 + "," + valueMatch + "}";
    Value valueSchema1(valueRes1.begin(), valueRes1.end());
    EXPECT_EQ(DistributedDBNbTestTools::Put(*delegate, KEY_1, valueSchema1), OK);

    /**
     * @tc.steps: step2. put(k2,value2) that value2's field is more than define of schema.
     * @tc.expected: step2. put failed.
     */
    string valueRes2;
    valueRes2 = valueRes2 + "{" + VALUE_MATCH_1 + "," + VALUE_MATCH_2 + "," + "\"field19\": 20" + "}";
    Value valueSchema2(valueRes2.begin(), valueRes2.end());
    EXPECT_EQ(DistributedDBNbTestTools::Put(*delegate, KEY_2, valueSchema2), INVALID_VALUE_FIELDS);

    /**
     * @tc.steps: step3. put(k3,value3) that there exist a field is not start with char or _ in value3.
     * @tc.expected: step3. put failed.
     */
    string valueRes3;
    valueRes3 = valueRes3 + "{" + VALUE_MATCH_1 + "," + VALUE_MATCH_2 + "," + "\"19field\": 20" + "}";
    Value valueSchema3(valueRes3.begin(), valueRes3.end());
    EXPECT_EQ(DistributedDBNbTestTools::Put(*delegate, KEY_3, valueSchema3), INVALID_VALUE_FIELDS);

    /**
     * @tc.steps: step4. put(k4,value4) that there exist a field'level is upseted in value4.
     * @tc.expected: step4. put failed(less field notnull).
     */
    string valueRes = "\"_field1\":{\"field1\":\"abc\",\"field2\":null,\"field3\":{\"field6\":\"def\","
        "\"field5\":{\"field4\":\"fxy\",\"field7\":[]}}}";
    string valueRes4;
    valueRes4 = valueRes4 + "{" + valueRes + "," + VALUE_MATCH_2 + "}";
    Value valueSchema4(valueRes4.begin(), valueRes4.end());
    EXPECT_EQ(DistributedDBNbTestTools::Put(*delegate, KEY_4, valueSchema4), CONSTRAIN_VIOLATION);
    EXPECT_TRUE(EndCaseDeleteDB(manager, delegate, STORE_ID_2, option.isMemoryDb));
}

bool PutAfterReplace(string &operateStr, const string findStr, const vector<string> replaceStr,
    KvStoreNbDelegate *&delegate)
{
    if (findStr == "[]" || findStr == "{}") {
        string valueMatch = operateStr;
        if (valueMatch.find(findStr) == string::npos) {
            MST_LOG("Can't find findStr in operateStr!");
            return false;
        }
        valueMatch.insert(valueMatch.find(findStr) + 1, "\"field8\"\"error\"");
        string valueRes;
        if (operateStr == VALUE_MATCH_1) {
            valueRes = valueRes + "{" + valueMatch + "," + VALUE_MATCH_2 + "}";
        } else {
            valueRes = valueRes + "{" + VALUE_MATCH_1 + "," + valueMatch + "}";
        }
        Value valueSchema(valueRes.begin(), valueRes.end());
        MST_LOG("valueSchema is %s", valueRes.c_str());
        EXPECT_EQ(DistributedDBNbTestTools::Put(*delegate, KEY_1, valueSchema), INVALID_FORMAT);
        return true;
    }
    for (const auto &iter : replaceStr) {
        string valueMatch = operateStr;
        if (valueMatch.find(findStr) == string::npos) {
            MST_LOG("Can't find findStr in operateStr!");
            return false;
        }
        valueMatch.replace(valueMatch.find(findStr), findStr.size(), iter);
        string valueRes;
        if (operateStr == VALUE_MATCH_1) {
            valueRes = valueRes + "{" + valueMatch + "," + VALUE_MATCH_2 + "}";
        } else {
            valueRes = valueRes + "{" + VALUE_MATCH_1 + "," + valueMatch + "}";
        }
        Value valueSchema(valueRes.begin(), valueRes.end());
        MST_LOG("valueSchema is %s", valueRes.c_str());
        if (iter == "null") {
            EXPECT_EQ(DistributedDBNbTestTools::Put(*delegate, KEY_1, valueSchema), CONSTRAIN_VIOLATION);
        } else if (iter == "0X2B") {
            EXPECT_EQ(DistributedDBNbTestTools::Put(*delegate, KEY_1, valueSchema), INVALID_FORMAT);
        } else {
            EXPECT_EQ(DistributedDBNbTestTools::Put(*delegate, KEY_1, valueSchema), INVALID_FIELD_TYPE);
        }
    }
    return true;
}
#ifdef USING_NEW_VER_JSONCPP
/**
 * @tc.name: SchemaPut 003
 * @tc.desc: Verify that PUT value will return error if the field attributes are different with define of schema
 *     when SCHEMA_MODE = STRICT.
 * @tc.type: FUNC
 * @tc.require: SR000DR9JO
 * @tc.author: fengxiaoyun
 */
HWTEST_F(DistributeddbNbSchemaDbTest, SchemaPut003, TestSize.Level0)
{
    KvStoreNbDelegate *delegate = nullptr;
    KvStoreDelegateManager *manager = nullptr;
    Option option = g_option;
    option.isMemoryDb = false;
    option.schema = SpliceToSchema(VALID_VERSION_1, VALID_MODE_1, VALID_DEFINE_1, VALID_INDEX_1);
    delegate = DistributedDBNbTestTools::GetNbDelegateSuccess(manager, g_dbParameter2, option);
    ASSERT_TRUE(manager != nullptr && delegate != nullptr);

    /**
     * @tc.steps: step1-2. put(k1,value1) that value1's type or not_null is different with define of schema.
     * @tc.expected: step1-2. put failed.
     */
    // the define of schema's type is string
    string findStr1 = "\"abc\"";
    vector<string> replaceStr1 = {"123", "true", "false", "null"};
    EXPECT_TRUE(PutAfterReplace(VALUE_MATCH_1, findStr1, replaceStr1, delegate));

    // the define of schema's type is bool
    string findStr2 = "false";
    vector<string> replaceStr2 = {"\"false\"", "\"true\"", "null"};
    EXPECT_TRUE(PutAfterReplace(VALUE_MATCH_2, findStr2, replaceStr2, delegate));

    // the define of schema's type is INTEGER
    string findStr3 = "-1000000";
    vector<string> replaceStr3 = {"\"-1000000\"", "10.5", "-10.00001", std::to_string(INT32_MAX) + "1",
        std::to_string(INT32_MIN) + "1", "0X2B", "null"};
    EXPECT_TRUE(PutAfterReplace(VALUE_MATCH_2, findStr3, replaceStr3, delegate));

    // the define of schema's type is LONG
    string findStr4 = "666";
    vector<string> replaceStr4 = {"\"666\"", "10.5", "10.10", std::to_string(INT64_MAX) + "1",
        std::to_string(INT64_MIN) + "1", "0X2B", + "null"};
    EXPECT_TRUE(PutAfterReplace(VALUE_MATCH_2, findStr4, replaceStr4, delegate));

    // the define of schema's type is DOUBLE
    string findStr5 = "-1.05e-4";
    vector<string> replaceStr5 = {"\"-1.05e4\"", "1" + std::to_string(DBL_MAX),
        "-1" + std::to_string(DBL_MAX), "0X2B", "null"};
    EXPECT_TRUE(PutAfterReplace(VALUE_MATCH_2, findStr5, replaceStr5, delegate));

    /**
     * @tc.steps: step3. put(k2,value2) that value2 isn't Json format.
     * @tc.expected: step3. put failed.
     */
    string valueRes;
    valueRes = valueRes + "{" + VALUE_MATCH_1 + "," + VALUE_MATCH_2 + "," + "}";
    Value valueSchema(valueRes.begin(), valueRes.end());
    EXPECT_EQ(DistributedDBNbTestTools::Put(*delegate, KEY_2, valueSchema), INVALID_FORMAT);

    /**
     * @tc.steps: step4. put(k3,value3) that value3's Json array or Json object isn't Json format.
     * @tc.expected: step4. put failed.
     */
    string findStr6 = "[]";
    vector<string> replaceStr6 = {""};
    EXPECT_TRUE(PutAfterReplace(VALUE_MATCH_1, findStr6, replaceStr6, delegate));
    string findStr7 = "{}";
    EXPECT_TRUE(PutAfterReplace(VALUE_MATCH_2, findStr7, replaceStr6, delegate));
    EXPECT_TRUE(EndCaseDeleteDB(manager, delegate, STORE_ID_2, option.isMemoryDb));
}
#endif
/**
 * @tc.name: SchemaPut 004
 * @tc.desc: Verify that PUT value will return OK if the format is the same as define of schema with
 *     SCHEMA_MODE = COMPATIBLE.
 * @tc.type: FUNC
 * @tc.require: SR000DR9JO
 * @tc.author: fengxiaoyun
 */
HWTEST_F(DistributeddbNbSchemaDbTest, SchemaPut004, TestSize.Level0)
{
    KvStoreNbDelegate *delegate = nullptr;
    KvStoreDelegateManager *manager = nullptr;
    Option option = g_option;
    option.isMemoryDb = false;
    option.schema = SpliceToSchema(VALID_VERSION_1, VALID_MODE_2, VALID_DEFINE_1, VALID_INDEX_2);
    delegate = DistributedDBNbTestTools::GetNbDelegateSuccess(manager, g_dbParameter2, option);
    ASSERT_TRUE(manager != nullptr && delegate != nullptr);

    /**
     * @tc.steps: step1. put(k1,value1) that value1's format is the same as define of schema.
     * @tc.expected: step1. put successfully.
     */
    string valueRes1;
    valueRes1 = valueRes1 + "{" + VALUE_MATCH_1 + "," + VALUE_MATCH_2 + "}";
    Value valueSchema1(valueRes1.begin(), valueRes1.end());
    EXPECT_EQ(DistributedDBNbTestTools::Put(*delegate, KEY_1, valueSchema1), OK);

    /**
     * @tc.steps: step2. put(k2,value2) that value2's format is the same as define of schema except there exist an
     *     repeat field in the same level.
     * @tc.expected: step2. put successfully.
     */
    string valueRes2;
    valueRes2 = valueRes2 + "{" + VALUE_MATCH_1 + "," + VALUE_MATCH_2 + "," + "\"field17\": 20000000" + "}";
    Value valueSchema2(valueRes2.begin(), valueRes2.end());
    EXPECT_EQ(DistributedDBNbTestTools::Put(*delegate, KEY_2, valueSchema2), OK);

    /**
     * @tc.steps: step3. put(k3,value3) that value3's format is the same as define of schema except the order of
     *    fields in the same level is being upset.
     * @tc.expected: step3. put successfully.
     */
    string valueRes3;
    valueRes3 = valueRes3 + "{" + VALUE_MATCH_2 + "," + VALUE_MATCH_1 + "}";
    Value valueSchema3(valueRes3.begin(), valueRes3.end());
    EXPECT_EQ(DistributedDBNbTestTools::Put(*delegate, KEY_3, valueSchema3), OK);

    /**
     * @tc.steps: step3. put(k4,value4) that value4's field is more than define of schema.
     * @tc.expected: step3. put successfully.
     */
    string valueRes4;
    valueRes4 = valueRes4 + "{" + VALUE_MATCH_2 + "," + VALUE_MATCH_1 + "," + "\"field20\": 2e7" + "}";
    Value valueSchema4(valueRes4.begin(), valueRes4.end());
    EXPECT_EQ(DistributedDBNbTestTools::Put(*delegate, KEY_4, valueSchema4), OK);

    /**
     * @tc.steps: step4. Get(k1,k2,k3,k4).
     * @tc.expected: step4. Get(k1)=value1, Get(k2)=value2, Get(k3)=value3, Get(k4)=value4.
     */
    Value valueResult;
    EXPECT_EQ(DistributedDBNbTestTools::Get(*delegate, KEY_1, valueResult), OK);
    EXPECT_EQ(valueResult, valueSchema1);
    EXPECT_EQ(DistributedDBNbTestTools::Get(*delegate, KEY_2, valueResult), OK);
    EXPECT_EQ(valueResult, valueSchema2);
    EXPECT_EQ(DistributedDBNbTestTools::Get(*delegate, KEY_3, valueResult), OK);
    EXPECT_EQ(valueResult, valueSchema3);
    EXPECT_EQ(DistributedDBNbTestTools::Get(*delegate, KEY_4, valueResult), OK);
    EXPECT_EQ(valueResult, valueSchema4);
    EXPECT_TRUE(EndCaseDeleteDB(manager, delegate, STORE_ID_2, option.isMemoryDb));
}

/**
 * @tc.name: SchemaPut 005
 * @tc.desc: Verify that PUT value will return error if the format is different from define of schema
 *     when SCHEMA_MODE = COMPATIBLE.
 * @tc.type: FUNC
 * @tc.require: SR000DR9JO
 * @tc.author: fengxiaoyun
 */
HWTEST_F(DistributeddbNbSchemaDbTest, SchemaPut005, TestSize.Level0)
{
    KvStoreNbDelegate *delegate = nullptr;
    KvStoreDelegateManager *manager = nullptr;
    Option option = g_option;
    option.isMemoryDb = false;
    option.schema = SpliceToSchema(VALID_VERSION_1, VALID_MODE_2, VALID_DEFINE_1, VALID_INDEX_1);
    delegate = DistributedDBNbTestTools::GetNbDelegateSuccess(manager, g_dbParameter2, option);
    ASSERT_TRUE(manager != nullptr && delegate != nullptr);

    /**
     * @tc.steps: step1. put(k1,value1) that value1's field is less than define of schema.
     * @tc.expected: step1. put ok(lack field no notnull).
     */
    string valueMatch = VALUE_MATCH_2;
    valueMatch.erase(valueMatch.size() - 9, 9); // erase 9 chars starting at 9 st last.
    string valueRes1;
    valueRes1 = valueRes1 + "{" + VALUE_MATCH_1 + "," + valueMatch + "}";
    Value valueSchema1(valueRes1.begin(), valueRes1.end());
    EXPECT_EQ(DistributedDBNbTestTools::Put(*delegate, KEY_1, valueSchema1), OK);

    /**
     * @tc.steps: step2. put(k2,value2) that value2's field is more than define of schema.
     * @tc.expected: step2. put failed.
     */
    string valueRes2;
    valueRes2 = valueRes2 + "{" + VALUE_MATCH_1 + "," + VALUE_MATCH_2 + "," + "\"19field\": 20" + "}";
    Value valueSchema2(valueRes2.begin(), valueRes2.end());
    EXPECT_EQ(DistributedDBNbTestTools::Put(*delegate, KEY_2, valueSchema2), OK);

    /**
     * @tc.steps: step3. put(k3,value3) that there exist a field'level is upseted in value4.
     * @tc.expected: step3. put failed(lack field notnull).
     */
    string valueRes = "\"_field1\":{\"field1\":\"abc\",\"field2\":null,\"field3\":{\"field6\":\"def\","
    "\"field5\":{\"field4\":\"fxy\",\"field7\":[]}}}";
    string valueRes3;
    valueRes3 = valueRes3 + "{" + valueRes + "," + VALUE_MATCH_2 + "}";
    Value valueSchema3(valueRes3.begin(), valueRes3.end());
    EXPECT_EQ(DistributedDBNbTestTools::Put(*delegate, KEY_3, valueSchema3), CONSTRAIN_VIOLATION);

    /**
     * @tc.steps: step4. put(k4,value4) that value4 isn't Json format.
     * @tc.expected: step4. put failed.
     */
    string valueRes4;
    valueRes4 = valueRes4 + "{" + VALUE_MATCH_1 + "," + VALUE_MATCH_2 + "," + "}";
    Value valueSchema4(valueRes4.begin(), valueRes4.end());
    EXPECT_EQ(DistributedDBNbTestTools::Put(*delegate, KEY_4, valueSchema4), INVALID_FORMAT);

    /**
     * @tc.steps: step4. put(k5,value5) that value5's Json array or Json object isn't Json format.
     * @tc.expected: step4. put failed.
     */
    string findStr = "[]";
    vector<string> replaceStr = {""};
    PutAfterReplace(VALUE_MATCH_1, findStr, replaceStr, delegate);
    string findStr1 = "{}";
    PutAfterReplace(VALUE_MATCH_2, findStr1, replaceStr, delegate);
    EXPECT_TRUE(EndCaseDeleteDB(manager, delegate, STORE_ID_2, option.isMemoryDb));
}
#ifdef USING_NEW_VER_JSONCPP
/**
 * @tc.name: SchemaPut 006
 * @tc.desc: Verify that PUT value will return error if the field attributes are different with define of schema
 *     when SCHEMA_MODE = COMPATIBLE.
 * @tc.type: FUNC
 * @tc.require: SR000DR9JO
 * @tc.author: fengxiaoyun
 */
HWTEST_F(DistributeddbNbSchemaDbTest, SchemaPut006, TestSize.Level0)
{
    KvStoreNbDelegate *delegate = nullptr;
    KvStoreDelegateManager *manager = nullptr;
    Option option = g_option;
    option.isMemoryDb = false;
    option.schema = SpliceToSchema(VALID_VERSION_1, VALID_MODE_2, VALID_DEFINE_1, VALID_INDEX_1);
    delegate = DistributedDBNbTestTools::GetNbDelegateSuccess(manager, g_dbParameter2, option);
    ASSERT_TRUE(manager != nullptr && delegate != nullptr);

    /**
     * @tc.steps: step1-2. put(k1,value1) that value1's type or not_null is different with define of schema.
     * @tc.expected: step1-2. put failed.
     */
    // the define of schema's type is string
    string findStr1 = "\"abc\"";
    vector<string> replaceStr1 = {"123", "true", "false", "null"};
    PutAfterReplace(VALUE_MATCH_1, findStr1, replaceStr1, delegate);

    // the define of schema's type is bool
    string findStr2 = "false";
    vector<string> replaceStr2 = {"\"false\"", "\"true\"", "null"};
    PutAfterReplace(VALUE_MATCH_2, findStr2, replaceStr2, delegate);

    // the define of schema's type is INTEGER
    string findStr3 = "-1000000";
    vector<string> replaceStr3 = {"\"-1000000\"", "10.5", "-10.00001", std::to_string(INT32_MAX) + "1",
        std::to_string(INT32_MIN) + "1", "0X2B", "null"};
    PutAfterReplace(VALUE_MATCH_2, findStr3, replaceStr3, delegate);

    // the define of schema's type is LONG
    string findStr4 = "666";
    vector<string> replaceStr4 = {"\"666\"", "10.5", "-10.10", std::to_string(INT64_MAX) + "1",
        std::to_string(INT64_MIN) + "1", "0X2B", "null"};
    PutAfterReplace(VALUE_MATCH_2, findStr4, replaceStr4, delegate);

    // the define of schema's type is DOUBLE
    string findStr5 = "-1.05e-4";
    vector<string> replaceStr5 = {"\"-1.05e4\"", "1" + std::to_string(DBL_MAX),
        "-1" + std::to_string(DBL_MAX), "0X2B", "null"};
    PutAfterReplace(VALUE_MATCH_2, findStr5, replaceStr5, delegate);
    EXPECT_TRUE(EndCaseDeleteDB(manager, delegate, STORE_ID_2, option.isMemoryDb));
}
#endif
/**
 * @tc.name: SchemaPut 007
 * @tc.desc: Verify that if there are duplicate fields in schema, the latter will prevail when put.
 * @tc.type: FUNC
 * @tc.require: SR000DR9JO
 * @tc.author: fengxiaoyun
 */
HWTEST_F(DistributeddbNbSchemaDbTest, SchemaPut007, TestSize.Level0)
{
    KvStoreNbDelegate *delegate = nullptr;
    KvStoreDelegateManager *manager = nullptr;
    Option option = g_option;
    option.isMemoryDb = false;
    option.schema = SpliceToSchema(VALID_VERSION_1, VALID_MODE_1, VALID_DEFINE_1, VALID_INDEX_2);
    delegate = DistributedDBNbTestTools::GetNbDelegateSuccess(manager, g_dbParameter2, option);
    ASSERT_TRUE(manager != nullptr && delegate != nullptr);

    /**
     * @tc.steps: step1. put(k1,value1) that value1 transfer all the value that repeat field in the same level.
     * @tc.expected: step1. put failed.
     */
    string valueRes1;
    valueRes1 = valueRes1 + "{" + VALUE_MATCH_1 + "," + VALUE_MATCH_2 + ",\"field1\": \"false\"" + "}";
    Value valueSchema1(valueRes1.begin(), valueRes1.end());
    MST_LOG("valueRes1 is %s", valueRes1.c_str());
    EXPECT_EQ(DistributedDBNbTestTools::Put(*delegate, KEY_1, valueSchema1), INVALID_FIELD_TYPE);

    /**
     * @tc.steps: step2.4. put(k2,value2) that value2's format is the same as define of schema.
     * @tc.expected: step2.4. put successfully.
     */
    string valueRes2;
    valueRes2 = valueRes2 + "{" + VALUE_MATCH_1 + "," + VALUE_MATCH_2 + "}";
    Value valueSchema2(valueRes2.begin(), valueRes2.end());
    MST_LOG("valueRes2 is %s", valueRes2.c_str());
    EXPECT_EQ(DistributedDBNbTestTools::Put(*delegate, KEY_2, valueSchema2), OK);

    /**
     * @tc.steps: step3. put(k3,value3) that value3 transfer only one that repeat field in the different level.
     * @tc.expected: step3. put failed(field require not null but has default value).
     */
    string deleteStr = VALUE_MATCH_1;
    string findStr = "\"field1\":\"abc\",";
    deleteStr.erase(deleteStr.find(findStr), findStr.size());
    string valueRes3;
    valueRes3 = valueRes3 + "{" + deleteStr + "," + VALUE_MATCH_2 + "}";
    Value valueSchema3(valueRes3.begin(), valueRes3.end());
    EXPECT_EQ(DistributedDBNbTestTools::Put(*delegate, KEY_3, valueSchema3), OK);

    /**
     * @tc.steps: step5. put(k4,value4) that value4's Json Array has large fields so that the total fields are
     *     over 256.
     * @tc.expected: step5. put successfully.
     */
    LongDefine param;
    param.recordNum = TWO_FIVE_SIX_RECORDS;
    param.recordSize = RECORDSIZE;
    param.prefix = 'k';
    string longField;
    GetLongSchemaDefine(param, longField);
    string valueMatch = VALUE_MATCH_1;
    valueMatch.insert(valueMatch.find("[]") + 1, longField);
    string valueRes4;
    valueRes4 = valueRes4 + "{" + valueMatch + "," + VALUE_MATCH_2 + "}";
    Value valueSchema4(valueRes4.begin(), valueRes4.end());
    EXPECT_EQ(DistributedDBNbTestTools::Put(*delegate, KEY_4, valueSchema4), OK);

    /**
     * @tc.steps: step6. put(k5,value5) that value5's Json Object has large fields so that the total fields are
     *     over 256.
     * @tc.expected: step6. put successfully.
     */
    string valueMatch2 = VALUE_MATCH_2;
    longField.erase(0, 1);
    longField.erase(longField.size() - 1, 1);
    valueMatch2.insert(valueMatch2.find("{}") + 1, longField);
    string valueRes5;
    valueRes5 = valueRes5 + "{" + VALUE_MATCH_1 + "," + valueMatch2 + "}";
    Value valueSchema5(valueRes5.begin(), valueRes5.end());
    EXPECT_EQ(DistributedDBNbTestTools::Put(*delegate, KEY_5, valueSchema5), OK);
    EXPECT_TRUE(EndCaseDeleteDB(manager, delegate, STORE_ID_2, option.isMemoryDb));
}

bool SchemaIndexQuery(const DBParameters parameters, const std::string dbPath, const std::string schemaIndex)
{
    int count = 0;
    std::string identifier = parameters.userId + "-" + parameters.appId + "-" + parameters.storeId;
    std::string hashIdentifierRes = TransferStringToHashHexString(identifier);
    const std::string mainDbName = dbPath + hashIdentifierRes + DATABASE_INFOR_FILE;
    EncrypteAttribute attribute = {g_option.isEncryptedDb, g_option.passwd};

    const std::string SCHEMA_INDEX_QUERY_SQL = "select count(*) from sqlite_master where name= \'"
        + schemaIndex + "\' and type = \'index\'";

    DistributedTestTools::QuerySpecifiedData(mainDbName, SCHEMA_INDEX_QUERY_SQL, attribute, count);

    return (count == 1);
}
/**
 * @tc.name: SchemaIndex 001
 * @tc.desc: Verify that the schema db created with non-index mode can be upgrade to index mode when open it again with
 *    schema that include schema_index.
 * @tc.type: FUNC
 * @tc.require: SR000DR9JQ
 * @tc.author: fengxiaoyun
 */
HWTEST_F(DistributeddbNbSchemaDbTest, SchemaIndex001, TestSize.Level1)
{
    /**
     * @tc.steps: step1. create schema db that without schema_index and close but don't delete it.
     * @tc.expected: step1. operate successfully.
     */
    KvStoreNbDelegate *delegate = nullptr;
    KvStoreDelegateManager *manager = nullptr;
    Option option = g_option;
    option.isMemoryDb = false;
    option.schema = SpliceToSchema(VALID_VERSION_1, VALID_MODE_2, VALID_DEFINE_1);
    delegate = DistributedDBNbTestTools::GetNbDelegateSuccess(manager, g_dbParameter2, option);
    ASSERT_TRUE(manager != nullptr && delegate != nullptr);
    EXPECT_EQ(manager->CloseKvStore(delegate), OK);
    delegate = nullptr;

    /**
     * @tc.steps: step2. open the db again with schema_index mode, and the schema_index is
     *    [$.field9,$.field10.field10,$._field1.field3.field4,$._field1.field3.field5.field6].
     * @tc.expected: step2. open successfully.
     */
    option.schema = SpliceToSchema(VALID_VERSION_1, VALID_MODE_2, VALID_DEFINE_1, VALID_INDEX_4);
    option.createIfNecessary = false;
    delegate = DistributedDBNbTestTools::GetNbDelegateSuccess(manager, g_dbParameter2, option);
    ASSERT_TRUE(manager != nullptr && delegate != nullptr);

    /**
     * @tc.steps: step3. check is the schema_index exist.
     * @tc.expected: step3. can find 4 schema_indexes in schema table.
     */
    EXPECT_TRUE(SchemaIndexQuery(g_dbParameter2, NB_DIRECTOR, "$.field9"));
    EXPECT_TRUE(SchemaIndexQuery(g_dbParameter2, NB_DIRECTOR, "$.field10.field10"));
    EXPECT_TRUE(SchemaIndexQuery(g_dbParameter2, NB_DIRECTOR, "$._field1.field3.field4"));
    EXPECT_TRUE(SchemaIndexQuery(g_dbParameter2, NB_DIRECTOR, "$._field1.field3.field5.field6"));

    EXPECT_TRUE(EndCaseDeleteDB(manager, delegate, STORE_ID_2, option.isMemoryDb));
}

/**
 * @tc.name: SchemaIndex 002
 * @tc.desc: Verify that if the schema db created with index mode, then it can be degrade to non-index mode db
 *    when open it again with schema that do not include schema_index.
 * @tc.type: FUNC
 * @tc.require: SR000DR9JQ
 * @tc.author: fengxiaoyun
 */
HWTEST_F(DistributeddbNbSchemaDbTest, SchemaIndex002, TestSize.Level1)
{
    /**
     * @tc.steps: step1. create schema db that with schema_index.
     * @tc.expected: step1. create successfully.
     */
    KvStoreNbDelegate *delegate = nullptr;
    KvStoreDelegateManager *manager = nullptr;
    Option option = g_option;
    option.isMemoryDb = false;
    option.schema = SpliceToSchema(VALID_VERSION_1, VALID_MODE_2, VALID_DEFINE_1, VALID_INDEX_4);
    delegate = DistributedDBNbTestTools::GetNbDelegateSuccess(manager, g_dbParameter2, option);
    ASSERT_TRUE(manager != nullptr && delegate != nullptr);
    /**
     * @tc.steps: step2. check is the schema_index exist.
     * @tc.expected: step2. can find 4 schema_indexes in schema table.
     */
    EXPECT_TRUE(SchemaIndexQuery(g_dbParameter2, NB_DIRECTOR, "$.field9"));
    EXPECT_TRUE(SchemaIndexQuery(g_dbParameter2, NB_DIRECTOR, "$.field10.field10"));
    EXPECT_TRUE(SchemaIndexQuery(g_dbParameter2, NB_DIRECTOR, "$._field1.field3.field4"));
    EXPECT_TRUE(SchemaIndexQuery(g_dbParameter2, NB_DIRECTOR, "$._field1.field3.field5.field6"));
    EXPECT_EQ(manager->CloseKvStore(delegate), OK);
    delegate = nullptr;

    /**
     * @tc.steps: step3. open the db again with non_index mode.
     * @tc.expected: step3. open successfully.
     */
    option.schema = SpliceToSchema(VALID_VERSION_1, VALID_MODE_2, VALID_DEFINE_1);
    option.createIfNecessary = false;
    delegate = DistributedDBNbTestTools::GetNbDelegateSuccess(manager, g_dbParameter2, option);
    ASSERT_TRUE(manager != nullptr && delegate != nullptr);
    /**
     * @tc.steps: step4. check is the schema_index exist.
     * @tc.expected: step4. can't find the schema_index in schema table.
     */
    EXPECT_FALSE(SchemaIndexQuery(g_dbParameter2, NB_DIRECTOR, "$.field9"));
    EXPECT_FALSE(SchemaIndexQuery(g_dbParameter2, NB_DIRECTOR, "$.field10.field10"));
    EXPECT_FALSE(SchemaIndexQuery(g_dbParameter2, NB_DIRECTOR, "$._field1.field3.field4"));
    EXPECT_FALSE(SchemaIndexQuery(g_dbParameter2, NB_DIRECTOR, "$._field1.field3.field5.field6"));

    EXPECT_TRUE(EndCaseDeleteDB(manager, delegate, STORE_ID_2, option.isMemoryDb));
}

void CloseKvStoreTemp(KvStoreDelegateManager *&manager, KvStoreNbDelegate *&delegate)
{
    EXPECT_EQ(manager->CloseKvStore(delegate), OK);
    delegate = nullptr;
    delete manager;
    manager = nullptr;
}
/**
 * @tc.name: SchemaIndex 003
 * @tc.desc: Verify that whether the schema db has schema_index depended on is the first delegate has schema_index
 * @tc.type: FUNC
 * @tc.require: SR000DR9JQ
 * @tc.author: fengxiaoyun
 */
HWTEST_F(DistributeddbNbSchemaDbTest, SchemaIndex003, TestSize.Level1)
{
    /**
     * @tc.steps: step1. create schema db that without schema_index and close it but do not delete it.
     * @tc.expected: step1. create and close successfully.
     */
    KvStoreNbDelegate *delegate = nullptr;
    KvStoreDelegateManager *manager = nullptr;
    Option option = g_option;
    option.isMemoryDb = false;
    option.schema = SpliceToSchema(VALID_VERSION_1, VALID_MODE_2, VALID_DEFINE_1);
    delegate = DistributedDBNbTestTools::GetNbDelegateSuccess(manager, g_dbParameter2, option);
    ASSERT_TRUE(manager != nullptr && delegate != nullptr);
    EXPECT_EQ(manager->CloseKvStore(delegate), OK);
    delegate = nullptr;

    /**
     * @tc.steps: step2. open the db again with index mode, the schema_index is VALID_INDEX_4.
     * @tc.expected: step2. open successfully.
     */
    KvStoreNbDelegate *delegate1 = nullptr;
    KvStoreDelegateManager *manager1 = nullptr;
    option.schema = SpliceToSchema(VALID_VERSION_1, VALID_MODE_2, VALID_DEFINE_1, VALID_INDEX_4);
    option.createIfNecessary = false;
    delegate1 = DistributedDBNbTestTools::GetNbDelegateSuccess(manager1, g_dbParameter2, option);
    ASSERT_TRUE(manager1 != nullptr && delegate1 != nullptr);

    /**
     * @tc.steps: step3. open the db again with non-index mode.
     * @tc.expected: step3. open failed and return SCHEMA_MISMATCH.
     */
    KvStoreNbDelegate *delegate2 = nullptr;
    KvStoreDelegateManager *manager2 = nullptr;
    option.schema = SpliceToSchema(VALID_VERSION_1, VALID_MODE_2, VALID_DEFINE_1);
    DBStatus status;
    delegate2 = DistributedDBNbTestTools::GetNbDelegateStatus(manager2, status, g_dbParameter2, option);
    ASSERT_EQ(delegate2, nullptr);
    EXPECT_EQ(status, SCHEMA_MISMATCH);

    /**
     * @tc.steps: step4. open the db again using index mode with the index that is different from the index which used
     *    by the first delegate.
     * @tc.expected: step4. open failed and return SCHEMA_MISMATCH.
     */
    KvStoreNbDelegate *delegate3 = nullptr;
    KvStoreDelegateManager *manager3 = nullptr;
    std::string validIndexTemp = "[\"$.field9\", \"$._field1.field3.field5.field6\"]";
    option.schema = SpliceToSchema(VALID_VERSION_1, VALID_MODE_2, VALID_DEFINE_1, validIndexTemp);
    delegate3 = DistributedDBNbTestTools::GetNbDelegateStatus(manager3, status, g_dbParameter2, option);
    ASSERT_EQ(delegate3, nullptr);
    EXPECT_EQ(status, SCHEMA_MISMATCH);

    /**
     * @tc.steps: step5. open the db again using index mode with the index that is used by the first delegate.
     * @tc.expected: step5. open successfully.
     */
    KvStoreNbDelegate *delegate4 = nullptr;
    KvStoreDelegateManager *manager4 = nullptr;
    option.schema = SpliceToSchema(VALID_VERSION_1, VALID_MODE_2, VALID_DEFINE_1, VALID_INDEX_4);
    delegate4 = DistributedDBNbTestTools::GetNbDelegateStatus(manager4, status, g_dbParameter2, option);
    ASSERT_NE(delegate4, nullptr);
    EXPECT_EQ(status, OK);

    CloseKvStoreTemp(manager1, delegate1);
    CloseKvStoreTemp(manager4, delegate4);
    EXPECT_EQ(manager->DeleteKvStore(STORE_ID_2), OK);
    delete manager;
    manager = nullptr;
}

/**
 * @tc.name: SchemaIndex 004
 * @tc.desc: Verify that if open the schema db has schema_index circularly using wrong schema index and non-index will
 *    remove the valid index
 * @tc.type: FUNC
 * @tc.require: SR000DR9JQ
 * @tc.author: fengxiaoyun
 */
HWTEST_F(DistributeddbNbSchemaDbTest, SchemaIndex004, TestSize.Level2)
{
    /**
     * @tc.steps: step1. create schema db that with schema_index and close it but do not delete it.
     * @tc.expected: step1. create and close successfully.
     */
    KvStoreNbDelegate *delegate = nullptr;
    KvStoreDelegateManager *manager = nullptr;
    Option option = g_option;
    option.isMemoryDb = false;
    option.schema = SpliceToSchema(VALID_VERSION_1, VALID_MODE_2, VALID_DEFINE_1, VALID_INDEX_4);
    delegate = DistributedDBNbTestTools::GetNbDelegateSuccess(manager, g_dbParameter2, option);
    ASSERT_TRUE(manager != nullptr && delegate != nullptr);
    EXPECT_EQ(manager->CloseKvStore(delegate), OK);
    delegate = nullptr;

    /**
     * @tc.steps: step2. check is the schema_index exist.
     * @tc.expected: step2. can find 4 schema_indexes in schema table.
     */
    EXPECT_TRUE(SchemaIndexQuery(g_dbParameter2, NB_DIRECTOR, "$.field9"));
    EXPECT_TRUE(SchemaIndexQuery(g_dbParameter2, NB_DIRECTOR, "$.field10.field10"));
    EXPECT_TRUE(SchemaIndexQuery(g_dbParameter2, NB_DIRECTOR, "$._field1.field3.field4"));
    EXPECT_TRUE(SchemaIndexQuery(g_dbParameter2, NB_DIRECTOR, "$._field1.field3.field5.field6"));

    /**
     * @tc.steps: step3. open the db with the different index and non-index circularly.
     * @tc.expected: step3. open success.
     */
    std::string validIndexTemp = "[\"$.field9\", \"$._field1.field3.field5.field6\"]";
    KvStoreNbDelegate *delegate1 = nullptr;
    KvStoreDelegateManager *manager1 = nullptr;
    option.createIfNecessary = false;
    for (int index =0; index < HUNDRED_TIMES; index++) {
        option.schema = SpliceToSchema(VALID_VERSION_1, VALID_MODE_2, VALID_DEFINE_1, validIndexTemp);
        delegate1 = DistributedDBNbTestTools::GetNbDelegateSuccess(manager1, g_dbParameter2, option);
        ASSERT_TRUE(manager1 != nullptr && delegate1 != nullptr);
        CloseKvStoreTemp(manager1, delegate1);

        option.schema = SpliceToSchema(VALID_VERSION_1, VALID_MODE_2, VALID_DEFINE_1, VALID_INDEX_1);
        delegate1 = DistributedDBNbTestTools::GetNbDelegateSuccess(manager1, g_dbParameter2, option);
        ASSERT_TRUE(manager1 != nullptr && delegate1 != nullptr);
        CloseKvStoreTemp(manager1, delegate1);
    }

    /**
     * @tc.steps: step4. check is the schema_index exist.
     * @tc.expected: step4. can't find the schema_index in schema table.
     */
    EXPECT_FALSE(SchemaIndexQuery(g_dbParameter2, NB_DIRECTOR, "$.field9"));
    EXPECT_FALSE(SchemaIndexQuery(g_dbParameter2, NB_DIRECTOR, "$.field10.field10"));
    EXPECT_FALSE(SchemaIndexQuery(g_dbParameter2, NB_DIRECTOR, "$._field1.field3.field4"));
    EXPECT_FALSE(SchemaIndexQuery(g_dbParameter2, NB_DIRECTOR, "$._field1.field3.field5.field6"));

    EXPECT_EQ(manager->DeleteKvStore(STORE_ID_2), OK);
    delete manager;
    manager = nullptr;
}

void RandIndex(bool isFirst)
{
    std::string validIndexTemp;
    if (isFirst) {
        validIndexTemp = validIndexTemp + "[\"$.field9\", \"$._field1.field3.field4\"]";
    } else {
        validIndexTemp = validIndexTemp + "[\"$.field9\", \"$._field1.field3.field5.field6\"]";
    }
    std::string validIndexTemp2 = "[\"$.field9\"]";
    vector<std::string> indexes = {validIndexTemp, validIndexTemp2};
    int randSubscript = GetRandInt(0, 1);

    KvStoreNbDelegate *delegate = nullptr;
    KvStoreDelegateManager *manager = nullptr;
    Option option = g_option;
    option.isMemoryDb = false;
    option.schema = SpliceToSchema(VALID_VERSION_1, VALID_MODE_2, VALID_DEFINE_1, indexes[randSubscript]);
    delegate = DistributedDBNbTestTools::GetNbDelegateSuccess(manager, g_dbParameter2, option);
    if (delegate != nullptr) {
        EXPECT_EQ(manager->CloseKvStore(delegate), OK);
        delegate = nullptr;
    }
    delete manager;
    manager = nullptr;
}

/**
 * @tc.name: SchemaIndex 005
 * @tc.desc: Verify that Concurrency operate index won't cause the process to crash
 * @tc.type: FUNC
 * @tc.require: SR000DR9JQ
 * @tc.author: fengxiaoyun
 */
HWTEST_F(DistributeddbNbSchemaDbTest, SchemaIndex005, TestSize.Level1)
{
    /**
     * @tc.steps: step1. create schema db that with index $.field9 and close it but do not delete it.
     * @tc.expected: step1. create and close successfully.
     */
    KvStoreNbDelegate *delegate = nullptr;
    KvStoreDelegateManager *manager = nullptr;
    Option option = g_option;
    option.isMemoryDb = false;
    std::string validIndexTemp = "[\"$.field9\"]";
    option.schema = SpliceToSchema(VALID_VERSION_1, VALID_MODE_2, VALID_DEFINE_1, validIndexTemp);
    delegate = DistributedDBNbTestTools::GetNbDelegateSuccess(manager, g_dbParameter2, option);
    ASSERT_TRUE(manager != nullptr && delegate != nullptr);
    EXPECT_EQ(manager->CloseKvStore(delegate), OK);
    delegate = nullptr;

    /**
     * @tc.steps: step2. check is the schema_index exist.
     * @tc.expected: step2. can find 1 schema_index in schema table.
     */
    EXPECT_TRUE(SchemaIndexQuery(g_dbParameter2, NB_DIRECTOR, "$.field9"));

    /**
     * @tc.steps: step3. start 5 threads to increase or decrease index field, 3 of which randly increase or decrease
     *    [$._field1.field3.field4] and the rest of them increase or decrease [$._field1.field3.field5.field6] randly.
     * @tc.expected: step3. the program runs normally.
     */
    vector<thread> threads;
    threads.push_back(std::thread(RandIndex, true));
    threads.push_back(std::thread(RandIndex, true));
    threads.push_back(std::thread(RandIndex, true));
    threads.push_back(std::thread(RandIndex, false));
    threads.push_back(std::thread(RandIndex, false));

    for (auto &th : threads) {
        th.join();
    }

    /**
     * @tc.steps: step4. check is the schema_index exist.
     * @tc.expected: step4. can find the index $.field9 in schema table.
     */
    EXPECT_TRUE(SchemaIndexQuery(g_dbParameter2, NB_DIRECTOR, "$.field9"));

    EXPECT_EQ(manager->DeleteKvStore(STORE_ID_2), OK);
    delete manager;
    manager = nullptr;
}

/**
 * @tc.name: SkipTest 001
 * @tc.desc: Verify that SCHEMA_SKIPSIZE must be integer in range [0, 4M-2] or it will return INVALID_SCHEMA
 * @tc.type: FUNC
 * @tc.require: SR000DR9JQ
 * @tc.author: fengxiaoyun
 */
HWTEST_F(DistributeddbNbSchemaDbTest, SkipTest001, TestSize.Level1)
{
    /**
     * @tc.steps: step1. create schema db that with invalid skip size "10.5", "true", "*&", "汉字", "-10", "4194305".
     * @tc.expected: step1. create failed and return INVALID_SCHEMA.
     */
    KvStoreNbDelegate *delegate = nullptr;
    KvStoreDelegateManager *manager = nullptr;
    Option option = g_option;
    option.isMemoryDb = false;
    DBStatus status;
    vector<std::string> skipSize = {"10.5", "\"20\"", "true", "-10", "*&", "汉字", "4194303", "4194304"};
    for (unsigned long index = 0; index < skipSize.size(); index++) {
        option.schema = SpliceToSchema(VALID_VERSION_1, VALID_MODE_2, VALID_DEFINE_1, VALID_INDEX_1, skipSize[index]);
        delegate = DistributedDBNbTestTools::GetNbDelegateStatus(manager, status, g_dbParameter2, option);
        EXPECT_EQ(delegate, nullptr);
        EXPECT_EQ(status, INVALID_SCHEMA);
    }

    /**
     * @tc.steps: step2. create schema db that with invalid skip size "10.0", "1.23e2", "4194303", "4194304".
     * @tc.expected: step2. create failed and return INVALID_SCHEMA.
     */
    skipSize.clear();
    skipSize = {"0", "10", "123", "4194302"};
    for (unsigned long index = 0; index < skipSize.size(); index++) {
        option.schema = SpliceToSchema(VALID_VERSION_1, VALID_MODE_2, VALID_DEFINE_1, VALID_INDEX_1, skipSize[index]);
        delegate = DistributedDBNbTestTools::GetNbDelegateStatus(manager, status, g_dbParameter2, option);
        EXPECT_NE(delegate, nullptr);
        EXPECT_EQ(status, OK);
        EXPECT_EQ(manager->CloseKvStore(delegate), OK);
        delegate = nullptr;
        EXPECT_EQ(manager->DeleteKvStore(STORE_ID_2), OK);
        delete manager;
        manager = nullptr;
    }
}

/**
 * @tc.name: SkipTest 002
 * @tc.desc: Verify that the value must be with prefix whose size is SCHEMA_SKIPSIZE or it will put failed.
 * @tc.type: FUNC
 * @tc.require: SR000DR9JQ
 * @tc.author: fengxiaoyun
 */
HWTEST_F(DistributeddbNbSchemaDbTest, SkipTest002, TestSize.Level0)
{
    /**
     * @tc.steps: step1. create schema db that with valid skip size "16".
     * @tc.expected: step1. create success.
     */
    KvStoreNbDelegate *delegate = nullptr;
    KvStoreDelegateManager *manager = nullptr;
    Option option = g_option;
    option.isMemoryDb = false;
    DBStatus status;
    std::string skipSize = "16";
    const std::string validDefine = "{\"field1\":\"STRING,NOT NULL,DEFAULT 'fxy'\"}";
    option.schema = SpliceToSchema(VALID_VERSION_1, VALID_MODE_2, validDefine, VALID_INDEX_1, skipSize);
    delegate = DistributedDBNbTestTools::GetNbDelegateStatus(manager, status, g_dbParameter2, option);
    EXPECT_NE(delegate, nullptr);
    EXPECT_EQ(status, OK);

    /**
     * @tc.steps: step2. put (k1, v1) to db, v1.size() < 16, put (k2, v2) to db, v2.size() > 16 but has no prefix.
     * @tc.expected: step2. put failed.
     */
    std::string schemaValue = "{\"field1\":\"fxy\"}";

    Value value1;
    value1.assign(15, 'a'); // 15 bytes char, which is shorter than 16
    std::string schemaValueMiddle = "aa{\"field1\":\"fxy\"}";
    Value value2(schemaValueMiddle.begin(), schemaValueMiddle.end());
    EXPECT_EQ(delegate->Put(KEY_1, value1), INVALID_FORMAT);
    EXPECT_EQ(delegate->Put(KEY_2, value2), INVALID_FORMAT);
    /**
     * @tc.steps: step3. put 3 valid records which has the same schema define.
     * @tc.expected: step3. create failed and return INVALID_SCHEMA.
     */
    string validPre1 = "abbbbabbbbabbbbb";
    string validPre2 = "abbbbabbbbabbbbc";
    string validPre3 = "abbbbabbbbabbbbd";

    std::string defineValue1 = validPre1 + schemaValue;
    std::string defineValue2 = validPre2 + schemaValue;
    std::string defineValue3 = validPre3 + schemaValue;
    Value value4(defineValue1.begin(), defineValue1.end());
    Value value5(defineValue2.begin(), defineValue2.end());
    Value value6(defineValue3.begin(), defineValue3.end());
    EXPECT_EQ(delegate->Put(KEY_4, value4), OK);
    EXPECT_EQ(delegate->Put(KEY_5, value5), OK);
    EXPECT_EQ(delegate->Put(KEY_6, value6), OK);

    /**
     * @tc.steps: step4. query the result in the db on field = "fxy".
     * @tc.expected: step4. can find 3 records in the db.
     */
    vector<Entry> entries;
    Query query = Query::Select().EqualTo("$.field1", "fxy");
    EXPECT_EQ(delegate->GetEntries(query, entries), OK);
    EXPECT_EQ(entries.size(), static_cast<unsigned long>(3)); // 3 record

    /**
     * @tc.steps: step5. delete k3, and query the result in the db on field = "fxy".
     * @tc.expected: step5. can find 2 records in the db.
     */
    EXPECT_EQ(delegate->Delete(KEY_6), OK);
    query = Query::Select().EqualTo("$.field1", "fxy");
    EXPECT_EQ(delegate->GetEntries(query, entries), OK);
    vector<Entry> entriesExpect = {{KEY_4, value4}, {KEY_5, value5}};
    EXPECT_TRUE(CompareEntriesVector(entries, entriesExpect));

    EXPECT_TRUE(EndCaseDeleteDB(manager, delegate, STORE_ID_2, option.isMemoryDb));
}

/**
 * @tc.name: SkipTest 003
 * @tc.desc: Verify that when the field in pre-SCHEMA_SKIPSIZE, it will be ignored.
 * @tc.type: FUNC
 * @tc.require: SR000DR9JQ
 * @tc.author: fengxiaoyun
 */
HWTEST_F(DistributeddbNbSchemaDbTest, SkipTest003, TestSize.Level0)
{
    /**
     * @tc.steps: step1. create schema db that with valid skip size "16".
     * @tc.expected: step1. create success.
     */
    KvStoreNbDelegate *delegate = nullptr;
    KvStoreDelegateManager *manager = nullptr;
    Option option = g_option;
    option.isMemoryDb = false;
    DBStatus status;
    std::string skipSize = "16";
    const std::string validDefine = "{\"field1\":\"STRING,NOT NULL,DEFAULT 'fxy'\"}";
    option.schema = SpliceToSchema(VALID_VERSION_1, VALID_MODE_2, validDefine, VALID_INDEX_1, skipSize);
    delegate = DistributedDBNbTestTools::GetNbDelegateStatus(manager, status, g_dbParameter2, option);
    EXPECT_NE(delegate, nullptr);
    EXPECT_EQ(status, OK);

    /**
     * @tc.steps: step2. put (k1, v1) to db, which has field2 in the prefix char only.
     * @tc.expected: step2. put successfully.
     */
    string validPre1 = "{\"field1\":\"abc\"}{\"field1\":\"fxy\"}";
    string validPre2 = "{\"field1\":\"abc\"}{\"field1\":\"fxy\", \"field2\": \"abc\"}";

    Value value1(validPre1.begin(), validPre1.end());
    Value value2(validPre2.begin(), validPre2.end());
    EXPECT_EQ(delegate->Put(KEY_1, value1), OK);

    /**
     * @tc.steps: step3. query the result in the db on field2 = "fxy".
     * @tc.expected: step3. can't find records in the db.
     */
    vector<Entry> entries;
    Query query = Query::Select().EqualTo("$.field1", "abc");
    EXPECT_EQ(delegate->GetEntries(query, entries), NOT_FOUND);

    /**
     * @tc.steps: step4. put (k2, v2) to db and query the records in db on field2 = "fxy".
     * @tc.expected: step4. can find one record in the db.
     */
    EXPECT_EQ(delegate->Put(KEY_2, value2), OK);
    query = Query::Select().EqualTo("$.field1", "fxy");
    EXPECT_EQ(delegate->GetEntries(query, entries), OK);
    vector<Entry> entriesExpect = {{KEY_1, value1}, {KEY_2, value2}};
    EXPECT_TRUE(CompareEntriesVector(entries, entriesExpect));

    EXPECT_TRUE(EndCaseDeleteDB(manager, delegate, STORE_ID_2, option.isMemoryDb));
}

/**
 * @tc.name: SkipTest 004
 * @tc.desc: Verify that when open the db that has a valid schema-size with a schema-size that not equal to define,
 *    or without schema-size, it will open failed and return SCHEMA_MISMATCH
 * @tc.type: FUNC
 * @tc.require: SR000DR9JQ
 * @tc.author: fengxiaoyun
 */
HWTEST_F(DistributeddbNbSchemaDbTest, SkipTest004, TestSize.Level1)
{
    /**
     * @tc.steps: step1. create schema db that with valid skip size "16".
     * @tc.expected: step1. create success.
     */
    map<string, KvStoreNbDelegate *> delegate;
    map<string, KvStoreDelegateManager *> manager;
    Option option = g_option;
    option.isMemoryDb = false;
    DBStatus status;
    std::string skipSize = "16";
    option.schema = SpliceToSchema(VALID_VERSION_1, VALID_MODE_2, VALID_DEFINE_1, VALID_INDEX_1, skipSize);
    delegate["ValidSkip"] = DistributedDBNbTestTools::GetNbDelegateStatus(manager["ValidSkip"],
        status, g_dbParameter2, option);
    EXPECT_NE(delegate["ValidSkip"], nullptr);
    EXPECT_EQ(status, OK);
    EXPECT_EQ(manager["ValidSkip"]->CloseKvStore(delegate["ValidSkip"]), OK);
    delegate["ValidSkip"] = nullptr;
    delete manager["ValidSkip"];
    manager["ValidSkip"] = nullptr;

    /**
     * @tc.steps: step2. open the db that without SCHEMA_SIZE.
     * @tc.expected: step2. open failed and return SCHEMA_MISMATCH.
     */
    option.schema = SpliceToSchema(VALID_VERSION_1, VALID_MODE_2, VALID_DEFINE_1, VALID_INDEX_1);
    delegate["NoSkip"] = DistributedDBNbTestTools::GetNbDelegateStatus(manager["NoSkip"],
        status, g_dbParameter2, option);
    EXPECT_EQ(delegate["NoSkip"], nullptr);
    EXPECT_EQ(status, SCHEMA_MISMATCH);
    /**
     * @tc.steps: step3. open the db again with the SCHEMA_SIZE that larger or smaller than skipSize.
     * @tc.expected: step3. open failed and return SCHEMA_MISMATCH.
     */
    skipSize = "17";
    option.schema = SpliceToSchema(VALID_VERSION_1, VALID_MODE_2, VALID_DEFINE_1, VALID_INDEX_1, skipSize);
    delegate["longerSkip"] = DistributedDBNbTestTools::GetNbDelegateStatus(manager["longerSkip"],
        status, g_dbParameter2, option);
    EXPECT_EQ(delegate["longerSkip"], nullptr);
    EXPECT_EQ(status, SCHEMA_MISMATCH);

    skipSize = "15";
    option.schema = SpliceToSchema(VALID_VERSION_1, VALID_MODE_2, VALID_DEFINE_1, VALID_INDEX_1, skipSize);
    delegate["shorterSkip"] = DistributedDBNbTestTools::GetNbDelegateStatus(manager["shorterSkip"],
        status, g_dbParameter2, option);
    EXPECT_EQ(delegate["shorterSkip"], nullptr);
    EXPECT_EQ(status, SCHEMA_MISMATCH);

    /**
     * @tc.steps: step4. open the db the right SCHEMA_SIZE.
     * @tc.expected: step4. open successfully.
     */
    skipSize = "16";
    option.schema = SpliceToSchema(VALID_VERSION_1, VALID_MODE_2, VALID_DEFINE_1, VALID_INDEX_4, skipSize);
    delegate["ValidSkip"] = DistributedDBNbTestTools::GetNbDelegateStatus(manager["ValidSkip"],
        status, g_dbParameter2, option);
    EXPECT_NE(delegate["ValidSkip"], nullptr);
    EXPECT_EQ(manager["ValidSkip"]->CloseKvStore(delegate["ValidSkip"]), OK);
    delegate["ValidSkip"] = nullptr;
    EXPECT_EQ(status, OK);

    EXPECT_EQ(manager["ValidSkip"]->DeleteKvStore(STORE_ID_2), OK);
    delete manager["ValidSkip"];
    manager["ValidSkip"] = nullptr;
}

/**
 * @tc.name: SkipTest 005
 * @tc.desc: Verify that it can open one db that with schema-size with a schema that include schema-size.
 *    And also, it will must need the right schema-size that used the first time to open the db after that.
 * @tc.type: FUNC
 * @tc.require: SR000DR9JQ
 * @tc.author: fengxiaoyun
 */
HWTEST_F(DistributeddbNbSchemaDbTest, SkipTest005, TestSize.Level1)
{
    /**
     * @tc.steps: step1. create schema db that without schema.
     * @tc.expected: step1. create success.
     */
    map<string, KvStoreNbDelegate *> delegate;
    map<string, KvStoreDelegateManager *>manager;
    Option option = g_option;
    option.isMemoryDb = false;
    DBStatus status;
    delegate["Normal"] = DistributedDBNbTestTools::GetNbDelegateStatus(manager["Normal"],
        status, g_dbParameter2, option);
    ASSERT_NE(delegate["Normal"], nullptr);
    EXPECT_EQ(status, OK);
    EXPECT_EQ(manager["Normal"]->CloseKvStore(delegate["Normal"]), OK);
    delegate["Normal"] = nullptr;

    /**
     * @tc.steps: step2. open the db again with the schema and SCHEMA_SIZE = 16.
     * @tc.expected: step2. open successfully.
     */
    std::string skipSize = "16";
    option.schema = SpliceToSchema(VALID_VERSION_1, VALID_MODE_2, VALID_DEFINE_1, VALID_INDEX_3, skipSize);
    delegate["Skip1"] = DistributedDBNbTestTools::GetNbDelegateStatus(manager["Skip1"], status, g_dbParameter2, option);
    ASSERT_NE(delegate["Skip1"], nullptr);
    EXPECT_EQ(status, OK);

    /**
     * @tc.steps: step3. open the db again with the SCHEMA_SIZE = 15.
     * @tc.expected: step3. open failed and return SCHEMA_MISMATCH.
     */
    skipSize = "15";
    option.schema = SpliceToSchema(VALID_VERSION_1, VALID_MODE_2, VALID_DEFINE_1, VALID_INDEX_1, skipSize);
    delegate["Skip2"] = DistributedDBNbTestTools::GetNbDelegateStatus(manager["Skip2"], status, g_dbParameter2, option);
    ASSERT_EQ(delegate["Skip2"], nullptr);
    EXPECT_EQ(status, SCHEMA_MISMATCH);

    /**
     * @tc.steps: step3. open the db that without SCHEMA_SIZE.
     * @tc.expected: step3. open failed and return SCHEMA_MISMATCH.
     */
    skipSize = "16";
    option.schema = SpliceToSchema(VALID_VERSION_1, VALID_MODE_2, VALID_DEFINE_1, VALID_INDEX_3, skipSize);
    delegate["Skip2"] = DistributedDBNbTestTools::GetNbDelegateStatus(manager["Skip2"], status, g_dbParameter2, option);
    ASSERT_NE(delegate["Skip2"], nullptr);
    EXPECT_EQ(status, OK);
    EXPECT_EQ(manager["Skip2"]->CloseKvStore(delegate["Skip2"]), OK);
    delegate["Skip2"] = nullptr;
    delete manager["Skip2"];
    manager["Skip2"] = nullptr;

    EXPECT_EQ(manager["Skip1"]->CloseKvStore(delegate["Skip1"]), OK);
    delegate["Skip1"] = nullptr;
    delete manager["Skip1"];
    manager["Skip1"] = nullptr;
    EXPECT_EQ(manager["Normal"]->DeleteKvStore(STORE_ID_2), OK);
    delete manager["Normal"];
    manager["Normal"] = nullptr;
}

/**
 * @tc.name: DataTypeVerify 001
 * @tc.desc: Verify that it can Verify the data type of INTEGER and LONG criticality value rightly.
 * @tc.type: FUNC
 * @tc.require: SR000DR9JQ
 * @tc.author: fengxiaoyun
 */
HWTEST_F(DistributeddbNbSchemaDbTest, DataTypeVerify001, TestSize.Level0)
{
    /**
     * @tc.steps: step1. create schema db that with schema include the field of INTEGER LONG and DOUBLE type.
     * @tc.expected: step1. create success.
     */
    KvStoreNbDelegate *delegate = nullptr;
    KvStoreDelegateManager *manager = nullptr;
    Option option = g_option;
    option.isMemoryDb = false;
    std::string schemaDefine = "{\"field1\":\"INTEGER ,DEFAULT null\",\"field2\":\"LONG ,DEFAULT null\","
        "\"field3\":\"DOUBLE ,DEFAULT null\"}";
    option.schema = SpliceToSchema(VALID_VERSION_1, VALID_MODE_1, schemaDefine, VALID_INDEX_1);
    delegate = DistributedDBNbTestTools::GetNbDelegateSuccess(manager, g_dbParameter2, option);
    ASSERT_TRUE(manager != nullptr && delegate != nullptr);

    /**
     * @tc.steps: step2. put the records where field1 is INT32_MIN, INT32_MAX, INT32_MIN - 1, and INT32_MAX + 1.
     * @tc.expected: step2. when field1 is INT32_MIN, INT32_MAX can put successfully,
     *    but when it is INT32_MIN - 1, and INT32_MAX + 1 will be failed.
     */
    string int32Min = "{\"field1\":-2147483648,\"field2\":null, \"field3\":null}";
    string int32Max = "{\"field1\":2147483647,\"field2\":null, \"field3\":null}";
    string int32MinSub1 = "{\"field1\":-2147483649,\"field2\":null, \"field3\":null}";
    string int32MaxPlus1 = "{\"field1\":2147483648,\"field2\":null, \"field3\":null}";
    Value value1(int32Min.begin(), int32Min.end());
    Value value2(int32Max.begin(), int32Max.end());
    Value value3(int32MinSub1.begin(), int32MinSub1.end());
    Value value4(int32MaxPlus1.begin(), int32MaxPlus1.end());
    EXPECT_EQ(delegate->Put(KEY_1, value1), OK);
    EXPECT_EQ(delegate->Put(KEY_2, value2), OK);
    EXPECT_EQ(delegate->Put(KEY_3, value3), INVALID_FIELD_TYPE);
    EXPECT_EQ(delegate->Put(KEY_4, value4), INVALID_FIELD_TYPE);

    /**
     * @tc.steps: step3. put the records where field2 is INT64_MIN, INT64_MAX, INT64_MIN - 1, and INT64_MAX + 1.
     * @tc.expected: step3. when field2 is INT64_MIN, INT64_MAX can put successfully,
     *    but when it is INT64_MIN - 1, and INT64_MAX + 1 will be failed.
     */
    string int64Min = "{\"field1\":null,\"field2\":-9223372036854775808, \"field3\":null}";
    string int64Max = "{\"field1\":null,\"field2\":9223372036854775807, \"field3\":null}";
    string int64MinSub1 = "{\"field1\":null,\"field2\":-9223372036854775809, \"field3\":null}";
    string int64MaxPlus1 = "{\"field1\":null,\"field2\":9223372036854775808, \"field3\":null}";
    Value value5(int64Min.begin(), int64Min.end());
    Value value6(int64Max.begin(), int64Max.end());
    Value value7(int64MinSub1.begin(), int64MinSub1.end());
    Value value8(int64MaxPlus1.begin(), int64MaxPlus1.end());
    EXPECT_EQ(delegate->Put(KEY_1, value5), OK);
    EXPECT_EQ(delegate->Put(KEY_2, value6), OK);
    EXPECT_EQ(delegate->Put(KEY_3, value7), INVALID_FIELD_TYPE);
    EXPECT_EQ(delegate->Put(KEY_4, value8), INVALID_FIELD_TYPE);

    EXPECT_TRUE(EndCaseDeleteDB(manager, delegate, STORE_ID_2, option.isMemoryDb));
}

/**
 * @tc.name: DataTypeVerify 002
 * @tc.desc: Verify that it can Verify the data type of INTEGER and LONG criticality value rightly.
 * @tc.type: FUNC
 * @tc.require: SR000DR9JQ
 * @tc.author: fengxiaoyun
 */
HWTEST_F(DistributeddbNbSchemaDbTest, DataTypeVerify002, TestSize.Level0)
{
    /**
     * @tc.steps: step1. create schema db that with schema include the field of INTEGER LONG and DOUBLE type.
     * @tc.expected: step1. create success.
     */
    KvStoreNbDelegate *delegate = nullptr;
    KvStoreDelegateManager *manager = nullptr;
    Option option = g_option;
    option.isMemoryDb = false;
    std::string schemaDefine = "{\"field1\":\"INTEGER ,DEFAULT null\",\"field2\":\"LONG ,DEFAULT null\","
        "\"field3\":\"DOUBLE ,DEFAULT null\"}";
    option.schema = SpliceToSchema(VALID_VERSION_1, VALID_MODE_1, schemaDefine, VALID_INDEX_1);
    delegate = DistributedDBNbTestTools::GetNbDelegateSuccess(manager, g_dbParameter2, option);
    ASSERT_TRUE(manager != nullptr && delegate != nullptr);

    /**
     * @tc.steps: step2. put the records where field1 is -10.1, 10.1, 10E5, and 10.123E5.
     * @tc.expected: step2. all of them will be failed.
     */
    string negativeAndPoitInt = "{\"field1\":-10.1,\"field2\":null, \"field3\":null}";
    string positiveAndPoitInt = "{\"field1\":10.1,\"field2\":null, \"field3\":null}";
    string scientificNotationInt = "{\"field1\":10E5,\"field2\":null, \"field3\":null}";
    string scientificNotationAndPoitInt = "{\"field1\":10.123E5,\"field2\":null, \"field3\":null}";
    Value value1(negativeAndPoitInt.begin(), negativeAndPoitInt.end());
    Value value2(positiveAndPoitInt.begin(), positiveAndPoitInt.end());
    Value value3(scientificNotationInt.begin(), scientificNotationInt.end());
    Value value4(scientificNotationAndPoitInt.begin(), scientificNotationAndPoitInt.end());
    EXPECT_EQ(delegate->Put(KEY_1, value1), INVALID_FIELD_TYPE);
    EXPECT_EQ(delegate->Put(KEY_2, value2), INVALID_FIELD_TYPE);
    EXPECT_EQ(delegate->Put(KEY_3, value3), INVALID_FIELD_TYPE);
    EXPECT_EQ(delegate->Put(KEY_4, value4), INVALID_FIELD_TYPE);

    /**
     * @tc.steps: step3. put the records where field2 is -10.1, 10.1, 10E5, and 10.123E5.
     * @tc.expected: step3. all of them will be failed.
     */
    string negativeAndPoitLong = "{\"field1\":null,\"field2\":-10.1, \"field3\":null}";
    string positiveAndPoitLong = "{\"field1\":null,\"field2\":10.1, \"field3\":null}";
    string scientificNotationLong = "{\"field1\":null,\"field2\":10E5, \"field3\":null}";
    string scientificNotationAndPoitLong = "{\"field1\":null,\"field2\":10.123E5, \"field3\":null}";
    Value value5(negativeAndPoitLong.begin(), negativeAndPoitLong.end());
    Value value6(positiveAndPoitLong.begin(), positiveAndPoitLong.end());
    Value value7(scientificNotationLong.begin(), scientificNotationLong.end());
    Value value8(scientificNotationAndPoitLong.begin(), scientificNotationAndPoitLong.end());
    EXPECT_EQ(delegate->Put(KEY_1, value5), INVALID_FIELD_TYPE);
    EXPECT_EQ(delegate->Put(KEY_2, value6), INVALID_FIELD_TYPE);
    EXPECT_EQ(delegate->Put(KEY_3, value7), INVALID_FIELD_TYPE);
    EXPECT_EQ(delegate->Put(KEY_4, value8), INVALID_FIELD_TYPE);

    /**
     * @tc.steps: step4. put the records where field3 is -10.1, 10.1, 10E5, and 10.123E5.
     * @tc.expected: step4. all of them will be success.
     */
    string negativeAndPoitDouble = "{\"field1\":null,\"field2\":null, \"field3\":-10.1}";
    string positiveAndPoitDouble = "{\"field1\":null,\"field2\":null, \"field3\":10.1}";
    string scientificNotationDouble = "{\"field1\":null,\"field2\":null, \"field3\":10E5}";
    string scientificNotationAndPoitDouble = "{\"field1\":null,\"field2\":null, \"field3\":10.123E5}";
    Value value9(negativeAndPoitDouble.begin(), negativeAndPoitDouble.end());
    Value value10(positiveAndPoitDouble.begin(), positiveAndPoitDouble.end());
    Value value11(scientificNotationDouble.begin(), scientificNotationDouble.end());
    Value value12(scientificNotationAndPoitDouble.begin(), scientificNotationAndPoitDouble.end());
    EXPECT_EQ(delegate->Put(KEY_1, value9), OK);
    EXPECT_EQ(delegate->Put(KEY_2, value10), OK);
    EXPECT_EQ(delegate->Put(KEY_3, value11), OK);
    EXPECT_EQ(delegate->Put(KEY_4, value12), OK);

    EXPECT_TRUE(EndCaseDeleteDB(manager, delegate, STORE_ID_2, option.isMemoryDb));
}
namespace {
std::string GenerateLargeDepth(int depth)
{
    string resultStr;
    for (int index = 1; index <= depth; index++) {
        std::string ind = std::to_string(index);
        if (index == depth) {
            resultStr = resultStr + "\"filed" + ind + "\":" + ind;
        } else {
            resultStr = resultStr + "\"filed" + ind + "\":{";
        }
    }
    resultStr.append(depth - 1, '}');
    return resultStr;
}
const int SKIPSTR_LEN = 16;
const int JSON_LEN = 99;
}

/**
 * @tc.name: JsonTest 001
 * @tc.desc: Verify that it can parse the Json string which nesting depth is equal to 100.
 * @tc.type: FUNC
 * @tc.require: SR000F3L0Q
 * @tc.author: fengxiaoyun
 */
HWTEST_F(DistributeddbNbSchemaDbTest, JsonTest001, TestSize.Level0)
{
    /**
     * @tc.steps: step1. create schema db that with schema_define = {"field":{}, "field1":"STRING"}.
     * @tc.expected: step1. create success.
     */
    KvStoreNbDelegate *delegate = nullptr;
    KvStoreDelegateManager *manager = nullptr;
    Option option = g_option;
    option.isMemoryDb = false;
    std::string schemaDefine = "{\"field\":{}, \"field1\": \"STRING\"}";
    string schemaIndex = "[\"$.field1\"]";
    std::string skipSize = "16";
    option.schema = SpliceToSchema(VALID_VERSION_1, VALID_MODE_1, schemaDefine, schemaIndex, skipSize);
    delegate = DistributedDBNbTestTools::GetNbDelegateSuccess(manager, g_dbParameter2, option);
    ASSERT_TRUE(manager != nullptr && delegate != nullptr);

    /**
     * @tc.steps: step2. put the records where the depth of Json is equal to 100.
     * @tc.expected: step2. put successfully.
     */
    string skipStr(SKIPSTR_LEN, '0');
    string field1Str = skipStr + "{\"field\":" + "{\"field1\": \"string\"}" + ",\"field1\":\"abc\"}";
    string field1Int = skipStr + "{\"field\":" + "{\"field1\": 0}" + ",\"field1\":\"bcd\"}";
    string field1Long = skipStr + "{\"field\":" + "{\"field1\": 500}" + ",\"field1\":\"cde\"}";
    string field1Float = skipStr + "{\"field\":" + "{\"field1\": 3.14}" + ",\"field1\":\"def\"}";
    string field1 = GenerateLargeDepth(JSON_LEN);
    string field1Object = skipStr + "{\"field\":{" + field1 + "},\"field1\":\"efg\"}";
    Value value1(field1Str.begin(), field1Str.end());
    Value value2(field1Int.begin(), field1Int.end());
    Value value3(field1Long.begin(), field1Long.end());
    Value value4(field1Float.begin(), field1Float.end());
    Value value5(field1Object.begin(), field1Object.end());
    EXPECT_EQ(delegate->Put(KEY_1, value1), OK);
    EXPECT_EQ(delegate->Put(KEY_2, value2), OK);
    EXPECT_EQ(delegate->Put(KEY_3, value3), OK);
    EXPECT_EQ(delegate->Put(KEY_4, value4), OK);
    EXPECT_EQ(delegate->Put(KEY_5, value5), OK);

    /**
     * @tc.steps: step3. query data in db.
     * @tc.expected: step3. operate successfully.
     */
    vector<Entry> entriesResult;
    KvStoreResultSet *resultSet = nullptr;
    Query query = Query::Select().PrefixKey({'k'}).OrderBy("$.field1", true);
    EXPECT_EQ(delegate->GetEntries(query, entriesResult), OK);
    EXPECT_EQ(delegate->GetEntries(query, resultSet), OK);
    EXPECT_TRUE(entriesResult.size() == FIVE_RECORDS);
    EXPECT_TRUE(resultSet->GetCount() == FIVE_RECORDS);
    EXPECT_EQ(delegate->CloseResultSet(resultSet), OK);
    EXPECT_TRUE(EndCaseDeleteDB(manager, delegate, STORE_ID_2, option.isMemoryDb));
}

/**
 * @tc.name: JsonTest 002
 * @tc.desc: Verify that it can parse the Json string which include "\n,\r,\t".
 * @tc.type: FUNC
 * @tc.require: SR000F3L0Q
 * @tc.author: fengxiaoyun
 */
HWTEST_F(DistributeddbNbSchemaDbTest, JsonTest002, TestSize.Level0)
{
    /**
     * @tc.steps: step1. create schema db that with schema_define = {"field":{}, "field1":"STRING"}.
     * @tc.expected: step1. create success.
     */
    KvStoreNbDelegate *delegate = nullptr;
    KvStoreDelegateManager *manager = nullptr;
    Option option = g_option;
    option.isMemoryDb = false;
    std::string schemaDefine = "{\"field\":{}, \"field1\": \"STRING\"}";
    std::string skipSize = "16";
    option.schema = SpliceToSchema(VALID_VERSION_1, VALID_MODE_1, schemaDefine, VALID_INDEX_1, skipSize);
    delegate = DistributedDBNbTestTools::GetNbDelegateSuccess(manager, g_dbParameter2, option);
    ASSERT_TRUE(manager != nullptr && delegate != nullptr);

    /**
     * @tc.steps: step2. put the records where the Json string includes "\n,\r,\t".
     * @tc.expected: step2. put successfully.
     */
    string skipStr(SKIPSTR_LEN, '\\');
    string field1Str = skipStr + "{\"field\":" + "{\"\\nfield1\": \"a\"}" + ",\"field1\":\"abc\"}";
    string fieldBool = skipStr + "{\"field\":" + "{\"\\rfield1\": true}" + ",\"field1\":\"bcd\"}";
    // test Json string with space.
    string field1Int = skipStr + "{\"field\":" + "{\"field1\":  100}" + ",\"field1\":\"cde\"}";
    string field1Long = skipStr + "{\"field\":" + "{\"field1\": 1000}" + ",\"field1\":\"def\\t\"}";
    string field1Float = skipStr + "{\"field\":" + "{\"field1\": 3.14}" + ",\"field1\":\"efg\"}";
    Value value1(field1Str.begin(), field1Str.end());
    Value value2(fieldBool.begin(), fieldBool.end());
    Value value3(field1Int.begin(), field1Int.end());
    Value value4(field1Long.begin(), field1Long.end());
    Value value5(field1Float.begin(), field1Float.end());
    EXPECT_EQ(delegate->Put(KEY_1, value1), OK);
    EXPECT_EQ(delegate->Put(KEY_2, value2), OK);
    EXPECT_EQ(delegate->Put(KEY_3, value3), OK);
    EXPECT_EQ(delegate->Put(KEY_4, value4), OK);
    EXPECT_EQ(delegate->Put(KEY_5, value5), OK);

    /**
     * @tc.steps: step3. query data in db.
     * @tc.expected: step3. operate successfully.
     */
    vector<Entry> entriesResult;
    KvStoreResultSet *resultSet = nullptr;
    Query query = Query::Select().IsNotNull("$.field1").Limit(10, 0); // query from 0 offset 10.
    EXPECT_EQ(delegate->GetEntries(query, entriesResult), OK);
    EXPECT_EQ(delegate->GetEntries(query, resultSet), OK);
    EXPECT_TRUE(entriesResult.size() == FIVE_RECORDS);
    EXPECT_TRUE(resultSet->GetCount() == FIVE_RECORDS);
    EXPECT_EQ(delegate->CloseResultSet(resultSet), OK);
    EXPECT_TRUE(EndCaseDeleteDB(manager, delegate, STORE_ID_2, option.isMemoryDb));
}
}
#endif