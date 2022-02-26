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

#define LOG_TAG "BroadcastSenderImpl"

#include "broadcast_sender_impl.h"
#include "common_event_manager.h"
#include "common_event_support.h"
#include "log_print.h"
#include "ohos/aafwk/base/array_wrapper.h"
#include "ohos/aafwk/base/string_wrapper.h"

namespace OHOS::DistributedKv {
using namespace OHOS::EventFwk;
using namespace OHOS::AAFwk;
class CommonEventSubscriberListener : public CommonEventSubscriber {
public:
    explicit CommonEventSubscriberListener(const CommonEventSubscribeInfo &subscriberInfo);
    virtual ~CommonEventSubscriberListener(){};
    virtual void OnReceiveEvent(const CommonEventData &data);
};

CommonEventSubscriberListener::CommonEventSubscriberListener(const CommonEventSubscribeInfo &subscriberInfo)
    : CommonEventSubscriber(subscriberInfo)
{}

void CommonEventSubscriberListener::OnReceiveEvent(const CommonEventData &data)
{
    ZLOGI("receive event.");
}

void BroadcastSenderImpl::SendEvent(const EventParams &params)
{
    ZLOGI("SendEvent code.");
    bool result = false;
    WantParams parameters;
    parameters.SetParam(PKG_NAME, String::Box(params.appId));
    Want want;
    want.SetAction(ACTION_NAME).SetParams(parameters);
    CommonEventData commonEventData(want);
    MatchingSkills matchingSkills;
    matchingSkills.AddEvent(ACTION_NAME);
    CommonEventSubscribeInfo subscribeInfo(matchingSkills);
    auto subscriberPtr = std::make_shared<CommonEventSubscriberListener>(subscribeInfo);
    if (CommonEventManager::SubscribeCommonEvent(subscriberPtr)) {
        result = CommonEventManager::PublishCommonEvent(commonEventData);
    }
    CommonEventManager::UnSubscribeCommonEvent(subscriberPtr);
    ZLOGI("SendEvent result:%{public}d.", result);
}
}
