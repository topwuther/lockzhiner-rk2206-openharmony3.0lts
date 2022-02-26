/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
 *
 * HDF is dual licensed: you can use it either under the terms of
 * the GPL, or the BSD license, at your option.
 * See the LICENSE file in the root of this repository for complete details.
 */

#include "audio_platform_base.h"
#include "audio_core.h"

#define HDF_LOG_TAG audio_platform_base

struct PlatformData *PlatformDataFromDevice(const struct AudioCard *card)
{
    if (card == NULL || card->rtd == NULL || card->rtd->platform == NULL) {
        AUDIO_DRIVER_LOG_ERR("param is null.");
        return NULL;
    }
    return card->rtd->platform->devData;
}

int32_t PlatformCreatePlatformHost(const struct AudioCard *card, struct PlatformHost **platformHost)
{
    if (platformHost == NULL) {
        AUDIO_DRIVER_LOG_ERR("input param platformHost is invalid.");
        return HDF_ERR_INVALID_PARAM;
    }
    if (card == NULL || card->rtd == NULL || card->rtd->platform == NULL) {
        AUDIO_DRIVER_LOG_ERR("input para is NULL.");
        return HDF_FAILURE;
    }

    *platformHost = PlatformHostFromDevice(card->rtd->platform->device);
    if (*platformHost == NULL) {
        AUDIO_DRIVER_LOG_ERR("PlatformHostFromDevice faile.");
        return HDF_FAILURE;
    }
    return HDF_SUCCESS;
}

int32_t AudioDataBigEndianChange(char *srcData, uint32_t audioLen, enum DataBitWidth bitWidth)
{
    uint64_t i;
    uint16_t framesize;
    char temp;
    if (srcData == NULL) {
        AUDIO_DRIVER_LOG_ERR("srcData is NULL.");
        return HDF_FAILURE;
    }

    switch (bitWidth) {
        case DATA_BIT_WIDTH8:
            framesize = 1; /* 1 byte */
            break;
        case DATA_BIT_WIDTH16:
            framesize = 2; /* 2 bytes */
            break;
        case DATA_BIT_WIDTH24:
            framesize = 3; /* 3 bytes */
            break;
        default:
            framesize = 2; /* default 2 bytes */
            break;
    }

    for (i = 0; i < audioLen; i += framesize) {
        temp = srcData[i];
        srcData[i] = srcData[i + framesize - 1];
        srcData[i + framesize - 1] = temp;
    }
    AUDIO_DRIVER_LOG_DEBUG("audioLen = %d\n", audioLen);
    return HDF_SUCCESS;
}
