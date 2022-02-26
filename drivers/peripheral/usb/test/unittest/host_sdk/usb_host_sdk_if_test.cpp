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

#include <cstdio>
#include <cstring>
#include <unistd.h>
#include <gtest/gtest.h>
extern "C" {
#include "usb_host_sdk_if_test.h"
#include "hdf_base.h"
#include "hdf_log.h"
#include "osal_mem.h"
#include "osal_time.h"
#include "securec.h"
#include "usb_interface.h"
}

#define USB_PIPE_DIR_OFFSET          7

using namespace std;
using namespace testing::ext;

namespace {
class UsbHostSdkIfTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();
};

static struct UsbSession *session = NULL;
static struct AcmDevice deviceService;
static struct AcmDevice *acm = &deviceService;
static struct UsbInterface *ecm_dataIface = NULL;
static struct UsbInterface *ecm_intIface = NULL;
static UsbInterfaceHandle *ecm_data_devHandle = NULL;
static UsbInterfaceHandle *ecm_int_devHandle = NULL;

void UsbHostSdkIfTest::SetUpTestCase()
{
}

void UsbHostSdkIfTest::TearDownTestCase()
{
}

void UsbHostSdkIfTest::SetUp()
{
}

void UsbHostSdkIfTest::TearDown()
{
}

static void AcmReadBulk(struct UsbRequest *req)
{
    uint32_t size;
    int status = req->compInfo.status;
    size = req->compInfo.actualLength;
    printf("Bulk status:%d,actualLength:%d\n", status, size);
    return;
}

static void AcmWriteBulk(struct UsbRequest *req)
{
    int status;

    if (req == NULL) {
        printf("%s:%d req is NULL!", __func__, __LINE__);
        return;
    }

    status = req->compInfo.status;
    printf("Bulk Write status:%d\n", status);
    struct AcmWb *wb  = (struct AcmWb *)req->compInfo.userData;
    switch (status) {
        case 0:
            wb->use = 0;
            break;
        case -ECONNRESET:
        case -ENOENT:
        case -ESHUTDOWN:
            return;
        default:
            return;
    }

    return;
}

static int AcmWriteBufAlloc(struct AcmDevice *acm)
{
    int i;
    struct AcmWb *wb;
    for (wb = &acm->wb[0],i = 0; i < ACM_NW; i++,wb++) {
        wb->buf = (uint8_t *)OsalMemCalloc(acm->writeSize);
        if (!wb->buf) {
            while (i != 0) {
                --i;
                --wb;
                OsalMemFree(wb->buf);
                wb->buf = NULL;
            }
            return -HDF_ERR_MALLOC_FAIL;
        }
    }
    return 0;
}

static void AcmProcessNotification(struct AcmDevice *acm, unsigned char *buf)
{
    struct UsbCdcNotification *dr = (struct UsbCdcNotification *)buf;
    switch (dr->bNotificationType) {
        case USB_DDK_CDC_NOTIFY_NETWORK_CONNECTION:
            printf("%s - network connection: %d\n", __func__, dr->wValue);
            break;
        case USB_DDK_CDC_NOTIFY_SERIAL_STATE:
            printf("the serial State change\n");
            break;
        default:
            printf("%s-%d received: index %d len %d\n",
                __func__,
                dr->bNotificationType, dr->wIndex, dr->wLength);
    }
    return;
}

static void AcmCtrlIrq(struct UsbRequest *req)
{
    if (req == NULL) {
        printf("%s:%d req is NULL!", __func__, __LINE__);
        return;
    }
    int retval, ret;
    struct AcmDevice *acm = (struct AcmDevice *)req->compInfo.userData;
    unsigned int expectedSize, copySize, allocSize;
    int status = req->compInfo.status;
    struct UsbCdcNotification *dr = (struct UsbCdcNotification *)req->compInfo.buffer;
    unsigned int currentSize = req->compInfo.actualLength;
    printf("Irqstatus:%d,actualLength:%u\n", status, currentSize);
    switch (status) {
        case 0:
            break;
        default:
            return;
    }
    if (acm->nbIndex) {
        dr = (struct UsbCdcNotification *)acm->notificationBuffer;
    }
    if (dr == NULL) {
        printf("%s:%d dr is NULL!", __func__, __LINE__);
        return;
    }
    expectedSize = sizeof(struct UsbCdcNotification) + Le16ToCpu(dr->wLength);
    if (currentSize < expectedSize) {
        if (acm->nbSize < expectedSize) {
            if (acm->nbSize) {
                OsalMemFree(acm->notificationBuffer);
                acm->nbSize = 0;
            }
            allocSize = expectedSize;
            acm->notificationBuffer = (uint8_t *)OsalMemCalloc(allocSize);
            if (!acm->notificationBuffer) {
                return;
            }
            acm->nbSize = allocSize;
        }
        copySize = MIN(currentSize, expectedSize - acm->nbIndex);
        ret = memcpy_s(&acm->notificationBuffer[acm->nbIndex], acm->nbSize - acm->nbIndex,
               req->compInfo.buffer, copySize);
        if (ret) {
            printf("memcpy_s fail\n");
        }
        acm->nbIndex += copySize;
        currentSize = acm->nbIndex;
    }
    if (currentSize >= expectedSize) {
        AcmProcessNotification(acm, (unsigned char *)dr);
        acm->nbIndex = 0;
    }

    retval = UsbSubmitRequestAsync(req);

    printf("%s:%d exit", __func__, __LINE__);
}

static struct UsbControlRequest UsbControlMsg(struct TestControlMsgData msgData)
{
    struct UsbControlRequest dr;
    dr.target = (UsbRequestTargetType)(msgData.requestType & TARGET_MASK);
    dr.reqType = (UsbControlRequestType)((msgData.requestType >> 5) & REQUEST_TYPE_MASK);
    dr.directon = (UsbRequestDirection)((msgData.requestType >> 7) & DIRECTION_MASK);
    dr.request = msgData.request;
    dr.value = CpuToLe16(msgData.value);
    dr.index = CpuToLe16(msgData.index);
    dr.buffer = msgData.data;
    dr.length = CpuToLe16(msgData.size);
    return dr;
}

/**
 * @tc.number    : CheckHostSdkIfInit001
 * @tc.name      :
 * @tc.type      : PERF
 * @tc.level     : Level 1
 */
HWTEST_F(UsbHostSdkIfTest, CheckHostSdkIfInit001, TestSize.Level1)
{
    int ret;

    ret = UsbInitHostSdk(&session);
    EXPECT_EQ(HDF_SUCCESS, ret);
    acm->session = session;
}

/**
 * @tc.number    : CheckHostSdkIfExit001
 * @tc.name      :
 * @tc.type      : PERF
 * @tc.level     : Level 1
 */
HWTEST_F(UsbHostSdkIfTest, CheckHostSdkIfExit001, TestSize.Level1)
{
    int ret;

    ret = UsbExitHostSdk(acm->session);
    EXPECT_EQ(HDF_SUCCESS, ret);
}

/**
 * @tc.number    : CheckHostSdkIfInit002
 * @tc.name      :
 * @tc.type      : PERF
 * @tc.level     : Level 1
 */
HWTEST_F(UsbHostSdkIfTest, CheckHostSdkIfInit002, TestSize.Level1)
{
    int ret;

    ret = UsbInitHostSdk(NULL);
    EXPECT_EQ(HDF_SUCCESS, ret);
}

/**
 * @tc.number    : CheckHostSdkIfExit002
 * @tc.name      :
 * @tc.type      : PERF
 * @tc.level     : Level 1
 */
HWTEST_F(UsbHostSdkIfTest, CheckHostSdkIfExit002, TestSize.Level1)
{
    int ret;

    ret = UsbExitHostSdk(NULL);
    EXPECT_EQ(HDF_SUCCESS, ret);
}

/**
 * @tc.number    : CheckHostSdkIfInit003
 * @tc.name      :
 * @tc.type      : PERF
 * @tc.level     : Level 1
 */
HWTEST_F(UsbHostSdkIfTest, CheckHostSdkIfInit003, TestSize.Level1)
{
    int ret;
    int i;

    for (i = 0; i < 100; i++)
    {
        ret = UsbInitHostSdk(&session);
        EXPECT_EQ(HDF_SUCCESS, ret);
        acm->session = session;
        ret = UsbExitHostSdk(acm->session);
        EXPECT_EQ(HDF_SUCCESS, ret);
    }
}

/**
 * @tc.number    : CheckHostSdkIfInit004
 * @tc.name      :
 * @tc.type      : PERF
 * @tc.level     : Level 1
 */
HWTEST_F(UsbHostSdkIfTest, CheckHostSdkIfInit004, TestSize.Level1)
{
    int ret;
    int i;

    for (i = 0; i < 100; i++)
    {
        ret = UsbInitHostSdk(NULL);
        EXPECT_EQ(HDF_SUCCESS, ret);
        ret = UsbExitHostSdk(NULL);
        EXPECT_EQ(HDF_SUCCESS, ret);
    }
}

/**
 * @tc.number    : CheckHostSdkIfInit005
 * @tc.name      :
 * @tc.type      : PERF
 * @tc.level     : Level 1
 */
HWTEST_F(UsbHostSdkIfTest, CheckHostSdkIfInit005, TestSize.Level1)
{
    int ret;
    int i;

    for (i = 0; i < 100; i++)
    {
        ret = UsbInitHostSdk(NULL);
        EXPECT_EQ(HDF_SUCCESS, ret);
    }
}

/**
 * @tc.number    : CheckHostSdkIfExit002
 * @tc.name      :
 * @tc.type      : PERF
 * @tc.level     : Level 1
 */
HWTEST_F(UsbHostSdkIfTest, CheckHostSdkIfExit003, TestSize.Level1)
{
    int ret;
    int i;
    for (i = 0; i < 100; i++)
    {
        ret = UsbExitHostSdk(NULL);
        EXPECT_EQ(HDF_SUCCESS, ret);
    }
}


/**
 * @tc.number    : CheckHostSdkIfInit006
 * @tc.name      :
 * @tc.type      : PERF
 * @tc.level     : Level 1
 */
HWTEST_F(UsbHostSdkIfTest, CheckHostSdkIfInit006, TestSize.Level1)
{
    int ret;

    ret = UsbInitHostSdk(&session);
    EXPECT_EQ(HDF_SUCCESS, ret);
    acm->session = session;
}

/**
 * @tc.number    : CheckHostSdkIfClaimInterface001
 * @tc.name      :
 * @tc.type      : PERF
 * @tc.level     : Level 1
 */
HWTEST_F(UsbHostSdkIfTest, CheckHostSdkIfClaimInterface001, TestSize.Level1)
{
    acm->busNum = 1U;
    acm->devAddr = 2U;
    acm->interfaceIndex = 1U;

    acm->dataIface =  (struct UsbInterface *)UsbClaimInterface(acm->session, acm->busNum,
        acm->devAddr, acm->interfaceIndex);
    EXPECT_NE(nullptr,  acm->dataIface);
}

/**
 * @tc.number    : CheckHostSdkIfReleaseInterface001
 * @tc.name      :
 * @tc.type      : PERF
 * @tc.level     : Level 1
 */
HWTEST_F(UsbHostSdkIfTest, CheckHostSdkIfReleaseInterface001, TestSize.Level1)
{
    int ret;

    ret = UsbReleaseInterface(acm->dataIface);
    EXPECT_EQ(HDF_SUCCESS, ret);
}

/**
 * @tc.number    : CheckHostSdkIfClaimInterface002
 * @tc.name      :
 * @tc.type      : PERF
 * @tc.level     : Level 1
 */
HWTEST_F(UsbHostSdkIfTest, CheckHostSdkIfClaimInterface002, TestSize.Level1)
{
    acm->busNum = 1U;
    acm->devAddr = 2U;
    acm->interfaceIndex = 0U;

    acm->intIface =  (struct UsbInterface *)UsbClaimInterface(acm->session, acm->busNum,
        acm->devAddr, acm->interfaceIndex);
    EXPECT_NE(nullptr,  acm->intIface);
}

/**
 * @tc.number    : CheckHostSdkIfReleaseInterface002
 * @tc.name      :
 * @tc.type      : PERF
 * @tc.level     : Level 1
 */
HWTEST_F(UsbHostSdkIfTest, CheckHostSdkIfReleaseInterface002, TestSize.Level1)
{
    int ret;

    ret = UsbReleaseInterface(acm->intIface);
    EXPECT_EQ(HDF_SUCCESS, ret);
}

/**
 * @tc.number    : CheckHostSdkIfClaimInterface003
 * @tc.name      :
 * @tc.type      : PERF
 * @tc.level     : Level 1
 */
HWTEST_F(UsbHostSdkIfTest, CheckHostSdkIfClaimInterface003, TestSize.Level1)
{
    acm->busNum = 1U;
    acm->devAddr = 2U;
    acm->interfaceIndex = 255U;

    acm->ctrIface =  (struct UsbInterface *)UsbClaimInterface(acm->session, acm->busNum,
        acm->devAddr, acm->interfaceIndex);
    EXPECT_NE(nullptr,  acm->ctrIface);
}

/**
 * @tc.number    : CheckHostSdkIfReleaseInterface003
 * @tc.name      :
 * @tc.type      : PERF
 * @tc.level     : Level 1
 */
HWTEST_F(UsbHostSdkIfTest, CheckHostSdkIfReleaseInterface003, TestSize.Level1)
{
    int ret;

    ret = UsbReleaseInterface(acm->ctrIface);
    EXPECT_EQ(HDF_SUCCESS, ret);
}

/**
 * @tc.number    : CheckHostSdkIfReleaseInterface004
 * @tc.name      :
 * @tc.type      : PERF
 * @tc.level     : Level 1
 */
HWTEST_F(UsbHostSdkIfTest, CheckHostSdkIfReleaseInterface004, TestSize.Level1)
{
    int ret;

    ret = UsbReleaseInterface(NULL);
    EXPECT_NE(HDF_SUCCESS, ret);
}


/**
 * @tc.number    : CheckHostSdkIfClaimInterface004
 * @tc.name      :
 * @tc.type      : PERF
 * @tc.level     : Level 1
 */
HWTEST_F(UsbHostSdkIfTest, CheckHostSdkIfClaimInterface004, TestSize.Level1)
{
    acm->busNum = 1U;
    acm->devAddr = 2U;
    acm->interfaceIndex = 2U;

    acm->dataIface =  (struct UsbInterface *)UsbClaimInterface(acm->session, acm->busNum,
        acm->devAddr, acm->interfaceIndex);
    EXPECT_NE(nullptr,  acm->dataIface);
}

/**
 * @tc.number    : CheckHostSdkIfClaimInterface005
 * @tc.name      :
 * @tc.type      : PERF
 * @tc.level     : Level 1
 */
HWTEST_F(UsbHostSdkIfTest, CheckHostSdkIfClaimInterface005, TestSize.Level1)
{
    acm->busNum = 1U;
    acm->devAddr = 2U;
    acm->interfaceIndex = 3U;

    acm->dataIface =  (struct UsbInterface *)UsbClaimInterface(acm->session, acm->busNum,
        acm->devAddr, acm->interfaceIndex);
    EXPECT_NE(nullptr,  acm->dataIface);
}

/**
 * @tc.number    : CheckHostSdkIfClaimInterface006
 * @tc.name      :
 * @tc.type      : PERF
 * @tc.level     : Level 1
 */
HWTEST_F(UsbHostSdkIfTest, CheckHostSdkIfClaimInterface006, TestSize.Level1)
{
    acm = &deviceService;
    acm->busNum = 1U;
    acm->devAddr = 2U;

    acm->interfaceIndex = 3U;
    acm->dataIface =  (struct UsbInterface *)UsbClaimInterface(acm->session, acm->busNum,
        acm->devAddr, acm->interfaceIndex);
    EXPECT_NE(nullptr,  acm->dataIface);

    acm->interfaceIndex = 2U;
    acm->intIface =  (struct UsbInterface *)UsbClaimInterface(acm->session, acm->busNum,
        acm->devAddr, acm->interfaceIndex);
    EXPECT_NE(nullptr,  acm->intIface);

    acm->interfaceIndex = 0U;
    ecm_intIface =  (struct UsbInterface *)UsbClaimInterface(acm->session, acm->busNum,
        acm->devAddr, acm->interfaceIndex);
    EXPECT_NE(nullptr,  ecm_intIface);

    acm->interfaceIndex = 1U;
    ecm_dataIface =  (struct UsbInterface *)UsbClaimInterface(acm->session, acm->busNum,
        acm->devAddr, acm->interfaceIndex);
    EXPECT_NE(nullptr,  ecm_dataIface);

    acm->interfaceIndex = 255U;
    acm->ctrIface =  (struct UsbInterface *)UsbClaimInterface(acm->session, acm->busNum,
        acm->devAddr, acm->interfaceIndex);
    EXPECT_NE(nullptr,  acm->ctrIface);
}

/**
 * @tc.number    : CheckHostSdkIfOpenInterface001
 * @tc.name      :
 * @tc.type      : PERF
 * @tc.level     : Level 1
 */
HWTEST_F(UsbHostSdkIfTest, CheckHostSdkIfOpenInterface001, TestSize.Level1)
{
    acm->data_devHandle = UsbOpenInterface(acm->dataIface);
    EXPECT_NE(nullptr,  acm->data_devHandle);
}

/**
 * @tc.number    : CheckHostSdkIfCloseInterface001
 * @tc.name      :
 * @tc.type      : PERF
 * @tc.level     : Level 1
 */
HWTEST_F(UsbHostSdkIfTest, CheckHostSdkIfCloseInterface001, TestSize.Level1)
{
    int ret;

    ret = UsbCloseInterface(acm->data_devHandle);
    EXPECT_EQ(HDF_SUCCESS, ret);
}

/**
 * @tc.number    : CheckHostSdkIfOpenInterface002
 * @tc.name      :
 * @tc.type      : PERF
 * @tc.level     : Level 1
 */
HWTEST_F(UsbHostSdkIfTest, CheckHostSdkIfOpenInterface002, TestSize.Level1)
{
    acm->int_devHandle = UsbOpenInterface(acm->intIface);
    EXPECT_NE(nullptr,  acm->int_devHandle);
}

/**
 * @tc.number    : CheckHostSdkIfCloseInterface002
 * @tc.name      :
 * @tc.type      : PERF
 * @tc.level     : Level 1
 */
HWTEST_F(UsbHostSdkIfTest, CheckHostSdkIfCloseInterface002, TestSize.Level1)
{
    int ret;

    ret = UsbCloseInterface(acm->int_devHandle);
    EXPECT_EQ(HDF_SUCCESS, ret);
}

/**
 * @tc.number    : CheckHostSdkIfOpenInterface003
 * @tc.name      :
 * @tc.type      : PERF
 * @tc.level     : Level 1
 */
HWTEST_F(UsbHostSdkIfTest, CheckHostSdkIfOpenInterface003, TestSize.Level1)
{
    acm->ctrl_devHandle = UsbOpenInterface(acm->ctrIface);
    EXPECT_NE(nullptr,  acm->ctrl_devHandle);
}

/**
 * @tc.number    : CheckHostSdkIfCloseInterface003
 * @tc.name      :
 * @tc.type      : PERF
 * @tc.level     : Level 1
 */
HWTEST_F(UsbHostSdkIfTest, CheckHostSdkIfCloseInterface003, TestSize.Level1)
{
    int ret;

    ret = UsbCloseInterface(NULL);
    EXPECT_NE(HDF_SUCCESS, ret);
}

/**
 * @tc.number    : CheckHostSdkIfCloseInterface003
 * @tc.name      :
 * @tc.type      : PERF
 * @tc.level     : Level 1
 */
HWTEST_F(UsbHostSdkIfTest, CheckHostSdkIfCloseInterface004, TestSize.Level1)
{
    int ret;

    ret = UsbCloseInterface(acm->ctrl_devHandle);
    EXPECT_EQ(HDF_SUCCESS, ret);
}

/**
 * @tc.number    : CheckHostSdkIfOpenInterface004
 * @tc.name      :
 * @tc.type      : PERF
 * @tc.level     : Level 1
 */
HWTEST_F(UsbHostSdkIfTest, CheckHostSdkIfOpenInterface004, TestSize.Level1)
{
    acm->data_devHandle = UsbOpenInterface(NULL);
    EXPECT_EQ(nullptr,  acm->data_devHandle);
}

/**
 * @tc.number    : CheckHostSdkIfOpenInterface005
 * @tc.name      :
 * @tc.type      : PERF
 * @tc.level     : Level 1
 */
HWTEST_F(UsbHostSdkIfTest, CheckHostSdkIfOpenInterface005, TestSize.Level1)
{
    int i;

    for (i = 0; i < 100; i++)
    {
        acm->data_devHandle = UsbOpenInterface(acm->dataIface);
        EXPECT_NE(nullptr,  acm->data_devHandle);
    }
}

/**
 * @tc.number    : CheckHostSdkIfOpenInterface006
 * @tc.name      :
 * @tc.type      : PERF
 * @tc.level     : Level 1
 */
HWTEST_F(UsbHostSdkIfTest, CheckHostSdkIfOpenInterface006, TestSize.Level1)
{
    acm->data_devHandle = UsbOpenInterface(acm->dataIface);
    EXPECT_NE(nullptr,  acm->data_devHandle);
    acm->int_devHandle = UsbOpenInterface(acm->intIface);
    EXPECT_NE(nullptr,  acm->int_devHandle);
    acm->ctrl_devHandle = UsbOpenInterface(acm->ctrIface);
    EXPECT_NE(nullptr,  acm->ctrl_devHandle);
    ecm_data_devHandle = UsbOpenInterface(ecm_dataIface);
    EXPECT_NE(nullptr,  ecm_data_devHandle);
    ecm_int_devHandle = UsbOpenInterface(ecm_intIface);
    EXPECT_NE(nullptr,  ecm_int_devHandle);
}

/**
 * @tc.number    : CheckHostSdkIfSelectInterfaceSetting001
 * @tc.name      :
 * @tc.type      : PERF
 * @tc.level     : Level 1
 */
HWTEST_F(UsbHostSdkIfTest, CheckHostSdkIfSelectInterfaceSetting001, TestSize.Level1)
{
    int ret;
    int settingIndex = 0;

    ret = UsbSelectInterfaceSetting(ecm_data_devHandle, settingIndex, &ecm_dataIface);
    EXPECT_EQ(HDF_SUCCESS, ret);
}

/**
 * @tc.number    : CheckHostSdkIfSelectInterfaceSetting002
 * @tc.name      :
 * @tc.type      : PERF
 * @tc.level     : Level 1
 */
HWTEST_F(UsbHostSdkIfTest, CheckHostSdkIfSelectInterfaceSetting002, TestSize.Level1)
{
    int ret;
    int settingIndex = 10;

    ret = UsbSelectInterfaceSetting(ecm_data_devHandle, settingIndex, &ecm_dataIface);
    EXPECT_EQ(HDF_FAILURE, ret);
}

/**
 * @tc.number    : CheckHostSdkIfSelectInterfaceSetting003
 * @tc.name      :
 * @tc.type      : PERF
 * @tc.level     : Level 1
 */
HWTEST_F(UsbHostSdkIfTest, CheckHostSdkIfSelectInterfaceSetting003, TestSize.Level1)
{
    int ret;
    int settingIndex = 100;

    ret = UsbSelectInterfaceSetting(ecm_data_devHandle, settingIndex, &ecm_dataIface);
    EXPECT_EQ(HDF_FAILURE, ret);
}

/**
 * @tc.number    : CheckHostSdkIfSelectInterfaceSetting004
 * @tc.name      :
 * @tc.type      : PERF
 * @tc.level     : Level 1
 */
HWTEST_F(UsbHostSdkIfTest, CheckHostSdkIfSelectInterfaceSetting004, TestSize.Level1)
{
    int ret;
    int settingIndex = 200;

    ret = UsbSelectInterfaceSetting(ecm_data_devHandle, settingIndex, &ecm_dataIface);
    EXPECT_EQ(HDF_FAILURE, ret);
}

/**
 * @tc.number    : CheckHostSdkIfSelectInterfaceSetting005
 * @tc.name      :
 * @tc.type      : PERF
 * @tc.level     : Level 1
 */
HWTEST_F(UsbHostSdkIfTest, CheckHostSdkIfSelectInterfaceSetting005, TestSize.Level1)
{
    int ret;
    int settingIndex = 255;

    ret = UsbSelectInterfaceSetting(ecm_data_devHandle, settingIndex, &ecm_dataIface);
    EXPECT_EQ(HDF_FAILURE, ret);
}

/**
 * @tc.number    : CheckHostSdkIfSelectInterfaceSetting006
 * @tc.name      :
 * @tc.type      : PERF
 * @tc.level     : Level 1
 */
HWTEST_F(UsbHostSdkIfTest, CheckHostSdkIfSelectInterfaceSetting006, TestSize.Level1)
{
    int ret;
    int settingIndex = 1;

    ret = UsbSelectInterfaceSetting(ecm_data_devHandle, settingIndex, &ecm_dataIface);
    EXPECT_EQ(HDF_SUCCESS, ret);
}

/**
 * @tc.number    : CheckHostSdkIfClaimInterface007
 * @tc.name      :
 * @tc.type      : PERF
 * @tc.level     : Level 1
 */
HWTEST_F(UsbHostSdkIfTest, CheckHostSdkIfClaimInterface007, TestSize.Level1)
{
    acm->busNum = 1U;
    acm->devAddr = 2U;
    acm->interfaceIndex = 0U;

    ecm_intIface =  (struct UsbInterface *)UsbClaimInterface(acm->session, acm->busNum, acm->devAddr, acm->interfaceIndex);
    EXPECT_NE(nullptr,  ecm_intIface);
}

/**
 * @tc.number    : CheckHostSdkIfClaimInterface008
 * @tc.name      :
 * @tc.type      : PERF
 * @tc.level     : Level 1
 */
HWTEST_F(UsbHostSdkIfTest, CheckHostSdkIfClaimInterface008, TestSize.Level1)
{
    acm->busNum = 100U;
    acm->devAddr = 200U;
    acm->interfaceIndex = 100U;

    ecm_dataIface =  (struct UsbInterface *)UsbClaimInterface(acm->session, acm->busNum, acm->devAddr, acm->interfaceIndex);
    EXPECT_EQ(nullptr,  ecm_dataIface);
}

/**
 * @tc.number    : CheckHostSdkIfClaimInterface009
 * @tc.name      :
 * @tc.type      : PERF
 * @tc.level     : Level 1
 */
HWTEST_F(UsbHostSdkIfTest, CheckHostSdkIfClaimInterface009, TestSize.Level1)
{
    acm->busNum = 1U;
    acm->devAddr = 2U;
    acm->interfaceIndex = 1U;

    ecm_dataIface =  (struct UsbInterface *)UsbClaimInterface(acm->session, acm->busNum, acm->devAddr, acm->interfaceIndex);
    EXPECT_NE(nullptr,  ecm_dataIface);
}

/**
 * @tc.number    : CheckHostSdkIfGetPipe001
 * @tc.name      :
 * @tc.type      : PERF
 * @tc.level     : Level 1
 */
HWTEST_F(UsbHostSdkIfTest, CheckHostSdkIfGetPipe001, TestSize.Level1)
{
    int ret;
    struct UsbPipeInfo p;
    int i;

    for (i = 0;  i <= acm->dataIface->info.pipeNum; i++) {
        ret = UsbGetPipeInfo(NULL, acm->dataIface->info.curAltSetting, i, &p);
        if (ret < 0) {
            continue;
        }
        if ((p.pipeDirection == USB_PIPE_DIRECTION_IN) && (p.pipeType == USB_PIPE_TYPE_BULK)) {
            struct UsbPipeInfo *pi = (UsbPipeInfo *)OsalMemCalloc(sizeof(*pi));
            EXPECT_NE(nullptr,  pi);
            p.interfaceId = acm->dataIface->info.interfaceIndex;
            *pi = p;
            acm->dataInPipe = pi;
            break;
        }
    }
    EXPECT_EQ(nullptr,  acm->dataInPipe);
}

/**
 * @tc.number    : CheckHostSdkIfGetPipe002
 * @tc.name      :
 * @tc.type      : PERF
 * @tc.level     : Level 1
 */
HWTEST_F(UsbHostSdkIfTest, CheckHostSdkIfGetPipe002, TestSize.Level1)
{
    int ret;
    struct UsbPipeInfo p;
    int i;

    for (i = 0;  i <= acm->dataIface->info.pipeNum; i++) {
        ret = UsbGetPipeInfo(acm->data_devHandle, acm->dataIface->info.curAltSetting, i, &p);
        if (ret < 0) {
            continue;
        }
        if ((p.pipeDirection == USB_PIPE_DIRECTION_IN) && (p.pipeType == USB_PIPE_TYPE_BULK)) {
            struct UsbPipeInfo *pi = (UsbPipeInfo *)OsalMemCalloc(sizeof(*pi));
            EXPECT_NE(nullptr,  pi);
            p.interfaceId = acm->dataIface->info.interfaceIndex;
            *pi = p;
            acm->dataInPipe = pi;
            break;
        }
    }
    EXPECT_NE(nullptr,  acm->dataInPipe);
}

/**
 * @tc.number    : CheckHostSdkIfGetPipe003
 * @tc.name      :
 * @tc.type      : PERF
 * @tc.level     : Level 1
 */
HWTEST_F(UsbHostSdkIfTest, CheckHostSdkIfGetPipe003, TestSize.Level1)
{
    int ret;
    struct UsbPipeInfo p;
    int i;

    for (i = 0;  i <= acm->dataIface->info.pipeNum; i++) {
        ret = UsbGetPipeInfo(NULL, acm->dataIface->info.curAltSetting, i, &p);
        if (ret < 0) {
            continue;
        }
        if ((p.pipeDirection == USB_PIPE_DIRECTION_OUT) && (p.pipeType == USB_PIPE_TYPE_BULK)) {
            struct UsbPipeInfo *pi = (UsbPipeInfo *)OsalMemCalloc(sizeof(*pi));
            EXPECT_NE(nullptr,  pi);
            p.interfaceId = acm->dataIface->info.interfaceIndex;
            *pi = p;
            acm->dataOutPipe = pi;
            break;
        }
    }
    EXPECT_EQ(nullptr,  acm->dataOutPipe);
}

/**
 * @tc.number    : CheckHostSdkIfGetPipe004
 * @tc.name      :
 * @tc.type      : PERF
 * @tc.level     : Level 1
 */
HWTEST_F(UsbHostSdkIfTest, CheckHostSdkIfGetPipe004, TestSize.Level1)
{
    int ret;
    struct UsbPipeInfo p;
    int i;

    for (i = 0;  i <= acm->dataIface->info.pipeNum; i++) {
        ret = UsbGetPipeInfo(acm->data_devHandle, acm->dataIface->info.curAltSetting, i, &p);
        if (ret < 0) {
            continue;
        }
        if ((p.pipeDirection == USB_PIPE_DIRECTION_OUT) && (p.pipeType == USB_PIPE_TYPE_BULK)) {
            struct UsbPipeInfo *pi = (UsbPipeInfo *)OsalMemCalloc(sizeof(*pi));
            EXPECT_NE(nullptr,  pi);
            p.interfaceId = acm->dataIface->info.interfaceIndex;
            *pi = p;
            acm->dataOutPipe = pi;
            break;
        }
    }
    EXPECT_NE(nullptr,  acm->dataOutPipe);
}

/**
 * @tc.number    : CheckHostSdkIfGetPipe005
 * @tc.name      :
 * @tc.type      : PERF
 * @tc.level     : Level 1
 */
HWTEST_F(UsbHostSdkIfTest, CheckHostSdkIfGetPipe005, TestSize.Level1)
{
    int ret;
    struct UsbPipeInfo p;
    int i;

    for (i = 0;  i <= acm->intIface->info.pipeNum; i++) {
        ret = UsbGetPipeInfo(NULL, acm->intIface->info.curAltSetting, i, &p);
        if (ret < 0) {
            continue;
        }
        if ((p.pipeDirection == USB_PIPE_DIRECTION_IN) && (p.pipeType == USB_PIPE_TYPE_INTERRUPT)) {
            struct UsbPipeInfo *pi = (UsbPipeInfo *)OsalMemCalloc(sizeof(*pi));
            p.interfaceId = acm->intIface->info.interfaceIndex;
            *pi = p;
            acm->intPipe = pi;
            break;
        }
    }
    EXPECT_EQ(nullptr,  acm->intPipe);
}

/**
 * @tc.number    : CheckHostSdkIfGetPipe006
 * @tc.name      :
 * @tc.type      : PERF
 * @tc.level     : Level 1
 */
HWTEST_F(UsbHostSdkIfTest, CheckHostSdkIfGetPipe006, TestSize.Level1)
{
    int ret;
    struct UsbPipeInfo p;
    int i;

    for (i = 0;  i <= acm->intIface->info.pipeNum; i++) {
        ret = UsbGetPipeInfo(acm->int_devHandle, acm->intIface->info.curAltSetting, i, &p);
        if (ret < 0) {
            continue;
        }
        if ((p.pipeDirection == USB_PIPE_DIRECTION_IN) && (p.pipeType == USB_PIPE_TYPE_INTERRUPT)) {
            struct UsbPipeInfo *pi = (UsbPipeInfo *)OsalMemCalloc(sizeof(*pi));
            p.interfaceId = acm->intIface->info.interfaceIndex;
            *pi = p;
            acm->intPipe = pi;
            break;
        }
    }
    EXPECT_NE(nullptr,  acm->intPipe);
}

/**
 * @tc.number    : CheckHostSdkIfGetPipe007
 * @tc.name      :
 * @tc.type      : PERF
 * @tc.level     : Level 1
 */
HWTEST_F(UsbHostSdkIfTest, CheckHostSdkIfGetPipe007, TestSize.Level1)
{
    int ret;
    struct UsbPipeInfo p;
    int i;

    acm->interfaceIndex = 255;

    for (i = 0;  i <= acm->ctrIface->info.pipeNum; i++) {
        ret = UsbGetPipeInfo(NULL, acm->ctrIface->info.curAltSetting, i, &p);
        if (ret < 0) {
            continue;
        }
        if ((p.pipeDirection == USB_PIPE_DIRECTION_OUT) && (p.pipeType == USB_PIPE_TYPE_CONTROL)) {
            struct UsbPipeInfo *pi = (UsbPipeInfo *)OsalMemCalloc(sizeof(*pi));
            p.interfaceId = acm->interfaceIndex;
            *pi = p;
            acm->ctrPipe = pi;
            break;
        }
    }
    EXPECT_EQ(nullptr,  acm->ctrPipe);
}

/**
 * @tc.number    : CheckHostSdkIfGetPipe008
 * @tc.name      :
 * @tc.type      : PERF
 * @tc.level     : Level 1
 */
HWTEST_F(UsbHostSdkIfTest, CheckHostSdkIfGetPipe008, TestSize.Level1)
{
    int ret;
    struct UsbPipeInfo p;
    int i;

    acm->interfaceIndex = 255;

    for (i = 0;  i <= acm->ctrIface->info.pipeNum; i++) {
        ret = UsbGetPipeInfo(acm->ctrl_devHandle, acm->ctrIface->info.curAltSetting, i, &p);
        if (ret < 0) {
            continue;
        }
        if ((p.pipeDirection == USB_PIPE_DIRECTION_OUT) && (p.pipeType == USB_PIPE_TYPE_CONTROL)) {
            struct UsbPipeInfo *pi = (UsbPipeInfo *)OsalMemCalloc(sizeof(*pi));
            p.interfaceId = acm->interfaceIndex;
            *pi = p;
            acm->ctrPipe = pi;
            break;
        }
    }
    EXPECT_NE(nullptr,  acm->ctrPipe);
}

/**
 * @tc.number    : CheckHostSdkIfGetPipe009
 * @tc.name      :
 * @tc.type      : PERF
 * @tc.level     : Level 1
 */
HWTEST_F(UsbHostSdkIfTest, CheckHostSdkIfGetPipe009, TestSize.Level1)
{
    int ret;
    struct UsbPipeInfo p;

    ret = UsbGetPipeInfo(NULL, 0, 0, &p);
    EXPECT_NE(HDF_SUCCESS,  ret);
}

/**
 * @tc.number    : CheckHostSdkIfGetPipe010
 * @tc.name      :
 * @tc.type      : PERF
 * @tc.level     : Level 1
 */
HWTEST_F(UsbHostSdkIfTest, CheckHostSdkIfGetPipe010, TestSize.Level1)
{
    int ret;

    ret = UsbGetPipeInfo(acm->ctrl_devHandle, 0, 0, NULL);
    EXPECT_NE(HDF_SUCCESS,  ret);

}

/**
 * @tc.number    : CheckHostSdkIfGetPipe0011
 * @tc.name      :
 * @tc.type      : PERF
 * @tc.level     : Level 1
 */
HWTEST_F(UsbHostSdkIfTest, CheckHostSdkIfGetPipe011, TestSize.Level1)
{
    int ret;
    struct UsbPipeInfo p;

    ret = UsbGetPipeInfo(acm->ctrl_devHandle, 0, 0, &p);
    EXPECT_EQ(HDF_SUCCESS,  ret);
}

/**
 * @tc.number    : CheckHostSdkIfAllocRequest001
 * @tc.name      :
 * @tc.type      : PERF
 * @tc.level     : Level 1
 */
HWTEST_F(UsbHostSdkIfTest, CheckHostSdkIfAllocRequest001, TestSize.Level1)
{
    int i;

    acm->readSize = acm->dataInPipe->maxPacketSize;
    printf("------readSize = [%d]------\n", acm->readSize);
    for (i = 0; i < ACM_NR; i++) {
        acm->readReq[i] = UsbAllocRequest(NULL, 0, acm->readSize);
        EXPECT_EQ(nullptr,  acm->readReq[i]);
    }
}

/**
 * @tc.number    : CheckHostSdkIfAllocRequest002
 * @tc.name      :
 * @tc.type      : PERF
 * @tc.level     : Level 1
 */
HWTEST_F(UsbHostSdkIfTest, CheckHostSdkIfAllocRequest002, TestSize.Level1)
{
    int i;

    acm->readSize = acm->dataInPipe->maxPacketSize;
    printf("------readSize = [%d]------\n", acm->readSize);
    for (i = 0; i < ACM_NR; i++) {
        acm->readReq[i] = UsbAllocRequest(acm->data_devHandle, 0, acm->readSize);
        EXPECT_NE(nullptr,  acm->readReq[i]);
    }
}

/**
 * @tc.number    : CheckHostSdkIfFreeRequest001
 * @tc.name      :
 * @tc.type      : PERF
 * @tc.level     : Level 1
 */
HWTEST_F(UsbHostSdkIfTest, CheckHostSdkIfFreeRequest001, TestSize.Level1)
{
    int ret;
    int i;

    for (i = 0; i < ACM_NR; i++) {
        ret = UsbFreeRequest(acm->readReq[i]);
        EXPECT_EQ(HDF_SUCCESS, ret);
    }
}

/**
 * @tc.number    : CheckHostSdkIfAllocRequest003
 * @tc.name      :
 * @tc.type      : PERF
 * @tc.level     : Level 1
 */
HWTEST_F(UsbHostSdkIfTest, CheckHostSdkIfAllocRequest003, TestSize.Level1)
{
    int i;
    int ret;

    acm->writeSize = acm->dataOutPipe->maxPacketSize;
    ret = AcmWriteBufAlloc(acm);
    EXPECT_EQ(HDF_SUCCESS, ret);

    for (i = 0; i < ACM_NW; i++) {
        acm->wb[i].request = UsbAllocRequest(NULL, 0, acm->writeSize);
        acm->wb[i].instance = acm;
        EXPECT_EQ(nullptr,  acm->wb[i].request);
    }
}

/**
 * @tc.number    : CheckHostSdkIfAllocRequest004
 * @tc.name      :
 * @tc.type      : PERF
 * @tc.level     : Level 1
 */
HWTEST_F(UsbHostSdkIfTest, CheckHostSdkIfAllocRequest004, TestSize.Level1)
{
    int i;
    int ret;

    acm->writeSize = acm->dataOutPipe->maxPacketSize;
    ret = AcmWriteBufAlloc(acm);
    EXPECT_EQ(HDF_SUCCESS, ret);

    for (i = 0; i < ACM_NW; i++) {
        acm->wb[i].request = UsbAllocRequest(acm->data_devHandle, 0, acm->writeSize);
        acm->wb[i].instance = acm;
        EXPECT_NE(nullptr,  acm->wb[i].request);
    }
}

/**
 * @tc.number    : CheckHostSdkIfFreeRequest002
 * @tc.name      :
 * @tc.type      : PERF
 * @tc.level     : Level 1
 */
HWTEST_F(UsbHostSdkIfTest, CheckHostSdkIfFreeRequest002, TestSize.Level1)
{
    int ret;
    int i;

    for (i = 0; i < ACM_NR; i++) {
        ret = UsbFreeRequest(acm->wb[i].request);
        EXPECT_EQ(HDF_SUCCESS, ret);
    }
}

/**
 * @tc.number    : CheckHostSdkIfAllocRequest005
 * @tc.name      :
 * @tc.type      : PERF
 * @tc.level     : Level 1
 */
HWTEST_F(UsbHostSdkIfTest, CheckHostSdkIfAllocRequest005, TestSize.Level1)
{
    acm->intSize = acm->intPipe->maxPacketSize;
    acm->notifyReq = UsbAllocRequest(NULL, 0, acm->intSize);
    EXPECT_EQ(nullptr,  acm->notifyReq);
}

/**
 * @tc.number    : CheckHostSdkIfAllocRequest006
 * @tc.name      :
 * @tc.type      : PERF
 * @tc.level     : Level 1
 */
HWTEST_F(UsbHostSdkIfTest, CheckHostSdkIfAllocRequest006, TestSize.Level1)
{
    acm->intSize = acm->intPipe->maxPacketSize;
    acm->notifyReq = UsbAllocRequest(acm->int_devHandle, 0, acm->intSize);
    EXPECT_NE(nullptr,  acm->notifyReq);
}

/**
 * @tc.number    : CheckHostSdkIfFreeRequest003
 * @tc.name      :
 * @tc.type      : PERF
 * @tc.level     : Level 1
 */
HWTEST_F(UsbHostSdkIfTest, CheckHostSdkIfFreeRequest003, TestSize.Level1)
{
    int ret;

    ret = UsbFreeRequest(acm->notifyReq);
    EXPECT_EQ(HDF_SUCCESS, ret);
}

/**
 * @tc.number    : CheckHostSdkIfAllocRequest007
 * @tc.name      :
 * @tc.type      : PERF
 * @tc.level     : Level 1
 */
HWTEST_F(UsbHostSdkIfTest, CheckHostSdkIfAllocRequest007, TestSize.Level1)
{
    acm->ctrlSize = sizeof (struct UsbCdcLineCoding);
    acm->ctrlReq = UsbAllocRequest(NULL, 0, acm->ctrlSize);
    EXPECT_EQ(nullptr,  acm->ctrlReq);
}

/**
 * @tc.number    : CheckHostSdkIfAllocRequest008
 * @tc.name      :
 * @tc.type      : PERF
 * @tc.level     : Level 1
 */
HWTEST_F(UsbHostSdkIfTest, CheckHostSdkIfAllocRequest008, TestSize.Level1)
{
    acm->ctrlSize = sizeof (struct UsbCdcLineCoding);
    acm->ctrlReq = UsbAllocRequest(acm->ctrl_devHandle, 0, acm->ctrlSize);
    EXPECT_NE(nullptr,  acm->ctrlReq);
}

/**
 * @tc.number    : CheckHostSdkIfFreeRequest004
 * @tc.name      :
 * @tc.type      : PERF
 * @tc.level     : Level 1
 */
HWTEST_F(UsbHostSdkIfTest, CheckHostSdkIfFreeRequest004, TestSize.Level1)
{
    int ret;

    ret = UsbFreeRequest(acm->ctrlReq);
    EXPECT_EQ(HDF_SUCCESS, ret);
}

/**
 * @tc.number    : CheckHostSdkIfFreeRequest005
 * @tc.name      :
 * @tc.type      : PERF
 * @tc.level     : Level 1
 */
HWTEST_F(UsbHostSdkIfTest, CheckHostSdkIfFreeRequest005, TestSize.Level1)
{
    int ret;

    ret = UsbFreeRequest(NULL);
    EXPECT_NE(HDF_SUCCESS, ret);
}

/**
 * @tc.number    : CheckHostSdkIfAllocRequest009
 * @tc.name      :
 * @tc.type      : PERF
 * @tc.level     : Level 1
 */
HWTEST_F(UsbHostSdkIfTest, CheckHostSdkIfAllocRequest009, TestSize.Level1)
{
    int i;

    acm->readSize = acm->dataInPipe->maxPacketSize;
    for (i = 0; i < ACM_NR; i++) {
        acm->readReq[i] = UsbAllocRequest(acm->data_devHandle, 0, acm->readSize);
        EXPECT_NE(nullptr,  acm->readReq[i]);
    }

    acm->writeSize = acm->dataOutPipe->maxPacketSize;
    for (int i = 0; i < ACM_NW; i++) {
        acm->wb[i].request = UsbAllocRequest(acm->data_devHandle, 0, acm->writeSize);
        acm->wb[i].instance = acm;
        EXPECT_NE(nullptr,  acm->wb[i].request);
    }

    acm->intSize = acm->intPipe->maxPacketSize;
    acm->notifyReq = UsbAllocRequest(acm->int_devHandle, 0, acm->intSize);
    EXPECT_NE(nullptr,  acm->notifyReq);
    acm->ctrlSize = sizeof (struct UsbCdcLineCoding);
    acm->ctrlReq = UsbAllocRequest(acm->ctrl_devHandle, 0, acm->ctrlSize);
    EXPECT_NE(nullptr,  acm->ctrlReq);
}

/**
 * @tc.number    : CheckHostSdkIfAllocRequest010
 * @tc.name      :
 * @tc.type      : PERF
 * @tc.level     : Level 1
 */
HWTEST_F(UsbHostSdkIfTest, CheckHostSdkIfAllocRequest010, TestSize.Level1)
{
    struct UsbRequest *req = UsbAllocRequest(NULL, 0, 0);
    EXPECT_EQ(nullptr, req);
}

/**
 * @tc.number    : CheckHostSdkIfFillRequest001
 * @tc.name      :
 * @tc.type      : PERF
 * @tc.level     : Level 1
 */
HWTEST_F(UsbHostSdkIfTest, CheckHostSdkIfFillRequest001, TestSize.Level1)
{
    int ret;
    struct UsbRequestParams readParmas;
    int i;

    for (i = 0; i < 1; i++) {
        readParmas.userData = (void *)acm;
        readParmas.pipeAddress = acm->dataInPipe->pipeAddress;
        readParmas.pipeId = acm->dataInPipe->pipeId;
        readParmas.interfaceId = acm->dataInPipe->interfaceId;
        readParmas.callback = AcmReadBulk;
        readParmas.requestType = USB_REQUEST_PARAMS_DATA_TYPE;
        readParmas.timeout = USB_CTRL_SET_TIMEOUT;
        readParmas.dataReq.numIsoPackets = 0;
        readParmas.dataReq.directon = (UsbRequestDirection)((acm->dataInPipe->pipeDirection >> 7) & 0x1);
        readParmas.dataReq.length = acm->readSize;
        ret = UsbFillRequest(acm->readReq[i], acm->data_devHandle, &readParmas);
        EXPECT_EQ(HDF_SUCCESS, ret);
    }
}

/**
 * @tc.number    : CheckHostSdkIfFillRequest002
 * @tc.name      :
 * @tc.type      : PERF
 * @tc.level     : Level 1
 */
HWTEST_F(UsbHostSdkIfTest, CheckHostSdkIfFillRequest002, TestSize.Level1)
{
    int ret;
    struct UsbRequestParams parmas;
    int i;
    char sendData[] = {"abcde\0"};
    uint32_t size = strlen(sendData) + 1;

    acm->writeSize = acm->dataOutPipe->maxPacketSize;
    size = (size > acm->writeSize) ? acm->writeSize : size;

    for (i = 0; i < 1; i++) {
        acm->wb[i].len = size;
        ret = memcpy_s(acm->wb[i].buf, acm->writeSize, sendData, size);
        if (ret) {
            printf("memcpy_s fial");
        }

        parmas.interfaceId = acm->dataOutPipe->interfaceId;
        parmas.pipeAddress = acm->dataOutPipe->pipeAddress;
        parmas.pipeId = acm->dataOutPipe->pipeId;
        parmas.callback = AcmWriteBulk;
        parmas.requestType = USB_REQUEST_PARAMS_DATA_TYPE;
        parmas.timeout = USB_CTRL_SET_TIMEOUT;
        parmas.dataReq.numIsoPackets = 0;
        parmas.userData = (void *)&acm->wb[i];
        parmas.dataReq.length = acm->wb[i].len;
        parmas.dataReq.buffer = acm->wb[i].buf;
        ret = UsbFillRequest(acm->wb[i].request, acm->data_devHandle, &parmas);
        EXPECT_EQ(HDF_SUCCESS, ret);
    }
}

/**
 * @tc.number    : CheckHostSdkIfFillRequest003
 * @tc.name      :
 * @tc.type      : PERF
 * @tc.level     : Level 1
 */
HWTEST_F(UsbHostSdkIfTest, CheckHostSdkIfFillRequest003, TestSize.Level1)
{
    int ret;
    struct UsbRequestParams intParmas;

    intParmas.userData = (void *)acm;
    intParmas.pipeAddress = acm->intPipe->pipeAddress;
    intParmas.pipeId = acm->intPipe->pipeId;
    intParmas.interfaceId = acm->intPipe->interfaceId;
    intParmas.callback = AcmCtrlIrq;
    intParmas.requestType = USB_REQUEST_PARAMS_DATA_TYPE;
    intParmas.timeout = USB_CTRL_SET_TIMEOUT;
    intParmas.dataReq.numIsoPackets = 0;
    intParmas.dataReq.directon = (UsbRequestDirection)((acm->intPipe->pipeDirection >> USB_PIPE_DIR_OFFSET) & DIRECTION_MASK);
    intParmas.dataReq.length = acm->intSize;
    ret = UsbFillRequest(acm->notifyReq, acm->int_devHandle, &intParmas);
    EXPECT_EQ(HDF_SUCCESS, ret);
}

/**
 * @tc.number    : CheckHostSdkIfFillRequest004
 * @tc.name      :
 * @tc.type      : PERF
 * @tc.level     : Level 1
 */
HWTEST_F(UsbHostSdkIfTest, CheckHostSdkIfFillRequest004, TestSize.Level1)
{
    int ret;
    struct UsbRequestParams parmas;
    uint16_t index = 0;
    uint16_t value = 0;
    struct TestControlMsgData msgData;

    parmas.interfaceId = USB_CTRL_INTERFACE_ID;
    parmas.pipeAddress = 0;
    parmas.pipeId = 0;
    parmas.callback = AcmCtrlIrq;
    parmas.requestType = USB_REQUEST_PARAMS_CTRL_TYPE;
    parmas.timeout = USB_CTRL_SET_TIMEOUT;

    acm->lineCoding.dwDTERate = CpuToLe32(9600);
    acm->lineCoding.bCharFormat = CHARFORMAT;
    acm->lineCoding.bParityType = USB_CDC_NO_PARITY;
    acm->lineCoding.bDataBits = USB_CDC_1_STOP_BITS;

    msgData.request = USB_DDK_CDC_REQ_SET_LINE_CODING;
    msgData.requestType = USB_DDK_TYPE_CLASS | USB_DDK_RECIP_INTERFACE;
    msgData.value = value;
    msgData.index = index;
    msgData.data = &acm->lineCoding;
    msgData.size = sizeof (struct UsbCdcLineCoding);
    parmas.ctrlReq = UsbControlMsg(msgData);
    ret = UsbFillRequest(acm->ctrlReq, acm->ctrl_devHandle, &parmas);
    EXPECT_EQ(HDF_SUCCESS, ret);
}


/**
 * @tc.number    : CheckHostSdkIfFillRequest005
 * @tc.name      :
 * @tc.type      : PERF
 * @tc.level     : Level 1
 */
HWTEST_F(UsbHostSdkIfTest, CheckHostSdkIfFillRequest005, TestSize.Level1)
{
    int ret;
    struct UsbRequestParams readParmas;
    int i;

    for (i = 0; i < ACM_NR; i++) {
        readParmas.userData = (void *)acm;
        readParmas.pipeAddress = acm->dataInPipe->pipeAddress;
        readParmas.pipeId = acm->dataInPipe->pipeId;
        readParmas.interfaceId = acm->dataInPipe->interfaceId;
        readParmas.callback = AcmReadBulk;
        readParmas.requestType = USB_REQUEST_PARAMS_DATA_TYPE;
        readParmas.timeout = USB_CTRL_SET_TIMEOUT;
        readParmas.dataReq.numIsoPackets = 0;
        readParmas.dataReq.directon = (UsbRequestDirection)((acm->dataInPipe->pipeDirection >> 7) & 0x1);
        readParmas.dataReq.length = acm->readSize;
        ret = UsbFillRequest(acm->readReq[i], acm->data_devHandle, &readParmas);
        EXPECT_EQ(HDF_SUCCESS, ret);
    }
}

/**
 * @tc.number    : CheckHostSdkIfFillRequest006
 * @tc.name      :
 * @tc.type      : PERF
 * @tc.level     : Level 1
 */
HWTEST_F(UsbHostSdkIfTest, CheckHostSdkIfFillRequest006, TestSize.Level1)
{
    int ret;
    struct UsbRequestParams parmas;
    int i;
    char sendData[] = {"abcde\0"};
    uint32_t size = strlen(sendData) + 1;

    acm->writeSize = acm->dataOutPipe->maxPacketSize;
    size = (size > acm->writeSize) ? acm->writeSize : size;

    for (i = 0; i < ACM_NR; i++) {
        acm->wb[i].len = size;
        ret = memcpy_s(acm->wb[i].buf, acm->writeSize, sendData, size);
        if (ret) {
            printf("memcpy_s fial");
        }

        parmas.interfaceId = acm->dataOutPipe->interfaceId;
        parmas.pipeAddress = acm->dataOutPipe->pipeAddress;
        parmas.pipeId = acm->dataOutPipe->pipeId;
        parmas.callback = AcmWriteBulk;
        parmas.requestType = USB_REQUEST_PARAMS_DATA_TYPE;
        parmas.timeout = USB_CTRL_SET_TIMEOUT;
        parmas.dataReq.numIsoPackets = 0;
        parmas.userData = (void *)&acm->wb[i];
        parmas.dataReq.length = acm->wb[i].len;
        parmas.dataReq.buffer = acm->wb[i].buf;
        ret = UsbFillRequest(acm->wb[i].request, acm->data_devHandle, &parmas);
        EXPECT_EQ(HDF_SUCCESS, ret);
    }
}

/**
 * @tc.number    : CheckHostSdkIfFillRequest007
 * @tc.name      :
 * @tc.type      : PERF
 * @tc.level     : Level 1
 */
HWTEST_F(UsbHostSdkIfTest, CheckHostSdkIfFillRequest007, TestSize.Level1)
{
    int ret;
    struct UsbRequestParams intParmas;

    intParmas.userData = (void *)acm;
    intParmas.pipeAddress = acm->intPipe->pipeAddress;
    intParmas.pipeId = acm->intPipe->pipeId;
    intParmas.interfaceId = acm->intPipe->interfaceId;
    intParmas.callback = AcmCtrlIrq;
    intParmas.requestType = USB_REQUEST_PARAMS_DATA_TYPE;
    intParmas.timeout = USB_CTRL_SET_TIMEOUT;
    intParmas.dataReq.numIsoPackets = 0;
    intParmas.dataReq.directon = (UsbRequestDirection)((acm->intPipe->pipeDirection >> USB_PIPE_DIR_OFFSET) & DIRECTION_MASK);
    intParmas.dataReq.length = acm->intSize;
    ret = UsbFillRequest(acm->notifyReq, acm->int_devHandle, &intParmas);
    EXPECT_EQ(HDF_SUCCESS, ret);
}

/**
 * @tc.number    : CheckHostSdkIfFillRequest008
 * @tc.name      :
 * @tc.type      : PERF
 * @tc.level     : Level 1
 */
HWTEST_F(UsbHostSdkIfTest, CheckHostSdkIfFillRequest008, TestSize.Level1)
{
    int ret;
    struct UsbRequestParams parmas;
    uint16_t index = 0;
    uint16_t value = 0;
    struct TestControlMsgData msgData;

    parmas.interfaceId = USB_CTRL_INTERFACE_ID;
    parmas.pipeAddress = 0;
    parmas.pipeId = 0;
    parmas.callback = AcmCtrlIrq;
    parmas.requestType = USB_REQUEST_PARAMS_CTRL_TYPE;
    parmas.timeout = USB_CTRL_SET_TIMEOUT;

    acm->lineCoding.dwDTERate = CpuToLe32(9600);
    acm->lineCoding.bCharFormat = CHARFORMAT;
    acm->lineCoding.bParityType = USB_CDC_NO_PARITY;
    acm->lineCoding.bDataBits = USB_CDC_1_STOP_BITS;

    msgData.request = USB_DDK_CDC_REQ_SET_LINE_CODING;
    msgData.requestType = USB_DDK_TYPE_CLASS | USB_DDK_RECIP_INTERFACE;
    msgData.value = value;
    msgData.index = index;
    msgData.data = &acm->lineCoding;
    msgData.size = sizeof (struct UsbCdcLineCoding);
    parmas.ctrlReq = UsbControlMsg(msgData);
    ret = UsbFillRequest(acm->ctrlReq, acm->ctrl_devHandle, &parmas);
    EXPECT_EQ(HDF_SUCCESS, ret);
}

/**
 * @tc.number    : CheckHostSdkIfFillRequest009
 * @tc.name      :
 * @tc.type      : PERF
 * @tc.level     : Level 1
 */
HWTEST_F(UsbHostSdkIfTest, CheckHostSdkIfFillRequest009, TestSize.Level1)
{
    int ret;

    ret = UsbFillRequest(NULL, NULL, NULL);
    EXPECT_NE(HDF_SUCCESS, ret);
}

/**
 * @tc.number    : CheckHostSdkIfFillRequest010
 * @tc.name      :
 * @tc.type      : PERF
 * @tc.level     : Level 1
 */
HWTEST_F(UsbHostSdkIfTest, CheckHostSdkIfFillRequest010, TestSize.Level1)
{
    int ret;
    struct UsbRequestParams params;

    ret = UsbFillRequest(NULL, NULL, &params);
    EXPECT_NE(HDF_SUCCESS, ret);
}

/**
 * @tc.number    : CheckHostSdkIfFillRequest011
 * @tc.name      :
 * @tc.type      : PERF
 * @tc.level     : Level 1
 */
HWTEST_F(UsbHostSdkIfTest, CheckHostSdkIfFillRequest011, TestSize.Level1)
{
    int ret;
    UsbInterfaceHandle interfaceHandle;

    ret = UsbFillRequest(NULL, &interfaceHandle, NULL);
    EXPECT_NE(HDF_SUCCESS, ret);
}

/**
 * @tc.number    : CheckHostSdkIfFillRequest012
 * @tc.name      :
 * @tc.type      : PERF
 * @tc.level     : Level 1
 */
HWTEST_F(UsbHostSdkIfTest, CheckHostSdkIfFillRequest012, TestSize.Level1)
{
    int ret;
    struct UsbRequest request;

    ret = UsbFillRequest(&request, NULL, NULL);
    EXPECT_NE(HDF_SUCCESS, ret);
}

/**
 * @tc.number    : CheckHostSdkIfFillRequest013
 * @tc.name      :
 * @tc.type      : PERF
 * @tc.level     : Level 1
 */
HWTEST_F(UsbHostSdkIfTest, CheckHostSdkIfFillRequest013, TestSize.Level1)
{
    int ret;
    struct UsbRequestParams params;
    UsbInterfaceHandle interfaceHandle;

    ret = UsbFillRequest(NULL, &interfaceHandle, &params);
    EXPECT_NE(HDF_SUCCESS, ret);
}

/**
 * @tc.number    : CheckHostSdkIfFillRequest014
 * @tc.name      :
 * @tc.type      : PERF
 * @tc.level     : Level 1
 */
HWTEST_F(UsbHostSdkIfTest, CheckHostSdkIfFillRequest014, TestSize.Level1)
{
    int ret;
    UsbInterfaceHandle interfaceHandle;
    struct UsbRequest request;

    ret = UsbFillRequest(&request, &interfaceHandle, NULL);
    EXPECT_NE(HDF_SUCCESS, ret);
}

/**
 * @tc.number    : CheckHostSdkIfFillRequest015
 * @tc.name      :
 * @tc.type      : PERF
 * @tc.level     : Level 1
 */
HWTEST_F(UsbHostSdkIfTest, CheckHostSdkIfFillRequest015, TestSize.Level1)
{
    int ret;
    struct UsbRequestParams params;
    struct UsbRequest request;

    ret = UsbFillRequest(&request, NULL, &params);
    EXPECT_NE(HDF_SUCCESS, ret);
}



/**
 * @tc.number    : CheckHostSdkIfClearInterfaceHalt001
 * @tc.name      :
 * @tc.type      : PERF
 * @tc.level     : Level 1
 */
HWTEST_F(UsbHostSdkIfTest, CheckHostSdkIfClearInterfaceHalt001, TestSize.Level1)
{
    int ret;

    ret = UsbClearInterfaceHalt(acm->data_devHandle, acm->dataInPipe->pipeAddress);
    EXPECT_EQ(HDF_SUCCESS, ret);
}

/**
 * @tc.number    : CheckHostSdkIfClearInterfaceHalt002
 * @tc.name      :
 * @tc.type      : PERF
 * @tc.level     : Level 1
 */
HWTEST_F(UsbHostSdkIfTest, CheckHostSdkIfClearInterfaceHalt002, TestSize.Level1)
{
    int ret;

    ret = UsbClearInterfaceHalt(acm->data_devHandle, acm->dataOutPipe->pipeAddress);
    EXPECT_EQ(HDF_SUCCESS, ret);
}

/**
 * @tc.number    : CheckHostSdkIfClearInterfaceHalt003
 * @tc.name      :
 * @tc.type      : PERF
 * @tc.level     : Level 1
 */
HWTEST_F(UsbHostSdkIfTest, CheckHostSdkIfClearInterfaceHalt003, TestSize.Level1)
{
    int ret;

    ret = UsbClearInterfaceHalt(acm->int_devHandle, acm->intPipe->pipeAddress);
    EXPECT_NE(HDF_SUCCESS, ret);
}

/**
 * @tc.number    : CheckHostSdkIfClearInterfaceHalt004
 * @tc.name      :
 * @tc.type      : PERF
 * @tc.level     : Level 1
 */
HWTEST_F(UsbHostSdkIfTest, CheckHostSdkIfClearInterfaceHalt004, TestSize.Level1)
{
    int ret;

    ret = UsbClearInterfaceHalt(acm->ctrl_devHandle, acm->ctrPipe->pipeAddress);
    EXPECT_NE(HDF_SUCCESS, ret);
}

/**
 * @tc.number    : CheckHostSdkIfClearInterfaceHalt005
 * @tc.name      :
 * @tc.type      : PERF
 * @tc.level     : Level 1
 */
HWTEST_F(UsbHostSdkIfTest, CheckHostSdkIfClearInterfaceHalt005, TestSize.Level1)
{
    int ret;

    ret = UsbClearInterfaceHalt(NULL, 0);
    EXPECT_NE(HDF_SUCCESS, ret);
}

/**
 * @tc.number    : CheckHostSdkIfRemoveInterface001
 * @tc.name      :
 * @tc.type      : PERF
 * @tc.level     : Level 1
 */
HWTEST_F(UsbHostSdkIfTest, CheckHostSdkIfRemoveInterface001, TestSize.Level1)
{
    int ret;
    UsbInterfaceStatus status = USB_INTERFACE_STATUS_REMOVE;

    ret = UsbAddOrRemoveInterface(acm->session, acm->busNum, acm->devAddr,
        acm->dataIface->info.interfaceIndex, status);
    EXPECT_EQ(HDF_SUCCESS, ret);
}

/**
 * @tc.number    : CheckHostSdkIfAddInterface001
 * @tc.name      :
 * @tc.type      : PERF
 * @tc.level     : Level 1
 */
HWTEST_F(UsbHostSdkIfTest, CheckHostSdkIfAddInterface001, TestSize.Level1)
{
    int ret;
    UsbInterfaceStatus status = USB_INTERFACE_STATUS_ADD;

    sleep(1);

    ret = UsbAddOrRemoveInterface(acm->session, acm->busNum, acm->devAddr,
        acm->dataIface->info.interfaceIndex, status);
    EXPECT_EQ(HDF_SUCCESS, ret);
}

/**
 * @tc.number    : CheckHostSdkIfRemoveInterface002
 * @tc.name      :
 * @tc.type      : PERF
 * @tc.level     : Level 1
 */
HWTEST_F(UsbHostSdkIfTest, CheckHostSdkIfRemoveInterface002, TestSize.Level1)
{
    int ret;
    UsbInterfaceStatus status = USB_INTERFACE_STATUS_REMOVE;

    sleep(1);

    ret = UsbAddOrRemoveInterface(acm->session, acm->busNum, acm->devAddr,
        acm->intIface->info.interfaceIndex, status);
    EXPECT_EQ(HDF_SUCCESS, ret);
}

/**
 * @tc.number    : CheckHostSdkIfAddInterface002
 * @tc.name      :
 * @tc.type      : PERF
 * @tc.level     : Level 1
 */
HWTEST_F(UsbHostSdkIfTest, CheckHostSdkIfAddInterface002, TestSize.Level1)
{
    int ret;
    UsbInterfaceStatus status = USB_INTERFACE_STATUS_ADD;

    sleep(1);

    ret = UsbAddOrRemoveInterface(acm->session, acm->busNum, acm->devAddr,
        acm->intIface->info.interfaceIndex, status);
    EXPECT_EQ(HDF_SUCCESS, ret);
}

/**
 * @tc.number    : CheckHostSdkIfRemoveInterface003
 * @tc.name      :
 * @tc.type      : PERF
 * @tc.level     : Level 1
 */
HWTEST_F(UsbHostSdkIfTest, CheckHostSdkIfRemoveInterface003, TestSize.Level1)
{
    int ret;
    UsbInterfaceStatus status = USB_INTERFACE_STATUS_REMOVE;

    ret = UsbAddOrRemoveInterface(acm->session, acm->busNum, acm->devAddr,
        acm->ctrIface->info.interfaceIndex, status);
    EXPECT_EQ(HDF_SUCCESS, ret);
}

/**
 * @tc.number    : CheckHostSdkIfAddInterface003
 * @tc.name      :
 * @tc.type      : PERF
 * @tc.level     : Level 1
 */
HWTEST_F(UsbHostSdkIfTest, CheckHostSdkIfAddInterface003, TestSize.Level1)
{
    int ret;
    UsbInterfaceStatus status = USB_INTERFACE_STATUS_ADD;

    ret = UsbAddOrRemoveInterface(acm->session, acm->busNum, acm->devAddr,
        acm->ctrIface->info.interfaceIndex, status);
    EXPECT_EQ(HDF_SUCCESS, ret);
}

}
