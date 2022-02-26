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

#ifndef BASE_NOTIFICATION_ANS_STANDARD_INNERKITS_BASE_INCLUDE_ANS_INNER_ERRORS_H
#define BASE_NOTIFICATION_ANS_STANDARD_INNERKITS_BASE_INCLUDE_ANS_INNER_ERRORS_H

#include "errors.h"

namespace OHOS {
namespace Notification {
/**
 * ErrCode layout
 *
 * +-----+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
 * | Bit |31|30|29|28|27|26|25|24|23|22|21|20|19|18|17|16|15|14|13|12|11|10|09|08|07|06|05|04|03|02|01|00|
 * +-----+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
 * |Field|Reserved|        Subsystem      |  Module      |                              Code             |
 * +-----+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
 */

constexpr uint32_t EVENT_SUB_MODULE_OFFSET = 8;  // sub-module offset value

// ANS's module const defined.
enum AnsModule : uint32_t {
    ANS_MODULE_COMMON = 0x00,
};

// Offset of common event sub-system's errcode base.
constexpr ErrCode ANS_COMMON_ERR_OFFSET = ErrCodeOffset(SUBSYS_NOTIFICATION, ANS_MODULE_COMMON);

// Error code defined.
enum ErrorCode : uint32_t {
    ERR_ANS_SERVICE_NOT_READY = ANS_COMMON_ERR_OFFSET + 1,
    ERR_ANS_SERVICE_NOT_CONNECTED,
    ERR_ANS_INVALID_PARAM,
    ERR_ANS_INVALID_UID,
    ERR_ANS_INVALID_PID,
    ERR_ANS_INVALID_BUNDLE,
    ERR_ANS_NOT_ALLOWED,
    ERR_ANS_PARCELABLE_FAILED,
    ERR_ANS_TRANSACT_FAILED,
    ERR_ANS_REMOTE_DEAD,
    ERR_ANS_NO_MEMORY,
    ERR_ANS_TASK_ERR,
    ERR_ANS_NON_SYSTEM_APP,
    ERR_ANS_NOTIFICATION_NOT_EXISTS,
    ERR_ANS_NOTIFICATION_IS_UNREMOVABLE,
    ERR_ANS_OVER_MAX_ACITVE_PERSECOND,
    ERR_ANS_ICON_OVER_SIZE,
    ERR_ANS_PICTURE_OVER_SIZE,
    ERR_ANS_PREFERENCES_NOTIFICATION_DB_OPERATION_FAILED,
    ERR_ANS_PREFERENCES_NOTIFICATION_SLOT_NOT_EXIST,
    ERR_ANS_PREFERENCES_NOTIFICATION_BUNDLE_NOT_EXIST,
    ERR_ANS_PREFERENCES_NOTIFICATION_SLOT_TYPE_NOT_EXIST,
    ERR_ANS_PREFERENCES_NOTIFICATION_SLOTGROUP_NOT_EXIST,
    ERR_ANS_PREFERENCES_NOTIFICATION_SLOTGROUP_ID_INVALID,
    ERR_ANS_PREFERENCES_NOTIFICATION_SLOTGROUP_EXCEED_MAX_NUM,
};
}  // namespace Notification
}  // namespace OHOS

#endif  // BASE_NOTIFICATION_ANS_STANDARD_INNERKITS_BASE_INCLUDE_ANS_INNER_ERRORS_H
