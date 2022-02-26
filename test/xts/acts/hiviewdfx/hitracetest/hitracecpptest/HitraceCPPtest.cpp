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
#include <ctime>
#include <gtest/gtest.h>
#include <pthread.h>

#include "hitrace/hitrace.h"
#include "hitrace/hitraceid.h"
#include "hitrace/trace.h"
#include "file_utils.h"

using namespace OHOS;
using namespace HiviewDFX;
using namespace testing;
using namespace testing::ext;
using namespace std;

class HitraceCPPtest : public testing::Test {
public:

    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();

private:
};
void HitraceCPPtest::SetUp()
{
}
void HitraceCPPtest::TearDown()
{
}
void HitraceCPPtest::SetUpTestCase()
{
}
void HitraceCPPtest::TearDownTestCase()
{
}
/**
 * @tc.name HiTrace C++ interface test
 * @tc.number DFX_DFT_HiTrace_CPP_0001
 * @tc.desc HiTrace C++ interface test
 */
HWTEST_F(HitraceCPPtest, Hitrace_interface, Function|MediumTest|Level1)
{
    GTEST_LOG_(INFO) << "Hitrace_interface start" << endl;
    HiTraceFlag hitraceflag = HITRACE_FLAG_INCLUDE_ASYNC;
    HiTraceId hiTraceId = HiTrace::Begin("test", HITRACE_FLAG_INCLUDE_ASYNC);
    HiTrace::SetId(hiTraceId);
    hiTraceId = HiTrace::GetId();
    ASSERT_TRUE(hiTraceId.IsValid());

    hiTraceId.EnableFlag(hitraceflag);
    ASSERT_TRUE(hiTraceId.IsFlagEnabled(hitraceflag));

    int hitraceflags = HITRACE_FLAG_FAULT_TRIGGER;
    hiTraceId.SetFlags(hitraceflags);
    hitraceflags = hiTraceId.GetFlags();
    ASSERT_TRUE(hitraceflags == HITRACE_FLAG_FAULT_TRIGGER);

    uint64_t chainId = 10000;
    hiTraceId.SetChainId(chainId);
    chainId = hiTraceId.GetChainId();
    ASSERT_TRUE(chainId == 10000);

    uint64_t spanId = 12345678;
    hiTraceId.SetSpanId(spanId);
    spanId = hiTraceId.GetSpanId();
    ASSERT_TRUE(spanId == 12345678);

    HiTraceId childId = HiTrace::CreateSpan();
    EXPECT_EQ(1, childId.IsValid());

    /* set child id to thread id */
    HiTrace::SetId(childId);
    /* continue to create child span */
    HiTraceId grandChildId = HiTrace::CreateSpan();
    EXPECT_EQ(1, grandChildId.IsValid());
    EXPECT_EQ(grandChildId.GetFlags(), childId.GetFlags());
    EXPECT_EQ(grandChildId.GetChainId(), childId.GetChainId());
    EXPECT_EQ(grandChildId.GetParentSpanId(), childId.GetSpanId());

    grandChildId.SetChainId(chainId);
    chainId = grandChildId.GetChainId();
    ASSERT_TRUE(chainId == 10000);

    grandChildId.SetSpanId(spanId);
    spanId = grandChildId.GetSpanId();
    ASSERT_TRUE(spanId == 12345678);
    HiTrace::ClearId();
    HiTraceId clearId = HiTrace::GetId();
    EXPECT_EQ(0, clearId.IsValid());
    HiTrace::End(hiTraceId);
    GTEST_LOG_(INFO) << "Hitrace_interface end" << endl;
}

/**
 * @tc.name HiTrace C++ Begin interface, when flag is invalid.
 * @tc.number DFX_DFT_HiTrace_CPP_0002
 * @tc.desc HiTrace C++ interface test
 */
HWTEST_F(HitraceCPPtest, Hitrace_interface1, Function|MediumTest|Level1)
{
    GTEST_LOG_(INFO) << "Hitrace_interface1 start" << endl;
    /* begin with invalid flag */
    HiTraceId invalidFlagId = HiTrace::Begin("invalid param", HITRACE_FLAG_MAX+1);
    EXPECT_EQ(0, invalidFlagId.IsValid());
    HiTrace::End(invalidFlagId);

    invalidFlagId = HiTrace::Begin("invalid param", -1);
    EXPECT_EQ(0, invalidFlagId.IsValid());
    HiTrace::End(invalidFlagId);
    GTEST_LOG_(INFO) << "Hitrace_interface1 end" << endl;
}
/**
 * @tc.name HiTrace C++ Begin interface, when name is invalid.
 * @tc.number DFX_DFT_HiTrace_CPP_0003
 * @tc.desc HiTrace C++ interface test
 */
HWTEST_F(HitraceCPPtest, Hitrace_interface2, Function|MediumTest|Level1)
{
    GTEST_LOG_(INFO) << "Hitrace_interface2 start" << endl;
    /* begin with invalid name */
    HiTraceId invalidNameId = HiTrace::Begin("", HITRACE_FLAG_TP_INFO);
    EXPECT_EQ(1, invalidNameId.IsValid());
    HiTrace::End(invalidNameId);
    GTEST_LOG_(INFO) << "Hitrace_interface2 end" << endl;
}
/**
 * @tc.name HiTrace C++ Begin interface, when name is very long.
 * @tc.number DFX_DFT_HiTrace_CPP_0004
 * @tc.desc HiTrace C++ interface test
 */
HWTEST_F(HitraceCPPtest, Hitrace_interface3, Function|MediumTest|Level1)
{
    GTEST_LOG_(INFO) << "Hitrace_interface3 start" << endl;
    std::string name = "test";
    for (int i = 0; i < 1000; i++) {
        name += "test";
    }
    HiTraceId traceId = HiTrace::Begin(name, HITRACE_FLAG_TP_INFO);
    EXPECT_EQ(1, traceId.IsValid());
    HiTrace::End(traceId);
    GTEST_LOG_(INFO) << "Hitrace_interface3 end" << endl;
}
/**
 * @tc.name Repeated call Begin interface.
 * @tc.number DFX_DFT_HiTrace_CPP_0005
 * @tc.desc HiTrace C++ interface test
 */
HWTEST_F(HitraceCPPtest, Hitrace_interface4, Function|MediumTest|Level1)
{
    GTEST_LOG_(INFO) << "Hitrace_interface4 start" << endl;
    HiTraceId traceId = HiTrace::Begin("test", HITRACE_FLAG_TP_INFO);
    HiTraceId traceId1 = HiTrace::Begin("test1", HITRACE_FLAG_TP_INFO);
    EXPECT_EQ(1, traceId.IsValid());
    EXPECT_EQ(0, traceId1.IsValid());
    HiTrace::End(traceId);
    HiTrace::End(traceId1);
    GTEST_LOG_(INFO) << "Hitrace_interface4 end" << endl;
}
/**
 * @tc.name End invalid traceid
 * @tc.number DFX_DFT_HiTrace_CPP_0006
 * @tc.desc HiTrace C++ interface test
 */
HWTEST_F(HitraceCPPtest, Hitrace_interface5, Function|MediumTest|Level1)
{
    GTEST_LOG_(INFO) << "Hitrace_interface5 start" << endl;
    HiTraceId traceId = HiTrace::Begin("test", HITRACE_FLAG_TP_INFO);
    EXPECT_EQ(1, traceId.IsValid());
    HiTraceId invalidId;
    invalidId.SetChainId(1000);
    invalidId.SetSpanId(1000);
    HiTrace::End(invalidId);
    HiTraceId wrongEndId = HiTrace::GetId();
    EXPECT_EQ(1, wrongEndId.IsValid());
    ASSERT_TRUE(wrongEndId.GetChainId() == traceId.GetChainId());
    EXPECT_EQ(1, wrongEndId.IsFlagEnabled(HITRACE_FLAG_TP_INFO));
    HiTrace::End(traceId);
    GTEST_LOG_(INFO) << "Hitrace_interface5 end" << endl;
}
/**
 * @tc.name End traceId that has ended.
 * @tc.number DFX_DFT_HiTrace_CPP_0007
 * @tc.desc HiTrace C++ interface test
 */
HWTEST_F(HitraceCPPtest, Hitrace_interface6, Function|MediumTest|Level1)
{
    GTEST_LOG_(INFO) << "Hitrace_interface6 start" << endl;
    HiTraceId traceId = HiTrace::Begin("invalid param1", HITRACE_FLAG_TP_INFO);
    EXPECT_EQ(1, traceId.IsValid());
    HiTrace::End(traceId);
    traceId = HiTrace::GetId();
    EXPECT_EQ(0, traceId.IsValid());
    HiTrace::End(traceId);
    traceId = HiTrace::GetId();
    EXPECT_EQ(0, traceId.IsValid());
    GTEST_LOG_(INFO) << "Hitrace_interface6 end" << endl;
}