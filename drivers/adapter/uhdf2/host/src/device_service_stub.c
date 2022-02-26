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

#include "hdf_sbuf.h"
#include "device_service_stub.h"
#include "devsvc_manager_clnt.h"
#include "hdf_base.h"
#include "osal_mem.h"
#include "hdf_log.h"

int DeviceServiceStubDispatch(
    struct HdfRemoteService *stub, int code, struct HdfSBuf *data, struct HdfSBuf *reply)
{
    struct DeviceServiceStub *service = (struct DeviceServiceStub *)stub;
    struct IDeviceIoService *ioService = service->super.deviceObject.service;
    int ret = HDF_FAILURE;

    if (ioService == NULL) {
        return HDF_FAILURE;
    }

    struct HdfDeviceIoClient client = {
        .device = &service->super.deviceObject,
        .priv = NULL,
    };

    if (ioService->Dispatch != NULL) {
        ret = ioService->Dispatch(&client, code, data, reply);
    }
    return ret;
}

static struct HdfRemoteDispatcher g_deviceServiceDispatcher = {
    .Dispatch = DeviceServiceStubDispatch
};

int DeviceServiceStubPublishService(struct HdfDeviceNode *service, const char *serviceName)
{
    int status = HDF_FAILURE;
    struct DeviceServiceStub *fullService = (struct DeviceServiceStub *)service;
    const struct HdfDeviceInfo *deviceInfo = service->deviceInfo;

    if (fullService->remote != NULL) {
        HDF_LOGE("%{public}s:service %{public}s already published", __func__, serviceName);
        return HDF_ERR_INVALID_OBJECT;
    }

    if (deviceInfo->policy == SERVICE_POLICY_PUBLIC || deviceInfo->policy == SERVICE_POLICY_CAPACITY) {
        fullService->remote = HdfRemoteServiceObtain((struct HdfObject *)fullService, &g_deviceServiceDispatcher);
        if (fullService->remote == NULL) {
            return HDF_ERR_MALLOC_FAIL;
        }
        struct DevSvcManagerClnt *serviceManager =
            (struct DevSvcManagerClnt *)DevSvcManagerClntGetInstance();
        if (serviceManager != NULL) {
            status = DevSvcManagerClntAddService(serviceName, &fullService->super.deviceObject);
        }
    }

    return status;
}

void DeviceServiceStubConstruct(struct DeviceServiceStub *inst)
{
    HdfDeviceNodeConstruct(&inst->super);
    struct IDeviceNode *serviceIf = (struct IDeviceNode *)inst;
    if (serviceIf != NULL) {
        serviceIf->PublishService = DeviceServiceStubPublishService;
    }
}

struct HdfObject *DeviceServiceStubCreate()
{
    struct DeviceServiceStub *instance =
        (struct DeviceServiceStub *)OsalMemCalloc(sizeof(struct DeviceServiceStub));
    if (instance != NULL) {
        DeviceServiceStubConstruct(instance);
    }
    return (struct HdfObject *)instance;
}

void DeviceServiceStubRelease(struct HdfObject *object)
{
    struct DeviceServiceStub *instance = (struct DeviceServiceStub *)object;
    if (instance != NULL) {
        if (instance->remote != NULL) {
            HdfRemoteServiceRecycle(instance->remote);
            instance->remote = NULL;
        }
        HdfDeviceNodeDestruct(&instance->super);
        OsalMemFree(instance);
    }
}

