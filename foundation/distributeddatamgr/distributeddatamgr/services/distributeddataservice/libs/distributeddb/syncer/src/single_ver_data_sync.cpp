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

#include "single_ver_data_sync.h"

#include "db_common.h"
#include "log_print.h"
#include "single_ver_sync_state_machine.h"
#include "performance_analysis.h"
#include "message_transform.h"
#include "generic_single_ver_kv_entry.h"
#include "single_ver_sync_target.h"
#include "db_constant.h"

namespace DistributedDB {
namespace {
    // index 0 for packetId in data request
    // if ack reserve size is 1, reserve is {localWaterMark}
    // if ack reserve size is above 2, reserve is {localWaterMark, packetId ...}
    constexpr uint32_t REQUEST_PACKET_RESERVED_INDEX_PACKETID = 0;
    constexpr uint32_t ACK_PACKET_RESERVED_INDEX_PACKETID = 1;
    constexpr uint32_t ACK_PACKET_RESERVED_INDEX_LOCAL_WATER_MARK = 0; // index 0 for localWaterMark

    void SetMessageHeadInfo(Message &message, uint16_t inMsgType, const std::string &inTarget, uint32_t inSequenceId,
        uint32_t inSessionId)
    {
        message.SetMessageType(inMsgType);
        message.SetTarget(inTarget);
        message.SetSequenceId(inSequenceId);
        message.SetSessionId(inSessionId);
    }
}

DataRequestPacket::~DataRequestPacket()
{
    for (auto &entry : data_) {
        delete entry;
        entry = nullptr;
    }
}

void DataRequestPacket::SetData(std::vector<SendDataItem> &data)
{
    data_ = std::move(data);
}

const std::vector<SendDataItem> &DataRequestPacket::GetData() const
{
    return data_;
}

void DataRequestPacket::SetEndWaterMark(WaterMark waterMark)
{
    endWaterMark_ = waterMark;
}

WaterMark DataRequestPacket::GetEndWaterMark() const
{
    return endWaterMark_;
}

void DataRequestPacket::SetLocalWaterMark(WaterMark waterMark)
{
    localWaterMark_ = waterMark;
}

WaterMark DataRequestPacket::GetLocalWaterMark() const
{
    return localWaterMark_;
}

void DataRequestPacket::SetPeerWaterMark(WaterMark waterMark)
{
    peerWaterMark_ = waterMark;
}

WaterMark DataRequestPacket::GetPeerWaterMark() const
{
    return peerWaterMark_;
}

void DataRequestPacket::SetSendCode(int32_t errCode)
{
    sendCode_ = errCode;
}

int32_t DataRequestPacket::GetSendCode() const
{
    return sendCode_;
}

void DataRequestPacket::SetMode(int32_t mode)
{
    mode_ = mode;
}

int32_t DataRequestPacket::GetMode() const
{
    return mode_;
}

void DataRequestPacket::SetSessionId(uint32_t sessionId)
{
    sessionId_ = sessionId;
}

uint32_t DataRequestPacket::GetSessionId() const
{
    return sessionId_;
}

void DataRequestPacket::SetVersion(uint32_t version)
{
    version_ = version;
}

uint32_t DataRequestPacket::GetVersion() const
{
    return version_;
}

void DataRequestPacket::SetReserved(std::vector<uint64_t> &reserved)
{
    reserved_ = std::move(reserved);
}

std::vector<uint64_t> DataRequestPacket::GetReserved() const
{
    return reserved_;
}

uint64_t DataRequestPacket::GetPacketId() const
{
    uint64_t packetId = 0;
    std::vector<uint64_t> DataRequestReserve = GetReserved();
    if (DataRequestReserve.size() > REQUEST_PACKET_RESERVED_INDEX_PACKETID) {
        return DataRequestReserve[REQUEST_PACKET_RESERVED_INDEX_PACKETID];
    } else {
        return packetId;
    }
}

uint32_t DataRequestPacket::CalculateLen() const
{
    uint64_t totalLen = GenericSingleVerKvEntry::CalculateLens(data_, version_); // for data
    totalLen += Parcel::GetUInt64Len(); // endWaterMark
    totalLen += Parcel::GetUInt64Len(); // localWaterMark
    totalLen += Parcel::GetUInt64Len(); // peerWaterMark
    totalLen += Parcel::GetIntLen(); // sendCode
    totalLen += Parcel::GetIntLen(); // mode
    totalLen += Parcel::GetUInt32Len(); // sessionId
    totalLen += Parcel::GetUInt32Len(); // version
    totalLen += Parcel::GetVectorLen<uint64_t>(reserved_); // reserved

    if (version_ > SOFTWARE_VERSION_RELEASE_2_0) {
        totalLen += Parcel::GetUInt32Len(); // flag bit0 used for isLastSequence
    }
    totalLen = Parcel::GetEightByteAlign(totalLen); // 8-byte align
    if (totalLen > INT32_MAX) {
        return 0;
    }
    return totalLen;
}

void DataRequestPacket::SetFlag(uint32_t flag)
{
    flag_ = flag;
}

uint32_t DataRequestPacket::GetFlag() const
{
    return flag_;
}

bool DataRequestPacket::IsLastSequence() const
{
    return flag_ & IS_LAST_SEQUENCE;
}

void DataRequestPacket::SetLastSequence()
{
    flag_ = flag_ | IS_LAST_SEQUENCE;
}

void DataRequestPacket::SetBasicInfo(int sendCode, uint32_t version, WaterMark localMark, WaterMark peerMark,
    int32_t mode)
{
    SetSendCode(sendCode);
    SetVersion(version);
    SetLocalWaterMark(localMark);
    SetPeerWaterMark(peerMark);
    SetMode(mode);
}

void DataAckPacket::SetData(uint64_t data)
{
    data_ = data;
}

uint64_t DataAckPacket::GetData() const
{
    return data_;
}

void DataAckPacket::SetRecvCode(int32_t errorCode)
{
    recvCode_ = errorCode;
}

int32_t DataAckPacket::GetRecvCode() const
{
    return recvCode_;
}

void DataAckPacket::SetVersion(uint32_t version)
{
    version_ = version;
}

uint32_t DataAckPacket::GetVersion() const
{
    return version_;
}

void DataAckPacket::SetReserved(std::vector<uint64_t> &reserved)
{
    reserved_ = std::move(reserved);
}

std::vector<uint64_t> DataAckPacket::GetReserved() const
{
    return reserved_;
}

uint64_t DataAckPacket::GetPacketId() const
{
    uint64_t packetId = 0;
    std::vector<uint64_t> DataAckReserve = GetReserved();
    if (DataAckReserve.size() > ACK_PACKET_RESERVED_INDEX_PACKETID) {
        return DataAckReserve[ACK_PACKET_RESERVED_INDEX_PACKETID];
    } else {
        return packetId;
    }
}

bool DataAckPacket::IsPacketIdValid(uint64_t packetId)
{
    return (packetId > 0);
}

uint32_t DataAckPacket::CalculateLen() const
{
    uint64_t len = Parcel::GetUInt64Len(); // ackWaterMark
    len += Parcel::GetIntLen(); // recvCode
    len += Parcel::GetUInt32Len(); // version
    len += Parcel::GetVectorLen<uint64_t>(reserved_); // reserved

    len = Parcel::GetEightByteAlign(len);
    if (len > INT32_MAX) {
        return 0;
    }
    return len;
}

SingleVerDataSync::SingleVerDataSync()
    : mtuSize_(0),
      storage_(nullptr),
      communicateHandle_(nullptr),
      metadata_(nullptr)
{
}

SingleVerDataSync::~SingleVerDataSync()
{
    storage_ = nullptr;
    communicateHandle_ = nullptr;
    metadata_ = nullptr;
}

int SingleVerDataSync::RegisterTransformFunc()
{
    TransformFunc func;
    func.computeFunc = std::bind(&SingleVerDataSync::CalculateLen, std::placeholders::_1);
    func.serializeFunc = std::bind(&SingleVerDataSync::Serialization, std::placeholders::_1,
                                   std::placeholders::_2, std::placeholders::_3);
    func.deserializeFunc = std::bind(&SingleVerDataSync::DeSerialization, std::placeholders::_1,
                                     std::placeholders::_2, std::placeholders::_3);
    return MessageTransform::RegTransformFunction(DATA_SYNC_MESSAGE, func);
}

int SingleVerDataSync::Initialize(IKvDBSyncInterface *inStorage, ICommunicator *inCommunicateHandle,
    std::shared_ptr<Metadata> &inMetadata, const std::string &deviceId)
{
    if ((inStorage == nullptr) || (inCommunicateHandle == nullptr) || (inMetadata == nullptr)) {
        return -E_INVALID_ARGS;
    }
    storage_ = static_cast<SingleVerKvDBSyncInterface *>(inStorage);
    communicateHandle_ = inCommunicateHandle;
    metadata_ = inMetadata;
    mtuSize_ = communicateHandle_->GetCommunicatorMtuSize(deviceId) * 9 / 10; // get the 9/10 of the size.
    std::vector<uint8_t> label = inStorage->GetIdentifier();
    label.resize(3); // only show 3 Bytes enough
    label_ = DBCommon::VectorToHexString(label);
    deviceId_ = deviceId;
    return E_OK;
}

bool SingleVerDataSync::IsPacketValid(const Message *inMsg)
{
    if (inMsg == nullptr) {
        return false;
    }
    if (inMsg->GetMessageId() != DATA_SYNC_MESSAGE) {
        LOGE("[DataSync][IsPacketValid] Message Id ERROR! messageId=%d", inMsg->GetMessageId());
        return false;
    }
    int msgType = inMsg->GetMessageType();
    if (msgType != TYPE_REQUEST && msgType != TYPE_RESPONSE && msgType != TYPE_NOTIFY) {
        LOGE("[DataSync][IsPacketValid] Message type ERROR! message type=%d", msgType);
        return false;
    }
    return true;
}

int SingleVerDataSync::Serialization(uint8_t *buffer, uint32_t length, const Message *inMsg)
{
    if ((buffer == nullptr) || !(IsPacketValid(inMsg))) {
        return -E_MESSAGE_ID_ERROR;
    }

    switch (inMsg->GetMessageType()) {
        case TYPE_REQUEST:
            return DataPacketSerialization(buffer, length, inMsg);
        case TYPE_RESPONSE:
        case TYPE_NOTIFY:
            return AckPacketSerialization(buffer, length, inMsg);
        default:
            return -E_MESSAGE_TYPE_ERROR;
    }
}

int SingleVerDataSync::DeSerialization(const uint8_t *buffer, uint32_t length, Message *inMsg)
{
    if ((buffer == nullptr) || !(IsPacketValid(inMsg))) {
        return -E_MESSAGE_ID_ERROR;
    }

    switch (inMsg->GetMessageType()) {
        case TYPE_REQUEST:
            return DataPacketDeSerialization(buffer, length, inMsg);
        case TYPE_RESPONSE:
        case TYPE_NOTIFY:
            return AckPacketDeSerialization(buffer, length, inMsg);
        default:
            return -E_MESSAGE_TYPE_ERROR;
    }
}

uint32_t SingleVerDataSync::CalculateLen(const Message *inMsg)
{
    if (!(IsPacketValid(inMsg))) {
        return 0;
    }
    uint32_t len = 0;
    int errCode;
    switch (inMsg->GetMessageType()) {
        case TYPE_REQUEST:
            errCode = DataPacketCalculateLen(inMsg, len);
            if (errCode != E_OK) {
                LOGE("[DataSync][CalculateLen] calculate data request packet len failed, errCode=%d", errCode);
                return 0;
            }
            return len;
        case TYPE_RESPONSE:
        case TYPE_NOTIFY:
            errCode = AckPacketCalculateLen(inMsg, len);
            if (errCode != E_OK) {
                LOGE("[DataSync][CalculateLen] calculate data notify packet len failed errCode=%d", errCode);
                return 0;
            }
            return len;
        default:
            return 0;
    }
}

int SingleVerDataSync::DataPacketCalculateLen(const Message *inMsg, uint32_t &len)
{
    const DataRequestPacket *packet = inMsg->GetObject<DataRequestPacket>();
    if (packet == nullptr) {
        return -E_INVALID_ARGS;
    }

    len = packet->CalculateLen();
    return E_OK;
}

int SingleVerDataSync::AckPacketCalculateLen(const Message *inMsg, uint32_t &len)
{
    const DataAckPacket *packet = inMsg->GetObject<DataAckPacket>();
    if (packet == nullptr) {
        return -E_INVALID_ARGS;
    }

    len = packet->CalculateLen();
    return E_OK;
}

std::string SingleVerDataSync::GetLocalDeviceName()
{
    std::string deviceInfo;
    if (communicateHandle_ != nullptr) {
        communicateHandle_->GetLocalIdentity(deviceInfo);
    }
    return deviceInfo;
}

std::string SingleVerDataSync::TransferForeignOrigDevName(const std::string &deviceName)
{
    // Get the hash device name.
    std::string localName = GetLocalDeviceName();
    if (DBCommon::TransferHashString(localName) == deviceName) {
        return "";
    }
    return deviceName;
}

std::string SingleVerDataSync::TransferLocalOrigDevName(const std::string &origName)
{
    std::string localName = GetLocalDeviceName();
    if (origName.empty()) {
        return DBCommon::TransferHashString(localName);
    }
    return origName;
}

int SingleVerDataSync::DataPacketSyncerPartSerialization(Parcel &parcel, const DataRequestPacket *packet)
{
    // endWaterMark
    int errCode = parcel.WriteUInt64(packet->GetEndWaterMark());
    if (errCode != E_OK) {
        return errCode;
    }
    // localWaterMark
    errCode = parcel.WriteUInt64(packet->GetLocalWaterMark());
    if (errCode != E_OK) {
        return errCode;
    }
    // peerWaterMark
    errCode = parcel.WriteUInt64(packet->GetPeerWaterMark());
    if (errCode != E_OK) {
        return errCode;
    }
    // sendCode
    errCode = parcel.WriteInt(packet->GetSendCode());
    if (errCode != E_OK) {
        return errCode;
    }
    // mode
    errCode = parcel.WriteInt(packet->GetMode());
    if (errCode != E_OK) {
        return errCode;
    }
    // sessionId
    errCode = parcel.WriteUInt32(packet->GetSessionId());
    if (errCode != E_OK) {
        return errCode;
    }
    // reserved
    errCode = parcel.WriteVector<uint64_t>(packet->GetReserved());
    if (errCode != E_OK) {
        return errCode;
    }
    if (packet->GetVersion() > SOFTWARE_VERSION_RELEASE_2_0) {
        errCode = parcel.WriteUInt32(packet->GetFlag());
        if (errCode != E_OK) {
            return errCode;
        }
    }
    parcel.EightByteAlign();
    return errCode;
}

int SingleVerDataSync::DataPacketSerialization(uint8_t *buffer, uint32_t length, const Message *inMsg)
{
    const DataRequestPacket *packet = inMsg->GetObject<DataRequestPacket>();
    if (packet == nullptr) {
        return -E_INVALID_ARGS;
    }
    Parcel parcel(buffer, length);

    // version
    int errCode = parcel.WriteUInt32(packet->GetVersion());
    if (errCode != E_OK) {
        return errCode;
    }
    // sendDataItems
    const std::vector<SendDataItem> &data = packet->GetData();
    errCode = GenericSingleVerKvEntry::SerializeDatas(data, parcel, packet->GetVersion());
    if (errCode != E_OK) {
        return errCode;
    }
    return SingleVerDataSync::DataPacketSyncerPartSerialization(parcel, packet);
}

int SingleVerDataSync::DataPacketSyncerPartDeSerialization(Parcel &parcel, DataRequestPacket *packet,
    uint32_t packLen, uint32_t length, uint32_t version)
{
    WaterMark waterMark;
    WaterMark localWaterMark;
    WaterMark peerWaterMark;
    int32_t sendCode;
    int32_t mode;
    uint32_t sessionId;
    uint32_t flag = 0;
    std::vector<uint64_t> reserved;

    packLen += parcel.ReadUInt64(waterMark);
    packLen += parcel.ReadUInt64(localWaterMark);
    packLen += parcel.ReadUInt64(peerWaterMark);
    packLen += parcel.ReadInt(sendCode);
    packLen += parcel.ReadInt(mode);
    packLen += parcel.ReadUInt32(sessionId);
    packLen += parcel.ReadVector<uint64_t>(reserved);
    if (version > SOFTWARE_VERSION_RELEASE_2_0) {
        packLen += parcel.ReadUInt32(flag);
        packet->SetFlag(flag);
    }
    packLen = Parcel::GetEightByteAlign(packLen);
    if (packLen != length || parcel.IsError()) {
        LOGE("[DataSync][DataPacketDeSerialization] deserialize failed! input len=%lu,packLen=%lu", length, packLen);
        return -E_LENGTH_ERROR;
    }
    packet->SetEndWaterMark(waterMark);
    packet->SetLocalWaterMark(localWaterMark);
    packet->SetPeerWaterMark(peerWaterMark);
    packet->SetSendCode(sendCode);
    packet->SetMode(mode);
    packet->SetSessionId(sessionId);
    packet->SetReserved(reserved);
    return E_OK;
}

int SingleVerDataSync::DataPacketDeSerialization(const uint8_t *buffer, uint32_t length, Message *inMsg)
{
    std::vector<SendDataItem> dataItems;
    uint32_t version;
    Parcel parcel(const_cast<uint8_t *>(buffer), length);
    uint32_t packLen = parcel.ReadUInt32(version);
    if (parcel.IsError()) {
        return -E_INVALID_ARGS;
    }
    DataRequestPacket *packet = new (std::nothrow) DataRequestPacket();
    if (packet == nullptr) {
        return -E_OUT_OF_MEMORY;
    }
    if (version > SOFTWARE_VERSION_CURRENT) {
        packet->SetVersion(version);
        packet->SetSendCode(-E_VERSION_NOT_SUPPORT);
        int errCode = inMsg->SetExternalObject<>(packet);
        if (errCode != E_OK) {
            delete packet;
            packet = nullptr;
        }
        return errCode;
    }
    packet->SetVersion(version);
    packLen += GenericSingleVerKvEntry::DeSerializeDatas(dataItems, parcel);
    packet->SetData(dataItems);
    int errCode = SingleVerDataSync::DataPacketSyncerPartDeSerialization(parcel, packet, packLen, length, version);
    if (errCode != E_OK) {
        delete packet;
        packet = nullptr;
        return errCode;
    }
    errCode = inMsg->SetExternalObject<>(packet);
    if (errCode != E_OK) {
        delete packet;
        packet = nullptr;
    }
    return errCode;
}

int SingleVerDataSync::AckPacketSyncerPartSerializationV1(Parcel &parcel, const DataAckPacket *packet)
{
    int errCode = parcel.WriteUInt64(packet->GetData());
    if (errCode != E_OK) {
        return errCode;
    }
    errCode = parcel.WriteInt(packet->GetRecvCode());
    if (errCode != E_OK) {
        return errCode;
    }
    errCode = parcel.WriteVector<uint64_t>(packet->GetReserved());
    if (errCode != E_OK) {
        return errCode;
    }
    parcel.EightByteAlign();
    return errCode;
}

int SingleVerDataSync::AckPacketSerialization(uint8_t *buffer, uint32_t length, const Message *inMsg)
{
    const DataAckPacket *packet = inMsg->GetObject<DataAckPacket>();
    if (packet == nullptr) {
        return -E_INVALID_ARGS;
    }

    Parcel parcel(buffer, length);
    int errCode = parcel.WriteUInt32(packet->GetVersion());
    if (errCode != E_OK) {
        return errCode;
    }
    // now V1 compatible for softWareVersion :{101, 102}
    return SingleVerDataSync::AckPacketSyncerPartSerializationV1(parcel, packet);
}

int SingleVerDataSync::AckPacketSyncerPartDeSerializationV1(Parcel &parcel, DataAckPacket &packet)
{
    WaterMark mark;
    int32_t errCode;
    std::vector<uint64_t> reserved;

    parcel.ReadUInt64(mark);
    parcel.ReadInt(errCode);
    parcel.ReadVector<uint64_t>(reserved);
    if (parcel.IsError()) {
        return -E_INVALID_ARGS;
    }
    packet.SetData(mark);
    packet.SetRecvCode(errCode);
    packet.SetReserved(reserved);
    return E_OK;
}

int SingleVerDataSync::AckPacketDeSerialization(const uint8_t *buffer, uint32_t length, Message *inMsg)
{
    DataAckPacket packet;
    Parcel parcel(const_cast<uint8_t *>(buffer), length);
    uint32_t version;

    parcel.ReadUInt32(version);
    if (parcel.IsError()) {
        return -E_INVALID_ARGS;
    }
    if (version > SOFTWARE_VERSION_CURRENT) {
        packet.SetVersion(version);
        packet.SetRecvCode(-E_VERSION_NOT_SUPPORT);
        return inMsg->SetCopiedObject<>(packet);
    }
    packet.SetVersion(version);
    // now V1 compatible for softWareVersion :{101, 102}
    int errCode = SingleVerDataSync::AckPacketSyncerPartDeSerializationV1(parcel, packet);
    if (errCode != E_OK) {
        return errCode;
    }

    return inMsg->SetCopiedObject<>(packet);
}

int SingleVerDataSync::Send(SingleVerSyncTaskContext *context, const Message *message, const CommErrHandler &handler,
    uint32_t packetLen)
{
    bool startFeedDogRet = false;
    if (packetLen > mtuSize_ && mtuSize_ > 0) {
        uint32_t time = static_cast<uint32_t>(static_cast<uint64_t>(packetLen) *
            static_cast<uint64_t>(DBConstant::AUTO_SYNC_TIMEOUT) / mtuSize_); // no overflow
        startFeedDogRet = context->StartFeedDogForSync(time, SyncDirectionFlag::SEND);
    }
    int errCode = communicateHandle_->SendMessage(context->GetDeviceId(), message, false, SEND_TIME_OUT, handler);
    if (errCode != E_OK) {
        LOGE("[DataSync][Send] send message failed, errCode=%d", errCode);
        if (startFeedDogRet) {
            context->StopFeedDogForSync(SyncDirectionFlag::SEND);
        }
    }
    return errCode;
}

int SingleVerDataSync::GetData(SingleVerSyncTaskContext *context, std::vector<SendDataItem> &outData, size_t packetSize)
{
    int errCode;
    if (context->GetRetryStatus() == SyncTaskContext::NEED_RETRY) {
        context->SetRetryStatus(SyncTaskContext::NO_NEED_RETRY);
        LOGI("[DataSync][GetData] resend data");
        errCode = GetUnsyncData(context, outData, packetSize);
    } else {
        ContinueToken token;
        context->GetContinueToken(token);
        if (token == nullptr) {
            errCode = GetUnsyncData(context, outData, packetSize);
        } else {
            LOGD("[DataSync][GetData] get data from token");
            // if there is data to send, read out data, and update local watermark, send data
            errCode = GetNextUnsyncData(context, outData, packetSize);
        }
    }
    if (errCode == E_OK || errCode == -E_UNFINISHED) {
        TransDbDataItemToSendDataItem(context, outData);
    }
    if (errCode == -E_UNFINISHED) {
        LOGD("[DataSync][GetData] not finished.");
    }
    if (errCode == -E_EKEYREVOKED) {
        context->SetTaskErrCode(-E_EKEYREVOKED);
    }
    if (errCode == -E_BUSY) {
        context->SetTaskErrCode(-E_BUSY);
    }
    return errCode;
}

int SingleVerDataSync::GetDataWithRerformanceRecord(SingleVerSyncTaskContext *context,
    std::vector<SendDataItem> &outData)
{
    uint32_t version = std::min(context->GetRemoteSoftwareVersion(), SOFTWARE_VERSION_CURRENT);
    size_t packetSize = (version > SOFTWARE_VERSION_RELEASE_2_0) ?
        DBConstant::MAX_HPMODE_PACK_ITEM_SIZE : DBConstant::MAX_NORMAL_PACK_ITEM_SIZE;
    PerformanceAnalysis *performance = PerformanceAnalysis::GetInstance();
    if (performance != nullptr) {
        performance->StepTimeRecordStart(PT_TEST_RECORDS::RECORD_READ_DATA);
    }
    int errCode = GetData(context, outData, packetSize);
    if (performance != nullptr) {
        performance->StepTimeRecordEnd(PT_TEST_RECORDS::RECORD_READ_DATA);
    }
    return errCode;
}

int SingleVerDataSync::GetUnsyncData(SingleVerSyncTaskContext *context, std::vector<SendDataItem> &outData,
    size_t packetSize)
{
    WaterMark startMark;
    metadata_->GetLocalWaterMark(context->GetDeviceId(), startMark);
    WaterMark endMark = context->GetEndMark();
    if ((endMark == 0) || (startMark > endMark)) {
        return E_OK;
    }
    ContinueToken token = nullptr;
    context->GetContinueToken(token);
    if (token != nullptr) {
        storage_->ReleaseContinueToken(token);
    }
    DataSizeSpecInfo syncDataSizeInfo = {mtuSize_, packetSize};
    int errCode = storage_->GetSyncData(startMark, endMark, outData, token, syncDataSizeInfo);
    context->SetContinueToken(token);
    if (errCode != E_OK && errCode != -E_UNFINISHED) {
        LOGE("[DataSync][GetUnsyncData] get unsync data failed,errCode=%d", errCode);
    }
    return errCode;
}

int SingleVerDataSync::GetNextUnsyncData(SingleVerSyncTaskContext *context, std::vector<SendDataItem> &outData,
    size_t packetSize)
{
    ContinueToken token;
    context->GetContinueToken(token);
    DataSizeSpecInfo syncDataSizeInfo = {mtuSize_, packetSize};
    int errCode = storage_->GetSyncDataNext(outData, token, syncDataSizeInfo);
    context->SetContinueToken(token);
    if (errCode != E_OK && errCode != -E_UNFINISHED) {
        LOGE("[DataSync][GetNextUnsyncData] get next unsync data failed, errCode=%d", errCode);
    }
    return errCode;
}

int SingleVerDataSync::SaveData(const SingleVerSyncTaskContext *context, const std::vector<SendDataItem> &inData)
{
    if (inData.empty()) {
        return E_OK;
    }
    PerformanceAnalysis *performance = PerformanceAnalysis::GetInstance();
    if (performance != nullptr) {
        performance->StepTimeRecordStart(PT_TEST_RECORDS::RECORD_SAVE_DATA);
    }

    TransSendDataItemToLocal(context, inData);
    int errCode = storage_->PutSyncData(inData, context->GetDeviceId());
    if (performance != nullptr) {
        performance->StepTimeRecordEnd(PT_TEST_RECORDS::RECORD_SAVE_DATA);
    }
    if (errCode != E_OK) {
        LOGE("[DataSync][SaveData] save sync data failed,errCode=%d", errCode);
    }
    return errCode;
}

TimeStamp SingleVerDataSync::GetMaxSendDataTime(const std::vector<SendDataItem> &inData, bool isNeedInit,
    WaterMark localMark)
{
    TimeStamp stamp = 0;
    if (isNeedInit) {
        stamp = localMark;
    }
    for (size_t i = 0; i < inData.size(); i++) {
        if (inData[i] == nullptr) {
            continue;
        }
        TimeStamp tempStamp = inData[i]->GetTimestamp();
        if (stamp < tempStamp) {
            stamp = tempStamp;
        }
    }
    return stamp;
}

TimeStamp SingleVerDataSync::GetMinSendDataTime(const std::vector<SendDataItem> &inData, WaterMark localMark)
{
    TimeStamp stamp = localMark;
    for (size_t i = 0; i < inData.size(); i++) {
        if (inData[i] == nullptr) {
            continue;
        }
        TimeStamp tempStamp = inData[i]->GetTimestamp();
        if (stamp > tempStamp) {
            stamp = tempStamp;
        }
    }
    return stamp;
}

int SingleVerDataSync::SaveLocalWaterMark(const DeviceID &deviceId, WaterMark waterMark)
{
    PerformanceAnalysis *performance = PerformanceAnalysis::GetInstance();
    if (performance != nullptr) {
        performance->StepTimeRecordStart(PT_TEST_RECORDS::RECORD_SAVE_LOCAL_WATERMARK);
    }
    int errCode = metadata_->SaveLocalWaterMark(deviceId, waterMark);
    if (performance != nullptr) {
        performance->StepTimeRecordEnd(PT_TEST_RECORDS::RECORD_SAVE_LOCAL_WATERMARK);
    }
    if (errCode != E_OK) {
        LOGE("[DataSync][SaveLocalWaterMark] save metadata local watermark failed,errCode=%d", errCode);
    }
    return errCode;
}

int SingleVerDataSync::SavePeerWaterMark(const DeviceID &deviceId, WaterMark waterMark)
{
    return metadata_->SavePeerWaterMark(deviceId, waterMark);
}

int SingleVerDataSync::RemoveDeviceData(SingleVerSyncTaskContext *context, const Message *message,
    WaterMark maxSendDataTime)
{
    int errCode = storage_->RemoveDeviceData(context->GetDeviceId(), true);
    if (errCode != E_OK) {
        (void)SendAck(context, message, errCode, maxSendDataTime);
        return errCode;
    }
    if (context->GetRemoteSoftwareVersion() == SOFTWARE_VERSION_EARLIEST) {
        (void)SaveLocalWaterMark(context->GetDeviceId(), 0); // avoid repeat clear in ack
    }
    return E_OK;
}

void SingleVerDataSync::TransDbDataItemToSendDataItem(const SingleVerSyncTaskContext *context,
    std::vector<SendDataItem> &outData)
{
    for (size_t i = 0; i < outData.size(); i++) {
        if (outData[i] == nullptr) {
            continue;
        }
        outData[i]->SetOrigDevice(TransferLocalOrigDevName(outData[i]->GetOrigDevice()));
        if (i == 0 || i == (outData.size() - 1)) {
            LOGD("[DataSync][TransToSendItem] printData packet=%d,timeStamp=%llu,flag=%llu", i,
                outData[i]->GetTimestamp(), outData[i]->GetFlag());
        }
    }
}

void SingleVerDataSync::TransSendDataItemToLocal(const SingleVerSyncTaskContext *context,
    const std::vector<SendDataItem> &data)
{
    TimeOffset offset = context->GetTimeOffset();
    TimeStamp currentLocalTime = context->GetCurrentLocalTime();
    for (auto &item : data) {
        if (item == nullptr) {
            continue;
        }
        item->SetOrigDevice(TransferForeignOrigDevName(item->GetOrigDevice()));
        TimeStamp tempTimestamp = item->GetTimestamp();
        TimeStamp tempWriteTimestamp = item->GetWriteTimestamp();
        item->SetTimestamp(tempTimestamp - offset);
        if (tempWriteTimestamp != 0) {
            item->SetWriteTimestamp(tempWriteTimestamp - offset);
        }

        if (item->GetTimestamp() > currentLocalTime) {
            item->SetTimestamp(currentLocalTime);
        }
        if (item->GetWriteTimestamp() > currentLocalTime) {
            item->SetWriteTimestamp(currentLocalTime);
        }
    }
}

int SingleVerDataSync::PushStart(SingleVerSyncTaskContext *context)
{
    if (context == nullptr) {
        return -E_INVALID_ARGS;
    }
    std::vector<SendDataItem> outData;
    WaterMark localMark;
    WaterMark peerMark;
    metadata_->GetLocalWaterMark(context->GetDeviceId(), localMark);
    metadata_->GetPeerWaterMark(context->GetDeviceId(), peerMark);
    uint32_t version = std::min(context->GetRemoteSoftwareVersion(), SOFTWARE_VERSION_CURRENT);
    LOGD("[DataSync][PushStart] localMark=%llu,endmark=%llu,peerMark=%llu,label=%s,dev=%s{private}", localMark,
        context->GetEndMark(), peerMark, label_.c_str(), GetDeviceId().c_str());
    // get data
    int errCode = GetDataWithRerformanceRecord(context, outData);
    if (errCode != E_OK && errCode != -E_UNFINISHED) {
        LOGE("[DataSync][PushStart] get data failed, errCode=%d", errCode);
        return errCode;
    }

    DataRequestPacket *packet = new (std::nothrow) DataRequestPacket;
    if (packet == nullptr) {
        LOGE("[DataSync][PushStart] new DataRequestPacket error");
        return -E_OUT_OF_MEMORY;
    }
    bool isUpdateWaterMark = (outData.size() > 0);
    TimeStamp maxSendDateTime = GetMaxSendDataTime(outData, true, localMark);
    TimeStamp minSendDateTime = GetMinSendDataTime(outData, localMark);
    context->SetSequenceStartAndEndTimeStamp(minSendDateTime, maxSendDateTime);
    if (errCode == E_OK) {
        context->SetSessionEndTimeStamp(maxSendDateTime);
        packet->SetLastSequence();
    }
    packet->SetData(outData);
    packet->SetBasicInfo(errCode, version, localMark, peerMark, SyncOperation::PUSH);
    SetPacketId(packet, context, version);
    errCode = SendDataPacket(packet, context);
    PerformanceAnalysis *performance = PerformanceAnalysis::GetInstance();
    if (performance != nullptr) {
        performance->StepTimeRecordEnd(PT_TEST_RECORDS::RECORD_MACHINE_START_TO_PUSH_SEND);
    }
    if (errCode == E_OK && isUpdateWaterMark) {
        SaveLocalWaterMark(context->GetDeviceId(), maxSendDateTime + 1);
    }
    return errCode;
}

int SingleVerDataSync::PushPullStart(SingleVerSyncTaskContext *context)
{
    if (context == nullptr) {
        return -E_INVALID_ARGS;
    }
    WaterMark localMark;
    WaterMark peerMark;
    metadata_->GetLocalWaterMark(context->GetDeviceId(), localMark);
    metadata_->GetPeerWaterMark(context->GetDeviceId(), peerMark);
    LOGD("[DataSync][PushPull] localMark=%llu,endmark=%llu,peerMark=%llu,label=%s,dev=%s{private}", localMark,
        context->GetEndMark(), peerMark, label_.c_str(), GetDeviceId().c_str());
    // get data
    std::vector<SendDataItem> outData;
    int errCode = GetDataWithRerformanceRecord(context, outData);
    // once get data occur E_EKEYREVOKED error, should also send request to remote dev to pull data.
    if (context->GetRemoteSoftwareVersion() > SOFTWARE_VERSION_RELEASE_2_0  && errCode == -E_EKEYREVOKED) {
        errCode = E_OK;
    }
    if (errCode != E_OK && errCode != -E_UNFINISHED) {
        return errCode;
    }

    DataRequestPacket *packet = new (std::nothrow) DataRequestPacket;
    if (packet == nullptr) {
        LOGE("[DataSync][PushPullStart] new DataRequestPacket error");
        return -E_OUT_OF_MEMORY;
    }
    bool isUpdateWaterMark = (outData.size() > 0);
    TimeStamp maxSendDateTime = GetMaxSendDataTime(outData, true, localMark);
    TimeStamp minSendDateTime = GetMinSendDataTime(outData, localMark);
    uint32_t version = std::min(context->GetRemoteSoftwareVersion(), SOFTWARE_VERSION_CURRENT);
    context->SetSequenceStartAndEndTimeStamp(minSendDateTime, maxSendDateTime);
    if (errCode == E_OK) {
        context->SetSessionEndTimeStamp(maxSendDateTime);
        packet->SetLastSequence();
    }
    packet->SetData(outData);
    packet->SetBasicInfo(errCode, version, localMark, peerMark, SyncOperation::PUSH_AND_PULL);
    packet->SetEndWaterMark(context->GetEndMark());
    packet->SetSessionId(context->GetRequestSessionId());
    SetPacketId(packet, context, version);
    int sendErrCode = SendDataPacket(packet, context);
    if (sendErrCode == E_OK && isUpdateWaterMark) {
        SaveLocalWaterMark(context->GetDeviceId(), maxSendDateTime + 1);
    }
    return sendErrCode;
}

int SingleVerDataSync::PullRequestStart(SingleVerSyncTaskContext *context)
{
    if (context == nullptr) {
        return -E_INVALID_ARGS;
    }
    DataRequestPacket *packet = new (std::nothrow) DataRequestPacket;
    if (packet == nullptr) {
        LOGE("[DataSync][PullRequest]new DataRequestPacket error");
        return -E_OUT_OF_MEMORY;
    }
    WaterMark peerMark;
    WaterMark localMark;
    metadata_->GetPeerWaterMark(context->GetDeviceId(), peerMark);
    metadata_->GetLocalWaterMark(context->GetDeviceId(), localMark);
    packet->SetLocalWaterMark(localMark);
    packet->SetPeerWaterMark(peerMark);
    packet->SetMode(SyncOperation::PULL);
    WaterMark endMark = context->GetEndMark();
    packet->SetEndWaterMark(endMark);
    packet->SetSessionId(context->GetRequestSessionId());
    uint32_t version = std::min(context->GetRemoteSoftwareVersion(), SOFTWARE_VERSION_CURRENT);
    packet->SetVersion(version);
    packet->SetLastSequence();
    SetPacketId(packet, context, version);
    LOGD("[DataSync][PullRequest]peerMark=%llu,localMark=%llu,endMark=%llu,IsLastSeq=%d,label=%s,dev=%s{private}",
        peerMark, localMark, endMark, packet->IsLastSequence(), label_.c_str(), GetDeviceId().c_str());
    return SendDataPacket(packet, context);
}

int SingleVerDataSync::PullResponseStart(SingleVerSyncTaskContext *context)
{
    if (context == nullptr) {
        return -E_INVALID_ARGS;
    }
    WaterMark localMark;
    WaterMark peerMark;
    metadata_->GetLocalWaterMark(context->GetDeviceId(), localMark);
    metadata_->GetPeerWaterMark(context->GetDeviceId(), peerMark);
    LOGD("[DataSync][PullResponse] localMark=%llu,pullendmark=%llu,peerMark=%llu,label=%s,dev=%s{private}",
        localMark, context->GetEndMark(), peerMark, label_.c_str(), GetDeviceId().c_str());
    // get data
    std::vector<SendDataItem> outData;
    int errCode = GetDataWithRerformanceRecord(context, outData);
    if (errCode != E_OK && errCode != -E_UNFINISHED) {
        if (context->GetRemoteSoftwareVersion() > SOFTWARE_VERSION_RELEASE_2_0) {
            SendPullResponseDataPkt(errCode, outData, context);
        }
        return errCode;
    }

    // if send finished
    int ackCode = E_OK;
    ContinueToken token = nullptr;
    context->GetContinueToken(token);
    if (errCode == E_OK && token == nullptr) {
        LOGD("[DataSync][PullResponse] send last frame end");
        ackCode = SEND_FINISHED;
    }
    bool isUpdateWaterMark = (outData.size() > 0);
    TimeStamp maxSendDateTime = GetMaxSendDataTime(outData, true, localMark);
    TimeStamp minSendDateTime = GetMinSendDataTime(outData, localMark);
    context->SetSequenceStartAndEndTimeStamp(minSendDateTime, maxSendDateTime);
    if (errCode == E_OK) {
        context->SetSessionEndTimeStamp(maxSendDateTime);
    }
    errCode = SendPullResponseDataPkt(ackCode, outData, context);
    if (errCode == E_OK && isUpdateWaterMark) {
        SaveLocalWaterMark(context->GetDeviceId(), maxSendDateTime + 1);
    }
    return errCode;
}

void SingleVerDataSync::UpdatePeerWaterMark(const SingleVerSyncTaskContext *context, WaterMark peerWatermark)
{
    if (peerWatermark == 0) {
        return;
    }
    PerformanceAnalysis *performance = PerformanceAnalysis::GetInstance();
    if (performance != nullptr) {
        performance->StepTimeRecordStart(PT_TEST_RECORDS::RECORD_SAVE_PEER_WATERMARK);
    }

    int errCode = SavePeerWaterMark(context->GetDeviceId(), peerWatermark + 1);
    if (performance != nullptr) {
        performance->StepTimeRecordEnd(PT_TEST_RECORDS::RECORD_SAVE_PEER_WATERMARK);
    }
    if (errCode != E_OK) {
        LOGE("[DataSync][UpdatePeerWaterMark] save peer water mark failed,errCode=%d", errCode);
    }
}

int SingleVerDataSync::RequestRecvPre(SingleVerSyncTaskContext *context, const Message *message)
{
    if (context == nullptr || message == nullptr) {
        return -E_INVALID_ARGS;
    }
    const DataRequestPacket *packet = message->GetObject<DataRequestPacket>();
    if (packet == nullptr) {
        return -E_INVALID_ARGS;
    }
    if (context->GetRemoteSoftwareVersion() <= SOFTWARE_VERSION_BASE) {
        uint16_t remoteCommunicatorVersion = 0;
        if (communicateHandle_->GetRemoteCommunicatorVersion(context->GetDeviceId(), remoteCommunicatorVersion) ==
            -E_NOT_FOUND) {
            LOGE("[DataSync][RequestRecvPre] get remote communicator version failed");
            return -E_VERSION_NOT_SUPPORT;
        }
        // If remote is not the first version, we need check SOFTWARE_VERSION_BASE
        if (remoteCommunicatorVersion == 0) {
            LOGI("[DataSync] set remote version 0");
            context->SetRemoteSoftwareVersion(SOFTWARE_VERSION_EARLIEST);
            return E_OK;
        } else {
            LOGI("[DataSync][RequestRecvPre] need do ability sync");
            SendAck(context, message, -E_NEED_ABILITY_SYNC, 0);
            return -E_WAIT_NEXT_MESSAGE;
        }
    }
    int32_t sendCode = packet->GetSendCode();
    if (sendCode == -E_VERSION_NOT_SUPPORT) {
        LOGE("[DataSync] Version mismatch: ver=%u, current=%u", packet->GetVersion(), SOFTWARE_VERSION_CURRENT);
        (void)SendAck(context, message, -E_VERSION_NOT_SUPPORT, 0);
        return -E_WAIT_NEXT_MESSAGE;
    }
    // only deal with pull response packet errCode
    if (sendCode != E_OK && sendCode != SEND_FINISHED &&
        message->GetSessionId() == context->GetRequestSessionId()) {
        LOGE("[DataSync][RequestRecvPre] remote pullResponse getData sendCode=%d", sendCode);
        return sendCode;
    }
    int errCode = RunPermissionCheck(context, message, packet);
    if (errCode != E_OK) {
        return errCode;
    }
    uint32_t version = std::min(context->GetRemoteSoftwareVersion(), SOFTWARE_VERSION_CURRENT);
    if (version > SOFTWARE_VERSION_RELEASE_2_0) {
        errCode = CheckSchemaStrategy(context, message);
    }
    return errCode;
}

int SingleVerDataSync::RequestRecv(SingleVerSyncTaskContext *context, const Message *message,
    WaterMark &pullEndWatermark)
{
    int errCode = RequestRecvPre(context, message);
    if (errCode != E_OK) {
        return errCode;
    }
    const DataRequestPacket *packet = message->GetObject<DataRequestPacket>();
    const std::vector<SendDataItem> &data = packet->GetData();
    LOGI("[DataSync][RequestRecv] remote ver=%u,size=%d,errCode=%d,Label=%s,dev=%s{private}", packet->GetVersion(),
        data.size(), packet->GetSendCode(), label_.c_str(), GetDeviceId().c_str());
    WaterMark packetLocalMark = packet->GetLocalWaterMark();
    WaterMark peerMark;
    metadata_->GetPeerWaterMark(context->GetDeviceId(), peerMark);
    context->SetReceiveWaterMarkErr(false);
    WaterMark maxSendDataTime = GetMaxSendDataTime(data, false);
    if (packetLocalMark > peerMark) {
        LOGI("[DataSync][RequestRecv] packetLocalMark=%llu,current=%llu", packetLocalMark, peerMark);
        return SendLocalWaterMarkAck(context, message);
    } else if ((packetLocalMark == 0) && (peerMark != 0) && context->IsNeedClearRemoteStaleData()) {
        // need to clear remote device history data
        errCode = RemoveDeviceData(context, message, maxSendDataTime);
        if (errCode != E_OK) {
            return errCode;
        }
    }
    GetPullEndWatermark(context, packet, pullEndWatermark);
    // save data first
    errCode = SaveData(context, data);
    if (errCode != E_OK) {
        (void)SendAck(context, message, errCode, maxSendDataTime);
        return errCode;
    }
    if (pullEndWatermark > 0 && !storage_->IsReadable()) { // pull mode
        pullEndWatermark = 0;
        errCode = SendAck(context, message, -E_EKEYREVOKED, maxSendDataTime);
    } else {
        // if data is empty, we don't know the max timestap of this packet.
        errCode = SendAck(context, message, !data.empty() ? E_OK : WATER_MARK_INVALID, maxSendDataTime);
    }
    RemotePushFinished(packet->GetSendCode(), packet->GetMode(), message->GetSessionId(),
        context->GetRequestSessionId());
    UpdatePeerWaterMark(context, maxSendDataTime);
    if (errCode != E_OK) {
        return errCode;
    }
    if (packet->GetSendCode() == SEND_FINISHED) {
        return -E_RECV_FINISHED;
    }

    return errCode;
}

int SingleVerDataSync::SendDataPacket(const DataRequestPacket *packet, SingleVerSyncTaskContext *context)
{
    Message *message = new (std::nothrow) Message(DATA_SYNC_MESSAGE);
    if (message == nullptr) {
        LOGE("[DataSync][SendDataPacket] new message error");
        delete packet;
        packet = nullptr;
        return -E_OUT_OF_MEMORY;
    }
    uint32_t packetLen = packet->CalculateLen();
    int errCode = message->SetExternalObject(packet);
    if (errCode != E_OK) {
        delete packet;
        packet = nullptr;
        delete message;
        message = nullptr;
        LOGE("[DataSync][SendDataPacket] set external object failed errCode=%d", errCode);
        return errCode;
    }
    SetMessageHeadInfo(*message, TYPE_REQUEST, context->GetDeviceId(), context->GetSequenceId(),
        context->GetRequestSessionId());
    PerformanceAnalysis *performance = PerformanceAnalysis::GetInstance();
    if (performance != nullptr) {
        performance->StepTimeRecordStart(PT_TEST_RECORDS::RECORD_DATA_SEND_REQUEST_TO_ACK_RECV);
    }

    CommErrHandler handler = std::bind(&SyncTaskContext::CommErrHandlerFunc, std::placeholders::_1,
        context, message->GetSessionId());
    errCode = Send(context, message, handler, packetLen);
    if (errCode != E_OK) {
        delete message;
        message = nullptr;
    }

    return errCode;
}

int SingleVerDataSync::SendPullResponseDataPkt(int ackCode, std::vector<SendDataItem> &inData,
    SingleVerSyncTaskContext *context)
{
    DataRequestPacket *packet = new (std::nothrow) DataRequestPacket;
    if (packet == nullptr) {
        LOGE("[DataSync][SendPullResponseDataPkt] new data request packet error");
        return -E_OUT_OF_MEMORY;
    }
    size_t size = inData.size();
    WaterMark localMark;
    WaterMark peerMark;
    metadata_->GetLocalWaterMark(context->GetDeviceId(), localMark);
    metadata_->GetPeerWaterMark(context->GetDeviceId(), peerMark);
    uint32_t version = std::min(context->GetRemoteSoftwareVersion(), SOFTWARE_VERSION_CURRENT);
    packet->SetBasicInfo(ackCode, version, localMark, peerMark, SyncOperation::PUSH);
    if (ackCode == SEND_FINISHED) {
        packet->SetLastSequence();
    }
    SetPacketId(packet, context, version);
    packet->SetData(inData);
    uint32_t packetLen = packet->CalculateLen();
    Message *message = new (std::nothrow) Message(DATA_SYNC_MESSAGE);
    if (message == nullptr) {
        LOGE("[DataSync][SendPullResponseDataPkt] new message error");
        delete packet;
        packet = nullptr;
        return -E_OUT_OF_MEMORY;
    }
    LOGI("[DataSync][SendPullResponseDataPkt] size=%d,code=%d,LastSequence=%d,label=%s,dev=%s{private}",
        size, ackCode, packet->IsLastSequence(), label_.c_str(), GetDeviceId().c_str());
    int errCode = message->SetExternalObject(packet);
    if (errCode != E_OK) {
        delete packet;
        packet = nullptr;
        delete message;
        message = nullptr;
        LOGE("[SendPullResponseDataPkt] set external object failed, errCode=%d", errCode);
        return errCode;
    }
    SetMessageHeadInfo(*message, TYPE_REQUEST, context->GetDeviceId(), context->GetSequenceId(),
        context->GetResponseSessionId());
    SendResetWatchDogPacket(context, packetLen);

    errCode = Send(context, message, nullptr, packetLen);
    if (errCode != E_OK) {
        delete message;
        message = nullptr;
    }
    return errCode;
}

void SingleVerDataSync::SetAckData(DataAckPacket &ackPacket, SingleVerSyncTaskContext *context, int32_t recvCode,
    WaterMark maxSendDataTime) const
{
    // send ack packet
    if ((recvCode == E_OK) && (maxSendDataTime != 0)) {
        ackPacket.SetData(maxSendDataTime + 1); // + 1 to next start
    } else if (recvCode != WATER_MARK_INVALID) {
        WaterMark mark;
        metadata_->GetPeerWaterMark(context->GetDeviceId(), mark);
        ackPacket.SetData(mark);
    }
}

int SingleVerDataSync::SendAck(SingleVerSyncTaskContext *context, const Message *message, int32_t recvCode,
    WaterMark maxSendDataTime)
{
    const DataRequestPacket *packet = message->GetObject<DataRequestPacket>();
    if (packet == nullptr) {
        return -E_INVALID_ARGS;
    }
    Message *ackMessage = new (std::nothrow) Message(DATA_SYNC_MESSAGE);
    if (ackMessage == nullptr) {
        LOGE("[DataSync][SendAck] new message error");
        return -E_OUT_OF_MEMORY;
    }
    DataAckPacket ack;
    ack.SetRecvCode(recvCode);
    SetAckData(ack, context, recvCode, maxSendDataTime);
    WaterMark localMark;
    metadata_->GetLocalWaterMark(context->GetDeviceId(), localMark);
    std::vector<uint64_t> reserved {localMark};
    uint32_t version = std::min(context->GetRemoteSoftwareVersion(), SOFTWARE_VERSION_CURRENT);
    uint64_t packetId = 0;
    if (version > SOFTWARE_VERSION_RELEASE_2_0) {
        packetId = packet->GetPacketId(); // above 102 version data request reserve[0] store packetId value
    }
    if (version > SOFTWARE_VERSION_RELEASE_2_0 && packetId > 0) {
        reserved.push_back(packetId);
    }
    ack.SetReserved(reserved);
    ack.SetVersion(version);
    int errCode = ackMessage->SetCopiedObject(ack);
    if (errCode != E_OK) {
        delete ackMessage;
        ackMessage = nullptr;
        LOGE("[DataSync][SendAck] set copied object failed, errcode=%d", errCode);
        return errCode;
    }
    SetMessageHeadInfo(*ackMessage, TYPE_RESPONSE, context->GetDeviceId(), message->GetSequenceId(),
        message->GetSessionId());

    errCode = Send(context, ackMessage, nullptr, 0);
    if (errCode != E_OK) {
        delete ackMessage;
        ackMessage = nullptr;
    }
    PerformanceAnalysis *performance = PerformanceAnalysis::GetInstance();
    if (performance != nullptr) {
        performance->StepTimeRecordEnd(PT_TEST_RECORDS::RECORD_DATA_REQUEST_RECV_TO_SEND_ACK);
    }
    return errCode;
}

int SingleVerDataSync::SendLocalWaterMarkAck(SingleVerSyncTaskContext *context, const Message *message)
{
    context->SetReceiveWaterMarkErr(true);
    const DataRequestPacket *packet = message->GetObject<DataRequestPacket>();
    if (packet == nullptr) {
        return -E_INVALID_ARGS;
    }
    Message *ackMessage = new (std::nothrow) Message(DATA_SYNC_MESSAGE);
    if (ackMessage == nullptr) {
        LOGE("[DataSync][LocalWaterMarkAck] new message error");
        return -E_OUT_OF_MEMORY;
    }

    DataAckPacket ack;
    WaterMark peerMark;
    metadata_->GetPeerWaterMark(context->GetDeviceId(), peerMark);
    uint32_t version = std::min(context->GetRemoteSoftwareVersion(), SOFTWARE_VERSION_CURRENT);
    ack.SetData(peerMark);
    WaterMark localMark;
    metadata_->GetLocalWaterMark(context->GetDeviceId(), localMark);
    uint64_t packetId = 0;
    if (version > SOFTWARE_VERSION_RELEASE_2_0) {
        packetId = packet->GetPacketId(); // above 102 version data request reserve[0] store packetId value
    }
    std::vector<uint64_t> reserved {localMark};
    if (version > SOFTWARE_VERSION_RELEASE_2_0 && packetId > 0) {
        reserved.push_back(packetId);
    }
    ack.SetReserved(reserved);
    ack.SetRecvCode(LOCAL_WATER_MARK_NOT_INIT);
    ack.SetVersion(version);
    int errCode = ackMessage->SetCopiedObject(ack);
    if (errCode != E_OK) {
        delete ackMessage;
        ackMessage = nullptr;
        LOGE("[DataSync][LocalWaterMarkAck] set copied object failed, errcode=%d", errCode);
        return errCode;
    }
    SetMessageHeadInfo(*ackMessage, TYPE_RESPONSE, context->GetDeviceId(), message->GetSequenceId(),
        message->GetSessionId());
    errCode = Send(context, ackMessage, nullptr, 0);
    if (errCode != E_OK) {
        delete ackMessage;
        ackMessage = nullptr;
    }
    LOGI("[DataSync][LocalWaterMarkAck] LOCAL_WATER_MARK_NOT_INIT peerMark=%llu,localMark=%llu", peerMark, localMark);
    return errCode;
}

int SingleVerDataSync::AckRecv(SingleVerSyncTaskContext *context, const Message *message)
{
    if (context == nullptr || message == nullptr) {
        return -E_INVALID_ARGS;
    }

    const DataAckPacket *packet = message->GetObject<DataAckPacket>();
    if (packet == nullptr) {
        return -E_INVALID_ARGS;
    }
    int32_t recvCode = packet->GetRecvCode();
    LOGD("[DataSync][AckRecv] ver=%u,recvCode=%d,myversion=%u,label=%s,dev=%s{private}", packet->GetVersion(),
        recvCode, SOFTWARE_VERSION_CURRENT, label_.c_str(), GetDeviceId().c_str());
    if (recvCode == -E_VERSION_NOT_SUPPORT) {
        LOGE("[DataSync][AckRecv] Version mismatch");
        return -E_VERSION_NOT_SUPPORT;
    }

    if (recvCode == -E_NEED_ABILITY_SYNC || recvCode == -E_NOT_PERMIT) {
        // after set sliding window sender err, we can ReleaseContinueToken, avoid crash
        context->SetSlidingWindowSenderErr(true);
        LOGI("[DataSync][AckRecv] Data sync abort,recvCode = %d,label = %s,dev = %s{private}", recvCode,
            label_.c_str(), GetDeviceId().c_str());
        context->ReleaseContinueToken();
        return recvCode;
    }
    uint64_t data = packet->GetData();
    if (recvCode == LOCAL_WATER_MARK_NOT_INIT) {
        return DealWaterMarkException(context, data, packet->GetReserved());
    }

    if (recvCode == -E_SAVE_DATA_NOTIFY && data != 0) {
        // data only use low 32bit
        context->StartFeedDogForSync(static_cast<uint32_t>(data), SyncDirectionFlag::RECEIVE);
        LOGI("[DataSync][AckRecv] notify ResetWatchDog=%llu,label=%s,dev=%s{private}", data, label_.c_str(),
            GetDeviceId().c_str());
    }

    if (recvCode != E_OK && recvCode != WATER_MARK_INVALID) {
        LOGW("[DataSync][AckRecv] Received a uncatched recvCode=%d,label=%s,dev=%s{private}", recvCode,
            label_.c_str(), GetDeviceId().c_str());
        return recvCode;
    }

    // Judge if send finished
    ContinueToken token;
    context->GetContinueToken(token);
    if (((message->GetSessionId() == context->GetResponseSessionId()) ||
        (message->GetSessionId() == context->GetRequestSessionId())) && (token == nullptr)) {
        return -E_NO_DATA_SEND;
    }

    // send next data
    return -E_SEND_DATA;
}

void SingleVerDataSync::SendSaveDataNotifyPacket(SingleVerSyncTaskContext *context, uint32_t pktVersion,
    uint32_t sessionId, uint32_t sequenceId)
{
    Message *ackMessage = new (std::nothrow) Message(DATA_SYNC_MESSAGE);
    if (ackMessage == nullptr) {
        LOGE("[DataSync][SaveDataNotify] new message failed");
        return;
    }

    DataAckPacket ack;
    ack.SetRecvCode(-E_SAVE_DATA_NOTIFY);
    ack.SetVersion(pktVersion);
    int errCode = ackMessage->SetCopiedObject(ack);
    if (errCode != E_OK) {
        delete ackMessage;
        ackMessage = nullptr;
        LOGE("[DataSync][SendSaveDataNotifyPacket] set copied object failed,errcode=%d", errCode);
        return;
    }
    SetMessageHeadInfo(*ackMessage, TYPE_NOTIFY, context->GetDeviceId(), sequenceId, sessionId);

    errCode = Send(context, ackMessage, nullptr, 0);
    if (errCode != E_OK) {
        delete ackMessage;
        ackMessage = nullptr;
    }
    LOGD("[DataSync][SaveDataNotify] Send SaveDataNotify packet Finished,errcode=%d,label=%s,dev=%s{private}",
        errCode, label_.c_str(), GetDeviceId().c_str());
}

void SingleVerDataSync::GetPullEndWatermark(const SingleVerSyncTaskContext *context, const DataRequestPacket *packet,
    WaterMark &pullEndWatermark) const
{
    if (packet == nullptr) {
        return;
    }
    if ((packet->GetMode() == SyncOperation::PULL) || (packet->GetMode() == SyncOperation::PUSH_AND_PULL)) {
        WaterMark endMark = packet->GetEndWaterMark();
        TimeOffset offset;
        metadata_->GetTimeOffset(context->GetDeviceId(), offset);
        pullEndWatermark = endMark - offset;
        LOGD("[DataSync][PullEndWatermark] packetEndMark=%llu,offset=%llu,endWaterMark=%llu,label=%s,dev=%s{private}",
            endMark, offset, pullEndWatermark, label_.c_str(), GetDeviceId().c_str());
    }
}

int SingleVerDataSync::DealWaterMarkException(SingleVerSyncTaskContext *context, WaterMark ackWaterMark,
    const std::vector<uint64_t> &reserved)
{
    // after set sliding window sender err, we can SaveLocalWaterMark, avoid sliding window sender re save a wrong
    // waterMark again.
    context->SetSlidingWindowSenderErr(true);
    WaterMark localMark;
    metadata_->GetLocalWaterMark(context->GetDeviceId(), localMark);
    LOGI("[DataSync][WaterMarkException] AckRecv LOCAL_WATER_MARK_NOT_INIT mark=%llu,label=%s,dev=%s{private}",
        ackWaterMark, label_.c_str(), GetDeviceId().c_str());
    int errCode = SaveLocalWaterMark(context->GetDeviceId(), ackWaterMark);
    if (errCode != E_OK) {
        return errCode;
    }
    context->SetRetryStatus(SyncTaskContext::NEED_RETRY);
    // for push_and_pull mode it may be EKEYREVOKED error before receive watermarkexception
    // should clear errCode and restart pushpull request.
    if (context->GetRemoteSoftwareVersion() > SOFTWARE_VERSION_RELEASE_2_0 &&
        context->GetMode() == SyncOperation::PUSH_AND_PULL && context->GetTaskErrCode() == -E_EKEYREVOKED) {
        context->SetTaskErrCode(E_OK);
    }

    if (reserved.empty()) {
        if (localMark != 0 && ackWaterMark == 0 && context->IsNeedClearRemoteStaleData()) {
            // need to clear remote historydata
            LOGI("[DataSync][WaterMarkException] AckRecv rebuilted,clear historydata,label=%s,dev=%s{private}",
                label_.c_str(), GetDeviceId().c_str());
            errCode = storage_->RemoveDeviceData(context->GetDeviceId(), true);
            if (errCode != E_OK) {
                return errCode;
            }
        }
    } else {
        WaterMark peerMark;
        metadata_->GetPeerWaterMark(context->GetDeviceId(), peerMark);
        if (reserved[ACK_PACKET_RESERVED_INDEX_LOCAL_WATER_MARK] == 0 && peerMark != 0 &&
            context->IsNeedClearRemoteStaleData()) {
            // need to clear remote historydata
            LOGI("[DataSync][WaterMarkException] AckRecv reserved not empty,rebuilted,clear historydata,label=%s,"
                "dev = %s{private}", label_.c_str(), GetDeviceId().c_str());
            errCode = storage_->RemoveDeviceData(context->GetDeviceId(), true);
            if (errCode != E_OK) {
                return errCode;
            }
        }
    }
    return -E_RE_SEND_DATA;
}

int SingleVerDataSync::RunPermissionCheck(SingleVerSyncTaskContext *context, const Message *message,
    const DataRequestPacket *packet)
{
    std::string appId = storage_->GetDbProperties().GetStringProp(KvDBProperties::APP_ID, "");
    std::string userId = storage_->GetDbProperties().GetStringProp(KvDBProperties::USER_ID, "");
    std::string storeId = storage_->GetDbProperties().GetStringProp(KvDBProperties::STORE_ID, "");
    uint8_t flag;
    int32_t mode = packet->GetMode();
    if (mode == SyncOperation::PUSH) {
        flag = CHECK_FLAG_RECEIVE;
    } else if (mode == SyncOperation::PULL) {
        flag = CHECK_FLAG_SEND;
    } else if (mode == SyncOperation::PUSH_AND_PULL) {
        flag = CHECK_FLAG_SEND | CHECK_FLAG_RECEIVE;
    } else {
        // before add permissionCheck, PushStart packet and pullResponse packet do not setMode.
        flag = CHECK_FLAG_RECEIVE;
    }
    int errCode = RuntimeContext::GetInstance()->RunPermissionCheck(userId, appId, storeId, context->GetDeviceId(),
        flag);
    if (errCode != E_OK) {
        LOGE("[DataSync][RunPermissionCheck] check failed flag=%d,Label=%s,dev=%s{private}", flag, label_.c_str(),
            GetDeviceId().c_str());
        if (context->GetRemoteSoftwareVersion() > SOFTWARE_VERSION_EARLIEST) { // ver 101 can't handle this errCode
            (void)SendAck(context, message, -E_NOT_PERMIT, 0);
        }
        return -E_NOT_PERMIT;
    }
    const std::vector<SendDataItem> &data = packet->GetData();
    WaterMark maxSendDataTime = GetMaxSendDataTime(data, false);
    uint32_t version = std::min(context->GetRemoteSoftwareVersion(), SOFTWARE_VERSION_CURRENT);
    if (version > SOFTWARE_VERSION_RELEASE_2_0 && (mode != SyncOperation::PULL) && !context->GetReceivcPermitCheck()) {
        bool permitReceive = CheckPermitReceiveData(context);
        if (permitReceive) {
            context->SetReceivcPermitCheck(true);
        } else {
            (void)SendAck(context, message, -E_SECURITY_OPTION_CHECK_ERROR, maxSendDataTime);
            return -E_SECURITY_OPTION_CHECK_ERROR;
        }
    }
    return errCode;
}

// used in pull response
void SingleVerDataSync::SendResetWatchDogPacket(SingleVerSyncTaskContext *context, uint32_t packetLen)
{
    if (mtuSize_ >= packetLen || mtuSize_ == 0) {
        return;
    }
    uint64_t data = static_cast<uint64_t>(packetLen) * static_cast<uint64_t>(DBConstant::AUTO_SYNC_TIMEOUT) / mtuSize_;

    Message *ackMessage = new (std::nothrow) Message(DATA_SYNC_MESSAGE);
    if (ackMessage == nullptr) {
        LOGE("[DataSync][ResetWatchDog] new message failed");
        return;
    }

    DataAckPacket ack;
    ack.SetData(data);
    ack.SetRecvCode(-E_SAVE_DATA_NOTIFY);
    ack.SetVersion(std::min(context->GetRemoteSoftwareVersion(), SOFTWARE_VERSION_CURRENT));
    int errCode = ackMessage->SetCopiedObject(ack);
    if (errCode != E_OK) {
        delete ackMessage;
        ackMessage = nullptr;
        LOGE("[DataSync][ResetWatchDog] set copied object failed, errcode=%d", errCode);
        return;
    }
    SetMessageHeadInfo(*ackMessage, TYPE_NOTIFY, context->GetDeviceId(), context->GetSequenceId(),
        context->GetResponseSessionId());

    errCode = Send(context, ackMessage, nullptr, 0);
    if (errCode != E_OK) {
        delete ackMessage;
        ackMessage = nullptr;
        LOGE("[DataSync][ResetWatchDog] Send packet failed,errcode=%d,label=%s,dev = %s{private}", errCode,
            label_.c_str(), GetDeviceId().c_str());
    } else {
        LOGI("[DataSync][ResetWatchDog] data = %llu,label=%s,dev=%s{private}", data, label_.c_str(),
            GetDeviceId().c_str());
    }
}

void SingleVerDataSync::SendAck(SingleVerSyncTaskContext *context, uint32_t sessionId, uint32_t sequenceId,
    uint64_t packetId)
{
    Message *ackMessage = new (std::nothrow) Message(DATA_SYNC_MESSAGE);
    if (ackMessage == nullptr) {
        LOGE("[DataSync][SendAck] new message error");
        return;
    }
    DataAckPacket ack;
    ack.SetRecvCode(E_OK);
    uint32_t version = std::min(context->GetRemoteSoftwareVersion(), SOFTWARE_VERSION_CURRENT);
    WaterMark localMark;
    metadata_->GetLocalWaterMark(context->GetDeviceId(), localMark);
    std::vector<uint64_t> reserved {localMark};
    if (version > SOFTWARE_VERSION_RELEASE_2_0 && packetId > 0) {
        reserved.push_back(packetId);
    }
    ack.SetReserved(reserved);
    ack.SetVersion(version);
    int errCode = ackMessage->SetCopiedObject(ack);
    if (errCode != E_OK) {
        delete ackMessage;
        ackMessage = nullptr;
        LOGE("[DataSync][SendAck] set copied object failed, errcode=%d", errCode);
        return;
    }
    SetMessageHeadInfo(*ackMessage, TYPE_RESPONSE, context->GetDeviceId(), sequenceId, sessionId);
    errCode = Send(context, ackMessage, nullptr, 0);
    if (errCode != E_OK) {
        delete ackMessage;
        ackMessage = nullptr;
    }
}

int32_t SingleVerDataSync::ReSend(SingleVerSyncTaskContext *context, DataSyncReSendInfo reSendInfo)
{
    if (context == nullptr) {
        return -E_INVALID_ARGS;
    }
    LOGI("[DataSync][ReSend] start=%llu,end=%llu,label=%s,dev=%s{private}", reSendInfo.start, reSendInfo.end,
        label_.c_str(), GetDeviceId().c_str());
    std::vector<SendDataItem> outData;
    ContinueToken token = nullptr;
    uint32_t version = std::min(context->GetRemoteSoftwareVersion(), SOFTWARE_VERSION_CURRENT);
    size_t packetSize = (version > SOFTWARE_VERSION_RELEASE_2_0) ?
        DBConstant::MAX_HPMODE_PACK_ITEM_SIZE : DBConstant::MAX_NORMAL_PACK_ITEM_SIZE;
    DataSizeSpecInfo reSendDataSizeInfo = {mtuSize_, packetSize};
    int errCode = storage_->GetSyncData(reSendInfo.start, reSendInfo.end + 1, outData, token, reSendDataSizeInfo);
    if (token != nullptr) {
        storage_->ReleaseContinueToken(token);
    }
    if (errCode == -E_BUSY || errCode == -E_EKEYREVOKED) {
        context->SetTaskErrCode(errCode);
        return errCode;
    }
    if (errCode != E_OK && errCode != -E_UNFINISHED) {
        return errCode;
    }
    WaterMark localMark;
    WaterMark peerMark;
    metadata_->GetPeerWaterMark(context->GetDeviceId(), peerMark);
    metadata_->GetLocalWaterMark(context->GetDeviceId(), localMark);
    DataRequestPacket *packet = new (std::nothrow) DataRequestPacket;
    if (packet == nullptr) {
        LOGE("[DataSync][ReSend] new DataRequestPacket error");
        return -E_OUT_OF_MEMORY;
    }
    if (context->GetSessionEndTimeStamp() == reSendInfo.end) {
        LOGI("[DataSync][ReSend] lastid,label=%s,dev=%s{private}", label_.c_str(), GetDeviceId().c_str());
        packet->SetLastSequence();
    }
    packet->SetData(outData);
    packet->SetBasicInfo(errCode, version, reSendInfo.start, peerMark, SyncOperation::PUSH);
    if (version > SOFTWARE_VERSION_RELEASE_2_0) {
        std::vector<uint64_t> reserved {reSendInfo.packetId};
        packet->SetReserved(reserved);
    }
    errCode = SendReSendPacket(packet, context, reSendInfo.sessionId, reSendInfo.sequenceId);
    if (errCode == E_OK && localMark < reSendInfo.end) {
        // resend.end may not update in localwatermark while E_TIMEOUT occurred in send message last time.
        SaveLocalWaterMark(context->GetDeviceId(), reSendInfo.end + 1);
    }
    return errCode;
}

int SingleVerDataSync::SendReSendPacket(const DataRequestPacket *packet, SingleVerSyncTaskContext *context,
    uint32_t sessionId, uint32_t sequenceId)
{
    Message *message = new (std::nothrow) Message(DATA_SYNC_MESSAGE);
    if (message == nullptr) {
        LOGE("[DataSync][SendDataPacket] new message error");
        delete packet;
        packet = nullptr;
        return -E_OUT_OF_MEMORY;
    }
    uint32_t packetLen = packet->CalculateLen();
    int errCode = message->SetExternalObject(packet);
    if (errCode != E_OK) {
        delete packet;
        packet = nullptr;
        delete message;
        message = nullptr;
        LOGE("[DataSync][SendReSendPacket] SetExternalObject failed errCode=%d", errCode);
        return errCode;
    }
    SetMessageHeadInfo(*message, TYPE_REQUEST, context->GetDeviceId(), sequenceId, sessionId);
    CommErrHandler handler = std::bind(&SyncTaskContext::CommErrHandlerFunc, std::placeholders::_1,
        context, message->GetSessionId());
    errCode = Send(context, message, handler, packetLen);
    if (errCode != E_OK) {
        delete message;
        message = nullptr;
    }
    return errCode;
}

bool SingleVerDataSync::IsPermitRemoteDeviceRecvData(const std::string &deviceId,
    const SecurityOption &remoteSecOption) const
{
    SecurityOption localSecOption;
    if (remoteSecOption.securityLabel == NOT_SURPPORT_SEC_CLASSIFICATION) {
        return true;
    }
    int errCode = storage_->GetSecurityOption(localSecOption);
    if (errCode == -E_NOT_SUPPORT) {
        return true;
    }
    return RuntimeContext::GetInstance()->CheckDeviceSecurityAbility(deviceId, localSecOption);
}

bool SingleVerDataSync::IsPermitLocalDeviceRecvData(const std::string &deviceId,
    const SecurityOption &remoteSecOption) const
{
    return RuntimeContext::GetInstance()->CheckDeviceSecurityAbility(deviceId, remoteSecOption);
}

int SingleVerDataSync::CheckPermitSendData(int mode, SingleVerSyncTaskContext *context)
{
    uint32_t version = std::min(context->GetRemoteSoftwareVersion(), SOFTWARE_VERSION_CURRENT);
    // for pull mode it just need to get data, no need to send data.
    if (version <= SOFTWARE_VERSION_RELEASE_2_0 || mode == SyncOperation::PULL) {
        return E_OK;
    }
    if (context->GetSendPermitCheck()) {
        return E_OK;
    }
    bool isPermitSync = true;
    std::string deviceId = context->GetDeviceId();
    SecurityOption remoteSecOption = context->GetRemoteSeccurityOption();
    if (mode == SyncOperation::PUSH || mode == SyncOperation::PUSH_AND_PULL || mode == SyncOperation::RESPONSE_PULL) {
        isPermitSync = IsPermitRemoteDeviceRecvData(deviceId, remoteSecOption);
    }
    LOGI("[DataSync][PermitSendData] mode=%d,dev=%s{private},label=%d,flag=%d,PermitSync=%d", mode, deviceId.c_str(),
        remoteSecOption.securityLabel, remoteSecOption.securityFlag, isPermitSync);
    if (isPermitSync) {
        context->SetSendPermitCheck(true);
        return E_OK;
    }
    if (mode == SyncOperation::PUSH || mode == SyncOperation::PUSH_AND_PULL) {
        context->SetTaskErrCode(-E_SECURITY_OPTION_CHECK_ERROR);
        return -E_SECURITY_OPTION_CHECK_ERROR;
    }
    if (mode == SyncOperation::RESPONSE_PULL) {
        std::vector<SendDataItem> outData;
        SendPullResponseDataPkt(-E_SECURITY_OPTION_CHECK_ERROR, outData, context);
        return -E_SECURITY_OPTION_CHECK_ERROR;
    }
    return E_OK;
}

std::string SingleVerDataSync::GetLabel() const
{
    return label_;
}

std::string SingleVerDataSync::GetDeviceId() const
{
    return deviceId_;
}

bool SingleVerDataSync::CheckPermitReceiveData(const SingleVerSyncTaskContext *context)
{
    SecurityOption remoteSecOption = context->GetRemoteSeccurityOption();
    std::string localDeviceId;
    if (communicateHandle_ == nullptr || remoteSecOption.securityLabel == NOT_SURPPORT_SEC_CLASSIFICATION) {
        return true;
    }
    communicateHandle_->GetLocalIdentity(localDeviceId);
    bool isPermitSync = IsPermitLocalDeviceRecvData(localDeviceId, remoteSecOption);
    if (isPermitSync) {
        return isPermitSync;
    }
    LOGE("[DataSync][PermitReceiveData] check failed: permitReceive=%d, localDev=%s{private}, seclabel=%d, secflag=%d",
        isPermitSync, localDeviceId.c_str(), remoteSecOption.securityLabel, remoteSecOption.securityFlag);
    return isPermitSync;
}

int SingleVerDataSync::CheckSchemaStrategy(SingleVerSyncTaskContext *context, const Message *message)
{
    SyncStrategy localStrategy = context->GetSyncStrategy();
    if (!context->GetIsSchemaSync()) {
        LOGE("[DataSync][CheckSchemaStrategy] isSchemaSync=%d check failed", context->GetIsSchemaSync());
        (void)SendAck(context, message, -E_NEED_ABILITY_SYNC, 0);
        return -E_NEED_ABILITY_SYNC;
    }
    if (!localStrategy.permitSync) {
        LOGE("[DataSync][CheckSchemaStrategy] Strategy permitSync=%d check failed", localStrategy.permitSync);
        (void)SendAck(context, message, -E_SCHEMA_MISMATCH, 0);
        return -E_SCHEMA_MISMATCH;
    }
    return E_OK;
}

void SingleVerDataSync::RemotePushFinished(int sendCode, int mode, uint32_t msgSessionId, uint32_t contextSessionId)
{
    if ((mode != SyncOperation::PUSH) && (mode != SyncOperation::PUSH_AND_PULL)) {
        return;
    }

    if ((sendCode == E_OK) && (msgSessionId != 0) && (msgSessionId != contextSessionId))  {
        storage_->NotifyRemotePushFinished(deviceId_);
    }
}

void SingleVerDataSync::SetPacketId(DataRequestPacket *packet, SingleVerSyncTaskContext *context, uint32_t version)
{
    if (version > SOFTWARE_VERSION_RELEASE_2_0) {
        context->IncPacketId(); // begin from 1
        std::vector<uint64_t> reserved {context->GetPacketId()};
        packet->SetReserved(reserved);
    }
}
} // namespace DistributedDB
