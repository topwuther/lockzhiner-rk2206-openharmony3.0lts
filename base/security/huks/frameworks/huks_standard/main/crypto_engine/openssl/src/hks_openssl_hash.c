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

#include "hks_openssl_hash.h"

#include <openssl/evp.h>

#include "hks_log.h"
#include "hks_openssl_engine.h"
#include "hks_type_inner.h"

static int32_t CheckDigestAlg(uint32_t alg)
{
    switch (alg) {
        case HKS_DIGEST_SHA256:
        case HKS_DIGEST_SHA384:
        case HKS_DIGEST_SHA512:
            break;
        default:
            HKS_LOG_E("Unsupport HASH Type!");
            return HKS_ERROR_INVALID_DIGEST;
    }
    return HKS_SUCCESS;
}

static int32_t HashCheckParam(uint32_t alg, const struct HksBlob *msg, struct HksBlob *hash)
{
    if (CheckDigestAlg(alg) != HKS_SUCCESS) {
        HKS_LOG_E("Unsupport HASH Type!");
        return HKS_ERROR_INVALID_DIGEST;
    }
    if (HksOpensslCheckBlob(hash) != HKS_SUCCESS) {
        HKS_LOG_E("Invalid param hash!");
        return HKS_ERROR_INVALID_ARGUMENT;
    }
    if (HksOpensslCheckBlob(msg) != HKS_SUCCESS) {
        HKS_LOG_E("Invalid param msg!");
        return HKS_ERROR_INVALID_ARGUMENT;
    }
    return HKS_SUCCESS;
}

int32_t HksOpensslHash(uint32_t alg, const struct HksBlob *msg, struct HksBlob *hash)
{
    int32_t ret = HashCheckParam(alg, msg, hash);
    if (ret != HKS_SUCCESS) {
        HKS_LOG_E("Invalid Params!");
        return HKS_ERROR_INVALID_ARGUMENT;
    }

    const EVP_MD *opensslAlg = GetOpensslAlg(alg);
    if (opensslAlg == NULL) {
        HKS_LOG_E("get openssl algorithm fail");
        return HKS_ERROR_CRYPTO_ENGINE_ERROR;
    }

    ret = EVP_Digest(msg->data, msg->size, hash->data, &hash->size, opensslAlg, NULL);
    if (ret != HKS_OPENSSL_SUCCESS) {
        HksLogOpensslError();
        return HKS_ERROR_CRYPTO_ENGINE_ERROR;
    }
    return HKS_SUCCESS;
}
