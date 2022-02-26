/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
 *
 * HDF is dual licensed: you can use it either under the terms of
 * the GPL, or the BSD license, at your option.
 * See the LICENSE file in the root of this repository for complete details.
 */

#ifndef HI3516_COMMON_H
#define HI3516_COMMON_H

enum AuidoBitWidth {
    BIT_WIDTH8  =  8,      /* 8 bit witdth */
    BIT_WIDTH16 =  16,     /* 16 bit witdth */
    BIT_WIDTH18 =  18,     /* 18 bit witdth */
    BIT_WIDTH20 =  20,     /* 20 bit witdth */
    BIT_WIDTH24 =  24,     /* 24 bit witdth */
    BIT_WIDTH32 =  32,     /* 32 bit witdth */
};

typedef enum {
    AUDIO_SAMPLE_RATE_8000   = 8000,    /* 8kHz sample_rate */
    AUDIO_SAMPLE_RATE_12000  = 12000,   /* 12kHz sample_rate */
    AUDIO_SAMPLE_RATE_11025  = 11025,   /* 11.025kHz sample_rate */
    AUDIO_SAMPLE_RATE_16000  = 16000,   /* 16kHz sample_rate */
    AUDIO_SAMPLE_RATE_22050  = 22050,   /* 22.050kHz sample_rate */
    AUDIO_SAMPLE_RATE_24000  = 24000,   /* 24kHz sample_rate */
    AUDIO_SAMPLE_RATE_32000  = 32000,   /* 32kHz sample_rate */
    AUDIO_SAMPLE_RATE_44100  = 44100,   /* 44.1kHz sample_rate */
    AUDIO_SAMPLE_RATE_48000  = 48000,   /* 48kHz sample_rate */
    AUDIO_SAMPLE_RATE_64000  = 64000,   /* 64kHz sample_rate */
    AUDIO_SAMPLE_RATE_96000  = 96000,   /* 96kHz sample_rate */
    AUDIO_SAMPLE_RATE_BUTT,
} AudioSampleRate;

typedef enum {
    AUDIO_BIT_WIDTH_8   = 0,   /* 8bit width */
    AUDIO_BIT_WIDTH_16  = 1,   /* 16bit width */
    AUDIO_BIT_WIDTH_24  = 2,   /* 24bit width */
    AUDIO_BIT_WIDTH_BUTT,
} AudioBitWidth;

#endif