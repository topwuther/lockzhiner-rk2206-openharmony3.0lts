/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
 *
 * HDF is dual licensed: you can use it either under the terms of
 * the GPL, or the BSD license, at your option.
 * See the LICENSE file in the root of this repository for complete details.
 */

#include "audio_host_test.h"
#include "audio_host.h"

#define HDF_LOG_TAG audio_host_test

int32_t AudioKcontrolTestGetCodec(void)
{
    struct HdfDeviceObject *device = NULL;
    struct AudioHost *audioHost = NULL;
    HDF_LOGI("%s: enter", __func__);

    audioHost = AudioHostCreateAndBind(device);
    if (audioHost != NULL) {
        HDF_LOGE("%s: codecDevice is not NULL", __func__);
        return HDF_FAILURE;
    }

    HDF_LOGI("%s: success", __func__);
    return HDF_SUCCESS;
}

int32_t GetCardTestInstance(void)
{
    int i;
    const char *audioServiceName[] = {
        "codec_service_0",
        "codec_service_1",
        "dma_service_0",
        "dai_service",
        "audio_service_0",
        "audio_service_1",
        "render_service",
        "capture_service",
        "control_service",
    };
    HDF_LOGI("%s: enter", __func__);

    for (i = 0; i < sizeof(audioServiceName) / sizeof(audioServiceName[0]); ++i) {
        if (GetCardInstance(audioServiceName[i]) == NULL) {
            HDF_LOGE("%s: get %s fail!", __func__, audioServiceName[i]);
        }
    }

    HDF_LOGI("%s: success", __func__);
    return HDF_SUCCESS;
}

int32_t AudioHostTestDestroy(void)
{
    struct AudioHost *host = NULL;
    HDF_LOGI("%s: enter", __func__);

    AudioHostDestroy(host);
    HDF_LOGI("%s: success", __func__);
    return HDF_SUCCESS;
}
