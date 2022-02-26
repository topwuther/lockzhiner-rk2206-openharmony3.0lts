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

#include "common_event_listener.h"
#include "event_log_wrapper.h"

namespace OHOS {
namespace EventFwk {
CommonEventListener::CommonEventListener(const std::shared_ptr<CommonEventSubscriber> &commonEventSubscriber)
    : commonEventSubscriber_(commonEventSubscriber)
{
    Init();
}

CommonEventListener::~CommonEventListener()
{}

void CommonEventListener::NotifyEvent(const CommonEventData &commonEventData, const bool &ordered, const bool &sticky)
{
    EVENT_LOGI("enter");

    if (!IsReady()) {
        EVENT_LOGE("not ready");
        return;
    }

    std::function<void()> onReceiveEventFunc =
        std::bind(&CommonEventListener::OnReceiveEvent, this, commonEventData, ordered, sticky);
    handler_->PostTask(onReceiveEventFunc);
}

ErrCode CommonEventListener::Init()
{
    EVENT_LOGD("ready to init");

    if (runner_ == nullptr) {
        if (!commonEventSubscriber_) {
            EVENT_LOGE("Failed to init with CommonEventSubscriber nullptr");
            return ERR_INVALID_OPERATION;
        }
        if (CommonEventSubscribeInfo::HANDLER == commonEventSubscriber_->GetSubscribeInfo().GetThreadMode()) {
            runner_ = EventRunner::GetMainEventRunner();
        } else {
            runner_ = EventRunner::Create(true);
        }
        if (!runner_) {
            EVENT_LOGE("Failed to init due to create runner error");
            return ERR_INVALID_OPERATION;
        }
    }

    if (handler_ == nullptr) {
        handler_ = std::make_shared<EventHandler>(runner_);
        if (!handler_) {
            EVENT_LOGE("Failed to init due to create handler error");
            return ERR_INVALID_OPERATION;
        }
    }

    return ERR_OK;
}

bool CommonEventListener::IsReady()
{
    if (runner_ == nullptr) {
        EVENT_LOGE("runner is not ready");
        return false;
    }

    if (handler_ == nullptr) {
        EVENT_LOGE("handler is not ready");
        return false;
    }

    return true;
}

void CommonEventListener::OnReceiveEvent(
    const CommonEventData &commonEventData, const bool &ordered, const bool &sticky)
{
    EVENT_LOGI("enter");

    int code = commonEventData.GetCode();
    std::string data = commonEventData.GetData();

    std::shared_ptr<AsyncCommonEventResult> result =
        std::make_shared<AsyncCommonEventResult>(code, data, ordered, sticky, this);
    if (result == nullptr) {
        EVENT_LOGE("Failed to create AsyncCommonEventResult");
        return;
    }

    if (!commonEventSubscriber_) {
        EVENT_LOGE("CommonEventSubscriber ptr is nullptr");
        return;
    }
    commonEventSubscriber_->SetAsyncCommonEventResult(result);

    commonEventSubscriber_->OnReceiveEvent(commonEventData);

    if ((commonEventSubscriber_->GetAsyncCommonEventResult() != nullptr) && ordered) {
        commonEventSubscriber_->GetAsyncCommonEventResult()->FinishCommonEvent();
    }
}

void CommonEventListener::Stop()
{
    if (handler_) {
        handler_.reset();
    }

    if (CommonEventSubscribeInfo::HANDLER == commonEventSubscriber_->GetSubscribeInfo().GetThreadMode()) {
        return;
    }

    if (runner_) {
        runner_.reset();
    }
}
}  // namespace EventFwk
}  // namespace OHOS