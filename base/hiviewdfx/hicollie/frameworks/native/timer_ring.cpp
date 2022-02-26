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

#include "timer_ring.h"

#include <chrono>
#include <thread>
#include <unistd.h>

#include "xcollie_define.h"
#include "xcollie_utils.h"

namespace OHOS {
namespace HiviewDFX {
TimerRing::TimerRing() : threadInSleep_(false), lastTimeout_(0), lastDuration_(0), ringPos_(-1)
{
    // 1. init nodes, add node to free list
    freeTaskNodes_.clear();
    for (unsigned int i = 0; i < MAX_XCOLLIE_NUM; i++) {
        struct TaskNode* node = &taskNodes_[i];
        InitTaskNode(node, i);
        freeTaskNodes_.push_back(node);
    }
}

TimerRing::~TimerRing()
{
    XCOLLIE_LOGI("~TimerRing exit...");
}

bool TimerRing::ReadyToWork()
{
    ringPos_ = 0;
    return true;
}

void TimerRing::InitTaskNode(struct TaskNode* taskNode, unsigned int seq)
{
    taskNode->id = INVALID_ID;
    taskNode->seq = seq;
    taskNode->pos = 0;
    taskNode->round = 0;
    taskNode->timerTask.name = "";
    taskNode->timerTask.timeout = 0;
    taskNode->timerTask.loop = false;
    taskNode->timerTask.func = nullptr;
    taskNode->timerTask.inputTimerPara = nullptr;
}

bool TimerRing::Run()
{
    // sleep interval
    if (lastDuration_ < (TIMER_RING_CHECK_INTERVAL * MILLI_SECONDS)) {
        std::this_thread::sleep_for(std::chrono::milliseconds(
            (TIMER_RING_CHECK_INTERVAL * MILLI_SECONDS) - lastDuration_));
    }

    lastDuration_ = 0;

    // find task and do time out
    auto start = std::chrono::system_clock::now();
    DoTimeoutTask();
    auto end = std::chrono::system_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    lastDuration_ += duration.count();

    // if no task, then sleep
    start = std::chrono::system_clock::now();
    TrySleep();
    end = std::chrono::system_clock::now();
    duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    lastDuration_ += duration.count();

    return true;
}

void TimerRing::TryRecycle(std::list<struct TaskNode*> &timeoutNodes)
{
    std::for_each(timeoutNodes.begin(), timeoutNodes.end(), [&](struct TaskNode* &node) {
        if (node->timerTask.loop) {
            /* if need loop, add to ring again */
            node->pos = CALC_RING_POS(node->timerTask.timeout);
            node->round = CALC_RING_ROUND(node->timerTask.timeout);
            ringTaskNodes_[node->pos].push_back(node);
        } else {
            /* if no loop, add to free list */
            InitTaskNode(node, node->seq);
            freeTaskNodes_.push_back(node);
        }
    });
}

void TimerRing::DoTimeoutTask()
{
    std::list<struct TaskNode*> timeoutNodes;
    {
        std::lock_guard<std::mutex> lock(lock_);
        for (auto it = ringTaskNodes_[ringPos_].begin(); it != ringTaskNodes_[ringPos_].end();) {
            struct TaskNode* node = *it;
            if (node->round == 0) {
                node->pos = -1;
                timeoutNodes.push_back(node);
                it = ringTaskNodes_[ringPos_].erase(it);
            } else {
                node->round--;
                it++;
            }
        }
        if (timeoutNodes.size() > 0) {
            lastTimeout_ = time(nullptr);
        }
    }

    /* run timeout callback */
    for (auto it : timeoutNodes) {
        if (it->timerTask.func) {
            XCOLLIE_LOGD("Trigger %s:0x%x Callback Function ... ", it->timerTask.name.c_str(), it->id);
            it->timerTask.func(it->timerTask.inputTimerPara);
        }
    }

    /* try recycle and do next loop */
    std::lock_guard<std::mutex> lock(lock_);
    TryRecycle(timeoutNodes);

    /* point to the next timeout node */
    ringPos_ = CALC_RING_POS(TIMER_RING_CHECK_INTERVAL);
}

void TimerRing::TrySleep()
{
    std::unique_lock<std::mutex> lock(lock_);
    if ((freeTaskNodes_.size() == MAX_XCOLLIE_NUM) &&
        (difftime(time(nullptr), lastTimeout_) > TIMER_RING_CHECK_INTERVAL * MAX_DELAY_COUNT)) {
        threadInSleep_ = true;
        condition_.wait(lock, [this] {
            return threadInSleep_ == false;
        });
    }
}

void TimerRing::TryNotify()
{
    std::unique_lock<std::mutex> lock(lock_);
    if (threadInSleep_) {
        threadInSleep_ = false;
        condition_.notify_one();
    }
}

int TimerRing::AddTask(const struct TimerTask &task)
{
    std::unique_lock<std::mutex> lock(lock_);
    // 1. find task from freelist
    if (task.timeout == 0) {
        XCOLLIE_LOGE("timeout %u invalid, add %s failed", task.timeout, task.name.c_str());
        return INVALID_ID;
    }
    if (freeTaskNodes_.empty()) {
        XCOLLIE_LOGE("no free task node, add %s failed", task.name.c_str());
        return INVALID_ID;
    }
    struct TaskNode* taskNode = freeTaskNodes_.front();
    freeTaskNodes_.pop_front();
    // 2. init task
    time_t timestamp = time(nullptr);
    taskNode->id = (((static_cast<unsigned int>(timestamp) % MAX_XCOLLIE_NUM) << COOKIE_SHIFT) | taskNode->seq);
    taskNode->timerTask.timeout = task.timeout;
    taskNode->timerTask.name = task.name;
    taskNode->timerTask.inputTimerPara = task.inputTimerPara;
    taskNode->timerTask.func = task.func;
    taskNode->timerTask.loop = task.loop;
    unsigned int timeout = ((task.timeout < TIMER_RING_CHECK_INTERVAL) ? TIMER_RING_CHECK_INTERVAL : task.timeout);
    taskNode->round = CALC_RING_ROUND(timeout);
    taskNode->pos = CALC_RING_POS(timeout);
    if (threadInSleep_) {
        taskNode->pos = CALC_RING_POS(timeout - TIMER_RING_CHECK_INTERVAL);
    }
    // 3. add task to timering
    XCOLLIE_LOGD("TimerRing::AddTask ringPos_=%d, pos=%d, round=%d, timeout=%u, id =0x%x, name=%s",
        ringPos_, taskNode->pos, taskNode->round, timeout,
        taskNode->id, taskNode->timerTask.name.c_str());
    ringTaskNodes_[taskNode->pos].push_back(taskNode);

    /* if thread in sleep, then notify */
    if (threadInSleep_) {
        threadInSleep_ = false;
        condition_.notify_one();
    }
    return taskNode->id;
}

struct InputTimerPara* TimerRing::CancelTask(int id)
{
    if (id <= INVALID_ID) {
        return nullptr;
    }
    // 1. find tasknode
    unsigned int seq = WRAP_SEQ(static_cast<unsigned int>(id));
    if (seq >= MAX_XCOLLIE_NUM) {
        XCOLLIE_LOGE("cancel task failed, seq %u limit, id 0x%x valid", seq, id);
        return nullptr;
    }

    // 2.  move to free task list
    std::lock_guard<std::mutex> lock(lock_);
    struct TaskNode* taskNode = &taskNodes_[seq];
    if ((taskNode->id != id) || (taskNode->pos == -1)) {
        XCOLLIE_LOGW("already release id 0x%x", id);
        return nullptr;
    }
    XCOLLIE_LOGD("TimerRing::CancelTask ringPos_=%d, pos=%d id =0x%x,name=%s",  ringPos_,
        taskNode->pos, taskNode->id, taskNode->timerTask.name.c_str());
    ringTaskNodes_[taskNode->pos].remove(taskNode);
    struct InputTimerPara* inputPara = taskNode->timerTask.inputTimerPara;
    InitTaskNode(taskNode, taskNode->seq);
    freeTaskNodes_.push_back(taskNode);
    lastTimeout_ = time(nullptr);
    return inputPara;
}

bool TimerRing::UpdateTask(int id, unsigned int timeout)
{
    if (id <= INVALID_ID) {
        return false;
    }
    // 1. find tasknode
    if (timeout == 0) {
        XCOLLIE_LOGE("timeout %u invalid, update failed", timeout);
        return false;
    }
    unsigned int seq = WRAP_SEQ(static_cast<unsigned int>(id));
    if (seq >= MAX_XCOLLIE_NUM) {
        XCOLLIE_LOGE("update task failed, seq %u limit, id 0x%x valid", seq, id);
        return false;
    }

    // 2. update tasknode
    std::lock_guard<std::mutex> lock(lock_);
    struct TaskNode* taskNode = &taskNodes_[seq];
    if ((taskNode->id != id) || (taskNode->pos == -1)) {
        XCOLLIE_LOGW("update task failed, already release id 0x%x", id);
        return false;
    }
    taskNode->round = CALC_RING_ROUND(timeout);
    int pos = CALC_RING_POS(timeout);
    if (pos == taskNode->pos) {
        return true;
    }
    XCOLLIE_LOGD("TimerRing::UpdateTask ringPos_=%d, pos=%d id =0x%x,name=%s", ringPos_, pos,
        taskNode->id, taskNode->timerTask.name.c_str());
    // update ring list
    ringTaskNodes_[taskNode->pos].remove(taskNode);
    taskNode->pos = pos;
    ringTaskNodes_[pos].push_back(taskNode);
    return true;
}
} // end of namespace HiviewDFX
} // end of namespace OHOS
