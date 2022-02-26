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

#include "hdf_log.h"
#include "hdf_base.h"
#include "hdf_device_desc.h"
#include "hdf_pnp_driver.h"
#include "pal_if.h"

#define HDF_LOG_TAG hdf_pnp_driver

static int g_gpioFlg = 0;

int32_t PnpDriverDispatch(struct HdfDeviceIoClient *client, int cmdId, struct HdfSBuf *data, struct HdfSBuf *reply)
{
    (void)client;
    if (data == NULL || reply == NULL) {
        HDF_LOGE("%{public}s failed, input is null", __func__);
        return HDF_FAILURE;
    }
    int32_t ret = HDF_SUCCESS;
    switch (cmdId) {
        case HDF_PNP_MES_TEST: {
            const char *str = HdfSbufReadString(data);
            HDF_LOGE("%{public}s pnp get str is %{public}s", __func__, str);
            HdfSbufWriteInt32(reply, 1);
            break;
        }
        case HDF_PNP_MES_FAULT: {
            // test to crash process
            const char *str = NULL;
            HDF_LOGE("%{public}s test to crash procss is %{public}c", __func__, str[1]);
            break;
        }
        case HDF_PNP_MES_SECUREC: {
            if (g_gpioFlg == 0) {
                ret = HDF_FAILURE;
            } else {
                ret = HDF_SUCCESS;
            }
            break;
        }
        default: {
            HDF_LOGE("%{public}s failed, cmd id is %{public}d", __func__, cmdId);
            break;
        }
    }
    return ret;
}

int32_t HdfPnpDriverBind(struct HdfDeviceObject *para)
{
    HDF_LOGE("%{public}s", __func__);
    static struct IDeviceIoService virtualService = {
        .object.objectId = 1,
        .Dispatch = PnpDriverDispatch,
    };
    para->service = &virtualService;
    return 0;
}

int32_t HdfPnpDriverInit(struct HdfDeviceObject *para)
{
    HDF_LOGE("%{public}s enter!", __func__);
    if (para == NULL) {
        HDF_LOGE("%{public}s init failed, input is null", __func__);
        return HDF_FAILURE;
    }
    DeviceHandle *palHandle = DeviceHandleCreate(PAL_GPIO_TYPE, NULL);
    if (palHandle == NULL) {
        HDF_LOGE("%{public}s %{public}d open gpio failed", __func__, __LINE__);
        g_gpioFlg = 0;
    } else {
        HDF_LOGE("%{public}s %{public}d open gpio success", __func__, __LINE__);
        DeviceHandleDestroy(palHandle);
        g_gpioFlg = 1;
    }
    return 0;
}

void HdfPnpDriverRelease(struct HdfDeviceObject *para)
{
    HDF_LOGE("%{public}s enter!", __func__);
    if (para == NULL) {
        HDF_LOGE("%{public}s release failed, input is null", __func__);
        return;
    }
    para->service = NULL;
}

struct HdfDriverEntry g_sampleDriverEntry = {
    .moduleVersion = 1,
    .Bind = HdfPnpDriverBind,
    .Init = HdfPnpDriverInit,
    .Release = HdfPnpDriverRelease,
    .moduleName = "pnp_driver",
};

HDF_INIT(g_sampleDriverEntry);