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

#ifndef PROTOCOLPROTO_H
#define PROTOCOLPROTO_H

#include <cstdint>
#include "message.h"
#include "frame_header.h"
#include "parse_result.h"
#include "serial_buffer.h"
#include "message_transform.h"
#include "communicator_type_define.h"

namespace DistributedDB {
struct PhyHeaderInfo {
    uint64_t sourceId;
    uint32_t frameId;
    FrameType frameType;
};

class ProtocolProto {
public:
    // For application layer frame
    static uint32_t GetAppLayerFrameHeaderLength();
    static uint32_t GetLengthBeforeSerializedData();
    // For communication layer frame
    static uint32_t GetCommLayerFrameHeaderLength();

    // For handling application layer message. Return a heap object.
    static SerialBuffer *ToSerialBuffer(const Message *inMsg, int &outErrorNo, bool onlyMsgHeader = false);
    static Message *ToMessage(const SerialBuffer *inBuff, int &outErrorNo, bool onlyMsgHeader = false);

    // For handling communication layer frame. Return a heap object.
    static SerialBuffer *BuildEmptyFrameForVersionNegotiate(int &outErrorNo);
    static SerialBuffer *BuildFeedbackMessageFrame(const Message *inMsg, const LabelType &inLabel, int &outErrorNo);
    static SerialBuffer *BuildLabelExchange(uint64_t inDistinctValue, uint64_t inSequenceId,
        const std::set<LabelType> &inLabels, int &outErrorNo);
    static SerialBuffer *BuildLabelExchangeAck(uint64_t inDistinctValue, uint64_t inSequenceId, int &outErrorNo);

    // Return E_OK if no error happened. outPieces.size equal zero means not split, in this case, use ori buff.
    static int SplitFrameIntoPacketsIfNeed(const SerialBuffer *inBuff, uint32_t inMtuSize,
        std::vector<std::vector<uint8_t>> &outPieces);
    static int AnalyzeSplitStructure(const ParseResult &inResult, uint32_t &outFragLen, uint32_t &outLastFragLen);
    // inFrame is the destination, pktBytes and pktLength are the source, fragOffset and fragLength give the boundary
    static int CombinePacketIntoFrame(SerialBuffer *inFrame, const uint8_t *pktBytes, uint32_t pktLength,
        uint32_t fragOffset, uint32_t fragLength);

    // Must not be called in multi-thread
    // Return E_ALREADY_REGISTER if msgId is already registered
    // Return E_INVALID_ARGS if member of inFunc not all valid
    static int RegTransformFunction(uint32_t msgId, const TransformFunc &inFunc);

    // For application layer frame. In send case. Focus on frame.
    static int SetDivergeHeader(SerialBuffer *inBuff, const LabelType &inCommLabel);
    // For both application and communication layer frame. In send case. Focus on frame.
    static int SetPhyHeader(SerialBuffer *inBuff, const PhyHeaderInfo &inInfo);

    // In receive case, return error if parse fail.
    static int CheckAndParsePacket(const std::string &srcTarget, const uint8_t *bytes, uint32_t length,
        ParseResult &outResult);
    // The CommPhyHeader had already been parsed into outResult
    static int CheckAndParseFrame(const SerialBuffer *inBuff, ParseResult &outResult);

    // Dfx method for helping debugging
    static void DisplayPacketInformation(const uint8_t *bytes, uint32_t length);

    ProtocolProto() = delete;
    ~ProtocolProto() = delete;
private:
    static int CalculateXorSum(const uint8_t *bytes, uint32_t length, uint64_t &outSum);

    // For handling application layer message
    static int CalculateDataSerializeLength(const Message *inMsg, uint32_t &outLength);
    static int SerializeMessage(SerialBuffer *inBuff, const Message *inMsg);
    static int DeSerializeMessage(const SerialBuffer *inBuff, Message *inMsg, bool onlyMsgHeader);
    static bool IsSupportMessageVersion(uint16_t version);
    static bool IsFeedbackErrorMessage(uint32_t errorNo);

    static int ParseCommPhyHeader(const std::string &srcTarget, const uint8_t *bytes, uint32_t length,
        ParseResult &inResult);
    static int ParseCommPhyHeaderCheckMagicAndVersion(const uint8_t *bytes, uint32_t length);
    static int ParseCommPhyHeaderCheckField(const std::string &srcTarget, const CommPhyHeader &phyHeader,
        const uint8_t *bytes, uint32_t length);
    static int ParseCommPhyOptHeader(const uint8_t *bytes, uint32_t length, ParseResult &inResult);
    static int ParseCommDivergeHeader(const uint8_t *bytes, uint32_t length, ParseResult &inResult);
    static int ParseCommLayerPayload(const uint8_t *bytes, uint32_t length, ParseResult &inResult);
    static int ParseLabelExchange(const uint8_t *bytes, uint32_t length, ParseResult &inResult);
    static int ParseLabelExchangeAck(const uint8_t *bytes, uint32_t length, ParseResult &inResult);

    static int FrameFragmentation(const uint8_t *splitStartBytes, uint32_t splitLength, uint16_t fragCount,
        const CommPhyHeader &framePhyHeader, std::vector<std::vector<uint8_t>> &outPieces);
    static int FillFragmentPacket(const CommPhyHeader &phyHeader, const CommPhyOptHeader &phyOptHeader,
        const uint8_t *fragBytes, uint32_t fragLen, std::vector<uint8_t> &outPacket);

    static std::map<uint32_t, TransformFunc> msgIdMapFunc_;
};
} // namespace DistributedDB

#endif // PROTOCOLPROTO_H
