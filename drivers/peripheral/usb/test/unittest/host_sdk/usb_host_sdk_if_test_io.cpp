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

static void AcmCtrlIrq(struct UsbRequest *req)
{
    if (req == NULL) {
        printf("%s:%d req is NULL!", __func__, __LINE__);
        return;
    }
    int status = req->compInfo.status;
    unsigned int currentSize = req->compInfo.actualLength;
    printf("Irqstatus:%d,actualLength:%u\n", status, currentSize);
    switch (status) {
        case 0:
            break;
        default:
            return;
    }

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
static void AcmGetPipe()
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
        }

        if ((p.pipeDirection == USB_PIPE_DIRECTION_OUT) && (p.pipeType == USB_PIPE_TYPE_BULK)) {
            struct UsbPipeInfo *pi = (UsbPipeInfo *)OsalMemCalloc(sizeof(*pi));
            EXPECT_NE(nullptr,  pi);
            p.interfaceId = acm->dataIface->info.interfaceIndex;
            *pi = p;
            acm->dataOutPipe = pi;
        }
    }

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
}

static void AcmGetRequest()
{
    int ret;
    int i;
    acm->readSize = acm->dataInPipe->maxPacketSize;
    for (i = 0; i < ACM_NR; i++) {
        acm->readReq[i] = UsbAllocRequest(acm->data_devHandle, 0, acm->readSize);
        EXPECT_NE(nullptr,  acm->readReq[i]);
    }

    acm->writeSize = acm->dataOutPipe->maxPacketSize;
    ret = AcmWriteBufAlloc(acm);
    EXPECT_EQ(HDF_SUCCESS, ret);

    for (i = 0; i < ACM_NW; i++) {
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

static void AcmFillReadRequest()
{
    int i;
    struct UsbRequestParams readParmas;
    int ret;
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

static void AcmFillWriteRequest()
{
    struct UsbRequestParams parmas;
    char sendData[] = {"abcde\0"};
    uint32_t size = strlen(sendData) + 1;
    int i;
    int ret;

    acm->writeSize = acm->dataOutPipe->maxPacketSize;
    size = (size > acm->writeSize) ? acm->writeSize : size;

    for (i = 0; i < ACM_NW; i++) {
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

static void AcmFillIntRequest()
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

static void AcmFillCtrlRequest()
{
    int ret;
    struct UsbRequestParams parmas;
    uint16_t index = 2;
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
    msgData.size = sizeof(struct UsbCdcLineCoding);
    parmas.ctrlReq = UsbControlMsg(msgData);
    ret = UsbFillRequest(acm->ctrlReq, acm->ctrl_devHandle, &parmas);
    EXPECT_EQ(HDF_SUCCESS, ret);
}

static void AcmInit()
{
    int ret;

    ret = UsbInitHostSdk(&session);
    EXPECT_EQ(HDF_SUCCESS, ret);
    acm->session = session;

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

    acm->interfaceIndex = 255U;
    acm->ctrIface =  (struct UsbInterface *)UsbClaimInterface(acm->session, acm->busNum,
        acm->devAddr, acm->interfaceIndex);
    EXPECT_NE(nullptr,  acm->ctrIface);

    acm->data_devHandle = UsbOpenInterface(acm->dataIface);
    EXPECT_NE(nullptr,  acm->data_devHandle);
    acm->int_devHandle = UsbOpenInterface(acm->intIface);
    EXPECT_NE(nullptr,  acm->int_devHandle);
    acm->ctrl_devHandle = UsbOpenInterface(acm->ctrIface);
    EXPECT_NE(nullptr,  acm->ctrl_devHandle);

    AcmGetPipe();
    AcmGetRequest();
    AcmFillReadRequest();
    AcmFillWriteRequest();
    AcmFillIntRequest();
    AcmFillCtrlRequest();
}

/**
 * @tc.number    : CheckHostSdkIfSubmitRequestSync001
 * @tc.name      :
 * @tc.type      : PERF
 * @tc.level     : Level 1
 */
HWTEST_F(UsbHostSdkIfTest, CheckHostSdkIfSubmitRequestSync001, TestSize.Level1)
{
    int ret;
    int i;
    AcmInit();

    for (i = 0; i < 1; i++) {
        printf("------UsbSubmitRequestSync i = [%d]------\n", i);
        ret = UsbSubmitRequestSync(acm->readReq[i]);
        EXPECT_EQ(HDF_SUCCESS, ret);
    }
}

/**
 * @tc.number    : CheckHostSdkIfSubmitRequestSync002
 * @tc.name      :
 * @tc.type      : PERF
 * @tc.level     : Level 1
 */
HWTEST_F(UsbHostSdkIfTest, CheckHostSdkIfSubmitRequestSync002, TestSize.Level1)
{
    int ret;
    int i;

    for (i = 0; i < 1; i++) {
        printf("------UsbSubmitRequestSync i = [%d]------\n", i);
        ret = UsbSubmitRequestSync(acm->wb[i].request);
        EXPECT_EQ(HDF_SUCCESS, ret);
    }
}

/**
 * @tc.number    : CheckHostSdkIfSubmitRequestSync003
 * @tc.name      :
 * @tc.type      : PERF
 * @tc.level     : Level 1
 */
HWTEST_F(UsbHostSdkIfTest, CheckHostSdkIfSubmitRequestSync003, TestSize.Level1)
{
    int ret;

    ret = UsbSubmitRequestSync(acm->notifyReq);
    EXPECT_EQ(HDF_SUCCESS, ret);
}

/**
 * @tc.number    : CheckHostSdkIfSubmitRequestSync004
 * @tc.name      :
 * @tc.type      : PERF
 * @tc.level     : Level 1
 */
HWTEST_F(UsbHostSdkIfTest, CheckHostSdkIfSubmitRequestSync004, TestSize.Level1)
{
    int ret;

    ret = UsbSubmitRequestSync(acm->notifyReq);
    EXPECT_EQ(HDF_SUCCESS, ret);
}

/**
 * @tc.number    : CheckHostSdkIfSubmitRequestAsync001
 * @tc.name      :
 * @tc.type      : PERF
 * @tc.level     : Level 1
 */
HWTEST_F(UsbHostSdkIfTest, CheckHostSdkIfSubmitRequestAsync001, TestSize.Level1)
{
    int ret;
    int i;

    for (i = 0; i < ACM_NR; i++) {
        ret = UsbSubmitRequestAsync(acm->readReq[i]);
        EXPECT_EQ(HDF_SUCCESS, ret);
    }
}

/**
 * @tc.number    : CheckHostSdkIfCancelRequest001
 * @tc.name      :
 * @tc.type      : PERF
 * @tc.level     : Level 1
 */
HWTEST_F(UsbHostSdkIfTest, CheckHostSdkIfCancelRequest001, TestSize.Level1)
{
    int ret;
    int i = 0;

    ret = UsbCancelRequest(acm->readReq[i]);
    EXPECT_EQ(HDF_SUCCESS, ret);
}

/**
 * @tc.number    : CheckHostSdkIfSubmitRequestAsync002
 * @tc.name      :
 * @tc.type      : PERF
 * @tc.level     : Level 1
 */
HWTEST_F(UsbHostSdkIfTest, CheckHostSdkIfSubmitRequestAsync002, TestSize.Level1)
{
    int ret;
    int i;

    for (i = 0; i < ACM_NR; i++) {
        ret = UsbSubmitRequestAsync(acm->wb[i].request);
        EXPECT_EQ(HDF_SUCCESS, ret);
    }
}

/**
 * @tc.number    : CheckHostSdkIfCancelRequest002
 * @tc.name      :
 * @tc.type      : PERF
 * @tc.level     : Level 1
 */
HWTEST_F(UsbHostSdkIfTest, CheckHostSdkIfCancelRequest002, TestSize.Level1)
{
    int ret;
    int i = 0;

    i = ACM_NR-1;
    ret = UsbCancelRequest(acm->wb[i].request);
    EXPECT_EQ(HDF_SUCCESS, ret);
}

/**
 * @tc.number    : CheckHostSdkIfSubmitRequestAsync003
 * @tc.name      :
 * @tc.type      : PERF
 * @tc.level     : Level 1
 */
HWTEST_F(UsbHostSdkIfTest, CheckHostSdkIfSubmitRequestAsync003, TestSize.Level1)
{
    int ret;

    ret = UsbSubmitRequestAsync(acm->notifyReq);
    EXPECT_EQ(HDF_SUCCESS, ret);
}

/**
 * @tc.number    : CheckHostSdkIfCancelRequest003
 * @tc.name      :
 * @tc.type      : PERF
 * @tc.level     : Level 1
 */
HWTEST_F(UsbHostSdkIfTest, CheckHostSdkIfCancelRequest003, TestSize.Level1)
{
    int ret;

    ret = UsbCancelRequest(acm->notifyReq);
    EXPECT_EQ(HDF_SUCCESS, ret);
}

/**
 * @tc.number    : CheckHostSdkIfSubmitRequestAsync004
 * @tc.name      :
 * @tc.type      : PERF
 * @tc.level     : Level 1
 */
HWTEST_F(UsbHostSdkIfTest, CheckHostSdkIfSubmitRequestAsync004, TestSize.Level1)
{
    int ret;

    ret = UsbSubmitRequestAsync(acm->readReq[0]);

    EXPECT_EQ(HDF_SUCCESS, ret);
}

/**
 * @tc.number    : CheckHostSdkIfCancelRequest004
 * @tc.name      :
 * @tc.type      : PERF
 * @tc.level     : Level 1
 */
HWTEST_F(UsbHostSdkIfTest, CheckHostSdkIfCancelRequest004, TestSize.Level1)
{
    int ret;

    ret = UsbSubmitRequestAsync(acm->readReq[0]);
    EXPECT_EQ(HDF_SUCCESS, ret);
}

}
