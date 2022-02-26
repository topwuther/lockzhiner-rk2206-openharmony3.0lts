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

#include <memory>
#include <deque>
#include <vector>
#include <atomic>
#include <functional>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <iostream>
#include "permission_log.h"

#include "distributed_permission_event_handler.h"

namespace OHOS {
namespace Security {
namespace Permission {
namespace {
static constexpr OHOS::HiviewDFX::HiLogLabel LABEL = {
    LOG_CORE, SECURITY_DOMAIN_PERMISSION, "DistributedPermissionEventHandlerMock"};
}
namespace {
class Task {
public:
    std::function<void()> callback = nullptr;
    std::string name = "undefined";
};

std::thread *runner__ = nullptr;
std::deque<Task *> tasks__;
int count__ = 0;
bool stopping__ = false;
std::mutex mutex__;
std::condition_variable empty__;

std::vector<std::string> delayedTasks_;

void Run__()
{
    PERMISSION_LOG_DEBUG(LABEL, "runner started");
    std::mutex runnerMutex__;
    std::unique_lock<std::mutex> notifyLock(runnerMutex__);
    while (!stopping__) {
        while (tasks__.empty()) {
            PERMISSION_LOG_DEBUG(LABEL, "runner waiting");
            empty__.wait(notifyLock);
        }
        if (stopping__) {
            break;
        }

        Task *task = nullptr;
        {
            std::unique_lock<std::mutex> lock(mutex__);
            task = tasks__.front();
            tasks__.pop_front();
        }

        if (task == nullptr) {
            PERMISSION_LOG_DEBUG(LABEL, "null task ");
            continue;
        }
        try {
            PERMISSION_LOG_DEBUG(LABEL, "run task, name: %{public}s", task->name.c_str());
            (task->callback)();
            PERMISSION_LOG_DEBUG(LABEL, "run task, name: %{public}s end", task->name.c_str());
        } catch (...) {
            PERMISSION_LOG_DEBUG(LABEL, "unknown error");
        }
        delete task;

        PERMISSION_LOG_DEBUG(LABEL, "run task %{public}s next", task->name.c_str());
    }
}

void Delay__(std::string taskname, std::function<void()> callback, int64_t delayTime)
{
    PERMISSION_LOG_DEBUG(LABEL, "task name in delay thread: %{public}s", taskname.c_str());

    std::chrono::system_clock::time_point tp1 = std::chrono::system_clock::now() + std::chrono::milliseconds(delayTime);
    while (!stopping__) {
        std::this_thread::sleep_until(tp1);
        std::chrono::system_clock::time_point tp2 = std::chrono::system_clock::now();
        if (tp1 <= tp2) {
            break;
        }
    }
    if (stopping__) {
        return;
    }

    std::unique_lock<std::mutex> lock(mutex__);
    int count = 0;
    for (auto it = delayedTasks_.begin(); it != delayedTasks_.end();) {
        if (stopping__) {
            return;
        }
        if (*it == taskname) {
            // move from delayed list to task list
            it = delayedTasks_.erase(it);

            Task *task = new Task();
            task->name = std::string(taskname);
            task->callback = callback;
            tasks__.push_back(task);
            empty__.notify_all();
            count++;
            PERMISSION_LOG_DEBUG(LABEL,
                "delay task attached. task: %{public}s, task: %{public}lu",
                task->name.c_str(),
                (unsigned long)task);
        } else {
            it++;
        }
    }
    if (count == 0) {
        PERMISSION_LOG_DEBUG(LABEL, "delay task not found, maybe removed. task: %{public}s", taskname.c_str());
    }
}
}  // namespace

namespace {
const std::string SEP = "$";
}
DistributedPermissionEventHandler::DistributedPermissionEventHandler(
    const std::shared_ptr<AppExecFwk::EventRunner> &runner)
    : AppExecFwk::EventHandler(runner)
{
    PERMISSION_LOG_INFO(LABEL, "enter");
    stopping__ = false;
    runner__ = nullptr;
    count__ = 0;
}

DistributedPermissionEventHandler::~DistributedPermissionEventHandler()
{
    PERMISSION_LOG_INFO(LABEL, "exit begin");
    if (runner__ != nullptr) {
        stopping__ = true;
        empty__.notify_all();
        runner__->detach();
    }
    PERMISSION_LOG_INFO(LABEL, "exit end");
}

bool DistributedPermissionEventHandler::ProxyPostTask(const std::function<void()> &callback, int64_t delayTime)
{
    return ProxyPostTask(callback, "unknown", delayTime);
}

bool DistributedPermissionEventHandler::ProxyPostTask(
    const std::function<void()> &callback, const std::string &name, int64_t delayTime)
{
    PERMISSION_LOG_DEBUG(LABEL, "post task begin1");
    std::unique_lock<std::mutex> lock(mutex__);
    PERMISSION_LOG_DEBUG(LABEL, "post task begin2");
    if (runner__ == nullptr) {
        PERMISSION_LOG_DEBUG(LABEL, "to start runner...");
        runner__ = new std::thread(Run__);
    }

    int maxDelay = 1000000;
    if (delayTime > maxDelay) {
        std::cerr << "delay time too large!" << std::endl;
        PERMISSION_LOG_ERROR(LABEL, "delay time too large! time is ms: %{public}ld", (long)delayTime);
        return false;
    }
    if (name.find(SEP) != name.npos) {
        std::cerr << "task name error, delete char $!" << std::endl;
        PERMISSION_LOG_ERROR(LABEL, "task name error, delete char $! name: %{public}s", name.c_str());
        return false;
    }

    // store to event runner too.
    int index = count__++;

    if (delayTime <= 0) {
        Task *task = new Task();
        task->name = std::to_string(index) + SEP + name;
        task->callback = callback;
        tasks__.push_back(task);
        empty__.notify_all();
        lock.unlock();
        PERMISSION_LOG_DEBUG(
            LABEL, "task attached. task name: %{public}s, task: %{public}lu", task->name.c_str(), (unsigned long)task);
        return true;
    }

    std::string taskname = std::to_string(index) + SEP + name;
    delayedTasks_.push_back(taskname);
    PERMISSION_LOG_DEBUG(LABEL, "task name: %{public}s", taskname.c_str());
    std::thread delay(Delay__, taskname, callback, delayTime);
    delay.detach();
    PERMISSION_LOG_DEBUG(LABEL, "task scheduled. ");
    return true;
}

void DistributedPermissionEventHandler::ProxyRemoveTask(const std::string &aname)
{
    std::string name = std::string(aname);
    PERMISSION_LOG_DEBUG(LABEL, "remove task begin, name: %{public}s", name.c_str());

    std::unique_lock<std::mutex> lock(mutex__);
    int count = 0;
    for (auto it = delayedTasks_.begin(); it != delayedTasks_.end();) {
        std::string nameSuffix = SEP + name;
        if (it->compare(it->size() - nameSuffix.size(), nameSuffix.size(), nameSuffix) == 0) {
            // delete from delayed list
            PERMISSION_LOG_DEBUG(LABEL, "delay task removed. task: %{public}s", (*it).c_str());
            it = delayedTasks_.erase(it);
            count++;
        } else {
            it++;
        }
    }

    PERMISSION_LOG_DEBUG(LABEL, "removed %{public}d task", count);
}
}  // namespace Permission
}  // namespace Security
}  // namespace OHOS