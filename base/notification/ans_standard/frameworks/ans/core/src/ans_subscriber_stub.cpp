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

#include <functional>

#include "ans_subscriber_stub.h"

#include "ans_const_define.h"
#include "ans_inner_errors.h"
#include "ans_log_wrapper.h"
#include "message_option.h"
#include "message_parcel.h"
#include "parcel.h"

namespace OHOS {
namespace Notification {
AnsSubscriberStub::AnsSubscriberStub()
{
    interfaces_.emplace(ON_SUBSCRIBE,
        std::bind(&AnsSubscriberStub::HandleOnSubscribe, this, std::placeholders::_1, std::placeholders::_2));
    interfaces_.emplace(ON_UNSUBSCRIBE,
        std::bind(&AnsSubscriberStub::HandleOnUnsubscribe, this, std::placeholders::_1, std::placeholders::_2));
    interfaces_.emplace(ON_CONSUMED,
        std::bind(&AnsSubscriberStub::HandleOnConsumed, this, std::placeholders::_1, std::placeholders::_2));
    interfaces_.emplace(ON_CONSUMED_MAP,
        std::bind(&AnsSubscriberStub::HandleOnConsumedMap, this, std::placeholders::_1, std::placeholders::_2));
    interfaces_.emplace(ON_CANCELED,
        std::bind(&AnsSubscriberStub::HandleOnCanceled, this, std::placeholders::_1, std::placeholders::_2));
    interfaces_.emplace(ON_CANCELED_MAP,
        std::bind(&AnsSubscriberStub::HandleOnCanceledMap, this, std::placeholders::_1, std::placeholders::_2));
    interfaces_.emplace(
        ON_UPDATED, std::bind(&AnsSubscriberStub::HandleOnUpdated, this, std::placeholders::_1, std::placeholders::_2));
    interfaces_.emplace(ON_DISTURB_MODE_CHANGED,
        std::bind(&AnsSubscriberStub::HandleOnDisturbModeChanged, this, std::placeholders::_1, std::placeholders::_2));
}

AnsSubscriberStub::~AnsSubscriberStub()
{}

int32_t AnsSubscriberStub::OnRemoteRequest(
    uint32_t code, MessageParcel &data, MessageParcel &reply, MessageOption &flags)
{
    std::u16string descriptor = AnsSubscriberStub::GetDescriptor();
    std::u16string remoteDescriptor = data.ReadInterfaceToken();
    if (descriptor != remoteDescriptor) {
        ANS_LOGW("[OnRemoteRequest] fail: invalid interface token!");
        return OBJECT_NULL;
    }

    auto it = interfaces_.find(code);
    if (it == interfaces_.end()) {
        ANS_LOGW("[OnRemoteRequest] fail: unknown code!");
        return IRemoteStub<IAnsSubscriber>::OnRemoteRequest(code, data, reply, flags);
    }

    auto fun = it->second;
    if (fun == nullptr) {
        ANS_LOGW("[OnRemoteRequest] fail: not find function!");
        return IRemoteStub<IAnsSubscriber>::OnRemoteRequest(code, data, reply, flags);
    }

    fun(data, reply);
    return NO_ERROR;
}

ErrCode AnsSubscriberStub::HandleOnSubscribe(MessageParcel &data, MessageParcel &reply)
{
    NotificationConstant::SubscribeResult result =
        static_cast<NotificationConstant::SubscribeResult>(data.ReadUint32());
    OnSubscribeResult(result);
    return ERR_OK;
}

ErrCode AnsSubscriberStub::HandleOnUnsubscribe(MessageParcel &data, MessageParcel &reply)
{
    NotificationConstant::SubscribeResult result =
        static_cast<NotificationConstant::SubscribeResult>(data.ReadUint32());
    OnUnsubscribeResult(result);
    return ERR_OK;
}

ErrCode AnsSubscriberStub::HandleOnConsumed(MessageParcel &data, MessageParcel &reply)
{
    sptr<Notification> notification = data.ReadParcelable<Notification>();
    if (!notification) {
        ANS_LOGW("[HandleOnConsumed] fail: notification ReadParcelable failed");
        return ERR_ANS_PARCELABLE_FAILED;
    }

    OnConsumed(notification);
    return ERR_OK;
}

ErrCode AnsSubscriberStub::HandleOnConsumedMap(MessageParcel &data, MessageParcel &reply)
{
    sptr<Notification> notification = data.ReadParcelable<Notification>();
    if (!notification) {
        ANS_LOGW("[HandleOnConsumedMap] fail: notification ReadParcelable failed");
        return ERR_ANS_PARCELABLE_FAILED;
    }

    bool existMap = false;
    if (!data.ReadBool(existMap)) {
        ANS_LOGW("[HandleOnConsumedMap] fail: read existMap failed");
        return ERR_ANS_PARCELABLE_FAILED;
    }

    sptr<NotificationSortingMap> notificationMap = nullptr;
    if (existMap) {
        notificationMap = data.ReadParcelable<NotificationSortingMap>();
        if (notificationMap == nullptr) {
            ANS_LOGW("[HandleOnConsumedMap] fail: read NotificationSortingMap failed");
            return ERR_ANS_PARCELABLE_FAILED;
        }
    }

    OnConsumed(notification, notificationMap);
    return ERR_OK;
}

ErrCode AnsSubscriberStub::HandleOnCanceled(MessageParcel &data, MessageParcel &reply)
{
    sptr<Notification> notification = data.ReadParcelable<Notification>();
    if (!notification) {
        ANS_LOGW("[HandleOnCanceled] fail: notification ReadParcelable failed");
        return ERR_ANS_PARCELABLE_FAILED;
    }

    OnCanceled(notification);
    return ERR_OK;
}

ErrCode AnsSubscriberStub::HandleOnCanceledMap(MessageParcel &data, MessageParcel &reply)
{
    sptr<Notification> notification = data.ReadParcelable<Notification>();
    if (!notification) {
        ANS_LOGW("[HandleOnCanceledMap] fail: notification ReadParcelable failed");
        return ERR_ANS_PARCELABLE_FAILED;
    }

    bool existMap = false;
    if (!data.ReadBool(existMap)) {
        ANS_LOGW("[HandleOnCanceledMap] fail: read existMap failed");
        return ERR_ANS_PARCELABLE_FAILED;
    }

    sptr<NotificationSortingMap> notificationMap = nullptr;
    if (existMap) {
        notificationMap = data.ReadParcelable<NotificationSortingMap>();
        if (notificationMap == nullptr) {
            ANS_LOGW("[HandleOnCanceledMap] fail: read NotificationSortingMap failed");
            return ERR_ANS_PARCELABLE_FAILED;
        }
    }

    int reason = 0;
    if (!data.ReadInt32(reason)) {
        ANS_LOGW("[HandleOnCanceledMap] fail: read reason failed");
        return ERR_ANS_PARCELABLE_FAILED;
    }

    OnCanceled(notification, notificationMap, reason);
    return ERR_OK;
}

ErrCode AnsSubscriberStub::HandleOnUpdated(MessageParcel &data, MessageParcel &reply)
{
    sptr<NotificationSortingMap> notificationMap = data.ReadParcelable<NotificationSortingMap>();
    if (!notificationMap) {
        ANS_LOGW("[HandleOnUpdated] fail: notificationMap ReadParcelable failed");
        return ERR_ANS_PARCELABLE_FAILED;
    }

    OnUpdated(notificationMap);
    return ERR_OK;
}

ErrCode AnsSubscriberStub::HandleOnDisturbModeChanged(MessageParcel &data, MessageParcel &reply)
{
    NotificationConstant::DisturbMode mode = static_cast<NotificationConstant::DisturbMode>(data.ReadUint32());
    OnDisturbModeChanged(mode);
    return ERR_OK;
}

void AnsSubscriberStub::OnSubscribeResult(NotificationConstant::SubscribeResult result)
{}

void AnsSubscriberStub::OnUnsubscribeResult(NotificationConstant::SubscribeResult result)
{}

void AnsSubscriberStub::OnConsumed(const sptr<Notification> &notification)
{}

void AnsSubscriberStub::OnConsumed(
    const sptr<Notification> &notification, const sptr<NotificationSortingMap> &notificationMap)
{}

void AnsSubscriberStub::OnCanceled(const sptr<Notification> &notification)
{}

void AnsSubscriberStub::OnCanceled(
    const sptr<Notification> &notification, const sptr<NotificationSortingMap> &notificationMap, int deleteReason)
{}

void AnsSubscriberStub::OnUpdated(const sptr<NotificationSortingMap> &notificationMap)
{}

void AnsSubscriberStub::OnDisturbModeChanged(NotificationConstant::DisturbMode mode)
{}

}  // namespace Notification
}  // namespace OHOS
