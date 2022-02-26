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

#ifndef ON_USING_PERMISSION_REMINDER_H
#define ON_USING_PERMISSION_REMINDER_H

#include <iremote_broker.h>

#include "permission_reminder_info.h"

namespace OHOS {
namespace Security {
namespace Permission {
class OnUsingPermissionReminder : public IRemoteBroker {
public:
    virtual void StartUsingPermission(const PermissionReminderInfo& permReminderInfo) = 0;

    virtual void StopUsingPermission(const PermissionReminderInfo& permReminderInfo) = 0;

    enum class Message {
        START_USING_PERMISSION,
        STOP_USING_PERMISSION,
    };

public:
    DECLARE_INTERFACE_DESCRIPTOR(u"ohos.security.permission.OnUsingPermissionReminder");
};
} // namespace Permission
} // namespace Security
} // namespace OHOS

#endif // ON_USING_PERMISSION_REMINDER_H
