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

#include <hdf_base.h>
#include <hdf_log.h>
#include <osal_mem.h>
#include <hdf_device_desc.h>

#include "isample.h"

#define HDF_LOG_TAG sample_service_c

static int32_t SampleServiceDispatch(struct HdfDeviceIoClient *client, int cmdId,
    struct HdfSBuf *data, struct HdfSBuf *reply)
{
    return SampleServiceOnRemoteRequest(client, cmdId, data, reply);
}

void HdfSampleDriverRelease(struct HdfDeviceObject *deviceObject)
{
    struct IDeviceIoService *testService = deviceObject->service;
    OsalMemFree(testService);
}

int HdfSampleDriverBind(struct HdfDeviceObject *deviceObject)
{
    HDF_LOGE("HdfSampleDriverBind enter!");

    struct IDeviceIoService *ioService = (struct IDeviceIoService *)OsalMemAlloc(sizeof(struct IDeviceIoService));
    if (ioService == NULL) {
        HDF_LOGE("HdfSampleDriverBind OsalMemAlloc IDeviceIoService failed!");
        return HDF_FAILURE;
    }

    ioService->Dispatch = SampleServiceDispatch;
    ioService->Open = NULL;
    ioService->Release = NULL;

    deviceObject->service = ioService;
    return HDF_SUCCESS;
}

int HdfSampleDriverInit(struct HdfDeviceObject *deviceObject)
{
    HDF_LOGE("HdfSampleDriverCInit enter, new hdi impl");
    return HDF_SUCCESS;
}

struct HdfDriverEntry g_sampleDriverEntry = {
    .moduleVersion = 1,
    .moduleName = "sample_service_c",
    .Bind = HdfSampleDriverBind,
    .Init = HdfSampleDriverInit,
    .Release = HdfSampleDriverRelease,
};

HDF_INIT(g_sampleDriverEntry);