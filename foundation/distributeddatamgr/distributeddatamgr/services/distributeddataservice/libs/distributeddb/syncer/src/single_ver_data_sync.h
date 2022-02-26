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

#ifndef SINGLE_VER_DATA_SYNC_NEW_H
#define SINGLE_VER_DATA_SYNC_NEW_H

#include "icommunicator.h"
#include "meta_data.h"
#include "single_ver_kvdb_sync_interface.h"
#include "single_ver_sync_task_context.h"
#include "sync_types.h"
#include "version.h"
#include "parcel.h"

namespace DistributedDB {
using SendDataItem = SingleVerKvEntry *;

struct DataSyncReSendInfo {
    uint32_t sessionId = 0;
    uint32_t sequenceId = 0;
    TimeStamp start = 0; // means localwatermark
    TimeStamp end = 0;
    uint64_t packetId = 0;
};

class DataRequestPacket {
public:
    DataRequestPacket() {};
    ~DataRequestPacket();

    void SetData(std::vector<SendDataItem> &data);

    const std::vector<SendDataItem> &GetData() const;

    void SetEndWaterMark(WaterMark waterMark);

    WaterMark GetEndWaterMark() const;

    void SetLocalWaterMark(WaterMark waterMark);

    WaterMark GetLocalWaterMark() const;

    void SetPeerWaterMark(WaterMark waterMark);

    WaterMark GetPeerWaterMark() const;

    void SetSendCode(int32_t errCode);

    int32_t GetSendCode() const;

    void SetMode(int32_t mode);

    int32_t GetMode() const;

    void SetSessionId(uint32_t sessionId);

    uint32_t GetSessionId() const;

    void SetVersion(uint32_t version);

    uint32_t GetVersion() const;

    uint32_t CalculateLen() const;

    void SetReserved(std::vector<uint64_t> &reserved);

    std::vector<uint64_t> GetReserved() const;

    uint64_t GetPacketId() const;

    void SetFlag(uint32_t flag);

    uint32_t GetFlag() const;

    bool IsLastSequence() const;

    void SetLastSequence();

    void SetBasicInfo(int sendCode, uint32_t version, WaterMark localMark, WaterMark peerMark, int32_t mode);

private:
    std::vector<SendDataItem> data_;
    WaterMark endWaterMark_ = 0;
    WaterMark localWaterMark_ = 0;
    WaterMark peerWaterMark_ = 0;
    int32_t sendCode_ = 0;
    int32_t mode_ = SyncOperation::INVALID;
    uint32_t sessionId_ = 0;
    uint32_t version_ = SOFTWARE_VERSION_CURRENT;
    std::vector<uint64_t> reserved_;
    uint32_t flag_ = 0; // bit 0 used for isLastSequence
    static const uint32_t IS_LAST_SEQUENCE = 0x1; // bit 0 used for isLastSequence, 1: is last, 0: not last
};

class DataAckPacket {
public:
    DataAckPacket() {};
    ~DataAckPacket() {};

    void SetData(uint64_t data);

    uint64_t GetData() const;

    void SetRecvCode(int32_t errorCode);

    int32_t GetRecvCode() const;

    void SetVersion(uint32_t version);

    uint32_t GetVersion() const;

    void SetReserved(std::vector<uint64_t> &reserved);

    std::vector<uint64_t> GetReserved() const;

    uint64_t GetPacketId() const;

    static bool IsPacketIdValid(uint64_t packetId);

    uint32_t CalculateLen() const;

private:
    /*
     * data_ is waterMark when revCode_ == LOCAL_WATER_MARK_NOT_INIT || revCode_ == E_OK;
     * data_ is timer in milliSeconds when revCode_ == -E_SAVE_DATA_NOTIFY && data_ != 0.
     */
    uint64_t data_ = 0;
    int32_t recvCode_ = 0;
    uint32_t version_ = SOFTWARE_VERSION_CURRENT;
    std::vector<uint64_t> reserved_;
};

class SingleVerDataSync {
public:
    SingleVerDataSync();
    ~SingleVerDataSync();

    DISABLE_COPY_ASSIGN_MOVE(SingleVerDataSync);

    static int Serialization(uint8_t *buffer, uint32_t length, const Message *inMsg);

    static int DeSerialization(const uint8_t *buffer, uint32_t length, Message *inMsg);

    static uint32_t CalculateLen(const Message *inMsg);

    static int RegisterTransformFunc();

    int Initialize(IKvDBSyncInterface *inStorage, ICommunicator *inCommunicateHandle,
        std::shared_ptr<Metadata> &inMetadata, const std::string &deviceId);

    int PushStart(SingleVerSyncTaskContext *context);

    int PushPullStart(SingleVerSyncTaskContext *context);

    int PullRequestStart(SingleVerSyncTaskContext *context);

    int PullResponseStart(SingleVerSyncTaskContext *context);

    int RequestRecv(SingleVerSyncTaskContext *context, const Message *message, WaterMark &pullEndWatermark);

    int AckRecv(SingleVerSyncTaskContext *context, const Message *message);

    void SendSaveDataNotifyPacket(SingleVerSyncTaskContext *context, uint32_t pktVersion, uint32_t sessionId,
        uint32_t sequenceId);

    void SendAck(SingleVerSyncTaskContext *context, uint32_t sessionId, uint32_t sequenceId, uint64_t packetId);

    int32_t ReSend(SingleVerSyncTaskContext *context, DataSyncReSendInfo reSendInfo);

    int CheckPermitSendData(int mode, SingleVerSyncTaskContext *context);

    std::string GetLabel() const;

    std::string GetDeviceId() const;

private:
    static const int SEND_FINISHED = 0xff;
    static const int LOCAL_WATER_MARK_NOT_INIT = 0xaa;
    static const int PEER_WATER_MARK_NOT_INIT = 0x55;
    static const int WATER_MARK_INVALID = 0xbb;
    static const int MTU_SIZE = 28311552; // 27MB

    static int AckPacketCalculateLen(const Message *inMsg, uint32_t &len);

    static int DataPacketCalculateLen(const Message *inMsg, uint32_t &len);

    static int DataPacketSerialization(uint8_t *buffer, uint32_t length, const Message *inMsg);

    static int DataPacketDeSerialization(const uint8_t *buffer, uint32_t length, Message *inMsg);

    static int AckPacketSerialization(uint8_t *buffer, uint32_t length, const Message *inMsg);

    static int AckPacketDeSerialization(const uint8_t *buffer, uint32_t length, Message *inMsg);

    static bool IsPacketValid(const Message *inMsg);

    static TimeStamp GetMaxSendDataTime(const std::vector<SendDataItem> &inData, bool isNeedInit,
        WaterMark localMark = 0);

    static TimeStamp GetMinSendDataTime(const std::vector<SendDataItem> &inData, WaterMark localMark);

    int GetData(SingleVerSyncTaskContext *context, std::vector<SendDataItem> &outData, size_t packetSize);

    int GetDataWithRerformanceRecord(SingleVerSyncTaskContext *context, std::vector<SendDataItem> &outData);

    int Send(SingleVerSyncTaskContext *context, const Message *message, const CommErrHandler &handler,
        uint32_t packetLen);

    int GetUnsyncData(SingleVerSyncTaskContext *context, std::vector<SendDataItem> &outData, size_t packetSize);

    int GetNextUnsyncData(SingleVerSyncTaskContext *context, std::vector<SendDataItem> &outData, size_t packetSize);

    int SaveData(const SingleVerSyncTaskContext *context, const std::vector<SendDataItem> &inData);

    int SaveLocalWaterMark(const DeviceID &deviceId, WaterMark waterMark);

    int SavePeerWaterMark(const DeviceID &deviceId, WaterMark waterMark);

    int RemoveDeviceData(SingleVerSyncTaskContext *context, const Message *message, WaterMark maxSendDataTime);

    void TransSendDataItemToLocal(const SingleVerSyncTaskContext *context,
        const std::vector<SendDataItem> &data);

    void TransDbDataItemToSendDataItem(const SingleVerSyncTaskContext *context,
        std::vector<SendDataItem> &outData);

    int SendDataPacket(const DataRequestPacket *packet, SingleVerSyncTaskContext *context);

    void SetAckData(DataAckPacket &ackPacket,  SingleVerSyncTaskContext *context, int32_t recvCode,
        WaterMark maxSendDataTime) const;

    int SendAck(SingleVerSyncTaskContext *context, const Message *message, int32_t recvCode,
        WaterMark maxSendDataTime);

    int SendLocalWaterMarkAck(SingleVerSyncTaskContext *context, const Message *message);

    void UpdatePeerWaterMark(const SingleVerSyncTaskContext *context, WaterMark peerWatermark);

    std::string GetLocalDeviceName();

    std::string TransferForeignOrigDevName(const std::string &deviceName);

    std::string TransferLocalOrigDevName(const std::string &origName);

    int RequestRecvPre(SingleVerSyncTaskContext *context, const Message *message);

    void GetPullEndWatermark(const SingleVerSyncTaskContext *context, const DataRequestPacket *packet,
        WaterMark &pullEndWatermark) const;

    int DealWaterMarkException(SingleVerSyncTaskContext *context, WaterMark ackWaterMark,
        const std::vector<uint64_t> &reserved);

    static int DataPacketSyncerPartSerialization(Parcel &parcel, const DataRequestPacket *packet);

    static int DataPacketSyncerPartDeSerialization(Parcel &parcel, DataRequestPacket *packet, uint32_t packLen,
        uint32_t length, uint32_t version);

    static int AckPacketSyncerPartSerializationV1(Parcel &parcel, const DataAckPacket *packet);

    static int AckPacketSyncerPartDeSerializationV1(Parcel &parcel, DataAckPacket &packet);

    int RunPermissionCheck(SingleVerSyncTaskContext *context, const Message *message,
        const DataRequestPacket *packet);

    void SendResetWatchDogPacket(SingleVerSyncTaskContext *context, uint32_t packetLen);

    int SendReSendPacket(const DataRequestPacket *packet, SingleVerSyncTaskContext *context,
        uint32_t sessionId, uint32_t sequenceId);

    int SendPullResponseDataPkt(int ackCode, std::vector<SendDataItem> &inData,
        SingleVerSyncTaskContext *context);

    void SetPacketId(DataRequestPacket *packet, SingleVerSyncTaskContext *context, uint32_t version);

    bool IsPermitRemoteDeviceRecvData(const std::string &deviceId, const SecurityOption &secOption) const;

    bool IsPermitLocalDeviceRecvData(const std::string &deviceId, const SecurityOption &remoteSecOption) const;

    bool CheckPermitReceiveData(const SingleVerSyncTaskContext *context);

    int CheckSchemaStrategy(SingleVerSyncTaskContext *context, const Message *message);

    void RemotePushFinished(int sendCode, int mode, uint32_t msgSessionId, uint32_t contetSessionId);

    uint32_t mtuSize_;
    SingleVerKvDBSyncInterface* storage_;
    ICommunicator* communicateHandle_;
    std::shared_ptr<Metadata> metadata_;
    std::string label_;
    std::string deviceId_;
};
}  // namespace DistributedDB

#endif // SINGLE_VER_DATA_SYNC_NEW_H

