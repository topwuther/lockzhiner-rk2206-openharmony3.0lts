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

#ifndef VALUE_HASH_CALC_H
#define VALUE_HASH_CALC_H

#include <vector>

#include <openssl/sha.h>

#include "db_types.h"
#include "db_errno.h"
#include "log_print.h"

namespace DistributedDB {
class ValueHashCalc {
public:
    ValueHashCalc() {};
    ~ValueHashCalc()
    {
        if (context_ != nullptr) {
            delete context_;
            context_ = nullptr;
        }
    }

    int Initialize()
    {
        context_ = new (std::nothrow) SHA256_CTX;
        if (context_ == nullptr) {
            return -E_OUT_OF_MEMORY;
        }

        int errCode = SHA256_Init(context_);
        if (errCode == 0) {
            LOGE("sha init failed:%d", errCode);
            return -E_CALC_HASH;
        }
        return E_OK;
    }

    int Update(const std::vector<uint8_t> &value)
    {
        if (context_ == nullptr) {
            return -E_CALC_HASH;
        }
        int errCode = SHA256_Update(context_, value.data(), value.size());
        if (errCode == 0) {
            LOGE("sha update failed:%d", errCode);
            return -E_CALC_HASH;
        }
        return E_OK;
    }

    int GetResult(std::vector<uint8_t> &value)
    {
        if (context_ == nullptr) {
            return -E_CALC_HASH;
        }

        value.resize(SHA256_DIGEST_LENGTH);
        int errCode = SHA256_Final(value.data(), context_);
        if (errCode == 0) {
            LOGE("sha get result failed:%d", errCode);
            return -E_CALC_HASH;
        }

        return E_OK;
    }

private:
    SHA256_CTX *context_ = nullptr;
};
}

#endif // VALUE_HASH_CALC_H
