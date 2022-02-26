/*
 * Copyright (c) 2020-2021 Huawei Device Co., Ltd.
 *
 * HDF is dual licensed: you can use it either under the terms of
 * the GPL, or the BSD license, at your option.
 * See the LICENSE file in the root of this repository for complete details.
 */

#include "hdf_log.h"
#include "osal_mem.h"
#include "platform_core.h"
#include "platform_device.h"
#include "platform_manager.h"

#define PLATFORM_DEV_NAME_DEFAULT "platform_device"

static void PlatformDeviceOnFirstGet(struct HdfSRef *sref)
{
    (void)sref;
}

static void PlatformDeviceOnLastPut(struct HdfSRef *sref)
{
    struct PlatformDevice *device = NULL;
    struct PlatformNotifierNode *pos = NULL;
    struct PlatformNotifierNode *tmp = NULL;

    if (sref == NULL) {
        return;
    }

    device = CONTAINER_OF(sref, struct PlatformDevice, ref);
    if (device == NULL) {
        HDF_LOGE("PlatformDeviceOnLastPut: get device is NULL!");
        return;
    }
    (void)OsalSpinLock(&device->spin);
    device->ready = false;

    DLIST_FOR_EACH_ENTRY_SAFE(pos, tmp, &device->notifiers, struct PlatformNotifierNode, node) {
        if (pos->notifier != NULL && pos->notifier->handle != NULL) {
            pos->notifier->handle(device, PLAT_EVENT_DEAD, pos->notifier->data);
        }
    }

    (void)OsalSpinUnlock(&device->spin);
}

struct IHdfSRefListener g_platObjListener = {
    .OnFirstAcquire = PlatformDeviceOnFirstGet,
    .OnLastRelease = PlatformDeviceOnLastPut,
};

static void PlatformEventHandle(struct PlatformDevice *device, enum PlatformEventType event, void *data)
{
    if (device == NULL) {
        return;
    }
    if (event == PLAT_EVENT_DEAD) {
        (void)OsalSemPost(&device->released);
    }
}

static struct PlatformNotifier g_platformEventNotifier = {
    .data = NULL,
    .handle = PlatformEventHandle,
};

void PlatformDeviceInit(struct PlatformDevice *device)
{
    if (device == NULL) {
        HDF_LOGW("PlatformDeviceInit: device is NULL");
        return;
    }

    if (device->name == NULL) {
        device->name = PLATFORM_DEV_NAME_DEFAULT;
    }

    (void)OsalSpinInit(&device->spin);
    (void)OsalSemInit(&device->released, 0);
    DListHeadInit(&device->node);
    DListHeadInit(&device->notifiers);
    HdfSRefConstruct(&device->ref, &g_platObjListener);

    if (PlatformDeviceRegNotifier(device, &g_platformEventNotifier) != HDF_SUCCESS) {
        HDF_LOGE("PlatformDeviceInit: reg notifier fail");
    }
    device->ready = true;
}

void PlatformDeviceUninit(struct PlatformDevice *device)
{
    if (device == NULL) {
        HDF_LOGW("PlatformDevice: device is NULL");
        return;
    }
    device->ready = false;

    /* make sure no reference anymore before exit. */
    (void)OsalSemWait(&device->released, HDF_WAIT_FOREVER);
    PlatformDeviceClearNotifier(device);
    (void)OsalSemDestroy(&device->released);
    (void)OsalSpinDestroy(&device->spin);
}

struct PlatformDevice *PlatformDeviceGet(struct PlatformDevice *device)
{
    if (device == NULL) {
        HDF_LOGE("PlatformDeviceGet: device is NULL");
        return NULL;
    }

    HdfSRefAcquire(&device->ref);
    return device;
}

void PlatformDevicePut(struct PlatformDevice *device)
{
    if (device != NULL) {
        HdfSRefRelease(&device->ref);
    }
}

int32_t PlatformDeviceRegNotifier(struct PlatformDevice *device, struct PlatformNotifier *notifier)
{
    struct PlatformNotifierNode *pNode = NULL;

    if (device == NULL || notifier == NULL) {
        return HDF_ERR_INVALID_OBJECT;
    }
    pNode = (struct PlatformNotifierNode *)OsalMemCalloc(sizeof(*pNode));
    if (pNode == NULL) {
        HDF_LOGE("PlatformDeviceRegNotifier: alloc pNode fail");
        return HDF_ERR_MALLOC_FAIL;
    }
    pNode->notifier = notifier;
    (void)OsalSpinLock(&device->spin);
    DListInsertTail(&pNode->node, &device->notifiers);
    (void)OsalSpinUnlock(&device->spin);
    return HDF_SUCCESS;
}

static void PlatformDeviceRemoveNotifier(struct PlatformDevice *device, struct PlatformNotifier *notifier)
{
    struct PlatformNotifierNode *pos = NULL;
    struct PlatformNotifierNode *tmp = NULL;

    if (device->notifiers.next == NULL || device->notifiers.prev == NULL) {
        HDF_LOGD("PlatformDeviceRemoveNotifier: notifiers not init.");
        return;
    }

    (void)OsalSpinLock(&device->spin);
    DLIST_FOR_EACH_ENTRY_SAFE(pos, tmp, &device->notifiers, struct PlatformNotifierNode, node) {
        if (pos->notifier != NULL) {
            /* if notifier not set, we remove all the notifier nodes. */
            if (notifier == pos->notifier) {
                DListRemove(&pos->node);
                OsalMemFree(pos);
                break; 
            } else if (notifier == NULL) {
                DListRemove(&pos->node);
                OsalMemFree(pos);
            }
        }
    }
    (void)OsalSpinUnlock(&device->spin);
}

void PlatformDeviceUnregNotifier(struct PlatformDevice *device, struct PlatformNotifier *notifier)
{
    if (device == NULL || notifier == NULL) {
        return;
    }
    PlatformDeviceRemoveNotifier(device, notifier);
}

void PlatformDeviceClearNotifier(struct PlatformDevice *device)
{
    if (device == NULL) {
        return;
    }
    PlatformDeviceRemoveNotifier(device, NULL);
}

int32_t PlatformDeviceAdd(struct PlatformDevice *device)
{
    struct PlatformManager *manager = NULL;

    if (device == NULL) {
        return HDF_ERR_INVALID_OBJECT;
    }

    PlatformDeviceInit(device);
    manager = device->manager;
    if (manager == NULL) {
        manager = PlatformManagerGet(PLATFORM_MODULE_DEFAULT);
    }
    return PlatformManagerAddDevice(manager, device);
}

void PlatformDeviceDel(struct PlatformDevice *device)
{
    struct PlatformManager *manager = NULL;

    if (device == NULL) {
        return;
    }

    manager = device->manager;
    if (manager == NULL) {
        manager = PlatformManagerGet(PLATFORM_MODULE_DEFAULT);
    }
    PlatformManagerDelDevice(manager, device);
    PlatformDeviceUninit(device);
}
