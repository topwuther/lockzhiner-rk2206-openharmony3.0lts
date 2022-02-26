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

#include "hitrace/hitracec.h"

#include <gtest/gtest.h>

namespace OHOS {
namespace HiviewDFX {

using namespace testing::ext;

#define HITRACE_DEBUG
#ifndef HITRACE_DEBUG
#define PRINT_ID(p)
#else
#define PRINT_ID(p)                                                                                                \
    printf(#p " valid:%d, ver:%d, chain:0x%llx, flags:%x, span:0x%x, pspan:0x%x.\n", static_cast<int>((p)->valid), \
           static_cast<int>((p)->ver), static_cast<long long>((p)->chainId), static_cast<int>((p)->flags),         \
           static_cast<int>((p)->spanId), static_cast<int>((p)->parentSpanId))
#endif

class HiTraceCTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();
};

void HiTraceCTest::SetUpTestCase()
{}

void HiTraceCTest::TearDownTestCase()
{}

void HiTraceCTest::SetUp()
{
    HiTraceClearId();
}

void HiTraceCTest::TearDown()
{}

/**
 * @tc.name: Dfx_HiTraceCTest_IdTest_001
 * @tc.desc: Get, set and clear trace id
 * @tc.type: FUNC
 * @tc.require: AR000CQVA0
 */
HWTEST_F(HiTraceCTest, IdTest_001, TestSize.Level1)
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
    HiTraceIdStruct initId = HiTraceGetId();
    EXPECT_EQ(0, HiTraceIsValid(&initId));
    PRINT_ID(&initId);

    // set thread id
    constexpr uint64_t chainId = 0xABCDEF;
    constexpr uint64_t spanId = 0x12345;
    constexpr uint64_t parentSpanId = 0x67890;
    constexpr int flags = HITRACE_FLAG_INCLUDE_ASYNC | HITRACE_FLAG_DONOT_CREATE_SPAN;
    HiTraceIdStruct setId;
    HiTraceInitId(&setId);
    HiTraceSetChainId(&setId, chainId);
    HiTraceSetFlags(&setId, flags);
    HiTraceSetSpanId(&setId, spanId);
    HiTraceSetParentSpanId(&setId, parentSpanId);
    PRINT_ID(&setId);

    HiTraceSetId(&setId);

    HiTraceIdStruct getId = HiTraceGetId();
    EXPECT_EQ(1, HiTraceIsValid(&getId));
    EXPECT_EQ(chainId, HiTraceGetChainId(&getId));
    EXPECT_EQ(HITRACE_FLAG_INCLUDE_ASYNC | HITRACE_FLAG_DONOT_CREATE_SPAN, HiTraceGetFlags(&getId));
    EXPECT_EQ(spanId, HiTraceGetSpanId(&getId));
    EXPECT_EQ(parentSpanId, HiTraceGetParentSpanId(&getId));
    PRINT_ID(&getId);

    // set invalid id
    HiTraceIdStruct invalidId;
    HiTraceInitId(&invalidId);

    HiTraceSetId(&invalidId);

    getId = HiTraceGetId();
    EXPECT_EQ(1, HiTraceIsValid(&getId));
    EXPECT_EQ(chainId, HiTraceGetChainId(&getId));
    EXPECT_EQ(HITRACE_FLAG_INCLUDE_ASYNC | HITRACE_FLAG_DONOT_CREATE_SPAN, HiTraceGetFlags(&getId));
    EXPECT_EQ(spanId, HiTraceGetSpanId(&getId));
    EXPECT_EQ(parentSpanId, HiTraceGetParentSpanId(&getId));
    PRINT_ID(&getId);

    // clear thread id
    HiTraceClearId();

    HiTraceIdStruct clearId = HiTraceGetId();
    EXPECT_EQ(0, HiTraceIsValid(&clearId));
    PRINT_ID(&clearId);
}

/**
 * @tc.name: Dfx_HiTraceCTest_IntfTest_001
 * @tc.desc: Interconversion between trace id and bytes array.
 * @tc.type: FUNC
 * @tc.require: AR000CQV9T
 */
HWTEST_F(HiTraceCTest, IntfTest_001, TestSize.Level1)
{
    /**
     * @tc.steps: step1. construct trace id and validate it.
     * @tc.expected: step1. trace id is valid.
     * @tc.steps: step2. convert trace id to bytes array.
     * @tc.expected: step2. convert success when array size >= id length.
     * @tc.steps: step3. convert bytes array to trace id.
     * @tc.expected: step3. convert success only when array size == id length.
     * @tc.steps: step4. convert invalid id to bytes array.
     * @tc.expected: step4. convert fail.
     * @tc.steps: step5. convert invalid bytes array to id.
     * @tc.expected: step5. convert fail.
     */
    // id to bytes
    constexpr uint64_t chainId = 0xABCDEF;
    constexpr uint64_t spanId = 0x12345;
    constexpr uint64_t parentSpanId = 0x67890;
    constexpr int flags = HITRACE_FLAG_INCLUDE_ASYNC | HITRACE_FLAG_DONOT_CREATE_SPAN;
    HiTraceIdStruct id = {HITRACE_ID_VALID, HITRACE_VER_1, chainId, flags, spanId, parentSpanId};
    EXPECT_EQ(1, HiTraceIsValid(&id));
    PRINT_ID(&id);

    constexpr int idLen = sizeof(HiTraceIdStruct);
    uint8_t bytes[idLen + 1];
    int len = HiTraceIdToBytes(&id, bytes, idLen - 1);
    EXPECT_EQ(0, len);
    len = HiTraceIdToBytes(&id, bytes, idLen + 1);
    EXPECT_EQ(idLen, len);
    len = HiTraceIdToBytes(&id, bytes, idLen);
    EXPECT_EQ(idLen, len);
    PRINT_ID(reinterpret_cast<HiTraceIdStruct*>(bytes));

    // bytes to id
    HiTraceIdStruct bytesToId = HiTraceBytesToId(bytes, idLen - 1);
    EXPECT_EQ(0, HiTraceIsValid(&bytesToId));
    bytesToId = HiTraceBytesToId(bytes, idLen + 1);
    EXPECT_EQ(0, HiTraceIsValid(&bytesToId));
    bytesToId = HiTraceBytesToId(bytes, idLen);
    EXPECT_EQ(1, HiTraceIsValid(&bytesToId));
    EXPECT_EQ(chainId, HiTraceGetChainId(&bytesToId));
    EXPECT_EQ(HITRACE_FLAG_INCLUDE_ASYNC | HITRACE_FLAG_DONOT_CREATE_SPAN, HiTraceGetFlags(&bytesToId));
    EXPECT_EQ(spanId, HiTraceGetSpanId(&bytesToId));
    EXPECT_EQ(parentSpanId, HiTraceGetParentSpanId(&bytesToId));
    PRINT_ID(&bytesToId);

    // set invalid id
    HiTraceIdStruct invalidId;
    HiTraceInitId(&invalidId);
    EXPECT_EQ(0, HiTraceIdToBytes(&invalidId, bytes, idLen));
    invalidId = HiTraceBytesToId(nullptr, idLen);
    EXPECT_EQ(0, HiTraceIsValid(&invalidId));
}

/**
 * @tc.name: Dfx_HiTraceCTest_IntfTest_002
 * @tc.desc: Start and stop trace.
 * @tc.type: FUNC
 * @tc.require: AR000CQV9T
 */
HWTEST_F(HiTraceCTest, IntfTest_002, TestSize.Level1)
{
    /**
     * @tc.steps: step1. start trace with flags, get trace id and validit it.
     * @tc.expected: step1. trace id and flags is valid.
     * @tc.steps: step2. stop trace, get trace id and validit it.
     * @tc.expected: step2. trace id is invalid.
     */
    // begin
    HiTraceIdStruct beginId = HiTraceBegin("test", HITRACE_FLAG_INCLUDE_ASYNC | HITRACE_FLAG_NO_BE_INFO);
    EXPECT_EQ(1, HiTraceIsValid(&beginId));
    EXPECT_EQ(1, HiTraceIsFlagEnabled(&beginId, HITRACE_FLAG_INCLUDE_ASYNC));
    EXPECT_EQ(1, HiTraceIsFlagEnabled(&beginId, HITRACE_FLAG_NO_BE_INFO));
    PRINT_ID(&beginId);

    // end
    HiTraceEnd(&beginId);

    HiTraceIdStruct endId = HiTraceGetId();
    EXPECT_EQ(0, HiTraceIsValid(&endId));
    PRINT_ID(&endId);
}

/**
 * @tc.name: Dfx_HiTraceCTest_IntfTest_003
 * @tc.desc: Start and stop trace with reentered.
 * @tc.type: FUNC
 * @tc.require: AR000CQV9T
 */
HWTEST_F(HiTraceCTest, IntfTest_003, TestSize.Level1)
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
    HiTraceIdStruct beginId = HiTraceBegin("begin", HITRACE_FLAG_INCLUDE_ASYNC);
    PRINT_ID(&beginId);

    // reenter begin
    HiTraceIdStruct reBeginId = HiTraceBegin("reenter begin", HITRACE_FLAG_TP_INFO);
    EXPECT_EQ(0, HiTraceIsValid(&reBeginId));
    EXPECT_NE(HiTraceGetChainId(&reBeginId), HiTraceGetChainId(&beginId));
    EXPECT_EQ(0, HiTraceIsFlagEnabled(&reBeginId, HITRACE_FLAG_INCLUDE_ASYNC));
    EXPECT_EQ(0, HiTraceIsFlagEnabled(&reBeginId, HITRACE_FLAG_TP_INFO));
    PRINT_ID(&reBeginId);

    // reenter end
    HiTraceEnd(&reBeginId);

    HiTraceIdStruct endId = HiTraceGetId();
    EXPECT_EQ(1, HiTraceIsValid(&endId));
    EXPECT_EQ(HiTraceGetChainId(&endId), HiTraceGetChainId(&beginId));
    EXPECT_EQ(1, HiTraceIsFlagEnabled(&endId, HITRACE_FLAG_INCLUDE_ASYNC));
    EXPECT_EQ(0, HiTraceIsFlagEnabled(&endId, HITRACE_FLAG_TP_INFO));
    PRINT_ID(&endId);

    // end with wrong chainId
    HiTraceIdStruct wrongBeginId = beginId;
    HiTraceSetChainId(&wrongBeginId, HiTraceGetChainId(&beginId) + 1);
    HiTraceEnd(&wrongBeginId);

    HiTraceIdStruct wrongEndId = HiTraceGetId();
    EXPECT_EQ(1, HiTraceIsValid(&wrongEndId));
    EXPECT_EQ(HiTraceGetChainId(&wrongEndId), HiTraceGetChainId(&beginId));
    EXPECT_EQ(1, HiTraceIsFlagEnabled(&wrongEndId, HITRACE_FLAG_INCLUDE_ASYNC));
    PRINT_ID(&wrongEndId);

    // end
    HiTraceEnd(&beginId);

    HiTraceIdStruct reEndId = HiTraceGetId();
    EXPECT_EQ(0, HiTraceIsValid(&reEndId));
    PRINT_ID(&reEndId);

    // end with invalid thread id
    HiTraceEnd(&beginId);

    HiTraceIdStruct endInvalidId = HiTraceGetId();
    EXPECT_EQ(0, HiTraceIsValid(&endInvalidId));
    PRINT_ID(&endInvalidId);
}

/**
 * @tc.name: Dfx_HiTraceCTest_SpanTest_001
 * @tc.desc: Create child and grand child span.
 * @tc.type: FUNC
 * @tc.require: AR000CQVA2
 */
HWTEST_F(HiTraceCTest, SpanTest_001, TestSize.Level1)
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
    HiTraceIdStruct id = HiTraceBegin("test", 0);
    EXPECT_EQ(0, HiTraceGetFlags(&id));
    EXPECT_EQ(0UL, HiTraceGetSpanId(&id));
    EXPECT_EQ(0UL, HiTraceGetParentSpanId(&id));
    PRINT_ID(&id);

    // create child span
    HiTraceIdStruct childId = HiTraceCreateSpan();
    EXPECT_EQ(1, HiTraceIsValid(&childId));
    EXPECT_EQ(HiTraceGetFlags(&childId), HiTraceGetFlags(&id));
    EXPECT_EQ(HiTraceGetChainId(&childId), HiTraceGetChainId(&id));
    EXPECT_EQ(HiTraceGetParentSpanId(&childId), HiTraceGetSpanId(&id));
    PRINT_ID(&childId);

    // set child id to thread id
    HiTraceSetId(&childId);

    // continue to create child span
    HiTraceIdStruct grandChildId = HiTraceCreateSpan();
    EXPECT_EQ(1, HiTraceIsValid(&grandChildId));
    EXPECT_EQ(HiTraceGetFlags(&grandChildId), HiTraceGetFlags(&id));
    EXPECT_EQ(HiTraceGetChainId(&grandChildId), HiTraceGetChainId(&id));
    EXPECT_EQ(HiTraceGetParentSpanId(&grandChildId), HiTraceGetSpanId(&childId));
    PRINT_ID(&grandChildId);

    // end
    HiTraceEnd(&id);
}

/**
 * @tc.name: Dfx_HiTraceCTest_SpanTest_002
 * @tc.desc: Start and stop trace with reentered.
 * @tc.type: FUNC
 * @tc.require: AR000CQVA2
 */
HWTEST_F(HiTraceCTest, SpanTest_002, TestSize.Level1)
{
    /**
     * @tc.steps: step1. start trace with HITRACE_FLAG_DONOT_CREATE_SPAN,
     *     get and check flags.
     * @tc.expected: step1. HITRACE_FLAG_DONOT_CREATE_SPAN is enabled.
     * @tc.steps: step2. create child id.
     * @tc.expected: step2. child id is same with parent id.
     */
    // begin with "donot create span" flag
    HiTraceIdStruct id = HiTraceBegin("test", HITRACE_FLAG_DONOT_CREATE_SPAN);
    EXPECT_EQ(1, HiTraceIsFlagEnabled(&id, HITRACE_FLAG_DONOT_CREATE_SPAN));
    PRINT_ID(&id);

    // create child span
    HiTraceIdStruct childId = HiTraceCreateSpan();
    EXPECT_EQ(1, HiTraceIsValid(&childId));
    EXPECT_EQ(HiTraceGetFlags(&childId), HiTraceGetFlags(&id));
    EXPECT_EQ(HiTraceGetChainId(&childId), HiTraceGetChainId(&id));
    EXPECT_EQ(HiTraceGetSpanId(&childId), HiTraceGetSpanId(&id));
    EXPECT_EQ(HiTraceGetParentSpanId(&childId), HiTraceGetParentSpanId(&id));
    PRINT_ID(&childId);

    // end
    HiTraceEnd(&id);
}

/**
 * @tc.name: Dfx_HiTraceCTest_TracepointTest_001
 * @tc.desc: Start trace with HITRACE_FLAG_TP_INFO flag.
 * @tc.type: FUNC
 * @tc.require: AR000CQVA3
 */
HWTEST_F(HiTraceCTest, TracepointTest_001, TestSize.Level1)
{
    /**
     * @tc.steps: step1. start trace with HITRACE_FLAG_TP_INFO,
     *     get and check flags.
     * @tc.expected: step1. HITRACE_FLAG_TP_INFO is enabled.
     * @tc.steps: step2. add trace point info with id and check logs.
     * @tc.expected: step2. trace point can be found in logs.
     * @tc.steps: step2. add trace point info with null and check logs.
     * @tc.expected: step2. trace point cannot be found in logs.
     */
    HiTraceIdStruct id = HiTraceBegin("test tp flag", HITRACE_FLAG_TP_INFO);
    EXPECT_EQ(1, HiTraceIsFlagEnabled(&id, HITRACE_FLAG_TP_INFO));
    HiTraceTracepoint(HITRACE_TP_CS, &id, "client send msg content %d", 12);

    HiTraceTracepoint(HITRACE_TP_CS, nullptr, "client send msg content %d", 12);

    HiTraceEnd(&id);
}

/**
 * @tc.name: Dfx_HiTraceCTest_TracepointTest_002
 * @tc.desc: Start trace without HITRACE_FLAG_TP_INFO flag.
 * @tc.type: FUNC
 * @tc.require: AR000CQVA3
 */
HWTEST_F(HiTraceCTest, TracepointTest_002, TestSize.Level1)
{
    /**
     * @tc.steps: step1. start trace without HITRACE_FLAG_TP_INFO flag.
     *     get and check flags.
     * @tc.expected: step1. HITRACE_FLAG_TP_INFO is not enabled.
     * @tc.steps: step2. add trace point info with id and check logs.
     * @tc.expected: step2. trace point cannot be found in logs.
     */
    // begin with tp flag
    HiTraceIdStruct id = HiTraceBegin("test no tp flag", HITRACE_FLAG_INCLUDE_ASYNC);
    EXPECT_EQ(0, HiTraceIsFlagEnabled(&id, HITRACE_FLAG_TP_INFO));
    HiTraceTracepoint(HITRACE_TP_CS, &id, "client send msg content %d", 12);

    HiTraceEnd(&id);
}

/**
 * @tc.name: Dfx_HiTraceCTest_TracepointTest_003
 * @tc.desc: Start trace with HITRACE_FLAG_D2D_TP_INFO flag.
 * @tc.type: FUNC
 * @tc.require: AR000CQVA3
 */
HWTEST_F(HiTraceCTest, TracepointTest_003, TestSize.Level1)
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
    HiTraceIdStruct id = HiTraceBegin("test D2D tp flag", HITRACE_FLAG_D2D_TP_INFO);
    EXPECT_EQ(1, HiTraceIsFlagEnabled(&id, HITRACE_FLAG_D2D_TP_INFO));
    HiTraceTracepointEx(HITRACE_CM_DEVICE, HITRACE_TP_CS, &id, "client send msg content %d", 12);
    HiTraceTracepointEx(HITRACE_CM_PROCESS, HITRACE_TP_CS, &id, "cannot be found %d", 22);
    HiTraceTracepointEx(HITRACE_CM_THREAD, HITRACE_TP_CS, &id, "cannot be found %d", 32);
    HiTraceTracepointEx(HITRACE_CM_DEFAULT, HITRACE_TP_CS, &id, "cannot be found %d", 42);

    HiTraceTracepointEx(HITRACE_CM_DEVICE, HITRACE_TP_CS, nullptr, "cannot be found %d", 13);

    HiTraceTracepoint(HITRACE_TP_CS, &id, "cannot be found %d", 14);

    HiTraceEnd(&id);
}

/**
 * @tc.name: Dfx_HiTraceCTest_TracepointTest_004
 * @tc.desc: Start trace without HITRACE_FLAG_D2D_TP_INFO flag.
 * @tc.type: FUNC
 * @tc.require: AR000CQVA3
 */
HWTEST_F(HiTraceCTest, TracepointTest_004, TestSize.Level1)
{
    /**
     * @tc.steps: step1. start trace without HITRACE_FLAG_D2D_TP_INFO flag.
     *     get and check flags.
     * @tc.expected: step1. HITRACE_FLAG_D2D_TP_INFO is not enabled.
     * @tc.steps: step2. add D2D trace point info with id and check logs.
     * @tc.expected: step2. trace point cannot be found in logs.
     */
    HiTraceIdStruct id = HiTraceBegin("test no D2D tp flag", HITRACE_FLAG_INCLUDE_ASYNC);
    EXPECT_EQ(0, HiTraceIsFlagEnabled(&id, HITRACE_FLAG_D2D_TP_INFO));
    HiTraceTracepointEx(HITRACE_CM_DEVICE, HITRACE_TP_CS, &id, "cannot be found %d", 12);
    HiTraceTracepointEx(HITRACE_CM_PROCESS, HITRACE_TP_CS, &id, "cannot be found %d", 22);
    HiTraceTracepointEx(HITRACE_CM_THREAD, HITRACE_TP_CS, &id, "cannot be found %d", 32);
    HiTraceTracepointEx(HITRACE_CM_DEFAULT, HITRACE_TP_CS, &id, "cannot be found %d", 42);

    HiTraceEnd(&id);
}

/**
 * @tc.name: Dfx_HiTraceCTest_TracepointTest_005
 * @tc.desc: Start trace with HITRACE_FLAG_D2D_TP_INFO and HITRACE_FLAG_TP_INFO flag.
 * @tc.type: FUNC
 * @tc.require: AR000CQVA3
 */
HWTEST_F(HiTraceCTest, TracepointTest_005, TestSize.Level1)
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
    HiTraceIdStruct id = HiTraceBegin("test D2D | TP tp flag", HITRACE_FLAG_D2D_TP_INFO | HITRACE_FLAG_TP_INFO);
    EXPECT_EQ(1, HiTraceIsFlagEnabled(&id, HITRACE_FLAG_D2D_TP_INFO));
    EXPECT_EQ(1, HiTraceIsFlagEnabled(&id, HITRACE_FLAG_TP_INFO));
    HiTraceTracepointEx(HITRACE_CM_DEVICE, HITRACE_TP_CS, &id, "client send msg content %d", 12);
    HiTraceTracepointEx(HITRACE_CM_PROCESS, HITRACE_TP_CS, &id, "client send msg content %d", 22);
    HiTraceTracepointEx(HITRACE_CM_THREAD, HITRACE_TP_CS, &id, "client send msg content %d", 32);
    HiTraceTracepointEx(HITRACE_CM_DEFAULT, HITRACE_TP_CS, &id, "client send msg content %d", 42);

    HiTraceTracepoint(HITRACE_TP_CS, &id, "client send msg content %d", 13);

    HiTraceEnd(&id);
}

/**
 * @tc.name: Dfx_HiTraceCTest_TracepointTest_006
 * @tc.desc: Start trace without HITRACE_FLAG_D2D_TP_INFO, but with HITRACE_FLAG_TP_INFO flag.
 * @tc.type: FUNC
 * @tc.require: AR000CQVA3
 */
HWTEST_F(HiTraceCTest, TracepointTest_006, TestSize.Level1)
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
    HiTraceIdStruct id = HiTraceBegin("test no D2D, but tp flag", HITRACE_FLAG_TP_INFO);
    EXPECT_EQ(0, HiTraceIsFlagEnabled(&id, HITRACE_FLAG_D2D_TP_INFO));
    EXPECT_EQ(1, HiTraceIsFlagEnabled(&id, HITRACE_FLAG_TP_INFO));
    HiTraceTracepointEx(HITRACE_CM_DEVICE, HITRACE_TP_CS, &id, "client send msg content %d", 12);
    HiTraceTracepointEx(HITRACE_CM_PROCESS, HITRACE_TP_CS, &id, "client send msg content %d", 22);
    HiTraceTracepointEx(HITRACE_CM_THREAD, HITRACE_TP_CS, &id, "client send msg content %d", 32);
    HiTraceTracepointEx(HITRACE_CM_DEFAULT, HITRACE_TP_CS, &id, "client send msg content %d", 42);

    HiTraceTracepoint(HITRACE_TP_CS, &id, "client send msg content %d", 13);

    HiTraceEnd(&id);
}

/**
 * @tc.name: Dfx_HiTraceCTest_SyncAsyncTest_001
 * @tc.desc: Start trace with SYNC or ASYNC.
 * @tc.type: FUNC
 * @tc.require: AR000CQ0G7
 */
HWTEST_F(HiTraceCTest, SyncAsyncTest_001, TestSize.Level1)
{
    /**
     * @tc.steps: step1. start trace without HITRACE_FLAG_INCLUDE_ASYNC flag.
     *    get and check flags.
     * @tc.expected: step1. HITRACE_FLAG_INCLUDE_ASYNC is not enabled.
     * @tc.steps: step2. start trace with HITRACE_FLAG_INCLUDE_ASYNC flag.
     *    get and check flags.
     * @tc.expected: step2. HITRACE_FLAG_INCLUDE_ASYNC is enabled.
     */
    // begin with sync flag
    HiTraceIdStruct syncId = HiTraceBegin("test sync only", HITRACE_FLAG_TP_INFO);
    EXPECT_EQ(0, HiTraceIsFlagEnabled(&syncId, HITRACE_FLAG_INCLUDE_ASYNC));
    HiTraceTracepoint(HITRACE_TP_CS, &syncId, "client send msg: %s", "sync");

    HiTraceEnd(&syncId);

    // begin with async flag
    HiTraceIdStruct asyncId = HiTraceBegin("test sync+async", HITRACE_FLAG_INCLUDE_ASYNC | HITRACE_FLAG_TP_INFO);
    EXPECT_EQ(1, HiTraceIsFlagEnabled(&asyncId, HITRACE_FLAG_INCLUDE_ASYNC));
    HiTraceTracepoint(HITRACE_TP_CS, &asyncId, "client send msg: %s", "async");

    HiTraceEnd(&asyncId);
}

}  // namespace HiviewDFX
}  // namespace OHOS
