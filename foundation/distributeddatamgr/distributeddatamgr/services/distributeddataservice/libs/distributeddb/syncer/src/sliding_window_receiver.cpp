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

#include "sliding_window_receiver.h"
#include "sync_task_context.h"

namespace DistributedDB {
SlidingWindowReceiver::~SlidingWindowReceiver()
{
    Clear();
    dataSync_ = nullptr;
    context_ = nullptr;
}

void SlidingWindowReceiver::Clear()
{
    StopTimer();
    std::lock_guard<std::mutex> lock(lock_);
    ClearMap();
    isWaterMarkErrHappened_ = false;
}

int SlidingWindowReceiver::Initialize(SingleVerSyncTaskContext *context,
    std::shared_ptr<SingleVerDataSync> &dataSync)
{
    if (context == nullptr || dataSync == nullptr) {
        LOGE("[slwr] Initialize invalid args");
        return -E_INVALID_ARGS;
    }
    context_ = context;
    dataSync_ = dataSync;
    return E_OK;
}

int SlidingWindowReceiver::Receive(Message *inMsg)
{
    LOGD("[slwr] receive msg");
    if (inMsg == nullptr) {
        return -E_INVALID_ARGS;
    }
    int errCode = PutMsg(inMsg);
    if (errCode == E_OK) {
        StopTimer();
        StartTimer();
        DealMsg();
        return -E_NOT_NEED_DELETE_MSG;
    }
    return errCode;
}

int SlidingWindowReceiver::PutMsg(Message *inMsg)
{
    const DataRequestPacket *packet = inMsg->GetObject<DataRequestPacket>();
    if (packet == nullptr) {
        return -E_INVALID_ARGS;
    }
    uint32_t sessionId = inMsg->GetSessionId();
    uint32_t sequenceId = inMsg->GetSequenceId();
    bool isLastSequence = packet->IsLastSequence();
    uint64_t packetId = packet->GetPacketId(); // above 102 version data request reserve[0] store packetId value
    std::unique_lock<std::mutex> lock(lock_);
    if (workingId_ != 0 && sessionId_ != 0) {
        LOGI("[PutMsg] task is running, wait for workdingId=%u end,seId=%u", workingId_, sequenceId);
        workingTaskcv_.wait(lock);
    }
    if (sessionId_ != sessionId) {
        ResetInfo();
        sessionId_ = sessionId;
        messageMap_[sequenceId] = inMsg;
        SetEndField(isLastSequence, sequenceId);
        return E_OK;
    }
    // maybe remote has not receive ack, we resend ack.
    if (sequenceId <= hasFinishedMaxId_) {
        LOGI("[slwr] seId=%u,FinishedMId_=%u,label=%s", sequenceId, hasFinishedMaxId_, dataSync_->GetLabel().c_str());
        lock.unlock();
        dataSync_->SendAck(context_, sessionId_, sequenceId, packetId);
        return -E_SLIDING_WINDOW_RECEIVER_INVALID_MSG;
    }
    int errCode = ErrHandle(sequenceId);
    if (errCode != E_OK) {
        return errCode;
    }
    if (messageMap_.count(sequenceId) > 0) {
        LOGI("[slwr] PutMsg sequenceId already in map");
        delete messageMap_[sequenceId];
        messageMap_[sequenceId] = nullptr;
    }
    messageMap_[sequenceId] = inMsg;
    SetEndField(isLastSequence, sequenceId);
    return E_OK;
}

void SlidingWindowReceiver::DealMsg()
{
    while (true) {
        Message *msg = nullptr;
        {
            std::lock_guard<std::mutex> lock(lock_);
            if (workingId_ != 0 || messageMap_.count(hasFinishedMaxId_ + 1) == 0) {
                LOGI("[slwr] DealMsg do nothing workingId_=%u,hasFinishedMaxId_=%u,label=%s,deviceId=%s{private}",
                    workingId_, hasFinishedMaxId_, dataSync_->GetLabel().c_str(), context_->GetDeviceId().c_str());
                return;
            }
            workingId_ = hasFinishedMaxId_ + 1;
            msg = messageMap_[workingId_];
            messageMap_.erase(workingId_);
            LOGI("[slwr] DealMsg workingId_=%u,label=%s,deviceId=%s{private}", workingId_,
                dataSync_->GetLabel().c_str(), context_->GetDeviceId().c_str());
        }
        int errCode = context_->HandleDataRequestRecv(msg);
        delete msg;
        msg = nullptr;
        {
            std::lock_guard<std::mutex> lock(lock_);
            workingId_ = 0;
            bool isWaterMarkErr = context_->IsReceiveWaterMarkErr();
            if (isWaterMarkErr || errCode == -E_NEED_ABILITY_SYNC) {
                ClearMap();
                hasFinishedMaxId_ = 0;
                endId_ = 0;
                isWaterMarkErrHappened_ = true;
            } else {
                hasFinishedMaxId_++;
                LOGI("[slwr] DealMsg ok hasFinishedMaxId_=%u,label=%s,deviceId=%s{private}", hasFinishedMaxId_,
                    dataSync_->GetLabel().c_str(), context_->GetDeviceId().c_str());
            }
            context_->SetReceiveWaterMarkErr(false);
            workingTaskcv_.notify_all();
        }
    }
}

int SlidingWindowReceiver::TimeOut(TimerId timerId)
{
    {
        std::lock_guard<std::mutex> lock(lock_);
        LOGI("[slwr] TimeOut,timerId[%llu], timerId_[%llu]", timerId, timerId_);
        if (timerId == timerId_) {
            ClearMap();
        }
        timerId_ = 0;
    }
    RuntimeContext::GetInstance()->RemoveTimer(timerId);
    return E_OK;
}

void SlidingWindowReceiver::StartTimer()
{
    LOGD("[slwr] StartTimer");
    std::lock_guard<std::mutex> lock(lock_);
    TimerId timerId = 0;
    RefObject::IncObjRef(context_);
    TimerAction timeOutCallback = std::bind(&SlidingWindowReceiver::TimeOut, this, std::placeholders::_1);
    int errCode = RuntimeContext::GetInstance()->SetTimer(IDLE_TIME_OUT, timeOutCallback,
        [this]() {
            int errCode = RuntimeContext::GetInstance()->ScheduleTask([this]() {
                RefObject::DecObjRef(context_);
            });
            if (errCode != E_OK) {
                LOGE("[slwr] timer finalizer ScheduleTask, errCode=%d", errCode);
            }
        }, timerId);
    if (errCode != E_OK) {
        RefObject::DecObjRef(context_);
        LOGE("[slwr] timer ScheduleTask, errCode=%d", errCode);
        return;
    }
    timerId_ = timerId;
}

void SlidingWindowReceiver::StopTimer()
{
    TimerId timerId;
    {
        std::lock_guard<std::mutex> lock(lock_);
        LOGD("[slwr] StopTimer,remove Timer id[%llu]", timerId_);
        timerId = timerId_;
        if (timerId_ == 0) {
            return;
        }
        timerId_ = 0;
    }
    RuntimeContext::GetInstance()->RemoveTimer(timerId);
}

void SlidingWindowReceiver::ClearMap()
{
    LOGD("[slwr] ClearMap");
    for (auto &iter : messageMap_) {
        delete iter.second;
        iter.second = nullptr;
    }
    messageMap_.clear();
}

void SlidingWindowReceiver::ResetInfo()
{
    ClearMap();
    hasFinishedMaxId_ = 0;
    workingId_ = 0;
    endId_ = 0;
}

int SlidingWindowReceiver::ErrHandle(uint32_t sequenceId)
{
    if (sequenceId == workingId_ || (endId_ != 0 && sequenceId > endId_)) {
        LOGI("[slwr] PutMsg sequenceId:%u, endId_:%u, workingId_:%u!", sequenceId, endId_, workingId_);
        return -E_SLIDING_WINDOW_RECEIVER_INVALID_MSG;
    }
    // if waterMark err when DealMsg(), the waterMark of msg in messageMap_ is also err, so we clear messageMap_
    // but before the sender receive the waterMark err ack, the receiver may receive waterMark err msg, so before
    // receiver receive the sequenceId 1 msg, it will drop msg.
    // note, there is a low probability risk, if sender receive the waterMark err ack, it correct the warterMark then
    // resend data, if the msg of bigger sequenceId arrive earlier than sequenceId 1, it will be drop, the sender
    // will sync timeout.
    if (isWaterMarkErrHappened_) {
        if (sequenceId == 1) {
            isWaterMarkErrHappened_ = false;
        } else {
            return -E_SLIDING_WINDOW_RECEIVER_INVALID_MSG; // drop invalid packet
        }
    }
    return E_OK;
}

void SlidingWindowReceiver::SetEndField(bool isLastSequence, uint32_t sequenceId)
{
    if (isLastSequence) {
        endId_ = sequenceId;
    }
}
}  // namespace DistributedDB