/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
 *
 * HDF is dual licensed: you can use it either under the terms of
 * the GPL, or the BSD license, at your option.
 * See the LICENSE file in the root of this repository for complete details.
 */

#ifndef AUDIO_CONTROL_H
#define AUDIO_CONTROL_H

#include "hdf_dlist.h"


#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* __cplusplus */

struct AudioCtrlElemId {
    const char *cardServiceName;
    int32_t iface;
    const char *itemName; /* ASCII name of item */
};

struct AudioCtrlElemInfo {
    struct AudioCtrlElemId id;
    uint32_t count; /* count of values */
    int32_t type; /* R: value type - AUDIODRV_CTL_ELEM_IFACE_MIXER_* */
    int32_t min; /* R: minimum value */
    int32_t max; /* R: maximum value */
};

struct AudioCtrlElemValue {
    struct AudioCtrlElemId id;
    uint32_t value[2];
};

struct AudioKcontrol;
typedef int32_t (*KconfigInfo_t)(const struct AudioKcontrol *kcontrol, struct AudioCtrlElemInfo *elemInfo);
typedef int32_t (*KconfigGet_t)(const struct AudioKcontrol *kcontrol, struct AudioCtrlElemValue *elemValue);
typedef int32_t (*KconfigSet_t)(const struct AudioKcontrol *kcontrol, const struct AudioCtrlElemValue *elemValue);

/* mixer control */
struct AudioMixerControl {
    int32_t min;
    int32_t max;
    int32_t platformMax;
    uint32_t mask;
    uint32_t reg;
    uint32_t rreg; /* right sound channel reg */
    uint32_t shift;
    uint32_t rshift; /* right sound channel reg shift */
    uint32_t invert;
};

struct AudioKcontrol {
    const char *name; /* ASCII name of item */
    int32_t iface;
    KconfigInfo_t Info;
    KconfigGet_t Get;
    KconfigSet_t Set;
    void *privateData;
    void *pri;
    unsigned long privateValue;
    struct DListHead list; /* list of controls */
};

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */

#endif
