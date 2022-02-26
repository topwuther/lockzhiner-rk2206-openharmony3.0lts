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
#include <cstdint>
#include <vector>
#include "app_types.h"
using namespace testing::ext;
using namespace OHOS::AppDistributedKv;

class AppBlobTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
};

void AppBlobTest::SetUpTestCase(void)
{}

void AppBlobTest::TearDownTestCase(void)
{}

void AppBlobTest::SetUp(void)
{}

void AppBlobTest::TearDown(void)
{}

/**
  * @tc.name: AppBlobSize001
  * @tc.desc: Construct a Blob and check its size.
  * @tc.type: FUNC
  * @tc.require: AR000CCPOL
  * @tc.author: liqiao
  */
HWTEST_F(AppBlobTest, AppBlobSize001, TestSize.Level0)
{
    AppBlob blob1;
    EXPECT_EQ(blob1.Size(), (size_t)0);
    AppBlob blob2 = "1234567890";
    EXPECT_EQ(blob2.Size(), (size_t)10);
    AppBlob blob3("12345");
    EXPECT_EQ(blob3.Size(), (size_t)5);
    std::string strTmp = "123";
    const char *chr = strTmp.c_str();
    AppBlob blob4(chr);
    EXPECT_EQ(blob4.Size(), (size_t)3);
    std::vector<uint8_t> vec = {'1', '2', '3', '4'};
    AppBlob blob5(vec);
    EXPECT_EQ(blob5.Size(), (size_t)4);
    const char *chr1 = strTmp.c_str();
    AppBlob blob6(chr1, strlen(chr1));
    EXPECT_EQ(blob6.Size(), (size_t)3);
}

/**
  * @tc.name: AppBlobEmpty001
  * @tc.desc: Construct a Blob and check its empty.
  * @tc.type: FUNC
  * @tc.require: AR000CCPOL
  * @tc.author: liqiao
  */
HWTEST_F(AppBlobTest, AppBlobEmpty001, TestSize.Level0)
{
    AppBlob blob1;
    EXPECT_EQ(blob1.Empty(), true);
    AppBlob blob2 = "1234567890";
    EXPECT_EQ(blob2.Empty(), false);
    AppBlob blob3("12345");
    EXPECT_EQ(blob3.Empty(), false);
    std::string strTmp = "123";
    const char *chr = strTmp.c_str();
    AppBlob blob4(chr);
    EXPECT_EQ(blob4.Empty(), false);
    std::vector<uint8_t> vec = {'1', '2', '3', '4'};
    AppBlob blob5(vec);
    EXPECT_EQ(blob5.Empty(), false);
    const char *chr1 = strTmp.c_str();
    AppBlob blob6(chr1, strlen(chr1));
    EXPECT_EQ(blob6.Empty(), false);
}

/**
  * @tc.name: AppBlobCompare001
  * @tc.desc: Construct a Blob and check its StartsWith function.
  * @tc.type: FUNC
  * @tc.require: AR000CCPOL
  * @tc.author: liqiao
  */
HWTEST_F(AppBlobTest, AppBlobCompare001, TestSize.Level0)
{
    AppBlob blob1 = "1234567890";
    AppBlob blob2("12345");
    EXPECT_EQ(blob1.Compare(blob2), 1);
    EXPECT_EQ(blob2.Compare(blob1), -1);
    AppBlob blob3("12345");
    EXPECT_EQ(blob2.Compare(blob3), 0);
}

/**
  * @tc.name: AppBlobData001
  * @tc.desc: Construct a Blob and check its Data function.
  * @tc.type: FUNC
  * @tc.require: AR000CCPOL
  * @tc.author: liqiao
  */
HWTEST_F(AppBlobTest, AppBlobData001, TestSize.Level0)
{
    std::vector<uint8_t> result = {'1', '2', '3', '4'};
    AppBlob blob1("1234");
    EXPECT_EQ(blob1.Data(), result);
    std::vector<uint8_t> result2 = {'1', '2', '3', '4', '5'};
    AppBlob blob2("12345");
    EXPECT_EQ(blob2.Data(), result2);
}

/**
  * @tc.name: AppBlobToString001
  * @tc.desc: Construct a Blob and check its ToString function.
  * @tc.type: FUNC
  * @tc.require: AR000CCPOL
  * @tc.author: liqiao
  */
HWTEST_F(AppBlobTest, AppBlobToString001, TestSize.Level0)
{
    AppBlob blob1("1234");
    std::string str = "1234";
    EXPECT_EQ(blob1.ToString(), str);
}

/**
  * @tc.name: AppBlobOperatorEqual001
  * @tc.desc: Construct a Blob and check its operator== function.
  * @tc.type: FUNC
  * @tc.require: AR000CCPOL
  * @tc.author: liqiao
  */
HWTEST_F(AppBlobTest, AppBlobOperatorEqual001, TestSize.Level0)
{
    AppBlob blob1("1234");
    AppBlob blob2("1234");
    EXPECT_EQ(blob1 == blob2, true);
    AppBlob blob3("12345");
    EXPECT_EQ(blob1 == blob3, false);
}

/**
  * @tc.name: AppBlobOperator002
  * @tc.desc: Construct a Blob and check its operator= function.
  * @tc.type: FUNC
  * @tc.require: AR000CCPOL
  * @tc.author: liqiao
  */
HWTEST_F(AppBlobTest, AppBlobOperator002, TestSize.Level0)
{
    AppBlob blob1("1234");
    AppBlob blob2 = blob1;
    EXPECT_EQ(blob1 == blob2, true);
    EXPECT_EQ(blob2.ToString(), "1234");
    blob2 = blob1;
    EXPECT_EQ(blob1 == blob2, true);
    EXPECT_EQ(blob2.ToString(), "1234");
    blob2 = std::move(blob1);
    EXPECT_EQ(blob1 == blob2, true);
    EXPECT_EQ(blob2.ToString(), "1234");
}

/**
  * @tc.name: AppBlobOperator003
  * @tc.desc: Construct a Blob and check its operator= function.
  * @tc.type: FUNC
  * @tc.require: AR000CCPOL
  * @tc.author: liqiao
  */
HWTEST_F(AppBlobTest, AppBlobOperator003, TestSize.Level0)
{
    AppBlob blob1("1234");
    AppBlob blob2 = std::move(blob1);
    EXPECT_EQ(blob1 == blob2, false);
    EXPECT_EQ(blob1.Empty(), true);
    EXPECT_EQ(blob2.ToString(), "1234");
}