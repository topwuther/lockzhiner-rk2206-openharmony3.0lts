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

#include "xcollie_timeout_test.h"

#include "timer_ring.h"
#include "xcollie.h"
#include "xcollie_inner.h"
#include "xcollie_utils.h"

using namespace testing::ext;

namespace OHOS {
namespace HiviewDFX {
XCollieTimeoutTest::XCollieTimeoutTest():callbackCnt_(0)
{
}

XCollieTimeoutTest::~XCollieTimeoutTest()
{
}

void XCollieTimeoutTest::SetUpTestCase(void)
{
}

void XCollieTimeoutTest::TearDownTestCase(void)
{
}

void XCollieTimeoutTest::SetUp(void)
{
}

void XCollieTimeoutTest::TearDown(void)
{
}

void XCollieTimeoutTest::TimerCallback(void *data)
{
    callbackCnt_++;
}

void XCollieTimeoutTest::SetCallbackCnt(int cnt)
{
    callbackCnt_ = cnt;
}

/**
 * @tc.name: XCollieTimerNoTimeoutTest
 * @tc.desc: Verify timer, add, cancel, update timer
 * @tc.type: FUNC
 * @tc.require: SR000CPN2F AR000CTAMB
 * @tc.author: yangjing
 */
HWTEST_F(XCollieTimeoutTest, XCollieTimerNoTimeout_001, TestSize.Level2)
{
    /**
     * @tc.steps: step1.add timer
     * @tc.expected: step1. Add timer successfully
     */
    SetCallbackCnt(0);
    int id = XCollie::GetInstance().SetTimer("NoTimeoutTimer", 1, nullptr, nullptr, XCOLLIE_FLAG_NOOP);
    ASSERT_NE(id, INVALID_ID);

    /**
     * @tc.steps: step2.update timer
     * @tc.expected: step2. update timer successfully
     */
    bool ret = XCollie::GetInstance().UpdateTimer(id, 3);
    ASSERT_EQ(ret, true);

    /**
     * @tc.steps: step3.cancel timer
     * @tc.expected: step3. cancel timer successfully
     */
    XCollie::GetInstance().CancelTimer(id);
}

/**
 * @tc.name: XCollieTimerTimeoutTest
 * @tc.desc: Verify timer, timer timeout
 * @tc.type: FUNC
 * @tc.require: SR000CPN2F AR000CTAMB
 * @tc.author: yangjing
 */
HWTEST_F(XCollieTimeoutTest, XCollieTimerTimeout_002, TestSize.Level2)
{
    SetCallbackCnt(0);
    auto func = [this](void *data) {
        this->TimerCallback(data);
    };
    /**
     * @tc.steps: step1.add timer
     * @tc.expected: step1. Add timer successfully
     */
    int id = XCollie::GetInstance().SetTimer("TimeoutTimer", 1, func, this, XCOLLIE_FLAG_NOOP);
    ASSERT_NE(id, INVALID_ID);
    std::this_thread::sleep_for(std::chrono::seconds(3));
    ASSERT_EQ(GetCallbackCnt(), 1);

    /**
     * @tc.steps: step2. timer timeout, update timer
     * @tc.expected: step2. update timer failed
     */
    bool ret = XCollie::GetInstance().UpdateTimer(id, 1);
    ASSERT_EQ(ret, false);
    XCollie::GetInstance().CancelTimer(id);
}

/**
 * @tc.name: XCollieTimerAddTaskOverFlow
 * @tc.desc: Verify timer, timer task number over flow
 * @tc.type: FUNC
 * @tc.require: SR000CPN2F AR000CTAMB
 * @tc.author: yangjing
 */
HWTEST_F(XCollieTimeoutTest, XCollieTimerAddTaskOverFlow_003, TestSize.Level3)
{
    SetCallbackCnt(0);
    XCollieInner::GetInstance().SetRecoveryFlag(false);
    auto func = [this](void *data) {
        this->TimerCallback(data);
    };
    /**
     * @tc.steps: step1.add MAX_XCOLLIE_NUM timer
     * @tc.expected: step1. Add timer successfully
     */
    int id[MAX_XCOLLIE_NUM];
    for (unsigned int i = 0; i < MAX_XCOLLIE_NUM; i++) {
        id[i] = XCollie::GetInstance().SetTimer("TimeoutTimer", 1, func, this, XCOLLIE_FLAG_DEFAULT);
        ASSERT_NE(id[i], INVALID_ID);
    }

    /**
     * @tc.steps: step2.add MAX_XCOLLIE_NUM+1 timer
     * @tc.expected: step2. Add timer failed
     */
    int last = XCollie::GetInstance().SetTimer("TimeoutTimer", 1, nullptr, nullptr, XCOLLIE_FLAG_NOOP);
    ASSERT_EQ(last, INVALID_ID);

    /**
     * @tc.steps: step3. wait MAX_XCOLLIE_NUM timer timeout
     * @tc.expected: step3. timer timeout successfully
     */
    std::this_thread::sleep_for(std::chrono::seconds(4));
    ASSERT_EQ(static_cast<unsigned int>(GetCallbackCnt()), MAX_XCOLLIE_NUM);

    /**
     * @tc.steps: step4.add MAX_XCOLLIE_NUM+1 timer
     * @tc.expected: step4. Add timer successfully
     */
    last = XCollie::GetInstance().SetTimer("TimeoutTimer", 1, nullptr, nullptr, XCOLLIE_FLAG_NOOP);
    ASSERT_NE(last, INVALID_ID);
    XCollie::GetInstance().CancelTimer(last);
}

/**
 * @tc.name: XCollieTimerRoundTest
 * @tc.desc: Verify xcollie timer round test
 * @tc.type: FUNC
 * @tc.require: SR000CPN2F AR000CTAMB
 * @tc.author: yangjing
 */
HWTEST_F(XCollieTimeoutTest, XCollieTimerRound_004, TestSize.Level4)
{
    XCOLLIE_LOGI("XCollieTimerRoundTest start...");

    SetCallbackCnt(0);
    XCollieInner::GetInstance().SetRecoveryFlag(false);
    auto func = [this](void *data) {
        this->TimerCallback(data);
    };

    /**
     * @tc.steps: step1.add timer, timeout larger then ring length
     * @tc.expected: step1. Add timer successfully
     */
    int last = XCollie::GetInstance().SetTimer("TimeoutTimer", 61, func, nullptr, XCOLLIE_FLAG_NOOP);
    ASSERT_NE(last, INVALID_ID);

    /**
     * @tc.steps: step2. wait  timer timeout
     * @tc.expected: step2. timer timeout successfully
     */
    std::this_thread::sleep_for(std::chrono::seconds(65));
    ASSERT_EQ(GetCallbackCnt(), 1);
}
} // end of namespace HiviewDFX
} // end of namespace OHOS
