/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
 *
 * HDF is dual licensed: you can use it either under the terms of
 * the GPL, or the BSD license, at your option.
 * See the LICENSE file in the root of this repository for complete details.
 */
#include "audio_sapm_test.h"
#include "audio_sapm.h"

#define HDF_LOG_TAG audio_host_test

int32_t AudioSapmTestNewComponents(void)
{
    struct AudioCard *audioCard = NULL;
    struct AudioSapmComponent *component = NULL;
    int32_t maxNum = 0;
    int32_t ret;
    HDF_LOGI("%s: enter", __func__);

    ret = AudioSapmNewComponents(audioCard, component, maxNum);
    if (ret != HDF_SUCCESS) {
        HDF_LOGE("%s: audioCart or component is NULL", __func__);
    }

    HDF_LOGI("%s: success", __func__);
    return HDF_SUCCESS;
}

int32_t AudioSapmTestAddRoutes(void)
{
    struct AudioCard *audioCard = NULL;
    struct AudioSapmRoute *route = NULL;
    int32_t routeMaxNum = 0;
    int32_t ret;
    HDF_LOGI("%s: enter", __func__);

    ret = AudioSapmAddRoutes(audioCard, route, routeMaxNum);
    if (ret != HDF_SUCCESS) {
        HDF_LOGE("%s: audioCard or route is NULL", __func__);
    }

    HDF_LOGI("%s: success", __func__);
    return HDF_SUCCESS;
}

int32_t AudioSapmTestNewControls(void)
{
    struct AudioCard *audioCard = NULL;
    int32_t ret;
    HDF_LOGI("%s: enter", __func__);

    ret = AudioSapmNewControls(audioCard);
    if (ret != HDF_SUCCESS) {
        HDF_LOGE("%s: audioCard is NULL", __func__);
    }

    HDF_LOGI("%s: success", __func__);
    return HDF_SUCCESS;
}

int32_t AudioSapmTestPowerComponents(void)
{
    struct AudioCard *audioCard = NULL;
    int32_t ret;
    HDF_LOGI("%s: enter", __func__);

    ret = AudioSapmPowerComponents(audioCard);
    if (ret != HDF_SUCCESS) {
        HDF_LOGE("%s: audioCard is NULL", __func__);
    }

    HDF_LOGI("%s: success", __func__);
    return HDF_SUCCESS;
}


int32_t AudioSapmTestRefreshTime(void)
{
    bool bRefresh = true;
    u64 time;
    HDF_LOGI("%s: enter", __func__);

    time =  AudioSapmRefreshTime(bRefresh);
    if (time == 0) {
        HDF_LOGE("%s: error", __func__);
    }

    HDF_LOGI("%s: success", __func__);
    return HDF_SUCCESS;
}

