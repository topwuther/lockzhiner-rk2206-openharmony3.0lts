/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
 *
 * HDF is dual licensed: you can use it either under the terms of
 * the GPL, or the BSD license, at your option.
 * See the LICENSE file in the root of this repository for complete details.
 */

#include "hi3516_codec_ops.h"
#include "audio_sapm.h"
#include "hi3516_aiao_impl.h"
#include "hi3516_codec_impl.h"
#include "osal_io.h"
#include "osal_irq.h"

#define HDF_LOG_TAG hi3516_codec_ops

#define ACODEC_CTRLREG0_ADDR 0x2c  /* adcl,adcr, dacl, dacr power ctrl reg */
#define ACODEC_DACREG1_ADDR 0x38  /* codec ctrl reg 3 */
#define ACODEC_ADCREG0_ADDR 0x3c  /* codec ctrl reg 4 */
#define ACODEC_REG18_ADDR 0x48  /* codec ctrl reg 18 */
#define ACODEC_ANACTRLREG0_ADDR 0x14 /* codec ana ctrl reg 0 */
#define ACODEC_ANACTRLREG3_ADDR 0x20 /* codec ana ctrl reg 0 */
#define ACODEC_CTRLREG1_ADDR 0x30  /* acodec ctrl reg1 */
#define AIAO_RX_IF_ATTRI_ADDR 0x1000 /* aiao receive channel interface attribute */
#define AIAO_TX_IF_ATTRI_ADDR 0x2000 /* aiao tranfer channel interface attribute */
#define AIAO_TX_DSP_CTRL_ADDR 0x2004

static const struct AudioMixerControl g_audioRegParams[] = {
    {
        .reg = AIAO_TX_DSP_CTRL_ADDR, /* [0] output volume */
        .rreg = AIAO_TX_DSP_CTRL_ADDR,
        .shift = 8,
        .rshift = 8,
        .min = 0x28,
        .max = 0x7F,
        .mask = 0x7F,
        .invert = 0,
    }, {
        .reg = ACODEC_ADCREG0_ADDR, /* [1] input volume */
        .rreg = ACODEC_ADCREG0_ADDR,
        .shift = 24,
        .rshift = 24,
        .min = 0x0,
        .max = 0x57,
        .mask = 0x7F,
        .invert = 1,
    }, {
        .reg = ACODEC_DACREG1_ADDR, /* [2] output mute */
        .rreg = ACODEC_DACREG1_ADDR,
        .shift = 31,
        .rshift = 31,
        .min = 0x0,
        .max = 0x1,
        .mask = 0x1,
        .invert = 0,
    }, {
        .reg = ACODEC_ADCREG0_ADDR, /* [3] input mute */
        .rreg = ACODEC_ADCREG0_ADDR,
        .shift = 31,
        .rshift = 31,
        .min = 0x0,
        .max = 0x1,
        .mask = 0x1,
        .invert = 0,
    }, {
        .reg = ACODEC_ANACTRLREG3_ADDR, /* [4] left mic gain */
        .rreg = ACODEC_ANACTRLREG3_ADDR,
        .shift = 16,
        .rshift = 16,
        .min = 0x0,
        .max = 0xF,
        .mask = 0x1F,
        .invert = 0,
    }, {
        .reg = ACODEC_ANACTRLREG3_ADDR, /* [5] right mic gain */
        .rreg = ACODEC_ANACTRLREG3_ADDR,
        .shift = 24,
        .rshift = 24,
        .min = 0x0,
        .max = 0xF,
        .mask = 0x1F,
        .invert = 0,
    }, {
        .reg = ACODEC_REG18_ADDR, /* [6] external codec enable */
        .rreg = ACODEC_REG18_ADDR,
        .shift = 1,
        .rshift = 1,
        .min = 0x0,
        .max = 0x1,
        .mask = 0x1,
        .invert = 0,
    }, {
        .reg = ACODEC_REG18_ADDR, /* [7] internally codec enable */
        .rreg = ACODEC_REG18_ADDR,
        .shift = 0,
        .rshift = 0,
        .min = 0x0,
        .max = 0x1,
        .mask = 0x1,
        .invert = 0,
    }, {
        .reg = AIAO_TX_IF_ATTRI_ADDR, /* [8] aiao render channel mode */
        .rreg = AIAO_TX_IF_ATTRI_ADDR,
        .shift = 16,
        .rshift = 16,
        .min = 0x0,
        .max = 0x7,
        .mask = 0x7,
        .invert = 0,
    }, {
        .reg = AIAO_RX_IF_ATTRI_ADDR, /* [9] aiao capture channel mode */
        .rreg = AIAO_RX_IF_ATTRI_ADDR,
        .shift = 16,
        .rshift = 16,
        .min = 0x0,
        .max = 0x7,
        .mask = 0x7,
        .invert = 0,
    },
};

static const struct AudioKcontrol g_audioControls[] = {
    {
        .iface = AUDIODRV_CTL_ELEM_IFACE_DAC,
        .name = "Main Playback Volume",
        .Info = AudioInfoCtrlOps,
        .Get = AudioCodecAiaoGetCtrlOps,
        .Set = AudioCodecAiaoSetCtrlOps,
        .privateValue = (unsigned long)&g_audioRegParams[PLAYBACK_VOLUME],
    }, {
        .iface = AUDIODRV_CTL_ELEM_IFACE_ADC,
        .name = "Main Capture Volume",
        .Info = AudioInfoCtrlOps,
        .Get = AudioCodecGetCtrlOps,
        .Set = AudioCodecSetCtrlOps,
        .privateValue = (unsigned long)&g_audioRegParams[CAPTURE_VOLUME],
    }, {
        .iface = AUDIODRV_CTL_ELEM_IFACE_DAC,
        .name = "Playback Mute",
        .Info = AudioInfoCtrlOps,
        .Get = AudioCodecGetCtrlOps,
        .Set = AudioCodecSetCtrlOps,
        .privateValue = (unsigned long)&g_audioRegParams[PLAYBACK_MUTE],
    }, {
        .iface = AUDIODRV_CTL_ELEM_IFACE_ADC,
        .name = "Capture Mute",
        .Info = AudioInfoCtrlOps,
        .Get = AudioCodecGetCtrlOps,
        .Set = AudioCodecSetCtrlOps,
        .privateValue = (unsigned long)&g_audioRegParams[CAPTURE_MUTE],
    }, {
        .iface = AUDIODRV_CTL_ELEM_IFACE_GAIN,
        .name = "Mic Left Gain",
        .Info = AudioInfoCtrlOps,
        .Get = AudioCodecGetCtrlOps,
        .Set = AudioCodecSetCtrlOps,
        .privateValue = (unsigned long)&g_audioRegParams[LEFT_GAIN],
    }, {
        .iface = AUDIODRV_CTL_ELEM_IFACE_GAIN,
        .name = "Mic Right Gain",
        .Info = AudioInfoCtrlOps,
        .Get = AudioCodecGetCtrlOps,
        .Set = AudioCodecSetCtrlOps,
        .privateValue = (unsigned long)&g_audioRegParams[RIGHT_GAIN],
    }, {
        .iface = AUDIODRV_CTL_ELEM_IFACE_ACODEC,
        .name = "External Codec Enable",
        .Info = AudioInfoCtrlOps,
        .Get = AudioCodecGetCtrlOps,
        .Set = AudioCodecSetCtrlOps,
        .privateValue = (unsigned long)&g_audioRegParams[EXTERNAL_CODEC_ENABLE],
    }, {
        .iface = AUDIODRV_CTL_ELEM_IFACE_ACODEC,
        .name = "Internally Codec Enable",
        .Info = AudioInfoCtrlOps,
        .Get = AudioCodecGetCtrlOps,
        .Set = AudioCodecSetCtrlOps,
        .privateValue = (unsigned long)&g_audioRegParams[INTERNALLY_CODEC_ENABLE],
    }, {
        .iface = AUDIODRV_CTL_ELEM_IFACE_AIAO,
        .name = "Render Channel Mode",
        .Info = AudioInfoCtrlOps,
        .Get = AudioCodecAiaoGetCtrlOps,
        .Set = AudioCodecAiaoSetCtrlOps,
        .privateValue = (unsigned long)&g_audioRegParams[RENDER_CHANNEL_MODE],
    }, {
        .iface = AUDIODRV_CTL_ELEM_IFACE_AIAO,
        .name = "Captrue Channel Mode",
        .Info = AudioInfoCtrlOps,
        .Get = AudioCodecAiaoGetCtrlOps,
        .Set = AudioCodecAiaoSetCtrlOps,
        .privateValue = (unsigned long)&g_audioRegParams[CAPTRUE_CHANNEL_MODE],
    },
};

static const struct AudioMixerControl g_audioSapmRegParams[] = {
    {
        .reg = ACODEC_ANACTRLREG3_ADDR, /* LPGA MIC 0 -- connect MIC */
        .rreg = ACODEC_ANACTRLREG3_ADDR,
        .shift = 23,
        .rshift = 23,
        .min = 0x0,
        .max = 0x1,
        .mask = 0x1,
        .invert = 0,
    }, {
        .reg = ACODEC_ANACTRLREG3_ADDR, /* RPGA MIC 0 -- connect MIC */
        .rreg = ACODEC_ANACTRLREG3_ADDR,
        .shift = 31,
        .rshift = 31,
        .min = 0x0,
        .max = 0x1,
        .mask = 0x1,
        .invert = 0,
    }, {
        .reg = ACODEC_CTRLREG1_ADDR, /* [2] dacl to dacr mixer */
        .rreg = ACODEC_CTRLREG1_ADDR,
        .shift = 27,
        .rshift = 27,
        .min = 0x0,
        .max = 0x1,
        .mask = 0x1,
        .invert = 0,
    }, {
        .reg = ACODEC_CTRLREG1_ADDR, /* [3] dacr to dacl mixer */
        .rreg = ACODEC_CTRLREG1_ADDR,
        .shift = 26,
        .rshift = 26,
        .min = 0x0,
        .max = 0x1,
        .mask = 0x1,
        .invert = 0,
      },
};

static struct AudioKcontrol g_audioSapmDACLControls[] = {
    {
        .iface = AUDIODRV_CTL_ELEM_IFACE_DAC,
        .name = "Dacl enable",
        .Info = AudioInfoCtrlOps,
        .Get = AudioCodecSapmGetCtrlOps,
        .Set = AudioCodecSapmSetCtrlOps,
        .privateValue = (unsigned long)&g_audioSapmRegParams[DACL2DACR],
    },
};

static struct AudioKcontrol g_audioSapmDACRControls[] = {
    {
        .iface = AUDIODRV_CTL_ELEM_IFACE_DAC,
        .name = "Dacr enable",
        .Info = AudioInfoCtrlOps,
        .Get = AudioCodecSapmGetCtrlOps,
        .Set = AudioCodecSapmSetCtrlOps,
        .privateValue = (unsigned long)&g_audioSapmRegParams[DACR2DACL],
    },
};

static struct AudioKcontrol g_audioSapmLPGAControls[] = {
    {
        .iface = AUDIODRV_CTL_ELEM_IFACE_PGA,
        .name = "LPGA MIC Switch",
        .Info = AudioInfoCtrlOps,
        .Get = AudioCodecSapmGetCtrlOps,
        .Set = AudioCodecSapmSetCtrlOps,
        .privateValue = (unsigned long)&g_audioSapmRegParams[LPGA_MIC],
    },
};

static struct AudioKcontrol g_audioSapmRPGAControls[] = {
    {
        .iface = AUDIODRV_CTL_ELEM_IFACE_PGA,
        .name = "RPGA MIC Switch",
        .Info = AudioInfoCtrlOps,
        .Get = AudioCodecSapmGetCtrlOps,
        .Set = AudioCodecSapmSetCtrlOps,
        .privateValue = (unsigned long)&g_audioSapmRegParams[RPGA_MIC],
    },
};

static const struct AudioSapmComponent g_streamDomainComponents[] = {
    {
        .sapmType = AUDIO_SAPM_ADC, /* ADCL */
        .componentName = "ADCL",
        .reg = ACODEC_ANACTRLREG3_ADDR,
        .mask = 0x1,
        .shift = 15,
        .invert = 1,
    }, {
        .sapmType = AUDIO_SAPM_ADC, /* ADCR */
        .componentName = "ADCR",
        .reg = ACODEC_ANACTRLREG3_ADDR,
        .mask = 0x1,
        .shift = 14,
        .invert = 1,
    }, {
        .sapmType = AUDIO_SAPM_DAC, /* DACL */
        .componentName = "DACL",
        .reg = ACODEC_ANACTRLREG0_ADDR,
        .mask = 0x1,
        .shift = 11,
        .invert = 1,
    }, {
        .sapmType = AUDIO_SAPM_DAC, /* DACR */
        .componentName = "DACR",
        .reg = ACODEC_ANACTRLREG0_ADDR,
        .mask = 0x1,
        .shift = 12,
        .invert = 1,
    }, {
        .sapmType = AUDIO_SAPM_PGA, /* LPGA */
        .componentName = "LPGA",
        .reg = ACODEC_ANACTRLREG3_ADDR,
        .mask = 0x1,
        .shift = 13,
        .invert = 1,
        .kcontrolNews = g_audioSapmLPGAControls,
        .kcontrolsNum = HDF_ARRAY_SIZE(g_audioSapmLPGAControls),
    }, {
        .sapmType = AUDIO_SAPM_PGA, /* RPGA */
        .componentName = "RPGA",
        .reg = ACODEC_ANACTRLREG3_ADDR,
        .mask = 0x1,
        .shift = 12,
        .invert = 1,
        .kcontrolNews = g_audioSapmRPGAControls,
        .kcontrolsNum = HDF_ARRAY_SIZE(g_audioSapmRPGAControls),
    }, {
        .sapmType = AUDIO_SAPM_SPK, /* SPKL */
        .componentName = "SPKL",
        .reg = AUDIO_SAPM_NOPM,
        .mask = 0x1,
        .shift = 0,
        .invert = 0,
        .kcontrolNews = g_audioSapmDACLControls,
        .kcontrolsNum = HDF_ARRAY_SIZE(g_audioSapmDACLControls),
    }, {
        .sapmType = AUDIO_SAPM_SPK, /* SPKR */
        .componentName = "SPKR",
        .reg = AUDIO_SAPM_NOPM,
        .mask = 0x1,
        .shift = 0,
        .invert = 0,
        .kcontrolNews = g_audioSapmDACRControls,
        .kcontrolsNum = HDF_ARRAY_SIZE(g_audioSapmDACRControls),
    }, {
        .sapmType = AUDIO_SAPM_MIC, /* MIC */
        .componentName = "MIC",
        .reg = AUDIO_SAPM_NOPM,
        .mask = 0x1,
        .shift = 0,
        .invert = 0,
    },
};

static const struct AudioSapmRoute g_audioRoutes[] = {
    { "SPKL", "Dacl enable", "DACL"},
    { "SPKR", "Dacr enable", "DACR"},

    { "ADCL", NULL, "LPGA"},
    { "LPGA", "LPGA MIC Switch", "MIC"},

    { "ADCR", NULL, "RPGA"},
    { "RPGA", "RPGA MIC Switch", "MIC"},
};

int32_t CodecDeviceInit(struct AudioCard *audioCard, struct CodecDevice *codec)
{
    const int innerCodecEnable = 7;
    struct VirtualAddress *virtualAdd = NULL;
    struct AudioMixerControl mixerCtrl = g_audioRegParams[innerCodecEnable];

    if ((audioCard == NULL) || (codec == NULL || codec->device == NULL)) {
        AUDIO_DRIVER_LOG_ERR("input para is NULL.");
        return HDF_ERR_INVALID_OBJECT;
    }
    virtualAdd = (struct VirtualAddress *)OsalMemCalloc(sizeof(struct VirtualAddress));
    if (virtualAdd == NULL) {
        AUDIO_DRIVER_LOG_ERR("virtualAdd fail!");
        return HDF_ERR_MALLOC_FAIL;
    }
    virtualAdd->acodecVir = (uintptr_t)OsalIoRemap(ACODEC_REG_BASE, ACODEC_MAX_REG_SIZE);
    virtualAdd->aiaoVir = (uintptr_t)OsalIoRemap(AIAO_REG_BASE, AIAO_MAX_REG_SIZE);
    codec->device->priv = virtualAdd;

    /*  default inner codec */
    if (AudioUpdateCodecRegBits(codec, &mixerCtrl, 1) != HDF_SUCCESS) {
        AUDIO_DRIVER_LOG_ERR("update reg bits fail.");
        return HDF_FAILURE;
    }

    if (CodecHalSysInit() != HDF_SUCCESS) {
        AUDIO_DRIVER_LOG_ERR("CodecHalSysInit fail.");
        return HDF_FAILURE;
    }

    AcodecDeviceInit();

    if (AudioAddControls(audioCard, g_audioControls, HDF_ARRAY_SIZE(g_audioControls)) != HDF_SUCCESS) {
        AUDIO_DRIVER_LOG_ERR("add controls fail.");
        return HDF_FAILURE;
    }

    if (AudioSapmNewComponents(audioCard, g_streamDomainComponents,
        HDF_ARRAY_SIZE(g_streamDomainComponents)) != HDF_SUCCESS) {
        AUDIO_DRIVER_LOG_ERR("new components fail.");
        return HDF_FAILURE;
    }

    if (AudioSapmAddRoutes(audioCard, g_audioRoutes, HDF_ARRAY_SIZE(g_audioRoutes)) != HDF_SUCCESS) {
        AUDIO_DRIVER_LOG_ERR("add route fail.");
        return HDF_FAILURE;
    }

    if (AudioSapmNewControls(audioCard) != HDF_SUCCESS) {
        AUDIO_DRIVER_LOG_ERR("add sapm controls fail.");
        return HDF_FAILURE;
    }

    if (AudioSapmSleep(audioCard) != HDF_SUCCESS) {
        AUDIO_DRIVER_LOG_ERR("add sapm sleep modular fail.");
        return HDF_FAILURE;
    }

    AUDIO_DRIVER_LOG_DEBUG("success.");
    return HDF_SUCCESS;
}

int32_t CodecDaiDeviceInit(const struct AudioCard *card, const struct DaiDevice *device)
{
    if (device == NULL || device->devDaiName == NULL) {
        AUDIO_DRIVER_LOG_ERR("input para is NULL.");
        return HDF_FAILURE;
    }

    AUDIO_DRIVER_LOG_DEBUG("codec dai device name: %s\n", device->devDaiName);
    (void)card;
    return HDF_SUCCESS;
}

static int32_t FramatToBitWidth(enum AudioFormat format, unsigned int *bitWidth)
{
    switch (format) {
        case AUDIO_FORMAT_PCM_8_BIT:
            *bitWidth = BIT_WIDTH8;
            break;

        case AUDIO_FORMAT_PCM_16_BIT:
            *bitWidth = BIT_WIDTH16;
            break;

        case AUDIO_FORMAT_PCM_24_BIT:
            *bitWidth = BIT_WIDTH24;
            break;

        case AUDIO_FORMAT_PCM_32_BIT:
            *bitWidth = BIT_WIDTH32;
            break;

        case AUDIO_FORMAT_AAC_MAIN:
        case AUDIO_FORMAT_AAC_LC:
        case AUDIO_FORMAT_AAC_LD:
        case AUDIO_FORMAT_AAC_ELD:
        case AUDIO_FORMAT_AAC_HE_V1:
        case AUDIO_FORMAT_AAC_HE_V2:
            break;

        default:
            AUDIO_DRIVER_LOG_ERR("format: %d is not define.", format);
            return HI_FAILURE;
            break;
    }

    return HI_SUCCESS;
}


int32_t CodecDaiHwParams(const struct AudioCard *card,
    const struct AudioPcmHwParams *param, const struct DaiDevice *device)
{
    int ret;
    unsigned int bitWidth;
    (void)card;
    (void)device;

    AUDIO_DRIVER_LOG_DEBUG("entry.");
    if (param == NULL || param->cardServiceName == NULL) {
        AUDIO_DRIVER_LOG_ERR("input para is NULL.");
        return HDF_FAILURE;
    }

    ret = FramatToBitWidth(param->format, &bitWidth);
    if (ret != HDF_SUCCESS) {
        AUDIO_DRIVER_LOG_ERR("FramatToBitWidth: fail.");
        return HDF_FAILURE;
    }

    ret = AcodecSetI2s1Fs(param->rate);
    if (ret != HDF_SUCCESS) {
        AUDIO_DRIVER_LOG_ERR("AcodecSetI2s1Fs fail.");
        return HDF_FAILURE;
    }

    ret = AcodecSetI2s1DataWidth(bitWidth);
    if (ret != HDF_SUCCESS) {
        AUDIO_DRIVER_LOG_ERR("AcodecSetI2s1DataWidth fail.");
        return HDF_FAILURE;
    }

    AUDIO_DRIVER_LOG_DEBUG("channels = %d, rate = %d, periodSize = %d, \
        periodCount = %d, format = %d, cardServiceName = %s \n",
        param->channels, param->rate, param->periodSize,
        param->periodCount, (uint32_t)param->format, param->cardServiceName);

    AUDIO_DRIVER_LOG_DEBUG("success.");
    return HDF_SUCCESS;
}

int32_t CodecDaiStartup(const struct AudioCard *card, const struct DaiDevice *device)
{
    (void)card;
    (void)device;

    return HDF_SUCCESS;
}

