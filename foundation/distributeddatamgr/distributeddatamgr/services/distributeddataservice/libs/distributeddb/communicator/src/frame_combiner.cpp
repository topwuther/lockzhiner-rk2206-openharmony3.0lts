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

#include "frame_combiner.h"
#include <set>
#include "log_print.h"
#include "protocol_proto.h"

namespace DistributedDB {
static const uint32_t MAX_WORK_PER_SRC_TARGET = 1; // Only allow 1 CombineWork for each target
static const int SURVAIL_PERIOD_IN_MILLISECOND = 10000; // Period is 10 s

void FrameCombiner::Initialize()
{
    RuntimeContext *context = RuntimeContext::GetInstance();
    TimerAction action = [this](TimerId inTimerId)->int{
        PeriodicalSurveillance();
        return E_OK;
    };
    TimerFinalizer finalizer = [this]() {
        timerRemovedIndicator_.SendSemaphore();
    };
    int errCode = context->SetTimer(SURVAIL_PERIOD_IN_MILLISECOND, action, finalizer, timerId_);
    if (errCode != E_OK) {
        LOGE("[Combiner][Init] Set timer fail, errCode=%d.", errCode);
        return;
    }
    isTimerWork_ = true;
}

void FrameCombiner::Finalize()
{
    // First: Stop the timer
    if (isTimerWork_) {
        RuntimeContext *context = RuntimeContext::GetInstance();
        context->RemoveTimer(timerId_);
        timerRemovedIndicator_.WaitSemaphore();
    }

    // Second: Clear the combineWorkPool_
    for (auto &eachSource : combineWorkPool_) {
        for (auto &eachFrame : eachSource.second) {
            delete eachFrame.second.buffer;
            eachFrame.second.buffer = nullptr;
        }
    }
}

SerialBuffer *FrameCombiner::AssembleFrameFragment(const uint8_t *bytes, uint32_t length,
    const ParseResult &inPacketInfo, ParseResult &outFrameInfo, int &outErrorNo)
{
    uint64_t sourceId = inPacketInfo.GetSourceId();
    uint32_t frameId = inPacketInfo.GetFrameId();
    std::lock_guard<std::mutex> overallLockGuard(overallMutex_);
    if (combineWorkPool_[sourceId].count(frameId) != 0) {
        // CombineWork already exist
        int errCode = ContinueExistCombineWork(bytes, length, inPacketInfo);
        if (errCode != E_OK) {
            LOGE("[Combiner][Assemble] Continue work fail, errCode=%d.", errCode);
            outErrorNo = errCode;
            return nullptr;
        }

        if (combineWorkPool_[sourceId][frameId].status.IsCombineDone()) {
            // We can parse the combined frame here, or outside this class.
            LOGI("[Combiner][Assemble] Combine done, sourceId=%llu, frameId=%u.", ULL(sourceId), frameId);
            SerialBuffer *outFrame = combineWorkPool_[sourceId][frameId].buffer;
            outFrameInfo = combineWorkPool_[sourceId][frameId].frameInfo;
            outErrorNo = E_OK;
            combineWorkPool_[sourceId].erase(frameId);
            return outFrame; // The caller is responsible for release the outFrame
        }
    } else {
        // CombineWork not exist and even existing work number reaches the limitation. Try create work first.
        int errCode = CreateNewCombineWork(bytes, length, inPacketInfo);
        if (errCode != E_OK) {
            LOGE("[Combiner][Assemble] Create work fail, errCode=%d.", errCode);
            outErrorNo = errCode;
            return nullptr;
        }
        // After successfully create work, the existing work number may exceed the limitation
        // If so, choose one from works of this target with lowest progressId and abort it
        if (combineWorkPool_[sourceId].size() > MAX_WORK_PER_SRC_TARGET) {
            AbortCombineWorkBySource(sourceId);
        }
    }
    outErrorNo = E_OK;
    return nullptr;
}

void FrameCombiner::PeriodicalSurveillance()
{
    std::lock_guard<std::mutex> overallLockGuard(overallMutex_);
    for (auto &eachSource : combineWorkPool_) {
        std::set<uint32_t> frameToAbort;
        for (auto &eachFrame : eachSource.second) {
            if (!eachFrame.second.status.CheckProgress()) {
                LOGW("[Combiner][Surveil] Source=%llu, frame=%u has no progress, this combine work will be aborted.",
                    ULL(eachSource.first), eachFrame.first);
                // Free this combine work first
                delete eachFrame.second.buffer;
                eachFrame.second.buffer = nullptr;
                // Record this frame in abort list
                frameToAbort.insert(eachFrame.first);
            }
        }
        // Remove the combine work from map
        for (auto &entry : frameToAbort) {
            eachSource.second.erase(entry);
        }
    }
}

int FrameCombiner::ContinueExistCombineWork(const uint8_t *bytes, uint32_t length, const ParseResult &inPacketInfo)
{
    uint64_t sourceId = inPacketInfo.GetSourceId();
    uint32_t frameId = inPacketInfo.GetFrameId();
    CombineWork &oriWork = combineWorkPool_[sourceId][frameId]; // Be care here must be reference
    if (!CheckPacketWithOriWork(inPacketInfo, oriWork)) {
        LOGE("[Combiner][ContinueWork] Check packet fail, sourceId=%llu, frameId=%u.", ULL(sourceId), ULL(frameId));
        return -E_COMBINE_FAIL;
    }

    uint32_t fragOffset = oriWork.status.GetThisFragmentOffset(inPacketInfo.GetFragNo());
    uint32_t fragLength = oriWork.status.GetThisFragmentLength(inPacketInfo.GetFragNo());
    int errCode = ProtocolProto::CombinePacketIntoFrame(oriWork.buffer, bytes, length, fragOffset, fragLength);
    if (errCode != E_OK) {
        // We can consider abort this work, but here we choose not to affect it
        LOGE("[Combiner][ContinueWork] Combine packet fail, sourceId=%llu, frameId=%u.", ULL(sourceId), ULL(frameId));
        return -E_COMBINE_FAIL;
    }

    oriWork.status.UpdateProgressId(incProgressId_++);
    oriWork.status.CheckInFragmentNo(inPacketInfo.GetFragNo());
    return E_OK;
}

int FrameCombiner::CreateNewCombineWork(const uint8_t *bytes, uint32_t length, const ParseResult &inPacketInfo)
{
    uint32_t fragLen = 0;
    uint32_t lastFragLen = 0;
    int errCode = ProtocolProto::AnalyzeSplitStructure(inPacketInfo, fragLen, lastFragLen);
    if (errCode != E_OK) {
        LOGE("[Combiner][CreateWork] Analyze fail, errCode=%d.", errCode);
        return errCode;
    }

    CombineWork work;

    work.frameInfo.SetPacketLen(inPacketInfo.GetFrameLen());
    work.frameInfo.SetSourceId(inPacketInfo.GetSourceId());
    work.frameInfo.SetFrameId(inPacketInfo.GetFrameId());
    work.frameInfo.SetFrameTypeInfo(inPacketInfo.GetFrameTypeInfo());
    work.frameInfo.SetFrameLen(inPacketInfo.GetFrameLen());
    work.frameInfo.SetFragCount(inPacketInfo.GetFragCount());

    work.status.SetFragmentLen(fragLen);
    work.status.SetLastFragmentLen(lastFragLen);
    work.status.SetFragmentCount(inPacketInfo.GetFragCount());

    work.buffer = CreateNewFrameBuffer(inPacketInfo);
    if (work.buffer == nullptr) {
        return -E_OUT_OF_MEMORY;
    }

    uint32_t fragOffset = work.status.GetThisFragmentOffset(inPacketInfo.GetFragNo());
    uint32_t fragLength = work.status.GetThisFragmentLength(inPacketInfo.GetFragNo());
    errCode = ProtocolProto::CombinePacketIntoFrame(work.buffer, bytes, length, fragOffset, fragLength);
    if (errCode != E_OK) {
        delete work.buffer;
        work.buffer = nullptr;
        return errCode;
    }

    totalSizeByByte_ += work.buffer->GetSize();
    work.status.UpdateProgressId(incProgressId_++);
    work.status.CheckInFragmentNo(inPacketInfo.GetFragNo());
    combineWorkPool_[inPacketInfo.GetSourceId()][inPacketInfo.GetFrameId()] = work;
    return E_OK;
}

void FrameCombiner::AbortCombineWorkBySource(uint64_t inSourceId)
{
    if (combineWorkPool_[inSourceId].size() == 0) {
        return;
    }
    uint32_t toBeAbortFrameId = 0;
    uint64_t toBeAbortProgressId = UINT64_MAX;
    for (auto &entry : combineWorkPool_[inSourceId]) {
        if (entry.second.status.GetProgressId() < toBeAbortProgressId) {
            toBeAbortProgressId = entry.second.status.GetProgressId();
            toBeAbortFrameId = entry.first;
        }
    }
    // Do Abort!
    LOGW("[Combiner][AbortWork] Abort Incomplete CombineWork, sourceId=%llu, frameId=%u.",
        ULL(inSourceId), toBeAbortFrameId);
    delete combineWorkPool_[inSourceId][toBeAbortFrameId].buffer;
    combineWorkPool_[inSourceId][toBeAbortFrameId].buffer = nullptr;
    combineWorkPool_[inSourceId].erase(toBeAbortFrameId);
}

bool FrameCombiner::CheckPacketWithOriWork(const ParseResult &inPacketInfo, const CombineWork &inWork)
{
    if (inPacketInfo.GetFrameLen() != inWork.frameInfo.GetFrameLen()) {
        LOGE("[Combiner][CheckPacket] FrameLen mismatch %u vs %u.", inPacketInfo.GetFrameLen(),
            inWork.frameInfo.GetFrameLen());
        return false;
    }
    if (inPacketInfo.GetFragCount() != inWork.frameInfo.GetFragCount()) {
        LOGE("[Combiner][CheckPacket] FragCount mismatch %u vs %u.", inPacketInfo.GetFragCount(),
            inWork.frameInfo.GetFragCount());
        return false;
    }
    if (inPacketInfo.GetFragNo() >= inPacketInfo.GetFragCount()) {
        LOGE("[Combiner][CheckPacket] FragNo=%u illegal vs FragCount=%u.", inPacketInfo.GetFragNo(),
            inPacketInfo.GetFragCount());
        return false;
    }
    if (inWork.status.IsFragNoAlreadyExist(inPacketInfo.GetFragNo())) {
        LOGE("[Combiner][CheckPacket] FragNo=%u already exist.", inPacketInfo.GetFragNo());
        return false;
    }
    return true;
}

SerialBuffer *FrameCombiner::CreateNewFrameBuffer(const ParseResult &inInfo)
{
    SerialBuffer *buffer = new (std::nothrow) SerialBuffer();
    if (buffer == nullptr) {
        return nullptr;
    }
    uint32_t frameHeaderLength = (inInfo.GetFrameTypeInfo() != FrameType::APPLICATION_MESSAGE) ?
        ProtocolProto::GetCommLayerFrameHeaderLength() : ProtocolProto::GetAppLayerFrameHeaderLength();
    int errCode = buffer->AllocBufferByTotalLength(inInfo.GetFrameLen(), frameHeaderLength);
    if (errCode != E_OK) {
        LOGE("[Combiner][CreateBuffer] Alloc Buffer Fail.");
        delete buffer;
        buffer = nullptr;
        return nullptr;
    }
    return buffer;
}
} // namespace DistributedDB