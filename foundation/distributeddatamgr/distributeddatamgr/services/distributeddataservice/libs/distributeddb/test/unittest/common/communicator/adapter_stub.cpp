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

#include "adapter_stub.h"
#include "log_print.h"
#include "db_errno.h"
#include "endian_convert.h"
#include "frame_header.h"

using namespace DistributedDB;

namespace {
    const uint32_t MTU_SIZE = 5 * 1024 * 1024; // 5 M, 1024 is scale
}

/*
 * Override Part
 */
AdapterStub::~AdapterStub()
{
    // Do nothing
}

int AdapterStub::StartAdapter()
{
    return E_OK;
}

void AdapterStub::StopAdapter()
{
    // Do nothing
}

uint32_t AdapterStub::GetMtuSize()
{
    return MTU_SIZE;
}

uint32_t AdapterStub::GetMtuSize(const std::string &target)
{
    return GetMtuSize();
}

int AdapterStub::GetLocalIdentity(std::string &outTarget)
{
    outTarget = localTarget_;
    return E_OK;
}

int AdapterStub::SendBytes(const std::string &dstTarget, const uint8_t *bytes, uint32_t length)
{
    LOGI("[UT][Stub][Send] Send length=%d to dstTarget=%s begin.", length, dstTarget.c_str());
    ApplySendBlock();

    if (QuerySendRetry(dstTarget)) {
        LOGI("[UT][Stub][Send] Retry for %s true.", dstTarget.c_str());
        return -E_WAIT_RETRY;
    }

    if (QuerySendTotalLoss()) {
        LOGI("[UT][Stub][Send] Total loss for %s true.", dstTarget.c_str());
        return E_OK;
    }

    if (QuerySendPartialLoss()) {
        LOGI("[UT][Stub][Send] Partial loss for %s true.", dstTarget.c_str());
        return E_OK;
    }

    std::lock_guard<std::mutex> onChangeLockGuard(onChangeMutex_);
    if (targetMapAdapter_.count(dstTarget) == 0) {
        LOGI("[UT][Stub][Send] dstTarget=%s not found.", dstTarget.c_str());
        return -E_NOT_FOUND;
    }

    ApplySendBitError(bytes, length);

    AdapterStub *toAdapter = targetMapAdapter_[dstTarget];
    toAdapter->DeliverBytes(localTarget_, bytes, length);
    LOGI("[UT][Stub][Send] Send to dstTarget=%s end.", dstTarget.c_str());
    return E_OK;
}

int AdapterStub::RegBytesReceiveCallback(const BytesReceiveCallback &onReceive, const Finalizer &inOper)
{
    std::lock_guard<std::mutex> onReceiveLockGuard(onReceiveMutex_);
    return RegCallBack(onReceive, onReceiveHandle_, inOper, onReceiveFinalizer_);
}

int AdapterStub::RegTargetChangeCallback(const TargetChangeCallback &onChange, const Finalizer &inOper)
{
    std::lock_guard<std::mutex> onChangeLockGuard(onChangeMutex_);
    return RegCallBack(onChange, onChangeHandle_, inOper, onChangeFinalizer_);
}

int AdapterStub::RegSendableCallback(const SendableCallback &onSendable, const Finalizer &inOper)
{
    std::lock_guard<std::mutex> onSendableLockGuard(onSendableMutex_);
    return RegCallBack(onSendable, onSendableHandle_, inOper, onSendableFinalizer_);
}

/*
 * Extended Part
 */
void AdapterStub::ConnectAdapterStub(AdapterStub *thisStub, AdapterStub *thatStub)
{
    LOGI("[UT][Stub][ConnectAdapter] thisStub=%s, thatStub=%s.", thisStub->GetLocalTarget().c_str(),
        thatStub->GetLocalTarget().c_str());
    thisStub->Connect(thatStub);
    thatStub->Connect(thisStub);
}

void AdapterStub::DisconnectAdapterStub(AdapterStub *thisStub, AdapterStub *thatStub)
{
    LOGI("[UT][Stub][DisconnectAdapter] thisStub=%s, thatStub=%s.", thisStub->GetLocalTarget().c_str(),
        thatStub->GetLocalTarget().c_str());
    thisStub->Disconnect(thatStub);
    thatStub->Disconnect(thisStub);
}

AdapterStub::AdapterStub(const std::string &inLocalTarget)
    : localTarget_(inLocalTarget)
{
}

const std::string &AdapterStub::GetLocalTarget()
{
    return localTarget_;
}

void AdapterStub::Connect(AdapterStub *inStub)
{
    LOGI("[UT][Stub][Connect] thisStub=%s, thatStub=%s.", localTarget_.c_str(), inStub->GetLocalTarget().c_str());
    std::lock_guard<std::mutex> onChangeLockGuard(onChangeMutex_);
    targetMapAdapter_[inStub->GetLocalTarget()] = inStub;
    if (onChangeHandle_) {
        onChangeHandle_(inStub->GetLocalTarget(), true);
    }
}

void AdapterStub::Disconnect(AdapterStub *inStub)
{
    LOGI("[UT][Stub][Disconnect] thisStub=%s, thatStub=%s.", localTarget_.c_str(), inStub->GetLocalTarget().c_str());
    std::lock_guard<std::mutex> onChangeLockGuard(onChangeMutex_);
    targetMapAdapter_.erase(inStub->GetLocalTarget());
    if (onChangeHandle_) {
        onChangeHandle_(inStub->GetLocalTarget(), false);
    }
}

void AdapterStub::DeliverBytes(const std::string &srcTarget, const uint8_t *bytes, uint32_t length)
{
    std::lock_guard<std::mutex> onReceiveLockGuard(onReceiveMutex_);
    if (onReceiveHandle_) {
        onReceiveHandle_(srcTarget, bytes, length);
    }
}

/*
 * Simulate Part
 */
void AdapterStub::SimulateSendBlock()
{
    LOGI("[UT][Stub][Block] Before Lock.");
    block_.lock();
    LOGI("[UT][Stub][Block] After Lock.");
}

void AdapterStub::SimulateSendBlockClear()
{
    LOGI("[UT][Stub][UnBlock] Before UnLock.");
    block_.unlock();
    LOGI("[UT][Stub][UnBlock] After UnLock.");
}

void AdapterStub::SimulateSendRetry(const std::string &dstTarget)
{
    std::lock_guard<std::mutex> retryLockGuard(retryMutex_);
    targetRetrySet_.insert(dstTarget);
}

void AdapterStub::SimulateSendRetryClear(const std::string &dstTarget)
{
    bool isSetBefore = false;
    {
        std::lock_guard<std::mutex> retryLockGuard(retryMutex_);
        if (targetRetrySet_.count(dstTarget) == 0) {
            return;
        }
        isSetBefore = true;
        targetRetrySet_.erase(dstTarget);
    }
    if (isSetBefore) {
        std::lock_guard<std::mutex> onSendableLockGuard(onSendableMutex_);
        if (onSendableHandle_) {
            onSendableHandle_(dstTarget);
        }
    }
}

void AdapterStub::SimulateSendPartialLoss()
{
    isPartialLossSimulated_ = true;
}

void AdapterStub::SimulateSendPartialLossClear()
{
    isPartialLossSimulated_ = false;
}

void AdapterStub::SimulateSendTotalLoss()
{
    isTotalLossSimulated_ = true;
}

void AdapterStub::SimulateSendTotalLossClear()
{
    isTotalLossSimulated_ = false;
}

void AdapterStub::SimulateSendBitErrorInMagicField(bool doFlag, uint16_t inMagic)
{
    doChangeMagicFlag_ = doFlag;
    magicField_ = inMagic;
}

void AdapterStub::SimulateSendBitErrorInVersionField(bool doFlag, uint16_t inVersion)
{
    doChangeVersionFlag_ = doFlag;
    versionField_ = inVersion;
}

void AdapterStub::SimulateSendBitErrorInCheckSumField(bool doFlag, uint64_t inCheckSum)
{
    doChangeCheckSumFlag_ = doFlag;
    checkSumField_ = inCheckSum;
}

void AdapterStub::SimulateSendBitErrorInPacketLenField(bool doFlag, uint32_t inPacketLen)
{
    doChangePacketLenFlag_ = doFlag;
    packetLenField_ = inPacketLen;
}

void AdapterStub::SimulateSendBitErrorInPacketTypeField(bool doFlag, uint8_t inPacketType)
{
    doChangePacketTypeFlag_ = doFlag;
    packetTypeField_ = inPacketType;
}

void AdapterStub::SimulateSendBitErrorInPaddingLenField(bool doFlag, uint8_t inPaddingLen)
{
    doChangePaddingLenFlag_ = doFlag;
    paddingLenField_ = inPaddingLen;
}

void AdapterStub::SimulateSendBitErrorInMessageIdField(bool doFlag, uint32_t inMessageId)
{
    doChangeMessageIdFlag_ = doFlag;
    messageIdField_ = inMessageId;
}

void AdapterStub::ApplySendBlock()
{
    LOGI("[UT][Stub][ApplyBlock] Before Lock&UnLock.");
    block_.lock();
    block_.unlock();
    LOGI("[UT][Stub][ApplyBlock] After Lock&UnLock.");
}

bool AdapterStub::QuerySendRetry(const std::string &dstTarget)
{
    std::lock_guard<std::mutex> retryLockGuard(retryMutex_);
    if (targetRetrySet_.count(dstTarget) == 0) {
        return false;
    } else {
        return true;
    }
}

bool AdapterStub::QuerySendPartialLoss()
{
    if (isPartialLossSimulated_) {
        uint64_t count = countForPartialLoss_.fetch_add(1, std::memory_order_seq_cst);
        if (count % 2 == 0) { // 2 is half
            return true;
        }
    }
    return false;
}

bool AdapterStub::QuerySendTotalLoss()
{
    return isTotalLossSimulated_;
}

namespace {
uint64_t CalculateXorSum(const uint8_t *bytes, uint32_t length)
{
    if (length % sizeof(uint64_t) != 0) {
        return 0;
    }
    int count = length / sizeof(uint64_t);
    auto array = reinterpret_cast<const uint64_t *>(bytes);
    uint64_t outSum = 0;
    for (int i = 0; i < count; i++) {
        outSum ^= array[i];
    }
    return outSum;
}
const uint32_t LENGTH_BEFORE_SUM_RANGE = sizeof(uint64_t) + sizeof(uint64_t);
}

void AdapterStub::ApplySendBitError(const uint8_t *bytes, uint32_t length)
{
    // Change field in CommPhyHeader
    if (length < sizeof(CommPhyHeader)) {
        return;
    }
    auto edibleBytes = const_cast<uint8_t *>(bytes);
    auto phyHeader = reinterpret_cast<CommPhyHeader *>(edibleBytes);
    if (doChangeMagicFlag_) {
        phyHeader->magic = HostToNet(magicField_);
    }
    if (doChangeVersionFlag_) {
        phyHeader->version = HostToNet(versionField_);
    }
    if (doChangeCheckSumFlag_) {
        phyHeader->checkSum = HostToNet(checkSumField_);
    }
    if (doChangePacketLenFlag_) {
        phyHeader->packetLen = HostToNet(packetLenField_);
    }
    if (doChangePacketTypeFlag_) {
        phyHeader->packetType = HostToNet(packetTypeField_);
    }
    if (doChangePaddingLenFlag_) {
        phyHeader->paddingLen = HostToNet(paddingLenField_);
    }
    // Change field in MessageHeader. Assumpt that no fragment
    if (length < sizeof(CommPhyHeader) + sizeof(CommDivergeHeader) + sizeof(MessageHeader)) {
        return;
    }
    edibleBytes += (sizeof(CommPhyHeader) + sizeof(CommDivergeHeader));
    auto msgHeader = reinterpret_cast<MessageHeader *>(edibleBytes);
    if (doChangeMessageIdFlag_) {
        msgHeader->messageId = HostToNet(messageIdField_);
        phyHeader->checkSum = HostToNet(CalculateXorSum(bytes + LENGTH_BEFORE_SUM_RANGE,
            length - LENGTH_BEFORE_SUM_RANGE));
    }
}
