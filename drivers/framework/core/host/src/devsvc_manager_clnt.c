/*
 * Copyright (c) 2020-2021 Huawei Device Co., Ltd.
 *
 * HDF is dual licensed: you can use it either under the terms of
 * the GPL, or the BSD license, at your option.
 * See the LICENSE file in the root of this repository for complete details.
 */

#include "devsvc_manager_clnt.h"
#include "devmgr_service.h"
#include "devsvc_manager.h"
#include "hdf_attribute_manager.h"
#include "hdf_base.h"
#include "hdf_log.h"
#include "hdf_object_manager.h"

#define HDF_LOG_TAG devsvc_manager_clnt

int DevSvcManagerClntAddService(const char *svcName, struct HdfDeviceObject *service)
{
    struct DevSvcManagerClnt *devSvcMgrClnt = DevSvcManagerClntGetInstance();
    if (devSvcMgrClnt == NULL) {
        HDF_LOGE("failed to add service, client is null");
        return HDF_FAILURE;
    }

    struct IDevSvcManager *serviceManager = devSvcMgrClnt->devSvcMgrIf;
    if (serviceManager == NULL || serviceManager->AddService == NULL) {
        HDF_LOGE("serviceManager AddService function is null");
        return HDF_FAILURE;
    }
    return serviceManager->AddService(serviceManager, svcName, service);
}

const struct HdfObject *DevSvcManagerClntGetService(const char *svcName)
{
    struct DevSvcManagerClnt *devSvcMgrClnt = DevSvcManagerClntGetInstance();
    if (devSvcMgrClnt == NULL) {
        HDF_LOGE("failed to get service, client is null");
        return NULL;
    }

    struct IDevSvcManager *serviceManager = devSvcMgrClnt->devSvcMgrIf;
    if (serviceManager == NULL || serviceManager->GetService == NULL) {
        HDF_LOGE("serviceManager GetService function is null");
        return NULL;
    }
    return serviceManager->GetService(serviceManager, svcName);
}

struct HdfDeviceObject *DevSvcManagerClntGetDeviceObject(const char *svcName)
{
    struct DevSvcManagerClnt *devSvcMgrClnt = DevSvcManagerClntGetInstance();
    if (devSvcMgrClnt == NULL) {
        HDF_LOGE("failed to get device object, client is null");
        return NULL;
    }

    struct IDevSvcManager *serviceManager = devSvcMgrClnt->devSvcMgrIf;
    if (serviceManager == NULL || serviceManager->GetObject == NULL) {
        HDF_LOGE("failed to get device object, method not implement");
        return NULL;
    }
    return serviceManager->GetObject(serviceManager, svcName);
}

struct HdfDeviceObject *HdfRegisterDevice(const char *moduleName, const char *serviceName, const void *privateData)
{
    int ret;
    if (!HdfDeviceListAdd(moduleName, serviceName, privateData)) {
        HDF_LOGE("%s device info add failed!", __func__);
        return NULL;
    }
    ret = DevmgrServiceLoadDevice(serviceName);
    if (ret != HDF_SUCCESS) {
        HDF_LOGE("%s load device %s failed!", __func__, serviceName);
        HdfDeviceListDel(moduleName, serviceName);
        return NULL;
    }
    return DevSvcManagerClntGetDeviceObject(serviceName);
}

void HdfUnregisterDevice(const char *moduleName, const char *serviceName)
{
    if (DevmgrServiceUnLoadDevice(serviceName) != HDF_SUCCESS) {
        HDF_LOGE("%s:failed to unload device %s !", __func__, serviceName);
    }
    HdfDeviceListDel(moduleName, serviceName);
}

int DevSvcManagerClntSubscribeService(const char *svcName, struct SubscriberCallback callback)
{
    struct DevSvcManagerClnt *devSvcMgrClnt = DevSvcManagerClntGetInstance();
    if (devSvcMgrClnt == NULL) {
        HDF_LOGE("failed to subscribe service, client is null");
        return HDF_FAILURE;
    }

    struct IDevSvcManager *serviceManager = devSvcMgrClnt->devSvcMgrIf;
    if (serviceManager == NULL || serviceManager->SubscribeService == NULL) {
        HDF_LOGE("failed to subscribe service, method not implement");
        return HDF_FAILURE;
    }
    return serviceManager->SubscribeService(serviceManager, svcName, callback);
}

int DevSvcManagerClntUnsubscribeService(const char *svcName)
{
    struct DevSvcManagerClnt *devSvcMgrClnt = DevSvcManagerClntGetInstance();
    if (devSvcMgrClnt == NULL) {
        HDF_LOGE("failed to unsubscribe service, client is null");
        return HDF_FAILURE;
    }

    struct IDevSvcManager *serviceManager = devSvcMgrClnt->devSvcMgrIf;
    if (serviceManager == NULL || serviceManager->UnsubscribeService == NULL) {
        HDF_LOGE("failed to unsubscribe service, method not implement");
        return HDF_FAILURE;
    }
    return serviceManager->UnsubscribeService(serviceManager, svcName);
}

void DevSvcManagerClntRemoveService(const char *svcName)
{
    struct DevSvcManagerClnt *devSvcMgrClnt = DevSvcManagerClntGetInstance();
    if (devSvcMgrClnt == NULL) {
        HDF_LOGE("failed to remove service, devSvcMgrClnt is null");
        return;
    }

    struct IDevSvcManager *serviceManager = devSvcMgrClnt->devSvcMgrIf;
    if (serviceManager == NULL || serviceManager->RemoveService == NULL) {
        HDF_LOGE("failed to remove service, method not implement");
        return;
    }
    serviceManager->RemoveService(serviceManager, svcName);
}

static void DevSvcManagerClntConstruct(struct DevSvcManagerClnt *inst)
{
    inst->devSvcMgrIf = (struct IDevSvcManager *)HdfObjectManagerGetObject(HDF_OBJECT_ID_DEVSVC_MANAGER);
}

struct DevSvcManagerClnt *DevSvcManagerClntGetInstance()
{
    static struct DevSvcManagerClnt *instance = NULL;
    if (instance == NULL) {
        static struct DevSvcManagerClnt singletonInstance;
        DevSvcManagerClntConstruct(&singletonInstance);
        instance = &singletonInstance;
    }
    return instance;
}

void DevSvcManagerClntFreeInstance(struct DevSvcManagerClnt *instance)
{
    if (instance != NULL) {
        HdfObjectManagerFreeObject((struct HdfObject *)instance->devSvcMgrIf);
    }
}

