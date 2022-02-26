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

#ifndef DISTRIBUTEDDATAMGR_VALUE_HASH_H
#define DISTRIBUTEDDATAMGR_VALUE_HASH_H

#include <vector>
#include <openssl/sha.h>

namespace OHOS {
namespace DistributedKv {
class ValueHash {
public:
    ValueHash() {};
    ~ValueHash() {};

    bool CalcValueHash(const std::string &input, std::string &res)
    {
        SHA256_CTX context;
        std::vector<uint8_t> value(input.begin(), input.end());
        if (!SHA256_Init(&context)) {
            return false;
        }
        if (!SHA256_Update(&context, value.data(), value.size())) {
            return false;
        }

        std::vector<uint8_t> result;
        result.resize(SHA256_DIGEST_LENGTH);
        if (!SHA256_Final(result.data(), &context)) {
            return false;
        }
        if (result.size() < LEN) {
            res = std::string(result.begin(), result.end());
            return true;
        }
        res = std::string(result.end() - LEN, result.end());
        return true;
    }
private:
    static constexpr int LEN = 8; // 8 is the substring length
};
}  // namespace DistributedKv
}  // namespace OHOS
#endif // DISTRIBUTEDDATAMGR_VALUE_HASH_H
