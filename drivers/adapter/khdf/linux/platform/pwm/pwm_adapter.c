/*
 * pwm_adapter.c
 *
 * pwm driver adapter of linux
 *
 * Copyright (c) 2020-2021 Huawei Device Co., Ltd.
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 */

#include <linux/pwm.h>
#include "device_resource_if.h"
#include "hdf_base.h"
#include "hdf_log.h"
#include "osal_io.h"
#include "osal_mem.h"
#include "pwm_core.h"
#include "pwm_if.h"

#define HDF_LOG_TAG HDF_PWM_LINUX_ADAPTER

int32_t HdfPwmOpen(struct PwmDev *pwm)
{
    struct pwm_device *device = NULL;

    if (pwm == NULL) {
        HDF_LOGE("%s: pwm is null", __func__);
        return HDF_ERR_INVALID_PARAM;
    }
    device = pwm_request(pwm->num, NULL);
    if (IS_ERR(device)) {
        HDF_LOGE("%s: pwm_request pwm%d fail", __func__, pwm->num);
        return HDF_FAILURE;
    }
    pwm->cfg.period = device->state.period;
    pwm->cfg.duty = device->state.duty_cycle;
    pwm->cfg.polarity = device->state.polarity;
    pwm->cfg.status = device->state.enabled ? PWM_ENABLE_STATUS : PWM_DISABLE_STATUS;
    pwm->priv = device;
    return HDF_SUCCESS;
}

int32_t HdfPwmClose(struct PwmDev *pwm)
{
    if (pwm == NULL) {
        HDF_LOGE("%s: pwm is null", __func__);
        return HDF_ERR_INVALID_PARAM;
    }
    pwm_free((struct pwm_device *)pwm->priv);
    return HDF_SUCCESS;
}

int32_t HdfPwmSetConfig(struct PwmDev *pwm, struct PwmConfig *config)
{
    struct pwm_state state;

    if (pwm == NULL || pwm->priv == NULL || config == NULL) {
        HDF_LOGE("%s: hp reg or config is null", __func__);
        return HDF_ERR_INVALID_PARAM;
    }

    state.duty_cycle = config->duty;
    state.enabled = (config->status == PWM_ENABLE_STATUS) ? true : false;
    state.period = config->period;
    state.polarity = config->polarity;
    return pwm_apply_state(pwm->priv, &state);
}

struct PwmMethod g_pwmOps = {
    .setConfig = HdfPwmSetConfig,
    .open = HdfPwmOpen,
    .close = HdfPwmClose,
};

static int32_t HdfPwmBind(struct HdfDeviceObject *obj)
{
    (void)obj;
    return HDF_SUCCESS;
}

static int32_t HdfPwmInit(struct HdfDeviceObject *obj)
{
    int32_t ret;
    uint32_t num;
    struct PwmDev *pwm = NULL;
    struct DeviceResourceIface *iface = NULL;

    HDF_LOGI("%s: entry", __func__);
    if (obj == NULL) {
        HDF_LOGE("%s: obj is null", __func__);
        return HDF_ERR_INVALID_OBJECT;
    }
    iface = DeviceResourceGetIfaceInstance(HDF_CONFIG_SOURCE);
    if (iface == NULL || iface->GetUint32 == NULL) {
        HDF_LOGE("%s: face is invalid", __func__);
        return HDF_FAILURE;
    }
    if (iface->GetUint32(obj->property, "num", &num, 0) != HDF_SUCCESS) {
        HDF_LOGE("%s: read num fail", __func__);
        return HDF_FAILURE;
    }
    pwm = (struct PwmDev *)OsalMemCalloc(sizeof(*pwm));
    if (pwm == NULL) {
        HDF_LOGE("%s: OsalMemCalloc pwm error", __func__);
        return HDF_ERR_MALLOC_FAIL;
    }
    pwm->cfg.number = 0;
    pwm->num = num;
    pwm->method = &g_pwmOps;
    pwm->busy = false;
    ret = PwmDeviceAdd(obj, pwm);
    if (ret != HDF_SUCCESS) {
        HDF_LOGE("%s: error probe, ret is %d", __func__, ret);
        OsalMemFree(pwm);
    }
    return ret;
}

static void HdfPwmRelease(struct HdfDeviceObject *obj)
{
    struct PwmDev *pwm = NULL;

    HDF_LOGI("%s: entry", __func__);
    if (obj == NULL) {
        HDF_LOGE("%s: obj is null", __func__);
        return;
    }
    pwm = (struct PwmDev *)obj->service;
    if (pwm == NULL) {
        HDF_LOGE("%s: pwm is null", __func__);
        return;
    }
    PwmDeviceRemove(obj, pwm);
    OsalMemFree(pwm);
}

struct HdfDriverEntry g_hdfPwm = {
    .moduleVersion = 1,
    .moduleName = "HDF_PLATFORM_PWM",
    .Bind = HdfPwmBind,
    .Init = HdfPwmInit,
    .Release = HdfPwmRelease,
};

HDF_INIT(g_hdfPwm);
