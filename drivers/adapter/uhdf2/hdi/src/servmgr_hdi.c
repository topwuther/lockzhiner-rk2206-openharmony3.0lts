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

#include "servmgr_hdi.h"
#include <devsvc_manager_proxy.h>
#include <devsvc_manager_stub.h>
#include <hdf_base.h>
#include <hdf_log.h>
#include <osal_mem.h>

static int32_t ServiceManagerHdiCall(struct HDIServiceManager *iServMgr, int32_t id,
    struct HdfSBuf *data, struct HdfSBuf *reply)
{
    if (iServMgr->remote == NULL || iServMgr->remote->dispatcher == NULL ||
        iServMgr->remote->dispatcher->Dispatch == NULL) {
        return HDF_ERR_INVALID_OBJECT;
    }

    return iServMgr->remote->dispatcher->Dispatch(iServMgr->remote, id, data, reply);
}


struct HdfRemoteService *HDIServiceManagerGetService(struct HDIServiceManager *iServMgr, const char* serviceName)
{
    if (iServMgr == NULL || serviceName == NULL) {
        return NULL;
    }

    struct HdfSBuf *data = NULL;
    struct HdfSBuf *reply = NULL;
    struct HdfRemoteService *service = NULL;

    do {
        data = HdfSBufTypedObtain(SBUF_IPC);
        reply = HdfSBufTypedObtain(SBUF_IPC);
        if (data == NULL || reply == NULL) {
            break;
        }

        if (!HdfSbufWriteString(data, serviceName)) {
            break;
        }
        int status = ServiceManagerHdiCall(iServMgr, DEVSVC_MANAGER_GET_SERVICE, data, reply);
        if (status == HDF_SUCCESS) {
            service = HdfSBufReadRemoteService(reply);
        } else {
            HDF_LOGI("%{public}s: %{public}s not found", __func__, serviceName);
        }
    } while (0);

    if (reply != NULL) {
        HdfSBufRecycle(reply);
    }
    if (data != NULL) {
        HdfSBufRecycle(data);
    }
    return service;
}

void HDIServiceManagerConstruct(struct HDIServiceManager *inst)
{
    inst->GetService = HDIServiceManagerGetService;
}

struct HDIServiceManager *HDIServiceManagerGet(void)
{
    struct HdfRemoteService *remote = HdfRemoteServiceGet(DEVICE_SERVICE_MANAGER_SA_ID);
    if (remote == NULL) {
        HDF_LOGE("%{public}s: hdi service %{public}s not found", __func__, DEVICE_SERVICE_MANAGER);
        return NULL;
    }

    struct HDIServiceManager *iServMgr = OsalMemAlloc(sizeof(struct HDIServiceManager));
    if (iServMgr == NULL) {
        HDF_LOGE("%{public}s: OOM", __func__);
        HdfRemoteServiceRecycle(remote);
        return NULL;
    }

    iServMgr->remote = remote;
    HDIServiceManagerConstruct(iServMgr);
    return iServMgr;
}

void HDIServiceManagerRelease(struct HDIServiceManager *servmgr)
{
    if (servmgr == NULL) {
        return;
    }

    HdfRemoteServiceRecycle(servmgr->remote);
    OsalMemFree(servmgr);
}