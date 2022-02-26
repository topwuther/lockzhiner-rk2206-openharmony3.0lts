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

#include "send_task_scheduler.h"
#include <algorithm>
#include "db_errno.h"
#include "log_print.h"
#include "serial_buffer.h"

namespace DistributedDB {
// In current parameters, the scheduler will hold 160 MB in extreme situation.
// In actual runtime situation, the scheduler will hold no more than 100 MB.
static constexpr uint32_t MAX_CAPACITY = 67108864; // 64 M bytes
static constexpr uint32_t EXTRA_CAPACITY_FOR_NORMAL_PRIORITY = 33554432; // 32 M bytes
static constexpr uint32_t EXTRA_CAPACITY_FOR_HIGH_PRIORITY = 67108864; // 64 M bytes

SendTaskScheduler::~SendTaskScheduler()
{
    Finalize();
}

void SendTaskScheduler::Initialize()
{
    priorityOrder_.clear();
    priorityOrder_.push_back(Priority::HIGH);
    priorityOrder_.push_back(Priority::NORMAL);
    priorityOrder_.push_back(Priority::LOW);
    for (auto &prio : priorityOrder_) {
        extraCapacityInByteByPrio_[prio] = 0;
        taskCountByPrio_[prio] = 0;
        taskDelayCountByPrio_[prio] = 0;
        taskGroupByPrio_[prio] = TaskListByTarget();
    }
    extraCapacityInByteByPrio_[Priority::NORMAL] = EXTRA_CAPACITY_FOR_NORMAL_PRIORITY;
    extraCapacityInByteByPrio_[Priority::HIGH] = EXTRA_CAPACITY_FOR_HIGH_PRIORITY;
}

void SendTaskScheduler::Finalize()
{
    while (GetTotalTaskCount() != 0) {
        SendTask task;
        SendTaskInfo taskInfo;
        int errCode = ScheduleOutSendTask(task, taskInfo);
        if (errCode != E_OK) {
            LOGE("[Scheduler][Final] INTERNAL ERROR.");
            break; // Not possible to happen
        }
        LOGW("[Scheduler][Finalize] dstTarget=%s{private}, delayFlag=%d, taskPrio=%d", task.dstTarget.c_str(),
            taskInfo.delayFlag, static_cast<int>(taskInfo.taskPrio));
        FinalizeLastScheduleTask();
    }
}

int SendTaskScheduler::AddSendTaskIntoSchedule(const SendTask &inTask, Priority inPrio)
{
    std::lock_guard<std::mutex> overallLockGuard(overallMutex_);
    if (curTotalSizeByByte_ >= MAX_CAPACITY + extraCapacityInByteByPrio_[inPrio]) {
        return -E_CONTAINER_FULL;
    }

    uint32_t taskSizeByByte = inTask.buffer->GetSize();
    curTotalSizeByByte_ += taskSizeByByte;
    curTotalSizeByTask_++;
    if (policyMap_.count(inTask.dstTarget) == 0) {
        policyMap_[inTask.dstTarget] = TargetPolicy::NO_DELAY;
    }
    if (policyMap_[inTask.dstTarget] == TargetPolicy::DELAY) {
        delayTaskCount_++;
        taskDelayCountByPrio_[inPrio]++;
    }

    taskCountByPrio_[inPrio]++;
    taskOrderByPrio_[inPrio].push_back(inTask.dstTarget);
    taskGroupByPrio_[inPrio][inTask.dstTarget].push_back(inTask);
    return E_OK;
}

int SendTaskScheduler::ScheduleOutSendTask(SendTask &outTask)
{
    SendTaskInfo taskInfo;
    int errCode = ScheduleOutSendTask(outTask, taskInfo);
    if (errCode == E_OK) {
        LOGI("[Scheduler][OutTask] dstTarget=%s{private}, delayFlag=%d, taskPrio=%d", outTask.dstTarget.c_str(),
            taskInfo.delayFlag, static_cast<int>(taskInfo.taskPrio));
    }
    return errCode;
}

int SendTaskScheduler::ScheduleOutSendTask(SendTask &outTask, SendTaskInfo &outTaskInfo)
{
    std::lock_guard<std::mutex> overallLockGuard(overallMutex_);
    if (curTotalSizeByTask_ == 0) {
        return -E_CONTAINER_EMPTY;
    }

    if (delayTaskCount_ == curTotalSizeByTask_) {
        // Tasks are all in delay status
        int errCode = ScheduleDelayTask(outTask, outTaskInfo);
        if (errCode == E_OK) {
            // Update last schedule location
            lastScheduleTarget_ = outTask.dstTarget;
            lastSchedulePriority_ = outTaskInfo.taskPrio;
            scheduledFlag_ = true;
        }
        return errCode;
    } else {
        // There are some tasks not in delay status
        int errCode = ScheduleNoDelayTask(outTask, outTaskInfo);
        if (errCode == E_OK) {
            // Update last schedule location
            lastScheduleTarget_ = outTask.dstTarget;
            lastSchedulePriority_ = outTaskInfo.taskPrio;
            scheduledFlag_ = true;
        }
        return errCode;
    }
}

int SendTaskScheduler::FinalizeLastScheduleTask()
{
    std::lock_guard<std::mutex> overallLockGuard(overallMutex_);
    if (curTotalSizeByTask_ == 0) {
        return -E_CONTAINER_EMPTY;
    }
    if (!scheduledFlag_) {
        return -E_NOT_PERMIT;
    }

    // Retrieve last scheduled task
    SendTask task = taskGroupByPrio_[lastSchedulePriority_][lastScheduleTarget_].front();

    bool isFullBefore = (curTotalSizeByByte_ >= MAX_CAPACITY);
    uint32_t taskSize = task.buffer->GetSize();
    curTotalSizeByByte_ -= taskSize;
    bool isFullAfter = (curTotalSizeByByte_ >= MAX_CAPACITY);

    curTotalSizeByTask_--;
    taskCountByPrio_[lastSchedulePriority_]--;
    if (policyMap_[lastScheduleTarget_] == TargetPolicy::DELAY) {
        delayTaskCount_--;
        taskDelayCountByPrio_[lastSchedulePriority_]--;
    }

    for (auto iter = taskOrderByPrio_[lastSchedulePriority_].begin();
        iter != taskOrderByPrio_[lastSchedulePriority_].end(); ++iter) {
        if (*iter == lastScheduleTarget_) {
            taskOrderByPrio_[lastSchedulePriority_].erase(iter);
            break;
        }
    }

    taskGroupByPrio_[lastSchedulePriority_][lastScheduleTarget_].pop_front();
    delete task.buffer;
    task.buffer = nullptr;
    scheduledFlag_ = false;

    if (isFullBefore && !isFullAfter) {
        return -E_CONTAINER_FULL_TO_NOTFULL;
    }
    if (curTotalSizeByTask_ == 0) {
        return -E_CONTAINER_NOTEMPTY_TO_EMPTY;
    }
    if (curTotalSizeByTask_ == delayTaskCount_) {
        return -E_CONTAINER_ONLY_DELAY_TASK;
    }

    return E_OK;
}

int SendTaskScheduler::DelayTaskByTarget(const std::string &inTarget)
{
    std::lock_guard<std::mutex> overallLockGuard(overallMutex_);
    if (policyMap_.count(inTarget) == 0) {
        LOGE("[Scheduler][DelayTask] Not found inTarget=%s{private}", inTarget.c_str());
        return -E_NOT_FOUND;
    }
    if (policyMap_[inTarget] == TargetPolicy::DELAY) {
        return E_OK;
    }

    policyMap_[inTarget] = TargetPolicy::DELAY;
    for (auto &prio : priorityOrder_) {
        size_t count = taskGroupByPrio_[prio][inTarget].size();
        taskDelayCountByPrio_[prio] += static_cast<uint32_t>(count);
        delayTaskCount_ += static_cast<uint32_t>(count);
    }
    return E_OK;
}

int SendTaskScheduler::NoDelayTaskByTarget(const std::string &inTarget)
{
    std::lock_guard<std::mutex> overallLockGuard(overallMutex_);
    if (policyMap_.count(inTarget) == 0) {
        LOGE("[Scheduler][NoDelayTask] Not found inTarget=%s{private}", inTarget.c_str());
        return -E_NOT_FOUND;
    }
    if (policyMap_[inTarget] == TargetPolicy::NO_DELAY) {
        return E_OK;
    }

    policyMap_[inTarget] = TargetPolicy::NO_DELAY;
    for (auto &prio : priorityOrder_) {
        size_t count = taskGroupByPrio_[prio][inTarget].size();
        // Logic guarantee that former not smaller than latter
        taskDelayCountByPrio_[prio] -= static_cast<uint32_t>(count);
        delayTaskCount_ -= static_cast<uint32_t>(count);
    }
    return E_OK;
}

uint32_t SendTaskScheduler::GetTotalTaskCount() const
{
    std::lock_guard<std::mutex> overallLockGuard(overallMutex_);
    return curTotalSizeByTask_;
}

uint32_t SendTaskScheduler::GetNoDelayTaskCount() const
{
    std::lock_guard<std::mutex> overallLockGuard(overallMutex_);
    // delayTaskCount_ never greater than curTotalSizeByTask_
    return curTotalSizeByTask_ - delayTaskCount_;
}

int SendTaskScheduler::ScheduleDelayTask(SendTask &outTask, SendTaskInfo &outTaskInfo)
{
    for (auto &prio : priorityOrder_) {
        if (taskCountByPrio_[prio] == 0) {
            // No task of this priority
            continue;
        }
        // Logic guarantee that lists access below will not be empty
        std::string dstTarget = taskOrderByPrio_[prio].front();
        outTask = taskGroupByPrio_[prio][dstTarget].front();
        outTaskInfo.delayFlag = true;
        outTaskInfo.taskPrio = prio;
        return E_OK;
    }
    LOGE("[Scheduler][ScheduleDelay] INTERNAL ERROR : NO TASK.");
    return -E_INTERNAL_ERROR;
}

int SendTaskScheduler::ScheduleNoDelayTask(SendTask &outTask, SendTaskInfo &outTaskInfo)
{
    for (auto &prio : priorityOrder_) {
        if (taskCountByPrio_[prio] == 0 || taskCountByPrio_[prio] == taskDelayCountByPrio_[prio]) {
            // No no_delay_task of this priority
            continue;
        }
        // Logic guarantee that lists accessed below will not be empty
        std::string dstTarget;
        bool findFlag = false; // Not necessary in fact
        for (auto iter = taskOrderByPrio_[prio].begin(); iter != taskOrderByPrio_[prio].end(); ++iter) {
            // Logic guarantee that there is at least one target in orderList that is NO_DELAY
            dstTarget = *iter;
            if (policyMap_[dstTarget] == TargetPolicy::NO_DELAY) {
                findFlag = true;
                break;
            }
        }
        if (!findFlag) {
            LOGE("[Scheduler][ScheduleNoDelay] INTERNAL ERROR : NO_DELAY NOT FOUND.");
            return -E_INTERNAL_ERROR;
        }

        outTask = taskGroupByPrio_[prio][dstTarget].front();
        outTaskInfo.delayFlag = false;
        outTaskInfo.taskPrio = prio;
        return E_OK;
    }
    LOGE("[Scheduler][ScheduleNoDelay] INTERNAL ERROR : NO TASK.");
    return -E_INTERNAL_ERROR;
}
}
