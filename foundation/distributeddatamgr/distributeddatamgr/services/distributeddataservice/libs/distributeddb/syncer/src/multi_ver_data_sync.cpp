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

#ifndef OMIT_MULTI_VER
#include "multi_ver_data_sync.h"

#include "parcel.h"
#include "log_print.h"
#include "sync_types.h"
#include "message_transform.h"
#include "performance_analysis.h"
#include "db_constant.h"

namespace DistributedDB {
// Class MultiVerRequestPacket
uint32_t MultiVerRequestPacket::CalculateLen() const
{
    uint64_t len = Parcel::GetIntLen();
    len = Parcel::GetEightByteAlign(len);
    len += Parcel::GetMultiVerCommitLen(commit_);
    if (len > INT32_MAX) {
        return 0;
    }
    return len;
}

void MultiVerRequestPacket::SetCommit(MultiVerCommitNode &commit)
{
    commit_ = std::move(commit);
}

void MultiVerRequestPacket::GetCommit(MultiVerCommitNode &commit) const
{
    commit = commit_;
}

void MultiVerRequestPacket::SetErrCode(int32_t errCode)
{
    errCode_ = errCode;
}

int32_t MultiVerRequestPacket::GetErrCode() const
{
    return errCode_;
}

// Class MultiVerAckPacket
uint32_t MultiVerAckPacket::CalculateLen() const
{
    uint64_t len = Parcel::GetIntLen();
    len = Parcel::GetEightByteAlign(len);
    for (const auto &iter : entries_) {
        len += Parcel::GetVectorCharLen(iter);
        if (len > INT32_MAX) {
            return 0;
        }
    }
    return len;
}

void MultiVerAckPacket::SetData(std::vector<std::vector<uint8_t>> &data)
{
    entries_ = std::move(data);
}

void MultiVerAckPacket::GetData(std::vector<std::vector<uint8_t>> &data) const
{
    data = entries_;
}

void MultiVerAckPacket::SetErrorCode(int32_t errCode)
{
    errorCode_ = errCode;
}

void MultiVerAckPacket::GetErrorCode(int32_t &errCode) const
{
    errCode = errorCode_;
}

// Class MultiVerDataSync
MultiVerDataSync::~MultiVerDataSync()
{
    storagePtr_ = nullptr;
    communicateHandle_ = nullptr;
}

int MultiVerDataSync::Serialization(uint8_t *buffer, uint32_t length, const Message *inMsg)
{
    if ((buffer == nullptr) || !(IsPacketValid(inMsg, TYPE_RESPONSE) || IsPacketValid(inMsg, TYPE_REQUEST))) {
        return -E_INVALID_ARGS;
    }

    switch (inMsg->GetMessageType()) {
        case TYPE_REQUEST:
            return RequestPacketSerialization(buffer, length, inMsg);
        case TYPE_RESPONSE:
            return AckPacketSerialization(buffer, length, inMsg);
        default:
            return -E_MESSAGE_TYPE_ERROR;
    }
}

int MultiVerDataSync::DeSerialization(const uint8_t *buffer, uint32_t length, Message *inMsg)
{
    if ((buffer == nullptr) || !(IsPacketValid(inMsg, TYPE_RESPONSE) || IsPacketValid(inMsg, TYPE_REQUEST))) {
        return -E_MESSAGE_ID_ERROR;
    }

    switch (inMsg->GetMessageType()) {
        case TYPE_REQUEST:
            return RequestPacketDeSerialization(buffer, length, inMsg);
        case TYPE_RESPONSE:
            return AckPacketDeSerialization(buffer, length, inMsg);
        default:
            return -E_MESSAGE_TYPE_ERROR;
    }
}

uint32_t MultiVerDataSync::CalculateLen(const Message *inMsg)
{
    if (!(IsPacketValid(inMsg, TYPE_RESPONSE) || IsPacketValid(inMsg, TYPE_REQUEST))) {
        return 0;
    }

    uint32_t len = 0;
    int errCode = E_OK;
    switch (inMsg->GetMessageType()) {
        case TYPE_REQUEST:
            errCode = RequestPacketCalculateLen(inMsg, len);
            break;
        case TYPE_RESPONSE:
            errCode = AckPacketCalculateLen(inMsg, len);
            break;
        default:
            return 0;
    }
    if (errCode != E_OK) {
        return 0;
    }
    return len;
}

int MultiVerDataSync::RegisterTransformFunc()
{
    TransformFunc func;
    func.computeFunc = std::bind(&MultiVerDataSync::CalculateLen, std::placeholders::_1);
    func.serializeFunc = std::bind(&MultiVerDataSync::Serialization, std::placeholders::_1,
                                   std::placeholders::_2, std::placeholders::_3);
    func.deserializeFunc = std::bind(&MultiVerDataSync::DeSerialization, std::placeholders::_1,
                                     std::placeholders::_2, std::placeholders::_3);
    return MessageTransform::RegTransformFunction(MULTI_VER_DATA_SYNC_MESSAGE, func);
}

int MultiVerDataSync::Initialize(MultiVerKvDBSyncInterface *storagePtr, ICommunicator *communicateHandle)
{
    if ((storagePtr == nullptr) || (communicateHandle == nullptr)) {
        return -E_INVALID_ARGS;
    }
    storagePtr_ = storagePtr;
    communicateHandle_ = communicateHandle;
    return E_OK;
}

void MultiVerDataSync::TimeOutCallback(MultiVerSyncTaskContext *context, const Message *message) const
{
    return;
}

int MultiVerDataSync::SyncStart(MultiVerSyncTaskContext *context)
{
    if (context == nullptr) {
        return -E_INVALID_ARGS;
    }
    LOGD("MultiVerDataSync::SyncStart dst=%s{private}, begin", context->GetDeviceId().c_str());
    PerformanceAnalysis *performance = PerformanceAnalysis::GetInstance();
    if (performance != nullptr) {
        performance->StepTimeRecordStart(MV_TEST_RECORDS::RECORD_DATA_GET_VALID_COMMIT);
    }
    MultiVerCommitNode commit;
    int errCode = GetValidCommit(context, commit);
    if (performance != nullptr) {
        performance->StepTimeRecordEnd(MV_TEST_RECORDS::RECORD_DATA_GET_VALID_COMMIT);
    }
    if (errCode != E_OK) {
        // sync don't need start
        SendFinishedRequest(context);
        return errCode;
    }

    errCode = SendRequestPacket(context, commit);
    LOGD("MultiVerDataSync::SyncStart dst=%s{private}, end", context->GetDeviceId().c_str());
    return errCode;
}

int MultiVerDataSync::RequestRecvCallback(const MultiVerSyncTaskContext *context, const Message *message)
{
    if (message == nullptr || context == nullptr) {
        return -E_INVALID_ARGS;
    }

    if (!IsPacketValid(message, TYPE_REQUEST)) {
        return -E_INVALID_ARGS;
    }

    const MultiVerRequestPacket *packet = message->GetObject<MultiVerRequestPacket>();
    if (packet == nullptr) {
        return -E_INVALID_ARGS;
    }
    if (packet->GetErrCode() == -E_LAST_SYNC_FRAME) {
        return -E_LAST_SYNC_FRAME;
    }
    MultiVerCommitNode commit;
    packet->GetCommit(commit);
    PerformanceAnalysis *performance = PerformanceAnalysis::GetInstance();
    if (performance != nullptr) {
        performance->StepTimeRecordStart(MV_TEST_RECORDS::RECORD_GET_COMMIT_DATA);
    }
    std::vector<MultiVerKvEntry *> dataEntries;
    int errCode = GetCommitData(commit, dataEntries);
    if (performance != nullptr) {
        performance->StepTimeRecordEnd(MV_TEST_RECORDS::RECORD_GET_COMMIT_DATA);
    }
    if (errCode != E_OK) {
        LOGE("MultiVerDataSync::RequestRecvCallback : GetCommitData ERR, errno = %d", errCode);
    }

    errCode = SendAckPacket(context, dataEntries, errCode, message);
    for (auto &iter : dataEntries) {
        ReleaseKvEntry(iter);
        iter = nullptr;
    }
    LOGD("MultiVerDataSync::RequestRecvCallback : SendAckPacket, errno = %d, dst = %s{private}",
         errCode, context->GetDeviceId().c_str());
    return errCode;
}

int MultiVerDataSync::AckRecvCallback(MultiVerSyncTaskContext *context, const Message *message)
{
    if (message == nullptr) {
        return -E_INVALID_ARGS;
    }
    if (!IsPacketValid(message, TYPE_RESPONSE) || (context == nullptr)) {
        return -E_INVALID_ARGS;
    }

    const MultiVerAckPacket *packet = message->GetObject<MultiVerAckPacket>();
    if (packet == nullptr) {
        return -E_INVALID_ARGS;
    }
    int32_t errCode = E_OK;
    packet->GetErrorCode(errCode);
    if (errCode != E_OK) {
        return errCode;
    }
    std::vector<std::vector<uint8_t>> dataEntries;
    std::vector<MultiVerKvEntry *> entries;
    std::vector<ValueSliceHash> valueHashes;
    MultiVerKvEntry *entry = nullptr;

    packet->GetData(dataEntries);
    for (auto &iter : dataEntries) {
        MultiVerKvEntry *item = CreateKvEntry(iter);
        entries.push_back(item);
    }
    context->ReleaseEntries();
    context->SetEntries(entries);
    context->SetEntriesIndex(0);
    context->SetEntriesSize(entries.size());
    LOGD("MultiVerDataSync::AckRecvCallback src=%s{private}, entries num = %llu",
        context->GetDeviceId().c_str(), entries.size());

    if (entries.size() > 0) {
        entry = entries[0];
        errCode = entry->GetValueHash(valueHashes);
        if (errCode != E_OK) {
            return errCode;
        }
    }
    context->SetValueSliceHashNodes(valueHashes);
    context->SetValueSlicesIndex(0);
    context->SetValueSlicesSize(valueHashes.size());
    LOGD("MultiVerDataSync::AckRecvCallback src=%s{private}, ValueSlicesSize num = %llu",
        context->GetDeviceId().c_str(), valueHashes.size());
    return errCode;
}

int MultiVerDataSync::PutCommitData(const MultiVerCommitNode &commit, const std::vector<MultiVerKvEntry *> &entries,
    const std::string &deviceName)
{
    return storagePtr_->PutCommitData(commit, entries, deviceName);
}

int MultiVerDataSync::MergeSyncCommit(const MultiVerCommitNode &commit, const std::vector<MultiVerCommitNode> &commits)
{
    return storagePtr_->MergeSyncCommit(commit, commits);
}

void MultiVerDataSync::ReleaseKvEntry(const MultiVerKvEntry *entry)
{
    return storagePtr_->ReleaseKvEntry(entry);
}

void MultiVerDataSync::SendFinishedRequest(const MultiVerSyncTaskContext *context)
{
    if (context == nullptr) {
        return;
    }
    MultiVerRequestPacket *packet = new (std::nothrow) MultiVerRequestPacket();
    if (packet == nullptr) {
        LOGE("MultiVerRequestPacket::SendRequestPacket : new packet error");
        return;
    }
    packet->SetErrCode(-E_LAST_SYNC_FRAME);
    Message *message = new (std::nothrow) Message(MULTI_VER_DATA_SYNC_MESSAGE);
    if (message == nullptr) {
        delete packet;
        packet = nullptr;
        LOGE("MultiVerDataSync::SendRequestPacket : new message error");
        return;
    }
    message->SetMessageType(TYPE_REQUEST);
    message->SetTarget(context->GetDeviceId());
    int errCode = message->SetExternalObject(packet);
    if (errCode != E_OK) {
        delete packet;
        packet = nullptr;
        delete message;
        message = nullptr;
        LOGE("[MultiVerDataSync][SendFinishedRequest] : SetExternalObject failed errCode:%d", errCode);
        return;
    }
    message->SetSessionId(context->GetRequestSessionId());
    message->SetSequenceId(context->GetSequenceId());

    errCode = Send(message->GetTarget(), message);
    if (errCode != E_OK) {
        delete message;
        message = nullptr;
        LOGE("[MultiVerDataSync][SendFinishedRequest] SendFinishedRequest failed, err %d", errCode);
    }
    LOGI("[MultiVerDataSync][SendFinishedRequest] SendFinishedRequest dst=%s{private}", context->GetDeviceId().c_str());
}

int MultiVerDataSync::RequestPacketCalculateLen(const Message *inMsg, uint32_t &len)
{
    if ((inMsg == nullptr) || !IsPacketValid(inMsg, TYPE_REQUEST)) {
        return -E_INVALID_ARGS;
    }
    const MultiVerRequestPacket *packet = inMsg->GetObject<MultiVerRequestPacket>();
    if (packet == nullptr) {
        return -E_INVALID_ARGS;
    }

    len = packet->CalculateLen();
    return E_OK;
}

int MultiVerDataSync::RequestPacketSerialization(uint8_t *buffer, uint32_t length, const Message *inMsg)
{
    if ((buffer == nullptr) || !IsPacketValid(inMsg, TYPE_REQUEST)) {
        return -E_INVALID_ARGS;
    }
    const MultiVerRequestPacket *packet = inMsg->GetObject<MultiVerRequestPacket>();
    if ((packet == nullptr) || (length != packet->CalculateLen())) {
        return -E_INVALID_ARGS;
    }

    MultiVerCommitNode commit;
    packet->GetCommit(commit);
    int32_t ackCode = packet->GetErrCode();

    Parcel parcel(buffer, length);
    int errCode = parcel.WriteInt(ackCode);
    if (errCode != E_OK) {
        return -E_SECUREC_ERROR;
    }
    parcel.EightByteAlign();

    // commitMap Serialization
    errCode = parcel.WriteMultiVerCommit(commit);
    if (errCode != E_OK) {
        return -E_SECUREC_ERROR;
    }

    return errCode;
}

int MultiVerDataSync::RequestPacketDeSerialization(const uint8_t *buffer, uint32_t length, Message *inMsg)
{
    if ((buffer == nullptr) || !IsPacketValid(inMsg, TYPE_REQUEST)) {
        return -E_INVALID_ARGS;
    }

    MultiVerCommitNode commit;
    Parcel parcel(const_cast<uint8_t *>(buffer), length);
    int32_t pktErrCode;
    uint64_t packLen = parcel.ReadInt(pktErrCode);
    if (parcel.IsError()) {
        return -E_INVALID_ARGS;
    }
    parcel.EightByteAlign();
    packLen = Parcel::GetEightByteAlign(packLen);
    // commit DeSerialization
    packLen += parcel.ReadMultiVerCommit(commit);
    if (packLen != length || parcel.IsError()) {
        return -E_INVALID_ARGS;
    }
    MultiVerRequestPacket *packet = new (std::nothrow) MultiVerRequestPacket();
    if (packet == nullptr) {
        LOGE("MultiVerDataSync::RequestPacketDeSerialization : new packet error");
        return -E_OUT_OF_MEMORY;
    }
    packet->SetCommit(commit);
    packet->SetErrCode(pktErrCode);
    int errCode = inMsg->SetExternalObject<>(packet);
    if (errCode != E_OK) {
        delete packet;
        packet = nullptr;
    }
    return errCode;
}

int MultiVerDataSync::AckPacketCalculateLen(const Message *inMsg, uint32_t &len)
{
    if (!IsPacketValid(inMsg, TYPE_RESPONSE)) {
        return -E_INVALID_ARGS;
    }

    const MultiVerAckPacket *packet = inMsg->GetObject<MultiVerAckPacket>();
    if (packet == nullptr) {
        return -E_INVALID_ARGS;
    }
    len = packet->CalculateLen();
    return E_OK;
}

int MultiVerDataSync::AckPacketSerialization(uint8_t *buffer, uint32_t length, const Message *inMsg)
{
    if ((buffer == nullptr) || !IsPacketValid(inMsg, TYPE_RESPONSE)) {
        return -E_INVALID_ARGS;
    }
    const MultiVerAckPacket *packet = inMsg->GetObject<MultiVerAckPacket>();
    if ((packet == nullptr) || (length != packet->CalculateLen())) {
        return -E_INVALID_ARGS;
    }

    Parcel parcel(buffer, length);
    std::vector<std::vector<uint8_t>> entries;

    packet->GetData(entries);
    int32_t errCode = E_OK;
    packet->GetErrorCode(errCode);
    // errCode Serialization
    errCode = parcel.WriteInt(errCode);
    if (errCode != E_OK) {
        return -E_SECUREC_ERROR;
    }
    parcel.EightByteAlign();

    // commits vector Serialization
    for (const auto &iter : entries) {
        errCode = parcel.WriteVectorChar(iter);
        if (errCode != E_OK) {
            return -E_SECUREC_ERROR;
        }
    }

    return errCode;
}

int MultiVerDataSync::AckPacketDeSerialization(const uint8_t *buffer, uint32_t length, Message *inMsg)
{
    if ((buffer == nullptr) || !IsPacketValid(inMsg, TYPE_RESPONSE)) {
        return -E_INVALID_ARGS;
    }

    Parcel parcel(const_cast<uint8_t *>(buffer), length);
    int32_t pktErrCode;

    // errCode DeSerialization
    uint32_t packLen = parcel.ReadInt(pktErrCode);
    if (parcel.IsError()) {
        return -E_INVALID_ARGS;
    }
    parcel.EightByteAlign();
    packLen = Parcel::GetEightByteAlign(packLen);

    // commits vector DeSerialization
    std::vector<std::vector<uint8_t>> entries;
    while (packLen < length) {
        std::vector<uint8_t> data;
        packLen += parcel.ReadVectorChar(data);
        // A valid dataItem got, Save to storage
        entries.push_back(data);
        if (parcel.IsError()) {
            return -E_INVALID_ARGS;
        }
    }
    MultiVerAckPacket *packet = new (std::nothrow) MultiVerAckPacket();
    if (packet == nullptr) {
        LOGE("MultiVerDataSync::AckPacketDeSerialization : new packet error");
        return -E_OUT_OF_MEMORY;
    }
    packet->SetData(entries);
    packet->SetErrorCode(pktErrCode);
    int errCode = inMsg->SetExternalObject<>(packet);
    if (errCode != E_OK) {
        delete packet;
        packet = nullptr;
    }
    return errCode;
}

bool MultiVerDataSync::IsPacketValid(const Message *inMsg, uint16_t messageType)
{
    if ((inMsg == nullptr) || (inMsg->GetMessageId() != MULTI_VER_DATA_SYNC_MESSAGE)) {
        return false;
    }
    if (messageType != inMsg->GetMessageType()) {
        return false;
    }
    return true;
}

int MultiVerDataSync::GetValidCommit(MultiVerSyncTaskContext *context, MultiVerCommitNode &commit)
{
    int commitsSize = context->GetCommitsSize();
    if (commitsSize > DBConstant::MAX_COMMIT_SIZE) {
        LOGE("MultiVerDataSync::GetValidCommit failed, to large!");
        return -E_LENGTH_ERROR;
    }
    int index = context->GetCommitIndex();
    if (context->GetRetryStatus() == SyncTaskContext::NEED_RETRY) {
        context->SetRetryStatus(SyncTaskContext::NO_NEED_RETRY);
        index--;
    }
    index = (index < 0) ? 0 : index;
    LOGD("MultiVerDataSync::GetValidCommit begin, dst=%s{private}, index = %d", context->GetDeviceId().c_str(), index);
    while (index < commitsSize) {
        MultiVerCommitNode commitItem;
        context->GetCommit(index, commitItem);
        LOGD("MultiVerDataSync::GetValidCommit , dst=%s{private}, index = %d, commitsSize = %d",
            context->GetDeviceId().c_str(), index, commitsSize);

        index++;
        context->SetCommitIndex(index);
        if (IsCommitExisted(commitItem)) {
            continue;
        }
        commit = commitItem;
        LOGD("MultiVerDataSync::GetValidCommit ok, dst=%s{private}, commit index = %d",
             context->GetDeviceId().c_str(), index);
        return E_OK;
    }
    LOGD("MultiVerDataSync::GetValidCommit not found, dst=%s{private}", context->GetDeviceId().c_str());
    return -E_NOT_FOUND;
}

bool MultiVerDataSync::IsCommitExisted(const MultiVerCommitNode &commit)
{
    return storagePtr_->IsCommitExisted(commit);
}

int MultiVerDataSync::Send(const DeviceID &deviceId, const Message *inMsg)
{
    int errCode = communicateHandle_->SendMessage(deviceId, inMsg, false, SEND_TIME_OUT);
    if (errCode != E_OK) {
        LOGE("MultiVerDataSync::Send ERR! ERR = %d", errCode);
    }
    return errCode;
}

int MultiVerDataSync::SendRequestPacket(const MultiVerSyncTaskContext *context, MultiVerCommitNode &commit)
{
    MultiVerRequestPacket *packet = new (std::nothrow) MultiVerRequestPacket();
    if (packet == nullptr) {
        LOGE("MultiVerRequestPacket::SendRequestPacket : new packet error");
        return -E_OUT_OF_MEMORY;
    }
    packet->SetCommit(commit);
    Message *message = new (std::nothrow) Message(MULTI_VER_DATA_SYNC_MESSAGE);
    if (message == nullptr) {
        delete packet;
        packet = nullptr;
        LOGE("MultiVerDataSync::SendRequestPacket : new message error");
        return -E_OUT_OF_MEMORY;
    }
    message->SetMessageType(TYPE_REQUEST);
    message->SetTarget(context->GetDeviceId());
    int errCode = message->SetExternalObject(packet);
    if (errCode != E_OK) {
        delete packet;
        packet = nullptr;
        delete message;
        message = nullptr;
        LOGE("[MultiVerDataSync][SendRequestPacket] : SetExternalObject failed errCode:%d", errCode);
        return errCode;
    }
    message->SetSessionId(context->GetRequestSessionId());
    message->SetSequenceId(context->GetSequenceId());

    PerformanceAnalysis *performance = PerformanceAnalysis::GetInstance();
    if (performance != nullptr) {
        performance->StepTimeRecordStart(MV_TEST_RECORDS::RECORD_DATA_ENTRY_SEND_REQUEST_TO_ACK_RECV);
    }
    errCode = Send(message->GetTarget(), message);
    if (errCode != E_OK) {
        delete message;
        message = nullptr;
    }
    LOGD("MultiVerDataSync::SendRequestPacket end");
    return errCode;
}

int MultiVerDataSync::SendAckPacket(const MultiVerSyncTaskContext *context,
    const std::vector<MultiVerKvEntry *> &dataItems, int retCode, const Message *message)
{
    if (message == nullptr) {
        LOGE("MultiVerDataSync::SendAckPacket : message is nullptr");
        return -E_INVALID_ARGS;
    }

    MultiVerAckPacket *packet = new (std::nothrow) MultiVerAckPacket();
    if (packet == nullptr) {
        LOGE("MultiVerDataSync::SendAckPack et : packet is nullptr");
        return -E_OUT_OF_MEMORY;
    }
    Message *ackMessage = new (std::nothrow) Message(MULTI_VER_DATA_SYNC_MESSAGE);
    if (ackMessage == nullptr) {
        delete packet;
        packet = nullptr;
        LOGE("MultiVerDataSync::SendAckPacket : new message error");
        return -E_OUT_OF_MEMORY;
    }

    std::vector<std::vector<uint8_t>> entries;
    for (const auto &iter : dataItems) {
        std::vector<uint8_t> item;
        iter->GetSerialData(item);
        entries.push_back(item);
    }
    packet->SetData(entries);
    packet->SetErrorCode(static_cast<int32_t>(retCode));

    ackMessage->SetMessageType(TYPE_RESPONSE);
    ackMessage->SetTarget(context->GetDeviceId());
    int errCode = ackMessage->SetExternalObject(packet);
    if (errCode != E_OK) {
        delete packet;
        packet = nullptr;
        delete ackMessage;
        ackMessage = nullptr;
        LOGE("[MultiVerDataSync][SendAckPacket] : SetExternalObject failed errCode:%d", errCode);
        return errCode;
    }
    ackMessage->SetSequenceId(message->GetSequenceId());
    ackMessage->SetSessionId(message->GetSessionId());
    errCode = Send(ackMessage->GetTarget(), ackMessage);
    if (errCode != E_OK) {
        delete ackMessage;
        ackMessage = nullptr;
    }
    LOGD("MultiVerDataSync::SendAckPacket end, dst=%s{private}, errCode = %d", context->GetDeviceId().c_str(), errCode);
    return errCode;
}

int MultiVerDataSync::GetCommitData(const MultiVerCommitNode &commit, std::vector<MultiVerKvEntry *> &entries)
{
    return storagePtr_->GetCommitData(commit, entries);
}

MultiVerKvEntry *MultiVerDataSync::CreateKvEntry(const std::vector<uint8_t> &entry)
{
    return storagePtr_->CreateKvEntry(entry);
}
}
#endif