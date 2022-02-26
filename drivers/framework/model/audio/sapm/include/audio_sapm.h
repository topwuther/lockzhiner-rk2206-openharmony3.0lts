/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
 *
 * HDF is dual licensed: you can use it either under the terms of
 * the GPL, or the BSD license, at your option.
 * See the LICENSE file in the root of this repository for complete details.
 */

#ifndef AUDIO_SAPM_H
#define AUDIO_SAPM_H

#include "audio_core.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* __cplusplus */

#define SAPM_POLL_TIME 10000        /* 10s */
#define SAPM_SLEEP_TIME (3 * 60000) /* 3min */

#define MIXER_REG_ADDR 10 /* mixer address -- Temporarily defined as this value */

#define SAPM_POWER_DOWN 0
#define SAPM_POWER_UP 1

#define CONNECT_CODEC_PIN 1
#define UNCONNECT_CODEC_PIN 0

#define EXIST_EXTERNAL_WIDGET 1
#define UNEXIST_EXTERNAL_WIDGET 1

#define CONNECT_SINK_AND_SOURCE 1
#define UNCONNECT_SINK_AND_SOURCE 0

/* sapm widget types */
enum AudioSapmType {
    AUDIO_SAPM_INPUT = 0,       /* input pin */
    AUDIO_SAPM_OUTPUT,          /* output pin */
    AUDIO_SAPM_MUX,             /* selects 1 analog signal from many inputs */
    AUDIO_SAPM_VIRT_MUX,        /* virtual version of snd_soc_dapm_mux */
    AUDIO_SAPM_VALUE_MUX,       /* selects 1 analog signal from many inputs */
    AUDIO_SAPM_MIXER,           /* mixes several analog signals together */
    AUDIO_SAPM_MIXER_NAMED_CTRL, /* mixer with named controls */
    AUDIO_SAPM_PGA,             /* programmable gain/attenuation (volume) */
    AUDIO_SAPM_OUT_DRV,         /* output driver */
    AUDIO_SAPM_ADC,             /* analog to digital converter */
    AUDIO_SAPM_DAC,             /* digital to analog converter */
    AUDIO_SAPM_MICBIAS,         /* microphone bias (power) */
    AUDIO_SAPM_MIC,             /* microphone */
    AUDIO_SAPM_HP,              /* headphones */
    AUDIO_SAPM_SPK,             /* speaker */
    AUDIO_SAPM_LINE,            /* line input/output */
    AUDIO_SAPM_ANALOG_SWITCH,   /* analog switch */
    AUDIO_SAPM_VMID,            /* codec bias/vmid - to minimise pops */
    AUDIO_SAPM_PRE,             /* machine specific pre component - exec first */
    AUDIO_SAPM_POST,            /* machine specific post component - exec last */
    AUDIO_SAPM_SUPPLY,          /* power/clock supply */
    AUDIO_SAPM_AIF_IN,          /* audio interface input */
    AUDIO_SAPM_AIF_OUT,         /* audio interface output */
};

/* component has no PM register bit */
#define AUDIO_SAPM_NOPM    (-1)

/* dapm stream operations */
#define AUDIO_SAPM_STREAM_NOP           0x0
#define AUDIO_SAPM_STREAM_START         0x1
#define AUDIO_SAPM_STREAM_STOP          0x2
#define AUDIO_SAPM_STREAM_SUSPEND       0x4
#define AUDIO_SAPM_STREAM_RESUME        0x8
#define AUDIO_SAPM_STREAM_PAUSE_PUSH    0x10
#define AUDIO_SAPM_STREAM_PAUSE_RELEASE 0x20

/* sapm event types */
#define AUDIO_SAPM_PRE_PMU    0x1     /* before component power up */
#define AUDIO_SAPM_POST_PMU   0x2     /* after component power up */
#define AUDIO_SAPM_PRE_PMD    0x4     /* before component power down */
#define AUDIO_SAPM_POST_PMD   0x8     /* after component power down */
#define AUDIO_SAPM_PRE_REG    0x10    /* before audio path setup */
#define AUDIO_SAPM_POST_REG   0x20    /* after audio path setup */
#define AUDIO_SAPM_PRE_POST_PMD (AUDIO_SAPM_PRE_PMD | AUDIO_SAPM_POST_PMD)

enum AudioBiasLevel {
    AUDIO_BIAS_OFF = 0,
    AUDIO_BIAS_STANDBY = 1,
    AUDIO_BIAS_PREPARE = 2,
    AUDIO_BIAS_ON = 3,
};

/* SAPM context */
struct AudioSapmContext {
    int32_t componentNum; /* number of components in this context */
    enum AudioBiasLevel biasLevel;
    enum AudioBiasLevel suspendBiasLevel;

    struct CodecDevice *codec; /* parent codec */
    struct PlatformDevice *platform; /* parent platform */
    struct AudioCard *card; /* parent card */

    /* used during SAPM updates */
    enum AudioBiasLevel targetBiasLevel;
    struct DListHead list;
};

/* enumerated kcontrol */
struct AudioEnumKcontrol {
    uint32_t reg;
    uint32_t reg2;
    uint8_t shiftLeft;
    uint8_t shiftRight;
    uint32_t max;
    uint32_t mask;
    const char * const *texts;
    const uint32_t *values;
    void *sapm;
};

/* sapm audio path between two components */
struct AudioSapmpath {
    char *name;

    /* source (input) and sink (output) components */
    struct AudioSapmComponent *source;
    struct AudioSapmComponent *sink;
    struct AudioKcontrol *kcontrol;

    /* status */
    uint8_t connect; /* source and sink components are connected */
    uint8_t walked; /* path has been walked */
    uint8_t weak; /* path ignored for power management */

    int32_t (*connected)(struct AudioSapmComponent *source, struct AudioSapmComponent *sink);

    struct DListHead listSource;
    struct DListHead listSink;
    struct DListHead list;
};

/* sapm component */
struct AudioSapmComponent {
    enum AudioSapmType sapmType;
    char *componentName; /* component name */
    char *streamName; /* stream name */
    struct AudioSapmContext *sapm;
    struct CodecDevice *codec; /* parent codec */
    struct AccessoryDevice *accessory; /* parent accessory */
    struct PlatformDevice *platform; /* parent platform */

    /* sapm control */
    int16_t reg; /* negative reg = no direct sapm */
    uint8_t shift; /* bits to shift */
    uint8_t invert; /* invert the power bit */
    uint8_t mask;
    uint8_t connected; /* connected codec pin */
    uint8_t external; /* has external components */
    uint8_t active; /* active stream on DAC, ADC's */
    uint8_t newPower; /* power checked this run */
    uint8_t power;
    uint8_t newCpt;

    /* external events */
    uint16_t eventFlags;   /* flags to specify event types */
    int32_t (*Event)(struct AudioSapmComponent*, struct AudioKcontrol *, int32_t);

    /* power check callback */
    int32_t (*PowerCheck)(const struct AudioSapmComponent *);

    /* kcontrols that relate to this component */
    int32_t kcontrolsNum;
    struct AudioKcontrol *kcontrolNews;
    struct AudioKcontrol **kcontrols;

    struct DListHead list;

    /* component input and outputs */
    struct DListHead sources;
    struct DListHead sinks;

    /* used during SAPM updates */
    struct DListHead powerList;
    struct DListHead dirty;

    /* reserve clock interface */
    int32_t (*PowerClockOp)(struct AudioSapmComponent *);
};

struct AudioSapmRoute {
    const char *sink;
    const char *control;
    const char *source;

    /* Note: currently only supported for links where source is a supply */
    uint32_t (*Connected)(struct AudioSapmComponent *source,
        struct AudioSapmComponent *sink);
};

int32_t AudioSapmNewComponents(struct AudioCard *audioCard,
    const struct AudioSapmComponent *component, int32_t cptMaxNum);
int32_t AudioSapmAddRoutes(struct AudioCard *audioCard,
    const struct AudioSapmRoute *route, int32_t routeMaxNum);
int32_t AudioSapmNewControls(struct AudioCard *audioCard);
int AudioSapmPowerComponents(struct AudioCard *audioCard);
int32_t AudioSapmSleep(const struct AudioCard *audioCard);
uint64_t AudioSapmRefreshTime(bool bRefresh);
int32_t AudioSampPowerUp(const struct AudioCard *card);
int32_t AudioSampSetPowerMonitor(struct AudioCard *card, bool powerMonitorState);

int32_t AudioCodecSapmSetCtrlOps(const struct AudioKcontrol *kcontrol, const struct AudioCtrlElemValue *elemValue);
int32_t AudioCodecSapmGetCtrlOps(const struct AudioKcontrol *kcontrol, struct AudioCtrlElemValue *elemValue);
int32_t AudioAccessorySapmSetCtrlOps(const struct AudioKcontrol *kcontrol, const struct AudioCtrlElemValue *elemValue);
int32_t AudioAccessorySapmGetCtrlOps(const struct AudioKcontrol *kcontrol, struct AudioCtrlElemValue *elemValue);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */

#endif /* AUDIO_SAPM_H */
