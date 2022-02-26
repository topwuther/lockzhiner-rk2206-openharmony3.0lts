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

#ifndef BASE_NOTIFICATION_ANS_STANDARD_INNERKITS_BASE_INCLUDE_ANS_CONST_DEFINE_H
#define BASE_NOTIFICATION_ANS_STANDARD_INNERKITS_BASE_INCLUDE_ANS_CONST_DEFINE_H

namespace OHOS {
namespace Notification {

// Max active notification number
constexpr uint32_t MAX_ACTIVE_NUM = 1000;
constexpr uint32_t MAX_ACTIVE_NUM_PERAPP = 100;
constexpr uint32_t MAX_ACTIVE_NUM_PERSECOND = 10;
constexpr uint32_t MAX_SLOT_NUM = 5;
constexpr uint32_t MAX_SLOT_GROUP_NUM = 4;
constexpr uint32_t MAX_ICON_SIZE = 50 * 1024;
constexpr uint32_t MAX_PICTURE_SIZE = 2 * 1024 * 1024;

}  // namespace Notification
}  // namespace OHOS

#endif  // BASE_NOTIFICATION_ANS_STANDARD_INNERKITS_BASE_INCLUDE_ANS_CONST_DEFINE_H
