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

#ifndef VALUE_SLICE_SYNC_H
#define VALUE_SLICE_SYNC_H

#ifndef OMIT_MULTI_VER
#include <vector>

#include "multi_ver_kvdb_sync_interface.h"
#include "icommunicator.h"
#include "multi_ver_sync_task_context.h"

namespace DistributedDB {
class ValueSliceHashPacket {
public:
    ValueSliceHashPacket() : errCode_(E_OK) {};
    ~ValueSliceHashPacket() {};

    uint32_t CalculateLen() const;

    void SetValueSliceHash(ValueSliceHash &hash);

    void GetValueSliceHash(ValueSliceHash &hash) const;

    void SetErrCode(int32_t errCode);

    int32_t GetErrCode() const;
private:
    ValueSliceHash valueSliceHash_;
    int32_t errCode_;
};

class ValueSlicePacket {
public:
    ValueSlicePacket() : errorCode_(0) {};
    ~ValueSlicePacket() {};

    uint32_t CalculateLen() const;

    void SetData(const ValueSlice &data);

    void GetData(ValueSlice &data) const;

    void SetErrorCode(int32_t errCode);

    void GetErrorCode(int32_t &errCode) const;
private:
    ValueSlice valueSlice_;
    int32_t errorCode_;
};

class ValueSliceSync {
public:
    ValueSliceSync() : storagePtr_(nullptr), communicateHandle_(nullptr) {};
    ~ValueSliceSync();
    DISABLE_COPY_ASSIGN_MOVE(ValueSliceSync);

    static int RegisterTransformFunc();

    int Initialize(MultiVerKvDBSyncInterface *storagePtr, ICommunicator *communicateHandle);

    static int Serialization(uint8_t *buffer, uint32_t length, const Message *inMsg);

    static int DeSerialization(const uint8_t *buffer, uint32_t length, Message *inMsg);

    static uint32_t CalculateLen(const Message *inMsg);

    int SyncStart(MultiVerSyncTaskContext *context);

    int RequestRecvCallback(const MultiVerSyncTaskContext *context, const Message *message);

    int AckRecvCallback(const MultiVerSyncTaskContext *context, const Message *message);

    void SendFinishedRequest(const MultiVerSyncTaskContext *context);

private:
    static int RequestPacketCalculateLen(const Message *inMsg, uint32_t &len);

    static int RequestPacketSerialization(uint8_t *buffer, uint32_t length, const Message *inMsg);

    static int RequestPacketDeSerialization(const uint8_t *buffer, uint32_t length, Message *inMsg);

    static int AckPacketCalculateLen(const Message *inMsg, uint32_t &len);

    static int AckPacketSerialization(uint8_t *buffer, uint32_t length, const Message *inMsg);

    static int AckPacketDeSerialization(const uint8_t *buffer, uint32_t length, Message *inMsg);

    static bool IsPacketValid(const Message *inMsg, uint16_t messageType);

    int GetValidValueSliceHashNode(MultiVerSyncTaskContext *context, ValueSliceHash &valueHashNode);

    int Send(const DeviceID &deviceId, const Message *inMsg);

    int SendRequestPacket(const MultiVerSyncTaskContext *context, ValueSliceHash &valueSliceHash);

    int SendAckPacket(const MultiVerSyncTaskContext *context, const ValueSlice &value, int ackCode,
        const Message *message);

    bool IsValueSliceExisted(const ValueSliceHash &value);

    int GetValueSlice(const ValueSliceHash &hashValue, ValueSlice &sliceValue);

    int PutValueSlice(const ValueSliceHash &hashValue, const ValueSlice &sliceValue);

    static const int MAX_VALUE_NODE_SIZE;
    MultiVerKvDBSyncInterface *storagePtr_;
    ICommunicator *communicateHandle_;
};
}

#endif
#endif