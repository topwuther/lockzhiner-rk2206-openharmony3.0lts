/*
 * Copyright (c) 2020-2021 Huawei Device Co., Ltd.
 *
 * HDF is dual licensed: you can use it either under the terms of
 * the GPL, or the BSD license, at your option.
 * See the LICENSE file in the root of this repository for complete details.
 */

#include "uart_if.h"
#include "securec.h"
#ifndef __USER__
#include "devsvc_manager_clnt.h"
#else
#include "hdf_io_service_if.h"
#endif
#include "hdf_log.h"
#include "osal_mem.h"
#ifndef __USER__
#include "uart_core.h"
#endif

#define HDF_LOG_TAG uart_if_c
#define UART_HOST_NAME_LEN 32

static void *UartGetObjGetByBusNum(uint32_t num)
{
    int ret;
    void *obj = NULL;
    char *name = NULL;

    name = (char *)OsalMemCalloc(UART_HOST_NAME_LEN + 1);
    if (name == NULL) {
        return NULL;
    }
    ret = snprintf_s(name, UART_HOST_NAME_LEN + 1, UART_HOST_NAME_LEN,
        "HDF_PLATFORM_UART_%u", num);
    if (ret < 0) {
        HDF_LOGE("%s: snprintf_s failed", __func__);
        OsalMemFree(name);
        return NULL;
    }

#ifdef __USER__
    obj = (void *)HdfIoServiceBind(name);
#else
    obj = (void *)DevSvcManagerClntGetService(name);
#endif
    OsalMemFree(name);
    return obj;
}

static void UartPutObjByPointer(const void *obj)
{
    if (obj == NULL) {
        return;
    }
#ifdef __USER__
    HdfIoServiceRecycle((struct HdfIoService *)obj);
#endif
};

DevHandle UartOpen(uint32_t port)
{
    int32_t ret;
    void *handle = NULL;

    handle = UartGetObjGetByBusNum(port);
    if (handle == NULL) {
        HDF_LOGE("%s: get handle error", __func__);
        return NULL;
    }

#ifdef __USER__
    struct HdfIoService *service = (struct HdfIoService *)handle;
    if (service->dispatcher == NULL || service->dispatcher->Dispatch == NULL) {
        HDF_LOGE("%s: service is invalid", __func__);
        UartPutObjByPointer(handle);
        return NULL;
    }
    ret = service->dispatcher->Dispatch(&service->object, UART_IO_INIT, NULL, NULL);
#else
    ret = UartHostInit((struct UartHost *)handle);
#endif
    if (ret != HDF_SUCCESS) {
        HDF_LOGE("%s: UartHostInit error, ret %d", __func__, ret);
        UartPutObjByPointer(handle);
        return NULL;
    }

    return (DevHandle)handle;
}

void UartClose(DevHandle handle)
{
    int32_t ret;
    if (handle == NULL) {
        HDF_LOGE("%s: handle is NULL", __func__);
        return;
    }

#ifdef __USER__
    struct HdfIoService *service = (struct HdfIoService *)handle;
    if (service->dispatcher == NULL || service->dispatcher->Dispatch == NULL) {
        HDF_LOGE("%s: service is invalid", __func__);
        UartPutObjByPointer(handle);
        return;
    }
    ret = service->dispatcher->Dispatch(&service->object, UART_IO_DEINIT, NULL, NULL);
#else
    ret = UartHostDeinit((struct UartHost *)handle);
#endif
    if (ret != HDF_SUCCESS) {
        HDF_LOGE("%s: UartHostDeinit error, ret %d", __func__, ret);
    }
    UartPutObjByPointer(handle);
}

#ifdef __USER__
static int32_t UartUserReceive(DevHandle handle, void *data, uint32_t size, enum UartIoCmd cmd)
{
    int32_t ret;
    uint32_t rLen;
    const void *rBuf = NULL;
    struct HdfSBuf *reply = NULL;
    struct HdfIoService *service = (struct HdfIoService *)handle;

    if (service == NULL || service->dispatcher == NULL || service->dispatcher->Dispatch == NULL) {
        HDF_LOGE("%s: service is invalid", __func__);
        return HDF_ERR_INVALID_PARAM;
    }
    /* Four bytes are used to store the buffer length, and four bytes are used to align the memory. */
    reply = HdfSBufObtain(size + sizeof(uint64_t));
    if (reply == NULL) {
        HDF_LOGE("%s: failed to obtain reply", __func__);
        return HDF_ERR_MALLOC_FAIL;
    }
    ret = service->dispatcher->Dispatch(&service->object, cmd, NULL, reply);
    if (ret != HDF_SUCCESS) {
        HDF_LOGE("%s: failed to read, ret %d", __func__, ret);
        goto __EXIT;
    }
    if (!HdfSbufReadBuffer(reply, &rBuf, &rLen)) {
        HDF_LOGE("%s: sbuf read buffer failed", __func__);
        ret = HDF_ERR_IO;
        goto __EXIT;
    }
    if (size != rLen && cmd != UART_IO_READ) {
        HDF_LOGE("%s: read error, size %u, rLen %u", __func__, size, rLen);
        ret = HDF_FAILURE;
        goto __EXIT;
    }
    if (memcpy_s(data, size, rBuf, rLen) != EOK) {
        HDF_LOGE("%s: memcpy rBuf failed", __func__);
        ret = HDF_ERR_IO;
        goto __EXIT;
    }
    if (cmd == UART_IO_READ) {
        ret = rLen;
    }
__EXIT:
    HdfSBufRecycle(reply);
    return ret;
}

static int32_t UartUserSend(DevHandle handle, void *data, uint32_t size, enum UartIoCmd cmd)
{
    int32_t ret;
    struct HdfSBuf *buf = NULL;
    struct HdfIoService *service = (struct HdfIoService *)handle;

    if (service == NULL || service->dispatcher == NULL || service->dispatcher->Dispatch == NULL) {
        HDF_LOGE("%s: service is invalid", __func__);
        return HDF_ERR_INVALID_PARAM;
    }
    buf = HdfSBufObtain(size);
    if (buf == NULL) {
        HDF_LOGE("%s: failed to obtain buf", __func__);
        return HDF_ERR_MALLOC_FAIL;
    }
    if (!HdfSbufWriteBuffer(buf, data, size)) {
        HDF_LOGE("%s: sbuf write buffer failed", __func__);
        HdfSBufRecycle(buf);
        return HDF_ERR_IO;
    }

    ret = service->dispatcher->Dispatch(&service->object, cmd, buf, NULL);
    if (ret != HDF_SUCCESS) {
        HDF_LOGE("%s: failed to write, ret %d", __func__, ret);
    }
    HdfSBufRecycle(buf);
    return ret;
}
#endif

int32_t UartRead(DevHandle handle, uint8_t *data, uint32_t size)
{
#ifdef __USER__
    return UartUserReceive(handle, data, size, UART_IO_READ);
#else
    return UartHostRead((struct UartHost *)handle, data, size);
#endif
}

int32_t UartWrite(DevHandle handle, uint8_t *data, uint32_t size)
{
#ifdef __USER__
    return UartUserSend(handle, data, size, UART_IO_WRITE);
#else
    return UartHostWrite((struct UartHost *)handle, data, size);
#endif
}

int32_t UartGetBaud(DevHandle handle, uint32_t *baudRate)
{
#ifdef __USER__
    return UartUserReceive(handle, baudRate, sizeof(*baudRate), UART_IO_GET_BAUD);
#else
    return UartHostGetBaud((struct UartHost *)handle, baudRate);
#endif
}

int32_t UartSetBaud(DevHandle handle, uint32_t baudRate)
{
#ifdef __USER__
    return UartUserSend(handle, &baudRate, sizeof(baudRate), UART_IO_SET_BAUD);
#else
    return UartHostSetBaud((struct UartHost *)handle, baudRate);
#endif
}

int32_t UartGetAttribute(DevHandle handle, struct UartAttribute *attribute)
{
#ifdef __USER__
    return UartUserReceive(handle, attribute, sizeof(*attribute), UART_IO_GET_ATTRIBUTE);
#else
    return UartHostGetAttribute((struct UartHost *)handle, attribute);
#endif
}

int32_t UartSetAttribute(DevHandle handle, struct UartAttribute *attribute)
{
#ifdef __USER__
    return UartUserSend(handle, attribute, sizeof(*attribute), UART_IO_SET_ATTRIBUTE);
#else
    return UartHostSetAttribute((struct UartHost *)handle, attribute);
#endif
}

int32_t UartSetTransMode(DevHandle handle, enum UartTransMode mode)
{
#ifdef __USER__
    return UartUserSend(handle, &mode, sizeof(mode), UART_IO_SET_TRANSMODE);
#else
    return UartHostSetTransMode((struct UartHost *)handle, mode);
#endif
}
