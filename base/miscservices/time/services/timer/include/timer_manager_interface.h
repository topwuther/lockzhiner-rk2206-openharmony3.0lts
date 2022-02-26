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

#ifndef TIMER_MANAGER_INTERFACE_H
#define TIMER_MANAGER_INTERFACE_H

#include <memory>
#include "want_agent.h"

namespace OHOS {
namespace MiscServices {
struct TimerEntry {
    uint64_t id;
    int type;
    uint64_t windowLength;
    uint64_t interval;
    int flag;
    std::function<void (const uint64_t)> callback;
    uint64_t uid;
};

class ITimerManager {
public:
    enum TimerFlag {
        STANDALONE = 1 << 0,
        WAKE_FROM_IDLE = 1 << 1,
        ALLOW_WHILE_IDLE = 1 << 2,
        ALLOW_WHILE_IDLE_UNRESTRICTED = 1 << 3,
        IDLE_UNTIL = 1 << 4,
    };

    enum TimerType {
        RTC_WAKEUP = 0,
        RTC = 1,
        ELAPSED_REALTIME_WAKEUP = 2,
        ELAPSED_REALTIME = 3
    };

    virtual uint64_t CreateTimer(int type, uint64_t windowLength,  uint64_t interval, int flag,
                                    std::function<void(const uint64_t)> callback,
                                    uint64_t uid) = 0;

    virtual bool StartTimer(uint64_t timerNumber, uint64_t triggerTime) = 0;
    virtual bool StopTimer(uint64_t timerNumber) = 0;
    virtual bool DestroyTimer(uint64_t timerNumber) = 0;
    virtual ~ITimerManager() = default;
}; // ITimerManager
} // MiscService
} // OHOS

#endif