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

#include "audio_cache.h"

#include <string>

#include "securec.h"

namespace KWS {
const int32_t MAX_BUFFER_SIZE = 36000;
const int32_t MIN_BUFFER_SIZE = 8000;

AudioCache::AudioCache()
    : maxBufferSize_(0), left_(0), right_(0), buffer_(nullptr), prepared_(false)
{
}

AudioCache::~AudioCache()
{
    if (prepared_ && buffer_ != nullptr) {
        delete[] buffer_;
        buffer_ = nullptr;
    }
    prepared_ = false;
    maxBufferSize_ = 0;
    left_ = 0;
    right_ = 0;
}

bool AudioCache::Init(int32_t maxSize)
{
    if (prepared_) {
        printf("[AudioCache]Cache has ready init.\n");
        return false;
    }
    if (maxSize > MAX_BUFFER_SIZE || maxSize < MIN_BUFFER_SIZE) {
        printf("[AudioCache]maxSize out of range, init failed.\n");
        return false;
    }
    maxBufferSize_ = maxSize;
    buffer_ = new (std::nothrow) uint8_t[maxBufferSize_];
    if (buffer_ == nullptr) {
        printf("[AudioCache]Fail to allocate buffer for given size.\n");
        return false;
    }
    errno_t ret = memset_s(buffer_, maxBufferSize_, 0x00, maxBufferSize_);
    if (ret != EOK) {
        printf("[AudioCache]Cache buffer init failed.\n");
        return false;
    }
    prepared_ = true;
    return true;
}

bool AudioCache::AppendBuffer(int32_t bufferSize, const uint8_t *buffer)
{
    if (!prepared_) {
        return false;
    }
    if (bufferSize + right_ >= maxBufferSize_) {
        left_ = 0;
        right_ = 0;
    }
    size_t remainLength = maxBufferSize_ - right_;
    size_t length = (bufferSize > remainLength) ? remainLength : bufferSize;
    size_t copySize = sizeof(uint8_t) * length;
    errno_t ret = memcpy_s(&buffer_[right_], copySize, buffer, copySize);
    if (ret != EOK) {
        printf("[AudioCache]AppendBuffer failed.\n");
        return false;
    }
    right_ += length;
    return true;
}

size_t AudioCache::GetCapturedBuffer(uintptr_t &samplePtr)
{
    if (right_ - left_ <= 0 || buffer_ == nullptr) {
        return 0;
    }
    samplePtr = reinterpret_cast<uintptr_t>(buffer_ + left_);
    size_t length = right_ - left_;
    left_ = right_;
    return length;
}
}  // namespace KWS