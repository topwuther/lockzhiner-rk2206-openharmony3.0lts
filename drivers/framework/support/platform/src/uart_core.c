/*
 * Copyright (c) 2020-2021 Huawei Device Co., Ltd.
 *
 * HDF is dual licensed: you can use it either under the terms of
 * the GPL, or the BSD license, at your option.
 * See the LICENSE file in the root of this repository for complete details.
 */

#include "uart_core.h"
#include "hdf_log.h"
#include "osal_mem.h"
#include "uart_if.h"

#define HDF_LOG_TAG uart_core

int32_t UartHostInit(struct UartHost *host)
{
    int32_t ret;

    if (host == NULL || host->method == NULL) {
        return HDF_ERR_INVALID_PARAM;
    }
    if (OsalAtomicRead(&host->atom) == 1) {
        return HDF_ERR_DEVICE_BUSY;
    }
    OsalAtomicInc(&host->atom);
    if (host->method->Init != NULL) {
        ret = host->method->Init(host);
        if (ret != HDF_SUCCESS) {
            OsalAtomicDec(&host->atom);
            return ret;
        }
    }
    return HDF_SUCCESS;
}

int32_t UartHostDeinit(struct UartHost *host)
{
    int32_t ret;

    if (host == NULL || host->method == NULL) {
        return HDF_ERR_INVALID_PARAM;
    }
    if (host->method->Deinit != NULL) {
        ret = host->method->Deinit(host);
        if (ret != HDF_SUCCESS) {
            return ret;
        }
    }
    OsalAtomicDec(&host->atom);
    return HDF_SUCCESS;
}

static int32_t UartUserRead(struct UartHost *host, struct HdfSBuf *reply)
{
    size_t size;
    int32_t len;
    uint8_t *buf = NULL;

    size = HdfSbufGetCapacity(reply);
    if (size == 0) {
        HDF_LOGE("%s: the size of Sbuf is 0", __func__);
        return HDF_ERR_INVALID_PARAM;
    }
    buf = (uint8_t *)OsalMemCalloc(sizeof(*buf) * size);
    if (buf == NULL) {
        HDF_LOGE("%s: OsalMemCalloc error", __func__);
        return HDF_ERR_MALLOC_FAIL;
    }
    len = UartHostRead(host, buf, size);
    if (len <= 0) {
        HDF_LOGE("%s: UartHostRead error, len is %d", __func__, len);
        OsalMemFree(buf);
        return len;
    }
    if (!HdfSbufWriteBuffer(reply, buf, len)) {
        HDF_LOGE("%s: sbuf write buffer failed", __func__);
        OsalMemFree(buf);
        return HDF_ERR_IO;
    }
    OsalMemFree(buf);
    return HDF_SUCCESS;
}

static int32_t UartUserWrite(struct UartHost *host, struct HdfSBuf *data)
{
    size_t size;
    uint8_t *buf = NULL;

    if (!HdfSbufReadBuffer(data, (const void **)&buf, &size)) {
        HDF_LOGE("%s: sbuf read buffer failed", __func__);
        return HDF_ERR_IO;
    }
    return UartHostWrite(host, buf, size);
}

static int32_t UartUserGetBaud(struct UartHost *host, struct HdfSBuf *reply)
{
    int32_t ret;
    uint32_t baudRate;

    ret = UartHostGetBaud(host, &baudRate);
    if (ret != HDF_SUCCESS) {
        return ret;
    }
    if (!HdfSbufWriteBuffer(reply, &baudRate, sizeof(baudRate))) {
        HDF_LOGE("%s: sbuf write buffer failed", __func__);
        return HDF_ERR_IO;
    }
    return HDF_SUCCESS;
}

static int32_t UartUserSetBaud(struct UartHost *host, struct HdfSBuf *data)
{
    size_t size;
    uint32_t *baudRate = NULL;

    if (!HdfSbufReadBuffer(data, (const void **)&baudRate, &size)) {
        HDF_LOGE("%s: sbuf read buffer failed", __func__);
        return HDF_ERR_IO;
    }
    return UartHostSetBaud(host, *baudRate);
}

static int32_t UartUserGetAttribute(struct UartHost *host, struct HdfSBuf *reply)
{
    int32_t ret;
    struct UartAttribute attribute;

    ret = UartHostGetAttribute(host, &attribute);
    if (ret != HDF_SUCCESS) {
        return ret;
    }
    if (!HdfSbufWriteBuffer(reply, &attribute, sizeof(attribute))) {
        HDF_LOGE("%s: sbuf write buffer failed", __func__);
        return HDF_ERR_IO;
    }
    return HDF_SUCCESS;
}

static int32_t UartUserSetAttribute(struct UartHost *host, struct HdfSBuf *data)
{
    size_t size;
    struct UartAttribute *attribute = NULL;

    if (!HdfSbufReadBuffer(data, (const void **)&attribute, &size)) {
        HDF_LOGE("%s: sbuf read buffer failed", __func__);
        return HDF_ERR_IO;
    }
    return UartHostSetAttribute(host, attribute);
}

static int32_t UartUserSetTransMode(struct UartHost *host, struct HdfSBuf *data)
{
    size_t size;
    enum UartTransMode *mode = NULL;

    if (!HdfSbufReadBuffer(data, (const void **)&mode, &size) || mode == NULL) {
        HDF_LOGE("%s: sbuf read buffer failed", __func__);
        return HDF_ERR_IO;
    }
    return UartHostSetTransMode(host, *mode);
}

static int32_t UartIoDispatch(struct HdfDeviceIoClient *client, int cmd,
    struct HdfSBuf *data, struct HdfSBuf *reply)
{
    struct UartHost *host = NULL;

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

    host = (struct UartHost *)client->device->service;
    switch (cmd) {
        case UART_IO_INIT:
            return UartHostInit(host);
        case UART_IO_DEINIT:
            return UartHostDeinit(host);
        case UART_IO_READ:
            return UartUserRead(host, reply);
        case UART_IO_WRITE:
            return UartUserWrite(host, data);
        case UART_IO_GET_BAUD:
            return UartUserGetBaud(host, reply);
        case UART_IO_SET_BAUD:
            return UartUserSetBaud(host, data);
        case UART_IO_GET_ATTRIBUTE:
            return UartUserGetAttribute(host, reply);
        case UART_IO_SET_ATTRIBUTE:
            return UartUserSetAttribute(host, data);
        case UART_IO_SET_TRANSMODE:
            return UartUserSetTransMode(host, data);
        default:
            return HDF_ERR_NOT_SUPPORT;
    }
}

void UartHostDestroy(struct UartHost *host)
{
    if (host == NULL) {
        return;
    }
    OsalMemFree(host);
}

struct UartHost *UartHostCreate(struct HdfDeviceObject *device)
{
    struct UartHost *host = NULL;

    if (device == NULL) {
        HDF_LOGE("%s: invalid parameter", __func__);
        return NULL;
    }
    host = (struct UartHost *)OsalMemCalloc(sizeof(*host));
    if (host == NULL) {
        HDF_LOGE("%s: OsalMemCalloc error", __func__);
        return NULL;
    }
    host->device = device;
    device->service = &(host->service);
    host->device->service->Dispatch = UartIoDispatch;
    OsalAtomicSet(&host->atom, 0);
    host->priv = NULL;
    host->method = NULL;
    return host;
}
