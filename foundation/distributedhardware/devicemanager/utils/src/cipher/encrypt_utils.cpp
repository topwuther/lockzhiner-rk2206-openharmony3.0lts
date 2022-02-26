/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "encrypt_utils.h"

#include "mbedtls/base64.h"
#include "mbedtls/gcm.h"
#include "mbedtls/md.h"

#include "mbedtls/ctr_drbg.h"
#include "mbedtls/entropy.h"

#include <random>
#include <cstdint>
#include <cstring>
#include "device_manager_errno.h"
#include "device_manager_log.h"
#include "securec.h"

namespace OHOS {
namespace DistributedHardware {
int32_t EncryptUtils::MbedtlsBase64Encode(uint8_t *dst, size_t dlen, size_t *olen,
    const uint8_t *src, size_t slen)
{
    return mbedtls_base64_encode(dst, dlen, olen, src, slen);
}

int32_t EncryptUtils::MbedtlsBase64Decode(uint8_t *dst, size_t dlen, size_t *olen,
    const uint8_t *src, size_t slen)
{
    return mbedtls_base64_decode(dst, dlen, olen, src, slen);
}

int32_t EncryptUtils::GenRandInt(int32_t randMin, int32_t randMax)
{
    std::random_device randDevice;
    std::mt19937 genRand(randDevice());
    std::uniform_int_distribution<int> disRand(randMin, randMax);
    return disRand(genRand);
}

int64_t EncryptUtils::GenRandLongLong(int64_t randMin, int64_t randMax)
{
    std::random_device randDevice;
    std::mt19937 genRand(randDevice());
    std::uniform_int_distribution<long long> disRand(randMin, randMax);
    return disRand(genRand);
}

int32_t EncryptUtils::GetRandomData(uint8_t *randStr, uint32_t len)
{
    mbedtls_entropy_context *entropy = nullptr;
    mbedtls_ctr_drbg_context *ctrDrbg = nullptr;
    int32_t ret = DEVICEMANAGER_FAILED;
    do {
        if (randStr == nullptr || len == 0) {
            break;
        }
        entropy = (mbedtls_entropy_context *)malloc(sizeof(mbedtls_entropy_context));
        if (entropy == nullptr) {
            break;
        }
        ctrDrbg = (mbedtls_ctr_drbg_context *)malloc(sizeof(mbedtls_ctr_drbg_context));
        if (ctrDrbg == nullptr) {
            break;
        }
        mbedtls_ctr_drbg_init(ctrDrbg);
        mbedtls_entropy_init(entropy);
        ret = mbedtls_ctr_drbg_seed(ctrDrbg, mbedtls_entropy_func, entropy, nullptr, 0);
        if (ret != 0) {
            break;
        }
        ret = mbedtls_ctr_drbg_random(ctrDrbg, randStr, len);
        if (ret != 0) {
            break;
        }
        ret = DEVICEMANAGER_OK;
    } while (0);
    if (entropy != nullptr) {
        free(entropy);
    }
    if (ctrDrbg != nullptr) {
        free(ctrDrbg);
    }
    return ret;
}

int32_t EncryptUtils::MbedtlsEncrypt(const uint8_t *plainText, int32_t plainTextLen, uint8_t *cipherText,
    int32_t cipherTextLen, int32_t *outLen)
{
    // Security algorithms do not support open source. Customize if required
    if (memcpy_s(cipherText, cipherTextLen, plainText, plainTextLen) != DEVICEMANAGER_OK) {
        return DEVICEMANAGER_COPY_FAILED;
    }
    *outLen = plainTextLen;
    return DEVICEMANAGER_OK;
}

int32_t EncryptUtils::MbedtlsDecrypt(const uint8_t *cipherText, int32_t cipherTextLen, uint8_t *plainText,
    int32_t plainTextLen, int32_t *outLen)
{
    // Security algorithms do not support open source. Customize if required
    (void)outLen;
    if (memcpy_s(plainText, plainTextLen, cipherText, cipherTextLen) != DEVICEMANAGER_OK) {
        return DEVICEMANAGER_COPY_FAILED;
    }
    return DEVICEMANAGER_OK;
}

bool EncryptUtils::MbedtlsGenRandomStr(char *szOut, int32_t szOutLen, bool numberOnly)
{
    const int32_t MIN_OUT_LENGTH = 2;
    if (szOut == nullptr || szOutLen <= MIN_OUT_LENGTH) {
        return false;
    }
    szOut[--szOutLen] = 0;
    GetRandomData((uint8_t*)szOut, szOutLen);
    const int32_t NUMBER_COUNT = 10;
    const int32_t ALPHA_COUNT = 26;
    const int32_t ALPHA_BYTE_COUNT = 2;
    int32_t M = numberOnly ? NUMBER_COUNT : (NUMBER_COUNT + ALPHA_BYTE_COUNT * ALPHA_COUNT);
    for (int32_t i = 0; i < szOutLen; i++) {
        // 0~9,A~Z,a~z
        uint32_t idx = ((uint32_t)szOut[i] % M);
        char base;
        if (idx < NUMBER_COUNT) {
            base = '0';
        } else if (idx >= NUMBER_COUNT && idx < (NUMBER_COUNT + ALPHA_COUNT)) {
            base = 'A';
            idx -= NUMBER_COUNT;
        } else {
            base = 'a';
            idx -= (NUMBER_COUNT + ALPHA_COUNT);
        }
        szOut[i] = base + idx;
    }
    return true;
}
} // namespace DistributedHardware
} // namespace OHOS
