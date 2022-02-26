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

#include "usbfn_device.h"
#include "usbfn_request.h"
#include "usbfn_interface.h"
#include "usbfn_io_mgr.h"
#include "usbfn_dev_mgr.h"
#include "usbfn_cfg_mgr.h"

#define HDF_LOG_TAG usbfn_sdk_if

static int IsDescriptorOk(struct UsbFnDeviceDesc *des)
{
    int i, j;
    struct UsbFnStrings **strings = NULL;
    struct UsbFnFunction *functions = NULL;
    if (des == NULL) {
        HDF_LOGE("%s: des null", __func__);
        goto ERR_DES;
    }
    if (des->deviceDesc == NULL || des->deviceStrings == NULL || des->configs == NULL) {
        HDF_LOGE("%s: deviceDesc  deviceStrings configs null", __func__);
        goto ERR_DES;
    }

    strings = des->deviceStrings;
    if (strings[0] == NULL) {
        HDF_LOGE("%s: strings null", __func__);
        goto ERR_DES;
    }

    for (i = 0; des->configs[i] != NULL; i++) {
        for (j = 0; des->configs[i]->functions[j] != NULL; j++) {
            functions = des->configs[i]->functions[j];
            if (functions->fsDescriptors == NULL) {
                HDF_LOGE("%s: fsDescriptors null", __func__);
                goto ERR_DES;
            }
        }
    }
    if (i == 0 || j == 0) {
        HDF_LOGE("%s: configs functions null", __func__);
        goto ERR_DES;
    }

    return 0;
ERR_DES:
    HDF_LOGE("%s: DeviceDesc bad", __func__);
    return HDF_ERR_INVALID_PARAM;
}

const struct UsbFnDevice *UsbFnCreateDevice(const char *udcName,
    const struct UsbFnDescriptorData *descriptor)
{
    int ret;
    const struct DeviceResourceNode *property = NULL;
    struct UsbFnDeviceDesc *des = NULL;

    if (udcName == NULL || descriptor == NULL) {
        HDF_LOGE("%s: INVALID PARAM", __func__);
        return NULL;
    }
    if (descriptor->type == USBFN_DESC_DATA_TYPE_PROP) {
        property = descriptor->property;
        des = UsbFnCfgMgrGetInstanceFromHCS(property);
    } else {
        des = descriptor->descriptor;
    }
    ret = IsDescriptorOk(des);
    if (ret) {
        return NULL;
    }

    return (struct UsbFnDevice *)UsbFnMgrDeviceCreate(udcName, des, property);
}

int UsbFnRemoveDevice(struct UsbFnDevice *fnDevice)
{
    if (fnDevice == NULL) {
        HDF_LOGE("%s: INVALID PARAM", __func__);
        return HDF_ERR_INVALID_PARAM;
    }
    return UsbFnMgrDeviceRemove(fnDevice);
}

const struct UsbFnDevice *UsbFnGetDevice(const char *udcName)
{
    if (udcName == NULL) {
        HDF_LOGE("%s: INVALID PARAM", __func__);
        return NULL;
    }
    return (struct UsbFnDevice *)UsbFnMgrDeviceGet((const char *)udcName);
}

int UsbFnGetDeviceState(struct UsbFnDevice *fnDevice, UsbFnDeviceState *devState)
{
    if (fnDevice == NULL || devState == NULL) {
        HDF_LOGE("%s: INVALID PARAM", __func__);
        return HDF_ERR_INVALID_PARAM;
    }
    return UsbFnMgrDeviceGetState(fnDevice, devState);
}

const struct UsbFnInterface *UsbFnGetInterface(struct UsbFnDevice *fnDevice,
    uint8_t interfaceIndex)
{
    if (fnDevice == NULL) {
        HDF_LOGE("%s: INVALID PARAM", __func__);
        return NULL;
    }
    return (struct UsbFnInterface *)UsbFnMgrDeviceGetInterface(fnDevice, interfaceIndex);
}

int UsbFnStartRecvInterfaceEvent(struct UsbFnInterface *interface,
    uint32_t eventMask, UsbFnEventCallback callback, void *context)
{
    if (interface == NULL || eventMask == 0 || callback == NULL) {
        HDF_LOGE("%s: INVALID_PARAM", __func__);
        return HDF_ERR_INVALID_PARAM;
    }
    return UsbFnMgrStartRecvEvent(interface, eventMask, callback, context);
}

int UsbFnStopRecvInterfaceEvent(struct UsbFnInterface *interface)
{
    if (interface == NULL) {
        HDF_LOGE("%s: INVALID PARAM", __func__);
        return HDF_ERR_INVALID_PARAM;
    }
    return UsbFnStopRecvEvent(interface);
}

UsbFnInterfaceHandle UsbFnOpenInterface(struct UsbFnInterface *interface)
{
    if (interface == NULL) {
        HDF_LOGE("%s: INVALID PARAM", __func__);
        return NULL;
    }
    return (UsbFnInterfaceHandle)UsbFnIoMgrInterfaceOpen(interface);
}

int UsbFnCloseInterface(UsbFnInterfaceHandle handle)
{
    if (handle == NULL) {
        HDF_LOGE("%s: INVALID PARAM", __func__);
        return HDF_ERR_INVALID_PARAM;
    }
    return UsbFnIoMgrInterfaceClose((struct UsbHandleMgr *)handle);
}

int UsbFnGetInterfacePipeInfo(struct UsbFnInterface *interface,
    uint8_t pipeId, struct UsbFnPipeInfo *info)
{
    if (info == NULL || interface == NULL) {
        HDF_LOGE("%s: INVALID PARAM", __func__);
        return HDF_ERR_INVALID_PARAM;
    }
    return UsbFnIoMgrInterfaceGetPipeInfo(interface, pipeId, info);
}

int UsbFnRegistInterfaceProp(const struct UsbFnInterface *interface,
    const struct UsbFnRegistInfo *registInfo)
{
    if (registInfo == NULL || interface == NULL) {
        HDF_LOGE("%s: INVALID PARAM", __func__);
        return HDF_ERR_INVALID_PARAM;
    }
    return UsbFnCfgMgrRegisterProp(interface, registInfo);
}

int UsbFnGetInterfaceProp(const struct UsbFnInterface *interface,
    const char *name, char *value)
{
    if (name == NULL || interface == NULL) {
        HDF_LOGE("%s: INVALID PARAM", __func__);
        return HDF_ERR_INVALID_PARAM;
    }
    return UsbFnCfgMgrGetProp(interface, name, value);
}

int UsbFnSetInterfaceProp(const struct UsbFnInterface *interface,
    const char *name, const char *value)
{
    if (name == NULL || interface == NULL) {
        HDF_LOGE("%s: INVALID PARAM", __func__);
        return HDF_ERR_INVALID_PARAM;
    }
    return UsbFnCfgMgrSetProp(interface, name, value);
}

struct UsbFnRequest *UsbFnAllocRequest(UsbFnInterfaceHandle handle, uint8_t pipe, uint32_t len)
{
    struct UsbHandleMgr *handleMgr = handle;
    if (handle == NULL || len > MAX_BUFLEN || len == 0 || pipe >= handleMgr->numFd) {
        HDF_LOGE("%s: INVALID PARAM", __func__);
        return NULL;
    }
    return UsbFnIoMgrRequestAlloc(handleMgr, pipe + 1, len);
}

struct UsbFnRequest *UsbFnAllocCtrlRequest(UsbFnInterfaceHandle handle, uint32_t len)
{
    struct UsbHandleMgr *handleMgr = handle;
    if (handle == NULL || len > MAX_BUFLEN || len == 0) {
        HDF_LOGE("%s: INVALID PARAM", __func__);
        return NULL;
    }
    return UsbFnIoMgrRequestAlloc(handleMgr, 0, len);
}

int UsbFnFreeRequest(struct UsbFnRequest *req)
{
    if (req == NULL) {
        HDF_LOGE("%s: INVALID PARAM", __func__);
        return HDF_ERR_INVALID_PARAM;
    }
    return  UsbFnIoMgrRequestFree(req);
}

int UsbFnGetRequestStatus(struct UsbFnRequest *req, UsbRequestStatus *status)
{
    if (req == NULL || status == NULL) {
        HDF_LOGE("%s: INVALID PARAM", __func__);
        return HDF_ERR_INVALID_PARAM;
    }
    return UsbFnIoMgrRequestGetStatus(req, status);
}

int UsbFnSubmitRequestAsync(struct UsbFnRequest *req)
{
    if (req == NULL) {
        HDF_LOGE("%s: INVALID PARAM", __func__);
        return HDF_ERR_INVALID_PARAM;
    }
    return UsbFnIoMgrRequestSubmitAsync(req);
}

int UsbFnCancelRequest(struct UsbFnRequest *req)
{
    if (req == NULL) {
        HDF_LOGE("%s: INVALID PARAM", __func__);
        return HDF_ERR_INVALID_PARAM;
    }
    return UsbFnIoMgrRequestCancel(req);
}

int UsbFnSubmitRequestSync(struct UsbFnRequest *req, uint32_t timeout)
{
    if (req == NULL) {
        HDF_LOGE("%s: INVALID PARAM", __func__);
        return HDF_ERR_INVALID_PARAM;
    }
    return UsbFnIoMgrRequestSubmitSync(req, timeout);
}

int UsbFnMemTestTrigger(bool enable)
{
    return UsbFnAdpMemTestTrigger(enable);
}

