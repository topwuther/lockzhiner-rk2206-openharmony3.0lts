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
#ifndef PERMISSION_CHANGE_RECIEVER_H
#define PERMISSION_CHANGE_RECIEVER_H

#include "common_event.h"
#include "common_event_manager.h"
#include "external_deps.h"
#include "permission_fetcher.h"
#include "permission_record_manager.h"

#include "permission_log.h"

namespace OHOS {
namespace Security {
namespace Permission {

class PermissionChangeReceiver final {
public:
    static void Register();
    static void OnReceive(const EventFwk::CommonEventData &data);

private:
    static void PackageAdded(int32_t uid);
    static void PackageRemoved(int32_t uid);
    static void PackageReplaced(int32_t uid);
    static void UserRemoved(int32_t userId);

public:
    class AppEventSubscriber : public EventFwk::CommonEventSubscriber {
    public:
        // explicit AppEventSubscriber(const EventFwk::CommonEventSubscribeInfo &sp) : CommonEventSubscriber(sp){};
        // ~CommonEventSubscriber() = default;
        // ~AppEventSubscriber() = default;
        /**
         * A constructor used to create a CommonEventSubscriber instance with the
         * subscribeInfo parameter passed.
         *
         * @param subscriberInfo the subscriberInfo
         */
        explicit AppEventSubscriber(const EventFwk::CommonEventSubscribeInfo &subscribeInfo)
            : EventFwk::CommonEventSubscriber(subscribeInfo){};

        /**
         * A deconstructor used to deconstruct
         *
         */
        ~AppEventSubscriber() = default;

        virtual void OnReceiveEvent(const EventFwk::CommonEventData &data) override;
    };
};

}  // namespace Permission
}  // namespace Security
}  // namespace OHOS
#endif  // PERMISSION_CHANGE_MANAGER_H