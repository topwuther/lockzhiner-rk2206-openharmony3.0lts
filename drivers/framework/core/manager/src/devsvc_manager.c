/*
 * Copyright (c) 2020-2021 Huawei Device Co., Ltd.
 *
 * HDF is dual licensed: you can use it either under the terms of
 * the GPL, or the BSD license, at your option.
 * See the LICENSE file in the root of this repository for complete details.
 */

#include "devsvc_manager.h"
#include "devmgr_service.h"
#include "hdf_base.h"
#include "hdf_cstring.h"
#include "hdf_log.h"
#include "hdf_object_manager.h"
#include "hdf_service_record.h"

#define HDF_LOG_TAG devsvc_manager

static struct DevSvcRecord *DevSvcManagerSearchService(struct IDevSvcManager *inst, uint32_t serviceKey)
{
    struct HdfSListIterator it;
    struct DevSvcRecord *record = NULL;
    struct DevSvcRecord *searchResult = NULL;
    struct DevSvcManager *devSvcManager = (struct DevSvcManager *)inst;
    if (devSvcManager == NULL) {
        HDF_LOGE("failed to search service, devSvcManager is null");
        return NULL;
    }

    OsalMutexLock(&devSvcManager->mutex);
    HdfSListIteratorInit(&it, &devSvcManager->services);
    while (HdfSListIteratorHasNext(&it)) {
        record = (struct DevSvcRecord *)HdfSListIteratorNext(&it);
        if ((record != NULL) && (record->key == serviceKey)) {
            searchResult = record;
            break;
        }
    }
    OsalMutexUnlock(&devSvcManager->mutex);
    return searchResult;
}

int DevSvcManagerAddService(struct IDevSvcManager *inst, const char *svcName, struct HdfDeviceObject *service)
{
    struct DevSvcManager *devSvcManager = (struct DevSvcManager *)inst;
    if ((devSvcManager == NULL) || (service == NULL) || (svcName == NULL)) {
        HDF_LOGE("failed to add service, input param is null");
        return HDF_FAILURE;
    }

    struct DevSvcRecord *record = DevSvcRecordNewInstance();
    if (record == NULL) {
        HDF_LOGE("failed to add service , record is null");
        return HDF_FAILURE;
    }

    record->key = HdfStringMakeHashKey(svcName, 0);
    record->value = service;
    OsalMutexLock(&devSvcManager->mutex);
    HdfSListAdd(&devSvcManager->services, &record->entry);
    OsalMutexUnlock(&devSvcManager->mutex);
    return HDF_SUCCESS;
}

int DevSvcManagerSubscribeService(struct IDevSvcManager *inst, const char *svcName, struct SubscriberCallback callBack)
{
    int ret = HDF_FAILURE;
    struct DevSvcManager *devSvcMgr = (struct DevSvcManager *)inst;
    if (svcName == NULL || devSvcMgr == NULL) {
        return ret;
    }

    struct HdfObject *deviceService = DevSvcManagerGetService(inst, svcName);
    if (deviceService != NULL) {
        if (callBack.OnServiceConnected != NULL) {
            callBack.OnServiceConnected(callBack.deviceObject, deviceService);
        }
        return HDF_SUCCESS;
    }

    return DevmgrServiceLoadDevice(svcName);
}

void DevSvcManagerRemoveService(struct IDevSvcManager *inst, const char *svcName)
{
    struct DevSvcManager *devSvcManager = (struct DevSvcManager *)inst;
    uint32_t serviceKey = HdfStringMakeHashKey(svcName, 0);
    if (svcName == NULL || devSvcManager == NULL) {
        return;
    }
    struct DevSvcRecord *serviceRecord = DevSvcManagerSearchService(inst, serviceKey);
    if (serviceRecord != NULL) {
        OsalMutexLock(&devSvcManager->mutex);
        HdfSListRemove(&devSvcManager->services, &serviceRecord->entry);
        OsalMutexUnlock(&devSvcManager->mutex);
    }
    DevSvcRecordFreeInstance(serviceRecord);
}

struct HdfDeviceObject *DevSvcManagerGetObject(struct IDevSvcManager *inst, const char *svcName)
{
    uint32_t serviceKey = HdfStringMakeHashKey(svcName, 0);
    if (svcName == NULL) {
        HDF_LOGE("Get service failed, svcName is null");
        return NULL;
    }
    struct DevSvcRecord *serviceRecord = DevSvcManagerSearchService(inst, serviceKey);
    if (serviceRecord != NULL) {
        return serviceRecord->value;
    }
    return NULL;
}

struct HdfObject *DevSvcManagerGetService(struct IDevSvcManager *inst, const char *svcName)
{
    struct HdfDeviceObject *deviceObject = DevSvcManagerGetObject(inst, svcName);
    if (deviceObject == NULL) {
        return NULL;
    }
    return (struct HdfObject *)deviceObject->service;
}

bool DevSvcManagerConstruct(struct DevSvcManager *inst)
{
    if (inst == NULL) {
        HDF_LOGE("%s: inst is null!", __func__);
        return false;
    }
    struct IDevSvcManager *devSvcMgrIf = &inst->super;
    devSvcMgrIf->AddService = DevSvcManagerAddService;
    devSvcMgrIf->SubscribeService = DevSvcManagerSubscribeService;
    devSvcMgrIf->UnsubscribeService = NULL;
    devSvcMgrIf->RemoveService = DevSvcManagerRemoveService;
    devSvcMgrIf->GetService = DevSvcManagerGetService;
    devSvcMgrIf->GetObject = DevSvcManagerGetObject;
    HdfSListInit(&inst->services);
    if (OsalMutexInit(&inst->mutex) != HDF_SUCCESS) {
        HDF_LOGE("failed to create device service manager mutex");
        return false;
    }
    return true;
}

struct HdfObject *DevSvcManagerCreate()
{
    static bool isDevSvcManagerInit = false;
    static struct DevSvcManager devSvcManagerInstance;
    if (!isDevSvcManagerInit) {
        if (!DevSvcManagerConstruct(&devSvcManagerInstance)) {
            return NULL;
        }
        isDevSvcManagerInit = true;
    }
    return (struct HdfObject *)&devSvcManagerInstance;
}

void DevSvcManagerRelease(struct HdfObject *object)
{
    struct DevSvcManager *devSvcManager = (struct DevSvcManager *)object;
    if (object == NULL) {
        return;
    }
    HdfSListFlush(&devSvcManager->services, DevSvcRecordDelete);
    OsalMutexDestroy(&devSvcManager->mutex);
}

struct IDevSvcManager *DevSvcManagerGetInstance()
{
    static struct IDevSvcManager *instance = NULL;
    if (instance == NULL) {
        instance = (struct IDevSvcManager *)HdfObjectManagerGetObject(HDF_OBJECT_ID_DEVSVC_MANAGER);
    }
    return instance;
}
