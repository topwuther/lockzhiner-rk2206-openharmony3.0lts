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

#include "hdf_device_full.h"
#include "device_service_stub.h"
#include "hdf_base.h"
#include "hdf_device_info_full.h"
#include "hdf_log.h"
#include "osal_mem.h"

#define HDF_LOG_TAG hdf_device_full

struct HdfDeviceFull *HdfDeviceFullReinterpretCast(struct IHdfDevice *device)
{
    return (struct HdfDeviceFull *)device;
}

static int HdfDeviceFullAttach(
    struct IHdfDevice *device, struct HdfDeviceNode *deviceService)
{
    struct DeviceThread *deviceThread = NULL;
    struct HdfDeviceFull *fullDevice = HdfDeviceFullReinterpretCast(device);
    if (fullDevice == NULL) {
        HDF_LOGE("HdfDeviceFullAttach failed, fullDevice is null");
        return HDF_FAILURE;
    }

    if (fullDevice->deviceThread == NULL) {
        fullDevice->deviceThread = DeviceThreadNewInstance();
    }

    deviceThread = fullDevice->deviceThread;
    if (deviceThread != NULL) {
        struct HdfThread *thread = (struct HdfThread *)&deviceThread->super;
        if (!thread->IsRunning(thread)) {
            thread->Start(thread);
        }
        // modified list operation.
        DListInsertTail(&deviceService->entry, &fullDevice->super.devNodes);
        DeviceThreadAttach(deviceThread, device, deviceService);
    }
    return HDF_SUCCESS;
}

static void HdfDeviceFullDettach(struct IHdfDevice *device, struct HdfDeviceNode *devNode)
{
    struct HdfDeviceFull *fullDevice = HdfDeviceFullReinterpretCast(device);
    if (fullDevice == NULL || devNode == NULL) {
        HDF_LOGE("%{public}s input is null", __func__);
        return;
    }

    if (devNode->driverEntry != NULL && devNode->driverEntry->Release != NULL) {
        devNode->driverEntry->Release(&devNode->deviceObject);
    }
    struct DevHostService *hostService = devNode->hostService;
    if (hostService != NULL) {
        HdfServiceObserverRemoveRecord(&hostService->observer, devNode->deviceInfo->svcName);
    }
    HdfDeviceInfoFullFreeInstance((struct HdfDeviceInfoFull *)devNode->deviceInfo);
    DeviceServiceStubRelease(&devNode->super.object);

    if (fullDevice->deviceThread != NULL) {
        fullDevice->deviceThread->looper.Stop(&fullDevice->deviceThread->looper);
        DeviceThreadFreeInstance(fullDevice->deviceThread);
        fullDevice->deviceThread = NULL;
    }
}

void HdfDeviceFullConstruct(struct HdfDeviceFull *inst)
{
    if (inst != NULL) {
        struct IHdfDevice *deviceIf = (struct IHdfDevice *)&inst->super;
        HdfDeviceConstruct(&inst->super);
        deviceIf->Attach = HdfDeviceFullAttach;
        deviceIf->Detach = HdfDeviceFullDettach;
    }
}

struct HdfObject *HdfDeviceFullCreate()
{
    struct HdfDeviceFull *device =
        (struct HdfDeviceFull *)OsalMemCalloc(sizeof(struct HdfDeviceFull));
    if (device != NULL) {
        HdfDeviceFullConstruct(device);
    }
    return (struct HdfObject *)device;
}


void HdfDeviceFullRelease(struct HdfObject *object)
{
    struct HdfDeviceFull *fullDevice = (struct HdfDeviceFull *)object;
    if (fullDevice != NULL) {
        HdfDeviceDestruct(&fullDevice->super);
        if (fullDevice->deviceThread != NULL) {
            DeviceThreadFreeInstance(fullDevice->deviceThread);
        }
        OsalMemFree(fullDevice);
    }
}

