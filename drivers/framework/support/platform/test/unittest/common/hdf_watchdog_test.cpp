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
#include <string>
#include <unistd.h>
#include <gtest/gtest.h>
#include "hdf_uhdf_test.h"
#include "hdf_io_service_if.h"

using namespace testing::ext;

// pal watchdog test case number
enum WatchdogTestCmd {
    WATCHDOG_TEST_SET_GET_TIMEOUT = 0,
    WATCHDOG_TEST_START_STOP = 1,
    WATCHDOG_TEST_FEED = 2,
    WATCHDOG_TEST_RELIABILITY = 3,
    WATCHDOG_TEST_BARK = 4,
    WATCHDOG_TEST_MAX = 5,
};

class HdfLiteWatchdogTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();
};

void HdfLiteWatchdogTest::SetUpTestCase()
{
    HdfTestOpenService();
}

void HdfLiteWatchdogTest::TearDownTestCase()
{
    HdfTestCloseService();
}

void HdfLiteWatchdogTest::SetUp()
{
}

void HdfLiteWatchdogTest::TearDown()
{
}

/**
  * @tc.name: HdfLiteWatchdogTestSetGetTimeout001
  * @tc.desc: watchdog function test
  * @tc.type: FUNC
  * @tc.require: AR000F868G
  */
HWTEST_F(HdfLiteWatchdogTest, HdfLiteWatchdogTestSetGetTimeout001, TestSize.Level1)
{
    struct HdfTestMsg msg = {TEST_PAL_WDT_TYPE, WATCHDOG_TEST_SET_GET_TIMEOUT, -1};
    EXPECT_EQ(0, HdfTestSendMsgToService(&msg));
}

/**
  * @tc.name: HdfLiteWatchdogTestStartStop001
  * @tc.desc: watchdog function test
  * @tc.type: FUNC
  * @tc.require: AR000F868G
  */
HWTEST_F(HdfLiteWatchdogTest, HdfLiteWatchdogTestStartStop001, TestSize.Level1)
{
    struct HdfTestMsg msg = {TEST_PAL_WDT_TYPE, WATCHDOG_TEST_START_STOP, -1};
    EXPECT_EQ(0, HdfTestSendMsgToService(&msg));
}

/**
  * @tc.name: HdfLiteWatchdogTestFeed001
  * @tc.desc: watchdog function test
  * @tc.type: FUNC
  * @tc.require: AR000F868G
  */
HWTEST_F(HdfLiteWatchdogTest, HdfLiteWatchdogTestFeed001, TestSize.Level1)
{
    struct HdfTestMsg msg = {TEST_PAL_WDT_TYPE, WATCHDOG_TEST_FEED, -1};
    EXPECT_EQ(0, HdfTestSendMsgToService(&msg));
}

/**
  * @tc.name: HdfLiteWatchdogTestReliability001
  * @tc.desc: watchdog function test
  * @tc.type: FUNC
  * @tc.require: AR000F868G
  */
HWTEST_F(HdfLiteWatchdogTest, HdfLiteWatchdogTestReliability001, TestSize.Level1)
{
    struct HdfTestMsg msg = {TEST_PAL_WDT_TYPE, WATCHDOG_TEST_RELIABILITY, -1};
    EXPECT_EQ(0, HdfTestSendMsgToService(&msg));
}

