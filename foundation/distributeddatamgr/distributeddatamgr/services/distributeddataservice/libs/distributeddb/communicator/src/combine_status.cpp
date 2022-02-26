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

#include "combine_status.h"

namespace DistributedDB {
void CombineStatus::UpdateProgressId(uint64_t inProgressId)
{
    progressId_ = inProgressId;
    hasProgressFlag_ = true;
}

uint64_t CombineStatus::GetProgressId() const
{
    return progressId_;
}

bool CombineStatus::CheckProgress()
{
    bool preFlag = hasProgressFlag_;
    hasProgressFlag_ = false;
    return preFlag;
}

void CombineStatus::SetFragmentLen(uint32_t inFragLen)
{
    fragmentLen_ = inFragLen;
}

void CombineStatus::SetLastFragmentLen(uint32_t inLastFragLen)
{
    lastFragmentLen_ = inLastFragLen;
}

uint32_t CombineStatus::GetThisFragmentLength(uint16_t inFragNo) const
{
    // It had already been checked outside that inFragNo smaller than fragmentCount_
    return ((inFragNo != fragmentCount_ - 1) ? fragmentLen_ : lastFragmentLen_); // subtract by 1 for index
}

uint32_t CombineStatus::GetThisFragmentOffset(uint16_t inFragNo) const
{
    // It had already been checked outside that inFragNo smaller than fragmentCount_
    return fragmentLen_ * inFragNo; // It can be guaranteed no overflow will happen by multiply
}

void CombineStatus::SetFragmentCount(uint16_t inFragCount)
{
    fragmentCount_ = inFragCount;
}

bool CombineStatus::IsFragNoAlreadyExist(uint16_t inFragNo) const
{
    return (combinedFragmentNo_.count(inFragNo) != 0) ? true : false;
}

void CombineStatus::CheckInFragmentNo(uint16_t inFragNo)
{
    if (inFragNo >= fragmentCount_) {
        return;
    }
    combinedFragmentNo_.insert(inFragNo);
}

bool CombineStatus::IsCombineDone() const
{
    if (combinedFragmentNo_.size() < fragmentCount_) {
        return false;
    }
    return true;
}
} // namespace DistributedDB
