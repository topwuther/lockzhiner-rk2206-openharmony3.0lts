/*
 * Copyright (C) 2021 Huawei Device Co., Ltd.
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

#ifndef TIMER_HANDLER_H
#define TIMER_HANDLER_H

#include <array>
#include <cstdint>
#include <string>
#include <chrono>
#include <memory>

#include "time_common.h"

namespace OHOS {
namespace MiscServices {
static const size_t ALARM_TYPE_COUNT = 5;
static const size_t N_TIMER_FDS = ALARM_TYPE_COUNT + 1;
typedef std::array<int, N_TIMER_FDS> TimerFds;

class TimerHandler {
public:
    static std::shared_ptr<TimerHandler> Create();
    int Set(uint32_t type, std::chrono::nanoseconds when);
    uint32_t WaitForAlarm();
    ~TimerHandler();
private:
    TimerHandler(const TimerFds &fds, int epollfd);
    const TimerFds fds_;
    const int epollFd_;
};
} // MiscService
} // OHOS
#endif