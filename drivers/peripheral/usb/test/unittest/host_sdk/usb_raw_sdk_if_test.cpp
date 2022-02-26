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

static struct UsbSession *session = NULL;
static struct AcmDevice *acm = NULL;
static struct AcmDevice deviceService;
static UsbRawHandle *devHandle = NULL;
static UsbRawDevice *dev = NULL;
static int activeConfig;
static bool g_stopIoThreadFlag = false;

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

static void AcmProcessNotification(struct AcmDevice *acm, unsigned char *buf)
{
    struct UsbCdcNotification *dr = (struct UsbCdcNotification *)buf;

    printf("%s:%d entry!", __func__, __LINE__);

    switch (dr->bNotificationType) {
        case USB_DDK_CDC_NOTIFY_NETWORK_CONNECTION:
            printf("%s - network connection: %d\n", __func__, dr->wValue);
            break;
        case USB_DDK_CDC_NOTIFY_SERIAL_STATE:
            printf("the serial State change\n");
            break;
        default:
            printf("%s-%d received: index %d len %d\n", __func__, dr->bNotificationType, dr->wIndex, dr->wLength);
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
    unsigned int expectedSize, copySize, allocSize;
    int ret;

    printf("Irqstatus:%d,actualLength:%u\n", req->status, currentSize);

    if (req->status != USB_REQUEST_COMPLETED) {
        goto exit;
    }

    if (acm->nbIndex) {
        dr = (struct UsbCdcNotification *)acm->notificationBuffer;
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
                goto exit;
            }
            acm->nbSize = allocSize;
        }
        copySize = MIN(currentSize, expectedSize - acm->nbIndex);
        ret = memcpy_s(&acm->notificationBuffer[acm->nbIndex], acm->nbSize - acm->nbIndex,
            req->buffer, copySize);
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

    if (UsbRawSubmitRequest(req)) {
        printf("%s - UsbRawSubmitRequest failed", __func__);
    }

exit:
    printf("%s:%d exit", __func__, __LINE__);

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

static void AcmCtrlReqCallback(const void *requestArg)
{
    printf("%s:%d entry!", __func__, __LINE__);
}

static int UsbParseConfigDescriptor(struct AcmDevice *acm, struct UsbRawConfigDescriptor *config)
{
    uint8_t numInterfaces;
    uint8_t i;
    uint8_t j;
    int ret;
    uint8_t ifaceClass;
    uint8_t numEndpoints;
    const struct UsbRawInterface *interface = NULL;

    numInterfaces = config->configDescriptor.bNumInterfaces;
    printf("------numInterfaces = [%d]------\n", numInterfaces);
    for (i = 0; i < numInterfaces; i++) {
        interface = config->interface[i];
        ifaceClass = interface->altsetting->interfaceDescriptor.bInterfaceClass;
        numEndpoints = interface->altsetting->interfaceDescriptor.bNumEndpoints;

        ret = UsbRawClaimInterface(acm->devHandle, i);
        if (ret) {
            printf("%s:%d claim interface %u failed\n", __func__, __LINE__, i);
            continue;
        }

        switch (ifaceClass) {
            case USB_DDK_CLASS_COMM:
                acm->ctrlIface = i;
                acm->notifyEp = (UsbEndpoint *)OsalMemAlloc(sizeof(struct UsbEndpoint));
                if (acm->notifyEp == NULL) {
                    printf("%s:%d allocate endpoint failed\n", __func__, __LINE__);
                }
                /* get the first endpoint by default */
                acm->notifyEp->addr = interface->altsetting->endPoint[0].endpointDescriptor.bEndpointAddress;
                acm->notifyEp->interval = interface->altsetting->endPoint[0].endpointDescriptor.bInterval;
                acm->notifyEp->maxPacketSize = interface->altsetting->endPoint[0].endpointDescriptor.wMaxPacketSize;
                break;
            case USB_DDK_CLASS_CDC_DATA:
                acm->dataIface = i;
                for (j = 0; j < numEndpoints; j++) {
                    const struct UsbRawEndpointDescriptor *endPoint = &interface->altsetting->endPoint[j];

                    /* get bulk in endpoint */
                    if ((endPoint->endpointDescriptor.bEndpointAddress & USB_DDK_ENDPOINT_DIR_MASK) == USB_DDK_DIR_IN) {
                        acm->dataInEp = (UsbEndpoint *)OsalMemAlloc(sizeof(struct UsbEndpoint));
                        if (acm->dataInEp == NULL) {
                            printf("%s:%d allocate dataInEp failed\n", __func__, __LINE__);
                            break;
                        }
                        acm->dataInEp->addr = endPoint->endpointDescriptor.bEndpointAddress;
                        acm->dataInEp->interval = endPoint->endpointDescriptor.bInterval;
                        acm->dataInEp->maxPacketSize = endPoint->endpointDescriptor.wMaxPacketSize;
                    } else { /* get bulk out endpoint */
                        acm->dataOutEp = (UsbEndpoint *)OsalMemAlloc(sizeof(struct UsbEndpoint));
                        if (acm->dataOutEp == NULL) {
                            printf("%s:%d allocate dataOutEp failed\n", __func__, __LINE__);
                            break;
                        }
                        acm->dataOutEp->addr = endPoint->endpointDescriptor.bEndpointAddress;
                        acm->dataOutEp->interval = endPoint->endpointDescriptor.bInterval;
                        acm->dataOutEp->maxPacketSize = endPoint->endpointDescriptor.wMaxPacketSize;
                    }
                }
                break;
            default:
                printf("%s:%d wrong descriptor type\n", __func__, __LINE__);
                break;
        }
    }

    return HDF_SUCCESS;
}

/**
 * @tc.number    : CheckRawSdkIfInit001
 * @tc.name      :
 * @tc.type      : PERF
 * @tc.level     : Level 1
 */
HWTEST_F(UsbHostSdkIfTest, CheckRawSdkIfInit001, TestSize.Level1)
{
    int32_t ret;

    ret = UsbRawInit(&session);
    EXPECT_EQ(HDF_SUCCESS, ret);
    acm->session = session;
}

/**
 * @tc.number    : CheckRawSdkIfExit001
 * @tc.name      :
 * @tc.type      : PERF
 * @tc.level     : Level 1
 */
HWTEST_F(UsbHostSdkIfTest, CheckRawSdkIfExit001, TestSize.Level1)
{
    int32_t ret;

    ret = UsbRawExit(acm->session);
    EXPECT_EQ(HDF_SUCCESS, ret);
}

/**
 * @tc.number    : CheckRawSdkIfInit002
 * @tc.name      :
 * @tc.type      : PERF
 * @tc.level     : Level 1
 */
HWTEST_F(UsbHostSdkIfTest, CheckRawSdkIfInit002, TestSize.Level1)
{
    int32_t ret;

    ret = UsbRawInit(NULL);
    EXPECT_EQ(HDF_SUCCESS, ret);
    acm->session = session;
}

/**
 * @tc.number    : CheckRawSdkIfExit002
 * @tc.name      :
 * @tc.type      : PERF
 * @tc.level     : Level 1
 */
HWTEST_F(UsbHostSdkIfTest, CheckRawSdkIfExit002, TestSize.Level1)
{
    int32_t ret;

    ret = UsbRawExit(NULL);
    EXPECT_EQ(HDF_SUCCESS, ret);
}

/**
 * @tc.number    : CheckRawSdkIfInit003
 * @tc.name      :
 * @tc.type      : PERF
 * @tc.level     : Level 1
 */
HWTEST_F(UsbHostSdkIfTest, CheckRawSdkIfInit003, TestSize.Level1)
{
    int ret;
    int i;

    for (i = 0; i < 100; i++)
    {
        ret = UsbRawInit(&session);
        EXPECT_EQ(HDF_SUCCESS, ret);
        acm->session = session;
        ret = UsbRawExit(acm->session);
        EXPECT_EQ(HDF_SUCCESS, ret);
    }
}

/**
 * @tc.number    : CheckRawSdkIfInit004
 * @tc.name      :
 * @tc.type      : PERF
 * @tc.level     : Level 1
 */
HWTEST_F(UsbHostSdkIfTest, CheckRawSdkIfInit004, TestSize.Level1)
{
    int ret;
    int i;

    for (i = 0; i < 100; i++)
    {
        ret = UsbRawInit(NULL);
        EXPECT_EQ(HDF_SUCCESS, ret);
        ret = UsbRawExit(NULL);
        EXPECT_EQ(HDF_SUCCESS, ret);
    }
}

/**
 * @tc.number    : CheckRawSdkIfInit005
 * @tc.name      :
 * @tc.type      : PERF
 * @tc.level     : Level 1
 */
HWTEST_F(UsbHostSdkIfTest, CheckRawSdkIfInit005, TestSize.Level1)
{
    int32_t ret;

    ret = UsbRawInit(&session);
    EXPECT_EQ(HDF_SUCCESS, ret);
    acm->session = session;
}

/**
 * @tc.number    : CheckRawSdkIfOpenDevice001
 * @tc.name      :
 * @tc.type      : PERF
 * @tc.level     : Level 1
 */
HWTEST_F(UsbHostSdkIfTest, CheckRawSdkIfOpenDevice001, TestSize.Level1)
{
    acm->busNum = 1U;
    acm->devAddr = 2U;

    devHandle = UsbRawOpenDevice(NULL, acm->busNum, acm->devAddr);
    EXPECT_EQ(nullptr,  devHandle);
    acm->devHandle = devHandle;
}

/**
 * @tc.number    : CheckRawSdkIfOpenDevice002
 * @tc.name      :
 * @tc.type      : PERF
 * @tc.level     : Level 1
 */
HWTEST_F(UsbHostSdkIfTest, CheckRawSdkIfOpenDevice002, TestSize.Level1)
{
    acm->busNum = 1U;
    acm->devAddr = 100U;

    devHandle = UsbRawOpenDevice(acm->session, acm->busNum, acm->devAddr);
    EXPECT_EQ(nullptr,  devHandle);
    acm->devHandle = devHandle;
}

/**
 * @tc.number    : CheckRawSdkIfOpenDevice003
 * @tc.name      :
 * @tc.type      : PERF
 * @tc.level     : Level 1
 */
HWTEST_F(UsbHostSdkIfTest, CheckRawSdkIfOpenDevice003, TestSize.Level1)
{
    acm->busNum = 1U;
    acm->devAddr = 255U;

    devHandle = UsbRawOpenDevice(acm->session, acm->busNum, acm->devAddr);
    EXPECT_EQ(nullptr,  devHandle);
    acm->devHandle = devHandle;
}

/**
 * @tc.number    : CheckRawSdkIfOpenDevice004
 * @tc.name      :
 * @tc.type      : PERF
 * @tc.level     : Level 1
 */
HWTEST_F(UsbHostSdkIfTest, CheckRawSdkIfOpenDevice004, TestSize.Level1)
{
    acm->busNum = 100U;
    acm->devAddr = 2U;

    devHandle = UsbRawOpenDevice(acm->session, acm->busNum, acm->devAddr);
    EXPECT_EQ(nullptr,  devHandle);
    acm->devHandle = devHandle;
}

/**
 * @tc.number    : CheckRawSdkIfOpenDevice005
 * @tc.name      :
 * @tc.type      : PERF
 * @tc.level     : Level 1
 */
HWTEST_F(UsbHostSdkIfTest, CheckRawSdkIfOpenDevice005, TestSize.Level1)
{
    acm->busNum = 255U;
    acm->devAddr = 2U;

    devHandle = UsbRawOpenDevice(acm->session, acm->busNum, acm->devAddr);
    EXPECT_EQ(nullptr,  devHandle);
    acm->devHandle = devHandle;
}

/**
 * @tc.number    : CheckRawSdkIfOpenDevice006
 * @tc.name      :
 * @tc.type      : PERF
 * @tc.level     : Level 1
 */
HWTEST_F(UsbHostSdkIfTest, CheckRawSdkIfOpenDevice006, TestSize.Level1)
{
    acm->busNum = 1U;
    acm->devAddr = 2U;

    devHandle = UsbRawOpenDevice(acm->session, acm->busNum, acm->devAddr);
    EXPECT_NE(nullptr,  devHandle);
    acm->devHandle = devHandle;
}

/**
 * @tc.number    : CheckRawSdkIfResetDevice001
 * @tc.name      :
 * @tc.type      : PERF
 * @tc.level     : Level 1
 */
HWTEST_F(UsbHostSdkIfTest, CheckRawSdkIfResetDevice001, TestSize.Level1)
{
    int32_t ret;

    ret = UsbRawResetDevice(NULL);
    EXPECT_EQ(HDF_ERR_INVALID_PARAM, ret);
}

/**
 * @tc.number    : CheckRawSdkIfResetDevice002
 * @tc.name      :
 * @tc.type      : PERF
 * @tc.level     : Level 1
 */
HWTEST_F(UsbHostSdkIfTest, CheckRawSdkIfResetDevice002, TestSize.Level1)
{
    int32_t ret;

    ret = UsbRawResetDevice(acm->devHandle);
    EXPECT_EQ(HDF_SUCCESS, ret);
}

/**
 * @tc.number    : CheckRawSdkIfCloseDevice001
 * @tc.name      :
 * @tc.type      : PERF
 * @tc.level     : Level 1
 */
HWTEST_F(UsbHostSdkIfTest, CheckRawSdkIfCloseDevice001, TestSize.Level1)
{
    int32_t ret;

    ret = UsbRawCloseDevice(NULL);
    EXPECT_EQ(HDF_ERR_INVALID_PARAM, ret);
}

/**
 * @tc.number    : CheckRawSdkIfCloseDevice002
 * @tc.name      :
 * @tc.type      : PERF
 * @tc.level     : Level 1
 */
HWTEST_F(UsbHostSdkIfTest, CheckRawSdkIfCloseDevice002, TestSize.Level1)
{
    int32_t ret;

    ret = UsbRawCloseDevice(acm->devHandle);
    EXPECT_EQ(HDF_SUCCESS, ret);
}

/**
 * @tc.number    : CheckRawSdkIfOpenDevice007
 * @tc.name      :
 * @tc.type      : PERF
 * @tc.level     : Level 1
 */
HWTEST_F(UsbHostSdkIfTest, CheckRawSdkIfOpenDevice007, TestSize.Level1)
{
    acm->busNum = 1U;
    acm->devAddr = 2U;

    devHandle = UsbRawOpenDevice(session, acm->busNum, acm->devAddr);
    EXPECT_NE(nullptr,  devHandle);
    acm->devHandle = devHandle;
}

/**
 * @tc.number    : CheckRawSdkIfGetConfiguration001
 * @tc.name      :
 * @tc.type      : PERF
 * @tc.level     : Level 1
 */
HWTEST_F(UsbHostSdkIfTest, CheckRawSdkIfGetConfiguration001, TestSize.Level1)
{
    int32_t ret;

    ret = UsbRawGetConfiguration(NULL, &activeConfig);
    EXPECT_EQ(HDF_ERR_INVALID_PARAM, ret);
}

/**
 * @tc.number    : CheckRawSdkIfGetConfiguration002
 * @tc.name      :
 * @tc.type      : PERF
 * @tc.level     : Level 1
 */
HWTEST_F(UsbHostSdkIfTest, CheckRawSdkIfGetConfiguration002, TestSize.Level1)
{
    int32_t ret;

    ret = UsbRawGetConfiguration(acm->devHandle, NULL);
    EXPECT_EQ(HDF_ERR_INVALID_PARAM, ret);
}

/**
 * @tc.number    : CheckRawSdkIfGetConfiguration003
 * @tc.name      :
 * @tc.type      : PERF
 * @tc.level     : Level 1
 */
HWTEST_F(UsbHostSdkIfTest, CheckRawSdkIfGetConfiguration003, TestSize.Level1)
{
    int32_t ret;

    ret = UsbRawGetConfiguration(NULL, NULL);
    EXPECT_EQ(HDF_ERR_INVALID_PARAM, ret);
}

/**
 * @tc.number    : CheckRawSdkIfGetConfiguration004
 * @tc.name      :
 * @tc.type      : PERF
 * @tc.level     : Level 1
 */
HWTEST_F(UsbHostSdkIfTest, CheckRawSdkIfGetConfiguration004, TestSize.Level1)
{
    int32_t ret;

    ret = UsbRawGetConfiguration(acm->devHandle, &activeConfig);
    printf("------activeConfig = [%d]------\n", activeConfig);
    EXPECT_EQ(HDF_SUCCESS, ret);
}


/**
 * @tc.number    : CheckRawSdkIfGetDevice001
 * @tc.name      :
 * @tc.type      : PERF
 * @tc.level     : Level 1
 */
HWTEST_F(UsbHostSdkIfTest, CheckRawSdkIfGetDevice001, TestSize.Level1)
{
    dev = UsbRawGetDevice(NULL);
    EXPECT_EQ(nullptr,  dev);
}

/**
 * @tc.number    : CheckRawSdkIfGetDevice002
 * @tc.name      :
 * @tc.type      : PERF
 * @tc.level     : Level 1
 */
HWTEST_F(UsbHostSdkIfTest, CheckRawSdkIfGetDevice002, TestSize.Level1)
{
    dev = UsbRawGetDevice(acm->devHandle);
    EXPECT_NE(nullptr,  dev);
}

/**
 * @tc.number    : CheckRawSdkIfGetConfigDescriptor001
 * @tc.name      :
 * @tc.type      : PERF
 * @tc.level     : Level 1
 */
HWTEST_F(UsbHostSdkIfTest, CheckRawSdkIfGetConfigDescriptor001, TestSize.Level1)
{
    int32_t ret;

    ret = UsbRawGetConfigDescriptor(NULL, activeConfig, &acm->config);
    EXPECT_EQ(HDF_ERR_INVALID_PARAM, ret);
}

/**
 * @tc.number    : CheckRawSdkIfGetConfigDescriptor002
 * @tc.name      :
 * @tc.type      : PERF
 * @tc.level     : Level 1
 */
HWTEST_F(UsbHostSdkIfTest, CheckRawSdkIfGetConfigDescriptor002, TestSize.Level1)
{
    int32_t ret;

    ret = UsbRawGetConfigDescriptor(dev, activeConfig, NULL);
    EXPECT_EQ(HDF_ERR_INVALID_PARAM, ret);
}

/**
 * @tc.number    : CheckRawSdkIfGetConfigDescriptor003
 * @tc.name      :
 * @tc.type      : PERF
 * @tc.level     : Level 1
 */
HWTEST_F(UsbHostSdkIfTest, CheckRawSdkIfGetConfigDescriptor003, TestSize.Level1)
{
    int32_t ret;

    ret = UsbRawGetConfigDescriptor(NULL, activeConfig, NULL);
    EXPECT_EQ(HDF_ERR_INVALID_PARAM, ret);
}

/**
 * @tc.number    : CheckRawSdkIfGetConfigDescriptor004
 * @tc.name      :
 * @tc.type      : PERF
 * @tc.level     : Level 1
 */
HWTEST_F(UsbHostSdkIfTest, CheckRawSdkIfGetConfigDescriptor004, TestSize.Level1)
{
    int32_t ret;

    printf("------activeConfig = [%d]------\n", activeConfig);
    ret = UsbRawGetConfigDescriptor(dev, activeConfig, &acm->config);
    EXPECT_EQ(HDF_SUCCESS, ret);
}

/**
 * @tc.number    : CheckRawSdkIfSetConfiguration001
 * @tc.name      :
 * @tc.type      : PERF
 * @tc.level     : Level 1
 */
HWTEST_F(UsbHostSdkIfTest, CheckRawSdkIfSetConfiguration001, TestSize.Level1)
{
    int32_t ret;
    int config = 0;

    ret = UsbRawSetConfiguration(NULL, config);
    EXPECT_EQ(HDF_ERR_INVALID_PARAM, ret);
}

/**
 * @tc.number    : CheckRawSdkIfSetConfiguration002
 * @tc.name      :
 * @tc.type      : PERF
 * @tc.level     : Level 1
 */
HWTEST_F(UsbHostSdkIfTest, CheckRawSdkIfSetConfiguration002, TestSize.Level1)
{
    int32_t ret;
    int config = 0;

    ret = UsbRawSetConfiguration(acm->devHandle, config);
    EXPECT_EQ(HDF_SUCCESS, ret);
}

/**
 * @tc.number    : CheckRawSdkIfSetConfiguration003
 * @tc.name      :
 * @tc.type      : PERF
 * @tc.level     : Level 1
 */
HWTEST_F(UsbHostSdkIfTest, CheckRawSdkIfSetConfiguration003, TestSize.Level1)
{
    int32_t ret;
    int config = 1;

    ret = UsbRawSetConfiguration(acm->devHandle, config);
    EXPECT_EQ(HDF_SUCCESS, ret);
}

/**
 * @tc.number    : CheckRawSdkIfSetConfiguration004
 * @tc.name      :
 * @tc.type      : PERF
 * @tc.level     : Level 1
 */
HWTEST_F(UsbHostSdkIfTest, CheckRawSdkIfSetConfiguration004, TestSize.Level1)
{
    int32_t ret;
    int config = 10;

    ret = UsbRawSetConfiguration(acm->devHandle, config);
    EXPECT_EQ(HDF_FAILURE, ret);
}

/**
 * @tc.number    : CheckRawSdkIfSetConfiguration005
 * @tc.name      :
 * @tc.type      : PERF
 * @tc.level     : Level 1
 */
HWTEST_F(UsbHostSdkIfTest, CheckRawSdkIfSetConfiguration005, TestSize.Level1)
{
    int32_t ret;
    int config = 100;

    ret = UsbRawSetConfiguration(acm->devHandle, config);
    EXPECT_EQ(HDF_FAILURE, ret);
}

/**
 * @tc.number    : CheckRawSdkIfSetConfiguration006
 * @tc.name      :
 * @tc.type      : PERF
 * @tc.level     : Level 1
 */
HWTEST_F(UsbHostSdkIfTest, CheckRawSdkIfSetConfiguration006, TestSize.Level1)
{
    int32_t ret;
    int config = 200;

    ret = UsbRawSetConfiguration(acm->devHandle, config);
    EXPECT_EQ(HDF_FAILURE, ret);
}

/**
 * @tc.number    : CheckRawSdkIfSetConfiguration007
 * @tc.name      :
 * @tc.type      : PERF
 * @tc.level     : Level 1
 */
HWTEST_F(UsbHostSdkIfTest, CheckRawSdkIfSetConfiguration007, TestSize.Level1)
{
    int32_t ret;
    int config = 255;

    ret = UsbRawSetConfiguration(acm->devHandle, config);
    EXPECT_EQ(HDF_FAILURE, ret);
}

/**
 * @tc.number    : CheckRawSdkIfSetConfiguration008
 * @tc.name      :
 * @tc.type      : PERF
 * @tc.level     : Level 1
 */
HWTEST_F(UsbHostSdkIfTest, CheckRawSdkIfSetConfiguration008, TestSize.Level1)
{
    int32_t ret;
    int config = 1;

    ret = UsbRawSetConfiguration(acm->devHandle, config);
    EXPECT_EQ(HDF_SUCCESS, ret);
}

/**
 * @tc.number    : CheckRawSdkIfGetDeviceDescriptor001
 * @tc.name      :
 * @tc.type      : PERF
 * @tc.level     : Level 1
 */
HWTEST_F(UsbHostSdkIfTest, CheckRawSdkIfGetDeviceDescriptor001, TestSize.Level1)
{
    struct UsbDeviceDescriptor desc;
    int ret;

    ret = UsbRawGetDeviceDescriptor(NULL, &desc);
    EXPECT_EQ(HDF_ERR_INVALID_PARAM, ret);
}

/**
 * @tc.number    : CheckRawSdkIfGetDeviceDescriptor002
 * @tc.name      :
 * @tc.type      : PERF
 * @tc.level     : Level 1
 */
HWTEST_F(UsbHostSdkIfTest, CheckRawSdkIfGetDeviceDescriptor002, TestSize.Level1)
{
    int ret;

    ret = UsbRawGetDeviceDescriptor(dev, NULL);
    EXPECT_EQ(HDF_ERR_INVALID_PARAM, ret);
}

/**
 * @tc.number    : CheckRawSdkIfGetDeviceDescriptor003
 * @tc.name      :
 * @tc.type      : PERF
 * @tc.level     : Level 1
 */
HWTEST_F(UsbHostSdkIfTest, CheckRawSdkIfGetDeviceDescriptor003, TestSize.Level1)
{
    int ret;

    ret = UsbRawGetDeviceDescriptor(NULL, NULL);
    EXPECT_EQ(HDF_ERR_INVALID_PARAM, ret);
}

/**
 * @tc.number    : CheckRawSdkIfGetDeviceDescriptor004
 * @tc.name      :
 * @tc.type      : PERF
 * @tc.level     : Level 1
 */
HWTEST_F(UsbHostSdkIfTest, CheckRawSdkIfGetDeviceDescriptor004, TestSize.Level1)
{
    struct UsbDeviceDescriptor desc;
    int ret;

    ret = UsbRawGetDeviceDescriptor(dev, &desc);
    EXPECT_EQ(HDF_SUCCESS, ret);
}

/**
 * @tc.number    : CheckRawSdkIfGetConfigDescriptor005
 * @tc.name      :
 * @tc.type      : PERF
 * @tc.level     : Level 1
 */
HWTEST_F(UsbHostSdkIfTest, CheckRawSdkIfGetConfigDescriptor005, TestSize.Level1)
{
    int32_t ret;

    printf("------activeConfig = [%d]------\n", activeConfig);
    ret = UsbRawGetConfigDescriptor(dev, activeConfig, &acm->config);
    EXPECT_EQ(HDF_SUCCESS, ret);
}

/**
 * @tc.number    : CheckRawSdkIfGetDeviceDescriptor005
 * @tc.name      :
 * @tc.type      : PERF
 * @tc.level     : Level 1
 */
HWTEST_F(UsbHostSdkIfTest, CheckRawSdkIfGetDeviceDescriptor005, TestSize.Level1)
{
    struct UsbDeviceDescriptor desc;
    int ret;

    ret = UsbRawGetDeviceDescriptor(dev, &desc);
    EXPECT_EQ(HDF_SUCCESS, ret);
}

/**
 * @tc.number    : CheckRawSdkIfClaimInterface001
 * @tc.name      :
 * @tc.type      : PERFs
 * @tc.level     : Level 1
 */
HWTEST_F(UsbHostSdkIfTest, CheckRawSdkIfClaimInterface001, TestSize.Level1)
{
    int ret;
    int interfaceNumber = 1;

    ret = UsbRawClaimInterface(NULL, interfaceNumber);
    EXPECT_EQ(HDF_ERR_INVALID_PARAM, ret);
}

/**
 * @tc.number    : CheckRawSdkIfClaimInterface002
 * @tc.name      :
 * @tc.type      : PERFs
 * @tc.level     : Level 1
 */
HWTEST_F(UsbHostSdkIfTest, CheckRawSdkIfClaimInterface002, TestSize.Level1)
{
    int ret;
    int interfaceNumber = 1;

    ret = UsbRawClaimInterface(devHandle, interfaceNumber);
    EXPECT_EQ(HDF_SUCCESS, ret);
}

/**
 * @tc.number    : CheckRawSdkIfClaimInterface003
 * @tc.name      :
 * @tc.type      : PERFs
 * @tc.level     : Level 1
 */
HWTEST_F(UsbHostSdkIfTest, CheckRawSdkIfClaimInterface003, TestSize.Level1)
{
    int ret;
    int interfaceNumber = 0;

    ret = UsbRawClaimInterface(devHandle, interfaceNumber);
    EXPECT_EQ(HDF_SUCCESS, ret);
}

/**
 * @tc.number    : CheckRawSdkIfClaimInterface004
 * @tc.name      :
 * @tc.type      : PERFs
 * @tc.level     : Level 1
 */
HWTEST_F(UsbHostSdkIfTest, CheckRawSdkIfClaimInterface004, TestSize.Level1)
{
    int ret;
    int interfaceNumber = 255;

    ret = UsbRawClaimInterface(devHandle, interfaceNumber);
    EXPECT_EQ(HDF_ERR_INVALID_PARAM, ret);
}

/**
 * @tc.number    : CheckRawSdkIfClaimInterface005
 * @tc.name      :
 * @tc.type      : PERFs
 * @tc.level     : Level 1
 */
HWTEST_F(UsbHostSdkIfTest, CheckRawSdkIfClaimInterface005, TestSize.Level1)
{
    int ret;

    ret = UsbParseConfigDescriptor(acm, acm->config);
    EXPECT_EQ(HDF_SUCCESS, ret);
}

/**
 * @tc.number    : CheckRawSdkIfReleaseInterface001
 * @tc.name      :
 * @tc.type      : PERFs
 * @tc.level     : Level 1
 */
HWTEST_F(UsbHostSdkIfTest, CheckRawSdkIfReleaseInterface001, TestSize.Level1)
{
    int ret;

    ret = UsbRawReleaseInterface(NULL, acm->ctrlIface);
    EXPECT_EQ(HDF_ERR_INVALID_PARAM, ret);
}

/**
 * @tc.number    : CheckRawSdkIfReleaseInterface002
 * @tc.name      :
 * @tc.type      : PERFs
 * @tc.level     : Level 1
 */
HWTEST_F(UsbHostSdkIfTest, CheckRawSdkIfReleaseInterface002, TestSize.Level1)
{
    int ret;

    ret = UsbRawReleaseInterface(acm->devHandle, acm->ctrlIface);
    EXPECT_EQ(HDF_SUCCESS, ret);
}

/**
 * @tc.number    : CheckRawSdkIfReleaseInterface003
 * @tc.name      :
 * @tc.type      : PERFs
 * @tc.level     : Level 1
 */
HWTEST_F(UsbHostSdkIfTest, CheckRawSdkIfReleaseInterface003, TestSize.Level1)
{
    int ret;

    ret = UsbRawReleaseInterface(NULL, acm->dataIface);
    EXPECT_EQ(HDF_ERR_INVALID_PARAM, ret);
}

/**
 * @tc.number    : CheckRawSdkIfReleaseInterface004
 * @tc.name      :
 * @tc.type      : PERFs
 * @tc.level     : Level 1
 */
HWTEST_F(UsbHostSdkIfTest, CheckRawSdkIfReleaseInterface004, TestSize.Level1)
{
    int ret;

    ret = UsbRawReleaseInterface(acm->devHandle, acm->dataIface);
    EXPECT_EQ(HDF_SUCCESS, ret);
}

/**
 * @tc.number    : CheckRawSdkIfClaimInterface006
 * @tc.name      :
 * @tc.type      : PERFs
 * @tc.level     : Level 1
 */
HWTEST_F(UsbHostSdkIfTest, CheckRawSdkIfClaimInterface006, TestSize.Level1)
{
    int ret;

    ret = UsbParseConfigDescriptor(acm, acm->config);
    EXPECT_EQ(HDF_SUCCESS, ret);
}

/**
 * @tc.number    : CheckRawSdkIfAllocRequest001
 * @tc.name      :
 * @tc.type      : PERFs
 * @tc.level     : Level 1
 */
HWTEST_F(UsbHostSdkIfTest, CheckRawSdkIfAllocRequest001, TestSize.Level1)
{
    int i;
    int ret;

    ret = AcmWriteBufAlloc(acm);
    EXPECT_EQ(HDF_SUCCESS, ret);

    for (i = 0; i < ACM_NW; i++) {
        acm->wb[i].request = UsbRawAllocRequest(NULL, 0, acm->dataOutEp->maxPacketSize);
        acm->wb[i].instance = acm;
        EXPECT_EQ(nullptr,  acm->wb[i].request);
    }
}

/**
 * @tc.number    : CheckRawSdkIfAllocRequest002
 * @tc.name      :
 * @tc.type      : PERFs
 * @tc.level     : Level 1
 */
HWTEST_F(UsbHostSdkIfTest, CheckRawSdkIfAllocRequest002, TestSize.Level1)
{
    int i;
    int ret;

    ret = AcmWriteBufAlloc(acm);
    EXPECT_EQ(HDF_SUCCESS, ret);

    for (i = 0; i < ACM_NW; i++) {
        acm->wb[i].request = UsbRawAllocRequest(acm->devHandle, 0, acm->dataOutEp->maxPacketSize);
        acm->wb[i].instance = acm;
        ((struct UsbHostRequest *)(acm->wb[i].request))->devHandle = (struct UsbDeviceHandle *)acm->devHandle;
        EXPECT_NE(nullptr,  acm->wb[i].request);
    }
}

/**
 * @tc.number    : CheckRawSdkIfAllocRequest003
 * @tc.name      :
 * @tc.type      : PERFs
 * @tc.level     : Level 1
 */
HWTEST_F(UsbHostSdkIfTest, CheckRawSdkIfAllocRequest003, TestSize.Level1)
{
    int i;

    for (i = 0; i < ACM_NR; i++) {
        acm->readReq[i] = UsbRawAllocRequest(NULL, 0, acm->dataInEp->maxPacketSize);
        EXPECT_EQ(nullptr,  acm->readReq[i]);
    }
}

/**
 * @tc.number    : CheckRawSdkIfAllocRequest004
 * @tc.name      :
 * @tc.type      : PERFs
 * @tc.level     : Level 1
 */
HWTEST_F(UsbHostSdkIfTest, CheckRawSdkIfAllocRequest004, TestSize.Level1)
{
    int i;

    for (i = 0; i < ACM_NR; i++) {
        acm->readReq[i] = UsbRawAllocRequest(acm->devHandle, 0, acm->dataInEp->maxPacketSize);
        ((struct UsbHostRequest *)(acm->readReq[i]))->devHandle = (struct UsbDeviceHandle *)acm->devHandle;
        EXPECT_NE(nullptr,  acm->readReq[i]);
    }
}

/**
 * @tc.number    : CheckRawSdkIfAllocRequest005
 * @tc.name      :
 * @tc.type      : PERFs
 * @tc.level     : Level 1
 */
HWTEST_F(UsbHostSdkIfTest, CheckRawSdkIfAllocRequest005, TestSize.Level1)
{
    acm->ctrlReq = UsbRawAllocRequest(NULL, 0, USB_CTRL_REQ_SIZE);
    EXPECT_EQ(nullptr,  acm->ctrlReq);
}

/**
 * @tc.number    : CheckRawSdkIfAllocRequest006
 * @tc.name      :
 * @tc.type      : PERFs
 * @tc.level     : Level 1
 */
HWTEST_F(UsbHostSdkIfTest, CheckRawSdkIfAllocRequest006, TestSize.Level1)
{
    acm->ctrlReq = UsbRawAllocRequest(acm->devHandle, 0, USB_CTRL_REQ_SIZE);
    ((struct UsbHostRequest *)(acm->ctrlReq))->devHandle = (struct UsbDeviceHandle *)acm->devHandle;
    EXPECT_NE(nullptr,  acm->ctrlReq);
}

/**
 * @tc.number    : CheckRawSdkIfAllocRequest007
 * @tc.name      :
 * @tc.type      : PERFs
 * @tc.level     : Level 1
 */
HWTEST_F(UsbHostSdkIfTest, CheckRawSdkIfAllocRequest007, TestSize.Level1)
{
    acm->notifyReq = UsbRawAllocRequest(NULL, 0, acm->notifyEp->maxPacketSize);
    EXPECT_EQ(nullptr,  acm->notifyReq);
}

/**
 * @tc.number    : CheckRawSdkIfAllocRequest008
 * @tc.name      :
 * @tc.type      : PERFs
 * @tc.level     : Level 1
 */
HWTEST_F(UsbHostSdkIfTest, CheckRawSdkIfAllocRequest008, TestSize.Level1)
{
    acm->notifyReq = UsbRawAllocRequest(acm->devHandle, 0, acm->notifyEp->maxPacketSize);
    ((struct UsbHostRequest *)(acm->notifyReq))->devHandle = (struct UsbDeviceHandle *)acm->devHandle;
    EXPECT_NE(nullptr,  acm->notifyReq);
}

/**
 * @tc.number    : CheckRawSdkIfFreeRequest001
 * @tc.name      :
 * @tc.type      : PERFs
 * @tc.level     : Level 1
 */
HWTEST_F(UsbHostSdkIfTest, CheckRawSdkIfFreeRequest001, TestSize.Level1)
{
    int32_t ret;
    int i;

    for (i = 0; i < ACM_NW; i++) {
        ret = UsbRawFreeRequest(acm->wb[i].request);
        EXPECT_EQ(HDF_SUCCESS, ret);
    }
}

/**
 * @tc.number    : CheckRawSdkIfFreeRequest002
 * @tc.name      :
 * @tc.type      : PERFs
 * @tc.level     : Level 1
 */
HWTEST_F(UsbHostSdkIfTest, CheckRawSdkIfFreeRequest002, TestSize.Level1)
{
    int32_t ret;
    int i;

    for (i = 0; i < ACM_NW; i++) {
        ret = UsbRawFreeRequest(acm->readReq[i]);
        EXPECT_EQ(HDF_SUCCESS, ret);
    }
}

/**
 * @tc.number    : CheckRawSdkIfFreeRequest003
 * @tc.name      :
 * @tc.type      : PERFs
 * @tc.level     : Level 1
 */
HWTEST_F(UsbHostSdkIfTest, CheckRawSdkIfFreeRequest003, TestSize.Level1)
{
    int32_t ret;

    ret = UsbRawFreeRequest(acm->ctrlReq);
    EXPECT_EQ(HDF_SUCCESS, ret);
}

/**
 * @tc.number    : CheckRawSdkIfFreeRequest004
 * @tc.name      :
 * @tc.type      : PERFs
 * @tc.level     : Level 1
 */
HWTEST_F(UsbHostSdkIfTest, CheckRawSdkIfFreeRequest004, TestSize.Level1)
{
    int32_t ret;

    ret = UsbRawFreeRequest(acm->notifyReq);
    EXPECT_EQ(HDF_SUCCESS, ret);
}

/**
 * @tc.number    : CheckRawSdkIfFreeRequest005
 * @tc.name      :
 * @tc.type      : PERFs
 * @tc.level     : Level 1
 */
HWTEST_F(UsbHostSdkIfTest, CheckRawSdkIfFreeRequest005, TestSize.Level1)
{
    int32_t ret;

    ret = UsbRawFreeRequest(NULL);
    EXPECT_EQ(HDF_ERR_INVALID_PARAM, ret);
}

/**
 * @tc.number    : CheckRawSdkIfAllocRequest009
 * @tc.name      :
 * @tc.type      : PERFs
 * @tc.level     : Level 1
 */
HWTEST_F(UsbHostSdkIfTest, CheckRawSdkIfAllocRequest009, TestSize.Level1)
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

/**
 * @tc.number    : CheckRawSdkIfGetDescriptor001
 * @tc.name      :
 * @tc.type      : PERF
 * @tc.level     : Level 1
 */
HWTEST_F(UsbHostSdkIfTest, CheckRawSdkIfGetDescriptor001, TestSize.Level1)
{
    UsbRawDescriptorParam param;
    unsigned char data[100];
    int ret;

    ret = UsbRawGetDescriptor(NULL, acm->devHandle, &param, data);
    EXPECT_EQ(HDF_ERR_INVALID_PARAM, ret);
}

/**
 * @tc.number    : CheckRawSdkIfGetDescriptor002
 * @tc.name      :
 * @tc.type      : PERF
 * @tc.level     : Level 1
 */
HWTEST_F(UsbHostSdkIfTest, CheckRawSdkIfGetDescriptor002, TestSize.Level1)
{
    UsbRawDescriptorParam param;
    unsigned char data[100];
    int ret;

    ret = UsbRawGetDescriptor(acm->ctrlReq, NULL, &param, data);
    EXPECT_EQ(HDF_ERR_INVALID_PARAM, ret);
}

/**
 * @tc.number    : CheckRawSdkIfGetDescriptor003
 * @tc.name      :
 * @tc.type      : PERF
 * @tc.level     : Level 1
 */
HWTEST_F(UsbHostSdkIfTest, CheckRawSdkIfGetDescriptor003, TestSize.Level1)
{
    UsbRawDescriptorParam param;
    unsigned char data[100];
    int ret;

    ret = UsbRawGetDescriptor(NULL, NULL, &param, data);
    EXPECT_EQ(HDF_ERR_INVALID_PARAM, ret);
}

/**
 * @tc.number    : CheckRawSdkIfGetDescriptor004
 * @tc.name      :
 * @tc.type      : PERF
 * @tc.level     : Level 1
 */
HWTEST_F(UsbHostSdkIfTest, CheckRawSdkIfGetDescriptor004, TestSize.Level1)
{
    unsigned char data[100];
    int ret;

    ret = UsbRawGetDescriptor(acm->ctrlReq, acm->devHandle, NULL, data);
    EXPECT_EQ(HDF_ERR_INVALID_PARAM, ret);
}

/**
 * @tc.number    : CheckRawSdkIfGetDescriptor005
 * @tc.name      :
 * @tc.type      : PERF
 * @tc.level     : Level 1
 */
HWTEST_F(UsbHostSdkIfTest, CheckRawSdkIfGetDescriptor005, TestSize.Level1)
{
    UsbRawDescriptorParam param;
    int ret;

    ret = UsbRawGetDescriptor(acm->ctrlReq, acm->devHandle, &param, NULL);
    EXPECT_EQ(HDF_ERR_INVALID_PARAM, ret);
}

/**
 * @tc.number    : CheckRawSdkIfGetDescriptor006
 * @tc.name      :
 * @tc.type      : PERF
 * @tc.level     : Level 1
 */
HWTEST_F(UsbHostSdkIfTest, CheckRawSdkIfGetDescriptor006, TestSize.Level1)
{
    UsbRawDescriptorParam param;
    unsigned char data[100];
    int ret;

    param.descType = 0;
    param.descIndex = 0;
    param.length = sizeof(data);

    ret = UsbRawGetDescriptor(acm->ctrlReq, acm->devHandle, &param, data);
    EXPECT_EQ(HDF_ERR_IO, ret);
}

/**
 * @tc.number    : CheckRawSdkIfGetDescriptor007
 * @tc.name      :
 * @tc.type      : PERF
 * @tc.level     : Level 1
 */
HWTEST_F(UsbHostSdkIfTest, CheckRawSdkIfGetDescriptor007, TestSize.Level1)
{
    unsigned char data[100];
    int ret;

    ret = UsbRawGetDescriptor(acm->ctrlReq, NULL, NULL, data);
    EXPECT_EQ(HDF_ERR_INVALID_PARAM, ret);
}

/**
 * @tc.number    : CheckRawSdkIfGetDescriptor008
 * @tc.name      :
 * @tc.type      : PERF
 * @tc.level     : Level 1
 */
HWTEST_F(UsbHostSdkIfTest, CheckRawSdkIfGetDescriptor008, TestSize.Level1)
{
    int ret;

    ret = UsbRawGetDescriptor(acm->ctrlReq, acm->devHandle, NULL, NULL);
    EXPECT_EQ(HDF_ERR_INVALID_PARAM, ret);
}

/**
 * @tc.number    : CheckRawSdkIfGetDescriptor009
 * @tc.name      :
 * @tc.type      : PERF
 * @tc.level     : Level 1
 */
HWTEST_F(UsbHostSdkIfTest, CheckRawSdkIfGetDescriptor009, TestSize.Level1)
{
    unsigned char data[100];
    int ret;

    ret = UsbRawGetDescriptor(NULL, acm->devHandle, NULL, data);
    EXPECT_EQ(HDF_ERR_INVALID_PARAM, ret);
}

/**
 * @tc.number    : CheckRawSdkIfGetDescriptor010
 * @tc.name      :
 * @tc.type      : PERF
 * @tc.level     : Level 1
 */
HWTEST_F(UsbHostSdkIfTest, CheckRawSdkIfGetDescriptor010, TestSize.Level1)
{
    UsbRawDescriptorParam param;
    unsigned char data[100];
    int ret;

    param.descType = 0;
    param.descIndex = 0;
    param.length = sizeof(data);

    ret = UsbRawGetDescriptor(NULL, acm->devHandle, &param, NULL);
    EXPECT_EQ(HDF_ERR_INVALID_PARAM, ret);
}

/**
 * @tc.number    : CheckRawSdkIfGetDescriptor011
 * @tc.name      :
 * @tc.type      : PERF
 * @tc.level     : Level 1
 */
HWTEST_F(UsbHostSdkIfTest, CheckRawSdkIfGetDescriptor011, TestSize.Level1)
{
    UsbRawDescriptorParam param;
    unsigned char data[100];
    int ret;

    param.descType = 0;
    param.descIndex = 0;
    param.length = sizeof(data);

    ret = UsbRawGetDescriptor(acm->ctrlReq,NULL, &param, NULL);
    EXPECT_EQ(HDF_ERR_INVALID_PARAM, ret);
}

/**
 * @tc.number    : CheckRawSdkIfGetDescriptor012
 * @tc.name      :
 * @tc.type      : PERF
 * @tc.level     : Level 1
 */
HWTEST_F(UsbHostSdkIfTest, CheckRawSdkIfGetDescriptor012, TestSize.Level1)
{
    unsigned char data[100];
    int ret;

    ret = UsbRawGetDescriptor(NULL, NULL, NULL, data);
    EXPECT_EQ(HDF_ERR_INVALID_PARAM, ret);
}

/**
 * @tc.number    : CheckRawSdkIfGetDescriptor013
 * @tc.name      :
 * @tc.type      : PERF
 * @tc.level     : Level 1
 */
HWTEST_F(UsbHostSdkIfTest, CheckRawSdkIfGetDescriptor013, TestSize.Level1)
{
    UsbRawDescriptorParam param;
    unsigned char data[100];
    int ret;

    param.descType = 0;
    param.descIndex = 0;
    param.length = sizeof(data);

    ret = UsbRawGetDescriptor(NULL, NULL, &param, NULL);
    EXPECT_EQ(HDF_ERR_INVALID_PARAM, ret);
}

/**
 * @tc.number    : CheckRawSdkIfGetDescriptor014
 * @tc.name      :
 * @tc.type      : PERF
 * @tc.level     : Level 1
 */
HWTEST_F(UsbHostSdkIfTest, CheckRawSdkIfGetDescriptor014, TestSize.Level1)
{
    int ret;

    ret = UsbRawGetDescriptor(NULL, acm->devHandle, NULL, NULL);
    EXPECT_EQ(HDF_ERR_INVALID_PARAM, ret);
}

/**
 * @tc.number    : CheckRawSdkIfGetDescriptor015
 * @tc.name      :
 * @tc.type      : PERF
 * @tc.level     : Level 1
 */
HWTEST_F(UsbHostSdkIfTest, CheckRawSdkIfGetDescriptor015, TestSize.Level1)
{
    int ret;

    ret = UsbRawGetDescriptor(acm->ctrlReq, NULL, NULL, NULL);
    EXPECT_EQ(HDF_ERR_INVALID_PARAM, ret);
}

/**
 * @tc.number    : CheckRawSdkIfGetDescriptor016
 * @tc.name      :
 * @tc.type      : PERF
 * @tc.level     : Level 1
 */
HWTEST_F(UsbHostSdkIfTest, CheckRawSdkIfGetDescriptor016, TestSize.Level1)
{
    int ret;

    ret = UsbRawGetDescriptor(NULL, NULL, NULL, NULL);
    EXPECT_EQ(HDF_ERR_INVALID_PARAM, ret);
}





/**
 * @tc.number    : CheckRawSdkIfFillBulkRequest001
 * @tc.name      :
 * @tc.type      : PERFs
 * @tc.level     : Level 1
 */
HWTEST_F(UsbHostSdkIfTest, CheckRawSdkIfFillBulkRequest001, TestSize.Level1)
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
 * @tc.number    : CheckRawSdkIfFillBulkRequest002
 * @tc.name      :
 * @tc.type      : PERFs
 * @tc.level     : Level 1
 */
HWTEST_F(UsbHostSdkIfTest, CheckRawSdkIfFillBulkRequest002, TestSize.Level1)
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

/**
 * @tc.number    : CheckRawSdkIfFillInterruptRequest001
 * @tc.name      :
 * @tc.type      : PERFs
 * @tc.level     : Level 1
 */
HWTEST_F(UsbHostSdkIfTest, CheckRawSdkIfFillInterruptRequest001, TestSize.Level1)
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
 * @tc.number    : CheckRawSdkIfFillInterruptRequest002
 * @tc.name      :
 * @tc.type      : PERFs
 * @tc.level     : Level 1
 */
HWTEST_F(UsbHostSdkIfTest, CheckRawSdkIfFillInterruptRequest002, TestSize.Level1)
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

    ret = UsbRawFillInterruptRequest(NULL, acm->devHandle, &fillRequestData);
    EXPECT_EQ(HDF_ERR_INVALID_PARAM, ret);
}

/**
 * @tc.number    : CheckRawSdkIfFillInterruptRequest003
 * @tc.name      :
 * @tc.type      : PERFs
 * @tc.level     : Level 1
 */
HWTEST_F(UsbHostSdkIfTest, CheckRawSdkIfFillInterruptRequest003, TestSize.Level1)
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

    ret = UsbRawFillInterruptRequest(acm->notifyReq, NULL, &fillRequestData);
    EXPECT_EQ(HDF_ERR_INVALID_PARAM, ret);
}

/**
 * @tc.number    : CheckRawSdkIfFillInterruptRequest004
 * @tc.name      :
 * @tc.type      : PERFs
 * @tc.level     : Level 1
 */
HWTEST_F(UsbHostSdkIfTest, CheckRawSdkIfFillInterruptRequest004, TestSize.Level1)
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

    ret = UsbRawFillInterruptRequest(acm->notifyReq, acm->devHandle, NULL);
    EXPECT_EQ(HDF_ERR_INVALID_PARAM, ret);
}


/**
 * @tc.number    : CheckRawSdkIfFillControlRequest001
 * @tc.name      :
 * @tc.type      : PERFs
 * @tc.level     : Level 1
 */
HWTEST_F(UsbHostSdkIfTest, CheckRawSdkIfFillControlRequest001, TestSize.Level1)
{
    struct UsbRawFillRequestData fillRequestData;
    int ret;
    int completed = 0;

    fillRequestData.callback  = AcmCtrlReqCallback;
    fillRequestData.userData  = &completed;
    fillRequestData.timeout   = USB_CTRL_SET_TIMEOUT;

    ret = UsbRawFillControlRequest(acm->ctrlReq, acm->devHandle, &fillRequestData);
    EXPECT_EQ(HDF_SUCCESS, ret);
}

/**
 * @tc.number    : CheckRawSdkIfFillControlRequest002
 * @tc.name      :
 * @tc.type      : PERFs
 * @tc.level     : Level 1
 */
HWTEST_F(UsbHostSdkIfTest, CheckRawSdkIfFillControlRequest002, TestSize.Level1)
{
    struct UsbRawFillRequestData fillRequestData;
    int ret;
    int completed = 0;

    fillRequestData.callback  = AcmCtrlReqCallback;
    fillRequestData.userData  = &completed;
    fillRequestData.timeout   = USB_CTRL_SET_TIMEOUT;

    ret = UsbRawFillControlRequest(NULL, acm->devHandle, &fillRequestData);
    EXPECT_EQ(HDF_ERR_INVALID_PARAM, ret);
}

/**
 * @tc.number    : CheckRawSdkIfFillControlRequest003
 * @tc.name      :
 * @tc.type      : PERFs
 * @tc.level     : Level 1
 */
HWTEST_F(UsbHostSdkIfTest, CheckRawSdkIfFillControlRequest003, TestSize.Level1)
{
    struct UsbRawFillRequestData fillRequestData;
    int ret;
    int completed = 0;

    fillRequestData.callback  = AcmCtrlReqCallback;
    fillRequestData.userData  = &completed;
    fillRequestData.timeout   = USB_CTRL_SET_TIMEOUT;

    ret = UsbRawFillControlRequest(acm->ctrlReq, NULL, &fillRequestData);
    EXPECT_EQ(HDF_ERR_INVALID_PARAM, ret);
}

/**
 * @tc.number    : CheckRawSdkIfFillControlRequest004
 * @tc.name      :
 * @tc.type      : PERFs
 * @tc.level     : Level 1
 */
HWTEST_F(UsbHostSdkIfTest, CheckRawSdkIfFillControlRequest004, TestSize.Level1)
{
    struct UsbRawFillRequestData fillRequestData;
    int ret;
    int completed = 0;

    fillRequestData.callback  = AcmCtrlReqCallback;
    fillRequestData.userData  = &completed;
    fillRequestData.timeout   = USB_CTRL_SET_TIMEOUT;

    ret = UsbRawFillControlRequest(acm->ctrlReq, acm->devHandle, NULL);
    EXPECT_EQ(HDF_ERR_INVALID_PARAM, ret);
}

/**
 * @tc.number    : CheckRawSdkIfFillControlRequest005
 * @tc.name      :
 * @tc.type      : PERFs
 * @tc.level     : Level 1
 */
HWTEST_F(UsbHostSdkIfTest, CheckRawSdkIfFillControlRequest005, TestSize.Level1)
{
    struct UsbRawFillRequestData fillRequestData;
    int ret;
    int completed = 0;

    fillRequestData.callback  = AcmCtrlReqCallback;
    fillRequestData.userData  = &completed;
    fillRequestData.timeout   = USB_CTRL_SET_TIMEOUT;

    ret = UsbRawFillControlRequest(NULL, acm->devHandle, NULL);
    EXPECT_EQ(HDF_ERR_INVALID_PARAM, ret);
}

/**
 * @tc.number    : CheckRawSdkIfFillControlRequest006
 * @tc.name      :
 * @tc.type      : PERFs
 * @tc.level     : Level 1
 */
HWTEST_F(UsbHostSdkIfTest, CheckRawSdkIfFillControlRequest006, TestSize.Level1)
{
    struct UsbRawFillRequestData fillRequestData;
    int ret;
    int completed = 0;

    fillRequestData.callback  = AcmCtrlReqCallback;
    fillRequestData.userData  = &completed;
    fillRequestData.timeout   = USB_CTRL_SET_TIMEOUT;

    ret = UsbRawFillControlRequest(acm->ctrlReq, NULL, NULL);
    EXPECT_EQ(HDF_ERR_INVALID_PARAM, ret);
}

/**
 * @tc.number    : CheckRawSdkIfFillControlRequest007
 * @tc.name      :
 * @tc.type      : PERFs
 * @tc.level     : Level 1
 */
HWTEST_F(UsbHostSdkIfTest, CheckRawSdkIfFillControlRequest007, TestSize.Level1)
{
    struct UsbRawFillRequestData fillRequestData;
    int ret;
    int completed = 0;

    fillRequestData.callback  = AcmCtrlReqCallback;
    fillRequestData.userData  = &completed;
    fillRequestData.timeout   = USB_CTRL_SET_TIMEOUT;

    ret = UsbRawFillControlRequest(NULL, NULL, NULL);
    EXPECT_EQ(HDF_ERR_INVALID_PARAM, ret);
}

/**
 * @tc.number    : CheckRawSdkIfFillControlRequest008
 * @tc.name      :
 * @tc.type      : PERFs
 * @tc.level     : Level 1
 */
HWTEST_F(UsbHostSdkIfTest, CheckRawSdkIfFillControlRequest008, TestSize.Level1)
{
    struct UsbRawFillRequestData fillRequestData;
    int ret;
    int completed = 0;

    fillRequestData.callback  = AcmCtrlReqCallback;
    fillRequestData.userData  = &completed;
    fillRequestData.timeout   = USB_CTRL_SET_TIMEOUT;

    ret = UsbRawFillControlRequest(NULL, NULL, &fillRequestData);
    EXPECT_EQ(HDF_ERR_INVALID_PARAM, ret);
}

/**
 * @tc.number    : CheckRawSdkIfFillIsoRequest001
 * @tc.name      :
 * @tc.type      : PERFs
 * @tc.level     : Level 1
 */
HWTEST_F(UsbHostSdkIfTest, CheckRawSdkIfFillIsoRequest001, TestSize.Level1)
{

}

/**
 * @tc.number    : CheckRawSdkIfFillControlSetup001
 * @tc.name      :
 * @tc.type      : PERFs
 * @tc.level     : Level 1
 */
HWTEST_F(UsbHostSdkIfTest, CheckRawSdkIfFillControlSetup001, TestSize.Level1)
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

    ret = UsbRawFillControlSetup(NULL, &ctrlReq);
    EXPECT_EQ(HDF_ERR_INVALID_PARAM, ret);
}

/**
 * @tc.number    : CheckRawSdkIfFillControlSetup002
 * @tc.name      :
 * @tc.type      : PERFs
 * @tc.level     : Level 1
 */
HWTEST_F(UsbHostSdkIfTest, CheckRawSdkIfFillControlSetup002, TestSize.Level1)
{
    unsigned char setup[100] = {0};
    int ret;

    ret = UsbRawFillControlSetup(setup, NULL);
    EXPECT_EQ(HDF_ERR_INVALID_PARAM, ret);
}

/**
 * @tc.number    : CheckRawSdkIfFillControlSetup003
 * @tc.name      :
 * @tc.type      : PERFs
 * @tc.level     : Level 1
 */
HWTEST_F(UsbHostSdkIfTest, CheckRawSdkIfFillControlSetup003, TestSize.Level1)
{
    int ret;

    ret = UsbRawFillControlSetup(NULL, NULL);
    EXPECT_EQ(HDF_ERR_INVALID_PARAM, ret);
}

/**
 * @tc.number    : CheckRawSdkIfFillControlSetup004
 * @tc.name      :
 * @tc.type      : PERFs
 * @tc.level     : Level 1
 */
HWTEST_F(UsbHostSdkIfTest, CheckRawSdkIfFillControlSetup004, TestSize.Level1)
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
    ctrlReq.index       = 0;
    ctrlReq.data        = (unsigned char *)&acm->lineCoding;
    ctrlReq.length      = sizeof(struct UsbCdcLineCoding);
    ctrlReq.timeout     = USB_CTRL_SET_TIMEOUT;

    ret = UsbRawFillControlSetup(setup, &ctrlReq);
    EXPECT_EQ(HDF_SUCCESS, ret);
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

    for (i = 0; i < ACM_NW; i++) {
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
 * @tc.number    : CheckRawSdkIfFreeConfigDescriptor001
 * @tc.name      :
 * @tc.type      : PERF
 * @tc.level     : Level 1
 */
HWTEST_F(UsbHostSdkIfTest, CheckRawSdkIfFreeConfigDescriptor001, TestSize.Level1)
{
    UsbRawFreeConfigDescriptor(NULL);
}

/**
 * @tc.number    : CheckRawSdkIfFreeConfigDescriptor002
 * @tc.name      :
 * @tc.type      : PERF
 * @tc.level     : Level 1
 */
HWTEST_F(UsbHostSdkIfTest, CheckRawSdkIfFreeConfigDescriptor002, TestSize.Level1)
{
    UsbRawFreeConfigDescriptor(acm->config);
}
}
