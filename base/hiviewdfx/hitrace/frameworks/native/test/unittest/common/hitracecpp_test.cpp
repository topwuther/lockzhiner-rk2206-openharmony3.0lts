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

#include "hitrace/hitrace.h"

#include <gtest/gtest.h>

namespace OHOS {
namespace HiviewDFX {

using namespace testing::ext;

class HiTraceCppTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();
};

void HiTraceCppTest::SetUpTestCase()
{}

void HiTraceCppTest::TearDownTestCase()
{}

void HiTraceCppTest::SetUp()
{
    HiTrace::ClearId();
}

void HiTraceCppTest::TearDown()
{}

/**
 * @tc.name: Dfx_HiTraceCppTest_IdTest_001
 * @tc.desc: Get, set and clear trace id
 * @tc.type: FUNC
 * @tc.require: AR000CQV9U
 */
HWTEST_F(HiTraceCppTest, IdTest_001, TestSize.Level1)
{
    /**
     * @tc.steps: step1. get and validate trace id.
     * @tc.expected: step1. trace id is invalid.
     * @tc.steps: step2. construct trace id with chain id, span id, parent span id
     *     and flags and set it into context, then get and validate it.
     * @tc.expected: step2. trace id is valid with same chain id, span id, parent
     *     span id and flags.
     * @tc.steps: step3. construct invalid trace id and set into context, then get
     *     and validate it.
     * @tc.expected: step3. trace id is the same with step2.
     * @tc.steps: step4. clear trace id, then get and validate it.
     * @tc.expected: step4. trace id is invalid.
     */
    HiTraceId initId = HiTrace::GetId();
    EXPECT_EQ(0, initId.IsValid());
    /* set thread id */
    constexpr uint64_t CHAIN_ID = 0xABCDEF;
    constexpr uint64_t SPAN_ID = 0x12345;
    constexpr uint64_t PARENT_SPAN_ID = 0x67890;

    initId.SetChainId(CHAIN_ID);
    initId.EnableFlag(HITRACE_FLAG_INCLUDE_ASYNC);
    initId.EnableFlag(HITRACE_FLAG_DONOT_CREATE_SPAN);
    initId.SetSpanId(SPAN_ID);
    initId.SetParentSpanId(PARENT_SPAN_ID);

    EXPECT_EQ(1, initId.IsValid());
    EXPECT_EQ(CHAIN_ID, initId.GetChainId());
    EXPECT_EQ(HITRACE_FLAG_INCLUDE_ASYNC | HITRACE_FLAG_DONOT_CREATE_SPAN, initId.GetFlags());
    EXPECT_EQ(SPAN_ID, initId.GetSpanId());
    EXPECT_EQ(PARENT_SPAN_ID, initId.GetParentSpanId());

    HiTrace::SetId(initId);

    HiTraceId getId = HiTrace::GetId();
    EXPECT_EQ(1, getId.IsValid());
    EXPECT_EQ(CHAIN_ID, getId.GetChainId());
    EXPECT_EQ(HITRACE_FLAG_INCLUDE_ASYNC | HITRACE_FLAG_DONOT_CREATE_SPAN, getId.GetFlags());
    EXPECT_EQ(SPAN_ID, getId.GetSpanId());
    EXPECT_EQ(PARENT_SPAN_ID, getId.GetParentSpanId());

    HiTraceId invalidId;
    HiTrace::SetId(invalidId);

    getId = HiTrace::GetId();
    EXPECT_EQ(1, getId.IsValid());
    EXPECT_EQ(CHAIN_ID, getId.GetChainId());
    EXPECT_EQ(HITRACE_FLAG_INCLUDE_ASYNC | HITRACE_FLAG_DONOT_CREATE_SPAN, getId.GetFlags());
    EXPECT_EQ(SPAN_ID, getId.GetSpanId());
    EXPECT_EQ(PARENT_SPAN_ID, getId.GetParentSpanId());

    HiTrace::ClearId();
    HiTraceId clearId = HiTrace::GetId();
    EXPECT_EQ(0, clearId.IsValid());
}

/**
 * @tc.name: Dfx_HiTraceCppTest_IntfTest_001
 * @tc.desc: Interconversion between trace id and bytes array.
 * @tc.type: FUNC
 * @tc.require: AR000CQV9U
 */
HWTEST_F(HiTraceCppTest, IntfTest_001, TestSize.Level1)
{
    /**
     * @tc.steps: step1. get trace id and validate it.
     * @tc.expected: step1. trace id is invalid.
     * @tc.steps: step2. construct trace id and validate it.
     * @tc.expected: step2. trace id is valid.
     * @tc.steps: step3. convert trace id to bytes array.
     * @tc.expected: step3. convert success when array size >= id length.
     * @tc.steps: step4. convert bytes array to trace id.
     * @tc.expected: step4. convert success only when array size == id length.
     * @tc.steps: step5. convert invalid id to bytes array.
     * @tc.expected: step5. convert fail.
     * @tc.steps: step6. convert invalid bytes array to id.
     * @tc.expected: step6. convert fail.
     */
    HiTraceId initId = HiTrace::GetId();
    EXPECT_EQ(0, initId.IsValid());
    constexpr uint64_t CHAIN_ID = 0xABCDEF;
    constexpr uint64_t SPAN_ID = 0x12345;
    constexpr uint64_t PARENT_SPAN_ID = 0x67890;
    constexpr int FLAGS = HITRACE_FLAG_INCLUDE_ASYNC | HITRACE_FLAG_DONOT_CREATE_SPAN;

    initId.SetChainId(CHAIN_ID);
    initId.SetFlags(FLAGS);
    initId.SetSpanId(SPAN_ID);
    initId.SetParentSpanId(PARENT_SPAN_ID);
    EXPECT_EQ(1, initId.IsValid());
    constexpr int ID_LEN = sizeof(HiTraceIdStruct);

    uint8_t bytes[ID_LEN + 1];
    int len = initId.ToBytes(bytes, ID_LEN - 1);
    EXPECT_EQ(0, len);
    len = initId.ToBytes(bytes, ID_LEN + 1);
    EXPECT_EQ(ID_LEN, len);
    len = initId.ToBytes(bytes, ID_LEN);
    EXPECT_EQ(ID_LEN, len);

    /* bytes to id */
    HiTraceId bytesToId = HiTraceId(bytes, ID_LEN - 1);
    EXPECT_EQ(0, bytesToId.IsValid());
    bytesToId = HiTraceId(bytes, ID_LEN + 1);
    EXPECT_EQ(0, bytesToId.IsValid());
    bytesToId = HiTraceId(bytes, ID_LEN);
    EXPECT_EQ(1, bytesToId.IsValid());
    EXPECT_EQ(CHAIN_ID, bytesToId.GetChainId());
    EXPECT_EQ(HITRACE_FLAG_INCLUDE_ASYNC | HITRACE_FLAG_DONOT_CREATE_SPAN, bytesToId.GetFlags());
    EXPECT_EQ(SPAN_ID, bytesToId.GetSpanId());
    EXPECT_EQ(PARENT_SPAN_ID, bytesToId.GetParentSpanId());

    /* set invalid id */
    HiTraceId invalidId;
    EXPECT_EQ(0, invalidId.ToBytes(bytes, ID_LEN));
    invalidId = HiTraceId(nullptr, ID_LEN);
    EXPECT_EQ(0, invalidId.IsValid());
}

/**
 * @tc.name: Dfx_HiTraceCppTest_IntfTest_002
 * @tc.desc: Start and stop trace.
 * @tc.type: FUNC
 * @tc.require: AR000CQV9U
 */
HWTEST_F(HiTraceCppTest, IntfTest_002, TestSize.Level1)
{
    /**
     * @tc.steps: step1. start trace with flags, get trace id and validit it.
     * @tc.expected: step1. trace id and flags is valid.
     * @tc.steps: step2. stop trace, get trace id and validit it.
     * @tc.expected: step2. trace id is invalid.
     */
    HiTraceId beginId = HiTrace::Begin("test", HITRACE_FLAG_INCLUDE_ASYNC | HITRACE_FLAG_NO_BE_INFO);
    EXPECT_EQ(1, beginId.IsValid());
    EXPECT_EQ(1, beginId.IsFlagEnabled(HITRACE_FLAG_INCLUDE_ASYNC));
    EXPECT_EQ(1, beginId.IsFlagEnabled(HITRACE_FLAG_NO_BE_INFO));

    HiTrace::End(beginId);

    HiTraceId endId = HiTrace::GetId();
    EXPECT_EQ(0, endId.IsValid());
}

/**
 * @tc.name: Dfx_HiTraceCppTest_IntfTest_003
 * @tc.desc: Start and stop trace with reentered.
 * @tc.type: FUNC
 * @tc.require: AR000CQV9U
 */
HWTEST_F(HiTraceCppTest, IntfTest_003, TestSize.Level1)
{
    /**
     * @tc.steps: step1. start trace twice and get 2nd trace id.
     * @tc.expected: step1. 2nd trace is invalid.
     * @tc.steps: step2. get trace id and check.
     * @tc.expected: step2. trace id is valid and same with 1st id.
     * @tc.steps: step3. set chain id with wrong id and get trace id.
     * @tc.expected: step3. trace id is valid and same with 1st id.
     * @tc.steps: step4. stop trace twice and get trace id.
     * @tc.expected: step4. trace id is invalid.
     */
    /* begin */
    HiTraceId beginId = HiTrace::Begin("begin", HITRACE_FLAG_INCLUDE_ASYNC);
    /* reenter begin */
    HiTraceId reBeginId = HiTrace::Begin("reenter begin", HITRACE_FLAG_TP_INFO);
    EXPECT_EQ(0, reBeginId.IsValid());
    EXPECT_NE(reBeginId.GetChainId(), beginId.GetChainId());
    EXPECT_EQ(0, reBeginId.IsFlagEnabled(HITRACE_FLAG_INCLUDE_ASYNC));
    EXPECT_EQ(0, reBeginId.IsFlagEnabled(HITRACE_FLAG_TP_INFO));

    /* reenter end */
    HiTrace::End(reBeginId);

    HiTraceId endId = HiTrace::GetId();
    EXPECT_EQ(1, endId.IsValid());
    EXPECT_EQ(endId.GetChainId(), beginId.GetChainId());
    EXPECT_EQ(1, endId.IsFlagEnabled(HITRACE_FLAG_INCLUDE_ASYNC));
    EXPECT_EQ(0, endId.IsFlagEnabled(HITRACE_FLAG_TP_INFO));

    /* end with wrong chainId */
    HiTraceId wrongBeginId = beginId;
    wrongBeginId.SetChainId(beginId.GetChainId() + 1);
    HiTrace::End(wrongBeginId);

    HiTraceId wrongEndId = HiTrace::GetId();
    EXPECT_EQ(1, wrongEndId.IsValid());
    EXPECT_EQ(wrongEndId.GetChainId(), beginId.GetChainId());
    EXPECT_EQ(1, wrongEndId.IsFlagEnabled(HITRACE_FLAG_INCLUDE_ASYNC));

    /* end */
    HiTrace::End(beginId);

    HiTraceId reEndId = HiTrace::GetId();
    EXPECT_EQ(0, reEndId.IsValid());

    /* end with invalid thread id */
    HiTrace::End(beginId);

    HiTraceId endInvalidId = HiTrace::GetId();
    EXPECT_EQ(0, endInvalidId.IsValid());
}

/**
 * @tc.name: Dfx_HiTraceCppTest_SpanTest_001
 * @tc.desc: Create child and grand child span.
 * @tc.type: FUNC
 * @tc.require: AR000CQV9U
 */
HWTEST_F(HiTraceCppTest, SpanTest_001, TestSize.Level1)
{
    /**
     * @tc.steps: step1. start trace without HITRACE_FLAG_DONOT_CREATE_SPAN,
     *     get and check flags.
     * @tc.expected: step1. flags is same with set and span id is 0.
     * @tc.steps: step2. create child id.
     * @tc.expected: step2. child id has same span id with parent.
     * @tc.steps: step3. set child id into context.
     * @tc.steps: step4. create grand child id.
     * @tc.expected: step4. grand child id has same span id with parent and child.
     */
    /* begin with span flag */
    HiTraceId id = HiTrace::Begin("test", 0);
    EXPECT_EQ(0, id.GetFlags());
    EXPECT_EQ(0UL, id.GetSpanId());
    EXPECT_EQ(0UL, id.GetParentSpanId());

    /* create child span */
    HiTraceId childId = HiTrace::CreateSpan();
    EXPECT_EQ(1, childId.IsValid());
    EXPECT_EQ(childId.GetFlags(), id.GetFlags());
    EXPECT_EQ(childId.GetChainId(), id.GetChainId());
    EXPECT_EQ(childId.GetParentSpanId(), id.GetSpanId());

    /* set child id to thread id */
    HiTrace::SetId(childId);

    /* continue to create child span */
    HiTraceId grandChildId = HiTrace::CreateSpan();
    EXPECT_EQ(1, grandChildId.IsValid());
    EXPECT_EQ(grandChildId.GetFlags(), id.GetFlags());
    EXPECT_EQ(grandChildId.GetChainId(), id.GetChainId());
    EXPECT_EQ(grandChildId.GetParentSpanId(), childId.GetSpanId());

    /* end */
    HiTrace::End(id);
}

/**
 * @tc.name: Dfx_HiTraceCppTest_SpanTest_002
 * @tc.desc: Start and stop trace with reentered.
 * @tc.type: FUNC
 * @tc.require: AR000CQV9U
 */
HWTEST_F(HiTraceCppTest, SpanTest_002, TestSize.Level1)
{
    /**
     * @tc.steps: step1. start trace with HITRACE_FLAG_DONOT_CREATE_SPAN,
     *     get and check flags.
     * @tc.expected: step1. HITRACE_FLAG_DONOT_CREATE_SPAN is enabled.
     * @tc.steps: step2. create child id.
     * @tc.expected: step2. child id is same with parent id.
     */
    /* begin with "donot create span" flag */
    HiTraceId id = HiTrace::Begin("test", HITRACE_FLAG_DONOT_CREATE_SPAN);
    EXPECT_EQ(1, id.IsFlagEnabled(HITRACE_FLAG_DONOT_CREATE_SPAN));

    /* create child span */
    HiTraceId childId = HiTrace::CreateSpan();
    EXPECT_EQ(1, childId.IsValid());
    EXPECT_EQ(childId.GetFlags(), id.GetFlags());
    EXPECT_EQ(childId.GetChainId(), id.GetChainId());
    EXPECT_EQ(childId.GetSpanId(), id.GetSpanId());
    EXPECT_EQ(childId.GetParentSpanId(), id.GetParentSpanId());

    /* end */
    HiTrace::End(id);
}

/**
 * @tc.name: Dfx_HiTraceCppTest_TracepointTest_001
 * @tc.desc: Start trace with HITRACE_FLAG_TP_INFO flag.
 * @tc.type: FUNC
 * @tc.require: AR000CQVA3
 */
HWTEST_F(HiTraceCppTest, TracepointTest_001, TestSize.Level1)
{
    /**
     * @tc.steps: step1. start trace with HITRACE_FLAG_TP_INFO,
     *     get and check flags.
     * @tc.expected: step1. HITRACE_FLAG_TP_INFO is enabled.
     * @tc.steps: step2. add trace point info with id and check logs.
     * @tc.expected: step2. trace point can be found in logs.
     * @tc.steps: step3. add trace point info with null and check logs.
     * @tc.expected: step3. trace point cannot be found in logs.
     */
    /* begin with tp flag */
    HiTraceId invalidId;
    HiTraceId id = HiTrace::Begin("test tp flag", HITRACE_FLAG_TP_INFO);
    EXPECT_EQ(1, id.IsFlagEnabled(HITRACE_FLAG_TP_INFO));
    HiTrace::Tracepoint(HITRACE_TP_CS, id, "client send msg content %d", 12);
    HiTrace::Tracepoint(HITRACE_TP_CS, invalidId, "client send msg content %d", 12);
    HiTrace::End(id);
}

/**
 * @tc.name: Dfx_HiTraceCppTest_TracepointTest_002
 * @tc.desc: Start trace without HITRACE_FLAG_TP_INFO flag.
 * @tc.type: FUNC
 * @tc.require: AR000CQVA3
 */
HWTEST_F(HiTraceCppTest, TracepointTest_002, TestSize.Level1)
{
    /**
     * @tc.steps: step1. start trace without HITRACE_FLAG_TP_INFO flag.
     *     get and check flags.
     * @tc.expected: step1. HITRACE_FLAG_TP_INFO is not enabled.
     * @tc.steps: step2. add trace point info with id and check logs.
     * @tc.expected: step2. trace point cannot be found in logs.
     */
    /* begin with tp flag */
    HiTraceId id = HiTrace::Begin("test no tp flag", HITRACE_FLAG_INCLUDE_ASYNC);
    EXPECT_EQ(0, id.IsFlagEnabled(HITRACE_FLAG_TP_INFO));
    HiTrace::Tracepoint(HITRACE_TP_CS, id, "client send msg content %d", 12);

    HiTrace::End(id);
}

/**
 * @tc.name: Dfx_HiTraceCppTest_TracepointTest_003
 * @tc.desc: Start trace with HITRACE_FLAG_D2D_TP_INFO flag.
 * @tc.type: FUNC
 * @tc.require: AR000CQVA3
 */
HWTEST_F(HiTraceCppTest, TracepointTest_003, TestSize.Level1)
{
    /**
     * @tc.steps: step1. start trace with HITRACE_FLAG_D2D_TP_INFO,
     *     get and check flags.
     * @tc.expected: step1. HITRACE_FLAG_D2D_TP_INFO is enabled.
     * @tc.steps: step2. add D2D trace point info with id and check logs.
     * @tc.expected: step2. trace point can be found in logs.
     * @tc.steps: step2. add D2D trace point info with null and check logs.
     * @tc.expected: step2. trace point cannot be found in logs.
     * @tc.steps: step3. add trace point info with id and check logs.
     * @tc.expected: step3. trace point cannot be found in logs.
     */
    HiTraceId id = HiTrace::Begin("test D2D tp flag", HITRACE_FLAG_D2D_TP_INFO);
    EXPECT_EQ(1, id.IsFlagEnabled(HITRACE_FLAG_D2D_TP_INFO));
    HiTrace::Tracepoint(HITRACE_CM_DEVICE, HITRACE_TP_CS, id, "client send msg content %d", 12);
    HiTrace::Tracepoint(HITRACE_CM_PROCESS, HITRACE_TP_CS, id, "cannot be found %d", 22);
    HiTrace::Tracepoint(HITRACE_CM_THREAD, HITRACE_TP_CS, id, "cannot be found %d", 32);
    HiTrace::Tracepoint(HITRACE_CM_DEFAULT, HITRACE_TP_CS, id, "cannot be found %d", 42);

    HiTraceId invalidId;
    HiTrace::Tracepoint(HITRACE_CM_DEVICE, HITRACE_TP_CS, invalidId, "cannot be found %d", 13);

    HiTrace::Tracepoint(HITRACE_TP_CS, id, "cannot be found %d", 14);

    HiTrace::End(id);
}

/**
 * @tc.name: Dfx_HiTraceCppTest_TracepointTest_004
 * @tc.desc: Start trace without HITRACE_FLAG_D2D_TP_INFO flag.
 * @tc.type: FUNC
 * @tc.require: AR000CQVA3
 */
HWTEST_F(HiTraceCppTest, TracepointTest_004, TestSize.Level1)
{
    /**
     * @tc.steps: step1. start trace without HITRACE_FLAG_D2D_TP_INFO flag.
     *     get and check flags.
     * @tc.expected: step1. HITRACE_FLAG_D2D_TP_INFO is not enabled.
     * @tc.steps: step2. add D2D trace point info with id and check logs.
     * @tc.expected: step2. trace point cannot be found in logs.
     */
    HiTraceId id = HiTrace::Begin("test no D2D tp flag", HITRACE_FLAG_INCLUDE_ASYNC);
    EXPECT_EQ(0, id.IsFlagEnabled(HITRACE_FLAG_D2D_TP_INFO));
    HiTrace::Tracepoint(HITRACE_CM_DEVICE, HITRACE_TP_CS, id, "cannot be found %d", 12);
    HiTrace::Tracepoint(HITRACE_CM_PROCESS, HITRACE_TP_CS, id, "cannot be found %d", 22);
    HiTrace::Tracepoint(HITRACE_CM_THREAD, HITRACE_TP_CS, id, "cannot be found %d", 32);
    HiTrace::Tracepoint(HITRACE_CM_DEFAULT, HITRACE_TP_CS, id, "cannot be found %d", 42);

    HiTrace::End(id);
}

/**
 * @tc.name: Dfx_HiTraceCppTest_TracepointTest_005
 * @tc.desc: Start trace with HITRACE_FLAG_D2D_TP_INFO and HITRACE_FLAG_TP_INFO flag.
 * @tc.type: FUNC
 * @tc.require: AR000CQVA3
 */
HWTEST_F(HiTraceCppTest, TracepointTest_005, TestSize.Level1)
{
    /**
     * @tc.steps: step1. start trace with HITRACE_FLAG_D2D_TP_INFO | HITRACE_FLAG_TP_INFO,
     *     get and check flags.
     * @tc.expected: step1. HITRACE_FLAG_D2D_TP_INFO is enabled.
     * @tc.expected: step1. HITRACE_FLAG_TP_INFO is enabled.
     * @tc.steps: step2. add D2D trace point info with id and check logs.
     * @tc.expected: step2. trace point can be found in logs.
     * @tc.steps: step3. add trace point info with id and check logs.
     * @tc.expected: step3. trace point can be found in logs.
     */
    HiTraceId id = HiTrace::Begin("test D2D | TP tp flag", HITRACE_FLAG_D2D_TP_INFO | HITRACE_FLAG_TP_INFO);
    EXPECT_EQ(1, id.IsFlagEnabled(HITRACE_FLAG_D2D_TP_INFO));
    EXPECT_EQ(1, id.IsFlagEnabled(HITRACE_FLAG_TP_INFO));
    HiTrace::Tracepoint(HITRACE_CM_DEVICE, HITRACE_TP_CS, id, "client send msg content %d", 12);
    HiTrace::Tracepoint(HITRACE_CM_PROCESS, HITRACE_TP_CS, id, "client send msg content %d", 22);
    HiTrace::Tracepoint(HITRACE_CM_THREAD, HITRACE_TP_CS, id, "client send msg content %d", 32);
    HiTrace::Tracepoint(HITRACE_CM_DEFAULT, HITRACE_TP_CS, id, "client send msg content %d", 42);

    HiTrace::Tracepoint(HITRACE_TP_CS, id, "client send msg content %d", 13);

    HiTrace::End(id);
}

/**
 * @tc.name: Dfx_HiTraceCppTest_TracepointTest_006
 * @tc.desc: Start trace without HITRACE_FLAG_D2D_TP_INFO, but with HITRACE_FLAG_TP_INFO flag.
 * @tc.type: FUNC
 * @tc.require: AR000CQVA3
 */
HWTEST_F(HiTraceCppTest, TracepointTest_006, TestSize.Level1)
{
    /**
     * @tc.steps: step1. start trace with HITRACE_FLAG_TP_INFO flag.
     *     get and check flags.
     * @tc.expected: step1. HITRACE_FLAG_D2D_TP_INFO is not enabled.
     * * @tc.expected: step1. HITRACE_FLAG_TP_INFO is enabled.
     * @tc.steps: step2. add D2D trace point info with id and check logs.
     * @tc.expected: step2. trace point can be found in logs.
     * @tc.steps: step2. add trace point info with id and check logs.
     * @tc.expected: step2. trace point can be found in logs.
     */
    HiTraceId id = HiTrace::Begin("test no D2D, but tp flag", HITRACE_FLAG_TP_INFO);
    EXPECT_EQ(0, id.IsFlagEnabled(HITRACE_FLAG_D2D_TP_INFO));
    EXPECT_EQ(1, id.IsFlagEnabled(HITRACE_FLAG_TP_INFO));
    HiTrace::Tracepoint(HITRACE_CM_DEVICE, HITRACE_TP_CS, id, "client send msg content %d", 12);
    HiTrace::Tracepoint(HITRACE_CM_PROCESS, HITRACE_TP_CS, id, "client send msg content %d", 22);
    HiTrace::Tracepoint(HITRACE_CM_THREAD, HITRACE_TP_CS, id, "client send msg content %d", 32);
    HiTrace::Tracepoint(HITRACE_CM_DEFAULT, HITRACE_TP_CS, id, "client send msg content %d", 42);

    HiTrace::Tracepoint(HITRACE_TP_CS, id, "client send msg content %d", 13);

    HiTrace::End(id);
}

/**
 * @tc.name: Dfx_HiTraceCppTest_SyncAsyncTest_001
 * @tc.desc: Start trace with SYNC or ASYNC.
 * @tc.type: FUNC
 * @tc.require: AR000CQ0G7
 */
HWTEST_F(HiTraceCppTest, SyncAsyncTest_001, TestSize.Level1)
{
    /**
     * @tc.steps: step1. start trace without HITRACE_FLAG_INCLUDE_ASYNC flag.
     *    get and check flags.
     * @tc.expected: step1. HITRACE_FLAG_INCLUDE_ASYNC is not enabled.
     * @tc.steps: step2. start trace with HITRACE_FLAG_INCLUDE_ASYNC flag.
     *    get and check flags.
     * @tc.expected: step2. HITRACE_FLAG_INCLUDE_ASYNC is enabled.
     */
    /* begin with sync flag */
    HiTraceId syncId = HiTrace::Begin("test sync only", HITRACE_FLAG_TP_INFO);
    EXPECT_EQ(0, syncId.IsFlagEnabled(HITRACE_FLAG_INCLUDE_ASYNC));
    HiTrace::Tracepoint(HITRACE_TP_CS, syncId, "client send msg: %s", "sync");
        HiTrace::End(syncId);
    /* begin with async flag */
    HiTraceId asyncId = HiTrace::Begin("test sync+async", HITRACE_FLAG_INCLUDE_ASYNC | HITRACE_FLAG_TP_INFO);
    EXPECT_EQ(1, asyncId.IsFlagEnabled(HITRACE_FLAG_INCLUDE_ASYNC));
    HiTrace::Tracepoint(HITRACE_TP_CS, asyncId, "client send msg: %s", "async");

    HiTrace::End(asyncId);
}

/**
 * @tc.name: Dfx_HiTraceCppTest_InvalidParamTest_001
 * @tc.desc: Start trace with SYNC or ASYNC.
 * @tc.type: FUNC
 * @tc.require: AR000CQV9U
 */
HWTEST_F(HiTraceCppTest, InvalidParamTest_001, TestSize.Level1)
{
    /**
     * @tc.steps: step1. start trace with invalid flag and validate trace id.
     * @tc.expected: step1. trace id is invalid.
     * @tc.steps: step2. start trace with invalid name and validate trace id.
     * @tc.expected: step2. trace id is valid.
     */
    /* begin with invalid flag */
    HiTraceId invalidFlagId = HiTrace::Begin("invalid param", HITRACE_FLAG_MAX+1);
    EXPECT_EQ(0, invalidFlagId.IsValid());
    invalidFlagId = HiTrace::Begin("invalid param", -1);
    EXPECT_EQ(0, invalidFlagId.IsValid());
    HiTrace::End(invalidFlagId);

    /* begin with invalid name */
    HiTraceId invalidNameId = HiTrace::Begin("", HITRACE_FLAG_TP_INFO);
    EXPECT_EQ(1, invalidNameId.IsValid());
    HiTrace::End(invalidNameId);
}

}  // namespace HiviewDFX
}  // namespace OHOS
