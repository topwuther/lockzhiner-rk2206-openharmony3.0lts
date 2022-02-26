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
#include <algorithm>
#include "db_errno.h"
#include "json_object.h"

using namespace std;
using namespace testing::ext;
using namespace DistributedDB;

namespace {
    const int MAX_DEPTH_FOR_TEST = 10;
    const int STRING1_DEPTH = 12;
    const int STRING3_DEPTH = 6;

    // nest depth = 12 and valid.
    const string JSON_STRING1 = "{\"#14\":[[{\"#11\":{\"#8\":[{\"#5\":[[{\"#2\":[{\"#0\":\"value_\"},\"value_\"],"
        "\"#3\":\"value_\"},\"value_\"],\"value_\"],\"#6\":\"value_\"},\"value_\"],\"#9\":\"value_\"},"
        "\"#12\":\"value_\"},\"value_\"],\"value_\"],\"#15\":{\"#18\":{\"#16\":\"value_\"},\"#19\":\"value_\"}}";

    // nest depth = 12 and invalid happens in nest depth = 2.
    const string JSON_STRING2 = "{\"#17\":[\"just for mistake pls.[{\"#14\":[[{\"#11\":{\"#8\":{\"#5\":[{\"#2\":"
        "{\"#0\":\"value_\"},\"#3\":\"value_\"},\"value_\"],\"#6\":\"value_\"},\"#9\":\"value_\"},"
        "\"#12\":\"value_\"},\"value_\"],\"value_\"],\"#15\":\"value_\"},\"value_\"],\"value_\"],"
        "\"#18\":{\"#21\":{\"#19\":\"value_\"},\"#22\":\"value_\"}}";

    // nest depth = 6 and valid.
    const string JSON_STRING3 = "{\"#5\":[{\"#2\":[[{\"#0\":\"value_\"},\"value_\"],\"value_\"],\"#3\":\"value_\"},"
        "\"value_\"],\"#6\":{\"#7\":\"value_\",\"#8\":\"value_\"}}";

    // nest depth = 6 and invalid happens in nest depth = 3.
    const string JSON_STRING4 = "{\"#6\":[{\"#3\":\"just for mistake pls.[{\"#0\":[\"value_\"],\"#1\":\"value_\"},"
        "\"value_\"],\"#4\":\"value_\"},\"value_\"],\"#7\":{\"#8\":\"value_\",\"#9\":\"value_\"}}";

    // nest depth = 15 and invalid happens in nest depth = 11.
    const string JSON_STRING5 = "{\"#35\":[{\"#29\":{\"#23\":{\"#17\":{\"#11\":{\"#8\":[{\"#5\":[{\"#2\":"
        "\"just for mistake pls.[[[{\"#0\":\"value_\"},\"value_\"],\"value_\"],\"value_\"],\"#3\":\"value_\"},"
        "\"value_\"],\"#6\":\"value_\"},\"value_\"],\"#9\":\"value_\"},\"#12\":{\"#13\":\"value_\","
        "\"#14\":\"value_\"}},\"#18\":{\"#19\":\"value_\",\"#20\":\"value_\"}},\"#24\":{\"#25\":\"value_\","
        "\"#26\":\"value_\"}},\"#30\":{\"#31\":\"value_\",\"#32\":\"value_\"}},\"value_\"],\"#36\":"
        "{\"#37\":[\"value_\"],\"#38\":\"value_\"}}";

    uint32_t g_oriMaxNestDepth = 0;
}

class DistributedDBJsonPrecheckUnitTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp() {};
    void TearDown() {};
};

void DistributedDBJsonPrecheckUnitTest::SetUpTestCase(void)
{
    /**
     * @tc.setup: Specifies a maximum nesting depth of 10.
     */
    g_oriMaxNestDepth = JsonObject::SetMaxNestDepth(MAX_DEPTH_FOR_TEST);
}

void DistributedDBJsonPrecheckUnitTest::TearDownTestCase(void)
{
    /**
     * @tc.teardown: Reset nesting depth to origin value.
     */
    JsonObject::SetMaxNestDepth(g_oriMaxNestDepth);
}

/**
 * @tc.name: Precheck Valid String 001
 * @tc.desc: json string is legal
 * @tc.type: FUNC
 * @tc.require: AR000DR9K3
 * @tc.author: yiguang
 */
HWTEST_F(DistributedDBJsonPrecheckUnitTest, ParseValidString001, TestSize.Level0)
{
    /**
     * @tc.steps: step1. Check legal json string with nesting depth of 12.
     * @tc.expected: step1. return value = 12.
     */
    int stepOne = JsonObject::CalculateNestDepth(JSON_STRING1);
    EXPECT_TRUE(stepOne == STRING1_DEPTH);

    /**
     * @tc.steps: step2. Parsing of legal json string with nesting depth greater than 10 failed.
     * @tc.expected: step2. Parsing result failed.
     */
    JsonObject tempObj;
    int stepTwo = tempObj.Parse(JSON_STRING1);
    EXPECT_TRUE(stepTwo != E_OK);
}

/**
 * @tc.name: Precheck Valid String 002
 * @tc.desc: json string is legal
 * @tc.type: FUNC
 * @tc.require: AR000DR9K3
 * @tc.author: yiguang
 */
HWTEST_F(DistributedDBJsonPrecheckUnitTest, ParseValidString002, TestSize.Level0)
{
    /**
     * @tc.steps: step1. Check legal json string with nesting depth of 6.
     * @tc.expected: step1. return value = 6.
     */
    int stepOne = JsonObject::CalculateNestDepth(JSON_STRING3);
    EXPECT_TRUE(stepOne == STRING3_DEPTH);

    /**
     * @tc.steps: step2. Parsing of legal json string with nesting depth less than 10 success.
     * @tc.expected: step2. Parsing result success.
     */
    JsonObject tempObj;
    int stepTwo = tempObj.Parse(JSON_STRING3);
    EXPECT_TRUE(stepTwo == E_OK);
}

/**
 * @tc.name: Precheck invalid String 001
 * @tc.desc: The json string has been detected illegal before exceeding the specified nesting depth.
 * @tc.type: FUNC
 * @tc.require: AR000DR9K3
 * @tc.author: yiguang
 */
HWTEST_F(DistributedDBJsonPrecheckUnitTest, ParseInvalidString001, TestSize.Level0)
{
    /**
     * @tc.steps: step1. Parsing of illegal json string with nesting depth greater than 10 success.
     * @tc.expected: step1. Parsing result failed.
     */
    JsonObject tempObj;
    int stepOne = tempObj.Parse(JSON_STRING2);
    EXPECT_TRUE(stepOne != E_OK);
}

/**
 * @tc.name: Precheck invalid String 002
 * @tc.desc: The json string has been detected illegal before exceeding the specified nesting depth.
 * @tc.type: FUNC
 * @tc.require: AR000DR9K3
 * @tc.author: yiguang
 */
HWTEST_F(DistributedDBJsonPrecheckUnitTest, ParseInvalidString002, TestSize.Level0)
{
    /**
     * @tc.steps: step1. Parsing of illegal json string with nesting depth less than 10 success.
     * @tc.expected: step1. Parsing result failed.
     */
    JsonObject tempObj;
    int stepOne = tempObj.Parse(JSON_STRING4);
    EXPECT_TRUE(stepOne != E_OK);
}

/**
 * @tc.name: Precheck invalid String 003
 * @tc.desc: The json string has been detected illegal before exceeding the specified nesting depth.
 * @tc.type: FUNC
 * @tc.require: AR000DR9K3
 * @tc.author: yiguang
 */
HWTEST_F(DistributedDBJsonPrecheckUnitTest, ParseInvalidString003, TestSize.Level0)
{
    /**
     * @tc.steps: step1. Detect illegal json string with nesting depth greater than 10.
     * @tc.expected: step1. return value > 10.
     */
    int stepOne = JsonObject::CalculateNestDepth(JSON_STRING5);
    EXPECT_TRUE(stepOne > MAX_DEPTH_FOR_TEST);

    /**
     * @tc.steps: step2. Parsing of illegal json string with nesting depth greater than 10 success.
     * @tc.expected: step2. Parsing result failed.
     */
    JsonObject tempObj;
    int stepTwo = tempObj.Parse(JSON_STRING5);
    EXPECT_TRUE(stepTwo != E_OK);
}
#endif