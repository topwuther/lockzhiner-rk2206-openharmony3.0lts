/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
 *
 * HDF is dual licensed: you can use it either under the terms of
 * the GPL, or the BSD license, at your option.
 * See the LICENSE file in the root of this repository for complete details.
 */

#ifndef AUDIO_PLATFORM_IF_H
#define AUDIO_PLATFORM_IF_H

#include "audio_host.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* __cplusplus */

#define I2S_IOCFG2_BASE1 0x0020
#define I2S_IOCFG2_BASE2 0x0024
#define I2S_IOCFG2_BASE3 0x0028
#define I2S_IOCFG2_BASE4 0x002C
#define I2S_IOCFG2_BASE5 0x0030

#define I2S_IOCFG2_BASE1_VAL 0x663
#define I2S_IOCFG2_BASE2_VAL 0x673
#define I2S_IOCFG2_BASE3_VAL 0x573
#define I2S_IOCFG2_BASE4_VAL 0x473
#define I2S_IOCFG2_BASE5_VAL 0x433

#define I2S_IOCFG3_BASE1 0x44
#define I2S_IOCFG3_BASE1_VAL 0x0600

#define GPIO_BASE1 0x2010
#define GPIO_BASE2 0x2400
#define GPIO_BASE3 0x2010

#define GPIO_BASE2_VAL 0x000000ff
#define GPIO_BASE3_VAL 0x00000000

#define IOCFG2_BASE_ADDR 0x112F0000
#define IOCFG3_BASE_ADDR 0x10FF0000
#define GPIO_BASE_ADDR 0x120D0000
#define BASE_ADDR_REMAP_SIZE 0x10000

struct CircleBufInfo {
    uint32_t cirBufSize;
    uint32_t trafBufSize;
    uint32_t period;
    uint32_t periodSize;
    uint32_t periodCount;
    unsigned long phyAddr;
    uint32_t *virtAddr;
    uint32_t wbufOffSet;
    uint32_t wptrOffSet;
    uint32_t runStatus;
    uint32_t chnId;
    uint32_t enable;
    struct OsalMutex buffMutex;
    uint64_t framesPosition;
};

struct PcmInfo {
    /* The number of channels in a frame */
    uint32_t channels;
    /* The number of frames per second */
    uint32_t rate;
    uint32_t bitWidth;
    uint32_t frameSize;
    bool isBigEndian;
    bool isSignedData;
    uint32_t startThreshold;
    uint32_t stopThreshold;
    uint32_t silenceThreshold;
    uint32_t totalStreamSize;
};

/* platform related definitions */
struct AudioPlatformOps {
    int32_t (*HwParams)(const struct AudioCard *, const struct AudioPcmHwParams *);
    int32_t (*RenderTrigger)(struct AudioCard *, int);
    int32_t (*CaptureTrigger)(struct AudioCard *, int);
    uint32_t (*Pointer)(struct AudioCard *);
    int32_t (*Write)(const struct AudioCard *, struct AudioTxData *);
    int32_t (*Read)(const struct AudioCard *, struct AudioRxData *);
    int32_t (*MmapWrite)(const struct AudioCard *, struct AudioTxMmapData *);
    int32_t (*MmapRead)(const struct AudioCard *, struct AudioRxMmapData *);
    int32_t (*RenderPrepare)(const struct AudioCard *);
    int32_t (*CapturePrepare)(const struct AudioCard *);
    int32_t (*RenderStart)(struct AudioCard *);
    int32_t (*CaptureStart)(struct AudioCard *);
    int32_t (*RenderStop)(struct AudioCard *);
    int32_t (*CaptureStop)(struct AudioCard *);
    int32_t (*RenderPause)(struct AudioCard *);
    int32_t (*CapturePause)(struct AudioCard *);
    int32_t (*RenderResume)(struct AudioCard *);
    int32_t (*CaptureResume)(struct AudioCard *);
};

struct PlatformDevice {
    const char *devPlatformName;
    struct PlatformData *devData;
    struct HdfDeviceObject *device;
    struct DListHead list;
};

struct PlatformData {
    const char *drvPlatformName;
    /* platform driver callbacks */
    int32_t (*PlatformInit)(const struct AudioCard *, const struct PlatformDevice *);
    /* pcm creation and destruction */
    int32_t (*PcmNew)(struct PlatformDevice *);
    void (*PcmFree)(struct PlatformDevice *);
    /* platform stream ops */
    struct AudioPlatformOps *ops;
};

/* Platform host is defined in platform driver */
struct PlatformHost {
    struct IDeviceIoService service;
    struct HdfDeviceObject *device;
    void *priv;
    bool platformInitFlag;
    struct CircleBufInfo renderBufInfo;
    struct CircleBufInfo captureBufInfo;
    struct PcmInfo pcmInfo;
};

static inline struct PlatformHost *PlatformHostFromDevice(struct HdfDeviceObject *device)
{
    return (device == NULL) ? NULL : (struct PlatformHost *)device->service;
}

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */

#endif
