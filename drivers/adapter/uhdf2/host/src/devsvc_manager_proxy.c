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

#include "devsvc_manager_proxy.h"
#include "device_service_stub.h"
#include "devsvc_manager_stub.h"
#include "hdf_base.h"
#include "hdf_log.h"
#include "hdf_sbuf.h"
#include "osal_mem.h"

#define HDF_LOG_TAG devsvc_manager_proxy

static int DevSvcManagerProxyAddService(
    struct IDevSvcManager *inst, const char *svcName, struct HdfDeviceObject *service)
{
    struct DevSvcManagerProxy *serviceProxy = (struct DevSvcManagerProxy *)inst;
    if (service == NULL || svcName == NULL) {
        HDF_LOGE("%{public}s:service or name is null", __func__);
        return HDF_ERR_INVALID_PARAM;
    }
    if ((serviceProxy == NULL) || (serviceProxy->remote == NULL)) {
        HDF_LOGE("Add service failed, serviceProxy is invalid");
        return HDF_ERR_INVALID_PARAM;
    }

    int status = HDF_FAILURE;
    struct HdfSBuf *data = HdfSBufTypedObtain(SBUF_IPC);
    struct HdfSBuf *reply = HdfSBufTypedObtain(SBUF_IPC);
    struct HdfRemoteDispatcher *dispatcher = NULL;
    struct HdfRemoteService *remoteService = NULL;
    do {
        if ((data == NULL) || (reply == NULL)) {
            HDF_LOGE("Add service failed, failed to obtain sbuf");
            break;
        }

        remoteService = serviceProxy->remote;
        dispatcher = remoteService->dispatcher;
        if (!HdfSbufWriteString(data, svcName)) {
            HDF_LOGE("Add service failed, failed to write service name");
            break;
        }

        struct HdfDeviceNode *devNode = HDF_SLIST_CONTAINER_OF(
            struct HdfDeviceObject, service, struct HdfDeviceNode, deviceObject);
        struct DeviceServiceStub *deviceFullService = (struct DeviceServiceStub *)devNode;
        if (deviceFullService->remote == NULL) {
            HDF_LOGE("%{public}s: device service is broken", __func__);
            break;
        }

        if (HdfSBufWriteRemoteService(data,  deviceFullService->remote) != HDF_SUCCESS) {
            HDF_LOGE("Add service failed, failed to write remote object");
            break;
        }
        status = dispatcher->Dispatch(remoteService, DEVSVC_MANAGER_ADD_SERVICE, data, reply);
        HDF_LOGI("servmgr add service %{public}s, result is %{public}d", svcName, status);
    } while(0);

    if (reply != NULL) {
        HdfSBufRecycle(reply);
    }
    if (data != NULL) {
        HdfSBufRecycle(data);
    }
    return status;
}

struct HdfObject *DevSvcManagerProxyGetService(struct IDevSvcManager *inst, const char *svcName)
{
    int status = HDF_FAILURE;
    struct HdfSBuf *data = HdfSBufTypedObtain(SBUF_IPC);
    struct HdfSBuf *reply = HdfSBufTypedObtain(SBUF_IPC);
    struct HdfRemoteDispatcher *dispatcher = NULL;
    struct HdfRemoteService *remoteService = NULL;
    struct DevSvcManagerProxy *serviceProxy = (struct DevSvcManagerProxy *) inst;
    if ((serviceProxy->remote == NULL) || (data == NULL) || (reply == NULL)) {
        HDF_LOGE("Get service failed, serviceProxy->remote or data or reply is null");
        goto finished;
    }
    dispatcher = serviceProxy->remote->dispatcher;
    HdfSbufWriteString(data, svcName);
    status = dispatcher->Dispatch(serviceProxy->remote, DEVSVC_MANAGER_GET_SERVICE, data, reply);
    if (status == HDF_SUCCESS) {
        remoteService = HdfSBufReadRemoteService(reply);
    }
finished:
    if (reply != NULL) {
        HdfSBufRecycle(reply);
    }
    if (data != NULL) {
        HdfSBufRecycle(data);
    }
    HDF_LOGI("DevSvcManagerProxyGetService finish, and status is %{public}d", status);
    return (remoteService == NULL) ? NULL : &remoteService->object_;
}

void DevSvcManagerProxyRemoveService(struct IDevSvcManager *inst, const char *svcName)
{
    if (inst == NULL || svcName == NULL) {
        return;
    }
    struct HdfSBuf *data = HdfSBufTypedObtain(SBUF_IPC);
    struct HdfSBuf *reply = HdfSBufTypedObtain(SBUF_IPC);
    struct HdfRemoteDispatcher *dispatcher = NULL;
    struct HdfRemoteService *remoteService = NULL;
    struct DevSvcManagerProxy *serviceProxy = (struct DevSvcManagerProxy *) inst;
    if ((serviceProxy->remote == NULL) || (data == NULL) || (reply == NULL)) {
        HDF_LOGE("Remove service failed, serviceProxy->remote or data or reply is null");
        goto finished;
    }
    remoteService = serviceProxy->remote;
    dispatcher = remoteService->dispatcher;
    HdfSbufWriteString(data, svcName);
    int status = dispatcher->Dispatch(remoteService, DEVSVC_MANAGER_REMOVE_SERVICE, data, reply);
    HDF_LOGD("Device service manager proxy remove service status is %{public}d", status);
finished:
    if (reply != NULL) {
        HdfSBufRecycle(reply);
    }
    if (data != NULL) {
        HdfSBufRecycle(data);
    }
}

void DevSvcManagerProxyConstruct(struct DevSvcManagerProxy *inst, struct HdfRemoteService *remote)
{
    inst->pvtbl.AddService = DevSvcManagerProxyAddService;
    inst->pvtbl.GetService = DevSvcManagerProxyGetService;
    inst->pvtbl.RemoveService = DevSvcManagerProxyRemoveService;
    inst->remote = remote;
}

static struct IDevSvcManager *DevSvcManagerProxyObtain(struct HdfRemoteService *remote)
{
    struct DevSvcManagerProxy *instance =
        (struct DevSvcManagerProxy *)OsalMemCalloc(sizeof(struct DevSvcManagerProxy));
    if (instance != NULL) {
        DevSvcManagerProxyConstruct(instance, remote);
    }
    return (struct IDevSvcManager *)instance;
}

struct HdfObject *DevSvcManagerProxyCreate()
{
    static struct IDevSvcManager *instance = NULL;
    if (instance == NULL) {
        struct HdfRemoteService *remote = HdfRemoteServiceGet(DEVICE_SERVICE_MANAGER_SA_ID);
        if (remote != NULL) {
            instance = DevSvcManagerProxyObtain(remote);
        }
    }
    return (struct HdfObject *)instance;
}

void DevSvcManagerProxyRelease(struct HdfObject *object)
{
    struct DevSvcManagerProxy *instance = (struct DevSvcManagerProxy *)object;
    if (instance != NULL) {
        if (instance->remote != NULL) {
            HdfRemoteServiceRecycle(instance->remote);
            instance->remote = NULL;
        }
        OsalMemFree(instance);
    }
}

