/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
 *
 * HDF is dual licensed: you can use it either under the terms of
 * the GPL, or the BSD license, at your option.
 * See the LICENSE file in the root of this repository for complete details.
 */

#ifndef HI3516_PLATFORM_OPS_H
#define HI3516_PLATFORM_OPS_H

#include "audio_core.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* __cplusplus */

int32_t AudioPlatformDeviceInit(const struct AudioCard *card, const struct PlatformDevice *platform);
int32_t PlatformHwParams(const struct AudioCard *card, const struct AudioPcmHwParams *param);
int32_t PlatformRenderPrepare(const struct AudioCard *card);
int32_t PlatformCapturePrepare(const struct AudioCard *card);
int32_t PlatformCapturePrepare(const struct AudioCard *card);
int32_t PlatformWrite(const struct AudioCard *card, struct AudioTxData *txData);
int32_t PlatformRead(const struct AudioCard *card, struct AudioRxData *rxData);
int32_t PlatformMmapWrite(const struct AudioCard *card, struct AudioTxMmapData *txMmapData);
int32_t PlatformMmapRead(const struct AudioCard *card, struct AudioRxMmapData *rxMmapData);
int32_t PlatformRenderStart(struct AudioCard *card);
int32_t PlatformCaptureStart(struct AudioCard *card);
int32_t PlatformRenderStop(struct AudioCard *card);
int32_t PlatformCaptureStop(struct AudioCard *card);
int32_t PlatformCaptureStop(struct AudioCard *card);
int32_t PlatformRenderPause(struct AudioCard *card);
int32_t PlatformRenderResume(struct AudioCard *card);
int32_t PlatformCapturePause(struct AudioCard *card);
int32_t PlatformCaptureResume(struct AudioCard *card);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */

#endif /* HI3516_CODEC_OPS_H */
