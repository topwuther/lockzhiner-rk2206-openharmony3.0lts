/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
 *
 * HDF is dual licensed: you can use it either under the terms of
 * the GPL, or the BSD license, at your option.
 * See the LICENSE file in the root of this repository for complete details.
 */

#ifndef AUDIO_PLATFORM_BASE_H
#define AUDIO_PLATFORM_BASE_H

#include "audio_platform_if.h"
#include "audio_host.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* __cplusplus */

enum DataBitWidth {
    DATA_BIT_WIDTH8  =  8,      /* 8 bit witdth */
    DATA_BIT_WIDTH16 =  16,     /* 16 bit witdth */
    DATA_BIT_WIDTH18 =  18,     /* 18 bit witdth */
    DATA_BIT_WIDTH20 =  20,     /* 20 bit witdth */
    DATA_BIT_WIDTH24 =  24,     /* 24 bit witdth */
    DATA_BIT_WIDTH32 =  32,     /* 32 bit witdth */
};

struct PlatformData *PlatformDataFromDevice(const struct AudioCard *card);
int32_t PlatformCreatePlatformHost(const struct AudioCard *card, struct PlatformHost **platformHost);
int32_t AudioDataBigEndianChange(char *srcData, uint32_t audioLen, enum DataBitWidth bitWidth);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */

#endif /* CODEC_CORE_H */
