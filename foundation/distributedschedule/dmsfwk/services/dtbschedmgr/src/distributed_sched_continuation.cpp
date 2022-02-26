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

#include "distributed_sched_continuation.h"
#include "dtbschedmgr_log.h"

using namespace std;
using namespace OHOS::AppExecFwk;

namespace OHOS {
namespace DistributedSchedule {
namespace {
constexpr int64_t CONTINUATION_DELAY_TIME = 20000;
}

void DSchedContinuation::Init(const FuncContinuationCallback& contCallback)
{
    auto runner = EventRunner::Create("dsched_continuation");
    continuationHandler_ = make_shared<ContinuationHandler>(runner, shared_from_this(), contCallback);
}

bool DSchedContinuation::PushAbilityToken(int32_t sessionId, const sptr<IRemoteObject>& abilityToken)
{
    if (abilityToken == nullptr) {
        HILOGE("DSchedContinuation::PushAbilityToken abilityToken null!");
        return false;
    }

    if (sessionId <= 0) {
        HILOGE("DSchedContinuation::PushAbilityToken sessionId invalid!");
        return false;
    }

    if (continuationHandler_ == nullptr) {
        HILOGE("DSchedContinuation::PushAbilityToken not initialized!");
        return false;
    }

    lock_guard<mutex> autoLock(continuationLock_);
    bool ret = true;
    ret = continuationHandler_->SendEvent(sessionId, 0, CONTINUATION_DELAY_TIME);
    if (!ret) {
        HILOGE("DSchedContinuation::PushAbilityToken SendEvent failed!");
        return false;
    }

    auto iterSession = continuationMap_.find(sessionId);
    if (iterSession != continuationMap_.end()) {
        HILOGE("DSchedContinuation::PushAbilityToken sessionId:%{public}d exist!", sessionId);
        return false;
    }
    (void)continuationMap_.emplace(sessionId, abilityToken);
    return true;
}

sptr<IRemoteObject> DSchedContinuation::PopAbilityToken(int32_t sessionId)
{
    if (sessionId <= 0) {
        HILOGE("DSchedContinuation::PopAbilityToken sessionId invalid");
        return nullptr;
    }

    lock_guard<mutex> autoLock(continuationLock_);
    auto iter = continuationMap_.find(sessionId);
    if (iter == continuationMap_.end()) {
        HILOGW("DSchedContinuation::PopAbilityToken not found sessionId:%{public}d", sessionId);
        return nullptr;
    }
    sptr<IRemoteObject> abilityToken = iter->second;
    (void)continuationMap_.erase(iter);
    if (continuationHandler_ != nullptr) {
        continuationHandler_->RemoveEvent(sessionId);
    }
    return abilityToken;
}

int32_t DSchedContinuation::GenerateSessionId()
{
    lock_guard<mutex> autoLock(continuationLock_);
    int32_t currValue = currSessionId_;
    if (++currSessionId_ <= 0) {
        currSessionId_ = 1;
    }
    return currValue;
}

void DSchedContinuation::ContinuationHandler::ProcessEvent(const InnerEvent::Pointer& event)
{
    if (event == nullptr) {
        HILOGE("ContinuationHandler::ProcessEvent event nullptr!");
        return;
    }

    auto dSchedContinuation = continuationObj_.lock();
    if (dSchedContinuation == nullptr) {
        HILOGE("ContinuationHandler::ProcessEvent continuation object failed!");
        return;
    }

    auto eventId = event->GetInnerEventId();
    int32_t sessionId = static_cast<int32_t>(eventId);
    if (sessionId <= 0) {
        HILOGW("ContinuationHandler::ProcessEvent sessionId invalid!");
        return;
    }

    auto abilityToken = dSchedContinuation->PopAbilityToken(sessionId);
    if (abilityToken == nullptr) {
        HILOGW("ContinuationHandler::ProcessEvent abilityToken nullptr!");
        return;
    }

    if (contCallback_ != nullptr) {
        contCallback_(abilityToken);
    }
}
} // namespace DistributedSchedule
} // namespace OHOS