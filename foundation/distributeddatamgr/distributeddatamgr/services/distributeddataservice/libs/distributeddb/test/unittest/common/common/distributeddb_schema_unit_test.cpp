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

#include "distributeddb_tools_unit_test.h"

#include <gtest/gtest.h>

#include "schema_utils.h"
#include "db_errno.h"
#include "log_print.h"

using namespace testing::ext;
using namespace DistributedDB;
using namespace DistributedDBUnitTest;
using namespace std;

class DistributedDBSchemalTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
};

void DistributedDBSchemalTest::SetUpTestCase(void)
{
}

void DistributedDBSchemalTest::TearDownTestCase(void)
{
}

void DistributedDBSchemalTest::SetUp(void)
{
}

void DistributedDBSchemalTest::TearDown(void)
{
}

namespace {
map<string, SchemaAttribute> g_schemaAttrDefTestDataDir;

void CheckSchemaAttribute(const SchemaAttribute &res, const SchemaAttribute &check)
{
    EXPECT_EQ(res.type, check.type);
    EXPECT_EQ(res.isIndexable, check.isIndexable);
    EXPECT_EQ(res.hasNotNullConstraint, check.hasNotNullConstraint);
    EXPECT_EQ(res.hasDefaultValue, check.hasDefaultValue);
    EXPECT_EQ(res.defaultValue.stringValue, check.defaultValue.stringValue);
    EXPECT_EQ(memcmp(&res.defaultValue, &check.defaultValue, 8), 0); // only check this unit 8 byte
}

void PreNumDataForParseAndCheckSchemaAttribute003()
{
    SchemaAttribute attributeRes;
    attributeRes.type = FieldType::LEAF_FIELD_INTEGER;
    attributeRes.defaultValue.integerValue = 0;
    attributeRes.hasDefaultValue = true;
    g_schemaAttrDefTestDataDir["INTEGER, DEFAULT 0"] = attributeRes;

    SchemaAttribute attributeRes1;
    attributeRes1.type = FieldType::LEAF_FIELD_INTEGER;
    attributeRes1.hasDefaultValue = true;
    attributeRes1.hasNotNullConstraint = true;
    attributeRes1.defaultValue.integerValue = INT32_MAX;
    g_schemaAttrDefTestDataDir["INTEGER, NOT NULL, DEFAULT " + std::to_string(INT32_MAX)] = attributeRes1;

    SchemaAttribute attributeRes2;
    attributeRes2.type = FieldType::LEAF_FIELD_INTEGER;
    attributeRes2.hasDefaultValue = true;
    attributeRes2.defaultValue.integerValue = 0;
    g_schemaAttrDefTestDataDir["INTEGER, DEFAULT +0"] = attributeRes2;

    SchemaAttribute attributeRes3;
    attributeRes3.type = FieldType::LEAF_FIELD_LONG;
    attributeRes3.hasDefaultValue = true;
    attributeRes3.defaultValue.longValue = 0;
    g_schemaAttrDefTestDataDir["LONG, DEFAULT -0"] = attributeRes3;

    SchemaAttribute attributeRes4;
    attributeRes4.type = FieldType::LEAF_FIELD_LONG;
    attributeRes4.hasNotNullConstraint = true;
    attributeRes4.hasDefaultValue = true;
    attributeRes4.defaultValue.longValue = LONG_MAX;
    g_schemaAttrDefTestDataDir["LONG, NOT NULL,DEFAULT " + std::to_string(LONG_MAX)] = attributeRes4;
}

void PreStringDataForParseAndCheckSchemaAttribute003()
{
    SchemaAttribute attributeRes5;
    attributeRes5.type = FieldType::LEAF_FIELD_STRING;
    attributeRes5.hasDefaultValue = true;
    attributeRes5.defaultValue.stringValue = "11ada%$%";
    g_schemaAttrDefTestDataDir["STRING , DEFAULT '11ada%$%'"] = attributeRes5;

    SchemaAttribute attributeRes6;
    attributeRes6.type = FieldType::LEAF_FIELD_STRING;
    attributeRes6.hasNotNullConstraint = true;
    attributeRes6.hasDefaultValue = true;
    attributeRes6.defaultValue.stringValue = "asdasd_\n\t";
    g_schemaAttrDefTestDataDir["STRING, NOT NULL , DEFAULT 'asdasd_\n\t'"] = attributeRes6;
}

void PreDoubleDataForParseAndCheckSchemaAttribute003()
{
    SchemaAttribute attributeRes7;
    attributeRes7.type = FieldType::LEAF_FIELD_DOUBLE;
    attributeRes7.hasDefaultValue = true;
    attributeRes7.defaultValue.doubleValue = 0;
    g_schemaAttrDefTestDataDir["DOUBLE,DEFAULT 0.0"] = attributeRes7;

    SchemaAttribute attributeRes8;
    attributeRes8.type = FieldType::LEAF_FIELD_DOUBLE;
    attributeRes8.hasDefaultValue = true;
    attributeRes8.defaultValue.doubleValue = 0;
    g_schemaAttrDefTestDataDir["DOUBLE,DEFAULT 0."] = attributeRes8;

    SchemaAttribute attributeRes9;
    attributeRes9.type = FieldType::LEAF_FIELD_DOUBLE;
    attributeRes9.hasDefaultValue = true;
    attributeRes9.defaultValue.doubleValue = 0.1; // test data
    g_schemaAttrDefTestDataDir["DOUBLE,DEFAULT 0.1"] = attributeRes9;

    SchemaAttribute attributeRes10;
    attributeRes10.type = FieldType::LEAF_FIELD_DOUBLE;
    attributeRes10.hasNotNullConstraint = true;
    attributeRes10.hasDefaultValue = true;
    attributeRes10.defaultValue.doubleValue = -0.123456; // test data
    g_schemaAttrDefTestDataDir["DOUBLE, NOT NULL,DEFAULT -0.123456"] = attributeRes10;

    SchemaAttribute attributeRes11;
    attributeRes11.type = FieldType::LEAF_FIELD_DOUBLE;
    attributeRes11.hasNotNullConstraint = false;
    attributeRes11.hasDefaultValue = true;
    attributeRes11.defaultValue.doubleValue = 0;
    g_schemaAttrDefTestDataDir["DOUBLE,DEFAULT +0.0"] = attributeRes11;

    // double -0 Has been manually verified
    SchemaAttribute attributeRes13;
    attributeRes13.type = FieldType::LEAF_FIELD_DOUBLE;
    attributeRes13.hasNotNullConstraint = true;
    attributeRes13.hasDefaultValue = true;
    attributeRes13.defaultValue.doubleValue = DBL_MAX;
    g_schemaAttrDefTestDataDir["DOUBLE, NOT NULL,DEFAULT " + std::to_string(DBL_MAX)] = attributeRes13;
}

void PreBoolDataForParseAndCheckSchemaAttribute003()
{
    SchemaAttribute attributeRes14;
    attributeRes14.type = FieldType::LEAF_FIELD_BOOL;
    attributeRes14.hasNotNullConstraint = false;
    attributeRes14.hasDefaultValue = true;
    attributeRes14.defaultValue.boolValue = false;
    g_schemaAttrDefTestDataDir["BOOL,DEFAULT false"] = attributeRes14;

    SchemaAttribute attributeRes15;
    attributeRes15.type = FieldType::LEAF_FIELD_BOOL;
    attributeRes15.hasNotNullConstraint = true;
    attributeRes15.hasDefaultValue = true;
    attributeRes15.defaultValue.boolValue = true;
    g_schemaAttrDefTestDataDir["BOOL, NOT NULL,DEFAULT true"] = attributeRes15;
}
} // namespace

/**
 * @tc.name: ParseAndCheckSchemaAttribute001
 * @tc.desc: Ability to recognize and parse the correct schema attribute format
 * @tc.type: FUNC
 * @tc.require: AR000DR9K3
 * @tc.author: sunpeng
 */
HWTEST_F(DistributedDBSchemalTest, ParseAndCheckSchemaAttribute001, TestSize.Level0)
{
    /**
     * @tc.steps: step1. Preset shcema attribute strings that are correctly written according to the definition.
     */
    SchemaAttribute attributeRes;
    attributeRes.type = FieldType::LEAF_FIELD_INTEGER;
    g_schemaAttrDefTestDataDir["INTEGER"] = attributeRes;

    SchemaAttribute attributeRes1;
    attributeRes1.type = FieldType::LEAF_FIELD_BOOL;
    attributeRes1.hasNotNullConstraint = true;
    g_schemaAttrDefTestDataDir["BOOL, NOT NULL"] = attributeRes1;

    SchemaAttribute attributeRes2;
    attributeRes2.type = FieldType::LEAF_FIELD_STRING;
    attributeRes2.hasDefaultValue = true;
    attributeRes2.defaultValue.stringValue = "dasdads";
    g_schemaAttrDefTestDataDir["STRING,DEFAULT 'dasdads'"] = attributeRes2;

    SchemaAttribute attributeRes3;
    attributeRes3.type = FieldType::LEAF_FIELD_DOUBLE;
    attributeRes3.hasDefaultValue = true;
    attributeRes3.hasNotNullConstraint = true;
    attributeRes3.defaultValue.doubleValue = -1.0;
    g_schemaAttrDefTestDataDir["\tDOUBLE  \t,\t\t\tNOT NULL   , DEFAULT -1.0"] = attributeRes3;

    SchemaAttribute attributeRes4;
    attributeRes4.type = FieldType::LEAF_FIELD_LONG;
    attributeRes4.hasNotNullConstraint = false;
    attributeRes4.hasDefaultValue = false;
    g_schemaAttrDefTestDataDir["LONG,DEFAULT null"] = attributeRes4;

    /**
     * @tc.steps: step2. Call interface
     * @tc.expected: step2. Returns E_OK and parses correctly.
     */
    for (auto &iter : g_schemaAttrDefTestDataDir) {
        SchemaAttribute attributeOut;
        LOGD("Attr : %s", iter.first.c_str());
        EXPECT_EQ(SchemaUtils::ParseAndCheckSchemaAttribute(iter.first, attributeOut), E_OK);
        CheckSchemaAttribute(iter.second, attributeOut);
    }
}

/**
 * @tc.name: ParseAndCheckSchemaAttribute002
 * @tc.desc: Can identify the wrong schema attribute format and report an error.
 * @tc.type: FUNC
 * @tc.require: AR000DR9K3
 * @tc.author: sunpeng
 */
HWTEST_F(DistributedDBSchemalTest, ParseAndCheckSchemaAttribute002, TestSize.Level0)
{
    /**
     * @tc.steps: step1. Preset shcema attributes based on definition error.
     */
    std::vector<std::string> preData = {
        "",
        "      ",
        "$INTEGER",
        "INTEGER NOT_NULL DEFAULT 1",
        "STRING \n DEFAULT 'a'",
        "BOOL,NOT   NULL",
        "LONG,NOT\tNULL",
        "BOOL,NOT null",
        "bool,not null",
        "BOOL,NOT NULL,default false",
        "INTEGER,",
        "BOOL, NOT NULL,",
        "BOOL, NOT NULL,DEFAULT ",
        "BOOL, DEFAULT false, NOT NULL",
        "DEFAULT 1, LONG, NOT NULL",
        "DEFAULT 1",
        "NOT NULL, DEFAULT x",
        ", NOT NULL DEFAULT 1",
        "LONG, NOT NULL, DEFAULT null"
    };
    string overflowDol = to_string(DBL_MAX);
    overflowDol = '1' + overflowDol;
    preData.push_back("DOUBLE, NOT NULL, DEFAULT " + overflowDol);
    preData.push_back("DOUBLE, NOT NULL, DEFAULT -" + overflowDol);

    preData.push_back("INTEGER, NOT NULL, DEFAULT 2147483648"); // int max + 1;
    preData.push_back("INTEGER, NOT NULL, DEFAULT -2147483649");
    preData.push_back("LONG, NOT NULL, DEFAULT 9223372036854775808"); // long max + 1;
    preData.push_back("LONG, NOT NULL, DEFAULT -9223372036854775809");

    /**
     * @tc.steps: step2. Call interface ParseAndCheckSchemaAttribute.
     * @tc.expected: step2. Returns -E_SCHEMA_PARSE_FAIL.
     */
    for (auto &iter : preData) {
        SchemaAttribute attributeOut;
        LOGD("Attr : %s", iter.c_str());
        EXPECT_EQ(SchemaUtils::ParseAndCheckSchemaAttribute(iter, attributeOut), -E_SCHEMA_PARSE_FAIL);
    }
}

/**
 * @tc.name: ParseAndCheckSchemaAttribute003
 * @tc.desc: Can correctly interpret the meaning of each keyword of the schema attribute
 * @tc.type: FUNC
 * @tc.require: AR000DR9K3
 * @tc.author: sunpeng
 */
HWTEST_F(DistributedDBSchemalTest, ParseAndCheckSchemaAttribute003, TestSize.Level0)
{
    /**
     * @tc.steps: step1. Preset shcema attributes based on defining correct format and content.
     */
    g_schemaAttrDefTestDataDir.clear();
    PreNumDataForParseAndCheckSchemaAttribute003();
    PreDoubleDataForParseAndCheckSchemaAttribute003();
    PreStringDataForParseAndCheckSchemaAttribute003();
    PreBoolDataForParseAndCheckSchemaAttribute003();

    /**
     * @tc.steps: step2. Call interface ParseAndCheckSchemaAttribute.
     * @tc.expected: step2. Returns E_OK and parses correctly.
     */
    for (auto &iter : g_schemaAttrDefTestDataDir) {
        SchemaAttribute attributeOut;
        EXPECT_EQ(SchemaUtils::ParseAndCheckSchemaAttribute(iter.first, attributeOut), E_OK);
        CheckSchemaAttribute(iter.second, attributeOut);
    }
}

/**
 * @tc.name: ParseAndCheckSchemaAttribute004
 * @tc.desc: Can correctly identify the meaning of the schema attribute field that is incorrectly parsed
 * @tc.type: FUNC
 * @tc.require: AR000DR9K3
 * @tc.author: sunpeng
 */
HWTEST_F(DistributedDBSchemalTest, ParseAndCheckSchemaAttribute004, TestSize.Level0)
{
    /**
     * @tc.steps: step1. Preset shcema attributes based on defining incorrect format and content.
     */
    std::vector<std::string> preData = {
        "LONG,NOT   NULL, DEFAULT '123'",
        "STRING,DEFAULT true",
        "INTEGER,NOT   NULL,DEFAULT MAX+1",
        "LONG,DEFAULT 0.0",
        "INTEGER,NOT   NULL,DEFAULT - 123",
        "INTEGER,DEFAULT 12 3",
        "LONG,NOT   NULL,DEFAULT 0xFF",
        "INTEGER,00",
        "DOUBLE,DEFAULT 123a",
        "DOUBLE,NOT   NULL,DEFAULT 0..0",
        "DOUBLE,DEFAULT 2e2",
        "DOUBLE,NOT   NULL,DEFAULT 1+1",
        "DOUBLE,NOT   NULL,DEFAULT .0",
        "DOUBLE,DEFAULT MAX+1",
        "STRING,DEFAULT 123",
        "STRING,NOT   NULL,DEFAULT 'ABC",
        "BOOL,DEFAULT TRUE",
        "INT",
        "long",
        "String",
        "STRING DEFAULT 'a'a",
    };

    /**
     * @tc.steps: step2. Call interface ParseAndCheckSchemaAttribute.
     * @tc.expected: step2. Returns -E_SCHEMA_PARSE_FAIL.
     */
    string overSize(4 * 1024 + 1, 'a');
    preData.push_back("STRING, DEFAULT '" + overSize + "'");
    LOGD("%s", preData[0].c_str());
    for (auto &iter : preData) {
        SchemaAttribute attributeOut;
        EXPECT_EQ(SchemaUtils::ParseAndCheckSchemaAttribute(iter, attributeOut), -E_SCHEMA_PARSE_FAIL);
    }
}

/**
 * @tc.name: CheckFieldName001
 * @tc.desc: Correctly identify field names
 * @tc.type: FUNC
 * @tc.require: AR000DR9K3
 * @tc.author: sunpeng
 */
HWTEST_F(DistributedDBSchemalTest, CheckFieldName001, TestSize.Level0)
{
    /**
     * @tc.steps: step1. Enter the preset correct string array into CheckFieldName and check.
     * @tc.expected: step1. Returns E_OK.
     */
    std::vector<std::string> preData = {
        "_abc",
        "_123abc",
        "a_123_",
    };
    string overSize(64, 'a');
    preData.push_back(overSize);
    for (auto &iter : preData) {
        EXPECT_EQ(SchemaUtils::CheckFieldName(iter), E_OK);
    }
}

/**
 * @tc.name: CheckFieldName002
 * @tc.desc: Identify illegal field name
 * @tc.type: FUNC
 * @tc.require: AR000DR9K3
 * @tc.author: sunpeng
 */
HWTEST_F(DistributedDBSchemalTest, CheckFieldName002, TestSize.Level0)
{
    /**
     * @tc.steps: step1. Enter the preset incorrect string array into CheckFieldName and check.
     * @tc.expected: step1. Returns -E_SCHEMA_PARSE_FAIL.
     */
    std::vector<std::string> preData = {
        "123abc",
        "$.LONG",
        "",
        "     abc",
        "\tabc"
    };
    string overSize(65, 'a');
    preData.push_back(overSize);
    for (auto &iter : preData) {
        EXPECT_EQ(SchemaUtils::CheckFieldName(iter), -E_SCHEMA_PARSE_FAIL);
    }

}

/**
 * @tc.name: ParseAndCheckFieldPath001
 * @tc.desc: Correctly identify and parse shema index fields
 * @tc.type: FUNC
 * @tc.require: AR000DR9K3
 * @tc.author: sunpeng
 */
HWTEST_F(DistributedDBSchemalTest, ParseAndCheckFieldPath001, TestSize.Level0)
{
    /**
     * @tc.steps: step1. Enter the array of preset correct strings into ParseAndCheckFieldPath and check result.
     * @tc.expected: step1. Returns E_OK and Parse correctly.
     */
    vector<pair<string, vector<string> > > testPreData {
        // test
        // ans
        {"$.abc.def.fg",
        {"abc", "def", "fg"}},

        {"$.abc._def.fg",
        {"abc", "_def", "fg"}},

        {"$._.__.___",
        {"_", "__", "___"}},

        {"$._.__1234.abc455545",
        {"_", "__1234", "abc455545"}},

        {" $.abc._def.fg",
        {"abc", "_def", "fg"}},

        {" $.a.a.a.a",
        {"a", "a", "a", "a"}},

        {"$.abc._def.fg ",
        {"abc", "_def", "fg"}},

        {"  $.abc._def.fg   ",
        {"abc", "_def", "fg"}},

        {"\t$.abc.def.fg ",
        {"abc", "def", "fg"}},

        {" $.abc.def.fg\r\t",
        {"abc", "def", "fg"}},

        {"\r$.abc.def.fg\t\r",
        {"abc", "def", "fg"}},
    };

    for (auto &iter : testPreData) {
        FieldPath ans;
        EXPECT_EQ(SchemaUtils::ParseAndCheckFieldPath(iter.first, ans), E_OK);
        EXPECT_EQ(ans, iter.second);
    }
}

/**
 * @tc.name: ParseAndCheckFieldPath002
 * @tc.desc: Correctly identify illegal shema index fields
 * @tc.type: FUNC
 * @tc.require: AR000DR9K3
 * @tc.author: sunpeng
 */
HWTEST_F(DistributedDBSchemalTest, ParseAndCheckFieldPath002, TestSize.Level0)
{
    /**
     * @tc.steps: step1. Enter the array of preset illegal strings into ParseAndCheckFieldPath and check result.
     * @tc.expected: step1. Returns -E_SCHEMA_PARSE_FAIL.
     */
    vector<string> testPreData {
        "",
        "\t",
        "\r",
        "\r\t",
        "  ",
        "$",
        "$.",
        "    .   ",
        "$$",
        "$.$",
        "$.a.b.c.d.e",
        "$..abc",
        "$.abc..def.fg",
        "$abc.def.fg.",
        "$.123",
        "$.abc123%",
        "$.abc.\0.fg",
        "$.abc.fg.\0",
        "\"$.abc.def.fg\"",
        "$.\"abc\".def.fg",
        "$.\"abc\n.def.fg",
    };

    for (auto &iter : testPreData) {
        FieldPath ans;
        EXPECT_EQ(SchemaUtils::ParseAndCheckFieldPath(iter, ans), -E_SCHEMA_PARSE_FAIL);
    }
}