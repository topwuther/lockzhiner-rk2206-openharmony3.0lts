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

#include "xcollie_watchdog_test.h"

#include <gtest/gtest.h>
#include <string>

#include "xcollie.h"
#include "xcollie_checker_test.h"
#include "xcollie_define.h"
#include "xcollie_inner.h"
#include "xcollie_utils.h"

using namespace testing::ext;

namespace OHOS {
namespace HiviewDFX {
void XCollieWatchdogTest::SetUpTestCase(void)
{
}

void XCollieWatchdogTest::TearDownTestCase(void)
{
}

void XCollieWatchdogTest::SetUp(void)
{
}

void XCollieWatchdogTest::TearDown(void)
{
}

void XCollieWatchdogTest::Init(const int timeout)
{
    XCollieInner::GetInstance().SetCheckStatus(CheckStatus::COMPLETED);
    XCollieInner::GetInstance().SetCheckerInterval(timeout);
    XCollieInner::GetInstance().SetRecoveryFlag(false);
}

/**
 * @tc.name: XCollieWatchdogNoBlockTest
 * @tc.desc: Verify watchdog, registered service has no block.
 * @tc.type: FUNC
 * @tc.require: SR000CPN2F AR000CTAMB
 * @tc.author: yangjing
 */
HWTEST_F(XCollieWatchdogTest, XCollieWatchdogNoBlock_001, TestSize.Level2)
{
    int timeout = 1;
    XCOLLIE_LOGI("XCollieWatchdogNoBlockTest start...");

    /**
     * @tc.steps: step1.init watchdog interval
     */
    Init(timeout);

    /**
     * @tc.steps: step2. register service, monitor thread and lock
     */
    sptr<XCollieCheckerTest> checkerTest = new XCollieCheckerTest("CheckerTest_NoBlock_001");
    XCollie::GetInstance().RegisterXCollieChecker(checkerTest, XCOLLIE_LOCK | XCOLLIE_THREAD);

    /**
     * @tc.steps: step3. check watchdog could call service's callback
     * @tc.expected: step3. watchdog calls service's callback successfully
     */
    std::this_thread::sleep_for(std::chrono::seconds(5));
    EXPECT_GE(checkerTest->GetTestLockNumber(), 2);
    EXPECT_GE(checkerTest->GetTestThreadNumber(), 2);
    XCollieInner::GetInstance().UnRegisterXCollieChecker(checkerTest);
    checkerTest = nullptr;
}

/**
 * @tc.name: XCollieWatchdogBlockHalfTest
 * @tc.desc: Verify watchdog, registered service has half block.
 * @tc.type: FUNC
 * @tc.require: SR000CPN2F AR000CTAMB
 * @tc.author: yangjing
 */
HWTEST_F(XCollieWatchdogTest, XCollieWatchdogBlockHalf_002, TestSize.Level2)
{
    int timeout = 1;

    XCOLLIE_LOGI("XCollieWatchdogBlockHalfTest start...");

    /**
     * @tc.steps: step1.init watchdog interval
     */
    Init(timeout);
    /**
     * @tc.steps: step2. register service, monitor lock
     */
    sptr<XCollieCheckerTest> checkerTest = new XCollieCheckerTest("CheckerTest_BlockHalf_002");
    checkerTest->SetBlockTime(1);
    XCollie::GetInstance().RegisterXCollieChecker(checkerTest, XCOLLIE_LOCK);

    /**
     * @tc.steps: step3. check watchdog could call service's callback
     * @tc.expected: step3. watchdog calls service's callback successfully
     */
    std::this_thread::sleep_for(std::chrono::seconds(5));
    EXPECT_GE(checkerTest->GetTestLockNumber(), 2);
    XCollieInner::GetInstance().UnRegisterXCollieChecker(checkerTest);
    checkerTest = nullptr;
}

/**
 * @tc.name: XCollieWatchdogOneServiceBlockTest
 * @tc.desc: Verify watchdog, registered service has thread block.
 * @tc.type: FUNC
 * @tc.require: SR000CPN2F AR000CTAMB
 * @tc.author: yangjing
 */
HWTEST_F(XCollieWatchdogTest, XCollieWatchdogOneServiceBlockTest_003, TestSize.Level3)
{
    int timeout = 1;

    XCOLLIE_LOGI("XCollieWatchdogOneServiceBlockTest start...");

    /**
     * @tc.steps: step1.init watchdog interval
     */
    Init(timeout);

    /**
     * @tc.steps: step2. register service, monitor lock
     */
    sptr<XCollieCheckerTest> checkerTest = new XCollieCheckerTest("CheckerTest_Block_003");
    checkerTest->SetBlockTime(5);
    XCollie::GetInstance().RegisterXCollieChecker(checkerTest, XCOLLIE_THREAD);
    std::this_thread::sleep_for(std::chrono::seconds(4));

    /**
     * @tc.steps: step3. check watchdog could call service's callback, and monitor service lock
     * @tc.expected: step3. watchdog calls service's callback successfully
     * @tc.expected: step3. watchdog monitor service lock
     */
    EXPECT_EQ(checkerTest->GetTestThreadNumber(), 1);
    EXPECT_EQ(XCollieInner::GetInstance().GetBlockdServiceName() == "CheckerTest_Block_003", true);
    XCollieInner::GetInstance().UnRegisterXCollieChecker(checkerTest);
    checkerTest = nullptr;
}


/**
 * @tc.name: XCollieWatchdogTwoServiceThreadBlock
 * @tc.desc: Verify watchdog, register two services, and one service has thread block.
 * @tc.type: FUNC
 * @tc.require: SR000CPN2F AR000CTAMB
 * @tc.author: yangjing
 */
HWTEST_F(XCollieWatchdogTest, XCollieWatchdogTwoServiceThreadBlock_004, TestSize.Level3)
{
    int timeout = 1;

    XCOLLIE_LOGI("XCollieWatchdogTwoServiceThreadBlock start...");

    /**
     * @tc.steps: step1.init watchdog interval
     */
    Init(timeout);

    /**
     * @tc.steps: step2. register two services, monitor lock
     */
    sptr<XCollieCheckerTest> checkerThreadTest = new XCollieCheckerTest("CheckerTest_ThreadBlock_004");
    checkerThreadTest->SetBlockTime(5);
    XCollie::GetInstance().RegisterXCollieChecker(checkerThreadTest, XCOLLIE_THREAD);

    sptr<XCollieCheckerTest> checkerLockTest = new XCollieCheckerTest("CheckerTest_LockBlock_004");
    checkerLockTest->SetBlockTime(0);
    XCollie::GetInstance().RegisterXCollieChecker(checkerLockTest, XCOLLIE_LOCK);
    std::this_thread::sleep_for(std::chrono::seconds(4));

    /**
     * @tc.steps: step3. check watchdog could call service's callback, and monitor service lock
     * @tc.expected: step3. watchdog calls service's callback successfully
     * @tc.expected: step3. watchdog monitor service lock
     */
    EXPECT_GE(checkerThreadTest->GetTestThreadNumber(), 1);
    EXPECT_GE(checkerLockTest->GetTestLockNumber(), 1);
    EXPECT_EQ(XCollieInner::GetInstance().GetBlockdServiceName() == "CheckerTest_ThreadBlock_004", true);
    XCollieInner::GetInstance().UnRegisterXCollieChecker(checkerThreadTest);
    XCollieInner::GetInstance().UnRegisterXCollieChecker(checkerLockTest);
    checkerThreadTest = nullptr;
    checkerLockTest = nullptr;
}

/**
 * @tc.name: XCollieWatchdogTestTwoServiceLockBlock
 * @tc.desc: Verify watchdog, register two services, and one service has lock block.
 * @tc.type: FUNC
 * @tc.require: AR000CVLGG
 * @tc.author: yangjing
 */
HWTEST_F(XCollieWatchdogTest, XCollieWatchdogTestTwoServiceLockBlock_005, TestSize.Level3)
{
    int timeout = 1;

    XCOLLIE_LOGI("XCollieWatchdogTestTwoServiceLockBlock start...");

    /**
     * @tc.steps: step1.init watchdog interval
     */
    Init(timeout);

    /**
     * @tc.steps: step2. register two services, monitor lock
     */
    sptr<XCollieCheckerTest> checkerThreadTest = new XCollieCheckerTest("CheckerTest_ThreadBlock_005");
    checkerThreadTest->SetBlockTime(0);
    XCollie::GetInstance().RegisterXCollieChecker(checkerThreadTest, XCOLLIE_THREAD);

    sptr<XCollieCheckerTest> checkerLockTest = new XCollieCheckerTest("CheckerTest_LockBlock_005");
    checkerLockTest->SetBlockTime(6);
    XCollie::GetInstance().RegisterXCollieChecker(checkerLockTest, XCOLLIE_LOCK);
    std::this_thread::sleep_for(std::chrono::seconds(4));

    /**
     * @tc.steps: step3. check watchdog could call service's callback, and monitor service lock
     * @tc.expected: step3. watchdog calls service's callback successfully
     * @tc.expected: step3. watchdog monitor service lock
     */
    EXPECT_GE(checkerThreadTest->GetTestThreadNumber(), 1);
    EXPECT_GE(checkerLockTest->GetTestLockNumber(), 1);
    EXPECT_EQ(XCollieInner::GetInstance().GetBlockdServiceName() == "CheckerTest_LockBlock_005", true);
    XCollieInner::GetInstance().UnRegisterXCollieChecker(checkerThreadTest);
    XCollieInner::GetInstance().UnRegisterXCollieChecker(checkerLockTest);
    XCOLLIE_LOGI("XCollieWatchdogTestTwoServiceLockBlock exit...");
    checkerThreadTest = nullptr;
    checkerLockTest = nullptr;
}
} // end of namespace HiviewDFX
} // end of namespace OHOS
