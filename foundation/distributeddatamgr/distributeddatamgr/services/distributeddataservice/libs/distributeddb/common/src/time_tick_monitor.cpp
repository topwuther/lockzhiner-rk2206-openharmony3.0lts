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

#include "time_tick_monitor.h"

#include "db_errno.h"
#include "log_print.h"

namespace DistributedDB {
TimeTickMonitor::TimeTickMonitor()
    : timeChangedNotifier_(nullptr),
      runtimeCxt_(nullptr),
      monitorTimerId_(0),
      monitorCallback_(0),
      lastMonotonicTime_(0),
      lastSystemTime_(0),
      isStarted_(false)
{
}

TimeTickMonitor::~TimeTickMonitor()
{
    Stop();
    runtimeCxt_ = nullptr;
}

int TimeTickMonitor::Start()
{
    if (isStarted_) {
        return E_OK;
    }

    int errCode = PrepareNotifierChain();
    if (errCode != E_OK) {
        return errCode;
    }

    lastMonotonicTime_ = GetMonotonicTime();
    lastSystemTime_ = GetSysCurrentTime();
    monitorCallback_ = std::bind(&TimeTickMonitor::TimeTick, this, std::placeholders::_1);
    runtimeCxt_ = RuntimeContext::GetInstance();
    monitorTimerId_ = 0;
    errCode = runtimeCxt_->SetTimer(MONITOR_INTERVAL, monitorCallback_, nullptr, monitorTimerId_);
    if (errCode != E_OK) {
        return errCode;
    }
    isStarted_ = true;
    return E_OK;
}

void TimeTickMonitor::Stop()
{
    if (!isStarted_) {
        return;
    }

    timeChangedNotifier_->UnRegisterEventType(TIME_CHANGE_EVENT);
    RefObject::KillAndDecObjRef(timeChangedNotifier_);
    timeChangedNotifier_ = nullptr;
    runtimeCxt_->RemoveTimer(monitorTimerId_);
    isStarted_ = false;
}

NotificationChain::Listener *TimeTickMonitor::RegisterTimeChangedLister(const TimeChangedAction &action, int &errCode)
{
    if (timeChangedNotifier_ == nullptr) {
        errCode = -E_NOT_INIT;
        return nullptr;
    }

    if (action == nullptr) {
        errCode = -E_INVALID_ARGS;
        return nullptr;
    }

    return timeChangedNotifier_->RegisterListener(TIME_CHANGE_EVENT, action, nullptr, errCode);
}

int TimeTickMonitor::PrepareNotifierChain()
{
    std::lock_guard<std::mutex> autoLock(timeTickMonitorLock_);
    if (timeChangedNotifier_ != nullptr) {
        return E_OK;
    }

    timeChangedNotifier_ = new (std::nothrow) NotificationChain();
    if (timeChangedNotifier_ == nullptr) {
        return -E_OUT_OF_MEMORY;
    }

    int errCode = timeChangedNotifier_->RegisterEventType(TIME_CHANGE_EVENT);
    if (errCode != E_OK) {
        RefObject::KillAndDecObjRef(timeChangedNotifier_);
        timeChangedNotifier_ = nullptr;
    }
    return errCode;
}

int TimeTickMonitor::TimeTick(TimerId timerId)
{
    if (timerId != monitorTimerId_) {
        return -E_INVALID_ARGS;
    }

    uint64_t monotonicTime = GetMonotonicTime();
    uint64_t systemTime = GetSysCurrentTime();
    int64_t monotonicOffset = monotonicTime - lastMonotonicTime_;
    int64_t systemOffset = systemTime - lastSystemTime_;
    lastMonotonicTime_ = monotonicTime;
    lastSystemTime_ = systemTime;
    int64_t changedOffset = systemOffset - monotonicOffset;
    if (std::abs(changedOffset) > MAX_NOISE) {
        LOGI("Local system time may be changed! changedOffset %ld", changedOffset);
        timeChangedNotifier_->NotifyEvent(TIME_CHANGE_EVENT, &changedOffset);
    }
    return E_OK;
}

TimeStamp TimeTickMonitor::GetSysCurrentTime()
{
    uint64_t curTime = 0;
    int errCode = OS::GetCurrentSysTimeInMicrosecond(curTime);
    if (errCode != E_OK) {
        LOGE("TimeTickMonitor:get system time failed!");
        return INVALID_TIMESTAMP;
    }
    return curTime;
}

TimeStamp TimeTickMonitor::GetMonotonicTime()
{
    uint64_t time;
    int errCode = OS::GetMonotonicRelativeTimeInMicrosecond(time);
    if (errCode != E_OK) {
        LOGE("GetMonotonicTime ERR! err = %d", errCode);
        return INVALID_TIMESTAMP;
    }
    return time;
}

void TimeTickMonitor::NotifyTimeChange(TimeOffset offset) const
{
    std::lock_guard<std::mutex> lock(timeTickMonitorLock_);
    if (timeChangedNotifier_ == nullptr) {
        LOGD("NotifyTimeChange fail, timeChangedNotifier_ is null.");
        return;
    }
    timeChangedNotifier_->NotifyEvent(TIME_CHANGE_EVENT, static_cast<void *>(&offset));
}
} // namespace DistributedDB