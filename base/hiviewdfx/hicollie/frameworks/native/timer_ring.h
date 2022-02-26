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

#ifndef RELIABILITY_XCOLLIE_TIMERRING_H
#define RELIABILITY_XCOLLIE_TIMERRING_H

#include <time.h>

#include <condition_variable>
#include <functional>
#include <list>
#include <mutex>
#include <string>
#include <vector>

#include "thread_ex.h"

namespace OHOS {
namespace HiviewDFX {
struct TimerTask {
    std::string name;
    unsigned int timeout;
    bool loop;
    std::function<void (struct InputTimerPara*)> func;
    struct InputTimerPara* inputTimerPara;
};

struct InputTimerPara {
    std::string name;
    unsigned int timeout;
    unsigned int flag;
    int tid;
    time_t startTime;
    std::function<void (void*)> func;
    void* arg;
};

struct TaskNode {
    int id; // -1 if not alloc
    unsigned int seq;
    int pos; // the position of timer ring
    int round; // timer ring round
    struct TimerTask timerTask;
};

static const unsigned int MAX_XCOLLIE_SHIFT = 7;
static const unsigned int COOKIE_SHIFT = 8;
static const unsigned int MAX_XCOLLIE_NUM = (1 << MAX_XCOLLIE_SHIFT);
static const unsigned int TIMER_RING_CHECK_INTERVAL = 1;
static const unsigned int MAX_TIMERRING_SIZE = 60;
static const unsigned int MAX_DELAY_COUNT = 3;
static const unsigned int MILLI_SECONDS = 1000;

#define WRAP_SEQ(id) ((id) & ((1 << COOKIE_SHIFT) - 1))
#define CALC_RING_POS(timeout) ((ringPos_ + (((timeout) - 1) / TIMER_RING_CHECK_INTERVAL + 1)) % MAX_TIMERRING_SIZE)
#define CALC_RING_ROUND(timeout) (((timeout) / TIMER_RING_CHECK_INTERVAL) / MAX_TIMERRING_SIZE)

static const std::string XCOLLIE_THREAD_NAME = "XCollieThread";
class TimerRing : public Thread {
public:
    TimerRing();
    ~TimerRing() override;
    int AddTask(const struct TimerTask &task);
    struct InputTimerPara* CancelTask(int id);
    bool UpdateTask(int id, unsigned int timeout);
    bool ReadyToWork() override;
    bool Run() override;
    std::string GetName() const
    {
        return XCOLLIE_THREAD_NAME;
    };
    void TryNotify();
private:
    void TryRecycle(std::list<struct TaskNode*> &timeoutNodes);
    void InitTaskNode(struct TaskNode* taskNode, unsigned int seq);
    void DoTimeoutTask();
    void TrySleep();

    std::vector<struct TaskNode> taskNodes_ = std::vector<struct TaskNode>(MAX_XCOLLIE_NUM);
    std::list<struct TaskNode*> freeTaskNodes_ = std::list<struct TaskNode*>();
    std::vector<std::list<struct TaskNode*>> ringTaskNodes_ =
        std::vector<std::list<struct TaskNode*>>(MAX_TIMERRING_SIZE);

    mutable std::mutex lock_;
    std::condition_variable condition_;
    bool threadInSleep_;
    time_t lastTimeout_;
    uint64_t lastDuration_;
    int ringPos_;
};
} // end of namespace HiviewDFX
} // end of namespace OHOS
#endif
