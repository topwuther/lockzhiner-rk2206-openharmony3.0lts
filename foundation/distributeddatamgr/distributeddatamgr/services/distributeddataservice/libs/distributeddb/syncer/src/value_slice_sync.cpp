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
#include "value_slice_sync.h"

#include "parcel.h"
#include "log_print.h"
#include "sync_types.h"
#include "message_transform.h"
#include "performance_analysis.h"
#include "db_constant.h"

namespace DistributedDB {
const int ValueSliceSync::MAX_VALUE_NODE_SIZE = 100000;

// Class ValueSliceHashPacket
uint32_t ValueSliceHashPacket::CalculateLen() const
{
    uint64_t len = Parcel::GetIntLen();
    len = Parcel::GetEightByteAlign(len);
    len += Parcel::GetVectorCharLen(valueSliceHash_);
    if (len > INT32_MAX) {
        return 0;
    }
    return len;
}

void ValueSliceHashPacket::SetValueSliceHash(ValueSliceHash &hash)
{
    valueSliceHash_ = std::move(hash);
}

void ValueSliceHashPacket::GetValueSliceHash(ValueSliceHash &hash) const
{
    hash = valueSliceHash_;
}

void ValueSliceHashPacket::SetErrCode(int32_t errCode)
{
    errCode_ = errCode;
}

int32_t ValueSliceHashPacket::GetErrCode() const
{
    return errCode_;
}

// Class ValueSlicePacket
uint32_t ValueSlicePacket::CalculateLen() const
{
    uint64_t len = Parcel::GetIntLen();
    len = Parcel::GetEightByteAlign(len);
    len += Parcel::GetVectorCharLen(valueSlice_);
    if (len > INT32_MAX) {
        return 0;
    }
    return len;
}

void ValueSlicePacket::SetData(const ValueSlice &data)
{
    valueSlice_ = std::move(data);
}

void ValueSlicePacket::GetData(ValueSlice &data) const
{
    data = valueSlice_;
}

void ValueSlicePacket::SetErrorCode(int32_t errCode)
{
    errorCode_ = errCode;
}

void ValueSlicePacket::GetErrorCode(int32_t &errCode) const
{
    errCode = errorCode_;
}

// Class ValueSliceSync
ValueSliceSync::~ValueSliceSync()
{
    storagePtr_ = nullptr;
    communicateHandle_ = nullptr;
}

int ValueSliceSync::Serialization(uint8_t *buffer, uint32_t length, const Message *inMsg)
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

int ValueSliceSync::DeSerialization(const uint8_t *buffer, uint32_t length, Message *inMsg)
{
    if ((buffer == nullptr) || !(IsPacketValid(inMsg, TYPE_RESPONSE) || IsPacketValid(inMsg, TYPE_REQUEST))) {
        return -E_INVALID_ARGS;
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

uint32_t ValueSliceSync::CalculateLen(const Message *inMsg)
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

int ValueSliceSync::RegisterTransformFunc()
{
    TransformFunc func;
    func.computeFunc = std::bind(&ValueSliceSync::CalculateLen, std::placeholders::_1);
    func.serializeFunc = std::bind(&ValueSliceSync::Serialization, std::placeholders::_1,
                                   std::placeholders::_2, std::placeholders::_3);
    func.deserializeFunc = std::bind(&ValueSliceSync::DeSerialization, std::placeholders::_1,
                                     std::placeholders::_2, std::placeholders::_3);
    return MessageTransform::RegTransformFunction(VALUE_SLICE_SYNC_MESSAGE, func);
}

int ValueSliceSync::Initialize(MultiVerKvDBSyncInterface *storagePtr, ICommunicator *communicateHandle)
{
    if ((storagePtr == nullptr) || (communicateHandle == nullptr)) {
        return -E_INVALID_ARGS;
    }
    storagePtr_ = storagePtr;
    communicateHandle_ = communicateHandle;
    return E_OK;
}

int ValueSliceSync::SyncStart(MultiVerSyncTaskContext *context)
{
    if (context == nullptr) {
        return -E_INVALID_ARGS;
    }
    int entriesIndex = context->GetEntriesIndex();
    int entriesSize = context->GetEntriesSize();
    if (entriesSize > DBConstant::MAX_ENTRIES_SIZE) {
        LOGE("ValueSliceSync::entriesSize too large %d", entriesSize);
        return -E_INVALID_ARGS;
    }
    while (entriesIndex < entriesSize) {
        PerformanceAnalysis *performance = PerformanceAnalysis::GetInstance();
        if (performance != nullptr) {
            performance->StepTimeRecordStart(MV_TEST_RECORDS::RECORD_GET_VALUE_SLICE_NODE);
        }
        ValueSliceHash valueSliceHashNode;
        int errCode = GetValidValueSliceHashNode(context, valueSliceHashNode);
        if (performance != nullptr) {
            performance->StepTimeRecordEnd(MV_TEST_RECORDS::RECORD_GET_VALUE_SLICE_NODE);
        }
        LOGD("ValueSliceSync::SyncStart begin errCode = %d", errCode);
        if (errCode == E_OK) {
            errCode = SendRequestPacket(context, valueSliceHashNode);
            LOGD("ValueSliceSync::SyncStart send request packet dst=%s{private}, errCode = %d",
                context->GetDeviceId().c_str(), errCode);
            return errCode;
        }
        // move to next entry
        MultiVerKvEntry *entry = nullptr;
        std::vector<ValueSliceHash> valueHashes;
        entriesIndex++;
        if (entriesIndex < entriesSize) {
            LOGD("ValueSliceSync::SyncStart begin entriesIndex = %d, entriesSize = %d", entriesIndex, entriesSize);
            context->SetEntriesIndex(entriesIndex);
            context->GetEntry(entriesIndex, entry);
            errCode = entry->GetValueHash(valueHashes);
            if (errCode != E_OK) {
                LOGE("ValueSliceSync::entry->GetValueHash %d", errCode);
                return errCode;
            }
            context->SetValueSliceHashNodes(valueHashes);
            context->SetValueSlicesIndex(0);
            context->SetValueSlicesSize(valueHashes.size());
        } else {
            // all entries are received, move to next commit
            return -E_NOT_FOUND;
        }
    }
    return -E_NOT_FOUND;
}

int ValueSliceSync::RequestRecvCallback(const MultiVerSyncTaskContext *context, const Message *message)
{
    if (!IsPacketValid(message, TYPE_REQUEST) || context == nullptr) {
        return -E_INVALID_ARGS;
    }

    const ValueSliceHashPacket *packet = message->GetObject<ValueSliceHashPacket>();
    if (packet == nullptr) {
        return -E_INVALID_ARGS;
    }
    ValueSliceHash valueSliceHashNode;
    packet->GetValueSliceHash(valueSliceHashNode);
    if ((packet->GetErrCode() == -E_LAST_SYNC_FRAME) && valueSliceHashNode.empty()) {
        return -E_LAST_SYNC_FRAME;
    }
    ValueSlice valueSlice;
    PerformanceAnalysis *performance = PerformanceAnalysis::GetInstance();
    if (performance != nullptr) {
        performance->StepTimeRecordStart(MV_TEST_RECORDS::RECORD_READ_VALUE_SLICE);
    }
    int errCode = GetValueSlice(valueSliceHashNode, valueSlice);
    if (performance != nullptr) {
        performance->StepTimeRecordEnd(MV_TEST_RECORDS::RECORD_READ_VALUE_SLICE);
    }
    if (errCode != E_OK) {
        LOGE("ValueSliceSync::RequestRecvCallback : GetValueSlice ERR, errno = %d", errCode);
    }
    errCode = SendAckPacket(context, valueSlice, errCode, message);
    LOGD("ValueSliceSync::RequestRecvCallback : SendAckPacket, errno = %d, dst = %s{private}", errCode,
         context->GetDeviceId().c_str());
    if (packet->GetErrCode() == -E_LAST_SYNC_FRAME) {
        return -E_LAST_SYNC_FRAME;
    }
    return errCode;
}

int ValueSliceSync::AckRecvCallback(const MultiVerSyncTaskContext *context, const Message *message)
{
    if (!IsPacketValid(message, TYPE_RESPONSE) || (context == nullptr)) {
        return -E_INVALID_ARGS;
    }

    const ValueSlicePacket *packet = message->GetObject<ValueSlicePacket>();
    if (packet == nullptr) {
        return -E_INVALID_ARGS;
    }
    int errCode = E_OK;
    packet->GetErrorCode(errCode);
    ValueSlice valueSlice;
    packet->GetData(valueSlice);
    if (errCode != E_OK) {
        return errCode;
    }
    int index = context->GetValueSlicesIndex();
    ValueSliceHash hashValue;
    context->GetValueSliceHashNode(index, hashValue);
    PerformanceAnalysis *performance = PerformanceAnalysis::GetInstance();
    if (performance != nullptr) {
        performance->StepTimeRecordStart(MV_TEST_RECORDS::RECORD_SAVE_VALUE_SLICE);
    }
    errCode = PutValueSlice(hashValue, valueSlice);
    if (performance != nullptr) {
        performance->StepTimeRecordEnd(MV_TEST_RECORDS::RECORD_SAVE_VALUE_SLICE);
    }
    LOGD("ValueSliceSync::AckRecvCallback PutValueSlice finished, src=%s{private}, errCode = %d",
        context->GetDeviceId().c_str(), errCode);
    if (errCode != E_OK) {
        return errCode;
    }
    return errCode;
}

void ValueSliceSync::SendFinishedRequest(const MultiVerSyncTaskContext *context)
{
    if (context == nullptr) {
        return;
    }

    ValueSliceHashPacket *packet = new (std::nothrow) ValueSliceHashPacket();
    if (packet == nullptr) {
        return;
    }

    packet->SetErrCode(-E_LAST_SYNC_FRAME);
    Message *message = new (std::nothrow) Message(VALUE_SLICE_SYNC_MESSAGE);
    if (message == nullptr) {
        delete packet;
        packet = nullptr;
        return;
    }

    int errCode = message->SetExternalObject(packet);
    if (errCode != E_OK) {
        delete packet;
        packet = nullptr;
        delete message;
        message = nullptr;
        return;
    }

    message->SetMessageType(TYPE_REQUEST);
    message->SetTarget(context->GetDeviceId());
    message->SetSessionId(context->GetRequestSessionId());
    message->SetSequenceId(context->GetSequenceId());
    errCode = Send(message->GetTarget(), message);
    if (errCode != E_OK) {
        delete message;
        message = nullptr;
        LOGE("[ValueSliceSync][SendRequestPacket] SendRequestPacket failed, err %d", errCode);
    }
    LOGI("[ValueSliceSync][SendRequestPacket] SendRequestPacket dst=%s{private}", context->GetDeviceId().c_str());
}

int ValueSliceSync::RequestPacketCalculateLen(const Message *inMsg, uint32_t &len)
{
    const ValueSliceHashPacket *packet = inMsg->GetObject<ValueSliceHashPacket>();
    if (packet == nullptr) {
        return -E_INVALID_ARGS;
    }

    len = packet->CalculateLen();
    return E_OK;
}

int ValueSliceSync::RequestPacketSerialization(uint8_t *buffer, uint32_t length, const Message *inMsg)
{
    const ValueSliceHashPacket *packet = inMsg->GetObject<ValueSliceHashPacket>();
    if ((packet == nullptr) || (length != packet->CalculateLen())) {
        return -E_INVALID_ARGS;
    }

    Parcel parcel(buffer, length);
    ValueSliceHash valueSliceHash;
    packet->GetValueSliceHash(valueSliceHash);
    int32_t ackCode = packet->GetErrCode();
    // errCode Serialization
    int32_t errCode = parcel.WriteInt(ackCode);
    if (errCode != E_OK) {
        return -E_SECUREC_ERROR;
    }
    parcel.EightByteAlign();
    // commitMap Serialization
    errCode = parcel.WriteVectorChar(valueSliceHash);
    if (errCode != E_OK) {
        return -E_SECUREC_ERROR;
    }

    return errCode;
}

int ValueSliceSync::RequestPacketDeSerialization(const uint8_t *buffer, uint32_t length, Message *inMsg)
{
    Parcel parcel(const_cast<uint8_t *>(buffer), length);

    int ackCode = 0;
    // errCode DeSerialization
    uint32_t packLen = parcel.ReadInt(ackCode);
    parcel.EightByteAlign();
    packLen = Parcel::GetEightByteAlign(packLen);

    ValueSliceHash valueSliceHash;
    // commit DeSerialization
    packLen += parcel.ReadVectorChar(valueSliceHash);
    if (packLen != length || parcel.IsError()) {
        return -E_INVALID_ARGS;
    }
    ValueSliceHashPacket *packet = new (std::nothrow) ValueSliceHashPacket();
    if (packet == nullptr) {
        LOGE("ValueSliceSync::AckPacketDeSerialization : new packet error");
        return -E_OUT_OF_MEMORY;
    }

    packet->SetValueSliceHash(valueSliceHash);
    int errCode = inMsg->SetExternalObject<>(packet);
    if (errCode != E_OK) {
        delete packet;
        packet = nullptr;
    }
    return errCode;
}

int ValueSliceSync::AckPacketCalculateLen(const Message *inMsg, uint32_t &len)
{
    const ValueSlicePacket *packet = inMsg->GetObject<ValueSlicePacket>();
    if (packet == nullptr) {
        return -E_INVALID_ARGS;
    }
    len = packet->CalculateLen();
    return E_OK;
}

int ValueSliceSync::AckPacketSerialization(uint8_t *buffer, uint32_t length, const Message *inMsg)
{
    if ((buffer == nullptr) || !IsPacketValid(inMsg, TYPE_RESPONSE)) {
        return -E_INVALID_ARGS;
    }
    const ValueSlicePacket *packet = inMsg->GetObject<ValueSlicePacket>();
    if ((packet == nullptr) || (length != packet->CalculateLen())) {
        return -E_INVALID_ARGS;
    }

    Parcel parcel(buffer, length);
    ValueSlice valueSlice;
    packet->GetData(valueSlice);
    int32_t ackCode = 0;
    packet->GetErrorCode(ackCode);
    // errCode Serialization
    int32_t errCode = parcel.WriteInt(ackCode);
    if (errCode != E_OK) {
        return -E_SECUREC_ERROR;
    }
    parcel.EightByteAlign();

    // commits vector Serialization
    errCode = parcel.WriteVectorChar(valueSlice);
    if (errCode != E_OK) {
        return -E_SECUREC_ERROR;
    }

    return errCode;
}

int ValueSliceSync::AckPacketDeSerialization(const uint8_t *buffer, uint32_t length, Message *inMsg)
{
    Parcel parcel(const_cast<uint8_t *>(buffer), length);
    int32_t ackCode = 0;
    uint32_t packLen = 0;
    ValueSlice valueSlice;

    // errCode DeSerialization
    packLen += parcel.ReadInt(ackCode);
    parcel.EightByteAlign();
    packLen = Parcel::GetEightByteAlign(packLen);
    // valueSlice DeSerialization
    packLen += parcel.ReadVectorChar(valueSlice);
    if (packLen != length || parcel.IsError()) {
        LOGE("ValueSliceSync::AckPacketSerialization data error, packLen = %lu, length = %lu", packLen, length);
        return -E_INVALID_ARGS;
    }
    ValueSlicePacket *packet = new (std::nothrow) ValueSlicePacket();
    if (packet == nullptr) {
        LOGE("ValueSliceSync::AckPacketDeSerialization : new packet error");
        return -E_OUT_OF_MEMORY;
    }
    packet->SetData(valueSlice);
    packet->SetErrorCode(ackCode);
    int errCode = inMsg->SetExternalObject<>(packet);
    if (errCode != E_OK) {
        delete packet;
        packet = nullptr;
    }
    return errCode;
}

bool ValueSliceSync::IsPacketValid(const Message *inMsg, uint16_t messageType)
{
    if ((inMsg == nullptr) || (inMsg->GetMessageId() != VALUE_SLICE_SYNC_MESSAGE)) {
        return false;
    }
    if (messageType != inMsg->GetMessageType()) {
        return false;
    }
    return true;
}

int ValueSliceSync::GetValidValueSliceHashNode(MultiVerSyncTaskContext *context, ValueSliceHash &valueHashNode)
{
    int index = context->GetValueSlicesIndex();
    int valueNodesSize = context->GetValueSlicesSize();
    if (valueNodesSize > MAX_VALUE_NODE_SIZE) {
        LOGD("ValueSliceSync::GetValidValueSliceHashNode failed, too large!");
        return -E_LENGTH_ERROR;
    }
    LOGD("ValueSliceSync::GetValidValueSliceHashNode ValueSlicesSize = %d", valueNodesSize);
    if (context->GetRetryStatus() == SyncTaskContext::NEED_RETRY) {
        context->SetRetryStatus(SyncTaskContext::NO_NEED_RETRY);
        index--;
    }
    std::vector<ValueSliceHash> valueSliceHashNodes;
    context->GetValueSliceHashNodes(valueSliceHashNodes);
    index = (index < 0) ? 0 : index;
    while (index < valueNodesSize) {
        if (IsValueSliceExisted(valueSliceHashNodes[index])) {
            index++;
            context->SetValueSlicesIndex(index);
            continue;
        }
        valueHashNode = valueSliceHashNodes[index];
        return E_OK;
    }
    return -E_NOT_FOUND;
}

int ValueSliceSync::Send(const DeviceID &deviceId, const Message *inMsg)
{
    int errCode = communicateHandle_->SendMessage(deviceId, inMsg, false, SEND_TIME_OUT);
    if (errCode != E_OK) {
        LOGE("ValueSliceSync::Send ERR! err = %d", errCode);
    }
    return errCode;
}

int ValueSliceSync::SendRequestPacket(const MultiVerSyncTaskContext *context, ValueSliceHash &valueSliceHash)
{
    ValueSliceHashPacket *packet = new (std::nothrow) ValueSliceHashPacket();
    if (packet == nullptr) {
        LOGE("ValueSliceSync::SendRequestPacket : new packet error");
        return -E_OUT_OF_MEMORY;
    }

    packet->SetValueSliceHash(valueSliceHash);
    Message *message = new (std::nothrow) Message(VALUE_SLICE_SYNC_MESSAGE);
    if (message == nullptr) {
        delete packet;
        packet = nullptr;
        LOGE("ValueSliceSync::SendRequestPacket : new message error");
        return -E_OUT_OF_MEMORY;
    }

    int errCode = message->SetExternalObject<>(packet);
    if (errCode != E_OK) {
        delete packet;
        packet = nullptr;
        delete message;
        message = nullptr;
        return errCode;
    }

    message->SetMessageType(TYPE_REQUEST);
    message->SetTarget(context->GetDeviceId());
    message->SetSessionId(context->GetRequestSessionId());
    message->SetSequenceId(context->GetSequenceId());
    PerformanceAnalysis *performance = PerformanceAnalysis::GetInstance();
    if (performance != nullptr) {
        performance->StepTimeRecordStart(MV_TEST_RECORDS::RECORD_VALUE_SLICE_SEND_REQUEST_TO_ACK_RECV);
    }
    errCode = Send(message->GetTarget(), message);
    if (errCode != E_OK) {
        delete message;
        message = nullptr;
    }
    return errCode;
}

int ValueSliceSync::SendAckPacket(const MultiVerSyncTaskContext *context, const ValueSlice &value,
    int ackCode, const Message *message)
{
    ValueSlicePacket *packet = new (std::nothrow) ValueSlicePacket();
    if (packet == nullptr) {
        LOGE("ValueSliceSync::SendAckPacket : packet is nullptr");
        return -E_OUT_OF_MEMORY;
    }

    Message *ackMessage = new (std::nothrow) Message(VALUE_SLICE_SYNC_MESSAGE);
    if (ackMessage == nullptr) {
        delete packet;
        packet = nullptr;
        LOGE("ValueSliceSync::SendAckPacket : new message error");
        return -E_OUT_OF_MEMORY;
    }

    packet->SetData(value);
    packet->SetErrorCode(static_cast<int32_t>(ackCode));
    int errCode = ackMessage->SetExternalObject<>(packet);
    if (errCode != E_OK) {
        delete packet;
        packet = nullptr;
        delete ackMessage;
        ackMessage = nullptr;
        return errCode;
    }

    ackMessage->SetMessageType(TYPE_RESPONSE);
    ackMessage->SetTarget(context->GetDeviceId());
    ackMessage->SetSequenceId(message->GetSequenceId());
    ackMessage->SetSessionId(message->GetSessionId());
    errCode = Send(ackMessage->GetTarget(), ackMessage);
    if (errCode != E_OK) {
        delete ackMessage;
        ackMessage = nullptr;
    }

    return errCode;
}

bool ValueSliceSync::IsValueSliceExisted(const ValueSliceHash &value)
{
    return storagePtr_->IsValueSliceExisted(value);
}

int ValueSliceSync::GetValueSlice(const ValueSliceHash &hashValue, ValueSlice &sliceValue)
{
    return storagePtr_->GetValueSlice(hashValue, sliceValue);
}

int ValueSliceSync::PutValueSlice(const ValueSliceHash &hashValue, const ValueSlice &sliceValue)
{
    return storagePtr_->PutValueSlice(hashValue, sliceValue);
}
} // namespace DistributedDB
#endif
