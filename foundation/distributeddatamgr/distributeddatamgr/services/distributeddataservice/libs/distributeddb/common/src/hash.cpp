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

#include "hash.h"

namespace DistributedDB {
uint64_t Hash::HashFunc(const std::string &input)
{
    uint64_t hash = 0;
    size_t idx = 0;

    for (idx = 0; idx < input.size(); idx++) {
        hash = (hash * PRIME_SEED) + input.at(idx);
    }

    return hash;
}

uint32_t Hash::Hash32Func(const std::string &input)
{
    uint32_t hash = 0;
    size_t idx = 0;

    for (idx = 0; idx < input.size(); idx++) {
        hash = (hash << 4) + input.at(idx); // 4 is offset
        uint32_t x = (hash & 0xf0000000);
        if (x != 0) {
            hash ^= (x >> 24); // 24 is offset
        }
        hash &= ~x;
    }
    return (hash & 0x7fffffff);
}
} // namespace DistributedDB
