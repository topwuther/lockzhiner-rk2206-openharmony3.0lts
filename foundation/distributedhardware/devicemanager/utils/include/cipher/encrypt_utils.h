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

#ifndef OHOS_DEVICE_MANAGER_ENCRYPT_UTILS_H
#define OHOS_DEVICE_MANAGER_ENCRYPT_UTILS_H

#include <cstdint>
#include <stdlib.h>

namespace OHOS {
namespace DistributedHardware {
class EncryptUtils {
public:
    static int32_t MbedtlsBase64Encode(uint8_t *dst, size_t dlen, size_t *olen,
        const uint8_t *src, size_t slen);
    static int32_t MbedtlsBase64Decode(uint8_t *dst, size_t dlen, size_t *olen,
        const uint8_t *src, size_t slen);
    static int32_t GenRandInt(int32_t randMin, int32_t randMax);
    static int64_t GenRandLongLong(int64_t randMin, int64_t randMax);
    static int32_t GetRandomData(uint8_t *randStr, uint32_t len);
    static int32_t MbedtlsEncrypt(const uint8_t *plainText, int32_t plainTextLen, uint8_t *cipherText,
        int32_t cipherTextLen, int32_t *outLen);
    static int32_t MbedtlsDecrypt(const uint8_t *cipherText, int32_t cipherTextLen, uint8_t *plainText,
        int32_t plainTextLen, int32_t *outLen);
    /**
     * szOut最后一位为结束符，比如 szOutLen=4 可能返回 "abc"
     * @param szOutLen 至少2
     * @param numberOnly 是否只生成数据 如果为true 则期望返回随机数字 如果为否，则期望返回随机字符
    */
    static bool MbedtlsGenRandomStr(char *szOut, int32_t szOutLen, bool numberOnly);
};
} // namespace DistributedHardware
} // namespace OHOS
#endif // OHOS_DEVICE_MANAGER_ENCRYPT_UTILS_H