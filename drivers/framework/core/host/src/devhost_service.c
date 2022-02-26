/*
 * Copyright (c) 2020-2021 Huawei Device Co., Ltd.
 *
 * HDF is dual licensed: you can use it either under the terms of
 * the GPL, or the BSD license, at your option.
 * See the LICENSE file in the root of this repository for complete details.
 */

#include "devhost_service.h"
#include "devmgr_service_clnt.h"
#include "devsvc_manager_clnt.h"
#include "hdf_base.h"
#include "hdf_driver_loader.h"
#include "hdf_log.h"
#include "hdf_object_manager.h"
#include "osal_mem.h"
#include "power_state_token.h"

#define HDF_LOG_TAG devhost_service

static struct HdfDevice *DevHostServiceFindDevice(struct DevHostService *hostService, uint16_t deviceId)
{
    struct HdfDevice *deviceNode = NULL;
    if (hostService == NULL) {
        HDF_LOGE("failed to find driver, hostService is null");
        return NULL;
    }

    DLIST_FOR_EACH_ENTRY(deviceNode, &hostService->devices, struct HdfDevice, node) {
        if (deviceNode->deviceId == deviceId) {
            return deviceNode;
        }
    }
    return NULL;
}

static void DevHostServiceFreeDevice(struct DevHostService *hostService, uint16_t deviceId)
{
    struct HdfDevice *device = DevHostServiceFindDevice(hostService, deviceId);
    if (device != NULL) {
        DListRemove(&device->node);
        HdfDeviceFreeInstance(device);
    }
}

static struct HdfDevice *DevHostServiceGetDevice(struct DevHostService *inst, uint16_t deviceId)
{
    struct HdfDevice *device = DevHostServiceFindDevice(inst, deviceId);
    if (device == NULL) {
        device = HdfDeviceNewInstance();
        if (device == NULL) {
            HDF_LOGE("Dev host service failed to create driver instance");
            return NULL;
        }
        device->hostId = inst->hostId;
        device->deviceId = deviceId;
        DListInsertHead(&device->node, &inst->devices);
    }
    return device;
}

int DevHostServiceAddDevice(struct IDevHostService *inst, const struct HdfDeviceInfo *deviceInfo)
{
    int ret = HDF_FAILURE;
    struct HdfDevice *device = NULL;
    struct HdfDeviceNode *devNode = NULL;
    struct DevHostService *hostService = CONTAINER_OF(inst, struct DevHostService, super);
    struct IDriverLoader *driverLoader = HdfDriverLoaderGetInstance();

    if (inst == NULL || deviceInfo == NULL || driverLoader == NULL || driverLoader->LoadNode == NULL) {
        HDF_LOGE("failed to add device, input param is null");
        return ret;
    }

    device = DevHostServiceGetDevice(hostService, deviceInfo->deviceId);
    if (device == NULL || device->super.Attach == NULL) {
        ret = HDF_DEV_ERR_NO_DEVICE;
        goto error;
    }

    devNode = driverLoader->LoadNode(driverLoader, deviceInfo);
    if (devNode == NULL) {
        ret = HDF_DEV_ERR_NO_DEVICE_SERVICE;
        goto error;
    }
    devNode->hostService = hostService;
    ret = device->super.Attach(&device->super, devNode);
    if (ret != HDF_SUCCESS) {
        goto error;
    }
    return HDF_SUCCESS;

error:
    DevHostServiceFreeDevice(hostService, device->deviceId);
    return ret;
}

static struct HdfDeviceNode *DevHostServiceSeparateDeviceNode(struct DListHead *deviceNodes,
    const struct HdfDeviceInfo *deviceInfo)
{
    struct HdfDeviceNode *deviceNode = NULL;
    DLIST_FOR_EACH_ENTRY(deviceNode, deviceNodes, struct HdfDeviceNode, entry) {
        if (strcmp(deviceNode->deviceInfo->svcName, deviceInfo->svcName) == 0 &&
            strcmp(deviceNode->deviceInfo->moduleName, deviceInfo->moduleName) == 0) {
            DListRemove(&deviceNode->entry);
            return deviceNode;
        }
    }
    return NULL;
}

int DevHostServiceDelDevice(struct IDevHostService *inst, const struct HdfDeviceInfo *deviceInfo)
{
    struct HdfDevice *device = NULL;
    struct DevHostService *hostService = (struct DevHostService *)inst;
    struct IDriverLoader *driverLoader =  HdfDriverLoaderGetInstance();

    if ((deviceInfo == NULL) || (driverLoader == NULL) || (driverLoader->UnLoadNode == NULL)) {
        HDF_LOGE("failed to del device, input param is null");
        return HDF_FAILURE;
    }

    device = DevHostServiceFindDevice(hostService, deviceInfo->deviceId);
    if (device == NULL) {
        HDF_LOGW("failed to del device, device is not exist");
        return HDF_SUCCESS;
    }

    driverLoader->UnLoadNode(driverLoader, deviceInfo);
    struct HdfDeviceNode *devNode = DevHostServiceSeparateDeviceNode(&device->devNodes, deviceInfo);
    if (device->super.Detach != NULL) {
        device->super.Detach(&device->super, devNode);
    } else {
        HdfDeviceNodeFreeInstance(devNode);
    }
    DevSvcManagerClntRemoveService(deviceInfo->svcName);
    if (DListIsEmpty(&device->devNodes)) {
        DevHostServiceFreeDevice(hostService, device->deviceId);
    }
    return HDF_SUCCESS;
}

static int DevHostServiceStartService(struct IDevHostService *service)
{
    struct DevHostService *hostService = (struct DevHostService*)service;
    if (hostService == NULL) {
        HDF_LOGE("failed to start device service, hostService is null");
        return HDF_FAILURE;
    }
    return DevmgrServiceClntAttachDeviceHost(hostService->hostId, service);
}

static int ApplyDevicesPowerState(struct HdfDevice *device, uint32_t state)
{
    struct HdfDeviceNode *deviceNode = NULL;
    int ret = HDF_SUCCESS;

    /* The power management strategy is to ignore devices that fail to
     operate and avoid more devices that fail to sleep or wake up */
    if (IsPowerWakeState(state)) {
        DLIST_FOR_EACH_ENTRY(deviceNode, &device->devNodes, struct HdfDeviceNode, entry) {
            if (deviceNode->powerToken != NULL) {
                ret = PowerStateChange(deviceNode->powerToken, state);
                if (ret != HDF_SUCCESS) {
                    HDF_LOGE("device %s failed to resume(%d)", deviceNode->driverEntry->moduleName, state);
                }
            }
        }
    } else {
        DLIST_FOR_EACH_ENTRY_REVERSE(deviceNode, &device->devNodes, struct HdfDeviceNode, entry) {
            if (deviceNode->powerToken != NULL) {
                ret = PowerStateChange(deviceNode->powerToken, state);
                if (ret != HDF_SUCCESS) {
                    HDF_LOGE("device %s failed to suspend(%d)", deviceNode->driverEntry->moduleName, state);
                }
            }
        }
    }

    return HDF_SUCCESS;
}

static int DevHostServicePmNotify(struct IDevHostService *service, uint32_t state)
{
    struct HdfDevice *device = NULL;
    int ret = HDF_SUCCESS;
    struct DevHostService *hostService = CONTAINER_OF(service, struct DevHostService, super);
    if (hostService == NULL) {
        HDF_LOGE("failed to start device service, hostService is null");
        return HDF_FAILURE;
    }

    HDF_LOGD("host(%s) set power state=%u", hostService->hostName, state);
    if (IsPowerWakeState(state)) {
        DLIST_FOR_EACH_ENTRY_REVERSE(device, &hostService->devices, struct HdfDevice, node) {
            if (ApplyDevicesPowerState(device, state) != HDF_SUCCESS) {
                ret = HDF_FAILURE;
            }
        }
    } else {
        DLIST_FOR_EACH_ENTRY(device, &hostService->devices, struct HdfDevice, node) {
            if (ApplyDevicesPowerState(device, state) != HDF_SUCCESS) {
                ret = HDF_FAILURE;
            }
        }
    }

    return ret;
}

void DevHostServiceConstruct(struct DevHostService *service)
{
    struct IDevHostService *hostServiceIf = &service->super;
    if (hostServiceIf != NULL) {
        hostServiceIf->AddDevice = DevHostServiceAddDevice;
        hostServiceIf->DelDevice = DevHostServiceDelDevice;
        hostServiceIf->StartService = DevHostServiceStartService;
        hostServiceIf->PmNotify = DevHostServicePmNotify;
        DListHeadInit(&service->devices);
        HdfServiceObserverConstruct(&service->observer);
    }
}

void DevHostServiceDestruct(struct DevHostService *service)
{
    if (service == NULL) {
        return;
    }

    struct HdfDevice *device = NULL;
    struct HdfDevice *tmp = NULL;
    DLIST_FOR_EACH_ENTRY_SAFE(device, tmp, &service->devices, struct HdfDevice, node) {
        HdfDeviceFreeInstance(device);
    }
    HdfServiceObserverDestruct(&service->observer);
}

struct HdfObject *DevHostServiceCreate()
{
    struct DevHostService *devHostService = (struct DevHostService *)OsalMemCalloc(sizeof(struct DevHostService));
    if (devHostService != NULL) {
        DevHostServiceConstruct(devHostService);
    }
    return (struct HdfObject *)devHostService;
}

void DevHostServiceRelease(struct HdfObject *object)
{
    struct DevHostService *devHostService = (struct DevHostService *)object;
    if (devHostService != NULL) {
        DevHostServiceDestruct(devHostService);
        OsalMemFree(devHostService);
    }
}

struct IDevHostService *DevHostServiceNewInstance(uint16_t hostId, const char *hostName)
{
    struct DevHostService *hostService =
        (struct DevHostService *)HdfObjectManagerGetObject(HDF_OBJECT_ID_DEVHOST_SERVICE);
    if (hostService != NULL) {
        hostService->hostId = hostId;
        hostService->hostName = hostName;
    }
    return (struct IDevHostService *)hostService;
}

void DevHostServiceFreeInstance(struct IDevHostService *service)
{
    if (service != NULL) {
        HdfObjectManagerFreeObject(&service->object);
    }
}
