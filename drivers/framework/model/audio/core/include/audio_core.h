/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
 *
 * HDF is dual licensed: you can use it either under the terms of
 * the GPL, or the BSD license, at your option.
 * See the LICENSE file in the root of this repository for complete details.
 */

#ifndef AUDIO_CORE_H
#define AUDIO_CORE_H

#include "audio_host.h"
#include "audio_control.h"
#include "audio_codec_if.h"
#include "audio_platform_if.h"
#include "audio_dai_if.h"
#include "audio_accessory_if.h"
#include "audio_dsp_if.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* __cplusplus */

#define CHANNEL_MAX_NUM 2
#define CHANNEL_MIN_NUM 1

#define AUDIO_DAI_LINK_COMPLETE 1
#define AUDIO_DAI_LINK_UNCOMPLETE 0

#define AUDIO_DRIVER_LOG_ERR(fmt, arg...) do { \
    HDF_LOGE("[%s][line:%d]: " fmt, __func__, __LINE__, ##arg); \
    } while (0)

#define AUDIO_DRIVER_LOG_INFO(fmt, arg...) do { \
    HDF_LOGI("[%s][line:%d]: " fmt, __func__, __LINE__, ##arg); \
    } while (0)

#define AUDIO_DRIVER_LOG_WARNING(fmt, arg...) do { \
    HDF_LOGW("[%s][line:%d]: " fmt, __func__, __LINE__, ##arg); \
    } while (0)

#define AUDIO_DRIVER_LOG_DEBUG(fmt, arg...) do { \
    HDF_LOGD("[%s][line:%d]: " fmt, __func__, __LINE__, ##arg); \
    } while (0)

enum AudioDeviceType {
    AUDIO_DAI_DEVICE,
    AUDIO_DSP_DEVICE,
    AUDIO_PLATFORM_DEVICE,
    AUDIO_CODEC_DEVICE,
    AUDIO_ACCESSORY_DEVICE,
    AUDIO_DEVICE_BUTT,
};

enum PlayStatus {
    STREAM_START = 4,
    STREAM_STOP,
};

/* Dai registration interface */
int32_t AudioSocRegisterDai(struct HdfDeviceObject *device, struct DaiData *data);
/* Platform registration interface */
int32_t AudioSocRegisterPlatform(struct HdfDeviceObject *device, struct PlatformData *data);
/* Codec registration interface */
int32_t AudioRegisterCodec(struct HdfDeviceObject *device, struct CodecData *codecData, struct DaiData *daiData);
int32_t AudioBindDaiLink(struct AudioCard *audioCard, const struct AudioConfigData *configData);
int32_t AudioSocDeviceRegister(struct HdfDeviceObject *device, void *data, enum AudioDeviceType deviceType);
int32_t AudioSocRegisterDsp(struct HdfDeviceObject *device, struct DaiData *data);
int32_t AudioRegisterAccessory(struct HdfDeviceObject *device, struct AccessoryData *data, struct DaiData *daiData);
int32_t AudioUpdateCodecRegBits(struct CodecDevice *codec,
    const struct AudioMixerControl *mixerControl, uint32_t value);
int32_t AudioUpdateAccessoryRegBits(struct AccessoryDevice *accessory,
    const struct AudioMixerControl *mixerControl, uint32_t value);
int32_t AudioUpdateCodecAiaoRegBits(struct CodecDevice *codec, const struct AudioMixerControl *mixerControl,
    uint32_t value);
int32_t AudioUpdateAccessoryAiaoRegBits(struct AccessoryDevice *accessory,
    const struct AudioMixerControl *mixerControl, uint32_t value);
struct CodecDevice *AudioKcontrolGetCodec(const struct AudioKcontrol *kcontrol);
struct AccessoryDevice *AudioKcontrolGetAccessory(const struct AudioKcontrol *kcontrol);
int32_t AudioAddControls(struct AudioCard *audioCard,
    const struct AudioKcontrol *controls, int32_t controlMaxNum);
struct AudioKcontrol *AudioAddControl(const struct AudioCard *audioCard, const struct AudioKcontrol *ctl);

int32_t AudioCodecReadReg(const struct CodecDevice *codec, uint32_t reg, uint32_t *val);
int32_t AudioCodecWriteReg(const struct CodecDevice *codec, uint32_t reg, uint32_t val);
int32_t AudioAccessoryReadReg(const struct AccessoryDevice *accessory, uint32_t reg, uint32_t *val);
int32_t AudioAccessoryWriteReg(const struct AccessoryDevice *accessory, uint32_t reg, uint32_t val);
int32_t AudioCodecAiaoReadReg(const struct CodecDevice *codec, uint32_t reg, uint32_t *val);
int32_t AudioCodecAiaoWriteReg(struct CodecDevice *codec, uint32_t reg, uint32_t val);
int32_t AudioAccessoryAiaoReadReg(const struct AccessoryDevice *accessory, uint32_t reg, uint32_t *val);
int32_t AudioAccessoryAiaoWriteReg(const struct AccessoryDevice *accessory, uint32_t reg, uint32_t val);

int32_t AudioInfoCtrlOps(const struct AudioKcontrol *kcontrol, struct AudioCtrlElemInfo *elemInfo);
int32_t AudioCodecGetCtrlOps(const struct AudioKcontrol *kcontrol, struct AudioCtrlElemValue *elemValue);
int32_t AudioCodecSetCtrlOps(const struct AudioKcontrol *kcontrol, const struct AudioCtrlElemValue *elemValue);
int32_t AudioAccessoryGetCtrlOps(const struct AudioKcontrol *kcontrol, struct AudioCtrlElemValue *elemValue);
int32_t AudioAccessorySetCtrlOps(const struct AudioKcontrol *kcontrol, const struct AudioCtrlElemValue *elemValue);
int32_t AudioCodecAiaoGetCtrlOps(const struct AudioKcontrol *kcontrol, struct AudioCtrlElemValue *elemValue);
int32_t AudioCodecAiaoSetCtrlOps(const struct AudioKcontrol *kcontrol,  const struct AudioCtrlElemValue *elemValue);
int32_t AudioAccessoryAiaoGetCtrlOps(const struct AudioKcontrol *kcontrol, struct AudioCtrlElemValue *elemValue);
int32_t AudioAccessoryAiaoSetCtrlOps(const struct AudioKcontrol *kcontrol, const struct AudioCtrlElemValue *elemValue);
int32_t AudioRegisterDsp(struct HdfDeviceObject *device, struct DspData *dspData, struct DaiData *DaiData);
#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */

#endif /* AUDIO_CORE_H */
