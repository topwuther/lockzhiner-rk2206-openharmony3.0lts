/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
 *
 * HDF is dual licensed: you can use it either under the terms of
 * the GPL, or the BSD license, at your option.
 * See the LICENSE file in the root of this repository for complete details.
 */

#include "hi3516_codec_ops.h"
#include "audio_codec_base.h"
#include "audio_core.h"

#define HDF_LOG_TAG hi3516_codec_adapter

struct CodecData g_codecData = {
    .Init = CodecDeviceInit,
    .Read = CodecDeviceReadReg,
    .Write = CodecDeviceWriteReg,
    .AiaoRead = CodecAiaoDeviceReadReg,
    .AiaoWrite = CodecAiaoDeviceWriteReg,
};

struct AudioDaiOps g_codecDaiDeviceOps = {
    .Startup = CodecDaiStartup,
    .HwParams = CodecDaiHwParams,
};

struct DaiData g_codecDaiData = {
    .DaiInit = CodecDaiDeviceInit,
    .ops = &g_codecDaiDeviceOps,
};

/* HdfDriverEntry implementations */
static int32_t CodecDriverBind(struct HdfDeviceObject *device)
{
    struct CodecHost *codecHost = NULL;

    AUDIO_DRIVER_LOG_DEBUG("entry!");

    if (device == NULL) {
        AUDIO_DRIVER_LOG_ERR("input para is NULL.");
        return HDF_FAILURE;
    }

    codecHost = (struct CodecHost *)OsalMemCalloc(sizeof(*codecHost));
    if (codecHost == NULL) {
        AUDIO_DRIVER_LOG_ERR("malloc codecHost fail!");
        return HDF_FAILURE;
    }

    codecHost->device = device;
    device->service = &codecHost->service;

    AUDIO_DRIVER_LOG_INFO("success!");
    return HDF_SUCCESS;
}

static int32_t CodecDriverInit(struct HdfDeviceObject *device)
{
    int32_t ret;

    AUDIO_DRIVER_LOG_DEBUG("entry.");
    if (device == NULL) {
        AUDIO_DRIVER_LOG_ERR("device is NULL.");
        return HDF_ERR_INVALID_OBJECT;
    }

    ret = CodecGetServiceName(device, &g_codecData.drvCodecName);
    if (ret != HDF_SUCCESS) {
        AUDIO_DRIVER_LOG_ERR("get codec service name fail.");
        return ret;
    }

    ret = CodecGetDaiName(device, &g_codecDaiData.drvDaiName);
    if (ret != HDF_SUCCESS) {
        AUDIO_DRIVER_LOG_ERR("get codec dai name fail.");
        return ret;
    }

    ret = AudioRegisterCodec(device, &g_codecData, &g_codecDaiData);
    if (ret != HDF_SUCCESS) {
        AUDIO_DRIVER_LOG_ERR("register dai fail.");
        return ret;
    }

    AUDIO_DRIVER_LOG_INFO("Success.");
    return HDF_SUCCESS;
}

static void CodecDriverRelease(struct HdfDeviceObject *device)
{
    struct CodecHost *codecHost = NULL;
    struct VirtualAddress *virtualAdd = NULL;

    if (device == NULL) {
        AUDIO_DRIVER_LOG_ERR("device is NULL");
        return;
    }

    if (device->priv != NULL) {
        virtualAdd = (struct VirtualAddress *)device->priv;
        OsalIoUnmap((void *)((uintptr_t)(void*)&virtualAdd->acodecVir));
        OsalIoUnmap((void *)((uintptr_t)(void*)&virtualAdd->aiaoVir));
        OsalMemFree(device->priv);
    }

    codecHost = (struct CodecHost *)device->service;
    if (codecHost == NULL) {
        HDF_LOGE("CodecDriverRelease: codecHost is NULL");
        return;
    }
    OsalMemFree(codecHost);
}

/* HdfDriverEntry definitions */
struct HdfDriverEntry g_codecDriverEntry = {
    .moduleVersion = 1,
    .moduleName = "CODEC_HI3516",
    .Bind = CodecDriverBind,
    .Init = CodecDriverInit,
    .Release = CodecDriverRelease,
};
HDF_INIT(g_codecDriverEntry);
