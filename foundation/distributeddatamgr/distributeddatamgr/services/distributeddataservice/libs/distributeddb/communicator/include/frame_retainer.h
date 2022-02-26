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

#ifndef FRAME_RETAINER_H
#define FRAME_RETAINER_H

#include <map>
#include <list>
#include <mutex>
#include <cstdint>
#include "macro_utils.h"
#include "runtime_context.h"

namespace DistributedDB {
class SerialBuffer; // Forward Declarations

struct FrameInfo {
    SerialBuffer *buffer;
    std::string srcTarget;
    LabelType commLabel;
    uint32_t frameId;
};

struct RetainWork {
    SerialBuffer *buffer;
    uint32_t frameId;
    uint32_t remainTime; // in second
};

class FrameRetainer {
public:
    FrameRetainer() = default; // Default constructor must be explicitly provided due to DISABLE_COPY_ASSIGN_MOVE
    ~FrameRetainer() = default; // Since constructor must be provided, codedex demand deconstructor be provided as well
    DISABLE_COPY_ASSIGN_MOVE(FrameRetainer);

    // Start the timer to clear up overtime frames
    void Initialize();
    // Stop the timer and clear the RetainWorkPool
    void Finalize();

    // Always accept the frame, which may be retained actually or perhaps discarded immediately.
    void RetainFrame(const FrameInfo &inFrame);

    // Out frames will be in the order of retention. The retainer no longer in charge of the returned frames.
    std::list<FrameInfo> FetchFramesForSpecificCommunicator(const LabelType &inCommLabel);

private:
    // This methed called from timer, it has overallMutex_ protect itself inside the method
    void PeriodicalSurveillance();

    // Following method should be called under protection of overallMutex_ outside the method
    void DiscardObsoleteFramesIfNeed();
    void ShrinkRetainWorkPool();

    mutable std::mutex overallMutex_;

    TimerId timerId_ = 0; // 0 is invalid timerId
    bool isTimerWork_ = false;

    uint32_t totalSizeByByte_ = 0;
    uint32_t totalRetainFrames_ = 0;

    uint64_t incRetainOrder_ = 0;
    std::map<LabelType, std::map<std::string, std::map<uint64_t, RetainWork>>> retainWorkPool_;
};
} // namespace DistributedDB

#endif // FRAME_RETAINER_H
