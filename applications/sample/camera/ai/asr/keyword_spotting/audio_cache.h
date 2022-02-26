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

#ifndef AUDIO_CACHE_H
#define AUDIO_CACHE_H

#include <cstdint>
#include <mutex>

namespace KWS {
class AudioCache {
public:
    AudioCache();
    virtual ~AudioCache();
    bool Init(int32_t maxSize);
    size_t GetCapturedBuffer(uintptr_t &samplePtr);
    bool AppendBuffer(int32_t bufferSize, const uint8_t *buffer);

private:
    int32_t maxBufferSize_;
    int32_t left_;
    int32_t right_;
    uint8_t *buffer_;
    bool prepared_;
};
}  // namespace KWS
#endif  // AUDIO_CACHE_H