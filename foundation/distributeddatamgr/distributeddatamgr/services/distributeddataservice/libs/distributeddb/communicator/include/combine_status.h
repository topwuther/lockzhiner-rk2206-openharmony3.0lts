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

#ifndef COMBINE_STATUS_H
#define COMBINE_STATUS_H

#include <set>
#include <mutex>
#include <cstdint>

namespace DistributedDB {
/*
 * Class CombineStatus does not support multi-thread.
 * It should be protected by mutex in multi-thread environment
 */
class CombineStatus {
public:
    void UpdateProgressId(uint64_t inProgressId);
    uint64_t GetProgressId() const;
    bool CheckProgress();

    void SetFragmentLen(uint32_t inFragLen);
    void SetLastFragmentLen(uint32_t inLastFragLen);
    uint32_t GetThisFragmentLength(uint16_t inFragNo) const;
    uint32_t GetThisFragmentOffset(uint16_t inFragNo) const;

    void SetFragmentCount(uint16_t inFragCount);
    bool IsFragNoAlreadyExist(uint16_t inFragNo) const;
    void CheckInFragmentNo(uint16_t inFragNo);
    bool IsCombineDone() const;

private:
    uint64_t progressId_ = 0;
    bool hasProgressFlag_ = true;

    uint32_t fragmentLen_ = 0; // Indicate the length of fragment that is split from a frame except the last one
    uint32_t lastFragmentLen_ = 0; // Indicate the length of the last fragment that is split from a frame

    uint16_t fragmentCount_ = 0;
    std::set<uint16_t> combinedFragmentNo_;
};
} // namespace DistributedDB

#endif // COMBINE_STATUS_H
