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

#include "communicator.h"
#include "db_common.h"
#include "log_print.h"
#include "protocol_proto.h"

namespace DistributedDB {
Communicator::Communicator(CommunicatorAggregator *inCommAggregator, const LabelType &inLabel)
    : commAggrHandle_(inCommAggregator), commLabel_(inLabel)
{
    RefObject::IncObjRef(commAggrHandle_); // Rely on CommunicatorAggregator, hold its reference.
}

Communicator:: ~Communicator()
{
    RefObject::DecObjRef(commAggrHandle_); // Communicator no longer hold the reference of CommunicatorAggregator.
    onMessageHandle_ = nullptr;
    onConnectHandle_ = nullptr;
    onSendableHandle_ = nullptr;
    commAggrHandle_ = nullptr;
}

int Communicator::RegOnMessageCallback(const OnMessageCallback &onMessage, const Finalizer &inOper)
{
    std::lock_guard<std::mutex> messageHandleLockGuard(messageHandleMutex_);
    return RegCallBack(onMessage, onMessageHandle_, inOper, onMessageFinalizer_);
}

int Communicator::RegOnConnectCallback(const OnConnectCallback &onConnect, const Finalizer &inOper)
{
    std::lock_guard<std::mutex> connectHandleLockGuard(connectHandleMutex_);
    int errCode = RegCallBack(onConnect, onConnectHandle_, inOper, onConnectFinalizer_);
    if (onConnect && errCode == E_OK) {
        // Register action and success
        for (auto &entry : onlineTargets_) {
            LOGI("[Comm][RegConnect] Label=%s, online target=%s{private}.", VEC_TO_STR(commLabel_), entry.c_str());
            onConnectHandle_(entry, true);
        }
    }
    return errCode;
}

int Communicator::RegOnSendableCallback(const std::function<void(void)> &onSendable, const Finalizer &inOper)
{
    std::lock_guard<std::mutex> sendableHandleLockGuard(sendableHandleMutex_);
    return RegCallBack(onSendable, onSendableHandle_, inOper, onSendableFinalizer_);
}

void Communicator::Activate()
{
    commAggrHandle_->ActivateCommunicator(commLabel_);
}

uint32_t Communicator::GetCommunicatorMtuSize() const
{
    return commAggrHandle_->GetCommunicatorAggregatorMtuSize();
}

uint32_t Communicator::GetCommunicatorMtuSize(const std::string &target) const
{
    return commAggrHandle_->GetCommunicatorAggregatorMtuSize(target);
}

int Communicator::GetLocalIdentity(std::string &outTarget) const
{
    return commAggrHandle_->GetLocalIdentity(outTarget);
}

int Communicator::SendMessage(const std::string &dstTarget, const Message *inMsg, bool nonBlock, uint32_t timeout)
{
    return SendMessage(dstTarget, inMsg, nonBlock, timeout, nullptr);
}

int Communicator::SendMessage(const std::string &dstTarget, const Message *inMsg, bool nonBlock, uint32_t timeout,
    const OnSendEnd &onEnd)
{
    if (dstTarget.size() == 0 || inMsg == nullptr) {
        return -E_INVALID_ARGS;
    }

    int error = E_OK;
    // if error is not E_OK , null pointer will be returned
    SerialBuffer *buffer = ProtocolProto::ToSerialBuffer(inMsg, error);
    if (error != E_OK) {
        LOGE("[Comm][Send] Serial fail, label=%s, error=%d.", VEC_TO_STR(commLabel_), error);
        return error;
    }
    int errCode = ProtocolProto::SetDivergeHeader(buffer, commLabel_);
    if (errCode != E_OK) {
        LOGE("[Comm][Send] Set header fail, label=%s, errCode=%d.", VEC_TO_STR(commLabel_), errCode);
        delete buffer;
        buffer = nullptr;
        return errCode;
    }

    TaskConfig config{nonBlock, timeout, inMsg->GetPriority()};
    errCode = commAggrHandle_->CreateSendTask(dstTarget, buffer, FrameType::APPLICATION_MESSAGE, config, onEnd);
    if (errCode == E_OK) {
        // if ok, free inMsg, otherwise the caller should take over inMsg
        delete inMsg;
        inMsg = nullptr;
    } else {
        // if send fails, free buffer, otherwise buffer should be taked over by comminucator aggregator
        delete buffer;
        buffer = nullptr;
    }
    return errCode;
}

void Communicator::OnBufferReceive(const std::string &srcTarget, const SerialBuffer *inBuf)
{
    std::lock_guard<std::mutex> messageHandleLockGuard(messageHandleMutex_);
    if (srcTarget.size() != 0 && inBuf != nullptr && onMessageHandle_) {
        int error = E_OK;
        // if error is not E_OK, null pointer will be returned
        Message *message = ProtocolProto::ToMessage(inBuf, error);
        delete inBuf;
        inBuf = nullptr;
        // message is not nullptr if error is E_OK or error is E_NOT_REGISTER.
        // for the former case the message will be handled and release by sync module.
        // for the latter case the message is released in TriggerUnknownMessageFeedback.
        if (error != E_OK) {
            LOGE("[Comm][Receive] ToMessage fail, label=%s, error=%d.", VEC_TO_STR(commLabel_), error);
            if (error == -E_VERSION_NOT_SUPPORT) {
                TriggerVersionNegotiation(srcTarget);
            } else if (error == -E_NOT_REGISTER) {
                TriggerUnknownMessageFeedback(srcTarget, message);
            }
            return;
        }
        LOGI("[Comm][Receive] label=%s, srcTarget=%s{private}.", VEC_TO_STR(commLabel_), srcTarget.c_str());
        onMessageHandle_(srcTarget, message);
    } else {
        LOGE("[Comm][Receive] label=%s, src.size=%zu or buf or handle invalid.", VEC_TO_STR(commLabel_),
            srcTarget.size());
        if (inBuf != nullptr) {
            delete inBuf;
            inBuf = nullptr;
        }
    }
}

void Communicator::OnConnectChange(const std::string &target, bool isConnect)
{
    std::lock_guard<std::mutex> connectHandleLockGuard(connectHandleMutex_);
    if (target.size() == 0) {
        LOGE("[Comm][Connect] Target size zero, label=%s.", VEC_TO_STR(commLabel_));
        return;
    }
    if (isConnect) {
        onlineTargets_.insert(target);
    } else {
        onlineTargets_.erase(target);
    }
    LOGI("[Comm][Connect] Label=%s, target=%s{private}, Online=%d", VEC_TO_STR(commLabel_), target.c_str(), isConnect);
    if (onConnectHandle_) {
        onConnectHandle_(target, isConnect);
    } else {
        LOGI("[Comm][Connect] Handle invalid currently.");
    }
}

void Communicator::OnSendAvailable()
{
    std::lock_guard<std::mutex> sendableHandleLockGuard(sendableHandleMutex_);
    if (onSendableHandle_) {
        onSendableHandle_();
    }
}

LabelType Communicator::GetCommunicatorLabel() const
{
    return commLabel_;
}

int Communicator::GetRemoteCommunicatorVersion(const std::string &target, uint16_t &outVersion) const
{
    return commAggrHandle_->GetRemoteCommunicatorVersion(target, outVersion);
}

void Communicator::TriggerVersionNegotiation(const std::string &dstTarget)
{
    LOGI("[Comm][TrigVer] Do version negotiate with target=%s{private}.", dstTarget.c_str());
    int errCode = E_OK;
    SerialBuffer *buffer = ProtocolProto::BuildEmptyFrameForVersionNegotiate(errCode);
    if (errCode != E_OK) {
        LOGE("[Comm][TrigVer] Build empty frame fail, errCode=%d", errCode);
        return;
    }

    TaskConfig config{true, 0, Priority::HIGH};
    errCode = commAggrHandle_->CreateSendTask(dstTarget, buffer, FrameType::EMPTY, config);
    if (errCode != E_OK) {
        LOGE("[Comm][TrigVer] Send empty frame fail, errCode=%d", errCode);
        // if send fails, free buffer, otherwise buffer will be taked over by comminucator aggregator
        delete buffer;
        buffer = nullptr;
    }
}

void Communicator::TriggerUnknownMessageFeedback(const std::string &dstTarget, Message* &oriMsg)
{
    if (oriMsg == nullptr || oriMsg->GetMessageType() != TYPE_REQUEST) {
        LOGI("[Comm][TrigFeedback] Do nothing for unknown message with type not request.");
        // Do not have to do feedback if the message is not a request type message
        delete oriMsg;
        oriMsg = nullptr;
        return;
    }

    LOGI("[Comm][TrigFeedback] Do unknown message feedback with target=%s{private}.", dstTarget.c_str());
    oriMsg->SetMessageType(TYPE_RESPONSE);
    oriMsg->SetErrorNo(E_FEEDBACK_UNKNOWN_MESSAGE);

    int errCode = E_OK;
    SerialBuffer *buffer = ProtocolProto::BuildFeedbackMessageFrame(oriMsg, commLabel_, errCode);
    delete oriMsg;
    oriMsg = nullptr;
    if (errCode != E_OK) {
        LOGE("[Comm][TrigFeedback] Build unknown message feedback frame fail, errCode=%d", errCode);
        return;
    }

    TaskConfig config{true, 0, Priority::HIGH};
    errCode = commAggrHandle_->CreateSendTask(dstTarget, buffer, FrameType::APPLICATION_MESSAGE, config);
    if (errCode != E_OK) {
        LOGE("[Comm][TrigFeedback] Send unknown message feedback frame fail, errCode=%d", errCode);
        // if send fails, free buffer, otherwise buffer will be taked over by comminucator aggregator
        delete buffer;
        buffer = nullptr;
    }
}

DEFINE_OBJECT_TAG_FACILITIES(Communicator)
} // namespace DistributedDB
