/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
 *
 * HDF is dual licensed: you can use it either under the terms of
 * the GPL, or the BSD license, at your option.
 * See the LICENSE file in the root of this repository for complete details.
 */

#ifndef AUDIO_ACCESSORY_BASE_H
#define AUDIO_ACCESSORY_BASE_H

#include "audio_host.h"
#include "audio_control.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* __cplusplus */

enum I2sFrequency {
    I2S_SAMPLE_FREQUENCY_8000  = 8000,    /* 8kHz sample_rate */
    I2S_SAMPLE_FREQUENCY_11025 = 11025,   /* 11.025kHz sample_rate */
    I2S_SAMPLE_FREQUENCY_12000 = 12000,   /* 12kHz sample_rate */
    I2S_SAMPLE_FREQUENCY_16000 = 16000,   /* 16kHz sample_rate */
    I2S_SAMPLE_FREQUENCY_22050 = 22050,   /* 22.050kHz sample_rate */
    I2S_SAMPLE_FREQUENCY_24000 = 24000,   /* 24kHz sample_rate */
    I2S_SAMPLE_FREQUENCY_32000 = 32000,   /* 32kHz sample_rate */
    I2S_SAMPLE_FREQUENCY_44100 = 44100,   /* 44.1kHz sample_rate */
    I2S_SAMPLE_FREQUENCY_48000 = 48000,   /* 48kHz sample_rate */
    I2S_SAMPLE_FREQUENCY_64000 = 64000,   /* 64kHz sample_rate */
    I2S_SAMPLE_FREQUENCY_88200 = 88200,   /* 88.2kHz sample_rate */
    I2S_SAMPLE_FREQUENCY_96000 = 96000    /* 96kHz sample_rate */
};

enum I2sFrequencyRegVal {
    I2S_SAMPLE_FREQUENCY_REG_VAL_8000  = 0x0,   /* 8kHz sample_rate */
    I2S_SAMPLE_FREQUENCY_REG_VAL_11025 = 0x1,   /* 11.025kHz sample_rate */
    I2S_SAMPLE_FREQUENCY_REG_VAL_12000 = 0x2,   /* 12kHz sample_rate */
    I2S_SAMPLE_FREQUENCY_REG_VAL_16000 = 0x3,   /* 16kHz sample_rate */
    I2S_SAMPLE_FREQUENCY_REG_VAL_22050 = 0x4,   /* 22.050kHz sample_rate */
    I2S_SAMPLE_FREQUENCY_REG_VAL_24000 = 0x5,   /* 24kHz sample_rate */
    I2S_SAMPLE_FREQUENCY_REG_VAL_32000 = 0x6,   /* 32kHz sample_rate */
    I2S_SAMPLE_FREQUENCY_REG_VAL_44100 = 0x7,   /* 44.1kHz sample_rate */
    I2S_SAMPLE_FREQUENCY_REG_VAL_48000 = 0x8,   /* 48kHz sample_rate */
    I2S_SAMPLE_FREQUENCY_REG_VAL_64000 = 0x9,   /* 64kHz sample_rate */
    I2S_SAMPLE_FREQUENCY_REG_VAL_88200 = 0xA,   /* 88.2kHz sample_rate */
    I2S_SAMPLE_FREQUENCY_REG_VAL_96000 = 0xB    /* 96kHz sample_rate */
};

enum I2sFormatRegVal {
    I2S_SAMPLE_FORMAT_REG_VAL_MSB_24    = 0x2,    /*  MSB-justified data up to 24 bits */
    I2S_SAMPLE_FORMAT_REG_VAL_24        = 0x3,    /*  I2S data up to 24 bits */
    I2S_SAMPLE_FORMAT_REG_VAL_LSB_16    = 0x4,    /*  LSB-justified 16-bit data */
    I2S_SAMPLE_FORMAT_REG_VAL_LSB_18    = 0x5,    /*  LSB-justified 18-bit data */
    I2S_SAMPLE_FORMAT_REG_VAL_LSB_20    = 0x6,    /*  LSB-justified 20-bit data */
    I2S_SAMPLE_FORMAT_REG_VAL_LSB_24    = 0x7,    /*  LSB-justified 24-bit data */
};

int32_t FormatToBitWidth(enum AudioFormat format, uint16_t *bitWidth);
int32_t RateToFrequency(uint32_t rate, uint16_t *freq);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */

#endif
