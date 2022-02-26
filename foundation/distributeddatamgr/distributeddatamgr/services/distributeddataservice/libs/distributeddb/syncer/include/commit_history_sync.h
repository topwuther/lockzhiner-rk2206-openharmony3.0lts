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

#ifndef COMMIT_HISTORY_SYNC_H
#define COMMIT_HISTORY_SYNC_H

#ifndef OMIT_MULTI_VER
#include <vector>
#include <map>

#include "multi_ver_kvdb_sync_interface.h"
#include "icommunicator.h"
#include "multi_ver_sync_task_context.h"
#include "sync_types.h"
#include "version.h"

namespace DistributedDB {
class CommitHistorySyncRequestPacket {
public:
    CommitHistorySyncRequestPacket() {};
    ~CommitHistorySyncRequestPacket() {};

    uint32_t CalculateLen() const;

    void SetCommitMap(std::map<std::string, MultiVerCommitNode> &inMap);

    void GetCommitMap(std::map<std::string, MultiVerCommitNode> &outMap) const;

    void SetVersion(uint32_t version);

    uint32_t GetVersion() const;

    void SetReserved(std::vector<uint64_t> &reserved);

    std::vector<uint64_t> GetReserved() const;

private:
    std::map<std::string, MultiVerCommitNode> commitMap_;
    uint32_t version_ = SOFTWARE_VERSION_CURRENT;
    std::vector<uint64_t> reserved_;
};

class CommitHistorySyncAckPacket {
public:
    CommitHistorySyncAckPacket() : errorCode_(0) {};
    ~CommitHistorySyncAckPacket() {};

    uint32_t CalculateLen() const;

    void SetData(std::vector<MultiVerCommitNode> &inData);

    void GetData(std::vector<MultiVerCommitNode> &outData) const;

    void SetErrorCode(int32_t errorCode);

    void GetErrorCode(int32_t &errorCode) const;

    void SetVersion(uint32_t version);

    uint32_t GetVersion() const;

    void SetReserved(std::vector<uint64_t> &reserved);

    std::vector<uint64_t> GetReserved() const;

private:
    int32_t errorCode_;
    uint32_t version_ = SOFTWARE_VERSION_CURRENT;
    std::vector<MultiVerCommitNode> commits_;
    std::vector<uint64_t> reserved_;
};

class CommitHistorySync {
public:
    CommitHistorySync() : storagePtr_(nullptr), communicateHandle_(nullptr) {};
    ~CommitHistorySync();
    DISABLE_COPY_ASSIGN_MOVE(CommitHistorySync);

    static int RegisterTransformFunc();

    int Initialize(MultiVerKvDBSyncInterface *storagePtr, ICommunicator *communicateHandle);

    static int Serialization(uint8_t *buffer, uint32_t length, const Message *inMsg);

    static int DeSerialization(const uint8_t *buffer, uint32_t length, Message *inMsg);

    static uint32_t CalculateLen(const Message *inMsg);

    void TimeOutCallback(MultiVerSyncTaskContext *context, const Message *message) const;

    int SyncStart(MultiVerSyncTaskContext *context);

    int RequestRecvCallback(const MultiVerSyncTaskContext *context, const Message *message);

    int AckRecvCallback(MultiVerSyncTaskContext *context, const Message *message);

private:
    static int RequestPacketCalculateLen(const Message *inMsg, uint32_t &len);

    static int RequestPacketSerialization(uint8_t *buffer, uint32_t length, const Message *inMsg);

    static int RequestPacketDeSerialization(const uint8_t *buffer, uint32_t length, Message *inMsg);

    static int AckPacketCalculateLen(const Message *inMsg, uint32_t &len);

    static int AckPacketSerialization(uint8_t *buffer, uint32_t length, const Message *inMsg);

    static int AckPacketDeSerialization(const uint8_t *buffer, uint32_t length, Message *inMsg);

    static bool IsPacketValid(const Message *inMsg, uint16_t messageType);

    int Send(const DeviceID &deviceId, const Message *inMsg);

    int GetDeviceLatestCommit(std::map<std::string, MultiVerCommitNode> &);

    int GetCommitTree(const std::map<std::string, MultiVerCommitNode> &, std::vector<MultiVerCommitNode> &);

    int SendRequestPacket(const MultiVerSyncTaskContext *context,
        std::map<std::string, MultiVerCommitNode> &commitMap);

    int SendAckPacket(const MultiVerSyncTaskContext *context, std::vector<MultiVerCommitNode> &commits,
        int ackCode, const Message *message);

    int GetLocalDeviceInfo(std::string &deviceInfo);

    int RunPermissionCheck(const std::string &deviceId) const;

    MultiVerKvDBSyncInterface *storagePtr_;
    ICommunicator *communicateHandle_;
};
}  // namespace DistributedDB

#endif
#endif