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

#ifndef SLIDING_WINDOW_SENDER_H
#define SLIDING_WINDOW_SENDER_H
#include <memory>

#include "message.h"
#include "single_ver_sync_task_context.h"
#include "single_ver_data_sync.h"

namespace DistributedDB {
struct ReSendInfo {
    TimeStamp start = 0;
    TimeStamp end = 0;
    // packetId is used for matched ackpacket packetId which saved in ackPacket.reserve
    // if equaled, means need to handle the ack, or drop. it is always increased
    uint64_t packetId = 0;
};

class SlidingWindowSender {
public:
    SlidingWindowSender() = default;
    ~SlidingWindowSender();
    DISABLE_COPY_ASSIGN_MOVE(SlidingWindowSender);

    int Initialize(SingleVerSyncTaskContext *context, std::shared_ptr<SingleVerDataSync> &dataSync);
    // start one dataSync, mode can be push pushAndPull or pullResponse
    int SendStart(int32_t mode, SingleVerSyncTaskContext *context, std::shared_ptr<SingleVerDataSync> &dataSync);
    int AckRecv(const Message *message);
    int PreHandleAckRecv(const Message *message);
    void Clear();
    void SetErr(bool isErr);
private:
    int ParamCheck(int32_t mode, const SingleVerSyncTaskContext *context,
        std::shared_ptr<SingleVerDataSync> &dataSync);
    void Init(int32_t mode, SingleVerSyncTaskContext *context, std::shared_ptr<SingleVerDataSync> &dataSync);
    int UpdateInfo();
    int InnerSend();
    void InnerClear();
    int ReSend() const; // when timeout, used for reSend data

    // initial max window size
    static const int MAX_WINDOW_SIZE = 5;
    std::mutex lock_;
    // 0 is default invalid sessionId.
    uint32_t sessionId_ = 0;
    // the sequenceId as key
    std::map<uint32_t, ReSendInfo> reSendMap_;
    // remaining sending window
    int32_t windowSize_ = 0;
    // in this session, the max sequenceId has been sent
    uint32_t maxSequenceIdhasSent_ = 0;
    // int this session, all data has been sent, but all ack has received or not
    bool isAllDataHasSent_ = false;
    SingleVerSyncTaskContext *context_ = nullptr;
    std::shared_ptr<SingleVerDataSync> dataSync_ = nullptr;
    int mode_ = 0; // sync mode, e.g. push
    bool isErr_ = true;
};
}  // namespace DistributedDB
#endif // SLIDING_WINDOW_SENDER_H

