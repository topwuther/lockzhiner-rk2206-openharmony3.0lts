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

#ifndef A2DP_CODEC_WRAPPER_H
#define A2DP_CODEC_WRAPPER_H

#include <cstddef>
#include <cstdint>
#include <list>

#include "a2dp_codec_config.h"
#include "base_def.h"
#include "packet.h"

namespace bluetooth {
// A2dpEncoderObserver represent a2dp sink or source that is responsible of
// reading audio data for encoding, or enque a2dp source packet for
// transmission.
class A2dpEncoderObserver {
public:
    virtual ~A2dpEncoderObserver() = default;
    virtual uint32_t Read(uint8_t **buf, uint32_t size) = 0;
    // pktTimeStamp will be added in packet's head.
    virtual bool EnqueuePacket(const Packet *packet, size_t frames, uint32_t bytes, uint32_t pktTimeStamp) const = 0;
};

// A2dp encoder interface
class A2dpEncoder {
public:
    A2dpEncoder(A2dpCodecConfig *config, A2dpEncoderObserver *observer)
        : config_(config), observer_(observer), transmitQueueLength_(0)
    {}
    virtual ~A2dpEncoder() = default;
    virtual void ResetFeedingState(void) = 0;
    virtual void SetTransmitQueueLength(size_t length)
    {
        transmitQueueLength_ = length;
    }
    virtual void SendFrames(uint64_t timeStampUs) = 0;
    virtual void UpdateEncoderParam() = 0;

protected:
    DISALLOW_COPY_AND_ASSIGN(A2dpEncoder);
    A2dpCodecConfig *config_;
    A2dpEncoderObserver *observer_;
    size_t transmitQueueLength_;
};

// A2dpDecoderObserver is responsible of receiving decoded audio data from a2dp
// decoder
class A2dpDecoderObserver {
public:
    virtual ~A2dpDecoderObserver() = default;
    virtual void DataAvailable(uint8_t *buf, uint32_t size) = 0;
};

// A2dp decoder interface
class A2dpDecoder {
public:
    A2dpDecoder(A2dpDecoderObserver *observer) : observer_(observer)
    {}
    virtual ~A2dpDecoder() = default;
    virtual bool DecodePacket(uint8_t *data, uint16_t size) = 0;

protected:
    DISALLOW_COPY_AND_ASSIGN(A2dpDecoder);
    A2dpDecoderObserver *observer_;
};
}  // namespace bluetooth

#endif  // !A2DP_CODEC_WRAPPER_H
