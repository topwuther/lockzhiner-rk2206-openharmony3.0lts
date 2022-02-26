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

#include "timer_call_back.h"

namespace OHOS {
namespace MiscServices {
namespace {
    const int WANTAGENT_CODE_ELEVEN = 11;
}
std::mutex TimerCallback::instanceLock_;
sptr<TimerCallback> TimerCallback::instance_;

std::map<uint64_t, std::shared_ptr<ITimerInfo>> TimerCallback::TimerInfoMap_;
std::mutex TimerCallback::timerInfoMutex_;

TimerCallback::TimerCallback()
{
}

TimerCallback::~TimerCallback()
{
    TimerInfoMap_.clear();
}

sptr<TimerCallback> TimerCallback::GetInstance()
{
    if (instance_ == nullptr) {
        std::lock_guard<std::mutex> autoLock(instanceLock_);
        if (instance_ == nullptr) {
            instance_ = new TimerCallback;
        }
    }
    return instance_;
}
    
bool TimerCallback::InsertTimerCallbackInfo(const uint64_t timerId,
                                            const std::shared_ptr<ITimerInfo> &timerInfo)
{
    TIME_HILOGD(TIME_MODULE_SERVICE, "start.");
    if (timerInfo == nullptr) {
        return false;
    }

    std::lock_guard<std::mutex> lock(timerInfoMutex_);
    auto info = TimerInfoMap_.find(timerId);
    if (info != TimerInfoMap_.end()) {
        TIME_HILOGE(TIME_MODULE_CLIENT, "timer info already insert.");
        return false;
    } else {
        TimerInfoMap_[timerId] = timerInfo;
    }
    TIME_HILOGD(TIME_MODULE_SERVICE, "end.");
    return true;
}

bool TimerCallback::RemoveTimerCallbackInfo(const uint64_t timerId)
{
    TIME_HILOGD(TIME_MODULE_SERVICE, "start.");
    std::lock_guard<std::mutex> lock(timerInfoMutex_);
    auto info = TimerInfoMap_.find(timerId);
    if (info != TimerInfoMap_.end()) {
        TimerInfoMap_.erase(info);
        TIME_HILOGD(TIME_MODULE_SERVICE, "end.");
        return true;
    } 
    TIME_HILOGD(TIME_MODULE_SERVICE, "end.");
    return false;
}

void TimerCallback::NotifyTimer(const uint64_t timerId)
{
    TIME_HILOGD(TIME_MODULE_SERVICE, "start.");
    std::lock_guard<std::mutex> lock(timerInfoMutex_);
    auto it = TimerInfoMap_.find(timerId);
    if (it != TimerInfoMap_.end()) {
        TIME_HILOGD(TIME_MODULE_SERVICE, "ontrigger.");
        it->second->OnTrigger();

        if (it->second->wantAgent != nullptr) {
            TIME_HILOGD(TIME_MODULE_SERVICE, "trigger wantagent.");
            std::shared_ptr<AppExecFwk::Context> context = std::make_shared<OHOS::AppExecFwk::AbilityContext>();
            std::shared_ptr<AAFwk::Want> want =
                Notification::WantAgent::WantAgentHelper::GetWant(it->second->wantAgent);
          
            OHOS::Notification::WantAgent::TriggerInfo paramsInfo("", nullptr, want, WANTAGENT_CODE_ELEVEN);
            Notification::WantAgent::WantAgentHelper::TriggerWantAgent(context,
                it->second->wantAgent, nullptr, paramsInfo);
        }
    }
    TIME_HILOGD(TIME_MODULE_SERVICE, "end.");
}
} // namespace MiscServices
} // namespace OHOS