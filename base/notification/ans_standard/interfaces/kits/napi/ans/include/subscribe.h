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
#ifndef BASE_NOTIFICATION_ANS_STANDARD_KITS_NAPI_INCLUDE_SUBSCRIBE_H
#define BASE_NOTIFICATION_ANS_STANDARD_KITS_NAPI_INCLUDE_SUBSCRIBE_H

#include "common.h"

namespace OHOS {
namespace NotificationNapi {
using namespace OHOS::Notification;

class SubscriberInstance : public NotificationSubscriber {
public:
    SubscriberInstance();

    virtual ~SubscriberInstance();

    virtual void OnCanceled(const std::shared_ptr<OHOS::Notification::Notification> &request) override;

    virtual void OnCanceled(const std::shared_ptr<OHOS::Notification::Notification> &request,
        const std::shared_ptr<NotificationSortingMap> &sortingMap, int deleteReason) override;

    virtual void OnConsumed(const std::shared_ptr<OHOS::Notification::Notification> &request) override;

    virtual void OnConsumed(const std::shared_ptr<OHOS::Notification::Notification> &request,
        const std::shared_ptr<NotificationSortingMap> &sortingMap) override;

    virtual void OnUpdate(const std::shared_ptr<NotificationSortingMap> &sortingMap) override;

    virtual void OnSubscribeResult(NotificationConstant::SubscribeResult result) override;

    virtual void OnUnsubscribeResult(NotificationConstant::SubscribeResult result) override;

    virtual void OnDied() override;

    virtual void OnDisturbModeChanged(int disturbMode) override;

    void SetCallbackInfo(const napi_env &env, const std::string &type, const napi_ref &ref);

private:
    void SetCancelCallbackInfo(const napi_env &env, const napi_ref &ref);

    void SetConsumeCallbackInfo(const napi_env &env, const napi_ref &ref);

    void SetUpdateCallbackInfo(const napi_env &env, const napi_ref &ref);

    void SetSubscribeCallbackInfo(const napi_env &env, const napi_ref &ref);

    void SetUnsubscribeCallbackInfo(const napi_env &env, const napi_ref &ref);

    void SetDieCallbackInfo(const napi_env &env, const napi_ref &ref);

    void SetDisturbModeCallbackInfo(const napi_env &env, const napi_ref &ref);

private:
    struct CallbackInfo {
        napi_env env = nullptr;
        napi_ref ref = nullptr;
    };

    CallbackInfo canceCallbackInfo_;
    CallbackInfo consumeCallbackInfo_;
    CallbackInfo updateCallbackInfo_;
    CallbackInfo subscribeCallbackInfo_;
    CallbackInfo unsubscribeCallbackInfo_;
    CallbackInfo dieCallbackInfo_;
    CallbackInfo disturbModeCallbackInfo_;
};

struct SubscriberInstancesInfo {
    napi_ref ref = nullptr;
    SubscriberInstance *subscriber = nullptr;
};

static std::mutex mutex_;
static std::vector<SubscriberInstancesInfo> subscriberInstances_;

bool HasNotificationSubscriber(const napi_env &env, const napi_value &value, SubscriberInstancesInfo &subscriberInfo);
bool AddSubscriberInstancesInfo(const napi_env &env, const SubscriberInstancesInfo &subscriberInfo);
bool DelSubscriberInstancesInfo(const napi_env &env, SubscriberInstance *subscriber);

napi_value Subscribe(napi_env env, napi_callback_info info);

}  // namespace NotificationNapi
}  // namespace OHOS
#endif  // BASE_NOTIFICATION_ANS_STANDARD_KITS_NAPI_INCLUDE_SUBSCRIBE_H