/*
 * Copyright (c) 2020-2021 Huawei Device Co., Ltd.
 *
 * HDF is dual licensed: you can use it either under the terms of
 * the GPL, or the BSD license, at your option.
 * See the LICENSE file in the root of this repository for complete details.
 */

#include "watchdog_if.h"
#include "devsvc_manager_clnt.h"
#include "hdf_base.h"
#include "hdf_log.h"
#include "osal_mem.h"
#include "securec.h"
#include "watchdog_core.h"

#define HDF_LOG_TAG watchdog_if

#define WATCHDOG_ID_MAX   8
#define WATCHDOG_NAME_LEN 32

static struct Watchdog *WatchdogGetById(int16_t wdtId)
{
    char *serviceName = NULL;
    struct Watchdog *service = NULL;

    if (wdtId < 0 || wdtId >= WATCHDOG_ID_MAX) {
        HDF_LOGE("WatchdogGetById: invalid id:%d", wdtId);
        return NULL;
    }
    serviceName = OsalMemCalloc(WATCHDOG_NAME_LEN + 1);
    if (serviceName == NULL) {
        return NULL;
    }
    if (snprintf_s(serviceName, WATCHDOG_NAME_LEN + 1, WATCHDOG_NAME_LEN,
        "HDF_PLATFORM_WATCHDOG_%d", wdtId) < 0) {
        HDF_LOGE("WatchdogGetById: format service name fail!");
        OsalMemFree(serviceName);
        return NULL;
    }
    service = (struct Watchdog *)DevSvcManagerClntGetService(serviceName);
    if (service == NULL) {
        HDF_LOGE("WatchdogGetById: get service fail!");
    }
    OsalMemFree(serviceName);
    return service;
}

DevHandle WatchdogOpen(int16_t wdtId)
{
    struct Watchdog *service = NULL;

    service = WatchdogGetById(wdtId);
    if (service == NULL) {
        return NULL;
    }
    WatchdogGetPrivData((struct WatchdogCntlr *)service);
    return (DevHandle)service;
}

void WatchdogClose(DevHandle handle)
{
    WatchdogReleasePriv((struct WatchdogCntlr *)handle);
    (void)handle;
}

int32_t WatchdogGetStatus(DevHandle handle, int32_t *status)
{
    if (handle == NULL) {
        return HDF_ERR_INVALID_OBJECT;
    }
    return WatchdogCntlrGetStatus((struct WatchdogCntlr *)handle, status);
}

int32_t WatchdogStart(DevHandle handle)
{
    if (handle == NULL) {
        return HDF_ERR_INVALID_OBJECT;
    }
    return WatchdogCntlrStart((struct WatchdogCntlr *)handle);
}

int32_t WatchdogStop(DevHandle handle)
{
    if (handle == NULL) {
        return HDF_ERR_INVALID_OBJECT;
    }
    return WatchdogCntlrStop((struct WatchdogCntlr *)handle);
}

int32_t WatchdogSetTimeout(DevHandle handle, uint32_t seconds)
{
    if (handle == NULL) {
        return HDF_ERR_INVALID_OBJECT;
    }
    return WatchdogCntlrSetTimeout((struct WatchdogCntlr *)handle, seconds);
}

int32_t WatchdogGetTimeout(DevHandle handle, uint32_t *seconds)
{
    if (handle == NULL) {
        return HDF_ERR_INVALID_OBJECT;
    }
    return WatchdogCntlrGetTimeout((struct WatchdogCntlr *)handle, seconds);
}

int32_t WatchdogFeed(DevHandle handle)
{
    if (handle == NULL) {
        return HDF_ERR_INVALID_OBJECT;
    }
    return WatchdogCntlrFeed((struct WatchdogCntlr *)handle);
}
