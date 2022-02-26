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

#ifndef ENDIAN_CONVERT_H
#define ENDIAN_CONVERT_H

#include <cstdint>
#include <cstddef>

namespace DistributedDB {
inline bool IsBigEndian()
{
    uint32_t data = 0x12345678; // 0x12345678 only used here, for endian test
    uint8_t *firstByte = reinterpret_cast<uint8_t *>(&data);
    if (*firstByte == 0x12) { // 0x12 only used here, for endian test
        return true;
    }
    return false;
}

template<typename T> T HostToNet(const T &from)
{
    if (IsBigEndian()) {
        return from;
    } else {
        T to;
        size_t typeLen = sizeof(T);
        const uint8_t *fromByte = reinterpret_cast<const uint8_t *>(&from);
        uint8_t *toByte = reinterpret_cast<uint8_t *>(&to);
        for (size_t i = 0; i < typeLen; i++) {
            toByte[i] = fromByte[typeLen - i - 1]; // 1 is for index boundary
        }
        return to;
    }
}

template<typename T> T NetToHost(const T &from)
{
    return HostToNet(from);
}
}

#endif