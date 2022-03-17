/*
 * Copyright (c) 2022 FuZhou Lockzhiner Electronic Co., Ltd. All rights reserved.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <stdint.h>
#include "hdf_device_desc.h"
#include "hdf_log.h"
#include "device_resource_if.h"
#include "osal.h"
#include "pwm_core.h"
#include "pwm_if.h"

#include "lz_hardware.h"

#define HDF_LOG_TAG     pwm_driver_rk2206

typedef enum enum_pwm_port_id {
    PWM_PORT_ID_0 = 0,
    PWM_PORT_ID_1 = 1,
    PWM_PORT_ID_7 = 7,
    PWM_PORT_ID_MAX
}PWM_PORT_ID_E;

typedef struct tagHcsInfo {
    uint32_t num;
    uint32_t portId;
    uint32_t gpio;
    uint32_t duty;
    uint32_t cycle;
} hcs_info_s;

typedef struct tagPwmInfo {
    struct PwmDev dev;
    hcs_info_s hcs_info;
    bool supportPolarity;
}pwm_info_s;


////////////////////////////////////////////////////////////////////


static int32_t HdfPwmDriverSetConfig(struct PwmDev *pwm, struct PwmConfig *config)
{
    pwm_info_s *ppwm_info = NULL;
    hcs_info_s *phcs_info = NULL;
    
    if (pwm == NULL)
    {
        HDF_LOGE("%s, %d: pwm is null", __func__, __LINE__);
        return HDF_ERR_INVALID_PARAM;
    }
    if (config == NULL)
    {
        HDF_LOGE("%s, %d: config is null", __func__, __LINE__);
        return HDF_ERR_INVALID_PARAM;
    }

    ppwm_info = (pwm_info_s *)PwmGetPriv(pwm);
    if (ppwm_info == NULL)
    {
        HDF_LOGE("%s, %d: ppwm_info is null", __func__, __LINE__);
        return HDF_ERR_INVALID_PARAM;
    }

    phcs_info = (hcs_info_s *)&ppwm_info->hcs_info;

    if ((pwm->cfg.polarity != config->polarity) && !(ppwm_info->supportPolarity))
    {
        HDF_LOGE("%s, %d: not support set pwm polartiy", __func__, __LINE__);
        return HDF_ERR_NOT_SUPPORT;
    }

    if (config->status == PWM_DISABLE_STATUS)
    {
        LzPwmStop(phcs_info->num);
        return HDF_SUCCESS;
    }

    if ((config->polarity != PWM_NORMAL_POLARITY) && (config->polarity != PWM_INVERTED_POLARITY))
    {
        HDF_LOGE("%s, %d: polarity %u is invalid", __func__, __LINE__, config->polarity);
        return HDF_ERR_INVALID_PARAM;
    }

    if ((config->duty < 1) || (config->duty > config->period))
    {
        HDF_LOGE("%s, %d: duty %u is not support, min dutyCycle 1 max dutyCycle", __func__, __LINE__, config->duty);
        return HDF_ERR_INVALID_PARAM;
    }

    LzPwmStop(phcs_info->num);
    if ((pwm->cfg.polarity != config->polarity) && (ppwm_info->supportPolarity))
    {
        /* no thing to do */
    }

    phcs_info->duty = pwm->cfg.duty;
    phcs_info->cycle = pwm->cfg.period;
    LzPwmStart(phcs_info->portId, phcs_info->duty, phcs_info->cycle);
    
    return HDF_SUCCESS;
}

static int32_t HdfPwmDriverOpen(struct PwmDev *pwm)
{
    pwm_info_s *ppwm_info = NULL;

    if (pwm == NULL)
    {
        HDF_LOGE("%s, %d: pwm is null", __func__, __LINE__);
        return HDF_ERR_INVALID_PARAM;
    }

    ppwm_info = (pwm_info_s *)PwmGetPriv(pwm);
    if (ppwm_info == NULL)
    {
        HDF_LOGE("%s, %d: ppwm_info is null", __func__, __LINE__);
        return HDF_ERR_INVALID_PARAM;
    }

    switch (ppwm_info->hcs_info.portId)
    {
        case PWM_PORT_ID_0:
            {
                PwmBusIo pwm_bus = {
                    .pwm = {
                        .gpio = ppwm_info->hcs_info.gpio,
                        .func = MUX_FUNC1,
                        .type = PULL_DOWN,
                        .drv = DRIVE_KEEP,
                        .dir = LZGPIO_DIR_KEEP,
                        .val = LZGPIO_LEVEL_KEEP
                    },
                    .id = FUNC_ID_PWM0,
                    .mode = FUNC_MODE_NONE,
                };

                PwmIoInit(pwm_bus);
                LzPwmInit(ppwm_info->hcs_info.portId);
            }
            break;
        case PWM_PORT_ID_1:
            {
                PwmBusIo pwm_bus = {
                    .pwm = {
                        .gpio = ppwm_info->hcs_info.gpio,
                        .func = MUX_FUNC1,
                        .type = PULL_DOWN,
                        .drv = DRIVE_KEEP,
                        .dir = LZGPIO_DIR_KEEP,
                        .val = LZGPIO_LEVEL_KEEP
                    },
                    .id = FUNC_ID_PWM1,
                    .mode = FUNC_MODE_NONE,
                };

                PwmIoInit(pwm_bus);
                LzPwmInit(ppwm_info->hcs_info.portId);
            }
            break;
        case PWM_PORT_ID_7:
            {
                PwmBusIo pwm_bus = {
                    .pwm = {
                        .gpio = ppwm_info->hcs_info.gpio,
                        .func = MUX_FUNC1,
                        .type = PULL_DOWN,
                        .drv = DRIVE_KEEP,
                        .dir = LZGPIO_DIR_KEEP,
                        .val = LZGPIO_LEVEL_KEEP
                    },
                    .id = FUNC_ID_PWM7,
                    .mode = FUNC_MODE_NONE,
                };

                PwmIoInit(pwm_bus);
                LzPwmInit(ppwm_info->hcs_info.portId);
            }
            break;
        default:
            HDF_LOGE("%s, %d: pwm portId %u is error", __func__, __LINE__, ppwm_info->hcs_info.portId);
        return HDF_ERR_INVALID_PARAM;
    }
    
    return HDF_SUCCESS;
}

static int32_t HdfPwmDriverClose(struct PwmDev *pwm)
{
    pwm_info_s *ppwm_info = NULL;

    if (pwm == NULL)
    {
        HDF_LOGE("%s, %d: pwm is null", __func__, __LINE__);
        return HDF_ERR_INVALID_PARAM;
    }

    ppwm_info = (pwm_info_s *)PwmGetPriv(pwm);
    if (ppwm_info == NULL)
    {
        HDF_LOGE("%s, %d: ppwm_info is null", __func__, __LINE__);
        return HDF_ERR_INVALID_PARAM;
    }

    switch (ppwm_info->hcs_info.portId)
    {
        case PWM_PORT_ID_0:
        case PWM_PORT_ID_1:
        case PWM_PORT_ID_7:
            LzPwmDeinit(ppwm_info->hcs_info.portId);
            break;
        default:
            HDF_LOGE("%s, %d: pwm portId %u is error", __func__, __LINE__, ppwm_info->hcs_info.portId);
        return HDF_ERR_INVALID_PARAM;
    }
    return HDF_SUCCESS;
}

struct PwmMethod g_pwmMethod = {
    .setConfig = HdfPwmDriverSetConfig,
    .open = HdfPwmDriverOpen,
    .close = HdfPwmDriverClose,
};

static void HdfPwmDriverRemove(pwm_info_s *ppwm_info)
{
    OsalMemFree(ppwm_info);
}

static int32_t HdfPwmDriverProbe(pwm_info_s *ppwm_info, struct HdfDeviceObject *device)
{
    uint32_t tmp;
    struct DeviceResourceIface *iface = NULL;
    hcs_info_s *phcs_info = (hcs_info_s *)&ppwm_info->hcs_info;

    iface = DeviceResourceGetIfaceInstance(HDF_CONFIG_SOURCE);
    if ((iface == NULL) || (iface->GetUint32 == NULL))
    {
        HDF_LOGE("%s: face is invalid", __func__);
        return HDF_FAILURE;
    }

    if (iface->GetUint32(device->property, "num", &phcs_info->num, 0) != HDF_SUCCESS)
    {
        HDF_LOGE("%s, %d: hcs read num failed!", __func__, __LINE__);
        return HDF_FAILURE;
    }
    ppwm_info->dev.num = phcs_info->num;

    if (iface->GetUint32(device->property, "portId", &phcs_info->portId, 0) != HDF_SUCCESS)
    {
        HDF_LOGE("%s, %d: hcs read portId failed!", __func__, __LINE__);
        return HDF_FAILURE;
    }

    if (iface->GetUint32(device->property, "gpio", &phcs_info->gpio, 0) != HDF_SUCCESS)
    {
        HDF_LOGE("%s, %d: hcs gpio portId failed!", __func__, __LINE__);
        return HDF_FAILURE;
    }

    if (iface->GetUint32(device->property, "duty", &phcs_info->duty, 0) != HDF_SUCCESS)
    {
        HDF_LOGE("%s, %d: hcs duty portId failed!", __func__, __LINE__);
        return HDF_FAILURE;
    }

    if (iface->GetUint32(device->property, "cycle", &phcs_info->cycle, 0) != HDF_SUCCESS)
    {
        HDF_LOGE("%s, %d: hcs cycle portId failed!", __func__, __LINE__);
        return HDF_FAILURE;
    }

    ppwm_info->supportPolarity = false;
    ppwm_info->dev.method = &g_pwmMethod;
    ppwm_info->dev.cfg.duty = phcs_info->duty;
    ppwm_info->dev.cfg.period = phcs_info->cycle;
    ppwm_info->dev.cfg.polarity = PWM_NORMAL_POLARITY;
    ppwm_info->dev.cfg.status = PWM_DISABLE_STATUS;
    ppwm_info->dev.cfg.number = phcs_info->num;
    ppwm_info->dev.busy = false;
    PwmSetPriv(&ppwm_info->dev, ppwm_info);
    
    if (PwmDeviceAdd(device, &ppwm_info->dev) != HDF_SUCCESS)
    {
        HDF_LOGE("%s, %d: PwmDeviceAdd failed!", __func__, __LINE__);
        return HDF_FAILURE;
    }

    return HDF_SUCCESS;
}


static int32_t HdfPwmDriverBind(struct HdfDeviceObject *device)
{
    (void)device;
    return HDF_SUCCESS;
}

static int32_t HdfPwmDriverInit(struct HdfDeviceObject *device)
{
    int32_t ret;
    pwm_info_s *ppwm_info = NULL;

    printf("%s, %d: Hdf Pwm Driver Entry\n", __FILE__, __LINE__);
    HDF_LOGI("%s: entry", __func__);
    if (device == NULL)
    {
        HDF_LOGE("%s: device is null", __func__);
        return HDF_ERR_INVALID_OBJECT;
    }

    ppwm_info = (pwm_info_s *)OsalMemCalloc(sizeof(pwm_info_s));
    if (ppwm_info == NULL)
    {
        HDF_LOGE("%s: ppwm_info OsalMemCalloc error", __func__);
        return HDF_ERR_MALLOC_FAIL;
    }

    ret = HdfPwmDriverProbe(ppwm_info, device);
    if (ret != HDF_SUCCESS)
    {
        HDF_LOGE("%s: HdfPWmDriverProbe error, ret is %d", __func__, ret);
        return ret;
    }

    return HDF_SUCCESS;
}

static void HdfPwmDriverRelease(struct HdfDeviceObject *device)
{
    pwm_info_s *ppwm_info = NULL;

    HDF_LOGI("%s: entry", __func__);
    if (device == NULL)
    {
        HDF_LOGE("%s, %d: device is null", __func__, __LINE__);
        return;
    }

    ppwm_info = (pwm_info_s *)device->service;
    if (ppwm_info == NULL)
    {
        HDF_LOGE("%s, %d: ppwm_info is null", __func__, __LINE__);
        return;
    }

    PwmDeviceRemove(device, &ppwm_info->dev);
    HdfPwmDriverRemove(ppwm_info);
}

struct HdfDriverEntry g_pwmDriverEntry = {
    .moduleVersion = 1,
    .moduleName = "pwm_driver_rk2206",
    .Bind = HdfPwmDriverBind,
    .Init = HdfPwmDriverInit,
    .Release = HdfPwmDriverRelease,
};

HDF_INIT(g_pwmDriverEntry);
