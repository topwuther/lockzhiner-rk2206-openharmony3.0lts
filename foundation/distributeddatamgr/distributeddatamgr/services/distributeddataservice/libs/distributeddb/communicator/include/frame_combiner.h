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

#ifndef FRAME_COMBINER_H
#define FRAME_COMBINER_H

#include <map>
#include <mutex>
#include <cstdint>
#include "semaphore.h"
#include "macro_utils.h"
#include "parse_result.h"
#include "combine_status.h"
#include "runtime_context.h"

namespace DistributedDB {
class SerialBuffer; // Forward Declarations

struct CombineWork {
    SerialBuffer *buffer;
    CombineStatus status;
    ParseResult frameInfo;
};

class FrameCombiner {
public:
    FrameCombiner() = default; // Default constructor must be explicitly provided due to DISABLE_COPY_ASSIGN_MOVE
    ~FrameCombiner() = default; // Since constructor must be provided, codedex demand deconstructor be provided as well
    DISABLE_COPY_ASSIGN_MOVE(FrameCombiner);

    // Start the timer to supervise the progress
    void Initialize();
    // Clear the CombineWorkPool and stop the timer
    void Finalize();

    // outErrorNo is set E_OK if nothing error happened.
    // Return nullptr if error happened or no combination is done.
    // Return a valid buffer as well as a valid outFrameResult if combination done.
    // The caller is responsible for release the buffer.
    SerialBuffer *AssembleFrameFragment(const uint8_t *bytes, uint32_t length, const ParseResult &inPacketInfo,
        ParseResult &outFrameInfo, int &outErrorNo);

private:
    // This methed called from timer, it has overallMutex_ protect itself inside the method
    void PeriodicalSurveillance();

    // Following method should be called under protection of overallMutex_ outside the method
    int ContinueExistCombineWork(const uint8_t *bytes, uint32_t length, const ParseResult &inPacketInfo);
    int CreateNewCombineWork(const uint8_t *bytes, uint32_t length, const ParseResult &inPacketInfo);
    void AbortCombineWorkBySource(uint64_t inSourceId);

    bool CheckPacketWithOriWork(const ParseResult &inPacketInfo, const CombineWork &inWork);
    SerialBuffer *CreateNewFrameBuffer(const ParseResult &inInfo);

    mutable std::mutex overallMutex_;

    TimerId timerId_ = 0; // 0 is invalid timerId
    bool isTimerWork_ = false;
    Semaphore timerRemovedIndicator_{0};
    uint64_t incProgressId_ = 0;
    uint64_t totalSizeByByte_ = 0;
    std::map<uint64_t, std::map<uint32_t, CombineWork>> combineWorkPool_;
};
} // namespace DistributedDB

#endif // FRAME_COMBINER_H
