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
#include <map>
#include <thread>
#include <unistd.h>
#include <vector>
#include <gtest/gtest.h>
#include <hdf_log.h>
#include <osal_mem.h>
#include "securec.h"
#include "sample_proxy.h"

using namespace OHOS;
using namespace testing::ext;
using namespace OHOS::HDI::Sample::V1_0;

#define HDF_LOG_TAG sample_client_cpp_test

constexpr const char *TEST_SERVICE_NAME = "sample_service_cpp";

class SampleObjCPPTest : public testing::Test {
public:
    static void SetUpTestCase(){}
    static void TearDownTestCase(){}
    void SetUp(){}
    void TearDown(){}
};

HWTEST_F(SampleObjCPPTest, SampleObjCPPTest_001, TestSize.Level1)
{
    sptr<ISample> sampleObj = ISample::Get(TEST_SERVICE_NAME);
    ASSERT_TRUE(sampleObj != nullptr);

    bool input = true;
    bool output = false;

    int32_t ec = sampleObj->BooleanTypeTest(input, output);

    ASSERT_EQ(ec, HDF_SUCCESS);
    ASSERT_EQ(output, input);
}

HWTEST_F(SampleObjCPPTest, SampleObjCPPTest_002, TestSize.Level1)
{
    sptr<ISample> sampleObj = ISample::Get(TEST_SERVICE_NAME);
    ASSERT_TRUE(sampleObj != nullptr);

    int8_t input = 5;
    int8_t output;

    int32_t ec = sampleObj->ByteTypeTest(input, output);

    ASSERT_EQ(ec, HDF_SUCCESS);
    ASSERT_EQ(input, output);
}

HWTEST_F(SampleObjCPPTest, SampleObjCPPTest_003, TestSize.Level1)
{
    sptr<ISample> sampleObj = ISample::Get(TEST_SERVICE_NAME);
    ASSERT_TRUE(sampleObj != nullptr);

    int16_t input = 5;
    int16_t output;

    int32_t ec = sampleObj->ShortTypeTest(input, output);

    ASSERT_EQ(ec, HDF_SUCCESS);
    ASSERT_EQ(input, output);
}

HWTEST_F(SampleObjCPPTest, SampleObjCPPTest_004, TestSize.Level1)
{
    sptr<ISample> sampleObj = ISample::Get(TEST_SERVICE_NAME);
    ASSERT_TRUE(sampleObj != nullptr);

    int32_t input = 5;
    int32_t output;

    int32_t ec = sampleObj->IntTypeTest(input, output);

    ASSERT_EQ(ec, HDF_SUCCESS);
    ASSERT_EQ(input, output);
}

HWTEST_F(SampleObjCPPTest, SampleObjCPPTest_005, TestSize.Level1)
{
    sptr<ISample> sampleObj = ISample::Get(TEST_SERVICE_NAME);
    ASSERT_TRUE(sampleObj != nullptr);

    int64_t input = 5;
    int64_t output;

    int32_t ec = sampleObj->LongTypeTest(input, output);

    ASSERT_EQ(ec, HDF_SUCCESS);
    ASSERT_EQ(input, output);
}

HWTEST_F(SampleObjCPPTest, SampleObjCPPTest_006, TestSize.Level1)
{
    sptr<ISample> sampleObj = ISample::Get(TEST_SERVICE_NAME);
    ASSERT_TRUE(sampleObj != nullptr);

    float input = 5;
    float output;

    int32_t ec = sampleObj->FloatTypeTest(input, output);

    ASSERT_EQ(ec, HDF_SUCCESS);
    ASSERT_EQ(input, output);
}

HWTEST_F(SampleObjCPPTest, SampleObjCPPTest_007, TestSize.Level1)
{
    sptr<ISample> sampleObj = ISample::Get(TEST_SERVICE_NAME);
    ASSERT_TRUE(sampleObj != nullptr);

    double input = 5;
    double output;

    int32_t ec = sampleObj->DoubleTypeTest(input, output);

    ASSERT_EQ(ec, HDF_SUCCESS);
    ASSERT_EQ(input, output);
}

HWTEST_F(SampleObjCPPTest, SampleObjCPPTest_008, TestSize.Level1)
{
    sptr<ISample> sampleObj = ISample::Get(TEST_SERVICE_NAME);
    ASSERT_TRUE(sampleObj != nullptr);

    std::string input("cpp sample client test");
    std::string output;

    int32_t ec = sampleObj->StringTypeTest(input, output);

    ASSERT_EQ(ec, HDF_SUCCESS);
    EXPECT_EQ(input, output);
}

HWTEST_F(SampleObjCPPTest, SampleObjCPPTest_009, TestSize.Level1)
{
    sptr<ISample> sampleObj = ISample::Get(TEST_SERVICE_NAME);
    ASSERT_TRUE(sampleObj != nullptr);

    uint8_t input = 5;
    uint8_t output;

    int32_t ec = sampleObj->UcharTypeTest(input, output);

    ASSERT_EQ(ec, HDF_SUCCESS);
    ASSERT_EQ(input, output);
}

HWTEST_F(SampleObjCPPTest, SampleObjCPPTest_010, TestSize.Level1)
{
    sptr<ISample> sampleObj = ISample::Get(TEST_SERVICE_NAME);
    ASSERT_TRUE(sampleObj != nullptr);

    uint16_t input = 5;
    uint16_t output;

    int32_t ec = sampleObj->UshortTypeTest(input, output);

    ASSERT_EQ(ec, HDF_SUCCESS);
    ASSERT_EQ(input, output);
}

HWTEST_F(SampleObjCPPTest, SampleObjCPPTest_011, TestSize.Level1)
{
    sptr<ISample> sampleObj = ISample::Get(TEST_SERVICE_NAME);
    ASSERT_TRUE(sampleObj != nullptr);

    uint32_t input = 5;
    uint32_t output;

    int32_t ec = sampleObj->UintTypeTest(input, output);

    ASSERT_EQ(ec, HDF_SUCCESS);
    ASSERT_EQ(input, output);
}
HWTEST_F(SampleObjCPPTest, SampleObjCPPTest_012, TestSize.Level1)
{
    sptr<ISample> sampleObj = ISample::Get(TEST_SERVICE_NAME);
    ASSERT_TRUE(sampleObj != nullptr);

    uint64_t input = 5;
    uint64_t output;

    int32_t ec = sampleObj->UlongTypeTest(input, output);

    ASSERT_EQ(ec, HDF_SUCCESS);
    ASSERT_EQ(input, output);
}

HWTEST_F(SampleObjCPPTest, SampleObjCPPTest_013, TestSize.Level1)
{
    sptr<ISample> sampleObj = ISample::Get(TEST_SERVICE_NAME);
    ASSERT_TRUE(sampleObj != nullptr);

    std::list<int8_t> input{1, 2, 3, 4, 5};
    std::list<int8_t> output;

    int32_t ec = sampleObj->ListTypeTest(input, output);

    ASSERT_EQ(ec, HDF_SUCCESS);
    ASSERT_EQ(output.size(), input.size());

    auto inIter = input.begin();
    auto outIter = output.begin();
    while (inIter != input.end() && outIter != output.end()) {
        ASSERT_EQ(*(inIter++), *(outIter++));
    }
}

HWTEST_F(SampleObjCPPTest, SampleObjCPPTest_014, TestSize.Level1)
{
    sptr<ISample> sampleObj = ISample::Get(TEST_SERVICE_NAME);
    ASSERT_TRUE(sampleObj != nullptr);

    std::map<int8_t, int8_t> input;
    for (int8_t i = 0; i < 10; i++) {
        input[i] = i;
    }
    std::map<int8_t, int8_t> output;

    int32_t ec = sampleObj->MapTypeTest(input, output);

    ASSERT_EQ(ec, HDF_SUCCESS);
    ASSERT_EQ(input.size(), output.size());

    std::map<int8_t, int8_t>::iterator inIter = input.begin();
    for (; inIter != input.begin(); inIter++) {
        std::map<int8_t, int8_t>::iterator outIter = output.find(inIter->first);

        ASSERT_TRUE(outIter != output.end());
        ASSERT_EQ(outIter->second, inIter->second);
    }
}

HWTEST_F(SampleObjCPPTest, SampleObjCPPTest_015, TestSize.Level1)
{
    sptr<ISample> sampleObj = ISample::Get(TEST_SERVICE_NAME);
    ASSERT_TRUE(sampleObj != nullptr);

    std::vector<int8_t> input;
    std::vector<int8_t> output;

    int32_t ec = sampleObj->ArrayTypeTest(input, output);

    ASSERT_EQ(ec, HDF_SUCCESS);
    ASSERT_EQ(input.size(), output.size());

    for (uint32_t i = 0; i < output.size(); i++) {
        ASSERT_EQ(output[i], input[i]);
    }
}

HWTEST_F(SampleObjCPPTest, SampleObjCPPTest_016, TestSize.Level1)
{
    sptr<ISample> sampleObj = ISample::Get(TEST_SERVICE_NAME);
    ASSERT_TRUE(sampleObj != nullptr);

    StructSample input = {
        .first = 5,
        .second = 10,
    };

    StructSample output;
    int32_t ec = sampleObj->StructTypeTest(input, output);
    ASSERT_EQ(ec, HDF_SUCCESS);
    ASSERT_EQ(output.first, input.first);
    ASSERT_EQ(output.second, input.second);
}

HWTEST_F(SampleObjCPPTest, SampleObjCPPTest_017, TestSize.Level1)
{
    sptr<ISample> sampleObj = ISample::Get(TEST_SERVICE_NAME);
    ASSERT_TRUE(sampleObj != nullptr);

    EnumSample input = EnumSample::MEM_SECOND;
    EnumSample output;

    int32_t ec = sampleObj->EnumTypeTest(input, output);

    ASSERT_EQ(ec, HDF_SUCCESS);
    ASSERT_EQ(output, input);
}
