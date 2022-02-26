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

#include "sync_operation.h"

#include "db_errno.h"
#include "log_print.h"
#include "performance_analysis.h"

namespace DistributedDB {
SyncOperation::SyncOperation(uint32_t syncId, const std::vector<std::string> &devices,
    int mode, const UserCallback &userCallback, bool isBlockSync)
    : devices_(devices),
      syncId_(syncId),
      mode_(mode),
      userCallback_(userCallback),
      isBlockSync_(isBlockSync),
      isAutoSync_(false),
      isFinished_(false),
      semaphore_(nullptr)
{
}

SyncOperation::~SyncOperation()
{
    LOGD("SyncOperation::~SyncOperation()");
    Finalize();
}

int SyncOperation::Initialize()
{
    LOGD("[SyncOperation] Init SyncOperation id:%d.", syncId_);
    AutoLock lockGuard(this);
    for (const std::string &deviceId : devices_) {
        statuses_.insert(std::pair<std::string, int>(deviceId, WAITING));
    }
    if (mode_ == AUTO_PUSH) {
        mode_ = PUSH;
        isAutoSync_ = true;
    } else if (mode_ == AUTO_PULL) {
        mode_ = PULL;
        isAutoSync_ = true;
    }

    if (isBlockSync_) {
        semaphore_ = std::make_unique<Semaphore>(0);
    }

    return E_OK;
}

void SyncOperation::SetOnSyncFinalize(const OnSyncFinalize &callback)
{
    onFinalize_ = callback;
}

void SyncOperation::SetOnSyncFinished(const OnSyncFinished &callback)
{
    onFinished_ = callback;
}

void SyncOperation::SetStatus(const std::string &deviceId, int status)
{
    LOGD("[SyncOperation] SetStatus dev %s{private} status %d", deviceId.c_str(), status);
    AutoLock lockGuard(this);
    if (IsKilled()) {
        LOGE("[SyncOperation] SetStatus failed, the SyncOperation has been killed!");
        return;
    }
    if (isFinished_) {
        LOGI("[SyncOperation] SetStatus already finished");
        return;
    }

    auto iter = statuses_.find(deviceId);
    if (iter != statuses_.end()) {
        if (iter->second >= FINISHED_ALL) {
            return;
        }
        iter->second = status;
        return;
    }
}

int SyncOperation::GetStatus(const std::string &deviceId) const
{
    AutoLock lockGuard(this);
    auto iter = statuses_.find(deviceId);
    if (iter != statuses_.end()) {
        return iter->second;
    }
    return -E_INVALID_ARGS;
}

uint32_t SyncOperation::GetSyncId() const
{
    return syncId_;
}

int SyncOperation::GetMode() const
{
    return mode_;
}

void SyncOperation::Finished()
{
    {
        AutoLock lockGuard(this);
        if (IsKilled() || isFinished_) {
            return;
        }
        isFinished_ = true;
    }
    PerformanceAnalysis *performance = PerformanceAnalysis::GetInstance();
    if (performance != nullptr) {
        performance->StepTimeRecordEnd(PT_TEST_RECORDS::RECORD_ACK_RECV_TO_USER_CALL_BACK);
    }
    if (userCallback_) {
        LOGI("[SyncOperation] Sync %d finished call onComplete.", syncId_);
        userCallback_(statuses_);
    }
    if (onFinished_) {
        LOGD("[SyncOperation] Sync %d finished call onFinished.", syncId_);
        onFinished_(syncId_);
    }
}

const std::vector<std::string> &SyncOperation::GetDevices() const
{
    return devices_;
}

void SyncOperation::WaitIfNeed()
{
    if (isBlockSync_ && (semaphore_ != nullptr)) {
        LOGD("[SyncOperation] Wait.");
        semaphore_->WaitSemaphore();
    }
}

void SyncOperation::NotifyIfNeed()
{
    if (isBlockSync_ && (semaphore_ != nullptr)) {
        LOGD("[SyncOperation] Notify.");
        semaphore_->SendSemaphore();
    }
}

bool SyncOperation::IsAutoSync() const
{
    return isAutoSync_;
}

bool SyncOperation::IsBlockSync() const
{
    return isBlockSync_;
}

bool SyncOperation::CheckIsAllFinished() const
{
    AutoLock lockGuard(this);
    for (const auto &iter : statuses_) {
        if (iter.second < FINISHED_ALL) {
            return false;
        }
    }
    return true;
}

void SyncOperation::Finalize()
{
    if ((syncId_ > 0) && onFinalize_) {
        LOGD("[SyncOperation] Callback SyncOperation onFinalize.");
        onFinalize_();
    }
}

DEFINE_OBJECT_TAG_FACILITIES(SyncOperation)
} // namespace DistributedDB