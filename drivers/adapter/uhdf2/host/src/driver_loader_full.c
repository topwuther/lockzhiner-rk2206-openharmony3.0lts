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

#include "driver_loader_full.h"
#include <dlfcn.h>
#include <errno.h>
#include <unistd.h>
#include <stdlib.h>
#include <limits.h>
#include "hdf_device.h"
#include "hdf_device_info_full.h"
#include "hdf_device_node.h"
#include "hdf_log.h"
#include "osal_mem.h"
#include "securec.h"

#define DRIVER_DESC "driverDesc"
#define HDF_LOG_TAG driver_loader_full
#ifdef __OHOS_STANDARD_SYS__
#define DRIVER_PATH "/system/lib/"
#else
#define DRIVER_PATH "/system/lib64/"
#endif

static struct DriverLoaderFull *g_fullLoader = NULL;

struct HdfDriverEntry *HdfDriverLoaderGetDriverEntry(const struct HdfDeviceInfo *deviceInfo)
{
    void *deviceHandle = NULL;
    struct HdfDriverEntry **deviceEntry = NULL;
    struct HdfDeviceInfoFull *fullAttribute = (struct HdfDeviceInfoFull *)deviceInfo;
    char realPath[PATH_MAX] = { 0 };
    char driverPath[PATH_MAX] = { 0 };
    if (deviceInfo == NULL || deviceInfo->moduleName == NULL) {
        return NULL;
    }

    if (strcat_s(driverPath, sizeof(driverPath) - 1, DRIVER_PATH) != EOK) {
        HDF_LOGE("%{public}s get driver path failed", __func__);
        return NULL;
    }

    if (strcat_s(driverPath, (sizeof(driverPath) - 1 - sizeof(DRIVER_PATH)), deviceInfo->moduleName) != EOK) {
        HDF_LOGE("%{public}s get full driver path failed", __func__);
        return NULL;
    }

    if (realpath(driverPath, realPath) == NULL) {
        HDF_LOGE("%{public}s no valid, errno:%{public}d", driverPath, errno);
        return NULL;
    }
    deviceHandle = dlopen(realPath, RTLD_LAZY);
    if (deviceHandle == NULL) {
        HDF_LOGE("Get device entry failed, %{public}s load fail", realPath);
        return NULL;
    }
    fullAttribute->deviceHandle = deviceHandle;
    deviceEntry = (struct HdfDriverEntry **)dlsym(deviceHandle, DRIVER_DESC);
    if (deviceEntry == NULL) {
        HDF_LOGE("Get device entry failed, dlsym failed");
        dlclose(deviceHandle);
        fullAttribute->deviceHandle = NULL;
        return NULL;
    }
    return *deviceEntry;
}

void HdfDriverLoaderFullConstruct(struct DriverLoaderFull *inst)
{
    struct HdfDriverLoader *pvtbl = (struct HdfDriverLoader *)inst;
    HdfDriverLoaderConstruct(pvtbl);
    pvtbl->super.GetDriverEntry = HdfDriverLoaderGetDriverEntry;
}

struct HdfObject *HdfDriverLoaderFullCreate()
{
    if (g_fullLoader == NULL) {
        struct DriverLoaderFull *instance =
            (struct DriverLoaderFull *)OsalMemCalloc(sizeof(struct DriverLoaderFull));
        if (instance != NULL) {
            HdfDriverLoaderFullConstruct(instance);
            g_fullLoader = instance;
        }
    }
    return (struct HdfObject *)g_fullLoader;
}

void HdfDriverLoaderFullRelease(struct HdfObject *object)
{
    struct DriverLoaderFull *instance = (struct DriverLoaderFull *)object;
    if (instance == g_fullLoader) {
        g_fullLoader = NULL;
    }
    if (instance != NULL) {
        OsalMemFree(instance);
    }
}
