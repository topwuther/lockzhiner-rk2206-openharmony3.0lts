/*
 * Copyright (c) 2020-2021 Huawei Device Co., Ltd.
 *
 * HDF is dual licensed: you can use it either under the terms of
 * the GPL, or the BSD license, at your option.
 * See the LICENSE file in the root of this repository for complete details.
 */

#include "devmgr_service.h"
#include "devhost_service_clnt.h"
#include "device_token_clnt.h"
#include "hdf_attribute_manager.h"
#include "hdf_base.h"
#include "hdf_driver_installer.h"
#include "hdf_host_info.h"
#include "hdf_log.h"
#include "hdf_object_manager.h"

#define HDF_LOG_TAG devmgr_service

static int DevmgrServiceActiveDevice(struct DevHostServiceClnt *hostClnt,
    struct HdfDeviceInfo *deviceInfo, bool isLoad)
{
    struct IDevHostService *devHostSvcIf = (struct IDevHostService *)hostClnt->hostService;
    if (devHostSvcIf == NULL) {
        return HDF_FAILURE;
    }

    if (isLoad && (deviceInfo->preload != DEVICE_PRELOAD_ENABLE)) {
        int ret = devHostSvcIf->AddDevice(devHostSvcIf, deviceInfo);
        if (ret == HDF_SUCCESS) {
            deviceInfo->preload = DEVICE_PRELOAD_ENABLE;
        }
        return ret;
    } else if (!isLoad && (deviceInfo->preload != DEVICE_PRELOAD_DISABLE)) {
        devHostSvcIf->DelDevice(devHostSvcIf, deviceInfo);
        deviceInfo->preload = DEVICE_PRELOAD_DISABLE;
        return HDF_SUCCESS;
    } else {
        return HDF_FAILURE;
    }
}

static int DevmgrServiceFindAndActiveDevice(const char *svcName, bool isLoad)
{
    struct HdfSListIterator itDeviceInfo;
    struct HdfDeviceInfo *deviceInfo = NULL;
    struct DevHostServiceClnt *hostClnt = NULL;
    struct DevmgrService *devMgrSvc = (struct DevmgrService *)DevmgrServiceGetInstance();
    if (devMgrSvc == NULL || svcName == NULL) {
        return HDF_ERR_INVALID_PARAM;
    }

    DLIST_FOR_EACH_ENTRY(hostClnt, &devMgrSvc->hosts, struct DevHostServiceClnt, node) {
        HdfSListIteratorInit(&itDeviceInfo, hostClnt->deviceInfos);
        while (HdfSListIteratorHasNext(&itDeviceInfo)) {
            deviceInfo = (struct HdfDeviceInfo *)HdfSListIteratorNext(&itDeviceInfo);
            if (strcmp(deviceInfo->svcName, svcName) == 0) {
                return DevmgrServiceActiveDevice(hostClnt, deviceInfo, isLoad);
            }
        }
    }

    return HDF_FAILURE;
}

int32_t DevmgrServiceLoadLeftDriver(struct DevmgrService *devMgrSvc)
{
    int32_t ret;
    struct HdfSListIterator itDeviceInfo;
    struct HdfDeviceInfo *deviceInfo = NULL;
    struct DevHostServiceClnt *hostClnt = NULL;
    if (devMgrSvc == NULL) {
        return HDF_FAILURE;
    }

    DLIST_FOR_EACH_ENTRY(hostClnt, &devMgrSvc->hosts, struct DevHostServiceClnt, node) {
        HdfSListIteratorInit(&itDeviceInfo, hostClnt->deviceInfos);
        while (HdfSListIteratorHasNext(&itDeviceInfo)) {
            deviceInfo = (struct HdfDeviceInfo *)HdfSListIteratorNext(&itDeviceInfo);
            if (deviceInfo->preload == DEVICE_PRELOAD_ENABLE_STEP2) {
                ret = DevmgrServiceActiveDevice(hostClnt, deviceInfo, true);
                if (ret != HDF_SUCCESS) {
                    HDF_LOGE("%s:failed to load driver %s", __func__, deviceInfo->moduleName);
                }
            }
        }
    }
    return HDF_SUCCESS;
}

int DevmgrServiceLoadDevice(const char *svcName)
{
    return DevmgrServiceFindAndActiveDevice(svcName, true);
}

int DevmgrServiceUnLoadDevice(const char *svcName)
{
    return DevmgrServiceFindAndActiveDevice(svcName, false);
}

static struct DevHostServiceClnt *DevmgrServiceFindDeviceHost(struct IDevmgrService *inst, uint16_t hostId)
{
    struct DevHostServiceClnt *hostClnt = NULL;
    struct DevmgrService *dmService = (struct DevmgrService *)inst;
    if (dmService == NULL) {
        HDF_LOGE("failed to find device host, dmService is null");
        return NULL;
    }

    DLIST_FOR_EACH_ENTRY(hostClnt, &dmService->hosts, struct DevHostServiceClnt, node) {
        if (hostClnt->hostId == hostId) {
            return hostClnt;
        }
    }
    HDF_LOGE("failed to find host, host id is %u", hostId);
    return NULL;
}

static void DevmgrServiceUpdateStatus(struct DevHostServiceClnt *hostClnt, uint16_t deviceId, uint16_t status)
{
    struct HdfSListIterator it;
    struct HdfDeviceInfo *deviceInfo = NULL;

    HdfSListIteratorInit(&it, hostClnt->deviceInfos);
    while (HdfSListIteratorHasNext(&it)) {
        deviceInfo = (struct HdfDeviceInfo *)HdfSListIteratorNext(&it);
        if (deviceInfo->deviceId == deviceId) {
            deviceInfo->status = status;
            HDF_LOGD("%s host:%s %u device:%s %u status:%u", __func__, hostClnt->hostName,
                hostClnt->hostId, deviceInfo->svcName, deviceId, deviceInfo->status);
            return;
        }
    }
    HDF_LOGE("%s: not find device %u in host %u", __func__, deviceId, hostClnt->hostId);
    return;
}

static int DevmgrServiceAttachDevice(
    struct IDevmgrService *inst, const struct HdfDeviceInfo *deviceInfo, struct IHdfDeviceToken *token)
{
    if (deviceInfo == NULL) {
        HDF_LOGE("failed to attach device, deviceInfo is null");
        return HDF_FAILURE;
    }
    struct DevHostServiceClnt *hostClnt = DevmgrServiceFindDeviceHost(inst, deviceInfo->hostId);
    if (hostClnt == NULL) {
        HDF_LOGE("failed to attach device, hostClnt is null");
        return HDF_FAILURE;
    }
    struct DeviceTokenClnt *tokenClnt = DeviceTokenClntNewInstance(token);
    if (tokenClnt == NULL) {
        HDF_LOGE("failed to attach device, tokenClnt is null");
        return HDF_FAILURE;
    }

    tokenClnt->deviceInfo = deviceInfo;
    DevmgrServiceUpdateStatus(hostClnt, deviceInfo->deviceId, HDF_SERVICE_USABLE);
    HdfSListAdd(&hostClnt->devices, &tokenClnt->node);
    return HDF_SUCCESS;
}

static int DevmgrServiceAttachDeviceHost(
    struct IDevmgrService *inst, uint16_t hostId, struct IDevHostService *hostService)
{
    struct DevHostServiceClnt *hostClnt = DevmgrServiceFindDeviceHost(inst, hostId);
    if (hostClnt == NULL) {
        HDF_LOGE("failed to attach device host, hostClnt is null");
        return HDF_FAILURE;
    }
    if (hostService == NULL) {
        HDF_LOGE("failed to attach device host, hostService is null");
        return HDF_FAILURE;
    }

    hostClnt->hostService = hostService;
    hostClnt->deviceInfos = HdfAttributeManagerGetDeviceList(hostClnt->hostId, hostClnt->hostName);
    if (hostClnt->deviceInfos == NULL) {
        HDF_LOGW("failed to get device list ");
        return HDF_SUCCESS;
    }
    hostClnt->devCount = HdfSListCount(hostClnt->deviceInfos);
    return DevHostServiceClntInstallDriver(hostClnt);
}

static int DevmgrServiceStartDeviceHosts(struct DevmgrService *inst)
{
    struct HdfSList hostList;
    struct HdfSListIterator it;
    struct HdfHostInfo *hostAttr = NULL;
    struct DevHostServiceClnt *hostClnt = NULL;
    struct IDriverInstaller *installer = NULL;
    installer = DriverInstallerGetInstance();
    if (installer == NULL || installer->StartDeviceHost == NULL) {
        HDF_LOGE("installer or installer->StartDeviceHost is null");
        return HDF_FAILURE;
    }
    HdfSListInit(&hostList);
    if (!HdfAttributeManagerGetHostList(&hostList)) {
        HDF_LOGW("%s: host list is null", __func__);
        return HDF_SUCCESS;
    }
    HdfSListIteratorInit(&it, &hostList);
    while (HdfSListIteratorHasNext(&it)) {
        hostAttr = (struct HdfHostInfo *)HdfSListIteratorNext(&it);
        hostClnt = DevHostServiceClntNewInstance(hostAttr->hostId, hostAttr->hostName);
        if (hostClnt == NULL) {
            HDF_LOGW("failed to create new device host client");
            continue;
        }
        DListInsertTail(&hostClnt->node, &inst->hosts);
        hostClnt->hostPid = installer->StartDeviceHost(hostAttr->hostId, hostAttr->hostName);
        if (hostClnt->hostPid == HDF_FAILURE) {
            HDF_LOGW("failed to start device host, host id is %u", hostAttr->hostId);
            DListRemove(&hostClnt->node);
            DevHostServiceClntFreeInstance(hostClnt);
        }
    }
    HdfSListFlush(&hostList, HdfHostInfoDelete);
    return HDF_SUCCESS;
}

int DevmgrServiceStartService(struct IDevmgrService *inst)
{
    struct DevmgrService *dmService = (struct DevmgrService *)inst;
    if (dmService == NULL) {
        HDF_LOGE("failed to start device manager service, dmService is null");
        return HDF_FAILURE;
    }
    return DevmgrServiceStartDeviceHosts(dmService);
}

int DevmgrServicePowerStateChange(struct IDevmgrService *devmgrService, enum HdfPowerState powerState)
{
    struct DevHostServiceClnt *hostClient = NULL;
    struct DevmgrService *devmgr = NULL;
    int result = HDF_SUCCESS;

    if (devmgrService == NULL) {
        return HDF_ERR_INVALID_OBJECT;
    }

    if (!IsValidPowerState(powerState)) {
        HDF_LOGE("%s:invalid power event %u", __func__, powerState);
        return HDF_ERR_INVALID_PARAM;
    }
    devmgr = CONTAINER_OF(devmgrService, struct DevmgrService, super);

    if (IsPowerWakeState(powerState)) {
        HDF_LOGI("%s:wake state %u", __func__, powerState);
        DLIST_FOR_EACH_ENTRY(hostClient, &devmgr->hosts, struct DevHostServiceClnt, node) {
            if (hostClient->hostService != NULL) {
                if (hostClient->hostService->PmNotify(hostClient->hostService, powerState) != HDF_SUCCESS) {
                    result = HDF_FAILURE;
                }
            }
        }
    } else {
        HDF_LOGI("%s:suspend state %u", __func__, powerState);
        DLIST_FOR_EACH_ENTRY_REVERSE(hostClient, &devmgr->hosts, struct DevHostServiceClnt, node) {
            if (hostClient->hostService != NULL) {
                if (hostClient->hostService->PmNotify(hostClient->hostService, powerState) != HDF_SUCCESS) {
                    result = HDF_FAILURE;
                }
            }
        }
    }

    return result;
}

bool DevmgrServiceConstruct(struct DevmgrService *inst)
{
    if (OsalMutexInit(&inst->devMgrMutex) != HDF_SUCCESS) {
        HDF_LOGE("%s:failed to mutex init ", __func__);
        return false;
    }
    struct IDevmgrService *devMgrSvcIf = (struct IDevmgrService *)inst;
    if (devMgrSvcIf != NULL) {
        devMgrSvcIf->AttachDevice = DevmgrServiceAttachDevice;
        devMgrSvcIf->AttachDeviceHost = DevmgrServiceAttachDeviceHost;
        devMgrSvcIf->StartService = DevmgrServiceStartService;
        devMgrSvcIf->PowerStateChange = DevmgrServicePowerStateChange;
        DListHeadInit(&inst->hosts);
        return true;
    } else {
        return false;
    }
}

struct HdfObject *DevmgrServiceCreate()
{
    static bool isDevMgrServiceInit = false;
    static struct DevmgrService devmgrServiceInstance;
    if (!isDevMgrServiceInit) {
        if (!DevmgrServiceConstruct(&devmgrServiceInstance)) {
            return NULL;
        }
        isDevMgrServiceInit = true;
    }
    return (struct HdfObject *)&devmgrServiceInstance;
}

struct IDevmgrService *DevmgrServiceGetInstance()
{
    static struct IDevmgrService *instance = NULL;
    if (instance == NULL) {
        instance = (struct IDevmgrService *)HdfObjectManagerGetObject(HDF_OBJECT_ID_DEVMGR_SERVICE);
    }
    return instance;
}

void DevmgrServiceRelease(struct HdfObject *object)
{
    struct DevmgrService *devmgrService = (struct DevmgrService *)object;
    if (devmgrService == NULL) {
        return;
    }
    struct DevHostServiceClnt *hostClnt = NULL;
    struct DevHostServiceClnt *hostClntTmp = NULL;
    DLIST_FOR_EACH_ENTRY_SAFE(hostClnt, hostClntTmp, &devmgrService->hosts, struct DevHostServiceClnt, node) {
        DListRemove(&hostClnt->node);
        DevHostServiceClntDelete(hostClnt);	
    }

    OsalMutexDestroy(&devmgrService->devMgrMutex);
}
