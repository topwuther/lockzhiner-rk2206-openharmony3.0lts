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

#ifndef BASE_NOTIFICATION_ANS_STANDARD_SERVICES_ANS_INCLUDE_NOTIFICATION_RECORD_H
#define BASE_NOTIFICATION_ANS_STANDARD_SERVICES_ANS_INCLUDE_NOTIFICATION_RECORD_H

#include "refbase.h"

#include "notification.h"
#include "notification_bundle_option.h"
#include "notification_request.h"
#include "notification_slot.h"

namespace OHOS {
namespace Notification {

struct NotificationRecord {
    sptr<NotificationBundleOption> bundleOption;
    sptr<NotificationRequest> request;
    sptr<Notification> notification;
    sptr<NotificationSlot> slot;
};

}  // namespace Notification
}  // namespace OHOS

#endif  // BASE_NOTIFICATION_ANS_STANDARD_SERVICES_ANS_INCLUDE_NOTIFICATION_RECORD_H