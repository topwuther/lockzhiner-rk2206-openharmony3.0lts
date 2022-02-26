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

#include "async_common_event_result.h"
#include "common_event.h"
#include "event_log_wrapper.h"
#include "singleton.h"

namespace OHOS {
namespace EventFwk {
AsyncCommonEventResult::AsyncCommonEventResult(const int &resultCode, const std::string &resultData,
    const bool &ordered, const bool &sticky, const sptr<IRemoteObject> &token)
    : abortEvent_(false), finished_(false)
{
    resultCode_ = resultCode;
    resultData_ = resultData;
    ordered_ = ordered;
    sticky_ = sticky;
    token_ = token;
}

AsyncCommonEventResult::~AsyncCommonEventResult()
{}

bool AsyncCommonEventResult::SetCode(const int &code)
{
    if (!CheckSynchronous()) {
        return false;
    }

    resultCode_ = code;

    return true;
}

int AsyncCommonEventResult::GetCode() const
{
    return resultCode_;
}

bool AsyncCommonEventResult::SetData(const std::string &data)
{
    if (!CheckSynchronous()) {
        return false;
    }

    resultData_ = data;

    return true;
}

std::string AsyncCommonEventResult::GetData() const
{
    return resultData_;
}

bool AsyncCommonEventResult::SetCodeAndData(const int &code, const std::string &data)
{
    if (!CheckSynchronous()) {
        return false;
    }

    resultCode_ = code;
    resultData_ = data;

    return true;
}

bool AsyncCommonEventResult::AbortCommonEvent()
{
    if (!CheckSynchronous()) {
        return false;
    }

    abortEvent_ = true;

    return true;
}

bool AsyncCommonEventResult::ClearAbortCommonEvent()
{
    if (!CheckSynchronous()) {
        return false;
    }

    abortEvent_ = false;

    return true;
}

bool AsyncCommonEventResult::GetAbortCommonEvent() const
{
    return abortEvent_;
}

bool AsyncCommonEventResult::FinishCommonEvent()
{
    EVENT_LOGI("enter");

    if (!CheckSynchronous()) {
        return false;
    }

    if (finished_) {
        EVENT_LOGE("Common event already finished.");
        return false;
    }
    finished_ = true;

    return DelayedSingleton<CommonEvent>::GetInstance()->FinishReceiver(token_, resultCode_, resultData_, abortEvent_);
}

bool AsyncCommonEventResult::IsOrderedCommonEvent() const
{
    return ordered_;
}

bool AsyncCommonEventResult::IsStickyCommonEvent() const
{
    return sticky_;
}

bool AsyncCommonEventResult::CheckSynchronous() const
{
    bool ret = false;

    if (ordered_) {
        ret = true;
    } else {
        EVENT_LOGE("Subscriber want to set result for an unordered common event.");
        return false;
    }

    return ret;
}
}  // namespace EventFwk
}  // namespace OHOS
