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

#ifndef BASE_NOTIFICATION_ANS_STANDARD_FRAMEWORKS_ANS_CORE_INCLUDE_ANS_SUBSCRIBER_PROXY_H
#define BASE_NOTIFICATION_ANS_STANDARD_FRAMEWORKS_ANS_CORE_INCLUDE_ANS_SUBSCRIBER_PROXY_H

#include "ans_subscriber_interface.h"
#include "iremote_proxy.h"

namespace OHOS {
namespace Notification {

class AnsSubscriberProxy : public IRemoteProxy<IAnsSubscriber> {
public:
    AnsSubscriberProxy() = delete;
    explicit AnsSubscriberProxy(const sptr<IRemoteObject> &impl);
    ~AnsSubscriberProxy() override;
    DISALLOW_COPY_AND_MOVE(AnsSubscriberProxy);

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
    ErrCode InnerTransact(uint32_t code, MessageOption &flags, MessageParcel &data, MessageParcel &reply);
    static inline BrokerDelegator<AnsSubscriberProxy> delegator_;
};
}  // namespace Notification
}  // namespace OHOS

#endif  // BASE_NOTIFICATION_ANS_STANDARD_FRAMEWORKS_ANS_CORE_INCLUDE_ANS_SUBSCRIBER_PROXY_H
