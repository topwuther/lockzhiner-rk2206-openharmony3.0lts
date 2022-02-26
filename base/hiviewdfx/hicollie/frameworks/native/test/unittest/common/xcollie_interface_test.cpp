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

#include "xcollie_interface_test.h"

#include <gtest/gtest.h>
#include <string>

#include "xcollie.h"
#include "xcollie_checker_test.h"

using namespace testing::ext;

namespace OHOS {
namespace HiviewDFX {
void XCollieInterfaceTest::SetUpTestCase(void)
{
}

void XCollieInterfaceTest::TearDownTestCase(void)
{
}

void XCollieInterfaceTest::SetUp(void)
{
}

void XCollieInterfaceTest::TearDown(void)
{
}

/**
 * @tc.name: XCollieRegisterCheckerParamTest
 * @tc.desc: Verify xcollie register checker interface param
 * @tc.type: FUNC
 * @tc.require: SR000CPN2F AR000CTAMB
 * @tc.author: yangjing
 */
HWTEST_F(XCollieInterfaceTest, XCollieRegisterCheckerParam_001, TestSize.Level1)
{
    /**
     * @tc.steps: step1. input param checker is null
     * @tc.expected: step1. register checker failed;
     */
    XCollie::GetInstance().RegisterXCollieChecker(nullptr, XCOLLIE_LOCK | XCOLLIE_THREAD);

    /**
     * @tc.steps: step2. input param type is invalid
     * @tc.expected: step2. register checker failed;
     */
    sptr<XCollieCheckerTest> checkerTest = new XCollieCheckerTest("CheckerTest_NoBlock");
    XCollie::GetInstance().RegisterXCollieChecker(checkerTest, 0x100);
    checkerTest = nullptr;
}

/**
 * @tc.name: XCollieTimerParamTest
 * @tc.desc: Verify xcollie timer interface param
 * @tc.type: FUNC
 * @tc.require: SR000CPN2F AR000CTAMB
 * @tc.author: yangjing
 */
HWTEST_F(XCollieInterfaceTest, XCollieTimerParam_002, TestSize.Level1)
{
    /**
     * @tc.steps: step1. input param name include special string
     * @tc.expected: step1. set timer successfully;
     */
    int id = XCollie::GetInstance().SetTimer("TimeoutTimerxce!@#$%^&*()", 1, nullptr, nullptr, XCOLLIE_FLAG_NOOP);
    ASSERT_NE(id, INVALID_ID);

    /**
     * @tc.steps: step2. input param name include special string,update timer
     * @tc.expected: step2. update timer successfully;
     */
    bool ret = XCollie::GetInstance().UpdateTimer(id, 1);
    ASSERT_EQ(ret, true);
    XCollie::GetInstance().CancelTimer(id);

    /**
     * @tc.steps: step3. input param timeout is invalid
     * @tc.expected: step3. set timer failed;
     */
    id = XCollie::GetInstance().SetTimer("Timer", 0, nullptr, nullptr, XCOLLIE_FLAG_NOOP);
    ASSERT_EQ(id, INVALID_ID);

    /**
     * @tc.steps: step4. cancelTimer, input param id is invalid
     */
    XCollie::GetInstance().CancelTimer(-1);

    /**
     * @tc.steps: step5. updateTimer, param id is invalid
     * @tc.expected: step5. update timer failed;
     */
    ret = XCollie::GetInstance().UpdateTimer(-1, 10);
    ASSERT_EQ(ret, false);

    /**
     * @tc.steps: step6. updateTimer, param timeout is invalid
     * @tc.expected: step6. update timer failed;
     */
    ret = XCollie::GetInstance().UpdateTimer(10, 0);
    ASSERT_EQ(ret, false);
}
} // namespace HiviewDFX
} // namespace OHOS
