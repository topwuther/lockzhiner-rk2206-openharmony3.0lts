/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
 *
 * HDF is dual licensed: you can use it either under the terms of
 * the GPL, or the BSD license, at your option.
 * See the LICENSE file in the root of this repository for complete details.
 */

#ifndef AUDIO_STREAM_DISP_H
#define AUDIO_STREAM_DISP_H

#include "audio_host.h"
#include "audio_codec_if.h"
#include "audio_platform_if.h"
#include "audio_dai_if.h"
#include "audio_dsp_if.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* __cplusplus */

#define AUDIO_SERVICE_NAME_MAX_LEN 32

enum StreamDispMethodCmd {
    AUDIO_DRV_PCM_IOCTRL_HW_PARAMS,
    AUDIO_DRV_PCM_IOCTRL_RENDER_PREPARE,
    AUDIO_DRV_PCM_IOCTRL_CAPTURE_PREPARE,
    AUDIO_DRV_PCM_IOCTRL_WRITE,
    AUDIO_DRV_PCM_IOCTRL_READ,
    AUDIO_DRV_PCM_IOCTRL_RENDER_START,
    AUDIO_DRV_PCM_IOCTRL_RENDER_STOP,
    AUDIO_DRV_PCM_IOCTRL_CAPTURE_START,
    AUDIO_DRV_PCM_IOCTRL_CAPTURE_STOP,
    AUDIO_DRV_PCM_IOCTRL_RENDER_PAUSE,
    AUDIO_DRV_PCM_IOCTRL_CAPTURE_PAUSE,
    AUDIO_DRV_PCM_IOCTRL_RENDER_RESUME,
    AUDIO_DRV_PCM_IOCTRL_CAPTURE_RESUME,
    AUDIO_DRV_PCM_IOCTL_MMAP_BUFFER,
    AUDIO_DRV_PCM_IOCTL_MMAP_BUFFER_CAPTURE,
    AUDIO_DRV_PCM_IOCTL_MMAP_POSITION,
    AUDIO_DRV_PCM_IOCTL_MMAP_POSITION_CAPTURE,
    AUDIO_DRV_PCM_IOCTRL_DSP_DECODE,
    AUDIO_DRV_PCM_IOCTRL_DSP_ENCODE,
    AUDIO_DRV_PCM_IOCTRL_DSP_EQUALIZER,
    AUDIO_DRV_PCM_IOCTRL_BUTT,
};

typedef int32_t (*StreamDispCmdHandle)(const struct HdfDeviceIoClient *client,
    struct HdfSBuf *data, struct HdfSBuf *reply);

struct StreamDispCmdHandleList {
    enum StreamDispMethodCmd cmd;
    StreamDispCmdHandle func;
};

struct StreamHost {
    struct IDeviceIoService service;
    struct HdfDeviceObject *device;
    void *priv;
};

static inline struct StreamHost *StreamHostFromDevice(struct HdfDeviceObject *device)
{
    return (device == NULL) ? NULL : (struct StreamHost *)device->service;
}

int32_t StreamDispatch(struct HdfDeviceIoClient *client, int cmdId,
                       struct HdfSBuf *data, struct HdfSBuf *reply);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */

#endif /* AUDIO_STREAM_DISP_H */
