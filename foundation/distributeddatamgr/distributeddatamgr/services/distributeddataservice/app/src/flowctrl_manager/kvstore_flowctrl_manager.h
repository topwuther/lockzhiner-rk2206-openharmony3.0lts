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

#ifndef FOUNDATION_KVSTORE_FLOW_CTRL_MANAGER_H
#define FOUNDATION_KVSTORE_FLOW_CTRL_MANAGER_H

#include <atomic>

namespace OHOS {
namespace DistributedKv {
struct TokenBucket {
    uint64_t tokenBucketRefreshTime = 0; // last time to refresh the bucket

    uint64_t lastAccessTime = 0; // last time to access

    volatile std::atomic<int> leftNumInTokenBucket {0}; // rest numbers of token in the bucket

    int maxCapacity; // max capacity

    uint64_t refreshTimeGap; // time gap between refreshing
};

class KvStoreFlowCtrlManager {
public:
    KvStoreFlowCtrlManager() = delete;

    KvStoreFlowCtrlManager(const int burstCapacity, const int sustainedCapacity);

    ~KvStoreFlowCtrlManager() = default;

    bool IsTokenEnough();

    static const int BURST_REFRESH_TIME = 1000;

    static const int SUSTAINED_REFRESH_TIME = 60000;

private:
    void RefreshTokenBucket(TokenBucket &tokenBucket, uint64_t timeStamp);

    bool IsTokenEnoughSlice(TokenBucket &tokenBucket, uint64_t timeStamp);

    TokenBucket burstTokenBucket_; // token bucket to deal with events in a burst

    TokenBucket sustainedTokenBucket_; // token bucket to deal with sustained events.
};
}
}

#endif // FOUNDATION_KVSTORE_FLOW_CTRL_MANAGER_H
