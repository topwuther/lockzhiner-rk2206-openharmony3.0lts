/*
 * Copyright (c) 2020-2021 Huawei Device Co., Ltd.
 *
 * HDF is dual licensed: you can use it either under the terms of
 * the GPL, or the BSD license, at your option.
 * See the LICENSE file in the root of this repository for complete details.
 */

#include "hdf_device.h"
#include "hdf_base.h"
#include "hdf_cstring.h"
#include "hdf_device_node.h"
#include "hdf_device_token.h"
#include "hdf_log.h"
#include "hdf_object_manager.h"
#include "hdf_service_observer.h"
#include "osal_mem.h"

#define HDF_LOG_TAG hdf_device

static int HdfDeviceAttach(struct IHdfDevice *devInst, struct HdfDeviceNode *devNode)
{
    struct HdfDevice *device = (struct HdfDevice *)devInst;
    struct IDeviceNode *nodeIf = (struct IDeviceNode *)devNode;
    if (device == NULL || nodeIf == NULL || nodeIf->LaunchNode == NULL) {
        HDF_LOGE("failed to attach device, input params invalid");
        return HDF_ERR_INVALID_PARAM;
    }
    DListInsertTail(&devNode->entry, &device->devNodes);
    return nodeIf->LaunchNode(devNode, devInst);
}

void HdfDeviceConstruct(struct HdfDevice *device)
{
    device->super.Attach = HdfDeviceAttach;
    DListHeadInit(&device->devNodes);
}

void HdfDeviceDestruct(struct HdfDevice *device)
{
    struct HdfDeviceNode *devNode = NULL;
    DLIST_FOR_EACH_ENTRY(devNode, &device->devNodes, struct HdfDeviceNode, entry) {
        HdfDeviceNodeDelete(devNode);
    }
    DListHeadInit(&device->devNodes);
}

struct HdfObject *HdfDeviceCreate()
{
    struct HdfDevice *device =
        (struct HdfDevice *)OsalMemCalloc(sizeof(struct HdfDevice));
    if (device != NULL) {
        HdfDeviceConstruct(device);
    }
    return (struct HdfObject *)device;
}

void HdfDeviceRelease(struct HdfObject *object)
{
    struct HdfDevice *device = (struct HdfDevice *)object;
    if (device != NULL) {
        HdfDeviceDestruct(device);
        OsalMemFree(device);
    }
}

struct HdfDevice *HdfDeviceNewInstance()
{
    return (struct HdfDevice *)HdfObjectManagerGetObject(HDF_OBJECT_ID_DEVICE);
}

void HdfDeviceFreeInstance(struct HdfDevice *device)
{
    if (device != NULL) {
        HdfObjectManagerFreeObject(&device->super.object);
    }
}

void HdfDeviceDelete(struct HdfSListNode *listEntry)
{
    if (listEntry != NULL) {
        struct HdfDevice *device = (struct HdfDevice *)HDF_SLIST_CONTAINER_OF(
            struct HdfSListNode, listEntry, struct HdfDevice, node);
        HdfDeviceFreeInstance(device);
    }
}

