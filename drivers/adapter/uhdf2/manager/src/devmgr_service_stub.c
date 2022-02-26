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

#include "devmgr_service_stub.h"
#include "devhost_service_clnt.h"
#include "devhost_service_proxy.h"
#include "device_token_proxy.h"
#include "devmgr_pnp_service.h"
#include "devmgr_query_device.h"
#include "devmgr_virtual_service.h"
#include "devsvc_manager.h"
#include "hdf_base.h"
#include "hdf_log.h"
#include "hdf_sbuf.h"
#include "osal_mem.h"
#include "osal_sysevent.h"
#include "usb_pnp_manager.h"

#define HDF_LOG_TAG devmgr_service_stub

static int32_t DevmgrServiceStubDispatchAttachDevice(
    struct IDevmgrService *devmgrSvc, struct HdfSBuf *data)
{
    uint32_t hostId;
    uint32_t deviceId;
    if (!HdfSbufReadUint32(data, &hostId) || !HdfSbufReadUint32(data, &deviceId)) {
        HDF_LOGE("%{public}s:failed to get host id and device id", __func__);
        return HDF_ERR_INVALID_PARAM;
    }

    struct HdfDevTokenProxy *tokenClnt = HdfDevTokenProxyObtain(NULL);
    if (tokenClnt == NULL) {
        return HDF_FAILURE;
    }
    struct HdfDeviceInfo deviceInfo;
    deviceInfo.deviceId = deviceId;
    deviceInfo.hostId = hostId;
    return devmgrSvc->AttachDevice(devmgrSvc, &deviceInfo, &tokenClnt->super);
}

static int32_t DevmgrServiceStubDispatchPnpDevice(
    struct IDevmgrService *devmgrSvc, struct HdfSBuf *data, bool isReg)
{
    int32_t ret = HDF_FAILURE;

    const char *moduleName = HdfSbufReadString(data);
    if (moduleName == NULL) {
        return ret;
    }
    const char *serviceName = HdfSbufReadString(data);
    if (serviceName == NULL) {
        return ret;
    }

    if (isReg) {
        ret = DevmgrServiceRegPnpDevice(devmgrSvc, moduleName, serviceName, NULL, NULL);
    } else {
        ret = DevmgrServiceUnRegPnpDevice(devmgrSvc, moduleName, serviceName);
    }
    return ret;
}

int32_t DevmgrServiceStubDispatch(
    struct HdfRemoteService* stub, int code, struct HdfSBuf *data, struct HdfSBuf *reply)
{
    int32_t ret = HDF_FAILURE;
    struct DevmgrServiceStub *serviceStub = (struct DevmgrServiceStub *)stub;
    if (serviceStub == NULL) {
        return HDF_ERR_INVALID_PARAM;
    }
    struct IDevmgrService *super = (struct IDevmgrService *)&serviceStub->super;
    HDF_LOGE("%{public}s devmgr service stub dispatch cmd %{public}d", __func__, code);
    switch (code) {
        case DEVMGR_SERVICE_ATTACH_DEVICE_HOST: {
            uint32_t hostId = 0;
            if (!HdfSbufReadUint32(data, &hostId)) {
                HDF_LOGE("invalid host id");
                return HDF_FAILURE;
            }
            struct HdfRemoteService *service = HdfSBufReadRemoteService(data);
            struct IDevHostService *hostIf = DevHostServiceProxyObtain(hostId, service);
            ret = super->AttachDeviceHost(super, hostId, hostIf);
            break;
        }
        case DEVMGR_SERVICE_ATTACH_DEVICE: {
            ret = DevmgrServiceStubDispatchAttachDevice(super, data);
            break;
        }
        case DEVMGR_SERVICE_REGIST_PNP_DEVICE: {
            ret = DevmgrServiceStubDispatchPnpDevice(super, data, true);
            break;
        }
        case DEVMGR_SERVICE_UNREGIST_PNP_DEVICE: {
            ret = DevmgrServiceStubDispatchPnpDevice(super, data, false);
            break;
        }
        case DEVMGR_SERVICE_QUERY_DEVICE: {
            ret = DevFillQueryDeviceInfo(super, data, reply);
            break;
        }
        case DEVMGR_SERVICE_REGISTER_VIRTUAL_DEVICE: {
            ret = DevmgrServiceVirtualDevice(super, data, reply, true);
            break;
        }
        case DEVMGR_SERVICE_UNREGISTER_VIRTUAL_DEVICE: {
            ret = DevmgrServiceVirtualDevice(super, data, reply, false);
            break;
        }
        default:
            break;
    }
    if (ret != HDF_SUCCESS) {
        HDF_LOGE("%{public}s devmgr service stub dispach failed, cmd id is %{public}d", __func__, code);
        HdfSbufWriteInt32(reply, ret);
    }

    return ret;
}

static struct HdfRemoteDispatcher g_devmgrDispatcher = {
    .Dispatch = DevmgrServiceStubDispatch
};

int DevmgrServiceStubStartService(struct IDevmgrService *inst)
{
    int status = HDF_FAILURE;
    struct HdfRemoteService *remoteService = NULL;
    struct DevmgrServiceStub *fullService = (struct DevmgrServiceStub *)inst;
    struct IDevSvcManager *serviceManager = DevSvcManagerGetInstance();
    if (fullService == NULL) {
        HDF_LOGI("Start service failed, fullService is null");
        return HDF_FAILURE;
    }
    remoteService = HdfRemoteServiceObtain((struct HdfObject *)inst, &g_devmgrDispatcher);
    if ((remoteService == NULL) || (serviceManager == NULL)) {
        HDF_LOGI("Start service failed, remoteService or serviceManager is null");
        return HDF_FAILURE;
    }
    struct HdfDeviceObject *deviceObject = OsalMemCalloc(sizeof(struct HdfDeviceObject));
    if (deviceObject == NULL) {
        return HDF_FAILURE;
    }
    deviceObject->service = (struct IDeviceIoService *)remoteService;
    if (serviceManager->AddService != NULL) {
        status = DevSvcManagerAddService(
            serviceManager, DEVICE_MANAGER_SERVICE, deviceObject);
    }
    if (status != HDF_SUCCESS) {
        OsalMemFree(deviceObject);
        return status;
    }
    fullService->remote = remoteService;

    status = DevmgrServiceStartService((struct IDevmgrService *)&fullService->super);
    if (status == HDF_SUCCESS) {
        if (DevmgrUsbPnpManageEventHandle(inst) != HDF_SUCCESS) {
            HDF_LOGE("%s:%d DevmgrUsbPnpManageEventHandle error", __func__, __LINE__);
        }
    }

    return status;
}

static void DevmgrServiceStubConstruct(struct DevmgrServiceStub *inst)
{
    struct IDevmgrService *pvtbl = (struct IDevmgrService *)inst;

    DevmgrServiceFullConstruct(&inst->super);
    pvtbl->StartService = DevmgrServiceStubStartService;
    inst->remote = NULL;
    OsalMutexInit(&inst->devmgrStubMutx);
}

struct HdfObject *DevmgrServiceStubCreate()
{
    static struct DevmgrServiceStub *instance = NULL;
    if (instance == NULL) {
        instance = (struct DevmgrServiceStub *)OsalMemCalloc(sizeof(struct DevmgrServiceStub));
        if (instance == NULL) {
            HDF_LOGE("Creating devmgr service stub failed, alloc mem error");
            return NULL;
        }
        DevmgrServiceStubConstruct(instance);
    }
    return (struct HdfObject *)instance;
}

void DevmgrServiceStubRelease(struct HdfObject *object)
{
    struct DevmgrServiceStub *instance = (struct DevmgrServiceStub *)object;
    if (instance != NULL) {
        if (instance->remote != NULL) {
            HdfRemoteServiceRecycle(instance->remote);
            instance->remote = NULL;
        }
        OsalMutexDestroy(&instance->devmgrStubMutx);
        OsalMemFree(instance);
    }
}

