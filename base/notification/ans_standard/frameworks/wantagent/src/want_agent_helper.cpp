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

#include "want_agent_helper.h"
#include "ability_manager_client.h"
#include "hilog_wrapper.h"
#include "pending_want.h"
#include "want_agent_log_wrapper.h"
#include "want_sender_interface.h"
#include "want_sender_info.h"

using namespace OHOS::AAFwk;
using namespace OHOS::AppExecFwk;

namespace OHOS::Notification::WantAgent {
WantAgentHelper::WantAgentHelper()
{}

unsigned int WantAgentHelper::FlagsTransformer(const std::vector<WantAgentConstant::Flags> &flags)
{
    unsigned int wantFlags = 0;
    if (flags.empty()) {
        wantFlags |= FLAG_UPDATE_CURRENT;
        return wantFlags;
    }

    for (auto flag : flags) {
        switch (flag) {
            case WantAgentConstant::Flags::ONE_TIME_FLAG:
                wantFlags |= FLAG_ONE_SHOT;
                break;
            case WantAgentConstant::Flags::NO_BUILD_FLAG:
                wantFlags |= FLAG_NO_CREATE;
                break;
            case WantAgentConstant::Flags::CANCEL_PRESENT_FLAG:
                wantFlags |= FLAG_CANCEL_CURRENT;
                break;
            case WantAgentConstant::Flags::UPDATE_PRESENT_FLAG:
                wantFlags |= FLAG_UPDATE_CURRENT;
                break;
            case WantAgentConstant::Flags::CONSTANT_FLAG:
                wantFlags |= FLAG_IMMUTABLE;
                break;
            default:
                WANT_AGENT_LOGE("WantAgentHelper::flags is error.");
                break;
        }
    }
    return wantFlags;
}

std::shared_ptr<WantAgent> WantAgentHelper::GetWantAgent(
    const std::shared_ptr<Context> &context, const WantAgentInfo &paramsInfo)
{
    WANT_AGENT_LOGI("WantAgentHelper::GetWantAgent begin.");
    if (context == nullptr) {
        WANT_AGENT_LOGE("WantAgentHelper::GetWantAgent invalid input param.");
        return nullptr;
    }

    std::vector<std::shared_ptr<Want>> wants = paramsInfo.GetWants();
    if (wants.empty()) {
        WANT_AGENT_LOGE("WantAgentHelper::GetWantAgent invalid input param.");
        return nullptr;
    }

    HILOG_INFO("%{public}s:bundle name = %{public}s; ability name = %{public}s",
        __func__,
        wants[0]->GetElement().GetBundleName().c_str(),
        wants[0]->GetElement().GetAbilityName().c_str());

    unsigned int flags = FlagsTransformer(paramsInfo.GetFlags());
    if (flags == 0) {
        WANT_AGENT_LOGE("WantAgentHelper::flags invalid.");
        return nullptr;
    }

    std::shared_ptr<WantParams> extraInfo = paramsInfo.GetExtraInfo();
    std::shared_ptr<PendingWant> pendingWant = nullptr;
    int requestCode = paramsInfo.GetRequestCode();
    WantAgentConstant::OperationType operationType = paramsInfo.GetOperationType();
    switch (operationType) {
        case WantAgentConstant::OperationType::START_ABILITY:
            pendingWant = PendingWant::GetAbility(context, requestCode, wants[0], flags, extraInfo);
            break;
        case WantAgentConstant::OperationType::START_ABILITIES:
            pendingWant = PendingWant::GetAbilities(context, requestCode, wants, flags, extraInfo);
            break;
        case WantAgentConstant::OperationType::START_SERVICE:
            pendingWant = PendingWant::GetService(context, requestCode, wants[0], flags);
            break;
        case WantAgentConstant::OperationType::START_FOREGROUND_SERVICE:
            pendingWant = PendingWant::GetForegroundService(context, requestCode, wants[0], flags);
            break;
        case WantAgentConstant::OperationType::SEND_COMMON_EVENT:
            pendingWant = PendingWant::GetCommonEvent(context, requestCode, wants[0], flags);
            break;
        default:
            WANT_AGENT_LOGE("WantAgentHelper::GetWantAgent operation type is error.");
            break;
    }

    if (pendingWant == nullptr) {
        WANT_AGENT_LOGE("WantAgentHelper::GetWantAgent the wants does not meet the requirements.");
        return nullptr;
    }

    std::shared_ptr<WantAgent> agent = std::make_shared<WantAgent>(pendingWant);
    WANT_AGENT_LOGI("WantAgentHelper::GetWantAgent end.");
    return agent;
}

std::shared_ptr<WantAgent> WantAgentHelper::GetWantAgent(const WantAgentInfo &paramsInfo)
{
    std::vector<std::shared_ptr<Want>> wants = paramsInfo.GetWants();
    if (wants.empty()) {
        WANT_AGENT_LOGE("WantAgentHelper::GetWantAgent invalid input param.");
        return nullptr;
    }

    std::shared_ptr<Want> want = wants[0];
    if (want == nullptr) {
        WANT_AGENT_LOGE("WantAgentHelper::GetWantAgent invalid input param.");
        return nullptr;
    }

    WantsInfo wantsInfo;
    wantsInfo.want = *want;
    wantsInfo.resolvedTypes = want->GetType();
    if (paramsInfo.GetExtraInfo() != nullptr) {
        wantsInfo.want.SetParams(*paramsInfo.GetExtraInfo());
    }

    HILOG_INFO("%{public}s:bundle name = %{public}s; ability name = %{public}s",
        __func__,
        wantsInfo.want.GetElement().GetBundleName().c_str(),
        wantsInfo.want.GetElement().GetAbilityName().c_str());

    WantSenderInfo wantSenderInfo;
    wantSenderInfo.allWants.push_back(wantsInfo);
    wantSenderInfo.bundleName = want->GetOperation().GetBundleName();
    wantSenderInfo.flags = FlagsTransformer(paramsInfo.GetFlags());

    sptr<IWantSender> target = AbilityManagerClient::GetInstance()->GetWantSender(wantSenderInfo, nullptr);
    if (target == nullptr) {
        WANT_AGENT_LOGE("WantAgentHelper::GetWantAgent target is nullptr.");
        return nullptr;
    }
    std::shared_ptr<WantAgent> agent = std::make_shared<WantAgent>(std::make_shared<PendingWant>(target));

    return agent;
}

WantAgentConstant::OperationType WantAgentHelper::GetType(const std::shared_ptr<WantAgent> &agent)
{
    if (agent == nullptr || agent->GetPendingWant() == nullptr) {
        return WantAgentConstant::OperationType::UNKNOWN_TYPE;
    }

    return agent->GetPendingWant()->GetType(agent->GetPendingWant()->GetTarget());
}

void WantAgentHelper::TriggerWantAgent(const std::shared_ptr<Context> &context, const std::shared_ptr<WantAgent> &agent,
    const std::shared_ptr<CompletedCallback> &callback, const TriggerInfo &paramsInfo)
{
    HILOG_INFO("%{public}s:begin.", __func__);
    if ((context == nullptr) || (agent == nullptr)) {
        WANT_AGENT_LOGE("WantAgentHelper::TriggerWantAgent invalid input param.");
        return;
    }

    std::shared_ptr<PendingWant> pendingWant = agent->GetPendingWant();
    WantAgentConstant::OperationType type = GetType(agent);
    sptr<CompletedDispatcher> dispatcher = nullptr;
    if (callback != nullptr) {
        dispatcher = new (std::nothrow) CompletedDispatcher(pendingWant, callback, nullptr);
    }

    Send(context, pendingWant, type, dispatcher, paramsInfo);
}

void WantAgentHelper::Send(const std::shared_ptr<Context> &context, const std::shared_ptr<PendingWant> &pendingWant,
    WantAgentConstant::OperationType type, const sptr<CompletedDispatcher> &callBack, const TriggerInfo &paramsInfo)
{
    HILOG_INFO("%{public}s:begin.", __func__);
    if ((context == nullptr) || (pendingWant == nullptr)) {
        WANT_AGENT_LOGE("WantAgentHelper::Send invalid input param.");
        return;
    }

    pendingWant->Send(context,
        paramsInfo.GetResultCode(),
        paramsInfo.GetWant(),
        callBack,
        paramsInfo.GetPermission(),
        paramsInfo.GetExtraInfo(),
        pendingWant->GetTarget());
}

void WantAgentHelper::Cancel(const std::shared_ptr<WantAgent> &agent)
{
    if (agent == nullptr) {
        WANT_AGENT_LOGE("WantAgentHelper::Cancel WantAgent invalid input param.");
        return;
    }

    std::shared_ptr<PendingWant> pendingWant = agent->GetPendingWant();
    if (pendingWant == nullptr) {
        WANT_AGENT_LOGE("WantAgentHelper::Cancel PendingWant invalid input param.");
        return;
    }

    pendingWant->Cancel(pendingWant->GetTarget());
}

bool WantAgentHelper::JudgeEquality(
    const std::shared_ptr<WantAgent> &agent, const std::shared_ptr<WantAgent> &otherAgent)
{
    if ((agent == nullptr) && (otherAgent == nullptr)) {
        return true;
    }

    if ((agent == nullptr) || (otherAgent == nullptr)) {
        return false;
    }

    return PendingWant::Equals(agent->GetPendingWant(), otherAgent->GetPendingWant());
}

int WantAgentHelper::GetHashCode(const std::shared_ptr<WantAgent> &agent)
{
    if (agent == nullptr) {
        WANT_AGENT_LOGE("WantAgentHelper::GetHashCode WantAgent invalid input param.");
        return 0;
    }

    std::shared_ptr<PendingWant> pendingWant = agent->GetPendingWant();
    if (pendingWant == nullptr) {
        WANT_AGENT_LOGE("WantAgentHelper::GetHashCode PendingWant invalid input param.");
        return 0;
    }

    return pendingWant->GetHashCode(pendingWant->GetTarget());
}

std::string WantAgentHelper::GetBundleName(const std::shared_ptr<WantAgent> &agent)
{
    if (agent == nullptr) {
        WANT_AGENT_LOGE("WantAgentHelper::GetBundleName WantAgent invalid input param.");
        return "";
    }

    std::shared_ptr<PendingWant> pendingWant = agent->GetPendingWant();
    if (pendingWant == nullptr) {
        WANT_AGENT_LOGE("WantAgentHelper::GetBundleName PendingWant invalid input param.");
        return "";
    }

    return pendingWant->GetBundleName(pendingWant->GetTarget());
}

int WantAgentHelper::GetUid(const std::shared_ptr<WantAgent> &agent)
{
    if (agent == nullptr) {
        WANT_AGENT_LOGE("WantAgentHelper::GetUid WantAgent invalid input param.");
        return -1;
    }

    std::shared_ptr<PendingWant> pendingWant = agent->GetPendingWant();
    if (pendingWant == nullptr) {
        WANT_AGENT_LOGE("WantAgentHelper::GetUid PendingWant invalid input param.");
        return -1;
    }

    return pendingWant->GetUid(pendingWant->GetTarget());
}

std::shared_ptr<Want> WantAgentHelper::GetWant(const std::shared_ptr<WantAgent> &agent)
{
    if (agent == nullptr) {
        WANT_AGENT_LOGE("WantAgentHelper::GetWant WantAgent invalid input param.");
        return nullptr;
    }

    std::shared_ptr<PendingWant> pendingWant = agent->GetPendingWant();
    if (pendingWant == nullptr) {
        WANT_AGENT_LOGE("WantAgentHelper::GetWant PendingWant invalid input param.");
        return nullptr;
    }

    return pendingWant->GetWant(pendingWant->GetTarget());
}

void WantAgentHelper::RegisterCancelListener(
    const std::shared_ptr<CancelListener> &cancelListener, const std::shared_ptr<WantAgent> &agent)
{
    if (agent == nullptr) {
        WANT_AGENT_LOGE("WantAgentHelper::GetWant WantAgent invalid input param.");
        return;
    }

    std::shared_ptr<PendingWant> pendingWant = agent->GetPendingWant();
    if (pendingWant == nullptr) {
        WANT_AGENT_LOGE("WantAgentHelper::GetWant PendingWant invalid input param.");
        return;
    }

    pendingWant->RegisterCancelListener(cancelListener, pendingWant->GetTarget());
}

void WantAgentHelper::UnregisterCancelListener(
    const std::shared_ptr<CancelListener> &cancelListener, const std::shared_ptr<WantAgent> &agent)
{
    if (agent == nullptr) {
        WANT_AGENT_LOGE("WantAgentHelper::GetWant WantAgent invalid input param.");
        return;
    }

    std::shared_ptr<PendingWant> pendingWant = agent->GetPendingWant();
    if (pendingWant == nullptr) {
        WANT_AGENT_LOGE("WantAgentHelper::GetWant PendingWant invalid input param.");
        return;
    }

    pendingWant->UnregisterCancelListener(cancelListener, pendingWant->GetTarget());
}
}  // namespace OHOS::Notification::WantAgent