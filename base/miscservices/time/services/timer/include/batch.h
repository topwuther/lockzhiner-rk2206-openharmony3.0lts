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

#ifndef TIMER_BATCH_H
#define TIMER_BATCH_H

#include <cstdint>
#include <vector>
#include <chrono>
#include <functional>
#include "timer_info.h"
#include "time_common.h"
namespace OHOS {
namespace MiscServices {
class Batch {
public:
    Batch();
    explicit Batch(const TimerInfo &seed);
    virtual ~Batch() = default;
    std::chrono::steady_clock::time_point GetStart() const;
    std::chrono::steady_clock::time_point GetEnd() const;
    uint32_t GetFlags() const;
    size_t Size() const;
    std::shared_ptr<TimerInfo> Get(size_t index) const;
    bool CanHold(std::chrono::steady_clock::time_point whenElapsed,
                 std::chrono::steady_clock::time_point maxWhen) const;
    bool Add(const std::shared_ptr<TimerInfo> &alarm);
    bool Remove(const TimerInfo &alarm);
    bool Remove(std::function<bool(const TimerInfo &)> predicate);
    bool HasPackage(const std::string &package_name);
    bool HasWakeups() const;

private:
    std::chrono::steady_clock::time_point start_;
    std::chrono::steady_clock::time_point end_;
    uint32_t flags_;
    std::vector<std::shared_ptr<TimerInfo>> alarms_;
};
}
}
#endif