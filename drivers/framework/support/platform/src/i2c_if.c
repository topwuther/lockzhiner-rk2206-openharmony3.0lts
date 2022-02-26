/*
 * Copyright (c) 2020-2021 Huawei Device Co., Ltd.
 *
 * HDF is dual licensed: you can use it either under the terms of
 * the GPL, or the BSD license, at your option.
 * See the LICENSE file in the root of this repository for complete details.
 */

#include "i2c_if.h"
#ifndef __USER__
#include "devsvc_manager_clnt.h"
#endif
#include "hdf_base.h"
#ifdef __USER__
#include "hdf_io_service_if.h"
#endif
#include "hdf_log.h"
#ifndef __USER__
#include "i2c_core.h"
#endif
#include "osal_mem.h"
#include "securec.h"

#define HDF_LOG_TAG i2c_if

#define I2C_SERVICE_NAME "HDF_PLATFORM_I2C_MANAGER"

#ifdef __USER__
enum I2cIoCmd {
    I2C_IO_TRANSFER = 0,
    I2C_IO_OPEN = 1,
    I2C_IO_CLOSE = 2,
};

static void *I2cManagerGetService(void)
{
    static void *manager = NULL;

    if (manager != NULL) {
        return manager;
    }
    manager = (void *)HdfIoServiceBind(I2C_SERVICE_NAME);
    if (manager == NULL) {
        HDF_LOGE("I2cManagerGetService: fail to get i2c manager!");
    }
    return manager;
}
#endif

DevHandle I2cOpen(int16_t number)
{
#ifdef __USER__
    int32_t ret;
    struct HdfIoService *service = NULL;
    struct HdfSBuf *data = NULL;
    struct HdfSBuf *reply = NULL;
    uint32_t handle;

    service = (struct HdfIoService *)I2cManagerGetService();
    if (service == NULL) {
        return NULL;
    }
    data = HdfSBufObtainDefaultSize();
    if (data == NULL) {
        return NULL;
    }
    reply = HdfSBufObtainDefaultSize();
    if (reply == NULL) {
        HdfSBufRecycle(data);
        return NULL;
    }

    if (!HdfSbufWriteUint16(data, (uint16_t)number)) {
        HDF_LOGE("I2cOpen: write number fail!");
        HdfSBufRecycle(data);
        HdfSBufRecycle(reply);
        return NULL;
    }

    ret = service->dispatcher->Dispatch(&service->object, I2C_IO_OPEN, data, reply);
    if (ret != HDF_SUCCESS) {
        HDF_LOGE("I2cOpen: service call open fail:%d", ret);
        HdfSBufRecycle(data);
        HdfSBufRecycle(reply);
        return NULL;
    }

    if (!HdfSbufReadUint32(reply, &handle)) {
        HDF_LOGE("I2cOpen: read handle fail!");
        HdfSBufRecycle(data);
        HdfSBufRecycle(reply);
        return NULL;
    }
    HdfSBufRecycle(data);
    HdfSBufRecycle(reply);
    return (DevHandle)(uintptr_t)handle;
#else
    return (DevHandle)I2cCntlrGet(number);
#endif
}

void I2cClose(DevHandle handle)
{
#ifdef __USER__
    int32_t ret;
    struct HdfIoService *service = NULL;
    struct HdfSBuf *data = NULL;

    service = (struct HdfIoService *)I2cManagerGetService();
    if (service == NULL) {
        return;
    }

    data = HdfSBufObtainDefaultSize();
    if (data == NULL) {
        return;
    }

    if (!HdfSbufWriteUint32(data, (uint32_t)(uintptr_t)handle)) {
        HDF_LOGE("I2cClose: write handle fail!");
        HdfSBufRecycle(data);
        return;
    }

    ret = service->dispatcher->Dispatch(&service->object, I2C_IO_CLOSE, data, NULL);
    if (ret != HDF_SUCCESS) {
        HDF_LOGE("I2cClose: close handle fail:%d", ret);
    }
    HdfSBufRecycle(data);
#else
    I2cCntlrPut((struct I2cCntlr *)handle);
#endif
}

#ifdef __USER__
static int32_t I2cMsgWriteArray(DevHandle handle, struct HdfSBuf *data, struct I2cMsg *msgs, int16_t count)
{
    int16_t i;

    if (!HdfSbufWriteUint32(data, (uint32_t)(uintptr_t)handle)) {
        HDF_LOGE("I2cMsgWriteArray: write handle fail!");
        return HDF_ERR_IO;
    }

    if (!HdfSbufWriteBuffer(data, (uint8_t *)msgs, sizeof(*msgs) * count)) {
        HDF_LOGE("I2cMsgWriteArray: write msgs array fail!");
        return HDF_ERR_IO;
    }

    for (i = 0; i < count; i++) {
        if ((msgs[i].flags & I2C_FLAG_READ) != 0) {
            continue;
        }
        if (!HdfSbufWriteBuffer(data, (uint8_t *)msgs[i].buf, msgs[i].len)) {
            HDF_LOGE("I2cMsgWriteArray: write msg[%d] buf fail!", i);
            return HDF_ERR_IO;
        }
    }

    return HDF_SUCCESS;
}

static int32_t I2cMsgReadBack(struct HdfSBuf *data, struct I2cMsg *msg)
{
    uint32_t rLen;
    const void *rBuf = NULL;

    if ((msg->flags & I2C_FLAG_READ) == 0) {
        return HDF_SUCCESS; /* write msg no need to read back */
    }

    if (!HdfSbufReadBuffer(data, &rBuf, &rLen)) {
        HDF_LOGE("I2cMsgReadBack: read rBuf fail!");
        return HDF_ERR_IO;
    }
    if (msg->len != rLen) {
        HDF_LOGE("I2cMsgReadBack: err len:%u, rLen:%u", msg->len, rLen);
        if (rLen > msg->len) {
            rLen = msg->len;
        }
    }
    if (memcpy_s(msg->buf, msg->len, rBuf, rLen) != EOK) {
        HDF_LOGE("I2cMsgReadBack: memcpy rBuf fail!");
        return HDF_ERR_IO;
    }

    return HDF_SUCCESS;
}

static inline int32_t I2cMsgReadArray(struct HdfSBuf *reply, struct I2cMsg *msgs, int16_t count)
{
    int16_t i;
    int32_t ret;

    for (i = 0; i < count; i++) {
        ret = I2cMsgReadBack(reply, &msgs[i]);
        if (ret != HDF_SUCCESS) {
            return ret;
        }
    }
    return HDF_SUCCESS;
}

static int32_t I2cServiceTransfer(DevHandle handle, struct I2cMsg *msgs, int16_t count)
{
    int16_t i;
    int32_t ret;
    uint32_t recvLen = 0;
    struct HdfSBuf *data = NULL;
    struct HdfSBuf *reply = NULL;
    struct HdfIoService *service = NULL;

    data = HdfSBufObtainDefaultSize();
    if (data == NULL) {
        HDF_LOGE("I2cServiceTransfer: failed to obtain data!");
        return HDF_ERR_MALLOC_FAIL;
    }

    ret = I2cMsgWriteArray(handle, data, msgs, count);
    if (ret != HDF_SUCCESS) {
        HDF_LOGE("I2cServiceTransfer: failed to write msgs!");
        goto __EXIT;
    }

    for (i = 0; i < count; i++) {
        recvLen += ((msgs[i].flags & I2C_FLAG_READ) == 0) ? 0 : (msgs[i].len + sizeof(uint64_t));
    }
    reply = (recvLen == 0) ? HdfSBufObtainDefaultSize() : HdfSBufObtain(recvLen);
    if (reply == NULL) {
        HDF_LOGE("I2cServiceTransfer: failed to obtain reply!");
        ret = HDF_ERR_MALLOC_FAIL;
        goto __EXIT;
    }

    service = I2cManagerGetService();
    if (service == NULL) {
        ret = HDF_ERR_NOT_SUPPORT;
        goto __EXIT;
    }
    ret = service->dispatcher->Dispatch(&service->object, I2C_IO_TRANSFER, data, reply);
    if (ret != HDF_SUCCESS) {
        HDF_LOGE("I2cServiceTransfer: failed to send service call:%d", ret);
        goto __EXIT;
    }

    ret = I2cMsgReadArray(reply, msgs, count);
    if (ret != HDF_SUCCESS) {
        goto __EXIT;
    }

    ret = count;
__EXIT:
    if (data != NULL) {
        HdfSBufRecycle(data);
    }
    if (reply != NULL) {
        HdfSBufRecycle(reply);
    }
    return ret;
}
#endif

int32_t I2cTransfer(DevHandle handle, struct I2cMsg *msgs, int16_t count)
{
    if (handle == NULL) {
        return HDF_ERR_INVALID_OBJECT;
    }

    if (msgs == NULL || count <= 0) {
        HDF_LOGE("I2cTransfer: err params! msgs:%s, count:%d",
            (msgs == NULL) ? "0" : "x", count);
        return HDF_ERR_INVALID_PARAM;
    }

#ifdef __USER__
    return I2cServiceTransfer(handle, msgs, count);
#else
    return I2cCntlrTransfer((struct I2cCntlr *)handle, msgs, count);
#endif
}

