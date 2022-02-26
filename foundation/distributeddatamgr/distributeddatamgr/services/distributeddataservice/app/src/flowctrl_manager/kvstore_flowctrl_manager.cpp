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

#define LOG_TAG "KvStoreFlowCtrlManager"

#include "kvstore_flowctrl_manager.h"
#include <cinttypes>
#include <sys/time.h>

namespace OHOS {
namespace DistributedKv {
const int SECOND_TO_MICROSECOND = 1000;
uint64_t CurrentTimeMicros()
{
    struct timeval tv = { 0, 0 };
    gettimeofday(&tv, nullptr);
    return (tv.tv_sec * SECOND_TO_MICROSECOND + tv.tv_usec / SECOND_TO_MICROSECOND);
}

KvStoreFlowCtrlManager::KvStoreFlowCtrlManager(const int burstCapacity, const int sustainedCapacity)
{
    burstTokenBucket_.maxCapacity = burstCapacity;
    burstTokenBucket_.refreshTimeGap = BURST_REFRESH_TIME;
    sustainedTokenBucket_.maxCapacity = sustainedCapacity;
    sustainedTokenBucket_.refreshTimeGap = SUSTAINED_REFRESH_TIME;
}

void KvStoreFlowCtrlManager::RefreshTokenBucket(TokenBucket &tokenBucket, uint64_t timeStamp)
{
    tokenBucket.leftNumInTokenBucket = tokenBucket.maxCapacity;
    tokenBucket.tokenBucketRefreshTime = timeStamp;
}

bool KvStoreFlowCtrlManager::IsTokenEnough()
{
    int curTime = CurrentTimeMicros();
    if (IsTokenEnoughSlice(burstTokenBucket_, curTime) && IsTokenEnoughSlice(sustainedTokenBucket_, curTime)) {
        burstTokenBucket_.lastAccessTime = curTime;
        burstTokenBucket_.leftNumInTokenBucket--;

        sustainedTokenBucket_.lastAccessTime = curTime;
        sustainedTokenBucket_.leftNumInTokenBucket--;
        return true;
    }
    return false;
}

bool KvStoreFlowCtrlManager::IsTokenEnoughSlice(TokenBucket &tokenBucket, uint64_t timeStamp)
{
    // the first time to get token will be allowed;
    // if the gap between this time to get token and the least time to fill the bucket
    // to the full is larger than 10ms, this operation will be allowed;
    if (tokenBucket.tokenBucketRefreshTime == 0 ||
        timeStamp - tokenBucket.tokenBucketRefreshTime > tokenBucket.refreshTimeGap) {
        RefreshTokenBucket(tokenBucket, timeStamp);
        return true;
    } else {
        return tokenBucket.leftNumInTokenBucket >= 1;
    }
}
}
}
