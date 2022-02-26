/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
 *
 * HDF is dual licensed: you can use it either under the terms of
 * the GPL, or the BSD license, at your option.
 * See the LICENSE file in the root of this repository for complete details.
 */

#include "audio_parse_test.h"
#include "audio_parse.h"

#define HDF_LOG_TAG audio_parse_test

int32_t AudioFillTestConfigData(void)
{
    int32_t ret;
    struct HdfDeviceObject *device = NULL;
    struct AudioConfigData *configData = NULL;
    HDF_LOGI("%s: enter", __func__);

    ret = AudioFillConfigData(device, configData);
    if (ret == HDF_SUCCESS) {
        HDF_LOGE("%s: AudioFillConfigData fail! ret = %d", __func__, ret);
        return HDF_FAILURE;
    }

    HDF_LOGI("%s: success", __func__);
    return HDF_SUCCESS;
}
