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

#ifndef CRYPTO_UTILS_H
#define CRYPTO_UTILS_H

#include <string>
#include <vector>
#include "visibility.h"

namespace OHOS {
namespace DistributedKv {
class CryptoUtils {
public:
// get sha256 hash
// plainText: string to be hashed
// sha256 hash of the input string
KVSTORE_API static std::string Sha256(const std::string &plainText);

// get appId from bundle name
// bundleName: client declared bundleName
// userId: id of current user
// hashed id of this app. each char in returned string minus 'A' is the sha512 value of origin appId.
KVSTORE_API static std::string Sha256AppId(const std::string &bundleName, int userId = 0);

// get sha256 hash
// plainText: string to be hashed
// sha256 hash of the input string
KVSTORE_API static std::string Sha256UserId(const std::string &plainText);

// Use system entropy pool to generate true random number. note that it will run quiet slowly on system entropy
// pool running out.
// keyLen: returned vector length.
// true random 8-bit sequence.
KVSTORE_API static void GetRandomKey(int keyLen, std::vector<uint8_t> &key);
};
} // namespace DistributedKv
} // namespace OHOS
#endif // CRYPTO_UTILS_H