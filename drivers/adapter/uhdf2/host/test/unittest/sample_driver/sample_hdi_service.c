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
#include <hdf_sbuf.h>
#include <hdf_remote_service.h>
#include <hdf_log.h>
#include "sample_hdi.h"

int32_t SampleServicePing(struct HdfDeviceObject *device, const char* info, char** infoOut)
{
    (void)device;
    HDF_LOGI("Sample:info is %{public}s", info);
    *infoOut = strdup(info);
    return 0;
}

int32_t SampleServiceSum(struct HdfDeviceObject *device, int32_t x0, int32_t x1, int32_t *result)
{
    (void)device;
    *result = x0 + x1;

    return 0;
}

int32_t SampleServiceCallback(struct HdfDeviceObject *device, struct HdfRemoteService *callback, int32_t code)
{
    (void)device;
    struct HdfSBuf *dataSbuf = HdfSBufTypedObtain(SBUF_IPC);
    HdfSbufWriteInt32(dataSbuf, code);
    int ret = callback->dispatcher->Dispatch(callback, 0, dataSbuf, NULL);
    if (ret != HDF_SUCCESS) {
        HDF_LOGE("failed to do callback, ret = %{public}d", ret);
    }
    HdfSBufRecycle(dataSbuf);
    return ret;
}

static const struct SampleHdi g_sampleHdiImpl  = {
    .ping = SampleServicePing,
    .sum = SampleServiceSum,
    .callback = SampleServiceCallback,
};

const struct SampleHdi *SampleHdiImplInstance()
{
    return &g_sampleHdiImpl;
}