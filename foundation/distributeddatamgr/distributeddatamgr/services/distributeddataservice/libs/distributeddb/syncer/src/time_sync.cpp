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

#include "time_sync.h"

#include "parcel.h"
#include "log_print.h"
#include "sync_types.h"
#include "message_transform.h"
#include "version.h"
#include "isync_task_context.h"

namespace DistributedDB {
std::mutex TimeSync::timeSyncSetLock_;
std::set<TimeSync *> TimeSync::timeSyncSet_;
namespace {
    constexpr uint64_t TIME_SYNC_INTERVAL = 24 * 60 * 60 * 1000; // 24h
    constexpr int TRIP_DIV_HALF = 2;
    constexpr int64_t MAX_TIME_OFFSET_NOISE = 1 * 1000 * 10000; // 1s for 100ns
    constexpr int TIME_SYNC_WAIT_TIME = 5; // 5s
}

// Class TimeSyncPacket
TimeSyncPacket::TimeSyncPacket()
    : sourceTimeBegin_(0),
      sourceTimeEnd_(0),
      targetTimeBegin_(0),
      targetTimeEnd_(0),
      version_(TIME_SYNC_VERSION_V1)
{
}

TimeSyncPacket::~TimeSyncPacket()
{
}

void TimeSyncPacket::SetSourceTimeBegin(TimeStamp sourceTimeBegin)
{
    sourceTimeBegin_ = sourceTimeBegin;
}

TimeStamp TimeSyncPacket::GetSourceTimeBegin() const
{
    return sourceTimeBegin_;
}

void TimeSyncPacket::SetSourceTimeEnd(TimeStamp sourceTimeEnd)
{
    sourceTimeEnd_ = sourceTimeEnd;
}

TimeStamp TimeSyncPacket::GetSourceTimeEnd() const
{
    return sourceTimeEnd_;
}

void TimeSyncPacket::SetTargetTimeBegin(TimeStamp targetTimeBegin)
{
    targetTimeBegin_ = targetTimeBegin;
}

TimeStamp TimeSyncPacket::GetTargetTimeBegin() const
{
    return targetTimeBegin_;
}

void TimeSyncPacket::SetTargetTimeEnd(TimeStamp targetTimeEnd)
{
    targetTimeEnd_ = targetTimeEnd;
}

TimeStamp TimeSyncPacket::GetTargetTimeEnd() const
{
    return targetTimeEnd_;
}

void TimeSyncPacket::SetVersion(uint32_t version)
{
    version_ = version;
}

uint32_t TimeSyncPacket::GetVersion() const
{
    return version_;
}

uint32_t TimeSyncPacket::CalculateLen()
{
    uint32_t len = Parcel::GetUInt32Len();
    len += Parcel::GetUInt64Len();
    len += Parcel::GetUInt64Len();
    len += Parcel::GetUInt64Len();
    len += Parcel::GetUInt64Len();
    len = Parcel::GetEightByteAlign(len);
    return len;
}

// Class TimeSync
TimeSync::TimeSync()
    : communicateHandle_(nullptr),
      metadata_(nullptr),
      timeHelper_(nullptr),
      retryTime_(0),
      driverTimerId_(0),
      isSynced_(false),
      isAckReceived_(false),
      timeChangedListener_(nullptr),
      timeDriverLockCount_(0),
      isOnline_(true)
{
}

TimeSync::~TimeSync()
{
    Finalize();
    driverTimerId_ = 0;

    if (timeChangedListener_ != nullptr) {
        timeChangedListener_->Drop(true);
        timeChangedListener_ = nullptr;
    }
    timeHelper_ = nullptr;
    communicateHandle_ = nullptr;
    metadata_ = nullptr;

    std::lock_guard<std::mutex> lock(timeSyncSetLock_);
    timeSyncSet_.erase(this);
}

int TimeSync::RegisterTransformFunc()
{
    TransformFunc func;
    func.computeFunc = std::bind(&TimeSync::CalculateLen, std::placeholders::_1);
    func.serializeFunc = std::bind(&TimeSync::Serialization, std::placeholders::_1,
                                   std::placeholders::_2, std::placeholders::_3);
    func.deserializeFunc = std::bind(&TimeSync::DeSerialization, std::placeholders::_1,
                                     std::placeholders::_2, std::placeholders::_3);
    return MessageTransform::RegTransformFunction(TIME_SYNC_MESSAGE, func);
}

int TimeSync::Initialize(ICommunicator *communicator, std::shared_ptr<Metadata> &metadata,
    const IKvDBSyncInterface *storage, const DeviceID &deviceId)
{
    if ((communicator == nullptr) || (storage == nullptr) || (metadata == nullptr)) {
        return -E_INVALID_ARGS;
    }
    {
        std::lock_guard<std::mutex> lock(timeSyncSetLock_);
        timeSyncSet_.insert(this);
    }
    communicateHandle_ = communicator;
    metadata_ = metadata;
    deviceId_ = deviceId;
    timeHelper_ = std::make_unique<TimeHelper>();

    int errCode = timeHelper_->Initialize(storage, metadata_);
    if (errCode != E_OK) {
        timeHelper_ = nullptr;
        LOGE("[TimeSync] timeHelper Init failed, err %d.", errCode);
        return errCode;
    }

    driverCallback_ = std::bind(&TimeSync::TimeSyncDriver, this, std::placeholders::_1);
    errCode = RuntimeContext::GetInstance()->SetTimer(TIME_SYNC_INTERVAL, driverCallback_, nullptr, driverTimerId_);
    if (errCode != E_OK) {
        return errCode;
    }
    return errCode;
}

void TimeSync::Finalize()
{
    // Stop the timer
    LOGD("[TimeSync] Finalize enter!");
    RuntimeContext *runtimeContext = RuntimeContext::GetInstance();
    std::unique_lock<std::mutex> lock(timeDriverLock_);
    runtimeContext->RemoveTimer(driverTimerId_, true);
    timeDriverCond_.wait(lock, [this](){ return this->timeDriverLockCount_ == 0; });
    LOGD("[TimeSync] Finalized!");
}

int TimeSync::SyncStart(const CommErrHandler &handler)
{
    isOnline_ = true;
    TimeSyncPacket packet;
    TimeStamp startTime = timeHelper_->GetTime();
    packet.SetSourceTimeBegin(startTime);
    // send timeSync request
    LOGD("[TimeSync] startTime = %llu, dev = %s{private}", startTime, deviceId_.c_str());

    Message *message = new (std::nothrow) Message(TIME_SYNC_MESSAGE);
    if (message == nullptr) {
        return -E_OUT_OF_MEMORY;
    }

    message->SetMessageType(TYPE_REQUEST);
    message->SetPriority(Priority::NORMAL);
    int errCode = message->SetCopiedObject<>(packet);
    if (errCode != E_OK) {
        delete message;
        message = nullptr;
        return errCode;
    }

    errCode = SendPacket(deviceId_, message, handler);
    if (errCode != E_OK) {
        delete message;
        message = nullptr;
    }
    return errCode;
}

uint32_t TimeSync::CalculateLen(const Message *inMsg)
{
    if (!(IsPacketValid(inMsg, TYPE_RESPONSE) || IsPacketValid(inMsg, TYPE_REQUEST))) {
        return 0;
    }

    const TimeSyncPacket *packet = const_cast<TimeSyncPacket *>(inMsg->GetObject<TimeSyncPacket>());
    if (packet == nullptr) {
        return 0;
    }

    return TimeSyncPacket::CalculateLen();
}

int TimeSync::Serialization(uint8_t *buffer, uint32_t length, const Message *inMsg)
{
    if ((buffer == nullptr) || !(IsPacketValid(inMsg, TYPE_RESPONSE) || IsPacketValid(inMsg, TYPE_REQUEST))) {
        return -E_INVALID_ARGS;
    }
    const TimeSyncPacket *packet = inMsg->GetObject<TimeSyncPacket>();
    if ((packet == nullptr) || (length != TimeSyncPacket::CalculateLen())) {
        return -E_INVALID_ARGS;
    }

    Parcel parcel(buffer, length);
    TimeStamp srcBegin = packet->GetSourceTimeBegin();
    TimeStamp srcEnd = packet->GetSourceTimeEnd();
    TimeStamp targetBegin = packet->GetTargetTimeBegin();
    TimeStamp targetEnd = packet->GetTargetTimeEnd();

    int errCode = parcel.WriteUInt32(TIME_SYNC_VERSION_V1);
    if (errCode != E_OK) {
        return -E_SECUREC_ERROR;
    }
    errCode = parcel.WriteUInt64(srcBegin);
    if (errCode != E_OK) {
        return -E_SECUREC_ERROR;
    }
    errCode = parcel.WriteUInt64(srcEnd);
    if (errCode != E_OK) {
        return -E_SECUREC_ERROR;
    }
    errCode = parcel.WriteUInt64(targetBegin);
    if (errCode != E_OK) {
        return -E_SECUREC_ERROR;
    }
    errCode = parcel.WriteUInt64(targetEnd);
    if (errCode != E_OK) {
        return -E_SECUREC_ERROR;
    }
    parcel.EightByteAlign();

    return errCode;
}

int TimeSync::DeSerialization(const uint8_t *buffer, uint32_t length, Message *inMsg)
{
    if ((buffer == nullptr) || !(IsPacketValid(inMsg, TYPE_RESPONSE) || IsPacketValid(inMsg, TYPE_REQUEST))) {
        return -E_INVALID_ARGS;
    }
    TimeSyncPacket packet;
    Parcel parcel(const_cast<uint8_t *>(buffer), length);
    TimeStamp srcBegin;
    TimeStamp srcEnd;
    TimeStamp targetBegin;
    TimeStamp targetEnd;

    uint32_t version = 0;
    parcel.ReadUInt32(version);
    if (parcel.IsError()) {
        return -E_INVALID_ARGS;
    }
    if (version > TIME_SYNC_VERSION_V1) {
        packet.SetVersion(version);
        return inMsg->SetCopiedObject<>(packet);
    }
    parcel.ReadUInt64(srcBegin);
    parcel.ReadUInt64(srcEnd);
    parcel.ReadUInt64(targetBegin);
    parcel.ReadUInt64(targetEnd);
    if (parcel.IsError()) {
        return -E_INVALID_ARGS;
    }
    packet.SetSourceTimeBegin(srcBegin);
    packet.SetSourceTimeEnd(srcEnd);
    packet.SetTargetTimeBegin(targetBegin);
    packet.SetTargetTimeEnd(targetEnd);

    return inMsg->SetCopiedObject<>(packet);
}

int TimeSync::AckRecv(const Message *message)
{
    if (!IsPacketValid(message, TYPE_RESPONSE)) {
        return -E_INVALID_ARGS;
    }

    const TimeSyncPacket *packet = message->GetObject<TimeSyncPacket>();
    if (packet == nullptr) {
        LOGE("[TimeSync] AckRecv packet is null");
        return -E_INVALID_ARGS;
    }

    TimeSyncPacket packetData = TimeSyncPacket(*packet);
    TimeStamp sourceTimeEnd = timeHelper_->GetTime();
    packetData.SetSourceTimeEnd(sourceTimeEnd);
    if (packetData.GetSourceTimeBegin() > packetData.GetSourceTimeEnd() ||
        packetData.GetTargetTimeBegin() > packetData.GetTargetTimeEnd() ||
        packetData.GetSourceTimeEnd() > TimeHelper::MAX_VALID_TIME ||
        packetData.GetTargetTimeEnd() > TimeHelper::MAX_VALID_TIME) {
        LOGD("[TimeSync][AckRecv] Time valid check failed.");
        return -E_INVALID_TIME;
    }
    // calculate timeoffset of two devices
    TimeOffset offset = CalculateTimeOffset(packetData);
    LOGD("TimeSync::AckRecv, dev = %s{private}, sEnd = %llu, tEnd = %llu, sBegin = %llu, tBegin = %llu, offset = %lld",
        deviceId_.c_str(),
        packetData.GetSourceTimeEnd(),
        packetData.GetTargetTimeEnd(),
        packetData.GetSourceTimeBegin(),
        packetData.GetTargetTimeBegin(),
        offset);

    // save timeoffset into metadata, maybe a block action
    int errCode = SaveTimeOffset(deviceId_, offset);
    isSynced_ = true;
    {
        std::lock_guard<std::mutex> lock(cvLock_);
        isAckReceived_ = true;
    }
    conditionVar_.notify_all();
    ResetTimer();
    return errCode;
}

int TimeSync::RequestRecv(const Message *message)
{
    if (!IsPacketValid(message, TYPE_REQUEST)) {
        return -E_INVALID_ARGS;
    }
    TimeStamp targetTimeBegin = timeHelper_->GetTime();

    const TimeSyncPacket *packet = message->GetObject<TimeSyncPacket>();
    if (packet == nullptr) {
        return -E_INVALID_ARGS;
    }

    // build timeSync ack packet
    TimeSyncPacket ackPacket = TimeSyncPacket(*packet);
    ackPacket.SetTargetTimeBegin(targetTimeBegin);
    TimeStamp targetTimeEnd = timeHelper_->GetTime();
    ackPacket.SetTargetTimeEnd(targetTimeEnd);
    LOGD("TimeSync::RequestRecv, dev = %s{private}, sTimeEnd = %llu, tTimeEnd = %llu, sbegin = %llu, tbegin = %llu",
        deviceId_.c_str(), ackPacket.GetSourceTimeEnd(), ackPacket.GetTargetTimeEnd(), ackPacket.GetSourceTimeBegin(),
        ackPacket.GetTargetTimeBegin());
    if (ackPacket.GetSourceTimeBegin() > TimeHelper::MAX_VALID_TIME) {
        LOGD("[TimeSync][RequestRecv] Time valid check failed.");
        return -E_INVALID_TIME;
    }

    TimeOffset timeoffsetIgnoreRtt = ackPacket.GetSourceTimeBegin() - targetTimeBegin;
    TimeOffset metadataTimeoffset;
    metadata_->GetTimeOffset(deviceId_, metadataTimeoffset);

    // 2 is half of INT64_MAX
    if ((std::abs(metadataTimeoffset) >= INT64_MAX / 2) || (std::abs(timeoffsetIgnoreRtt) >= INT64_MAX / 2) ||
        (std::abs(metadataTimeoffset - timeoffsetIgnoreRtt) > MAX_TIME_OFFSET_NOISE)) {
        LOGI("[TimeSync][RequestRecv] timeoffSet invalid, should do time sync");
        isSynced_ = false;
    }

    Message *ackMessage = new (std::nothrow) Message(TIME_SYNC_MESSAGE);
    if (ackMessage == nullptr) {
        return -E_OUT_OF_MEMORY;
    }
    ackMessage->SetPriority(Priority::NORMAL);
    ackMessage->SetMessageType(TYPE_RESPONSE);
    ackMessage->SetTarget(deviceId_);
    int errCode = ackMessage->SetCopiedObject<>(ackPacket);
    if (errCode != E_OK) {
        delete ackMessage;
        ackMessage = nullptr;
        return errCode;
    }

    errCode = SendPacket(deviceId_, ackMessage);
    if (errCode != E_OK) {
        delete ackMessage;
        ackMessage = nullptr;
    }
    return errCode;
}

int TimeSync::SaveTimeOffset(const DeviceID &deviceID, TimeOffset timeOffset)
{
    return metadata_->SaveTimeOffset(deviceID, timeOffset);
}

TimeOffset TimeSync::CalculateTimeOffset(const TimeSyncPacket &timeSyncInfo)
{
    TimeOffset roundTrip = (timeSyncInfo.GetSourceTimeEnd() - timeSyncInfo.GetSourceTimeBegin()) -
        (timeSyncInfo.GetTargetTimeEnd() - timeSyncInfo.GetTargetTimeBegin());
    TimeOffset offset1 = timeSyncInfo.GetTargetTimeBegin() -
        timeSyncInfo.GetSourceTimeBegin() - (roundTrip / TRIP_DIV_HALF);
    TimeOffset offset2 = timeSyncInfo.GetTargetTimeEnd() + (roundTrip / TRIP_DIV_HALF) -
        timeSyncInfo.GetSourceTimeEnd();
    TimeOffset offset = (offset1 / TRIP_DIV_HALF) + (offset2 / TRIP_DIV_HALF);
    LOGD("TimeSync::CalculateTimeOffset roundTrip= %lld, offset1 = %lld, offset2 = %lld, offset = %lld",
        roundTrip, offset1, offset2, offset);
    return offset;
}

bool TimeSync::IsPacketValid(const Message *inMsg, uint16_t messageType)
{
    if (inMsg == nullptr) {
        return false;
    }
    if (inMsg->GetMessageId() != TIME_SYNC_MESSAGE) {
        LOGD("message Id = %d", inMsg->GetMessageId());
        return false;
    }
    if (messageType != inMsg->GetMessageType()) {
        LOGD("input Type = %d, inMsg type = %d", messageType, inMsg->GetMessageType());
        return false;
    }
    return true;
}

int TimeSync::SendPacket(const DeviceID &deviceId, const Message *message, const CommErrHandler &handler)
{
    int errCode = communicateHandle_->SendMessage(deviceId, message, false, SEND_TIME_OUT, handler);
    if (errCode != E_OK) {
        LOGE("[TimeSync] SendPacket failed, err %d", errCode);
    }
    return errCode;
}

int TimeSync::TimeSyncDriver(TimerId timerId)
{
    if (timerId != driverTimerId_) {
        return -E_INTERNAL_ERROR;
    }
    if (!isOnline_) {
        return E_OK;
    }
    std::lock_guard<std::mutex> lock(timeDriverLock_);
    int errCode = RuntimeContext::GetInstance()->ScheduleTask([this]() {
        CommErrHandler handler = std::bind(&TimeSync::CommErrHandlerFunc, std::placeholders::_1, this);
        (void)this->SyncStart(handler);
        std::lock_guard<std::mutex> innerLock(this->timeDriverLock_);
        this->timeDriverLockCount_--;
        this->timeDriverCond_.notify_all();
    });
    if (errCode != E_OK) {
        LOGE("[TimeSync][TimerSyncDriver] ScheduleTask failed err %d", errCode);
        return errCode;
    }
    timeDriverLockCount_++;
    return E_OK;
}

int TimeSync::GetTimeOffset(TimeOffset &outOffset)
{
    if (!isSynced_) {
        {
            std::lock_guard<std::mutex> lock(cvLock_);
            isAckReceived_ = false;
        }
        CommErrHandler handler = std::bind(&TimeSync::CommErrHandlerFunc, std::placeholders::_1, this);
        int errCode = SyncStart(handler);
        LOGD("TimeSync::GetTimeOffset start, current = %llu, errCode:%d", TimeHelper::GetSysCurrentTime(), errCode);
        std::unique_lock<std::mutex> lock(cvLock_);
        if (errCode != E_OK || !conditionVar_.wait_for(lock, std::chrono::seconds(TIME_SYNC_WAIT_TIME),
            [this](){ return this->isAckReceived_ == true; })) {
            LOGD("TimeSync::GetTimeOffset, retryTime_ = %d", retryTime_);
            retryTime_++;
            if (retryTime_ < MAX_RETRY_TIME) {
                lock.unlock();
                return GetTimeOffset(outOffset);
            }
            retryTime_ = 0;
            return -E_TIMEOUT;
        }
    }
    retryTime_ = 0;
    metadata_->GetTimeOffset(deviceId_, outOffset);
    return E_OK;
}

bool TimeSync::IsNeedSync() const
{
    return !isSynced_;
}

void TimeSync::SetOnline(bool isOnline)
{
    isOnline_ = isOnline;
}

void TimeSync::CommErrHandlerFunc(int errCode, TimeSync *timeSync)
{
    LOGD("[TimeSync][CommErrHandle] errCode:%d", errCode);
    std::lock_guard<std::mutex> lock(timeSyncSetLock_);
    if (timeSyncSet_.count(timeSync) == 0) {
        LOGI("[TimeSync][CommErrHandle] timeSync has been killed");
        return;
    }
    if (timeSync == nullptr) {
        LOGI("[TimeSync][CommErrHandle] timeSync is nullptr");
        return;
    }
    if (errCode != E_OK) {
        timeSync->SetOnline(false);
    } else {
        timeSync->SetOnline(true);
    }
}

void TimeSync::ResetTimer()
{
    std::unique_lock<std::mutex> lock(timeDriverLock_);
    RuntimeContext::GetInstance()->RemoveTimer(driverTimerId_, true);
    int errCode = RuntimeContext::GetInstance()->SetTimer(
        TIME_SYNC_INTERVAL, driverCallback_, nullptr, driverTimerId_);
    if (errCode != E_OK) {
        LOGW("[TimeSync] Reset TimeSync timer failed err :%d", errCode);
    }
}
} // namespace DistributedDB