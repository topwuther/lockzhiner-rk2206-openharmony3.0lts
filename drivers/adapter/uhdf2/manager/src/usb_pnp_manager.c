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

#include "usb_pnp_manager.h"
#include "devhost_service_clnt.h"
#include "devmgr_pnp_service.h"
#include "hdf_base.h"
#include "hdf_log.h"
#include "osal_mem.h"
#include "osal_time.h"
#include "securec.h"
#include "usb_ddk_pnp_loader.h"

#define HDF_LOG_TAG usb_pnp_manager

#define WAIT_PNP_SLEEP_TIME 10 // ms
#define WAIT_PNP_SLEEP_CNT  100

static int32_t UsbPnpManageStartPnpHost(const struct IDevmgrService *devmgrSvc)
{
    int32_t ret;
    int32_t i = 0;
    struct DevmgrService *inst = (struct DevmgrService *)devmgrSvc;
    if (inst == NULL) {
        return HDF_ERR_INVALID_PARAM;
    }
    struct DevHostServiceClnt *hostClnt = DevmgrServiceGetPnpHostClnt(inst);
    if (hostClnt == NULL) {
        ret = DevmgrServiceStartPnpHost(inst);
        if (ret != HDF_SUCCESS) {
            HDF_LOGE("%s:%d add pnp device failed!", __func__, __LINE__);
            return ret;
        }
        OsalMSleep(WAIT_PNP_SLEEP_TIME);
        hostClnt = DevmgrServiceGetPnpHostClnt(inst);
    }

    while (hostClnt == NULL || hostClnt->hostService == NULL) {
        OsalMSleep(WAIT_PNP_SLEEP_TIME);
        i++;
        if (i > WAIT_PNP_SLEEP_CNT) {
            HDF_LOGE("%s:%d!", __func__, __LINE__);
            break;
        }
    }

    return HDF_SUCCESS;
}

bool UsbPnpManagerWriteModuleName(struct HdfSBuf *sbuf, const char *moduleName)
{
    char modName[128] = {0};

    if (sprintf_s(modName, sizeof(modName) - 1, "lib%s.z.so", moduleName) < 0) {
        HDF_LOGE("%s: sprintf_s modName fail", __func__);
        return false;
    }

    return HdfSbufWriteString(sbuf, modName);
}

int32_t UsbPnpManagerRegisterOrUnregisterDevice(struct UsbPnpManagerDeviceInfo managerInfo)
{
    int32_t ret;

    ret = UsbPnpManageStartPnpHost(managerInfo.devmgrSvc);
    if (ret != HDF_SUCCESS) {
        HDF_LOGE("%s:%d privateData is NULL!", __func__, __LINE__);
        return ret;
    }

    if (managerInfo.isReg) {
        ret = DevmgrServiceRegPnpDevice(managerInfo.devmgrSvc, managerInfo.moduleName, managerInfo.serviceName,
            managerInfo.deviceMatchAttr, (const void *)managerInfo.privateData);
    } else {
        ret = DevmgrServiceUnRegPnpDevice(managerInfo.devmgrSvc, managerInfo.moduleName, managerInfo.serviceName);
    }

    return ret;
}

int DevmgrUsbPnpManageEventHandle(struct IDevmgrService *inst)
{
    int status;
    struct HdfIoService *usbPnpServ = HdfIoServiceBind(USB_PNP_NOTIFY_SERVICE_NAME);
    static struct HdfDevEventlistener usbPnpListener = {
        .callBack = UsbDdkPnpLoaderEventReceived,
    };
    usbPnpListener.priv = (void *)(inst);

    if (usbPnpServ == NULL) {
        HDF_LOGE("%s: HdfIoServiceBind faile.", __func__);
        return HDF_ERR_INVALID_OBJECT;
    }

    status = HdfDeviceRegisterEventListener(usbPnpServ, &usbPnpListener);
    if (status != HDF_SUCCESS) {
        HDF_LOGE("HdfDeviceRegisterEventListener faile status=%d", status);
        return status;
    }

    return UsbDdkPnpLoaderEventHandle();
}

bool DevmgrUsbPnpManageAddPrivateData(struct HdfDeviceInfoFull *deviceInfo, const void *privateData)
{
    int ret;

    if (privateData != NULL) {
        uint32_t length = ((struct HdfPrivateInfo *)(privateData))->length;
        deviceInfo->super.private = (const void *)OsalMemCalloc(length);
        if (deviceInfo->super.private != NULL) {
            ret = memcpy_s((void *)(deviceInfo->super.private), length, privateData, length);
            if ((ret != EOK) || (deviceInfo->super.private == NULL)) {
                HDF_LOGE("%s: memcpy_s private error", __func__);
                return false;
            }
        } else {
            HDF_LOGE("%s: OsalMemCalloc private error", __func__);
            return false;
        }
    }

    return true;
}
