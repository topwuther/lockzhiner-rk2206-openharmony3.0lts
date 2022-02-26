/*
 * Copyright (C) 2021 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *    http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "hc_dev_info.h"
#include "hc_error.h"
#include "hc_log.h"
#include "securec.h"

#ifndef LITE_DEVICE
#include "parameter.h"
#endif

#ifdef __cplusplus
extern "C" {
#endif

int32_t HcGetUdid(uint8_t *udid, int32_t udidLen)
{
    if (udid == NULL || udidLen < INPUT_UDID_LEN || udidLen > MAX_INPUT_UDID_LEN) {
        return HAL_ERR_INVALID_PARAM;
    }
#ifndef LITE_DEVICE
    int32_t ret = GetDevUdid((char *)udid, udidLen);
    if (ret == 0) {
        return HAL_SUCCESS;
    }
#endif
    LOGD("using fake udid");
    const char *udidTemp = "ABCDEF00ABCDEF00ABCDEF00ABCDEF00ABCDEF00ABCDEF00ABCDEF00ABCDEF00";
    (void)memset_s(udid, udidLen, 0, udidLen);
    if (memcpy_s(udid, udidLen, udidTemp, strlen(udidTemp)) != EOK) {
        return HAL_FAILED;
    }
    return HAL_SUCCESS;
}

const char *GetStoragePath()
{
#ifndef LITE_DEVICE
    const char *storageFile = "/data/data/deviceauth/hcgroup.dat";
#else
    const char *storageFile = "/storage/deviceauth/hcgroup.dat";
#endif
    return storageFile;
}

#ifdef __cplusplus
}
#endif
