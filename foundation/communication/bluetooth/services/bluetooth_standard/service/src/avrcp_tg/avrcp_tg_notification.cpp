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

#include "avrcp_tg_notification.h"
#include "avrcp_tg_vendor_player_application_settings.h"

namespace bluetooth {
AvrcTgNotifyPacket::AvrcTgNotifyPacket(uint8_t eventId, uint8_t crCode, uint8_t label)
    : interval_(AVRC_PLAYBACK_INTERVAL_1_SEC), playStatus_(AVRC_PLAY_STATUS_ERROR),
      volume_(AVRC_ABSOLUTE_VOLUME_INVALID)
{
    LOG_DEBUG("[AVRCP TG] AvrcTgNotifyPacket::%{public}s", __func__);

    crCode_ = crCode;
    pduId_ = AVRC_TG_PDU_ID_REGISTER_NOTIFICATION;
    parameterLength_ = AVRC_TG_NOTIFY_PARAMETER_LENGTH;
    eventId_ = eventId;
    label_ = label;
}

AvrcTgNotifyPacket::AvrcTgNotifyPacket(Packet *pkt, uint8_t label)
    : AvrcTgVendorPacket(),
      interval_(AVRC_PLAYBACK_INTERVAL_1_SEC),
      playStatus_(AVRC_PLAY_STATUS_ERROR),
      volume_(AVRC_ABSOLUTE_VOLUME_INVALID)
{
    LOG_DEBUG("[AVRCP TG] AvrcTgNotifyPacket::%{public}s", __func__);

    crCode_ = AVRC_TG_RSP_CODE_CHANGED;
    pduId_ = AVRC_TG_PDU_ID_REGISTER_NOTIFICATION;
    label_ = label;

    DisassemblePacket(pkt);
}

AvrcTgNotifyPacket::~AvrcTgNotifyPacket()
{
    LOG_DEBUG("[AVRCP TG] AvrcTgNotifyPacket::%{public}s", __func__);

    attributes_.clear();
    values_.clear();
}

Packet *AvrcTgNotifyPacket::AssembleParameters(Packet *pkt)
{
    LOG_DEBUG("[AVRCP TG] AvrcTgNotifyPacket::%{public}s", __func__);

    switch (eventId_) {
        case AVRC_TG_EVENT_ID_PLAYBACK_STATUS_CHANGED:
            AssemblePlaybackStatusChanged(pkt);
            break;
        case AVRC_TG_EVENT_ID_TRACK_CHANGED:
            AssembleTrackChanged(pkt);
            break;
        case AVRC_TG_EVENT_ID_PLAYBACK_POS_CHANGED:
            AssemblePlaybackPosChanged(pkt);
            break;
        case AVRC_TG_EVENT_ID_PLAYER_APPLICATION_SETTING_CHANGED:
            AssemblePlayerApplicationSettingChanged(pkt);
            break;
        case AVRC_TG_EVENT_ID_ADDRESSED_PLAYER_CHANGED:
            AssembleAddressedPlayerChanged(pkt);
            break;
        case AVRC_TG_EVENT_ID_UIDS_CHANGED:
            AssembleUidsChanged(pkt);
            break;
        case AVRC_TG_EVENT_ID_VOLUME_CHANGED:
            AssembleVolumeChanged(pkt);
            break;
        case AVRC_TG_EVENT_ID_TRACK_REACHED_END:
        case AVRC_TG_EVENT_ID_TRACK_REACHED_START:
        case AVRC_TG_EVENT_ID_NOW_PLAYING_CONTENT_CHANGED:
        case AVRC_TG_EVENT_ID_AVAILABLE_PLAYERS_CHANGED: {
            AssembleCommonChanged(pkt);
            break;
        }
        default: {
            crCode_ = AVRC_TG_RSP_CODE_REJECTED;
            status_ = AVRC_ES_CODE_INVALID_PARAMETER;
            AssembleRejectPacket();
            break;
        }
    }

    return pkt;
}

void AvrcTgNotifyPacket::AssemblePlaybackStatusChanged(Packet *pkt)
{
    LOG_DEBUG("[AVRCP TG] AvrcTgNotifyPacket::%{public}s", __func__);

    auto buffer = BufferMalloc(AVRC_TG_VENDOR_PACKET_TYPE_SIZE + AVRC_TG_VENDOR_PARAMETER_LENGTH_SIZE +
                               AVRC_TG_NOTIFY_EVENT_ID_SIZE + AVRC_TG_NOTIFY_EVENT_ID_PLAYBACK_STATUS_SIZE);
    auto bufferPtr = static_cast<uint8_t *>(BufferPtr(buffer));
    LOG_DEBUG("[AVRCP TG] BufferMalloc[%llu]",
        (AVRC_TG_VENDOR_PACKET_TYPE_SIZE + AVRC_TG_VENDOR_PARAMETER_LENGTH_SIZE + AVRC_TG_NOTIFY_EVENT_ID_SIZE +
            AVRC_TG_NOTIFY_EVENT_ID_PLAYBACK_STATUS_SIZE));

    uint16_t offset = 0x0000;
    offset += PushOctets1((bufferPtr + offset), packetType_);
    LOG_DEBUG("[AVRCP TG] packetType_[%x]", packetType_);

    parameterLength_ = AVRC_TG_NOTIFY_EVENT_ID_SIZE + AVRC_TG_NOTIFY_EVENT_ID_PLAYBACK_STATUS_SIZE;
    offset += PushOctets2((bufferPtr + offset), parameterLength_);
    LOG_DEBUG("[AVRCP TG] parameterLength_[%hu]", parameterLength_);

    offset += PushOctets1((bufferPtr + offset), eventId_);
    LOG_DEBUG("[AVRCP TG] eventId_[%hhx]", eventId_);

    offset += PushOctets1((bufferPtr + offset), playStatus_);
    LOG_DEBUG("[AVRCP TG] playStatus_[%hhx]", playStatus_);

    PacketPayloadAddLast(pkt, buffer);

    BufferFree(buffer);
}

void AvrcTgNotifyPacket::AssembleTrackChanged(Packet *pkt)
{
    LOG_DEBUG("[AVRCP TG] AvrcTgNotifyPacket::%{public}s", __func__);

    auto buffer = BufferMalloc(AVRC_TG_VENDOR_PACKET_TYPE_SIZE + AVRC_TG_VENDOR_PARAMETER_LENGTH_SIZE +
                               AVRC_TG_NOTIFY_EVENT_ID_SIZE + AVRC_TG_NOTIFY_EVENT_UID_SIZE);
    auto bufferPtr = static_cast<uint8_t *>(BufferPtr(buffer));
    LOG_DEBUG("[AVRCP TG] BufferMalloc[%llu]",
        (AVRC_TG_VENDOR_PACKET_TYPE_SIZE + AVRC_TG_VENDOR_PARAMETER_LENGTH_SIZE + AVRC_TG_NOTIFY_EVENT_ID_SIZE +
            AVRC_TG_NOTIFY_EVENT_UID_SIZE));

    uint16_t offset = 0x0000;
    offset += PushOctets1((bufferPtr + offset), packetType_);
    LOG_DEBUG("[AVRCP TG] packetType_[%x]", packetType_);

    parameterLength_ = AVRC_TG_NOTIFY_EVENT_ID_SIZE + AVRC_TG_NOTIFY_EVENT_UID_SIZE;
    offset += PushOctets2((bufferPtr + offset), parameterLength_);
    LOG_DEBUG("[AVRCP TG] parameterLength_[%{public}d]", parameterLength_);

    offset += PushOctets1((bufferPtr + offset), eventId_);
    LOG_DEBUG("[AVRCP TG] eventId_[%x]", eventId_);

    offset += PushOctets8((bufferPtr + offset), uid_);
    LOG_DEBUG("[AVRCP TG] uid_[%llx]", uid_);

    PacketPayloadAddLast(pkt, buffer);

    BufferFree(buffer);
}

void AvrcTgNotifyPacket::AssemblePlaybackPosChanged(Packet *pkt)
{
    LOG_DEBUG("[AVRCP TG] AvrcTgNotifyPacket::%{public}s", __func__);

    auto buffer = BufferMalloc(AVRC_TG_VENDOR_PACKET_TYPE_SIZE + AVRC_TG_VENDOR_PARAMETER_LENGTH_SIZE +
                               AVRC_TG_NOTIFY_EVENT_ID_SIZE + AVRC_TG_NOTIFY_EVENT_PLAYBACK_POSITION_SIZE);
    auto bufferPtr = static_cast<uint8_t *>(BufferPtr(buffer));
    LOG_DEBUG("[AVRCP TG] BufferMalloc[%llu]",
        (AVRC_TG_VENDOR_PACKET_TYPE_SIZE + AVRC_TG_VENDOR_PARAMETER_LENGTH_SIZE + AVRC_TG_NOTIFY_EVENT_ID_SIZE +
            AVRC_TG_NOTIFY_EVENT_PLAYBACK_POSITION_SIZE));

    uint16_t offset = 0x0000;
    offset += PushOctets1((bufferPtr + offset), packetType_);
    LOG_DEBUG("[AVRCP TG] packetType_[%x]", packetType_);

    parameterLength_ = AVRC_TG_NOTIFY_EVENT_ID_SIZE + AVRC_TG_NOTIFY_EVENT_PLAYBACK_POSITION_SIZE;
    offset += PushOctets2((bufferPtr + offset), parameterLength_);
    LOG_DEBUG("[AVRCP TG] parameterLength_[%{public}d]", parameterLength_);

    offset += PushOctets1((bufferPtr + offset), eventId_);
    LOG_DEBUG("[AVRCP TG] eventId_[%x]", eventId_);

    offset += PushOctets4((bufferPtr + offset), playbackPos_);
    LOG_DEBUG("[AVRCP TG] playbackPos_[%x]", playbackPos_);

    PacketPayloadAddLast(pkt, buffer);

    BufferFree(buffer);
}

void AvrcTgNotifyPacket::AssemblePlayerApplicationSettingChanged(Packet *pkt)
{
    LOG_DEBUG("[AVRCP TG] AvrcTgNotifyPacket::%{public}s", __func__);

    uint16_t numOfAttributes = GetAvailableParameterSize() / AVRC_TG_OFFSET_TWO_BITS;
    if (numOfAttributes > values_.size()) {
        numOfAttributes = values_.size();
    }

    size_t bufferSize = AVRC_TG_VENDOR_PACKET_TYPE_SIZE + AVRC_TG_VENDOR_PARAMETER_LENGTH_SIZE +
                        AVRC_TG_NOTIFY_EVENT_ID_SIZE + AVRC_TG_GCPASV_NUM_OF_VALUES_SIZE + numOfAttributes +
                        numOfAttributes;
    LOG_DEBUG("[AVRCP CT] BufferMalloc[%x]", bufferSize);

    auto buffer = BufferMalloc(bufferSize);
    auto bufferPtr = static_cast<uint8_t *>(BufferPtr(buffer));

    uint16_t offset = 0x0000;
    offset += PushOctets1((bufferPtr + offset), packetType_);
    LOG_DEBUG("[AVRCP TG] packetType_[%x]", packetType_);

    parameterLength_ =
        numOfAttributes + numOfAttributes + AVRC_TG_NOTIFY_EVENT_ID_SIZE + AVRC_TG_GCPASV_NUM_OF_VALUES_SIZE;
    offset += PushOctets2((bufferPtr + offset), parameterLength_);
    LOG_DEBUG("[AVRCP TG] parameterLength_[%u]", parameterLength_);

    offset += PushOctets1((bufferPtr + offset), eventId_);
    LOG_DEBUG("[AVRCP TG] eventId_[%u]", eventId_);

    offset += PushOctets1((bufferPtr + offset), numOfAttributes);
    LOG_DEBUG("[AVRCP TG] numOfAttributes[%u]", numOfAttributes);

    for (int i = 0; i < numOfAttributes; i++) {
        offset += PushOctets1((bufferPtr + offset), attributes_.at(i));
        LOG_DEBUG("[AVRCP TG] attribute[%u]", attributes_.at(i));
        offset += PushOctets1((bufferPtr + offset), values_.at(i));
        LOG_DEBUG("[AVRCP TG] value[%x]", values_.at(i));
    }
    attributes_.erase(attributes_.begin(), attributes_.begin() + numOfAttributes);
    values_.erase(values_.begin(), values_.begin() + numOfAttributes);

    LOG_DEBUG("[AVRCP TG] attributes_.size[%{public}d]", attributes_.size());
    LOG_DEBUG("[AVRCP TG] values_.size[%{public}d]", values_.size());

    PacketPayloadAddLast(pkt, buffer);

    BufferFree(buffer);
}

void AvrcTgNotifyPacket::AssembleAddressedPlayerChanged(Packet *pkt)
{
    LOG_DEBUG("[AVRCP TG] AvrcTgNotifyPacket::%{public}s", __func__);

    size_t bufferSize = AVRC_TG_VENDOR_PACKET_TYPE_SIZE + AVRC_TG_VENDOR_PARAMETER_LENGTH_SIZE +
                        AVRC_TG_NOTIFY_EVENT_ID_SIZE + AVRC_TG_NOTIFY_EVENT_PLAYER_ID_SIZE +
                        AVRC_TG_NOTIFY_EVENT_UID_COUNTER_SIZE;
    LOG_DEBUG("[AVRCP TG] BufferMalloc[%{public}d]", bufferSize);

    auto buffer = BufferMalloc(bufferSize);
    auto bufferPtr = static_cast<uint8_t *>(BufferPtr(buffer));

    uint16_t offset = 0x0000;
    offset += PushOctets1((bufferPtr + offset), packetType_);
    LOG_DEBUG("[AVRCP TG] packetType_[%x]", packetType_);

    parameterLength_ =
        AVRC_TG_NOTIFY_EVENT_ID_SIZE + AVRC_TG_NOTIFY_EVENT_PLAYER_ID_SIZE + AVRC_TG_NOTIFY_EVENT_UID_COUNTER_SIZE;
    offset += PushOctets2((bufferPtr + offset), parameterLength_);
    LOG_DEBUG("[AVRCP TG] parameterLength_[%{public}d]", parameterLength_);

    offset += PushOctets1((bufferPtr + offset), eventId_);
    LOG_DEBUG("[AVRCP TG] eventId_[%x]", eventId_);

    offset += PushOctets2((bufferPtr + offset), playerId_);
    LOG_DEBUG("[AVRCP TG] playerId_[%x]", playerId_);

    offset += PushOctets2((bufferPtr + offset), uidCounter_);
    LOG_DEBUG("[AVRCP TG] uidCounter_[%x]", uidCounter_);

    PacketPayloadAddLast(pkt, buffer);

    BufferFree(buffer);
}

void AvrcTgNotifyPacket::AssembleUidsChanged(Packet *pkt)
{
    LOG_DEBUG("[AVRCP TG] AvrcTgNotifyPacket::%{public}s", __func__);

    auto buffer = BufferMalloc(AVRC_TG_VENDOR_PACKET_TYPE_SIZE + AVRC_TG_VENDOR_PARAMETER_LENGTH_SIZE +
                               AVRC_TG_NOTIFY_EVENT_ID_SIZE + AVRC_TG_NOTIFY_EVENT_UID_COUNTER_SIZE);
    auto bufferPtr = static_cast<uint8_t *>(BufferPtr(buffer));
    LOG_DEBUG("[AVRCP TG] BufferMalloc[%llu]",
        (AVRC_TG_VENDOR_PACKET_TYPE_SIZE + AVRC_TG_VENDOR_PARAMETER_LENGTH_SIZE + AVRC_TG_NOTIFY_EVENT_ID_SIZE +
            AVRC_TG_NOTIFY_EVENT_UID_COUNTER_SIZE));

    uint16_t offset = 0x0000;
    offset += PushOctets1((bufferPtr + offset), packetType_);
    LOG_DEBUG("[AVRCP TG] packetType_[%x]", packetType_);

    parameterLength_ = AVRC_TG_NOTIFY_EVENT_ID_SIZE + AVRC_TG_NOTIFY_EVENT_UID_COUNTER_SIZE;
    offset += PushOctets2((bufferPtr + offset), parameterLength_);
    LOG_DEBUG("[AVRCP TG] parameterLength_[%{public}d]", parameterLength_);

    offset += PushOctets1((bufferPtr + offset), eventId_);
    LOG_DEBUG("[AVRCP TG] eventId_[%x]", eventId_);

    offset += PushOctets2((bufferPtr + offset), uidCounter_);
    LOG_DEBUG("[AVRCP TG] uidCounter_[%{public}d]", uidCounter_);

    PacketPayloadAddLast(pkt, buffer);

    BufferFree(buffer);
}

void AvrcTgNotifyPacket::AssembleVolumeChanged(Packet *pkt)
{
    LOG_DEBUG("[AVRCP TG] AvrcTgNotifyPacket::%{public}s", __func__);

    auto buffer = BufferMalloc(AVRC_TG_VENDOR_PACKET_TYPE_SIZE + AVRC_TG_VENDOR_PARAMETER_LENGTH_SIZE +
                               AVRC_TG_NOTIFY_EVENT_ID_SIZE + AVRC_TG_NOTIFY_EVENT_ID_VOLUME_SIZE);
    auto bufferPtr = static_cast<uint8_t *>(BufferPtr(buffer));
    LOG_DEBUG("[AVRCP TG] BufferMalloc[%llu]",
        (AVRC_TG_VENDOR_PACKET_TYPE_SIZE + AVRC_TG_VENDOR_PARAMETER_LENGTH_SIZE + AVRC_TG_NOTIFY_EVENT_ID_SIZE +
            AVRC_TG_NOTIFY_EVENT_ID_VOLUME_SIZE));

    uint16_t offset = 0x0000;
    offset += PushOctets1((bufferPtr + offset), packetType_);
    LOG_DEBUG("[AVRCP TG] packetType_[%x]", packetType_);

    parameterLength_ = AVRC_TG_NOTIFY_EVENT_ID_SIZE + AVRC_TG_NOTIFY_EVENT_ID_VOLUME_SIZE;
    offset += PushOctets2((bufferPtr + offset), parameterLength_);
    LOG_DEBUG("[AVRCP TG] parameterLength_[%{public}d]", parameterLength_);

    offset += PushOctets1((bufferPtr + offset), eventId_);
    LOG_DEBUG("[AVRCP TG] eventId_[%x]", eventId_);

    offset += PushOctets1((bufferPtr + offset), volume_);
    LOG_DEBUG("[AVRCP TG] volume_[%x]", volume_);

    PacketPayloadAddLast(pkt, buffer);

    BufferFree(buffer);
}

void AvrcTgNotifyPacket::AssembleCommonChanged(Packet *pkt)
{
    LOG_DEBUG("[AVRCP TG] AvrcTgNotifyPacket::%{public}s", __func__);

    auto buffer = BufferMalloc(AVRC_TG_VENDOR_PACKET_TYPE_SIZE + AVRC_TG_VENDOR_PARAMETER_LENGTH_SIZE +
                               AVRC_TG_NOTIFY_PARAMETER_LENGTH + AVRC_TG_NOTIFY_EVENT_ID_SIZE);
    auto bufferPtr = static_cast<uint8_t *>(BufferPtr(buffer));
    LOG_DEBUG("[AVRCP TG] BufferMalloc[%llu]",
        (AVRC_TG_VENDOR_PACKET_TYPE_SIZE + AVRC_TG_VENDOR_PARAMETER_LENGTH_SIZE + AVRC_TG_NOTIFY_PARAMETER_LENGTH +
            AVRC_TG_NOTIFY_EVENT_ID_SIZE));

    uint16_t offset = 0x0000;
    offset += PushOctets1((bufferPtr + offset), packetType_);
    LOG_DEBUG("[AVRCP TG] packetType_[%x]", packetType_);

    parameterLength_ = AVRC_TG_NOTIFY_EVENT_ID_SIZE;
    offset += PushOctets2((bufferPtr + offset), parameterLength_);
    LOG_DEBUG("[AVRCP TG] parameterLength_[%{public}d]", parameterLength_);

    offset += PushOctets1((bufferPtr + offset), eventId_);
    LOG_DEBUG("[AVRCP TG] eventId_[%x]", eventId_);

    PacketPayloadAddLast(pkt, buffer);

    BufferFree(buffer);
}

bool AvrcTgNotifyPacket::DisassembleParameters(uint8_t *buffer)
{
    LOG_DEBUG("[AVRCP TG] AvrcTgNotifyPacket::%{public}s", __func__);

    isValid_ = false;

    do {
        uint16_t offset = AVRC_TG_VENDOR_PARAMETER_LENGTH_OFFSET;
        uint64_t payload = 0x00;
        offset += PopOctets2((buffer + offset), payload);
        parameterLength_ = static_cast<uint16_t>(payload);
        LOG_DEBUG("[AVRCP TG] parameterLength_[%{public}d]", parameterLength_);

        payload = 0x00;
        offset += PopOctets1((buffer + offset), payload);
        eventId_ = static_cast<uint8_t>(payload);
        LOG_DEBUG("[AVRCP TG] eventId_[%x]", eventId_);
        if (!IsValidEventId()) {
            crCode_ = AVRC_TG_RSP_CODE_REJECTED;
            status_ = AVRC_ES_CODE_INVALID_PARAMETER;
            AssembleRejectPacket();
            break;
        }
        if (!IsSupportedEventId()) {
            crCode_ = AVRC_TG_RSP_CODE_NOT_IMPLEMENTED;
            status_ = AVRC_ES_CODE_INVALID_PARAMETER;
            AssembleRejectPacket();
            break;
        }

        payload = 0x00;
        offset += PopOctets4((buffer + offset), payload);
        interval_ = static_cast<uint32_t>(payload);
        LOG_DEBUG("[AVRCP TG] interval_[%u]", interval_);

        crCode_ = AVRC_TG_RSP_CODE_INTERIM;

        isValid_ = true;

        LOG_DEBUG("[AVRCP TG] isValid_[%{public}d]", isValid_);
    } while (false);

    return isValid_;
}

uint16_t AvrcTgNotifyPacket::GetNumberOfPackets(void)
{
    LOG_DEBUG("[AVRCP TG] AvrcTgNotifyPacket::%{public}s", __func__);

    uint16_t numOfPkts = AVRC_TG_NOTIFY_NUM_OF_PACKETS;

    if (eventId_ == AVRC_TG_EVENT_ID_PLAYER_APPLICATION_SETTING_CHANGED) {
        double paramSize = attributes_.size() + values_.size();
        if (GetAvailableParameterSize() != 0) {
            numOfPkts = ceil(paramSize / GetAvailableParameterSize());
        }
    }
    LOG_DEBUG("[AVRCP TG] numOfPkts[%u]", numOfPkts);

    return numOfPkts;
}

uint16_t AvrcTgNotifyPacket::GetAvailableParameterSize(void) const
{
    LOG_DEBUG("[AVRCP CT] AvrcTgNotifyPacket::%{public}s", __func__);

    uint16_t remain = mtu_ - (AVRC_TG_VENDOR_FIXED_OPERAND_SIZE + AVRC_TG_GCPASV_NUM_OF_VALUES_SIZE);
    if (remain > (AVRC_TG_GCPASV_MAX_NUM_OF_ATTRIBUTES + AVRC_TG_GCPASV_MAX_NUM_OF_VALUES)) {
        remain = (AVRC_TG_GCPASV_MAX_NUM_OF_ATTRIBUTES + AVRC_TG_GCPASV_MAX_NUM_OF_VALUES);
    }
    uint16_t unitsOfParam = AVRC_TG_GCPASV_ATTRIBUTE_SIZE + AVRC_TG_GCPASV_VALUE_SIZE;

    return (remain / unitsOfParam) * unitsOfParam;
}

bool AvrcTgNotifyPacket::IsValidEventId(void) const
{
    LOG_DEBUG("[AVRCP TG] AvrcTgNotifyPacket::%{public}s", __func__);

    return ((eventId_ >= AVRC_TG_EVENT_ID_PLAYBACK_STATUS_CHANGED) && (eventId_ <= AVRC_TG_EVENT_ID_VOLUME_CHANGED));
}

bool AvrcTgNotifyPacket::IsSupportedEventId(void) const
{
    LOG_DEBUG("[AVRCP TG] AvrcTgNotifyPacket::%{public}s", __func__);

    return !(eventId_ == AVRC_TG_EVENT_ID_BATT_STATUS_CHANGED || eventId_ == AVRC_TG_EVENT_ID_SYSTEM_STATUS_CHANGED);
}
};  // namespace bluetooth