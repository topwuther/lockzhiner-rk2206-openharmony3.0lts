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

#ifndef MULTI_VER_DATA_SYNC_H
#define MULTI_VER_DATA_SYNC_H

#ifndef OMIT_MULTI_VER
#include <vector>

#include "multi_ver_kvdb_sync_interface.h"
#include "icommunicator.h"
#include "sync_task_context.h"
#include "multi_ver_sync_task_context.h"

namespace DistributedDB {
class MultiVerRequestPacket {
public:
    MultiVerRequestPacket() : errCode_(E_OK) {};
    ~MultiVerRequestPacket() {};

    uint32_t CalculateLen() const;

    void SetCommit(MultiVerCommitNode &commit);

    void GetCommit(MultiVerCommitNode &commit) const;

    void SetErrCode(int32_t errCode);

    int32_t GetErrCode() const;
private:
    MultiVerCommitNode commit_;
    int32_t errCode_ = E_OK;
};

class MultiVerAckPacket {
public:
    MultiVerAckPacket() : errorCode_(0) {};
    ~MultiVerAckPacket() {};

    uint32_t CalculateLen() const;

    void SetData(std::vector<std::vector<uint8_t>> &data);

    void GetData(std::vector<std::vector<uint8_t>> &data) const;

    void SetErrorCode(int32_t errCode);

    void GetErrorCode(int32_t &errorCode) const;
private:
    std::vector<std::vector<uint8_t>> entries_;
    int32_t errorCode_;
};

class MultiVerDataSync {
public:
    MultiVerDataSync() : storagePtr_(nullptr), communicateHandle_(nullptr) {};
    ~MultiVerDataSync();
    DISABLE_COPY_ASSIGN_MOVE(MultiVerDataSync);

    static int RegisterTransformFunc();

    int Initialize(MultiVerKvDBSyncInterface *storagePtr, ICommunicator *communicateHandle);

    static int Serialization(uint8_t *buffer, uint32_t length, const Message *inMsg);

    static int DeSerialization(const uint8_t *buffer, uint32_t length, Message *inMsg);

    static uint32_t CalculateLen(const Message *inMsg);

    void TimeOutCallback(MultiVerSyncTaskContext *context, const Message *message) const;

    int SyncStart(MultiVerSyncTaskContext *context);

    int RequestRecvCallback(const MultiVerSyncTaskContext *context, const Message *message);

    int AckRecvCallback(MultiVerSyncTaskContext *context, const Message *message);

    int PutCommitData(const MultiVerCommitNode &commit, const std::vector<MultiVerKvEntry *> &entries,
        const std::string &deviceName);

    int MergeSyncCommit(const MultiVerCommitNode &commit, const std::vector<MultiVerCommitNode> &commits);

    void ReleaseKvEntry(const MultiVerKvEntry *entry);

    void SendFinishedRequest(const MultiVerSyncTaskContext *context);

private:
    static int RequestPacketCalculateLen(const Message *inMsg, uint32_t &len);

    static int RequestPacketSerialization(uint8_t *buffer, uint32_t length, const Message *inMsg);

    static int RequestPacketDeSerialization(const uint8_t *buffer, uint32_t length, Message *inMsg);

    static int AckPacketCalculateLen(const Message *inMsg, uint32_t &len);

    static int AckPacketSerialization(uint8_t *buffer, uint32_t length, const Message *inMsg);

    static int AckPacketDeSerialization(const uint8_t *buffer, uint32_t length, Message *inMsg);

    static bool IsPacketValid(const Message *inMsg, uint16_t messageType);

    int GetValidCommit(MultiVerSyncTaskContext *context, MultiVerCommitNode &commit);

    bool IsCommitExisted(const MultiVerCommitNode &);

    int Send(const DeviceID &deviceId, const Message *inMsg);

    int SendRequestPacket(const MultiVerSyncTaskContext *context, MultiVerCommitNode &commit);

    int SendAckPacket(const MultiVerSyncTaskContext *context, const std::vector<MultiVerKvEntry *> &dataItems,
        int retCode, const Message *message);

    int GetCommitData(const MultiVerCommitNode &commit, std::vector<MultiVerKvEntry *> &entries);

    MultiVerKvEntry *CreateKvEntry(const std::vector<uint8_t> &entry);

    MultiVerKvDBSyncInterface *storagePtr_;
    ICommunicator *communicateHandle_;
};
}

#endif
#endif