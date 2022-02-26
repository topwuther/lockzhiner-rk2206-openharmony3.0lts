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

#include "data_buffer.h"

namespace OHOS {
namespace AppDistributedKv {
DataBuffer::DataBuffer() : buf_(nullptr), size_(0), used_(0)
{}

DataBuffer::~DataBuffer()
{
    if (buf_ != nullptr) {
        delete[] buf_;
        buf_ = nullptr;
    }
}

bool DataBuffer::Init(size_t size)
{
    if (buf_ != nullptr) {
        return false;
    }
    size_ = std::min(size, MAX_DATA_LEN);
    buf_ = new(std::nothrow) char[size_]();
    if (buf_ == nullptr) {
        return false;
    }
    used_ = 0;
    return true;
}

const char *DataBuffer::GetBufPtr() const
{
    return buf_;
}

size_t DataBuffer::GetBufSize() const
{
    return size_;
}

void DataBuffer::SetBufUsed(size_t used)
{
    used_ = used;
}

size_t DataBuffer::GetBufUsed() const
{
    return used_;
}
}  // namespace AppDistributedKv
}  // namespace OHOS