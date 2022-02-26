/*
 * i2c_adapter.h
 *
 * i2c driver adapter of linux
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

#include <linux/i2c.h>
#include "hdf_device_desc.h"
#include "hdf_log.h"
#include "i2c_core.h"
#include "osal_mem.h"

#define HDF_LOG_TAG i2c_linux_adapter

static struct i2c_msg *CreateLinuxI2cMsgs(struct I2cMsg *msgs, int16_t count)
{
    int16_t i;
    struct i2c_msg *linuxMsgs = NULL;

    linuxMsgs = (struct i2c_msg *)OsalMemCalloc(sizeof(*linuxMsgs) * count);
    if (linuxMsgs == NULL) {
        HDF_LOGE("%s: malloc linux msgs fail!", __func__);
        return NULL;
    }

    for (i = 0; i < count; i++) {
        linuxMsgs[i].addr = msgs[i].addr;
        linuxMsgs[i].buf = msgs[i].buf;
        linuxMsgs[i].len = msgs[i].len;
        linuxMsgs[i].flags = msgs[i].flags;
    }
    return linuxMsgs;
}

static inline void FreeLinxI2cMsgs(struct i2c_msg *msgs, int16_t count)
{
    OsalMemFree(msgs);
    (void)count;
}

static int32_t LinuxI2cTransfer(struct I2cCntlr *cntlr, struct I2cMsg *msgs, int16_t count)
{
    int32_t ret;
    struct i2c_msg *linuxMsgs = NULL;

    if (cntlr == NULL || cntlr->priv == NULL) {
        HDF_LOGE("%s: cntlr or priv is null!", __func__);
        return HDF_ERR_INVALID_OBJECT;
    }
    if (msgs == NULL || count <= 0) {
        HDF_LOGE("%s: err params! count:%d", __func__, count);
        return HDF_ERR_INVALID_PARAM;
    }

    linuxMsgs = CreateLinuxI2cMsgs(msgs, count);
    if (linuxMsgs == NULL) {
        return HDF_ERR_MALLOC_FAIL;
    }

    ret = i2c_transfer((struct i2c_adapter *)cntlr->priv, linuxMsgs, count);
    FreeLinxI2cMsgs(linuxMsgs, count);
    return ret;
}

static struct I2cMethod g_method = {
    .transfer = LinuxI2cTransfer,
};

static int32_t LinuxI2cBind(struct HdfDeviceObject *device)
{
    (void)device;
    return HDF_SUCCESS;
}

static int LinuxI2cRemove(struct device *dev, void *data)
{
    struct I2cCntlr *cntlr = NULL;
    struct i2c_adapter *adapter = NULL;

    HDF_LOGI("%s: Enter", __func__);
    (void)data;

    if (dev == NULL) {
        HDF_LOGE("%s: dev is null", __func__);
        return HDF_ERR_INVALID_OBJECT;
    }

    if (dev->type != &i2c_adapter_type) {
        return HDF_SUCCESS; // continue remove
    }

    adapter = to_i2c_adapter(dev);
    cntlr = I2cCntlrGet(adapter->nr);
    if (cntlr != NULL) {
        I2cCntlrPut(cntlr);
        I2cCntlrRemove(cntlr);
        i2c_put_adapter(adapter);
        OsalMemFree(cntlr);
    }
    return HDF_SUCCESS;
}

static int LinuxI2cProbe(struct device *dev, void *data)
{
    int32_t ret;
    struct I2cCntlr *cntlr = NULL;
    struct i2c_adapter *adapter = NULL;

    (void)data;

    if (dev == NULL) {
        HDF_LOGE("%s: dev is null", __func__);
        return HDF_ERR_INVALID_OBJECT;
    }

    if (dev->type != &i2c_adapter_type) {
        return HDF_SUCCESS; // continue probe
    }

    HDF_LOGI("%s: Enter", __func__);
    adapter = to_i2c_adapter(dev);
    cntlr = (struct I2cCntlr *)OsalMemCalloc(sizeof(*cntlr));
    if (cntlr == NULL) {
        HDF_LOGE("%s: malloc cntlr fail!", __func__);
        return HDF_ERR_MALLOC_FAIL;
    }

    cntlr->busId = adapter->nr;
    cntlr->priv = adapter;
    cntlr->ops = &g_method;
    ret = I2cCntlrAdd(cntlr);
    if (ret != HDF_SUCCESS) {
        i2c_put_adapter(adapter);
        HDF_LOGE("%s: add controller fail:%d", __func__, ret);
        return ret;
    }
    HDF_LOGI("%s: i2c adapter %d add success", __func__, cntlr->busId);
    return HDF_SUCCESS;
}

static int32_t LinuxI2cInit(struct HdfDeviceObject *device)
{
    int32_t ret;

    HDF_LOGI("%s: Enter", __func__);
    if (device == NULL) {
        HDF_LOGE("%s: device is NULL", __func__);
        return HDF_ERR_INVALID_OBJECT;
    }

    ret = i2c_for_each_dev(NULL, LinuxI2cProbe);
    HDF_LOGI("%s: done", __func__);
    return ret;
}

static void LinuxI2cRelease(struct HdfDeviceObject *device)
{
    HDF_LOGI("%s: enter", __func__);
    if (device == NULL) {
        HDF_LOGE("%s: device is NULL", __func__);
        return;
    }

    (void)i2c_for_each_dev(NULL, LinuxI2cRemove);
}

struct HdfDriverEntry g_i2cLinuxDriverEntry = {
    .moduleVersion = 1,
    .Bind = LinuxI2cBind,
    .Init = LinuxI2cInit,
    .Release = LinuxI2cRelease,
    .moduleName = "linux_i2c_adapter",
};
HDF_INIT(g_i2cLinuxDriverEntry);
