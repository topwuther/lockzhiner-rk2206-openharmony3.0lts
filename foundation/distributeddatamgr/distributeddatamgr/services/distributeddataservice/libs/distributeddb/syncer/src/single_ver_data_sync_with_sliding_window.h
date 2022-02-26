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

#ifndef SINGLE_VER_DATA_SYNC_WITH_SLIDING_WINDOW_H
#define SINGLE_VER_DATA_SYNC_WITH_SLIDING_WINDOW_H

#include "sliding_window_sender.h"
#include "sliding_window_receiver.h"

namespace DistributedDB {
class SingleVerDataSyncWithSlidingWindow {
public:
    SingleVerDataSyncWithSlidingWindow() = default;
    ~SingleVerDataSyncWithSlidingWindow() = default;
    DISABLE_COPY_ASSIGN_MOVE(SingleVerDataSyncWithSlidingWindow);

    int SenderStart(int32_t mode, SingleVerSyncTaskContext *context, std::shared_ptr<SingleVerDataSync> &dataSync);
    int PreHandleSenderAckRecv(const Message *message);
    int SenderAckRecv(const Message *message);
    void SenderClear();
    void SetSenderErr(bool isErr);

    int ReceiverInit(SingleVerSyncTaskContext *context, std::shared_ptr<SingleVerDataSync> &dataSync);
    int SenderInit(SingleVerSyncTaskContext *context, std::shared_ptr<SingleVerDataSync> &dataSync);
    void ReceiverClear();
    int Receive(Message *inMsg);
private:
    SlidingWindowSender sender_;
    SlidingWindowReceiver receiver_;
};
}  // namespace DistributedDB
#endif // SINGLE_VER_DATA_SYNC_WITH_SLIDING_WINDOW_H

