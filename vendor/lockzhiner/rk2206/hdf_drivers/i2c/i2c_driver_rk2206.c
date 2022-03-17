/*
 * Copyright (c) 2022 FuZhou Lockzhiner Electronic Co., Ltd. All rights reserved.
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

#include "i2c_driver_rk2206.h"
#include <stdlib.h>
#include <securec.h>
#include "i2c_core.h"
#include "i2c_if.h"
#include "hdf_device_desc.h"
#include "hdf_log.h"

#define DEC_NUM 10
#define GROUP_PIN_NUM 8
#define I2C_INVALID_ADDR 0xFFFF


#if 0
/* I2cHostMethod method definitions */
static int32_t i2cHostTransfer(struct I2cCntlr *cntlr, struct I2cMsg *msgs, int16_t count);

struct I2cMethod g_i2cHostMethod = {
    .transfer = i2cHostTransfer,
};

static int32_t i2cHostTransfer(struct I2cCntlr *cntlr, struct I2cMsg *msgs, int16_t count)
{
    struct I2cDevice *device = NULL;
    if (cntlr == NULL || msgs == NULL || cntlr->priv == NULL) {
        HDF_LOGE("%s: I2cCntlr or msgs is NULL\r\n", __func__);
        return HDF_ERR_INVALID_PARAM;
    }

    device = (struct I2cDevice *)cntlr->priv;
    if (device == NULL) {
        HDF_LOGE("%s: I2cDevice is NULL\r\n", __func__);
        return HDF_DEV_ERR_NO_DEVICE;
    }

    return i2c_transfer(device, msgs, count);
}

static int32_t i2c_transfer(struct I2cDevice *device, struct I2cMsg *msgs, int16_t count)
{
    int ret;
    struct I2cMsg *msg = NULL;
    struct I2cMsg *msg2 = NULL;
    uint32_t i2cPort;
    if (device == NULL || msgs == NULL) {
        HDF_LOGE("%s: device or  msgs is NULL\r\n", __func__);
        return HDF_ERR_INVALID_PARAM;
    }

    i2cPort = (uint32_t)device->port;
    if (i2cPort > 0) {
        HDF_LOGE("i2c port %u not support\r\n", i2cPort);
        return HDF_ERR_NOT_SUPPORT;
    }
    if (HDF_SUCCESS != OsalMutexLock(&device->mutex)) {
        HDF_LOGE("%s %d OsalMutexTimedLock fail\r\n", __func__, __LINE__);
        return HDF_ERR_TIMEOUT;
    }
    for (int32_t i = 0; i < count; i++) {
        msg = &msgs[i];
        if (msg->flags == I2C_FLAG_READ) {
            ret = hal_i2c_task_recv(i2cPort, msg->addr, msg->buf, 0, msg->buf, msg->len, 0, NULL);
            if (ret) {
                HDF_LOGE("%s:%d,i2c recev fail, dev_addr = 0x%x, ret = %d\r\n", __func__, __LINE__, msg->addr, ret);
                OsalMutexUnlock(&device->mutex);
                return i;
            }
        } else if (msg->flags == I2C_FLAG_STOP) {
            i++;
            msg2 = &msgs[i];
            ret = hal_i2c_task_recv(i2cPort, msg->addr, msg->buf, msg->len, msg2->buf, msg2->len, 0, NULL);
            if (ret) {
                HDF_LOGE("%s:%d,i2c recev fail, dev_addr = 0x%x, ret = %d\r\n", __func__, __LINE__, msg->addr, ret);
                OsalMutexUnlock(&device->mutex);
                return i;
            }
        } else {
            ret = hal_i2c_task_send(i2cPort, msg->addr, msg->buf, msg->len, 0, NULL);
            if (ret) {
                HDF_LOGE("%s:%d,i2c send fail, dev_addr = 0x%x, ret = %d\r\n", __func__, __LINE__, msg->addr, ret);
                OsalMutexUnlock(&device->mutex);
                return i;
            }
        }
    }
    OsalMutexUnlock(&device->mutex);
    return count;
}
#endif


int32_t InitI2cDevice(struct I2cDevice *device)
{
    int32_t ret = -1;
    uint32_t i2cPort;
    struct I2cResource *resource = NULL;

    if (device == NULL) {
        HDF_LOGE("device is NULL\r\n");
        return HDF_ERR_INVALID_PARAM;
    }

    resource = &device->resource;
    if (resource == NULL) {
        HDF_LOGE("%s %d: invalid parameter\r\n", __func__, __LINE__);
        return HDF_ERR_INVALID_PARAM;
    }

    device->port = resource->port;
    i2cPort = device->port;
    if (i2cPort > 0) {
        HDF_LOGE("i2c port %u not support\r\n", i2cPort);
        return HDF_ERR_NOT_SUPPORT;
    }

    if (OsalMutexInit(&device->mutex) != HDF_SUCCESS) {
        HDF_LOGE("%s %d OsalMutexInit fail\r\n", __func__, __LINE__);
        return HDF_FAILURE;
    }

    if (HDF_SUCCESS != OsalMutexLock(&device->mutex)) {
        HDF_LOGE("%s %d osMutexWait fail\r\n", __func__, __LINE__);
        return HDF_ERR_TIMEOUT;
    }

    I2cBusIo i2c0m2 = {
        .scl =  {.gpio = GPIO0_PA1, .func = MUX_FUNC3, .type = PULL_NONE, .drv = DRIVE_KEEP, .dir = LZGPIO_DIR_KEEP, .val = LZGPIO_LEVEL_KEEP},
        .sda =  {.gpio = GPIO0_PA0, .func = MUX_FUNC3, .type = PULL_NONE, .drv = DRIVE_KEEP, .dir = LZGPIO_DIR_KEEP, .val = LZGPIO_LEVEL_KEEP},
        .id = FUNC_ID_I2C0,
        .mode = FUNC_MODE_M2,
    };

    if (i2cPort == 0)
    {
        if (I2cIoInit(i2c0m2) != LZ_HARDWARE_SUCCESS)
        {
            printf("init I2C I2C0 io fail\n");
        }
    }
    if (LzI2cInit(i2cPort, 100000) != LZ_HARDWARE_SUCCESS)
    {
        printf("init I2C I2C0 fail\n");
    }

    OsalMutexUnlock(&device->mutex);
    return ret;
}

static int32_t HostRestI2cDevice(struct I2cDevice *device)
{
    int32_t ret = -1;
    struct I2cResource *resource = NULL;
    uint32_t i2cPort;
    if (device == NULL) {
        HDF_LOGE("%s %d device is null\r\n", __func__, __LINE__);
        return HDF_ERR_INVALID_PARAM;
    }
    resource = &device->resource;
    if (resource == NULL) {
        HDF_LOGE("%s %d: invalid parameter\r\n", __func__, __LINE__);
        return HDF_ERR_INVALID_OBJECT;
    }

    device->port = resource->port;
    i2cPort = device->port;
    if (i2cPort > 0) {
        HDF_LOGE("i2c port %u not support\r\n", i2cPort);
        return HDF_ERR_NOT_SUPPORT;
    }

    LzI2cDeinit(i2cPort);

    return ret;
}

static uint32_t GetI2cDeviceResource(struct I2cDevice *device,
                                     const struct DeviceResourceNode *resourceNode)
{
    uint32_t tempPin = 0;
    struct I2cResource *resource = NULL;
    struct DeviceResourceIface *dri = NULL;
    if (device == NULL || resourceNode == NULL) {
        HDF_LOGE("device or resourceNode is NULL\r\n");
        return HDF_ERR_INVALID_PARAM;
    }
    resource = &device->resource;
    if (resource == NULL) {
        HDF_LOGE("%s %d: invalid parameter\r\n", __func__, __LINE__);
        return HDF_ERR_INVALID_OBJECT;
    }
    dri = DeviceResourceGetIfaceInstance(HDF_CONFIG_SOURCE);
    if (dri == NULL || dri->GetUint32 == NULL) {
        HDF_LOGE("DeviceResourceIface is invalid\r\n");
        return HDF_ERR_INVALID_OBJECT;
    }

    if (dri->GetUint32(resourceNode, "port", &resource->port, 0) != HDF_SUCCESS) {
        HDF_LOGE("i2c config port fail\r\n");
        return HDF_FAILURE;
    }

    if (dri->GetUint32(resourceNode, "sclPin", &tempPin, 0) != HDF_SUCCESS) {
        HDF_LOGE("i2c config sclPin fail\r\n");
        return HDF_FAILURE;
    }
    resource->sclPin = ((tempPin / DEC_NUM) * GROUP_PIN_NUM) + (tempPin % DEC_NUM);

    if (dri->GetUint32(resourceNode, "sdaPin", &tempPin, 0) != HDF_SUCCESS) {
        HDF_LOGE("i2c config sdaPin fail\r\n");
        return HDF_FAILURE;
    }
    resource->sdaPin = ((tempPin / DEC_NUM) * GROUP_PIN_NUM) + (tempPin % DEC_NUM);

    if (dri->GetUint32(resourceNode, "speed", &resource->speed, 0) != HDF_SUCCESS) {
        HDF_LOGE("i2c config speed fail\r\n");
        return HDF_FAILURE;
    }

    if (dri->GetUint32(resourceNode, "mode", &resource->mode, 0) != HDF_SUCCESS) {
        HDF_LOGE("i2c config mode fail\r\n");
        return HDF_FAILURE;
    }

    if (dri->GetUint32(resourceNode, "useDma", &resource->useDma, 0) != HDF_SUCCESS) {
        HDF_LOGE("i2c config useDma fail\r\n");
        return HDF_FAILURE;
    }

    if (dri->GetUint32(resourceNode, "useSync", &resource->useSync, 0) != HDF_SUCCESS) {
        HDF_LOGE("i2c config useSync fail\r\n");
        return HDF_FAILURE;
    }

    if (dri->GetUint32(resourceNode, "asMaster", &resource->asMaster, 0) != HDF_SUCCESS) {
        HDF_LOGE("i2c config asMaster fail\r\n");
        return HDF_FAILURE;
    }
    return HDF_SUCCESS;
}

static int32_t AttachI2cDevice(struct I2cCntlr *host, struct HdfDeviceObject *device)
{
    int32_t ret;
    struct I2cDevice *i2cDevice = NULL;
    struct I2cResource *resource = NULL;

    if (device == NULL || host == NULL) {
        HDF_LOGE("%s: device or host is NULL\r\n", __func__);
        return HDF_ERR_INVALID_PARAM;
    }
    i2cDevice = (struct I2cDevice *)OsalMemAlloc(sizeof(struct I2cDevice));
    if (i2cDevice == NULL) {
        HDF_LOGE("%s: OsalMemAlloc i2cDevice error\r\n", __func__);
        return HDF_ERR_MALLOC_FAIL;
    }
    (void)memset_s(i2cDevice, sizeof(struct I2cDevice), 0, sizeof(struct I2cDevice));
    ret = GetI2cDeviceResource(i2cDevice, device->property);
    if (ret != HDF_SUCCESS) {
        OsalMemFree(i2cDevice);
        return HDF_FAILURE;
    }
    resource = &i2cDevice->resource;
    if (resource == NULL) {
        HDF_LOGE("%s %d: invalid parameter\r\n", __func__, __LINE__);
        return HDF_ERR_INVALID_OBJECT;
    }

    host->priv = i2cDevice;
    host->busId = i2cDevice->port;

    return InitI2cDevice(i2cDevice);
}

static void i2cDriverRelease(struct HdfDeviceObject *device)
{
    struct I2cCntlr *i2cCntrl = NULL;
    struct I2cDevice *i2cDevice = NULL;

    if (device == NULL) {
        HDF_LOGE("%s: device is NULL\r\n", __func__);
        return;
    }
    i2cCntrl = device->priv;
    if (i2cCntrl == NULL || i2cCntrl->priv == NULL) {
        HDF_LOGE("%s: i2cCntrl is NULL\r\n", __func__);
        return;
    }
    i2cCntrl->ops = NULL;
    i2cDevice = (struct I2cDevice *)i2cCntrl->priv;
    OsalMemFree(i2cCntrl);

    if (i2cDevice != NULL) {
        OsalMutexDestroy(&i2cDevice->mutex);
        OsalMemFree(i2cDevice);
    }
}

static int32_t i2cDriverInit(struct HdfDeviceObject *device)
{
    int32_t ret;
    struct I2cCntlr *host = NULL;

    printf("%s, %d: entry!\n\n\n", __FILE__, __LINE__);

    if (device == NULL) {
        HDF_LOGE("%s: device is NULL\r\n", __func__);
        return HDF_ERR_INVALID_PARAM;
    }

    host = (struct I2cCntlr *)OsalMemAlloc(sizeof(struct I2cCntlr));
    if (host == NULL) {
        HDF_LOGE("%s: host is NULL\r\n", __func__);
        return HDF_ERR_MALLOC_FAIL;
    }
    (void)memset_s(host, sizeof(struct I2cCntlr), 0, sizeof(struct I2cCntlr));
    //host->ops = &g_i2cHostMethod;
    host->ops = NULL;
    device->priv = (void *)host;
    ret = AttachI2cDevice(host, device);
    if (ret != HDF_SUCCESS) {
        HDF_LOGE("%s: attach error\r\n", __func__);
        i2cDriverRelease(device);
        return HDF_DEV_ERR_ATTACHDEV_FAIL;
    }
    ret = I2cCntlrAdd(host);
    if (ret != HDF_SUCCESS) {
        i2cDriverRelease(device);
        return HDF_FAILURE;
    }
    return ret;
}

static int32_t i2cDriverBind(struct HdfDeviceObject *device)
{
    if (device == NULL) {
        HDF_LOGE("%s: I2c device object is NULL\r\n", __func__);
        return HDF_FAILURE;
    }
    return HDF_SUCCESS;
}




/* HdfDriverEntry definitions */
struct HdfDriverEntry g_i2cDriverEntry = {
    .moduleVersion = 1,
    .moduleName = "i2c_driver_rk2206",
    .Bind = i2cDriverBind,
    .Init = i2cDriverInit,
    .Release = i2cDriverRelease,
};

// Initialize HdfDriverEntry
HDF_INIT(g_i2cDriverEntry);

