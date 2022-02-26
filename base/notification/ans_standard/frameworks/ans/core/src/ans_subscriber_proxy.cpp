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

#include "ans_subscriber_proxy.h"

#include "ans_inner_errors.h"
#include "ans_log_wrapper.h"
#include "message_option.h"
#include "message_parcel.h"

namespace OHOS {
namespace Notification {
AnsSubscriberProxy::AnsSubscriberProxy(const sptr<IRemoteObject> &impl) : IRemoteProxy<IAnsSubscriber>(impl)
{}

AnsSubscriberProxy::~AnsSubscriberProxy()
{}

ErrCode AnsSubscriberProxy::InnerTransact(
    uint32_t code, MessageOption &flags, MessageParcel &data, MessageParcel &reply)
{
    auto remote = Remote();
    if (remote == nullptr) {
        ANS_LOGW("[InnerTransact] fail: get Remote fail code %{public}d", code);
        return ERR_DEAD_OBJECT;
    }

    int err = remote->SendRequest(code, data, reply, flags);
    switch (err) {
        case NO_ERROR: {
            return ERR_OK;
        }
        case DEAD_OBJECT: {
            ANS_LOGW("[InnerTransact] fail: ipcErr=%{public}d code %{public}d", err, code);
            return ERR_DEAD_OBJECT;
        }
        default: {
            ANS_LOGW("[InnerTransact] fail: ipcErr=%{public}d code %{public}d", err, code);
            return ERR_ANS_TRANSACT_FAILED;
        }
    }
}

void AnsSubscriberProxy::OnSubscribeResult(NotificationConstant::SubscribeResult result)
{
    MessageParcel data;
    if (!data.WriteInterfaceToken(AnsSubscriberProxy::GetDescriptor())) {
        ANS_LOGW("[OnSubscribeResult] fail: write interface token failed.");
        return;
    }

    if (!data.WriteUint32(result)) {
        ANS_LOGW("[OnSubscribeResult] fail: write result failed");
        return;
    }

    MessageParcel reply;
    MessageOption option = {MessageOption::TF_ASYNC};
    ErrCode transactResult = InnerTransact(ON_SUBSCRIBE, option, data, reply);
    if (transactResult != ERR_OK) {
        ANS_LOGW("[OnSubscribeResult] fail: transact ErrCode=ERR_ANS_TRANSACT_FAILED");
        return;
    }
}

void AnsSubscriberProxy::OnUnsubscribeResult(NotificationConstant::SubscribeResult result)
{
    MessageParcel data;
    if (!data.WriteInterfaceToken(AnsSubscriberProxy::GetDescriptor())) {
        ANS_LOGW("[OnUnsubscribeResult] fail: write interface token failed.");
        return;
    }

    if (!data.WriteUint32(result)) {
        ANS_LOGW("[OnUnsubscribeResult] fail: write result failed");
        return;
    }

    MessageParcel reply;
    MessageOption option = {MessageOption::TF_ASYNC};
    ErrCode transactResult = InnerTransact(ON_UNSUBSCRIBE, option, data, reply);
    if (transactResult != ERR_OK) {
        ANS_LOGW("[OnUnsubscribeResult] fail: transact ErrCode=ERR_ANS_TRANSACT_FAILED");
        return;
    }
}

void AnsSubscriberProxy::OnConsumed(const sptr<Notification> &notification)
{
    if (notification == nullptr) {
        ANS_LOGW("[OnConsumed] fail: notification is nullptr.");
        return;
    }

    MessageParcel data;
    if (!data.WriteInterfaceToken(AnsSubscriberProxy::GetDescriptor())) {
        ANS_LOGW("[OnConsumed] fail: write interface token failed.");
        return;
    }

    if (!data.WriteParcelable(notification)) {
        ANS_LOGW("[OnConsumed] fail: write notification failed.");
        return;
    }

    MessageParcel reply;
    MessageOption option = {MessageOption::TF_ASYNC};
    ErrCode result = InnerTransact(ON_CONSUMED, option, data, reply);
    if (result != ERR_OK) {
        ANS_LOGW("[OnConsumed] fail: transact ErrCode=ERR_ANS_TRANSACT_FAILED");
        return;
    }
}

void AnsSubscriberProxy::OnConsumed(
    const sptr<Notification> &notification, const sptr<NotificationSortingMap> &notificationMap)
{
    if (notification == nullptr) {
        ANS_LOGW("[OnConsumed] fail: notification is nullptr.");
        return;
    }

    MessageParcel data;
    if (!data.WriteInterfaceToken(AnsSubscriberProxy::GetDescriptor())) {
        ANS_LOGW("[OnConsumed] fail: write interface token failed.");
        return;
    }

    if (!data.WriteParcelable(notification)) {
        ANS_LOGW("[OnConsumed] fail: write notification failed.");
        return;
    }

    if (!data.WriteBool(notificationMap != nullptr)) {
        ANS_LOGW("[OnConsumed] fail: write existMap failed");
        return;
    }

    if (notificationMap != nullptr) {
        if (!data.WriteParcelable(notificationMap)) {
            ANS_LOGW("[OnConsumed] fail: write notificationMap failed");
            return;
        }
    }

    MessageParcel reply;
    MessageOption option = {MessageOption::TF_ASYNC};
    ErrCode result = InnerTransact(ON_CONSUMED_MAP, option, data, reply);
    if (result != ERR_OK) {
        ANS_LOGW("[OnConsumed] fail: transact ErrCode=ERR_ANS_TRANSACT_FAILED");
        return;
    }
}

void AnsSubscriberProxy::OnCanceled(const sptr<Notification> &notification)
{
    if (notification == nullptr) {
        ANS_LOGW("[OnCanceled] fail: notification is nullptr.");
        return;
    }

    MessageParcel data;
    if (!data.WriteInterfaceToken(AnsSubscriberProxy::GetDescriptor())) {
        ANS_LOGW("[OnCanceled] fail: write interface token failed.");
        return;
    }

    if (!data.WriteParcelable(notification)) {
        ANS_LOGW("[OnCanceled] fail: write notification failed.");
        return;
    }

    MessageParcel reply;
    MessageOption option = {MessageOption::TF_ASYNC};
    ErrCode result = InnerTransact(ON_CANCELED, option, data, reply);
    if (result != ERR_OK) {
        ANS_LOGW("[OnCanceled] fail: transact ErrCode=ERR_ANS_TRANSACT_FAILED");
        return;
    }
}

void AnsSubscriberProxy::OnCanceled(
    const sptr<Notification> &notification, const sptr<NotificationSortingMap> &notificationMap, int deleteReason)
{
    if (notification == nullptr) {
        ANS_LOGW("[OnCanceled] fail: notification is nullptr.");
        return;
    }

    MessageParcel data;
    if (!data.WriteInterfaceToken(AnsSubscriberProxy::GetDescriptor())) {
        ANS_LOGW("[OnCanceled] fail: write interface token failed.");
        return;
    }

    if (!data.WriteParcelable(notification)) {
        ANS_LOGW("[OnCanceled] fail: write notification failed.");
        return;
    }

    if (!data.WriteBool(notificationMap != nullptr)) {
        ANS_LOGW("[OnCanceled] fail: write existMap failed");
        return;
    }

    if (notificationMap != nullptr) {
        if (!data.WriteParcelable(notificationMap)) {
            ANS_LOGW("[OnCanceled] fail: write notificationMap failed");
            return;
        }
    }

    if (!data.WriteInt32(deleteReason)) {
        ANS_LOGW("[OnCanceled] fail: write deleteReason failed.");
        return;
    }

    MessageParcel reply;
    MessageOption option = {MessageOption::TF_ASYNC};
    ErrCode result = InnerTransact(ON_CANCELED_MAP, option, data, reply);
    if (result != ERR_OK) {
        ANS_LOGW("[OnCanceled] fail: transact ErrCode=ERR_ANS_TRANSACT_FAILED");
        return;
    }
}

void AnsSubscriberProxy::OnUpdated(const sptr<NotificationSortingMap> &notificationMap)
{
    if (notificationMap == nullptr) {
        ANS_LOGW("[OnUpdated] fail: notificationMap is empty.");
        return;
    }

    MessageParcel data;
    if (!data.WriteInterfaceToken(AnsSubscriberProxy::GetDescriptor())) {
        ANS_LOGW("[OnUpdated] fail: write interface token failed.");
        return;
    }

    if (!data.WriteParcelable(notificationMap)) {
        ANS_LOGW("[OnUpdated] fail: write notificationMap failed.");
        return;
    }

    MessageParcel reply;
    MessageOption option = {MessageOption::TF_ASYNC};
    ErrCode result = InnerTransact(ON_UPDATED, option, data, reply);
    if (result != ERR_OK) {
        ANS_LOGW("[OnUpdated] fail: transact ErrCode=ERR_ANS_TRANSACT_FAILED");
        return;
    }
}

void AnsSubscriberProxy::OnDisturbModeChanged(NotificationConstant::DisturbMode mode)
{
    MessageParcel data;
    if (!data.WriteInterfaceToken(AnsSubscriberProxy::GetDescriptor())) {
        ANS_LOGW("[OnDisturbModeChanged] fail: write interface token failed.");
        return;
    }

    if (!data.WriteUint32(mode)) {
        ANS_LOGW("[OnDisturbModeChanged] fail: write mode failed");
        return;
    }

    MessageParcel reply;
    MessageOption option = {MessageOption::TF_ASYNC};
    ErrCode result = InnerTransact(ON_DISTURB_MODE_CHANGED, option, data, reply);
    if (result != ERR_OK) {
        ANS_LOGW("[OnDisturbModeChanged] fail: transact ErrCode=ERR_ANS_TRANSACT_FAILED");
        return;
    }
}

}  // namespace Notification
}  // namespace OHOS
