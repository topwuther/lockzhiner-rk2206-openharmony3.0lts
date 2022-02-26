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

#include "single_ver_data_sync_with_sliding_window.h"

namespace DistributedDB {
int SingleVerDataSyncWithSlidingWindow::SenderStart(int32_t mode, SingleVerSyncTaskContext *context,
    std::shared_ptr<SingleVerDataSync> &dataSync)
{
    return sender_.SendStart(mode, context, dataSync);
}

int SingleVerDataSyncWithSlidingWindow::SenderAckRecv(const Message *message)
{
    return sender_.AckRecv(message);
}

int SingleVerDataSyncWithSlidingWindow::PreHandleSenderAckRecv(const Message *message)
{
    return sender_.PreHandleAckRecv(message);
}

void SingleVerDataSyncWithSlidingWindow::SenderClear()
{
    sender_.Clear();
}

void SingleVerDataSyncWithSlidingWindow::SetSenderErr(bool isErr)
{
    sender_.SetErr(isErr);
}

int SingleVerDataSyncWithSlidingWindow::ReceiverInit(SingleVerSyncTaskContext *context,
    std::shared_ptr<SingleVerDataSync> &dataSync)
{
    return receiver_.Initialize(context, dataSync);
}

int SingleVerDataSyncWithSlidingWindow::SenderInit(SingleVerSyncTaskContext *context,
    std::shared_ptr<SingleVerDataSync> &dataSync)
{
    return sender_.Initialize(context, dataSync);
}

void SingleVerDataSyncWithSlidingWindow::ReceiverClear()
{
    receiver_.Clear();
}

int SingleVerDataSyncWithSlidingWindow::Receive(Message *inMsg)
{
    return receiver_.Receive(inMsg);
}
}