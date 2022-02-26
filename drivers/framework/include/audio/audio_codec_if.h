/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
 *
 * HDF is dual licensed: you can use it either under the terms of
 * the GPL, or the BSD license, at your option.
 * See the LICENSE file in the root of this repository for complete details.
 */

#ifndef AUDIO_CODEC_IF_H
#define AUDIO_CODEC_IF_H

#include "audio_host.h"
#include "audio_control.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* __cplusplus */

#define AUDIODRV_CTL_ELEM_IFACE_DAC 0 /* virtual dac device */
#define AUDIODRV_CTL_ELEM_IFACE_ADC 1 /* virtual adc device */
#define AUDIODRV_CTL_ELEM_IFACE_GAIN 2 /* virtual adc device */
#define AUDIODRV_CTL_ELEM_IFACE_MIXER 3 /* virtual mixer device */
#define AUDIODRV_CTL_ELEM_IFACE_ACODEC 4 /* Acodec device */
#define AUDIODRV_CTL_ELEM_IFACE_PGA 5 /* PGA device */
#define AUDIODRV_CTL_ELEM_IFACE_AIAO 6 /* AIAO device */

struct VirtualAddress {
    unsigned long acodecVir;
    unsigned long aiaoVir;
};

struct CodecDevice {
    const char *devCodecName;
    struct CodecData *devData;
    struct HdfDeviceObject *device;
    struct DListHead list;
    struct OsalMutex mutex;
};

/* codec related definitions */
struct CodecData {
    const char *drvCodecName;
    /* Codec driver callbacks */
    int32_t (*Init)(struct AudioCard *, struct CodecDevice *);
    int32_t (*Read)(const struct CodecDevice *, uint32_t, uint32_t *);
    int32_t (*Write)(const struct CodecDevice *, uint32_t, uint32_t);
    int32_t (*AiaoRead)(const struct CodecDevice *, uint32_t, uint32_t *);
    int32_t (*AiaoWrite)(const struct CodecDevice *, uint32_t, uint32_t);
    const struct AudioKcontrol *controls;
    int numControls;
    const struct AudioSapmComponent *sapmComponents;
    int numSapmComponent;
    const struct AudioSapmRoute *sapmRoutes;
    int numSapmRoutes;
};

/* Codec host is defined in codec driver */
struct CodecHost {
    struct IDeviceIoService service;
    struct HdfDeviceObject *device;
    unsigned long priv;
    unsigned long aiaoPriv;
};

enum AudioRegParams {
    PLAYBACK_VOLUME = 0,
    CAPTURE_VOLUME,
    PLAYBACK_MUTE,
    CAPTURE_MUTE,
    LEFT_GAIN,
    RIGHT_GAIN,
    EXTERNAL_CODEC_ENABLE,
    INTERNALLY_CODEC_ENABLE,
    RENDER_CHANNEL_MODE,
    CAPTRUE_CHANNEL_MODE,
};

enum SapmRegParams {
    LPGA_MIC = 0,
    RPGA_MIC,
    DACL2DACR,
    DACR2DACL,
};

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */

#endif /* CODEC_CORE_H */
