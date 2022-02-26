/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
 *
 * HDF is dual licensed: you can use it either under the terms of
 * the GPL, or the BSD license, at your option.
 * See the LICENSE file in the root of this repository for complete details.
 */
#include "audio_stream_dispatch_test.h"
#include "audio_stream_dispatch.h"

#define HDF_LOG_TAG audio_host_test

int32_t AudioControlDispatchTestStreamDispatch(void)
{
    struct HdfDeviceIoClient *client = NULL;
    struct HdfSBuf *data = NULL;
    struct HdfSBuf *reply = NULL;
    int32_t cmdId = 0;
    int32_t ret;
    HDF_LOGI("%s: enter", __func__);

    ret = StreamDispatch(client, cmdId, data, reply);
    if (ret != HDF_SUCCESS) {
        HDF_LOGE("%s: (client || cmdId || data ||reply) is NULL", __func__);
    }

    HDF_LOGI("%s: success", __func__);
    return HDF_SUCCESS;
}

int32_t AudioControlDispatchTestStreamHostDestroy(void)
{
    struct StreamHost *host = NULL;
    HDF_LOGI("%s: enter", __func__);

    StreamHostDestroy(host);

    HDF_LOGI("%s: success", __func__);
    return HDF_SUCCESS;
}
