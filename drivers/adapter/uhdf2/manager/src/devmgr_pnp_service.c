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

#include "devmgr_pnp_service.h"
#include "devhost_service_clnt.h"
#include "hdf_device_info_full.h"
#include "hdf_dlist.h"
#include "hdf_driver_installer.h"
#include "hdf_log.h"
#include "osal_mem.h"
#include "osal_time.h"
#include "usb_pnp_manager.h"

#define PNP_SLEEP_TIME 50 // ms

static struct HdfSList *g_deviceInfos = NULL;
static uint16_t g_hostId = 0xFFFF;
static uint32_t g_deviceId = 0;

struct HdfSList *DevmgrServiceGetPnpDeviceInfo()
{
    return g_deviceInfos;
}

void DevmgrServiceSetPnpHostId(uint16_t hostId)
{
    struct HdfSListIterator it;
    struct HdfDeviceInfoFull *deviceInfo = NULL;
    g_hostId = hostId;
    HdfSListIteratorInit(&it, g_deviceInfos);
    while (HdfSListIteratorHasNext(&it)) {
        deviceInfo = (struct HdfDeviceInfoFull *)HdfSListIteratorNext(&it);
        deviceInfo->super.hostId = hostId;
    }
}

static bool DevmgrServiceIsPnpDeviceExist(const char *moduleName, const char *serviceName)
{
    struct HdfSListIterator it;
    struct HdfDeviceInfoFull *deviceInfo = NULL;
    HdfSListIteratorInit(&it, g_deviceInfos);
    while (HdfSListIteratorHasNext(&it)) {
        deviceInfo = (struct HdfDeviceInfoFull *)HdfSListIteratorNext(&it);
        if ((strcmp(deviceInfo->super.moduleName, moduleName) == 0) &&
            (strcmp(deviceInfo->super.svcName, serviceName) == 0)) {
            return true;
        }
    }
    return false;
}

static bool DevmgrServiceAddPnpDeviceInfo(struct HdfDeviceInfoFull *deviceInfo)
{
    if (g_deviceInfos == NULL) {
        g_deviceInfos = OsalMemCalloc(sizeof(struct HdfSList));
        if (g_deviceInfos == NULL) {
            return false;
        }
    }
    deviceInfo->super.deviceId = g_deviceId++; // HdfSListCount  g_deviceInfos
    deviceInfo->super.hostId = g_hostId;
    deviceInfo->super.policy = SERVICE_POLICY_PUBLIC;
    deviceInfo->super.preload = DEVICE_PRELOAD_ENABLE;
    HdfSListAdd(g_deviceInfos, &deviceInfo->super.node);
    return true;
}

static bool DevmgrServiceAddPnpDevice(
    const char *moduleName, const char *serviceName, const char *deviceMatchAttr, const void *privateData)
{
    if (moduleName == NULL || serviceName == NULL) {
        return false;
    }
    if (DevmgrServiceIsPnpDeviceExist(moduleName, serviceName)) {
        HDF_LOGE("%{public}s adding pnp device failed, %{public}s already exist", __func__, moduleName);
        return false;
    }
    struct HdfDeviceInfoFull *deviceInfo = HdfDeviceInfoFullNewInstance();
    if (deviceInfo == NULL) {
        return false;
    }
    deviceInfo->super.moduleName = strdup(moduleName);
    if (deviceInfo->super.moduleName == NULL) {
        HdfDeviceInfoFullFreeInstance(deviceInfo);
        return false;
    }
    deviceInfo->super.svcName = strdup(serviceName);
    if (deviceInfo->super.svcName == NULL) {
        HdfDeviceInfoFullFreeInstance(deviceInfo);
        return false;
    }

    deviceInfo->super.deviceMatchAttr = strdup(deviceMatchAttr);
    if (deviceInfo->super.deviceMatchAttr == NULL) {
        HdfDeviceInfoFullFreeInstance(deviceInfo);
        return false;
    }

    if (!DevmgrUsbPnpManageAddPrivateData(deviceInfo, privateData)) {
        HdfDeviceInfoFullFreeInstance(deviceInfo);
        return false;
    }

    if (!DevmgrServiceAddPnpDeviceInfo(deviceInfo)) {
        HdfDeviceInfoFullFreeInstance(deviceInfo);
        return false;
    }
    return true;
}

static void DevmgrServiceDelPnpDevice(const char *moduleName, const char *serviceName)
{
    struct HdfSListIterator it;
    struct HdfDeviceInfoFull *deviceInfo = NULL;
    HdfSListIteratorInit(&it, g_deviceInfos);
    while (HdfSListIteratorHasNext(&it)) {
        deviceInfo = (struct HdfDeviceInfoFull *)HdfSListIteratorNext(&it);
        if ((strcmp(deviceInfo->super.moduleName, moduleName) == 0) &&
            (strcmp(deviceInfo->super.svcName, serviceName) == 0)) {
            HdfSListRemove(g_deviceInfos, &deviceInfo->super.node);
            HdfDeviceInfoFullFreeInstance(deviceInfo);
            return;
        }
    }
}

struct DevHostServiceClnt *DevmgrServiceGetPnpHostClnt(struct DevmgrService *inst)
{
    struct DevHostServiceClnt *hostClnt = NULL;
    if (inst == NULL) {
        return NULL;
    }

    DLIST_FOR_EACH_ENTRY(hostClnt, &inst->hosts, struct DevHostServiceClnt, node) {
        if (strcmp(hostClnt->hostName, PNP_HOST_NAME) == 0) {
            return hostClnt;
        }
    }
    return NULL;
}

int32_t DevmgrServiceStartPnpHost(struct DevmgrService *inst)
{
    struct DevHostServiceClnt *hostClnt = NULL;
    struct IDriverInstaller *installer = NULL;
    int32_t ret = HDF_FAILURE;

    installer = DriverInstallerGetInstance();
    if ((installer == NULL) || (installer->StartDeviceHost == NULL)) {
        HDF_LOGE("%{public}s installer or installer->StartDeviceHost is null", __func__);
        return ret;
    }

    uint16_t hostId = DlistGetCount(&inst->hosts);
    DevmgrServiceSetPnpHostId(hostId);
    hostClnt = DevHostServiceClntNewInstance(hostId, PNP_HOST_NAME);
    if (hostClnt == NULL) {
        HDF_LOGW("%{public}s creating new device host client failed", __func__);
        return ret;
    }
    DListInsertTail(&hostClnt->node, &inst->hosts);
    ret = installer->StartDeviceHost(hostClnt->hostId, hostClnt->hostName);
    if (ret != HDF_SUCCESS) {
        HDF_LOGW("%{public}s starting host failed, host name is %{public}s", __func__, hostClnt->hostName);
        DListRemove(&hostClnt->node);
        DevHostServiceClntFreeInstance(hostClnt);
    }
    return ret;
}

static int DevmgrServiceInstallDevice(struct DevHostServiceClnt *hostClnt, const char *serviceName)
{
    int ret = HDF_FAILURE;
    struct HdfSListIterator it;
    struct HdfDeviceInfo *deviceInfo = NULL;
    struct IDevHostService *devHostSvcIf = NULL;

    devHostSvcIf = (struct IDevHostService *)hostClnt->hostService;
    if (devHostSvcIf == NULL || devHostSvcIf->AddDevice == NULL) {
        HDF_LOGE("devHostSvcIf or devHostSvcIf->AddDevice is null");
        return ret;
    }
    HdfSListIteratorInit(&it, hostClnt->deviceInfos);
    while (HdfSListIteratorHasNext(&it)) {
        deviceInfo = (struct HdfDeviceInfo *)HdfSListIteratorNext(&it);
        if (strcmp(deviceInfo->svcName, serviceName) == 0) {
            ret = devHostSvcIf->AddDevice(devHostSvcIf, deviceInfo);
            if (ret != HDF_SUCCESS) {
                HDF_LOGE("Installing %{public}s driver failed, ret = %{public}d", deviceInfo->svcName, ret);
            }
            break;
        }
    }
    return ret;
}

int32_t DevmgrServiceRegPnpDevice(
    struct IDevmgrService *devmgrSvc, const char *moduleName, const char *serviceName,
    const char *deviceMatchAttr, const void *privateData)
{
    int32_t ret = HDF_FAILURE;
    struct DevmgrService *inst = (struct DevmgrService *)devmgrSvc;
    if (inst == NULL) {
        return HDF_ERR_INVALID_PARAM;
    }
    struct DevHostServiceClnt *hostClnt = DevmgrServiceGetPnpHostClnt(inst);
    if (hostClnt == NULL) {
        ret = DevmgrServiceStartPnpHost(inst);
        if (ret != HDF_SUCCESS) {
            HDF_LOGE("%{public}s add pnp device failed!", __func__);
            return ret;
        }
        OsalMSleep(PNP_SLEEP_TIME);
        hostClnt = DevmgrServiceGetPnpHostClnt(inst);
    }
    if (hostClnt == NULL || hostClnt->hostService == NULL) {
        HDF_LOGE("%{public}s host service is null!", __func__);
        return HDF_ERR_INVALID_OBJECT;
    }
    if (!DevmgrServiceAddPnpDevice(moduleName, serviceName, deviceMatchAttr, privateData)) {
        HDF_LOGE("%{public}s add pnp device failed!", __func__);
        return ret;
    }

    hostClnt->deviceInfos = DevmgrServiceGetPnpDeviceInfo();
    ret = DevmgrServiceInstallDevice(hostClnt, serviceName);
    if (ret != HDF_SUCCESS) {
        DevmgrServiceDelPnpDevice(moduleName, serviceName);
        HDF_LOGE("%{public}s: failed %{public}d %{public}s", __func__, hostClnt->hostId, hostClnt->hostName);
    }
    return ret;
}


int32_t DevmgrServiceUnRegPnpDevice(
    struct IDevmgrService *devmgrSvc, const char *moduleName, const char *serviceName)
{
    int ret = HDF_FAILURE;
    struct DevmgrService *inst = (struct DevmgrService *)devmgrSvc;
    struct DevHostServiceClnt *hostClnt = DevmgrServiceGetPnpHostClnt(inst);
    if (hostClnt == NULL || hostClnt->hostService == NULL) {
        HDF_LOGE("%{public}s host service is not init", __func__);
        OsalMSleep(PNP_SLEEP_TIME);
        hostClnt = DevmgrServiceGetPnpHostClnt(inst);
        if (hostClnt == NULL || hostClnt->hostService == NULL) {
            HDF_LOGE("%{public}s host service is not init", __func__);
            return ret;
        }
    }

    struct HdfSListIterator it;
    struct HdfDeviceInfo *deviceInfo = NULL;
    struct IDevHostService *devHostSvcIf = NULL;

    devHostSvcIf = (struct IDevHostService *)hostClnt->hostService;
    if (devHostSvcIf->DelDevice == NULL) {
        HDF_LOGE("%{public}s host del device is not init", __func__);
        return ret;
    }
    HdfSListIteratorInit(&it, hostClnt->deviceInfos);
    while (HdfSListIteratorHasNext(&it)) {
        deviceInfo = (struct HdfDeviceInfo *)HdfSListIteratorNext(&it);
        if ((strcmp(deviceInfo->moduleName, moduleName) == 0) &&
            (strcmp(deviceInfo->svcName, serviceName) == 0)) {
            deviceInfo->status = HDF_SERVICE_UNUSABLE;
            ret = devHostSvcIf->DelDevice(devHostSvcIf, deviceInfo);
            DevmgrServiceDelPnpDevice(moduleName, serviceName);
        }
    }
    return ret;
}

