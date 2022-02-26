/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
 *
 * HDF is dual licensed: you can use it either under the terms of
 * the GPL, or the BSD license, at your option.
 * See the LICENSE file in the root of this repository for complete details.
 */

#include "audio_host.h"
#include "audio_codec_if.h"
#include "audio_core.h"
#include "audio_parse.h"

#define HDF_LOG_TAG audio_host

AUDIO_LIST_HEAD(cardManager);

/* Get a sound card instance */
struct AudioCard *GetCardInstance(const char *serviceName)
{
    struct AudioCard *audioCard = NULL;

    if (serviceName == NULL) {
        ADM_LOG_ERR("serviceName is NULL.");
        return NULL;
    }

    if (DListIsEmpty(&cardManager)) {
        ADM_LOG_ERR("cardManager is empty.");
        return NULL;
    }

    DLIST_FOR_EACH_ENTRY(audioCard, &cardManager, struct AudioCard, list) {
        if (audioCard->configData.cardServiceName == NULL) {
            ADM_LOG_ERR("cardServiceName is NULL.");
            return NULL;
        }

        if (strcmp(audioCard->configData.cardServiceName, serviceName) == 0) {
            return audioCard;
        }
    }
    return NULL;
}

static int32_t AudioCodecDevInit(struct AudioCard *audioCard)
{
    if (audioCard == NULL) {
        ADM_LOG_ERR("audioCard is NULL.");
        return HDF_ERR_IO;
    }

    ADM_LOG_DEBUG("entry.");
    struct AudioRuntimeDeivces *rtd = NULL;
    struct CodecDevice *codec = NULL;
    rtd = audioCard->rtd;
    if (rtd == NULL) {
        ADM_LOG_ERR("rtd is NULL.");
        return HDF_ERR_IO;
    }
    codec = rtd->codec;
    if (codec != NULL && codec->devData != NULL && codec->devData->Init != NULL) {
        /* codec initialization */
        int32_t ret = codec->devData->Init(audioCard, codec);
        if (ret != HDF_SUCCESS) {
            ADM_LOG_ERR("codec initialization fail ret=%d", ret);
            return HDF_ERR_IO;
        }
    }

    ADM_LOG_INFO("success.");
    return HDF_SUCCESS;
}

static int32_t AudioAccessoryDevInit(struct AudioCard *audioCard)
{
    if (audioCard == NULL) {
        ADM_LOG_ERR("audioCard is NULL.");
        return HDF_ERR_IO;
    }

    ADM_LOG_DEBUG("entry.");
    struct AudioRuntimeDeivces *rtd = NULL;
    struct AccessoryDevice *accessory = NULL;
    rtd = audioCard->rtd;
    if (rtd == NULL) {
        ADM_LOG_ERR("rtd is NULL.");
        return HDF_ERR_IO;
    }
    accessory = rtd->accessory;
    if (accessory != NULL && accessory->devData != NULL && accessory->devData->Init != NULL) {
        /* codec initialization */
        int32_t ret = accessory->devData->Init(audioCard, accessory);
        if (ret != HDF_SUCCESS) {
            ADM_LOG_ERR("accessory initialization fail ret=%d", ret);
            return HDF_ERR_IO;
        }
    }

    ADM_LOG_INFO("success.");
    return HDF_SUCCESS;
}

static int32_t AudioPlatformDevInit(const struct AudioCard *audioCard)
{
    struct AudioRuntimeDeivces *rtd = NULL;
    struct PlatformDevice *platform = NULL;

    if (audioCard == NULL) {
        ADM_LOG_ERR("input param is NULL.");
        return HDF_ERR_IO;
    }
    ADM_LOG_DEBUG("entry.");

    rtd = audioCard->rtd;
    if (rtd == NULL) {
        ADM_LOG_ERR("Platform rtd is NULL.");
        return HDF_ERR_IO;
    }
    platform = rtd->platform;
    if (platform != NULL && platform->devData != NULL && platform->devData->PlatformInit != NULL) {
        /* platform initialization */
        int32_t ret = platform->devData->PlatformInit(audioCard, platform);
        if (ret != HDF_SUCCESS) {
            ADM_LOG_ERR("platform initialization fail ret=%d", ret);
            return HDF_ERR_IO;
        }
    }

    ADM_LOG_INFO("success.");
    return HDF_SUCCESS;
}

static int32_t AudioDspDevInit(const struct AudioCard *audioCard)
{
    struct AudioRuntimeDeivces *rtd = NULL;
    struct DspDevice *dsp = NULL;

    if (audioCard == NULL) {
        ADM_LOG_ERR("audioCard is NULL.");
        return HDF_ERR_IO;
    }
    ADM_LOG_DEBUG("entry.");

    rtd = audioCard->rtd;
    if (rtd == NULL) {
        ADM_LOG_ERR("audioCard rtd object is NULL.");
        return HDF_ERR_IO;
    }

    dsp = rtd->dsp;
    if (dsp != NULL && dsp->devData != NULL && dsp->devData->DspInit != NULL) {
        /* dsp initialization */
        int32_t ret = dsp->devData->DspInit(dsp);
        if (ret != HDF_SUCCESS) {
            ADM_LOG_ERR("dsp initialization fail ret=%d", ret);
            return HDF_ERR_IO;
        }
    }

    ADM_LOG_INFO("success.");
    return HDF_SUCCESS;
}

static int32_t AudioCodecDaiDevInit(const struct AudioCard *audioCard)
{
    struct AudioRuntimeDeivces *rtd = NULL;
    struct DaiDevice *codecDai = NULL;

    if (audioCard == NULL) {
        ADM_LOG_ERR("audioCard is NULL.");
        return HDF_ERR_IO;
    }
    ADM_LOG_DEBUG("entry.");

    rtd = audioCard->rtd;
    if (rtd == NULL) {
        ADM_LOG_ERR("CodecDai rtd is NULL.");
        return HDF_ERR_IO;
    }
    codecDai = rtd->codecDai;
    if (codecDai != NULL && codecDai->devData != NULL && codecDai->devData->DaiInit != NULL) {
        /* codec dai initialization */
        int32_t ret = codecDai->devData->DaiInit(audioCard, codecDai);
        if (ret != HDF_SUCCESS) {
            ADM_LOG_ERR("codec dai initialization fail ret=%d", ret);
            return HDF_ERR_IO;
        }
    }

    ADM_LOG_INFO("success.");
    return HDF_SUCCESS;
}

static int32_t AudioAccessoryDaiDevInit(const struct AudioCard *audioCard)
{
    struct AudioRuntimeDeivces *rtd = NULL;
    struct DaiDevice *accessoryDai = NULL;

    if (audioCard == NULL) {
        ADM_LOG_ERR("audioCard is NULL.");
        return HDF_ERR_IO;
    }
    ADM_LOG_DEBUG("entry.");

    rtd = audioCard->rtd;
    if (rtd == NULL) {
        ADM_LOG_ERR("accessoryDai rtd is NULL.");
        return HDF_ERR_IO;
    }
    accessoryDai = rtd->accessoryDai;
    if (accessoryDai != NULL && accessoryDai->devData != NULL && accessoryDai->devData->DaiInit != NULL) {
        /* codec dai initialization */
        int32_t ret = accessoryDai->devData->DaiInit(audioCard, accessoryDai);
        if (ret != HDF_SUCCESS) {
            ADM_LOG_ERR("accessory dai initialization fail ret=%d", ret);
            return HDF_ERR_IO;
        }
    }

    ADM_LOG_INFO("success.");
    return HDF_SUCCESS;
}

static int32_t AudioCpuDaiDevInit(const struct AudioCard *audioCard)
{
    struct AudioRuntimeDeivces *rtd = NULL;
    struct DaiDevice *cpuDai = NULL;

    if (audioCard == NULL) {
        ADM_LOG_ERR("audioCard is NULL.");
        return HDF_ERR_IO;
    }
    ADM_LOG_DEBUG("entry.");

    rtd = audioCard->rtd;
    if (rtd == NULL) {
        ADM_LOG_ERR("cpuDai rtd is NULL.");
        return HDF_ERR_IO;
    }
    cpuDai = rtd->cpuDai;
    if (cpuDai != NULL && cpuDai->devData != NULL && cpuDai->devData->DaiInit != NULL) {
        /* cpu dai initialization */
        int32_t ret = cpuDai->devData->DaiInit(audioCard, cpuDai);
        if (ret != HDF_SUCCESS) {
            ADM_LOG_ERR("cpu dai initialization fail ret=%d", ret);
            return HDF_ERR_IO;
        }
    }

    ADM_LOG_INFO("success.");
    return HDF_SUCCESS;
}

static int32_t AudioDspDaiDevInit(const struct AudioCard *audioCard)
{
    struct AudioRuntimeDeivces *rtd = NULL;
    struct DaiDevice *dspDai = NULL;
    int ret;

    if (audioCard == NULL) {
        ADM_LOG_ERR("audioCard is NULL.");
        return HDF_ERR_IO;
    }
    ADM_LOG_DEBUG("dsp init entry.");

    rtd = audioCard->rtd;
    if (rtd == NULL) {
        ADM_LOG_ERR("dspDai rtd is NULL.");
        return HDF_ERR_IO;
    }

    dspDai = rtd->dspDai;
    if (dspDai != NULL && dspDai->devData != NULL && dspDai->devData->DaiInit != NULL) {
        ret = dspDai->devData->DaiInit(audioCard, dspDai);
        if (ret != HDF_SUCCESS) {
            ADM_LOG_ERR("dsp dai initialization fail ret=%d", ret);
            return HDF_ERR_IO;
        }
    }

    ADM_LOG_INFO("success.");
    return HDF_SUCCESS;
}

static int32_t AudioInitDaiLink(struct AudioCard *audioCard)
{
    if (audioCard == NULL) {
        ADM_LOG_ERR("audioCard is NULL.");
        return HDF_ERR_IO;
    }
    ADM_LOG_DEBUG("entry.");

    if (AudioPlatformDevInit(audioCard) || AudioCpuDaiDevInit(audioCard)) {
        ADM_LOG_ERR("Platform and CpuDai init fail.");
        return HDF_ERR_IO;
    }

    if ((AudioCodecDevInit(audioCard) || AudioCodecDaiDevInit(audioCard))) {
        ADM_LOG_ERR("codec init fail.");
        return HDF_ERR_IO;
    }

    if (AudioAccessoryDevInit(audioCard) || AudioAccessoryDaiDevInit(audioCard)) {
        ADM_LOG_ERR("Accessory init fail.");
        return HDF_ERR_IO;
    }

    if (AudioDspDevInit(audioCard) || AudioDspDaiDevInit(audioCard)) {
        ADM_LOG_ERR("Dsp Dai init fail.");
        return HDF_ERR_IO;
    }

    ADM_LOG_DEBUG("success.");
    return HDF_SUCCESS;
}

struct AudioHost *AudioHostCreateAndBind(struct HdfDeviceObject *device)
{
    struct AudioHost *audioHost = NULL;

    if (device == NULL) {
        ADM_LOG_ERR("device is NULL!");
        return NULL;
    }

    audioHost = (struct AudioHost *)OsalMemCalloc(sizeof(*audioHost));
    if (audioHost == NULL) {
        ADM_LOG_ERR("Malloc audio host fail!");
        return NULL;
    }
    audioHost->device = device;
    device->service = &audioHost->service;
    return audioHost;
}

/* HdfDriverEntry implementations */
static int32_t AudioDriverBind(struct HdfDeviceObject *device)
{
    struct AudioHost *audioHost = NULL;

    ADM_LOG_DEBUG("entry.");
    if (device == NULL) {
        ADM_LOG_ERR("device is NULL.");
        return HDF_ERR_INVALID_OBJECT;
    }

    audioHost = AudioHostCreateAndBind(device);
    if (audioHost == NULL) {
        ADM_LOG_ERR("audioHost create failed!");
        return HDF_FAILURE;
    }

    ADM_LOG_INFO("success.");
    return HDF_SUCCESS;
}

static int32_t AudioDriverInit(struct HdfDeviceObject *device)
{
    struct AudioCard *audioCard = NULL;
    struct AudioHost *audioHost = NULL;
    int32_t ret;

    ADM_LOG_DEBUG("entry.");
    if (device == NULL) {
        ADM_LOG_ERR("device is NULL.");
        return HDF_ERR_INVALID_OBJECT;
    }
    audioHost = AudioHostFromDevice(device);
    if (audioHost == NULL) {
        ADM_LOG_ERR("audioHost is NULL.");
        return HDF_FAILURE;
    }

    audioCard = (struct AudioCard *)OsalMemCalloc(sizeof(*audioCard));
    if (audioCard == NULL) {
        ADM_LOG_ERR("Malloc audioCard fail!");
        return HDF_FAILURE;
    }
    audioHost->priv = audioCard;
    /* Get node information through HCS file */
    ret = AudioFillConfigData(device, &(audioCard->configData));
    if (ret != HDF_SUCCESS) {
        ADM_LOG_ERR("AudioFillConfigData fail ret=%d", ret);
        return HDF_ERR_IO;
    }

    /* Bind specific codec、platform and dai device */
    AudioBindDaiLink(audioCard, &(audioCard->configData));
    if (!audioCard->rtd->complete) {
        ADM_LOG_ERR("AudioBindDaiLink fail!");
        return HDF_ERR_IO;
    }

    /* Initialize controls list */
    DListHeadInit(&audioCard->controls);
    DListHeadInit(&audioCard->components);
    DListHeadInit(&audioCard->paths);
    DListHeadInit(&audioCard->sapmDirty);
    /* Initialize hardware device */
    ret = AudioInitDaiLink(audioCard);
    if (ret != HDF_SUCCESS) {
        ADM_LOG_ERR("AudioInitDaiLink fail ret=%d", ret);
        return HDF_ERR_IO;
    }

    /* sound card added to list */
    DListInsertHead(&audioCard->list, &cardManager);

    ADM_LOG_INFO("success.");
    return HDF_SUCCESS;
}

static void AudioDriverRelease(struct HdfDeviceObject *device)
{
    struct AudioHost *audioHost = NULL;
    struct AudioCard *audioCard = NULL;

    ADM_LOG_DEBUG("entry.");
    if (device == NULL) {
        ADM_LOG_ERR("device is NULL.");
        return;
    }
    audioHost = AudioHostFromDevice(device);
    if (audioHost == NULL) {
        ADM_LOG_ERR("audioHost is NULL.");
        return;
    }

    if (audioHost->priv != NULL) {
        audioCard = (struct AudioCard *)audioHost->priv;
        if (audioCard->rtd != NULL) {
            OsalMemFree(audioCard->rtd);
        }
        OsalMemFree(audioHost->priv);
    }
    OsalMemFree(audioHost);

    ADM_LOG_INFO("success.");
}

/* HdfDriverEntry definitions */
struct HdfDriverEntry g_audioDriverEntry = {
    .moduleVersion = 1,
    .moduleName = "HDF_AUDIO",
    .Bind = AudioDriverBind,
    .Init = AudioDriverInit,
    .Release = AudioDriverRelease,
};
HDF_INIT(g_audioDriverEntry);
