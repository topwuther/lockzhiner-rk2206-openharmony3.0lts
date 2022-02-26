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

#ifndef TIMER_CALL_BACK_STUB_H
#define TIMER_CALL_BACK_STUB_H

#include <iremote_stub.h>
#include <nocopyable.h>
#include "time_common.h"
#include "itimer_call_back.h"
#include <inttypes.h>
namespace OHOS {
namespace MiscServices {
class TimerCallbackStub : public IRemoteStub<ITimerCallback> {
public:
    DISALLOW_COPY_AND_MOVE(TimerCallbackStub);
    TimerCallbackStub() = default;
    virtual ~TimerCallbackStub() = default;
    int OnRemoteRequest(uint32_t code, MessageParcel &data, MessageParcel &reply, MessageOption &option) override;

private:
    int OnTriggerStub(MessageParcel& data);
};
} // namespace MiscServices
} // namespace OHOS
#endif // TIMER_CALL_BACK_STUB_H
