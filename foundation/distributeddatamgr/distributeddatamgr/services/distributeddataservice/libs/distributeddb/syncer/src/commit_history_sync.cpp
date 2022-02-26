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
#include "commit_history_sync.h"

#include "sync_engine.h"
#include "parcel.h"
#include "log_print.h"
#include "message_transform.h"
#include "performance_analysis.h"
#include "db_constant.h"

namespace DistributedDB {
// Class CommitHistorySyncRequestPacket
uint32_t CommitHistorySyncRequestPacket::CalculateLen() const
{
    uint64_t len = Parcel::GetUInt64Len();
    // commitMap len
    for (const auto &iter : commitMap_) {
        len += Parcel::GetStringLen(iter.first);
        len += Parcel::GetMultiVerCommitLen(iter.second);
        if (len > INT32_MAX) {
            return 0;
        }
    }
    len += Parcel::GetUInt32Len(); // version
    len += Parcel::GetVectorLen<uint64_t>(reserved_); // reserved
    len = Parcel::GetEightByteAlign(len);
    if (len > INT32_MAX) {
        return 0;
    }
    return len;
}

void CommitHistorySyncRequestPacket::SetCommitMap(std::map<std::string, MultiVerCommitNode> &inMap)
{
    commitMap_ = std::move(inMap);
}

void CommitHistorySyncRequestPacket::GetCommitMap(std::map<std::string, MultiVerCommitNode> &outMap) const
{
    outMap = commitMap_;
}

void CommitHistorySyncRequestPacket::SetVersion(uint32_t version)
{
    version_ = version;
}

uint32_t CommitHistorySyncRequestPacket::GetVersion() const
{
    return version_;
}

void CommitHistorySyncRequestPacket::SetReserved(std::vector<uint64_t> &reserved)
{
    reserved_ = std::move(reserved);
}

std::vector<uint64_t> CommitHistorySyncRequestPacket::GetReserved() const
{
    return reserved_;
}

uint32_t CommitHistorySyncAckPacket::CalculateLen() const
{
    uint64_t len = Parcel::GetIntLen(); // errCode
    len += Parcel::GetUInt32Len(); // version
    len = Parcel::GetEightByteAlign(len);

    // commits vector len
    len += Parcel::GetMultiVerCommitsLen(commits_);
    len += Parcel::GetVectorLen<uint64_t>(reserved_); // reserved
    len = Parcel::GetEightByteAlign(len);
    if (len > INT32_MAX) {
        return 0;
    }
    return len;
}

void CommitHistorySyncAckPacket::SetData(std::vector<MultiVerCommitNode> &inData)
{
    commits_ = std::move(inData);
}

void CommitHistorySyncAckPacket::GetData(std::vector<MultiVerCommitNode> &outData) const
{
    outData = commits_;
}

void CommitHistorySyncAckPacket::SetErrorCode(int32_t errCode)
{
    errorCode_ = errCode;
}

void CommitHistorySyncAckPacket::GetErrorCode(int32_t &errCode) const
{
    errCode = errorCode_;
}

void CommitHistorySyncAckPacket::SetVersion(uint32_t version)
{
    version_ = version;
}

uint32_t CommitHistorySyncAckPacket::GetVersion() const
{
    return version_;
}

void CommitHistorySyncAckPacket::SetReserved(std::vector<uint64_t> &reserved)
{
    reserved_ = std::move(reserved);
}

std::vector<uint64_t> CommitHistorySyncAckPacket::GetReserved() const
{
    return reserved_;
}

// Class CommitHistorySync
CommitHistorySync::~CommitHistorySync()
{
    storagePtr_ = nullptr;
    communicateHandle_ = nullptr;
}

int CommitHistorySync::Serialization(uint8_t *buffer, uint32_t length, const Message *inMsg)
{
    if ((buffer == nullptr) || !(IsPacketValid(inMsg, TYPE_RESPONSE) || IsPacketValid(inMsg, TYPE_REQUEST))) {
        return -E_MESSAGE_ID_ERROR;
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

int CommitHistorySync::DeSerialization(const uint8_t *buffer, uint32_t length, Message *inMsg)
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

uint32_t CommitHistorySync::CalculateLen(const Message *inMsg)
{
    if (!(IsPacketValid(inMsg, TYPE_RESPONSE) || IsPacketValid(inMsg, TYPE_REQUEST))) {
        return 0;
    }

    uint32_t len = 0;
    int errCode = E_OK;
    switch (inMsg->GetMessageType()) {
        case TYPE_REQUEST:
            errCode = RequestPacketCalculateLen(inMsg, len);
            if (errCode != E_OK) {
                return 0;
            }
            return len;
        case TYPE_RESPONSE:
            errCode = AckPacketCalculateLen(inMsg, len);
            if (errCode != E_OK) {
                return 0;
            }
            return len;
        default:
            return 0;
    }
}

int CommitHistorySync::RegisterTransformFunc()
{
    TransformFunc func;
    func.computeFunc = std::bind(&CommitHistorySync::CalculateLen, std::placeholders::_1);
    func.serializeFunc = std::bind(&CommitHistorySync::Serialization, std::placeholders::_1,
                                   std::placeholders::_2, std::placeholders::_3);
    func.deserializeFunc = std::bind(&CommitHistorySync::DeSerialization, std::placeholders::_1,
                                     std::placeholders::_2, std::placeholders::_3);
    return MessageTransform::RegTransformFunction(COMMIT_HISTORY_SYNC_MESSAGE, func);
}

int CommitHistorySync::Initialize(MultiVerKvDBSyncInterface *storagePtr, ICommunicator *communicateHandle)
{
    if ((storagePtr == nullptr) || (communicateHandle == nullptr)) {
        return -E_INVALID_ARGS;
    }
    storagePtr_ = storagePtr;
    communicateHandle_ = communicateHandle;
    return E_OK;
}

void CommitHistorySync::TimeOutCallback(MultiVerSyncTaskContext *context, const Message *message) const
{
    return;
}

int CommitHistorySync::SyncStart(MultiVerSyncTaskContext *context)
{
    if (context == nullptr) {
        return -E_INVALID_ARGS;
    }
    PerformanceAnalysis *performance = PerformanceAnalysis::GetInstance();
    if (performance != nullptr) {
        performance->StepTimeRecordStart(MV_TEST_RECORDS::RECORD_GET_DEVICE_LATEST_COMMIT);
    }
    std::map<std::string, MultiVerCommitNode> commitMap;
    int errCode = GetDeviceLatestCommit(commitMap);
    if (performance != nullptr) {
        performance->StepTimeRecordEnd(MV_TEST_RECORDS::RECORD_GET_DEVICE_LATEST_COMMIT);
    }
    if ((errCode != E_OK) && (errCode != -E_NOT_FOUND)) {
        return errCode;
    }

    LOGD("CommitHistorySync::commitMap size = %zu, dst=%s{private}", commitMap.size(), context->GetDeviceId().c_str());
    return SendRequestPacket(context, commitMap);
}

int CommitHistorySync::RequestRecvCallback(const MultiVerSyncTaskContext *context, const Message *message)
{
    if (!IsPacketValid(message, TYPE_REQUEST) || context == nullptr) {
        return -E_INVALID_ARGS;
    }
    const CommitHistorySyncRequestPacket *packet = message->GetObject<CommitHistorySyncRequestPacket>();
    if (packet == nullptr) {
        return -E_INVALID_ARGS;
    }
    std::vector<MultiVerCommitNode> commits;
    int errCode = RunPermissionCheck(context->GetDeviceId());
    if (errCode == -E_NOT_PERMIT) {
        LOGE("CommitHistorySync::RequestRecvCallback RunPermissionCheck not pass");
        SendAckPacket(context, commits, errCode, message);
        return errCode;
    }
    std::map<std::string, MultiVerCommitNode> commitMap;
    packet->GetCommitMap(commitMap);
    uint32_t ver = packet->GetVersion();
    PerformanceAnalysis *performance = PerformanceAnalysis::GetInstance();
    if (performance != nullptr) {
        performance->StepTimeRecordStart(MV_TEST_RECORDS::RECORD_GET_COMMIT_TREE);
    }
    errCode = GetCommitTree(commitMap, commits);
    if (performance != nullptr) {
        performance->StepTimeRecordEnd(MV_TEST_RECORDS::RECORD_GET_COMMIT_TREE);
    }
    if (errCode != E_OK) {
        LOGE("CommitHistorySync::RequestRecvCallback : GetCommitTree ERR, errno = %d", errCode);
    }

    errCode = SendAckPacket(context, commits, errCode, message);
    LOGD("CommitHistorySync::RequestRecvCallback:SendAckPacket, errno = %d, dst=%s{private}, ver = %d, myversion = %u",
         errCode, context->GetDeviceId().c_str(), ver, SOFTWARE_VERSION_CURRENT);
    if (errCode == E_OK) {
        if (commitMap.empty()) {
            LOGD("[CommitHistorySync][RequestRecvCallback] no need to start SyncResponse");
            return -E_NOT_FOUND;
        }
    }
    return errCode;
}

int CommitHistorySync::AckRecvCallback(MultiVerSyncTaskContext *context, const Message *message)
{
    if (!IsPacketValid(message, TYPE_RESPONSE) || (context == nullptr)) {
        return -E_INVALID_ARGS;
    }

    std::vector<MultiVerCommitNode> commits;
    int32_t errCode;

    const CommitHistorySyncAckPacket *packet = message->GetObject<CommitHistorySyncAckPacket>();
    if (packet == nullptr) {
        return -E_INVALID_ARGS;
    }
    packet->GetErrorCode(errCode);
    if (errCode == -E_NOT_PERMIT) {
        LOGE("CommitHistorySync::AckRecvCallback RunPermissionCheck not pass");
        return errCode;
    }
    packet->GetData(commits);
    uint32_t ver = packet->GetVersion();
    context->SetCommits(commits);
    context->SetCommitIndex(0);
    context->SetCommitsSize(commits.size());
    LOGD("CommitHistorySync::AckRecvCallback end, CommitsSize = %llu, dst = %s{private}, ver = %d, myversion = %u",
        commits.size(), context->GetDeviceId().c_str(), ver, SOFTWARE_VERSION_CURRENT);
    return E_OK;
}

int CommitHistorySync::RequestPacketCalculateLen(const Message *inMsg, uint32_t &len)
{
    if (inMsg == nullptr) {
        return -E_INVALID_ARGS;
    }
    const CommitHistorySyncRequestPacket *packet = inMsg->GetObject<CommitHistorySyncRequestPacket>();
    if (packet == nullptr) {
        return -E_INVALID_ARGS;
    }

    if ((inMsg->GetMessageId() != COMMIT_HISTORY_SYNC_MESSAGE) || (inMsg->GetMessageType() != TYPE_REQUEST)) {
        return -E_INVALID_ARGS;
    }
    len = packet->CalculateLen();
    return E_OK;
}

int CommitHistorySync::RequestPacketSerialization(uint8_t *buffer, uint32_t length, const Message *inMsg)
{
    if ((buffer == nullptr) || (inMsg == nullptr)) {
        return -E_INVALID_ARGS;
    }
    const CommitHistorySyncRequestPacket *packet = inMsg->GetObject<CommitHistorySyncRequestPacket>();
    if ((packet == nullptr) || (length != packet->CalculateLen())) {
        return -E_INVALID_ARGS;
    }

    Parcel parcel(buffer, length);
    std::map<std::string, MultiVerCommitNode> commitMap;
    packet->GetCommitMap(commitMap);

    int errCode = parcel.WriteUInt64(commitMap.size());
    if (errCode != E_OK) {
        return -E_SECUREC_ERROR;
    }
    // commitMap Serialization
    for (auto &iter : commitMap) {
        errCode = parcel.WriteString(iter.first);
        if (errCode != E_OK) {
            return -E_SECUREC_ERROR;
        }
        errCode = parcel.WriteMultiVerCommit(iter.second);
        if (errCode != E_OK) {
            return -E_SECUREC_ERROR;
        }
    }
    errCode = parcel.WriteUInt32(packet->GetVersion());
    if (errCode != E_OK) {
        return -E_SECUREC_ERROR;
    }
    errCode = parcel.WriteVector<uint64_t>(packet->GetReserved());
    if (errCode != E_OK) {
        return -E_SECUREC_ERROR;
    }
    parcel.EightByteAlign();
    return errCode;
}

int CommitHistorySync::RequestPacketDeSerialization(const uint8_t *buffer, uint32_t length, Message *inMsg)
{
    if ((buffer == nullptr) || (inMsg == nullptr)) {
        return -E_INVALID_ARGS;
    }

    uint64_t packLen = 0;
    uint64_t len = 0;
    Parcel parcel(const_cast<uint8_t *>(buffer), length);
    packLen += parcel.ReadUInt64(len);
    if (len > DBConstant::MAX_DEVICES_SIZE) {
        LOGE("CommitHistorySync::RequestPacketDeSerialization : commitMap size too large = %llu", len);
        return -E_INVALID_ARGS;
    }
    // commitMap DeSerialization
    std::map<std::string, MultiVerCommitNode> commitMap;
    while (len > 0) {
        std::string key;
        MultiVerCommitNode val;
        packLen += parcel.ReadString(key);
        packLen += parcel.ReadMultiVerCommit(val);
        commitMap[key] = val;
        len--;
        if (parcel.IsError()) {
            return -E_INVALID_ARGS;
        }
    }
    uint32_t version;
    std::vector<uint64_t> reserved;
    packLen += parcel.ReadUInt32(version);
    packLen += parcel.ReadVector<uint64_t>(reserved);
    packLen = Parcel::GetEightByteAlign(packLen);
    if (packLen != length || parcel.IsError()) {
        LOGE("CommitHistorySync::RequestPacketDeSerialization : length error, input len = %lu, cac len = %llu",
            length, packLen);
        return -E_INVALID_ARGS;
    }
    CommitHistorySyncRequestPacket *packet = new (std::nothrow) CommitHistorySyncRequestPacket();
    if (packet == nullptr) {
        LOGE("CommitHistorySync::RequestPacketDeSerialization : new packet error");
        return -E_OUT_OF_MEMORY;
    }
    packet->SetCommitMap(commitMap);
    packet->SetVersion(version);
    packet->SetReserved(reserved);
    int errCode = inMsg->SetExternalObject<>(packet);
    if (errCode != E_OK) {
        delete packet;
        packet = nullptr;
    }
    return errCode;
}

int CommitHistorySync::AckPacketCalculateLen(const Message *inMsg, uint32_t &len)
{
    if (inMsg == nullptr) {
        return -E_INVALID_ARGS;
    }
    const CommitHistorySyncAckPacket *packet = inMsg->GetObject<CommitHistorySyncAckPacket>();
    if (packet == nullptr) {
        return -E_INVALID_ARGS;
    }

    if ((inMsg->GetMessageId() != COMMIT_HISTORY_SYNC_MESSAGE) || (inMsg->GetMessageType() != TYPE_RESPONSE)) {
        return -E_INVALID_ARGS;
    }
    len = packet->CalculateLen();
    return E_OK;
}

int CommitHistorySync::AckPacketSerialization(uint8_t *buffer, uint32_t length, const Message *inMsg)
{
    if ((buffer == nullptr) || (inMsg == nullptr)) {
        return -E_INVALID_ARGS;
    }
    const CommitHistorySyncAckPacket *packet = inMsg->GetObject<CommitHistorySyncAckPacket>();
    if ((packet == nullptr) || (length != packet->CalculateLen())) {
        return -E_INVALID_ARGS;
    }

    Parcel parcel(buffer, length);
    int32_t ackErrCode;
    std::vector<MultiVerCommitNode> commits;

    packet->GetData(commits);
    packet->GetErrorCode(ackErrCode);
    // errCode Serialization
    int errCode = parcel.WriteInt(ackErrCode);
    if (errCode != E_OK) {
        return -E_SECUREC_ERROR;
    }
    errCode = parcel.WriteUInt32(packet->GetVersion());
    if (errCode != E_OK) {
        return -E_SECUREC_ERROR;
    }
    parcel.EightByteAlign();

    // commits vector Serialization
    errCode = parcel.WriteMultiVerCommits(commits);
    if (errCode != E_OK) {
        return -E_SECUREC_ERROR;
    }
    errCode = parcel.WriteVector<uint64_t>(packet->GetReserved());
    if (errCode != E_OK) {
        return -E_SECUREC_ERROR;
    }
    parcel.EightByteAlign();
    return errCode;
}

int CommitHistorySync::AckPacketDeSerialization(const uint8_t *buffer, uint32_t length, Message *inMsg)
{
    std::vector<MultiVerCommitNode> commits;
    uint32_t packLen = 0;
    Parcel parcel(const_cast<uint8_t *>(buffer), length);
    int32_t pktErrCode;
    uint32_t version;
    std::vector<uint64_t> reserved;

    // errCode DeSerialization
    packLen += parcel.ReadInt(pktErrCode);
    packLen += parcel.ReadUInt32(version);
    parcel.EightByteAlign();
    packLen = Parcel::GetEightByteAlign(packLen);
    // commits vector DeSerialization
    packLen += parcel.ReadMultiVerCommits(commits);
    packLen += parcel.ReadVector<uint64_t>(reserved);
    packLen = Parcel::GetEightByteAlign(packLen);
    if (packLen != length || parcel.IsError()) {
        LOGE("CommitHistorySync::AckPacketDeSerialization : packet len error, input len = %u, cal len = %u",
            length, packLen);
        return -E_INVALID_ARGS;
    }
    CommitHistorySyncAckPacket *packet = new (std::nothrow) CommitHistorySyncAckPacket();
    if (packet == nullptr) {
        LOGE("CommitHistorySync::AckPacketDeSerialization : new packet error");
        return -E_OUT_OF_MEMORY;
    }
    packet->SetData(commits);
    packet->SetErrorCode(pktErrCode);
    packet->SetVersion(version);
    packet->SetReserved(reserved);
    int errCode = inMsg->SetExternalObject<>(packet);
    if (errCode != E_OK) {
        delete packet;
        packet = nullptr;
    }
    return errCode;
}

bool CommitHistorySync::IsPacketValid(const Message *inMsg, uint16_t messageType)
{
    if ((inMsg == nullptr) || (inMsg->GetMessageId() != COMMIT_HISTORY_SYNC_MESSAGE)) {
        return false;
    }
    if (messageType != inMsg->GetMessageType()) {
        return false;
    }
    return true;
}

int CommitHistorySync::Send(const DeviceID &deviceId, const Message *inMsg)
{
    int errCode = communicateHandle_->SendMessage(deviceId, inMsg, false, SEND_TIME_OUT);
    if (errCode != E_OK) {
        LOGE("CommitHistorySync::Send ERR! err = %d", errCode);
    }
    return errCode;
}

int CommitHistorySync::GetDeviceLatestCommit(std::map<std::string, MultiVerCommitNode> &commitMap)
{
    std::map<std::string, MultiVerCommitNode> readCommitMap;
    int errCode = storagePtr_->GetDeviceLatestCommit(readCommitMap);
    if (errCode != E_OK) {
        return errCode;
    }

    std::string localDevice;
    errCode = GetLocalDeviceInfo(localDevice);
    LOGD("GetLocalDeviceInfo : %s{private}, errCode = %d", localDevice.c_str(), errCode);
    if (errCode != E_OK) {
        return errCode;
    }

    for (auto &item : readCommitMap) {
        errCode = storagePtr_->TransferSyncCommitDevInfo(item.second, localDevice, false);
        if (errCode != E_OK) {
            break;
        }
        commitMap.insert(std::make_pair(item.second.deviceInfo, item.second));
    }

    return errCode;
}

int CommitHistorySync::GetCommitTree(const std::map<std::string, MultiVerCommitNode> &commitMap,
    std::vector<MultiVerCommitNode> &commits)
{
    std::map<std::string, MultiVerCommitNode> newCommitMap;

    std::string localDevice;
    int errCode = GetLocalDeviceInfo(localDevice);
    LOGD("GetLocalDeviceInfo : %s{private}, errCode = %d", localDevice.c_str(), errCode);
    if (errCode != E_OK) {
        return errCode;
    }

    for (const auto &item : commitMap) {
        MultiVerCommitNode commitNode = item.second;
        errCode = storagePtr_->TransferSyncCommitDevInfo(commitNode, localDevice, true);
        if (errCode != E_OK) {
            return errCode;
        }
        newCommitMap.insert(std::make_pair(commitNode.deviceInfo, commitNode));
    }

    errCode = storagePtr_->GetCommitTree(newCommitMap, commits);
    if (errCode != E_OK) {
        return errCode;
    }
    for (auto &commit : commits) {
        errCode = storagePtr_->TransferSyncCommitDevInfo(commit, localDevice, false);
        if (errCode != E_OK) {
            break;
        }
    }
    return errCode;
}

int CommitHistorySync::SendRequestPacket(const MultiVerSyncTaskContext *context,
    std::map<std::string, MultiVerCommitNode> &commitMap)
{
    CommitHistorySyncRequestPacket *packet = new (std::nothrow) CommitHistorySyncRequestPacket();
    if (packet == nullptr) {
        LOGE("CommitHistorySync::SendRequestPacket : new packet error");
        return -E_OUT_OF_MEMORY;
    }
    packet->SetCommitMap(commitMap);
    packet->SetVersion(SOFTWARE_VERSION_CURRENT);
    Message *message = new (std::nothrow) Message(COMMIT_HISTORY_SYNC_MESSAGE);
    if (message == nullptr) {
        LOGE("CommitHistorySync::SendRequestPacket : new message error");
        delete packet;
        packet = nullptr;
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
        LOGE("CommitHistorySync::SendRequestPacket : SetExternalObject failed errCode:%d", errCode);
        return errCode;
    }
    message->SetSessionId(context->GetRequestSessionId());
    message->SetSequenceId(context->GetSequenceId());

    PerformanceAnalysis *performance = PerformanceAnalysis::GetInstance();
    if (performance != nullptr) {
        performance->StepTimeRecordStart(MV_TEST_RECORDS::RECORD_COMMIT_SEND_REQUEST_TO_ACK_RECV);
    }
    errCode = Send(message->GetTarget(), message);
    if (errCode != E_OK) {
        LOGE("CommitHistorySync::SendRequestPacket : Send failed errCode:%d", errCode);
        delete message;
        message = nullptr;
    }
    return errCode;
}

int CommitHistorySync::SendAckPacket(const MultiVerSyncTaskContext *context,
    std::vector<MultiVerCommitNode> &commits, int ackCode, const Message *message)
{
    if (message == nullptr) {
        LOGE("CommitHistorySync::SendAckPacket : message is nullptr");
        return -E_INVALID_ARGS;
    }
    CommitHistorySyncAckPacket *packet = new (std::nothrow) CommitHistorySyncAckPacket();
    if (packet == nullptr) {
        LOGE("CommitHistorySync::SendAckPacket : packet is nullptr");
        return -E_OUT_OF_MEMORY;
    }
    Message *ackMessage = new (std::nothrow) Message(COMMIT_HISTORY_SYNC_MESSAGE);
    if (ackMessage == nullptr) {
        LOGE("CommitHistorySync::SendAckPacket : new message error");
        delete packet;
        packet = nullptr;
        return -E_OUT_OF_MEMORY;
    }

    packet->SetData(commits);
    packet->SetErrorCode(static_cast<int32_t>(ackCode));
    packet->SetVersion(SOFTWARE_VERSION_CURRENT);
    ackMessage->SetMessageType(TYPE_RESPONSE);
    ackMessage->SetTarget(context->GetDeviceId());
    int errCode = ackMessage->SetExternalObject(packet);
    if (errCode != E_OK) {
        delete packet;
        packet = nullptr;
        delete ackMessage;
        ackMessage = nullptr;
        LOGE("CommitHistorySync::SendAckPacket : SetExternalObject failed errCode:%d", errCode);
        return errCode;
    }
    ackMessage->SetSequenceId(message->GetSequenceId());
    ackMessage->SetSessionId(message->GetSessionId());
    errCode = Send(ackMessage->GetTarget(), ackMessage);
    if (errCode != E_OK) {
        LOGE("CommitHistorySync::SendAckPacket : Send failed errCode:%d", errCode);
        delete ackMessage;
        ackMessage = nullptr;
    }
    return errCode;
}

int CommitHistorySync::GetLocalDeviceInfo(std::string &deviceInfo)
{
    return communicateHandle_->GetLocalIdentity(deviceInfo);
}

int CommitHistorySync::RunPermissionCheck(const std::string &deviceId) const
{
    std::string appId = storagePtr_->GetDbProperties().GetStringProp(KvDBProperties::APP_ID, "");
    std::string userId = storagePtr_->GetDbProperties().GetStringProp(KvDBProperties::USER_ID, "");
    std::string storeId = storagePtr_->GetDbProperties().GetStringProp(KvDBProperties::STORE_ID, "");
    uint8_t flag = CHECK_FLAG_SEND;
    int errCode = RuntimeContext::GetInstance()->RunPermissionCheck(userId, appId, storeId, deviceId, flag);
    if (errCode != E_OK) {
        LOGE("[CommitHistorySync] RunPermissionCheck not pass errCode:%d, flag:%d", errCode, flag);
        return -E_NOT_PERMIT;
    }
    return errCode;
}
}
#endif