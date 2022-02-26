/*
 * usb_pnp_notify.c
 *
 * usb pnp notify adapter of linux
 *
 * Copyright (c) 2020-2021 Huawei Device Co., Ltd.
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 */

#include "usb_pnp_notify.h"
#include <linux/delay.h>
#include <linux/kthread.h>
#include <linux/notifier.h>
#include <linux/usb.h>
#include "hdf_device_desc.h"
#include "hdf_log.h"
#include "osal_mem.h"
#include "securec.h"

#define HDF_LOG_TAG USB_PNP_NOTIFY

static wait_queue_head_t g_usbPnpNotifyReportWait;
static struct task_struct *g_usbPnpNotifyReportThread = NULL;
static enum UsbPnpNotifyServiceCmd g_usbPnpNotifyCmdType = USB_PNP_NOTIFY_ADD_INTERFACE;
static enum UsbPnpNotifyRemoveType g_usbPnpNotifyRemoveType = USB_PNP_NOTIFY_REMOVE_BUS_DEV_NUM;
struct OsalMutex g_usbSendEventLock;
struct usb_device *g_usbDevice = NULL;
struct UsbPnpAddRemoveInfo g_usbPnpInfo;
struct DListHead g_usbPnpInfoListHead;
#if USB_PNP_NOTIFY_TEST_MODE == true
struct UsbPnpNotifyMatchInfoTable *g_testUsbPnpInfo = NULL;
#endif

static struct UsbPnpDeviceInfo *UsbPnpNotifyCreateInfo(void)
{
    struct UsbPnpDeviceInfo *infoTemp = NULL;
    unsigned char *ptr = NULL;
    static int32_t idNum = 0;

    ptr = OsalMemCalloc(sizeof(struct UsbPnpDeviceInfo));
    if (ptr == NULL) {
        HDF_LOGE("%s:%d OsalMemAlloc faile ", __func__, __LINE__);
        return NULL;
    } else {
        infoTemp = (struct UsbPnpDeviceInfo *)ptr;

        if (idNum++ >= INT32_MAX) {
            idNum = 0;
        }
        infoTemp->id = idNum;
        OsalMutexInit(&infoTemp->lock);
        infoTemp->status = USB_PNP_DEVICE_INIT_STATUS;
        DListHeadInit(&infoTemp->list);
        memset_s(infoTemp->interfaceRemoveStatus, USB_PNP_INFO_MAX_INTERFACES,
            0, sizeof(infoTemp->interfaceRemoveStatus));
        DListInsertTail(&infoTemp->list, &g_usbPnpInfoListHead);

        return infoTemp;
    }
}

static struct UsbPnpDeviceInfo *UsbPnpNotifyFindInfo(struct UsbInfoQueryPara queryPara)
{
    struct UsbPnpDeviceInfo *infoPos = NULL;
    struct UsbPnpDeviceInfo *infoTemp = NULL;
    bool findFlag = false;

    if (DListIsEmpty(&g_usbPnpInfoListHead)) {
        HDF_LOGE("%s:%d usb pnp list head is empty. ", __func__, __LINE__);
        return NULL;
    }

    DLIST_FOR_EACH_ENTRY_SAFE(infoPos, infoTemp, &g_usbPnpInfoListHead, struct UsbPnpDeviceInfo, list) {
        switch (queryPara.type) {
            case USB_INFO_NORMAL_TYPE:
                if ((infoPos->info.devNum == queryPara.devNum) && (infoPos->info.busNum == queryPara.busNum)) {
                    findFlag = true;
                }
                break;
            case USB_INFO_ID_TYPE:
                if (infoPos->id == queryPara.id) {
                    findFlag = true;
                }
                break;
            case USB_INFO_DEVICE_ADDRESS_TYPE:
                if (infoPos->info.usbDevAddr == queryPara.usbDevAddr) {
                    findFlag = true;
                }
                break;
            default:
                break;
        }

        if (findFlag == true) {
            break;
        }
    }

    if (findFlag == false) {
        HDF_LOGE("%s:%d the usb pnp info to be find does not exist. ", __func__, __LINE__);
        return NULL;
    } else {
        return infoPos;
    }
}

static HDF_STATUS UsbPnpNotifyDestroyInfo(struct UsbPnpDeviceInfo *deviceInfo)
{
    HDF_STATUS ret = HDF_SUCCESS;
    struct UsbPnpDeviceInfo *infoPos = NULL;
    struct UsbPnpDeviceInfo *infoTemp = NULL;
    bool findFlag = false;

    if (deviceInfo == NULL) {
        ret = HDF_FAILURE;
        HDF_LOGE("%s:%d the deviceInfo is NULL, ret=%d ", __func__, __LINE__, ret);
        return ret;
    }

    if (DListIsEmpty(&g_usbPnpInfoListHead)) {
        HDF_LOGI("%s:%d the g_usbPnpInfoListHead is empty.", __func__, __LINE__);
        return HDF_SUCCESS;
    }

    DLIST_FOR_EACH_ENTRY_SAFE(infoPos, infoTemp, &g_usbPnpInfoListHead, struct UsbPnpDeviceInfo, list) {
        if (infoPos->id == deviceInfo->id) {
            findFlag = true;
            DListRemove(&infoPos->list);
            OsalMemFree((void *)infoPos);
            infoPos = NULL;
            break;
        }
    }

    if (findFlag == false) {
        ret = HDF_FAILURE;
        HDF_LOGE("%s:%d the deviceInfoto be destroyed does not exist, ret=%d ", __func__, __LINE__, ret);
    }

    return ret;
}

static int32_t UsbPnpNotifyAddInitInfo(struct UsbPnpDeviceInfo *deviceInfo, union UsbPnpDeviceInfoData infoData)
{
    int32_t ret = HDF_SUCCESS;

    deviceInfo->info.usbDevAddr = (uintptr_t)infoData.usbDev;
    deviceInfo->info.devNum = infoData.usbDev->devnum;
    if (infoData.usbDev->bus == NULL) {
        HDF_LOGE("%s infoData.usbDev->bus=%px is NULL", __func__, infoData.usbDev->bus);
        ret = HDF_ERR_INVALID_PARAM;
        goto out;
    }
    deviceInfo->info.busNum = infoData.usbDev->bus->busnum;

    deviceInfo->info.deviceInfo.vendorId = le16_to_cpu(infoData.usbDev->descriptor.idVendor);
    deviceInfo->info.deviceInfo.productId = le16_to_cpu(infoData.usbDev->descriptor.idProduct);
    deviceInfo->info.deviceInfo.bcdDeviceLow = le16_to_cpu(infoData.usbDev->descriptor.bcdDevice);
    deviceInfo->info.deviceInfo.bcdDeviceHigh = le16_to_cpu(infoData.usbDev->descriptor.bcdDevice);
    deviceInfo->info.deviceInfo.deviceClass = infoData.usbDev->descriptor.bDeviceClass;
    deviceInfo->info.deviceInfo.deviceSubClass = infoData.usbDev->descriptor.bDeviceSubClass;
    deviceInfo->info.deviceInfo.deviceProtocol = infoData.usbDev->descriptor.bDeviceProtocol;

    if (infoData.usbDev->actconfig == NULL) {
        HDF_LOGE("%s infoData.usbDev->actconfig=%p is NULL", __func__, infoData.usbDev->actconfig);
        ret = HDF_ERR_INVALID_PARAM;
        goto out;
    }
    deviceInfo->info.numInfos = infoData.usbDev->actconfig->desc.bNumInterfaces;
    for (uint8_t i = 0; i < deviceInfo->info.numInfos; i++) {
        if ((infoData.usbDev->actconfig->interface[i] == NULL) ||
            (infoData.usbDev->actconfig->interface[i]->cur_altsetting == NULL)) {
            HDF_LOGE("%s interface[%d]=%p or interface[%d]->cur_altsetting=%p is NULL",
                __func__, i, infoData.usbDev->actconfig->interface[i], i,
                infoData.usbDev->actconfig->interface[i]->cur_altsetting);
            ret = HDF_ERR_INVALID_PARAM;
            goto out;
        }
        deviceInfo->info.interfaceInfo[i].interfaceClass =
        infoData.usbDev->actconfig->interface[i]->cur_altsetting->desc.bInterfaceClass;
        deviceInfo->info.interfaceInfo[i].interfaceSubClass =
        infoData.usbDev->actconfig->interface[i]->cur_altsetting->desc.bInterfaceSubClass;
        deviceInfo->info.interfaceInfo[i].interfaceProtocol =
        infoData.usbDev->actconfig->interface[i]->cur_altsetting->desc.bInterfaceProtocol;
        deviceInfo->info.interfaceInfo[i].interfaceNumber =
        infoData.usbDev->actconfig->interface[i]->cur_altsetting->desc.bInterfaceNumber;

        HDF_LOGI("%s:%d i=%d, interfaceInfo=0x%x-0x%x-0x%x-0x%x",
            __func__, __LINE__, i, deviceInfo->info.interfaceInfo[i].interfaceClass,
            deviceInfo->info.interfaceInfo[i].interfaceSubClass,
            deviceInfo->info.interfaceInfo[i].interfaceProtocol,
            deviceInfo->info.interfaceInfo[i].interfaceNumber);
    }

out:
    return ret;
}

static void UsbPnpNotifyAddInterfaceInitInfo(struct UsbPnpDeviceInfo *deviceInfo,
    union UsbPnpDeviceInfoData infoData, struct UsbPnpNotifyMatchInfoTable *infoTable)
{
    uint8_t i;
    uint8_t j;

    for (i = 0; i < deviceInfo->info.numInfos; i++) {
        if ((infoData.infoData->interfaceClass == deviceInfo->info.interfaceInfo[i].interfaceClass)
            && (infoData.infoData->interfaceSubClass == deviceInfo->info.interfaceInfo[i].interfaceSubClass)
            && (infoData.infoData->interfaceProtocol == deviceInfo->info.interfaceInfo[i].interfaceProtocol)
            && (infoData.infoData->interfaceNumber == deviceInfo->info.interfaceInfo[i].interfaceNumber)) {
            if (g_usbPnpNotifyCmdType == USB_PNP_NOTIFY_REMOVE_INTERFACE) {
                deviceInfo->interfaceRemoveStatus[i] = true;
            } else if (g_usbPnpNotifyCmdType == USB_PNP_NOTIFY_ADD_INTERFACE) {
                deviceInfo->interfaceRemoveStatus[i] = false;
            }
        }
    }

    if (g_usbPnpNotifyCmdType == USB_PNP_NOTIFY_REMOVE_INTERFACE) {
        infoTable->numInfos = 1;
        infoTable->interfaceInfo[0].interfaceClass = infoData.infoData->interfaceClass;
        infoTable->interfaceInfo[0].interfaceSubClass = infoData.infoData->interfaceSubClass;
        infoTable->interfaceInfo[0].interfaceProtocol = infoData.infoData->interfaceProtocol;
        infoTable->interfaceInfo[0].interfaceNumber = infoData.infoData->interfaceNumber;
    } else {
        for (i = 0, j = 0; i < deviceInfo->info.numInfos; i++) {
            if (deviceInfo->interfaceRemoveStatus[i] == true) {
                HDF_LOGI("%s:%d j=%d deviceInfo->interfaceRemoveStatus[%d] is true!", __func__, __LINE__, j, i);
                continue;
            }
            infoTable->interfaceInfo[j].interfaceClass = deviceInfo->info.interfaceInfo[i].interfaceClass;
            infoTable->interfaceInfo[j].interfaceSubClass = deviceInfo->info.interfaceInfo[i].interfaceSubClass;
            infoTable->interfaceInfo[j].interfaceProtocol = deviceInfo->info.interfaceInfo[i].interfaceProtocol;
            infoTable->interfaceInfo[j].interfaceNumber = deviceInfo->info.interfaceInfo[i].interfaceNumber;
            j++;

            HDF_LOGI("%s:%d i=%d, j=%d, interfaceInfo=0x%x-0x%x-0x%x-0x%x",
                __func__, __LINE__, i, j-1, infoTable->interfaceInfo[j-1].interfaceClass,
                infoTable->interfaceInfo[j-1].interfaceSubClass,
                infoTable->interfaceInfo[j-1].interfaceNumber,
                infoTable->interfaceInfo[j-1].interfaceNumber);
        }
        infoTable->numInfos = j;
    }
}

static int32_t UsbPnpNotifyInitInfo(
    struct HdfSBuf *sbuf, struct UsbPnpDeviceInfo *deviceInfo, union UsbPnpDeviceInfoData infoData)
{
    int32_t ret = HDF_SUCCESS;
    const void *data = NULL;

    if ((g_usbPnpNotifyCmdType == USB_PNP_NOTIFY_ADD_INTERFACE)
        || (g_usbPnpNotifyCmdType == USB_PNP_NOTIFY_REMOVE_INTERFACE)) {
        static struct UsbPnpNotifyMatchInfoTable infoTable;

        infoTable.usbDevAddr = deviceInfo->info.usbDevAddr;
        infoTable.devNum = deviceInfo->info.devNum;
        infoTable.busNum = deviceInfo->info.busNum;
        infoTable.deviceInfo.vendorId = deviceInfo->info.deviceInfo.vendorId;
        infoTable.deviceInfo.productId = deviceInfo->info.deviceInfo.productId;
        infoTable.deviceInfo.bcdDeviceLow = deviceInfo->info.deviceInfo.bcdDeviceLow;
        infoTable.deviceInfo.bcdDeviceHigh = deviceInfo->info.deviceInfo.bcdDeviceHigh;
        infoTable.deviceInfo.deviceClass = deviceInfo->info.deviceInfo.deviceClass;
        infoTable.deviceInfo.deviceSubClass = deviceInfo->info.deviceInfo.deviceSubClass;
        infoTable.deviceInfo.deviceProtocol = deviceInfo->info.deviceInfo.deviceProtocol;
        infoTable.removeType = g_usbPnpNotifyRemoveType;

        UsbPnpNotifyAddInterfaceInitInfo(deviceInfo, infoData, &infoTable);

        data = (const void *)(&infoTable);
    } else if (g_usbPnpNotifyCmdType == USB_PNP_NOTIFY_REPORT_INTERFACE) {
        ret = UsbPnpNotifyAddInitInfo(deviceInfo, infoData);
        if (ret != HDF_SUCCESS) {
            goto out;
        }

        data = (const void *)(&deviceInfo->info);
    } else {
        data = (const void *)(&deviceInfo->info);
    }

    if (!HdfSbufWriteBuffer(sbuf, data, sizeof(struct UsbPnpNotifyMatchInfoTable))) {
        HDF_LOGE("%s:%d sbuf write data failed", __func__, __LINE__);
        ret = HDF_FAILURE;
        goto out;
    }

out:
    return ret;
}

static int32_t UsbPnpNotifyGetDeviceInfo(void *eventData, union UsbPnpDeviceInfoData *pnpInfoData,
    struct UsbPnpDeviceInfo **deviceInfo)
{
    struct UsbInfoQueryPara infoQueryPara;

    if ((g_usbPnpNotifyCmdType == USB_PNP_NOTIFY_ADD_INTERFACE)
        || (g_usbPnpNotifyCmdType == USB_PNP_NOTIFY_REMOVE_INTERFACE)) {
        pnpInfoData->infoData = (struct UsbPnpAddRemoveInfo *)eventData;
    } else {
        pnpInfoData->usbDev = (struct usb_device *)eventData;
    }

    if ((g_usbPnpNotifyCmdType == USB_PNP_NOTIFY_ADD_INTERFACE)
        || (g_usbPnpNotifyCmdType == USB_PNP_NOTIFY_REMOVE_INTERFACE)) {
        infoQueryPara.type = USB_INFO_NORMAL_TYPE;
        infoQueryPara.devNum = pnpInfoData->infoData->devNum;
        infoQueryPara.busNum = pnpInfoData->infoData->busNum;
        *deviceInfo = UsbPnpNotifyFindInfo(infoQueryPara);
    } else if ((g_usbPnpNotifyCmdType == USB_PNP_NOTIFY_ADD_DEVICE)
        || (g_usbPnpNotifyCmdType == USB_PNP_NOTIFY_REMOVE_DEVICE)) {
        infoQueryPara.type = USB_INFO_DEVICE_ADDRESS_TYPE;
        infoQueryPara.usbDevAddr = (uintptr_t)pnpInfoData->usbDev;
        *deviceInfo = UsbPnpNotifyFindInfo(infoQueryPara);
    } else {
        *deviceInfo = UsbPnpNotifyCreateInfo();
    }

    if (*deviceInfo == NULL) {
        HDF_LOGE("%s:%d create or find info failed", __func__, __LINE__);
        return HDF_FAILURE;
    }

    (*deviceInfo)->info.removeType = g_usbPnpNotifyRemoveType;

    return HDF_SUCCESS;
}

static int32_t UsbPnpNotifyHdfSendEvent(const struct HdfDeviceObject *deviceObject,
    void *eventData)
{
    int32_t ret;
    struct UsbPnpDeviceInfo *deviceInfo = NULL;
    struct HdfSBuf *data = NULL;
    union UsbPnpDeviceInfoData pnpInfoData;

    if ((deviceObject == NULL) || (eventData == NULL)) {
        HDF_LOGE("%s deviceObject=%p or eventData=%p is NULL", __func__, deviceObject, eventData);
        return HDF_ERR_INVALID_PARAM;
    }

    data = HdfSBufObtainDefaultSize();
    if (data == NULL) {
        HDF_LOGE("%s:%d InitDataBlock failed", __func__, __LINE__);
        return HDF_FAILURE;
    }

    ret = UsbPnpNotifyGetDeviceInfo(eventData, &pnpInfoData, &deviceInfo);
    if (ret != HDF_SUCCESS) {
        HDF_LOGE("%s:%d UsbPnpNotifyInitInfo failed, ret=%d", __func__, __LINE__, ret);
        goto error_device_info;
    }

    ret = UsbPnpNotifyInitInfo(data, deviceInfo, pnpInfoData);
    if (ret != HDF_SUCCESS) {
        HDF_LOGE("%s:%d UsbPnpNotifyInitInfo failed, ret=%d", __func__, __LINE__, ret);
        goto out;
    }

    HDF_LOGI("%s:%d report one device information, %d usbDevAddr=0x%x, devNum=%d, busNum=%d, infoTable=%d-0x%x-0x%x!",
        __func__, __LINE__, g_usbPnpNotifyCmdType, (uint32_t)deviceInfo->info.usbDevAddr, deviceInfo->info.devNum,
        deviceInfo->info.busNum, deviceInfo->info.numInfos, deviceInfo->info.deviceInfo.vendorId,
        deviceInfo->info.deviceInfo.productId);

    OsalMutexLock(&deviceInfo->lock);
    if (deviceInfo->status == USB_PNP_DEVICE_INIT_STATUS) {
        ret = HdfDeviceSendEvent(deviceObject, g_usbPnpNotifyCmdType, data);
        if (ret != HDF_SUCCESS) {
            OsalMutexUnlock(&deviceInfo->lock);
            HDF_LOGE("%s:%d HdfDeviceSendEvent ret=%d", __func__, __LINE__, ret);
            goto out;
        }
        deviceInfo->status = USB_PNP_DEVICE_ADD_STATUS;
    } else {
        ret = HDF_ERR_INVALID_OBJECT;
    }
    OsalMutexUnlock(&deviceInfo->lock);

out:
    if ((ret != HDF_SUCCESS) || (g_usbPnpNotifyCmdType == USB_PNP_NOTIFY_REMOVE_DEVICE)) {
        if (UsbPnpNotifyDestroyInfo(deviceInfo) != HDF_SUCCESS) {
            HDF_LOGE("%s:%d UsbPnpNotifyDestroyInfo fail", __func__, __LINE__);
        }
    }
error_device_info:
    HdfSBufRecycle(data);
    return ret;
}

#if USB_PNP_NOTIFY_TEST_MODE == true
static void TestReadPnpInfo(struct HdfSBuf *data)
{
    uint32_t infoSize;
    bool flag;

    flag = HdfSbufReadBuffer(data, (const void **)(&g_testUsbPnpInfo), &infoSize);
    if ((flag == false) || (g_testUsbPnpInfo == NULL)) {
        HDF_LOGE("%s: fail to read g_testUsbPnpInfo, flag=%d, g_testUsbPnpInfo=%px", \
            __func__, flag, g_testUsbPnpInfo);
        return;
    }

    HDF_LOGI("%s:%d infoSize=%d g_testUsbPnpInfo=%px read success!", __func__, __LINE__, infoSize, g_testUsbPnpInfo);
}

static void TestPnpNotifyFillInfoTable(struct UsbPnpNotifyMatchInfoTable *infoTable)
{
    int8_t i;

    infoTable->usbDevAddr = g_testUsbPnpInfo->usbDevAddr;
    infoTable->devNum = g_testUsbPnpInfo->devNum;
    if (g_usbPnpNotifyCmdType == USB_PNP_NOTIFY_REMOVE_TEST) {
        infoTable->busNum = -1;
    } else {
        infoTable->busNum = g_testUsbPnpInfo->busNum;
    }

    infoTable->deviceInfo.vendorId = g_testUsbPnpInfo->deviceInfo.vendorId;
    infoTable->deviceInfo.productId = g_testUsbPnpInfo->deviceInfo.productId;
    infoTable->deviceInfo.bcdDeviceLow = g_testUsbPnpInfo->deviceInfo.bcdDeviceLow;
    infoTable->deviceInfo.bcdDeviceHigh = g_testUsbPnpInfo->deviceInfo.bcdDeviceHigh;
    infoTable->deviceInfo.deviceClass = g_testUsbPnpInfo->deviceInfo.deviceClass;
    infoTable->deviceInfo.deviceSubClass = g_testUsbPnpInfo->deviceInfo.deviceSubClass;
    infoTable->deviceInfo.deviceProtocol = g_testUsbPnpInfo->deviceInfo.deviceProtocol;

    infoTable->removeType = g_usbPnpNotifyRemoveType;

    if (g_usbPnpNotifyCmdType != USB_PNP_NOTIFY_REMOVE_TEST) {
        infoTable->numInfos = g_testUsbPnpInfo->numInfos;
        for (i = 0; i < infoTable->numInfos; i++) {
            infoTable->interfaceInfo[i].interfaceClass = g_testUsbPnpInfo->interfaceInfo[i].interfaceClass;
            infoTable->interfaceInfo[i].interfaceSubClass = g_testUsbPnpInfo->interfaceInfo[i].interfaceSubClass;
            infoTable->interfaceInfo[i].interfaceProtocol = g_testUsbPnpInfo->interfaceInfo[i].interfaceProtocol;
            infoTable->interfaceInfo[i].interfaceNumber = g_testUsbPnpInfo->interfaceInfo[i].interfaceNumber;
        }
    }
}

static int32_t TestPnpNotifyHdfSendEvent(const struct HdfDeviceObject *deviceObject)
{
    int32_t ret;
    struct UsbPnpNotifyMatchInfoTable infoTable;
    struct HdfSBuf *data = NULL;

    if ((deviceObject == NULL) || (g_testUsbPnpInfo == NULL)) {
        HDF_LOGE("%s deviceObject=%px or g_testUsbPnpInfo=%px is NULL", __func__, deviceObject, g_testUsbPnpInfo);
        return HDF_ERR_INVALID_PARAM;
    }

    data = HdfSBufObtainDefaultSize();
    if (data == NULL) {
        HDF_LOGE("%s InitDataBlock failed", __func__);
        return HDF_FAILURE;
    }

    TestPnpNotifyFillInfoTable(&infoTable);

    if (!HdfSbufWriteBuffer(data, (const void *)(&infoTable), sizeof(struct UsbPnpNotifyMatchInfoTable))) {
        HDF_LOGE("%s: sbuf write infoTable failed", __func__);
        goto out;
    }

    HDF_LOGI("%s: report one device information, %d usbDev=0x%x, devNum=%d, busNum=%d, infoTable=%d-0x%x-0x%x!", \
        __func__, g_usbPnpNotifyCmdType, (uint32_t)infoTable.usbDevAddr, infoTable.devNum, infoTable.busNum, \
        infoTable.numInfos, infoTable.deviceInfo.vendorId, infoTable.deviceInfo.productId);

    ret = HdfDeviceSendEvent(deviceObject, g_usbPnpNotifyCmdType, data);
    if (ret != HDF_SUCCESS) {
        HDF_LOGE("%s: HdfDeviceSendEvent error=%d", __func__, ret);
        goto out;
    }

    HdfSBufRecycle(data);
    return ret;

out:
    HdfSBufRecycle(data);
    return HDF_FAILURE;
}
#endif

static int UsbPnpNotifyFirstReport(struct usb_device *usbDev, void *data)
{
    int ret;
    struct HdfDeviceIoClient *client = (struct HdfDeviceIoClient *)data;

    ret = UsbPnpNotifyHdfSendEvent(client->device, usbDev);

    HDF_LOGI("%s: report all device information!", __func__);

    return ret;
}

static int UsbPnpNotifyReportThread(void* arg)
{
    int ret;
    struct HdfDeviceObject *deviceObject = (struct HdfDeviceObject *)arg;

    while (!kthread_should_stop()) {
#if USB_PNP_NOTIFY_TEST_MODE == false
        ret = wait_event_interruptible(g_usbPnpNotifyReportWait, g_usbDevice != NULL);
#else
        ret = wait_event_interruptible(g_usbPnpNotifyReportWait,
            ((g_usbDevice != NULL) || (g_testUsbPnpInfo != NULL)));
#endif
        if (!ret) {
            HDF_LOGI("%s: UsbPnpNotifyReportThread start!", __func__);
        }

        OsalMutexLock(&g_usbSendEventLock);
#if USB_PNP_NOTIFY_TEST_MODE == true
        if ((g_usbPnpNotifyCmdType == USB_PNP_NOTIFY_ADD_TEST) || \
            (g_usbPnpNotifyCmdType == USB_PNP_NOTIFY_REMOVE_TEST)) {
            ret = TestPnpNotifyHdfSendEvent(deviceObject);
        } else {
#endif
            if ((g_usbPnpNotifyCmdType == USB_PNP_NOTIFY_ADD_INTERFACE)
                || (g_usbPnpNotifyCmdType == USB_PNP_NOTIFY_REMOVE_INTERFACE)) {
                ret = UsbPnpNotifyHdfSendEvent(deviceObject, &g_usbPnpInfo);
            } else {
                ret = UsbPnpNotifyHdfSendEvent(deviceObject, g_usbDevice);
            }
#if USB_PNP_NOTIFY_TEST_MODE == true
        }
#endif
        if (ret != HDF_SUCCESS) {
            HDF_LOGI("%s: UsbPnpNotifyHdfSendEvent error=%d!", __func__, ret);
        }

        g_usbDevice = NULL;
#if USB_PNP_NOTIFY_TEST_MODE == true
        g_testUsbPnpInfo = NULL;
#endif
        OsalMutexUnlock(&g_usbSendEventLock);

        msleep(USB_PNP_NOTIFY_MSLEEP_TIME);
    }

    HDF_LOGI("%s: usb pnp notify handle kthread exiting!", __func__);

    return 0;
}

static int UsbPnpNotifyCallback(struct notifier_block *self, unsigned long action, void *dev)
{
    int32_t ret;
    struct UsbInfoQueryPara infoQueryPara;
    struct UsbPnpDeviceInfo *deviceInfo = NULL;
    union UsbPnpDeviceInfoData pnpInfoData;

    HDF_LOGI("%s: action=0x%lx", __func__, action);

    switch (action) {
        case USB_DEVICE_ADD:
            pnpInfoData.usbDev = dev;
            deviceInfo = UsbPnpNotifyCreateInfo();
            if (deviceInfo == NULL) {
                HDF_LOGE("%s:%d USB_DEVICE_ADD create info failed", __func__, __LINE__);
            } else {
                ret = UsbPnpNotifyAddInitInfo(deviceInfo, pnpInfoData);
                if (ret != HDF_SUCCESS) {
                    HDF_LOGE("%s:%d USB_DEVICE_ADD UsbPnpNotifyAddInitInfo error", __func__, __LINE__);
                } else {
                    OsalMutexLock(&g_usbSendEventLock);
                    g_usbDevice = dev;
                    g_usbPnpNotifyCmdType = USB_PNP_NOTIFY_ADD_DEVICE;
                    OsalMutexUnlock(&g_usbSendEventLock);
                    wake_up_interruptible(&g_usbPnpNotifyReportWait);
                }
            }
            break;
        case USB_DEVICE_REMOVE:
            infoQueryPara.type = USB_INFO_DEVICE_ADDRESS_TYPE;
            infoQueryPara.usbDevAddr = (uintptr_t)dev;
            deviceInfo = UsbPnpNotifyFindInfo(infoQueryPara);
            if (deviceInfo == NULL) {
                HDF_LOGE("%s:%d USB_DEVICE_REMOVE find info failed", __func__, __LINE__);
            } else {
                OsalMutexLock(&deviceInfo->lock);
                if (deviceInfo->status != USB_PNP_DEVICE_INIT_STATUS) {
                    deviceInfo->status = USB_PNP_DEVICE_INIT_STATUS;
                } else {
                    deviceInfo->status = USB_PNP_DEVICE_REMOVE_STATUS;
                }
                OsalMutexUnlock(&deviceInfo->lock);
                OsalMutexLock(&g_usbSendEventLock);
                g_usbDevice = dev;
                g_usbPnpNotifyCmdType = USB_PNP_NOTIFY_REMOVE_DEVICE;
                g_usbPnpNotifyRemoveType = USB_PNP_NOTIFY_REMOVE_BUS_DEV_NUM;
                OsalMutexUnlock(&g_usbSendEventLock);
                wake_up_interruptible(&g_usbPnpNotifyReportWait);
            }
            break;
        default:
            HDF_LOGI("%s: the action = 0x%lx is not defined!", __func__, action);
            break;
    }

    return NOTIFY_OK;
}

static struct notifier_block g_usbPnpNotifyNb = {
    .notifier_call =    UsbPnpNotifyCallback,
};

static void UsbPnpNotifyReadPnpInfo(struct HdfSBuf *data)
{
    struct UsbInfoQueryPara infoQueryPara;
    struct UsbPnpDeviceInfo *deviceInfo = NULL;
    struct UsbPnpAddRemoveInfo *usbPnpInfo = NULL;
    uint32_t infoSize;
    bool flag;

    flag = HdfSbufReadBuffer(data, (const void **)(&usbPnpInfo), &infoSize);
    if ((flag == false) || (usbPnpInfo == NULL)) {
        HDF_LOGE("%s: fail to read g_usbPnpInfo, flag=%d, usbPnpInfo=%px", \
            __func__, flag, usbPnpInfo);
        return;
    }

    g_usbPnpInfo.devNum = usbPnpInfo->devNum;
    g_usbPnpInfo.busNum = usbPnpInfo->busNum;
    g_usbPnpInfo.interfaceNumber = usbPnpInfo->interfaceNumber;
    g_usbPnpInfo.interfaceClass = usbPnpInfo->interfaceClass;
    g_usbPnpInfo.interfaceSubClass = usbPnpInfo->interfaceSubClass;
    g_usbPnpInfo.interfaceProtocol = usbPnpInfo->interfaceProtocol;

    g_usbDevice = (struct usb_device *)&g_usbPnpInfo;

    infoQueryPara.type = USB_INFO_NORMAL_TYPE;
    infoQueryPara.devNum = usbPnpInfo->devNum;
    infoQueryPara.busNum = usbPnpInfo->busNum;
    deviceInfo = UsbPnpNotifyFindInfo(infoQueryPara);
    if (deviceInfo == NULL) {
        HDF_LOGE("%s:%d add or remove interface find info failed", __func__, __LINE__);
    } else {
        OsalMutexLock(&deviceInfo->lock);
        if (deviceInfo->status != USB_PNP_DEVICE_INIT_STATUS) {
            deviceInfo->status = USB_PNP_DEVICE_INIT_STATUS;
        } else {
            deviceInfo->status = USB_PNP_DEVICE_INTERFACE_STATUS;
        }
        OsalMutexUnlock(&deviceInfo->lock);
    }

    HDF_LOGI("%s:%d infoSize=%d g_usbPnpInfo=%px-%d-%d-%d-%d-%d-%d read success!",
        __func__, __LINE__, infoSize, &g_usbPnpInfo, g_usbPnpInfo.devNum, g_usbPnpInfo.busNum,
        g_usbPnpInfo.interfaceNumber, g_usbPnpInfo.interfaceClass, g_usbPnpInfo.interfaceSubClass,
        g_usbPnpInfo.interfaceProtocol);
}

static int32_t UsbPnpNotifyDispatch(struct HdfDeviceIoClient *client, int cmd,
    struct HdfSBuf *data, struct HdfSBuf *reply)
{
    int32_t ret = HDF_SUCCESS;

    HDF_LOGI("%s: received cmd = %d", __func__, cmd);

    OsalMutexLock(&g_usbSendEventLock);
    g_usbPnpNotifyCmdType = cmd;

    switch (cmd) {
        case USB_PNP_NOTIFY_ADD_INTERFACE:
            UsbPnpNotifyReadPnpInfo(data);
            wake_up_interruptible(&g_usbPnpNotifyReportWait);
            break;
        case USB_PNP_NOTIFY_REMOVE_INTERFACE:
            UsbPnpNotifyReadPnpInfo(data);
            g_usbPnpNotifyRemoveType = USB_PNP_NOTIFY_REMOVE_INTERFACE_NUM;
            wake_up_interruptible(&g_usbPnpNotifyReportWait);
            break;
        case USB_PNP_NOTIFY_REPORT_INTERFACE:
            usb_for_each_dev((void *)client, UsbPnpNotifyFirstReport);
            break;
#if USB_PNP_NOTIFY_TEST_MODE == true
        case USB_PNP_NOTIFY_ADD_TEST:
            TestReadPnpInfo(data);
            wake_up_interruptible(&g_usbPnpNotifyReportWait);
            break;
        case USB_PNP_NOTIFY_REMOVE_TEST:
            TestReadPnpInfo(data);
            g_usbPnpNotifyRemoveType = g_testUsbPnpInfo->removeType;
            wake_up_interruptible(&g_usbPnpNotifyReportWait);
            break;
#endif
        default:
            ret = HDF_ERR_NOT_SUPPORT;
            break;
    }
    OsalMutexUnlock(&g_usbSendEventLock);

    if (!HdfSbufWriteInt32(reply, INT32_MAX)) {
        HDF_LOGE("%s: reply int32 fail", __func__);
    }

    return ret;
}

static int32_t UsbPnpNotifyBind(struct HdfDeviceObject *device)
{
    static struct IDeviceIoService pnpNotifyService = {
        .Dispatch = UsbPnpNotifyDispatch,
    };

    HDF_LOGI("%s: Enter!", __func__);

    if (device == NULL) {
        HDF_LOGE("%s: device is NULL!", __func__);
        return HDF_ERR_INVALID_OBJECT;
    }

    device->service = &pnpNotifyService;

    return HDF_SUCCESS;
}

static int32_t UsbPnpNotifyInit(struct HdfDeviceObject *device)
{
    static bool firstInitFlag = true;

    HDF_LOGI("%s: Enter!", __func__);

    if (device == NULL) {
        HDF_LOGE("%s: device is NULL", __func__);
        return HDF_ERR_INVALID_OBJECT;
    }

    if (firstInitFlag == true) {
        firstInitFlag = false;

        DListHeadInit(&g_usbPnpInfoListHead);
    }

    init_waitqueue_head(&g_usbPnpNotifyReportWait);

    OsalMutexInit(&g_usbSendEventLock);

    /* Add a new notify for usb pnp notify module. */
    usb_register_notify(&g_usbPnpNotifyNb);

    /* Creat thread to handle send usb interface information. */
    if (NULL == g_usbPnpNotifyReportThread) {
        g_usbPnpNotifyReportThread = kthread_run(UsbPnpNotifyReportThread,
            (void *)device, "usb pnp notify handle kthread");
    } else {
        HDF_LOGI("%s: g_usbPnpNotifyReportThread is already running!", __func__);
    }

    return HDF_SUCCESS;
}

static void UsbPnpNotifyRelease(struct HdfDeviceObject *device)
{
    if (device == NULL) {
        HDF_LOGI("%s: device is null", __func__);
        return;
    }

    if (g_usbPnpNotifyReportThread == NULL) {
        HDF_LOGI("%s: g_usbPnpNotifyReportThread is not running!", __func__);
    } else {
        kthread_stop(g_usbPnpNotifyReportThread);
    }

    usb_unregister_notify(&g_usbPnpNotifyNb);

    OsalMutexDestroy(&g_usbSendEventLock);

    HDF_LOGI("%s: release done!", __func__);

    return;
}

struct HdfDriverEntry g_usbPnpNotifyEntry = {
    .moduleVersion = 1,
    .Bind = UsbPnpNotifyBind,
    .Init = UsbPnpNotifyInit,
    .Release = UsbPnpNotifyRelease,
    .moduleName = "HDF_USB_PNP_NOTIFY",
};

HDF_INIT(g_usbPnpNotifyEntry);
