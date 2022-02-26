/*
 * Copyright (C) 2021 Huawei Device Co., Ltd.
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
#include <cstdarg>
#include <gtest/gtest.h>

#include "hitrace/hitracec.h"
#include "file_utils.h"

using namespace testing::ext;
using namespace std;

class HitraceCtest : public testing::Test {
public:

    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();

private:
};
void HitraceCtest::SetUp()
{
}
void HitraceCtest::TearDown()
{
}
void HitraceCtest::SetUpTestCase()
{
}
void HitraceCtest::TearDownTestCase()
{
}

/**
 * @tc.name HiTrace C interface test
 * @tc.number DFX_DFT_HiTrace_0001
 * @tc.desc HiTrace C interface test
*/
HWTEST_F(HitraceCtest, HitraceC_interface, Function|MediumTest|Level1)
{
    GTEST_LOG_(INFO) << "HitraceC_interface start" << endl;
    HiTraceIdStruct hiTraceId;
    HiTraceFlag hitraceflag = HITRACE_FLAG_INCLUDE_ASYNC;
    hiTraceId = HiTraceBegin("test", HITRACE_FLAG_INCLUDE_ASYNC);

    HiTraceSetId(&hiTraceId);
    hiTraceId = HiTraceGetId();
    ASSERT_TRUE(HiTraceIsValid(&hiTraceId));

    HiTraceEnableFlag(&hiTraceId, hitraceflag);
    ASSERT_TRUE(HiTraceIsFlagEnabled(&hiTraceId, hitraceflag));

    int hitraceflags = HITRACE_FLAG_FAULT_TRIGGER;
    HiTraceSetFlags(&hiTraceId, hitraceflags);
    hitraceflags = HiTraceGetFlags(&hiTraceId);
    ASSERT_TRUE(hitraceflags == HITRACE_FLAG_FAULT_TRIGGER);

    uint64_t chainId = 10000;
    HiTraceSetChainId(&hiTraceId, chainId);
    chainId = HiTraceGetChainId(&hiTraceId);
    ASSERT_TRUE(chainId==10000);

    uint64_t spanId = 12345678;
    HiTraceSetSpanId(&hiTraceId, spanId);
    spanId = HiTraceGetSpanId(&hiTraceId);
    ASSERT_TRUE(spanId == 12345678);

    HiTraceIdStruct childId = HiTraceCreateSpan();
    ASSERT_TRUE(HiTraceIsValid(&childId));

    /* set child id to thread id */
    HiTraceSetId(&childId);
    /* continue to create child span */
    HiTraceIdStruct grandChildId = HiTraceCreateSpan();
    ASSERT_TRUE(HiTraceIsValid(&grandChildId));
    EXPECT_EQ(HiTraceGetFlags(&grandChildId), HiTraceGetFlags(&childId));
    EXPECT_EQ(HiTraceGetChainId(&grandChildId), HiTraceGetChainId(&childId));
    EXPECT_EQ(HiTraceGetParentSpanId(&grandChildId), HiTraceGetSpanId(&childId));

    HiTraceSetChainId(&grandChildId, chainId);
    chainId = HiTraceGetChainId(&grandChildId);
    ASSERT_TRUE(chainId==10000);

    HiTraceSetSpanId(&grandChildId, spanId);
    spanId = HiTraceGetSpanId(&grandChildId);
    ASSERT_TRUE(spanId==12345678);
    HiTraceClearId();
    hiTraceId = HiTraceGetId();
    ASSERT_FALSE(HiTraceIsValid(&hiTraceId));
    HiTraceEnd(&hiTraceId);
    GTEST_LOG_(INFO) << "HitraceC_interface end" << endl;
}
/**
 * @tc.name HiTraceBegin interface test, when flag is invalid
 * @tc.number DFX_DFT_HiTrace_0002
 * @tc.desc HiTrace C interface test
 */
HWTEST_F(HitraceCtest, HitraceC_interface1, Function|MediumTest|Level1)
{
    GTEST_LOG_(INFO) << "HitraceC_interface1 start" << endl;
    /* begin with invalid flag */
    HiTraceIdStruct invalidFlagId = HiTraceBegin("invalid param", HITRACE_FLAG_MAX+1);
    EXPECT_EQ(0, HiTraceIsValid(&invalidFlagId));
    HiTraceEnd(&invalidFlagId);

    invalidFlagId = HiTraceBegin("invalid param1", -1);
    EXPECT_EQ(0, HiTraceIsValid(&invalidFlagId));
    HiTraceEnd(&invalidFlagId);
    GTEST_LOG_(INFO) << "HitraceC_interface1 end" << endl;
}
/**
 * @tc.name HiTraceBegin interface test, when name is invalid.
 * @tc.number DFX_DFT_HiTrace_0003
 * @tc.desc HiTrace C interface test
 */
HWTEST_F(HitraceCtest, HitraceC_interface2, Function|MediumTest|Level1)
{
    GTEST_LOG_(INFO) << "HitraceC_interface2 start" << endl;
    /* begin with invalid name */
    HiTraceIdStruct invalidNameId = HiTraceBegin("", HITRACE_FLAG_TP_INFO);
    EXPECT_EQ(1, HiTraceIsValid(&invalidNameId));
    HiTraceEnd(&invalidNameId);
    GTEST_LOG_(INFO) << "HitraceC_interface2 end" << endl;
}
/**
 * @tc.name HiTraceBegin interface test, when name is very long.
 * @tc.number DFX_DFT_HiTrace_0004
 * @tc.desc HiTrace C++ interface test
 */
HWTEST_F(HitraceCtest, Hitrace_interface3, Function|MediumTest|Level1)
{
    GTEST_LOG_(INFO) << "Hitrace_interface3 start" << endl;
    char name[4096] = "test";
    for (int i = 0; i < 1000; i++)
    {
        strcat(name, "test");
    }
    HiTraceIdStruct traceId = HiTraceBegin(name, HITRACE_FLAG_TP_INFO);
    EXPECT_EQ(1, HiTraceIsValid(&traceId));
    HiTraceEnd(&traceId);
    GTEST_LOG_(INFO) << "Hitrace_interface3 end" << endl;
}
/**
 * @tc.name Repeated call HiTraceBegin interface test.
 * @tc.number DFX_DFT_HiTrace_0005
 * @tc.desc HiTrace C interface test
 */
HWTEST_F(HitraceCtest, Hitrace_interface4, Function|MediumTest|Level1)
{
    GTEST_LOG_(INFO) << "Hitrace_interface4 start" << endl;
    HiTraceIdStruct traceId = HiTraceBegin("test", HITRACE_FLAG_TP_INFO);
    HiTraceIdStruct traceId1 = HiTraceBegin("test1", HITRACE_FLAG_TP_INFO);
    EXPECT_EQ(1, HiTraceIsValid(&traceId));
    EXPECT_EQ(0, HiTraceIsValid(&traceId1));
    HiTraceEnd(&traceId);
    HiTraceEnd(&traceId1);
    GTEST_LOG_(INFO) << "Hitrace_interface4 end" << endl;
}
/**
 * @tc.name End invalid traceid test.
 * @tc.number DFX_DFT_HiTrace_0006
 * @tc.desc HiTrace C interface test
 */
HWTEST_F(HitraceCtest, HitraceC_interface5, Function|MediumTest|Level1)
{
    GTEST_LOG_(INFO) << "HitraceC_interface5 start" << endl;
    HiTraceIdStruct traceId = HiTraceBegin("test", HITRACE_FLAG_TP_INFO);
    HiTraceIdStruct invalidId = {0};
    EXPECT_EQ(1, HiTraceIsValid(&traceId));
    HiTraceEnd(&invalidId);
    HiTraceIdStruct wrongEndId = HiTraceGetId();
    EXPECT_EQ(1, HiTraceIsValid(&wrongEndId));
    EXPECT_EQ(HiTraceGetChainId(&wrongEndId), HiTraceGetChainId(&traceId));
    EXPECT_EQ(1, HiTraceIsFlagEnabled(&wrongEndId, HITRACE_FLAG_TP_INFO));
    HiTraceEnd(&traceId);
    GTEST_LOG_(INFO) << "HitraceC_interface5 end" << endl;
}
/**
 * @tc.name End traceId that has ended.
 * @tc.number DFX_DFT_HiTrace_0007
 * @tc.desc HiTrace C interface test
 */
HWTEST_F(HitraceCtest, HitraceC_interface6, Function|MediumTest|Level1)
{
    GTEST_LOG_(INFO) << "HitraceC_interface6 start" << endl;
    HiTraceIdStruct traceId = HiTraceBegin("test", HITRACE_FLAG_TP_INFO);
    EXPECT_EQ(1, HiTraceIsValid(&traceId));
    HiTraceEnd(&traceId);
    traceId = HiTraceGetId();
    EXPECT_EQ(0, HiTraceIsValid(&traceId));
    HiTraceEnd(&traceId);
    traceId = HiTraceGetId();
    EXPECT_EQ(0, HiTraceIsValid(&traceId));
    GTEST_LOG_(INFO) << "HitraceC_interface6 end" << endl;
}