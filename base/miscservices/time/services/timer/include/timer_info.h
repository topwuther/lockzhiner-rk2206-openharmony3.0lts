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

#ifndef TIMER_INFO_H
#define TIMER_INFO_H

#include <cstdint>
#include <string>
#include <chrono>
#include <utility>
#include <functional>
#include "timer_manager_interface.h"

namespace OHOS {
namespace MiscServices {
class TimerInfo {
public:
    const uint64_t id;
    const int type;
    const std::chrono::milliseconds origWhen;
    const bool wakeup;
    const std::function<void (const uint64_t)> callback;
    const uint32_t flags;
    const uint64_t uid;

    uint64_t count {};
    std::chrono::milliseconds when;
    std::chrono::milliseconds windowLength;
    std::chrono::steady_clock::time_point whenElapsed;
    std::chrono::steady_clock::time_point maxWhenElapsed;
    std::chrono::steady_clock::time_point expectedWhenElapsed;
    std::chrono::steady_clock::time_point expectedMaxWhenElapsed;
    std::chrono::milliseconds repeatInterval;

    TimerInfo(uint64_t id, int type,
        std::chrono::milliseconds when,
        std::chrono::steady_clock::time_point whenElapsed,
        std::chrono::milliseconds windowLength,
        std::chrono::steady_clock::time_point maxWhen,
        std::chrono::milliseconds interval,
        std::function<void (const uint64_t)> callback,
        uint32_t flags,
        uint64_t uid);
    virtual ~TimerInfo() = default;
    bool operator==(const TimerInfo &other) const;
    bool Matches(const std::string &packageName) const;
};
} // MiscService
} // OHOS
#endif