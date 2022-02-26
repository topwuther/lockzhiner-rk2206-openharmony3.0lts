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

#include "osal_mem.h"
#include "osal_time.h"
#include "securec.h"
#include "hdf_base.h"
#include "hdf_log.h"
#include "hdf_usb_pnp_manage.h"
#include "hdf_base.h"
#include "hdf_log.h"
#include "osal_mem.h"
#include "osal_time.h"
#include "securec.h"
#include "usb_interface.h"
#include "hdf_usb_pnp_manage.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <dirent.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <stdlib.h>
#include <sys/time.h>
#include <signal.h>
#include <sys/mman.h>
#include <osal_thread.h>
#include "usbhost_sdkraw_speed.h"
#include "implementation/global_implementation.h"
#include "signal.h"
#include "usb_pnp_notify.h"


#define HDF_LOG_TAG                USB_HOST_ACM_RAW_API

#define USB_CTRL_REQ_SIZE          64
#define USB_IO_THREAD_STACK_SIZE   8192
#define USB_RAW_IO_SLEEP_MS_TIME        500
#define USB_RAW_IO_STOP_WAIT_MAX_TIME   2
#define USB_DEVICE_VENDOR_ID 0x12D1
#define USB_DEVICE_PRODUCT_ID 0x5000

static struct AcmDevice *acm = NULL;
static bool g_stopIoThreadFlag = false;
static unsigned int g_speedFlag = 0;
static uint64_t g_send_count = 0;
static uint64_t g_recv_count = 0;
static uint64_t g_byteTotal = 0;
static bool g_writeOrRead = TEST_WRITE;
static bool g_printData = false;
static struct OsalSem sem;
static struct OsalSem timeSem;
static uint32_t sigCnt = 0;
static void AcmTestBulkCallback(const void *requestArg);
static int32_t SerialBegin(struct AcmDevice *acm);

static int UsbIoThread(void *data)
{
    int ret;
    struct AcmDevice *acm = (struct AcmDevice *)data;

    for (;;) {
        if (acm == NULL) {
            printf("%s:%d acm is NULL", __func__, __LINE__);
            OsalMSleep(USB_RAW_IO_SLEEP_MS_TIME);
            continue;
        }

        if (acm->devHandle == NULL) {
            printf("%s:%d acm->devHandle is NULL!", __func__, __LINE__);
            OsalMSleep(USB_RAW_IO_SLEEP_MS_TIME);
            continue;
        }

        ret = UsbRawHandleRequests(acm->devHandle);
        if (ret < 0) {
            printf("%s:%d UsbRawHandleRequests failed, ret=%d ", \
                __func__, __LINE__, ret);
            if (ret == USB_REQUEST_NO_DEVICE) {
                printf("%s:%d, ret=%d", __func__, __LINE__, ret);
                OsalMSleep(USB_RAW_IO_SLEEP_MS_TIME);
            }
        }

        if (g_stopIoThreadFlag == true) {
            HDF_LOGD("%s:%d", __func__, __LINE__);
            g_stopIoThreadFlag = false;
            break;
        }
    }

    HDF_LOGD("%s:%d exit", __func__, __LINE__);

    return HDF_SUCCESS;
}

static int UsbIoSendThread(void *data)
{
    struct AcmDevice *acm = (struct AcmDevice *)data;

    while (!g_stopIoThreadFlag) {
        OsalSemWait(&sem, HDF_WAIT_FOREVER);
        if (!g_speedFlag) {
            SerialBegin(acm);
            g_send_count++;
        }
    }
    return 0;
}

static int UsbStartIo(struct AcmDevice *acm)
{
    struct OsalThreadParam threadCfg;
    int ret;

    HDF_LOGI("%s start", __func__);
    g_stopIoThreadFlag = false;

    /* creat Io thread */
    (void)memset_s(&threadCfg, sizeof(threadCfg), 0, sizeof(threadCfg));
    threadCfg.name      = "usb io thread";
    threadCfg.priority  = OSAL_THREAD_PRI_DEFAULT;
    threadCfg.stackSize = USB_IO_THREAD_STACK_SIZE;

    ret = OsalThreadCreate(&acm->ioThread, \
                           (OsalThreadEntry)UsbIoThread, (void *)acm);
    if (ret != HDF_SUCCESS) {
        HDF_LOGE("%s:%d OsalThreadCreate faile, ret=%d ",
                 __func__, __LINE__, ret);
        return ret;
    }

    ret = OsalThreadStart(&acm->ioThread, &threadCfg);
    if (ret != HDF_SUCCESS) {
        HDF_LOGE("%s:%d OsalThreadStart faile, ret=%d ",
                 __func__, __LINE__, ret);
        return ret;
    }

    (void)memset_s(&threadCfg, sizeof(threadCfg), 0, sizeof(threadCfg));
    threadCfg.name      = "usb io send thread";
    threadCfg.priority  = OSAL_THREAD_PRI_DEFAULT;
    threadCfg.stackSize = USB_IO_THREAD_STACK_SIZE;
    ret = OsalThreadCreate(&acm->ioSendThread, \
                               (OsalThreadEntry)UsbIoSendThread, (void *)acm);
    if (ret != HDF_SUCCESS) {
        HDF_LOGE("%s:%d OsalThreadCreate faile, ret=%d ",
                 __func__, __LINE__, ret);
        return ret;
    }

    ret = OsalThreadStart(&acm->ioSendThread, &threadCfg);
    if (ret != HDF_SUCCESS) {
        HDF_LOGE("%s:%d OsalThreadStart faile, ret=%d ",
                 __func__, __LINE__, ret);
        return ret;
    }

    return HDF_SUCCESS;
}

static int UsbStopIo(struct AcmDevice *acm)
{
    int ret;
    int32_t i = 0;

    printf("%s:%d", __func__, __LINE__);
    if (g_stopIoThreadFlag == false) {
        printf("%s:%d", __func__, __LINE__);
        g_stopIoThreadFlag = true;
    }
    printf("%s:%d", __func__, __LINE__);

    while (g_stopIoThreadFlag) {
        i++;
        OsalMSleep(USB_RAW_IO_SLEEP_MS_TIME);
        if (i > USB_RAW_IO_STOP_WAIT_MAX_TIME) {
            printf("%s:%d", __func__, __LINE__);
            g_stopIoThreadFlag = false;
        }
    }
    printf("%s:%d", __func__, __LINE__);

    ret = OsalThreadDestroy(&acm->ioThread);
    if (ret != HDF_SUCCESS) {
        printf("%s:%d OsalThreadDestroy faile, ret=%d ",
                 __func__, __LINE__, ret);
        return ret;
    }

    ret = OsalThreadDestroy(&acm->ioSendThread);
    if (ret != HDF_SUCCESS) {
        printf("%s:%d OsalThreadDestroy faile, ret=%d ",
                 __func__, __LINE__, ret);
        return ret;
    }

    return HDF_SUCCESS;
}

static int UsbGetConfigDescriptor(UsbRawHandle *devHandle, struct UsbRawConfigDescriptor **config)
{
    UsbRawDevice *dev = NULL;
    int activeConfig;
    int ret;

    if (devHandle == NULL) {
        printf("%s:%d devHandle is NULL",
                 __func__, __LINE__);
        return HDF_ERR_INVALID_PARAM;
    }

    ret = UsbRawGetConfiguration(devHandle, &activeConfig);
    if (ret) {
        printf("%s:%d UsbRawGetConfiguration failed, ret=%d",
                 __func__, __LINE__, ret);
        return HDF_FAILURE;
    }
    HDF_LOGE("%s:%d activeConfig=%d", __func__, __LINE__, activeConfig);
    dev = UsbRawGetDevice(devHandle);
    if (dev == NULL) {
        printf("%s:%d UsbRawGetDevice failed",
                 __func__, __LINE__);
        return HDF_FAILURE;
    }

    ret = UsbRawGetConfigDescriptor(dev, activeConfig, config);
    if (ret) {
        printf("UsbRawGetConfigDescriptor failed, ret=%d\n", ret);
        return HDF_FAILURE;
    }

    return HDF_SUCCESS;
}

static int UsbParseConfigDescriptor(struct AcmDevice *acm, struct UsbRawConfigDescriptor *config)
{
    uint8_t i;
    uint8_t j;
    int ret;

    if ((acm == NULL) || (config == NULL)) {
        printf("%s:%d acm or config is NULL",
                 __func__, __LINE__);
        return HDF_ERR_INVALID_PARAM;
    }

    for (i = 0; i < acm->interfaceCnt; i++) {
        uint8_t interfaceIndex = acm->interfaceIndex[i];
        const struct UsbRawInterface *interface = config->interface[interfaceIndex];
        uint8_t ifaceClass = interface->altsetting->interfaceDescriptor.bInterfaceClass;
        uint8_t numEndpoints = interface->altsetting->interfaceDescriptor.bNumEndpoints;
        if (acm->devHandle) {
            ret = UsbRawClaimInterface(acm->devHandle, interfaceIndex);
            if (ret) {
                printf("%s:%d claim interface %d failed",
                         __func__, __LINE__, i);
                return ret;
            }
        }

        switch (ifaceClass) {
            case USB_DDK_CLASS_COMM:
                acm->ctrlIface = interfaceIndex;
                acm->notifyEp = OsalMemAlloc(sizeof(struct UsbEndpoint));
                if (acm->notifyEp == NULL) {
                    printf("%s:%d allocate endpoint failed",
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
                        acm->dataInEp = OsalMemAlloc(sizeof(struct UsbEndpoint));
                        if (acm->dataInEp == NULL) {
                            printf("%s:%d allocate dataInEp failed",
                                     __func__, __LINE__);
                            break;
                        }
                        acm->dataInEp->addr = endPoint->endpointDescriptor.bEndpointAddress;
                        acm->dataInEp->interval = endPoint->endpointDescriptor.bInterval;
                        acm->dataInEp->maxPacketSize = endPoint->endpointDescriptor.wMaxPacketSize;
                    } else { /* get bulk out endpoint */
                        acm->dataOutEp = OsalMemAlloc(sizeof(struct UsbEndpoint));
                        if (acm->dataOutEp == NULL) {
                            printf("%s:%d allocate dataOutEp failed",
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
                printf("%s:%d wrong descriptor type", __func__, __LINE__);
                break;
        }
    }

    return HDF_SUCCESS;
}

static int UsbAllocDataRequests(struct AcmDevice *acm)
{
    int i;
    int ret;
    for (i = 0; i < TEST_CYCLE; i++) {
        struct AcmDb *snd = &acm->db[i];
        snd->request = UsbRawAllocRequest(acm->devHandle, 0, acm->dataEp->maxPacketSize);
        snd->instance = acm;
        if (snd->request == NULL) {
            printf("%s: UsbRawAllocRequest faild", __func__);
            return HDF_ERR_MALLOC_FAIL;
        }
        struct UsbRawFillRequestData reqData;
        reqData.endPoint      = acm->dataEp->addr;
        reqData.numIsoPackets = 0;
        reqData.callback      = AcmTestBulkCallback;
        reqData.userData      = (void *)snd;
        reqData.timeout       = USB_CTRL_SET_TIMEOUT;
        reqData.buffer        = snd->buf;
        reqData.length        = acm->dataSize;

        ret = UsbRawFillBulkRequest(snd->request, acm->devHandle, &reqData);
        if (ret) {
            printf("%s: FillInterruptRequest faile, ret=%d",
                     __func__, ret);
            return HDF_FAILURE;
        }
    }

    return HDF_SUCCESS;
}

static int UsbFreeDataRequests(struct AcmDevice *acm)
{
    int i;
    for (i = 0; i < TEST_CYCLE; i++) {
        struct AcmDb *snd = &acm->db[i];
        UsbRawCancelRequest(snd->request);
        UsbRawFreeRequest(snd->request);
    }
    return HDF_SUCCESS;
}

static int AcmDbAlloc(struct AcmDevice *acm)
{
    struct AcmDb *db = NULL;
    int i;

    for (i = 0; i < TEST_CYCLE; i++) {
        db = &acm->db[i];
        if (!db->use) {
            db->use = 1;
            db->len = 0;
            return i;
        }
    }
    return -1;
}

static int AcmDbIsAvail(struct AcmDevice *acm)
{
    int i;
    int n = TEST_CYCLE;

    for (i = 0; i < TEST_CYCLE; i++) {
        n -= acm->db[i].use;
    }
    return n;
}

static int AcmStartdb(struct AcmDevice *acm, struct AcmDb *db)
{
    int ret;
    ret = UsbRawSubmitRequest(db->request);
    if (ret) {
        printf("UsbRawSubmitRequest failed, ret=%d", ret);
        db->use = 0;
    }
    return ret;
}

static int AcmDataBufAlloc(struct AcmDevice *acm)
{
    struct AcmDb *db = &acm->db[0];
    int i;

    for (i = 0; i < TEST_CYCLE; i++, db++) {
        db->buf = OsalMemCalloc(acm->dataEp->maxPacketSize);
        if (!db->buf) {
            while (i != 0) {
                --i;
                --db;
                OsalMemFree(db->buf);
                db->buf = NULL;
            }
            return -HDF_ERR_MALLOC_FAIL;
        }
        else {
            memset_s(db->buf, acm->dataSize, 'b', acm->dataSize);
            db->instance = acm;
        }
    }
    return HDF_SUCCESS;
}
static int AcmDataBufFree(const struct AcmDevice *acm)
{

    struct AcmDb *db = (struct AcmDb *)&acm->db[0];
    int i;
    for (i = 0; i < TEST_CYCLE; i++, db++) {
        if (db->buf) {
            OsalMemFree(db->buf);
            db->use = 0;
        }
    }
    return 0;
}

static void AcmTestBulkCallback(const void *requestArg)
{
    struct UsbRawRequest *req = (struct UsbRawRequest *)requestArg;
    if (req == NULL) {
        HDF_LOGE("%s:%{pulib}d req is NULL!", __func__, __LINE__);
        return;
    }
    struct AcmDb *db  = (struct AcmDb *)req->userData;
    if (db == NULL) {
        HDF_LOGE("%s:%{pulib}d userData(db) is NULL!", __func__, __LINE__);
        return;
    }

    if (req->status == USB_REQUEST_COMPLETED) {
        if (g_byteTotal == 0) {
            OsalSemPost(&timeSem);
        }
        g_recv_count++;
        g_byteTotal += req->actualLength;
    } else {
        printf("status error\n");
    }

    if (g_printData == true) {
        for (int i = 0; i < req->actualLength; i++)
            printf("%c", req->buffer[i]);
        fflush(stdout);
    } else if (g_recv_count % 10000 == 0) {
        printf("#");
        fflush(stdout);
    }

    db->use = 0;
    OsalSemPost(&sem);
}

static int32_t SerialBegin(struct AcmDevice *acm)
{
    int32_t ret;
    struct AcmDb *db = NULL;
    int dbn;
    if (AcmDbIsAvail(acm)) {
        dbn = AcmDbAlloc(acm);
    } else {
        printf("no buf\n");
        return 0;
    }
    if (dbn < 0) {
        printf("AcmDbAlloc failed\n");
        return HDF_FAILURE;
    }
    db = &acm->db[dbn];
    db->len = acm->dataSize;

    ret = AcmStartdb(acm, db);
    return ret;
}

static void SpeedPrint(void)
{
    double speed = 0;

    sigCnt++;
    if (sigCnt * TEST_PRINT_TIME >= TEST_TIME) {
        g_speedFlag = 1;
    }
    speed = (g_byteTotal * 1.0) / (sigCnt * TEST_PRINT_TIME  * 1024 * 1024);
    printf("\nSpeed:%f MB/s\n", speed);
}

static void ShowHelp(const char *name)
{
    printf(">> usage:\n");
    printf(">>      %s [<busNum> <devAddr>]  <ifaceNum> <w>/<r> [printdata]> \n", name);
    printf("\n");
}

static void UsbGetDevInfo(int *busNum, int *devNum)
{
    struct UsbGetDevicePara paraData;
    struct usb_device *usbPnpDevice = NULL;
    paraData.type = USB_PNP_DEVICE_VENDOR_PRODUCT_TYPE;
    paraData.vendorId = USB_DEVICE_VENDOR_ID;
    paraData.productId = USB_DEVICE_PRODUCT_ID;
    usbPnpDevice = UsbPnpNotifyGetUsbDevice(paraData);
    if (usbPnpDevice == NULL) {
        printf("%s:%d UsbPnpNotifyGetUsbDevice is NULL!", __func__, __LINE__);
        return;
    }
    *busNum = (int)usbPnpDevice->address;
    *devNum = (int)usbPnpDevice->port_no;
    printf("%s:%d busNum=%d devNum=%d!\n", __func__, __LINE__, *busNum, *devNum);
}

static int32_t UsbSerialOpen()
{
    return HDF_SUCCESS;
}
static int32_t UsbSerialClose()
{
    if (!g_speedFlag) {
        g_speedFlag = true;
    }
    return HDF_SUCCESS;
}

static int32_t UsbSerialSpeed(struct HdfSBuf *data)
{
    struct UsbSession *session = NULL;
    UsbRawHandle *devHandle = NULL;
    int32_t ret = HDF_SUCCESS;
    int busNum = 1;
    int devAddr = 2;
    int ifaceNum = 3;
    uint32_t size;
    int i = 0;
    struct UsbSpeedTest *input = NULL;

    if (acm->busy) {
        printf("%s: speed test busy\n", __func__);
        ret = HDF_ERR_IO;
        goto end;
    }
    acm->busy = true;
    g_speedFlag = false;
    g_send_count = 0;
    g_recv_count = 0;
    g_byteTotal = 0;
    g_printData = false;
    g_writeOrRead = TEST_WRITE;
    sigCnt = 0;

    (void)HdfSbufReadBuffer(data, (const void **)&input, &size);
    if ((input == NULL) || (size != sizeof(struct UsbSpeedTest))) {
        printf("%s: sbuf read buffer failed\n", __func__);
        ret = HDF_ERR_IO;
        goto end;
    }

    UsbGetDevInfo(&busNum, &devAddr);
    if (input->paramNum == 6) {
        busNum = input->busNum;
        devAddr = input->devAddr;
        ifaceNum = input->ifaceNum;
        g_writeOrRead = input->writeOrRead;
        if (g_writeOrRead == TEST_READ)
        {
            g_printData = input->printData;
        }
    } else if (input->paramNum == 5) {
        busNum = input->busNum;
        devAddr = input->devAddr;
        ifaceNum = input->ifaceNum;
        g_writeOrRead = input->writeOrRead;
    } else if (input->paramNum == 3) {
        ifaceNum = input->ifaceNum;
        g_writeOrRead = input->writeOrRead;
    } else {
        printf("Error: parameter error! \n\n");
        ShowHelp("speedtest");
        ret = HDF_FAILURE;
        goto end;
    }

    acm->busNum = busNum;
    acm->devAddr = devAddr;
    acm->interfaceCnt = 1;
    acm->interfaceIndex[0] = ifaceNum;
    OsalSemInit(&sem, 0);
    OsalSemInit(&timeSem, 0);

    ret = UsbRawInit(&session);
    if (ret) {
        HDF_LOGE("%s: UsbRawInit faild\n", __func__);
        return HDF_ERR_IO;
        goto end;
    }

    devHandle = UsbRawOpenDevice(session, acm->busNum, acm->devAddr);
    if (devHandle == NULL) {
        HDF_LOGE("%s: UsbRawOpenDevice faild\n", __func__);
        ret =  HDF_FAILURE;
        goto end;
    }
    acm->devHandle = devHandle;
    ret = UsbGetConfigDescriptor(devHandle, &acm->config);
    if (ret) {
        HDF_LOGE("%s: UsbGetConfigDescriptor faild\n", __func__);
        ret =  HDF_FAILURE;
        goto end;
    }
    ret = UsbParseConfigDescriptor(acm, acm->config);
    if (ret != HDF_SUCCESS) {
        HDF_LOGE("%s: UsbParseConfigDescriptor faild\n", __func__);
        ret = HDF_FAILURE;
        goto end;
    }
    acm->dataSize = TEST_LENGTH;
    acm->dataEp = (g_writeOrRead == TEST_WRITE)?acm->dataOutEp:acm->dataInEp;

    ret = AcmDataBufAlloc(acm);
    if (ret < 0) {
        HDF_LOGE("%s: AcmDataBufAlloc faild\n", __func__);
        ret = HDF_FAILURE;
        goto end;
    }
    ret = UsbAllocDataRequests(acm);
    if (ret < 0) {
        HDF_LOGE("%s: UsbAllocDataRequests faild\n", __func__);
        ret = HDF_FAILURE;
        goto end;
    }

    ret = UsbStartIo(acm);
    if (ret) {
        HDF_LOGE("%s: UsbAllocReadRequests faild\n", __func__);
        goto end;
    }

    printf("test SDK rawAPI [%s]\n", g_writeOrRead?"write":"read");

    for (i = 0; i < TEST_CYCLE; i++) {
        SerialBegin(acm);
        g_send_count++;
    }

    OsalSemWait(&timeSem, TEST_TIME);
    while (!g_speedFlag) {
        OsalSemWait(&timeSem, TEST_PRINT_TIME*1000);
        SpeedPrint();
    }

    UsbStopIo(acm);
    UsbFreeDataRequests(acm);
    AcmDataBufFree(acm);
    for (i = 0; i < acm->interfaceCnt; i++) {
        uint8_t interfaceIndex = acm->interfaceIndex[i];
        UsbRawReleaseInterface(acm->devHandle, interfaceIndex);
    }
    UsbRawCloseDevice(acm->devHandle);
    UsbRawExit(session);
    OsalSemDestroy(&timeSem);
    OsalSemDestroy(&sem);
    acm->busy = false;
end:
    if (ret != HDF_SUCCESS) {
        printf("please check whether usb drv so is existing or not,like acm, ecm, if not, remove it and test again!\n");
    }
    return ret;
}


static int32_t AcmDeviceDispatch(struct HdfDeviceIoClient *client, int cmd,
    struct HdfSBuf *data, struct HdfSBuf *reply)
{

    if (client == NULL) {
        HDF_LOGE("%s: client is NULL", __func__);
        return HDF_ERR_INVALID_OBJECT;
    }

    if (client->device == NULL) {
        HDF_LOGE("%s: client->device is NULL", __func__);
        return HDF_ERR_INVALID_OBJECT;
    }

    if (client->device->service == NULL) {
        HDF_LOGE("%s: client->device->service is NULL", __func__);
        return HDF_ERR_INVALID_OBJECT;
    }

    acm = (struct AcmDevice *)client->device->service;

    switch (cmd) {
        case USB_SERIAL_OPEN:
            return UsbSerialOpen();
        case USB_SERIAL_CLOSE:
            return UsbSerialClose();
        case USB_SERIAL_SPEED:
            return UsbSerialSpeed(data);
        default:
            return HDF_ERR_NOT_SUPPORT;
    }

    return HDF_SUCCESS;
}

static int32_t AcmDriverBind(struct HdfDeviceObject *device)
{
    if (device == NULL) {
        HDF_LOGE("%s: device is null", __func__);
        return HDF_ERR_INVALID_OBJECT;
    }

    acm = (struct AcmDevice *)OsalMemCalloc(sizeof(*acm));
    if (acm == NULL) {
        HDF_LOGE("%s: Alloc usb acm device failed", __func__);
        return HDF_FAILURE;
    }

    acm->device  = device;
    device->service = &(acm->service);
    if (acm->device && acm->device->service) {
        acm->device->service->Dispatch = AcmDeviceDispatch;
    }

    return HDF_SUCCESS;
}

static int32_t AcmDriverInit(struct HdfDeviceObject *device)
{
    return 0;
}

static void AcmDriverRelease(struct HdfDeviceObject *device)
{
}


struct HdfDriverEntry g_usbRawApiSpeedDriverEntry = {
    .moduleVersion = 1,
    .moduleName    = "usb_rawapispeed",
    .Bind          = AcmDriverBind,
    .Init          = AcmDriverInit,
    .Release       = AcmDriverRelease,
};

HDF_INIT(g_usbRawApiSpeedDriverEntry);

