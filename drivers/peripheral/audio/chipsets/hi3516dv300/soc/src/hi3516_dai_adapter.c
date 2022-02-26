/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
 *
 * HDF is dual licensed: you can use it either under the terms of
 * the GPL, or the BSD license, at your option.
 * See the LICENSE file in the root of this repository for complete details.
 */

#include "audio_core.h"
#include "hi3516_aiao_impl.h"
#include "hi3516_codec_impl.h"
#include "osal_io.h"

#define HDF_LOG_TAG hi3516_dai_adapter

/* HdfDriverEntry implementations */
static int32_t DaiDriverBind(struct HdfDeviceObject *device)
{
    struct DaiHost *daiHost = NULL;
    AUDIO_DRIVER_LOG_DEBUG("entry!");

    if (device == NULL) {
        AUDIO_DRIVER_LOG_ERR("input para is NULL.");
        return HDF_FAILURE;
    }

    daiHost = (struct DaiHost *)OsalMemCalloc(sizeof(*daiHost));
    if (daiHost == NULL) {
        AUDIO_DRIVER_LOG_ERR("malloc host fail!");
        return HDF_FAILURE;
    }

    daiHost->device = device;
    daiHost->daiInitFlag = false;
    device->service = &daiHost->service;

    AUDIO_DRIVER_LOG_DEBUG("success!");
    return HDF_SUCCESS;
}

int32_t DaiDeviceInit(const struct AudioCard *audioCard, const struct DaiDevice *dai)
{
    int ret;
    struct DaiHost *daiHost = NULL;
    unsigned int chnId = 0;

    if (dai == NULL || dai->device == NULL || dai->devDaiName == NULL) {
        AUDIO_DRIVER_LOG_ERR("dai is nullptr.");
        return HDF_FAILURE;
    }
    AUDIO_DRIVER_LOG_DEBUG("cpu dai device name: %s\n", dai->devDaiName);
    (void)audioCard;

    daiHost = (struct DaiHost *)dai->device->service;
    if (daiHost == NULL) {
        AUDIO_DRIVER_LOG_ERR("dai host is NULL.");
        return HDF_FAILURE;
    }

    if (daiHost->daiInitFlag == true) {
        AUDIO_DRIVER_LOG_DEBUG("dai init complete!");
        return HDF_SUCCESS;
    }

    ret = AiaoHalSysInit();
    if (ret != HDF_SUCCESS) {
        AUDIO_DRIVER_LOG_ERR("AiaoHalSysInit: fail.");
        return HDF_FAILURE;
    }

    ret = I2sCrgCfgInit(chnId);
    if (ret != HDF_SUCCESS) {
        AUDIO_DRIVER_LOG_ERR("AiaoHalSysInit: fail.");
        return HDF_FAILURE;
    }

    daiHost->daiInitFlag = true;

    return HDF_SUCCESS;
}

static int32_t CheckSampleRate(unsigned int sampleRates)
{
    bool checkSampleRate = (sampleRates == AUDIO_SAMPLE_RATE_8000 ||
                           sampleRates == AUDIO_SAMPLE_RATE_12000 ||
                           sampleRates == AUDIO_SAMPLE_RATE_11025 ||
                           sampleRates == AUDIO_SAMPLE_RATE_16000 ||
                           sampleRates == AUDIO_SAMPLE_RATE_22050 ||
                           sampleRates == AUDIO_SAMPLE_RATE_24000 ||
                           sampleRates == AUDIO_SAMPLE_RATE_32000 ||
                           sampleRates == AUDIO_SAMPLE_RATE_44100 ||
                           sampleRates == AUDIO_SAMPLE_RATE_48000 ||
                           sampleRates == AUDIO_SAMPLE_RATE_64000 ||
                           sampleRates == AUDIO_SAMPLE_RATE_96000);
    if (checkSampleRate) {
        return HDF_SUCCESS;
    } else {
        AUDIO_DRIVER_LOG_ERR("FramatToSampleRate fail: Invalid sampling rate: %d.", sampleRates);
        return HDF_ERR_NOT_SUPPORT;
    }
}

static int32_t FramatToBitWidth(enum AudioFormat format, unsigned int *bitWidth)
{
    switch (format) {
        case AUDIO_FORMAT_PCM_16_BIT:
            *bitWidth = BIT_WIDTH16;
            break;

        case AUDIO_FORMAT_PCM_24_BIT:
            *bitWidth = BIT_WIDTH24;
            break;

        default:
            AUDIO_DRIVER_LOG_ERR("format: %d is not define.", format);
            return HI_FAILURE;
            break;
    }

    return HDF_SUCCESS;
}

int32_t AiSetClkAttr(struct PlatformHost *platformHost, const struct AudioPcmHwParams *param)
{
    int ret;
    unsigned int bitWidth;

    if (platformHost == NULL) {
        AUDIO_DRIVER_LOG_ERR("platform host is NULL.");
        return HDF_FAILURE;
    }

    if (param == NULL) {
        AUDIO_DRIVER_LOG_ERR("param is NULL.");
        return HDF_FAILURE;
    }

    ret = FramatToBitWidth(param->format, &bitWidth);
    if (ret != HDF_SUCCESS) {
        AUDIO_DRIVER_LOG_ERR("FramatToBitWidth: fail.");
        return HDF_FAILURE;
    }
    platformHost->pcmInfo.bitWidth = bitWidth;

    ret = AipSetSysCtlReg(platformHost->captureBufInfo.chnId, param->channels, bitWidth, param->rate);
    if (ret != HDF_SUCCESS) {
        AUDIO_DRIVER_LOG_ERR("AiSetClkAttr: fail.");
        return HDF_FAILURE;
    }

    ret = AipSetAttr(platformHost->captureBufInfo.chnId, param->channels, bitWidth);
    if (ret != HDF_SUCCESS) {
        AUDIO_DRIVER_LOG_ERR("AiSetClkAttr: fail.");
        return HDF_FAILURE;
    }

    return HDF_SUCCESS;
}

int32_t AoSetClkAttr(struct PlatformHost *platformHost, const struct AudioPcmHwParams *param)
{
    int ret;
    unsigned int bitWidth;

    if (platformHost == NULL) {
        AUDIO_DRIVER_LOG_ERR("platform host is nullptr.");
        return HDF_FAILURE;
    }

    if (param == NULL) {
        AUDIO_DRIVER_LOG_ERR("param is nullptr.");
        return HDF_FAILURE;
    }

    ret = FramatToBitWidth(param->format, &bitWidth);
    if (ret != HDF_SUCCESS) {
        AUDIO_DRIVER_LOG_ERR("FramatToBitWidth: fail.");
        return HDF_FAILURE;
    }
    platformHost->pcmInfo.bitWidth = bitWidth;

    ret = AopSetSysCtlReg(platformHost->renderBufInfo.chnId, param->channels, bitWidth, param->rate);
    if (ret != HDF_SUCCESS) {
        AUDIO_DRIVER_LOG_ERR("PlatformAopSetSysCtl: fail.");
        return HDF_FAILURE;
    }

    ret = AopSetAttr(platformHost->renderBufInfo.chnId, param->channels, bitWidth);
    if (ret != HDF_SUCCESS) {
        AUDIO_DRIVER_LOG_ERR("PlatformAopSetAttrReg: fail.");
        return HDF_FAILURE;
    }

    AUDIO_DRIVER_LOG_DEBUG("bitWidth: %d.", bitWidth);

    return HDF_SUCCESS;
}

int32_t DaiHwParams(const struct AudioCard *card, const struct AudioPcmHwParams *param, const struct DaiDevice *device)
{
    struct PlatformHost *platformHost = NULL;
    int ret;
    (void)device;

    AUDIO_DRIVER_LOG_DEBUG("entry.");

    if (card == NULL || card->rtd == NULL || card->rtd->platform == NULL ||
        param == NULL || param->cardServiceName == NULL) {
        AUDIO_DRIVER_LOG_ERR("input para is nullptr.");
        return HDF_FAILURE;
    }

    ret = CheckSampleRate(param->rate);
    if (ret != HDF_SUCCESS) {
        AUDIO_DRIVER_LOG_ERR("CheckSampleRate:  fail.");
        return HDF_ERR_NOT_SUPPORT;
    }

    platformHost = PlatformHostFromDevice(card->rtd->platform->device);
    if (platformHost == NULL) {
        AUDIO_DRIVER_LOG_ERR("platformHost is nullptr.");
        return HDF_FAILURE;
    }

    if (param->streamType == AUDIO_RENDER_STREAM) {
        ret = AoSetClkAttr(platformHost, param);
        if (ret != HDF_SUCCESS) {
            AUDIO_DRIVER_LOG_ERR("AoSetClkAttr:  fail.");
            return HDF_FAILURE;
        }
    } else if (param->streamType == AUDIO_CAPTURE_STREAM) {
        ret = AiSetClkAttr(platformHost, param);
        if (ret != HDF_SUCCESS) {
            AUDIO_DRIVER_LOG_ERR("AiSetClkAttr:  fail.");
            return HDF_FAILURE;
        }
    } else {
        AUDIO_DRIVER_LOG_ERR("param streamType is invalid.");
        return HDF_FAILURE;
    }

    AUDIO_DRIVER_LOG_DEBUG("channels = %d, rate = %d, periodSize = %d, \
        periodCount = %d, format = %d, cardServiceName = %s \n",
        param->channels, param->rate, param->periodSize,
        param->periodCount, (uint32_t)param->format, param->cardServiceName);

    AUDIO_DRIVER_LOG_DEBUG("success.");
    return HDF_SUCCESS;
}

int32_t DaiTrigger(const struct AudioCard *card, int cmd, const struct DaiDevice *device)
{
    (void)card;
    (void)device;
    (void)cmd;

    return HDF_SUCCESS;
}

struct AudioDaiOps g_daiDeviceOps = {
    .HwParams = DaiHwParams,
    .Trigger = DaiTrigger,
};

struct DaiData g_daiData = {
    .DaiInit = DaiDeviceInit,
    .ops = &g_daiDeviceOps,
};

static int32_t DaiGetServiceName(const struct HdfDeviceObject *device)
{
    const struct DeviceResourceNode *node = NULL;
    struct DeviceResourceIface *drsOps = NULL;
    int32_t ret;

    if (device == NULL) {
        AUDIO_DRIVER_LOG_ERR("input para is nullptr.");
        return HDF_FAILURE;
    }

    node = device->property;
    if (node == NULL) {
        AUDIO_DRIVER_LOG_ERR("drs node is nullptr.");
        return HDF_FAILURE;
    }
    drsOps = DeviceResourceGetIfaceInstance(HDF_CONFIG_SOURCE);
    if (drsOps == NULL || drsOps->GetString == NULL) {
        AUDIO_DRIVER_LOG_ERR("invalid drs ops fail!");
        return HDF_FAILURE;
    }

    ret = drsOps->GetString(node, "serviceName", &g_daiData.drvDaiName, 0);
    if (ret != HDF_SUCCESS) {
        AUDIO_DRIVER_LOG_ERR("read serviceName fail!");
        return ret;
    }

    return HDF_SUCCESS;
}

static int32_t DaiDriverInit(struct HdfDeviceObject *device)
{
    int32_t ret;
    AUDIO_DRIVER_LOG_DEBUG("entry.\n");

    if (device == NULL) {
        AUDIO_DRIVER_LOG_ERR("device is nullptr.");
        return HDF_ERR_INVALID_OBJECT;
    }

    ret = DaiGetServiceName(device);
    if (ret !=  HDF_SUCCESS) {
        AUDIO_DRIVER_LOG_ERR("get service name fail.");
        return ret;
    }

    ret = AudioSocDeviceRegister(device, (void *)&g_daiData, AUDIO_DAI_DEVICE);
    if (ret !=  HDF_SUCCESS) {
        AUDIO_DRIVER_LOG_ERR("register dai fail.");
        return ret;
    }

    AUDIO_DRIVER_LOG_DEBUG("success.\n");
    return HDF_SUCCESS;
}

/* HdfDriverEntry definitions */
struct HdfDriverEntry g_daiDriverEntry = {
    .moduleVersion = 1,
    .moduleName = "DAI_HI3516",
    .Bind = DaiDriverBind,
    .Init = DaiDriverInit,
    .Release = NULL,
};
HDF_INIT(g_daiDriverEntry);
