/*
 * Copyright (c) 2020-2021 Huawei Device Co., Ltd.
 *
 * HDF is dual licensed: you can use it either under the terms of
 * the GPL, or the BSD license, at your option.
 * See the LICENSE file in the root of this repository for complete details.
 */

#include "hdf_driver_loader.h"
#include "hdf_log.h"
#include "osal_mem.h"

static struct HdfDriverEntry *HdfDriverEntryConstruct(int32_t *driverCount)
{
    int i;
    *driverCount = (int32_t)(((uint8_t *)(HDF_DRIVER_END()) - (uint8_t *)(HDF_DRIVER_BEGIN())) / sizeof(size_t));
    if (*driverCount <= 0) {
        HDF_LOGE("%s: failed to hdf get device counts", __func__);
        return NULL;
    }
    struct HdfDriverEntry *driverEntry = OsalMemCalloc(*driverCount * sizeof(struct HdfDriverEntry));
    if (driverEntry == NULL) {
        HDF_LOGE("%s: failed to alloc driver entry mem", __func__);
        *driverCount = 0;
        return NULL;
    }
    size_t *addrBegin = (size_t *)(HDF_DRIVER_BEGIN());
    for (i = 0; i < *driverCount; i++) {
        driverEntry[i] = *(struct HdfDriverEntry *)(*addrBegin);
        addrBegin++;
    }
    return driverEntry;
}

struct HdfDriverEntry *HdfDriverLoaderGetDriverEntry(const struct HdfDeviceInfo *deviceInfo)
{
    int i;
    if ((deviceInfo == NULL) || (deviceInfo->moduleName == NULL) || (deviceInfo->svcName == NULL)) {
        HDF_LOGE("%s: failed to get device entry, input deviceInfo is NULL", __func__);
        return NULL;
    }
    static struct HdfDriverEntry *driverEntry = NULL;
    static int32_t driverCount = 0;
    if (driverEntry == NULL) {
        driverEntry = HdfDriverEntryConstruct(&driverCount);
        if (driverEntry == NULL) {
            HDF_LOGE("%s: failed to construct driver entry", __func__);
            return NULL;
        }
    }
    for (i = 0; i < driverCount; i++) {
        if (driverEntry == NULL) {
            HDF_LOGE("%s: driver entry is null", __func__);
            return NULL;
        }
        if (driverEntry[i].moduleName == NULL) {
            HDF_LOGE("%s: driver entry module name is null", __func__);
            continue;
        }
        if (strcmp(deviceInfo->moduleName, driverEntry[i].moduleName) == 0) {
            return &driverEntry[i];
        }
    }
    HDF_LOGE("failed to get device entry %s", deviceInfo->svcName);
    return NULL;
}

