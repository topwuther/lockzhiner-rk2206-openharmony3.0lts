/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
 *
 * HDF is dual licensed: you can use it either under the terms of
 * the GPL, or the BSD license, at your option.
 * See the LICENSE file in the root of this repository for complete details.
 */

#include "audio_accessory_base.h"
#include "audio_device_log.h"

#define HDF_LOG_TAG audio_accessory_base

int32_t FormatToBitWidth(enum AudioFormat format, uint16_t *bitWidth)
{
    // current set default format(standard) for 16/24 bit
    switch (format) {
        case AUDIO_FORMAT_PCM_16_BIT:
            *bitWidth = I2S_SAMPLE_FORMAT_REG_VAL_24;
            break;
        case AUDIO_FORMAT_PCM_24_BIT:
            *bitWidth = I2S_SAMPLE_FORMAT_REG_VAL_24;
            break;
        default:
            AUDIO_DEVICE_LOG_ERR("format: %d is not support.", format);
            return HDF_ERR_NOT_SUPPORT;
    }
    return HDF_SUCCESS;
}

int32_t RateToFrequency(uint32_t rate, uint16_t *freq)
{
    switch (rate) {
        case I2S_SAMPLE_FREQUENCY_8000:
            *freq = I2S_SAMPLE_FREQUENCY_REG_VAL_8000;
            break;
        case I2S_SAMPLE_FREQUENCY_11025:
            *freq = I2S_SAMPLE_FREQUENCY_REG_VAL_11025;
            break;
        case I2S_SAMPLE_FREQUENCY_12000:
            *freq = I2S_SAMPLE_FREQUENCY_REG_VAL_12000;
            break;
        case I2S_SAMPLE_FREQUENCY_16000:
            *freq = I2S_SAMPLE_FREQUENCY_REG_VAL_16000;
            break;
        case I2S_SAMPLE_FREQUENCY_22050:
            *freq = I2S_SAMPLE_FREQUENCY_REG_VAL_22050;
            break;
        case I2S_SAMPLE_FREQUENCY_24000:
            *freq = I2S_SAMPLE_FREQUENCY_REG_VAL_24000;
            break;
        case I2S_SAMPLE_FREQUENCY_32000:
            *freq = I2S_SAMPLE_FREQUENCY_REG_VAL_32000;
            break;
        case I2S_SAMPLE_FREQUENCY_44100:
            *freq = I2S_SAMPLE_FREQUENCY_REG_VAL_44100;
            break;
        case I2S_SAMPLE_FREQUENCY_48000:
            *freq = I2S_SAMPLE_FREQUENCY_REG_VAL_48000;
            break;
        case I2S_SAMPLE_FREQUENCY_64000:
            *freq = I2S_SAMPLE_FREQUENCY_REG_VAL_64000;
            break;
        case I2S_SAMPLE_FREQUENCY_88200:
            *freq = I2S_SAMPLE_FREQUENCY_REG_VAL_88200;
            break;
        case I2S_SAMPLE_FREQUENCY_96000:
            *freq = I2S_SAMPLE_FREQUENCY_REG_VAL_96000;
            break;
        default:
            AUDIO_DEVICE_LOG_ERR("rate: %d is not support.", rate);
            return HDF_ERR_NOT_SUPPORT;
    }
    return HDF_SUCCESS;
}
