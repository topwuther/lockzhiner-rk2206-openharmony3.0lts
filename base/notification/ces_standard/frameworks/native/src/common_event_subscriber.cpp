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

#include "common_event_subscriber.h"
#include "event_log_wrapper.h"

namespace OHOS {
namespace EventFwk {
CommonEventSubscriber::CommonEventSubscriber() : result_(nullptr)
{}

CommonEventSubscriber::CommonEventSubscriber(const CommonEventSubscribeInfo &subscribeInfo)
    : subscribeInfo_(subscribeInfo), result_(nullptr)
{}

CommonEventSubscriber::~CommonEventSubscriber()
{}

const CommonEventSubscribeInfo &CommonEventSubscriber::GetSubscribeInfo() const
{
    return subscribeInfo_;
}

bool CommonEventSubscriber::SetCode(const int &code)
{
    if (!CheckSynchronous()) {
        return false;
    }

    return result_->SetCode(code);
}

int CommonEventSubscriber::GetCode() const
{
    if (!CheckSynchronous()) {
        return 0;
    }

    return result_->GetCode();
}

bool CommonEventSubscriber::SetData(const std::string &data)
{
    if (!CheckSynchronous()) {
        return false;
    }

    return result_->SetData(data);
}

std::string CommonEventSubscriber::GetData() const
{
    if (!CheckSynchronous()) {
        return std::string();
    }

    return result_->GetData();
}

bool CommonEventSubscriber::SetCodeAndData(const int &code, const std::string &data)
{
    if (!CheckSynchronous()) {
        return false;
    }

    return result_->SetCodeAndData(code, data);
}

bool CommonEventSubscriber::AbortCommonEvent()
{
    if (!CheckSynchronous()) {
        return false;
    }

    return result_->AbortCommonEvent();
}

bool CommonEventSubscriber::ClearAbortCommonEvent()
{
    if (!CheckSynchronous()) {
        return false;
    }

    return result_->ClearAbortCommonEvent();
}

bool CommonEventSubscriber::GetAbortCommonEvent() const
{
    if (!CheckSynchronous()) {
        return false;
    }

    return result_->GetAbortCommonEvent();
}

std::shared_ptr<AsyncCommonEventResult> CommonEventSubscriber::GoAsyncCommonEvent()
{
    std::shared_ptr<AsyncCommonEventResult> res = result_;
    result_ = nullptr;
    return res;
}

bool CommonEventSubscriber::IsOrderedCommonEvent() const
{
    return (result_ != nullptr) ? result_->IsOrderedCommonEvent() : false;
}

bool CommonEventSubscriber::IsStickyCommonEvent() const
{
    return (result_ != nullptr) ? result_->IsStickyCommonEvent() : false;
}

bool CommonEventSubscriber::SetAsyncCommonEventResult(const std::shared_ptr<AsyncCommonEventResult> &result)
{
    result_ = result;

    return true;
}

std::shared_ptr<AsyncCommonEventResult> CommonEventSubscriber::GetAsyncCommonEventResult()
{
    return result_;
}

bool CommonEventSubscriber::CheckSynchronous() const
{
    if (!result_) {
        EVENT_LOGE("Call when result is not set.");
        return false;
    }
    if (!result_->CheckSynchronous()) {
        return false;
    }

    return true;
}
}  // namespace EventFwk
}  // namespace OHOS