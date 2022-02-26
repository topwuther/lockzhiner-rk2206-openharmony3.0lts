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

#include "types_export.h"

#include <algorithm>
#include <cstring>

namespace DistributedDB {
const size_t CipherPassword::MAX_PASSWORD_SIZE;

CipherPassword::CipherPassword()
{}

CipherPassword::~CipherPassword()
{
    (void)Clear();
}

bool CipherPassword::operator==(const CipherPassword &input) const
{
    if (size_ != input.GetSize()) {
        return false;
    }
    return memcmp(data_, input.GetData(), size_) == 0;
}

bool CipherPassword::operator!=(const CipherPassword &input) const
{
    return !(*this == input);
}

size_t CipherPassword::GetSize() const
{
    return size_;
}

const uint8_t* CipherPassword::GetData() const
{
    return data_;
}

int CipherPassword::SetValue(const uint8_t *inputData, size_t inputSize)
{
    if (inputSize > MAX_PASSWORD_SIZE) {
        return ErrorCode::OVERSIZE;
    }
    if (inputSize != 0 && inputData == nullptr) {
        return ErrorCode::INVALID_INPUT;
    }

    if (inputSize != 0) {
        std::copy(inputData, inputData + inputSize, data_);
    }

    size_t filledSize = std::min(size_, MAX_PASSWORD_SIZE);
    if (inputSize < filledSize) {
        std::fill(data_ + inputSize, data_ + filledSize, UCHAR_MAX);
    }

    size_ = inputSize;
    return ErrorCode::OK;
}

int CipherPassword::Clear()
{
    return SetValue(nullptr, 0);
}
} // namespace DistributedDB