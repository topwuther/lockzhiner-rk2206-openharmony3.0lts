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

#ifndef BASE_NOTIFICATION_ANS_STANDARD_SERVICES_PREFERENCES_CONSTANT_H
#define BASE_NOTIFICATION_ANS_STANDARD_SERVICES_PREFERENCES_CONSTANT_H

namespace OHOS {
namespace Notification {

const static std::string KEY_DISTURB_MODE = "ans_disturbMode";
const static std::string KEY_ENABLE_ALL_NOTIFICATION = "ans_notificationAll";
const static std::string KEY_BUNDLE_LABEL = "label_ans_bundle_";
const static std::string KEY_UNDER_LINE = "_";
const static std::string KEY_ANS_BUNDLE = "ans_bundle";

const static std::string KEY_BUNDLE_NAME = "name";
const static std::string KEY_BUNDLE_IMPORTANCE = "importance";
const static std::string KEY_BUNDLE_SHOW_BADGE = "showBadge";
const static std::string KEY_BUNDLE_BADGE_TOTAL_NUM = "badgeTotalNum";
const static std::string KEY_BUNDLE_PRIVATE_ALLOWED = "privateAllowed";
const static std::string KEY_BUNDLE_ENABLE_NOTIFICATION = "enabledNotification";

const static std::string KEY_GROUP = "group";
const static std::string KEY_GROUP_ID = "id";
const static std::string KEY_GROUP_NAME = "name";
const static std::string KEY_GROUP_DESCRIPTION = "groupDescription";
const static std::string KEY_GROUP_DISABLE = "isDisable";

const static std::string KEY_SLOT = "slot";
const static std::string KEY_SLOT_TYPE = "type";
const static std::string KEY_SLOT_ID = "id";
const static std::string KEY_SLOT_GROUPID = "groupId";
const static std::string KEY_SLOT_NAME = "name";
const static std::string KEY_SLOT_DESCRIPTION = "description";
const static std::string KEY_SLOT_LEVEL = "level";
const static std::string KEY_SLOT_SHOW_BADGE = "showBadge";
const static std::string KEY_SLOT_ENABLE_LIGHT = "enableLight";
const static std::string KEY_SLOT_ENABLE_VRBRATION = "enableVibration";
const static std::string KEY_SLOT_LED_LIGHT_COLOR = "ledLightColor";
const static std::string KEY_SLOT_LOCKSCREEN_VISIBLENESS = "lockscreenVisibleness";
const static std::string KEY_SLOT_SOUND = "sound";
const static std::string KEY_SLOT_VIBRATION_STYLE = "vibrationSytle";
const static std::string KEY_SLOT_ENABLE_BYPASS_DND = "enableBypassDnd";

// distributed database
const std::string APP_ID = "advanced_notification_service";
const std::string STORE_ID = "local_db";

const static int BUNDLE_IMPORTANCE = 3;
const static int BUNDLE_BADGE_TOTAL_NUM = 0;
const static int BUNDLE_PRIVATE_ALLOWED = false;
const static int BUNDLE_ENABLE_NOTIFICATION = true;
const static bool BUNDLE_SHOW_BADGE = false;

enum class BundleType {
    BUNDLE_NAME_TYPE = 1,
    BUNDLE_IMPORTANCE_TYPE,
    BUNDLE_SHOW_BADGE_TYPE,
    BUNDLE_BADGE_TOTAL_NUM_TYPE,
    BUNDLE_PRIVATE_ALLOWED_TYPE,
    BUNDLE_ENABLE_NOTIFICATION_TYPE,
};

}  // namespace Notification
}  // namespace OHOS

#endif  // BASE_NOTIFICATION_ANS_STANDARD_SERVICES_PREFERENCES_CONSTANT_H