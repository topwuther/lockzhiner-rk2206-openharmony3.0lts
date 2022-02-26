/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
 *
 * HDF is dual licensed: you can use it either under the terms of
 * the GPL, or the BSD license, at your option.
 * See the LICENSE file in the root of this repository for complete details.
 */

#include "tfa9879_accessory_impl.h"
#include "audio_accessory_if.h"
#include "audio_core.h"
#include "audio_sapm.h"
#include "hdf_log.h"

#define HDF_LOG_TAG "accessory"

struct AccessoryData g_accessoryData = {
    .Init = AccessoryDeviceInit,
    .Read = AccessoryDeviceReadReg,
    .Write = AccessoryDeviceWriteReg,
    .AiaoRead = AccessoryAiaoDeviceReadReg,
    .AiaoWrite = AccessoryAiaoDeviceWriteReg,
};

struct AudioDaiOps g_accessoryDaiDeviceOps = {
    .Startup = AccessoryDaiStartup,
    .HwParams = AccessoryDaiHwParams,
};

struct DaiData g_accessoryDaiData = {
    .drvDaiName = "accessory_dai",
    .DaiInit = AccessoryDaiDeviceInit,
    .ops = &g_accessoryDaiDeviceOps,
};

/* HdfDriverEntry */
static int32_t GetServiceName(const struct HdfDeviceObject *device)
{
    const struct DeviceResourceNode *node = NULL;
    struct DeviceResourceIface *drsOps = NULL;
    int32_t ret;
    if (device == NULL) {
        AUDIO_DRIVER_LOG_ERR("input HdfDeviceObject object is nullptr.");
        return HDF_FAILURE;
    }
    node = device->property;
    if (node == NULL) {
        AUDIO_DRIVER_LOG_ERR("get drs node is nullptr.");
        return HDF_FAILURE;
    }
    drsOps = DeviceResourceGetIfaceInstance(HDF_CONFIG_SOURCE);
    if (drsOps == NULL || drsOps->GetString == NULL) {
        AUDIO_DRIVER_LOG_ERR("drsOps or drsOps getString is null!");
        return HDF_FAILURE;
    }
    ret = drsOps->GetString(node, "serviceName", &g_accessoryData.drvAccessoryName, 0);
    if (ret != HDF_SUCCESS) {
        AUDIO_DRIVER_LOG_ERR("read serviceName fail!");
        return ret;
    }
    return HDF_SUCCESS;
}

/* HdfDriverEntry implementations */
static int32_t AccessoryDriverInit(struct HdfDeviceObject *device)
{
    int32_t ret;
    AUDIO_DRIVER_LOG_DEBUG("entry.\n");
    if (device == NULL) {
        AUDIO_DRIVER_LOG_ERR("device is NULL.");
        return HDF_ERR_INVALID_OBJECT;
    }
    ret = GetServiceName(device);
    if (ret !=  HDF_SUCCESS) {
        AUDIO_DRIVER_LOG_ERR("get service name fail.");
        return ret;
    }
    ret = AudioRegisterAccessory(device, &g_accessoryData, &g_accessoryDaiData);
    if (ret !=  HDF_SUCCESS) {
        AUDIO_DRIVER_LOG_ERR("register dai fail.");
        return ret;
    }
    AUDIO_DRIVER_LOG_INFO("success!");
    return HDF_SUCCESS;
}

/* HdfDriverEntry implementations */
static int32_t AccessoryDriverBind(struct HdfDeviceObject *device)
{
    (void)device;
    AUDIO_DRIVER_LOG_INFO("success!");
    return HDF_SUCCESS;
}

/* HdfDriverEntry definitions */
struct HdfDriverEntry g_accessoryDriverEntry = {
    .moduleVersion = 1,
    .moduleName = "CODEC_TFA9879",
    .Bind = AccessoryDriverBind,
    .Init = AccessoryDriverInit,
    .Release = NULL,
};
HDF_INIT(g_accessoryDriverEntry);
