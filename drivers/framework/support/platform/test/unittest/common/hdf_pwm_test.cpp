/*
 * Copyright (c) 2020-2021 Huawei Device Co., Ltd.
 *
 * HDF is dual licensed: you can use it either under the terms of
 * the GPL, or the BSD license, at your option.
 * See the LICENSE file in the root of this repository for complete details.
 */

#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <fcntl.h>
#include <gtest/gtest.h>
#include <string>
#include <unistd.h>
#include "hdf_uhdf_test.h"

using namespace testing::ext;

enum PwmTestCmd {
    PWM_SET_PERIOD_TEST = 0,
    PWM_SET_DUTY_TEST,
    PWM_SET_POLARITY_TEST,
    PWM_ENABLE_TEST,
    PWM_DISABLE_TEST,
    PWM_SET_CONFIG_TEST,
    PWM_GET_CONFIG_TEST,
    PWM_RELIABILITY_TEST,
    PWM_TEST_ALL,
};

class HdfLitePwmTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();
};

void HdfLitePwmTest::SetUpTestCase()
{
    HdfTestOpenService();
}

void HdfLitePwmTest::TearDownTestCase()
{
    HdfTestCloseService();
}

void HdfLitePwmTest::SetUp()
{
}

void HdfLitePwmTest::TearDown()
{
}

/**
  * @tc.name: PwmSetPeriodTest001
  * @tc.desc: pwm function test
  * @tc.type: FUNC
  * @tc.require: AR000F868C
  */
HWTEST_F(HdfLitePwmTest, PwmSetPeriodTest001, TestSize.Level1)
{
    struct HdfTestMsg msg = {TEST_PAL_PWM_TYPE, PWM_SET_PERIOD_TEST, -1};
    EXPECT_EQ(0, HdfTestSendMsgToService(&msg));
}

/**
  * @tc.name: PwmSetDutyTest001
  * @tc.desc: pwm function test
  * @tc.type: FUNC
  * @tc.require: AR000F868C
  */
HWTEST_F(HdfLitePwmTest, PwmSetDutyTest001, TestSize.Level1)
{
    struct HdfTestMsg msg = {TEST_PAL_PWM_TYPE, PWM_SET_DUTY_TEST, -1};
    EXPECT_EQ(0, HdfTestSendMsgToService(&msg));
}

/**
  * @tc.name: PwmSetPolarityTest001
  * @tc.desc: pwm function test
  * @tc.type: FUNC
  * @tc.require: AR000F868C
  */
HWTEST_F(HdfLitePwmTest, PwmSetPolarityTest001, TestSize.Level1)
{
    struct HdfTestMsg msg = { TEST_PAL_PWM_TYPE, PWM_SET_POLARITY_TEST, -1};
    EXPECT_EQ(0, HdfTestSendMsgToService(&msg));
}

/**
  * @tc.name: PwmSetConfigTest001
  * @tc.desc: pwm function test
  * @tc.type: FUNC
  * @tc.require: AR000F868D
  */
HWTEST_F(HdfLitePwmTest, PwmSetConfigTest001, TestSize.Level1)
{
    struct HdfTestMsg msg = {TEST_PAL_PWM_TYPE, PWM_SET_CONFIG_TEST, -1};
    EXPECT_EQ(0, HdfTestSendMsgToService(&msg));
}

/**
  * @tc.name: PwmGetConfigTest001
  * @tc.desc: pwm function test
  * @tc.type: FUNC
  * @tc.require: AR000F868D
  */
HWTEST_F(HdfLitePwmTest, PwmGetConfigTest001, TestSize.Level1)
{
    struct HdfTestMsg msg = {TEST_PAL_PWM_TYPE, PWM_GET_CONFIG_TEST, -1};
    EXPECT_EQ(0, HdfTestSendMsgToService(&msg));
}

/**
  * @tc.name: PwmEnableTest001
  * @tc.desc: pwm function test
  * @tc.type: FUNC
  * @tc.require: AR000F868D
  */
HWTEST_F(HdfLitePwmTest, PwmEnableTest001, TestSize.Level1)
{
    struct HdfTestMsg msg = {TEST_PAL_PWM_TYPE, PWM_ENABLE_TEST, -1};
    EXPECT_EQ(0, HdfTestSendMsgToService(&msg));
}

/**
  * @tc.name: PwmDisableTest001
  * @tc.desc: pwm function test
  * @tc.type: FUNC
  * @tc.require: AR000F868D
  */
HWTEST_F(HdfLitePwmTest, PwmDisableTest001, TestSize.Level1)
{
    struct HdfTestMsg msg = {TEST_PAL_PWM_TYPE, PWM_DISABLE_TEST, -1};
    EXPECT_EQ(0, HdfTestSendMsgToService(&msg));
}
