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

#include "want_agent.h"
#include "want_agent_log_wrapper.h"

namespace OHOS::Notification::WantAgent {
WantAgent::WantAgent(const std::shared_ptr<PendingWant> &pendingWant)
{
    pendingWant_ = pendingWant;
}

std::shared_ptr<PendingWant> WantAgent::GetPendingWant()
{
    return pendingWant_;
}

void WantAgent::SetPendingWant(const std::shared_ptr<PendingWant> &pendingWant)
{
    pendingWant_ = pendingWant;
}

bool WantAgent::Marshalling(Parcel &parcel) const
{
    if (!parcel.WriteParcelable(pendingWant_.get())) {
        WANT_AGENT_LOGE("parcel WriteString failed");
        return false;
    }

    return true;
}

WantAgent *WantAgent::Unmarshalling(Parcel &parcel)
{
    WantAgent *agent = new (std::nothrow) WantAgent();
    if (agent == nullptr) {
        WANT_AGENT_LOGE("read from parcel failed");
        return nullptr;
    }
    std::shared_ptr<PendingWant> pendingWant(parcel.ReadParcelable<PendingWant>());
    agent->SetPendingWant(pendingWant);

    return agent;
}
}  // namespace OHOS::Notification::WantAgent
