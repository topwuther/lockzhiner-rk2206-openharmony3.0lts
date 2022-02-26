/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
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

#include "vibrator_controller.h"
#include <securec.h>
#include "hdf_base.h"
#include "hdf_log.h"
#include "vibrator_if.h"

#define HDF_LOG_TAG vibrator_controller_c
#define EFFECT_SUN 64
#define VIBRATOR_SERVICE_NAME    "hdf_misc_vibrator"

static struct VibratorDevice *GetVibratorDevicePriv(void)
{
    static struct VibratorDevice vibratorDeviceData = {
        .initState = false,
        .ioService = NULL,
    };

    return &vibratorDeviceData;
}

static int32_t StartOnce(uint32_t duration)
{
    struct VibratorDevice *priv = GetVibratorDevicePriv();

    if (duration == 0) {
        HDF_LOGE("%s:invalid duration para", __func__);
        return HDF_ERR_INVALID_PARAM;
    }

    (void)OsalMutexLock(&priv->mutex);
    struct HdfSBuf *msg = HdfSBufObtainDefaultSize();
    if (msg == NULL) {
        HDF_LOGE("%s: get sbuf failed", __func__);
        (void)OsalMutexUnlock(&priv->mutex);
        return HDF_FAILURE;
    }

    if (!HdfSbufWriteUint32(msg, duration)) {
        HDF_LOGE("%s: write duration failed", __func__);
        HdfSBufRecycle(msg);
        (void)OsalMutexUnlock(&priv->mutex);
        return HDF_FAILURE;
    }

    if (priv->ioService == NULL || priv->ioService->dispatcher == NULL ||
        priv->ioService->dispatcher->Dispatch == NULL) {
        HDF_LOGE("%s: para invalid", __func__);
        HdfSBufRecycle(msg);
        (void)OsalMutexUnlock(&priv->mutex);
        return HDF_FAILURE;
    }

    int32_t ret = priv->ioService->dispatcher->Dispatch(&priv->ioService->object, VIBRATOR_IO_START_ONCE, msg, NULL);
    if (ret != HDF_SUCCESS) {
        HDF_LOGE("%s: dispatcher duration failed", __func__);
        HdfSBufRecycle(msg);
        (void)OsalMutexUnlock(&priv->mutex);
        return ret;
    }

    HdfSBufRecycle(msg);
    (void)OsalMutexUnlock(&priv->mutex);

    return HDF_SUCCESS;
}

static int32_t CovertVibratorEffectName(char *effectName, int32_t len)
{
    for (int i = 0; i < len; i++) {
        if (effectName[i] == '.') {
            effectName[i] = '_';
        }
    }
    return HDF_SUCCESS;
}

static int32_t Start(const char *effect)
{
    struct VibratorDevice *priv = GetVibratorDevicePriv();
    char effectName[EFFECT_SUN];

    if (effect == NULL) {
        HDF_LOGE("%s: start vibrator effect type invalid", __func__);
        return HDF_ERR_INVALID_PARAM;
    }

    if (strcpy_s(&effectName[0], EFFECT_SUN, effect) != HDF_SUCCESS) {
        HDF_LOGE("%s: failed to copy string", __func__);
        return HDF_FAILURE;
    }

    int32_t effectRet = CovertVibratorEffectName(effectName, EFFECT_SUN);
    if (effectRet == HDF_FAILURE) {
        HDF_LOGE("%s: covert effectName failed", __func__);
        return HDF_FAILURE;
    }

    (void)OsalMutexLock(&priv->mutex);
    struct HdfSBuf *msg = HdfSBufObtainDefaultSize();
    if (msg == NULL) {
        HDF_LOGE("%s: get sbuf failed", __func__);
        HdfSBufRecycle(msg);
        (void)OsalMutexUnlock(&priv->mutex);
        return HDF_FAILURE;
    }

    if (priv->ioService == NULL || priv->ioService->dispatcher == NULL ||
        priv->ioService->dispatcher->Dispatch == NULL) {
        HDF_LOGE("%s: para invalid", __func__);
        HdfSBufRecycle(msg);
        (void)OsalMutexUnlock(&priv->mutex);
        return HDF_FAILURE;
    }

    if (!HdfSbufWriteString(msg, effectName)) {
        HDF_LOGE("%s: write effectName failed", __func__);
        HdfSBufRecycle(msg);
        (void)OsalMutexUnlock(&priv->mutex);
        return HDF_FAILURE;
    }

    int32_t ret = priv->ioService->dispatcher->Dispatch(&priv->ioService->object, VIBRATOR_IO_START_EFFECT, msg, NULL);
    if (ret != HDF_SUCCESS) {
        HDF_LOGE("%s: dispatcher effect failed", __func__);
        HdfSBufRecycle(msg);
        (void)OsalMutexUnlock(&priv->mutex);
        return ret;
    }

    HdfSBufRecycle(msg);
    (void)OsalMutexUnlock(&priv->mutex);

    return HDF_SUCCESS;
}

static int32_t Stop(enum VibratorMode mode)
{
    struct VibratorDevice *priv = GetVibratorDevicePriv();

    (void)OsalMutexLock(&priv->mutex);
    struct HdfSBuf *msg = HdfSBufObtainDefaultSize();
    if (msg == NULL) {
        HDF_LOGE("%s: get sbuf failed", __func__);
        (void)OsalMutexUnlock(&priv->mutex);
        return HDF_FAILURE;
    }

    if (priv->ioService == NULL || priv->ioService->dispatcher == NULL ||
        priv->ioService->dispatcher->Dispatch == NULL) {
        HDF_LOGE("%s: para invalid", __func__);
        HdfSBufRecycle(msg);
        (void)OsalMutexUnlock(&priv->mutex);
        return HDF_FAILURE;
    }

    if (!HdfSbufWriteInt32(msg, mode)) {
        HDF_LOGE("%s: write mode failed", __func__);
        HdfSBufRecycle(msg);
        (void)OsalMutexUnlock(&priv->mutex);
        return HDF_FAILURE;
    }

    int32_t ret = priv->ioService->dispatcher->Dispatch(&priv->ioService->object, VIBRATOR_IO_STOP, msg, NULL);
    if (ret != HDF_SUCCESS) {
        HDF_LOGE("%s: dispatcher stop failed", __func__);
        HdfSBufRecycle(msg);
        (void)OsalMutexUnlock(&priv->mutex);
        return ret;
    }

    HdfSBufRecycle(msg);
    (void)OsalMutexUnlock(&priv->mutex);

    return HDF_SUCCESS;
}

const struct VibratorInterface *NewVibratorInterfaceInstance(void)
{
    static struct VibratorInterface vibratorDevInstance;
    struct VibratorDevice *priv = GetVibratorDevicePriv();

    if (priv->initState) {
        return &vibratorDevInstance;
    }

    OsalMutexInit(&priv->mutex);
    vibratorDevInstance.Start = Start;
    vibratorDevInstance.StartOnce = StartOnce;
    vibratorDevInstance.Stop = Stop;

    priv->ioService = HdfIoServiceBind(VIBRATOR_SERVICE_NAME);
    if (priv->ioService == NULL) {
        HDF_LOGE("%s: get vibrator ioService failed", __func__);
        OsalMutexDestroy(&priv->mutex);
        return NULL;
    }

    priv->initState = true;
    HDF_LOGD("get vibrator devInstance success");
    return &vibratorDevInstance;
}

int32_t FreeVibratorInterfaceInstance(void)
{
    struct VibratorDevice *priv = GetVibratorDevicePriv();

    if (!priv->initState) {
        HDF_LOGD("%s: vibrator instance had released", __func__);
        return HDF_SUCCESS;
    }

    if (priv->ioService != NULL) {
        HdfIoServiceRecycle(priv->ioService);
    }

    OsalMutexDestroy(&priv->mutex);

    return HDF_SUCCESS;
}