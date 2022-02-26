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

#include "mock_common_event_stub.h"

using namespace OHOS::EventFwk;

bool MockCommonEventStub::PublishCommonEvent(const CommonEventData &event, const CommonEventPublishInfo &publishinfo,
    const sptr<IRemoteObject> &commonEventListener)
{
    EVENT_LOGI("enter");

    return true;
}

bool MockCommonEventStub::SubscribeCommonEvent(
    const CommonEventSubscribeInfo &subscriberInfo, const sptr<IRemoteObject> &commonEventListener)
{
    EVENT_LOGI("enter");

    subscribeInfoPtr = std::make_shared<CommonEventSubscribeInfo>(subscriberInfo);

    return true;
}

bool MockCommonEventStub::DumpState(const std::string &event, std::vector<std::string> &state)
{
    EVENT_LOGI("enter");

    if (subscribeInfoPtr) {
        // get matchingSkills
        auto matchingSkills = subscribeInfoPtr->GetMatchingSkills();
        // get events
        auto events = matchingSkills.GetEvents();

        for (auto it : events) {
            state.emplace_back(it);
        }
    } else {
        // do nothing
    }

    subscribeInfoPtr = nullptr;

    return true;
}
