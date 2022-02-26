/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
 *
 * HDF is dual licensed: you can use it either under the terms of
 * the GPL, or the BSD license, at your option.
 * See the LICENSE file in the root of this repository for complete details.
 */

#ifndef AUDIO_CONTROL_DISPATCH_H
#define AUDIO_CONTROL_DISPATCH_H

#include "audio_host.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* __cplusplus */


#define AUDIODRV_CTRL_ELEM_TYPE_INTEGER 2 /* integer type */

enum ControlDispMethodCmd {
    AUDIODRV_CTRL_IOCTRL_ELEM_INFO,
    AUDIODRV_CTRL_IOCTRL_ELEM_READ,
    AUDIODRV_CTRL_IOCTRL_ELEM_WRITE,
    AUDIODRV_CTRL_IOCTRL_ELEM_BUTT,
};

typedef int32_t (*ControlDispCmdHandle)(const struct HdfDeviceIoClient *client,
    struct HdfSBuf *data, struct HdfSBuf *reply);

struct ControlDispCmdHandleList {
    enum ControlDispMethodCmd cmd;
    ControlDispCmdHandle func;
};

struct ControlHost {
    struct IDeviceIoService service;
    struct HdfDeviceObject *device;
    void *priv;
};

static inline struct ControlHost *ControlHostFromDevice(struct HdfDeviceObject *device)
{
    return (device == NULL) ? NULL : (struct ControlHost *)device->service;
}

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */

#endif /* AUDIO_CONTROL_H */
