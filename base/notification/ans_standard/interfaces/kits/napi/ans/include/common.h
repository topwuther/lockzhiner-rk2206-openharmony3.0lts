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
#ifndef BASE_NOTIFICATION_ANS_STANDARD_KITS_NAPI_INCLUDE_COMMON_H
#define BASE_NOTIFICATION_ANS_STANDARD_KITS_NAPI_INCLUDE_COMMON_H

#include "ans_log_wrapper.h"
#include "napi/native_api.h"
#include "napi/native_node_api.h"
#include "notification_helper.h"

namespace OHOS {
namespace NotificationNapi {
using namespace OHOS::Notification;

const std::int32_t STR_MAX_SIZE = 64;
const std::int32_t LONG_STR_MAX_SIZE = 1024;
const int NO_ERROR = 0;
const int ERROR = -1;

enum ContentType {
    NOTIFICATION_CONTENT_BASIC_TEXT,
    NOTIFICATION_CONTENT_LONG_TEXT,
    NOTIFICATION_CONTENT_PICTURE,
    NOTIFICATION_CONTENT_CONVERSATION,
    NOTIFICATION_CONTENT_MULTILINE,
    NOTIFICATION_CONTENT_MEDIA
};

enum SlotType {
    UNKNOWN_TYPE = 0,
    SOCIAL_COMMUNICATION = 1,
    SERVICE_INFORMATION = 2,
    CONTENT_INFORMATION = 3,
    OTHER_TYPES = 0xFFFF,
};

enum SlotLevel {
    LEVEL_NONE = 0,
    LEVEL_MIN = 1,
    LEVEL_LOW = 2,
    LEVEL_DEFAULT = 3,
    LEVEL_HIGH = 4,
};

enum NotificationReason {
    CLICK_REASON_DELETE,
    CANCEL_REASON_DELETE,
    CANCEL_ALL_REASON_DELETE,
    ERROR_REASON_DELETE,
    PACKAGE_CHANGED_REASON_DELETE,
    USER_STOPPED_REASON_DELETE,
    PACKAGE_BANNED_REASON_DELETE,
    APP_CANCEL_REASON_DELETE,
    APP_CANCEL_ALL_REASON_DELETE,
    APP_CANCEL_REASON_OTHER
};

enum SemanticActionButton {
    NONE_ACTION_BUTTON,
    REPLY_ACTION_BUTTON,
    READ_ACTION_BUTTON,
    UNREAD_ACTION_BUTTON,
    DELETE_ACTION_BUTTON,
    ARCHIVE_ACTION_BUTTON,
    MUTE_ACTION_BUTTON,
    UNMUTE_ACTION_BUTTON,
    THUMBS_UP_ACTION_BUTTON,
    THUMBS_DOWN_ACTION_BUTTON,
    CALL_ACTION_BUTTON
};

enum InputsSource { FREE_FORM_INPUT, OPTION };

enum DisturbMode { ALLOW_UNKNOWN, ALLOW_ALL, ALLOW_PRIORITY, ALLOW_NONE, ALLOW_ALARMS };

enum InputEditType { EDIT_AUTO, EDIT_DISABLED, EDIT_ENABLED };

struct NotificationSubscribeInfo {
    std::vector<std::string> bundleNames;
    int userId = 0;
    bool hasSubscribeInfo = false;
};

struct BundleOption {
    std::string bundle{};
    int uid{};
};

struct NotificationKey {
    int id{};
    std::string label{};
};

struct CallbackPromiseInfo {
    napi_ref callback = nullptr;
    napi_deferred deferred = nullptr;
    bool isCallback = false;
    int errorCode = 0;
};

class Common {
    Common();

    ~Common();

public:
    static napi_value NapiGetboolean(napi_env env, const bool &isValue);

    static napi_value NapiGetNull(napi_env env);

    static napi_value NapiGetUndefined(napi_env env);

    static napi_value GetCallbackErrorValue(napi_env env, int errCode);

    static void PaddingCallbackPromiseInfo(
        const napi_env &env, const napi_ref &callback, CallbackPromiseInfo &info, napi_value &promise);

    static void ReturnCallbackPromise(const napi_env &env, const CallbackPromiseInfo &info, const napi_value &result);

    static void SetCallback(
        const napi_env &env, const napi_ref &callbackIn, const int &errorCode, const napi_value &result);

    static void SetPromise(const napi_env &env, const napi_deferred &deferred, const napi_value &result);

    static napi_value JSParaError(const napi_env &env, const napi_ref &callback);

    static napi_value SetNotification(
        const napi_env &env, OHOS::Notification::Notification *notification, napi_value &result);

    static napi_value SetNotificationRequest(
        const napi_env &env, OHOS::Notification::NotificationRequest *request, napi_value &result);

    static napi_value SetNotificationSortingMap(
        const napi_env &env, const std::shared_ptr<NotificationSortingMap> &sortingMap, napi_value &result);

    static napi_value SetNotificationSorting(
        const napi_env &env, const NotificationSorting &sorting, napi_value &result);

    static napi_value SetNotificationSlot(const napi_env &env, const NotificationSlot &slot, napi_value &result);

    static napi_value SetNotificationContent(
        const napi_env &env, const std::shared_ptr<NotificationContent> &content, napi_value &result);

    static napi_value SetNotificationBasicContent(
        const napi_env &env, NotificationBasicContent *basicContent, napi_value &result);

    static napi_value SetNotificationLongTextContent(
        const napi_env &env, OHOS::Notification::NotificationLongTextContent *longTextContent, napi_value &result);

    static napi_value SetNotificationPictureContent(
        const napi_env &env, OHOS::Notification::NotificationPictureContent *pictureContent, napi_value &result);

    static napi_value SetNotificationConversationalContent(const napi_env &env,
        OHOS::Notification::NotificationConversationalContent *conversationalContent, napi_value &result);

    static napi_value SetNotificationMultiLineContent(
        const napi_env &env, OHOS::Notification::NotificationMultiLineContent *multiLineContent, napi_value &result);

    static napi_value SetMessageUser(const napi_env &env, const MessageUser &messageUser, napi_value &result);

    static napi_value SetConversationalMessage(const napi_env &env,
        const std::shared_ptr<NotificationConversationalMessage> &conversationalMessage, napi_value &result);

    static napi_value SetNotificationActionButton(
        const napi_env &env, const std::shared_ptr<NotificationActionButton> &actionButton, napi_value &result);

    static napi_value GetNotificationSubscriberInfo(
        const napi_env &env, const napi_value &value, NotificationSubscribeInfo &result);

    static napi_value GetNotificationRequest(
        const napi_env &env, const napi_value &value, NotificationRequest &request);

    static napi_value GetNotificationContent(
        const napi_env &env, const napi_value &value, NotificationRequest &request);

    static napi_value GetNotificationWantAgent(
        const napi_env &env, const napi_value &value, NotificationRequest &request);

    static napi_value GetNotificationSlot(const napi_env &env, const napi_value &value, NotificationSlot &slot);

    static napi_value GetBundleOption(const napi_env &env, const napi_value &value, BundleOption &option);

    static napi_value GetNotificationKey(const napi_env &env, const napi_value &value, NotificationKey &key);

    static bool ContentTypeJSToC(const enum ContentType &inType, enum NotificationContent::Type &outType);

    static bool ContentTypeCToJS(const enum NotificationContent::Type &inType, enum ContentType &outType);

    static bool SlotTypeJSToC(const enum SlotType &inType, enum NotificationConstant::SlotType &outType);

    static bool SlotTypeCToJS(const enum NotificationConstant::SlotType &inType, enum SlotType &outType);

    static bool SlotLevelJSToC(const SlotLevel &inLevel, NotificationSlot::NotificationLevel &outLevel);

    static bool SlotLevelCToJS(const NotificationSlot::NotificationLevel &inLevel, SlotLevel &outLevel);

    static bool ReasonCToJS(const int &inType, int &outType);

    static bool DisturbModeJSToC(const enum DisturbMode &inType, enum NotificationConstant::DisturbMode &outType);

    static napi_value CreateWantAgentByJS(const napi_env &env, const std::shared_ptr<WantAgent::WantAgent> &agent);

private:
    static const int ARGS_TWO = 2;
    static const int PARAM0 = 0;
    static const int PARAM1 = 1;
    static std::set<std::shared_ptr<WantAgent::WantAgent>> wantAgent_;
};

}  // namespace NotificationNapi
}  // namespace OHOS

#endif  // BASE_NOTIFICATION_ANS_STANDARD_KITS_NAPI_INCLUDE_COMMON_H