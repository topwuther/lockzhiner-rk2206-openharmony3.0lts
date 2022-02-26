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

#include "init.h"
#include "cancel.h"
#include "constant.h"
#include "display_badge.h"
#include "disturb_mode.h"
#include "enable_notification.h"
#include "get_active.h"
#include "publish.h"
#include "remove.h"
#include "slot.h"
#include "subscribe.h"
#include "unsubscribe.h"

namespace OHOS {
namespace NotificationNapi {
using namespace OHOS::Notification;

EXTERN_C_START

napi_value NotificationInit(napi_env env, napi_value exports)
{
    ANS_LOGI("notificationInit start");

    napi_property_descriptor desc[] = {
        DECLARE_NAPI_FUNCTION("subscribe", Subscribe),
        DECLARE_NAPI_FUNCTION("unsubscribe", Unsubscribe),
        DECLARE_NAPI_FUNCTION("publish", Publish),
        DECLARE_NAPI_FUNCTION("remove", Remove),
        DECLARE_NAPI_FUNCTION("removeAll", RemoveAll),
        DECLARE_NAPI_FUNCTION("getAllActiveNotifications", GetAllActiveNotifications),
        DECLARE_NAPI_FUNCTION("getActiveNotifications", GetActiveNotifications),
        DECLARE_NAPI_FUNCTION("getActiveNotificationCount", GetActiveNotificationCount),
        DECLARE_NAPI_FUNCTION("cancel", Cancel),
        DECLARE_NAPI_FUNCTION("cancelAll", CancelAll),
        DECLARE_NAPI_FUNCTION("addSlot", AddSlot),
        DECLARE_NAPI_FUNCTION("addSlots", AddSlots),
        DECLARE_NAPI_FUNCTION("setSlotByBundle", SetSlotByBundle),
        DECLARE_NAPI_FUNCTION("getSlot", GetSlot),
        DECLARE_NAPI_FUNCTION("getSlotNumByBundle", GetSlotNumByBundle),
        DECLARE_NAPI_FUNCTION("getSlots", GetSlots),
        DECLARE_NAPI_FUNCTION("getSlotsByBundle", GetSlotsByBundle),
        DECLARE_NAPI_FUNCTION("removeSlot", RemoveSlot),
        DECLARE_NAPI_FUNCTION("removeAllSlots", RemoveAllSlots),
        DECLARE_NAPI_FUNCTION("enableNotification", EnableNotification),
        DECLARE_NAPI_FUNCTION("isNotificationEnabled", IsNotificationEnabled),
        DECLARE_NAPI_FUNCTION("displayBadge", DisplayBadge),
        DECLARE_NAPI_FUNCTION("isBadgeDisplayed", IsBadgeDisplayed),
        DECLARE_NAPI_FUNCTION("setDoNotDisturbMode", SetDoNotDisturbMode),
    };

    NAPI_CALL(env, napi_define_properties(env, exports, sizeof(desc) / sizeof(desc[0]), desc));

    return exports;
}

/*
 * Module export function
 */
static napi_value Init(napi_env env, napi_value exports)
{
    /*
     * Propertise define
     */
    NotificationInit(env, exports);
    ConstantInit(env, exports);
    return exports;
}

/*
 * Module register function
 */
__attribute__((constructor)) void RegisterModule(void)
{
    napi_module_register(&_module);
}
EXTERN_C_END

}  // namespace NotificationNapi
}  // namespace OHOS