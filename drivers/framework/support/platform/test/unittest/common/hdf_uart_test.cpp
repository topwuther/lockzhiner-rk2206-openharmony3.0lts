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
#include "hdf_io_service_if.h"

using namespace testing::ext;

static int g_uartFd;
static const string HDF_TEST_NAME  = "/dev/hdf_test";

enum HdfLiteUartTestCmd {
    UAER_WRITE_TEST = 0,
    UART_READ_TEST,
    UART_SET_BAUD_TEST,
    UART_GET_BAUD_TEST,
    UART_SET_ATTRIBUTE_TEST,
    UART_GET_ATTRIBUTE_TEST,
    UART_SET_TRANSMODE_TEST,
    UART_RELIABILITY_TEST,
    UART_PERFORMANCE_TEST,
};

class HdfLiteUartTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();
};

void HdfLiteUartTest::SetUpTestCase()
{
    HdfTestOpenService();
}

void HdfLiteUartTest::TearDownTestCase()
{
    HdfTestCloseService();
}

void HdfLiteUartTest::SetUp()
{
}

void HdfLiteUartTest::TearDown()
{
}

#ifdef HDF_LITEOS_TEST
/**
 * @tc.name: UartSetTransModeTest001
 * @tc.desc: uart function test
 * @tc.type: FUNC
 * @tc.require: AR000F8689
 */
HWTEST_F(HdfLiteUartTest, UartSetTransModeTest001, TestSize.Level1)
{
    struct HdfTestMsg msg = {TEST_PAL_UART_TYPE, UART_SET_TRANSMODE_TEST, -1};
    EXPECT_EQ(0, HdfTestSendMsgToService(&msg));
}
#endif

/**
  * @tc.name: UartWriteTest001
  * @tc.desc: uart function test
  * @tc.type: FUNC
  * @tc.require: AR000F8689
  */
HWTEST_F(HdfLiteUartTest, UartWriteTest001, TestSize.Level1)
{
    struct HdfTestMsg msg = {TEST_PAL_UART_TYPE, UAER_WRITE_TEST, -1};
    EXPECT_EQ(0, HdfTestSendMsgToService(&msg));
}

/**
  * @tc.name: UartReadTest001
  * @tc.desc: uart function test
  * @tc.type: FUNC
  * @tc.require: AR000F8689
  */
HWTEST_F(HdfLiteUartTest, UartReadTest001, TestSize.Level1)
{
    struct HdfTestMsg msg = { TEST_PAL_UART_TYPE, UART_READ_TEST, -1};
    EXPECT_EQ(0, HdfTestSendMsgToService(&msg));
}

/**
  * @tc.name: UartSetBaudTest001
  * @tc.desc: uart function test
  * @tc.type: FUNC
  * @tc.require: AR000F8689
  */
HWTEST_F(HdfLiteUartTest, UartSetBaudTest001, TestSize.Level1)
{
    struct HdfTestMsg msg = {TEST_PAL_UART_TYPE, UART_SET_BAUD_TEST, -1};
    EXPECT_EQ(0, HdfTestSendMsgToService(&msg));
}

/**
  * @tc.name: UartGetBaudTest001
  * @tc.desc: uart function test
  * @tc.type: FUNC
  * @tc.require: AR000F8689
  */
HWTEST_F(HdfLiteUartTest, UartGetBaudTest001, TestSize.Level1)
{
    struct HdfTestMsg msg = {TEST_PAL_UART_TYPE, UART_GET_BAUD_TEST, -1};
    EXPECT_EQ(0, HdfTestSendMsgToService(&msg));
}

/**
  * @tc.name: UartSetAttributeTest001
  * @tc.desc: uart function test
  * @tc.type: FUNC
  * @tc.require: AR000F8689
  */
HWTEST_F(HdfLiteUartTest, UartSetAttributeTest001, TestSize.Level1)
{
    struct HdfTestMsg msg = {TEST_PAL_UART_TYPE, UART_SET_ATTRIBUTE_TEST, -1};
    EXPECT_EQ(0, HdfTestSendMsgToService(&msg));
}

/**
  * @tc.name: UartGetAttributeTest001
  * @tc.desc: uart function test
  * @tc.type: FUNC
  * @tc.require: AR000F8689
  */
HWTEST_F(HdfLiteUartTest, UartGetAttributeTest001, TestSize.Level1)
{
    struct HdfTestMsg msg = {TEST_PAL_UART_TYPE, UART_GET_ATTRIBUTE_TEST, -1};
    EXPECT_EQ(0, HdfTestSendMsgToService(&msg));
}

/**
  * @tc.name: UartReliabilityTest001
  * @tc.desc: uart function test
  * @tc.type: FUNC
  * @tc.require: AR000F8689
  */
HWTEST_F(HdfLiteUartTest, UartReliabilityTest001, TestSize.Level1)
{
    struct HdfTestMsg msg = {TEST_PAL_UART_TYPE, UART_RELIABILITY_TEST, -1};
    EXPECT_EQ(0, HdfTestSendMsgToService(&msg));
}
