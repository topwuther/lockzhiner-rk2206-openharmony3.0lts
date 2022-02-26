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

#ifndef BASE_NOTIFICATION_ANS_STANDARD_FRAMEWORKS_ANS_CORE_INCLUDE_ANS_SUBSCRIBER_STUB_H
#define BASE_NOTIFICATION_ANS_STANDARD_FRAMEWORKS_ANS_CORE_INCLUDE_ANS_SUBSCRIBER_STUB_H

#include "ans_subscriber_interface.h"
#include "iremote_stub.h"

namespace OHOS {
namespace Notification {

class AnsSubscriberStub : public IRemoteStub<IAnsSubscriber> {
public:
    AnsSubscriberStub();
    ~AnsSubscriberStub() override;
    DISALLOW_COPY_AND_MOVE(AnsSubscriberStub);

    virtual int OnRemoteRequest(
        uint32_t code, MessageParcel &data, MessageParcel &reply, MessageOption &option) override;

    void OnSubscribeResult(NotificationConstant::SubscribeResult result) override;
    void OnUnsubscribeResult(NotificationConstant::SubscribeResult result) override;
    void OnConsumed(const sptr<Notification> &notification) override;
    void OnConsumed(
        const sptr<Notification> &notification, const sptr<NotificationSortingMap> &notificationMap) override;
    void OnCanceled(const sptr<Notification> &notification) override;
    void OnCanceled(const sptr<Notification> &notification, const sptr<NotificationSortingMap> &notificationMap,
        int deleteReason) override;
    void OnUpdated(const sptr<NotificationSortingMap> &notificationMap) override;
    void OnDisturbModeChanged(NotificationConstant::DisturbMode mode) override;

private:
    std::map<uint32_t, std::function<ErrCode(MessageParcel &, MessageParcel &)>> interfaces_;

    ErrCode HandleOnSubscribe(MessageParcel &data, MessageParcel &reply);
    ErrCode HandleOnUnsubscribe(MessageParcel &data, MessageParcel &reply);
    ErrCode HandleOnConsumed(MessageParcel &data, MessageParcel &reply);
    ErrCode HandleOnConsumedMap(MessageParcel &data, MessageParcel &reply);
    ErrCode HandleOnCanceled(MessageParcel &data, MessageParcel &reply);
    ErrCode HandleOnCanceledMap(MessageParcel &data, MessageParcel &reply);
    ErrCode HandleOnUpdated(MessageParcel &data, MessageParcel &reply);
    ErrCode HandleOnDisturbModeChanged(MessageParcel &data, MessageParcel &reply);
};
}  // namespace Notification
}  // namespace OHOS

#endif  // BASE_NOTIFICATION_ANS_STANDARD_FRAMEWORKS_ANS_CORE_INCLUDE_ANS_SUBSCRIBER_STUB_H
