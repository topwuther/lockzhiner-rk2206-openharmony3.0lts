/*
 * Copyright (C) 2021 Huawei Device Co., Ltd.
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

#include "time_service_client.h"
#include "time_common.h"
#include <cinttypes>
#include "system_ability_definition.h"
#include "iservice_registry.h"
#include <mutex>

namespace OHOS {
namespace MiscServices {
std::mutex TimeServiceClient::instanceLock_;
sptr<TimeServiceClient> TimeServiceClient::instance_;
sptr<ITimeService> TimeServiceClient::timeServiceProxy_;
sptr<TimeSaDeathRecipient> TimeServiceClient::deathRecipient_;

TimeServiceClient::TimeServiceClient()
{
}

TimeServiceClient::~TimeServiceClient()
{
    if (timeServiceProxy_ != nullptr) {
        auto remoteObject = timeServiceProxy_->AsObject();
        if (remoteObject != nullptr) {
            remoteObject->RemoveDeathRecipient(deathRecipient_);
        }
    }
}

sptr<TimeServiceClient> TimeServiceClient::GetInstance()
{
    if (instance_ == nullptr) {
        std::lock_guard<std::mutex> autoLock(instanceLock_);
        if (instance_ == nullptr) {
            instance_ = new TimeServiceClient;
        }
    }
    return instance_;
}

sptr<ITimeService> TimeServiceClient::ConnectService()
{
    sptr<ISystemAbilityManager> systemAbilityManager =
        SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    if (systemAbilityManager == nullptr) {
        TIME_HILOGE(TIME_MODULE_CLIENT, "Getting SystemAbilityManager failed.");
        return nullptr;
    }

    auto systemAbility = systemAbilityManager->GetSystemAbility(TIME_SERVICE_ID);
    if (systemAbility == nullptr) {
        TIME_HILOGE(TIME_MODULE_CLIENT, "Get SystemAbility failed.");
        return nullptr;
    }
    deathRecipient_ = new TimeSaDeathRecipient();
    systemAbility->AddDeathRecipient(deathRecipient_);
    sptr<ITimeService> timeServiceProxy_ = iface_cast<ITimeService>(systemAbility);
    if (timeServiceProxy_ == nullptr) {
        TIME_HILOGE(TIME_MODULE_CLIENT, "Get TimeServiceProxy from SA failed.");
        return nullptr;
    }

    TIME_HILOGD(TIME_MODULE_CLIENT, "Getting TimeServiceProxy succeeded.");
    return timeServiceProxy_;
}

bool TimeServiceClient::TimeServiceClient::SetTime(const int64_t time)
{
    if (timeServiceProxy_ == nullptr) {
        TIME_HILOGW(TIME_MODULE_CLIENT, "Redo ConnectService");
        timeServiceProxy_ = ConnectService();
    }

    if (timeServiceProxy_ == nullptr) {
        TIME_HILOGE(TIME_MODULE_CLIENT, "SetTime quit because redoing ConnectService failed.");
        return false;
    }

    if (timeServiceProxy_->SetTime(time) != ERR_OK) {
        return false;
    }
    return true;
}

bool TimeServiceClient::SetTimeZone(const std::string timezoneId)
{
    if (timeServiceProxy_ == nullptr) {
        TIME_HILOGW(TIME_MODULE_CLIENT, "Redo ConnectService");
        timeServiceProxy_ = ConnectService();
    }

    if (timeServiceProxy_ == nullptr) {
        TIME_HILOGE(TIME_MODULE_CLIENT, "SetTimeZone quit because redoing ConnectService failed.");
        return false;
    }

    if (timeServiceProxy_->SetTimeZone(timezoneId) != ERR_OK) {
        return false;
    }
    return true;
}

uint64_t TimeServiceClient::CreateTimer(std::shared_ptr<ITimerInfo> TimerOptions)
{
    if (TimerOptions == nullptr) {
        TIME_HILOGW(TIME_MODULE_CLIENT, "Input nullptr");
        return 0;
    }
    if (timeServiceProxy_ == nullptr) {
        TIME_HILOGW(TIME_MODULE_CLIENT, "Redo ConnectService");
        timeServiceProxy_ = ConnectService();
    }

    if (timeServiceProxy_ == nullptr) {
        TIME_HILOGE(TIME_MODULE_CLIENT, " quit because redoing ConnectService failed.");
        return 0;
    }

    auto timerCallbackInfoObject = TimerCallback::GetInstance()->AsObject();
    if (!timerCallbackInfoObject) {
        TIME_HILOGE(TIME_MODULE_CLIENT, "New TimerCallback failed");
        return 0;
    }

    auto timerId = timeServiceProxy_->CreateTimer(TimerOptions->type,
                                                  TimerOptions->repeat,
                                                  TimerOptions->interval,
                                                  timerCallbackInfoObject);
    if (timerId == 0) {
        TIME_HILOGE(TIME_MODULE_CLIENT, "Create timer failed");
        return 0;
    }
    TIME_HILOGI(TIME_MODULE_SERVICE, "CreateTimer id: %{public}" PRId64 "", timerId);
    auto ret = TimerCallback::GetInstance()->InsertTimerCallbackInfo(timerId, TimerOptions);
    if (!ret) {
        return 0;
    }
    return timerId;
}

bool TimeServiceClient::StartTimer(uint64_t timerId, uint64_t triggerTime)
{
    if (timeServiceProxy_ == nullptr) {
        TIME_HILOGW(TIME_MODULE_CLIENT, "Redo ConnectService");
        timeServiceProxy_ = ConnectService();
    }

    if (timeServiceProxy_ == nullptr) {
        TIME_HILOGE(TIME_MODULE_CLIENT, "StartTimer quit because redoing ConnectService failed.");
        return false;
    }

    return timeServiceProxy_->StartTimer(timerId, triggerTime);
}

bool TimeServiceClient::StopTimer(uint64_t timerId)
{
    if (timeServiceProxy_ == nullptr) {
        TIME_HILOGW(TIME_MODULE_CLIENT, "Redo ConnectService");
        timeServiceProxy_ = ConnectService();
    }

    if (timeServiceProxy_ == nullptr) {
        TIME_HILOGE(TIME_MODULE_CLIENT, "StopTimer quit because redoing ConnectService failed.");
        return false;
    }

    return timeServiceProxy_->StopTimer(timerId);
}

bool TimeServiceClient::DestroyTimer(uint64_t timerId)
{
    if (timeServiceProxy_ == nullptr) {
        TIME_HILOGW(TIME_MODULE_CLIENT, "Redo ConnectService");
        timeServiceProxy_ = ConnectService();
    }

    if (timeServiceProxy_ == nullptr) {
        TIME_HILOGE(TIME_MODULE_CLIENT, "DestroyTimer quit because redoing ConnectService failed.");
        return false;
    }
    if (timeServiceProxy_->DestroyTimer(timerId)) {
        TimerCallback::GetInstance()->RemoveTimerCallbackInfo(timerId);
        return true;
    }
    return false;
}

std::string TimeServiceClient::GetTimeZone()
{
    std::string timeZoneId;
    if (timeServiceProxy_ == nullptr) {
        TIME_HILOGW(TIME_MODULE_CLIENT, "Redo ConnectService");
        timeServiceProxy_ = ConnectService();
    }

    if (timeServiceProxy_ == nullptr) {
        TIME_HILOGE(TIME_MODULE_CLIENT, "GetTimeZone quit because redoing ConnectService failed.");
        return std::string("");
    }

    if (timeServiceProxy_->GetTimeZone(timeZoneId) != ERR_OK) {
        TIME_HILOGE(TIME_MODULE_CLIENT, "get failed.");
        return std::string("");
    }
    return timeZoneId;
}

int64_t TimeServiceClient::GetWallTimeMs()
{
    int64_t times;
    if (timeServiceProxy_ == nullptr) {
        TIME_HILOGW(TIME_MODULE_CLIENT, "Redo ConnectService");
        timeServiceProxy_ = ConnectService();
    }

    if (timeServiceProxy_ == nullptr) {
        TIME_HILOGE(TIME_MODULE_CLIENT, "GetWallTimeMs quit because redoing ConnectService failed.");
        return -1;
    }
    if (timeServiceProxy_->GetWallTimeMs(times) != ERR_OK) {
        TIME_HILOGE(TIME_MODULE_CLIENT, "get failed.");
        return -1;
    }
    TIME_HILOGI(TIME_MODULE_SERVICE, "Result: %{public}" PRId64 "", times);
    return times;
}

int64_t TimeServiceClient::GetWallTimeNs()
{
    int64_t times;
    if (timeServiceProxy_ == nullptr) {
        TIME_HILOGW(TIME_MODULE_CLIENT, "Redo ConnectService");
        timeServiceProxy_ = ConnectService();
    }

    if (timeServiceProxy_ == nullptr) {
        TIME_HILOGE(TIME_MODULE_CLIENT, "GetWallTimeNs quit because redoing ConnectService failed.");
        return -1;
    }

    if (timeServiceProxy_->GetWallTimeNs(times) != ERR_OK) {
        TIME_HILOGE(TIME_MODULE_CLIENT, "get failed.");
        return -1;
    }
    TIME_HILOGI(TIME_MODULE_SERVICE, "Result: %{public}" PRId64 "", times);
    return times;
}

int64_t TimeServiceClient::GetBootTimeMs()
{
    int64_t times;
    if (timeServiceProxy_ == nullptr) {
        TIME_HILOGW(TIME_MODULE_CLIENT, "Redo ConnectService");
        timeServiceProxy_ = ConnectService();
    }

    if (timeServiceProxy_ == nullptr) {
        TIME_HILOGE(TIME_MODULE_CLIENT, "GetBootTimeMs quit because redoing ConnectService failed.");
        return -1;
    }

    if (timeServiceProxy_->GetBootTimeMs(times) != ERR_OK) {
        TIME_HILOGE(TIME_MODULE_CLIENT, "get failed.");
        return -1;
    }
    TIME_HILOGI(TIME_MODULE_SERVICE, "Result: %{public}" PRId64 "", times);
    return times;
}

int64_t TimeServiceClient::GetBootTimeNs()
{
    int64_t times;
    if (timeServiceProxy_ == nullptr) {
        TIME_HILOGW(TIME_MODULE_CLIENT, "Redo ConnectService");
        timeServiceProxy_ = ConnectService();
    }

    if (timeServiceProxy_ == nullptr) {
        TIME_HILOGE(TIME_MODULE_CLIENT, "GetBootTimeNs quit because redoing ConnectService failed.");
        return -1;
    }

    if (timeServiceProxy_->GetBootTimeNs(times) != ERR_OK) {
        TIME_HILOGE(TIME_MODULE_CLIENT, "get failed.");
        return -1;
    }
    TIME_HILOGI(TIME_MODULE_SERVICE, "Result: %{public}" PRId64 "", times);
    return times;
}

int64_t TimeServiceClient::GetMonotonicTimeMs()
{
    int64_t times;
    if (timeServiceProxy_ == nullptr) {
        TIME_HILOGW(TIME_MODULE_CLIENT, "Redo ConnectService");
        timeServiceProxy_ = ConnectService();
    }

    if (timeServiceProxy_ == nullptr) {
        TIME_HILOGE(TIME_MODULE_CLIENT, "GetMonotonicTimeMs quit because redoing ConnectService failed.");
        return -1;
    }

    if (timeServiceProxy_->GetMonotonicTimeMs(times) != ERR_OK) {
        TIME_HILOGE(TIME_MODULE_CLIENT, "get failed.");
        return -1;
    }
    TIME_HILOGI(TIME_MODULE_SERVICE, "Result: %{public}" PRId64 "", times);
    return times;
}

int64_t TimeServiceClient::GetMonotonicTimeNs()
{
    int64_t times;
    if (timeServiceProxy_ == nullptr) {
        TIME_HILOGW(TIME_MODULE_CLIENT, "Redo ConnectService");
        timeServiceProxy_ = ConnectService();
    }

    if (timeServiceProxy_ == nullptr) {
        TIME_HILOGE(TIME_MODULE_CLIENT, "GetMonotonicTimeNs quit because redoing ConnectService failed.");
        return -1;
    }

    if (timeServiceProxy_->GetMonotonicTimeNs(times) != ERR_OK) {
        TIME_HILOGE(TIME_MODULE_CLIENT, "get failed.");
        return -1;
    }
    TIME_HILOGI(TIME_MODULE_SERVICE, "Result: %{public}" PRId64 "", times);
    return times;
}

int64_t TimeServiceClient::GetThreadTimeMs()
{
    int64_t times;
    if (timeServiceProxy_ == nullptr) {
        TIME_HILOGW(TIME_MODULE_CLIENT, "Redo ConnectService");
        timeServiceProxy_ = ConnectService();
    }

    if (timeServiceProxy_ == nullptr) {
        TIME_HILOGE(TIME_MODULE_CLIENT, "GetThreadTimeMs quit because redoing ConnectService failed.");
        return -1;
    }

    if (timeServiceProxy_->GetThreadTimeMs(times) != ERR_OK) {
        TIME_HILOGE(TIME_MODULE_CLIENT, "get failed.");
        return -1;
    }
    TIME_HILOGI(TIME_MODULE_SERVICE, "Result: %{public}" PRId64 "", times);
    return times;
}

int64_t TimeServiceClient::GetThreadTimeNs()
{
    int64_t times;
    if (timeServiceProxy_ == nullptr) {
        TIME_HILOGW(TIME_MODULE_CLIENT, "Redo ConnectService");
        timeServiceProxy_ = ConnectService();
    }

    if (timeServiceProxy_ == nullptr) {
        TIME_HILOGE(TIME_MODULE_CLIENT, "GetThreadTimeNs quit because redoing ConnectService failed.");
        return -1;
    }
    if (timeServiceProxy_->GetThreadTimeNs(times) != ERR_OK) {
        TIME_HILOGE(TIME_MODULE_CLIENT, "get failed.");
        return -1;
    }
    TIME_HILOGI(TIME_MODULE_SERVICE, "Result: %{public}" PRId64 "", times);
    return times;
}


void TimeServiceClient::OnRemoteSaDied(const wptr<IRemoteObject> &remote)
{
    timeServiceProxy_ = ConnectService();
}

TimeSaDeathRecipient::TimeSaDeathRecipient()
{
}

void TimeSaDeathRecipient::OnRemoteDied(const wptr<IRemoteObject> &object)
{
    TIME_HILOGE(TIME_MODULE_CLIENT, "TimeSaDeathRecipient on remote systemAbility died.");
    TimeServiceClient::GetInstance()->OnRemoteSaDied(object);
}
} // namespace MiscServices
} // namespace OHOS
