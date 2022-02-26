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

#ifndef TIMER_CALL_BACK_PROXY_H
#define TIMER_CALL_BACK_PROXY_H

#include <iremote_proxy.h>
#include <nocopyable.h>

#include "itimer_call_back.h"
#include "time_common.h"
#include <inttypes.h>
namespace OHOS {
namespace MiscServices {
class TimerCallbackProxy : public IRemoteProxy<ITimerCallback> {
public:
    explicit TimerCallbackProxy(const sptr<IRemoteObject>& impl);

    ~TimerCallbackProxy();
    DISALLOW_COPY_AND_MOVE(TimerCallbackProxy);
    virtual void NotifyTimer(const uint64_t timerId) override;

private:
    static inline BrokerDelegator<TimerCallbackProxy> delegator_;
};
} // namespace PowerMgr
} // namespace OHOS

#endif // TIMER_CALL_BACK_PROXY_H
