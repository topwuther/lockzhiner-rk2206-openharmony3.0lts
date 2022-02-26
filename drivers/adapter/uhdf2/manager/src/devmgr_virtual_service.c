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

#include "devhost_service_clnt.h"
#include "devmgr_pnp_service.h"
#include "devmgr_service.h"
#include "devsvc_manager.h"
#include "hdf_base.h"
#include "hdf_cstring.h"
#include "hdf_log.h"
#include "hdf_sbuf.h"

#define HDF_LOG_TAG devmgr_reg_virtual_device

static void DevmgrUpdateDeviceType(struct HdfSList *list, const char *moduleName, const char *serviceName)
{
    struct HdfSListIterator it;
    struct HdfDeviceInfo *deviceInfo = NULL;

    HdfSListIteratorInit(&it, list);
    while (HdfSListIteratorHasNext(&it)) {
        deviceInfo = (struct HdfDeviceInfo *)HdfSListIteratorNext(&it);
        if ((deviceInfo != NULL) &&
            (strcmp(deviceInfo->moduleName, moduleName) == 0) && (strcmp(deviceInfo->svcName, serviceName) == 0)) {
            deviceInfo->deviceType = HDF_DEV_REMOTE_SERVICE;
            HDF_LOGD("%{public}s: set remote service %{public}s %{public}s %{public}d",
                __func__, moduleName, serviceName, deviceInfo->deviceType);
            return;
        }
    }
    HDF_LOGE("%{public}s: remote service %{public}s %{public}s not found", __func__, moduleName, serviceName);
}

int DevmgrServiceVirtualDevice(
    struct IDevmgrService *inst, struct HdfSBuf *data, struct HdfSBuf *reply, bool flag)
{
    int32_t ret;
    struct HdfSList *list = NULL;

    (void)reply;
    if (inst == NULL || data == NULL) {
        HDF_LOGE("%{public}s para invalid", __func__);
        return HDF_FAILURE;
    }

    const char *moduleName = HdfSbufReadString(data);
    const char *serviceName = HdfSbufReadString(data);
    if (moduleName == NULL || serviceName == NULL) {
        HDF_LOGE("%{public}s: reading string failed", __func__);
        return HDF_FAILURE;
    }

    if (flag) {
        ret = DevmgrServiceRegPnpDevice(inst, moduleName, serviceName, NULL, NULL);
        if (ret == HDF_SUCCESS) {
            list = DevmgrServiceGetPnpDeviceInfo();
            DevmgrUpdateDeviceType(list, moduleName, serviceName);
        }
    } else {
        ret = DevmgrServiceUnRegPnpDevice(inst, moduleName, serviceName);
    }

    return ret;
}

