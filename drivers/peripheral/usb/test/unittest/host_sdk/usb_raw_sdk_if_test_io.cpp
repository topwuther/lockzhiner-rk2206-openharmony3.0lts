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
#include "usb_raw_sdk_if_test.h"
#include "hdf_base.h"
#include "hdf_log.h"
#include "osal_mem.h"
#include "osal_time.h"
#include "securec.h"
#include "hdf_usb_pnp_manage.h"
}

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

#define USB_RAW_IO_SLEEP_MS_TIME    500
#define USB_IO_THREAD_STACK_SIZE   8192

static struct AcmDevice *acm = NULL;
static struct AcmDevice deviceService;
static UsbRawHandle *devHandle = NULL;
static UsbRawDevice *dev = NULL;
static int activeConfig;
static bool g_stopIoThreadFlag = false;

static void AcmRawInit();
static int UsbIoThread(void *data)
{
    int ret;
    struct AcmDevice *acm = (struct AcmDevice *)data;

    for (;;) {
        if (acm == NULL) {
            printf("%s:%d acm is NULL\n", __func__, __LINE__);
            OsalMSleep(USB_RAW_IO_SLEEP_MS_TIME);
            continue;
        }

        if (acm->devHandle == NULL) {
            printf("%s:%d acm->devHandle is NULL!\n", __func__, __LINE__);
            OsalMSleep(USB_RAW_IO_SLEEP_MS_TIME);
            continue;
        }
        ret = UsbRawHandleRequests(acm->devHandle);
        if (ret < 0) {
            printf("%s:%d UsbRawHandleRequests faile, ret=%d \n", __func__, __LINE__, ret);
            if (ret == HDF_DEV_ERR_NO_DEVICE) {
                printf("%s:%d, ret=%d\n", __func__, __LINE__, ret);
                OsalMSleep(USB_RAW_IO_SLEEP_MS_TIME);
            }
        }

        if (g_stopIoThreadFlag == true) {
            printf("%s:%d\n", __func__, __LINE__);
            g_stopIoThreadFlag = false;
            break;
        }
    }

    return HDF_SUCCESS;
}


static int UsbStartIo(struct AcmDevice *acm)
{
    struct OsalThreadParam threadCfg;
    int ret;

    printf("%s start\n", __func__);

    /* creat Io thread */
    (void)memset_s(&threadCfg, sizeof(threadCfg), 0, sizeof(threadCfg));
    threadCfg.name      = (char *)("usb io thread");
    threadCfg.priority  = OSAL_THREAD_PRI_LOW;
    threadCfg.stackSize = USB_IO_THREAD_STACK_SIZE;

    ret = OsalThreadCreate(&acm->ioThread, \
                           (OsalThreadEntry)UsbIoThread, (void *)acm);
    if (ret != HDF_SUCCESS) {
        printf("%s:%d OsalThreadCreate faile, ret=%d \n", __func__, __LINE__, ret);
        return ret;
    }

    ret = OsalThreadStart(&acm->ioThread, &threadCfg);
    if (ret != HDF_SUCCESS) {
        printf("%s:%d OsalThreadStart faile, ret=%d \n", __func__, __LINE__, ret);
        return ret;
    }

    return HDF_SUCCESS;
}

static int UsbStopIo(struct AcmDevice *acm)
{
    int ret;

    HDF_LOGD("%{public}s:%{public}d", __func__, __LINE__);
    if (g_stopIoThreadFlag == false) {
        HDF_LOGD("%{public}s:%{public}d", __func__, __LINE__);
        g_stopIoThreadFlag = true;
    }
    ret = OsalThreadDestroy(&acm->ioThread);
    if (ret != HDF_SUCCESS) {
        HDF_LOGE("%{public}s:%{public}d OsalThreadDestroy faile, ret=%{public}d ", __func__, __LINE__, ret);
        return ret;
    }

    return HDF_SUCCESS;
}

void UsbHostSdkIfTest::SetUpTestCase()
{
    acm = &deviceService;
    AcmRawInit();
    UsbStartIo(acm);
}

void UsbHostSdkIfTest::TearDownTestCase()
{
    acm = &deviceService;
    UsbStopIo(acm);
}

void UsbHostSdkIfTest::SetUp()
{
}

void UsbHostSdkIfTest::TearDown()
{
}

static void AcmWriteBulkCallback(const void *requestArg)
{
    struct UsbRawRequest *req = (struct UsbRawRequest *)requestArg;

    printf("%s:%d entry!", __func__, __LINE__);

    if (req == NULL) {
        printf("%s:%d req is NULL!", __func__, __LINE__);
        return;
    }
    struct AcmWb *wb  = (struct AcmWb *)req->userData;
    if (wb == NULL) {
        printf("%s:%d userData(wb) is NULL!", __func__, __LINE__);
        return;
    }

    if (req->status != USB_REQUEST_COMPLETED) {
        printf("%s: write req failed, status=%d", __func__, req->status);
    }

    wb->use = 0;
}

static void AcmReadBulkCallback(const void *requestArg)
{
    struct UsbRawRequest *req = (struct UsbRawRequest *)requestArg;

    printf("%s:%d entry!", __func__, __LINE__);

    if (req == NULL) {
        printf("%s:%d req is NULL!", __func__, __LINE__);
        return;
    }
    struct AcmDevice *acm = (struct AcmDevice *)req->userData;
    if (acm == NULL) {
        printf("%s:%d userData(acm) is NULL!", __func__, __LINE__);
        return;
    }
    size_t size = req->actualLength;

    switch (req->status) {
        case USB_REQUEST_COMPLETED:
            HDF_LOGD("Bulk status: %{public}d+size:%{public}zu\n", req->status, size);
            if (size) {
                uint8_t *data = req->buffer;
                uint32_t count;

                OsalMutexLock(&acm->readLock);
                if (DataFifoIsFull(&acm->port->readFifo)) {
                    DataFifoSkip(&acm->port->readFifo, size);
                }
                count = DataFifoWrite(&acm->port->readFifo, data, size);
                if (count != size) {
                    printf("%s: write %u less than expected %zu", __func__, count, size);
                }
                OsalMutexUnlock(&acm->readLock);
            }
            break;
        case USB_REQUEST_CANCELLED:
            printf("%s: the request is cancelled", __func__);
            break;
        default:
            printf("%s: the request is failed", __func__);
            break;
    }
}

static void AcmNotifyReqCallback(const void *requestArg)
{
    struct UsbRawRequest *req = (struct UsbRawRequest *)requestArg;

    printf("%s:%d entry!", __func__, __LINE__);

    if (req == NULL) {
        printf("%s:%d req is NULL!", __func__, __LINE__);
        return;
    }
    struct AcmDevice *acm = (struct AcmDevice *)req->userData;
    if (acm == NULL) {
        printf("%s:%d userData(acm) is NULL!", __func__, __LINE__);
        return;
    }
    struct UsbCdcNotification *dr = (struct UsbCdcNotification *)req->buffer;
    if (dr == NULL) {
        printf("%s:%d req->buffer(dr) is NULL!", __func__, __LINE__);
        return;
    }
    unsigned int currentSize = req->actualLength;
    printf("Irqstatus:%d,actualLength:%u\n", req->status, currentSize);
}

static int AcmWriteBufAlloc(struct AcmDevice *acm)
{
    struct AcmWb *wb = &acm->wb[0];
    int i;

    for (i = 0; i < ACM_NW; i++, wb++) {
        wb->buf = (uint8_t *)OsalMemCalloc(acm->dataOutEp->maxPacketSize);
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
    return HDF_SUCCESS;
}

static int UsbParseConfigDescriptor(struct AcmDevice *acm, struct UsbRawConfigDescriptor *config)
{
    uint8_t i;
    uint8_t j;
    int ret;

    if ((acm == NULL) || (config == NULL)) {
        HDF_LOGE("%{public}s:%{public}d acm or config is NULL",
                 __func__, __LINE__);
        return HDF_ERR_INVALID_PARAM;
    }
    acm->interfaceCnt = 2;
    acm->interfaceIndex[0] = 2;
    acm->interfaceIndex[1] = 3;

    for (i = 0; i < acm->interfaceCnt; i++) {
        uint8_t interfaceIndex = acm->interfaceIndex[i];
        const struct UsbRawInterface *interface = config->interface[interfaceIndex];
        uint8_t ifaceClass = interface->altsetting->interfaceDescriptor.bInterfaceClass;
        uint8_t numEndpoints = interface->altsetting->interfaceDescriptor.bNumEndpoints;

        ret = UsbRawClaimInterface(acm->devHandle, interfaceIndex);
        if (ret) {
            HDF_LOGE("%{public}s:%{public}d claim interface %{public}u failed",
                     __func__, __LINE__, i);
            return ret;
        }

        switch (ifaceClass) {
            case USB_DDK_CLASS_COMM:
                acm->ctrlIface = interfaceIndex;
                acm->notifyEp = (struct UsbEndpoint *)OsalMemAlloc(sizeof(struct UsbEndpoint));
                if (acm->notifyEp == NULL) {
                    HDF_LOGE("%{public}s:%{public}d allocate endpoint failed",
                             __func__, __LINE__);
                    break;
                }
                /* get the first endpoint by default */
                acm->notifyEp->addr = interface->altsetting->endPoint[0].endpointDescriptor.bEndpointAddress;
                acm->notifyEp->interval = interface->altsetting->endPoint[0].endpointDescriptor.bInterval;
                acm->notifyEp->maxPacketSize = interface->altsetting->endPoint[0].endpointDescriptor.wMaxPacketSize;
                break;
            case USB_DDK_CLASS_CDC_DATA:
                acm->dataIface = interfaceIndex;
                for (j = 0; j < numEndpoints; j++) {
                    const struct UsbRawEndpointDescriptor *endPoint = &interface->altsetting->endPoint[j];

                    /* get bulk in endpoint */
                    if ((endPoint->endpointDescriptor.bEndpointAddress & USB_DDK_ENDPOINT_DIR_MASK) == USB_DDK_DIR_IN) {
                        acm->dataInEp = (struct UsbEndpoint *)OsalMemAlloc(sizeof(struct UsbEndpoint));
                        if (acm->dataInEp == NULL) {
                            HDF_LOGE("%{public}s:%{public}d allocate dataInEp failed",
                                     __func__, __LINE__);
                            break;
                        }
                        acm->dataInEp->addr = endPoint->endpointDescriptor.bEndpointAddress;
                        acm->dataInEp->interval = endPoint->endpointDescriptor.bInterval;
                        acm->dataInEp->maxPacketSize = endPoint->endpointDescriptor.wMaxPacketSize;
                    } else { /* get bulk out endpoint */
                        acm->dataOutEp = (struct UsbEndpoint *)OsalMemAlloc(sizeof(struct UsbEndpoint));
                        if (acm->dataOutEp == NULL) {
                            HDF_LOGE("%{public}s:%{public}d allocate dataOutEp failed",
                                     __func__, __LINE__);
                            break;
                        }
                        acm->dataOutEp->addr = endPoint->endpointDescriptor.bEndpointAddress;
                        acm->dataOutEp->interval = endPoint->endpointDescriptor.bInterval;
                        acm->dataOutEp->maxPacketSize = endPoint->endpointDescriptor.wMaxPacketSize;
                    }
                }
                break;
            default:
                HDF_LOGE("%{public}s:%{public}d wrong descriptor type", __func__, __LINE__);
                break;
        }
    }

    return HDF_SUCCESS;
}

static void AcmRawAllocRequest()
{
    int i;

    for (i = 0; i < ACM_NW; i++) {
        acm->wb[i].request = UsbRawAllocRequest(acm->devHandle, 0, acm->dataOutEp->maxPacketSize);
        acm->wb[i].instance = acm;
        EXPECT_NE(nullptr,  acm->wb[i].request);
    }
    for (i = 0; i < ACM_NR; i++) {
        acm->readReq[i] = UsbRawAllocRequest(acm->devHandle, 0, acm->dataInEp->maxPacketSize);
        EXPECT_NE(nullptr,  acm->readReq[i]);
    }

    acm->ctrlReq = UsbRawAllocRequest(acm->devHandle, 0, USB_CTRL_REQ_SIZE);
    EXPECT_NE(nullptr,  acm->ctrlReq);

    acm->notifyReq = UsbRawAllocRequest(acm->devHandle, 0, acm->notifyEp->maxPacketSize);
    EXPECT_NE(nullptr,  acm->notifyReq);
}

static void AcmRawFillWriteReq()
{
    struct UsbRawFillRequestData reqData;
    int32_t ret;
    int i;
    uint32_t size;
    char sendData[] = {"abcde\0"};

    size = strlen(sendData) + 1;

    size = (size > acm->dataOutEp->maxPacketSize) ? acm->dataOutEp->maxPacketSize : size;

    for (i = 0; i < 1; i++) {
        AcmWb *snd = &acm->wb[i];
        snd->len = size;
        ret = memcpy_s(snd->buf, acm->dataOutEp->maxPacketSize, sendData, size);
        if (ret) {
            printf("memcpy_s fial");
        }

        reqData.endPoint      = acm->dataOutEp->addr;
        reqData.numIsoPackets = 0;
        reqData.callback      = AcmWriteBulkCallback;
        reqData.userData      = (void *)snd;
        reqData.timeout       = USB_CTRL_SET_TIMEOUT;
        reqData.buffer        = snd->buf;
        reqData.length        = snd->len;

        ret = UsbRawFillBulkRequest(snd->request, acm->devHandle, &reqData);
        EXPECT_EQ(HDF_SUCCESS, ret);
    }
}

static void AcmRawFillReadReq()
{
    struct UsbRawFillRequestData reqData;
    int32_t ret;
    int i;
    int size = acm->dataInEp->maxPacketSize;

    for (i = 0; i < 1; i++) {
        reqData.endPoint      = acm->dataInEp->addr;
        reqData.numIsoPackets = 0;
        reqData.callback      = AcmReadBulkCallback;
        reqData.userData      = (void *)acm;
        reqData.timeout       = USB_CTRL_SET_TIMEOUT;
        reqData.length        = size;

        ret = UsbRawFillBulkRequest(acm->readReq[i], acm->devHandle, &reqData);
        EXPECT_EQ(HDF_SUCCESS, ret);
    }
}


static void AcmRawFillIntReq()
{
    struct UsbRawFillRequestData fillRequestData;
    int32_t ret;
    int size = acm->notifyEp->maxPacketSize;

    fillRequestData.endPoint = acm->notifyEp->addr;
    fillRequestData.length = size;
    fillRequestData.numIsoPackets = 0;
    fillRequestData.callback = AcmNotifyReqCallback;
    fillRequestData.userData = (void *)acm;
    fillRequestData.timeout = USB_CTRL_SET_TIMEOUT;

    ret = UsbRawFillInterruptRequest(acm->notifyReq, acm->devHandle, &fillRequestData);
    EXPECT_EQ(HDF_SUCCESS, ret);
}


static void AcmRawFillCtrlReq()
{
    struct UsbControlRequestData ctrlReq;
    unsigned char setup[100] = {0};
    int ret;

    acm->lineCoding.dwDTERate = CpuToLe32(9600);
    acm->lineCoding.bCharFormat = CHARFORMAT;
    acm->lineCoding.bParityType = USB_CDC_NO_PARITY;
    acm->lineCoding.bDataBits = USB_CDC_1_STOP_BITS;

    ctrlReq.requestType = USB_DDK_DIR_OUT | USB_DDK_TYPE_CLASS | USB_DDK_RECIP_INTERFACE;
    ctrlReq.requestCmd  = USB_DDK_CDC_REQ_SET_LINE_CODING;
    ctrlReq.value       = CpuToLe16(0);
    ctrlReq.index       = 2;
    ctrlReq.data        = (unsigned char *)&acm->lineCoding;
    ctrlReq.length      = sizeof(struct UsbCdcLineCoding);
    ctrlReq.timeout     = USB_CTRL_SET_TIMEOUT;

    ret = UsbRawFillControlSetup(setup, &ctrlReq);
    EXPECT_EQ(HDF_SUCCESS, ret);
}

static void AcmRawInit()
{
    int32_t ret;
    struct UsbDeviceDescriptor desc;
    acm->busNum = 1U;
    acm->devAddr = 2U;

    ret = UsbRawInit(NULL);
    EXPECT_EQ(HDF_SUCCESS, ret);

    devHandle = UsbRawOpenDevice(NULL, acm->busNum, acm->devAddr);
    EXPECT_NE(nullptr,  devHandle);
    acm->devHandle = devHandle;

    ret = UsbRawGetConfiguration(acm->devHandle, &activeConfig);
    EXPECT_EQ(HDF_SUCCESS, ret);
    dev = UsbRawGetDevice(acm->devHandle);
    EXPECT_NE(nullptr,  dev);
    ret = UsbRawGetConfigDescriptor(dev, activeConfig, &acm->config);
    EXPECT_EQ(HDF_SUCCESS, ret);
    ret = UsbRawGetDeviceDescriptor(dev, &desc);
    EXPECT_EQ(HDF_SUCCESS, ret);
    ret = UsbParseConfigDescriptor(acm, acm->config);
    EXPECT_EQ(HDF_SUCCESS, ret);

    AcmRawAllocRequest();
    ret = AcmWriteBufAlloc(acm);
    EXPECT_EQ(HDF_SUCCESS, ret);
    AcmRawFillWriteReq();
    AcmRawFillReadReq();
    AcmRawFillIntReq();
    AcmRawFillCtrlReq();
}


/**
 * @tc.number    : CheckRawSdkIfSendControlRequest001
 * @tc.name      :
 * @tc.type      : PERFs
 * @tc.level     : Level 1
 */
HWTEST_F(UsbHostSdkIfTest, CheckRawSdkIfSendControlRequest001, TestSize.Level1)
{
    struct UsbControlRequestData ctrlReq;
    int ret;

    acm->lineCoding.dwDTERate = CpuToLe32(9600);
    acm->lineCoding.bCharFormat = CHARFORMAT;
    acm->lineCoding.bParityType = USB_CDC_NO_PARITY;
    acm->lineCoding.bDataBits = USB_CDC_1_STOP_BITS;

    ctrlReq.requestType = USB_DDK_DIR_OUT | USB_DDK_TYPE_CLASS | USB_DDK_RECIP_INTERFACE;
    ctrlReq.requestCmd  = USB_DDK_CDC_REQ_SET_LINE_CODING;
    ctrlReq.value       = CpuToLe16(0);
    ctrlReq.index       = 0;
    ctrlReq.data        = (unsigned char *)&acm->lineCoding;
    ctrlReq.length      = sizeof(struct UsbCdcLineCoding);
    ctrlReq.timeout     = USB_CTRL_SET_TIMEOUT;

    ret = UsbRawSendControlRequest(NULL, acm->devHandle, &ctrlReq);
    EXPECT_EQ(HDF_ERR_INVALID_PARAM, ret);
}

/**
 * @tc.number    : CheckRawSdkIfSendControlRequest002
 * @tc.name      :
 * @tc.type      : PERFs
 * @tc.level     : Level 1
 */
HWTEST_F(UsbHostSdkIfTest, CheckRawSdkIfSendControlRequest002, TestSize.Level1)
{
    struct UsbControlRequestData ctrlReq;
    int ret;

    acm->lineCoding.dwDTERate = CpuToLe32(9600);
    acm->lineCoding.bCharFormat = CHARFORMAT;
    acm->lineCoding.bParityType = USB_CDC_NO_PARITY;
    acm->lineCoding.bDataBits = USB_CDC_1_STOP_BITS;

    ctrlReq.requestType = USB_DDK_DIR_OUT | USB_DDK_TYPE_CLASS | USB_DDK_RECIP_INTERFACE;
    ctrlReq.requestCmd  = USB_DDK_CDC_REQ_SET_LINE_CODING;
    ctrlReq.value       = CpuToLe16(0);
    ctrlReq.index       = 0;
    ctrlReq.data        = (unsigned char *)&acm->lineCoding;
    ctrlReq.length      = sizeof(struct UsbCdcLineCoding);
    ctrlReq.timeout     = USB_CTRL_SET_TIMEOUT;

    ret = UsbRawSendControlRequest(acm->ctrlReq, NULL, &ctrlReq);
    EXPECT_EQ(HDF_ERR_INVALID_PARAM, ret);
}

/**
 * @tc.number    : CheckRawSdkIfSendControlRequest003
 * @tc.name      :
 * @tc.type      : PERFs
 * @tc.level     : Level 1
 */
HWTEST_F(UsbHostSdkIfTest, CheckRawSdkIfSendControlRequest003, TestSize.Level1)
{
    int ret;

    ret = UsbRawSendControlRequest(acm->ctrlReq, acm->devHandle, NULL);
    EXPECT_EQ(HDF_ERR_INVALID_PARAM, ret);
}

/**
 * @tc.number    : CheckRawSdkIfSendControlRequest004
 * @tc.name      :
 * @tc.type      : PERFs
 * @tc.level     : Level 1
 */
HWTEST_F(UsbHostSdkIfTest, CheckRawSdkIfSendControlRequest004, TestSize.Level1)
{
    struct UsbControlRequestData ctrlReq;
    int ret;

    acm->lineCoding.dwDTERate = CpuToLe32(9600);
    acm->lineCoding.bCharFormat = CHARFORMAT;
    acm->lineCoding.bParityType = USB_CDC_NO_PARITY;
    acm->lineCoding.bDataBits = USB_CDC_1_STOP_BITS;

    ctrlReq.requestType = USB_DDK_DIR_OUT | USB_DDK_TYPE_CLASS | USB_DDK_RECIP_INTERFACE;
    ctrlReq.requestCmd  = USB_DDK_CDC_REQ_SET_LINE_CODING;
    ctrlReq.value       = CpuToLe16(0);
    ctrlReq.index       = 2;
    ctrlReq.data        = (unsigned char *)&acm->lineCoding;
    ctrlReq.length      = sizeof(struct UsbCdcLineCoding);
    ctrlReq.timeout     = USB_CTRL_SET_TIMEOUT;

    ret = UsbRawSendControlRequest(acm->ctrlReq, acm->devHandle, &ctrlReq);
    EXPECT_NE(HDF_ERR_IO, ret);
}

/**
 * @tc.number    : CheckRawSdkIfSendControlRequest005
 * @tc.name      :
 * @tc.type      : PERFs
 * @tc.level     : Level 1
 */
HWTEST_F(UsbHostSdkIfTest, CheckRawSdkIfSendControlRequest005, TestSize.Level1)
{
    struct UsbControlRequestData ctrlReq;
    int ret;

    acm->lineCoding.dwDTERate = CpuToLe32(9600);
    acm->lineCoding.bCharFormat = CHARFORMAT;
    acm->lineCoding.bParityType = USB_CDC_NO_PARITY;
    acm->lineCoding.bDataBits = USB_CDC_1_STOP_BITS;

    ctrlReq.requestType = USB_DDK_DIR_OUT | USB_DDK_TYPE_CLASS | USB_DDK_RECIP_INTERFACE;
    ctrlReq.requestCmd  = USB_DDK_CDC_REQ_SET_LINE_CODING;
    ctrlReq.value       = CpuToLe16(0);
    ctrlReq.index       = 0;
    ctrlReq.data        = (unsigned char *)&acm->lineCoding;
    ctrlReq.length      = sizeof(struct UsbCdcLineCoding);
    ctrlReq.timeout     = USB_CTRL_SET_TIMEOUT;

    ret = UsbRawSendControlRequest(NULL, NULL, &ctrlReq);
    EXPECT_EQ(HDF_ERR_INVALID_PARAM, ret);
}

/**
 * @tc.number    : CheckRawSdkIfSendControlRequest006
 * @tc.name      :
 * @tc.type      : PERFs
 * @tc.level     : Level 1
 */
HWTEST_F(UsbHostSdkIfTest, CheckRawSdkIfSendControlRequest006, TestSize.Level1)
{
    int ret;

    ret = UsbRawSendControlRequest(NULL, acm->devHandle, NULL);
    EXPECT_EQ(HDF_ERR_INVALID_PARAM, ret);
}

/**
 * @tc.number    : CheckRawSdkIfSendControlRequest007
 * @tc.name      :
 * @tc.type      : PERFs
 * @tc.level     : Level 1
 */
HWTEST_F(UsbHostSdkIfTest, CheckRawSdkIfSendControlRequest007, TestSize.Level1)
{
    int ret;

    ret = UsbRawSendControlRequest(acm->ctrlReq, NULL, NULL);
    EXPECT_EQ(HDF_ERR_INVALID_PARAM, ret);
}

/**
 * @tc.number    : CheckRawSdkIfSendBulkRequest001
 * @tc.name      :
 * @tc.type      : PERFs
 * @tc.level     : Level 1
 */
HWTEST_F(UsbHostSdkIfTest, CheckRawSdkIfSendBulkRequest001, TestSize.Level1)
{
    struct UsbRequestData reqData;
    int32_t ret;
    int i;
    uint32_t size;
    char sendData[] = {"abcd\0"};

    size = strlen(sendData) + 1;
    size = (size > acm->dataOutEp->maxPacketSize) ? acm->dataOutEp->maxPacketSize : size;

    for (i = 0; i < 1; i++) {
        AcmWb *snd = &acm->wb[i];
        snd->len = size;
        ret = memcpy_s(snd->buf, acm->dataOutEp->maxPacketSize, sendData, size);
        if (ret) {
            printf("memcpy_s fial");
        }

        reqData.endPoint      = acm->dataOutEp->addr;
        reqData.timeout       = USB_CTRL_SET_TIMEOUT;
        reqData.data        = snd->buf;
        reqData.length        = snd->len;
        reqData.requested   = (int *)&size;
    }

    for (i = 0; i < 1; i++) {
        AcmWb *snd = &acm->wb[i];
        printf("UsbRawSendBulkRequest i = [%d]\n", i);
        ret = UsbRawSendBulkRequest(snd->request, acm->devHandle, &reqData);
        EXPECT_EQ(HDF_SUCCESS, ret);
    }
}

/**
 * @tc.number    : CheckRawSdkIfSendBulkRequest002
 * @tc.name      :
 * @tc.type      : PERFs
 * @tc.level     : Level 1
 */
HWTEST_F(UsbHostSdkIfTest, CheckRawSdkIfSendBulkRequest002, TestSize.Level1)
{
    struct UsbRequestData reqData;
    int32_t ret;
    int i;
    int size = acm->dataInEp->maxPacketSize;

    for (i = 0; i < 1; i++) {
        reqData.endPoint      = acm->dataInEp->addr;
        reqData.timeout       = USB_CTRL_SET_TIMEOUT;
        reqData.length        = size;
        reqData.data        = ((UsbRawRequest *)acm->readReq[i])->buffer;
        reqData.requested      = (int *)&size;
    }

    for (i = 0; i < 1; i++) {
        printf("UsbRawSendBulkRequest i = [%d]\n", i);
        ret = UsbRawSendBulkRequest(acm->readReq[i], acm->devHandle, &reqData);
        EXPECT_EQ(HDF_SUCCESS, ret);
    }
}

/**
 * @tc.number    : CheckRawSdkIfSendBulkRequest003
 * @tc.name      :
 * @tc.type      : PERFs
 * @tc.level     : Level 1
 */
HWTEST_F(UsbHostSdkIfTest, CheckRawSdkIfSendBulkRequest003, TestSize.Level1)
{
    struct UsbRequestData reqData;
    int32_t ret;
    int i;
    int size = acm->dataInEp->maxPacketSize;

    for (i = 0; i < 1; i++) {
        reqData.endPoint      = acm->dataInEp->addr;
        reqData.timeout       = USB_CTRL_SET_TIMEOUT;
        reqData.length        = size;
        reqData.data        = ((UsbRawRequest *)acm->readReq[i])->buffer;
        reqData.requested      = (int *)&size;
    }

    for (i = 0; i < 1; i++) {
        printf("UsbRawSendBulkRequest i = [%d]\n", i);
        ret = UsbRawSendBulkRequest(NULL, acm->devHandle, &reqData);
        EXPECT_EQ(HDF_ERR_INVALID_PARAM, ret);
    }
}

/**
 * @tc.number    : CheckRawSdkIfSendBulkRequest004
 * @tc.name      :
 * @tc.type      : PERFs
 * @tc.level     : Level 1
 */
HWTEST_F(UsbHostSdkIfTest, CheckRawSdkIfSendBulkRequest004, TestSize.Level1)
{
    struct UsbRequestData reqData;
    int32_t ret;
    int i;
    int size = acm->dataInEp->maxPacketSize;

    for (i = 0; i < 1; i++) {
        reqData.endPoint      = acm->dataInEp->addr;
        reqData.timeout       = USB_CTRL_SET_TIMEOUT;
        reqData.length        = size;
        reqData.data        = ((UsbRawRequest *)acm->readReq[i])->buffer;
        reqData.requested      = (int *)&size;
    }

    for (i = 0; i < 1; i++) {
        printf("UsbRawSendBulkRequest i = [%d]\n", i);
        ret = UsbRawSendBulkRequest(acm->readReq[i],NULL, &reqData);
        EXPECT_EQ(HDF_ERR_INVALID_PARAM, ret);
    }
}

/**
 * @tc.number    : CheckRawSdkIfSendBulkRequest005
 * @tc.name      :
 * @tc.type      : PERFs
 * @tc.level     : Level 1
 */
HWTEST_F(UsbHostSdkIfTest, CheckRawSdkIfSendBulkRequest005, TestSize.Level1)
{
    int32_t ret;
    int i;

    for (i = 0; i < 1; i++) {
        printf("UsbRawSendBulkRequest i = [%d]\n", i);
        ret = UsbRawSendBulkRequest(acm->readReq[i], acm->devHandle, NULL);
        EXPECT_EQ(HDF_ERR_INVALID_PARAM, ret);
    }
}

/**
 * @tc.number    : CheckRawSdkIfSendInterruptRequest001
 * @tc.name      :
 * @tc.type      : PERFs
 * @tc.level     : Level 1
 */
HWTEST_F(UsbHostSdkIfTest, CheckRawSdkIfSendInterruptRequest001, TestSize.Level1)
{
    struct UsbRequestData reqData;
    int32_t ret;
    int size = acm->notifyEp->maxPacketSize;

    reqData.endPoint = acm->notifyEp->addr;
    reqData.length = size;
    reqData.timeout = USB_CTRL_SET_TIMEOUT;
    reqData.data        = ((UsbRawRequest *)acm->notifyReq)->buffer;
    reqData.requested      = (int *)&size;

    ret = UsbRawSendInterruptRequest(acm->notifyReq, acm->devHandle, &reqData);
    EXPECT_EQ(HDF_SUCCESS, ret);
}

/**
 * @tc.number    : CheckRawSdkIfSendInterruptRequest002
 * @tc.name      :
 * @tc.type      : PERFs
 * @tc.level     : Level 1
 */
HWTEST_F(UsbHostSdkIfTest, CheckRawSdkIfSendInterruptRequest002, TestSize.Level1)
{
    struct UsbRequestData reqData;
    int32_t ret;
    int size = acm->notifyEp->maxPacketSize;

    reqData.endPoint = acm->notifyEp->addr;
    reqData.length = size;
    reqData.timeout = USB_CTRL_SET_TIMEOUT;
    reqData.data        = ((UsbRawRequest *)acm->notifyReq)->buffer;
    reqData.requested      = (int *)&size;

    ret = UsbRawSendInterruptRequest(NULL, acm->devHandle, &reqData);
    EXPECT_EQ(HDF_ERR_INVALID_PARAM, ret);
}

/**
 * @tc.number    : CheckRawSdkIfSendInterruptRequest003
 * @tc.name      :
 * @tc.type      : PERFs
 * @tc.level     : Level 1
 */
HWTEST_F(UsbHostSdkIfTest, CheckRawSdkIfSendInterruptRequest003, TestSize.Level1)
{
    struct UsbRequestData reqData;
    int32_t ret;
    int size = acm->notifyEp->maxPacketSize;

    reqData.endPoint = acm->notifyEp->addr;
    reqData.length = size;
    reqData.timeout = USB_CTRL_SET_TIMEOUT;
    reqData.data        = ((UsbRawRequest *)acm->notifyReq)->buffer;
    reqData.requested      = (int *)&size;

    ret = UsbRawSendInterruptRequest(acm->notifyReq, NULL, &reqData);
    EXPECT_EQ(HDF_ERR_INVALID_PARAM, ret);
}

/**
 * @tc.number    : CheckRawSdkIfSendInterruptRequest004
 * @tc.name      :
 * @tc.type      : PERFs
 * @tc.level     : Level 1
 */
HWTEST_F(UsbHostSdkIfTest, CheckRawSdkIfSendInterruptRequest004, TestSize.Level1)
{
    int32_t ret;

    ret = UsbRawSendInterruptRequest(acm->notifyReq, acm->devHandle, NULL);
    EXPECT_EQ(HDF_ERR_INVALID_PARAM, ret);
}

/**
 * @tc.number    : CheckRawSdkIfFillBulkRequest003
 * @tc.name      :
 * @tc.type      : PERFs
 * @tc.level     : Level 1
 */
HWTEST_F(UsbHostSdkIfTest, CheckRawSdkIfFillBulkRequest003, TestSize.Level1)
{
    struct UsbRawFillRequestData reqData;
    int32_t ret;
    int i;
    uint32_t size;
    char sendData[] = {"abcde\0"};

    size = strlen(sendData) + 1;

    size = (size > acm->dataOutEp->maxPacketSize) ? acm->dataOutEp->maxPacketSize : size;

    for (i = 0; i < ACM_NW; i++) {
        AcmWb *snd = &acm->wb[i];
        snd->len = size;
        ret = memcpy_s(snd->buf, acm->dataOutEp->maxPacketSize, sendData, size);
        if (ret) {
            printf("memcpy_s fial");
        }
        acm->transmitting++;

        reqData.endPoint      = acm->dataOutEp->addr;
        reqData.numIsoPackets = 0;
        reqData.callback      = AcmWriteBulkCallback;
        reqData.userData      = (void *)snd;
        reqData.timeout       = USB_CTRL_SET_TIMEOUT;
        reqData.buffer        = snd->buf;
        reqData.length        = snd->len;

        ret = UsbRawFillBulkRequest(snd->request, acm->devHandle, &reqData);
        EXPECT_EQ(HDF_SUCCESS, ret);
    }
}

/**
 * @tc.number    : CheckRawSdkIfFillBulkRequest004
 * @tc.name      :
 * @tc.type      : PERFs
 * @tc.level     : Level 1
 */
HWTEST_F(UsbHostSdkIfTest, CheckRawSdkIfFillBulkRequest004, TestSize.Level1)
{
    struct UsbRawFillRequestData reqData;
    int32_t ret;
    int i;
    int size = acm->dataInEp->maxPacketSize;

    for (i = 0; i < ACM_NR; i++) {
        reqData.endPoint      = acm->dataInEp->addr;
        reqData.numIsoPackets = 0;
        reqData.callback      = AcmReadBulkCallback;
        reqData.userData      = (void *)acm;
        reqData.timeout       = USB_CTRL_SET_TIMEOUT;
        reqData.length        = size;

        ret = UsbRawFillBulkRequest(acm->readReq[i], acm->devHandle, &reqData);
        EXPECT_EQ(HDF_SUCCESS, ret);
    }
}

/**
 * @tc.number    : CheckRawSdkIfFillInterruptRequest005
 * @tc.name      :
 * @tc.type      : PERFs
 * @tc.level     : Level 1
 */
HWTEST_F(UsbHostSdkIfTest, CheckRawSdkIfFillInterruptRequest005, TestSize.Level1)
{
    struct UsbRawFillRequestData fillRequestData;
    int32_t ret;
    int size = acm->notifyEp->maxPacketSize;

    fillRequestData.endPoint = acm->notifyEp->addr;
    fillRequestData.length = size;
    fillRequestData.numIsoPackets = 0;
    fillRequestData.callback = AcmNotifyReqCallback;
    fillRequestData.userData = (void *)acm;
    fillRequestData.timeout = USB_CTRL_SET_TIMEOUT;

    ret = UsbRawFillInterruptRequest(acm->notifyReq, acm->devHandle, &fillRequestData);
    EXPECT_EQ(HDF_SUCCESS, ret);
}


/**
 * @tc.number    : CheckRawSdkIfSubmitRequest001
 * @tc.name      :
 * @tc.type      : PERFs
 * @tc.level     : Level 1
 */
HWTEST_F(UsbHostSdkIfTest, CheckRawSdkIfSubmitRequest001, TestSize.Level1)
{
    int32_t ret;
    int i;

    for (i = 0; i < 1; i++) {
        AcmWb *snd = &acm->wb[i];
        printf("UsbRawSubmitRequest i = [%d]\n", i);
        ret = UsbRawSubmitRequest(snd->request);
        EXPECT_EQ(HDF_SUCCESS, ret);
    }
}

/**
 * @tc.number    : CheckRawSdkIfSubmitRequest002
 * @tc.name      :
 * @tc.type      : PERFs
 * @tc.level     : Level 1
 */
HWTEST_F(UsbHostSdkIfTest, CheckRawSdkIfSubmitRequest002, TestSize.Level1)
{
    int32_t ret;
    int i;

    for (i = 0; i < 1; i++) {
        printf("UsbRawSubmitRequest i = [%d]\n", i);
        ret = UsbRawSubmitRequest(acm->readReq[i]);
        EXPECT_EQ(HDF_SUCCESS, ret);
    }
}

/**
 * @tc.number    : CheckRawSdkIfSubmitRequest003
 * @tc.name      :
 * @tc.type      : PERFs
 * @tc.level     : Level 1
 */
HWTEST_F(UsbHostSdkIfTest, CheckRawSdkIfSubmitRequest003, TestSize.Level1)
{
    int32_t ret;

    ret = UsbRawSubmitRequest(acm->notifyReq);
    EXPECT_EQ(HDF_SUCCESS, ret);
}

/**
 * @tc.number    : CheckRawSdkIfSubmitRequest004
 * @tc.name      :
 * @tc.type      : PERFs
 * @tc.level     : Level 1
 */
HWTEST_F(UsbHostSdkIfTest, CheckRawSdkIfSubmitRequest004, TestSize.Level1)
{
    int32_t ret;

    ret = UsbRawSubmitRequest(NULL);
    EXPECT_EQ(HDF_ERR_INVALID_PARAM, ret);
}

/**
 * @tc.number    : CheckRawSdkIfCancelRequest001
 * @tc.name      :
 * @tc.type      : PERFs
 * @tc.level     : Level 1
 */
HWTEST_F(UsbHostSdkIfTest, CheckRawSdkIfCancelRequest001, TestSize.Level1)
{
    int32_t ret;
    int i;

    for (i = 0; i < ACM_NW; i++) {
        AcmWb *snd = &acm->wb[i];
        ret = UsbRawCancelRequest(snd->request);
        EXPECT_EQ(HDF_SUCCESS, ret);
    }
}

/**
 * @tc.number    : CheckRawSdkIfCancelRequest002
 * @tc.name      :
 * @tc.type      : PERFs
 * @tc.level     : Level 1
 */
HWTEST_F(UsbHostSdkIfTest, CheckRawSdkIfCancelRequest002, TestSize.Level1)
{
    int32_t ret;
    int i;

    for (i = 0; i < 1; i++) {
        ret = UsbRawCancelRequest(acm->readReq[i]);
        EXPECT_EQ(HDF_SUCCESS, ret);
    }
}

/**
 * @tc.number    : CheckRawSdkIfCancelRequest003
 * @tc.name      :
 * @tc.type      : PERFs
 * @tc.level     : Level 1
 */
HWTEST_F(UsbHostSdkIfTest, CheckRawSdkIfCancelRequest003, TestSize.Level1)
{
    int32_t ret;

    ret = UsbRawCancelRequest(acm->notifyReq);
    EXPECT_EQ(HDF_SUCCESS, ret);
}

/**
 * @tc.number    : CheckRawSdkIfCancelRequest004
 * @tc.name      :
 * @tc.type      : PERFs
 * @tc.level     : Level 1
 */
HWTEST_F(UsbHostSdkIfTest, CheckRawSdkIfCancelRequest004, TestSize.Level1)
{
    int32_t ret;

    ret = UsbRawCancelRequest(NULL);
    EXPECT_EQ(HDF_ERR_INVALID_PARAM, ret);
}

}
