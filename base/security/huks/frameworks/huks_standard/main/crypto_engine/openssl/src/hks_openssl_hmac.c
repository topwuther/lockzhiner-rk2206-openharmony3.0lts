/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
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

#include "hks_openssl_hmac.h"

#include <openssl/evp.h>
#include <openssl/hmac.h>

#include "hks_log.h"
#include "hks_openssl_engine.h"
#include "hks_type_inner.h"

static int32_t HmacCheckBuffer(const struct HksBlob *key, const struct HksBlob *msg, const struct HksBlob *mac)
{
    if (HksOpensslCheckBlob(key) != HKS_SUCCESS) {
        HKS_LOG_E("Invalid key point");
        return HKS_ERROR_INVALID_ARGUMENT;
    }
    if (HksOpensslCheckBlob(msg) != HKS_SUCCESS) {
        HKS_LOG_E("Invalid msg");
        return HKS_ERROR_INVALID_ARGUMENT;
    }
    if (HksOpensslCheckBlob(mac) != HKS_SUCCESS) {
        HKS_LOG_E("Invalid mac");
        return HKS_ERROR_INVALID_ARGUMENT;
    }
    return HKS_SUCCESS;
}

static uint32_t HmacGetDigestLen(uint32_t alg)
{
    switch (alg) {
        case HKS_DIGEST_SHA256:
            return HKS_DIGEST_SHA256_LEN;
        case HKS_DIGEST_SHA384:
            return HKS_DIGEST_SHA384_LEN;
        default:
            return HKS_DIGEST_SHA512_LEN;
    }
}

static int32_t HmacCheckParam(const struct HksBlob *key, uint32_t alg,
    const struct HksBlob *msg, const struct HksBlob *mac)
{
    if (HmacCheckBuffer(key, msg, mac) != HKS_SUCCESS) {
        HKS_LOG_E("Invalid Buffer Info");
        return HKS_ERROR_INVALID_ARGUMENT;
    }
    if ((alg != HKS_DIGEST_SHA256) && (alg != HKS_DIGEST_SHA384) && (alg != HKS_DIGEST_SHA512)) {
        HKS_LOG_E("Invalid alg(0x%x)", alg);
        return HKS_ERROR_INVALID_ARGUMENT;
    }

    uint32_t digestLen = HmacGetDigestLen(alg);
    if (mac->size < digestLen) {
        HKS_LOG_E("invalid mac->size(0x%x) for digestLen(0x%x)", mac->size, digestLen);
        return HKS_ERROR_INVALID_ARGUMENT;
    }
    return HKS_SUCCESS;
}

int32_t HksOpensslHmac(const struct HksBlob *key, uint32_t digestAlg, const struct HksBlob *msg,
    struct HksBlob *mac)
{
    if (HmacCheckParam(key, digestAlg, msg, mac) != HKS_SUCCESS) {
        return HKS_ERROR_INVALID_ARGUMENT;
    }

    const EVP_MD *opensslAlg = GetOpensslAlg(digestAlg);
    if (opensslAlg == NULL) {
        HKS_LOG_E("get openssl algorithm failed");
        return HKS_ERROR_CRYPTO_ENGINE_ERROR;
    }

    uint8_t *hmacData = HMAC(opensslAlg, key->data, (int32_t)key->size, msg->data, msg->size, mac->data, &mac->size);
    if (hmacData == NULL) {
        HKS_LOG_E("hmac process failed.");
        return HKS_ERROR_CRYPTO_ENGINE_ERROR;
    }
    return HKS_SUCCESS;
}
