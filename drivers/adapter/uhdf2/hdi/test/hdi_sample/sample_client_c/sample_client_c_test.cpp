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

#include <thread>
#include <unistd.h>
#include <vector>
#include <gtest/gtest.h>
#include <hdf_log.h>
#include <osal_mem.h>
#include "securec.h"
#include "isample.h"

using namespace OHOS;
using namespace testing::ext;

#define HDF_LOG_TAG    sample_client_c_test

constexpr const char *TEST_SERVICE_NAME = "sample_service_c";

class SampleObjCTest : public testing::Test {
public:
    static void SetUpTestCase(){}
    static void TearDownTestCase(){}
    void SetUp(){}
    void TearDown(){}
};

HWTEST_F(SampleObjCTest, SampleObjCTest_001, TestSize.Level1)
{
    struct ISample *sampleObj = HdiSampleGet(TEST_SERVICE_NAME);
    ASSERT_TRUE(sampleObj != nullptr);

    bool input = true;
    bool output = false;

    int32_t ec = sampleObj->BooleanTypeTest(sampleObj, input, &output);

    ASSERT_EQ(ec, HDF_SUCCESS);
    ASSERT_TRUE(output);

    HdiSampleRelease(sampleObj);
}

HWTEST_F(SampleObjCTest, SampleObjCTest_002, TestSize.Level1)
{
    struct ISample *sampleObj = HdiSampleGet(TEST_SERVICE_NAME);
    ASSERT_TRUE(sampleObj != nullptr);

    int8_t input = 10;
    int8_t output;

    int32_t ec = sampleObj->ByteTypeTest(sampleObj, input, &output);

    ASSERT_EQ(ec, HDF_SUCCESS);
    ASSERT_EQ(input, output);

    HdiSampleRelease(sampleObj);
}

HWTEST_F(SampleObjCTest, SampleObjCTest_003, TestSize.Level1)
{
    struct ISample *sampleObj = HdiSampleGet(TEST_SERVICE_NAME);
    ASSERT_TRUE(sampleObj != nullptr);

    int16_t input = 10;
    int16_t output;

    int32_t ec = sampleObj->ShortTypeTest(sampleObj, input, &output);

    ASSERT_EQ(ec, HDF_SUCCESS);
    ASSERT_EQ(input, output);

    HdiSampleRelease(sampleObj);
}

HWTEST_F(SampleObjCTest, SampleObjCTest_004, TestSize.Level1)
{
    struct ISample *sampleObj = HdiSampleGet(TEST_SERVICE_NAME);
    ASSERT_TRUE(sampleObj != nullptr);

    int32_t input = 10;
    int32_t output;

    int32_t ec = sampleObj->IntTypeTest(sampleObj, input, &output);

    ASSERT_EQ(ec, HDF_SUCCESS);
    ASSERT_EQ(input, output);

    HdiSampleRelease(sampleObj);
}

HWTEST_F(SampleObjCTest, SampleObjCTest_005, TestSize.Level1)
{
    struct ISample *sampleObj = HdiSampleGet(TEST_SERVICE_NAME);
    ASSERT_TRUE(sampleObj != nullptr);

    int64_t input = 10;
    int64_t output;

    int32_t ec = sampleObj->LongTypeTest(sampleObj, input, &output);

    ASSERT_EQ(ec, HDF_SUCCESS);
    ASSERT_EQ(input, output);

    HdiSampleRelease(sampleObj);
}

HWTEST_F(SampleObjCTest, SampleObjCTest_006, TestSize.Level1)
{
    struct ISample *sampleObj = HdiSampleGet(TEST_SERVICE_NAME);
    ASSERT_TRUE(sampleObj != nullptr);

    float input = 10;
    float output;

    int32_t ec = sampleObj->FloatTypeTest(sampleObj, input, &output);

    ASSERT_EQ(ec, HDF_SUCCESS);
    ASSERT_EQ(input, output);

    HdiSampleRelease(sampleObj);
}

HWTEST_F(SampleObjCTest, SampleObjCTest_007, TestSize.Level1)
{
    struct ISample *sampleObj = HdiSampleGet(TEST_SERVICE_NAME);
    ASSERT_TRUE(sampleObj != nullptr);

    double input = 10;
    double output;

    int32_t ec = sampleObj->DoubleTypeTest(sampleObj, input, &output);

    ASSERT_EQ(ec, HDF_SUCCESS);
    ASSERT_EQ(input, output);

    HdiSampleRelease(sampleObj);
}

HWTEST_F(SampleObjCTest, SampleObjCTest_008, TestSize.Level1)
{
    struct ISample *sampleObj = HdiSampleGet(TEST_SERVICE_NAME);
    ASSERT_TRUE(sampleObj != nullptr);

    const char *input = "c sample client test";
    char *output = nullptr;

    int32_t ec = sampleObj->StringTypeTest(sampleObj, input, &output);

    ASSERT_EQ(ec, HDF_SUCCESS);
    EXPECT_STREQ(input, output);

    HdiSampleRelease(sampleObj);
}

HWTEST_F(SampleObjCTest, SampleObjCTest_009, TestSize.Level1)
{
    struct ISample *sampleObj = HdiSampleGet(TEST_SERVICE_NAME);
    ASSERT_TRUE(sampleObj != nullptr);

    uint8_t input = 10;
    uint8_t output;

    int32_t ec = sampleObj->UcharTypeTest(sampleObj, input, &output);

    ASSERT_EQ(ec, HDF_SUCCESS);
    ASSERT_EQ(input, output);

    HdiSampleRelease(sampleObj);
}

HWTEST_F(SampleObjCTest, SampleObjCTest_010, TestSize.Level1)
{
    struct ISample *sampleObj = HdiSampleGet(TEST_SERVICE_NAME);
    ASSERT_TRUE(sampleObj != nullptr);

    uint16_t input = 10;
    uint16_t output;

    int32_t  ec = sampleObj->UshortTypeTest(sampleObj, input, &output);

    ASSERT_EQ(ec, HDF_SUCCESS);
    ASSERT_EQ(input, output);

    HdiSampleRelease(sampleObj);
}

HWTEST_F(SampleObjCTest, SampleObjCTest_011, TestSize.Level1)
{
    struct ISample *sampleObj = HdiSampleGet(TEST_SERVICE_NAME);
    ASSERT_TRUE(sampleObj != nullptr);

    uint32_t input = 10;
    uint32_t output;

    int32_t ec = sampleObj->UintTypeTest(sampleObj, input, &output);

    ASSERT_EQ(ec, HDF_SUCCESS);
    ASSERT_EQ(input, output);

    HdiSampleRelease(sampleObj);
}

HWTEST_F(SampleObjCTest, SampleObjCTest_012, TestSize.Level1)
{
    struct ISample *sampleObj = HdiSampleGet(TEST_SERVICE_NAME);
    ASSERT_TRUE(sampleObj != nullptr);

    uint64_t input = 10;
    uint64_t output;

    int32_t ec = sampleObj->UlongTypeTest(sampleObj, input, &output);

    ASSERT_EQ(ec, HDF_SUCCESS);
    ASSERT_EQ(input, output);

    HdiSampleRelease(sampleObj);
}

HWTEST_F(SampleObjCTest, SampleObjCTest_013, TestSize.Level1)
{
    struct ISample *sampleObj = HdiSampleGet(TEST_SERVICE_NAME);
    ASSERT_TRUE(sampleObj != nullptr);

    uint32_t inSize = 5;
    int8_t *input = (int8_t *)OsalMemAlloc(sizeof(int8_t) * inSize);
    ASSERT_TRUE(input != nullptr);

    for (uint32_t i = 0; i < inSize; i++) {
        input[i] = static_cast<int8_t>(i);
    }

    uint32_t outSize = 0;
    int8_t *output = nullptr;

    int32_t ec = sampleObj->ListTypeTest(sampleObj, input, inSize, &output, &outSize);

    ASSERT_EQ(ec, HDF_SUCCESS);
    ASSERT_TRUE(output != nullptr);
    ASSERT_EQ(outSize, inSize);

    for (uint32_t i = 0; i < outSize; i++) {
        ASSERT_EQ(output[i], input[i]);
    }

    if (input != nullptr) {
        OsalMemFree(input);
    }
    if (output != nullptr) {
        OsalMemFree(output);
    }
}

HWTEST_F(SampleObjCTest, SampleObjCTest_014, TestSize.Level1)
{
    struct ISample *sampleObj = HdiSampleGet(TEST_SERVICE_NAME);
    ASSERT_TRUE(sampleObj != nullptr);

    uint32_t inSize = 5;
    int8_t *input = (int8_t *)OsalMemAlloc(sizeof(int8_t) * inSize);
    ASSERT_TRUE(input != nullptr);

    for (uint32_t i = 0; i < inSize; i++) {
        input[i] = static_cast<int8_t>(i);
    }

    uint32_t outSize = 0;
    int8_t *output = nullptr;

    int32_t ec = sampleObj->ArrayTypeTest(sampleObj, input, inSize, &output, &outSize);

    ASSERT_EQ(ec, HDF_SUCCESS);
    ASSERT_TRUE(output != nullptr);
    ASSERT_EQ(outSize, inSize);

    for (uint32_t i = 0; i < outSize; i++) {
        ASSERT_EQ(output[i], input[i]);
    }

    if (input != nullptr) {
        OsalMemFree(input);
    }
    if (output != nullptr) {
        OsalMemFree(output);
    }
}

HWTEST_F(SampleObjCTest, SampleObjCTest_015, TestSize.Level1)
{
    struct ISample *sampleObj = HdiSampleGet(TEST_SERVICE_NAME);
    ASSERT_TRUE(sampleObj != nullptr);

    struct StructSample input = {
        .first = 1,
        .second = 2,
    };

    struct StructSample output;
    int32_t ec = sampleObj->StructTypeTest(sampleObj, &input, &output);

    ASSERT_EQ(ec, HDF_SUCCESS);
    ASSERT_EQ(output.first, input.first);
    ASSERT_EQ(output.second, input.second);

    HdiSampleRelease(sampleObj);
}

HWTEST_F(SampleObjCTest, SampleObjCTest_016, TestSize.Level1)
{
    struct ISample *sampleObj = HdiSampleGet(TEST_SERVICE_NAME);
    ASSERT_TRUE(sampleObj != nullptr);

    enum EnumSample input = MEM_SECOND;
    enum EnumSample output;

    int32_t ec = sampleObj->EnumTypeTest(sampleObj, input, &output);

    ASSERT_EQ(ec, HDF_SUCCESS);
    ASSERT_EQ(input, output);

    HdiSampleRelease(sampleObj);
}

