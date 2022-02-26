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

#include "batch.h"
#include <algorithm>

namespace OHOS {
namespace MiscServices {
const auto TYPE_NONWAKEUP_MASK = 0x1;

Batch::Batch()
    : start_ {std::chrono::steady_clock::time_point::min()},
      end_ {std::chrono::steady_clock::time_point::max()},
      flags_ {0}
{
}

Batch::Batch(const TimerInfo &seed)
    : start_ {seed.whenElapsed},
      end_ {seed.maxWhenElapsed},
      flags_ {seed.flags},
      alarms_ {std::make_shared<TimerInfo>(seed)}
{
}

size_t Batch::Size() const
{
    return alarms_.size();
}

std::shared_ptr<TimerInfo> Batch::Get(size_t index) const
{
    return (index >= alarms_.size()) ? nullptr : alarms_.at(index);
}

bool Batch::CanHold(std::chrono::steady_clock::time_point whenElapsed,
                    std::chrono::steady_clock::time_point maxWhen) const
{
    return (end_ > whenElapsed) && (start_ <= maxWhen);
}

bool Batch::Add(const std::shared_ptr<TimerInfo> &alarm)
{
    bool new_start = false;
    auto it = std::upper_bound(alarms_.begin(),
        alarms_.end(),
        alarm,
        [](const std::shared_ptr<TimerInfo> &first, const std::shared_ptr<TimerInfo> &second) {
            return first->whenElapsed < second->whenElapsed;
        });
    alarms_.insert(it, alarm); // 根据Alarm.when_elapsed从小到大排列

    if (alarm->whenElapsed > start_) {
        start_ = alarm->whenElapsed;
        new_start = true;
    }

    if (alarm->maxWhenElapsed < end_) {
            end_ = alarm->maxWhenElapsed;
    }

    flags_ |= alarm->flags;
    return new_start;
}

bool Batch::Remove(const TimerInfo &alarm)
{
    return Remove([alarm] (const TimerInfo &a) { return a == alarm; });
}

bool Batch::Remove(std::function<bool (const TimerInfo &)> predicate)
{
    TIME_HILOGD(TIME_MODULE_SERVICE, "start");
    bool didRemove = false;
    auto newStart = std::chrono::steady_clock::time_point::min();
    auto newEnd = std::chrono::steady_clock::time_point::max();
    uint32_t newFlags = 0;
    for (auto it = alarms_.begin(); it != alarms_.end();) {
        auto alarm = *it;
        TIME_HILOGD(TIME_MODULE_SERVICE, "looper");
        if (predicate(*alarm)) {
            TIME_HILOGD(TIME_MODULE_SERVICE, "erase");
            it = alarms_.erase(it);
            didRemove = true;
        } else {
            if (alarm->whenElapsed > newStart) {
                newStart = alarm->whenElapsed;
            }
            if (alarm->maxWhenElapsed < newEnd) {
                newEnd = alarm->maxWhenElapsed;
            }
            newFlags |= alarm->flags;
            ++it;
        }
    }
    TIME_HILOGD(TIME_MODULE_SERVICE, "end");
    if (didRemove) {
        start_ = newStart;
        end_ = newEnd;
        flags_ = newFlags;
    }
    return didRemove;
}

bool Batch::HasPackage(const std::string &package_name)
{
    return std::find_if(alarms_.begin(),
        alarms_.end(),
        [package_name] (const std::shared_ptr<TimerInfo> &alarm) {
            return alarm->Matches(package_name);
        }) != alarms_.end();
}

bool Batch::HasWakeups() const
{
    return std::any_of(alarms_.begin(), alarms_.begin(),
        [] (const std::shared_ptr<TimerInfo> &item) {
            return (static_cast<uint32_t>(item->type) & TYPE_NONWAKEUP_MASK) == 0;
        });
}

std::chrono::steady_clock::time_point Batch::GetStart() const
{
    return start_;
}

std::chrono::steady_clock::time_point Batch::GetEnd() const
{
    return end_;
}

uint32_t Batch::GetFlags() const
{
    return flags_;
}
} // MiscServices
} // OHOS