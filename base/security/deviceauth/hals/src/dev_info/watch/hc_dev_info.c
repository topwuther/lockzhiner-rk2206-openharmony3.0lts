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
#include "common_util.h"
#include "hc_error.h"
#include "hc_log.h"
#include "hc_types.h"

#define UDID_BYTE_LEN_IN_L0_DEVICE 32

extern uint8_t GetUdid(uint8_t *buf, uint8_t length);

int32_t HcGetUdid(uint8_t *udid, int32_t udidLen)
{
    if (udid == NULL || udidLen < INPUT_UDID_LEN || udidLen > MAX_INPUT_UDID_LEN) {
        return HAL_ERR_INVALID_PARAM;
    }
    uint8_t *udidInner = (uint8_t *)HcMalloc(UDID_BYTE_LEN_IN_L0_DEVICE, 0);
    if (udidInner == NULL) {
        return HAL_ERR_BAD_ALLOC;
    }
    uint8_t resultLen = GetUdid(udidInner, UDID_BYTE_LEN_IN_L0_DEVICE);
    if (resultLen != UDID_BYTE_LEN_IN_L0_DEVICE) {
        HcFree(udidInner);
        LOGE("GetUdid in L0 device failed, resultLen:%u", resultLen);
        return HAL_FAILED;
    }
    int32_t result = ByteToHexString(udidInner, UDID_BYTE_LEN_IN_L0_DEVICE, udid, udidLen);
    HcFree(udidInner);
    if (result != HAL_SUCCESS) {
        LOGE("ByteToHexString for udid failed, result:%d", result);
    }
    return result;
}

const char *GetStoragePath()
{
    return "user/Hichain/hcgroup.dat";
}