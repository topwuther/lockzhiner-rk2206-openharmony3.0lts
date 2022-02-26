/*
 * Copyright (c) 2020-2021 Huawei Device Co., Ltd.
 *
 * HDF is dual licensed: you can use it either under the terms of
 * the GPL, or the BSD license, at your option.
 * See the LICENSE file in the root of this repository for complete details.
 */

#include "hdf_device_node.h"
#include "devhost_service.h"
#include "devmgr_service_clnt.h"
#include "devsvc_manager_clnt.h"
#include "hdf_base.h"
#include "hdf_device_object.h"
#include "hdf_device_token.h"
#include "hdf_log.h"
#include "hdf_object_manager.h"
#include "hdf_observer_record.h"
#include "osal_mem.h"
#include "power_state_token.h"

#define HDF_LOG_TAG device_node

static int HdfDeviceNodePublishLocalService(
    struct HdfDeviceNode *devNode, const struct HdfDeviceInfo *deviceInfo)
{
    uint32_t matchId;
    if ((devNode == NULL) || (deviceInfo == NULL)) {
        HDF_LOGE("failed to publish local service, device is null");
        return HDF_FAILURE;
    }
    struct DevHostService *hostService = devNode->hostService;
    if (hostService == NULL) {
        HDF_LOGE("failed to publish local service, host service is null");
        return HDF_FAILURE;
    }
    matchId = HdfMakeHardwareId(deviceInfo->hostId, deviceInfo->deviceId);
    return HdfServiceObserverPublishService(&hostService->observer, deviceInfo->svcName,
        matchId, deviceInfo->policy, (struct HdfObject *)devNode->deviceObject.service);
}

static int HdfDeviceNodePublishService(
    struct HdfDeviceNode *devNode, const struct HdfDeviceInfo *deviceInfo, struct IHdfDevice *device)
{
    (void)device;
    int status = HDF_SUCCESS;
    if ((deviceInfo->policy == SERVICE_POLICY_NONE) ||
        ((deviceInfo->svcName != NULL) && (strlen(deviceInfo->svcName) == 0))) {
        HDF_LOGI("policy is %d", SERVICE_POLICY_NONE);
        return status;
    }

    struct IDeviceNode *nodeIf = &devNode->super;
    if ((deviceInfo->policy == SERVICE_POLICY_PUBLIC) ||
        (deviceInfo->policy == SERVICE_POLICY_CAPACITY)) {
        if (nodeIf->PublishService != NULL) {
            status = nodeIf->PublishService(devNode, deviceInfo->svcName);
        }
    }
    if (status == HDF_SUCCESS) {
        status = HdfDeviceNodePublishLocalService(devNode, deviceInfo);
    }
    return status;
}

int HdfDeviceLaunchNode(struct HdfDeviceNode *devNode, struct IHdfDevice *devInst)
{
    struct HdfDevice *device = (struct HdfDevice *)devInst;
    if (device == NULL || devNode == NULL) {
        HDF_LOGE("failed to launch service, device or service is null");
        return HDF_ERR_INVALID_PARAM;
    }

    struct HdfDriverEntry *driverEntry = devNode->driverEntry;
    const struct HdfDeviceInfo *deviceInfo = devNode->deviceInfo;
    struct IHdfDeviceToken *deviceToken = NULL;

    if (deviceInfo == NULL) {
        HDF_LOGE("failed to launch service, deviceInfo is null");
        return HDF_ERR_INVALID_PARAM;
    }

    if ((driverEntry == NULL) || (driverEntry->Init == NULL)) {
        HDF_LOGE("failed to launch service, deviceEntry invalid");
        return HDF_ERR_INVALID_PARAM;
    }
    int ret = driverEntry->Init(&devNode->deviceObject);
    if (ret != HDF_SUCCESS) {
        return HDF_DEV_ERR_DEV_INIT_FAIL;
    }
    ret = HdfDeviceNodePublishService(devNode, deviceInfo, devInst);
    if (ret != HDF_SUCCESS) {
        return HDF_DEV_ERR_PUBLISH_FAIL;
    }
    deviceToken = devNode->token;
    ret = DevmgrServiceClntAttachDevice(deviceInfo, deviceToken);
    if (ret != HDF_SUCCESS) {
        return HDF_DEV_ERR_ATTACHDEV_FAIL;
    }
    return ret;
}

int HdfDeviceNodeAddPowerStateListener(
    struct HdfDeviceNode *devNode, const struct IPowerEventListener *listener)
{
    if (devNode->powerToken != NULL) {
        return HDF_FAILURE;
    }

    devNode->powerToken = PowerStateTokenNewInstance(&devNode->deviceObject, listener);
    return (devNode->powerToken != NULL) ? HDF_SUCCESS : HDF_FAILURE;
}

void HdfDeviceNodeRemovePowerStateListener(
    struct HdfDeviceNode *devNode, const struct IPowerEventListener *listener)
{
    (void)listener;
    if (devNode == NULL || devNode->powerToken == NULL) {
        return;
    }

    PowerStateTokenFreeInstance(devNode->powerToken);
    devNode->powerToken = NULL;
}


int HdfDeviceNodePublishPublicService(struct HdfDeviceNode *devNode, const char *svcName)
{
    if ((devNode == NULL) || (devNode->deviceObject.service == NULL)) {
        HDF_LOGE("failed to publish public service: devNode is NULL");
        return HDF_FAILURE;
    }
    return DevSvcManagerClntAddService(svcName, &devNode->deviceObject);
}

void HdfDeviceNodeReclaimService(const char *svcName)
{
    DevSvcManagerClntRemoveService(svcName);
}

void HdfDeviceNodeConstruct(struct HdfDeviceNode *devNode)
{
    if (devNode != NULL) {
        struct IDeviceNode *nodeIf = &devNode->super;
        HdfDeviceObjectConstruct(&devNode->deviceObject);
        devNode->token = HdfDeviceTokenNewInstance();
        nodeIf->LaunchNode = HdfDeviceLaunchNode;
        nodeIf->PublishService = HdfDeviceNodePublishPublicService;
    }
}

void HdfDeviceNodeDestruct(struct HdfDeviceNode *devNode)
{
    if (devNode == NULL) {
        return;
    }
    HdfDeviceTokenFreeInstance(devNode->token);
    devNode->token = NULL;
    PowerStateTokenFreeInstance(devNode->powerToken);
    devNode->powerToken = NULL;
}

struct HdfDeviceNode *HdfDeviceNodeNewInstance()
{
    return (struct HdfDeviceNode *)HdfObjectManagerGetObject(HDF_OBJECT_ID_DEVICE_SERVICE);
}

void HdfDeviceNodeFreeInstance(struct HdfDeviceNode *devNode)
{
    HdfObjectManagerFreeObject((struct HdfObject *) devNode);
}

void HdfDeviceNodeDelete(struct HdfDeviceNode *devNode)
{
    if (devNode == NULL) {
        return;
    }

    if (devNode->driverEntry->Release != NULL) {
        devNode->driverEntry->Release(&devNode->deviceObject);
    }
    HdfDeviceNodeFreeInstance(devNode);
}

