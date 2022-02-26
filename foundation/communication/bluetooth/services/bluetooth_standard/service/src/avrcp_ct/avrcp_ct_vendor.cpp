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

#include "avrcp_ct_vendor.h"
#include "avrcp_ct_vendor_continuation.h"
#include "avrcp_ct_vendor_player_application_settings.h"
#include "securec.h"

namespace bluetooth {
/******************************************************************
 * AvrcCtVendorPacket                                             *
 ******************************************************************/

AvrcCtVendorPacket::AvrcCtVendorPacket() : AvrcCtPacket(), status_(AVRC_ES_CODE_INVALID)
{
    LOG_DEBUG("[AVRCP CT] AvrcCtVendorPacket::%{public}s", __func__);
}

AvrcCtVendorPacket::~AvrcCtVendorPacket()
{
    LOG_DEBUG("[AVRCP CT] AvrcCtVendorPacket::%{public}s", __func__);

    if (pkt_ != nullptr) {
        PacketFree(pkt_);
        pkt_ = nullptr;
    }
}

const Packet *AvrcCtVendorPacket::AssemblePacket(void)
{
    LOG_DEBUG("[AVRCP CT] AvrcCtVendorPacket::%{public}s", __func__);

    pkt_ = AssembleFixedOperands();
    pkt_ = AssembleParameters(pkt_);

    return pkt_;
}

bool AvrcCtVendorPacket::DisassemblePacket(Packet *pkt)
{
    LOG_DEBUG("[AVRCP CT] AvrcCtVendorPacket::%{public}s", __func__);

    isValid_ = IsValidParameterLength(pkt);
    if (isValid_) {
        auto buffer = static_cast<uint8_t *>(BufferPtr(PacketContinuousPayload(pkt)));

        uint16_t offset = AVRC_CT_VENDOR_CTYPE_OFFSET;
        uint64_t payload = 0x00;
        offset += PopOctets1((buffer + offset), payload);
        crCode_ = static_cast<uint8_t>(payload);
        LOG_DEBUG("[AVRCP CT] crCode_[%x]", crCode_);

        offset = AVRC_CT_VENDOR_COMPANY_ID_OFFSET;
        payload = 0x00;
        offset += PopOctets3((buffer + offset), payload);
        companyId_ = static_cast<uint32_t>(payload);
        LOG_DEBUG("[AVRCP CT] companyId_[%x]", companyId_);

        offset = AVRC_CT_VENDOR_PACKET_TYPE_OFFSET;
        payload = 0x00;
        offset += PopOctets1((buffer + offset), payload);
        packetType_ = static_cast<uint8_t>(payload);
        LOG_DEBUG("[AVRCP CT] packetType_[%x]", packetType_);

        if ((crCode_ == AVRC_CT_RSP_CODE_REJECTED || crCode_ == AVRC_CT_RSP_CODE_NOT_IMPLEMENTED) &&
            parameterLength_ == AVRC_TG_VENDOR_REJECT_PARAMETER_LENGTH) {
            isValid_ = false;
            offset = AVRC_CT_VENDOR_STATUS_OFFSET;
            payload = 0x00;
            offset += PopOctets1((buffer + offset), payload);
            status_ = static_cast<uint8_t>(payload);
            LOG_DEBUG("[AVRCP CT] status_[%x]", status_);
        } else {
            isValid_ = DisassembleParameters(buffer);
        }
    } else {
        LOG_DEBUG("[AVRCP CT] The parameter length is invalid!");
    }

    return isValid_;
}

Packet *AvrcCtVendorPacket::AssembleFixedOperands(void)
{
    LOG_DEBUG("[AVRCP CT] AvrcCtVendorPacket::%{public}s", __func__);

    pkt_ = PacketMalloc(0x00, 0x00, AVRC_CT_VENDOR_MIN_COMMAND_SIZE);
    auto buffer = static_cast<uint8_t *>(BufferPtr(PacketContinuousPayload(pkt_)));

    uint16_t offset = 0x00;
    offset += PushOctets1((buffer + offset), crCode_);
    LOG_DEBUG("[AVRCP CT] crCode_[%x]", crCode_);

    offset += PushOctets1((buffer + offset), (subunitType_ << AVRC_CT_VENDOR_MOVE_BIT_3) | subunitId_);
    LOG_DEBUG("[AVRCP CT] subunitType_[%x]", subunitType_);
    LOG_DEBUG("[AVRCP CT] subunitId_[%x]", subunitId_);
    LOG_DEBUG("[AVRCP CT] subunitType_ << 3 | subunitId_[%x]", subunitType_ << AVRC_CT_VENDOR_MOVE_BIT_3 | subunitId_);

    offset += PushOctets1((buffer + offset), opCode_);
    LOG_DEBUG("[AVRCP CT] opCode_[%x]", opCode_);

    offset += PushOctets3((buffer + offset), companyId_);
    LOG_DEBUG("[AVRCP CT] companyId_[%x]", companyId_);

    offset += PushOctets1((buffer + offset), pduId_);
    LOG_DEBUG("[AVRCP CT] pduId_[%x]", pduId_);

    offset += PushOctets1((buffer + offset), packetType_);
    LOG_DEBUG("[AVRCP CT] packetType_[%x]", packetType_);

    return pkt_;
}

bool AvrcCtVendorPacket::IsValidParameterLength(Packet *pkt)
{
    LOG_DEBUG("[AVRCP CT] AvrcCtVendorPacket::%{public}s", __func__);

    bool result = false;

    size_t size = PacketPayloadSize(pkt);
    if (size >= AVRC_CT_VENDOR_FIXED_OPERAND_SIZE) {
        auto buffer = static_cast<uint8_t *>(BufferPtr(PacketContinuousPayload(pkt)));

        uint16_t offset = AVRC_CT_VENDOR_PARAMETER_LENGTH_OFFSET;
        uint64_t payload = 0x00;
        offset += PopOctets2((buffer + offset), payload);
        parameterLength_ = static_cast<uint32_t>(payload);
        LOG_DEBUG("[AVRCP CT] parameterLength_[%{public}d]", parameterLength_);

        if (size - AVRC_CT_VENDOR_FIXED_OPERAND_SIZE == parameterLength_) {
            result = true;
        }
    } else {
        LOG_DEBUG("[AVRCP CT]: The size of the packet is invalid! - actual size[%uz] - valid min size[%llu]",
            size,
            AVRC_CT_VENDOR_FIXED_OPERAND_SIZE);
    }

    return result;
}

/******************************************************************
 * GetCapability                                                  *
 ******************************************************************/

AvrcCtGcPacket::AvrcCtGcPacket(uint8_t capabilityId) : AvrcCtVendorPacket()
{
    LOG_DEBUG("[AVRCP CT] AvrcCtGcPacket::%{public}s", __func__);

    crCode_ = AVRC_CT_CMD_CODE_STATUS;
    pduId_ = AVRC_CT_PDU_ID_GET_CAPABILITIES;
    parameterLength_ = AVRC_CT_GC_PARAMETER_LENGTH;
    capabilityId_ = capabilityId;
}

AvrcCtGcPacket::AvrcCtGcPacket(Packet *pkt) : AvrcCtVendorPacket()
{
    LOG_DEBUG("[AVRCP CT] AvrcCtGcPacket::%{public}s", __func__);

    crCode_ = AVRC_CT_CMD_CODE_STATUS;
    pduId_ = AVRC_CT_PDU_ID_GET_CAPABILITIES;

    DisassemblePacket(pkt);
}

AvrcCtGcPacket::~AvrcCtGcPacket()
{
    LOG_DEBUG("[AVRCP CT] AvrcCtGcPacket::%{public}s", __func__);

    companies_.clear();
    events_.clear();
}

Packet *AvrcCtGcPacket::AssembleParameters(Packet *pkt)
{
    LOG_DEBUG("[AVRCP CT] AvrcCtGcPacket::%{public}s", __func__);

    auto buffer = BufferMalloc(AVRC_CT_VENDOR_PARAMETER_LENGTH_SIZE + AVRC_CT_GC_PARAMETER_LENGTH);
    auto bufferPtr = static_cast<uint8_t *>(BufferPtr(buffer));
    LOG_DEBUG("[AVRCP TG] BufferMalloc[%llu]", (AVRC_CT_VENDOR_PARAMETER_LENGTH_SIZE + AVRC_CT_GC_PARAMETER_LENGTH));

    uint16_t offset = 0x0000;
    offset += PushOctets2((bufferPtr + offset), parameterLength_);
    LOG_DEBUG("[AVRCP CT] parameterLength_[%hu]", parameterLength_);

    offset += PushOctets1((bufferPtr + offset), capabilityId_);
    LOG_DEBUG("[AVRCP CT] capabilityId_[%hhu]", capabilityId_);

    PacketPayloadAddLast(pkt, buffer);

    BufferFree(buffer);
    buffer = nullptr;
    bufferPtr = nullptr;

    return pkt;
}

bool AvrcCtGcPacket::DisassembleParameters(uint8_t *buffer)
{
    LOG_DEBUG("[AVRCP CT] AvrcCtGcPacket::%{public}s", __func__);

    isValid_ = false;

    uint16_t offset = AVRC_CT_VENDOR_PARAMETER_LENGTH_OFFSET;
    uint64_t payload = 0x00;
    offset += PopOctets2((buffer + offset), payload);
    parameterLength_ = static_cast<uint16_t>(payload);
    LOG_DEBUG("[AVRCP CT] parameterLength_[%{public}d]", parameterLength_);

    offset += PopOctets1((buffer + offset), payload);
    capabilityId_ = static_cast<uint8_t>(payload);
    LOG_DEBUG("[AVRCP CT] capabilityId_[%{public}d]", capabilityId_);

    offset += PopOctets1((buffer + offset), payload);
    capabilityCount_ = static_cast<uint8_t>(payload);
    LOG_DEBUG("[AVRCP CT] capabilityCount_[%{public}d]", capabilityCount_);

    if (capabilityId_ == AVRC_CAPABILITY_COMPANYID) {
        for (int i = 0; i < capabilityCount_; i++) {
            payload = 0x00;
            offset += PopOctets3((buffer + offset), payload);
            companies_.push_back(payload);
            LOG_DEBUG("[AVRCP CT] companyId_[%x]", static_cast<uint32_t>(payload));
        }
    } else if (capabilityId_ == AVRC_CAPABILITY_EVENTID) {
        for (int i = 0; i < capabilityCount_; i++) {
            payload = 0x00;
            offset += PopOctets1((buffer + offset), payload);
            events_.push_back(static_cast<uint8_t>(payload));
            LOG_DEBUG("[AVRCP CT] events_[%x]", static_cast<uint8_t>(payload));
        }
    }

    isValid_ = true;

    return isValid_;
}

/******************************************************************
 * ListPlayerApplicationSettingAttribute                          *
 ******************************************************************/

AvrcCtLpasaPacket::AvrcCtLpasaPacket() : AvrcCtVendorPacket()
{
    LOG_DEBUG("[AVRCP CT] AvrcCtLpasaPacket::%{public}s", __func__);

    crCode_ = AVRC_CT_CMD_CODE_STATUS;
    pduId_ = AVRC_CT_PDU_ID_LIST_PLAYER_APPLICATION_SETTING_ATTRIBUTES;
    parameterLength_ = AVRC_CT_LPASA_PARAMETER_LENGTH;
}

AvrcCtLpasaPacket::AvrcCtLpasaPacket(Packet *pkt) : AvrcCtVendorPacket()
{
    LOG_DEBUG("[AVRCP CT] AvrcCtLpasaPacket::%{public}s", __func__);

    crCode_ = AVRC_CT_CMD_CODE_STATUS;
    pduId_ = AVRC_CT_PDU_ID_LIST_PLAYER_APPLICATION_SETTING_ATTRIBUTES;

    DisassemblePacket(pkt);
}

AvrcCtLpasaPacket::~AvrcCtLpasaPacket()
{
    LOG_DEBUG("[AVRCP CT] AvrcCtLpasaPacket::%{public}s", __func__);

    attributes_.clear();
}

Packet *AvrcCtLpasaPacket::AssembleParameters(Packet *pkt)
{
    LOG_DEBUG("[AVRCP CT] AvrcCtLpasaPacket::%{public}s", __func__);

    auto buffer = BufferMalloc(AVRC_CT_LPASA_PARAMETER_LENGTH_SIZE);
    auto bufferPtr = static_cast<uint8_t *>(BufferPtr(buffer));
    LOG_DEBUG("[AVRCP TG] BufferMalloc[%{public}d]", (AVRC_CT_LPASA_PARAMETER_LENGTH_SIZE));

    uint16_t offset = 0x0000;
    offset += PushOctets2((bufferPtr + offset), parameterLength_);
    LOG_DEBUG("[AVRCP CT] parameterLength_[%{public}d]", parameterLength_);

    PacketPayloadAddLast(pkt, buffer);

    BufferFree(buffer);
    buffer = nullptr;
    bufferPtr = nullptr;

    return pkt;
}

bool AvrcCtLpasaPacket::DisassembleParameters(uint8_t *buffer)
{
    LOG_DEBUG("[AVRCP CT] AvrcCtLpasaPacket::%{public}s", __func__);

    isValid_ = false;

    uint16_t offset = AVRC_CT_VENDOR_PARAMETER_LENGTH_OFFSET;
    uint64_t payload = 0x00;
    offset += PopOctets2((buffer + offset), payload);
    parameterLength_ = static_cast<uint16_t>(payload);
    LOG_DEBUG("[AVRCP CT] parameterLength_[%{public}d]", parameterLength_);

    offset += PopOctets1((buffer + offset), payload);
    numOfAttributes_ = static_cast<uint8_t>(payload);
    LOG_DEBUG("[AVRCP CT] numOfAttributes_[%{public}d]", numOfAttributes_);

    for (int i = 0; i < numOfAttributes_; i++) {
        offset += PopOctets1((buffer + offset), payload);
        attributes_.push_back(static_cast<uint8_t>(payload));
        LOG_DEBUG("[AVRCP CT] attributes[%x]", static_cast<uint8_t>(payload));
    }
    isValid_ = true;

    return isValid_;
}

/******************************************************************
 * ListPlayerApplicationSettingValues                             *
 ******************************************************************/

AvrcCtLpasvPacket::AvrcCtLpasvPacket(uint8_t attributes)
    : AvrcCtVendorPacket(), attribute_(AVRC_PLAYER_ATTRIBUTE_ILLEGAL)
{
    LOG_DEBUG("[AVRCP CT] AvrcCtLpasvPacket::%{public}s", __func__);

    crCode_ = AVRC_CT_CMD_CODE_STATUS;
    pduId_ = AVRC_CT_PDU_ID_LIST_PLAYER_APPLICATION_SETTING_VALUES;
    parameterLength_ = AVRC_CT_LPASV_PARAMETER_LENGTH;

    attribute_ = attributes;
}

AvrcCtLpasvPacket::AvrcCtLpasvPacket(Packet *pkt) : AvrcCtVendorPacket(), attribute_(AVRC_PLAYER_ATTRIBUTE_ILLEGAL)
{
    LOG_DEBUG("[AVRCP CT] AvrcCtLpasvPacket::%{public}s", __func__);

    crCode_ = AVRC_CT_CMD_CODE_STATUS;
    pduId_ = AVRC_CT_PDU_ID_LIST_PLAYER_APPLICATION_SETTING_VALUES;

    DisassemblePacket(pkt);
}

AvrcCtLpasvPacket::~AvrcCtLpasvPacket()
{
    LOG_DEBUG("[AVRCP CT] AvrcCtLpasvPacket::%{public}s", __func__);

    values_.clear();
}

Packet *AvrcCtLpasvPacket::AssembleParameters(Packet *pkt)
{
    LOG_DEBUG("[AVRCP CT] AvrcCtLpasvPacket::%{public}s", __func__);

    size_t bufferSize = AVRC_CT_VENDOR_PARAMETER_LENGTH_SIZE + AVRC_CT_LPASV_PARAMETER_LENGTH;
    LOG_DEBUG("[AVRCP CT] BufferMalloc[%{public}d]", bufferSize);

    auto buffer = BufferMalloc(bufferSize);
    auto bufferPtr = static_cast<uint8_t *>(BufferPtr(buffer));

    uint16_t offset = 0x0000;
    offset += PushOctets2((bufferPtr + offset), parameterLength_);
    LOG_DEBUG("[AVRCP CT] parameterLength_[%{public}d]", parameterLength_);

    offset += PushOctets1((bufferPtr + offset), attribute_);
    LOG_DEBUG("[AVRCP CT] attribute_[%x]", attribute_);

    PacketPayloadAddLast(pkt, buffer);

    BufferFree(buffer);
    buffer = nullptr;
    bufferPtr = nullptr;

    return pkt;
}

bool AvrcCtLpasvPacket::DisassembleParameters(uint8_t *buffer)
{
    LOG_DEBUG("[AVRCP CT] AvrcCtLpasvPacket::%{public}s", __func__);

    isValid_ = false;

    receivedFragments_++;
    LOG_DEBUG("[AVRCP CT] receivedFragments_[%{public}d]", receivedFragments_);

    uint16_t offset = AVRC_CT_VENDOR_PARAMETER_LENGTH_OFFSET;
    uint64_t payload = 0x00;
    offset += PopOctets2((buffer + offset), payload);
    parameterLength_ = static_cast<uint16_t>(payload);
    LOG_DEBUG("[AVRCP CT] parameterLength_[%{public}d]", parameterLength_);

    offset += PopOctets1((buffer + offset), payload);
    numOfValues_ = static_cast<uint8_t>(payload);
    LOG_DEBUG("[AVRCP CT] numOfValues_[%{public}d]", numOfValues_);

    for (int i = 0; i < numOfValues_; i++) {
        offset += PopOctets1((buffer + offset), payload);
        values_.push_back(static_cast<uint8_t>(payload));
        LOG_DEBUG("[AVRCP CT] value[%x]", values_.back());
    }

    numOfValues_ = values_.size();

    LOG_DEBUG("[AVRCP CT] values_.size[%{public}d]", values_.size());

    if (numOfValues_ == values_.size() && numOfValues_ > 0) {
        isValid_ = true;
    }

    LOG_DEBUG("[AVRCP CT] isValid_[%{public}d]", isValid_);

    return isValid_;
}

bool AvrcCtLpasvPacket::IsValidAttribute(void) const
{
    LOG_DEBUG("[AVRCP CT] AvrcCtLpasvPacket::%{public}s", __func__);

    bool result = false;

    do {
        if (attribute_ == AVRC_PLAYER_ATTRIBUTE_ILLEGAL) {
            break;
        }

        if (attribute_ >= AVRC_PLAYER_ATTRIBUTE_RESERVED_MIN && attribute_ <= AVRC_PLAYER_ATTRIBUTE_RESERVED_MAX) {
            break;
        }

        result = true;
    } while (false);

    LOG_DEBUG("[AVRCP CT] result[%{public}d]", result);

    return result;
}

/******************************************************************
 * GetCurrentPlayerApplicationSettingValue                        *
 ******************************************************************/

AvrcCtGcpasvPacket::AvrcCtGcpasvPacket(const std::vector<uint8_t> &attributes) : AvrcCtVendorPacket()
{
    LOG_DEBUG("[AVRCP CT] AvrcCtGcpasvPacket::%{public}s", __func__);

    crCode_ = AVRC_CT_CMD_CODE_STATUS;
    pduId_ = AVRC_CT_PDU_ID_GET_CURRENT_PLAYER_APPLICATION_SETTING_VALUE;
    attributes_ = attributes;
}

AvrcCtGcpasvPacket::AvrcCtGcpasvPacket(Packet *pkt) : AvrcCtVendorPacket()
{
    LOG_DEBUG("[AVRCP CT] AvrcCtGcpasvPacket::%{public}s", __func__);

    crCode_ = AVRC_CT_CMD_CODE_STATUS;
    pduId_ = AVRC_CT_PDU_ID_GET_CURRENT_PLAYER_APPLICATION_SETTING_VALUE;

    DisassemblePacket(pkt);
}

AvrcCtGcpasvPacket::~AvrcCtGcpasvPacket()
{
    LOG_DEBUG("[AVRCP CT] AvrcCtGcpasvPacket::%{public}s", __func__);

    attributes_.clear();
    values_.clear();
}

Packet *AvrcCtGcpasvPacket::AssembleParameters(Packet *pkt)
{
    LOG_DEBUG("[AVRCP CT] AvrcCtGcpasvPacket::%{public}s", __func__);

    size_t bufferSize =
        AVRC_CT_VENDOR_PARAMETER_LENGTH_SIZE + AVRC_CT_GCPASV_NUM_OF_ATTRIBUTES_SIZE + attributes_.size();
    LOG_DEBUG("[AVRCP CT] BufferMalloc[%{public}d]", bufferSize);

    auto buffer = BufferMalloc(bufferSize);
    auto bufferPtr = static_cast<uint8_t *>(BufferPtr(buffer));

    uint16_t offset = 0x0000;
    parameterLength_ = bufferSize - AVRC_CT_VENDOR_PARAMETER_LENGTH_SIZE;
    offset += PushOctets2((bufferPtr + offset), parameterLength_);
    LOG_DEBUG("[AVRCP CT] parameterLength_[%{public}d]", parameterLength_);

    numOfAttributes_ = attributes_.size();
    offset += PushOctets1((bufferPtr + offset), numOfAttributes_);
    LOG_DEBUG("[AVRCP CT] numOfAttributes_[%{public}d]", numOfAttributes_);

    for (auto attribute : attributes_) {
        offset += PushOctets1((bufferPtr + offset), attribute);
        LOG_DEBUG("[AVRCP CT] attribute[%x]", attribute);
    }

    PacketPayloadAddLast(pkt, buffer);

    BufferFree(buffer);
    buffer = nullptr;
    bufferPtr = nullptr;

    return pkt;
}

bool AvrcCtGcpasvPacket::DisassembleParameters(uint8_t *buffer)
{
    LOG_DEBUG("[AVRCP CT] AvrcCtGcpasvPacket::%{public}s", __func__);

    isValid_ = false;

    receivedFragments_++;
    LOG_DEBUG("[AVRCP CT] receivedFragments_[%{public}d]", receivedFragments_);

    uint16_t offset = AVRC_CT_VENDOR_PARAMETER_LENGTH_OFFSET;
    uint64_t payload = 0x00;
    offset += PopOctets2((buffer + offset), payload);
    parameterLength_ = static_cast<uint16_t>(payload);
    LOG_DEBUG("[AVRCP CT] parameterLength_[%{public}d]", parameterLength_);

    offset += PopOctets1((buffer + offset), payload);
    numOfValues_ = static_cast<uint8_t>(payload);
    LOG_DEBUG("[AVRCP CT] numOfValues_[%{public}d]", numOfValues_);

    for (int i = 0; i < numOfValues_; i++) {
        offset += PopOctets1((buffer + offset), payload);
        attributes_.push_back(static_cast<uint8_t>(payload));
        LOG_DEBUG("[AVRCP CT] attribute[%x]", attributes_.back());

        offset += PopOctets1((buffer + offset), payload);
        values_.push_back(static_cast<uint8_t>(payload));
        LOG_DEBUG("[AVRCP CT] value[%x]", values_.back());
    }

    if (numOfValues_ == values_.size() && numOfValues_ > 0) {
        isValid_ = true;
    }

    LOG_DEBUG("[AVRCP CT] isValid_[%{public}d]", isValid_);

    return isValid_;
}

bool AvrcCtGcpasvPacket::IsValidAttribute(void) const
{
    LOG_DEBUG("[AVRCP CT] AvrcCtGcpasvPacket::%{public}s", __func__);

    bool result = false;

    for (auto attribute : attributes_) {
        do {
            if (attribute == AVRC_PLAYER_ATTRIBUTE_ILLEGAL) {
                break;
            }

            if (attribute >= AVRC_PLAYER_ATTRIBUTE_RESERVED_MIN && attribute <= AVRC_PLAYER_ATTRIBUTE_RESERVED_MAX) {
                break;
            }

            result = true;
        } while (false);
    }

    LOG_DEBUG("[AVRCP CT] result[%{public}d]", result);

    return result;
}

/******************************************************************
 * SetPlayerApplicationSettingValue                               *
 ******************************************************************/

AvrcCtSpasvPacket::AvrcCtSpasvPacket(const std::vector<uint8_t> &attributes, const std::vector<uint8_t> &values)
    : AvrcCtVendorPacket()
{
    LOG_DEBUG("[AVRCP CT] AvrcCtSpasvPacket::%{public}s", __func__);

    crCode_ = AVRC_CT_CMD_CODE_CONTROL;
    pduId_ = AVRC_CT_PDU_ID_SET_PLAYER_APPLICATION_SETTING_VALUE;

    attributes_ = attributes;
    values_ = values;
}

AvrcCtSpasvPacket::AvrcCtSpasvPacket(Packet *pkt) : AvrcCtVendorPacket()
{
    LOG_DEBUG("[AVRCP CT] AvrcCtSpasvPacket::%{public}s", __func__);

    crCode_ = AVRC_CT_CMD_CODE_CONTROL;
    pduId_ = AVRC_CT_PDU_ID_SET_PLAYER_APPLICATION_SETTING_VALUE;

    DisassemblePacket(pkt);
}

AvrcCtSpasvPacket::~AvrcCtSpasvPacket()
{
    LOG_DEBUG("[AVRCP CT] AvrcCtSpasvPacket::%{public}s", __func__);

    attributes_.clear();
    values_.clear();
}

Packet *AvrcCtSpasvPacket::AssembleParameters(Packet *pkt)
{
    LOG_DEBUG("[AVRCP CT] AvrcCtSpasvPacket::%{public}s", __func__);

    size_t bufferSize = AVRC_CT_VENDOR_PARAMETER_LENGTH_SIZE + AVRC_CT_SPASV_NUM_OF_ATTRIBUTES_SIZE +
                        attributes_.size() + values_.size();
    LOG_DEBUG("[AVRCP CT] BufferMalloc[%{public}d]", bufferSize);

    auto buffer = BufferMalloc(bufferSize);
    auto bufferPtr = static_cast<uint8_t *>(BufferPtr(buffer));

    uint16_t offset = 0x0000;
    parameterLength_ = AVRC_CT_SPASV_NUM_OF_ATTRIBUTES_SIZE + attributes_.size() + values_.size();
    offset += PushOctets2((bufferPtr + offset), parameterLength_);
    LOG_DEBUG("[AVRCP CT] parameterLength_[%{public}d]", parameterLength_);

    numOfAttributes_ = attributes_.size();
    offset += PushOctets1((bufferPtr + offset), numOfAttributes_);
    LOG_DEBUG("[AVRCP CT] numOfAttributes_[%{public}d]", numOfAttributes_);

    for (int i = 0; i < numOfAttributes_; i++) {
        offset += PushOctets1((bufferPtr + offset), attributes_.at(i));
        LOG_DEBUG("[AVRCP CT] attribute[%x]", attributes_.at(i));
        offset += PushOctets1((bufferPtr + offset), values_.at(i));
        LOG_DEBUG("[AVRCP CT] value[%x]", values_.at(i));
    }

    PacketPayloadAddLast(pkt, buffer);

    BufferFree(buffer);
    buffer = nullptr;
    bufferPtr = nullptr;

    return pkt;
}

bool AvrcCtSpasvPacket::DisassembleParameters(uint8_t *buffer)
{
    LOG_DEBUG("[AVRCP CT] AvrcCtSpasvPacket::%{public}s", __func__);

    isValid_ = false;

    uint16_t offset = AVRC_CT_VENDOR_PARAMETER_LENGTH_OFFSET;
    uint64_t payload = 0x00;
    offset += PopOctets2((buffer + offset), payload);
    parameterLength_ = static_cast<uint16_t>(payload);
    LOG_DEBUG("[AVRCP CT] parameterLength_[%{public}d]", parameterLength_);

    isValid_ = true;

    LOG_DEBUG("[AVRCP CT] isValid_[%{public}d]", isValid_);

    return isValid_;
}

bool AvrcCtSpasvPacket::IsValidAttribute(void) const
{
    LOG_DEBUG("[AVRCP CT] AvrcCtSpasvPacket::%{public}s", __func__);

    bool result = false;

    for (auto attribute : attributes_) {
        do {
            if (attribute == AVRC_PLAYER_ATTRIBUTE_ILLEGAL) {
                break;
            }

            if (attribute >= AVRC_PLAYER_ATTRIBUTE_RESERVED_MIN && attribute <= AVRC_PLAYER_ATTRIBUTE_RESERVED_MAX) {
                break;
            }

            result = true;
        } while (false);
    }

    LOG_DEBUG("[AVRCP CT] result[%{public}d]", result);

    return result;
}

/******************************************************************
 * GetPlayerApplicationSettingAttributeText                       *
 ******************************************************************/

AvrcCtGpasatPacket::AvrcCtGpasatPacket(std::vector<uint8_t> attributes) : AvrcCtVendorPacket()
{
    LOG_DEBUG("[AVRCP CT] AvrcCtGpasatPacket::%{public}s", __func__);

    crCode_ = AVRC_CT_CMD_CODE_STATUS;
    pduId_ = AVRC_CT_PDU_ID_GET_PLAYER_APPLICATION_SETTING_ATTRIBUTE_TEXT;
    attributes_ = attributes;
    numOfAttributes_ = attributes.size();
}

AvrcCtGpasatPacket::AvrcCtGpasatPacket(Packet *pkt) : AvrcCtVendorPacket()
{
    LOG_DEBUG("[AVRCP CT] AvrcCtGpasatPacket::%{public}s", __func__);

    crCode_ = AVRC_CT_CMD_CODE_STATUS;
    pduId_ = AVRC_CT_PDU_ID_GET_PLAYER_APPLICATION_SETTING_ATTRIBUTE_TEXT;

    DisassemblePacket(pkt);
}

AvrcCtGpasatPacket::~AvrcCtGpasatPacket()
{
    LOG_DEBUG("[AVRCP CT] AvrcCtGpasatPacket::%{public}s", __func__);

    attributes_.clear();
    attributeName_.clear();
}

Packet *AvrcCtGpasatPacket::AssembleParameters(Packet *pkt)
{
    LOG_DEBUG("[AVRCP CT] AvrcCtGpasatPacket::%{public}s", __func__);

    size_t bufferSize = AVRC_CT_VENDOR_PARAMETER_LENGTH_SIZE + AVRC_CT_GPASAT_ATTRIBUTE_NUM_LENGTH + attributes_.size();
    LOG_DEBUG("[AVRCP CT] BufferMalloc[%{public}d]", bufferSize);
    auto buffer = BufferMalloc(bufferSize);
    auto bufferPtr = static_cast<uint8_t *>(BufferPtr(buffer));

    uint16_t offset = 0x0000;
    parameterLength_ = bufferSize - AVRC_CT_VENDOR_PARAMETER_LENGTH_SIZE;
    offset += PushOctets2((bufferPtr + offset), parameterLength_);
    LOG_DEBUG("[AVRCP CT] parameterLength_[%{public}d]", parameterLength_);

    offset += PushOctets1((bufferPtr + offset), numOfAttributes_);
    LOG_DEBUG("[AVRCP CT] numOfAttributes_[%{public}d]", numOfAttributes_);

    for (auto attribute : attributes_) {
        offset += PushOctets1((bufferPtr + offset), attribute);
        LOG_DEBUG("[AVRCP CT] attribute[%x]", attribute);
    }

    PacketPayloadAddLast(pkt, buffer);

    BufferFree(buffer);
    buffer = nullptr;
    bufferPtr = nullptr;

    return pkt;
}

bool AvrcCtGpasatPacket::DisassembleParameters(uint8_t *buffer)
{
    LOG_DEBUG("[AVRCP CT] AvrcCtGpasatPacket::%{public}s", __func__);

    isValid_ = false;

    receivedFragments_++;
    LOG_DEBUG("[AVRCP CT] receivedFragments_[%{public}d]", receivedFragments_);

    uint16_t offset = AVRC_CT_VENDOR_PARAMETER_LENGTH_OFFSET;
    uint64_t payload = 0x00;
    offset += PopOctets2((buffer + offset), payload);
    parameterLength_ = static_cast<uint16_t>(payload);
    LOG_DEBUG("[AVRCP CT] parameterLength_[%{public}d]", parameterLength_);

    offset += PopOctets1((buffer + offset), payload);
    numOfAttributes_ = static_cast<uint8_t>(payload);
    LOG_DEBUG("[AVRCP CT] numOfAttributes_[%{public}d]", numOfAttributes_);

    for (int i = 0; i < numOfAttributes_; i++) {
        offset += PopOctets1((buffer + offset), payload);
        attributes_.push_back(static_cast<uint32_t>(payload));
        LOG_DEBUG("[AVRCP CT] attributeId[%x]", attributes_.back());

        offset += PopOctets2((buffer + offset), payload);
        LOG_DEBUG("[AVRCP CT] characterSetID[%x]", static_cast<uint16_t>(payload));

        offset += PopOctets1((buffer + offset), payload);
        AttributeValueLength_ = static_cast<uint8_t>(payload);
        LOG_DEBUG("[AVRCP CT] AttributeValueLength[%{public}d]", AttributeValueLength_);

        char *tempName = nullptr;
        tempName = new char[AttributeValueLength_ + 1];
        for (int j = 0; j < AttributeValueLength_; j++) {
            payload = 0x00;
            offset += PopOctets1((buffer + offset), payload);
            tempName[j] = static_cast<char>(payload);
        }
        tempName[AttributeValueLength_] = '\0';
        std::string name(tempName);
        AvrcpCtSafeDeleteArray(tempName, AttributeValueLength_ + 1);
        attributeName_.push_back(name);
        LOG_DEBUG("[AVRCP CT] AttributeValue[%{public}s]", attributeName_.back().c_str());
    }

    isValid_ = true;
    LOG_DEBUG("[AVRCP CT] result[%{public}d]", isValid_);

    return isValid_;
}

/******************************************************************
 * GetPlayerApplicationSettingValueText                           *
 ******************************************************************/

AvrcCtGpasvtPacket::AvrcCtGpasvtPacket(uint8_t attribtueId, std::vector<uint8_t> values)
    : AvrcCtVendorPacket(), attributeId_(AVRC_PLAYER_ATTRIBUTE_ILLEGAL)
{
    LOG_DEBUG("[AVRCP CT] AvrcCtGpasvtPacket::%{public}s", __func__);

    crCode_ = AVRC_CT_CMD_CODE_STATUS;
    pduId_ = AVRC_CT_PDU_ID_GET_PLAYER_APPLICATION_SETTING_VALUE_TEXT;
    values_ = values;
    numOfValues_ = values.size();
    attributeId_ = attribtueId;
}

AvrcCtGpasvtPacket::AvrcCtGpasvtPacket(Packet *pkt) : AvrcCtVendorPacket(), attributeId_(AVRC_PLAYER_ATTRIBUTE_ILLEGAL)
{
    LOG_DEBUG("[AVRCP CT] AvrcCtGpasvtPacket::%{public}s", __func__);

    crCode_ = AVRC_CT_CMD_CODE_STATUS;
    pduId_ = AVRC_CT_PDU_ID_GET_PLAYER_APPLICATION_SETTING_VALUE_TEXT;

    DisassemblePacket(pkt);
}

AvrcCtGpasvtPacket::~AvrcCtGpasvtPacket()
{
    LOG_DEBUG("[AVRCP CT] AvrcCtGpasvtPacket::%{public}s", __func__);

    values_.clear();
    valueName_.clear();
}

Packet *AvrcCtGpasvtPacket::AssembleParameters(Packet *pkt)
{
    LOG_DEBUG("[AVRCP CT] AvrcCtGpasvtPacket::%{public}s", __func__);

    size_t bufferSize = AVRC_CT_VENDOR_PARAMETER_LENGTH_SIZE + AVRC_CT_GPASVT_PARAMETER_LENGTH_SIZE + numOfValues_;
    LOG_DEBUG("[AVRCP CT] BufferMalloc[%{public}d]", bufferSize);
    auto buffer = BufferMalloc(bufferSize);
    auto bufferPtr = static_cast<uint8_t *>(BufferPtr(buffer));

    uint16_t offset = 0x0000;
    parameterLength_ = bufferSize - AVRC_CT_VENDOR_PARAMETER_LENGTH_SIZE;
    offset += PushOctets2((bufferPtr + offset), parameterLength_);
    LOG_DEBUG("[AVRCP CT] parameterLength_[%{public}d]", parameterLength_);

    offset += PushOctets1((bufferPtr + offset), attributeId_);
    LOG_DEBUG("[AVRCP CT] attributeId_[%x]", attributeId_);

    offset += PushOctets1((bufferPtr + offset), numOfValues_);
    LOG_DEBUG("[AVRCP CT] numOfValues_[%{public}d]", numOfValues_);

    for (auto value : values_) {
        offset += PushOctets1((bufferPtr + offset), value);
        LOG_DEBUG("[AVRCP CT] value[%x]", value);
    }

    PacketPayloadAddLast(pkt, buffer);

    BufferFree(buffer);
    buffer = nullptr;
    bufferPtr = nullptr;

    return pkt;
}

bool AvrcCtGpasvtPacket::DisassembleParameters(uint8_t *buffer)
{
    LOG_DEBUG("[AVRCP CT] AvrcCtGpasvtPacket::%{public}s", __func__);

    isValid_ = false;

    receivedFragments_++;
    LOG_DEBUG("[AVRCP CT] receivedFragments_[%{public}d]", receivedFragments_);

    uint16_t offset = AVRC_CT_VENDOR_PARAMETER_LENGTH_OFFSET;
    uint64_t payload = 0x00;
    offset += PopOctets2((buffer + offset), payload);
    parameterLength_ = static_cast<uint16_t>(payload);
    LOG_DEBUG("[AVRCP CT] parameterLength_[%{public}d]", parameterLength_);

    offset += PopOctets1((buffer + offset), payload);
    numOfValues_ = static_cast<uint8_t>(payload);
    LOG_DEBUG("[AVRCP CT] numOfValues_[%{public}d]", numOfValues_);

    for (int i = 0; i < numOfValues_; i++) {
        offset += PopOctets1((buffer + offset), payload);
        values_.push_back(static_cast<uint32_t>(payload));
        LOG_DEBUG("[AVRCP CT] valueId[%{public}d]", values_.back());

        offset += PopOctets2((buffer + offset), payload);
        LOG_DEBUG("[AVRCP CT] characterSetID[%x]", static_cast<uint16_t>(payload));

        offset += PopOctets1((buffer + offset), payload);
        AttributeValueLength_ = static_cast<uint8_t>(payload);
        LOG_DEBUG("[AVRCP CT] AttributeValueLength[%{public}d]", AttributeValueLength_);

        char *tempName = nullptr;
        tempName = new char[AttributeValueLength_ + 1];
        for (int j = 0; j < AttributeValueLength_; j++) {
            offset += PopOctets1((buffer + offset), payload);
            tempName[j] = static_cast<char>(payload);
        }
        tempName[AttributeValueLength_] = '\0';
        std::string name(tempName);
        AvrcpCtSafeDeleteArray(tempName, AttributeValueLength_ + 1);
        valueName_.push_back(name);
        LOG_DEBUG("[AVRCP CT] Value[%{public}s]", valueName_.back().c_str());
    }

    isValid_ = true;

    LOG_DEBUG("[AVRCP CT] result[%{public}d]", isValid_);

    return isValid_;
}

/******************************************************************
 * GetElementAttribute                                            *
 ******************************************************************/

AvrcCtGeaPacket::AvrcCtGeaPacket(uint64_t identifier, std::vector<uint32_t> attributes) : AvrcCtVendorPacket()
{
    LOG_DEBUG("[AVRCP CT] AvrcCtGeaPacket::%{public}s", __func__);

    crCode_ = AVRC_CT_CMD_CODE_STATUS;
    pduId_ = AVRC_CT_PDU_ID_GET_ELEMENT_ATTRIBUTES;
    number_ = attributes.size();
    identifier_ = identifier;
    attributes_ = attributes;
}

AvrcCtGeaPacket::AvrcCtGeaPacket(Packet *pkt) : AvrcCtVendorPacket()
{
    LOG_DEBUG("[AVRCP CT] AvrcCtGeaPacket::%{public}s", __func__);

    crCode_ = AVRC_CT_CMD_CODE_STATUS;
    pduId_ = AVRC_CT_PDU_ID_GET_ELEMENT_ATTRIBUTES;

    DisassemblePacket(pkt);
}

AvrcCtGeaPacket::~AvrcCtGeaPacket()
{
    LOG_DEBUG("[AVRCP CT] AvrcCtGeaPacket::%{public}s", __func__);

    attributes_.clear();
    values_.clear();
}

bool AvrcCtGeaPacket::DisassemblePacketAttributes(uint8_t *buffer, uint16_t offset)
{
    LOG_DEBUG("[AVRCP CT] AvrcCtGeaPacket::%{public}s", __func__);
    uint64_t payload = 0x00;

    for (int i = 0; i < number_; i++) {
        offset += PopOctets4((buffer + offset), payload);
        size_t pos;
        size_t attrNum = attributes_.size();
        for (pos = 0; pos < attrNum; pos++) {
            if (attributes_.at(pos) == payload) {
                break;
            }
        }
        if (pos == attrNum) {
            attributes_.push_back(static_cast<uint32_t>(payload));
            LOG_DEBUG("[AVRCP CT] attributeId[%{public}d]", attributes_.back());
        } else {
            LOG_DEBUG("[AVRCP CT] attributeId[%{public}d]", attributes_[pos]);
        }
        offset += PopOctets2((buffer + offset), payload);
        auto characterSetID = static_cast<uint16_t>(payload);
        offset += PopOctets2((buffer + offset), payload);
        AttributeValueLength_ = static_cast<uint16_t>(payload);
        LOG_DEBUG("[AVRCP CT] characterSetID[%hu] AttributeValueLength_[%hu]", characterSetID, AttributeValueLength_);
        char tempName[AttributeValueLength_ + 1];
        if (memset_s(tempName, AttributeValueLength_ + 1, 0, AttributeValueLength_ + 1) != EOK) {
            return false;
        };
        int j;
        for (j = 0; j < AttributeValueLength_; j++) {
            payload = 0x00;
            offset += PopOctets1((buffer + offset), payload);
            LOG_DEBUG("[AVRCP CT] offset[%{public}d]", offset);
            tempName[j] = static_cast<char>(payload);
            if (offset >= parameterLength_ + AVRC_CT_VENDOR_MIN_COMMAND_SIZE + AVRC_CT_VENDOR_PARAMETER_LENGTH_SIZE ||
                offset >= AVRC_CT_GEA_MTU) {
                j++;
                break;
            }
        }
        tempName[j] = '\0';
        std::string name(tempName);
        if (pos == attrNum) {
            values_.push_back(name);
            LOG_DEBUG("[AVRCP CT] AttributeValue[%{public}s]", values_.back().c_str());
        } else {
            values_.at(pos).append(name);
            LOG_DEBUG("[AVRCP CT] AttributeValue[%{public}s]", values_.at(pos).c_str());
        }
    }
    return true;
}

Packet *AvrcCtGeaPacket::AssembleParameters(Packet *pkt)
{
    LOG_DEBUG("[AVRCP CT] AvrcCtGeaPacket::%{public}s", __func__);

    size_t bufferSize = AVRC_CT_VENDOR_PARAMETER_LENGTH_SIZE + AVRC_CT_GEA_IDENTIFIER_SIZE +
                        AVRC_CT_GEA_ATTRIBUTE_COUNT_SIZE + number_ * AVRC_CT_GEA_ATTRIBUTE_SIZE;
    LOG_DEBUG("[AVRCP CT] BufferMalloc[%{public}d]", bufferSize);
    auto buffer = BufferMalloc(bufferSize);
    auto bufferPtr = static_cast<uint8_t *>(BufferPtr(buffer));

    uint16_t offset = 0x0000;
    parameterLength_ = bufferSize - AVRC_CT_VENDOR_PARAMETER_LENGTH_SIZE;

    offset += PushOctets2((bufferPtr + offset), parameterLength_);
    LOG_DEBUG("[AVRCP CT] parameterLength_[%hu]", parameterLength_);

    offset += PushOctets8((bufferPtr + offset), identifier_);
    LOG_DEBUG("[AVRCP CT] identifier_[%llx]", identifier_);

    offset += PushOctets1((bufferPtr + offset), number_);
    LOG_DEBUG("[AVRCP CT] number_[%hhu]", number_);

    for (auto attribute : attributes_) {
        offset += PushOctets4((bufferPtr + offset), attribute);
        LOG_DEBUG("[AVRCP CT] attribute[%x]", attribute);
    }

    PacketPayloadAddLast(pkt, buffer);

    BufferFree(buffer);
    buffer = nullptr;
    bufferPtr = nullptr;

    return pkt;
}

bool AvrcCtGeaPacket::DisassembleParameters(uint8_t *buffer)
{
    LOG_DEBUG("[AVRCP CT] AvrcCtGeaPacket::%{public}s", __func__);

    isValid_ = false;
    receivedFragments_++;
    LOG_DEBUG("[AVRCP CT] receivedFragments_[%{public}d]", receivedFragments_);
    do {
        uint16_t offset = AVRC_CT_VENDOR_PARAMETER_LENGTH_OFFSET;
        uint64_t payload = 0x00;
        offset += PopOctets2((buffer + offset), payload);
        parameterLength_ = static_cast<uint16_t>(payload);
        LOG_DEBUG("[AVRCP CT] parameterLength_[%{public}d]", parameterLength_);

        offset += PopOctets1((buffer + offset), payload);
        number_ = static_cast<uint8_t>(payload);
        LOG_DEBUG("[AVRCP CT] number_[%{public}d]", number_);

        DisassemblePacketAttributes(buffer, offset);

        numOfAttributes_ = attributes_.size();
        numOfValues_ = values_.size();

        LOG_DEBUG("[AVRCP CT] values_.size[%{public}d]", values_.size());

        if (numOfValues_ > 0) {
            isValid_ = true;
        }
    } while (0);

    LOG_DEBUG("[AVRCP CT] result[%{public}d]", isValid_);

    return isValid_;
}

/******************************************************************
 * GetPlayStatus                                                  *
 ******************************************************************/

AvrcCtGpsPacket::AvrcCtGpsPacket()
    : AvrcCtVendorPacket(),
      songLength_(AVRC_PLAY_STATUS_INVALID_SONG_LENGTH),
      songPosition_(AVRC_PLAY_STATUS_INVALID_SONG_POSITION),
      playStatus_(AVRC_PLAY_STATUS_ERROR)
{
    LOG_DEBUG("[AVRCP CT] AvrcCtGpsPacket::%{public}s", __func__);

    crCode_ = AVRC_CT_CMD_CODE_STATUS;
    pduId_ = AVRC_CT_PDU_ID_GET_PLAY_STATUS;
    parameterLength_ = AVRC_CT_GPS_PARAMETER_LENGTH;
}

AvrcCtGpsPacket::AvrcCtGpsPacket(Packet *pkt)
    : AvrcCtVendorPacket(),
      songLength_(AVRC_PLAY_STATUS_INVALID_SONG_LENGTH),
      songPosition_(AVRC_PLAY_STATUS_INVALID_SONG_POSITION),
      playStatus_(AVRC_PLAY_STATUS_ERROR)
{
    LOG_DEBUG("[AVRCP CT] AvrcCtGpsPacket::%{public}s", __func__);

    crCode_ = AVRC_CT_CMD_CODE_STATUS;
    pduId_ = AVRC_CT_PDU_ID_GET_PLAY_STATUS;

    DisassemblePacket(pkt);
}

AvrcCtGpsPacket::~AvrcCtGpsPacket(void)
{
    LOG_DEBUG("[AVRCP CT] AvrcCtGpsPacket::%{public}s", __func__);
}

Packet *AvrcCtGpsPacket::AssembleParameters(Packet *pkt)
{
    LOG_DEBUG("[AVRCP CT] AvrcCtGpsPacket::%{public}s", __func__);

    size_t bufferSize = AVRC_CT_VENDOR_PARAMETER_LENGTH_SIZE + AVRC_CT_GPS_PARAMETER_LENGTH;
    LOG_DEBUG("[AVRCP CT] BufferMalloc[%{public}d]", bufferSize);

    auto buffer = BufferMalloc(bufferSize);
    auto bufferPtr = static_cast<uint8_t *>(BufferPtr(buffer));

    uint16_t offset = 0x0000;
    offset += PushOctets2((bufferPtr + offset), parameterLength_);
    LOG_DEBUG("[AVRCP CT] parameterLength_[%{public}d]", parameterLength_);

    PacketPayloadAddLast(pkt, buffer);

    BufferFree(buffer);
    buffer = nullptr;
    bufferPtr = nullptr;

    return pkt;
}

bool AvrcCtGpsPacket::DisassembleParameters(uint8_t *buffer)
{
    LOG_DEBUG("[AVRCP CT] AvrcCtGpsPacket::%{public}s", __func__);

    isValid_ = false;

    uint16_t offset = AVRC_CT_VENDOR_PARAMETER_LENGTH_OFFSET;
    uint64_t payload = 0x00;
    offset += PopOctets2((buffer + offset), payload);
    parameterLength_ = static_cast<uint16_t>(payload);
    LOG_DEBUG("[AVRCP CT] parameterLength_[%{public}d]", parameterLength_);

    switch (parameterLength_) {
        case AVRC_CT_VENDOR_PARAMETER_LENGTH:
            songLength_ = AVRC_PLAY_STATUS_INVALID_SONG_LENGTH;
            LOG_DEBUG("[AVRCP CT] songLength_[%{public}d]", songLength_);

            songPosition_ = AVRC_PLAY_STATUS_INVALID_SONG_POSITION;
            LOG_DEBUG("[AVRCP CT] songPosition_[%{public}d]", songPosition_);

            playStatus_ = AVRC_PLAY_STATUS_ERROR;
            LOG_DEBUG("[AVRCP CT] playStatus_[%x]", playStatus_);
            break;
        default:
            offset = AVRC_CT_GPS_SONG_LENGTH_OFFSET;
            payload = 0x00;
            offset += PopOctets4((buffer + offset), payload);
            songLength_ = static_cast<uint32_t>(payload);
            LOG_DEBUG("[AVRCP CT] songLength_[%{public}d]", songLength_);

            payload = 0x00;
            offset += PopOctets4((buffer + offset), payload);
            songPosition_ = static_cast<uint32_t>(payload);
            LOG_DEBUG("[AVRCP CT] songPosition_[%{public}d]", songPosition_);

            payload = 0x00;
            offset += PopOctets1((buffer + offset), payload);
            playStatus_ = static_cast<uint8_t>(payload);
            LOG_DEBUG("[AVRCP CT] playStatus_[%x]", playStatus_);

            if (crCode_ == AVRC_CT_RSP_CODE_STABLE || crCode_ == AVRC_CT_RSP_CODE_INTERIM) {
                isValid_ = true;
            }
            break;
    }

    return isValid_;
}

/******************************************************************
 * RequestContinuingResponse                                      *
 ******************************************************************/

AvrcCtRcrPacket::AvrcCtRcrPacket() : AvrcCtVendorPacket()
{
    LOG_DEBUG("[AVRCP CT] AvrcCtRcrPacket::%{public}s", __func__);

    crCode_ = AVRC_CT_CMD_CODE_CONTROL;
    pduId_ = AVRC_CT_PDU_ID_REQUEST_CONTINUING_RESPONSE;
    parameterLength_ = AVRC_CT_RCR_PARAMETER_LENGTH;
}

AvrcCtRcrPacket::AvrcCtRcrPacket(uint8_t pduId) : AvrcCtVendorPacket()
{
    LOG_DEBUG("[AVRCP CT] AvrcCtRcrPacket::%{public}s", __func__);

    crCode_ = AVRC_CT_CMD_CODE_CONTROL;
    pduId_ = AVRC_CT_PDU_ID_REQUEST_CONTINUING_RESPONSE;
    parameterLength_ = AVRC_CT_RCR_PARAMETER_LENGTH;
    continuePduId_ = pduId;
}

AvrcCtRcrPacket::~AvrcCtRcrPacket()
{
    LOG_DEBUG("[AVRCP CT] AvrcCtRcrPacket::%{public}s", __func__);
}

Packet *AvrcCtRcrPacket::AssembleParameters(Packet *pkt)
{
    LOG_DEBUG("[AVRCP CT] AvrcCtRcrPacket::%{public}s", __func__);

    auto buffer = BufferMalloc(AVRC_CT_VENDOR_PARAMETER_LENGTH_SIZE + AVRC_CT_RCR_PARAMETER_LENGTH);
    auto bufferPtr = static_cast<uint8_t *>(BufferPtr(buffer));

    uint16_t offset = 0x0000;
    offset += PushOctets2((bufferPtr + offset), parameterLength_);
    LOG_DEBUG("[AVRCP CT] parameterLength_[%{public}d]", parameterLength_);

    offset += PushOctets1((bufferPtr + offset), continuePduId_);
    LOG_DEBUG("[AVRCP CT] continuePduId_[%x]", continuePduId_);

    PacketPayloadAddLast(pkt, buffer);

    BufferFree(buffer);
    buffer = nullptr;
    bufferPtr = nullptr;

    return pkt;
}

bool AvrcCtRcrPacket::DisassembleParameters(uint8_t *buffer)
{
    LOG_DEBUG("[AVRCP CT] AvrcCtRcrPacket::%{public}s", __func__);

    isValid_ = false;

    return isValid_;
}

/******************************************************************
 * AbortContinuingResponse                                        *
 ******************************************************************/

AvrcCtAcrPacket::AvrcCtAcrPacket() : AvrcCtVendorPacket()
{
    LOG_DEBUG("[AVRCP CT] AvrcCtAcrPacket::%{public}s", __func__);

    crCode_ = AVRC_CT_CMD_CODE_CONTROL;
    pduId_ = AVRC_CT_PDU_ID_ABORT_CONTINUING_RESPONSE;
    parameterLength_ = AVRC_CT_ACR_PARAMETER_LENGTH;
}

AvrcCtAcrPacket::AvrcCtAcrPacket(uint8_t pduId) : AvrcCtVendorPacket()
{
    LOG_DEBUG("[AVRCP CT] AvrcCtAcrPacket::%{public}s", __func__);

    crCode_ = AVRC_CT_CMD_CODE_CONTROL;
    pduId_ = AVRC_CT_PDU_ID_ABORT_CONTINUING_RESPONSE;
    parameterLength_ = AVRC_CT_ACR_PARAMETER_LENGTH;
    continuePduId_ = pduId;
}

AvrcCtAcrPacket::AvrcCtAcrPacket(Packet *pkt) : AvrcCtVendorPacket()
{
    LOG_DEBUG("[AVRCP CT] AvrcCtAcrPacket::%{public}s", __func__);

    crCode_ = AVRC_CT_CMD_CODE_CONTROL;
    pduId_ = AVRC_CT_PDU_ID_ABORT_CONTINUING_RESPONSE;

    DisassemblePacket(pkt);
}

AvrcCtAcrPacket::~AvrcCtAcrPacket()
{
    LOG_DEBUG("[AVRCP CT] AvrcCtAcrPacket::%{public}s", __func__);
}

Packet *AvrcCtAcrPacket::AssembleParameters(Packet *pkt)
{
    LOG_DEBUG("[AVRCP CT] AvrcCtAcrPacket::%{public}s", __func__);

    auto buffer = BufferMalloc(AVRC_CT_VENDOR_PARAMETER_LENGTH_SIZE + AVRC_CT_ACR_PARAMETER_LENGTH);
    auto bufferPtr = static_cast<uint8_t *>(BufferPtr(buffer));

    uint16_t offset = 0x0000;
    offset += PushOctets2((bufferPtr + offset), parameterLength_);
    LOG_DEBUG("[AVRCP CT] parameterLength_[%{public}d]", parameterLength_);

    offset += PushOctets1((bufferPtr + offset), continuePduId_);
    LOG_DEBUG("[AVRCP CT] continuePduId_[%x]", continuePduId_);

    PacketPayloadAddLast(pkt, buffer);

    BufferFree(buffer);
    buffer = nullptr;
    bufferPtr = nullptr;

    return pkt;
}

bool AvrcCtAcrPacket::DisassembleParameters(uint8_t *buffer)
{
    LOG_DEBUG("[AVRCP CT] AvrcCtAcrPacket::%{public}s", __func__);

    isValid_ = false;

    uint16_t offset = AVRC_CT_VENDOR_PARAMETER_LENGTH_OFFSET;
    uint64_t payload = 0x00;
    offset += PopOctets2((buffer + offset), payload);
    parameterLength_ = static_cast<uint16_t>(payload);
    LOG_DEBUG("[AVRCP CT] parameterLength_[%{public}d]", parameterLength_);

    isValid_ = true;

    return isValid_;
}

/******************************************************************
 * SetAddressedPlayer                                             *
 ******************************************************************/

AvrcCtSapPacket::AvrcCtSapPacket(uint16_t playerId) : AvrcCtVendorPacket()
{
    LOG_DEBUG("[AVRCP CT] AvrcCtSapPacket::%{public}s", __func__);

    crCode_ = AVRC_CT_CMD_CODE_CONTROL;
    pduId_ = AVRC_CT_PDU_ID_SET_ADDRESSED_PLAYER;
    parameterLength_ = AVRC_CT_SAP_PARAMETER_LENGTH;
    playerId_ = playerId;
}

AvrcCtSapPacket::AvrcCtSapPacket(Packet *pkt) : AvrcCtVendorPacket()
{
    LOG_DEBUG("[AVRCP CT] AvrcCtSapPacket::%{public}s", __func__);

    crCode_ = AVRC_CT_CMD_CODE_CONTROL;
    pduId_ = AVRC_CT_PDU_ID_SET_ADDRESSED_PLAYER;

    DisassemblePacket(pkt);
}

AvrcCtSapPacket::~AvrcCtSapPacket()
{
    LOG_DEBUG("[AVRCP CT] AvrcCtSapPacket::%{public}s", __func__);
}

Packet *AvrcCtSapPacket::AssembleParameters(Packet *pkt)
{
    LOG_DEBUG("[AVRCP CT] AvrcCtSapPacket::%{public}s", __func__);

    auto buffer = BufferMalloc(AVRC_CT_VENDOR_PARAMETER_LENGTH_SIZE + AVRC_CT_SAP_PARAMETER_LENGTH);
    auto bufferPtr = static_cast<uint8_t *>(BufferPtr(buffer));
    LOG_DEBUG("[AVRCP CT] BufferMalloc[%llu]", (AVRC_CT_VENDOR_PARAMETER_LENGTH_SIZE + AVRC_CT_SAP_PARAMETER_LENGTH));

    uint16_t offset = 0x0000;
    offset += PushOctets2((bufferPtr + offset), parameterLength_);
    LOG_DEBUG("[AVRCP CT] parameterLength_[%{public}d]", parameterLength_);

    offset += PushOctets2((bufferPtr + offset), playerId_);
    LOG_DEBUG("[AVRCP CT] playerId_[%{public}d]", playerId_);

    PacketPayloadAddLast(pkt, buffer);

    BufferFree(buffer);
    buffer = nullptr;
    bufferPtr = nullptr;

    return pkt;
}

bool AvrcCtSapPacket::DisassembleParameters(uint8_t *buffer)
{
    LOG_DEBUG("[AVRCP CT] AvrcCtSapPacket::%{public}s", __func__);

    isValid_ = false;

    uint16_t offset = AVRC_CT_VENDOR_PARAMETER_LENGTH_OFFSET;
    uint64_t payload = 0x00;
    offset += PopOctets2((buffer + offset), payload);
    parameterLength_ = static_cast<uint16_t>(payload);
    LOG_DEBUG("[AVRCP CT] parameterLength_[%{public}d]", parameterLength_);

    offset += PopOctets1((buffer + offset), payload);
    status_ = static_cast<uint8_t>(payload);
    LOG_DEBUG("[AVRCP CT] status_[%x]", status_);

    isValid_ = true;

    return isValid_;
}

/******************************************************************
 * PlayItem                                                       *
 ******************************************************************/

AvrcCtPiPacket::AvrcCtPiPacket(uint8_t scope, uint64_t uid, uint16_t uidCounter)
    : AvrcCtVendorPacket(), scope_(AVRC_MEDIA_SCOPE_INVALID)
{
    LOG_DEBUG("[AVRCP CT] AvrcCtPiPacket::%{public}s", __func__);

    crCode_ = AVRC_CT_CMD_CODE_CONTROL;
    pduId_ = AVRC_CT_PDU_ID_PLAY_ITEM;
    parameterLength_ = AVRC_CT_PI_PARAMETER_LENGTH;
    scope_ = scope;
    uid_ = uid;
    uidCounter_ = uidCounter;
}

AvrcCtPiPacket::AvrcCtPiPacket(Packet *pkt) : AvrcCtVendorPacket()
{
    LOG_DEBUG("[AVRCP CT] AvrcCtPiPacket::%{public}s", __func__);

    crCode_ = AVRC_CT_CMD_CODE_CONTROL;
    pduId_ = AVRC_CT_PDU_ID_PLAY_ITEM;

    DisassemblePacket(pkt);
}

AvrcCtPiPacket::~AvrcCtPiPacket(void)
{
    LOG_DEBUG("[AVRCP CT] AvrcCtPiPacket::%{public}s", __func__);
}

Packet *AvrcCtPiPacket::AssembleParameters(Packet *pkt)
{
    LOG_DEBUG("[AVRCP CT] AvrcCtPiPacket::%{public}s", __func__);

    size_t bufferSize = AVRC_CT_VENDOR_PARAMETER_LENGTH_SIZE + AVRC_CT_PI_PARAMETER_LENGTH;
    LOG_DEBUG("[AVRCP CT] BufferMalloc[%{public}d]", bufferSize);

    auto buffer = BufferMalloc(bufferSize);
    auto bufferPtr = static_cast<uint8_t *>(BufferPtr(buffer));

    uint16_t offset = 0x00;
    offset += PushOctets2((bufferPtr + offset), parameterLength_);
    LOG_DEBUG("[AVRCP CT] parameterLength_[%{public}d]", parameterLength_);

    offset += PushOctets1((bufferPtr + offset), scope_);
    LOG_DEBUG("[AVRCP CT] scope_[%x]", scope_);

    offset += PushOctets8((bufferPtr + offset), uid_);
    LOG_DEBUG("[AVRCP CT] uid_[%llx]", uid_);

    offset += PushOctets2((bufferPtr + offset), uidCounter_);
    LOG_DEBUG("[AVRCP CT] uidCounter_[%{public}d]", uidCounter_);

    PacketPayloadAddLast(pkt, buffer);

    BufferFree(buffer);
    buffer = nullptr;
    bufferPtr = nullptr;

    return pkt;
}

bool AvrcCtPiPacket::DisassembleParameters(uint8_t *buffer)
{
    LOG_DEBUG("[AVRCP CT] AvrcCtPiPacket::%{public}s", __func__);

    isValid_ = false;

    uint16_t offset = AVRC_CT_VENDOR_PARAMETER_LENGTH_OFFSET;
    uint64_t payload = 0x00;
    offset += PopOctets2((buffer + offset), payload);
    parameterLength_ = static_cast<uint16_t>(payload);
    LOG_DEBUG("[AVRCP CT] parameterLength_[%{public}d]", parameterLength_);

    offset += PopOctets1((buffer + offset), payload);
    status_ = static_cast<uint8_t>(payload);
    LOG_DEBUG("[AVRCP CT] status_[%x]", status_);

    isValid_ = true;

    return isValid_;
}

/******************************************************************
 * AddToNowPlaying                                                *
 ******************************************************************/

AvrcCtAtnpPacket::AvrcCtAtnpPacket(uint8_t scope, uint64_t uid, uint16_t uidCounter)
    : AvrcCtVendorPacket(), scope_(AVRC_MEDIA_SCOPE_INVALID)
{
    LOG_DEBUG("[AVRCP CT] AvrcCtAtnpPacket::%{public}s", __func__);

    crCode_ = AVRC_CT_CMD_CODE_CONTROL;
    pduId_ = AVRC_CT_PDU_ID_ADD_TO_NOW_PLAYING;
    parameterLength_ = AVRC_CT_ATNP_PARAMETER_LENGTH;

    scope_ = scope;
    uid_ = uid;
    uidCounter_ = uidCounter;
}

AvrcCtAtnpPacket::AvrcCtAtnpPacket(Packet *pkt) : AvrcCtVendorPacket(), scope_(AVRC_MEDIA_SCOPE_INVALID)
{
    LOG_DEBUG("[AVRCP CT] AvrcCtAtnpPacket::%{public}s", __func__);

    crCode_ = AVRC_CT_CMD_CODE_CONTROL;
    pduId_ = AVRC_CT_PDU_ID_ADD_TO_NOW_PLAYING;

    DisassemblePacket(pkt);
}

AvrcCtAtnpPacket::~AvrcCtAtnpPacket()
{
    LOG_DEBUG("[AVRCP CT] AvrcCtAtnpPacket::%{public}s", __func__);
}

Packet *AvrcCtAtnpPacket::AssembleParameters(Packet *pkt)
{
    LOG_DEBUG("[AVRCP CT] AvrcCtAtnpPacket::%{public}s", __func__);

    auto buffer = BufferMalloc(AVRC_CT_VENDOR_PARAMETER_LENGTH_SIZE + AVRC_CT_ATNP_PARAMETER_LENGTH);
    auto bufferPtr = static_cast<uint8_t *>(BufferPtr(buffer));

    uint16_t offset = 0x0000;
    offset += PushOctets2((bufferPtr + offset), parameterLength_);
    LOG_DEBUG("[AVRCP CT] parameterLength_[%{public}d]", parameterLength_);

    offset += PushOctets1((bufferPtr + offset), scope_);
    LOG_DEBUG("[AVRCP CT] scope_[%hhx]", status_);

    offset += PushOctets8((bufferPtr + offset), uid_);
    LOG_DEBUG("[AVRCP CT] uid_[%llx]", uid_);

    offset += PushOctets2((bufferPtr + offset), uidCounter_);
    LOG_DEBUG("[AVRCP CT] uidCounter_[%hu]", uidCounter_);

    PacketPayloadAddLast(pkt, buffer);

    BufferFree(buffer);
    buffer = nullptr;
    bufferPtr = nullptr;

    return pkt;
}

bool AvrcCtAtnpPacket::DisassembleParameters(uint8_t *buffer)
{
    LOG_DEBUG("[AVRCP CT] AvrcCtAtnpPacket::%{public}s", __func__);

    isValid_ = false;

    uint16_t offset = AVRC_CT_VENDOR_PARAMETER_LENGTH_OFFSET;
    uint64_t payload = 0x00;
    offset += PopOctets2((buffer + offset), payload);
    parameterLength_ = static_cast<uint16_t>(payload);
    LOG_DEBUG("[AVRCP CT] parameterLength_[%{public}d]", parameterLength_);

    offset += PopOctets1((buffer + offset), payload);
    status_ = static_cast<uint8_t>(payload);
    LOG_DEBUG("[AVRCP CT] status_[%x]", status_);

    isValid_ = true;

    return isValid_;
}

/******************************************************************
 * SetAbsoluteVolume                                              *
 ******************************************************************/

AvrcCtSavPacket::AvrcCtSavPacket(uint8_t volume) : AvrcCtVendorPacket(), volume_(AVRC_ABSOLUTE_VOLUME_INVALID)
{
    LOG_DEBUG("[AVRCP CT] AvrcCtSavPacket::%{public}s", __func__);

    crCode_ = AVRC_CT_CMD_CODE_CONTROL;
    pduId_ = AVRC_CT_PDU_ID_SET_ABSOLUTE_VOLUME;
    parameterLength_ = AVRC_CT_SAV_PARAMETER_LENGTH;
    volume_ = volume;
}

AvrcCtSavPacket::AvrcCtSavPacket(Packet *pkt) : AvrcCtVendorPacket(), volume_(AVRC_ABSOLUTE_VOLUME_INVALID)
{
    LOG_DEBUG("[AVRCP CT] AvrcCtSavPacket::%{public}s", __func__);

    crCode_ = AVRC_CT_CMD_CODE_CONTROL;
    pduId_ = AVRC_CT_PDU_ID_SET_ABSOLUTE_VOLUME;

    DisassemblePacket(pkt);
}

AvrcCtSavPacket::~AvrcCtSavPacket()
{
    LOG_DEBUG("[AVRCP CT] AvrcCtSavPacket::%{public}s", __func__);
}

Packet *AvrcCtSavPacket::AssembleParameters(Packet *pkt)
{
    LOG_DEBUG("[AVRCP CT] AvrcCtSavPacket::%{public}s", __func__);

    size_t bufferSize = AVRC_CT_VENDOR_PARAMETER_LENGTH_SIZE + AVRC_CT_SAV_PARAMETER_LENGTH;
    LOG_DEBUG("[AVRCP CT] BufferMalloc[%{public}d]", bufferSize);

    auto buffer = BufferMalloc(bufferSize);
    auto bufferPtr = static_cast<uint8_t *>(BufferPtr(buffer));

    uint16_t offset = 0x0000;
    offset += PushOctets2((bufferPtr + offset), parameterLength_);
    LOG_DEBUG("[AVRCP CT] parameterLength_[%{public}d]", parameterLength_);

    offset += PushOctets1((bufferPtr + offset), volume_ & 0b01111111);
    LOG_DEBUG("[AVRCP CT] volume_[%x]", volume_);

    PacketPayloadAddLast(pkt, buffer);

    BufferFree(buffer);
    buffer = nullptr;
    bufferPtr = nullptr;

    return pkt;
}

bool AvrcCtSavPacket::DisassembleParameters(uint8_t *buffer)
{
    LOG_DEBUG("[AVRCP CT] AvrcCtSavPacket::%{public}s", __func__);

    isValid_ = false;

    uint16_t offset = AVRC_CT_VENDOR_PARAMETER_LENGTH_OFFSET;
    uint64_t payload = 0x00;
    offset += PopOctets2((buffer + offset), payload);
    parameterLength_ = static_cast<uint16_t>(payload);
    LOG_DEBUG("[AVRCP CT] parameterLength_[%{public}d]", parameterLength_);

    offset += PopOctets1((buffer + offset), payload);
    volume_ = static_cast<uint8_t>(payload) & 0b01111111;
    LOG_DEBUG("[AVRCP CT] volume_[%x]", volume_);

    isValid_ = true;

    return isValid_;
}
}  // namespace bluetooth
