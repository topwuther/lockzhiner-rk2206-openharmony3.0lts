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

#include "app_blob.h"
#include <securec.h>
namespace OHOS {
namespace AppDistributedKv {
AppBlob::AppBlob(const AppBlob &blob)
{
    blob_ = blob.Data();
}

AppBlob::AppBlob(AppBlob &&blob) noexcept
{
    blob_.swap(blob.blob_);
}

AppBlob &AppBlob::operator=(const AppBlob &blob)
{
    // Self-assignment detection
    if (&blob == this) {
        return *this;
    }

    blob_ = blob.Data();
    return *this;
}

AppBlob &AppBlob::operator=(AppBlob &&blob) noexcept
{
    // Self-assignment detection
    if (&blob == this) {
        return *this;
    }

    blob_.swap(blob.blob_);

    return *this;
}

AppBlob::AppBlob(const char *str, size_t n)
    : blob_()
{
    if (str != nullptr) {
        blob_ = std::vector<uint8_t>(str, str + n);
    }
}

AppBlob::AppBlob(const std::string &str)
    : blob_(str.begin(), str.end())
{
}

AppBlob::AppBlob(const char *str)
    : blob_()
{
    if (str != nullptr) {
        blob_ = std::vector<uint8_t>(str, str + strlen(str));
    }
}

AppBlob::AppBlob(const std::vector<uint8_t> &bytes)
    : blob_(bytes)
{
}

const std::vector<uint8_t> &AppBlob::Data() const
{
    return blob_;
}

size_t AppBlob::Size() const
{
    return blob_.size();
}

bool AppBlob::Empty() const
{
    return (blob_.empty());
}

bool AppBlob::operator==(const AppBlob &blob) const
{
    return blob_ == blob.blob_;
}

std::string AppBlob::ToString() const
{
    std::string str(blob_.begin(), blob_.end());
    return str;
}

int AppBlob::Compare(const AppBlob &blob) const
{
    if (blob_ < blob.blob_) {
        return -1;
    }
    if (blob_ == blob.blob_) {
        return 0;
    }
    return 1;
}
}  // namespace AppDistributedKv
}  // namespace OHOS
