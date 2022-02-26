/*
 * Copyright (c) 2020-2021 Huawei Device Co., Ltd.
 *
 * HDF is dual licensed: you can use it either under the terms of
 * the GPL, or the BSD license, at your option.
 * See the LICENSE file in the root of this repository for complete details.
 */

#include "hdf_device_node_ext.h"
#include "devsvc_manager_clnt.h"
#include "hdf_base.h"
#include "hdf_device_desc.h"
#include "hdf_io_service.h"
#include "hdf_log.h"
#include "hdf_sbuf.h"
#include "osal_mem.h"

#define HDF_LOG_TAG device_node_ext

static int DeviceNodeExtDispatch(struct HdfObject *stub, int code, struct HdfSBuf *data, struct HdfSBuf *reply)
{
    struct IDeviceIoService *deviceMethod = NULL;
    const struct HdfDeviceInfo *deviceInfo = NULL;
    struct HdfDeviceNode *devNode = NULL;

    if (stub == NULL) {
        HDF_LOGE("device ext dispatch: stub is null");
        return HDF_FAILURE;
    }
    uint64_t ioClientPtr = 0;
    if (!HdfSbufReadUint64(reply, &ioClientPtr) || ioClientPtr == 0) {
        HDF_LOGE("device ext dispatch: input ioClient is null");
        return HDF_FAILURE;
    }
    HdfSbufFlush(reply);
    devNode = CONTAINER_OF(stub, struct HdfDeviceNode, deviceObject);
    deviceMethod = devNode->deviceObject.service;
    if (deviceMethod == NULL) {
        HDF_LOGE("device ext dispatch: device service interface is null");
        return HDF_FAILURE;
    }
    deviceInfo = devNode->deviceInfo;
    if (deviceInfo == NULL) {
        HDF_LOGE("device ext dispatch: device deviceInfo is null");
        return HDF_FAILURE;
    }
    if (deviceInfo->policy == SERVICE_POLICY_CAPACITY) {
        if (deviceMethod->Dispatch == NULL) {
            HDF_LOGE("device ext dispatch: remote service dispatch method is null");
            return HDF_FAILURE;
        }
        return deviceMethod->Dispatch((struct HdfDeviceIoClient *)((uintptr_t)ioClientPtr), code, data, reply);
    }
    return HDF_FAILURE;
}

static int DeviceNodeExtPublishService(struct HdfDeviceNode *inst, const char *serviceName)
{
    const struct HdfDeviceInfo *deviceInfo = NULL;
    struct HdfDeviceObject *deviceObject = NULL;
    struct DeviceNodeExt *devNodeExt = (struct DeviceNodeExt *)inst;
    if (devNodeExt == NULL) {
        return HDF_FAILURE;
    }
    int ret = HdfDeviceNodePublishPublicService(inst, serviceName);
    if (ret != HDF_SUCCESS) {
        HDF_LOGE("failed to publish device service, ret is %d", ret);
        return HDF_FAILURE;
    }

    deviceInfo = inst->deviceInfo;
    deviceObject = &devNodeExt->super.deviceObject;
    if ((deviceObject->service == NULL) || (deviceInfo == NULL)) {
        HDF_LOGE("Device service interface or deviceInfo is null");
        return HDF_FAILURE;
    }
    if (deviceInfo->policy == SERVICE_POLICY_CAPACITY) {
        devNodeExt->ioService = HdfIoServicePublish(serviceName, deviceInfo->permission);
        if (devNodeExt->ioService != NULL) {
            devNodeExt->ioService->target = (struct HdfObject*)(&inst->deviceObject);
            static struct HdfIoDispatcher dispatcher = {
                .Dispatch = DeviceNodeExtDispatch
            };
            devNodeExt->ioService->dispatcher = &dispatcher;
        } else {
            HDF_LOGE("Device remote service bind failed");
            HdfDeviceNodeReclaimService(serviceName);
            return HDF_FAILURE;
        }
    }
    return HDF_SUCCESS;
}

static void DeviceNodeExtConstruct(struct DeviceNodeExt *inst)
{
    struct IDeviceNode *nodeIf = (struct IDeviceNode *)inst;
    if (nodeIf != NULL) {
        HdfDeviceNodeConstruct(&inst->super);
        nodeIf->PublishService = DeviceNodeExtPublishService;
    }
}

struct HdfObject *DeviceNodeExtCreate()
{
    struct DeviceNodeExt *instance =
        (struct DeviceNodeExt *)OsalMemCalloc(sizeof(struct DeviceNodeExt));
    if (instance != NULL) {
        DeviceNodeExtConstruct(instance);
        instance->ioService = NULL;
    }
    return (struct HdfObject *)instance;
}

void DeviceNodeExtRelease(struct HdfObject *object)
{
    struct DeviceNodeExt *instance = (struct DeviceNodeExt *)object;
    if (instance != NULL) {
        if (instance->ioService != NULL) {
            HdfIoServiceRemove(instance->ioService);
        }
        HdfDeviceNodeDestruct(&instance->super);
        OsalMemFree(instance);
    }
}

