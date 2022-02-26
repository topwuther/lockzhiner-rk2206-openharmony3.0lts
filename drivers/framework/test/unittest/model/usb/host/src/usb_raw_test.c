/*
 * Copyright (c) 2020-2021 Huawei Device Co., Ltd.
 *
 * HDF is dual licensed: you can use it either under the terms of
 * the GPL, or the BSD license, at your option.
 * See the LICENSE file in the root of this repository for complete details.
 */

#include "usb_raw_test.h"
#include "data_fifo.h"
#include "device_resource_if.h"
#include "hdf_base.h"
#include "hdf_device_desc.h"
#include "hdf_dlist.h"
#include "hdf_log.h"
#include "osal_mem.h"
#include "osal_time.h"
#include "usb_interface.h"
#ifdef LOSCFG_DRIVERS_HDF_USB_PNP_NOTIFY
#include "usb_pnp_notify.h"
#endif

#define HDF_LOG_TAG usb_raw_test_c

int32_t CheckRawSdkIfGetDeviceDescriptor001(void)
{
    struct UsbDeviceDescriptor desc;
    int ret;
    ret = UsbRawGetDeviceDescriptor(NULL, &desc);
    if (ret != HDF_ERR_INVALID_PARAM) {
        HDF_LOGE("%s: error", __func__);
        return HDF_FAILURE;
    }
    HDF_LOGE("%s: success", __func__);
    return HDF_SUCCESS;
}

int32_t CheckRawSdkIfClaimInterface001(void)
{
    int ret;
    int interfaceNumber = 1;

    ret = UsbRawClaimInterface(NULL, interfaceNumber);
    if (ret != HDF_ERR_INVALID_PARAM) {
        HDF_LOGE("%s: error", __func__);
        return HDF_FAILURE;
    }
    HDF_LOGE("%s: success", __func__);
    return HDF_SUCCESS;
}

int32_t CheckRawSdkIfClaimInterface005(void)
{
    int ret;

    ret = UsbParseConfigDescriptor(g_acm, g_acm->config);
    if (ret) {
        HDF_LOGE("%s: error", __func__);
        return HDF_FAILURE;
    }
    HDF_LOGE("%s: success", __func__);
    return HDF_SUCCESS;
}

int32_t CheckRawSdkIfReleaseInterface001(void)
{
    int ret;

    ret = UsbRawReleaseInterface(NULL, g_acm->ctrlIface);
    if (ret != HDF_ERR_INVALID_PARAM) {
        HDF_LOGE("%s: error", __func__);
        return HDF_FAILURE;
    }
    HDF_LOGE("%s: success", __func__);
    return HDF_SUCCESS;
}

int32_t CheckRawSdkIfReleaseInterface002(void)
{
    int ret;

    ret = UsbRawReleaseInterface(g_acm->devHandle, g_acm->ctrlIface);
    if (ret) {
        HDF_LOGE("%s: error", __func__);
        return HDF_FAILURE;
    }
    HDF_LOGE("%s: success", __func__);
    return HDF_SUCCESS;
}

int32_t CheckRawSdkIfReleaseInterface003(void)
{
    int ret;

    ret = UsbRawReleaseInterface(NULL, g_acm->dataIface);
    if (ret != HDF_ERR_INVALID_PARAM) {
        HDF_LOGE("%s: error", __func__);
        return HDF_FAILURE;
    }
    HDF_LOGE("%s: success", __func__);
    return HDF_SUCCESS;
}

int32_t CheckRawSdkIfReleaseInterface004(void)
{
    int ret;

    ret = UsbRawReleaseInterface(g_acm->devHandle, g_acm->dataIface);
    if (ret) {
        HDF_LOGE("%s: error", __func__);
        return HDF_FAILURE;
    }
    HDF_LOGE("%s: success", __func__);
    return HDF_SUCCESS;
}

int32_t CheckRawSdkIfClaimInterface006(void)
{
    int ret;

    ret = UsbParseConfigDescriptor(g_acm, g_acm->config);
    if (ret) {
        HDF_LOGE("%s: error", __func__);
        return HDF_FAILURE;
    }
    HDF_LOGE("%s: success", __func__);
    return HDF_SUCCESS;
}

int32_t CheckRawSdkIfAllocRequest001(void)
{
    int i;
    int ret;

    ret = AcmWriteBufAlloc(g_acm);
    if (ret) {
        HDF_LOGE("%s: error", __func__);
        return HDF_FAILURE;
    }

    for (i = 0; i < ACM_NW; i++) {
        g_acm->wb[i].request = UsbRawAllocRequest(NULL, 0, g_acm->dataOutEp.maxPacketSize);
        g_acm->wb[i].instance = g_acm;
        if (g_acm->wb[i].request) {
            HDF_LOGE("%s: error", __func__);
            return HDF_FAILURE;
        }
    }
    HDF_LOGE("%s: success", __func__);
    return HDF_SUCCESS;
}

int32_t CheckRawSdkIfAllocRequest002(void)
{
    int i;
    int ret;

    ret = AcmWriteBufAlloc(g_acm);
    if (ret) {
        HDF_LOGE("%s: error", __func__);
        return HDF_FAILURE;
    }

    for (i = 0; i < ACM_NW; i++) {
        g_acm->wb[i].request = UsbRawAllocRequest(g_acm->devHandle, 0, g_acm->dataOutEp.maxPacketSize);
        g_acm->wb[i].instance = g_acm;
        ((struct UsbHostRequest *)(g_acm->wb[i].request))->devHandle = (struct UsbDeviceHandle *)g_acm->devHandle;
        if (g_acm->wb[i].request == NULL) {
            HDF_LOGE("%s: error", __func__);
            return HDF_FAILURE;
        }
    }
    HDF_LOGE("%s: success", __func__);
    return HDF_SUCCESS;
}

int32_t CheckRawSdkIfAllocRequest003(void)
{
    int i;

    for (i = 0; i < ACM_NR; i++) {
        g_acm->readReq[i] = UsbRawAllocRequest(NULL, 0, g_acm->dataInEp.maxPacketSize);
        if (g_acm->readReq[i]) {
            HDF_LOGE("%s: error", __func__);
            return HDF_FAILURE;
        }
    }
    HDF_LOGE("%s: success", __func__);
    return HDF_SUCCESS;
}

int32_t CheckRawSdkIfAllocRequest004(void)
{
    int i;

    for (i = 0; i < ACM_NR; i++) {
        g_acm->readReq[i] = UsbRawAllocRequest(g_acm->devHandle, 0, g_acm->dataInEp.maxPacketSize);
        ((struct UsbHostRequest *)(g_acm->readReq[i]))->devHandle = (struct UsbDeviceHandle *)g_acm->devHandle;
        if (g_acm->readReq[i] == NULL) {
            HDF_LOGE("%s: error", __func__);
            return HDF_FAILURE;
        }
    }
    HDF_LOGE("%s: success", __func__);
    return HDF_SUCCESS;
}

int32_t CheckRawSdkIfAllocRequest005(void)
{
    g_acm->ctrlReq = UsbRawAllocRequest(NULL, 0, USB_CTRL_REQ_SIZE);
    if (g_acm->ctrlReq) {
        HDF_LOGE("%s: error", __func__);
        return HDF_FAILURE;
    }
    HDF_LOGE("%s: success", __func__);
    return HDF_SUCCESS;
}

int32_t CheckRawSdkIfAllocRequest006(void)
{
    g_acm->ctrlReq = UsbRawAllocRequest(g_acm->devHandle, 0, USB_CTRL_REQ_SIZE);
    ((struct UsbHostRequest *)(g_acm->ctrlReq))->devHandle = (struct UsbDeviceHandle *)g_acm->devHandle;
    if (g_acm->ctrlReq == NULL) {
        HDF_LOGE("%s: error", __func__);
        return HDF_FAILURE;
    }
    HDF_LOGE("%s: success", __func__);
    return HDF_SUCCESS;
}

int32_t CheckRawSdkIfAllocRequest007(void)
{
    g_acm->notifyReq = UsbRawAllocRequest(NULL, 0, g_acm->notifyEp.maxPacketSize);
    if (g_acm->notifyReq) {
        HDF_LOGE("%s: error", __func__);
        return HDF_FAILURE;
    }
    HDF_LOGE("%s: success", __func__);
    return HDF_SUCCESS;
}

int32_t CheckRawSdkIfAllocRequest008(void)
{
    g_acm->notifyReq = UsbRawAllocRequest(g_acm->devHandle, 0, g_acm->notifyEp.maxPacketSize);
    ((struct UsbHostRequest *)(g_acm->notifyReq))->devHandle = (struct UsbDeviceHandle *)g_acm->devHandle;
    if (g_acm->notifyReq == NULL) {
        HDF_LOGE("%s: error", __func__);
        return HDF_FAILURE;
    }
    HDF_LOGE("%s: success", __func__);
    return HDF_SUCCESS;
}
int32_t CheckRawSdkIfAllocRequest010(void)
{
    g_acm->isoReq = UsbRawAllocRequest(NULL, USB_ISO_PACKAT_CNT, g_acm->isoEp.maxPacketSize);
    if (g_acm->isoReq == NULL) {
        HDF_LOGE("%s: error", __func__);
        return HDF_FAILURE;
    }
    ((struct UsbHostRequest *)(g_acm->isoReq))->devHandle = (struct UsbDeviceHandle *)g_acm->devHandle;
    HDF_LOGE("%s: success", __func__);
    return HDF_SUCCESS;
}
int32_t CheckRawSdkIfAllocRequest011(void)
{
    g_acm->isoReq = UsbRawAllocRequest(g_acm->devHandle, USB_ISO_PACKAT_CNT, g_acm->isoEp.maxPacketSize);
    if (g_acm->isoReq == NULL) {
        HDF_LOGE("%s: error", __func__);
        return HDF_FAILURE;
    }
    ((struct UsbHostRequest *)(g_acm->isoReq))->devHandle = (struct UsbDeviceHandle *)g_acm->devHandle;
    HDF_LOGE("%s: success", __func__);
    return HDF_SUCCESS;
}

int32_t CheckRawSdkIfFreeRequest006(void)
{
    int32_t ret;

    ret = UsbRawFreeRequest(g_acm->isoReq);
    if (ret != HDF_ERR_INVALID_PARAM) {
        HDF_LOGE("%s: error", __func__);
        return HDF_FAILURE;
    }
    g_acm->isoReq = NULL;
    HDF_LOGE("%s: success", __func__);
    return HDF_SUCCESS;
}

int32_t CheckRawSdkIfFillIsoRequest001(void)
{
    struct UsbRawFillRequestData reqData;
    int32_t ret;
    int i;
    uint32_t size;
    char sendData[] = {"abcde\0"};
    size = strlen(sendData) + 1;
    printf("---size:%d\n", size);
    size = (size > g_acm->isoEp.maxPacketSize) ? g_acm->isoEp.maxPacketSize : size;
    for (i = 0; i < 1; i++) {
        reqData.endPoint      = g_acm->isoEp.addr;
        reqData.numIsoPackets = USB_ISO_PACKAT_CNT;
        reqData.callback      = AcmWriteIsoCallback;
        reqData.timeout       = USB_CTRL_SET_TIMEOUT;
        reqData.buffer        = (unsigned char*)sendData;
        reqData.length        = size;
        ret = UsbRawFillIsoRequest(g_acm->isoReq, g_acm->devHandle, &reqData);
        if (ret) {
            printf("%s: error++ret=%d\n", __func__, ret);
            return HDF_FAILURE;
        }
    }
    HDF_LOGE("%s: success", __func__);
    return HDF_SUCCESS;
}

int32_t CheckRawSdkIfFillIsoRequest002(void)
{
    struct UsbRawFillRequestData reqData;
    int32_t ret;
    int i;
    uint32_t size;
    char sendData[] = {"abcde\0"};
    size = strlen(sendData) + 1;
    printf("---size:%d\n", size);
    size = (size > g_acm->isoEp.maxPacketSize) ? g_acm->isoEp.maxPacketSize : size;
    for (i = 0; i < 1; i++) {
        reqData.endPoint      = g_acm->isoEp.addr;
        reqData.numIsoPackets = USB_ISO_PACKAT_CNT;
        reqData.callback      = AcmWriteIsoCallback;
        reqData.timeout       = USB_CTRL_SET_TIMEOUT;
        reqData.buffer        = (unsigned char*)sendData;
        reqData.length        = size;
        ret = UsbRawFillIsoRequest(NULL, g_acm->devHandle, &reqData);
        if (ret) {
            printf("%s: error++ret=%d\n", __func__, ret);
            return HDF_FAILURE;
        }
    }
    HDF_LOGE("%s: success", __func__);
    return HDF_SUCCESS;
}

int32_t CheckRawSdkIfFillIsoRequest003(void)
{
    struct UsbRawFillRequestData reqData;
    int32_t ret;
    int i;
    uint32_t size;
    char sendData[] = {"abcde\0"};
    size = strlen(sendData) + 1;
    printf("---size:%d\n", size);
    size = (size > g_acm->isoEp.maxPacketSize) ? g_acm->isoEp.maxPacketSize : size;
    for (i = 0; i < 1; i++) {
        reqData.endPoint      = g_acm->isoEp.addr;
        reqData.numIsoPackets = USB_ISO_PACKAT_CNT;
        reqData.callback      = AcmWriteIsoCallback;
        reqData.timeout       = USB_CTRL_SET_TIMEOUT;
        reqData.buffer        = (unsigned char*)sendData;
        reqData.length        = size;
        ret = UsbRawFillIsoRequest(g_acm->isoReq, g_acm->devHandle, NULL);
        if (ret) {
            printf("%s: error++ret=%d\n", __func__, ret);
            return HDF_FAILURE;
        }
    }
    HDF_LOGE("%s: success", __func__);
    return HDF_SUCCESS;
}

int32_t CheckRawSdkIfFillIsoRequest004(void)
{
    struct UsbRawFillRequestData reqData;
    int32_t ret;
    int i;
    uint32_t size;
    char sendData[] = {"abcde\0"};
    size = strlen(sendData) + 1;
    printf("---size:%d\n", size);
    size = (size > g_acm->isoEp.maxPacketSize) ? g_acm->isoEp.maxPacketSize : size;
    for (i = 0; i < 1; i++) {
        reqData.endPoint      = g_acm->isoEp.addr;
        reqData.numIsoPackets = USB_ISO_PACKAT_CNT;
        reqData.callback      = AcmWriteIsoCallback;
        reqData.timeout       = USB_CTRL_SET_TIMEOUT;
        reqData.buffer        = (unsigned char*)sendData;
        reqData.length        = size;
        ret = UsbRawFillIsoRequest(g_acm->isoReq, NULL, &reqData);
        if (ret) {
            printf("%s: error++ret=%d\n", __func__, ret);
            return HDF_FAILURE;
        }
    }
    HDF_LOGE("%s: success", __func__);
    return HDF_SUCCESS;
}

int32_t CheckRawSdkIfFillIsoRequest005(void)
{
    struct UsbRawFillRequestData reqData;
    int32_t ret;
    int i;
    uint32_t size;
    char sendData[] = {"abcde\0"};
    size = strlen(sendData) + 1;
    printf("---size:%d\n", size);
    size = (size > g_acm->isoEp.maxPacketSize) ? g_acm->isoEp.maxPacketSize : size;
    for (i = 0; i < 1; i++) {
        reqData.endPoint      = g_acm->isoEp.addr;
        reqData.numIsoPackets = USB_ISO_PACKAT_CNT;
        reqData.callback      = AcmWriteIsoCallback;
        reqData.timeout       = USB_CTRL_SET_TIMEOUT;
        reqData.buffer        = (unsigned char*)sendData;
        reqData.length        = size;
        ret = UsbRawFillIsoRequest(NULL, NULL, &reqData);
        if (ret) {
            printf("%s: error++ret=%d\n", __func__, ret);
            return HDF_FAILURE;
        }
    }
    HDF_LOGE("%s: success", __func__);
    return HDF_SUCCESS;
}

int32_t CheckRawSdkIfFillIsoRequest006(void)
{
    struct UsbRawFillRequestData reqData;
    int32_t ret;
    int i;
    uint32_t size;
    char sendData[] = {"abcde\0"};
    size = strlen(sendData) + 1;
    printf("---size:%d\n", size);
    size = (size > g_acm->isoEp.maxPacketSize) ? g_acm->isoEp.maxPacketSize : size;
    for (i = 0; i < 1; i++) {
        reqData.endPoint      = g_acm->isoEp.addr;
        reqData.numIsoPackets = USB_ISO_PACKAT_CNT;
        reqData.callback      = AcmWriteIsoCallback;
        reqData.timeout       = USB_CTRL_SET_TIMEOUT;
        reqData.buffer        = (unsigned char*)sendData;
        reqData.length        = size;
        ret = UsbRawFillIsoRequest(NULL, NULL, NULL);
        if (ret) {
            printf("%s: error++ret=%d\n", __func__, ret);
            return HDF_FAILURE;
        }
    }
    HDF_LOGE("%s: success", __func__);
    return HDF_SUCCESS;
}
int32_t CheckRawSdkIfFreeRequest001(void)
{
    int32_t ret;
    int i;

    for (i = 0; i < ACM_NW; i++) {
        ret = UsbRawFreeRequest(g_acm->wb[i].request);
        if (ret) {
            HDF_LOGE("%s: error", __func__);
            return HDF_FAILURE;
        }
        g_acm->wb[i].request = NULL;
    }
    AcmWriteBufFree(g_acm);
    HDF_LOGE("%s: success", __func__);
    return HDF_SUCCESS;
}

int32_t CheckRawSdkIfFreeRequest002(void)
{
    int32_t ret;
    int i;

    for (i = 0; i < ACM_NW; i++) {
        ret = UsbRawFreeRequest(g_acm->readReq[i]);
        if (ret) {
            HDF_LOGE("%s: error", __func__);
            return HDF_FAILURE;
        }
        g_acm->readReq[i] = NULL;
    }
    HDF_LOGE("%s: success", __func__);
    return HDF_SUCCESS;
}

int32_t CheckRawSdkIfFreeRequest003(void)
{
    int32_t ret;

    ret = UsbRawFreeRequest(g_acm->ctrlReq);
    if (ret) {
        HDF_LOGE("%s: error", __func__);
        return HDF_FAILURE;
    }
    g_acm->ctrlReq = NULL;
    HDF_LOGE("%s: success", __func__);
    return HDF_SUCCESS;
}

int32_t CheckRawSdkIfFreeRequest004(void)
{
    int32_t ret;

    ret = UsbRawFreeRequest(g_acm->notifyReq);
    if (ret) {
        HDF_LOGE("%s: error", __func__);
        return HDF_FAILURE;
    }
    g_acm->notifyReq = NULL;
    HDF_LOGE("%s: success", __func__);
    return HDF_SUCCESS;
}

int32_t CheckRawSdkIfFreeRequest005(void)
{
    int32_t ret;

    ret = UsbRawFreeRequest(NULL);
    if (ret != HDF_ERR_INVALID_PARAM) {
        HDF_LOGE("%s: error", __func__);
        return HDF_FAILURE;
    }
    HDF_LOGE("%s: success", __func__);
    return HDF_SUCCESS;
}

int32_t CheckRawSdkIfAllocRequest009(void)
{
    int i;
    int ret;

    ret = AcmWriteBufAlloc(g_acm);
    if (ret) {
        HDF_LOGE("%s: error", __func__);
        return HDF_FAILURE;
    }

    for (i = 0; i < ACM_NW; i++) {
        g_acm->wb[i].request = UsbRawAllocRequest(g_acm->devHandle, 0, g_acm->dataOutEp.maxPacketSize);
        g_acm->wb[i].instance = g_acm;
        if (g_acm->wb[i].request == NULL) {
            HDF_LOGE("%s: error", __func__);
            return HDF_FAILURE;
        }
    }

    for (i = 0; i < ACM_NR; i++) {
        g_acm->readReq[i] = UsbRawAllocRequest(g_acm->devHandle, 0, g_acm->dataInEp.maxPacketSize);
        if (g_acm->readReq[i] == NULL) {
            HDF_LOGE("%s: error", __func__);
            return HDF_FAILURE;
        }
    }

    g_acm->ctrlReq = UsbRawAllocRequest(g_acm->devHandle, 0, USB_CTRL_REQ_SIZE);
    if (g_acm->ctrlReq == NULL) {
        HDF_LOGE("%s: error", __func__);
        return HDF_FAILURE;
    }

    g_acm->notifyReq = UsbRawAllocRequest(g_acm->devHandle, 0, g_acm->notifyEp.maxPacketSize);
    if (g_acm->notifyReq == NULL) {
        HDF_LOGE("%s: error", __func__);
        return HDF_FAILURE;
    }
    HDF_LOGE("%s: success", __func__);
    return HDF_SUCCESS;
}

int32_t CheckRawSdkIfGetDescriptor001(void)
{
    struct UsbRawDescriptorParam param;
    unsigned char *data = NULL;
    int ret;

    data = OsalMemCalloc(USB_BUFFER_MAX_SIZE);
    if (data == NULL) {
        HDF_LOGE("%s:%d OsalMemCalloc error", __func__, __LINE__);
        return HDF_ERR_MALLOC_FAIL;
    }

    ret = UsbRawGetDescriptor(NULL, g_acm->devHandle, &param, data);
    if (ret != HDF_ERR_INVALID_PARAM) {
        HDF_LOGE("%s: error", __func__);
        ret = HDF_FAILURE;
        goto error;
    }
    HDF_LOGE("%s: success", __func__);
    ret = HDF_SUCCESS;
error:
    OsalMemFree(data);
    data = NULL;

    return ret;
}

int32_t CheckRawSdkIfGetDescriptor002(void)
{
    struct UsbRawDescriptorParam param;
    unsigned char *data = NULL;
    int ret;

    data = OsalMemCalloc(USB_BUFFER_MAX_SIZE);
    if (data == NULL) {
        HDF_LOGE("%s:%d OsalMemCalloc error", __func__, __LINE__);
        return HDF_ERR_MALLOC_FAIL;
    }

    ret = UsbRawGetDescriptor(g_acm->ctrlReq, NULL, &param, data);
    if (ret != HDF_ERR_INVALID_PARAM) {
        HDF_LOGE("%s: error", __func__);
        ret = HDF_FAILURE;
        goto error;
    }
    HDF_LOGE("%s: success", __func__);
    ret = HDF_SUCCESS;
error:
    OsalMemFree(data);
    data = NULL;

    return ret;
}

int32_t CheckRawSdkIfGetDescriptor003(void)
{
    struct UsbRawDescriptorParam param;
    unsigned char *data = NULL;
    int ret;

    data = OsalMemCalloc(USB_BUFFER_MAX_SIZE);
    if (data == NULL) {
        HDF_LOGE("%s:%d OsalMemCalloc error", __func__, __LINE__);
        return HDF_ERR_MALLOC_FAIL;
    }

    ret = UsbRawGetDescriptor(NULL, NULL, &param, data);
    if (ret != HDF_ERR_INVALID_PARAM) {
        HDF_LOGE("%s: error", __func__);
        ret = HDF_FAILURE;
        goto error;
    }
    HDF_LOGE("%s: success", __func__);
    ret = HDF_SUCCESS;
error:
    OsalMemFree(data);
    data = NULL;

    return ret;
}

int32_t CheckRawSdkIfGetDescriptor004(void)
{
    unsigned char *data = NULL;
    int ret;

    data = OsalMemCalloc(USB_BUFFER_MAX_SIZE);
    if (data == NULL) {
        HDF_LOGE("%s:%d OsalMemCalloc error", __func__, __LINE__);
        return HDF_ERR_MALLOC_FAIL;
    }

    ret = UsbRawGetDescriptor(g_acm->ctrlReq, g_acm->devHandle, NULL, data);
    if (ret != HDF_ERR_INVALID_PARAM) {
        HDF_LOGE("%s: error", __func__);
        ret = HDF_FAILURE;
        goto error;
    }
    HDF_LOGE("%s: success", __func__);
    ret = HDF_SUCCESS;
error:
    OsalMemFree(data);
    data = NULL;

    return ret;
}

int32_t CheckRawSdkIfGetDescriptor005(void)
{
    struct UsbRawDescriptorParam param;
    int ret;

    ret = UsbRawGetDescriptor(g_acm->ctrlReq, g_acm->devHandle, &param, NULL);
    if (ret != HDF_ERR_INVALID_PARAM) {
        HDF_LOGE("%s: error", __func__);
        return HDF_FAILURE;
    }
    HDF_LOGE("%s: success", __func__);
    return HDF_SUCCESS;
}

int32_t CheckRawSdkIfGetDescriptor006(void)
{
    struct UsbRawDescriptorParam param;
    unsigned char *data = NULL;
    int ret;

    data = OsalMemCalloc(USB_BUFFER_MAX_SIZE);
    if (data == NULL) {
        HDF_LOGE("%s:%d OsalMemCalloc error", __func__, __LINE__);
        return HDF_ERR_MALLOC_FAIL;
    }

    param.descType = USB_DESC_TYPE;
    param.descIndex = 0;
    param.length = USB_BUFFER_MAX_SIZE;

    ret = UsbRawGetDescriptor(g_acm->ctrlReq, g_acm->devHandle, &param, data);
    if (ret < 0) {
        HDF_LOGE("%s: error", __func__);
        ret = HDF_FAILURE;
        goto error;
    }
    HDF_LOGE("%s: success", __func__);
    ret = HDF_SUCCESS;
error:
    OsalMemFree(data);
    data = NULL;

    return ret;
}

int32_t CheckRawSdkIfGetDescriptor007(void)
{
    unsigned char *data = NULL;
    int ret;

    data = OsalMemCalloc(USB_BUFFER_MAX_SIZE);
    if (data == NULL) {
        HDF_LOGE("%s:%d OsalMemCalloc error", __func__, __LINE__);
        return HDF_ERR_MALLOC_FAIL;
    }

    ret = UsbRawGetDescriptor(g_acm->ctrlReq, NULL, NULL, data);
    if (ret != HDF_ERR_INVALID_PARAM) {
        HDF_LOGE("%s: error", __func__);
        ret = HDF_FAILURE;
        goto error;
    }
    HDF_LOGE("%s: success", __func__);
    ret = HDF_SUCCESS;
error:
    OsalMemFree(data);
    data = NULL;

    return ret;
}

int32_t CheckRawSdkIfGetDescriptor008(void)
{
    int ret;

    ret = UsbRawGetDescriptor(g_acm->ctrlReq, g_acm->devHandle, NULL, NULL);
    if (ret != HDF_ERR_INVALID_PARAM) {
        HDF_LOGE("%s: error", __func__);
        return HDF_FAILURE;
    }
    HDF_LOGE("%s: success", __func__);
    return HDF_SUCCESS;
}

int32_t CheckRawSdkIfGetDescriptor009(void)
{
    unsigned char *data = NULL;
    int ret;

    data = OsalMemCalloc(USB_BUFFER_MAX_SIZE);
    if (data == NULL) {
        HDF_LOGE("%s:%d OsalMemCalloc error", __func__, __LINE__);
        return HDF_ERR_MALLOC_FAIL;
    }

    ret = UsbRawGetDescriptor(NULL, g_acm->devHandle, NULL, data);
    if (ret != HDF_ERR_INVALID_PARAM) {
        HDF_LOGE("%s: error", __func__);
        ret = HDF_FAILURE;
        goto error;
    }
    HDF_LOGE("%s: success", __func__);
    ret = HDF_SUCCESS;
error:
    OsalMemFree(data);
    data = NULL;

    return ret;
}

int32_t CheckRawSdkIfGetDescriptor010(void)
{
    struct UsbRawDescriptorParam param;
    unsigned char data[100];
    int ret;

    param.descType = 0;
    param.descIndex = 0;
    param.length = sizeof(data);

    ret = UsbRawGetDescriptor(NULL, g_acm->devHandle, &param, NULL);
    if (ret != HDF_ERR_INVALID_PARAM) {
        HDF_LOGE("%s: error", __func__);
        return HDF_FAILURE;
    }
    HDF_LOGE("%s: success", __func__);
    return HDF_SUCCESS;
}

int32_t CheckRawSdkIfGetDescriptor011(void)
{
    struct UsbRawDescriptorParam param;
    unsigned char data[100];
    int ret;

    param.descType = 0;
    param.descIndex = 0;
    param.length = sizeof(data);

    ret = UsbRawGetDescriptor(g_acm->ctrlReq, NULL, &param, NULL);
    if (ret != HDF_ERR_INVALID_PARAM) {
        HDF_LOGE("%s: error", __func__);
        return HDF_FAILURE;
    }
    HDF_LOGE("%s: success", __func__);
    return HDF_SUCCESS;
}

int32_t CheckRawSdkIfGetDescriptor012(void)
{
    unsigned char *data = NULL;
    int ret;

    data = OsalMemCalloc(USB_BUFFER_MAX_SIZE);
    if (data == NULL) {
        HDF_LOGE("%s:%d OsalMemCalloc error", __func__, __LINE__);
        return HDF_ERR_MALLOC_FAIL;
    }

    ret = UsbRawGetDescriptor(NULL, NULL, NULL, data);
    if (ret != HDF_ERR_INVALID_PARAM) {
        HDF_LOGE("%s: error", __func__);
        ret = HDF_FAILURE;
        goto error;
    }
    HDF_LOGE("%s: success", __func__);
    ret = HDF_SUCCESS;
error:
    OsalMemFree(data);
    data = NULL;

    return ret;
}

int32_t CheckRawSdkIfGetDescriptor013(void)
{
    struct UsbRawDescriptorParam param;
    unsigned char data[100];
    int ret;

    param.descType = 0;
    param.descIndex = 0;
    param.length = sizeof(data);

    ret = UsbRawGetDescriptor(NULL, NULL, &param, NULL);
    if (ret != HDF_ERR_INVALID_PARAM) {
        HDF_LOGE("%s: error", __func__);
        return HDF_FAILURE;
    }
    HDF_LOGE("%s: success", __func__);
    return HDF_SUCCESS;
}

int32_t CheckRawSdkIfGetDescriptor014(void)
{
    int ret;

    ret = UsbRawGetDescriptor(NULL, g_acm->devHandle, NULL, NULL);
    if (ret != HDF_ERR_INVALID_PARAM) {
        HDF_LOGE("%s: error", __func__);
        return HDF_FAILURE;
    }
    HDF_LOGE("%s: success", __func__);
    return HDF_SUCCESS;
}

int32_t CheckRawSdkIfGetDescriptor015(void)
{
    int ret;

    ret = UsbRawGetDescriptor(g_acm->ctrlReq, NULL, NULL, NULL);
    if (ret != HDF_ERR_INVALID_PARAM) {
        HDF_LOGE("%s: error", __func__);
        return HDF_FAILURE;
    }
    HDF_LOGE("%s: success", __func__);
    return HDF_SUCCESS;
}

int32_t CheckRawSdkIfGetDescriptor016(void)
{
    int ret;

    ret = UsbRawGetDescriptor(NULL, NULL, NULL, NULL);
    if (ret != HDF_ERR_INVALID_PARAM) {
        HDF_LOGE("%s: error", __func__);
        return HDF_FAILURE;
    }
    HDF_LOGE("%s: success", __func__);
    return HDF_SUCCESS;
}

int32_t CheckRawSdkIfFillBulkRequest001(void)
{
    struct UsbRawFillRequestData reqData;
    int32_t ret;
    int i;
    uint32_t size;
    char sendData[] = {"abcde\0"};

    size = strlen(sendData) + 1;
    printf("---size:%d\n", size);

    size = (size > g_acm->dataOutEp.maxPacketSize) ? g_acm->dataOutEp.maxPacketSize : size;
    for (i = 0; i < 1; i++) {
        struct RawWb *snd = &g_acm->wb[i];
        snd->len = size;
        ret = memcpy_s(snd->buf, g_acm->dataOutEp.maxPacketSize, sendData, size);
        if (ret) {
            printf("memcpy_s fial");
        }
        g_acm->transmitting++;

        reqData.endPoint      = g_acm->dataOutEp.addr;
        reqData.numIsoPackets = 0;
        reqData.callback      = AcmWriteBulkCallback;
        reqData.userData      = (void *)snd;
        reqData.timeout       = USB_CTRL_SET_TIMEOUT;
        reqData.buffer        = snd->buf;
        reqData.length        = snd->len;
        printf("maxPacketSize:%d+snd->request:%p\n", g_acm->dataOutEp.maxPacketSize, snd->request);
        ret = UsbRawFillBulkRequest(snd->request, g_acm->devHandle, &reqData);
        if (ret) {
            printf("%s: error++ret=%d\n", __func__, ret);
            return HDF_FAILURE;
        }
    }
    HDF_LOGE("%s: success", __func__);
    return HDF_SUCCESS;
}

int32_t CheckRawSdkIfFillBulkRequest002(void)
{
    struct UsbRawFillRequestData reqData;
    int32_t ret;
    int i;
    int size = g_acm->dataInEp.maxPacketSize;

    for (i = 0; i < 1; i++) {
        reqData.endPoint      = g_acm->dataInEp.addr;
        reqData.numIsoPackets = 0;
        reqData.callback      = AcmReadBulkCallback;
        reqData.userData      = (void *)g_acm;
        reqData.timeout       = USB_CTRL_SET_TIMEOUT;
        reqData.length        = size;

        ret = UsbRawFillBulkRequest(g_acm->readReq[i], g_acm->devHandle, &reqData);
        if (ret) {
            HDF_LOGE("%s: error", __func__);
            return HDF_FAILURE;
        }
    }
    HDF_LOGE("%s: success", __func__);
    return HDF_SUCCESS;
}

int32_t CheckRawSdkIfFillInterruptRequest001(void)
{
    struct UsbRawFillRequestData fillRequestData;
    int32_t ret;
    int size = g_acm->notifyEp.maxPacketSize;

    fillRequestData.endPoint = g_acm->notifyEp.addr;
    fillRequestData.length = size;
    fillRequestData.numIsoPackets = 0;
    fillRequestData.callback = AcmNotifyReqCallback;
    fillRequestData.userData = (void *)g_acm;
    fillRequestData.timeout = USB_RAW_REQUEST_TIME_ZERO_MS;

    ret = UsbRawFillInterruptRequest(g_acm->notifyReq, g_acm->devHandle, &fillRequestData);
    if (ret) {
        HDF_LOGE("%s: error", __func__);
        return HDF_FAILURE;
    }
    HDF_LOGE("%s: success", __func__);
    return HDF_SUCCESS;
}

int32_t CheckRawSdkIfFillInterruptRequest002(void)
{
    struct UsbRawFillRequestData fillRequestData;
    int32_t ret;
    int size = g_acm->notifyEp.maxPacketSize;

    fillRequestData.endPoint = g_acm->notifyEp.addr;
    fillRequestData.length = size;
    fillRequestData.numIsoPackets = 0;
    fillRequestData.callback = AcmNotifyReqCallback;
    fillRequestData.userData = (void *)g_acm;
    fillRequestData.timeout = USB_RAW_REQUEST_TIME_ZERO_MS;

    ret = UsbRawFillInterruptRequest(NULL, g_acm->devHandle, &fillRequestData);
    if (ret != HDF_ERR_INVALID_PARAM) {
        HDF_LOGE("%s: error", __func__);
        return HDF_FAILURE;
    }
    HDF_LOGE("%s: success", __func__);
    return HDF_SUCCESS;
}

int32_t CheckRawSdkIfFillInterruptRequest003(void)
{
    struct UsbRawFillRequestData fillRequestData;
    int32_t ret;
    int size = g_acm->notifyEp.maxPacketSize;

    fillRequestData.endPoint = g_acm->notifyEp.addr;
    fillRequestData.length = size;
    fillRequestData.numIsoPackets = 0;
    fillRequestData.callback = AcmNotifyReqCallback;
    fillRequestData.userData = (void *)g_acm;
    fillRequestData.timeout = USB_RAW_REQUEST_TIME_ZERO_MS;

    ret = UsbRawFillInterruptRequest(g_acm->notifyReq, NULL, &fillRequestData);
    if (ret != HDF_ERR_INVALID_PARAM) {
        HDF_LOGE("%s: error", __func__);
        return HDF_FAILURE;
    }
    HDF_LOGE("%s: success", __func__);
    return HDF_SUCCESS;
}

int32_t CheckRawSdkIfFillInterruptRequest004(void)
{
    struct UsbRawFillRequestData fillRequestData;
    int32_t ret;
    int size = g_acm->notifyEp.maxPacketSize;

    fillRequestData.endPoint = g_acm->notifyEp.addr;
    fillRequestData.length = size;
    fillRequestData.numIsoPackets = 0;
    fillRequestData.callback = AcmNotifyReqCallback;
    fillRequestData.userData = (void *)g_acm;
    fillRequestData.timeout = USB_RAW_REQUEST_TIME_ZERO_MS;

    ret = UsbRawFillInterruptRequest(g_acm->notifyReq, g_acm->devHandle, NULL);
    if (ret != HDF_ERR_INVALID_PARAM) {
        HDF_LOGE("%s: error", __func__);
        return HDF_FAILURE;
    }
    HDF_LOGE("%s: success", __func__);
    return HDF_SUCCESS;
}

int32_t CheckRawSdkIfFillControlRequest001(void)
{
    struct UsbRawFillRequestData fillRequestData;
    int ret;
    int completed = 0;

    fillRequestData.callback  = AcmCtrlReqCallback;
    fillRequestData.userData  = &completed;
    fillRequestData.timeout   = USB_CTRL_SET_TIMEOUT;

    ret = UsbRawFillControlRequest(g_acm->ctrlReq, g_acm->devHandle, &fillRequestData);
    if (ret) {
        HDF_LOGE("%s: error", __func__);
        return HDF_FAILURE;
    }
    HDF_LOGE("%s: success", __func__);
    return HDF_SUCCESS;
}

int32_t CheckRawSdkIfFillControlRequest002(void)
{
    struct UsbRawFillRequestData fillRequestData;
    int ret;
    int completed = 0;

    fillRequestData.callback  = AcmCtrlReqCallback;
    fillRequestData.userData  = &completed;
    fillRequestData.timeout   = USB_CTRL_SET_TIMEOUT;

    ret = UsbRawFillControlRequest(NULL, g_acm->devHandle, &fillRequestData);
    if (ret != HDF_ERR_INVALID_PARAM) {
        HDF_LOGE("%s: error", __func__);
        return HDF_FAILURE;
    }
    HDF_LOGE("%s: success", __func__);
    return HDF_SUCCESS;
}

int32_t CheckRawSdkIfFillControlRequest003(void)
{
    struct UsbRawFillRequestData fillRequestData;
    int ret;
    int completed = 0;

    fillRequestData.callback  = AcmCtrlReqCallback;
    fillRequestData.userData  = &completed;
    fillRequestData.timeout   = USB_CTRL_SET_TIMEOUT;

    ret = UsbRawFillControlRequest(g_acm->ctrlReq, NULL, &fillRequestData);
    if (ret != HDF_ERR_INVALID_PARAM) {
        HDF_LOGE("%s: error", __func__);
        return HDF_FAILURE;
    }
    HDF_LOGE("%s: success", __func__);
    return HDF_SUCCESS;
}

int32_t CheckRawSdkIfFillControlRequest004(void)
{
    struct UsbRawFillRequestData fillRequestData;
    int ret;
    int completed = 0;

    fillRequestData.callback  = AcmCtrlReqCallback;
    fillRequestData.userData  = &completed;
    fillRequestData.timeout   = USB_CTRL_SET_TIMEOUT;

    ret = UsbRawFillControlRequest(g_acm->ctrlReq, g_acm->devHandle, NULL);
    if (ret != HDF_ERR_INVALID_PARAM) {
        HDF_LOGE("%s: error", __func__);
        return HDF_FAILURE;
    }
    HDF_LOGE("%s: success", __func__);
    return HDF_SUCCESS;
}

int32_t CheckRawSdkIfFillControlRequest005(void)
{
    struct UsbRawFillRequestData fillRequestData;
    int ret;
    int completed = 0;

    fillRequestData.callback  = AcmCtrlReqCallback;
    fillRequestData.userData  = &completed;
    fillRequestData.timeout   = USB_CTRL_SET_TIMEOUT;

    ret = UsbRawFillControlRequest(NULL, g_acm->devHandle, NULL);
    if (ret != HDF_ERR_INVALID_PARAM) {
        HDF_LOGE("%s: error", __func__);
        return HDF_FAILURE;
    }
    HDF_LOGE("%s: success", __func__);
    return HDF_SUCCESS;
}

int32_t CheckRawSdkIfFillControlRequest006(void)
{
    struct UsbRawFillRequestData fillRequestData;
    int ret;
    int completed = 0;

    fillRequestData.callback  = AcmCtrlReqCallback;
    fillRequestData.userData  = &completed;
    fillRequestData.timeout   = USB_CTRL_SET_TIMEOUT;

    ret = UsbRawFillControlRequest(g_acm->ctrlReq, NULL, NULL);
    if (ret != HDF_ERR_INVALID_PARAM) {
        HDF_LOGE("%s: error", __func__);
        return HDF_FAILURE;
    }
    HDF_LOGE("%s: success", __func__);
    return HDF_SUCCESS;
}

int32_t CheckRawSdkIfFillControlRequest007(void)
{
    struct UsbRawFillRequestData fillRequestData;
    int ret;
    int completed = 0;

    fillRequestData.callback  = AcmCtrlReqCallback;
    fillRequestData.userData  = &completed;
    fillRequestData.timeout   = USB_CTRL_SET_TIMEOUT;

    ret = UsbRawFillControlRequest(NULL, NULL, NULL);
    if (ret != HDF_ERR_INVALID_PARAM) {
        HDF_LOGE("%s: error", __func__);
        return HDF_FAILURE;
    }
    HDF_LOGE("%s: success", __func__);
    return HDF_SUCCESS;
}

int32_t CheckRawSdkIfFillControlRequest008(void)
{
    struct UsbRawFillRequestData fillRequestData;
    int ret;
    int completed = 0;

    fillRequestData.callback  = AcmCtrlReqCallback;
    fillRequestData.userData  = &completed;
    fillRequestData.timeout   = USB_CTRL_SET_TIMEOUT;

    ret = UsbRawFillControlRequest(NULL, NULL, &fillRequestData);
    if (ret != HDF_ERR_INVALID_PARAM) {
        HDF_LOGE("%s: error", __func__);
        return HDF_FAILURE;
    }
    HDF_LOGE("%s: success", __func__);
    return HDF_SUCCESS;
}

int32_t CheckRawSdkIfFillControlSetup001(void)
{
    struct UsbControlRequestData ctrlReq;
    int ret;

    g_acm->lineCoding.dwDTERate = CpuToLe32(DATARATE);
    g_acm->lineCoding.bCharFormat = CHARFORMAT;
    g_acm->lineCoding.bParityType = USB_CDC_NO_PARITY;
    g_acm->lineCoding.bDataBits = USB_CDC_1_STOP_BITS;

    ctrlReq.requestType = USB_DDK_DIR_OUT | USB_DDK_TYPE_CLASS | USB_DDK_RECIP_INTERFACE;
    ctrlReq.requestCmd  = USB_DDK_CDC_REQ_SET_LINE_CODING;
    ctrlReq.value       = CpuToLe16(0);
    ctrlReq.index       = 0;
    ctrlReq.data        = (unsigned char *)&g_acm->lineCoding;
    ctrlReq.length      = sizeof(struct UsbCdcLineCoding);
    ctrlReq.timeout     = USB_CTRL_SET_TIMEOUT;

    ret = UsbRawFillControlSetup(NULL, &ctrlReq);
    if (ret != HDF_ERR_INVALID_PARAM) {
        HDF_LOGE("%s: error", __func__);
        return HDF_FAILURE;
    }
    HDF_LOGE("%s: success", __func__);
    return HDF_SUCCESS;
}

int32_t CheckRawSdkIfFillControlSetup002(void)
{
    unsigned char *setup = NULL;
    int ret;

    setup = OsalMemCalloc(USB_BUFFER_MAX_SIZE);
    if (setup == NULL) {
        HDF_LOGE("%s:%d OsalMemCalloc error", __func__, __LINE__);
        return HDF_ERR_MALLOC_FAIL;
    }

    ret = UsbRawFillControlSetup(setup, NULL);
    if (ret != HDF_ERR_INVALID_PARAM) {
        HDF_LOGE("%s: error", __func__);
        ret = HDF_FAILURE;
        goto error;
    }
    HDF_LOGE("%s: success", __func__);
    ret = HDF_SUCCESS;
error:
    OsalMemFree(setup);
    setup = NULL;

    return ret;
}

int32_t CheckRawSdkIfFillControlSetup003(void)
{
    int ret;

    ret = UsbRawFillControlSetup(NULL, NULL);
    if (ret != HDF_ERR_INVALID_PARAM) {
        HDF_LOGE("%s: error", __func__);
        return HDF_FAILURE;
    }
    HDF_LOGE("%s: success", __func__);
    return HDF_SUCCESS;
}

int32_t CheckRawSdkIfFillControlSetup004(void)
{
    struct UsbControlRequestData ctrlReq;
    unsigned char *setup = NULL;
    int ret;

    g_acm->lineCoding.dwDTERate = CpuToLe32(DATARATE);
    g_acm->lineCoding.bCharFormat = CHARFORMAT;
    g_acm->lineCoding.bParityType = USB_CDC_NO_PARITY;
    g_acm->lineCoding.bDataBits = USB_CDC_1_STOP_BITS;

    ctrlReq.requestType = USB_DDK_DIR_OUT | USB_DDK_TYPE_CLASS | USB_DDK_RECIP_INTERFACE;
    ctrlReq.requestCmd  = USB_DDK_CDC_REQ_SET_LINE_CODING;
    ctrlReq.value       = CpuToLe16(0);
    ctrlReq.index       = 0;
    ctrlReq.data        = (unsigned char *)&g_acm->lineCoding;
    ctrlReq.length      = sizeof(struct UsbCdcLineCoding);
    ctrlReq.timeout     = USB_CTRL_SET_TIMEOUT;

    setup = OsalMemCalloc(USB_BUFFER_MAX_SIZE);
    if (setup == NULL) {
        HDF_LOGE("%s:%d OsalMemCalloc error", __func__, __LINE__);
        return HDF_ERR_MALLOC_FAIL;
    }

    ret = UsbRawFillControlSetup(setup, &ctrlReq);
    if (ret) {
        HDF_LOGE("%s: error", __func__);
        ret = HDF_FAILURE;
        goto error;
    }
    HDF_LOGE("%s: success", __func__);

error:
    OsalMemFree(setup);
    setup = NULL;

    return ret;
}

int32_t CheckRawSdkIfSendControlRequest001(void)
{
    struct UsbControlRequestData ctrlReq;
    int ret;

    g_acm->lineCoding.dwDTERate = CpuToLe32(DATARATE);
    g_acm->lineCoding.bCharFormat = CHARFORMAT;
    g_acm->lineCoding.bParityType = USB_CDC_NO_PARITY;
    g_acm->lineCoding.bDataBits = USB_CDC_1_STOP_BITS;
    ctrlReq.requestType = USB_DDK_DIR_OUT | USB_DDK_TYPE_CLASS | USB_DDK_RECIP_INTERFACE;
    ctrlReq.requestCmd  = USB_DDK_CDC_REQ_SET_LINE_CODING;
    ctrlReq.value       = CpuToLe16(0);
    ctrlReq.index       = 0;
    ctrlReq.data        = (unsigned char *)&g_acm->lineCoding;
    ctrlReq.length      = sizeof(struct UsbCdcLineCoding);
    ctrlReq.timeout     = USB_CTRL_SET_TIMEOUT;

    ret = UsbRawSendControlRequest(NULL, g_acm->devHandle, &ctrlReq);
    if (ret != HDF_ERR_INVALID_PARAM) {
        HDF_LOGE("%s: error", __func__);
        return HDF_FAILURE;
    }
    HDF_LOGE("%s: success", __func__);
    return HDF_SUCCESS;
}

int32_t CheckRawSdkIfSendControlRequest002(void)
{
    struct UsbControlRequestData ctrlReq;
    int ret;

    g_acm->lineCoding.dwDTERate = CpuToLe32(DATARATE);
    g_acm->lineCoding.bCharFormat = CHARFORMAT;
    g_acm->lineCoding.bParityType = USB_CDC_NO_PARITY;
    g_acm->lineCoding.bDataBits = USB_CDC_1_STOP_BITS;
    ctrlReq.requestType = USB_DDK_DIR_OUT | USB_DDK_TYPE_CLASS | USB_DDK_RECIP_INTERFACE;
    ctrlReq.requestCmd  = USB_DDK_CDC_REQ_SET_LINE_CODING;
    ctrlReq.value       = CpuToLe16(0);
    ctrlReq.index       = 0;
    ctrlReq.data        = (unsigned char *)&g_acm->lineCoding;
    ctrlReq.length      = sizeof(struct UsbCdcLineCoding);
    ctrlReq.timeout     = USB_CTRL_SET_TIMEOUT;

    ret = UsbRawSendControlRequest(g_acm->ctrlReq, NULL, &ctrlReq);
    if (ret != HDF_ERR_INVALID_PARAM) {
        HDF_LOGE("%s: error", __func__);
        return HDF_FAILURE;
    }
    HDF_LOGE("%s: success", __func__);
    return HDF_SUCCESS;
}

int32_t CheckRawSdkIfSendControlRequest003(void)
{
    int ret;
    ret = UsbRawSendControlRequest(g_acm->ctrlReq, g_acm->devHandle, NULL);
    if (ret != HDF_ERR_INVALID_PARAM) {
        HDF_LOGE("%s: error", __func__);
        return HDF_FAILURE;
    }
    HDF_LOGE("%s: success", __func__);
    return HDF_SUCCESS;
}

int32_t CheckRawSdkIfSendControlRequest004(void)
{
    struct UsbControlRequestData ctrlReq;
    int ret;

    g_acm->lineCoding.dwDTERate = CpuToLe32(DATARATE);
    g_acm->lineCoding.bCharFormat = CHARFORMAT;
    g_acm->lineCoding.bParityType = USB_CDC_NO_PARITY;
    g_acm->lineCoding.bDataBits = USB_CDC_1_STOP_BITS;
    ctrlReq.requestType = USB_DDK_DIR_OUT | USB_DDK_TYPE_CLASS | USB_DDK_RECIP_INTERFACE;
    ctrlReq.requestCmd  = USB_DDK_CDC_REQ_SET_LINE_CODING;
    ctrlReq.value       = CpuToLe16(0);
    ctrlReq.index       = 2;
    ctrlReq.data        = (unsigned char *)&g_acm->lineCoding;
    ctrlReq.length      = sizeof(struct UsbCdcLineCoding);
    ctrlReq.timeout     = USB_CTRL_SET_TIMEOUT;

    ret = UsbRawSendControlRequest(g_acm->ctrlReq, g_acm->devHandle, &ctrlReq);
    if (ret < 0) {
        HDF_LOGE("%s: error", __func__);
        return HDF_FAILURE;
    }
    HDF_LOGE("%s: success", __func__);
    return HDF_SUCCESS;
}

int32_t CheckRawSdkIfSendControlRequest005(void)
{
    struct UsbControlRequestData ctrlReq;
    int ret;

    g_acm->lineCoding.dwDTERate = CpuToLe32(DATARATE);
    g_acm->lineCoding.bCharFormat = CHARFORMAT;
    g_acm->lineCoding.bParityType = USB_CDC_NO_PARITY;
    g_acm->lineCoding.bDataBits = USB_CDC_1_STOP_BITS;
    ctrlReq.requestType = USB_DDK_DIR_OUT | USB_DDK_TYPE_CLASS | USB_DDK_RECIP_INTERFACE;
    ctrlReq.requestCmd  = USB_DDK_CDC_REQ_SET_LINE_CODING;
    ctrlReq.value       = CpuToLe16(0);
    ctrlReq.index       = 0;
    ctrlReq.data        = (unsigned char *)&g_acm->lineCoding;
    ctrlReq.length      = sizeof(struct UsbCdcLineCoding);
    ctrlReq.timeout     = USB_CTRL_SET_TIMEOUT;

    ret = UsbRawSendControlRequest(NULL, NULL, &ctrlReq);
    if (ret != HDF_ERR_INVALID_PARAM) {
        HDF_LOGE("%s: error", __func__);
        return HDF_FAILURE;
    }
    HDF_LOGE("%s: success", __func__);
    return HDF_SUCCESS;
}

int32_t CheckRawSdkIfSendControlRequest006(void)
{
    int ret;

    ret = UsbRawSendControlRequest(NULL, g_acm->devHandle, NULL);
    if (ret != HDF_ERR_INVALID_PARAM) {
        HDF_LOGE("%s: error", __func__);
        return HDF_FAILURE;
    }
    HDF_LOGE("%s: success", __func__);
    return HDF_SUCCESS;
}

int32_t CheckRawSdkIfSendControlRequest007(void)
{
    int ret;

    ret = UsbRawSendControlRequest(g_acm->ctrlReq, NULL, NULL);
    if (ret != HDF_ERR_INVALID_PARAM) {
        HDF_LOGE("%s: error", __func__);
        return HDF_FAILURE;
    }
    HDF_LOGE("%s: success", __func__);
    return HDF_SUCCESS;
}

int32_t CheckRawSdkIfSendBulkRequest001(void)
{
    struct UsbRequestData reqData;
    int32_t ret;
    int i;
    uint32_t size;
    char sendData[] = {"abcd\0"};

    size = strlen(sendData) + 1;
    size = (size > g_acm->dataOutEp.maxPacketSize) ? g_acm->dataOutEp.maxPacketSize : size;

    for (i = 0; i < 1; i++) {
        struct RawWb *snd = &g_acm->wb[i];
        snd->len = size;
        ret = memcpy_s(snd->buf, g_acm->dataOutEp.maxPacketSize, sendData, size);
        if (ret) {
            printf("memcpy_s fial");
        }
        g_acm->transmitting++;

        reqData.endPoint      = g_acm->dataOutEp.addr;
        reqData.timeout       = USB_RAW_REQUEST_TIME_ZERO_MS;
        reqData.data        = snd->buf;
        reqData.length        = snd->len;
        reqData.requested   = (int *)&size;
    }

    for (i = 0; i < 1; i++) {
        struct RawWb *snd = &g_acm->wb[i];
        printf("UsbRawSendBulkRequest i = [%d]\n", i);
        ret = UsbRawSendBulkRequest(snd->request, g_acm->devHandle, &reqData);
        if (ret) {
            printf("%s: error+ret:%d", __func__, ret);
            return HDF_FAILURE;
        }
    }
    HDF_LOGE("%s: success", __func__);
    return HDF_SUCCESS;
}

int32_t CheckRawSdkIfSendBulkRequest002(void)
{
    struct UsbRequestData reqData;
    int32_t ret;
    int i;
    int size = g_acm->dataInEp.maxPacketSize;

    for (i = 0; i < 1; i++) {
        reqData.endPoint      = g_acm->dataInEp.addr;
        reqData.timeout       = USB_RAW_REQUEST_TIME_ZERO_MS;
        reqData.length        = size;
        reqData.data        = ((struct UsbRawRequest *)g_acm->readReq[i])->buffer;
        reqData.requested      = (int *)&size;
    }

    for (i = 0; i < 1; i++) {
        printf("UsbRawSendBulkRequest i = [%d]\n", i);
        ret = UsbRawSendBulkRequest(g_acm->readReq[i], g_acm->devHandle, &reqData);
        if (ret) {
            HDF_LOGE("%s: error", __func__);
            return HDF_FAILURE;
        }
    }
    HDF_LOGE("%s: success", __func__);
    return HDF_SUCCESS;
}

int32_t CheckRawSdkIfSendBulkRequest003(void)
{
    struct UsbRequestData reqData;
    int32_t ret;
    int i;
    int size = g_acm->dataInEp.maxPacketSize;

    for (i = 0; i < 1; i++) {
        reqData.endPoint      = g_acm->dataInEp.addr;
        reqData.timeout       = USB_RAW_REQUEST_TIME_ZERO_MS;
        reqData.length        = size;
        reqData.data        = ((struct UsbRawRequest *)g_acm->readReq[i])->buffer;
        reqData.requested      = (int *)&size;
    }

    for (i = 0; i < 1; i++) {
        printf("UsbRawSendBulkRequest i = [%d]\n", i);
        ret = UsbRawSendBulkRequest(NULL, g_acm->devHandle, &reqData);
        if (ret != HDF_ERR_INVALID_PARAM) {
            HDF_LOGE("%s: error", __func__);
            return HDF_FAILURE;
        }
    }
    HDF_LOGE("%s: success", __func__);
    return HDF_SUCCESS;
}

int32_t CheckRawSdkIfSendBulkRequest004(void)
{
    struct UsbRequestData reqData;
    int32_t ret;
    int i;
    int size = g_acm->dataInEp.maxPacketSize;

    for (i = 0; i < 1; i++) {
        reqData.endPoint      = g_acm->dataInEp.addr;
        reqData.timeout       = USB_RAW_REQUEST_TIME_ZERO_MS;
        reqData.length        = size;
        reqData.data        = ((struct UsbRawRequest *)g_acm->readReq[i])->buffer;
        reqData.requested      = (int *)&size;
    }
    for (i = 0; i < 1; i++) {
        printf("UsbRawSendBulkRequest i = [%d]\n", i);
        ret = UsbRawSendBulkRequest(g_acm->readReq[i], NULL, &reqData);
        if (ret != HDF_ERR_INVALID_PARAM) {
            HDF_LOGE("%s: error", __func__);
            return HDF_FAILURE;
        }
    }
    HDF_LOGE("%s: success", __func__);
    return HDF_SUCCESS;
}

int32_t CheckRawSdkIfSendBulkRequest005(void)
{
    int32_t ret;
    int i;

    for (i = 0; i < 1; i++) {
        printf("UsbRawSendBulkRequest i = [%d]\n", i);
        ret = UsbRawSendBulkRequest(g_acm->readReq[i], g_acm->devHandle, NULL);
        if (ret != HDF_ERR_INVALID_PARAM) {
            HDF_LOGE("%s: error", __func__);
            return HDF_FAILURE;
        }
    }
    HDF_LOGE("%s: success", __func__);
    return HDF_SUCCESS;
}

int32_t CheckRawSdkIfSendInterruptRequest001(void)
{
    struct UsbRequestData reqData;
    int32_t ret;
    int size = g_acm->notifyEp.maxPacketSize;

    reqData.endPoint = g_acm->notifyEp.addr;
    reqData.length = size;
    reqData.timeout = USB_RAW_REQUEST_TIME_ZERO_MS;
    reqData.data        = ((struct UsbRawRequest *)g_acm->notifyReq)->buffer;
    reqData.requested      = (int *)&size;
    ret = UsbRawSendInterruptRequest(g_acm->notifyReq, g_acm->devHandle, &reqData);
    if (ret) {
        HDF_LOGE("%s: error", __func__);
        return HDF_FAILURE;
    }
    HDF_LOGE("%s: success", __func__);
    return HDF_SUCCESS;
}

int32_t CheckRawSdkIfSendInterruptRequest002(void)
{
    struct UsbRequestData reqData;
    int32_t ret;
    int size = g_acm->notifyEp.maxPacketSize;

    reqData.endPoint = g_acm->notifyEp.addr;
    reqData.length = size;
    reqData.timeout = USB_RAW_REQUEST_TIME_ZERO_MS;
    reqData.data        = ((struct UsbRawRequest *)g_acm->notifyReq)->buffer;
    reqData.requested      = (int *)&size;

    ret = UsbRawSendInterruptRequest(NULL, g_acm->devHandle, &reqData);
    if (ret != HDF_ERR_INVALID_PARAM) {
        HDF_LOGE("%s: error", __func__);
        return HDF_FAILURE;
    }
    HDF_LOGE("%s: success", __func__);
    return HDF_SUCCESS;
}

int32_t CheckRawSdkIfSendInterruptRequest003(void)
{
    struct UsbRequestData reqData;
    int32_t ret;
    int size = g_acm->notifyEp.maxPacketSize;

    reqData.endPoint = g_acm->notifyEp.addr;
    reqData.length = size;
    reqData.timeout = USB_RAW_REQUEST_TIME_ZERO_MS;
    reqData.data        = ((struct UsbRawRequest *)g_acm->notifyReq)->buffer;
    reqData.requested      = (int *)&size;

    ret = UsbRawSendInterruptRequest(g_acm->notifyReq, NULL, &reqData);
    if (ret != HDF_ERR_INVALID_PARAM) {
        HDF_LOGE("%s: error", __func__);
        return HDF_FAILURE;
    }
    HDF_LOGE("%s: success", __func__);
    return HDF_SUCCESS;
}

int32_t CheckRawSdkIfSendInterruptRequest004(void)
{
    int32_t ret;

    ret = UsbRawSendInterruptRequest(g_acm->notifyReq, g_acm->devHandle, NULL);
    if (ret != HDF_ERR_INVALID_PARAM) {
        HDF_LOGE("%s: error", __func__);
        return HDF_FAILURE;
    }
    HDF_LOGE("%s: success", __func__);
    return HDF_SUCCESS;
}

int32_t CheckRawSdkIfFillBulkRequest003(void)
{
    struct UsbRawFillRequestData reqData;
    int32_t ret;
    int i;
    uint32_t size;
    char sendData[] = {"abcde\0"};
    size = strlen(sendData) + 1;
    size = (size > g_acm->dataOutEp.maxPacketSize) ? g_acm->dataOutEp.maxPacketSize : size;

    for (i = 0; i < ACM_NW; i++) {
        struct RawWb *snd = &g_acm->wb[i];
        snd->len = size;
        ret = memcpy_s(snd->buf, g_acm->dataOutEp.maxPacketSize, sendData, size);
        if (ret) {
            printf("memcpy_s fial");
        }
        g_acm->transmitting++;

        reqData.endPoint      = g_acm->dataOutEp.addr;
        reqData.numIsoPackets = 0;
        reqData.callback      = AcmWriteBulkCallback;
        reqData.userData      = (void *)snd;
        reqData.timeout       = USB_CTRL_SET_TIMEOUT;
        reqData.buffer        = snd->buf;
        reqData.length        = snd->len;

        ret = UsbRawFillBulkRequest(snd->request, g_acm->devHandle, &reqData);
        if (ret) {
            HDF_LOGE("%s: error", __func__);
            return HDF_FAILURE;
        }
    }
    HDF_LOGE("%s: success", __func__);
    return HDF_SUCCESS;
}

int32_t CheckRawSdkIfFillBulkRequest004(void)
{
    struct UsbRawFillRequestData reqData;
    int32_t ret;
    int i;
    int size = g_acm->dataInEp.maxPacketSize;

    for (i = 0; i < ACM_NW; i++) {
        reqData.endPoint      = g_acm->dataInEp.addr;
        reqData.numIsoPackets = 0;
        reqData.callback      = AcmReadBulkCallback;
        reqData.userData      = (void *)g_acm;
        reqData.timeout       = USB_RAW_REQUEST_TIME_ZERO_MS;
        reqData.length        = size;
        ret = UsbRawFillBulkRequest(g_acm->readReq[i], g_acm->devHandle, &reqData);
        if (ret) {
            HDF_LOGE("%s: error", __func__);
            return HDF_FAILURE;
        }
    }
    HDF_LOGE("%s: success", __func__);
    return HDF_SUCCESS;
}

int32_t CheckRawSdkIfFillInterruptRequest005(void)
{
    struct UsbRawFillRequestData fillRequestData;
    int32_t ret;
    int size = g_acm->notifyEp.maxPacketSize;
    fillRequestData.endPoint = g_acm->notifyEp.addr;
    fillRequestData.length = size;
    fillRequestData.numIsoPackets = 0;
    fillRequestData.callback = AcmNotifyReqCallback;
    fillRequestData.userData = (void *)g_acm;
    fillRequestData.timeout = USB_CTRL_SET_TIMEOUT;
    ret = UsbRawFillInterruptRequest(g_acm->notifyReq, g_acm->devHandle, &fillRequestData);
    if (ret) {
        HDF_LOGE("%s: error", __func__);
        return HDF_FAILURE;
    }
    HDF_LOGE("%s: success", __func__);
    return HDF_SUCCESS;
}

int32_t CheckRawSdkIfSubmitRequest001(void)
{
    int32_t ret;
    int i;

    for (i = 0; i < ACM_NW; i++) {
        struct RawWb *snd = &g_acm->wb[i];
        printf("UsbRawSubmitRequest i = [%d]\n", i);
        ret = UsbRawSubmitRequest(snd->request);
        if (ret) {
            HDF_LOGE("%s: error", __func__);
            return HDF_FAILURE;
        }
    }
    HDF_LOGE("%s: success", __func__);
    return HDF_SUCCESS;
}

int32_t CheckRawSdkIfSubmitRequest002(void)
{
    int32_t ret;
    int i;

    for (i = 0; i < ACM_NW; i++) {
        printf("UsbRawSubmitRequest i = [%d]\n", i);
        ret = UsbRawSubmitRequest(g_acm->readReq[i]);
        if (ret) {
            HDF_LOGE("%s: error", __func__);
            return HDF_FAILURE;
        }
    }
    HDF_LOGE("%s: success", __func__);
    return HDF_SUCCESS;
}

int32_t CheckRawSdkIfSubmitRequest003(void)
{
    int32_t ret;

    ret = UsbRawSubmitRequest(g_acm->notifyReq);
    if (ret) {
        HDF_LOGE("%s: error", __func__);
        return HDF_FAILURE;
    }
    HDF_LOGE("%s: success", __func__);
    return HDF_SUCCESS;
}

int32_t CheckRawSdkIfSubmitRequest004(void)
{
    int32_t ret;

    ret = UsbRawSubmitRequest(NULL);
    if (ret != HDF_ERR_INVALID_PARAM) {
        HDF_LOGE("%s: error", __func__);
        return HDF_FAILURE;
    }
    HDF_LOGE("%s: success", __func__);
    return HDF_SUCCESS;
}

int32_t CheckRawSdkIfCancelRequest001(void)
{
    int32_t ret;
    int i;

    for (i = 0; i < ACM_NW; i++) {
        struct RawWb *snd = &g_acm->wb[i];
        ret = UsbRawCancelRequest(snd->request);
        if (ret) {
            HDF_LOGE("%s: error", __func__);
            return HDF_FAILURE;
        }
    }
    HDF_LOGE("%s: success", __func__);
    return HDF_SUCCESS;
}

int32_t CheckRawSdkIfCancelRequest002(void)
{
    int32_t ret;
    int i;

    for (i = 0; i < ACM_NR; i++) {
        ret = UsbRawCancelRequest(g_acm->readReq[i]);
        printf("%s+%d+ret:%d\n", __func__, __LINE__, ret);
        if (ret) {
            HDF_LOGE("%s: error", __func__);
            return HDF_FAILURE;
        }
    }
    HDF_LOGE("%s: success", __func__);
    return HDF_SUCCESS;
}

int32_t CheckRawSdkIfCancelRequest003(void)
{
    int32_t ret;

    ret = UsbRawCancelRequest(g_acm->notifyReq);
    if (ret) {
        HDF_LOGE("%s: error", __func__);
        return HDF_FAILURE;
    }
    HDF_LOGE("%s: success", __func__);
    return HDF_SUCCESS;
}

int32_t CheckRawSdkIfCancelRequest004(void)
{
    int32_t ret;

    ret = UsbRawCancelRequest(NULL);
    if (ret != HDF_ERR_INVALID_PARAM) {
        HDF_LOGE("%s: error", __func__);
        return HDF_FAILURE;
    }
    HDF_LOGE("%s: success", __func__);
    return HDF_SUCCESS;
}
