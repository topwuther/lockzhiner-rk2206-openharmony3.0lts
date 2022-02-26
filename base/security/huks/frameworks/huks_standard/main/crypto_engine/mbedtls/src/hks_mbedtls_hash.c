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

#ifdef _CUT_AUTHENTICATE_
#undef HKS_SUPPORT_HASH_C
#endif /* _CUT_AUTHENTICATE_ */

#ifdef HKS_SUPPORT_HASH_C

#include "hks_mbedtls_hash.h"

#include <mbedtls/sha256.h>
#include <mbedtls/sha512.h>

#include "hks_common_check.h"
#include "hks_log.h"
#include "hks_mbedtls_common.h"

int32_t HksMbedtlsHash(uint32_t alg, const struct HksBlob *msg, struct HksBlob *hash)
{
    int32_t ret;
    switch (alg) {
        case HKS_DIGEST_SHA256:
            ret = mbedtls_sha256_ret(msg->data, msg->size, hash->data, 0); /* 0 for SHA-256 */
            break;
        case HKS_DIGEST_SHA384:
            ret = mbedtls_sha512_ret(msg->data, msg->size, hash->data, 1); /* 1 for SHA-384 */
            break;
        case HKS_DIGEST_SHA512:
            ret = mbedtls_sha512_ret(msg->data, msg->size, hash->data, 0); /* 0 for SHA-512 */
            break;
        default:
            return HKS_ERROR_INVALID_DIGEST;
    }

    if (ret != HKS_MBEDTLS_SUCCESS) {
        HKS_LOG_E("Mbedtls hash failed! mbedtls ret = 0x%X", ret);
        return ret;
    }

    ret = HksGetDigestLen(alg, &(hash->size));
    if (ret != HKS_SUCCESS) {
        HKS_LOG_E("Get digest len failed!");
    }

    return ret;
}
#endif /* HKS_SUPPORT_HASH_C */
