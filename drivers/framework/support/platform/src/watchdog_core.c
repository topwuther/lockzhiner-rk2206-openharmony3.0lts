/*
 * Copyright (c) 2020-2021 Huawei Device Co., Ltd.
 *
 * HDF is dual licensed: you can use it either under the terms of
 * the GPL, or the BSD license, at your option.
 * See the LICENSE file in the root of this repository for complete details.
 */

#include "watchdog_core.h"
#include "hdf_log.h"
#include "osal_mem.h"

#define HDF_LOG_TAG watchdog_core

int32_t WatchdogCntlrAdd(struct WatchdogCntlr *cntlr)
{
    int32_t ret;

    if (cntlr == NULL) {
        return HDF_ERR_INVALID_OBJECT;
    }

    if (cntlr->device == NULL) {
        HDF_LOGE("WatchdogCntlrAdd: no device associated!");
        return HDF_ERR_INVALID_OBJECT;
    }

    if (cntlr->ops == NULL) {
        HDF_LOGE("WatchdogCntlrAdd: no ops supplied!");
        return HDF_ERR_INVALID_OBJECT;
    }

    ret = OsalSpinInit(&cntlr->lock);
    if (ret != HDF_SUCCESS) {
        HDF_LOGE("WatchdogCntlrAdd: spinlock init fail!");
        return ret;
    }

    cntlr->device->service = &cntlr->service;
    return HDF_SUCCESS;
}

void WatchdogCntlrRemove(struct WatchdogCntlr *cntlr)
{
    if (cntlr == NULL) {
        return;
    }

    if (cntlr->device == NULL) {
        HDF_LOGE("WatchdogCntlrRemove: no device associated!");
        return;
    }

    cntlr->device->service = NULL;
    (void)OsalSpinDestroy(&cntlr->lock);
}

void WatchdogGetPrivData(struct WatchdogCntlr *cntlr)
{
    if (cntlr == NULL || cntlr->ops == NULL) {
        return;
    }
    if (cntlr->ops->getPriv != NULL) {
        cntlr->ops->getPriv(cntlr);
    }
}

void WatchdogReleasePriv(struct WatchdogCntlr *cntlr)
{
    if (cntlr == NULL || cntlr->ops == NULL) {
        return;
    }
    if (cntlr->ops->releasePriv != NULL) {
        cntlr->ops->releasePriv(cntlr);
    }
}

int32_t WatchdogCntlrGetStatus(struct WatchdogCntlr *cntlr, int32_t *status)
{
    int32_t ret;
    uint32_t flags;

    if (cntlr == NULL) {
        return HDF_ERR_INVALID_OBJECT;
    }
    if (cntlr->ops == NULL || cntlr->ops->getStatus == NULL) {
        return HDF_ERR_NOT_SUPPORT;
    }
    if (status == NULL) {
        return HDF_ERR_INVALID_PARAM;
    }

    if (OsalSpinLockIrqSave(&cntlr->lock, &flags) != HDF_SUCCESS) {
        return HDF_ERR_DEVICE_BUSY;
    }
    ret = cntlr->ops->getStatus(cntlr, status);
    (void)OsalSpinUnlockIrqRestore(&cntlr->lock, &flags);
    return ret;
}

int32_t WatchdogCntlrStart(struct WatchdogCntlr *cntlr)
{
    int32_t ret;
    uint32_t flags;

    if (cntlr == NULL) {
        return HDF_ERR_INVALID_OBJECT;
    }
    if (cntlr->ops == NULL || cntlr->ops->start == NULL) {
        return HDF_ERR_NOT_SUPPORT;
    }

    if (OsalSpinLockIrqSave(&cntlr->lock, &flags) != HDF_SUCCESS) {
        return HDF_ERR_DEVICE_BUSY;
    }
    ret = cntlr->ops->start(cntlr);
    (void)OsalSpinUnlockIrqRestore(&cntlr->lock, &flags);
    return ret;
}

int32_t WatchdogCntlrStop(struct WatchdogCntlr *cntlr)
{
    int32_t ret;
    uint32_t flags;

    if (cntlr == NULL) {
        return HDF_ERR_INVALID_OBJECT;
    }
    if (cntlr->ops == NULL || cntlr->ops->stop == NULL) {
        return HDF_ERR_NOT_SUPPORT;
    }

    if (OsalSpinLockIrqSave(&cntlr->lock, &flags) != HDF_SUCCESS) {
        return HDF_ERR_DEVICE_BUSY;
    }
    ret = cntlr->ops->stop(cntlr);
    (void)OsalSpinUnlockIrqRestore(&cntlr->lock, &flags);
    return ret;
}

int32_t WatchdogCntlrSetTimeout(struct WatchdogCntlr *cntlr, uint32_t seconds)
{
    int32_t ret;
    uint32_t flags;

    if (cntlr == NULL) {
        return HDF_ERR_INVALID_OBJECT;
    }
    if (cntlr->ops == NULL || cntlr->ops->setTimeout == NULL) {
        return HDF_ERR_NOT_SUPPORT;
    }

    if (OsalSpinLockIrqSave(&cntlr->lock, &flags) != HDF_SUCCESS) {
        return HDF_ERR_DEVICE_BUSY;
    }
    ret = cntlr->ops->setTimeout(cntlr, seconds);
    (void)OsalSpinUnlockIrqRestore(&cntlr->lock, &flags);
    return ret;
}

int32_t WatchdogCntlrGetTimeout(struct WatchdogCntlr *cntlr, uint32_t *seconds)
{
    int32_t ret;
    uint32_t flags;

    if (cntlr == NULL) {
        return HDF_ERR_INVALID_OBJECT;
    }
    if (cntlr->ops == NULL || cntlr->ops->getTimeout == NULL) {
        return HDF_ERR_NOT_SUPPORT;
    }
    if (seconds == NULL) {
        return HDF_ERR_INVALID_PARAM;
    }

    if (OsalSpinLockIrqSave(&cntlr->lock, &flags) != HDF_SUCCESS) {
        return HDF_ERR_DEVICE_BUSY;
    }
    ret = cntlr->ops->getTimeout(cntlr, seconds);
    (void)OsalSpinUnlockIrqRestore(&cntlr->lock, &flags);
    return ret;
}

int32_t WatchdogCntlrFeed(struct WatchdogCntlr *cntlr)
{
    int32_t ret;
    uint32_t flags;

    if (cntlr == NULL) {
        return HDF_ERR_INVALID_OBJECT;
    }
    if (cntlr->ops == NULL || cntlr->ops->feed == NULL) {
        return HDF_ERR_NOT_SUPPORT;
    }

    if (OsalSpinLockIrqSave(&cntlr->lock, &flags) != HDF_SUCCESS) {
        return HDF_ERR_DEVICE_BUSY;
    }
    ret = cntlr->ops->feed(cntlr);
    (void)OsalSpinUnlockIrqRestore(&cntlr->lock, &flags);
    return ret;
}
