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

#include "completed_dispatcher.h"

namespace OHOS::Notification::WantAgent {
CompletedDispatcher::CompletedDispatcher(const std::shared_ptr<PendingWant> &pendingWant,
    const std::shared_ptr<CompletedCallback> &callback, const std::shared_ptr<AppExecFwk::EventHandler> &handler)
    : pendingWant_(pendingWant), callback_(callback), handler_(handler)
{}

void CompletedDispatcher::Send(const int32_t resultCode)
{}

void CompletedDispatcher::PerformReceive(const AAFwk::Want &want, int resultCode, const std::string &data,
    const AAFwk::WantParams &extras, bool serialized, bool sticky, int sendingUser)
{
    want_ = want;
    resultCode_ = resultCode;
    resultData_ = data;
    resultExtras_ = extras;
    if (handler_ == nullptr) {
        Run();
    }
}

void CompletedDispatcher::Run()
{
    if (callback_ != nullptr) {
        callback_->OnSendFinished(want_, resultCode_, resultData_, resultExtras_);
    }
}
}  // namespace OHOS::Notification::WantAgent