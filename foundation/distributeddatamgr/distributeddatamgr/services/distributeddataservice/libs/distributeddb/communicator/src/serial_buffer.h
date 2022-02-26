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

#ifndef SERIALBUFFER_H
#define SERIALBUFFER_H

#include <vector>
#include <cstdint>
#include <utility>
#include "macro_utils.h"

namespace DistributedDB {
class SerialBuffer {
public:
    SerialBuffer() = default; // Default constructor must be explicitly provided due to DISABLE_COPY_ASSIGN_MOVE
    ~SerialBuffer();

    DISABLE_COPY_ASSIGN_MOVE(SerialBuffer);

    // May be directly send out, so padding is needed
    int AllocBufferByPayloadLength(uint32_t inPayloadLen, uint32_t inHeaderLen);

    // In case assemble fragment to frame, so no padding is needed, using frameLen as inTotalLen
    int AllocBufferByTotalLength(uint32_t inTotalLen, uint32_t inHeaderLen);

    // In case directly received, inTotalLen not include the padding, using frameLen as inTotalLen
    int SetExternalBuff(const uint8_t *buff, uint32_t inTotalLen, uint32_t inHeaderLen);

    // Create a SerialBuffer that has a independent bytes_ and point to the same externalBytes_
    SerialBuffer *Clone(int &outErrorNo);
    // After return E_OK, this SerialBuffer can cross thread. Do nothing indeed if it already able to cross thread.
    int ConvertForCrossThread();

    uint32_t GetSize() const;

    std::pair<uint8_t *, uint32_t> GetWritableBytesForEntireBuffer();
    std::pair<uint8_t *, uint32_t> GetWritableBytesForEntireFrame();
    std::pair<uint8_t *, uint32_t> GetWritableBytesForHeader();
    std::pair<uint8_t *, uint32_t> GetWritableBytesForPayload();

    std::pair<const uint8_t *, uint32_t> GetReadOnlyBytesForEntireBuffer() const;
    std::pair<const uint8_t *, uint32_t> GetReadOnlyBytesForEntireFrame() const;
    std::pair<const uint8_t *, uint32_t> GetReadOnlyBytesForHeader() const;
    std::pair<const uint8_t *, uint32_t> GetReadOnlyBytesForPayload() const;
private:
    uint8_t *bytes_ = nullptr;
    const uint8_t *externalBytes_ = nullptr;
    uint32_t totalLen_ = 0;
    uint32_t headerLen_ = 0;
    uint32_t payloadLen_ = 0;
    uint32_t paddingLen_ = 0;
    bool isExternalStackMemory_ = false;
};
} // namespace DistributedDB

#endif // SERIALBUFFER_H
