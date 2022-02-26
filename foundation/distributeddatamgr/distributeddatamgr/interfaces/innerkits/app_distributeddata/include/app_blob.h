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

#ifndef DISTRIBUTED_KV_APP_BLOB_H
#define DISTRIBUTED_KV_APP_BLOB_H

#include <string>
#include <vector>
#include "visibility.h"

namespace OHOS {
namespace AppDistributedKv {
class AppBlob {
public:
    KVSTORE_API AppBlob() = default;
    KVSTORE_API ~AppBlob() = default;

    // Copy constructor for Blob.
    KVSTORE_API AppBlob(const AppBlob &blob);
    KVSTORE_API AppBlob &operator=(const AppBlob &blob);

    // Move constructor for Blob.
    KVSTORE_API AppBlob(AppBlob &&blob) noexcept;
    KVSTORE_API AppBlob &operator=(AppBlob &&blob) noexcept;

    // Construct a Blob using std::string.
    KVSTORE_API AppBlob(const std::string &str);

    // Construct a Blob using char pointer and len.
    KVSTORE_API AppBlob(const char *str, size_t n);

    // Construct a Blob using char pointer.
    KVSTORE_API AppBlob(const char *str);

    // Construct a Blob using std::vector<uint8_t>.
    KVSTORE_API AppBlob(const std::vector<uint8_t> &str);

    // Return a reference to the data of the blob.
    KVSTORE_API const std::vector<uint8_t> &Data() const;

    // Return the length (in bytes) of the referenced data.
    KVSTORE_API size_t Size() const;

    // Return true if the length of the referenced data is zero.
    KVSTORE_API bool Empty() const;

    KVSTORE_API bool operator==(const AppBlob &) const;

    // Change vector<uint8_t> to std::string.
    KVSTORE_API std::string ToString() const;

    // comparison.  Returns value:
    //   <  0 if "*this" <  "blob",
    //   == 0 if "*this" == "blob",
    //   >  0 if "*this" >  "blob"
    KVSTORE_API int Compare(const AppBlob &blob) const;

private:
    std::vector<uint8_t> blob_;
};
}  // namespace AppDistributedKv
}  // namespace OHOS

#endif  // DISTRIBUTED_KV_APP_BLOB_H
