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
#include <hdf_log.h>
#include <hdf_base.h>
#include <osal_mem.h>
#include <hdf_device_desc.h>
#include "sample_service_stub.h"

#define HDF_LOG_TAG sample_service_cpp

using namespace OHOS::HDI::Sample::V1_0;

struct HdfSampleService {
    struct IDeviceIoService ioservice;
    void *instance;
};

static int32_t SampleServiceDispatch(struct HdfDeviceIoClient *client, int cmdId,
    struct HdfSBuf *data, struct HdfSBuf *reply)
{
    struct HdfSampleService *hdfSampleService = CONTAINER_OF(
        client->device->service, struct HdfSampleService, ioservice);
    return SampleServiceOnRemoteRequest(hdfSampleService->instance, cmdId, *data, *reply);
}

int HdfSampleDriverInit(struct HdfDeviceObject *deviceObject)
{
    HDF_LOGE("HdfSampleDriverInit enter, new hdi impl");
    return HDF_SUCCESS;
}

int HdfSampleDriverBind(struct HdfDeviceObject *deviceObject)
{
    HDF_LOGI("HdfSampleDriverBind enter!");

    struct HdfSampleService *hdfSampleService = (struct HdfSampleService *)OsalMemAlloc(
        sizeof(struct HdfSampleService));
    if (hdfSampleService == nullptr) {
        HDF_LOGE("HdfSampleDriverBind OsalMemAlloc HdfSampleService failed!");
        return HDF_FAILURE;
    }

    hdfSampleService->ioservice.Dispatch = SampleServiceDispatch;
    hdfSampleService->ioservice.Open = NULL;
    hdfSampleService->ioservice.Release = NULL;
    hdfSampleService->instance = SampleStubInstance();

    deviceObject->service = &hdfSampleService->ioservice;
    return HDF_SUCCESS;
}

void HdfSampleDriverRelease(struct HdfDeviceObject *deviceObject)
{
    struct HdfSampleService *hdfSampleService = CONTAINER_OF(deviceObject->service, struct HdfSampleService, ioservice);
    SampleStubRelease(hdfSampleService->instance);
    OsalMemFree(hdfSampleService);
}

struct HdfDriverEntry g_sampleDriverEntry = {
    .moduleVersion = 1,
    .moduleName = "sample_service_cpp",
    .Bind = HdfSampleDriverBind,
    .Init = HdfSampleDriverInit,
    .Release = HdfSampleDriverRelease,
};

#ifndef __cplusplus
extern "C" {
#endif

HDF_INIT(g_sampleDriverEntry);

#ifndef __cplusplus
}
#endif