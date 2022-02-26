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

#ifndef MESSAGE_TRANSFORM_H
#define MESSAGE_TRANSFORM_H

#include <map>
#include <cstdint>
#include <functional>
#include "message.h"

namespace DistributedDB {
using ComputeLengthFunc = std::function<uint32_t(const Message *inMsg)>;
using SerializeFunc = std::function<int(uint8_t *buffer, uint32_t length, const Message *inMsg)>;
using DeserializeFunc = std::function<int(const uint8_t *buffer, uint32_t length, Message *inMsg)>;

struct TransformFunc {
    ComputeLengthFunc computeFunc;
    SerializeFunc serializeFunc;
    DeserializeFunc deserializeFunc;
};

class MessageTransform {
public:
    // Must not be called in multi-thread
    // Return E_ALREADY_REGISTER if msgId is already registered
    // Return E_INVALID_ARGS if member of inFunc not all valid
    // Calling ProtocolProto::RegTransformFunction
    static int RegTransformFunction(uint32_t msgId, const TransformFunc &inFunc);
};
}

#endif // MESSAGE_TRANSFORM_H