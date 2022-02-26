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
#ifndef I_TIMER_INFO_H
#define I_TIMER_INFO_H

#include <mutex>
#include "want_agent.h"

namespace OHOS {
namespace MiscServices {
class ITimerInfo {
public:
    ITimerInfo();
    ~ITimerInfo();

    int type;
    bool repeat;
    uint64_t interval;
    std::shared_ptr<OHOS::Notification::WantAgent::WantAgent> wantAgent;

    /**
    * Indicates the timing policy the timer use, which can be REALTIME or UTC.
    */
    const int TIMER_TYPE_REALTIME = 1 << 0;

    /**
    * Describes whether a timer will wake the device up.
    */
    const int TIMER_TYPE_WAKEUP = 1 << 1;

    /**
    * Describes whether a timer will be delivered precisely at a scheduled time.
    */
    const int TIMER_TYPE_EXACT = 1 << 2;

    /**
    * Indicates whether the timer waking up the system is supported in low-power mode.
    */
    const int TIMER_TYPE_IDLE = 1 << 3;

    /**
     * SetType set timer type
     * @para: type: TIMER_TYPE_REALTIME | TIMER_TYPE_WAKEUP 
     * 
     */
    virtual void SetType(const int &type) = 0;

    /**
     * SetRepeat set timer repeat or not
     * @para: repeat: bool
     * 
     */
    virtual void SetRepeat(bool repeat) = 0;

    /**
     * SetInterval set timer repeat interval
     * @para: repeat: uint64_t  >= 5000ms
     * 
     */
    virtual void SetInterval(const uint64_t &interval) = 0;
    virtual void SetWantAgent(std::shared_ptr<OHOS::Notification::WantAgent::WantAgent> wantAgent) = 0;
    virtual void OnTrigger() = 0;
};
} // MiscServices
} // OHOS

#endif // I_TIMER_INFO_H