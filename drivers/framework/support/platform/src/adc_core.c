/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
 *
 * HDF is dual licensed: you can use it either under the terms of
 * the GPL, or the BSD license, at your option.
 * See the LICENSE file in the root of this repository for complete details.
 */

#include "adc_core.h"
#include "hdf_device_desc.h"
#include "hdf_log.h"
#include "osal_mem.h"
#include "osal_spinlock.h"
#include "osal_time.h"
#include "platform_core.h"
#include "plat_log.h"

#define HDF_LOG_TAG adc_core_c
#define LOCK_WAIT_SECONDS_M 1
#define ADC_BUFF_SIZE 4

struct AdcManager {
    struct IDeviceIoService service;
    struct HdfDeviceObject *device;
    struct AdcDevice *devices[ADC_DEVICES_MAX];
    OsalSpinlock spin;
};

static struct AdcManager *g_adcManager = NULL;

static int32_t AdcDeviceLockDefault(struct AdcDevice *device)
{
    if (device == NULL) {
        return HDF_ERR_INVALID_OBJECT;
    }
    return OsalSpinLock(&device->spin);
}

static void AdcDeviceUnlockDefault(struct AdcDevice *device)
{
    if (device == NULL) {
        return;
    }
    (void)OsalSpinUnlock(&device->spin);
}

static const struct AdcLockMethod g_adcLockOpsDefault = {
    .lock = AdcDeviceLockDefault,
    .unlock = AdcDeviceUnlockDefault,
};

static int32_t AdcManagerAddDevice(struct AdcDevice *device)
{
    int32_t ret;
    struct AdcManager *manager = g_adcManager;

    if (device->devNum >= ADC_DEVICES_MAX) {
        HDF_LOGE("%s: devNum:%u exceed", __func__, device->devNum);
        return HDF_ERR_INVALID_OBJECT;
    }

    if (manager == NULL) {
        HDF_LOGE("%s: get adc manager fail", __func__);
        return HDF_ERR_NOT_SUPPORT;
    }

    if (OsalSpinLockIrq(&manager->spin) != HDF_SUCCESS) {
        HDF_LOGE("%s: lock adc manager fail", __func__);
        return HDF_ERR_DEVICE_BUSY;
    }

    if (manager->devices[device->devNum] != NULL) {
        HDF_LOGE("%s: adc device num:%u already exits", __func__, device->devNum);
        ret = HDF_FAILURE;
    } else {
        manager->devices[device->devNum] = device;
        ret = HDF_SUCCESS;
    }

    (void)OsalSpinUnlockIrq(&manager->spin);
    return ret;
}

static void AdcManagerRemoveDevice(struct AdcDevice *device)
{
    struct AdcManager *manager = g_adcManager;

    if (device->devNum < 0 || device->devNum >= ADC_DEVICES_MAX) {
        HDF_LOGE("%s: invalid devNum:%u", __func__, device->devNum);
        return;
    }

    if (manager == NULL) {
        HDF_LOGE("%s: get adc manager fail", __func__);
        return;
    }

    if (OsalSpinLockIrq(&manager->spin) != HDF_SUCCESS) {
        HDF_LOGE("%s: lock adc manager fail", __func__);
        return;
    }

    if (manager->devices[device->devNum] != device) {
        HDF_LOGE("%s: adc device(%u) not in manager", __func__, device->devNum);
    } else {
        manager->devices[device->devNum] = NULL;
    }

    (void)OsalSpinUnlockIrq(&manager->spin);
}

static struct AdcDevice *AdcManagerFindDevice(uint32_t number)
{
    struct AdcDevice *device = NULL;
    struct AdcManager *manager = g_adcManager;

    if (number < 0 || number >= ADC_DEVICES_MAX) {
        HDF_LOGE("%s: invalid devNum:%u", __func__, number);
        return NULL;
    }

    if (manager == NULL) {
        HDF_LOGE("%s: get adc manager fail", __func__);
        return NULL;
    }

    if (OsalSpinLockIrq(&manager->spin) != HDF_SUCCESS) {
        HDF_LOGE("%s: lock adc manager fail", __func__);
        return NULL;
    }

    device = manager->devices[number];
    (void)OsalSpinUnlockIrq(&manager->spin);

    return device;
}

int32_t AdcDeviceAdd(struct AdcDevice *device)
{
    int32_t ret;

    if (device == NULL) {
        return HDF_ERR_INVALID_OBJECT;
    }

    if (device->ops == NULL) {
        HDF_LOGE("%s: no ops supplied", __func__);
        return HDF_ERR_INVALID_OBJECT;
    }

    if (device->lockOps == NULL) {
        HDF_LOGI("%s: use default lockOps!", __func__);
        device->lockOps = &g_adcLockOpsDefault;
    }

    if (OsalSpinInit(&device->spin) != HDF_SUCCESS) {
        HDF_LOGE("%s: init lock failed", __func__);
        return HDF_FAILURE;
    }

    ret = AdcManagerAddDevice(device);
    if (ret != HDF_SUCCESS) {
        (void)OsalSpinDestroy(&device->spin);
    }
    return ret;
}

void AdcDeviceRemove(struct AdcDevice *device)
{
    if (device == NULL) {
        return;
    }
    AdcManagerRemoveDevice(device);
    (void)OsalSpinDestroy(&device->spin);
}

struct AdcDevice *AdcDeviceGet(uint32_t number)
{
    return AdcManagerFindDevice(number);
}

void AdcDevicePut(struct AdcDevice *device)
{
    (void)device;
}

static inline int32_t AdcDeviceLock(struct AdcDevice *device)
{
    if (device->lockOps == NULL || device->lockOps->lock == NULL) {
        return HDF_ERR_NOT_SUPPORT;
    }
    return device->lockOps->lock(device);
}

static inline void AdcDeviceUnlock(struct AdcDevice *device)
{
    if (device->lockOps != NULL && device->lockOps->unlock != NULL) {
        device->lockOps->unlock(device);
    }
}

int32_t AdcDeviceRead(struct AdcDevice *device, uint32_t channel, uint32_t *val)
{
    int32_t ret;

    if (device == NULL) {
        HDF_LOGE("%s: device is null", __func__);
        return HDF_ERR_INVALID_OBJECT;
    }

    if (device->ops == NULL || device->ops->read == NULL) {
        HDF_LOGE("%s: ops or read is null", __func__);
        return HDF_ERR_NOT_SUPPORT;
    }

    if (val == NULL) {
        HDF_LOGE("%s: invalid val pointer!", __func__);
        return HDF_ERR_INVALID_PARAM;
    }

    if (AdcDeviceLock(device) != HDF_SUCCESS) {
        HDF_LOGE("%s: lock add device failed", __func__);
        return HDF_ERR_DEVICE_BUSY;
    }

    ret = device->ops->read(device, channel, val);
    AdcDeviceUnlock(device);
    return ret;
}

int32_t AdcDeviceStart(struct AdcDevice *device)
{
    int32_t ret;

    if (device == NULL) {
        HDF_LOGE("%s: device is null", __func__);
        return HDF_ERR_INVALID_OBJECT;
    }

    if (device->ops == NULL || device->ops->start == NULL) {
        HDF_LOGE("%s: ops or start is null", __func__);
        return HDF_ERR_NOT_SUPPORT;
    }

    if (AdcDeviceLock(device) != HDF_SUCCESS) {
        HDF_LOGE("%s: lock add device failed", __func__);
        return HDF_ERR_DEVICE_BUSY;
    }

    ret = device->ops->start(device);
    AdcDeviceUnlock(device);
    return ret;
}

int32_t AdcDeviceStop(struct AdcDevice *device)
{
    int32_t ret;

    if (device == NULL) {
        HDF_LOGE("%s: device is null", __func__);
        return HDF_ERR_INVALID_OBJECT;
    }

    if (device->ops == NULL || device->ops->stop == NULL) {
        HDF_LOGE("%s: ops or stop is null", __func__);
        return HDF_ERR_NOT_SUPPORT;
    }

    if (AdcDeviceLock(device) != HDF_SUCCESS) {
        HDF_LOGE("%s: lock add device failed", __func__);
        return HDF_ERR_DEVICE_BUSY;
    }

    ret = device->ops->stop(device);
    AdcDeviceUnlock(device);
    return ret;
}

static int32_t AdcManagerIoOpen(struct HdfSBuf *data, struct HdfSBuf *reply)
{
    uint32_t number;

    if (!HdfSbufReadUint32(data, &number)) {
        return HDF_ERR_IO;
    }

    if (number < 0 || number >= ADC_DEVICES_MAX || reply == NULL) {
        return HDF_ERR_INVALID_PARAM;
    }

    if (AdcDeviceGet(number) == NULL) {
        return HDF_ERR_NOT_SUPPORT;
    }

    if (!HdfSbufWriteUint32(reply, number)) {
        return HDF_ERR_IO;
    }
    return HDF_SUCCESS;
}

static int32_t AdcManagerIoClose(struct HdfSBuf *data, struct HdfSBuf *reply)
{
    uint32_t number;

    if (!HdfSbufReadUint32(data, &number)) {
        return HDF_ERR_IO;
    }

    if (number < 0 || number >= ADC_DEVICES_MAX) {
        return HDF_ERR_INVALID_PARAM;
    }
    AdcDevicePut(AdcManagerFindDevice(number));
    return HDF_SUCCESS;
}

static int32_t AdcManagerIoRead(struct HdfSBuf *data, struct HdfSBuf *reply)
{
    (void)data;
    (void)reply;
    return HDF_SUCCESS;
}

static int32_t AdcManagerDispatch(struct HdfDeviceIoClient *client, int cmd,
    struct HdfSBuf *data, struct HdfSBuf *reply)
{
    int32_t ret;

    switch (cmd) {
        case ADC_IO_OPEN:
            return AdcManagerIoOpen(data, reply);
        case ADC_IO_CLOSE:
            return AdcManagerIoClose(data, reply);
        case ADC_IO_READ:
            return AdcManagerIoRead(data, reply);
        default:
            ret = HDF_ERR_NOT_SUPPORT;
            break;
    }
    return ret;
}

static int32_t AdcManagerInit(struct HdfDeviceObject *device)
{
    int32_t ret;
    struct AdcManager *manager = NULL;

    HDF_LOGI("%s: Enter", __func__);
    if (device == NULL) {
        HDF_LOGE("%s: device is null", __func__);
        return HDF_ERR_INVALID_OBJECT;
    }

    manager = (struct AdcManager *)OsalMemCalloc(sizeof(*manager));
    if (manager == NULL) {
        HDF_LOGE("%s: alloc manager failed", __func__);
        return HDF_ERR_MALLOC_FAIL;
    }

    ret = OsalSpinInit(&manager->spin);
    if (ret != HDF_SUCCESS) {
        HDF_LOGE("%s: spinlock init failed", __func__);
        OsalMemFree(manager);
        return HDF_FAILURE;
    }

    manager->device = device;
    g_adcManager = manager;
    device->service = &manager->service;
    device->service->Dispatch = AdcManagerDispatch;
    return HDF_SUCCESS;
}

static void AdcManagerRelease(struct HdfDeviceObject *device)
{
    struct AdcManager *manager = NULL;

    HDF_LOGI("%s: Enter", __func__);
    if (device == NULL) {
        HDF_LOGE("%s: device is null", __func__);
        return;
    }

    manager = (struct AdcManager *)device->service;
    if (manager == NULL) {
        HDF_LOGI("%s: no service bind", __func__);
        return;
    }

    g_adcManager = NULL;
    OsalMemFree(manager);
}

struct HdfDriverEntry g_adcManagerEntry = {
    .moduleVersion = 1,
    .Init = AdcManagerInit,
    .Release = AdcManagerRelease,
    .moduleName = "HDF_PLATFORM_ADC_MANAGER",
};
HDF_INIT(g_adcManagerEntry);
