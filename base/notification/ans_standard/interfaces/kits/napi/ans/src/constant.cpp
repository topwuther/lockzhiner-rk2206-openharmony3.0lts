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

#include "constant.h"
#include "common.h"

namespace OHOS {
namespace NotificationNapi {
using namespace OHOS::Notification;

void SetNamedPropertyByInteger(napi_env env, napi_value dstObj, int32_t objName, const char *propName)
{
    napi_value prop = nullptr;
    if (napi_create_int32(env, objName, &prop) == napi_ok) {
        napi_set_named_property(env, dstObj, propName, prop);
    }
}

napi_value NotificationReasonInit(napi_env env, napi_value exports)
{
    ANS_LOGI("%{public}s, called", __func__);

    napi_value obj = nullptr;
    napi_create_object(env, &obj);

    SetNamedPropertyByInteger(env, obj, NotificationReason::CLICK_REASON_DELETE, "CLICK_REASON_DELETE");
    SetNamedPropertyByInteger(env, obj, NotificationReason::CANCEL_REASON_DELETE, "CANCEL_REASON_DELETE");
    SetNamedPropertyByInteger(env, obj, NotificationReason::CANCEL_ALL_REASON_DELETE, "CANCEL_ALL_REASON_DELETE");
    SetNamedPropertyByInteger(env, obj, NotificationReason::ERROR_REASON_DELETE, "ERROR_REASON_DELETE");
    SetNamedPropertyByInteger(
        env, obj, NotificationReason::PACKAGE_CHANGED_REASON_DELETE, "PACKAGE_CHANGED_REASON_DELETE");
    SetNamedPropertyByInteger(env, obj, NotificationReason::USER_STOPPED_REASON_DELETE, "USER_STOPPED_REASON_DELETE");
    SetNamedPropertyByInteger(
        env, obj, NotificationReason::PACKAGE_BANNED_REASON_DELETE, "PACKAGE_BANNED_REASON_DELETE");
    SetNamedPropertyByInteger(env, obj, NotificationReason::APP_CANCEL_REASON_DELETE, "APP_CANCEL_REASON_DELETE");
    SetNamedPropertyByInteger(
        env, obj, NotificationReason::APP_CANCEL_ALL_REASON_DELETE, "APP_CANCEL_ALL_REASON_DELETE");
    SetNamedPropertyByInteger(env, obj, NotificationReason::APP_CANCEL_REASON_OTHER, "APP_CANCEL_REASON_OTHER");

    napi_property_descriptor exportFuncs[] = {
        DECLARE_NAPI_PROPERTY("NotificationReason", obj),
    };

    napi_define_properties(env, exports, sizeof(exportFuncs) / sizeof(*exportFuncs), exportFuncs);
    return exports;
}

napi_value SlotTypeInit(napi_env env, napi_value exports)
{
    ANS_LOGI("%{public}s, called", __func__);

    napi_value obj = nullptr;
    napi_create_object(env, &obj);

    SetNamedPropertyByInteger(env, obj, SlotType::UNKNOWN_TYPE, "UNKNOWN_TYPE");
    SetNamedPropertyByInteger(env, obj, SlotType::SOCIAL_COMMUNICATION, "SOCIAL_COMMUNICATION");
    SetNamedPropertyByInteger(env, obj, SlotType::SERVICE_INFORMATION, "SERVICE_INFORMATION");
    SetNamedPropertyByInteger(env, obj, SlotType::CONTENT_INFORMATION, "CONTENT_INFORMATION");
    SetNamedPropertyByInteger(env, obj, SlotType::OTHER_TYPES, "OTHER_TYPES");

    napi_property_descriptor exportFuncs[] = {
        DECLARE_NAPI_PROPERTY("SlotType", obj),
    };

    napi_define_properties(env, exports, sizeof(exportFuncs) / sizeof(*exportFuncs), exportFuncs);
    return exports;
}

napi_value SlotLevelInit(napi_env env, napi_value exports)
{
    ANS_LOGI("%{public}s, called", __func__);

    napi_value obj = nullptr;
    napi_create_object(env, &obj);

    SetNamedPropertyByInteger(env, obj, SlotLevel::LEVEL_NONE, "LEVEL_NONE");
    SetNamedPropertyByInteger(env, obj, SlotLevel::LEVEL_MIN, "LEVEL_MIN");
    SetNamedPropertyByInteger(env, obj, SlotLevel::LEVEL_LOW, "LEVEL_LOW");
    SetNamedPropertyByInteger(env, obj, SlotLevel::LEVEL_DEFAULT, "LEVEL_DEFAULT");
    SetNamedPropertyByInteger(env, obj, SlotLevel::LEVEL_HIGH, "LEVEL_HIGH");

    napi_property_descriptor exportFuncs[] = {
        DECLARE_NAPI_PROPERTY("SlotLevel", obj),
    };

    napi_define_properties(env, exports, sizeof(exportFuncs) / sizeof(*exportFuncs), exportFuncs);
    return exports;
}

napi_value SemanticActionButtonInit(napi_env env, napi_value exports)
{
    ANS_LOGI("%{public}s, called", __func__);

    napi_value obj = nullptr;
    napi_create_object(env, &obj);

    SetNamedPropertyByInteger(env, obj, SemanticActionButton::NONE_ACTION_BUTTON, "NONE_ACTION_BUTTON");
    SetNamedPropertyByInteger(env, obj, SemanticActionButton::REPLY_ACTION_BUTTON, "REPLY_ACTION_BUTTON");
    SetNamedPropertyByInteger(env, obj, SemanticActionButton::READ_ACTION_BUTTON, "READ_ACTION_BUTTON");
    SetNamedPropertyByInteger(env, obj, SemanticActionButton::UNREAD_ACTION_BUTTON, "UNREAD_ACTION_BUTTON");
    SetNamedPropertyByInteger(env, obj, SemanticActionButton::DELETE_ACTION_BUTTON, "DELETE_ACTION_BUTTON");
    SetNamedPropertyByInteger(env, obj, SemanticActionButton::ARCHIVE_ACTION_BUTTON, "ARCHIVE_ACTION_BUTTON");
    SetNamedPropertyByInteger(env, obj, SemanticActionButton::MUTE_ACTION_BUTTON, "MUTE_ACTION_BUTTON");
    SetNamedPropertyByInteger(env, obj, SemanticActionButton::UNMUTE_ACTION_BUTTON, "UNMUTE_ACTION_BUTTON");
    SetNamedPropertyByInteger(env, obj, SemanticActionButton::THUMBS_UP_ACTION_BUTTON, "THUMBS_UP_ACTION_BUTTON");
    SetNamedPropertyByInteger(env, obj, SemanticActionButton::THUMBS_DOWN_ACTION_BUTTON, "THUMBS_DOWN_ACTION_BUTTON");
    SetNamedPropertyByInteger(env, obj, SemanticActionButton::CALL_ACTION_BUTTON, "CALL_ACTION_BUTTON");

    napi_property_descriptor exportFuncs[] = {
        DECLARE_NAPI_PROPERTY("SemanticActionButton", obj),
    };

    napi_define_properties(env, exports, sizeof(exportFuncs) / sizeof(*exportFuncs), exportFuncs);
    return exports;
}

napi_value InputsSourceInit(napi_env env, napi_value exports)
{
    ANS_LOGI("%{public}s, called", __func__);

    napi_value obj = nullptr;
    napi_create_object(env, &obj);

    SetNamedPropertyByInteger(env, obj, InputsSource::FREE_FORM_INPUT, "FREE_FORM_INPUT");
    SetNamedPropertyByInteger(env, obj, InputsSource::OPTION, "OPTION");

    napi_property_descriptor exportFuncs[] = {
        DECLARE_NAPI_PROPERTY("InputsSource", obj),
    };

    napi_define_properties(env, exports, sizeof(exportFuncs) / sizeof(*exportFuncs), exportFuncs);
    return exports;
}

napi_value DoNotDisturbMode(napi_env env, napi_value exports)
{
    ANS_LOGI("%{public}s, called", __func__);

    napi_value obj = nullptr;
    napi_create_object(env, &obj);

    SetNamedPropertyByInteger(env, obj, DisturbMode::ALLOW_UNKNOWN, "ALLOW_UNKNOWN");
    SetNamedPropertyByInteger(env, obj, DisturbMode::ALLOW_ALL, "ALLOW_ALL");
    SetNamedPropertyByInteger(env, obj, DisturbMode::ALLOW_PRIORITY, "ALLOW_PRIORITY");
    SetNamedPropertyByInteger(env, obj, DisturbMode::ALLOW_NONE, "ALLOW_NONE");
    SetNamedPropertyByInteger(env, obj, DisturbMode::ALLOW_ALARMS, "ALLOW_ALARMS");

    napi_property_descriptor exportFuncs[] = {
        DECLARE_NAPI_PROPERTY("DoNotDisturbMode", obj),
    };

    napi_define_properties(env, exports, sizeof(exportFuncs) / sizeof(*exportFuncs), exportFuncs);
    return exports;
}

napi_value InputEditTypeInit(napi_env env, napi_value exports)
{
    ANS_LOGI("%{public}s, called", __func__);

    napi_value obj = nullptr;
    napi_create_object(env, &obj);

    SetNamedPropertyByInteger(env, obj, InputEditType::EDIT_AUTO, "EDIT_AUTO");
    SetNamedPropertyByInteger(env, obj, InputEditType::EDIT_DISABLED, "EDIT_DISABLED");
    SetNamedPropertyByInteger(env, obj, InputEditType::EDIT_ENABLED, "EDIT_ENABLED");

    napi_property_descriptor exportFuncs[] = {
        DECLARE_NAPI_PROPERTY("InputEditType", obj),
    };

    napi_define_properties(env, exports, sizeof(exportFuncs) / sizeof(*exportFuncs), exportFuncs);
    return exports;
}

napi_value ContentTypeInit(napi_env env, napi_value exports)
{
    ANS_LOGI("%{public}s, called", __func__);

    napi_value obj = nullptr;
    napi_create_object(env, &obj);

    SetNamedPropertyByInteger(
        env, obj, ContentType::NOTIFICATION_CONTENT_BASIC_TEXT, "NOTIFICATION_CONTENT_BASIC_TEXT");
    SetNamedPropertyByInteger(env, obj, ContentType::NOTIFICATION_CONTENT_LONG_TEXT, "NOTIFICATION_CONTENT_LONG_TEXT");
    SetNamedPropertyByInteger(env, obj, ContentType::NOTIFICATION_CONTENT_PICTURE, "NOTIFICATION_CONTENT_PICTURE");
    SetNamedPropertyByInteger(
        env, obj, ContentType::NOTIFICATION_CONTENT_CONVERSATION, "NOTIFICATION_CONTENT_CONVERSATION");
    SetNamedPropertyByInteger(env, obj, ContentType::NOTIFICATION_CONTENT_MULTILINE, "NOTIFICATION_CONTENT_MULTILINE");
    SetNamedPropertyByInteger(env, obj, ContentType::NOTIFICATION_CONTENT_MEDIA, "NOTIFICATION_CONTENT_MEDIA");

    napi_property_descriptor exportFuncs[] = {
        DECLARE_NAPI_PROPERTY("ContentType", obj),
    };

    napi_define_properties(env, exports, sizeof(exportFuncs) / sizeof(*exportFuncs), exportFuncs);
    return exports;
}

napi_value ConstantInit(napi_env env, napi_value exports)
{
    NotificationReasonInit(env, exports);
    SlotTypeInit(env, exports);
    SlotLevelInit(env, exports);
    SemanticActionButtonInit(env, exports);
    InputsSourceInit(env, exports);
    DoNotDisturbMode(env, exports);
    InputEditTypeInit(env, exports);
    ContentTypeInit(env, exports);
    return exports;
}

}  // namespace NotificationNapi
}  // namespace OHOS
