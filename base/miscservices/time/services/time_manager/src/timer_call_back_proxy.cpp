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


#include "timer_call_back_proxy.h"

namespace OHOS {
namespace MiscServices {
TimerCallbackProxy::TimerCallbackProxy(const sptr<IRemoteObject> &object) : IRemoteProxy<ITimerCallback>(object)
{
}


TimerCallbackProxy::~TimerCallbackProxy()
{
    TIME_HILOGD(TIME_MODULE_CLIENT, "TimerCallbackProxy instance destoryed");
}

void TimerCallbackProxy::NotifyTimer(const uint64_t timerId)
{
    TIME_HILOGD(TIME_MODULE_CLIENT, "start id: %{public}" PRId64 "", timerId);
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        return;
    }

    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    if (!data.WriteInterfaceToken(GetDescriptor())) {
        TIME_HILOGE(TIME_MODULE_CLIENT, "write descriptor failed!");
        return;
    }

    if (!data.WriteUint64(timerId)) {
        TIME_HILOGE(TIME_MODULE_CLIENT, "write timerId failed!");
        return;
    }
    int ret = remote->SendRequest(static_cast<int>(ITimerCallback::Message::NOTIFY_TIMER), data, reply, option);
    if (ret != ERR_OK) {
        TIME_HILOGE(TIME_MODULE_CLIENT, "SendRequest is failed, error code: %{public}d", ret);
    }
    TIME_HILOGI(TIME_MODULE_CLIENT, "end.");
}
} // namespace MiscServices
} // namespace OHOS
