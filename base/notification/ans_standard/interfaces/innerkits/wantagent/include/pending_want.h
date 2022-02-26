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

#ifndef BASE_NOTIFICATION_ANS_STANDARD_KITS_NATIVE_WANTAGENT_INCLUDE_PENDING_WANT_H
#define BASE_NOTIFICATION_ANS_STANDARD_KITS_NATIVE_WANTAGENT_INCLUDE_PENDING_WANT_H

#include <string>
#include <vector>
#include <mutex>
#include <memory>
#include "cancel_listener.h"
#include "context.h"
#include "completed_dispatcher.h"
#include "event_handler.h"
#include "want.h"
#include "want_agent_constant.h"
#include "want_receiver_stub.h"
#include "want_params.h"

namespace OHOS::Notification::WantAgent {
class PendingWant final : public std::enable_shared_from_this<PendingWant>, public Parcelable {
public:
    PendingWant(){};
    PendingWant(const sptr<AAFwk::IWantSender> &target);
    PendingWant(const sptr<AAFwk::IWantSender> &target, const sptr<IRemoteObject> whitelistToken);
    virtual ~PendingWant() = default;

    WantAgentConstant::OperationType GetType(const sptr<AAFwk::IWantSender> &target);

    /**
     * Retrieve a PendingWant that will start a new ability.
     *
     * @param context The Context in which this PendingWant should start
     * the ability.
     * @param requestCode Private request code for the sender
     * @param want Want of the ability to be launched.
     * @param flags May be FLAG_ONE_SHOT, FLAG_NO_CREATE},
     * FLAG_CANCEL_CURRENT, FLAG_UPDATE_CURRENT.
     *
     * @return Returns an existing or new PendingWant matching the given
     * parameters.  May return null only if FLAG_NO_CREATE has been
     * supplied.
     */
    static std::shared_ptr<PendingWant> GetAbility(const std::shared_ptr<AppExecFwk::Context> &context, int requestCode,
        const std::shared_ptr<AAFwk::Want> &want, unsigned int flags);

    /**
     * Retrieve a PendingWant that will start a new ability
     *
     * @param context The Context in which this PendingWant should start
     * the ability.
     * @param requestCode Private request code for the sender
     * @param want Want of the ability to be launched.
     * @param flags May be FLAG_ONE_SHOT, FLAG_NO_CREATE,
     * FLAG_CANCEL_CURRENT, FLAG_UPDATE_CURRENT.
     * @param options Additional options for how the ability should be started.
     * May be null if there are no options.
     *
     * @return Returns an existing or new PendingWant matching the given
     * parameters.  May return null only if FLAG_NO_CREATE has been
     * supplied.
     */
    static std::shared_ptr<PendingWant> GetAbility(const std::shared_ptr<AppExecFwk::Context> &context, int requestCode,
        const std::shared_ptr<AAFwk::Want> &want, unsigned int flags,
        const std::shared_ptr<AAFwk::WantParams> &options);

    /**
     * Like GetAbility(Context, int, Want, int)}, but allows an
     * array of Wants to be supplied.  The last Want in the array is
     * taken as the primary key for the PendingWant, like the single Want
     * given to GetAbility(Context, int, Want, int).
     *
     * @param context The Context in which this PendingWant should start
     * the ability.
     * @param requestCode Private request code for the sender
     * @param wants Array of Wants of the abilities to be launched.
     * @param flags May be FLAG_ONE_SHOT, FLAG_NO_CREATE,
     * FLAG_CANCEL_CURRENT, FLAG_UPDATE_CURRENT.
     *
     * @return Returns an existing or new PendingWant matching the given
     * parameters.  May return null only if FLAG_NO_CREATE has been
     * supplied.
     */
    static std::shared_ptr<PendingWant> GetAbilities(const std::shared_ptr<AppExecFwk::Context> &context,
        int requestCode, std::vector<std::shared_ptr<AAFwk::Want>> &wants, unsigned int flags);

    /**
     * Like GetAbility(Context, int, Want, int)}, but allows an
     * array of Wants to be supplied.  The last Want in the array is
     * taken as the primary key for the PendingWant, like the single Want
     * given to GetAbility(Context, int, Want, int).
     *
     * @param context The Context in which this PendingWant should start
     * the ability.
     * @param requestCode Private request code for the sender
     * @param wants Array of Wants of the abilities to be launched.
     * @param flags May be FLAG_ONE_SHOT, FLAG_NO_CREATE,
     * FLAG_CANCEL_CURRENT} link #FLAG_UPDATE_CURRENT,
     * FLAG_IMMUTABLE.
     *
     * @return Returns an existing or new PendingWant matching the given
     * parameters.  May return null only if FLAG_NO_CREATE has been
     * supplied.
     */
    static std::shared_ptr<PendingWant> GetAbilities(const std::shared_ptr<AppExecFwk::Context> &context,
        int requestCode, std::vector<std::shared_ptr<AAFwk::Want>> &wants, unsigned int flags,
        const std::shared_ptr<AAFwk::WantParams> &options);

    /**
     * Retrieve a PendingWant that will perform a common event.
     *
     * @param context The Context in which this PendingWant should perform
     * the common event.
     * @param requestCode Private request code for the sender
     * @param want The Want to be common event.
     * @param flags May be FLAG_ONE_SHOT, FLAG_NO_CREATE,
     * FLAG_CANCEL_CURRENT, FLAG_UPDATE_CURRENT,
     * FLAG_IMMUTABLE.
     *
     * @return Returns an existing or new PendingWant matching the given
     * parameters.  May return null only if FLAG_NO_CREATE has been
     * supplied.
     */
    static std::shared_ptr<PendingWant> GetCommonEvent(const std::shared_ptr<AppExecFwk::Context> &context,
        int requestCode, const std::shared_ptr<AAFwk::Want> &want, unsigned int flags);

    /**
     * Note that current user will be interpreted at the time the
     * common event is sent, not when the pending want is created.
     */
    static std::shared_ptr<PendingWant> GetCommonEventAsUser(const std::shared_ptr<AppExecFwk::Context> &context,
        int requestCode, const std::shared_ptr<AAFwk::Want> &want, unsigned int flags, int uid);

    /**
     * Retrieve a PendingWant that will start a service.
     *
     * @param context The Context in which this PendingWant should start
     * the service.
     * @param requestCode Private request code for the sender
     * @param want An Want describing the service to be started.
     * @param flags May be FLAG_ONE_SHOT, FLAG_NO_CREATE,
     * FLAG_CANCEL_CURRENT, FLAG_UPDATE_CURRENT,
     * FLAG_IMMUTABLE.
     *
     * @return Returns an existing or new PendingWant matching the given
     * parameters.  May return null only if FLAG_NO_CREATE has been
     * supplied.
     */
    static std::shared_ptr<PendingWant> GetService(const std::shared_ptr<AppExecFwk::Context> &context, int requestCode,
        const std::shared_ptr<AAFwk::Want> &want, unsigned int flags);

    /**
     * Retrieve a PendingWant that will start a foreground service.
     *
     * @param context The Context in which this PendingWant should start
     * the service.
     * @param requestCode Private request code for the sender
     * @param want An Want describing the service to be started.
     * @param flags May be FLAG_ONE_SHOT, FLAG_NO_CREATE,
     * FLAG_CANCEL_CURRENT, FLAG_UPDATE_CURRENT,
     * FLAG_IMMUTABLE .
     *
     * @return Returns an existing or new PendingWant matching the given
     * parameters.  May return null only if FLAG_NO_CREATE has been
     * supplied.
     */
    static std::shared_ptr<PendingWant> GetForegroundService(const std::shared_ptr<AppExecFwk::Context> &context,
        int requestCode, const std::shared_ptr<AAFwk::Want> &want, unsigned int flags);

    /**
     * @description: Marshals a Want into a Parcel.
     * Fields in the Want are marshalled separately. If any field fails to be marshalled, false is returned.
     * @param parcel Indicates the Parcel object for marshalling.
     * @return Returns true if the marshalling is successful; returns false otherwise.
     */
    virtual bool Marshalling(Parcel &parcel) const;

    /**
     * @description: Unmarshals a Want from a Parcel.
     * Fields in the Want are unmarshalled separately. If any field fails to be unmarshalled, false is returned.
     * @param parcel Indicates the Parcel object for unmarshalling.
     * @return Returns true if the unmarshalling is successful; returns false otherwise.
     */
    static PendingWant *Unmarshalling(Parcel &parcel);

    static bool Equals(
        const std::shared_ptr<PendingWant> &targetPendingWant, const std::shared_ptr<PendingWant> &otherPendingWant);

    void Cancel(const sptr<AAFwk::IWantSender> &target);

    void Send(const sptr<AAFwk::IWantSender> &target);

    void Send(int resultCode, const sptr<AAFwk::IWantSender> &target);

    void Send(const std::shared_ptr<AppExecFwk::Context> &context, int resultCode,
        const std::shared_ptr<AAFwk::Want> &want, const sptr<AAFwk::IWantSender> &target);

    void Send(int resultCode, const sptr<CompletedDispatcher> &onCompleted, const sptr<AAFwk::IWantSender> &target);

    void Send(const std::shared_ptr<AppExecFwk::Context> &context, int resultCode,
        const std::shared_ptr<AAFwk::Want> &want, const sptr<CompletedDispatcher> &onCompleted,
        const sptr<AAFwk::IWantSender> &target);

    void Send(const std::shared_ptr<AppExecFwk::Context> &context, int resultCode,
        const std::shared_ptr<AAFwk::Want> &want, const sptr<CompletedDispatcher> &onCompleted,
        const std::string &requiredPermission, const sptr<AAFwk::IWantSender> &target);

    void Send(const std::shared_ptr<AppExecFwk::Context> &context, int resultCode,
        const std::shared_ptr<AAFwk::Want> &want, const sptr<CompletedDispatcher> &onCompleted,
        const std::string &requiredPermission, const std::shared_ptr<AAFwk::WantParams> &options,
        const sptr<AAFwk::IWantSender> &target);

    int SendAndReturnResult(const std::shared_ptr<AppExecFwk::Context> &context, int resultCode,
        const std::shared_ptr<AAFwk::Want> &want, const sptr<CompletedDispatcher> &onCompleted,
        const std::string &requiredPermission, const std::shared_ptr<AAFwk::WantParams> &options,
        const sptr<AAFwk::IWantSender> &target);

    std::string GetBundleName(const sptr<AAFwk::IWantSender> &target);

    int GetUid(const sptr<AAFwk::IWantSender> &target);

    sptr<AAFwk::IWantSender> GetTarget();

    void SetTarget(const sptr<AAFwk::IWantSender> &target);

    void RegisterCancelListener(
        const std::shared_ptr<CancelListener> &cancelListener, const sptr<AAFwk::IWantSender> &target);

    void NotifyCancelListeners(int32_t resultCode);

    void UnregisterCancelListener(
        const std::shared_ptr<CancelListener> &cancelListener, const sptr<AAFwk::IWantSender> &target);

    int GetHashCode(const sptr<AAFwk::IWantSender> &target);

    std::shared_ptr<AAFwk::Want> GetWant(const sptr<AAFwk::IWantSender> &target);

private:
    std::mutex lock_object;
    sptr<AAFwk::IWantSender> target_;
    sptr<AAFwk::IWantReceiver> cancelReceiver_;
    sptr<IRemoteObject> whitelistToken_;
    std::vector<std::shared_ptr<CancelListener>> cancelListeners_;

    class CancelReceiver : public AAFwk::WantReceiverStub {
    private:
        std::weak_ptr<PendingWant> outerInstance_;

    public:
        explicit CancelReceiver(const std::weak_ptr<PendingWant> &outerInstance);
        virtual ~CancelReceiver() = default;

        void Send(const int32_t resultCode) override;
        void PerformReceive(const AAFwk::Want &want, int resultCode, const std::string &data,
            const AAFwk::WantParams &extras, bool serialized, bool sticky, int sendingUser) override;
    };

    static std::shared_ptr<PendingWant> BuildServicePendingWant(const std::shared_ptr<AppExecFwk::Context> &context,
        int requestCode, const std::shared_ptr<AAFwk::Want> &want, unsigned int flags,
        WantAgentConstant::OperationType serviceKind);
};
}  // namespace OHOS::Notification::WantAgent
#endif  // BASE_NOTIFICATION_ANS_STANDARD_KITS_NATIVE_WANTAGENT_INCLUDE_PENDING_WANT_H