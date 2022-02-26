/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
 *
 * HDF is dual licensed: you can use it either under the terms of
 * the GPL, or the BSD license, at your option.
 * See the LICENSE file in the root of this repository for complete details.
 */

#ifndef AUDIO_DAI_IF_H
#define AUDIO_DAI_IF_H

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* __cplusplus */

struct DaiDevice {
    const char *devDaiName;
    struct DaiData *devData;
    struct HdfDeviceObject *device;
    struct DListHead list;
};

struct AudioDaiOps {
    int32_t (*Startup)(const struct AudioCard *, const struct DaiDevice *);
    int32_t (*HwParams)(const struct AudioCard *, const struct AudioPcmHwParams *, const struct DaiDevice *);
    int32_t (*Trigger)(const struct AudioCard *, int, const struct DaiDevice *);
};

struct DaiData {
    const char *drvDaiName;
    /* DAI driver callbacks */
    int32_t (*DaiInit)(const struct AudioCard *, const struct DaiDevice *);
    /* ops */
    const struct AudioDaiOps *ops;
};

/* Dai host is defined in dai driver */
struct DaiHost {
    struct IDeviceIoService service;
    struct HdfDeviceObject *device;
    void *priv;
    bool daiInitFlag;
};

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */

#endif
