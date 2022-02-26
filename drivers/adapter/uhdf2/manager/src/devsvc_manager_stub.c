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

#include "devsvc_manager_stub.h"
#include "device_token_proxy.h"
#include "devmgr_service_stub.h"
#include "devsvc_manager.h"
#include "devsvc_manager_proxy.h"
#include "hdf_base.h"
#include "hdf_cstring.h"
#include "hdf_log.h"
#include "hdf_sbuf.h"
#include "hdf_slist.h"
#include "osal_mem.h"

#define HDF_LOG_TAG devsvc_manager_stub

static int32_t DevSvcManagerStubAddService(struct IDevSvcManager *super, struct HdfSBuf *data)
{
    int ret = HDF_FAILURE;
    struct DevSvcManagerStub *stub = (struct DevSvcManagerStub *)super;
    const char *name = HdfSbufReadString(data);
    if (name == NULL) {
        HDF_LOGE("%{public}s failed, name is null", __func__);
        return ret;
    }
    struct HdfRemoteService *service = HdfSBufReadRemoteService(data);
    if (service == NULL) {
        HDF_LOGE("%{public}s failed, service is null", __func__);
        return ret;
    }
    HdfRemoteServiceAddDeathRecipient(service, &stub->recipient);
    struct HdfDeviceObject *deviceObject = OsalMemCalloc(sizeof(struct HdfDeviceObject));
    if (deviceObject == NULL) {
        return ret;
    }
    deviceObject->service = (struct IDeviceIoService *)service;
    ret = super->AddService(super, name, deviceObject);
    if (ret != HDF_SUCCESS) {
        OsalMemFree(deviceObject);
    }
    return ret;
}

static int32_t DevSvcManagerStubGetService(
    struct IDevSvcManager *super, struct HdfSBuf *data, struct HdfSBuf *reply)
{
    int ret = HDF_FAILURE;
    const char *name = HdfSbufReadString(data);
    if (name == NULL) {
        HDF_LOGE("%{public}s failed, name is null", __func__);
        return ret;
    }
    struct HdfRemoteService *remoteService =
        (struct HdfRemoteService *)super->GetService(super, name);
    if (remoteService != NULL) {
        ret = HDF_SUCCESS;
        HdfSBufWriteRemoteService(reply, remoteService);
    } else {
        HDF_LOGE("service %{public}s not found", name);
    }

    return ret;
}

static int32_t DevSvcManagerStubRemoveService(struct IDevSvcManager *super, struct HdfSBuf *data)
{
    const char *name = HdfSbufReadString(data);
    if (name == NULL) {
        HDF_LOGE("%{public}s failed, name is null", __func__);
        return HDF_FAILURE;
    }
    struct HdfDeviceObject *deviceObject = super->GetObject(super, name);
    super->RemoveService(super, name);
    OsalMemFree(deviceObject);
    return HDF_SUCCESS;
}

int DevSvcManagerStubDispatch(
    struct HdfRemoteService* service, int code, struct HdfSBuf *data, struct HdfSBuf *reply)
{
    int ret = HDF_FAILURE;
    struct DevSvcManagerStub *stub = (struct DevSvcManagerStub *)service;
    if (stub == NULL) {
        HDF_LOGE("DevSvcManagerStubDispatch failed, object is null, code is %{public}d", code);
        return ret;
    }
    struct IDevSvcManager *super = (struct IDevSvcManager *)&stub->super;
    switch (code) {
        case DEVSVC_MANAGER_ADD_SERVICE: {
            ret = DevSvcManagerStubAddService(super, data);
            break;
        }
        case DEVSVC_MANAGER_GET_SERVICE: {
            ret = DevSvcManagerStubGetService(super, data, reply);
            break;
        }
        case DEVSVC_MANAGER_REMOVE_SERVICE: {
            ret = DevSvcManagerStubRemoveService(super, data);
            break;
        }
        default: {
            HDF_LOGE("Unknown code : %{public}d", code);
            ret = HDF_FAILURE;
        }
    }
    return ret;
}

void DevSvcManagerOnServiceDied(struct HdfDeathRecipient *recipient, struct HdfRemoteService *service)
{
    (void)service;
    struct DevSvcManagerStub *stub =
        HDF_SLIST_CONTAINER_OF(struct HdfDeathRecipient, recipient, struct DevSvcManagerStub, recipient);
    if (stub != NULL) {
        struct IDevSvcManager *svcOps = (struct IDevSvcManager *)stub;
        if (svcOps->RemoveService != NULL) {
            svcOps->RemoveService(svcOps, NULL);
        }
        return;
    }
}

bool DevSvcManagerStubConstruct(struct DevSvcManagerStub *inst)
{
    static struct HdfRemoteDispatcher dispatcher = {
        .Dispatch = DevSvcManagerStubDispatch
    };
    if (!DevSvcManagerConstruct(&inst->super)) {
        HDF_LOGE("Device service manager construct failed");
        return false;
    }
    inst->remote = HdfRemoteServiceObtain((struct HdfObject *)inst, &dispatcher);
    if (inst->remote == NULL) {
        HDF_LOGE("Device service manager failed to obtain remote service");
        return false;
    }

    inst->recipient.OnRemoteDied = DevSvcManagerOnServiceDied;

    int ret = HdfRemoteServiceRegister(DEVICE_SERVICE_MANAGER_SA_ID, inst->remote);
    if (ret != 0) {
        HDF_LOGE("Device service manager failed to publish hdi, %{public}d", ret);
    }

    HDF_LOGI("Device service manager publish success");
    return true;
}

struct HdfObject *DevSvcManagerStubCreate()
{
    static struct DevSvcManagerStub instance;
    if (instance.remote == NULL) {
        if (!DevSvcManagerStubConstruct(&instance)) {
            HDF_LOGE("Creating device service manager stub failed");
        }
    }
    return (struct HdfObject *)&instance;
}

void DevSvcManagerStubRelease(struct HdfObject *object)
{
    struct DevmgrServiceStub *instance = (struct DevmgrServiceStub *)object;
    if (instance != NULL) {
        if (instance->remote != NULL) {
            HdfRemoteServiceRecycle(instance->remote);
            instance->remote = NULL;
        }
    }
}

