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

#include "timer_info.h"

namespace OHOS {
namespace MiscServices {
bool TimerInfo::operator==(const TimerInfo &other) const
{
    return this->id == other.id;
}

bool TimerInfo::Matches(const std::string &packageName) const
{
    return false;
}

TimerInfo::TimerInfo(uint64_t _id, int _type,
                     std::chrono::milliseconds _when,
                     std::chrono::steady_clock::time_point _whenElapsed,
                     std::chrono::milliseconds _windowLength,
                     std::chrono::steady_clock::time_point _maxWhen,
                     std::chrono::milliseconds _interval,
                     std::function<void(const uint64_t)> _callback,
                     uint32_t _flags,
                     uint64_t _uid)
    : id {_id},
      type {_type},
      origWhen {_when},
      wakeup {_type == ITimerManager::ELAPSED_REALTIME_WAKEUP || _type == ITimerManager::RTC_WAKEUP},
      callback {std::move(_callback)},
      flags {_flags},
      uid {_uid},
      when {_when},
      windowLength {_windowLength},
      whenElapsed {_whenElapsed},
      maxWhenElapsed {_maxWhen},
      expectedWhenElapsed {_whenElapsed},
      expectedMaxWhenElapsed {_maxWhen},
      repeatInterval {_interval}
{
}
} // MiscServices
} // OHOS