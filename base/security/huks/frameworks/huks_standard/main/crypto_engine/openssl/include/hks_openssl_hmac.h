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

#ifndef HKS_OPENSSL_HMAC_H
#define HKS_OPENSSL_HMAC_H

#include "hks_crypto_hal.h"

#ifdef __cplusplus
extern "C" {
#endif

#define HKS_DIGEST_SHA256_LEN 32
#define HKS_DIGEST_SHA384_LEN 48
#define HKS_DIGEST_SHA512_LEN 64

int32_t HksOpensslHmac(const struct HksBlob *key, uint32_t digestAlg, const struct HksBlob *msg,
    struct HksBlob *mac);

#ifdef __cplusplus
}
#endif

#endif /* HKS_OPENSSL_HMAC_H */