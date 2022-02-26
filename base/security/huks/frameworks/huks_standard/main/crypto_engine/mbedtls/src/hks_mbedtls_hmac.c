/*
 * Copyright (c) 2020-2021 Huawei Device Co., Ltd.
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

#ifdef HKS_CONFIG_FILE
#include HKS_CONFIG_FILE
#else
#include "hks_config.h"
#endif

#ifdef HKS_SUPPORT_HMAC_C

#include "hks_mbedtls_hmac.h"

#include <mbedtls/md.h>

#include "hks_common_check.h"
#include "hks_log.h"
#include "hks_mbedtls_common.h"

int32_t HksMbedtlsHmac(const struct HksBlob *key,
    uint32_t digestAlg, const struct HksBlob *msg, struct HksBlob *mac)
{
    /* input params have been checked */
    uint32_t mbedtlsAlg;
    int32_t ret = HksToMbedtlsDigestAlg(digestAlg, &mbedtlsAlg);
    if (ret != HKS_SUCCESS) {
        return ret;
    }

    ret = mbedtls_md_hmac(mbedtls_md_info_from_type(mbedtlsAlg),
        key->data, key->size, msg->data, msg->size, mac->data);
    if (ret != HKS_MBEDTLS_SUCCESS) {
        HKS_LOG_E("Mbedtls hmac failed! mbedtls ret = 0x%X", ret);
        (void)memset_s(mac->data, mac->size, 0, mac->size);
        return ret;
    }

    ret = HksGetDigestLen(digestAlg, &(mac->size));
    if (ret != HKS_SUCCESS) {
        HKS_LOG_E("Get digest len failed!");
    }

    return ret;
}
#endif /* HKS_SUPPORT_HMAC_C */
