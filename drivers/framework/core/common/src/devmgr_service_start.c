/*
 * Copyright (c) 2020-2021 Huawei Device Co., Ltd.
 *
 * HDF is dual licensed: you can use it either under the terms of
 * the GPL, or the BSD license, at your option.
 * See the LICENSE file in the root of this repository for complete details.
 */

#include "devmgr_service_start.h"
#include "devhost_service_clnt.h"
#include "devmgr_service.h"
#include "devsvc_manager_clnt.h"
#include "hdf_base.h"
#include "hdf_device_node.h"
#include "hdf_io_service.h"
#include "hdf_log.h"
#include "hdf_sbuf.h"

#define DEV_MGR_NODE_PERM 0660

static int g_isQuickLoad = DEV_MGR_SLOW_LOAD;

static void GetDeviceServiceNameByClass(DeviceClass deviceClass, struct HdfSBuf *reply)
{
    struct HdfSListIterator itDeviceInfo;
    struct HdfDeviceInfo *deviceInfo = NULL;
    struct DevHostServiceClnt *hostClnt = NULL;
    struct DevmgrService *devMgrSvc = (struct DevmgrService *)DevmgrServiceGetInstance();
    if (devMgrSvc == NULL || reply == NULL) {
        return;
    }

    HdfSbufFlush(reply);
    DLIST_FOR_EACH_ENTRY(hostClnt, &devMgrSvc->hosts, struct DevHostServiceClnt, node) {
        HdfSListIteratorInit(&itDeviceInfo, hostClnt->deviceInfos);
        while (HdfSListIteratorHasNext(&itDeviceInfo)) {
            deviceInfo = (struct HdfDeviceInfo *)HdfSListIteratorNext(&itDeviceInfo);
            if (deviceInfo->policy == SERVICE_POLICY_CAPACITY) {
                struct HdfDeviceObject *deviceObject = DevSvcManagerClntGetDeviceObject(deviceInfo->svcName);
                if (deviceObject == NULL || (deviceObject->deviceClass != deviceClass)) {
                    continue;
                }
                HdfSbufWriteString(reply, deviceInfo->svcName);
            }
        }
    }
    HdfSbufWriteString(reply, NULL);
}

int DeviceManagerDispatch(struct HdfObject *stub, int code, struct HdfSBuf *data, struct HdfSBuf *reply)
{
    int ret = HDF_FAILURE;
    int32_t deviceClass = 0;
    const char *svcName = NULL;
    struct DevmgrService *devMgrSvc = (struct DevmgrService *)stub;
    if (data == NULL || devMgrSvc == NULL) {
        HDF_LOGE("%s: input param is invalid", __func__);
        return ret;
    }
    OsalMutexLock(&devMgrSvc->devMgrMutex);
    switch (code) {
        case DEVMGR_LOAD_SERVICE:
            svcName = HdfSbufReadString(data);
            if (svcName == NULL) {
                HDF_LOGE("%s: get svc name is null", __func__);
                break;
            }
            static struct SubscriberCallback callback = {
                .deviceObject = NULL,
                .OnServiceConnected = NULL,
            };
            ret = DevSvcManagerClntSubscribeService(svcName, callback);
            break;
        case DEVMGR_UNLOAD_SERVICE:
            svcName = HdfSbufReadString(data);
            if (svcName == NULL) {
                HDF_LOGE("%s: svc name is null", __func__);
                break;
            }
            ret = DevSvcManagerClntUnsubscribeService(svcName);
            break;
        case DEVMGR_GET_SERVICE:
            if (!HdfSbufReadInt32(data, &deviceClass)) {
                HDF_LOGE("%s: failed to get deviceClass", __func__);
                break;
            }
            GetDeviceServiceNameByClass(deviceClass, reply);
            ret = HDF_SUCCESS;
            break;
        default:
            HDF_LOGE("%s: unsupported configuration type: %d", __func__, code);
            break;
    }
    OsalMutexUnlock(&devMgrSvc->devMgrMutex);
    return ret;
}

void DeviceManagerSetQuickLoad(int loadFlag)
{
    g_isQuickLoad = loadFlag;
}

int DeviceManagerIsQuickLoad(void)
{
    return g_isQuickLoad;
}

int DeviceManagerStart(void)
{
    struct IDevmgrService *instance = DevmgrServiceGetInstance();

    if (instance == NULL || instance->StartService == NULL) {
        HDF_LOGE("device manager start failed, service instance is null");
        return HDF_FAILURE;
    }
    struct HdfIoService *ioService = HdfIoServicePublish(DEV_MGR_NODE, DEV_MGR_NODE_PERM);
    if (ioService != NULL) {
        static struct HdfIoDispatcher dispatcher = {
            .Dispatch = DeviceManagerDispatch,
        };
        ioService->dispatcher = &dispatcher;
        ioService->target = &instance->base;
    }
    return instance->StartService(instance);
}

int DeviceManagerStartStep2()
{
    if (DeviceManagerIsQuickLoad() == DEV_MGR_SLOW_LOAD) {
        HDF_LOGW("%s: device manager is not set to QuickLoad mode", __func__);
        return HDF_SUCCESS;
    }
    struct DevmgrService *devMgrSvc = (struct DevmgrService *)DevmgrServiceGetInstance();
    return DevmgrServiceLoadLeftDriver(devMgrSvc);
}

