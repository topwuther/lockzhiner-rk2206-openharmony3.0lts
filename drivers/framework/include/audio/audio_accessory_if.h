/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
 *
 * HDF is dual licensed: you can use it either under the terms of
 * the GPL, or the BSD license, at your option.
 * See the LICENSE file in the root of this repository for complete details.
 */

#ifndef AUDIO_ACCESSORY_IF_H
#define AUDIO_ACCESSORY_IF_H

#include "audio_host.h"
#include "audio_control.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* __cplusplus */

struct AccessoryDevice {
    const char *devAccessoryName;
    struct AccessoryData *devData;
    struct HdfDeviceObject *device;
    struct DListHead list;
    struct OsalMutex mutex;
};

struct AudioAccessoryOps {
    const char *devAccessoryName;
    struct AccessoryData *devData;
    struct HdfDeviceObject *device;
    struct DListHead list;
};

/* Accessory host is defined in accessory driver */
struct AccessoryHost {
    struct IDeviceIoService service; // accessory service
    struct HdfDeviceObject *device;  // accessory deovce
    void *priv;                     // accessory private data
};

struct AccessoryData {
    const char *drvAccessoryName;
    /* Accessory driver callbacks */
    int32_t (*Init)(struct AudioCard *, const struct AccessoryDevice *device);
    int32_t (*Read)(const struct AccessoryDevice *, uint32_t, uint32_t *);
    int32_t (*Write)(const struct AccessoryDevice *, uint32_t, uint32_t);
    int32_t (*AiaoRead)(const struct AccessoryDevice *, uint32_t, uint32_t *);
    int32_t (*AiaoWrite)(const struct AccessoryDevice *, uint32_t, uint32_t);

    const struct AudioKcontrol *controls;
    int numControls;
};

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */

#endif
