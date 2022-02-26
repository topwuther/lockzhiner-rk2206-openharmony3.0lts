/*
 * Copyright (c) 2020-2021 Huawei Device Co., Ltd.
 *
 * HDF is dual licensed: you can use it either under the terms of
 * the GPL, or the BSD license, at your option.
 * See the LICENSE file in the root of this repository for complete details.
 */

#include "hdf_driver_loader.h"
#include "devsvc_manager_clnt.h"
#include "hcs_tree_if.h"
#include "hdf_device_desc.h"
#include "hdf_device_node.h"
#include "hdf_log.h"
#include "hdf_object_manager.h"
#include "hdf_attribute_manager.h"

#define HDF_LOG_TAG driver_loader

struct HdfDeviceNode *HdfDriverLoaderLoadNode(
    struct IDriverLoader *loader, const struct HdfDeviceInfo *deviceInfo)
{
    struct HdfDriverEntry *driverEntry = NULL;
    struct HdfDeviceNode *devNode = NULL;
    if ((loader == NULL) || (loader->GetDriverEntry == NULL)) {
        HDF_LOGE("failed to load node, loader is invalid");
        return NULL;
    }

    driverEntry = loader->GetDriverEntry(deviceInfo);
    if (driverEntry == NULL) {
        HDF_LOGE("failed to load node, deviceEntry is null");
        return NULL;
    }

    devNode = HdfDeviceNodeNewInstance();
    if (devNode == NULL) {
        HDF_LOGE("failed to load node, device node is null");
        return NULL;
    }

    devNode->driverEntry = driverEntry;
    devNode->deviceInfo = deviceInfo;
    devNode->deviceObject.property = HcsGetNodeByMatchAttr(HdfGetRootNode(), deviceInfo->deviceMatchAttr);
    devNode->deviceObject.priv = (void *)(deviceInfo->private);
    if (devNode->deviceObject.property == NULL) {
        HDF_LOGW("failed to load node, property is null, match attr is: %s", deviceInfo->deviceMatchAttr);
    }

    if ((deviceInfo->policy == SERVICE_POLICY_PUBLIC) || (deviceInfo->policy == SERVICE_POLICY_CAPACITY)) {
        if (driverEntry->Bind == NULL) {
            HDF_LOGE("driver bind method is null");
            HdfDeviceNodeFreeInstance(devNode);
            return NULL;
        }
        if (driverEntry->Bind(&devNode->deviceObject) != 0) {
            HDF_LOGE("bind driver failed");
            HdfDeviceNodeFreeInstance(devNode);
            return NULL;
        }
    }
    return devNode;
}

void HdfDriverLoaderUnLoadNode(struct IDriverLoader *loader, const struct HdfDeviceInfo *deviceInfo)
{
    struct HdfDriverEntry *driverEntry = NULL;
    struct HdfDeviceObject *deviceObject = NULL;
    if ((loader == NULL) || (loader->GetDriverEntry == NULL)) {
        HDF_LOGE("failed to unload service, loader invalid");
        return;
    }

    driverEntry = loader->GetDriverEntry(deviceInfo);
    if (driverEntry == NULL) {
        HDF_LOGE("failed to unload service, driverEntry is null");
        return;
    }
    if (driverEntry->Release == NULL) {
        HDF_LOGI("device release func is null");
        return;
    }
    deviceObject = DevSvcManagerClntGetDeviceObject(deviceInfo->svcName);
    if (deviceObject != NULL) {
        driverEntry->Release(deviceObject);
    }
}

void HdfDriverLoaderConstruct(struct HdfDriverLoader *inst)
{
    if (inst != NULL) {
        struct IDriverLoader *driverLoaderIf = (struct IDriverLoader *)inst;
        driverLoaderIf->LoadNode = HdfDriverLoaderLoadNode;
        driverLoaderIf->UnLoadNode = HdfDriverLoaderUnLoadNode;
        driverLoaderIf->GetDriverEntry = HdfDriverLoaderGetDriverEntry;
    }
}

struct HdfObject *HdfDriverLoaderCreate()
{
    static bool isDriverLoaderInit = false;
    static struct HdfDriverLoader driverLoader;
    if (!isDriverLoaderInit) {
        HdfDriverLoaderConstruct(&driverLoader);
        isDriverLoaderInit = true;
    }
    return (struct HdfObject *)&driverLoader;
}

struct IDriverLoader *HdfDriverLoaderGetInstance()
{
    static struct IDriverLoader *instance = NULL;
    if (instance == NULL) {
        instance = (struct IDriverLoader *)HdfObjectManagerGetObject(HDF_OBJECT_ID_DRIVER_LOADER);
    }
    return instance;
}

