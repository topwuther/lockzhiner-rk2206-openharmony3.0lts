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

#ifndef PARCEL_H
#define PARCEL_H

#include <vector>
#include <cstdint>
#include <string>
#include <type_traits>

#include "endian_convert.h"
#include "securec.h"
#include "macro_utils.h"
#include "db_errno.h"
#include "log_print.h"
#ifndef OMIT_MULTI_VER
#include "multi_ver_def.h"
#endif

namespace DistributedDB {
class Parcel {
public:
    Parcel(uint8_t *inBuf, uint32_t length);
    ~Parcel();
    bool IsError() const;
    int WriteInt(int data);
    uint32_t ReadInt(int &val);
    int WriteUInt32(uint32_t data);
    uint32_t ReadUInt32(uint32_t &val);
    int WriteUInt64(uint64_t data);
    uint32_t ReadUInt64(uint64_t &val);
    int WriteVectorChar(const std::vector<uint8_t> &data);
    uint32_t ReadVectorChar(std::vector<uint8_t> &val);
    int WriteString(const std::string &inVal);
    uint32_t ReadString(std::string &outVal);
#ifndef OMIT_MULTI_VER
    int WriteMultiVerCommit(const MultiVerCommitNode &commit);
    uint32_t ReadMultiVerCommit(MultiVerCommitNode &commit);
    int WriteMultiVerCommits(const std::vector<MultiVerCommitNode> &commits);
    uint32_t ReadMultiVerCommits(std::vector<MultiVerCommitNode> &commits);
#endif

    template<typename T>
    int WriteVector(const std::vector<T> &data)
    {
        static_assert(std::is_pod<T>::value, "type T is not pod");
        if (data.size() > INT32_MAX || sizeof(T) > INT32_MAX) {
            isError_ = true;
            return -E_PARSE_FAIL;
        }
        uint32_t len = data.size();
        uint64_t stepLen = static_cast<uint64_t>(data.size()) * sizeof(T) + sizeof(uint32_t);
        len = HostToNet(len);
        if (isError_ || bufPtr_ == nullptr || stepLen > INT32_MAX || parcelLen_ + BYTE_8_ALIGN(stepLen) > totalLen_) {
            isError_ = true;
            return -E_PARSE_FAIL;
        }
        errno_t errCode = memcpy_s(bufPtr_, totalLen_ - parcelLen_, &len, sizeof(uint32_t));
        if (errCode != EOK) {
            isError_ = true;
            return -E_SECUREC_ERROR;
        }
        bufPtr_ += sizeof(uint32_t);
        for (auto iter : data) {
            *(reinterpret_cast<T *>(bufPtr_)) = HostToNet(iter);
            bufPtr_ += sizeof(T);
        }
        bufPtr_ += BYTE_8_ALIGN(stepLen) - stepLen;
        parcelLen_ += BYTE_8_ALIGN(stepLen);
        return errCode;
    }

    template<typename T>
    uint32_t ReadVector(std::vector<T> &val)
    {
        static_assert(std::is_pod<T>::value, "type T is not pod");
        uint32_t len;
        if (isError_ || bufPtr_ == nullptr || parcelLen_ + sizeof(uint32_t) > totalLen_ || sizeof(T) > INT32_MAX) {
            isError_ = true;
            return 0;
        }
        len = *(reinterpret_cast<uint32_t *>(bufPtr_));
        len = NetToHost(len);
        if (len > INT32_MAX) {
            isError_ = true;
            return 0;
        }
        uint64_t stepLen = static_cast<uint64_t>(len) * sizeof(T) + sizeof(uint32_t);
        if (stepLen > INT32_MAX || parcelLen_ + BYTE_8_ALIGN(stepLen) > totalLen_) {
            isError_ = true;
            return 0;
        }
        bufPtr_ += sizeof(uint32_t);
        val.resize(len);
        for (uint32_t i = 0; i < len; i++) {
            val[i] = NetToHost(*(reinterpret_cast<T *>(bufPtr_)));
            bufPtr_ += sizeof(T);
        }
        bufPtr_ += BYTE_8_ALIGN(stepLen) - stepLen;
        parcelLen_ += BYTE_8_ALIGN(stepLen);
        stepLen  = BYTE_8_ALIGN(stepLen);
        return static_cast<uint32_t>(stepLen);
    }

    int WriteBlob(const char *buffer, uint32_t bufLen);
    uint32_t ReadBlob(char *buffer, uint32_t bufLen);
    void EightByteAlign();
    static uint32_t GetIntLen();
    static uint32_t GetUInt32Len();
    static uint32_t GetUInt64Len();
    static uint32_t GetVectorCharLen(const std::vector<uint8_t> &data);

    template<typename T>
    static uint32_t GetVectorLen(const std::vector<T> &data)
    {
        if (data.size() > INT32_MAX || sizeof(T) > INT32_MAX) {
            return 0;
        }
        uint64_t len = sizeof(uint32_t) + static_cast<uint64_t>(data.size()) * sizeof(T);
        len = BYTE_8_ALIGN(len);
        if (len > INT32_MAX) {
            return 0;
        }
        return static_cast<uint32_t>(len);
    }

    static uint32_t GetEightByteAlign(uint32_t len);
    static uint32_t GetStringLen(const std::string &data);
#ifndef OMIT_MULTI_VER
    static uint32_t GetMultiVerCommitLen(const MultiVerCommitNode &commit);
    static uint32_t GetMultiVerCommitsLen(const std::vector<MultiVerCommitNode> &commits);
#endif
    static uint32_t GetAppendedLen();
private:
    bool isError_ = false;
    uint8_t *buf_ = nullptr;
    uint8_t *bufPtr_ = nullptr;
    uint64_t parcelLen_ = 0;
    uint64_t totalLen_ = 0;
};
} // namespace DistributedDB

#endif // PARCEL_H

