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

#ifndef PARSE_RESULT_H
#define PARSE_RESULT_H

#include <set>
#include <cstdint>
#include "communicator_type_define.h"

namespace DistributedDB {
class ParseResult {
public:
    void SetFrameId(uint32_t inFrameId)
    {
        frameId_ = inFrameId;
    }
    void SetSourceId(uint64_t inSourceId)
    {
        sourceId_ = inSourceId;
    }
    void SetPacketLen(uint32_t inPacketLen)
    {
        packetLen_ = inPacketLen;
    }
    void SetPaddingLen(uint32_t inPaddingLen)
    {
        paddingLen_ = inPaddingLen;
    }
    void SetFragmentFlag(bool inFlag)
    {
        isFragment_ = inFlag;
    }
    void SetFrameTypeInfo(FrameType inFrameType)
    {
        frameType_ = inFrameType;
    }
    void SetFrameLen(uint32_t inFrameLen)
    {
        frameLen_ = inFrameLen;
    }
    void SetFragCount(uint16_t inFragCount)
    {
        fragCount_ = inFragCount;
    }
    void SetFragNo(uint16_t inFragNo)
    {
        fragNo_ = inFragNo;
    }
    void SetPayloadLen(uint32_t inPayloadLen)
    {
        payloadLen_ = inPayloadLen;
    }
    void SetCommLabel(const LabelType &inCommLabel)
    {
        commLabel_ = inCommLabel;
    }
    void SetLabelExchangeDistinctValue(uint64_t inDistinctValue)
    {
        labelExchangeDistinctValue_ = inDistinctValue;
    }
    void SetLabelExchangeSequenceId(uint64_t inSequenceId)
    {
        labelExchangeSequenceId_ = inSequenceId;
    }
    void SetLatestCommLabels(const std::set<LabelType> &inLatestCommLabels)
    {
        latestCommLabels_ = inLatestCommLabels;
    }

    uint32_t GetFrameId() const
    {
        return frameId_;
    }
    uint64_t GetSourceId() const
    {
        return sourceId_;
    }
    uint32_t GetPacketLen() const
    {
        return packetLen_;
    }
    uint32_t GetPaddingLen() const
    {
        return paddingLen_;
    }
    bool IsFragment() const
    {
        return isFragment_;
    }
    FrameType GetFrameTypeInfo() const
    {
        return frameType_;
    }
    uint32_t GetFrameLen() const
    {
        return frameLen_;
    }
    uint16_t GetFragCount() const
    {
        return fragCount_;
    }
    uint16_t GetFragNo() const
    {
        return fragNo_;
    }
    uint32_t GetPayloadLen() const
    {
        return payloadLen_;
    }
    LabelType GetCommLabel() const
    {
        return commLabel_;
    }
    uint64_t GetLabelExchangeDistinctValue() const
    {
        return labelExchangeDistinctValue_;
    }
    uint64_t GetLabelExchangeSequenceId() const
    {
        return labelExchangeSequenceId_;
    }
    const std::set<LabelType>& GetLatestCommLabels() const
    {
        return latestCommLabels_;
    }

    void SetDbVersion(uint16_t dbVersion)
    {
        dbVersion_ = dbVersion;
    }

    uint16_t GetDbVersion() const
    {
        return dbVersion_;
    }
private:
    // For CommPhyHeader
    uint32_t frameId_ = 0;
    uint64_t sourceId_ = 0;
    uint32_t packetLen_ = 0;
    uint8_t paddingLen_ = 0;
    bool isFragment_ = false;
    FrameType frameType_ = FrameType::INVALID_MAX_FRAME_TYPE;
    // For CommPhyOptHeader
    uint32_t frameLen_ = 0;
    uint16_t fragCount_ = 0;
    uint16_t fragNo_ = 0;
    // For Application Layer Frame
    uint32_t payloadLen_ = 0;
    LabelType commLabel_;
    // For Communication Layer Frame
    uint64_t labelExchangeDistinctValue_ = 0; // For Both LabelExchange And LabelExchangeAck Frame
    uint64_t labelExchangeSequenceId_ = 0; // For Both LabelExchange And LabelExchangeAck Frame
    std::set<LabelType> latestCommLabels_; // For Only LabelExchange Frame
    uint16_t dbVersion_ = 0;
};
}

#endif // PARSE_RESULT_H
