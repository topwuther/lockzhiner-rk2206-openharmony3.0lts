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
#undef HKS_SUPPORT_ECDSA_C
#endif /* _CUT_AUTHENTICATE_ */

#ifdef HKS_SUPPORT_ECDSA_C

#include "hks_mbedtls_ecdsa.h"

#include <mbedtls/ctr_drbg.h>
#include <mbedtls/ecdsa.h>
#include <mbedtls/ecp.h>
#include <mbedtls/entropy.h>

#include "hks_log.h"
#include "hks_mbedtls_common.h"
#include "hks_mbedtls_ecc.h"

/* users must ensure the input params not null */
int32_t HksMbedtlsEcdsaSign(const struct HksBlob *key, const struct HksUsageSpec *usageSpec,
    const struct HksBlob *message, struct HksBlob *signature)
{
    int32_t ret = EccKeyCheck(key);
    if (ret != HKS_SUCCESS) {
        return ret;
    }

    mbedtls_ecp_group_id curveNist = MBEDTLS_ECP_DP_NONE;
    ret = HksMbedtlsEccGetKeyCurveNist((struct KeyMaterialEcc *)(key->data), &curveNist);
    if (ret != HKS_SUCCESS) {
        return ret;
    }

    mbedtls_ctr_drbg_context ctrDrbg;
    mbedtls_entropy_context entropy;
    ret = HksCtrDrbgSeed(&ctrDrbg, &entropy);
    if (ret != HKS_SUCCESS) {
        return ret;
    }

    mbedtls_ecdsa_context ctx;
    mbedtls_ecdsa_init(&ctx);

    do {
        ret = mbedtls_ecp_group_load(&(ctx.grp), curveNist);
        if (ret != HKS_MBEDTLS_SUCCESS) {
            HKS_LOG_E("Mbedtls ecp group load fail! mbedtls ret = 0x%X", ret);
            break;
        }

        ret = HksEccKeyMaterialToPri(key, &(ctx.d));
        if (ret != HKS_SUCCESS) {
            break;
        }

        uint32_t mbedtlsAlg;
        ret = HksToMbedtlsDigestAlg(usageSpec->digest, &mbedtlsAlg);
        if (ret != HKS_SUCCESS) {
            break;
        }

        ret = mbedtls_ecdsa_write_signature(&ctx, mbedtlsAlg, message->data, (size_t)message->size,
            signature->data, (size_t *)&(signature->size), mbedtls_ctr_drbg_random, &ctrDrbg);
        if (ret != HKS_MBEDTLS_SUCCESS) {
            HKS_LOG_E("Ecc mbedtls sign fail! mbedtls ret = 0x%X", ret);
            (void)memset_s(signature->data, signature->size, 0, signature->size);
        }
    } while (0);

    mbedtls_ctr_drbg_free(&ctrDrbg);
    mbedtls_entropy_free(&entropy);
    mbedtls_ecdsa_free(&ctx);
    return ret;
}

/* users must ensure the input params not null */
int32_t HksMbedtlsEcdsaVerify(const struct HksBlob *key, const struct HksUsageSpec *usageSpec,
    const struct HksBlob *message, const struct HksBlob *signature)
{
    (void)usageSpec;
    int32_t ret = EccKeyCheck(key);
    if (ret != HKS_SUCCESS) {
        return ret;
    }

    mbedtls_ecp_group_id curveNist = MBEDTLS_ECP_DP_NONE;
    ret = HksMbedtlsEccGetKeyCurveNist((struct KeyMaterialEcc *)(key->data), &curveNist);
    if (ret != HKS_SUCCESS) {
        HKS_LOG_E("Get ecc KeyCureNist fail! ret = 0x%X", ret);
        return ret;
    }

    mbedtls_ecdsa_context ctx;
    mbedtls_ecdsa_init(&ctx);

    do {
        ret = mbedtls_ecp_group_load(&(ctx.grp), curveNist);
        if (ret != HKS_MBEDTLS_SUCCESS) {
            HKS_LOG_E("Mbedtls ecp group load fail! mbedtls ret = 0x%X", ret);
            break;
        }

        ret = HksEccKeyMaterialToPub(key, &(ctx.Q));
        if (ret != HKS_SUCCESS) {
            break;
        }

        ret = mbedtls_ecdsa_read_signature(&ctx,
            message->data, message->size, signature->data, signature->size);
        if (ret != HKS_MBEDTLS_SUCCESS) {
            HKS_LOG_E("Ecc mbedtls verify fail! mbedtls ret = 0x%X", ret);
        }
    }while (0);

    mbedtls_ecdsa_free(&ctx);
    return ret;
}
#endif /* HKS_SUPPORT_ECDSA_C */
