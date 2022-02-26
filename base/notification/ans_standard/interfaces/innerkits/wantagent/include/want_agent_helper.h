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

#ifndef BASE_NOTIFICATION_ANS_STANDARD_KITS_NATIVE_WANTAGENT_INCLUDE_WANT_AGENT_HELPER_H
#define BASE_NOTIFICATION_ANS_STANDARD_KITS_NATIVE_WANTAGENT_INCLUDE_WANT_AGENT_HELPER_H

#include <string>
#include <memory>
#include "context.h"
#include "completed_callback.h"
#include "completed_dispatcher.h"
#include "event_handler.h"
#include "trigger_info.h"
#include "want.h"
#include "want_agent.h"
#include "want_agent_info.h"
#include "want_params.h"

namespace OHOS::Notification::WantAgent {
/**
 * A helper class used to obtain, trigger, cancel, and compare WantAgent objects and to obtain
 * the bundle name, UID, and hash code value of an WantAgent object.
 *
 */

static const int FLAG_ONE_SHOT = 1 << 30;
static const int FLAG_NO_CREATE = 1 << 29;
static const int FLAG_CANCEL_CURRENT = 1 << 28;
static const int FLAG_UPDATE_CURRENT = 1 << 27;
static const int FLAG_IMMUTABLE = 1 << 26;

class WantAgentHelper final : public std::enable_shared_from_this<WantAgentHelper> {
private:
    WantAgentHelper();
    virtual ~WantAgentHelper() = default;

public:
    /**
     * Obtains an WantAgent object.
     * The WantAgent class does not have any constructor, and you can only use this method to create an
     * WantAgent object.
     *
     * @param context Indicates the context of the caller. This parameter cannot be null.
     * @param paramsInfo Indicates the WantAgentInfo object that contains parameters of the
     * WantAgent object to create.
     * @return Returns the created WantAgent object.
     */
    static std::shared_ptr<WantAgent> GetWantAgent(
        const std::shared_ptr<OHOS::AppExecFwk::Context> &context, const WantAgentInfo &paramsInfo);

    /**
     * Obtains an WantAgent object.
     *
     * The WantAgent class does not have any constructor, and you can only use this method to create an
     * WantAgent object.
     *
     * @param paramsInfo Indicates the WantAgentInfo object that contains parameters of the
     * WantAgent object to create.
     * @return Returns the created WantAgent object.
     */
    static std::shared_ptr<WantAgent> GetWantAgent(const WantAgentInfo &paramsInfo);

    /**
     * Obtains an WantAgent object operation type.
     *
     * @param agent Indicates the WantAgent to trigger.
     * @return Returns the created WantAgent object.
     */
    static WantAgentConstant::OperationType GetType(const std::shared_ptr<WantAgent> &agent);

    /**
     * Triggers an WantAgent.
     *
     * After this method is called, events associated with the specified WantAgent will be executed,
     * such as starting an ability or sending a common event.
     *
     * @param context Indicates the context of the caller. This parameter cannot be null.
     * @param agent Indicates the WantAgent to trigger.
     * @param onCompleted Indicates the callback method to be called after the WantAgent is triggered.
     * This parameter can be null.
     * @param handler Indicates the thread for executing the callback indicated by OnCompleted.
     * If this parameter is null, the callback method will be executed in a thread in the thread pool of
     * the current process.
     * @param paramsInfo Indicates the TriggerInfo object that contains triggering parameters.
     */
    static void TriggerWantAgent(const std::shared_ptr<AppExecFwk::Context> &context,
        const std::shared_ptr<WantAgent> &agent, const std::shared_ptr<CompletedCallback> &callback,
        const TriggerInfo &paramsInfo);

    /**
     * Cancels an WantAgent.
     *
     * Only the application that creates the WantAgent can cancel it.
     *
     * @param agent Indicates the WantAgent to cancel.
     */
    static void Cancel(const std::shared_ptr<WantAgent> &agent);

    /**
     * Checks whether two WantAgent objects are the same.
     *
     * @param agent Indicates one of the WantAgent object to compare.
     * @param otherAgent Indicates the other WantAgent object to compare.
     * @return Returns true If the two objects are the same; returns false otherwise.
     */
    static bool JudgeEquality(const std::shared_ptr<WantAgent> &agent, const std::shared_ptr<WantAgent> &otherAgent);

    /**
     * Obtains the hash code value of an WantAgent.
     *
     * @param agent Indicates the antAgent whose hash code value is to be obtained.
     * @return Returns the hash code value of the WantAgent.
     */
    static int GetHashCode(const std::shared_ptr<WantAgent> &agent);

    /**
     * Obtains the bundle name of an WantAgent.
     *
     * @param agent Indicates the WantAgent whose bundle name is to be obtained.
     * @return Returns the bundle name of the WantAgent if any; returns {@code null} otherwise.
     */
    static std::string GetBundleName(const std::shared_ptr<WantAgent> &agent);

    /**
     * Obtains the user ID (UID) of an WantAgent.
     *
     * @param agent Indicates the WantAgent whose UID is to be obtained.
     * @return Returns the UID of the WantAgent if any; returns -1} otherwise.
     */
    static int GetUid(const std::shared_ptr<WantAgent> &agent);

    /**
     * Obtains the Want WantAgent.
     *
     * @param agent Indicates the WantAgent whose Want is to be obtained.
     * @return Returns the Want of the WantAgent.
     */
    static std::shared_ptr<AAFwk::Want> GetWant(const std::shared_ptr<WantAgent> &agent);

    /**
     * Register Cancel function Listener.
     *
     * @param cancelListener Register listener object.
     * @param agent Indicates the WantAgent whose bundle name is to be obtained.
     */
    static void RegisterCancelListener(
        const std::shared_ptr<CancelListener> &cancelListener, const std::shared_ptr<WantAgent> &agent);

    /**
     * Unregister Cancel function Listener.
     *
     * @param cancelListener Register listener object.
     * @param agent Indicates the WantAgent whose bundle name is to be obtained.
     */
    static void UnregisterCancelListener(
        const std::shared_ptr<CancelListener> &cancelListener, const std::shared_ptr<WantAgent> &agent);

private:
    static void Send(const std::shared_ptr<AppExecFwk::Context> &context,
        const std::shared_ptr<PendingWant> &pendingWant, WantAgentConstant::OperationType type,
        const sptr<CompletedDispatcher> &callBack, const TriggerInfo &paramsInfo);

    static unsigned int FlagsTransformer(const std::vector<WantAgentConstant::Flags> &flags);
};
}  // namespace OHOS::Notification::WantAgent
#endif  // BASE_NOTIFICATION_ANS_STANDARD_KITS_NATIVE_WANTAGENT_INCLUDE_WANT_AGENT_HELPER_H