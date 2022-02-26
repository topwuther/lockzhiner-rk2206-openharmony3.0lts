/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
 *
 * HDF is dual licensed: you can use it either under the terms of
 * the GPL, or the BSD license, at your option.
 * See the LICENSE file in the root of this repository for complete details.
 */

#ifndef AUDIO_DSP_IF_H
#define AUDIO_DSP_IF_H
#include "audio_host.h"
#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* __cplusplus */

struct DspDevice {
    const char *devDspName;
    struct DspData *devData;
    struct HdfDeviceObject *device;
    struct DListHead list;
};

struct AudioDspOps {
    int32_t (*Startup)(const struct AudioCard *, const struct DspDevice *);
    int32_t (*HwParams)(const struct AudioCard *, const struct AudioPcmHwParams *, const struct DspDevice *);
    int32_t (*Trigger)(struct AudioCard *, int, struct DspDevice *);
};

/* Dsp host is defined in dsp driver */
struct DspHost {
    struct IDeviceIoService service; // dsp service
    struct HdfDeviceObject *device;  // dsp device
    void *priv;                      // dsp private data
};

struct DspData {
    const char *drvDspName;
    /* dsp driver callbacks */
    int32_t (*DspInit)(const struct DspDevice *device);
    int32_t (*Read)(struct DspDevice *, uint8_t *, uint32_t);
    int32_t (*Write)(struct DspDevice *, uint8_t *, uint32_t);
    int32_t (*decode)(const struct AudioCard *, const uint8_t *, const struct DspDevice *);
    int32_t (*encode)(const struct AudioCard *, const uint8_t *, const struct DspDevice *);
    int32_t (*Equalizer)(const struct AudioCard *, const uint8_t *, const struct DspDevice *);
};

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */
#endif
