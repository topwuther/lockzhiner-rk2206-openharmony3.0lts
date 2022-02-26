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
#include "platform_test_result_listener.h"
using namespace OHOS::HiviewDFX;
bool PlatformTestResultListener::OnOrderedEvent(Event& msg)
{
    printf("cur listener:%s OnOrderedEvent eventId_:%u \n", name_.c_str(), msg.eventId_);
    orderEventCount_++;
    printf("cur listener orderEventCount_:%d.\n", orderEventCount_);
    processedOrderedEvents_[msg.GetValue("message")] = msg.sender_;
    if (msg.GetValue("Finish") == name_) {
        return true;
    }
    return false;
}

void PlatformTestResultListener::OnUnorderedEvent(const Event& msg)
{
    printf("cur listener:%s OnUnorderedEvent eventId_:%u \n", name_.c_str(), msg.eventId_);
    unorderEventCount_++;
    printf("cur listener unorderEventCount_:%d.\n", unorderEventCount_);
    processedUnorderedEvents_[msg.GetValue("message")] = msg.sender_;
}

std::string PlatformTestResultListener::GetListenerName()
{
    return name_;
}