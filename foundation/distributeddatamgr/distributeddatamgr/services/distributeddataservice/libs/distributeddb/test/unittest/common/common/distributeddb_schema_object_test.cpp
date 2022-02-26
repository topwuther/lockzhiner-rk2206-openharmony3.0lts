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
#include <cmath>

#include "db_errno.h"
#include "log_print.h"
#include "schema_object.h"
#include "schema_utils.h"

using namespace std;
using namespace testing::ext;
using namespace DistributedDB;

namespace {
    const std::string VALID_SCHEMA_FULL_DEFINE = "{\"SCHEMA_VERSION\":\"1.0\","
        "\"SCHEMA_MODE\":\"STRICT\","
        "\"SCHEMA_DEFINE\":{"
            "\"field_name1\":\"BOOL\","
            "\"field_name2\":{"
                "\"field_name3\":\"INTEGER, NOT NULL\","
                "\"field_name4\":\"LONG, DEFAULT 100\","
                "\"field_name5\":\"DOUBLE, NOT NULL, DEFAULT 3.14\","
                "\"field_name6\":\"STRING, NOT NULL, DEFAULT '3.1415'\","
                "\"field_name7\":[],"
                "\"field_name8\":{}"
            "}"
        "},"
        "\"SCHEMA_INDEXES\":[\"$.field_name1\", \"$.field_name2.field_name6\"]}";
    const std::string VALID_SCHEMA_INDEX_EMPTY = "{\"SCHEMA_VERSION\":\"1.0\","
        "\"SCHEMA_MODE\":\"STRICT\","
        "\"SCHEMA_DEFINE\":{"
            "\"field_name1\":\"BOOL\""
        "},"
        "\"SCHEMA_INDEXES\":[]}";
    const std::string VALID_SCHEMA_NO_INDEX = "{\"SCHEMA_VERSION\":\"1.0\","
        "\"SCHEMA_MODE\":\"STRICT\","
        "\"SCHEMA_DEFINE\":{"
            "\"field_name1\":\"BOOL\""
        "}}";
    const std::string VALID_SCHEMA_PRE_SUF_BLANK = "{\"SCHEMA_VERSION\":\" 1.0\","
        "\"SCHEMA_MODE\":\"STRICT  \","
        "\"SCHEMA_DEFINE\":{"
            "\"field_name1\":\"   BOOL    \""
        "}}";

    const std::string INVALID_SCHEMA_INVALID_JSON = "[\"$.field_name1\", \"$.field_name2.field_name6\"]";
    const std::string INVALID_SCHEMA_LESS_META_FIELD = "{\"SCHEMA_VERSION\":\" 1.0\","
        "\"SCHEMA_MODE\":\"STRICT\"}";
    const std::string INVALID_SCHEMA_MORE_META_FIELD = "{\"SCHEMA_VERSION\":\"1.0\","
        "\"SCHEMA_MODE\":\"STRICT\","
        "\"SCHEMA_DEFINE\":{"
            "\"field_name1\":\"BOOL\""
        "},"
        "\"SCHEMA_UNDEFINE_META_FIELD\":[]}";
    const std::string INVALID_SCHEMA_WRONG_VERSION = "{\"SCHEMA_VERSION\":\"1.1\","
        "\"SCHEMA_MODE\":\"STRICT\","
        "\"SCHEMA_DEFINE\":{"
            "\"field_name1\":\"BOOL\""
        "}}";
    const std::string INVALID_SCHEMA_WRONG_MODE = "{\"SCHEMA_VERSION\":\"1.0\","
        "\"SCHEMA_MODE\":\"WRONG_MODE\","
        "\"SCHEMA_DEFINE\":{"
            "\"field_name1\":\"BOOL\""
        "}}";

    const std::string INVALID_SCHEMA_DEFINE_EMPTY = "{\"SCHEMA_VERSION\":\"1.0\","
        "\"SCHEMA_MODE\":\"STRICT\","
        "\"SCHEMA_DEFINE\":{}}";
    const std::string INVALID_SCHEMA_DEFINE_NEST_TOO_DEEP = "{\"SCHEMA_VERSION\":\"1.0\","
        "\"SCHEMA_MODE\":\"STRICT\","
        "\"SCHEMA_DEFINE\":{"
            "\"field_name1\":{"
                "\"field_name2\":{"
                    "\"field_name3\":{"
                        "\"field_name4\":{"
                            "\"field_name5\":{"
        "}}}}}}}";
    const std::string INVALID_SCHEMA_DEFINE_INVALID_FIELD_NAME = "{\"SCHEMA_VERSION\":\"1.0\","
        "\"SCHEMA_MODE\":\"STRICT\","
        "\"SCHEMA_DEFINE\":{"
            "\"12345\":\"BOOL\""
        "}}";
    const std::string INVALID_SCHEMA_DEFINE_INVALID_FIELD_ATTR = "{\"SCHEMA_VERSION\":\"1.0\","
        "\"SCHEMA_MODE\":\"STRICT\","
        "\"SCHEMA_DEFINE\":{"
            "\"field_name1\":\"BOOL, NOT NULL, DEFAULT null\""
        "}}";
    const std::string INVALID_SCHEMA_DEFINE_INVALID_ARRAY_TYPE = "{\"SCHEMA_VERSION\":\"1.0\","
        "\"SCHEMA_MODE\":\"STRICT\","
        "\"SCHEMA_DEFINE\":{"
            "\"field_name1\":[3.14]"
        "}}";

    const std::string INVALID_SCHEMA_INDEX_INVALID = "{\"SCHEMA_VERSION\":\"1.0\","
        "\"SCHEMA_MODE\":\"STRICT\","
        "\"SCHEMA_DEFINE\":{"
            "\"field_name1\":\"BOOL\""
        "},"
        "\"SCHEMA_INDEXES\":[true, false]}";
    const std::string INVALID_SCHEMA_INDEX_PATH_INVALID = "{\"SCHEMA_VERSION\":\"1.0\","
        "\"SCHEMA_MODE\":\"STRICT\","
        "\"SCHEMA_DEFINE\":{"
            "\"field_name1\":\"BOOL\""
        "},"
        "\"SCHEMA_INDEXES\":[\".field_name1\"]}";
    const std::string INVALID_SCHEMA_INDEX_PATH_NOT_EXIST = "{\"SCHEMA_VERSION\":\"1.0\","
        "\"SCHEMA_MODE\":\"STRICT\","
        "\"SCHEMA_DEFINE\":{"
            "\"field_name1\":\"BOOL\""
        "},"
        "\"SCHEMA_INDEXES\":[\"$.field_name2\"]}";
    const std::string INVALID_SCHEMA_INDEX_PATH_NOT_INDEXABLE = "{\"SCHEMA_VERSION\":\"1.0\","
        "\"SCHEMA_MODE\":\"STRICT\","
        "\"SCHEMA_DEFINE\":{"
            "\"field_name1\":[]"
        "},"
        "\"SCHEMA_INDEXES\":[\"$.field_name1\"]}";
    const std::string INVALID_SCHEMA_INDEX_PATH_DUPLICATE = "{\"SCHEMA_VERSION\":\"1.0\","
        "\"SCHEMA_MODE\":\"STRICT\","
        "\"SCHEMA_DEFINE\":{"
            "\"field_name1\":\"BOOL\""
        "},"
        "\"SCHEMA_INDEXES\":[\"$.field_name1\", \"$.field_name1\"]}";

    const std::string SCHEMA_COMPARE_BASELINE = "{\"SCHEMA_VERSION\":\"1.0\","
        "\"SCHEMA_MODE\":\"COMPATIBLE\","
        "\"SCHEMA_DEFINE\":{"
            "\"field_name1\":\"BOOL, NOT NULL\","
            "\"field_name2\":\"INTEGER, DEFAULT 100\","
            "\"field_name3\":{"
                "\"field_name4\":\"STRING\","
                "\"field_name5\":{}"
        "}},"
        "\"SCHEMA_INDEXES\":[\"field_name1\"]}";
    const std::string SCHEMA_DEFINE_MORE_FIELD = "{\"SCHEMA_VERSION\":\"1.0\","
        "\"SCHEMA_MODE\":\"COMPATIBLE\","
        "\"SCHEMA_DEFINE\":{"
            "\"field_name1\":\"BOOL, NOT NULL\","
            "\"field_name2\":\"INTEGER, DEFAULT 100\","
            "\"field_name3\":{"
                "\"field_name4\":\"STRING\","
                "\"field_more1\":\"LONG\","
                "\"field_name5\":{"
                    "\"field_more2\":\"DOUBLE\""
        "}}},"
        "\"SCHEMA_INDEXES\":[\"field_name1\"]}";
    const std::string SCHEMA_DEFINE_MORE_FIELD_NOTNULL_FORBID = "{\"SCHEMA_VERSION\":\"1.0\","
        "\"SCHEMA_MODE\":\"COMPATIBLE\","
        "\"SCHEMA_DEFINE\":{"
            "\"field_name1\":\"BOOL, NOT NULL\","
            "\"field_name2\":\"INTEGER, DEFAULT 100\","
            "\"field_name3\":{"
                "\"field_name4\":\"STRING\","
                "\"field_more1\":\"LONG\","
                "\"field_name5\":{"
                    "\"field_more2\":\"DOUBLE, NOT NULL\""
        "}}},"
        "\"SCHEMA_INDEXES\":[\"field_name1\"]}";
    const std::string SCHEMA_DEFINE_MORE_FIELD_NOTNULL_PERMIT = "{\"SCHEMA_VERSION\":\"1.0\","
        "\"SCHEMA_MODE\":\"COMPATIBLE\","
        "\"SCHEMA_DEFINE\":{"
            "\"field_name1\":\"BOOL, NOT NULL\","
            "\"field_name2\":\"INTEGER, DEFAULT 100\","
            "\"field_name3\":{"
                "\"field_name4\":\"STRING\","
                "\"field_more1\":\"LONG, NOT NULL, DEFAULT 88\","
                "\"field_name5\":{"
                    "\"field_more2\":\"DOUBLE\""
        "}}},"
        "\"SCHEMA_INDEXES\":[\"field_name1\"]}";
    const std::string SCHEMA_DEFINE_LESS_FIELD = "{\"SCHEMA_VERSION\":\"1.0\","
        "\"SCHEMA_MODE\":\"COMPATIBLE\","
        "\"SCHEMA_DEFINE\":{"
            "\"field_name1\":\"BOOL, NOT NULL\","
            "\"field_name2\":\"INTEGER, DEFAULT 100\","
            "\"field_name3\":{"
                "\"field_name5\":{}"
        "}},"
        "\"SCHEMA_INDEXES\":[\"field_name1\"]}";
    const std::string SCHEMA_INDEX_MORE = "{\"SCHEMA_VERSION\":\"1.0\","
        "\"SCHEMA_MODE\":\"COMPATIBLE\","
        "\"SCHEMA_DEFINE\":{"
            "\"field_name1\":\"BOOL, NOT NULL\","
            "\"field_name2\":\"INTEGER, DEFAULT 100\","
            "\"field_name3\":{"
                "\"field_name4\":\"STRING\","
                "\"field_name5\":{}"
        "}},"
        "\"SCHEMA_INDEXES\":[\"field_name1\", [\"field_name2\", \"field_name3.field_name4\"]]}";
    const std::string SCHEMA_INDEX_LESS = "{\"SCHEMA_VERSION\":\"1.0\","
        "\"SCHEMA_MODE\":\"COMPATIBLE\","
        "\"SCHEMA_DEFINE\":{"
            "\"field_name1\":\"BOOL, NOT NULL\","
            "\"field_name2\":\"INTEGER, DEFAULT 100\","
            "\"field_name3\":{"
                "\"field_name4\":\"STRING\","
                "\"field_name5\":{}"
        "}},"
        "\"SCHEMA_INDEXES\":[]}";
    const std::string SCHEMA_INDEX_CHANGE = "{\"SCHEMA_VERSION\":\"1.0\","
        "\"SCHEMA_MODE\":\"COMPATIBLE\","
        "\"SCHEMA_DEFINE\":{"
            "\"field_name1\":\"BOOL, NOT NULL\","
            "\"field_name2\":\"INTEGER, DEFAULT 100\","
            "\"field_name3\":{"
                "\"field_name4\":\"STRING\","
                "\"field_name5\":{}"
        "}},"
        "\"SCHEMA_INDEXES\":[\"field_name1\", [\"field_name2\", \"field_name1\", \"field_name3.field_name4\"]]}";
    const std::string SCHEMA_DEFINE_MORE_FIELD_MORE_INDEX = "{\"SCHEMA_VERSION\":\"1.0\","
        "\"SCHEMA_MODE\":\"COMPATIBLE\","
        "\"SCHEMA_DEFINE\":{"
            "\"field_name1\":\"BOOL, NOT NULL\","
            "\"field_name2\":\"INTEGER, DEFAULT 100\","
            "\"field_name3\":{"
                "\"field_name4\":\"STRING\","
                "\"field_name5\":{"
                    "\"field_more1\":\"DOUBLE\""
        "}}},"
        "\"SCHEMA_INDEXES\":[\"field_name1\", [\"field_name2\", \"field_name3.field_name4\"]]}";
    const std::string SCHEMA_SKIPSIZE_DIFFER = "{\"SCHEMA_VERSION\":\"1.0\","
        "\"SCHEMA_MODE\":\"COMPATIBLE\","
        "\"SCHEMA_SKIPSIZE\":1,"
        "\"SCHEMA_DEFINE\":{"
            "\"field_name1\":\"BOOL, NOT NULL\","
            "\"field_name2\":\"INTEGER, DEFAULT 100\","
            "\"field_name3\":{"
                "\"field_name4\":\"STRING\","
                "\"field_name5\":{}"
        "}},"
        "\"SCHEMA_INDEXES\":[\"field_name1\"]}";
    const std::string SCHEMA_DEFINE_TYPE_DIFFER = "{\"SCHEMA_VERSION\":\"1.0\","
        "\"SCHEMA_MODE\":\"COMPATIBLE\","
        "\"SCHEMA_DEFINE\":{"
            "\"field_name1\":\"BOOL, NOT NULL\","
            "\"field_name2\":\"INTEGER, DEFAULT 100\","
            "\"field_name3\":{"
                "\"field_name4\":\"DOUBLE\","
                "\"field_name5\":{}"
        "}},"
        "\"SCHEMA_INDEXES\":[\"field_name1\"]}";
    const std::string SCHEMA_DEFINE_NOTNULL_DIFFER = "{\"SCHEMA_VERSION\":\"1.0\","
        "\"SCHEMA_MODE\":\"COMPATIBLE\","
        "\"SCHEMA_DEFINE\":{"
            "\"field_name1\":\"BOOL\","
            "\"field_name2\":\"INTEGER, DEFAULT 100\","
            "\"field_name3\":{"
                "\"field_name4\":\"STRING\","
                "\"field_name5\":{}"
        "}},"
        "\"SCHEMA_INDEXES\":[\"field_name1\"]}";
    const std::string SCHEMA_DEFINE_DEFAULT_DIFFER = "{\"SCHEMA_VERSION\":\"1.0\","
        "\"SCHEMA_MODE\":\"COMPATIBLE\","
        "\"SCHEMA_DEFINE\":{"
            "\"field_name1\":\"BOOL, NOT NULL\","
            "\"field_name2\":\"INTEGER, DEFAULT 88\","
            "\"field_name3\":{"
                "\"field_name4\":\"STRING\","
                "\"field_name5\":{}"
        "}},"
        "\"SCHEMA_INDEXES\":[\"field_name1\"]}";

    // Compare with VALID_SCHEMA_FULL_DEFINE
    const std::string VALUE_LESS_FIELD = "{\"field_name1\":true,"
        "\"field_name2\":{"
            "\"field_name3\":100,"
            "\"field_name8\":{"
                "\"field_name9\":200"
            "}"
        "}}";
    const std::string VALUE_MORE_FIELD = "{\"field_name1\":true,"
        "\"field_name2\":{"
            "\"field_name3\":100,"
            "\"field_name4\":8589934592,"
            "\"field_name5\":3.14,"
            "\"field_name6\":\"3.1415926\","
            "\"field_name7\":[true,1,\"inArray\"],"
            "\"field_name8\":{"
                "\"field_name9\":200"
            "},"
            "\"field_name10\":300"
        "}}";
    const std::string VALUE_TYPE_MISMATCH = "{\"field_name1\":true,"
        "\"field_name2\":{"
            "\"field_name3\":8589934592,"
            "\"field_name4\":100,"
            "\"field_name5\":3.14,"
            "\"field_name6\":\"3.1415926\","
            "\"field_name7\":[true,1,\"inArray\"],"
            "\"field_name8\":{"
                "\"field_name9\":200"
            "}"
        "}}";
    const std::string VALUE_NOT_NULL_VIOLATION = "{\"field_name1\":true,"
        "\"field_name2\":{"
            "\"field_name3\":null,"
            "\"field_name4\":8589934592,"
            "\"field_name5\":3.14,"
            "\"field_name6\":\"3.1415926\","
            "\"field_name7\":[true,1,\"inArray\"],"
            "\"field_name8\":{"
                "\"field_name9\":200"
            "}"
        "}}";
    const std::string VALUE_MATCH_STRICT_SCHEMA = "{\"field_name1\":true,"
        "\"field_name2\":{"
            "\"field_name3\":100,"
            "\"field_name4\":8589934592,"
            "\"field_name5\":3.14,"
            "\"field_name6\":\"3.1415926\","
            "\"field_name7\":[true,1,\"inArray\"],"
            "\"field_name8\":{"
                "\"field_name9\":200"
            "}"
        "}}";

    // For test lacking field.
    const std::string SCHEMA_FOR_TEST_NOTNULL_AND_DEFAULT = "{\"SCHEMA_VERSION\":\"1.0\","
        "\"SCHEMA_MODE\":\"COMPATIBLE\","
        "\"SCHEMA_DEFINE\":{"
            "\"no_notnull_no_default\":\"BOOL\","
            "\"level_0_nest_0\":{"
                "\"has_notnull_no_default\":\"INTEGER, NOT NULL\","
                "\"level_1_nest_0\":{"
                    "\"no_notnull_has_default\":\"LONG, DEFAULT 100\","
                    "\"has_notnull_has_default\":\"DOUBLE, NOT NULL, DEFAULT 3.14\","
                    "\"level_2_nest_0\":{"
                        "\"extra_0\":\"STRING, NOT NULL, DEFAULT '3.1415'\","
                        "\"extra_1\":\"DOUBLE\","
                        "\"extra_2\":[]"
                    "},"
                    "\"level_2_nest_1\":{"
                        "\"extra_3\":\"STRING\","
                        "\"extra_4\":{}"
                    "}"
                "}"
            "}"
        "}}";
    const std::string VALUE_NO_LACK_FIELD = "{"
        "\"no_notnull_no_default\":true,"
        "\"level_0_nest_0\":{"
            "\"has_notnull_no_default\":10010,"
            "\"level_1_nest_0\":{"
                "\"no_notnull_has_default\":10086,"
                "\"has_notnull_has_default\":1.38064,"
                "\"level_2_nest_0\":{"
                    "\"extra_0\":\"BLOOM\","
                    "\"extra_1\":2.71828,"
                    "\"extra_2\":[]"
                "},"
                "\"level_2_nest_1\":{"
                    "\"extra_3\":\"Prejudice\","
                    "\"extra_4\":{}"
                "}"
            "}"
        "}}";
    const std::string VALUE_LACK_LEVEL_0_NEST_0 = "{\"no_notnull_no_default\":true}";
    const std::string VALUE_LEVEL_0_NEST_0_NOT_OBJECT = "{\"no_notnull_no_default\":true,\"level_0_nest_0\":1}";
    const std::string VALUE_LACK_LEVEL_1_NEST_0 = "{"
        "\"no_notnull_no_default\":true,"
        "\"level_0_nest_0\":{"
            "\"has_notnull_no_default\":10010"
        "}}";

std::string SchemaSwitchMode(const std::string &oriSchemaStr)
{
    std::string resultSchemaStr = oriSchemaStr;
    auto iterForStrict = std::search(resultSchemaStr.begin(), resultSchemaStr.end(),
        KEYWORD_MODE_STRICT.begin(), KEYWORD_MODE_STRICT.end());
    auto iterForCompatible = std::search(resultSchemaStr.begin(), resultSchemaStr.end(),
        KEYWORD_MODE_COMPATIBLE.begin(), KEYWORD_MODE_COMPATIBLE.end());
    if (iterForStrict != resultSchemaStr.end()) {
        resultSchemaStr.replace(iterForStrict, iterForStrict + KEYWORD_MODE_STRICT.size(),
            KEYWORD_MODE_COMPATIBLE.begin(), KEYWORD_MODE_COMPATIBLE.end());
        return resultSchemaStr;
    }
    if (iterForCompatible != resultSchemaStr.end()) {
        resultSchemaStr.replace(iterForCompatible, iterForCompatible + KEYWORD_MODE_COMPATIBLE.size(),
            KEYWORD_MODE_STRICT.begin(), KEYWORD_MODE_STRICT.end());
        return resultSchemaStr;
    }
    return oriSchemaStr;
}
}

class DistributedDBSchemaObjectTest : public testing::Test {
public:
    static void SetUpTestCase(void) {};
    static void TearDownTestCase(void) {};
    void SetUp() {};
    void TearDown() {};
};

/**
 * @tc.name: Parse Valid Schema 001
 * @tc.desc: Parse Valid Schema
 * @tc.type: FUNC
 * @tc.require: AR000DR9K3
 * @tc.author: xiaozhenjian
 */
HWTEST_F(DistributedDBSchemaObjectTest, ParseValidSchema001, TestSize.Level0)
{
    /**
     * @tc.steps: step1. Parse valid schema with full define
     * @tc.expected: step1. Parse Success.
     */
    SchemaObject schema1;
    int stepOne = schema1.ParseFromSchemaString(VALID_SCHEMA_FULL_DEFINE);
    EXPECT_TRUE(stepOne == E_OK);

    /**
     * @tc.steps: step2. Parse valid schema with empty index
     * @tc.expected: step2. Parse Success.
     */
    SchemaObject schema2;
    int stepTwo = schema2.ParseFromSchemaString(VALID_SCHEMA_INDEX_EMPTY);
    EXPECT_TRUE(stepTwo == E_OK);

    /**
     * @tc.steps: step3. Parse valid schema with no index field
     * @tc.expected: step3. Parse Success.
     */
    SchemaObject schema3;
    int stepThree = schema3.ParseFromSchemaString(VALID_SCHEMA_NO_INDEX);
    EXPECT_TRUE(stepThree == E_OK);

    /**
     * @tc.steps: step4. Parse valid schema with prefix of suffix blank
     * @tc.expected: step4. Parse Success.
     */
    SchemaObject schema4;
    int stepFour = schema4.ParseFromSchemaString(VALID_SCHEMA_PRE_SUF_BLANK);
    EXPECT_TRUE(stepFour == E_OK);
}

/**
 * @tc.name: Parse Invalid Schema 001
 * @tc.desc: Parse Invalid Schema
 * @tc.type: FUNC
 * @tc.require: AR000DR9K3
 * @tc.author: xiaozhenjian
 */
HWTEST_F(DistributedDBSchemaObjectTest, ParseInvalidSchema001, TestSize.Level0)
{
    /**
     * @tc.steps: step1. Parse invalid schema which is not valid json
     * @tc.expected: step1. Parse Fail.
     */
    SchemaObject schema1;
    int stepOne = schema1.ParseFromSchemaString(INVALID_SCHEMA_INVALID_JSON);
    EXPECT_TRUE(stepOne != E_OK);

    /**
     * @tc.steps: step2. Parse invalid schema with less field in depth 0
     * @tc.expected: step2. Parse Fail.
     */
    SchemaObject schema2;
    int stepTwo = schema2.ParseFromSchemaString(INVALID_SCHEMA_LESS_META_FIELD);
    EXPECT_TRUE(stepTwo != E_OK);

    /**
     * @tc.steps: step3. Parse invalid schema with more field in depth 0
     * @tc.expected: step3. Parse Fail.
     */
    SchemaObject schema3;
    int stepThree = schema3.ParseFromSchemaString(INVALID_SCHEMA_MORE_META_FIELD);
    EXPECT_TRUE(stepThree != E_OK);

    /**
     * @tc.steps: step4. Parse invalid schema with wrong version
     * @tc.expected: step4. Parse Fail.
     */
    SchemaObject schema4;
    int stepFour = schema4.ParseFromSchemaString(INVALID_SCHEMA_WRONG_VERSION);
    EXPECT_TRUE(stepFour != E_OK);

    /**
     * @tc.steps: step5. Parse invalid schema with wrong mode
     * @tc.expected: step5. Parse Fail.
     */
    SchemaObject schema5;
    int stepFive = schema5.ParseFromSchemaString(INVALID_SCHEMA_WRONG_MODE);
    EXPECT_TRUE(stepFive != E_OK);
}

/**
 * @tc.name: Parse Invalid Schema 002
 * @tc.desc: Parse Invalid Schema
 * @tc.type: FUNC
 * @tc.require: AR000DR9K3
 * @tc.author: xiaozhenjian
 */
HWTEST_F(DistributedDBSchemaObjectTest, ParseInvalidSchema002, TestSize.Level0)
{
    /**
     * @tc.steps: step1. Parse invalid schema which is empty define
     * @tc.expected: step1. Parse Fail.
     */
    SchemaObject schema1;
    int stepOne = schema1.ParseFromSchemaString(INVALID_SCHEMA_DEFINE_EMPTY);
    EXPECT_TRUE(stepOne != E_OK);

    /**
     * @tc.steps: step2. Parse invalid schema with define nest too deep
     * @tc.expected: step2. Parse Fail.
     */
    SchemaObject schema2;
    int stepTwo = schema2.ParseFromSchemaString(INVALID_SCHEMA_DEFINE_NEST_TOO_DEEP);
    EXPECT_TRUE(stepTwo != E_OK);

    /**
     * @tc.steps: step3. Parse invalid schema with invalid fieldname in define
     * @tc.expected: step3. Parse Fail.
     */
    SchemaObject schema3;
    int stepThree = schema3.ParseFromSchemaString(INVALID_SCHEMA_DEFINE_INVALID_FIELD_NAME);
    EXPECT_TRUE(stepThree != E_OK);

    /**
     * @tc.steps: step4. Parse invalid schema with invalid field attribute in define
     * @tc.expected: step4. Parse Fail.
     */
    SchemaObject schema4;
    int stepFour = schema4.ParseFromSchemaString(INVALID_SCHEMA_DEFINE_INVALID_FIELD_ATTR);
    EXPECT_TRUE(stepFour != E_OK);

    /**
     * @tc.steps: step5. Parse invalid schema with not empty array in define
     * @tc.expected: step5. Parse Fail.
     */
    SchemaObject schema5;
    int stepFive = schema5.ParseFromSchemaString(INVALID_SCHEMA_DEFINE_INVALID_ARRAY_TYPE);
    EXPECT_TRUE(stepFive != E_OK);
}

/**
 * @tc.name: Parse Invalid Schema 003
 * @tc.desc: Parse Invalid Schema
 * @tc.type: FUNC
 * @tc.require: AR000DR9K3
 * @tc.author: xiaozhenjian
 */
HWTEST_F(DistributedDBSchemaObjectTest, ParseInvalidSchema003, TestSize.Level0)
{
    /**
     * @tc.steps: step1. Parse invalid schema with invalid array content
     * @tc.expected: step1. Parse Fail.
     */
    SchemaObject schema1;
    int stepOne = schema1.ParseFromSchemaString(INVALID_SCHEMA_INDEX_INVALID);
    EXPECT_TRUE(stepOne != E_OK);

    /**
     * @tc.steps: step2. Parse invalid schema with invalid path
     * @tc.expected: step2. Parse Fail.
     */
    SchemaObject schema2;
    int stepTwo = schema2.ParseFromSchemaString(INVALID_SCHEMA_INDEX_PATH_INVALID);
    EXPECT_TRUE(stepTwo != E_OK);

    /**
     * @tc.steps: step3. Parse invalid schema with path not exist
     * @tc.expected: step3. Parse Fail.
     */
    SchemaObject schema3;
    int stepThree = schema3.ParseFromSchemaString(INVALID_SCHEMA_INDEX_PATH_NOT_EXIST);
    EXPECT_TRUE(stepThree != E_OK);

    /**
     * @tc.steps: step4. Parse invalid schema with path not indexable
     * @tc.expected: step4. Parse Fail.
     */
    SchemaObject schema4;
    int stepFour = schema4.ParseFromSchemaString(INVALID_SCHEMA_INDEX_PATH_NOT_INDEXABLE);
    EXPECT_TRUE(stepFour != E_OK);

    /**
     * @tc.steps: step5. Parse invalid schema with duplicate
     * @tc.expected: step5. Parse Fail.
     */
    SchemaObject schema5;
    int stepFive = schema5.ParseFromSchemaString(INVALID_SCHEMA_INDEX_PATH_DUPLICATE);
    EXPECT_TRUE(stepFive != E_OK);
}

/**
 * @tc.name: Compare Equal Exactly 001
 * @tc.desc: Compare Equal Exactly
 * @tc.type: FUNC
 * @tc.require: AR000DR9K4
 * @tc.author: xiaozhenjian
 */
HWTEST_F(DistributedDBSchemaObjectTest, CompareEqualExactly001, TestSize.Level0)
{
    SchemaObject schemaOri;
    int errCode = schemaOri.ParseFromSchemaString(VALID_SCHEMA_FULL_DEFINE);
    EXPECT_TRUE(errCode == E_OK);

    /**
     * @tc.steps: step1. Compare two same schema with full define
     * @tc.expected: step1. Equal exactly.
     */
    int stepOne = schemaOri.CompareAgainstSchemaString(VALID_SCHEMA_FULL_DEFINE);
    EXPECT_TRUE(stepOne == -E_SCHEMA_EQUAL_EXACTLY);
}

/**
 * @tc.name: Compare Unequal Compatible 001
 * @tc.desc: Compare Unequal Compatible
 * @tc.type: FUNC
 * @tc.require: AR000DR9K4
 * @tc.author: xiaozhenjian
 */
HWTEST_F(DistributedDBSchemaObjectTest, CompareUnequalCompatible001, TestSize.Level0)
{
    SchemaObject compatibleSchema;
    int errCode = compatibleSchema.ParseFromSchemaString(SCHEMA_COMPARE_BASELINE);
    EXPECT_EQ(errCode, E_OK);

    /**
     * @tc.steps: step1. new schema index more
     * @tc.expected: step1. E_SCHEMA_UNEQUAL_COMPATIBLE.
     */
    errCode = compatibleSchema.CompareAgainstSchemaString(SCHEMA_INDEX_MORE);
    EXPECT_EQ(errCode, -E_SCHEMA_UNEQUAL_COMPATIBLE);

    /**
     * @tc.steps: step2. new schema index less
     * @tc.expected: step2. E_SCHEMA_UNEQUAL_COMPATIBLE.
     */
    errCode = compatibleSchema.CompareAgainstSchemaString(SCHEMA_INDEX_LESS);
    EXPECT_EQ(errCode, -E_SCHEMA_UNEQUAL_COMPATIBLE);

    /**
     * @tc.steps: step3. new schema index change
     * @tc.expected: step3. E_SCHEMA_UNEQUAL_COMPATIBLE.
     */
    errCode = compatibleSchema.CompareAgainstSchemaString(SCHEMA_INDEX_CHANGE);
    EXPECT_EQ(errCode, -E_SCHEMA_UNEQUAL_COMPATIBLE);
}

/**
 * @tc.name: Compare Unequal Compatible Upgrade 001
 * @tc.desc: Compare Unequal Compatible Upgrade
 * @tc.type: FUNC
 * @tc.require: AR000DR9K4
 * @tc.author: xiaozhenjian
 */
HWTEST_F(DistributedDBSchemaObjectTest, CompareUnequalCompatibleUpgrade001, TestSize.Level0)
{
    SchemaObject compatibleSchema;
    int errCode = compatibleSchema.ParseFromSchemaString(SCHEMA_COMPARE_BASELINE);
    EXPECT_EQ(errCode, E_OK);

    /**
     * @tc.steps: step1. compatible new schema more field define
     * @tc.expected: step1. E_SCHEMA_UNEQUAL_COMPATIBLE_UPGRADE.
     */
    errCode = compatibleSchema.CompareAgainstSchemaString(SCHEMA_DEFINE_MORE_FIELD);
    EXPECT_EQ(errCode, -E_SCHEMA_UNEQUAL_COMPATIBLE_UPGRADE);

    /**
     * @tc.steps: step2. compatible new schema more field with not null and default
     * @tc.expected: step2. E_SCHEMA_UNEQUAL_COMPATIBLE_UPGRADE.
     */
    errCode = compatibleSchema.CompareAgainstSchemaString(SCHEMA_DEFINE_MORE_FIELD_NOTNULL_PERMIT);
    EXPECT_EQ(errCode, -E_SCHEMA_UNEQUAL_COMPATIBLE_UPGRADE);

    /**
     * @tc.steps: step3. compatible new schema more field and more index
     * @tc.expected: step3. E_SCHEMA_UNEQUAL_COMPATIBLE_UPGRADE.
     */
    errCode = compatibleSchema.CompareAgainstSchemaString(SCHEMA_DEFINE_MORE_FIELD_MORE_INDEX);
    EXPECT_EQ(errCode, -E_SCHEMA_UNEQUAL_COMPATIBLE_UPGRADE);
}

/**
 * @tc.name: Compare Unequal Incompatible 001
 * @tc.desc: Compare Unequal Incompatible
 * @tc.type: FUNC
 * @tc.require: AR000DR9K4
 * @tc.author: xiaozhenjian
 */
HWTEST_F(DistributedDBSchemaObjectTest, CompareUnequalIncompatible001, TestSize.Level0)
{
    SchemaObject strictSchema;
    int errCode = strictSchema.ParseFromSchemaString(SchemaSwitchMode(SCHEMA_COMPARE_BASELINE));
    EXPECT_EQ(errCode, E_OK);
    SchemaObject compatibleSchema;
    errCode = compatibleSchema.ParseFromSchemaString(SCHEMA_COMPARE_BASELINE);
    EXPECT_EQ(errCode, E_OK);

    /**
     * @tc.steps: step1. strict new schema more field define
     * @tc.expected: step1. E_SCHEMA_UNEQUAL_INCOMPATIBLE.
     */
    errCode = strictSchema.CompareAgainstSchemaString(SchemaSwitchMode(SCHEMA_DEFINE_MORE_FIELD));
    EXPECT_EQ(errCode, -E_SCHEMA_UNEQUAL_INCOMPATIBLE);

    /**
     * @tc.steps: step2. compatible new schema more field but not null
     * @tc.expected: step2. E_SCHEMA_UNEQUAL_INCOMPATIBLE.
     */
    errCode = compatibleSchema.CompareAgainstSchemaString(SCHEMA_DEFINE_MORE_FIELD_NOTNULL_FORBID);
    EXPECT_EQ(errCode, -E_SCHEMA_UNEQUAL_INCOMPATIBLE);

    /**
     * @tc.steps: step3. new schema less field
     * @tc.expected: step3. E_SCHEMA_UNEQUAL_INCOMPATIBLE.
     */
    errCode = compatibleSchema.CompareAgainstSchemaString(SCHEMA_DEFINE_LESS_FIELD);
    EXPECT_EQ(errCode, -E_SCHEMA_UNEQUAL_INCOMPATIBLE);
    errCode = strictSchema.CompareAgainstSchemaString(SchemaSwitchMode(SCHEMA_DEFINE_LESS_FIELD));
    EXPECT_EQ(errCode, -E_SCHEMA_UNEQUAL_INCOMPATIBLE);

    /**
     * @tc.steps: step4. new schema skipsize differ
     * @tc.expected: step4. E_SCHEMA_UNEQUAL_INCOMPATIBLE.
     */
    errCode = compatibleSchema.CompareAgainstSchemaString(SCHEMA_SKIPSIZE_DIFFER);
    EXPECT_EQ(errCode, -E_SCHEMA_UNEQUAL_INCOMPATIBLE);

    /**
     * @tc.steps: step5. new schema type differ
     * @tc.expected: step5. E_SCHEMA_UNEQUAL_INCOMPATIBLE.
     */
    errCode = compatibleSchema.CompareAgainstSchemaString(SCHEMA_DEFINE_TYPE_DIFFER);
    EXPECT_EQ(errCode, -E_SCHEMA_UNEQUAL_INCOMPATIBLE);

    /**
     * @tc.steps: step6. new schema notnull differ
     * @tc.expected: step6. E_SCHEMA_UNEQUAL_INCOMPATIBLE.
     */
    errCode = compatibleSchema.CompareAgainstSchemaString(SCHEMA_DEFINE_NOTNULL_DIFFER);
    EXPECT_EQ(errCode, -E_SCHEMA_UNEQUAL_INCOMPATIBLE);

    /**
     * @tc.steps: step7. new schema default differ
     * @tc.expected: step7. E_SCHEMA_UNEQUAL_INCOMPATIBLE.
     */
    errCode = compatibleSchema.CompareAgainstSchemaString(SCHEMA_DEFINE_DEFAULT_DIFFER);
    EXPECT_EQ(errCode, -E_SCHEMA_UNEQUAL_INCOMPATIBLE);

    /**
     * @tc.steps: step8. new schema mode differ
     * @tc.expected: step8. E_SCHEMA_UNEQUAL_INCOMPATIBLE.
     */
    errCode = compatibleSchema.CompareAgainstSchemaString(SchemaSwitchMode(SCHEMA_COMPARE_BASELINE));
    EXPECT_EQ(errCode, -E_SCHEMA_UNEQUAL_INCOMPATIBLE);
}

/**
 * @tc.name: Check Value 001
 * @tc.desc: Check value both in strict and compatible mode
 * @tc.type: FUNC
 * @tc.require: AR000DR9K5
 * @tc.author: xiaozhenjian
 */
HWTEST_F(DistributedDBSchemaObjectTest, CheckValue001, TestSize.Level0)
{
    SchemaObject schemaStrict;
    int errCode = schemaStrict.ParseFromSchemaString(VALID_SCHEMA_FULL_DEFINE);
    EXPECT_TRUE(errCode == E_OK);

    /**
     * @tc.steps: step1. value has less field in strict mode
     * @tc.expected: step1. E_VALUE_MATCH_AMENDED.
     */
    ValueObject value1;
    errCode = value1.Parse(VALUE_LESS_FIELD);
    EXPECT_TRUE(errCode == E_OK);
    int stepOne = schemaStrict.CheckValueAndAmendIfNeed(ValueSource::FROM_LOCAL, value1);
    EXPECT_TRUE(stepOne == -E_VALUE_MATCH_AMENDED);

    /**
     * @tc.steps: step2. value has more field in strict mode
     * @tc.expected: step2. E_VALUE_MISMATCH_FEILD_COUNT.
     */
    ValueObject value2;
    errCode = value2.Parse(VALUE_MORE_FIELD);
    EXPECT_TRUE(errCode == E_OK);
    int stepTwo = schemaStrict.CheckValueAndAmendIfNeed(ValueSource::FROM_LOCAL, value2);
    EXPECT_TRUE(stepTwo == -E_VALUE_MISMATCH_FEILD_COUNT);

    /**
     * @tc.steps: step3. value type mismatch
     * @tc.expected: step3. E_VALUE_MISMATCH_FEILD_TYPE.
     */
    ValueObject value3;
    errCode = value3.Parse(VALUE_TYPE_MISMATCH);
    EXPECT_TRUE(errCode == E_OK);
    int stepThree = schemaStrict.CheckValueAndAmendIfNeed(ValueSource::FROM_LOCAL, value3);
    EXPECT_TRUE(stepThree == -E_VALUE_MISMATCH_FEILD_TYPE);

    /**
     * @tc.steps: step4. value not null violation
     * @tc.expected: step4. E_VALUE_MISMATCH_CONSTRAINT.
     */
    ValueObject value4;
    errCode = value4.Parse(VALUE_NOT_NULL_VIOLATION);
    EXPECT_TRUE(errCode == E_OK);
    int stepFour = schemaStrict.CheckValueAndAmendIfNeed(ValueSource::FROM_LOCAL, value4);
    EXPECT_TRUE(stepFour == -E_VALUE_MISMATCH_CONSTRAINT);

    /**
     * @tc.steps: step5. value exactly match strict mode
     * @tc.expected: step5. E_VALUE_MATCH.
     */
    ValueObject value5;
    errCode = value5.Parse(VALUE_MATCH_STRICT_SCHEMA);
    EXPECT_TRUE(errCode == E_OK);
    int stepFive = schemaStrict.CheckValueAndAmendIfNeed(ValueSource::FROM_LOCAL, value5);
    EXPECT_TRUE(stepFive == -E_VALUE_MATCH);

    /**
     * @tc.steps: step6. value has more field in compatible mode
     * @tc.expected: step6. E_VALUE_MATCH.
     */
    std::string compatibleSchemaString = SchemaSwitchMode(VALID_SCHEMA_FULL_DEFINE);
    SchemaObject schemaCompatible;
    errCode = schemaCompatible.ParseFromSchemaString(compatibleSchemaString);
    EXPECT_TRUE(errCode == E_OK);

    ValueObject value6;
    std::vector<uint8_t> moreFieldValueVector(VALUE_MORE_FIELD.begin(), VALUE_MORE_FIELD.end());
    errCode = value6.Parse(moreFieldValueVector);
    EXPECT_TRUE(errCode == E_OK);
    int stepSix = schemaCompatible.CheckValueAndAmendIfNeed(ValueSource::FROM_LOCAL, value6);
    EXPECT_TRUE(stepSix == -E_VALUE_MATCH);
}

/**
 * @tc.name: Check Value 002
 * @tc.desc: Check value that has offset
 * @tc.type: FUNC
 * @tc.require: AR000DR9K5
 * @tc.author: xiaozhenjian
 */
HWTEST_F(DistributedDBSchemaObjectTest, CheckValue002, TestSize.Level0)
{
    SchemaObject schemaStrict;
    int errCode = schemaStrict.ParseFromSchemaString(VALID_SCHEMA_FULL_DEFINE);
    EXPECT_TRUE(errCode == E_OK);

    /**
     * @tc.steps: step1. value has less field in strict mode
     * @tc.expected: step1. E_VALUE_MATCH and data before offset not change.
     */
    std::string beforeOffset = "BOM_CONTENT:";
    std::string strValue = beforeOffset + VALUE_MATCH_STRICT_SCHEMA;
    vector<uint8_t> vecValue(strValue.begin(), strValue.end());

    ValueObject value1;
    errCode = value1.Parse(vecValue.data(), vecValue.data() + vecValue.size(), beforeOffset.size());
    EXPECT_TRUE(errCode == E_OK);

    int stepOne = schemaStrict.CheckValueAndAmendIfNeed(ValueSource::FROM_LOCAL, value1);
    EXPECT_TRUE(stepOne == -E_VALUE_MATCH);

    std::string valueToString = value1.ToString();
    std::string valueBeforeOffset = valueToString.substr(0, beforeOffset.size());
    EXPECT_TRUE(valueBeforeOffset == beforeOffset);
}

/**
 * @tc.name: Value Edit 001
 * @tc.desc: Edit the value in right way
 * @tc.type: FUNC
 * @tc.require: AR000DR9K5
 * @tc.author: xiaozhenjian
 */
HWTEST_F(DistributedDBSchemaObjectTest, ValueEdit001, TestSize.Level0)
{
    /**
     * @tc.steps: step1. Insert value to ValueObject in different depth
     * @tc.expected: step1. Check insert successful
     */
    ValueObject testObject;
    FieldValue val;

    val.stringValue = "stringValue";
    int errCode = testObject.InsertField(FieldPath{"L1F1", "L2F1", "L3F1", "L4F1"}, FieldType::LEAF_FIELD_STRING, val);
    EXPECT_TRUE(errCode == E_OK);
    val.doubleValue = 1.1; // 1.1 for test
    errCode = testObject.InsertField(FieldPath{"L1F1", "L2F1", "L3F1", "L4F2"}, FieldType::LEAF_FIELD_DOUBLE, val);
    EXPECT_TRUE(errCode == E_OK);
    val.longValue = INT64_MAX;
    errCode = testObject.InsertField(FieldPath{"L1F1", "L2F1", "L3F2"}, FieldType::LEAF_FIELD_LONG, val);
    EXPECT_TRUE(errCode == E_OK);
    errCode = testObject.InsertField(FieldPath{"L1F1", "L2F2"}, FieldType::LEAF_FIELD_OBJECT, val);
    EXPECT_TRUE(errCode == E_OK);
    val.integerValue = INT32_MIN;
    errCode = testObject.InsertField(FieldPath{"L1F1", "L2F2", "L3F3"}, FieldType::LEAF_FIELD_INTEGER, val);
    EXPECT_TRUE(errCode == E_OK);
    val.boolValue = true;
    errCode = testObject.InsertField(FieldPath{"L1F1", "L2F2", "L3F4"}, FieldType::LEAF_FIELD_BOOL, val);
    EXPECT_TRUE(errCode == E_OK);
    errCode = testObject.InsertField(FieldPath{"L1F2"}, FieldType::LEAF_FIELD_NULL, val);
    EXPECT_TRUE(errCode == E_OK);

    /**
     * @tc.steps: step2. Delete value in ValueObject
     * @tc.expected: step2. Check delete successful
     */
    errCode = testObject.DeleteField(FieldPath{"L1F1", "L2F1", "L3F1", "L4F1"});
    EXPECT_TRUE(errCode == E_OK);
    errCode = testObject.DeleteField(FieldPath{"L1F1", "L2F1", "L3F1"});
    EXPECT_TRUE(errCode == E_OK);
    errCode = testObject.DeleteField(FieldPath{"L1F1"});
    EXPECT_TRUE(errCode == E_OK);
}

/**
 * @tc.name: Value Edit 002
 * @tc.desc: Edit the value in wrong way
 * @tc.type: FUNC
 * @tc.require: AR000DR9K5
 * @tc.author: xiaozhenjian
 */
HWTEST_F(DistributedDBSchemaObjectTest, ValueEdit002, TestSize.Level0)
{
    /**
     * @tc.steps: step1. Insert value to ValueObject in different depth
     * @tc.expected: step1. Check insert not successful
     */
    ValueObject testObject;
    FieldValue val;

    val.stringValue = "stringValue";
    int errCode = testObject.InsertField(FieldPath{"L1F1", "L2F1", "L3F1"}, FieldType::LEAF_FIELD_STRING, val);
    EXPECT_TRUE(errCode == E_OK);
    val.doubleValue = 1.1; // 1.1 for test
    errCode = testObject.InsertField(FieldPath{"L1F1", "L2F1", "L3F1"}, FieldType::LEAF_FIELD_DOUBLE, val);
    EXPECT_TRUE(errCode != E_OK);
    val.longValue = INT64_MAX;
    errCode = testObject.InsertField(FieldPath{"L1F1", "L2F1", "L3F1", "L4F1"}, FieldType::LEAF_FIELD_LONG, val);
    EXPECT_TRUE(errCode != E_OK);

    /**
     * @tc.steps: step2. Delete value in ValueObject
     * @tc.expected: step2. Check delete not successful
     */
    errCode = testObject.DeleteField(FieldPath{"L1F1", "L2F1", "L3F1", "L4F1"});
    EXPECT_TRUE(errCode != E_OK);
}

namespace {
void CheckValueLackField(const SchemaObject &schema, const std::string &oriValue, const std::string &lackField,
    int expectErrCode, ValueObject &externalValueObject)
{
    std::string valueStr = oriValue;
    auto startIter = std::search(valueStr.begin(), valueStr.end(), lackField.begin(), lackField.end());
    valueStr.erase(startIter, startIter + lackField.size());
    int errCode = externalValueObject.Parse(valueStr);
    EXPECT_EQ(errCode, E_OK);
    errCode = schema.CheckValueAndAmendIfNeed(ValueSource::FROM_LOCAL, externalValueObject);
    EXPECT_EQ(errCode, expectErrCode);
}

void CheckValueLackField(const SchemaObject &schema, const std::string &oriValue, const std::string &lackField,
    int expectErrCode)
{
    ValueObject valueObj;
    CheckValueLackField(schema, oriValue, lackField, expectErrCode, valueObj);
}
}

/**
 * @tc.name: Value LackField 001
 * @tc.desc: check the value which lack field
 * @tc.type: FUNC
 * @tc.require: AR000DR9K5
 * @tc.author: xiaozhenjian
 */
HWTEST_F(DistributedDBSchemaObjectTest, ValueLackField001, TestSize.Level0)
{
    SchemaObject schema;
    int errCode = schema.ParseFromSchemaString(SCHEMA_FOR_TEST_NOTNULL_AND_DEFAULT);
    EXPECT_EQ(errCode, E_OK);

    /**
     * @tc.steps: step1. check value lack no field
     * @tc.expected: step1. E_VALUE_MATCH
     */
    CheckValueLackField(schema, VALUE_NO_LACK_FIELD, "", -E_VALUE_MATCH);

    /**
     * @tc.steps: step2. check value lack field on no_notnull_no_default
     * @tc.expected: step2. E_VALUE_MATCH
     */
    CheckValueLackField(schema, VALUE_NO_LACK_FIELD, "\"no_notnull_no_default\":true,", -E_VALUE_MATCH);

    /**
     * @tc.steps: step3. check value lack field on has_notnull_no_default
     * @tc.expected: step3. E_VALUE_MISMATCH_CONSTRAINT
     */
    CheckValueLackField(schema, VALUE_NO_LACK_FIELD, "\"has_notnull_no_default\":10010,",
        -E_VALUE_MISMATCH_CONSTRAINT);

    /**
     * @tc.steps: step4. check value lack field on no_notnull_has_default
     * @tc.expected: step4. E_VALUE_MATCH_AMENDED
     */
    CheckValueLackField(schema, VALUE_NO_LACK_FIELD, "\"no_notnull_has_default\":10086,",
        -E_VALUE_MATCH_AMENDED);

    /**
     * @tc.steps: step5. check value lack field on has_notnull_has_default
     * @tc.expected: step5. E_VALUE_MATCH_AMENDED
     */
    CheckValueLackField(schema, VALUE_NO_LACK_FIELD, "\"has_notnull_has_default\":1.38064,",
        -E_VALUE_MATCH_AMENDED);

    /**
     * @tc.steps: step6. check value lack entire level_0_nest_0
     * @tc.expected: step6. E_VALUE_MISMATCH_CONSTRAINT
     */
    CheckValueLackField(schema, VALUE_LACK_LEVEL_0_NEST_0, "", -E_VALUE_MISMATCH_CONSTRAINT);

    /**
     * @tc.steps: step7. check value level_0_nest_0 not json_object
     * @tc.expected: step7. E_VALUE_MISMATCH_FEILD_TYPE
     */
    CheckValueLackField(schema, VALUE_LEVEL_0_NEST_0_NOT_OBJECT, "", -E_VALUE_MISMATCH_FEILD_TYPE);
}

/**
 * @tc.name: Value LackField 002
 * @tc.desc: check the value which lack field
 * @tc.type: FUNC
 * @tc.require: AR000DR9K5
 * @tc.author: xiaozhenjian
 */
HWTEST_F(DistributedDBSchemaObjectTest, ValueLackField002, TestSize.Level0)
{
    SchemaObject schema;
    int errCode = schema.ParseFromSchemaString(SCHEMA_FOR_TEST_NOTNULL_AND_DEFAULT);
    EXPECT_EQ(errCode, E_OK);

    /**
     * @tc.steps: step1. check value lack entire level_1_nest_0
     * @tc.expected: step1. E_VALUE_MATCH_AMENDED
     */
    ValueObject val;
    CheckValueLackField(schema, VALUE_LACK_LEVEL_1_NEST_0, "", -E_VALUE_MATCH_AMENDED, val);
    // Check Field Existence or not
    EXPECT_EQ(val.IsFieldPathExist(FieldPath{"level_0_nest_0", "level_1_nest_0"}), true);
    EXPECT_EQ(val.IsFieldPathExist(FieldPath{"level_0_nest_0", "level_1_nest_0", "no_notnull_has_default"}), true);
    EXPECT_EQ(val.IsFieldPathExist(FieldPath{"level_0_nest_0", "level_1_nest_0", "has_notnull_has_default"}), true);
    EXPECT_EQ(val.IsFieldPathExist(FieldPath{"level_0_nest_0", "level_1_nest_0", "level_2_nest_0"}), true);
    EXPECT_EQ(val.IsFieldPathExist(FieldPath{"level_0_nest_0", "level_1_nest_0", "level_2_nest_0", "extra_0"}), true);
    EXPECT_EQ(val.IsFieldPathExist(FieldPath{"level_0_nest_0", "level_1_nest_0", "level_2_nest_0", "extra_1"}), false);
    EXPECT_EQ(val.IsFieldPathExist(FieldPath{"level_0_nest_0", "level_1_nest_0", "level_2_nest_0", "extra_2"}), false);
    EXPECT_EQ(val.IsFieldPathExist(FieldPath{"level_0_nest_0", "level_1_nest_0", "level_2_nest_1"}), false);
    // Check Field value
    FieldValue theValue;
    EXPECT_EQ(val.GetFieldValueByFieldPath(FieldPath{"level_0_nest_0", "level_1_nest_0", "no_notnull_has_default"},
        theValue), E_OK);
    EXPECT_EQ(theValue.integerValue, 100);
    EXPECT_EQ(val.GetFieldValueByFieldPath(FieldPath{"level_0_nest_0", "level_1_nest_0", "has_notnull_has_default"},
        theValue), E_OK);
    EXPECT_LT(std::abs(theValue.doubleValue - 3.14), 0.1);
    EXPECT_EQ(val.GetFieldValueByFieldPath(FieldPath{"level_0_nest_0", "level_1_nest_0", "level_2_nest_0", "extra_0"},
        theValue), E_OK);
    EXPECT_EQ(theValue.stringValue == std::string("3.1415"), true);
}
#endif