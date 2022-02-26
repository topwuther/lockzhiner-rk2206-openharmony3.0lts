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

#ifndef I_TIMER_CALL_BACK_H
#define I_TIMER_CALL_BACK_H

#include "iremote_broker.h"

namespace OHOS {
namespace MiscServices {
class ITimerCallback : public IRemoteBroker {
public:
    DECLARE_INTERFACE_DESCRIPTOR(u"ohos.miscservices.time.ITimerCallback");

    /**
     * trigger timer by id.
     *
     * @param timerId  timerId
     *
     */
    virtual void NotifyTimer(const uint64_t timerId) = 0;

    enum Message {
        NOTIFY_TIMER = 1
    };
};
}  // namespace MiscServices
}  // namespace OHOS

#endif  // I_TIMER_CALL_BACK_H
