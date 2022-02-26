/*
 * Copyright (C) 2021 Huawei Device Co., Ltd.
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
#include "avrcp_ct_pass_through.h"

namespace bluetooth {
AvrcCtPassPacket::AvrcCtPassPacket(void) : stateFlag_(AVRC_KEY_STATE_INVALID), operationId_(AVRC_KEY_OPERATION_INVALID)
{
    LOG_DEBUG("[AVRCP CT] AvrcCtPassPacket::%{public}s", __func__);
}

AvrcCtPassPacket::AvrcCtPassPacket(uint8_t oper, uint8_t state) : stateFlag_(state), operationId_(oper)
{
    LOG_DEBUG("[AVRCP CT] AvrcCtPassPacket::%{public}s", __func__);

    operationId_ = oper;
    stateFlag_ = state;
}

AvrcCtPassPacket::AvrcCtPassPacket(Packet *pkt)
    : stateFlag_(AVRC_KEY_STATE_INVALID), operationId_(AVRC_KEY_OPERATION_INVALID)
{
    LOG_DEBUG("[AVRCP CT] AvrcCtPassPacket::%{public}s", __func__);

    DisassemblePacket(pkt);
}

AvrcCtPassPacket::~AvrcCtPassPacket(void)
{
    LOG_DEBUG("[AVRCP CT] AvrcCtPassPacket::%{public}s", __func__);

    if (pkt_ != nullptr) {
        PacketFree(pkt_);
        pkt_ = nullptr;
    }
}

const Packet *AvrcCtPassPacket::AssemblePacket(void)
{
    LOG_DEBUG("[AVRCP CT] AvrcCtPassPacket::%{public}s", __func__);

    pkt_ = PacketMalloc(0x00, 0x00, AVRC_CT_PASS_COMMAND_SIZE);
    auto buffer = static_cast<uint8_t *>(BufferPtr(PacketContinuousPayload(pkt_)));

    uint16_t offset = 0x00;
    offset += PushOctets1((buffer + offset), crCode_);
    offset += PushOctets1((buffer + offset), (subunitType_ << AVRC_CT_PASS_MOVE_BIT_3) | subunitId_);
    offset += PushOctets1((buffer + offset), opCode_);
    offset += PushOctets1((buffer + offset), (stateFlag_ << AVRC_CT_PASS_MOVE_BIT_7) | operationId_);
    offset += PushOctets1((buffer + offset), operationDataFieldLength_);

    return pkt_;
}

bool AvrcCtPassPacket::DisassemblePacket(Packet *pkt)
{
    LOG_DEBUG("[AVRCP CT] AvrcCtPassPacket::%{public}s", __func__);

    bool isValid = false;
    size_t size = PacketPayloadSize(pkt);
    if (size >= AVRC_CT_PASS_COMMAND_SIZE) {
        auto buffer = static_cast<uint8_t *>(BufferPtr(PacketContinuousPayload(pkt)));

        uint16_t offset = 0x00;
        uint64_t payload = 0x00;
        offset += PopOctets1((buffer + offset), payload);
        crCode_ = static_cast<uint8_t>(payload) & 0b00001111;

        /// Gets the company ID, because other operands are the default values.
        offset = AVRC_CT_PASS_OPERATION_ID_OFFSET;
        offset += PopOctets1((buffer + offset), payload);
        stateFlag_ = (static_cast<uint8_t>(payload) >> AVRC_CT_PASS_MOVE_BIT_7);
        operationId_ = (static_cast<uint8_t>(payload) & 0b01111111);

        if (IsSupportedKeyOperation() && IsValidKeyState()) {
            isValid = true;
        }
    } else {
        crCode_ = AVRC_CT_RSP_CODE_REJECTED;
        LOG_DEBUG("[AVRCP CT] Call - PacketPayloadRead - Failed! - size[%{public}d] < AVRC_CT_PASS_RESPONSE_SIZE[%{public}d]",
            size,
            AVRC_CT_PASS_COMMAND_SIZE);
    }

    return isValid;
}

bool AvrcCtPassPacket::IsSupportedKeyOperation(void) const
{
    LOG_DEBUG("[AVRCP CT] AvrcCtPassPacket::%{public}s", __func__);

    return IsSupportedKeyOperation(operationId_);
}

bool AvrcCtPassPacket::IsValidKeyState(void) const
{
    LOG_DEBUG("[AVRCP CT] AvrcCtPassPacket::%{public}s", __func__);

    return IsValidKeyState(stateFlag_);
}

bool AvrcCtPassPacket::IsSupportedKeyOperation(uint8_t key)
{
    LOG_DEBUG("[AVRCP CT] AvrcCtPassPacket::%{public}s", __func__);

    bool rtnSts = true;

    switch (key) {
        case AVRC_KEY_OPERATION_VOLUME_UP:
        case AVRC_KEY_OPERATION_VOLUME_DOWN:
        case AVRC_KEY_OPERATION_MUTE:
        case AVRC_KEY_OPERATION_PLAY:
        case AVRC_KEY_OPERATION_STOP:
        case AVRC_KEY_OPERATION_PAUSE:
        case AVRC_KEY_OPERATION_REWIND:
        case AVRC_KEY_OPERATION_FAST_FORWARD:
        case AVRC_KEY_OPERATION_FORWARD:
        case AVRC_KEY_OPERATION_BACKWARD:
            break;
        default:
            rtnSts = false;
    }

    return rtnSts;
}

bool AvrcCtPassPacket::IsValidKeyState(uint8_t state)
{
    LOG_DEBUG("[AVRCP CT] AvrcCtPassPacket::%{public}s", __func__);

    bool rtnSts = true;

    switch (state) {
        case AVRC_KEY_STATE_PRESS:
        case AVRC_KEY_STATE_RELEASE:
            break;
        default:
            rtnSts = false;
    }

    return rtnSts;
}
}  // namespace bluetooth
