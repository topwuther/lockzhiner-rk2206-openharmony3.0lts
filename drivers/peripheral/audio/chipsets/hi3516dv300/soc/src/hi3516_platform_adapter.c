/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
 *
 * HDF is dual licensed: you can use it either under the terms of
 * the GPL, or the BSD license, at your option.
 * See the LICENSE file in the root of this repository for complete details.
 */
 
#include "gpio_if.h"
#include <linux/slab.h>
#include "audio_core.h"
#include "hi3516_platform_ops.h"
#include "osal_mem.h"

#define HDF_LOG_TAG hi3516_platform_adapter

struct AudioPlatformOps g_platformDeviceOps = {
    .HwParams = PlatformHwParams,
    .Write = PlatformWrite,
    .Read = PlatformRead,
    .MmapWrite = PlatformMmapWrite,
    .MmapRead = PlatformMmapRead,
    .RenderPrepare = PlatformRenderPrepare,
    .CapturePrepare = PlatformCapturePrepare,
    .RenderStart = PlatformRenderStart,
    .CaptureStart = PlatformCaptureStart,
    .RenderStop = PlatformRenderStop,
    .CaptureStop = PlatformCaptureStop,
    .RenderPause = PlatformRenderPause,
    .CapturePause = PlatformCapturePause,
    .RenderResume = PlatformRenderResume,
    .CaptureResume = PlatformCaptureResume,
};

struct PlatformData g_platformData = {
    .PlatformInit = AudioPlatformDeviceInit,
    .ops = &g_platformDeviceOps,
};

/* HdfDriverEntry implementations */
static int32_t PlatformDriverBind(struct HdfDeviceObject *device)
{
    struct PlatformHost *platformHost = NULL;
    AUDIO_DRIVER_LOG_DEBUG("entry!");

    if (device == NULL) {
        AUDIO_DRIVER_LOG_ERR("input para is NULL.");
        return HDF_FAILURE;
    }

    platformHost = (struct PlatformHost *)OsalMemCalloc(sizeof(*platformHost));
    if (platformHost == NULL) {
        AUDIO_DRIVER_LOG_ERR("malloc host fail!");
        return HDF_FAILURE;
    }

    platformHost->device = device;
    platformHost->platformInitFlag = false;
    device->service = &platformHost->service;

    AUDIO_DRIVER_LOG_DEBUG("success!");
    return HDF_SUCCESS;
}

static int32_t PlatformGetServiceName(const struct HdfDeviceObject *device)
{
    const struct DeviceResourceNode *node = NULL;
    struct DeviceResourceIface *drsOps = NULL;
    int32_t ret;

    if (device == NULL) {
        AUDIO_DRIVER_LOG_ERR("para is NULL.");
        return HDF_FAILURE;
    }

    node = device->property;
    if (node == NULL) {
        AUDIO_DRIVER_LOG_ERR("node is NULL.");
        return HDF_FAILURE;
    }

    drsOps = DeviceResourceGetIfaceInstance(HDF_CONFIG_SOURCE);
    if (drsOps == NULL || drsOps->GetString == NULL) {
        AUDIO_DRIVER_LOG_ERR("get drsops object instance fail!");
        return HDF_FAILURE;
    }

    ret = drsOps->GetString(node, "serviceName", &g_platformData.drvPlatformName, 0);
    if (ret != HDF_SUCCESS) {
        AUDIO_DRIVER_LOG_ERR("read serviceName fail!");
        return ret;
    }

    return HDF_SUCCESS;
}

static int32_t PlatformDriverInit(struct HdfDeviceObject *device)
{
    int32_t ret;

    AUDIO_DRIVER_LOG_DEBUG("entry.\n");
    struct PlatformHost *platformHost = NULL;

    if (device == NULL) {
        AUDIO_DRIVER_LOG_ERR("device is NULL.");
        return HDF_ERR_INVALID_OBJECT;
    }

    ret = PlatformGetServiceName(device);
    if (ret !=  HDF_SUCCESS) {
        AUDIO_DRIVER_LOG_ERR("get service name fail.");
        return ret;
    }

    ret = AudioSocDeviceRegister(device, (void *)&g_platformData, AUDIO_PLATFORM_DEVICE);
    if (ret !=  HDF_SUCCESS) {
        AUDIO_DRIVER_LOG_ERR("register dai fail.");
        return ret;
    }

    platformHost = (struct PlatformHost *)device->service;
    if (NULL != platformHost) {
        OsalMutexInit(&platformHost->renderBufInfo.buffMutex);
        OsalMutexInit(&platformHost->captureBufInfo.buffMutex);
    }

    AUDIO_DRIVER_LOG_INFO("success.\n");
    return HDF_SUCCESS;
}

static void PlatformDriverRelease(struct HdfDeviceObject *device)
{
    struct PlatformHost *platformHost = NULL;

    if (device == NULL) {
        AUDIO_DRIVER_LOG_ERR("device is NULL");
        return;
    }

    platformHost = (struct PlatformHost *)device->service;
    if (platformHost == NULL) {
        AUDIO_DRIVER_LOG_ERR("platformHost is NULL");
        return;
    }

    OsalMutexDestroy(&platformHost->renderBufInfo.buffMutex);
    OsalMutexDestroy(&platformHost->captureBufInfo.buffMutex);
    OsalMemFree(platformHost);
}

/* HdfDriverEntry definitions */
struct HdfDriverEntry g_platformDriverEntry = {
    .moduleVersion = 1,
    .moduleName = "DMA_HI3516",
    .Bind = PlatformDriverBind,
    .Init = PlatformDriverInit,
    .Release = PlatformDriverRelease,
};
HDF_INIT(g_platformDriverEntry);
