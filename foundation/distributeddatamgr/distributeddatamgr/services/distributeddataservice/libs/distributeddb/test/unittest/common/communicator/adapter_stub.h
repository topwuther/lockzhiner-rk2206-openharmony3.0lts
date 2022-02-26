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

#ifndef ADAPTER_STUB_H
#define ADAPTER_STUB_H

#include <set>
#include <map>
#include <mutex>
#include <atomic>
#include <string>
#include <cstdint>
#include "iadapter.h"

namespace DistributedDB {
class AdapterStub : public IAdapter {
public:
    /*
     * Override Part
     */
    ~AdapterStub() override;

    int StartAdapter() override;
    void StopAdapter() override;
    uint32_t GetMtuSize() override;
    uint32_t GetMtuSize(const std::string &target) override;
    int GetLocalIdentity(std::string &outTarget) override;

    int SendBytes(const std::string &dstTarget, const uint8_t *bytes, uint32_t length) override;

    int RegBytesReceiveCallback(const BytesReceiveCallback &onReceive, const Finalizer &inOper) override;
    int RegTargetChangeCallback(const TargetChangeCallback &onChange, const Finalizer &inOper) override;
    int RegSendableCallback(const SendableCallback &onSendable, const Finalizer &inOper) override;

    /*
     * Extended Part
     */
    static void ConnectAdapterStub(AdapterStub *thisStub, AdapterStub *thatStub);
    static void DisconnectAdapterStub(AdapterStub *thisStub, AdapterStub *thatStub);

    explicit AdapterStub(const std::string &inLocalTarget);
    const std::string &GetLocalTarget();

    void SimulateSendBlock();
    void SimulateSendBlockClear();

    void SimulateSendRetry(const std::string &dstTarget);
    void SimulateSendRetryClear(const std::string &dstTarget);

    void SimulateSendPartialLoss();
    void SimulateSendPartialLossClear();

    void SimulateSendTotalLoss();
    void SimulateSendTotalLossClear();

    void SimulateSendBitErrorInMagicField(bool doFlag, uint16_t inMagic);
    void SimulateSendBitErrorInVersionField(bool doFlag, uint16_t inVersion);
    void SimulateSendBitErrorInCheckSumField(bool doFlag, uint64_t inCheckSum);
    void SimulateSendBitErrorInPacketLenField(bool doFlag, uint32_t inPacketLen);
    void SimulateSendBitErrorInPacketTypeField(bool doFlag, uint8_t inPacketType);
    void SimulateSendBitErrorInPaddingLenField(bool doFlag, uint8_t inPaddingLen);
    void SimulateSendBitErrorInMessageIdField(bool doFlag, uint32_t inMessageId);
private:
    void Connect(AdapterStub *inStub);
    void Disconnect(AdapterStub *inStub);
    void DeliverBytes(const std::string &srcTarget, const uint8_t *bytes, uint32_t length);

    void ApplySendBlock();
    bool QuerySendRetry(const std::string &dstTarget);
    bool QuerySendPartialLoss();
    bool QuerySendTotalLoss();
    void ApplySendBitError(const uint8_t *bytes, uint32_t length);

    std::string localTarget_;
    std::map<std::string, AdapterStub *> targetMapAdapter_;

    BytesReceiveCallback onReceiveHandle_;
    TargetChangeCallback onChangeHandle_;
    SendableCallback onSendableHandle_;
    Finalizer onReceiveFinalizer_;
    Finalizer onChangeFinalizer_;
    Finalizer onSendableFinalizer_;
    std::mutex onReceiveMutex_;
    std::mutex onChangeMutex_;
    std::mutex onSendableMutex_;

    // Member for simulation
    std::mutex block_;

    std::mutex retryMutex_;
    std::set<std::string> targetRetrySet_;

    std::atomic<bool> isPartialLossSimulated_{false};
    std::atomic<uint64_t> countForPartialLoss_{0};

    std::atomic<bool> isTotalLossSimulated_{false};

    bool doChangeMagicFlag_ = false;
    bool doChangeVersionFlag_ = false;
    bool doChangeCheckSumFlag_ = false;
    bool doChangePacketLenFlag_ = false;
    bool doChangePacketTypeFlag_ = false;
    bool doChangePaddingLenFlag_ = false;
    bool doChangeMessageIdFlag_ = false;
    uint16_t magicField_ = 0;
    uint16_t versionField_ = 0;
    uint64_t checkSumField_ = 0;
    uint32_t packetLenField_ = 0;
    uint8_t packetTypeField_ = 0;
    uint8_t paddingLenField_ = 0;
    uint32_t messageIdField_ = 0;
};
}

#endif // ADAPTER_STUB_H