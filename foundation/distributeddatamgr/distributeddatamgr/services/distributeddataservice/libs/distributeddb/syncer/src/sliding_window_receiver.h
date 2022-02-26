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

#ifndef SLIDING_WINDOW_RECEIVER_H
#define SLIDING_WINDOW_RECEIVER_H

#include <memory>

#include "message.h"
#include "single_ver_sync_task_context.h"
#include "single_ver_data_sync.h"

namespace DistributedDB {
class SlidingWindowReceiver {
public:
    SlidingWindowReceiver() = default;
    ~SlidingWindowReceiver();
    DISABLE_COPY_ASSIGN_MOVE(SlidingWindowReceiver);

    int Initialize(SingleVerSyncTaskContext *context, std::shared_ptr<SingleVerDataSync> &dataSync);
    int Receive(Message *inMsg);
    void Clear();
private:
    int PutMsg(Message *inMsg);
    void DealMsg();
    int TimeOut(TimerId timerId);
    void StartTimer();
    void StopTimer();
    void ClearMap();
    void ResetInfo();
    int ErrHandle(uint32_t sequenceId);
    void SetEndField(bool isLastSequence, uint32_t sequenceId);

    std::mutex lock_;
    std::condition_variable workingTaskcv_;
    // 0 is default invalid.
    uint32_t sessionId_ = 0;
    // first:sequenceId second:Message*, message data not deal.
    std::map<uint32_t, Message *> messageMap_;
    // 0 is has finished nothing; e.g. 3 is sequenceId 1 2 3 has finished, sequenceId 4 has not finished.
    uint32_t hasFinishedMaxId_ = 0;
    // 0 is idle
    uint32_t workingId_ = 0;
    // 0 is has not received the end pakcet now.
    uint32_t endId_ = 0;
    bool isWaterMarkErrHappened_ = false;
    // timeout for idle wait packet
    TimerId timerId_ = 0;
    static constexpr int IDLE_TIME_OUT = 5 * 60 * 1000; // 5min
    SingleVerSyncTaskContext *context_ = nullptr;
    std::shared_ptr<SingleVerDataSync> dataSync_;
};
}  // namespace DistributedDB
#endif // SLIDING_WINDOW_RECEIVER_H

