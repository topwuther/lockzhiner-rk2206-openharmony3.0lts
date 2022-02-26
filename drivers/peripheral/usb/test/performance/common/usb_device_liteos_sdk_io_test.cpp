/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
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
#include "hdf_usb_device_test.h"

using namespace testing::ext;

namespace ModuleTest {
class UsbFnDeviceSdkIOTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();
};

void UsbFnDeviceSdkIOTest::SetUpTestCase()
{
    HdfTestOpenService();
}

void UsbFnDeviceSdkIOTest::TearDownTestCase()
{
    HdfTestCloseService();
}

void UsbFnDeviceSdkIOTest::SetUp() {}

void UsbFnDeviceSdkIOTest::TearDown() {}

HWTEST_F(UsbFnDeviceSdkIOTest, UsbDeviceCreate001, TestSize.Level1)
{
    struct HdfTestMsg msg = {TEST_USB_DEVICE_TYPE, USB_DEVICE_CREATE_DEVICE, -1};
    EXPECT_EQ(0, HdfTestSendMsgToService(&msg));
}

HWTEST_F(UsbFnDeviceSdkIOTest, UsbDeviceRequestAsync001, TestSize.Level1)
{
    struct HdfTestMsg msg = {TEST_USB_DEVICE_TYPE, USB_DEVICE_REQUEST_ASYNC, -1};
    EXPECT_EQ(0, HdfTestSendMsgToService(&msg));
}

HWTEST_F(UsbFnDeviceSdkIOTest, UsbDeviceRequestAsync002, TestSize.Level1)
{
    struct HdfTestMsg msg = {TEST_USB_DEVICE_TYPE, USB_DEVICE_REQUEST_ASYNC002, -1};
    EXPECT_EQ(0, HdfTestSendMsgToService(&msg));
}

HWTEST_F(UsbFnDeviceSdkIOTest, UsbDeviceRequestAsync003, TestSize.Level1)
{
    struct HdfTestMsg msg = {TEST_USB_DEVICE_TYPE, USB_DEVICE_REQUEST_ASYNC003, -1};
    EXPECT_EQ(0, HdfTestSendMsgToService(&msg));
}

HWTEST_F(UsbFnDeviceSdkIOTest, UsbDeviceRequestAsync004, TestSize.Level1)
{
    struct HdfTestMsg msg = {TEST_USB_DEVICE_TYPE, USB_DEVICE_REQUEST_ASYNC004, -1};
    EXPECT_EQ(0, HdfTestSendMsgToService(&msg));
}

HWTEST_F(UsbFnDeviceSdkIOTest, UsbDeviceRequestAsync005, TestSize.Level1)
{
    struct HdfTestMsg msg = {TEST_USB_DEVICE_TYPE, USB_DEVICE_REQUEST_ASYNC005, -1};
    EXPECT_EQ(0, HdfTestSendMsgToService(&msg));
}

HWTEST_F(UsbFnDeviceSdkIOTest, UsbDeviceRequestSync001, TestSize.Level1)
{
    struct HdfTestMsg msg = {TEST_USB_DEVICE_TYPE, USB_DEVICE_REQUEST_SYNC, -1};
    EXPECT_EQ(0, HdfTestSendMsgToService(&msg));
}

HWTEST_F(UsbFnDeviceSdkIOTest, UsbDeviceRequestSync002, TestSize.Level1)
{
    struct HdfTestMsg msg = {TEST_USB_DEVICE_TYPE, USB_DEVICE_REQUEST_SYNC002, -1};
    EXPECT_EQ(0, HdfTestSendMsgToService(&msg));
}

HWTEST_F(UsbFnDeviceSdkIOTest, UsbDeviceRequestSync003, TestSize.Level1)
{
    struct HdfTestMsg msg = {TEST_USB_DEVICE_TYPE, USB_DEVICE_REQUEST_SYNC003, -1};
    EXPECT_EQ(0, HdfTestSendMsgToService(&msg));
}

HWTEST_F(UsbFnDeviceSdkIOTest, UsbDeviceRequestSync004, TestSize.Level1)
{
    struct HdfTestMsg msg = {TEST_USB_DEVICE_TYPE, USB_DEVICE_REQUEST_SYNC004, -1};
    EXPECT_EQ(0, HdfTestSendMsgToService(&msg));
}

HWTEST_F(UsbFnDeviceSdkIOTest, UsbDeviceRequestSync005, TestSize.Level1)
{
    struct HdfTestMsg msg = {TEST_USB_DEVICE_TYPE, USB_DEVICE_REQUEST_SYNC005, -1};
    EXPECT_EQ(0, HdfTestSendMsgToService(&msg));
}

HWTEST_F(UsbFnDeviceSdkIOTest, UsbDeviceRequestSync006, TestSize.Level1)
{
    struct HdfTestMsg msg = {TEST_USB_DEVICE_TYPE, USB_DEVICE_REQUEST_SYNC006, -1};
    EXPECT_EQ(0, HdfTestSendMsgToService(&msg));
}

HWTEST_F(UsbFnDeviceSdkIOTest, UsbDeviceRequestSync007, TestSize.Level1)
{
    struct HdfTestMsg msg = {TEST_USB_DEVICE_TYPE, USB_DEVICE_REQUEST_SYNC007, -1};
    EXPECT_EQ(0, HdfTestSendMsgToService(&msg));
}

HWTEST_F(UsbFnDeviceSdkIOTest, UsbDeviceCancelRequest001, TestSize.Level1)
{
    struct HdfTestMsg msg = {TEST_USB_DEVICE_TYPE, USB_DEVICE_CANCEL_REQUEST, -1};
    EXPECT_EQ(0, HdfTestSendMsgToService(&msg));
}

HWTEST_F(UsbFnDeviceSdkIOTest, UsbDeviceCancelRequest002, TestSize.Level1)
{
    struct HdfTestMsg msg = {TEST_USB_DEVICE_TYPE, USB_DEVICE_CANCEL_REQUEST002, -1};
    EXPECT_EQ(0, HdfTestSendMsgToService(&msg));
}

HWTEST_F(UsbFnDeviceSdkIOTest, UsbDeviceCancelRequest003, TestSize.Level1)
{
    struct HdfTestMsg msg = {TEST_USB_DEVICE_TYPE, USB_DEVICE_CANCEL_REQUEST003, -1};
    EXPECT_EQ(0, HdfTestSendMsgToService(&msg));
}

HWTEST_F(UsbFnDeviceSdkIOTest, UsbDeviceCancelRequest004, TestSize.Level1)
{
    struct HdfTestMsg msg = {TEST_USB_DEVICE_TYPE, USB_DEVICE_CANCEL_REQUEST004, -1};
    EXPECT_EQ(0, HdfTestSendMsgToService(&msg));
}

HWTEST_F(UsbFnDeviceSdkIOTest, UsbDeviceCancelRequest005, TestSize.Level1)
{
    struct HdfTestMsg msg = {TEST_USB_DEVICE_TYPE, USB_DEVICE_CANCEL_REQUEST005, -1};
    EXPECT_EQ(0, HdfTestSendMsgToService(&msg));
}

HWTEST_F(UsbFnDeviceSdkIOTest, UsbDeviceCancelRequest006, TestSize.Level1)
{
    struct HdfTestMsg msg = {TEST_USB_DEVICE_TYPE, USB_DEVICE_CANCEL_REQUEST006, -1};
    EXPECT_EQ(0, HdfTestSendMsgToService(&msg));
}

HWTEST_F(UsbFnDeviceSdkIOTest, UsbDeviceDelete001, TestSize.Level1)
{
    struct HdfTestMsg msg = {TEST_USB_DEVICE_TYPE, USB_DEVICE_DELETE_DEVICE002, -1};
    EXPECT_EQ(0, HdfTestSendMsgToService(&msg));
}
};
