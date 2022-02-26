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
#include "kv_store_delegate_manager.h"
#include "distributeddb_tools_unit_test.h"
#include "schema_utils.h"
#include "schema_object.h"

using namespace testing::ext;
using namespace DistributedDB;
using namespace DistributedDBUnitTest;

namespace {
    const std::string APP_ID = "SCHEMA";
    const std::string USER_ID = "UPGRADE";
    std::string g_testDir;
    KvStoreDelegateManager g_manager(APP_ID, USER_ID);

    DBStatus g_kvDelegateStatus = INVALID_ARGS;
    KvStoreNbDelegate *g_kvDelegatePtr = nullptr;
    auto g_kvDelegateCallback = bind(&DistributedDBToolsUnitTest::KvStoreNbDelegateCallback,
        std::placeholders::_1, std::placeholders::_2, std::ref(g_kvDelegateStatus), std::ref(g_kvDelegatePtr));

    std::string g_baseSchema;
    DBStatus g_expectError = SCHEMA_VIOLATE_VALUE;

    std::string StringEraser(const std::string &oriString, const std::string &toErase)
    {
        std::string resStr = oriString;
        auto iter = std::search(resStr.begin(), resStr.end(), toErase.begin(), toErase.end());
        resStr.erase(iter, iter + toErase.size());
        return resStr;
    }
    std::string StringReplacer(const std::string &oriString, const std::string &toErase, const std::string &toRepalce)
    {
        std::string resStr = oriString;
        auto iter = std::search(resStr.begin(), resStr.end(), toErase.begin(), toErase.end());
        resStr.replace(iter, iter + toErase.size(), toRepalce);
        return resStr;
    }

    const std::string SCHEMA_INC_FIELD = "{\"SCHEMA_VERSION\":\"1.0\","
        "\"SCHEMA_MODE\":\"COMPATIBLE\","
        "\"SCHEMA_DEFINE\":{"
            "\"field_1\":\"LONG, NOT NULL, DEFAULT 100\","
            "\"field_2\":{"
                "\"field_3\":\"STRING, DEFAULT 'OpenHarmony'\","
                "\"field_4\":\"INTEGER\""
            "}"
        "},"
        "\"SCHEMA_INDEXES\":[\"field_1\", [\"field_2.field_3\", \"field_2.field_4\"]]}";
    const std::string SCHEMA_BASE = StringReplacer(StringEraser(SCHEMA_INC_FIELD, ",\"field_4\":\"INTEGER\""),
        "[\"field_2.field_3\", \"field_2.field_4\"]", "\"field_2.field_3\"");
    const std::string SCHEMA_INC_FIELD_NOTNULL = StringReplacer(SCHEMA_INC_FIELD, "\"INTEGER\"",
        "\"INTEGER, NOT NULL\"");
    const std::string SCHEMA_INC_FIELD_DEFAULT = StringReplacer(SCHEMA_INC_FIELD, "\"INTEGER\"",
        "\"INTEGER, DEFAULT 88\"");
    const std::string SCHEMA_INC_FIELD_NOTNULL_DEFAULT = StringReplacer(SCHEMA_INC_FIELD, "\"INTEGER\"",
        "\"INTEGER, NOT NULL, DEFAULT 88\"");

    const std::string VALUE_BASE_LACK = "{\"field_1\":LONG_VAL}";
    const std::string VALUE_BASE = "{\"field_1\":LONG_VAL,\"field_2\":{\"field_3\":STR_VAL}}";
    const std::string VALUE_FIELD_FULL = "{\"field_1\":LONG_VAL,\"field_2\":{\"field_3\":STR_VAL,\"field_4\":INT_VAL}}";

    std::string SchemaSwitchMode(const std::string &oriSchemaStr)
    {
        std::string resStr = oriSchemaStr;
        auto iterStrict = std::search(resStr.begin(), resStr.end(), KEYWORD_MODE_STRICT.begin(),
            KEYWORD_MODE_STRICT.end());
        auto iterCompatible = std::search(resStr.begin(), resStr.end(), KEYWORD_MODE_COMPATIBLE.begin(),
            KEYWORD_MODE_COMPATIBLE.end());
        if (iterStrict != resStr.end()) {
            resStr.replace(iterStrict, iterStrict + KEYWORD_MODE_STRICT.size(), KEYWORD_MODE_COMPATIBLE.begin(),
                KEYWORD_MODE_COMPATIBLE.end());
            return resStr;
        }
        if (iterCompatible != resStr.end()) {
            resStr.replace(iterCompatible, iterCompatible + KEYWORD_MODE_COMPATIBLE.size(), KEYWORD_MODE_STRICT.begin(),
                KEYWORD_MODE_STRICT.end());
            return resStr;
        }
        return oriSchemaStr;
    }
    bool SchemaChecker(const std::string schema)
    {
        SchemaObject schemaObj;
        return (schemaObj.ParseFromSchemaString(schema) == E_OK);
    }
    std::vector<uint8_t> ToVec(const std::string &inStr)
    {
        std::vector<uint8_t> outVec(inStr.begin(), inStr.end());
        return outVec;
    }
    std::string ToStr(const std::vector<uint8_t> &inVec)
    {
        std::string outStr(inVec.begin(), inVec.end());
        return outStr;
    }

    const std::map<std::string, std::vector<uint8_t>> VALUE_MAP {
        {"LACK", ToVec(StringReplacer(VALUE_BASE_LACK, "LONG_VAL", "1"))},
        {"BASE", ToVec(StringReplacer(StringReplacer(VALUE_BASE, "LONG_VAL", "2"), "STR_VAL", "\"OS\""))},
        {"FULL", ToVec(StringReplacer(StringReplacer(StringReplacer(VALUE_FIELD_FULL, "LONG_VAL", "3"), "STR_VAL",
            "\"TEST\""), "INT_VAL", "33"))},
        {"BASE_WRONG_TYPE", ToVec(StringReplacer(StringReplacer(VALUE_BASE, "LONG_VAL", "2"), "STR_VAL", "10086"))},
        {"FULL_NULL", ToVec(StringReplacer(StringReplacer(StringReplacer(VALUE_FIELD_FULL, "LONG_VAL", "3"),
            "STR_VAL", "\"TEST\""), "INT_VAL", "null"))},
        {"FULL_WRONG_TYPE", ToVec(StringReplacer(StringReplacer(StringReplacer(VALUE_FIELD_FULL, "LONG_VAL", "3"),
            "STR_VAL", "\"TEST\""), "INT_VAL", "\"UT\""))},
    };

    // Key begin from "KEY_1"
    void InsertPresetEntry(KvStoreNbDelegate &delegate, const std::vector<std::string> &selection)
    {
        int count = 0;
        for (const auto &eachSel : selection) {
            ASSERT_NE(VALUE_MAP.count(eachSel), 0ul);
            DBStatus ret = delegate.Put(ToVec(std::string("KEY_") + std::to_string(++count)), VALUE_MAP.at(eachSel));
            ASSERT_EQ(ret, OK);
        }
    }
}

class DistributedDBInterfacesSchemaDatabaseUpgradeTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
};

void DistributedDBInterfacesSchemaDatabaseUpgradeTest::SetUpTestCase(void)
{
    DistributedDBToolsUnitTest::TestDirInit(g_testDir);
    KvStoreConfig config{g_testDir};
    g_manager.SetKvStoreConfig(config);
    ASSERT_EQ(SchemaChecker(SCHEMA_BASE), true);
    ASSERT_EQ(SchemaChecker(SCHEMA_INC_FIELD), true);
    ASSERT_EQ(SchemaChecker(SCHEMA_INC_FIELD_NOTNULL), true);
    ASSERT_EQ(SchemaChecker(SCHEMA_INC_FIELD_DEFAULT), true);
    ASSERT_EQ(SchemaChecker(SCHEMA_INC_FIELD_NOTNULL_DEFAULT), true);
}

void DistributedDBInterfacesSchemaDatabaseUpgradeTest::TearDownTestCase(void)
{
    if (DistributedDBToolsUnitTest::RemoveTestDbFiles(g_testDir) != 0) {
        LOGE("[TestSchemaUpgrade] Remove test directory error.");
    }
}

void DistributedDBInterfacesSchemaDatabaseUpgradeTest::SetUp(void)
{
    g_kvDelegateStatus = INVALID_ARGS;
    g_kvDelegatePtr = nullptr;
}

void DistributedDBInterfacesSchemaDatabaseUpgradeTest::TearDown(void)
{
    if (g_kvDelegatePtr != nullptr) {
        ASSERT_EQ(g_manager.CloseKvStore(g_kvDelegatePtr), OK);
        g_kvDelegatePtr = nullptr;
    }
}

/**
  * @tc.name: UpgradeFromKv001
  * @tc.desc: Schema database upgrade from kv database, exist value match compatible schema(mismatch strict schema)
  * @tc.type: FUNC
  * @tc.require: AR000F3OPD
  * @tc.author: xiaozhenjian
  */
HWTEST_F(DistributedDBInterfacesSchemaDatabaseUpgradeTest, UpgradeFromKv001, TestSize.Level1)
{
    /**
     * @tc.steps: step1. Prepare kv database with value match compatible schema(mismatch strict schema) then close
     * @tc.expected: step1. E_OK
     */
    std::string storeId = "UpgradeFromKv001";
    KvStoreNbDelegate::Option option;
    g_manager.GetKvStore(storeId, option, g_kvDelegateCallback);
    ASSERT_TRUE(g_kvDelegatePtr != nullptr);
    ASSERT_EQ(g_kvDelegateStatus, OK);

    InsertPresetEntry(*g_kvDelegatePtr, std::vector<std::string>{"LACK", "BASE", "LACK", "BASE", "FULL"});
    DBStatus ret = g_kvDelegatePtr->Delete(ToVec("KEY_4"));
    ASSERT_EQ(ret, OK);
    ASSERT_EQ(g_manager.CloseKvStore(g_kvDelegatePtr), OK);
    g_kvDelegatePtr = nullptr;

    /**
     * @tc.steps: step2. Upgrade to schema(strict) database
     * @tc.expected: step2. SCHEMA_VIOLATE_VALUE
     */
    option.schema = SchemaSwitchMode(SCHEMA_BASE);
    g_manager.GetKvStore(storeId, option, g_kvDelegateCallback);
    ASSERT_TRUE(g_kvDelegatePtr == nullptr);
    ASSERT_EQ(g_kvDelegateStatus, SCHEMA_VIOLATE_VALUE);

    /**
     * @tc.steps: step3. Upgrade to schema(compatible) database
     * @tc.expected: step3. E_OK
     */
    option.schema = SCHEMA_BASE;
    g_manager.GetKvStore(storeId, option, g_kvDelegateCallback);
    ASSERT_TRUE(g_kvDelegatePtr != nullptr);
    ASSERT_EQ(g_kvDelegateStatus, OK);

    /**
     * @tc.steps: step4. Query field_2.field_3 equal OpenHarmony
     * @tc.expected: step4. E_OK, KEY_1
     */
    Query query = Query::Select().EqualTo("field_2.field_3", "OpenHarmony");
    std::vector<Entry> entries;
    EXPECT_EQ(g_kvDelegatePtr->GetEntries(query, entries), OK);
    ASSERT_EQ(entries.size(), 2ul);
    EXPECT_EQ(ToStr(entries[0].key), std::string("KEY_1"));
    EXPECT_EQ(ToStr(entries[1].key), std::string("KEY_3"));
    std::string valStr = ToStr(entries[0].value);
    std::string defaultVal = "OpenHarmony";
    auto iter = std::search(valStr.begin(), valStr.end(), defaultVal.begin(), defaultVal.end());
    EXPECT_TRUE(iter != valStr.end());
}

/**
  * @tc.name: UpgradeFromKv002
  * @tc.desc: Schema database upgrade from kv database, exist value mismatch compatible schema
  * @tc.type: FUNC
  * @tc.require: AR000F3OPD
  * @tc.author: xiaozhenjian
  */
HWTEST_F(DistributedDBInterfacesSchemaDatabaseUpgradeTest, UpgradeFromKv002, TestSize.Level1)
{
    /**
     * @tc.steps: step1. Prepare kv database with value mismatch compatible schema then close
     * @tc.expected: step1. E_OK
     */
    std::string storeId = "UpgradeFromKv002";
    KvStoreNbDelegate::Option option;
    g_manager.GetKvStore(storeId, option, g_kvDelegateCallback);
    ASSERT_TRUE(g_kvDelegatePtr != nullptr);
    ASSERT_EQ(g_kvDelegateStatus, OK);

    InsertPresetEntry(*g_kvDelegatePtr, std::vector<std::string>{"BASE_WRONG_TYPE"});
    ASSERT_EQ(g_manager.CloseKvStore(g_kvDelegatePtr), OK);
    g_kvDelegatePtr = nullptr;

    /**
     * @tc.steps: step2. Upgrade to schema(compatible) database
     * @tc.expected: step2. SCHEMA_VIOLATE_VALUE
     */
    option.schema = SCHEMA_BASE;
    g_manager.GetKvStore(storeId, option, g_kvDelegateCallback);
    ASSERT_TRUE(g_kvDelegatePtr == nullptr);
    ASSERT_EQ(g_kvDelegateStatus, SCHEMA_VIOLATE_VALUE);
}

namespace {
void TestUpgradeFromSchema(const std::string &storeId, const std::vector<std::string> &selection,
    const std::string &newSchema, bool expectMatch, uint32_t expectCount)
{
    LOGI("[TestUpgradeFromSchema] StoreId=%s", storeId.c_str());
    /**
     * @tc.steps: step1. Prepare kv database with value then close
     * @tc.expected: step1. E_OK
     */
    KvStoreNbDelegate::Option option;
    option.schema = g_baseSchema;
    g_manager.GetKvStore(storeId, option, g_kvDelegateCallback);
    ASSERT_TRUE(g_kvDelegatePtr != nullptr);
    ASSERT_EQ(g_kvDelegateStatus, OK);

    InsertPresetEntry(*g_kvDelegatePtr, selection);
    ASSERT_EQ(g_manager.CloseKvStore(g_kvDelegatePtr), OK);
    g_kvDelegatePtr = nullptr;

    /**
     * @tc.steps: step2. Upgrade to schema database
     * @tc.expected: step2. OK or SCHEMA_VIOLATE_VALUE
     */
    option.schema = newSchema;
    g_manager.GetKvStore(storeId, option, g_kvDelegateCallback);
    if (expectMatch) {
        ASSERT_TRUE(g_kvDelegatePtr != nullptr);
        ASSERT_EQ(g_kvDelegateStatus, OK);
    } else {
        ASSERT_TRUE(g_kvDelegatePtr == nullptr);
        ASSERT_EQ(g_kvDelegateStatus, g_expectError);
    }

    /**
     * @tc.steps: step3. Query field_2.field_3
     * @tc.expected: step3. E_OK
     */
    if (expectMatch) {
        Query query = Query::Select().EqualTo("field_2.field_4", 88); // 88 is the default value in the testcase.
        std::vector<Entry> entries;
        if (expectCount == 0) {
            EXPECT_EQ(g_kvDelegatePtr->GetEntries(query, entries), NOT_FOUND);
        } else {
            ASSERT_EQ(g_kvDelegatePtr->GetEntries(query, entries), OK);
            EXPECT_EQ(entries.size(), expectCount);
        }
        ASSERT_EQ(g_manager.CloseKvStore(g_kvDelegatePtr), OK);
        g_kvDelegatePtr = nullptr;
    }
}
}

/**
  * @tc.name: UpgradeFromSchema001
  * @tc.desc: Schema database upgrade from kv database, exist value match new schema
  * @tc.type: FUNC
  * @tc.require: AR000F3OPD
  * @tc.author: xiaozhenjian
  */
HWTEST_F(DistributedDBInterfacesSchemaDatabaseUpgradeTest, UpgradeFromSchema001, TestSize.Level1)
{
    g_baseSchema = SCHEMA_BASE;
    g_expectError = SCHEMA_VIOLATE_VALUE;
    TestUpgradeFromSchema("UpgradeFromSchema001_1", std::vector<std::string>{"LACK", "BASE", "FULL", "FULL_NULL"},
        SCHEMA_INC_FIELD, true, 0);
    TestUpgradeFromSchema("UpgradeFromSchema001_2", std::vector<std::string>{"LACK", "BASE", "FULL", "FULL_NULL"},
        SCHEMA_INC_FIELD_DEFAULT, true, 2);
    TestUpgradeFromSchema("UpgradeFromSchema001_3", std::vector<std::string>{"LACK", "BASE", "FULL"},
        SCHEMA_INC_FIELD_NOTNULL_DEFAULT, true, 2);
}

/**
  * @tc.name: UpgradeFromSchema002
  * @tc.desc: Schema database upgrade from kv database, exist value mismatch new schema
  * @tc.type: FUNC
  * @tc.require: AR000F3OPD
  * @tc.author: xiaozhenjian
  */
HWTEST_F(DistributedDBInterfacesSchemaDatabaseUpgradeTest, UpgradeFromSchema002, TestSize.Level1)
{
    g_baseSchema = SCHEMA_BASE;
    g_expectError = SCHEMA_VIOLATE_VALUE;
    TestUpgradeFromSchema("UpgradeFromSchema002_1", std::vector<std::string>{"LACK", "BASE", "FULL", "FULL_WRONG_TYPE"},
        SCHEMA_INC_FIELD, false, 0);
    TestUpgradeFromSchema("UpgradeFromSchema002_2", std::vector<std::string>{"LACK", "BASE", "FULL", "FULL_WRONG_TYPE"},
        SCHEMA_INC_FIELD_DEFAULT, false, 0);
    TestUpgradeFromSchema("UpgradeFromSchema002_3", std::vector<std::string>{"LACK", "BASE", "FULL", "FULL_NULL"},
        SCHEMA_INC_FIELD_NOTNULL_DEFAULT, false, 0);
}

/**
  * @tc.name: UpgradeFromSchema003
  * @tc.desc: Schema database upgrade from kv database, new schema incompatible with old schema
  * @tc.type: FUNC
  * @tc.require: AR000F3OPD
  * @tc.author: xiaozhenjian
  */
HWTEST_F(DistributedDBInterfacesSchemaDatabaseUpgradeTest, UpgradeFromSchema003, TestSize.Level1)
{
    // Compatible schema can increase field, but must not be null without defaut.
    g_baseSchema = SCHEMA_BASE;
    g_expectError = SCHEMA_MISMATCH;
    TestUpgradeFromSchema("UpgradeFromSchema003_1", std::vector<std::string>{"LACK", "BASE", "FULL", "FULL_NULL"},
        SCHEMA_INC_FIELD_NOTNULL, false, 0);
    // Strict schema can not incrase field
    g_baseSchema = SchemaSwitchMode(SCHEMA_BASE);
    TestUpgradeFromSchema("UpgradeFromSchema003_2", std::vector<std::string>{"LACK", "BASE"},
        SchemaSwitchMode(SCHEMA_INC_FIELD), false, 0);
}
#endif