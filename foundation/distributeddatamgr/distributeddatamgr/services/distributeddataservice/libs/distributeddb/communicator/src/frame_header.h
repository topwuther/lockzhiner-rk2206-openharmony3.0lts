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

#ifndef FRAMEHEADER_H
#define FRAMEHEADER_H

#include <cstdint>
#include "communicator_type_define.h"

namespace DistributedDB {
/*
 * packetType: Bit0:   FragmentFlag: 1: Fragmented 0: Not Fragmented
 *             Bit1~3: Reserved
 *             Bit4~7: FrameType
 */
struct CommPhyHeader {
    uint16_t magic;         // Magic code to discern byte stream
    uint16_t version;       // Version to differentiate fields layout
    uint32_t packetLen;     // Length of total packet, include CommHeader and Padding
    uint64_t checkSum;      // Check sum of data that follows CommPhyHeader
    uint64_t sourceId;      // Indicate where this packet from
    uint32_t frameId;       // FrameId to identify frame
    uint8_t packetType;     // Some bits works individually, the high four bits indicates frameType
    uint8_t paddingLen;     // Unit byte, range from 0 to 7.
    uint16_t dbIntVer;      // Auxiliary info to help recognize db version in the future
};

/*
 * Whether a physical packet contains CommPhyOptHeader depend on FragmentFlag of packetType in CommPhyHeader
 */
struct CommPhyOptHeader {
    uint32_t frameLen;      // Indicate length of frame before fragmentation. Frame include CommHeader no padding
    uint16_t fragCount;     // Indicate how many fragments this frame is divided into
    uint16_t fragNo;        // Indicate which fragment this packet is. start from 0.
};

/*
 * Whether a physical packet contains CommDivergeHeader depend on FrameType of packetType in CommPhyHeader
 */
struct CommDivergeHeader {
    uint16_t version;       // Version to differentiate fields layout
    uint16_t reserved;      // Reserved for future usage
    uint32_t payLoadLen;    // Indicate length of data that follows CommDivergeHeader
    uint8_t commLabel[COMM_LABEL_LENGTH]; // Indicate which communicator to hand out this frame
};

/*
 * MessageHeader used to describe a message
 */
struct MessageHeader {
    uint16_t version;       // Version to differentiate fields layout
    uint16_t messageType;   // Distinguish request/response/notify
    uint32_t messageId;     // Indicate message command
    uint32_t sessionId;     // For matching request and response
    uint32_t sequenceId;    // Sequence of message
    uint32_t errorNo;       // Indicate no error when zero
    uint32_t dataLen;       // Indicate length of data that follows MessageHeader
};
} // namespace DistributedDB

#endif // FRAMEHEADER_H
