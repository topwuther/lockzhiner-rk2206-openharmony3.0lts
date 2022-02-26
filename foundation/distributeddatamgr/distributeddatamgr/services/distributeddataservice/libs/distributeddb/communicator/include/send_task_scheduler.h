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

#ifndef SEND_TASK_SCHEDULER_H
#define SEND_TASK_SCHEDULER_H

#include <map>
#include <list>
#include <mutex>
#include <vector>
#include <string>
#include <cstdint>
#include "macro_utils.h"
#include "communicator_type_define.h"

namespace DistributedDB {
enum class TargetPolicy {
    NO_DELAY = 0,
    DELAY = 1,
};

class SerialBuffer; // Forward Declaration

struct SendTask {
    SerialBuffer *buffer;
    std::string dstTarget;
    OnSendEnd onEnd;
};

struct SendTaskInfo {
    bool delayFlag;
    Priority taskPrio;
};

using TaskListByTarget = std::map<std::string, std::list<SendTask>>;

class SendTaskScheduler {
public:
    SendTaskScheduler() = default; // Default constructor must be explicitly provided due to DISABLE_COPY_ASSIGN_MOVE
    ~SendTaskScheduler();

    DISABLE_COPY_ASSIGN_MOVE(SendTaskScheduler);

    void Initialize();
    // This method for consumer
    void Finalize();

    // This method for producer, support multiple thread
    int AddSendTaskIntoSchedule(const SendTask &inTask, Priority inPrio);
    // This method for consumer, not recommend for multiple thread
    int ScheduleOutSendTask(SendTask &outTask);
    int ScheduleOutSendTask(SendTask &outTask, SendTaskInfo &outTaskInfo);
    // This method for consumer, call ScheduleOutSendTask at least one time before each calling this
    int FinalizeLastScheduleTask();

    // These two mothods influence the task that will be schedule out next time
    int DelayTaskByTarget(const std::string &inTarget);
    int NoDelayTaskByTarget(const std::string &inTarget);

    uint32_t GetTotalTaskCount() const;
    uint32_t GetNoDelayTaskCount() const;

private:
    int ScheduleDelayTask(SendTask &outTask, SendTaskInfo &outTaskInfo);
    int ScheduleNoDelayTask(SendTask &outTask, SendTaskInfo &outTaskInfo);

    mutable std::mutex overallMutex_;
    uint32_t curTotalSizeByByte_ = 0;
    uint32_t curTotalSizeByTask_ = 0;
    uint32_t delayTaskCount_ = 0;

    std::vector<Priority> priorityOrder_;
    std::map<Priority, uint32_t> extraCapacityInByteByPrio_;
    std::map<std::string, TargetPolicy> policyMap_;

    std::map<Priority, uint32_t> taskCountByPrio_;
    std::map<Priority, uint32_t> taskDelayCountByPrio_;
    std::map<Priority, std::list<std::string>> taskOrderByPrio_;
    std::map<Priority, TaskListByTarget> taskGroupByPrio_;

    bool scheduledFlag_ = false;
    std::string lastScheduleTarget_;
    Priority lastSchedulePriority_ = Priority::LOW;
};
}

#endif