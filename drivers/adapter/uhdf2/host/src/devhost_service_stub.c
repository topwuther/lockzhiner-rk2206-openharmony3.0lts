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

#include "devhost_service_stub.h"
#include "devhost_service_proxy.h"
#include "dev_attribute_parcel.h"
#include "hdf_base.h"
#include "hdf_log.h"
#include "hdf_sbuf.h"
#include "hdf_security.h"
#include "osal_mem.h"
#include "unistd.h"

#define HDF_LOG_TAG devhost_service_stub

static void DevHostSetCurrentSecurec(const char *hostName)
{
    (void) hostName;
}

static int DevHostServiceStubDispatch(
    struct HdfRemoteService *stub, int code, struct HdfSBuf *data, struct HdfSBuf *reply)
{
    (void)reply;
    int ret = HDF_FAILURE;
    if (stub == NULL || data == NULL) {
        return ret;
    }
    struct DevHostServiceStub *serviceStub = (struct DevHostServiceStub *)stub;
    struct IDevHostService *serviceIf = (struct IDevHostService *)&serviceStub->super;
    DevHostSetCurrentSecurec(serviceStub->super.super.hostName);
    OsalMutexLock(&serviceStub->hostSvcMutex);
    switch (code) {
        case DEVHOST_SERVICE_ADD_DEVICE: {
            if ((serviceIf == NULL) || (serviceIf->AddDevice == NULL)) {
                HDF_LOGE("serviceIf or serviceIf->AddDevice is NULL");
                break;
            }
            struct HdfDeviceInfoFull *attribute = DeviceAttributeFullRead(data);
            if (attribute == NULL) {
                HDF_LOGE("Dispatch failed, attribute is null");
                break;
            }
            ret = serviceIf->AddDevice(serviceIf, &attribute->super);
            if (ret != HDF_SUCCESS) {
                HDF_LOGE("Dispatch failed, add service failed and ret is %{public}d", ret);
            }
            break;
        }
        case DEVHOST_SERVICE_DEL_DEVICE: {
            if ((serviceIf == NULL) || (serviceIf->DelDevice == NULL)) {
                HDF_LOGE("serviceIf or serviceIf->DelDevice is NULL");
                break;
            }
            struct HdfDeviceInfoFull *attribute = DeviceAttributeFullRead(data);
            if (attribute == NULL) {
                HDF_LOGE("Dispatch failed, attribute is null");
                break;
            }
            ret = serviceIf->DelDevice(serviceIf, &attribute->super);
            if (ret != HDF_SUCCESS) {
                HDF_LOGE("Dispatch failed, del service failed and ret is %{public}d", ret);
            }
            break;
        }
        default: {
            HDF_LOGE("DevHostServiceStubDispatch unknown code:%{public}d", code);
            break;
        }
    }
    OsalMutexUnlock(&serviceStub->hostSvcMutex);
    return ret;
}


static void DevHostServiceStubConstruct(struct DevHostServiceStub *inst)
{
    static struct HdfRemoteDispatcher dispatcher = {
        .Dispatch = DevHostServiceStubDispatch
    };

    DevHostServiceFullConstruct(&inst->super);
    inst->remote = HdfRemoteServiceObtain((struct HdfObject *)inst, &dispatcher);

    OsalMutexInit(&inst->hostSvcMutex);
}

struct HdfObject *DevHostServiceStubCreate(void)
{
    struct DevHostServiceStub *instance =
        (struct DevHostServiceStub *)OsalMemCalloc(sizeof(struct DevHostServiceStub));
    if (instance != NULL) {
        DevHostServiceStubConstruct(instance);
        return (struct HdfObject *)&instance->super;
    }
    return NULL;
}


void DevHostServiceStubRelease(struct HdfObject *object)
{
    struct DevHostServiceStub *instance = (struct DevHostServiceStub *)object;
    if (instance != NULL) {
        DevHostServiceFullDestruct(&instance->super);
        if (instance->remote != NULL) {
            HdfRemoteServiceRecycle(instance->remote);
            instance->remote = NULL;
        }
        OsalMutexDestroy(&instance->hostSvcMutex);
        OsalMemFree(instance);
    }
}

