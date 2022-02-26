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

#include "crypto_utils.h"

#include <list>
#include <random>
#include <regex>
#include <vector>
#include "openssl/sha.h"

namespace OHOS {
namespace DistributedKv {
std::string CryptoUtils::Sha256(const std::string &plainText)
{
    unsigned char hash[SHA256_DIGEST_LENGTH] = "";
    SHA256_CTX ctx;
    SHA256_Init(&ctx);
    SHA256_Update(&ctx, plainText.c_str(), plainText.size());
    SHA256_Final(hash, &ctx);
    std::string hashToHex;
    // here we translate sha256 hash to hexadecimal. each 8-bit char will be presented by two characters([0-9a-f])
    constexpr int CHAR_WIDTH = 8;
    constexpr int HEX_WIDTH = 4;
    constexpr unsigned char HEX_MASK = 0xf;
    constexpr int HEX_A = 10;
    hashToHex.reserve(SHA256_DIGEST_LENGTH * (CHAR_WIDTH / HEX_WIDTH));
    for (unsigned char i : hash) {
        unsigned char hex = i >> HEX_WIDTH;
        if (hex < HEX_A) {
            hashToHex.push_back('0' + hex);
        } else {
            hashToHex.push_back('a' + hex - HEX_A);
        }
        hex = i & HEX_MASK;
        if (hex < HEX_A) {
            hashToHex.push_back('0' + hex);
        } else {
            hashToHex.push_back('a' + hex - HEX_A);
        }
    }
    return hashToHex;
}

std::string CryptoUtils::Sha256AppId(const std::string &bundleName, int userId)
{
    return bundleName;
}

std::string CryptoUtils::Sha256UserId(const std::string &plainText)
{
    std::regex pattern("^[0-9]+$");
    if (!std::regex_match(plainText, pattern)) {
        return plainText;
    }

    std::string::size_type sizeType;
    int64_t plainVal;
    std::string::size_type int64MaxLen(std::to_string(INT64_MAX).size());
    // plain text length must be less than INT64_MAX string.
    try {
        plainVal = static_cast<int64_t>(std::stoll(plainText, &sizeType));
    } catch (const std::out_of_range &) {
        plainVal = static_cast<int64_t>(std::stoll(
            plainText.substr(plainText.size() - int64MaxLen + 1, int64MaxLen - 1), &sizeType));
    } catch (const std::exception &) {
        return plainText;
    }

    union UnionLong {
        int64_t val;
        unsigned char byteLen[sizeof(int64_t)];
    };
    UnionLong unionLong {};
    unionLong.val = plainVal;
    std::list<char> unionList(std::begin(unionLong.byteLen), std::end(unionLong.byteLen));
    unionList.reverse();
    std::vector<char> unionVec(unionList.begin(), unionList.end());

    unsigned char hash[SHA256_DIGEST_LENGTH];
    SHA256_CTX ctx;
    SHA256_Init(&ctx);
    SHA256_Update(&ctx, unionVec.data(), sizeof(int64_t));
    SHA256_Final(hash, &ctx);

    const char* hexArray = "0123456789ABCDEF";
    char* hexChars = new char[SHA256_DIGEST_LENGTH * 2 + 1];
    for (int i = 0; i < SHA256_DIGEST_LENGTH; i++) {
        unsigned int value = hash[i] & 0xFF;
        hexChars[i * 2] = hexArray[value >> 4];
        hexChars[i * 2 + 1] = hexArray[value & 0x0F];
    }
    hexChars[SHA256_DIGEST_LENGTH * 2] = '\0';
    std::string res(hexChars);
    delete []hexChars;
    return res;
}

void CryptoUtils::GetRandomKey(int keyLen, std::vector<uint8_t> &key)
{
    constexpr int UINT8_T_MAX = 255;
    std::random_device randomDevice;
    std::uniform_int_distribution<int> distribution(0, UINT8_T_MAX);
    key.clear();
    for (int i = 0; i < keyLen; i++) {
        key.push_back(static_cast<uint8_t>(distribution(randomDevice)));
    }
}
} // namespace DistributedKv
} // namespace OHOS
