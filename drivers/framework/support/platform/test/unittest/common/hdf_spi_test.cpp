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

// pal spi test case number
enum HdfTestCaseCmd {
    SPI_TRANSFER_TEST,
    SPI_DMA_TRANSFER_TEST,
    SPI_INT_TRANSFER_TEST,
    SPI_RELIABILITY_TEST,
    SPI_PERFORMANCE_TEST,
};

/*enum HdfTestCaseCmd {
    SPI_SET_CFG_TEST = 0,
    SPI_TRANSFER_TEST,
    SPI_WRITE_TEST,
    SPI_READ_TEST,
    SPI_DMA_TEST,
    SPI_INTERRUPT_TEST,
    SPI_RELIABILITY_TEST,
    SPI_PERFORMANCE_TEST,
};
*/

class HdfLiteSpiTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();
};

void HdfLiteSpiTest::SetUpTestCase()
{
    HdfTestOpenService();
}

void HdfLiteSpiTest::TearDownTestCase()
{
    HdfTestCloseService();
}

void HdfLiteSpiTest::SetUp()
{
}

void HdfLiteSpiTest::TearDown()
{
}

/**
  * @tc.name: SpiSetCfgTest001
  * @tc.desc: spi function test
  * @tc.type: FUNC
  * @tc.require: SR000DQ0VO
  */
/*HWTEST_F(HdfLiteSpiTest, SpiSetCfgTest001, TestSize.Level1)
{
    struct HdfTestMsg msg = {TEST_PAL_SPI_TYPE, SPI_SET_CFG_TEST, -1};
    EXPECT_EQ(0, HdfTestSendMsgToService(&msg));
}
*/

/**
  * @tc.name: SpiTransferTest001
  * @tc.desc: spi function test
  * @tc.type: FUNC
  * @tc.require: SR000DQ0VO
  */
HWTEST_F(HdfLiteSpiTest, SpiTransferTest001, TestSize.Level1)
{
    struct HdfTestMsg msg = {TEST_PAL_SPI_TYPE, SPI_TRANSFER_TEST, -1};
    EXPECT_EQ(0, HdfTestSendMsgToService(&msg));
}

/**
  * @tc.name: SpiWriteTest001
  * @tc.desc: spi function test
  * @tc.type: FUNC
  * @tc.require: SR000DQ0VO
  */
/*HWTEST_F(HdfLiteSpiTest, SpiWriteTest001, TestSize.Level1)
{
    struct HdfTestMsg msg = { TEST_PAL_SPI_TYPE, SPI_WRITE_TEST, -1};
    EXPECT_EQ(0, HdfTestSendMsgToService(&msg));
}
*/

/**
  * @tc.name: SpiReadTest001
  * @tc.desc: Spi function test
  * @tc.type: FUNC
  * @tc.require: SR000DQ0VO
  */
/*HWTEST_F(HdfLiteSpiTest, SpiReadTest001, TestSize.Level1)
{
    struct HdfTestMsg msg = {TEST_PAL_SPI_TYPE, SPI_READ_TEST, -1};
    EXPECT_EQ(0, HdfTestSendMsgToService(&msg));
}
*/

/**
  * @tc.name: SpiDmaTransferTest001
  * @tc.desc: Spi function test
  * @tc.type: FUNC
  * @tc.require: NA
  */
HWTEST_F(HdfLiteSpiTest, SpiDmaTransferTest001, TestSize.Level1)
{
    struct HdfTestMsg msg = {TEST_PAL_SPI_TYPE, SPI_DMA_TRANSFER_TEST, -1};
    EXPECT_EQ(0, HdfTestSendMsgToService(&msg));
}

/**
  * @tc.name: SpiIntTransferTest001
  * @tc.desc: Spi function test
  * @tc.type: FUNC
  * @tc.require: NA
  */
HWTEST_F(HdfLiteSpiTest, SpiIntTransferTest001, TestSize.Level1)
{
    struct HdfTestMsg msg = {TEST_PAL_SPI_TYPE, SPI_INT_TRANSFER_TEST, -1};
    EXPECT_EQ(0, HdfTestSendMsgToService(&msg));
}

/**
  * @tc.name: SpiReliabilityTest001
  * @tc.desc: spi function test
  * @tc.type: FUNC
  * @tc.require: SR000DQ0VO
  */
HWTEST_F(HdfLiteSpiTest, SpiReliabilityTest001, TestSize.Level1)
{
    struct HdfTestMsg msg = {TEST_PAL_SPI_TYPE, SPI_RELIABILITY_TEST, -1};
    EXPECT_EQ(0, HdfTestSendMsgToService(&msg));
}
