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

static int g_UsbFd;
static const string HDF_TEST_NAME  = "/dev/hdf_test";

enum HdfLiteUsbTestCmd {
    USB_WRITE_TEST = 0,
    USB_HOSTSDK_INIT_001_TEST,
    Usb_HOSTSDK_EXIT_001_TEST,
    USB_HOSTSDK_INIT_002_TEST,
    USB_HOSTSDK_EXIT_002_TEST,
    USB_HOSTSDK_INIT_003_TEST,
    USB_HOSTSDK_INIT_004_TEST,
    USB_HOSTSDK_INIT_005_TEST,
    USB_HOSTSDK_CLAIM_INTERFACE_001_TEST,
    USB_HOSTSDK_RELEASE_INTERFACE_001_TEST,
    USB_HOSTSDK_CLAIM_INTERFACE_002_TEST,
    USB_HOSTSDK_RELEASE_INTERFACE_002_TEST,
    USB_HOSTSDK_CLAIM_INTERFACE_003_TEST,
    USB_HOSTSDK_RELEASE_INTERFACE_003_TEST,
    USB_HOSTSDK_CLAIM_INTERFACE_004_TEST,
    USB_HOSTSDK_CLAIM_INTERFACE_005_TEST,
    USB_HOSTSDK_CLAIM_INTERFACE_006_TEST,
    USB_HOSTSDK_OPEN_INTERFACE_001_TEST,
    USB_HOSTSDK_CLOSE_INTERFACE_001_TEST,
    USB_HOSTSDK_OPEN_INTERFACE_002_TEST,
    USB_HOSTSDK_CLOSE_INTERFACE_002_TEST,
    USB_HOSTSDK_OPEN_INTERFACE_003_TEST,
    USB_HOSTSDK_CLOSE_INTERFACE_003_TEST,
    USB_HOSTSDK_OPEN_INTERFACE_004_TEST,
    USB_HOSTSDK_OPEN_INTERFACE_005_TEST,
    USB_HOSTSDK_CLOSE_INTERFACE_005_TEST,
    USB_HOSTSDK_OPEN_INTERFACE_006_TEST,
    USB_HOSTSDK_SELECT_INTERFACE_001_TEST,
    USB_HOSTSDK_SELECT_INTERFACE_002_TEST,
    USB_HOSTSDK_SELECT_INTERFACE_003_TEST,
    USB_HOSTSDK_SELECT_INTERFACE_004_TEST,
    USB_HOSTSDK_SELECT_INTERFACE_005_TEST,
    USB_HOSTSDK_SELECT_INTERFACE_006_TEST,
    USB_HOSTSDK_CLAIM_INTERFACE_007_TEST,
    USB_HOSTSDK_CLAIM_INTERFACE_008_TEST,
    USB_HOSTSDK_GET_PIPE_001_TEST,
    USB_HOSTSDK_GET_PIPE_002_TEST,
    USB_HOSTSDK_GET_PIPE_003_TEST,
    USB_HOSTSDK_GET_PIPE_004_TEST,
    USB_HOSTSDK_GET_PIPE_005_TEST,
    USB_HOSTSDK_GET_PIPE_006_TEST,
    USB_HOSTSDK_GET_PIPE_007_TEST,
    USB_HOSTSDK_GET_PIPE_008_TEST,
    USB_HOSTSDK_ALLOC_REQUEST_001_TEST,
    USB_HOSTSDK_ALLOC_REQUEST_002_TEST,
    USB_HOSTSDK_FREE_REQUEST_001_TEST,
    USB_HOSTSDK_ALLOC_REQUEST_003_TEST,
    USB_HOSTSDK_ALLOC_REQUEST_004_TEST,
    USB_HOSTSDK_FREE_REQUEST_002_TEST,
    USB_HOSTSDK_ALLOC_REQUEST_005_TEST,
    USB_HOSTSDK_ALLOC_REQUEST_006_TEST,
    USB_HOSTSDK_FREE_REQUEST_003_TEST,
    USB_HOSTSDK_ALLOC_REQUEST_007_TEST,
    USB_HOSTSDK_ALLOC_REQUEST_008_TEST,
    USB_HOSTSDK_ALLOC_REQUEST_010_TEST,
    USB_HOSTSDK_ALLOC_REQUEST_011_TEST,
    USB_HOSTSDK_FREE_REQUEST_006_TEST,
    USB_HOSTSDK_FILL_ISO_REQUEST_001_TEST,
    USB_HOSTSDK_FILL_ISO_REQUEST_002_TEST,
    USB_HOSTSDK_FILL_ISO_REQUEST_003_TEST,
    USB_HOSTSDK_FILL_ISO_REQUEST_004_TEST,
    USB_HOSTSDK_FILL_ISO_REQUEST_005_TEST,
    USB_HOSTSDK_FILL_ISO_REQUEST_006_TEST,
    USB_HOSTSDK_FREE_REQUEST_004_TEST,
    USB_HOSTSDK_ALLOC_REQUEST_009_TEST,
    USB_HOSTSDK_FILL_REQUEST_001_TEST,
    USB_HOSTSDK_FILL_REQUEST_002_TEST,
    USB_HOSTSDK_FILL_REQUEST_003_TEST,
    USB_HOSTSDK_FILL_REQUEST_004_TEST,
    USB_HOSTSDK_SUBMIT_REQUEST_SYNC_001_TEST,
    USB_HOSTSDK_SUBMIT_REQUEST_SYNC_002_TEST,
    USB_HOSTSDK_SUBMIT_REQUEST_SYNC_003_TEST,
    USB_HOSTSDK_SUBMIT_REQUEST_SYNC_004_TEST,
    USB_HOSTSDK_FILL_REQUEST_005_TEST,
    USB_HOSTSDK_FILL_REQUEST_006_TEST,
    USB_HOSTSDK_FILL_REQUEST_007_TEST,
    USB_HOSTSDK_FILL_REQUEST_008_TEST,
    USB_HOSTSDK_SUBMIT_REQUEST_ASYNC_001_TEST,
    USB_HOSTSDK_SUBMIT_CANCEL_REQUEST_001_TEST,
    USB_HOSTSDK_SUBMIT_REQUEST_ASYNC_002_TEST,
    USB_HOSTSDK_SUBMIT_CANCEL_REQUEST_002_TEST,
    USB_HOSTSDK_SUBMIT_REQUEST_ASYNC_003_TEST,
    USB_HOSTSDK_SUBMIT_CANCEL_REQUEST_003_TEST,
    USB_HOSTSDK_SUBMIT_REQUEST_ASYNC_004_TEST,
    USB_HOSTSDK_SUBMIT_CANCEL_REQUEST_004_TEST,
    USB_HOSTSDK_CLEAR_INTERFACE_HALT_002_TEST,
    USB_HOSTSDK_CLEAR_INTERFACE_HALT_003_TEST,
    USB_HOSTSDK_CLEAR_INTERFACE_HALT_004_TEST,
    USB_HOSTSDK_REMOVE_INTERFACE_001_TEST,
    USB_HOSTSDK_ADD_INTERFACE_001_TEST,
    USB_HOSTSDK_REMOVE_INTERFACE_002_TEST,
    USB_HOSTSDK_ADD_INTERFACE_002_TEST,
    USB_HOSTSDK_REMOVE_INTERFACE_003_TEST,
    USB_HOSTSDK_ADD_INTERFACE_003_TEST,
    USB_HOSTSDK_CLOSE_INTERFACE_006_TEST,
};

class HdfLiteUsbTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();
};

void HdfLiteUsbTest::SetUpTestCase()
{
    HdfTestOpenService();
}

void HdfLiteUsbTest::TearDownTestCase()
{
    HdfTestCloseService();
}

void HdfLiteUsbTest::SetUp()
{
}

void HdfLiteUsbTest::TearDown()
{
}
/**
 * @tc.number    : CheckHostSdkIfInit001
 * @tc.name      :
 * @tc.type      : PERF
 * @tc.level     : Level 1
 */
HWTEST_F(HdfLiteUsbTest, CheckHostSdkIfInit001, TestSize.Level1)
{
    struct HdfTestMsg msg = {TEST_USB_HOST_TYPE, USB_HOSTSDK_INIT_001_TEST, -1};
    EXPECT_EQ(0, HdfTestSendMsgToService(&msg));
}

/**
 * @tc.number    : CheckHostSdkIfExit001
 * @tc.name      :
 * @tc.type      : PERF
 * @tc.level     : Level 1
 */
HWTEST_F(HdfLiteUsbTest, CheckHostSdkIfExit001, TestSize.Level1)
{
    struct HdfTestMsg msg = {TEST_USB_HOST_TYPE, Usb_HOSTSDK_EXIT_001_TEST, -1};
    EXPECT_EQ(0, HdfTestSendMsgToService(&msg));
}

/**
 * @tc.number    : CheckHostSdkIfInit002
 * @tc.name      :
 * @tc.type      : PERF
 * @tc.level     : Level 1
 */
HWTEST_F(HdfLiteUsbTest, CheckHostSdkIfInit002, TestSize.Level1)
{
    struct HdfTestMsg msg = {TEST_USB_HOST_TYPE, USB_HOSTSDK_INIT_002_TEST, -1};
    EXPECT_EQ(0, HdfTestSendMsgToService(&msg));
}

/**
 * @tc.number    : CheckHostSdkIfExit002
 * @tc.name      :
 * @tc.type      : PERF
 * @tc.level     : Level 1
 */
HWTEST_F(HdfLiteUsbTest, CheckHostSdkIfExit002, TestSize.Level1)
{   struct HdfTestMsg msg = {TEST_USB_HOST_TYPE, USB_HOSTSDK_EXIT_002_TEST, -1};
    EXPECT_EQ(0, HdfTestSendMsgToService(&msg));
}

/**
 * @tc.number    : CheckHostSdkIfInit003
 * @tc.name      :
 * @tc.type      : PERF
 * @tc.level     : Level 1
 */
HWTEST_F(HdfLiteUsbTest, CheckHostSdkIfInit003, TestSize.Level1)
{
    struct HdfTestMsg msg = {TEST_USB_HOST_TYPE, USB_HOSTSDK_INIT_003_TEST, -1};
    EXPECT_EQ(0, HdfTestSendMsgToService(&msg));
}

/**
 * @tc.number    : CheckHostSdkIfInit004
 * @tc.name      :
 * @tc.type      : PERF
 * @tc.level     : Level 1
 */
HWTEST_F(HdfLiteUsbTest, CheckHostSdkIfInit004, TestSize.Level1)
{
    struct HdfTestMsg msg = {TEST_USB_HOST_TYPE, USB_HOSTSDK_INIT_004_TEST, -1};
    EXPECT_EQ(0, HdfTestSendMsgToService(&msg));
}

/**
 * @tc.number    : CheckHostSdkIfInit005
 * @tc.name      :
 * @tc.type      : PERF
 * @tc.level     : Level 1
 */
HWTEST_F(HdfLiteUsbTest, CheckHostSdkIfInit005, TestSize.Level1)
{
    struct HdfTestMsg msg = {TEST_USB_HOST_TYPE, USB_HOSTSDK_INIT_005_TEST, -1};
    EXPECT_EQ(0, HdfTestSendMsgToService(&msg));
}

/**
 * @tc.number    : CheckHostSdkIfClaimInterface001
 * @tc.name      :
 * @tc.type      : PERF
 * @tc.level     : Level 1
 */
HWTEST_F(HdfLiteUsbTest, CheckHostSdkIfClaimInterface001, TestSize.Level1)
{
    struct HdfTestMsg msg = {TEST_USB_HOST_TYPE, USB_HOSTSDK_CLAIM_INTERFACE_001_TEST, -1};
    EXPECT_EQ(0, HdfTestSendMsgToService(&msg));
}

/**
 * @tc.number    : CheckHostSdkIfReleaseInterface001
 * @tc.name      :
 * @tc.type      : PERF
 * @tc.level     : Level 1
 */
HWTEST_F(HdfLiteUsbTest, CheckHostSdkIfReleaseInterface001, TestSize.Level1)
{
    struct HdfTestMsg msg = {TEST_USB_HOST_TYPE, USB_HOSTSDK_RELEASE_INTERFACE_001_TEST, -1};
    EXPECT_EQ(0, HdfTestSendMsgToService(&msg));
}

/**
 * @tc.number    : CheckHostSdkIfClaimInterface002
 * @tc.name      :
 * @tc.type      : PERF
 * @tc.level     : Level 1
 */
HWTEST_F(HdfLiteUsbTest, CheckHostSdkIfClaimInterface002, TestSize.Level1)
{
    struct HdfTestMsg msg = {TEST_USB_HOST_TYPE, USB_HOSTSDK_CLAIM_INTERFACE_002_TEST, -1};
    EXPECT_EQ(0, HdfTestSendMsgToService(&msg));
}

/**
 * @tc.number    : CheckHostSdkIfReleaseInterface002
 * @tc.name      :
 * @tc.type      : PERF
 * @tc.level     : Level 1
 */
HWTEST_F(HdfLiteUsbTest, CheckHostSdkIfReleaseInterface002, TestSize.Level1)
{
    struct HdfTestMsg msg = {TEST_USB_HOST_TYPE, USB_HOSTSDK_RELEASE_INTERFACE_002_TEST, -1};
    EXPECT_EQ(0, HdfTestSendMsgToService(&msg));
}

/**
 * @tc.number    : CheckHostSdkIfClaimInterface003
 * @tc.name      :
 * @tc.type      : PERF
 * @tc.level     : Level 1
 */
HWTEST_F(HdfLiteUsbTest, CheckHostSdkIfClaimInterface003, TestSize.Level1)
{
    struct HdfTestMsg msg = {TEST_USB_HOST_TYPE, USB_HOSTSDK_CLAIM_INTERFACE_003_TEST, -1};
    EXPECT_EQ(0, HdfTestSendMsgToService(&msg));
}

/**
 * @tc.number    : CheckHostSdkIfReleaseInterface003
 * @tc.name      :
 * @tc.type      : PERF
 * @tc.level     : Level 1
 */
HWTEST_F(HdfLiteUsbTest, CheckHostSdkIfReleaseInterface003, TestSize.Level1)
{
    struct HdfTestMsg msg = {TEST_USB_HOST_TYPE, USB_HOSTSDK_RELEASE_INTERFACE_003_TEST, -1};
    EXPECT_EQ(0, HdfTestSendMsgToService(&msg));
}

/**
 * @tc.number    : CheckHostSdkIfClaimInterface004
 * @tc.name      :
 * @tc.type      : PERF
 * @tc.level     : Level 1
 */
HWTEST_F(HdfLiteUsbTest, CheckHostSdkIfClaimInterface004, TestSize.Level1)
{
    struct HdfTestMsg msg = {TEST_USB_HOST_TYPE, USB_HOSTSDK_CLAIM_INTERFACE_004_TEST, -1};
    EXPECT_EQ(0, HdfTestSendMsgToService(&msg));
}

/**
 * @tc.number    : CheckHostSdkIfClaimInterface005
 * @tc.name      :
 * @tc.type      : PERF
 * @tc.level     : Level 1
 */
HWTEST_F(HdfLiteUsbTest, CheckHostSdkIfClaimInterface005, TestSize.Level1)
{
    struct HdfTestMsg msg = {TEST_USB_HOST_TYPE, USB_HOSTSDK_CLAIM_INTERFACE_005_TEST, -1};
    EXPECT_EQ(0, HdfTestSendMsgToService(&msg));
}

/**
 * @tc.number    : CheckHostSdkIfClaimInterface006
 * @tc.name      :
 * @tc.type      : PERF
 * @tc.level     : Level 1
 */
HWTEST_F(HdfLiteUsbTest, CheckHostSdkIfClaimInterface006, TestSize.Level1)
{
    struct HdfTestMsg msg = {TEST_USB_HOST_TYPE, USB_HOSTSDK_CLAIM_INTERFACE_006_TEST, -1};
    EXPECT_EQ(0, HdfTestSendMsgToService(&msg));
}

/**
 * @tc.number    : CheckHostSdkIfOpenInterface001
 * @tc.name      :
 * @tc.type      : PERF
 * @tc.level     : Level 1
 */
HWTEST_F(HdfLiteUsbTest, CheckHostSdkIfOpenInterface001, TestSize.Level1)
{
    struct HdfTestMsg msg = {TEST_USB_HOST_TYPE, USB_HOSTSDK_OPEN_INTERFACE_001_TEST, -1};
    EXPECT_EQ(0, HdfTestSendMsgToService(&msg));
}

/**
 * @tc.number    : CheckHostSdkIfCloseInterface001
 * @tc.name      :
 * @tc.type      : PERF
 * @tc.level     : Level 1
 */
HWTEST_F(HdfLiteUsbTest, CheckHostSdkIfCloseInterface001, TestSize.Level1)
{
    struct HdfTestMsg msg = {TEST_USB_HOST_TYPE, USB_HOSTSDK_CLOSE_INTERFACE_001_TEST, -1};
    EXPECT_EQ(0, HdfTestSendMsgToService(&msg));
}

/**
 * @tc.number    : CheckHostSdkIfOpenInterface002
 * @tc.name      :
 * @tc.type      : PERF
 * @tc.level     : Level 1
 */
HWTEST_F(HdfLiteUsbTest, CheckHostSdkIfOpenInterface002, TestSize.Level1)
{
    struct HdfTestMsg msg = {TEST_USB_HOST_TYPE, USB_HOSTSDK_OPEN_INTERFACE_002_TEST, -1};
    EXPECT_EQ(0, HdfTestSendMsgToService(&msg));
}

/**
 * @tc.number    : CheckHostSdkIfCloseInterface002
 * @tc.name      :
 * @tc.type      : PERF
 * @tc.level     : Level 1
 */
HWTEST_F(HdfLiteUsbTest, CheckHostSdkIfCloseInterface002, TestSize.Level1)
{
    struct HdfTestMsg msg = {TEST_USB_HOST_TYPE, USB_HOSTSDK_CLOSE_INTERFACE_002_TEST, -1};
    EXPECT_EQ(0, HdfTestSendMsgToService(&msg));
}

/**
 * @tc.number    : CheckHostSdkIfOpenInterface003
 * @tc.name      :
 * @tc.type      : PERF
 * @tc.level     : Level 1
 */
HWTEST_F(HdfLiteUsbTest, CheckHostSdkIfOpenInterface003, TestSize.Level1)
{
    struct HdfTestMsg msg = {TEST_USB_HOST_TYPE, USB_HOSTSDK_OPEN_INTERFACE_003_TEST, -1};
    EXPECT_EQ(0, HdfTestSendMsgToService(&msg));
}

/**
 * @tc.number    : CheckHostSdkIfCloseInterface003
 * @tc.name      :
 * @tc.type      : PERF
 * @tc.level     : Level 1
 */
HWTEST_F(HdfLiteUsbTest, CheckHostSdkIfCloseInterface003, TestSize.Level1)
{
    struct HdfTestMsg msg = {TEST_USB_HOST_TYPE, USB_HOSTSDK_CLOSE_INTERFACE_003_TEST, -1};
    EXPECT_EQ(0, HdfTestSendMsgToService(&msg));
}

/**
 * @tc.number    : CheckHostSdkIfOpenInterface004
 * @tc.name      :
 * @tc.type      : PERF
 * @tc.level     : Level 1
 */
HWTEST_F(HdfLiteUsbTest, CheckHostSdkIfOpenInterface004, TestSize.Level1)
{
    struct HdfTestMsg msg = {TEST_USB_HOST_TYPE, USB_HOSTSDK_OPEN_INTERFACE_004_TEST, -1};
    EXPECT_EQ(0, HdfTestSendMsgToService(&msg));
}

/**
 * @tc.number    : CheckHostSdkIfOpenInterface005
 * @tc.name      :
 * @tc.type      : PERF
 * @tc.level     : Level 1
 */
HWTEST_F(HdfLiteUsbTest, CheckHostSdkIfOpenInterface005, TestSize.Level1)
{
    struct HdfTestMsg msg = {TEST_USB_HOST_TYPE, USB_HOSTSDK_OPEN_INTERFACE_005_TEST, -1};
    EXPECT_EQ(0, HdfTestSendMsgToService(&msg));
}

/**
 * @tc.number    : CheckHostSdkIfCloseInterface005
 * @tc.name      :
 * @tc.type      : PERF
 * @tc.level     : Level 1
 */
HWTEST_F(HdfLiteUsbTest, CheckHostSdkIfCloseInterface005, TestSize.Level1)
{
    struct HdfTestMsg msg = {TEST_USB_HOST_TYPE, USB_HOSTSDK_CLOSE_INTERFACE_005_TEST, -1};
    EXPECT_EQ(0, HdfTestSendMsgToService(&msg));
}

/**
 * @tc.number    : CheckHostSdkIfOpenInterface006
 * @tc.name      :
 * @tc.type      : PERF
 * @tc.level     : Level 1
 */
HWTEST_F(HdfLiteUsbTest, CheckHostSdkIfOpenInterface006, TestSize.Level1)
{
    struct HdfTestMsg msg = {TEST_USB_HOST_TYPE, USB_HOSTSDK_OPEN_INTERFACE_006_TEST, -1};
    EXPECT_EQ(0, HdfTestSendMsgToService(&msg));
}

/**
 * @tc.number    : CheckHostSdkIfSelectInterfaceSetting001
 * @tc.name      :
 * @tc.type      : PERF
 * @tc.level     : Level 1
 */
HWTEST_F(HdfLiteUsbTest, CheckHostSdkIfSelectInterfaceSetting001, TestSize.Level1)
{
    struct HdfTestMsg msg = {TEST_USB_HOST_TYPE, USB_HOSTSDK_SELECT_INTERFACE_001_TEST, -1};
    EXPECT_EQ(0, HdfTestSendMsgToService(&msg));
}

/**
 * @tc.number    : CheckHostSdkIfSelectInterfaceSetting002
 * @tc.name      :
 * @tc.type      : PERF
 * @tc.level     : Level 1
 */
HWTEST_F(HdfLiteUsbTest, CheckHostSdkIfSelectInterfaceSetting002, TestSize.Level1)
{
    struct HdfTestMsg msg = {TEST_USB_HOST_TYPE, USB_HOSTSDK_SELECT_INTERFACE_002_TEST, -1};
    EXPECT_EQ(HDF_FAILURE, HdfTestSendMsgToService(&msg));
}

/**
 * @tc.number    : CheckHostSdkIfSelectInterfaceSetting003
 * @tc.name      :
 * @tc.type      : PERF
 * @tc.level     : Level 1
 */
HWTEST_F(HdfLiteUsbTest, CheckHostSdkIfSelectInterfaceSetting003, TestSize.Level1)
{
    struct HdfTestMsg msg = {TEST_USB_HOST_TYPE, USB_HOSTSDK_SELECT_INTERFACE_003_TEST, -1};
    EXPECT_EQ(HDF_FAILURE, HdfTestSendMsgToService(&msg));
}

/**
 * @tc.number    : CheckHostSdkIfSelectInterfaceSetting004
 * @tc.name      :
 * @tc.type      : PERF
 * @tc.level     : Level 1
 */
HWTEST_F(HdfLiteUsbTest, CheckHostSdkIfSelectInterfaceSetting004, TestSize.Level1)
{
    struct HdfTestMsg msg = {TEST_USB_HOST_TYPE, USB_HOSTSDK_SELECT_INTERFACE_004_TEST, -1};
    EXPECT_EQ(HDF_FAILURE, HdfTestSendMsgToService(&msg));
}

/**
 * @tc.number    : CheckHostSdkIfSelectInterfaceSetting005
 * @tc.name      :
 * @tc.type      : PERF
 * @tc.level     : Level 1
 */
HWTEST_F(HdfLiteUsbTest, CheckHostSdkIfSelectInterfaceSetting005, TestSize.Level1)
{
    struct HdfTestMsg msg = {TEST_USB_HOST_TYPE, USB_HOSTSDK_SELECT_INTERFACE_005_TEST, -1};
    EXPECT_EQ(HDF_FAILURE, HdfTestSendMsgToService(&msg));
}

/**
 * @tc.number    : CheckHostSdkIfSelectInterfaceSetting006
 * @tc.name      :
 * @tc.type      : PERF
 * @tc.level     : Level 1
 */
HWTEST_F(HdfLiteUsbTest, CheckHostSdkIfSelectInterfaceSetting006, TestSize.Level1)
{
    struct HdfTestMsg msg = {TEST_USB_HOST_TYPE, USB_HOSTSDK_SELECT_INTERFACE_006_TEST, -1};
    EXPECT_EQ(0, HdfTestSendMsgToService(&msg));
}

/**
 * @tc.number    : CheckHostSdkIfClaimInterface007
 * @tc.name      :
 * @tc.type      : PERF
 * @tc.level     : Level 1
 */
HWTEST_F(HdfLiteUsbTest, CheckHostSdkIfClaimInterface007, TestSize.Level1)
{
    struct HdfTestMsg msg = {TEST_USB_HOST_TYPE, USB_HOSTSDK_CLAIM_INTERFACE_007_TEST, -1};
    EXPECT_EQ(0, HdfTestSendMsgToService(&msg));
}

/**
 * @tc.number    : CheckHostSdkIfClaimInterface008
 * @tc.name      :
 * @tc.type      : PERF
 * @tc.level     : Level 1
 */
HWTEST_F(HdfLiteUsbTest, CheckHostSdkIfClaimInterface008, TestSize.Level1)
{
    struct HdfTestMsg msg = {TEST_USB_HOST_TYPE, USB_HOSTSDK_CLAIM_INTERFACE_008_TEST, -1};
    EXPECT_EQ(0, HdfTestSendMsgToService(&msg));
}

/**
 * @tc.number    : CheckHostSdkIfGetPipe001
 * @tc.name      :
 * @tc.type      : PERF
 * @tc.level     : Level 1
 */
HWTEST_F(HdfLiteUsbTest, CheckHostSdkIfGetPipe001, TestSize.Level1)
{
    struct HdfTestMsg msg = {TEST_USB_HOST_TYPE, USB_HOSTSDK_GET_PIPE_001_TEST, -1};
    EXPECT_EQ(HDF_FAILURE, HdfTestSendMsgToService(&msg));
}

/**
 * @tc.number    : CheckHostSdkIfGetPipe002
 * @tc.name      :
 * @tc.type      : PERF
 * @tc.level     : Level 1
 */
HWTEST_F(HdfLiteUsbTest, CheckHostSdkIfGetPipe002, TestSize.Level1)
{
    struct HdfTestMsg msg = {TEST_USB_HOST_TYPE, USB_HOSTSDK_GET_PIPE_002_TEST, -1};
    EXPECT_EQ(0, HdfTestSendMsgToService(&msg));
}

/**
 * @tc.number    : CheckHostSdkIfGetPipe003
 * @tc.name      :
 * @tc.type      : PERF
 * @tc.level     : Level 1
 */
HWTEST_F(HdfLiteUsbTest, CheckHostSdkIfGetPipe003, TestSize.Level1)
{
    struct HdfTestMsg msg = {TEST_USB_HOST_TYPE, USB_HOSTSDK_GET_PIPE_003_TEST, -1};
    EXPECT_EQ(HDF_FAILURE, HdfTestSendMsgToService(&msg));
}

/**
 * @tc.number    : CheckHostSdkIfGetPipe004
 * @tc.name      :
 * @tc.type      : PERF
 * @tc.level     : Level 1
 */
HWTEST_F(HdfLiteUsbTest, CheckHostSdkIfGetPipe004, TestSize.Level1)
{
    struct HdfTestMsg msg = {TEST_USB_HOST_TYPE, USB_HOSTSDK_GET_PIPE_004_TEST, -1};
    EXPECT_EQ(0, HdfTestSendMsgToService(&msg));
}

/**
 * @tc.number    : CheckHostSdkIfGetPipe005
 * @tc.name      :
 * @tc.type      : PERF
 * @tc.level     : Level 1
 */
HWTEST_F(HdfLiteUsbTest, CheckHostSdkIfGetPipe005, TestSize.Level1)
{
    struct HdfTestMsg msg = {TEST_USB_HOST_TYPE, USB_HOSTSDK_GET_PIPE_005_TEST, -1};
    EXPECT_EQ(HDF_FAILURE, HdfTestSendMsgToService(&msg));
}

/**
 * @tc.number    : CheckHostSdkIfGetPipe006
 * @tc.name      :
 * @tc.type      : PERF
 * @tc.level     : Level 1
 */
HWTEST_F(HdfLiteUsbTest, CheckHostSdkIfGetPipe006, TestSize.Level1)
{
    struct HdfTestMsg msg = {TEST_USB_HOST_TYPE, USB_HOSTSDK_GET_PIPE_006_TEST, -1};
    EXPECT_EQ(0, HdfTestSendMsgToService(&msg));
}

/**
 * @tc.number    : CheckHostSdkIfGetPipe007
 * @tc.name      :
 * @tc.type      : PERF
 * @tc.level     : Level 1
 */
HWTEST_F(HdfLiteUsbTest, CheckHostSdkIfGetPipe007, TestSize.Level1)
{
    struct HdfTestMsg msg = {TEST_USB_HOST_TYPE, USB_HOSTSDK_GET_PIPE_007_TEST, -1};
    EXPECT_EQ(HDF_FAILURE, HdfTestSendMsgToService(&msg));
}

/**
 * @tc.number    : CheckHostSdkIfGetPipe008
 * @tc.name      :
 * @tc.type      : PERF
 * @tc.level     : Level 1
 */
HWTEST_F(HdfLiteUsbTest, CheckHostSdkIfGetPipe008, TestSize.Level1)
{
    struct HdfTestMsg msg = {TEST_USB_HOST_TYPE, USB_HOSTSDK_GET_PIPE_008_TEST, -1};
    EXPECT_EQ(0, HdfTestSendMsgToService(&msg));
}

/**
 * @tc.number    : CheckHostSdkIfAllocRequest001
 * @tc.name      :
 * @tc.type      : PERF
 * @tc.level     : Level 1
 */
HWTEST_F(HdfLiteUsbTest, CheckHostSdkIfAllocRequest001, TestSize.Level1)
{
    struct HdfTestMsg msg = {TEST_USB_HOST_TYPE, USB_HOSTSDK_ALLOC_REQUEST_001_TEST, -1};
    EXPECT_EQ(0, HdfTestSendMsgToService(&msg));
}

/**
 * @tc.number    : CheckHostSdkIfAllocRequest002
 * @tc.name      :
 * @tc.type      : PERF
 * @tc.level     : Level 1
 */
HWTEST_F(HdfLiteUsbTest, CheckHostSdkIfAllocRequest002, TestSize.Level1)
{
    struct HdfTestMsg msg = {TEST_USB_HOST_TYPE, USB_HOSTSDK_ALLOC_REQUEST_002_TEST, -1};
    EXPECT_EQ(0, HdfTestSendMsgToService(&msg));
}

/**
 * @tc.number    : CheckHostSdkIfFreeRequest001
 * @tc.name      :
 * @tc.type      : PERF
 * @tc.level     : Level 1
 */
HWTEST_F(HdfLiteUsbTest, CheckHostSdkIfFreeRequest001, TestSize.Level1)
{
    struct HdfTestMsg msg = {TEST_USB_HOST_TYPE, USB_HOSTSDK_FREE_REQUEST_001_TEST, -1};
    EXPECT_EQ(0, HdfTestSendMsgToService(&msg));
}

/**
 * @tc.number    : CheckHostSdkIfAllocRequest003
 * @tc.name      :
 * @tc.type      : PERF
 * @tc.level     : Level 1
 */
HWTEST_F(HdfLiteUsbTest, CheckHostSdkIfAllocRequest003, TestSize.Level1)
{
    struct HdfTestMsg msg = {TEST_USB_HOST_TYPE, USB_HOSTSDK_ALLOC_REQUEST_003_TEST, -1};
    EXPECT_EQ(0, HdfTestSendMsgToService(&msg));
}

/**
 * @tc.number    : CheckHostSdkIfAllocRequest004
 * @tc.name      :
 * @tc.type      : PERF
 * @tc.level     : Level 1
 */
HWTEST_F(HdfLiteUsbTest, CheckHostSdkIfAllocRequest004, TestSize.Level1)
{
    struct HdfTestMsg msg = {TEST_USB_HOST_TYPE, USB_HOSTSDK_ALLOC_REQUEST_004_TEST, -1};
    EXPECT_EQ(0, HdfTestSendMsgToService(&msg));
}

/**
 * @tc.number    : CheckHostSdkIfFreeRequest002
 * @tc.name      :
 * @tc.type      : PERF
 * @tc.level     : Level 1
 */
HWTEST_F(HdfLiteUsbTest, CheckHostSdkIfFreeRequest002, TestSize.Level1)
{
    struct HdfTestMsg msg = {TEST_USB_HOST_TYPE, USB_HOSTSDK_FREE_REQUEST_002_TEST, -1};
    EXPECT_EQ(0, HdfTestSendMsgToService(&msg));
}

/**
 * @tc.number    : CheckHostSdkIfAllocRequest005
 * @tc.name      :
 * @tc.type      : PERF
 * @tc.level     : Level 1
 */
HWTEST_F(HdfLiteUsbTest, CheckHostSdkIfAllocRequest005, TestSize.Level1)
{
    struct HdfTestMsg msg = {TEST_USB_HOST_TYPE, USB_HOSTSDK_ALLOC_REQUEST_005_TEST, -1};
    EXPECT_EQ(0, HdfTestSendMsgToService(&msg));
}

/**
 * @tc.number    : CheckHostSdkIfAllocRequest006
 * @tc.name      :
 * @tc.type      : PERF
 * @tc.level     : Level 1
 */
HWTEST_F(HdfLiteUsbTest, CheckHostSdkIfAllocRequest006, TestSize.Level1)
{
    struct HdfTestMsg msg = {TEST_USB_HOST_TYPE, USB_HOSTSDK_ALLOC_REQUEST_006_TEST, -1};
    EXPECT_EQ(0, HdfTestSendMsgToService(&msg));
}
/**
 * @tc.number    : CheckRawSdkIfAllocRequest010
 * @tc.name      :
 * @tc.type      : PERFs
 * @tc.level     : Level 1
 */
HWTEST_F(HdfLiteUsbTest, CheckHostSdkIfAllocRequest010, TestSize.Level1)
{
    struct HdfTestMsg msg = {TEST_USB_HOST_TYPE, USB_HOSTSDK_ALLOC_REQUEST_010_TEST, -1};
    EXPECT_EQ(HDF_FAILURE, HdfTestSendMsgToService(&msg));
}

/**
 * @tc.number    : CheckRawSdkIfAllocRequest011
 * @tc.name      :
 * @tc.type      : PERFs
 * @tc.level     : Level 1
 */
HWTEST_F(HdfLiteUsbTest, CheckHostSdkIfAllocRequest011, TestSize.Level1)
{
    struct HdfTestMsg msg = {TEST_USB_HOST_TYPE, USB_HOSTSDK_ALLOC_REQUEST_011_TEST, -1};
    EXPECT_EQ(HDF_FAILURE, HdfTestSendMsgToService(&msg));
}

/**
 * @tc.number    : CheckRawSdkIfFillIsoRequest001
 * @tc.name      :
 * @tc.type      : PERFs
 * @tc.level     : Level 1
 */
HWTEST_F(HdfLiteUsbTest, CheckHostSdkIfFillIsoRequest001, TestSize.Level1)
{
    struct HdfTestMsg msg = {TEST_USB_HOST_TYPE, USB_HOSTSDK_FILL_ISO_REQUEST_001_TEST, -1};
    EXPECT_EQ(HDF_FAILURE, HdfTestSendMsgToService(&msg));
}

/**
 * @tc.number    : CheckRawSdkIfFillIsoRequest002
 * @tc.name      :
 * @tc.type      : PERFs
 * @tc.level     : Level 1
 */
HWTEST_F(HdfLiteUsbTest, CheckHostSdkIfFillIsoRequest002, TestSize.Level1)
{
    struct HdfTestMsg msg = {TEST_USB_HOST_TYPE, USB_HOSTSDK_FILL_ISO_REQUEST_002_TEST, -1};
    EXPECT_EQ(HDF_FAILURE, HdfTestSendMsgToService(&msg));
}

/**
 * @tc.number    : CheckRawSdkIfFillIsoRequest003
 * @tc.name      :
 * @tc.type      : PERFs
 * @tc.level     : Level 1
 */
HWTEST_F(HdfLiteUsbTest, CheckHostSdkIfFillIsoRequest003, TestSize.Level1)
{
    struct HdfTestMsg msg = {TEST_USB_HOST_TYPE, USB_HOSTSDK_FILL_ISO_REQUEST_003_TEST, -1};
    EXPECT_EQ(HDF_FAILURE, HdfTestSendMsgToService(&msg));
}

/**
 * @tc.number    : CheckRawSdkIfFillIsoRequest004
 * @tc.name      :
 * @tc.type      : PERFs
 * @tc.level     : Level 1
 */
HWTEST_F(HdfLiteUsbTest, CheckHostSdkIfFillIsoRequest004, TestSize.Level1)
{
    struct HdfTestMsg msg = {TEST_USB_HOST_TYPE, USB_HOSTSDK_FILL_ISO_REQUEST_004_TEST, -1};
    EXPECT_EQ(HDF_FAILURE, HdfTestSendMsgToService(&msg));
}
/**
 * @tc.number    : CheckRawSdkIfFillIsoRequest005
 * @tc.name      :
 * @tc.type      : PERFs
 * @tc.level     : Level 1
 */
HWTEST_F(HdfLiteUsbTest, CheckHostSdkIfFillIsoRequest005, TestSize.Level1)
{
    struct HdfTestMsg msg = {TEST_USB_HOST_TYPE, USB_HOSTSDK_FILL_ISO_REQUEST_005_TEST, -1};
    EXPECT_EQ(HDF_FAILURE, HdfTestSendMsgToService(&msg));
}

/**
 * @tc.number    : CheckRawSdkIfFillIsoRequest006
 * @tc.name      :
 * @tc.type      : PERFs
 * @tc.level     : Level 1
 */
HWTEST_F(HdfLiteUsbTest, CheckHostSdkIfFillIsoRequest006, TestSize.Level1)
{
    struct HdfTestMsg msg = {TEST_USB_HOST_TYPE, USB_HOSTSDK_FILL_ISO_REQUEST_006_TEST, -1};
    EXPECT_EQ(HDF_FAILURE, HdfTestSendMsgToService(&msg));
}

/**
 * @tc.number    : CheckRawSdkIfFreeRequest006
 * @tc.name      :
 * @tc.type      : PERFs
 * @tc.level     : Level 1
 */
HWTEST_F(HdfLiteUsbTest, CheckHostSdkIfFreeRequest006, TestSize.Level1)
{
    struct HdfTestMsg msg = {TEST_USB_HOST_TYPE, USB_HOSTSDK_FREE_REQUEST_006_TEST, -1};
    EXPECT_EQ(HDF_FAILURE, HdfTestSendMsgToService(&msg));
}

/**
 * @tc.number    : CheckHostSdkIfFreeRequest003
 * @tc.name      :
 * @tc.type      : PERF
 * @tc.level     : Level 1
 */
HWTEST_F(HdfLiteUsbTest, CheckHostSdkIfFreeRequest003, TestSize.Level1)
{
    struct HdfTestMsg msg = {TEST_USB_HOST_TYPE, USB_HOSTSDK_FREE_REQUEST_003_TEST, -1};
    EXPECT_EQ(0, HdfTestSendMsgToService(&msg));
}

/**
 * @tc.number    : CheckHostSdkIfAllocRequest007
 * @tc.name      :
 * @tc.type      : PERF
 * @tc.level     : Level 1
 */
HWTEST_F(HdfLiteUsbTest, CheckHostSdkIfAllocRequest007, TestSize.Level1)
{
    struct HdfTestMsg msg = {TEST_USB_HOST_TYPE, USB_HOSTSDK_ALLOC_REQUEST_007_TEST, -1};
    EXPECT_EQ(0, HdfTestSendMsgToService(&msg));
}

/**
 * @tc.number    : CheckHostSdkIfAllocRequest008
 * @tc.name      :
 * @tc.type      : PERF
 * @tc.level     : Level 1
 */
HWTEST_F(HdfLiteUsbTest, CheckHostSdkIfAllocRequest008, TestSize.Level1)
{
    struct HdfTestMsg msg = {TEST_USB_HOST_TYPE, USB_HOSTSDK_ALLOC_REQUEST_008_TEST, -1};
    EXPECT_EQ(0, HdfTestSendMsgToService(&msg));
}

/**
 * @tc.number    : CheckHostSdkIfFreeRequest004
 * @tc.name      :
 * @tc.type      : PERF
 * @tc.level     : Level 1
 */
HWTEST_F(HdfLiteUsbTest, CheckHostSdkIfFreeRequest004, TestSize.Level1)
{
    struct HdfTestMsg msg = {TEST_USB_HOST_TYPE, USB_HOSTSDK_FREE_REQUEST_004_TEST, -1};
    EXPECT_EQ(0, HdfTestSendMsgToService(&msg));
}

/**
 * @tc.number    : CheckHostSdkIfAllocRequest009
 * @tc.name      :
 * @tc.type      : PERF
 * @tc.level     : Level 1
 */
HWTEST_F(HdfLiteUsbTest, CheckHostSdkIfAllocRequest009, TestSize.Level1)
{
    struct HdfTestMsg msg = {TEST_USB_HOST_TYPE, USB_HOSTSDK_ALLOC_REQUEST_009_TEST, -1};
    EXPECT_EQ(0, HdfTestSendMsgToService(&msg));
}

/**
 * @tc.number    : CheckHostSdkIfFillRequest001
 * @tc.name      :
 * @tc.type      : PERF
 * @tc.level     : Level 1
 */
HWTEST_F(HdfLiteUsbTest, CheckHostSdkIfFillRequest001, TestSize.Level1)
{
    struct HdfTestMsg msg = {TEST_USB_HOST_TYPE, USB_HOSTSDK_FILL_REQUEST_001_TEST, -1};
    EXPECT_EQ(0, HdfTestSendMsgToService(&msg));
}

/**
 * @tc.number    : CheckHostSdkIfFillRequest002
 * @tc.name      :
 * @tc.type      : PERF
 * @tc.level     : Level 1
 */
HWTEST_F(HdfLiteUsbTest, CheckHostSdkIfFillRequest002, TestSize.Level1)
{
    struct HdfTestMsg msg = {TEST_USB_HOST_TYPE, USB_HOSTSDK_FILL_REQUEST_002_TEST, -1};
    EXPECT_EQ(0, HdfTestSendMsgToService(&msg));
}

/**
 * @tc.number    : CheckHostSdkIfFillRequest003
 * @tc.name      :
 * @tc.type      : PERF
 * @tc.level     : Level 1
 */
HWTEST_F(HdfLiteUsbTest, CheckHostSdkIfFillRequest003, TestSize.Level1)
{
    struct HdfTestMsg msg = {TEST_USB_HOST_TYPE, USB_HOSTSDK_FILL_REQUEST_003_TEST, -1};
    EXPECT_EQ(0, HdfTestSendMsgToService(&msg));
}

/**
 * @tc.number    : CheckHostSdkIfFillRequest004
 * @tc.name      :
 * @tc.type      : PERF
 * @tc.level     : Level 1
 */
HWTEST_F(HdfLiteUsbTest, CheckHostSdkIfFillRequest004, TestSize.Level1)
{
    struct HdfTestMsg msg = {TEST_USB_HOST_TYPE, USB_HOSTSDK_FILL_REQUEST_004_TEST, -1};
    EXPECT_EQ(0, HdfTestSendMsgToService(&msg));
}

/**
 * @tc.number    : CheckHostSdkIfFillRequest005
 * @tc.name      :
 * @tc.type      : PERF
 * @tc.level     : Level 1
 */
HWTEST_F(HdfLiteUsbTest, CheckHostSdkIfFillRequest005, TestSize.Level1)
{
    struct HdfTestMsg msg = {TEST_USB_HOST_TYPE, USB_HOSTSDK_FILL_REQUEST_005_TEST, -1};
    EXPECT_EQ(0, HdfTestSendMsgToService(&msg));
}

/**
 * @tc.number    : CheckHostSdkIfFillRequest006
 * @tc.name      :
 * @tc.type      : PERF
 * @tc.level     : Level 1
 */
HWTEST_F(HdfLiteUsbTest, CheckHostSdkIfFillRequest006, TestSize.Level1)
{
    struct HdfTestMsg msg = {TEST_USB_HOST_TYPE, USB_HOSTSDK_FILL_REQUEST_006_TEST, -1};
    EXPECT_EQ(0, HdfTestSendMsgToService(&msg));
}

/**
 * @tc.number    : CheckHostSdkIfFillRequest007
 * @tc.name      :
 * @tc.type      : PERF
 * @tc.level     : Level 1
 */
HWTEST_F(HdfLiteUsbTest, CheckHostSdkIfFillRequest007, TestSize.Level1)
{
    struct HdfTestMsg msg = {TEST_USB_HOST_TYPE, USB_HOSTSDK_FILL_REQUEST_007_TEST, -1};
    EXPECT_EQ(0, HdfTestSendMsgToService(&msg));
}

/**
 * @tc.number    : CheckHostSdkIfFillRequest008
 * @tc.name      :
 * @tc.type      : PERF
 * @tc.level     : Level 1
 */
HWTEST_F(HdfLiteUsbTest, CheckHostSdkIfFillRequest008, TestSize.Level1)
{
    struct HdfTestMsg msg = {TEST_USB_HOST_TYPE, USB_HOSTSDK_FILL_REQUEST_008_TEST, -1};
    EXPECT_EQ(0, HdfTestSendMsgToService(&msg));
}


/**
 * @tc.number    : CheckHostSdkIfSubmitRequestAsync001
 * @tc.name      :
 * @tc.type      : PERF
 * @tc.level     : Level 1
 */
HWTEST_F(HdfLiteUsbTest, CheckHostSdkIfSubmitRequestAsync001, TestSize.Level1)
{
    struct HdfTestMsg msg = {TEST_USB_HOST_TYPE, USB_HOSTSDK_SUBMIT_REQUEST_ASYNC_001_TEST, -1};
    EXPECT_EQ(0, HdfTestSendMsgToService(&msg));
}

/**
 * @tc.number    : CheckHostSdkIfCancelRequest001
 * @tc.name      :
 * @tc.type      : PERF
 * @tc.level     : Level 1
 */
HWTEST_F(HdfLiteUsbTest, CheckHostSdkIfCancelRequest001, TestSize.Level1)
{
    struct HdfTestMsg msg = {TEST_USB_HOST_TYPE, USB_HOSTSDK_SUBMIT_CANCEL_REQUEST_001_TEST, -1};
    EXPECT_EQ(0, HdfTestSendMsgToService(&msg));
}

/**
 * @tc.number    : CheckHostSdkIfSubmitRequestAsync002
 * @tc.name      :
 * @tc.type      : PERF
 * @tc.level     : Level 1
 */
HWTEST_F(HdfLiteUsbTest, CheckHostSdkIfSubmitRequestAsync002, TestSize.Level1)
{
    struct HdfTestMsg msg = {TEST_USB_HOST_TYPE, USB_HOSTSDK_SUBMIT_REQUEST_ASYNC_002_TEST, -1};
    EXPECT_EQ(0, HdfTestSendMsgToService(&msg));
}

/**
 * @tc.number    : CheckHostSdkIfCancelRequest002
 * @tc.name      :
 * @tc.type      : PERF
 * @tc.level     : Level 1
 */
HWTEST_F(HdfLiteUsbTest, CheckHostSdkIfCancelRequest002, TestSize.Level1)
{
    struct HdfTestMsg msg = {TEST_USB_HOST_TYPE, USB_HOSTSDK_SUBMIT_CANCEL_REQUEST_002_TEST, -1};
    EXPECT_EQ(0, HdfTestSendMsgToService(&msg));
}

/**
 * @tc.number    : CheckHostSdkIfSubmitRequestAsync003
 * @tc.name      :
 * @tc.type      : PERF
 * @tc.level     : Level 1
 */
HWTEST_F(HdfLiteUsbTest, CheckHostSdkIfSubmitRequestAsync003, TestSize.Level1)
{
    struct HdfTestMsg msg = {TEST_USB_HOST_TYPE, USB_HOSTSDK_SUBMIT_REQUEST_ASYNC_003_TEST, -1};
    EXPECT_EQ(0, HdfTestSendMsgToService(&msg));
}

/**
 * @tc.number    : CheckHostSdkIfCancelRequest003
 * @tc.name      :
 * @tc.type      : PERF
 * @tc.level     : Level 1
 */
HWTEST_F(HdfLiteUsbTest, CheckHostSdkIfCancelRequest003, TestSize.Level1)
{
    struct HdfTestMsg msg = {TEST_USB_HOST_TYPE, USB_HOSTSDK_SUBMIT_CANCEL_REQUEST_003_TEST, -1};
    EXPECT_EQ(0, HdfTestSendMsgToService(&msg));
}

/**
 * @tc.number    : CheckHostSdkIfSubmitRequestAsync004
 * @tc.name      :
 * @tc.type      : PERF
 * @tc.level     : Level 1
 */
HWTEST_F(HdfLiteUsbTest, CheckHostSdkIfSubmitRequestAsync004, TestSize.Level1)
{
    struct HdfTestMsg msg = {TEST_USB_HOST_TYPE, USB_HOSTSDK_SUBMIT_REQUEST_ASYNC_004_TEST, -1};
    EXPECT_EQ(0, HdfTestSendMsgToService(&msg));
}

/**
 * @tc.number    : CheckHostSdkIfCancelRequest004
 * @tc.name      :
 * @tc.type      : PERF
 * @tc.level     : Level 1
 */
HWTEST_F(HdfLiteUsbTest, CheckHostSdkIfCancelRequest004, TestSize.Level1)
{
    struct HdfTestMsg msg = {TEST_USB_HOST_TYPE, USB_HOSTSDK_SUBMIT_CANCEL_REQUEST_004_TEST, -1};
    EXPECT_EQ(0, HdfTestSendMsgToService(&msg));
}

/**
 * @tc.number    : CheckHostSdkIfClearInterfaceHalt002
 * @tc.name      :
 * @tc.type      : PERF
 * @tc.level     : Level 1
 */
HWTEST_F(HdfLiteUsbTest, CheckHostSdkIfClearInterfaceHalt002, TestSize.Level1)
{
    struct HdfTestMsg msg = {TEST_USB_HOST_TYPE, USB_HOSTSDK_CLEAR_INTERFACE_HALT_002_TEST, -1};
    EXPECT_EQ(0, HdfTestSendMsgToService(&msg));
}

/**
 * @tc.number    : CheckHostSdkIfClearInterfaceHalt003
 * @tc.name      :
 * @tc.type      : PERF
 * @tc.level     : Level 1
 */
HWTEST_F(HdfLiteUsbTest, CheckHostSdkIfClearInterfaceHalt003, TestSize.Level1)
{
    struct HdfTestMsg msg = {TEST_USB_HOST_TYPE, USB_HOSTSDK_CLEAR_INTERFACE_HALT_003_TEST, -1};
    EXPECT_EQ(0, HdfTestSendMsgToService(&msg));
}

/**
 * @tc.number    : CheckHostSdkIfClearInterfaceHalt004
 * @tc.name      :
 * @tc.type      : PERF
 * @tc.level     : Level 1
 */
HWTEST_F(HdfLiteUsbTest, CheckHostSdkIfClearInterfaceHalt004, TestSize.Level1)
{
    struct HdfTestMsg msg = {TEST_USB_HOST_TYPE, USB_HOSTSDK_CLEAR_INTERFACE_HALT_004_TEST, -1};
    EXPECT_EQ(0, HdfTestSendMsgToService(&msg));
}

/**
 * @tc.number    : CheckHostSdkIfRemoveInterface001
 * @tc.name      :
 * @tc.type      : PERF
 * @tc.level     : Level 1
 */
HWTEST_F(HdfLiteUsbTest, CheckHostSdkIfRemoveInterface001, TestSize.Level1)
{
    struct HdfTestMsg msg = {TEST_USB_HOST_TYPE, USB_HOSTSDK_REMOVE_INTERFACE_001_TEST, -1};
    EXPECT_EQ(0, HdfTestSendMsgToService(&msg));
}

/**
 * @tc.number    : CheckHostSdkIfAddInterface001
 * @tc.name      :
 * @tc.type      : PERF
 * @tc.level     : Level 1
 */
HWTEST_F(HdfLiteUsbTest, CheckHostSdkIfAddInterface001, TestSize.Level1)
{
    struct HdfTestMsg msg = {TEST_USB_HOST_TYPE, USB_HOSTSDK_ADD_INTERFACE_001_TEST, -1};
    EXPECT_EQ(0, HdfTestSendMsgToService(&msg));
}

/**
 * @tc.number    : CheckHostSdkIfRemoveInterface002
 * @tc.name      :
 * @tc.type      : PERF
 * @tc.level     : Level 1
 */
HWTEST_F(HdfLiteUsbTest, CheckHostSdkIfRemoveInterface002, TestSize.Level1)
{
    struct HdfTestMsg msg = {TEST_USB_HOST_TYPE, USB_HOSTSDK_REMOVE_INTERFACE_002_TEST, -1};
    EXPECT_EQ(0, HdfTestSendMsgToService(&msg));
}

/**
 * @tc.number    : CheckHostSdkIfAddInterface002
 * @tc.name      :
 * @tc.type      : PERF
 * @tc.level     : Level 1
 */
HWTEST_F(HdfLiteUsbTest, CheckHostSdkIfAddInterface002, TestSize.Level1)
{
    struct HdfTestMsg msg = {TEST_USB_HOST_TYPE, USB_HOSTSDK_ADD_INTERFACE_002_TEST, -1};
    EXPECT_EQ(0, HdfTestSendMsgToService(&msg));
}

/**
 * @tc.number    : CheckHostSdkIfRemoveInterface003
 * @tc.name      :
 * @tc.type      : PERF
 * @tc.level     : Level 1
 */
HWTEST_F(HdfLiteUsbTest, CheckHostSdkIfRemoveInterface003, TestSize.Level1)
{
    struct HdfTestMsg msg = {TEST_USB_HOST_TYPE, USB_HOSTSDK_REMOVE_INTERFACE_003_TEST, -1};
    EXPECT_EQ(0, HdfTestSendMsgToService(&msg));
}

/**
 * @tc.number    : CheckHostSdkIfAddInterface003
 * @tc.name      :
 * @tc.type      : PERF
 * @tc.level     : Level 1
 */
HWTEST_F(HdfLiteUsbTest, CheckHostSdkIfAddInterface003, TestSize.Level1)
{
    struct HdfTestMsg msg = {TEST_USB_HOST_TYPE, USB_HOSTSDK_ADD_INTERFACE_003_TEST, -1};
    EXPECT_EQ(0, HdfTestSendMsgToService(&msg));
}

/**
 * @tc.number    : CheckHostSdkIfCloseInterface006
 * @tc.name      :
 * @tc.type      : PERF
 * @tc.level     : Level 1
 */
HWTEST_F(HdfLiteUsbTest, CheckHostSdkIfCloseInterface006, TestSize.Level1)
{
    struct HdfTestMsg msg = {TEST_USB_HOST_TYPE, USB_HOSTSDK_CLOSE_INTERFACE_006_TEST, -1};
    EXPECT_EQ(0, HdfTestSendMsgToService(&msg));
}
