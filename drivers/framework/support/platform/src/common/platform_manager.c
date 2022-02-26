/*
 * Copyright (c) 2020-2021 Huawei Device Co., Ltd.
 *
 * HDF is dual licensed: you can use it either under the terms of
 * the GPL, or the BSD license, at your option.
 * See the LICENSE file in the root of this repository for complete details.
 */

#include "hdf_log.h"
#include "osal_mem.h"
#include "osal_sem.h"
#include "platform_core.h"
#include "platform_device.h"
#include "platform_manager.h"

#define PLATFORM_MANAGER_NAME_DEFAULT "PlatformManagerDefault"

static void PlatformManagerInit(struct PlatformManager *manager)
{
    if (manager->name == NULL) {
        manager->name = PLATFORM_MANAGER_NAME_DEFAULT;
    }

    OsalSpinInit(&manager->spin);
    DListHeadInit(&manager->devices);
}

struct PlatformManager *PlatformManagerCreate(const char *name)
{
    struct PlatformManager *manager = NULL;

    manager = (struct PlatformManager *)OsalMemCalloc(sizeof(*manager));
    if (manager == NULL) {
        HDF_LOGE("PlatformManagerCreate: malloc fail!");
        return NULL;
    }
    manager->name = name;
    PlatformManagerInit(manager);
    return manager;
}

struct PlatformManager *PlatformManagerGet(enum PlatformModuleType module)
{
    struct PlatformManager *manager = NULL;
    struct PlatformModuleInfo *info = NULL;

    info = PlatformModuleInfoGet(module);
    if (info == NULL) {
        HDF_LOGE("PlatformManagerGet: get module(%d) info failed", module);
        return NULL;
    }

    PlatformGlobalLock();
    if (info->priv == NULL) {
        manager = PlatformManagerCreate(info->moduleName);
        info->priv = manager;
    } else {
        manager = (struct PlatformManager *)info->priv;
    }
    PlatformGlobalUnlock();

    return manager;
}

int32_t PlatformManagerAddDevice(struct PlatformManager *manager, struct PlatformDevice *device)
{
    struct PlatformDevice *tmp = NULL;
    bool repeatId = false;

    if (manager == NULL || device == NULL) {
        return HDF_ERR_INVALID_OBJECT;
    }
    device->manager = manager;

    if (PlatformDeviceGet(device) == NULL) { // keep a reference by manager
        return HDF_PLT_ERR_DEV_GET;
    }

    (void)OsalSpinLock(&manager->spin);
    DLIST_FOR_EACH_ENTRY(tmp, &manager->devices, struct PlatformDevice, node) {
        if (tmp != NULL && tmp->magic == device->magic) {
            repeatId = true;
            HDF_LOGE("PlatformManagerAddDevice: repeated magic:%u!", device->magic);
            break;
        }
    }
    if (!repeatId) {
        DListInsertTail(&device->node, &manager->devices);
    }
    (void)OsalSpinUnlock(&manager->spin);

    if (repeatId) {
        PlatformDevicePut(device);
    }
    return repeatId ? HDF_PLT_ERR_ID_REPEAT : HDF_SUCCESS;
}

void PlatformManagerDelDevice(struct PlatformManager *manager, struct PlatformDevice *device)
{
    if (manager == NULL || device == NULL) {
        return;
    }

    if (device->manager != manager) {
        HDF_LOGE("PlatformManagerDelDevice: manager mismatch!");
        return;
    }

    (void)OsalSpinLock(&manager->spin);
    if (!DListIsEmpty(&device->node)) {
        DListRemove(&device->node);
    }
    (void)OsalSpinUnlock(&manager->spin);
    PlatformDevicePut(device);  // put the reference hold by manager
}

struct PlatformDevice *PlatformManagerFindDevice(struct PlatformManager *manager, void *data,
    bool (*match)(struct PlatformDevice *pdevice, void *data))
{
    struct PlatformDevice *tmp = NULL;
    struct PlatformDevice *pdevice = NULL;

    if (manager == NULL || match == NULL) {
        return NULL;
    }
    if (manager->devices.prev == NULL || manager->devices.next == NULL) {
        HDF_LOGD("PlatformManagerFindDevice: devices not init.");
        return NULL;
    }

    (void)OsalSpinLock(&manager->spin);
    DLIST_FOR_EACH_ENTRY(tmp, &manager->devices, struct PlatformDevice, node) {
        if (tmp != NULL && match(tmp, data)) {
            pdevice = PlatformDeviceGet(tmp);
        }
    }
    (void)OsalSpinUnlock(&manager->spin);

    return pdevice;
}

static bool PlatformDeviceMatchByMagic(struct PlatformDevice *device, void *data)
{
    uint32_t magic = (uint32_t)(uintptr_t)data;

    return (device != NULL && device->magic == magic);
}

struct PlatformDevice *PlatformManagerGetDeviceByMagic(struct PlatformManager *manager, uint32_t magic)
{
    if (manager == NULL) {
        return NULL;
    }
    return PlatformManagerFindDevice(manager, (void *)(uintptr_t)magic, PlatformDeviceMatchByMagic);
}
