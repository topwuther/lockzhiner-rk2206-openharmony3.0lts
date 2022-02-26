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

#define LOG_TAG "Blob"

#include "blob.h"
#include <securec.h>
#include "log_print.h"

namespace OHOS {
namespace DistributedKv {
Blob::Blob() { }

Blob::Blob(const Blob &blob)
{
    blob_ = blob.Data();
}

Blob::Blob(Blob &&blob)
{
    blob_.swap(blob.blob_);
}

Blob &Blob::operator=(const Blob &blob)
{
    // Self-assignment detection
    if (&blob == this) {
        return *this;
    }

    blob_ = blob.Data();

    return *this;
}

Blob &Blob::operator=(Blob &&blob)
{
    // Self-assignment detection
    if (&blob == this) {
        return *this;
    }

    blob_.swap(blob.blob_);

    return *this;
}

Blob::Blob(const char *str, size_t n)
    : blob_()
{
    if (str != nullptr) {
        blob_ = std::vector<uint8_t>(str, str + n);
    }
}

Blob::Blob(const std::string &str)
    : blob_(str.begin(), str.end())
{
}

Blob::Blob(const char *str)
    : blob_()
{
    if (str != nullptr) {
        blob_ = std::vector<uint8_t>(str, str + strlen(str));
    }
}

Blob::Blob(const std::vector<uint8_t> &bytes)
    : blob_(bytes)
{
}

Blob::Blob(std::vector<uint8_t> &&bytes)
    : blob_(std::move(bytes))
{
}

const std::vector<uint8_t> &Blob::Data() const
{
    return blob_;
}

size_t Blob::Size() const
{
    return blob_.size();
}

int Blob::RawSize() const
{
    return sizeof(int) + blob_.size();
}

bool Blob::Empty() const
{
    return blob_.empty();
}

uint8_t Blob::operator[](size_t n) const
{
    if (n >= Size()) {
        ZLOGE("Trying to get a out-of-range Blob member.");
        return 0;
    }
    return blob_[n];
}

bool Blob::operator==(const Blob &blob) const
{
    return blob_ == blob.blob_;
}

void Blob::Clear()
{
    blob_.clear();
}

std::string Blob::ToString() const
{
    std::string str(blob_.begin(), blob_.end());
    return str;
}

int Blob::Compare(const Blob &blob) const
{
    if (blob_ < blob.blob_) {
        return -1;
    }
    if (blob_ == blob.blob_) {
        return 0;
    }
    return 1;
}

bool Blob::StartsWith(const Blob &blob) const
{
    size_t len = blob.Size();
    if (Size() < len) {
        return false;
    }

    for (size_t i = 0; i < len; ++i) {
        if (blob_[i] != blob.blob_[i]) {
            return false;
        }
    }
    return true;
}

bool Blob::Marshalling(Parcel &parcel) const
{
    return parcel.WriteUInt8Vector(this->blob_);
}

Blob *Blob::Unmarshalling(Parcel &parcel)
{
    std::vector<uint8_t> blobData;
    if (!parcel.ReadUInt8Vector(&blobData)) {
        return nullptr;
    }
    return new Blob(blobData);
}

/* write blob size and data to memory buffer. return error when bufferLeftSize not enough. */
bool Blob::WriteToBuffer(uint8_t *&cursorPtr, int &bufferLeftSize) const
{
    if (cursorPtr == nullptr || bufferLeftSize < static_cast<int>(blob_.size() + sizeof(int))) {
        return false;
    }
    *reinterpret_cast<int32_t *>(cursorPtr) = static_cast<int32_t>(blob_.size());
    bufferLeftSize -= sizeof(int32_t);
    cursorPtr += sizeof(int32_t);
    errno_t err = memcpy_s(cursorPtr, bufferLeftSize, blob_.data(), blob_.size());
    if (err != EOK) {
        return false;
    }
    cursorPtr += blob_.size();
    bufferLeftSize -= blob_.size();
    return true;
}

/* read a blob from memory buffer. */
bool Blob::ReadFromBuffer(const uint8_t *&cursorPtr, int &bufferLeftSize)
{
    if (cursorPtr == nullptr || bufferLeftSize < static_cast<int>(sizeof(int))) {
        return false;
    }
    int blobSize = *reinterpret_cast<const int *>(cursorPtr);
    bufferLeftSize -= sizeof(int) + blobSize;
    if (blobSize < 0 || bufferLeftSize < 0) {
        return false;
    }
    cursorPtr += sizeof(int);
    blob_ = std::vector<uint8_t>(cursorPtr, cursorPtr + blobSize);
    cursorPtr += blobSize;
    return true;
}
}  // namespace DistributedKv
}  // namespace OHOS
