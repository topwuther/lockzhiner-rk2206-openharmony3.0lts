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

#include "sliding_window_sender.h"

namespace DistributedDB {
SlidingWindowSender::~SlidingWindowSender()
{
    Clear();
    context_ = nullptr;
    dataSync_ = nullptr;
}

int SlidingWindowSender::ParamCheck(int32_t mode, const SingleVerSyncTaskContext *context,
    std::shared_ptr<SingleVerDataSync> &dataSync)
{
    if (context == nullptr) {
        LOGE("[slws] SendStart invalid context");
        return -E_INVALID_ARGS;
    }
    if (dataSync == nullptr) {
        LOGE("[slws] SendStart invalid context");
        return -E_INVALID_ARGS;
    }
    return E_OK;
}

void SlidingWindowSender::Init(int32_t mode, SingleVerSyncTaskContext *context,
    std::shared_ptr<SingleVerDataSync> &dataSync)
{
    isErr_ = false;
    mode_ = mode;
    context_->SetSessionEndTimeStamp(0);
    context_->ReSetSequenceId();
    maxSequenceIdhasSent_ = 0;
    windowSize_ = MAX_WINDOW_SIZE;
    if (mode == SyncOperation::PUSH || mode == SyncOperation::PUSH_AND_PULL || mode == SyncOperation::PULL) {
        sessionId_ = context->GetRequestSessionId();
    } else {
        sessionId_ = context->GetResponseSessionId();
    }
}

int SlidingWindowSender::Initialize(SingleVerSyncTaskContext *context, std::shared_ptr<SingleVerDataSync> &dataSync)
{
    if (context == nullptr || dataSync == nullptr) {
        LOGE("[slws] Initialize invalid args");
        return -E_INVALID_ARGS;
    }
    context_ = context;
    dataSync_ = dataSync;
    return E_OK;
}

int SlidingWindowSender::SendStart(int32_t mode, SingleVerSyncTaskContext *context,
    std::shared_ptr<SingleVerDataSync> &dataSync)
{
    int errCode = ParamCheck(mode, context, dataSync);
    if (errCode != E_OK) {
        return errCode;
    }
    LOGI("[slws] SendStart,mode=%d,label=%s,device=%s{private}", mode_, dataSync->GetLabel().c_str(),
        context->GetDeviceId().c_str());
    errCode = dataSync->CheckPermitSendData(mode, context);
    if (errCode != E_OK) {
        return errCode;
    }
    std::lock_guard<std::mutex> lock(lock_);
    if (sessionId_ != 0) { // auto sync timeout resend
        return ReSend();
    }
    Init(mode, context, dataSync);
    if (mode == SyncOperation::PUSH) {
        errCode = dataSync->PushStart(context);
    } else if (mode == SyncOperation::PUSH_AND_PULL) {
        errCode = dataSync->PushPullStart(context);
    } else if (mode == SyncOperation::PULL) {
        errCode = dataSync->PullRequestStart(context);
    } else {
        errCode = dataSync->PullResponseStart(context);
    }
    if (context->IsSkipTimeoutError(errCode)) {
        // if E_TIMEOUT occurred, means send message pressure is high, put into resend map and wait for resend.
        // just return to avoid higher pressure for send.
        UpdateInfo();
        return errCode;
    }
    if (errCode != E_OK) {
        LOGE("[slws] SendStart errCode=%d", errCode);
        return errCode;
    }
    errCode = UpdateInfo();
    if (mode == SyncOperation::PUSH_AND_PULL && context_->GetTaskErrCode() == -E_EKEYREVOKED) {
        LOGE("wait for recv finished for push and pull mode");
        return -E_EKEYREVOKED;
    }
    if (errCode == -E_FINISHED) {
        return E_OK;
    }
    return InnerSend();
}

int SlidingWindowSender::UpdateInfo()
{
    ReSendInfo reSendInfo;
    reSendInfo.start = context_->GetSequenceStartTimeStamp();
    reSendInfo.end = context_->GetSequenceEndTimeStamp();
    reSendInfo.packetId = context_->GetPacketId();
    maxSequenceIdhasSent_++;
    reSendMap_[maxSequenceIdhasSent_] = reSendInfo;
    windowSize_--;
    LOGI("[slws] mode=%d,start=%llu,end=%llu,seqId=%d,packetId=%llu,window_size=%d,label=%s,device=%s{private}", mode_,
        reSendInfo.start, reSendInfo.end, maxSequenceIdhasSent_, reSendInfo.packetId, windowSize_,
        dataSync_->GetLabel().c_str(), context_->GetDeviceId().c_str());
    ContinueToken token;
    context_->GetContinueToken(token);
    if (token == nullptr) {
        isAllDataHasSent_ = true;
        LOGI("[slws] UpdateInfo all data has sent");
        return -E_FINISHED;
    }
    return E_OK;
}

int SlidingWindowSender::InnerSend()
{
    while (true) {
        if (windowSize_ <= 0 || isAllDataHasSent_) {
            LOGI("[slws] InnerSend windowSize_=%d,isAllDataHasSent_=%d", windowSize_, isAllDataHasSent_);
            return E_OK;
        }
        if (isErr_ || context_ == nullptr) {
            LOGI("[slws] InnerSend isErr");
            return -E_SLIDING_WINDOW_SENDER_ERR;
        }
        int errCode;
        context_->IncSequenceId();
        if (mode_ == SyncOperation::PUSH || mode_ == SyncOperation::PUSH_AND_PULL) {
            errCode = dataSync_->PushStart(context_);
        } else {
            errCode = dataSync_->PullResponseStart(context_);
        }
        if (mode_ == SyncOperation::PUSH_AND_PULL && errCode == -E_EKEYREVOKED) {
            LOGE("errCode = %d, wait for recv finished for push and pull mode", errCode);
            isAllDataHasSent_ = true;
            return -E_EKEYREVOKED;
        }
        if (context_->IsSkipTimeoutError(errCode)) {
            // if E_TIMEOUT occurred, means send message pressure is high, put into resend map and wait for resend.
            // just return to avoid higher pressure for send.
            UpdateInfo();
            return errCode;
        }
        if (errCode != E_OK) {
            isErr_ = true;
            LOGE("[slws] InnerSend errCode=%d", errCode);
            return errCode;
        }
        errCode = UpdateInfo();
        if (errCode == -E_FINISHED) {
            return E_OK;
        }
    }
}

int SlidingWindowSender::PreHandleAckRecv(const Message *message)
{
    if (message == nullptr) {
        LOGE("[slws] AckRecv message nullptr");
        return -E_INVALID_ARGS;
    }
    if (message->GetMessageType() == TYPE_NOTIFY) {
        return E_OK;
    }
    const DataAckPacket *packet = message->GetObject<DataAckPacket>();
    if (packet == nullptr) {
        return -E_INVALID_ARGS;
    }
    uint64_t packetId = packet->GetPacketId(); // above 102 version data request reserve[0] store packetId value
    std::lock_guard<std::mutex> lock(lock_);
    uint32_t sequenceId = message->GetSequenceId();
    if (reSendMap_.count(sequenceId) != 0) {
        uint64_t originalPacketId = reSendMap_[sequenceId].packetId;
        if (DataAckPacket::IsPacketIdValid(packetId) && packetId != originalPacketId) {
            LOGE("[slws] packetId[%llu] is not match with original[%llu]", packetId, originalPacketId);
            return -E_SLIDING_WINDOW_RECEIVER_INVALID_MSG;
        }
    }
    return E_OK;
}

int SlidingWindowSender::AckRecv(const Message *message)
{
    if (message == nullptr) {
        LOGE("[slws] AckRecv message nullptr");
        return -E_INVALID_ARGS;
    }
    const DataAckPacket *packet = message->GetObject<DataAckPacket>();
    if (packet == nullptr) {
        return -E_INVALID_ARGS;
    }
    uint64_t packetId = packet->GetPacketId(); // above 102 version data request reserve[0] store packetId value
    uint32_t sessionId = message->GetSessionId();
    uint32_t sequenceId = message->GetSequenceId();
    LOGI("[slws] AckRecv sequecneId=%d,packetId=%llu,label=%s,dev=%s{private}", sequenceId, packetId,
        dataSync_->GetLabel().c_str(), context_->GetDeviceId().c_str());

    std::lock_guard<std::mutex> lock(lock_);
    if (sessionId != sessionId_) {
        LOGI("[slws] AckRecv sessionId is different");
        return E_OK;
    }
    if (reSendMap_.count(sequenceId) != 0) {
        reSendMap_.erase(sequenceId);
        windowSize_++;
        LOGI("[slws] AckRecv window_size=%d", windowSize_);
    } else {
        LOGI("[slws] AckRecv sequenceId not in map");
        return E_OK;
    }
    if (!isAllDataHasSent_) {
        return InnerSend();
    } else if (reSendMap_.size() == 0) {
        context_->SetOperationStatus(SyncOperation::SEND_FINISHED);
        LOGI("[slws] AckRecv all finished,label=%s,dev=%s{private}", dataSync_->GetLabel().c_str(),
            context_->GetDeviceId().c_str());
        InnerClear();
        return -E_FINISHED;
    }
    return E_OK;
}

int SlidingWindowSender::ReSend() const
{
    if (isErr_) {
        LOGI("[slws] ReSend InnerSend isErr");
        return -E_INTERNAL_ERROR;
    }
    if (reSendMap_.empty()) {
        LOGI("[slws] ReSend map empty");
        return -E_INTERNAL_ERROR;
    }
    uint32_t sequenceId = reSendMap_.begin()->first;
    ReSendInfo reSendInfo = reSendMap_.begin()->second;
    LOGI("[slws] ReSend mode=%d,start=%llu,end=%llu,seqId=%d,packetId=%llu,windowsize=%d,label=%s,deviceId=%s{private}",
        mode_, reSendInfo.start, reSendInfo.end, sequenceId, reSendInfo.packetId, windowSize_,
        dataSync_->GetLabel().c_str(), context_->GetDeviceId().c_str());
    DataSyncReSendInfo dataReSendInfo = {sessionId_, sequenceId, reSendInfo.start, reSendInfo.end, reSendInfo.packetId};
    return dataSync_->ReSend(context_, dataReSendInfo);
}

void SlidingWindowSender::Clear()
{
    std::lock_guard<std::mutex> lock(lock_);
    LOGD("[slws] clear sender info");
    InnerClear();
}

void SlidingWindowSender::InnerClear()
{
    sessionId_ = 0;
    reSendMap_.clear();
    windowSize_ = 0;
    maxSequenceIdhasSent_ = 0;
    isAllDataHasSent_ = false;
    isErr_ = false;
}

void SlidingWindowSender::SetErr(bool isErr)
{
    std::lock_guard<std::mutex> lock(lock_);
    isErr_ = isErr;
}
}  // namespace DistributedDB