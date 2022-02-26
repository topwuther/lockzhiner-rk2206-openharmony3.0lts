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

#include "subscriber_death_recipient.h"
#include "common_event_subscriber_manager.h"
#include "event_log_wrapper.h"

namespace OHOS {
namespace EventFwk {
void SubscriberDeathRecipient::OnRemoteDied(const wptr<IRemoteObject> &remote)
{
    EVENT_LOGI("OnRemoteDied start");

    if (remote == nullptr) {
        EVENT_LOGE("remote object is null");
        return;
    }

    sptr<IRemoteObject> object = remote.promote();
    if (!object) {
        EVENT_LOGE("object is null");
        return;
    }

    DelayedSingleton<CommonEventSubscriberManager>::GetInstance()->RemoveSubscriber(object);

    EVENT_LOGI("OnRemoteDied end");
}
}  // namespace EventFwk
}  // namespace OHOS
