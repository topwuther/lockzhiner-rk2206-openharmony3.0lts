/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
 *
 * HDF is dual licensed: you can use it either under the terms of
 * the GPL, or the BSD license, at your option.
 * See the LICENSE file in the root of this repository for complete details.
 */

#include <iostream>
#include <thread>
#include <unistd.h>
#include <gtest/gtest.h>
#include <hdf_log.h>
#include <osal_mem.h>
#include <securec.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "test/cpp_test/struct_test/v1_0/client/struct_test_proxy.h"

using namespace OHOS;
using namespace testing::ext;
using namespace test::cpp_test::struct_test::v1_0;
using namespace test::cpp_test::types::v1_0;

static sptr<IStructTest> g_testClient = nullptr;

class StructTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase(){}
    void SetUp(){}
    void TearDown(){}
};

void StructTest::SetUpTestCase()
{
    g_testClient = IStructTest::Get();
    if (g_testClient == nullptr) {
        printf("StructTest: get g_testClient failed.\n");
    }
}

HWTEST_F(StructTest, StructTest_001, TestSize.Level0)
{
    ASSERT_NE(nullptr, g_testClient);
}

static std::string ESampleToStr(ESample obj)
{
    switch (obj) {
        case ESample::MEM_ONE:
            return "MEM_ONE";
        case ESample::MEM_TWO:
            return "MEM_TWO";
        case ESample::MEM_THREE:
            return "MEM_THREE";
        default:
            return "unknown";
    }
}

static void PrintSSample(const SSample& obj)
{
    std::cout << "{";
    std::cout << "m1:" << (obj.m1 ? 1 : 0) << ", ";
    std::cout << "m2:" << obj.m2 << ", ";
    std::cout << "m3:" << obj.m3 << ", ";
    std::cout << "m4:" << obj.m4;
    std::cout << "}";
}

HWTEST_F(StructTest, StructTest_002, TestSize.Level0)
{
    SSample srcObj = {true, 1, 1000.125, "hello world"};

    SSample destObj;

    int32_t ec = g_testClient->SSampleTest(srcObj, destObj);
    ASSERT_EQ(ec, HDF_SUCCESS);

    EXPECT_EQ((srcObj.m1 ? 1 : 0), (destObj.m1 ? 1 : 0));
    EXPECT_EQ(srcObj.m2, destObj.m2);
    EXPECT_DOUBLE_EQ(srcObj.m3, destObj.m3);
    EXPECT_EQ(srcObj.m4, destObj.m4);

    PrintSSample(srcObj);
    std::cout << "\n";
    PrintSSample(destObj);
    std::cout << "\n";
    std::cout << "----------------------------" << std::endl;
}

static void PrintSSample2(const SSample2& obj)
{
    std::cout << "{";

    std::cout << "m1:" << obj.m1 << ", ";
    std::cout << "m2:" << (obj.m2 ? 1 : 0) << ", ";
    std::cout << "m3:" << obj.m3 << ", ";
    std::cout << "m4:" << obj.m4 << ", ";
    std::cout << "m5:" << obj.m5 << ", ";
    std::cout << "m6:" << obj.m6 << ", ";
    std::cout << "m7:" << obj.m7 << ", ";
    std::cout << "m8:" << obj.m8 << ", ";
    std::cout << "m9:" << obj.m9 << ", ";
    std::cout << "m10:" << obj.m10 << ", ";
    std::cout << "m11:" << obj.m11;
    std::cout << "}";
}

HWTEST_F(StructTest, StructTest_003, TestSize.Level0)
{
    SSample2 srcObj = {true, 1, 2, 3, 4, 65, 20, 30, 40, 100.25, 1000.125};

    SSample2 destObj;

    int32_t ec = g_testClient->SSample2Test(srcObj, destObj);
    ASSERT_EQ(ec, HDF_SUCCESS);

    EXPECT_EQ((srcObj.m1 ? 1 : 0), (destObj.m1 ? 1 : 0));
    EXPECT_EQ(srcObj.m2, destObj.m2);
    EXPECT_EQ(srcObj.m3, destObj.m3);
    EXPECT_EQ(srcObj.m4, destObj.m4);
    EXPECT_EQ(srcObj.m5, destObj.m5);
    EXPECT_EQ(srcObj.m6, destObj.m6);
    EXPECT_EQ(srcObj.m7, destObj.m7);
    EXPECT_EQ(srcObj.m8, destObj.m8);
    EXPECT_EQ(srcObj.m9, destObj.m9);
    EXPECT_FLOAT_EQ(srcObj.m10, destObj.m10);
    EXPECT_DOUBLE_EQ(srcObj.m11, destObj.m11);

    PrintSSample2(srcObj);
    std::cout << "\n";
    PrintSSample2(destObj);
    std::cout << "\n";
    std::cout << "----------------------------" << std::endl;
}

static void PrintSSample3(const SSample3& obj)
{
    std::cout << "{";
    std::cout << "m1:" << obj.m1 << ", ";
    std::cout << "m2:" << ESampleToStr(obj.m2) << ", ";

    std::cout << "m3:";
    PrintSSample2(obj.m3);
    std::cout << ", ";

    std::cout << "m4:" << obj.m4;
}

HWTEST_F(StructTest, StructTest_004, TestSize.Level0)
{
    int fd = open("/fdtest3.txt", O_CREAT | O_RDWR | O_TRUNC, 0644);
    SSample3 srcObj = {
        "hello world",
        ESample::MEM_ONE,
        {true, 1, 2, 3, 4, 65, 20, 30, 40, 100.25, 1000.125},
        fd,
    };

    SSample3 destObj;
    int32_t ec = g_testClient->SSample3Test(srcObj, destObj);
    ASSERT_EQ(ec, HDF_SUCCESS);

    EXPECT_EQ(srcObj.m1, destObj.m1);
    EXPECT_EQ(srcObj.m2, destObj.m2);

    EXPECT_EQ((srcObj.m3.m1 ? 1 : 0), (destObj.m3.m1 ? 1 : 0));
    EXPECT_EQ(srcObj.m3.m2, destObj.m3.m2);
    EXPECT_EQ(srcObj.m3.m3, destObj.m3.m3);
    EXPECT_EQ(srcObj.m3.m4, destObj.m3.m4);
    EXPECT_EQ(srcObj.m3.m5, destObj.m3.m5);
    EXPECT_EQ(srcObj.m3.m6, destObj.m3.m6);
    EXPECT_EQ(srcObj.m3.m7, destObj.m3.m7);
    EXPECT_EQ(srcObj.m3.m8, destObj.m3.m8);
    EXPECT_EQ(srcObj.m3.m9, destObj.m3.m9);
    EXPECT_FLOAT_EQ(srcObj.m3.m10, destObj.m3.m10);
    EXPECT_DOUBLE_EQ(srcObj.m3.m11, destObj.m3.m11);

    PrintSSample3(srcObj);
    std::cout << "\n";
    PrintSSample3(destObj);
    std::cout << "\n";
    std::cout << "----------------------------" << std::endl;
    close(srcObj.m4);
    close(destObj.m4);
}

static void PrintSSample4(const SSample4& obj)
{
    std::cout << "{\n";

    std::cout << "m1:{";
    for (size_t i = 0; i < obj.m1.size(); i++) {
        std::cout << (obj.m1[i] ? 1 : 0) << ", ";
    }
    std::cout << "},\n";

    std::cout << "m2:{";
    for (size_t i = 0; i < obj.m2.size(); i++) {
        std::cout << obj.m2[i] << ", ";
    }
    std::cout << "},\n";

    std::cout << "m3:{";
    for (size_t i = 0; i < obj.m3.size(); i++) {
        std::cout << obj.m3[i] << ", ";
    }
    std::cout << "},\n";

    std::cout << "m4:{";
    for (size_t i = 0; i < obj.m4.size(); i++) {
        std::cout << obj.m4[i] << ", ";
    }
    std::cout << "},\n";

    std::cout << "m5:{";
    for (size_t i = 0; i < obj.m5.size(); i++) {
        std::cout << obj.m5[i] << ", ";
    }
    std::cout << "},\n";

    std::cout << "m6:{";
    for (size_t i = 0; i < obj.m6.size(); i++) {
        std::cout << obj.m6[i] << ", ";
    }
    std::cout << "},\n";

    std::cout << "m7:{";
    for (size_t i = 0; i < obj.m7.size(); i++) {
        std::cout << obj.m7[i] << ", ";
    }
    std::cout << "},\n";

    std::cout << "m8:{";
    for (size_t i = 0; i < obj.m8.size(); i++) {
        std::cout << obj.m8[i] << ", ";
    }
    std::cout << "},\n";

    std::cout << "m9:{";
    for (size_t i = 0; i < obj.m9.size(); i++) {
        std::cout << obj.m9[i] << ", ";
    }
    std::cout << "},\n";

    std::cout << "m10:{";
    for (size_t i = 0; i < obj.m10.size(); i++) {
        std::cout << obj.m10[i] << ", ";
    }
    std::cout << "},\n";

    std::cout << "m11:{";
    for (size_t i = 0; i < obj.m11.size(); i++) {
        std::cout << obj.m11[i] << ", ";
    }
    std::cout << "},\n";

    std::cout << "m12:{";
    for (size_t i = 0; i < obj.m12.size(); i++) {
        std::cout << obj.m12[i] << ", ";
    }
    std::cout << "},\n";

    std::cout << "m13:{";
    for (size_t i = 0; i < obj.m13.size(); i++) {
        std::cout << ESampleToStr(obj.m13[i]) << ", ";
    }
    std::cout << "},\n";

    std::cout << "m14:{";
    for (size_t i = 0; i < obj.m13.size(); i++) {
        PrintSSample(obj.m14[i]);
        std::cout << ", ";
    }
    std::cout << "},\n";

    std::cout << "m15:{";
    for (size_t i = 0; i < obj.m15.size(); i++) {
        std::cout << obj.m15[i] << ", ";
    }
    std::cout << "}";
    std::cout << "}\n";
}

HWTEST_F(StructTest, StructTest_005, TestSize.Level0)
{
    SSample4 srcObj = {
        {true, false},
        {65, 66},
        {3, 4},
        {5, 6},
        {7, 8},
        {97, 98},
        {30, 40},
        {50, 60},
        {70, 80},
        {10.5, 20.5},
        {30.125, 30.125},
        {"hello", "world"},
        {ESample::MEM_ONE, ESample::MEM_THREE},
        {{true, 1, 1000.125, "hello"}, {false, 1, 1000.125, "world"}},
        {sptr<SequenceData>(new SequenceData(1, 1.2, "hello")), sptr<SequenceData>(new SequenceData(2, 2.2, "world"))},
    };

    SSample4 destObj;

    int32_t ec = g_testClient->SSample4Test(srcObj, destObj);
    ASSERT_EQ(ec, HDF_SUCCESS);

    for (size_t i = 0; i < srcObj.m1.size(); i++) {
        EXPECT_EQ((srcObj.m1[i] ? 1 : 0), (destObj.m1[i] ? 1 : 0));
    }

    for (size_t i = 0; i < srcObj.m2.size(); i++) {
        EXPECT_EQ(srcObj.m2[i], destObj.m2[i]);
    }

    for (size_t i = 0; i < srcObj.m3.size(); i++) {
        EXPECT_EQ(srcObj.m3[i], destObj.m3[i]);
    }

    for (size_t i = 0; i < srcObj.m4.size(); i++) {
        EXPECT_EQ(srcObj.m4[i], destObj.m4[i]);
    }

    for (size_t i = 0; i < srcObj.m5.size(); i++) {
        EXPECT_EQ(srcObj.m5[i], destObj.m5[i]);
    }

    for (size_t i = 0; i < srcObj.m6.size(); i++) {
        EXPECT_EQ(srcObj.m6[i], destObj.m6[i]);
    }

    for (size_t i = 0; i < srcObj.m7.size(); i++) {
        EXPECT_EQ(srcObj.m7[i], destObj.m7[i]);
    }

    for (size_t i = 0; i < srcObj.m8.size(); i++) {
        EXPECT_EQ(srcObj.m8[i], destObj.m8[i]);
    }

    for (size_t i = 0; i < srcObj.m9.size(); i++) {
        EXPECT_EQ(srcObj.m9[i], destObj.m9[i]);
    }

    for (size_t i = 0; i < srcObj.m10.size(); i++) {
        EXPECT_FLOAT_EQ(srcObj.m10[i], destObj.m10[i]);
    }

    for (size_t i = 0; i < srcObj.m11.size(); i++) {
        EXPECT_DOUBLE_EQ(srcObj.m11[i], destObj.m11[i]);
    }

    for (size_t i = 0; i < srcObj.m12.size(); i++) {
        EXPECT_EQ(srcObj.m12[i], destObj.m12[i]);
    }

    for (size_t i = 0; i < srcObj.m13.size(); i++) {
        EXPECT_EQ(srcObj.m13[i], destObj.m13[i]);
    }

    for (size_t i = 0; i < srcObj.m14.size(); i++) {
        EXPECT_EQ(((srcObj.m14[i]).m1 ? 1 : 0), ((destObj.m14[i]).m1 ? 1 : 0));
        EXPECT_EQ((srcObj.m14[i]).m2, (destObj.m14[i]).m2);
        EXPECT_DOUBLE_EQ((srcObj.m14[i]).m3, (destObj.m14[i]).m3);
        EXPECT_EQ((srcObj.m14[i]).m4, (destObj.m14[i]).m4);
    }

    for (size_t i = 0; i < srcObj.m15.size(); i++) {
        sptr<SequenceData> var1 = srcObj.m15[i];
        sptr<SequenceData> var2 = destObj.m15[i];

        if (var1 != nullptr && var2 != nullptr) {
            EXPECT_EQ(var1->m1_, var2->m1_);
            EXPECT_DOUBLE_EQ(var1->m2_, var2->m2_);
            EXPECT_EQ(var1->m3_, var2->m3_);
        } else {
            std::cout << "var1 or var2 is nullptr" << std::endl;
        }
    }

    PrintSSample4(srcObj);
    PrintSSample4(destObj);
    std::cout << "\n";
    std::cout << "--------------------------------------\n";
}

static void PrintSSample5(const SSample5& obj)
{
    std::cout << "{\n";

    std::cout << "m1:{";
    for (size_t i = 0; i < obj.m1.size(); i++) {
        std::cout << obj.m1[i] << ", ";
    }
    std::cout << "},\n";

    std::cout << "m2:{";
    for (size_t i = 0; i < obj.m2.size(); i++) {
        std::cout << obj.m2[i] << ", ";
    }
    std::cout << "},\n";

    std::cout << "m3:{";
    for (size_t i = 0; i < obj.m3.size(); i++) {
        std::cout << obj.m3[i] << ", ";
    }
    std::cout << "},\n";

    std::cout << "m4:{";
    for (size_t i = 0; i < obj.m4.size(); i++) {
        std::cout << obj.m4[i] << ", ";
    }
    std::cout << "},\n";

    std::cout << "m5:{";
    for (size_t i = 0; i < obj.m5.size(); i++) {
        std::cout << obj.m5[i] << ", ";
    }
    std::cout << "},\n";

    std::cout << "m6:{";
    for (size_t i = 0; i < obj.m6.size(); i++) {
        std::cout << obj.m6[i] << ", ";
    }
    std::cout << "},\n";

    std::cout << "m7:{";
    for (size_t i = 0; i < obj.m7.size(); i++) {
        std::cout << obj.m7[i] << ", ";
    }
    std::cout << "},\n";

    std::cout << "m8:{";
    for (size_t i = 0; i < obj.m8.size(); i++) {
        std::cout << obj.m8[i] << ", ";
    }
    std::cout << "},\n";

    std::cout << "m9:{";
    for (size_t i = 0; i < obj.m9.size(); i++) {
        std::cout << obj.m9[i] << ", ";
    }
    std::cout << "},\n";

    std::cout << "m10:{";
    for (size_t i = 0; i < obj.m10.size(); i++) {
        std::cout << obj.m10[i] << ", ";
    }
    std::cout << "},\n";

    std::cout << "m11:{";
    for (size_t i = 0; i < obj.m11.size(); i++) {
        std::cout << obj.m11[i] << ", ";
    }
    std::cout << "},\n";

    std::cout << "m12:{";
    for (size_t i = 0; i < obj.m12.size(); i++) {
        std::cout << obj.m12[i] << ", ";
    }
    std::cout << "},\n";

    std::cout << "m13:{";
    for (size_t i = 0; i < obj.m13.size(); i++) {
        std::cout << ESampleToStr(obj.m13[i]) << ", ";
    }
    std::cout << "},\n";

    std::cout << "m14:{";
    for (size_t i = 0; i < obj.m14.size(); i++) {
        PrintSSample(obj.m14[i]);
        std::cout << ", \n";
    }
    std::cout << ",\n";

    std::cout << "m13:{";
    for (size_t i = 0; i < obj.m15.size(); i++) {
        std::cout << obj.m15[i] << ", ";
    }
    std::cout << "},\n";

    std::cout << "}";
}

HWTEST_F(StructTest, StructTest_006, TestSize.Level0)
{
    SSample5 srcObj = {
        {true, false},
        {65, 66},
        {3, 4},
        {5, 6},
        {7, 8},
        {97, 98},
        {30, 40},
        {50, 60},
        {70, 80},
        {10.5, 20.5},
        {30.125, 30.125},
        {"hello", "world"},
        {ESample::MEM_ONE, ESample::MEM_THREE},
        {{true, 1, 1000.125, "hello"}, {false, 1, 1000.125, "world"}},
        {sptr<SequenceData>(new SequenceData(1, 1.2, "hello")), sptr<SequenceData>(new SequenceData(2, 2.2, "world"))},
    };

    SSample5 destObj;

    int32_t ec = g_testClient->SSample5Test(srcObj, destObj);
    ASSERT_EQ(ec, HDF_SUCCESS);

    for (size_t i = 0; i < srcObj.m1.size(); i++) {
        EXPECT_EQ((srcObj.m1[i] ? 1 : 0), (destObj.m1[i] ? 1 : 0));
    }

    for (size_t i = 0; i < srcObj.m2.size(); i++) {
        EXPECT_EQ(srcObj.m2[i], destObj.m2[i]);
    }

    for (size_t i = 0; i < srcObj.m3.size(); i++) {
        EXPECT_EQ(srcObj.m3[i], destObj.m3[i]);
    }

    for (size_t i = 0; i < srcObj.m4.size(); i++) {
        EXPECT_EQ(srcObj.m4[i], destObj.m4[i]);
    }

    for (size_t i = 0; i < srcObj.m5.size(); i++) {
        EXPECT_EQ(srcObj.m5[i], destObj.m5[i]);
    }

    for (size_t i = 0; i < srcObj.m6.size(); i++) {
        EXPECT_EQ(srcObj.m6[i], destObj.m6[i]);
    }

    for (size_t i = 0; i < srcObj.m7.size(); i++) {
        EXPECT_EQ(srcObj.m7[i], destObj.m7[i]);
    }

    for (size_t i = 0; i < srcObj.m8.size(); i++) {
        EXPECT_EQ(srcObj.m8[i], destObj.m8[i]);
    }

    for (size_t i = 0; i < srcObj.m9.size(); i++) {
        EXPECT_EQ(srcObj.m9[i], destObj.m9[i]);
    }

    for (size_t i = 0; i < srcObj.m10.size(); i++) {
        EXPECT_FLOAT_EQ(srcObj.m10[i], destObj.m10[i]);
    }

    for (size_t i = 0; i < srcObj.m11.size(); i++) {
        EXPECT_DOUBLE_EQ(srcObj.m11[i], destObj.m11[i]);
    }

    for (size_t i = 0; i < srcObj.m12.size(); i++) {
        EXPECT_EQ(srcObj.m12[i], destObj.m12[i]);
    }

    for (size_t i = 0; i < srcObj.m13.size(); i++) {
        EXPECT_EQ(srcObj.m13[i], destObj.m13[i]);
    }

    for (size_t i = 0; i < srcObj.m14.size(); i++) {
        EXPECT_EQ(((srcObj.m14[i]).m1 ? 1 : 0), ((destObj.m14[i]).m1 ? 1 : 0));
        EXPECT_EQ((srcObj.m14[i]).m2, (destObj.m14[i]).m2);
        EXPECT_DOUBLE_EQ((srcObj.m14[i]).m3, (destObj.m14[i]).m3);
        EXPECT_EQ((srcObj.m14[i]).m4, (destObj.m14[i]).m4);
    }

    for (size_t i = 0; i < srcObj.m15.size(); i++) {
        sptr<SequenceData> var1 = srcObj.m15[i];
        sptr<SequenceData> var2 = destObj.m15[i];

        if (var1 != nullptr && var2 != nullptr) {
            EXPECT_EQ(var1->m1_, var2->m1_);
            EXPECT_DOUBLE_EQ(var1->m2_, var2->m2_);
            EXPECT_EQ(var1->m3_, var2->m3_);
        } else {
            std::cout << "var1 or var2 is nullptr" << std::endl;
        }
    }

    PrintSSample5(srcObj);
    PrintSSample5(destObj);
    std::cout << "\n";
    std::cout << "--------------------------------------\n";
}