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

#include "common.h"
#include <cinttypes>
#include "napi_common.h"
#include "notification_long_text_content.h"
#include "notification_multiline_content.h"
#include "notification_slot.h"
#include "pixel_map_napi.h"
#include "publish.h"
#include "want_agent.h"

namespace OHOS {
namespace NotificationNapi {
std::set<std::shared_ptr<WantAgent::WantAgent>> Common::wantAgent_;
static napi_value GetIdByPublish(const napi_env &env, const napi_value &value, NotificationRequest &request);
static napi_value GetSlotType(const napi_env &env, const napi_value &value, NotificationRequest &request);
static napi_value GetNotificationIsOngoing(const napi_env &env, const napi_value &value, NotificationRequest &request);
static napi_value GetNotificationIsUnremovable(
    const napi_env &env, const napi_value &value, NotificationRequest &request);
static napi_value GetNotificationDeliveryTime(
    const napi_env &env, const napi_value &value, NotificationRequest &request);
static napi_value GetNotificationtapDismissed(
    const napi_env &env, const napi_value &value, NotificationRequest &request);
static napi_value GetNotificationRemovalWantAgent(
    const napi_env &env, const napi_value &value, NotificationRequest &request);
static napi_value GetNotificationMaxScreenWantAgent(
    const napi_env &env, const napi_value &value, NotificationRequest &request);
static napi_value GetNotificationAutoDeletedTime(
    const napi_env &env, const napi_value &value, NotificationRequest &request);
static napi_value GetNotificationClassification(
    const napi_env &env, const napi_value &value, NotificationRequest &request);
static napi_value GetNotificationColor(const napi_env &env, const napi_value &value, NotificationRequest &request);
static napi_value GetNotificationColorEnabled(
    const napi_env &env, const napi_value &value, NotificationRequest &request);
static napi_value GetNotificationIsAlertOnce(
    const napi_env &env, const napi_value &value, NotificationRequest &request);
static napi_value GetNotificationIsStopwatch(
    const napi_env &env, const napi_value &value, NotificationRequest &request);
static napi_value GetNotificationIsCountDown(
    const napi_env &env, const napi_value &value, NotificationRequest &request);
static napi_value GetNotificationStatusBarText(
    const napi_env &env, const napi_value &value, NotificationRequest &request);
static napi_value GetNotificationLabel(const napi_env &env, const napi_value &value, NotificationRequest &request);
static napi_value GetNotificationBadgeIconStyle(
    const napi_env &env, const napi_value &value, NotificationRequest &request);
static napi_value GetNotificationShowDeliveryTime(
    const napi_env &env, const napi_value &value, NotificationRequest &request);
static napi_value GetNotificationNotificationActionButtons(
    const napi_env &env, const napi_value &value, NotificationRequest &request);
static napi_value GetNotificationSmallIcon(const napi_env &env, const napi_value &value, NotificationRequest &request);
static napi_value GetNotificationLargeIcon(const napi_env &env, const napi_value &value, NotificationRequest &request);
static napi_value GetNotificationContentType(const napi_env &env, const napi_value &result, int32_t &type);
static napi_value GetNotificationBasicContent(
    const napi_env &env, const napi_value &result, NotificationRequest &request);
static napi_value GetNotificationLongTextContent(
    const napi_env &env, const napi_value &result, NotificationRequest &request);
static napi_value GetNotificationPictureContent(
    const napi_env &env, const napi_value &result, NotificationRequest &request);
static napi_value GetNotificationConversationalContent(
    const napi_env &env, const napi_value &result, NotificationRequest &request);
static napi_value GetNotificationMultiLineContent(
    const napi_env &env, const napi_value &result, NotificationRequest &request);
static napi_value GetNotificationMultiLineContentLines(const napi_env &env, const napi_value &result,
    std::shared_ptr<OHOS::Notification::NotificationMultiLineContent> &multiLineContent);
static napi_value GetNotificationExtraInfo(const napi_env &env, const napi_value &value, NotificationRequest &request);

Common::Common()
{}

Common::~Common()
{}

napi_value Common::NapiGetboolean(napi_env env, const bool &isValue)
{
    napi_value result = nullptr;
    napi_get_boolean(env, isValue, &result);
    return result;
}

napi_value Common::NapiGetNull(napi_env env)
{
    napi_value result = nullptr;
    napi_get_null(env, &result);
    return result;
}

napi_value Common::NapiGetUndefined(napi_env env)
{
    napi_value result = nullptr;
    napi_get_undefined(env, &result);
    return result;
}

napi_value Common::GetCallbackErrorValue(napi_env env, int errCode)
{
    napi_value result = nullptr;
    napi_value eCode = nullptr;
    NAPI_CALL(env, napi_create_int32(env, errCode, &eCode));
    NAPI_CALL(env, napi_create_object(env, &result));
    NAPI_CALL(env, napi_set_named_property(env, result, "code", eCode));
    return result;
}

void Common::PaddingCallbackPromiseInfo(
    const napi_env &env, const napi_ref &callback, CallbackPromiseInfo &info, napi_value &promise)
{
    ANS_LOGI("enter");

    if (callback) {
        info.callback = callback;
        info.isCallback = true;
    } else {
        napi_deferred deferred = nullptr;
        NAPI_CALL_RETURN_VOID(env, napi_create_promise(env, &deferred, &promise));
        info.deferred = deferred;
        info.isCallback = false;
    }
}

void Common::ReturnCallbackPromise(const napi_env &env, const CallbackPromiseInfo &info, const napi_value &result)
{
    ANS_LOGI("enter");

    if (info.isCallback) {
        SetCallback(env, info.callback, info.errorCode, result);
    } else {
        SetPromise(env, info.deferred, result);
    }
    ANS_LOGI("ReturnCallbackPromise end");
}

void Common::SetCallback(
    const napi_env &env, const napi_ref &callbackIn, const int &errorCode, const napi_value &result)
{
    ANS_LOGI("enter");
    napi_value undefined = nullptr;
    napi_get_undefined(env, &undefined);

    napi_value callback = nullptr;
    napi_value resultout = nullptr;
    napi_get_reference_value(env, callbackIn, &callback);
    napi_value results[ARGS_TWO] = {nullptr};
    results[PARAM0] = GetCallbackErrorValue(env, errorCode);
    results[PARAM1] = result;
    NAPI_CALL_RETURN_VOID(env, napi_call_function(env, undefined, callback, ARGS_TWO, &results[PARAM0], &resultout));
    ANS_LOGI("end");
}

void Common::SetPromise(const napi_env &env, const napi_deferred &deferred, const napi_value &result)
{
    ANS_LOGI("enter");
    napi_resolve_deferred(env, deferred, result);
    ANS_LOGI("end");
}

napi_value Common::JSParaError(const napi_env &env, const napi_ref &callback)
{
    if (callback) {
        return Common::NapiGetNull(env);
    } else {
        napi_value promise = nullptr;
        napi_deferred deferred = nullptr;
        napi_create_promise(env, &deferred, &promise);
        SetPromise(env, deferred, Common::NapiGetNull(env));
        return promise;
    }
}

napi_value Common::SetNotification(
    const napi_env &env, OHOS::Notification::Notification *notification, napi_value &result)
{
    ANS_LOGI("enter");

    if (notification == nullptr) {
        ANS_LOGE("notification is nullptr");
        return NapiGetboolean(env, false);
    }
    napi_value value = nullptr;
    NotificationRequest request = notification->GetNotificationRequest();
    if (!SetNotificationRequest(env, &request, result)) {
        return NapiGetboolean(env, false);
    }

    // hashCode?: string
    napi_create_string_utf8(env, notification->GetKey().c_str(), NAPI_AUTO_LENGTH, &value);
    napi_set_named_property(env, result, "hashCode", value);

    // isFloatingIcon ?: boolean
    napi_get_boolean(env, notification->IsFloatingIcon(), &value);
    napi_set_named_property(env, result, "isFloatingIcon", value);

    // readonly creatorBundleName?: string
    napi_create_string_utf8(env, notification->GetCreateBundle().c_str(), NAPI_AUTO_LENGTH, &value);
    napi_set_named_property(env, result, "creatorBundleName", value);

    // readonly creatorUid?: number
    napi_create_int32(env, notification->GetUid(), &value);
    napi_set_named_property(env, result, "creatorUid", value);

    // readonly creatorPid?: number
    napi_create_int32(env, notification->GetPid(), &value);
    napi_set_named_property(env, result, "creatorPid", value);

    return NapiGetboolean(env, true);
}

napi_value Common::SetNotificationRequest(
    const napi_env &env, OHOS::Notification::NotificationRequest *request, napi_value &result)
{
    ANS_LOGI("SetNotificationRequest start");

    napi_value value = nullptr;

    if (request == nullptr) {
        ANS_LOGE("request is nullptr");
        return NapiGetboolean(env, false);
    }

    // content: NotificationContent
    std::shared_ptr<NotificationContent> content = request->GetContent();
    if (content) {
        napi_value contentResult = nullptr;
        napi_create_object(env, &contentResult);
        if (!SetNotificationContent(env, content, contentResult)) {
            ANS_LOGE("SetNotificationContent call failed");
            return NapiGetboolean(env, false);
        }
        napi_set_named_property(env, result, "content", contentResult);
    } else {
        ANS_LOGE("content is nullptr");
        return NapiGetboolean(env, false);
    }

    // id?: number
    napi_create_int32(env, request->GetNotificationId(), &value);
    napi_set_named_property(env, result, "id", value);

    // slotType?: SlotType
    enum SlotType outType = SlotType::UNKNOWN_TYPE;
    if (!SlotTypeCToJS(request->GetSlotType(), outType)) {
        return NapiGetboolean(env, false);
    }
    napi_create_int32(env, outType, &value);
    napi_set_named_property(env, result, "slotType", value);

    // isOngoing?: boolean
    napi_get_boolean(env, request->IsInProgress(), &value);
    napi_set_named_property(env, result, "isOngoing", value);

    // isUnremovable?: boolean
    napi_get_boolean(env, request->IsUnremovable(), &value);
    napi_set_named_property(env, result, "isUnremovable", value);

    // deliveryTime?: number
    napi_create_int64(env, request->GetDeliveryTime(), &value);
    napi_set_named_property(env, result, "deliveryTime", value);

    // tapDismissed?: boolean
    napi_get_boolean(env, request->IsTapDismissed(), &value);
    napi_set_named_property(env, result, "tapDismissed", value);

    // autoDeletedTime?: number
    napi_create_int64(env, request->GetAutoDeletedTime(), &value);
    napi_set_named_property(env, result, "autoDeletedTime", value);

    // wantAgent?: WantAgent
    std::shared_ptr<WantAgent::WantAgent> agent = request->GetWantAgent();
    if (agent) {
        napi_value wantAgent = nullptr;
        wantAgent = CreateWantAgentByJS(env, agent);
        napi_set_named_property(env, result, "wantAgent", wantAgent);
    } else {
        napi_set_named_property(env, result, "wantAgent", NapiGetNull(env));
    }

    // extraInfo ?: {[key:string] : any}
    std::shared_ptr<AAFwk::WantParams> additionalData = request->GetAdditionalData();
    if (additionalData) {
        napi_value extraInfo = nullptr;
        extraInfo = OHOS::AppExecFwk::WrapWantParams(env, *additionalData);
        napi_set_named_property(env, result, "extraInfo", extraInfo);
    }

    // removalWantAgent ?: WantAgent
    std::shared_ptr<WantAgent::WantAgent> removalAgent = request->GetRemovalWantAgent();
    if (removalAgent) {
        napi_value wantAgent = nullptr;
        wantAgent = CreateWantAgentByJS(env, removalAgent);
        napi_set_named_property(env, result, "removalWantAgent", wantAgent);
    } else {
        napi_set_named_property(env, result, "removalWantAgent", NapiGetNull(env));
    }

    // maxScreenWantAgent ? WantAgent
    std::shared_ptr<WantAgent::WantAgent> maxScreenAgent = request->GetMaxScreenWantAgent();
    if (maxScreenAgent) {
        napi_value wantAgent = nullptr;
        wantAgent = CreateWantAgentByJS(env, maxScreenAgent);
        napi_set_named_property(env, result, "maxScreenWantAgent", wantAgent);
    } else {
        napi_set_named_property(env, result, "maxScreenWantAgent", NapiGetNull(env));
    }

    // classification ?: string
    napi_create_string_utf8(env, request->GetClassification().c_str(), NAPI_AUTO_LENGTH, &value);
    napi_set_named_property(env, result, "classification", value);

    // color ?: number
    napi_create_uint32(env, request->GetColor(), &value);
    napi_set_named_property(env, result, "color", value);

    // colorEnabled ?: boolean
    napi_get_boolean(env, request->IsColorEnabled(), &value);
    napi_set_named_property(env, result, "colorEnabled", value);

    // isAlertOnce ?: boolean
    napi_get_boolean(env, request->IsAlertOneTime(), &value);
    napi_set_named_property(env, result, "isAlertOnce", value);

    // isStopwatch ?: boolean
    napi_get_boolean(env, request->IsShowStopwatch(), &value);
    napi_set_named_property(env, result, "isStopwatch", value);

    // isCountDown ?: boolean
    napi_get_boolean(env, request->IsCountdownTimer(), &value);
    napi_set_named_property(env, result, "isCountDown", value);

    // statusBarText ?: string
    napi_create_string_utf8(env, request->GetStatusBarText().c_str(), NAPI_AUTO_LENGTH, &value);
    napi_set_named_property(env, result, "statusBarText", value);

    // isFloatingIcon ?: boolean
    napi_get_boolean(env, request->IsFloatingIcon(), &value);
    napi_set_named_property(env, result, "isFloatingIcon", value);

    // label ?: string
    napi_create_string_utf8(env, request->GetLabel().c_str(), NAPI_AUTO_LENGTH, &value);
    napi_set_named_property(env, result, "label", value);

    // badgeIconStyle ?: number
    int badgeIconStyle = (int)request->GetBadgeIconStyle();
    napi_create_int32(env, badgeIconStyle, &value);
    napi_set_named_property(env, result, "badgeIconStyle", value);

    // showDeliveryTime ?: boolean
    napi_get_boolean(env, request->IsShowDeliveryTime(), &value);
    napi_set_named_property(env, result, "showDeliveryTime", value);

    // actionButtons?: Array<NotificationActionButton>
    napi_value arr = nullptr;
    int count = 0;
    napi_create_array(env, &arr);
    for (auto vec : request->GetActionButtons()) {
        if (vec) {
            napi_value actionButtonResult = nullptr;
            napi_create_object(env, &actionButtonResult);
            if (SetNotificationActionButton(env, vec, actionButtonResult)) {
                napi_set_element(env, arr, count, actionButtonResult);
                count++;
            }
        }
    }
    napi_set_named_property(env, result, "actionButtons", arr);

    // smallIcon ?: image.PixelMap
    std::shared_ptr<Media::PixelMap> littleIcon = request->GetLittleIcon();
    if (littleIcon) {
        napi_value smallIconResult = nullptr;
        napi_valuetype valuetype = napi_undefined;
        smallIconResult = Media::PixelMapNapi::CreatePixelMap(env, littleIcon);
        NAPI_CALL(env, napi_typeof(env, smallIconResult, &valuetype));
        if (valuetype == napi_undefined) {
            ANS_LOGI("smallIconResult is undefined");
            napi_set_named_property(env, result, "smallIcon", NapiGetNull(env));
        } else {
            napi_set_named_property(env, result, "smallIcon", smallIconResult);
        }
    }

    // largeIcon ?: image.PixelMap
    std::shared_ptr<Media::PixelMap> largeIcon = request->GetBigIcon();
    if (largeIcon) {
        napi_value largeIconResult = nullptr;
        napi_valuetype valuetype = napi_undefined;
        largeIconResult = Media::PixelMapNapi::CreatePixelMap(env, largeIcon);
        NAPI_CALL(env, napi_typeof(env, largeIconResult, &valuetype));
        if (valuetype == napi_undefined) {
            ANS_LOGI("largeIconResult is undefined");
            napi_set_named_property(env, result, "largeIcon", NapiGetNull(env));
        } else {
            napi_set_named_property(env, result, "largeIcon", largeIconResult);
        }
    }

    // readonly creatorBundleName?: string
    napi_create_string_utf8(env, request->GetCreatorBundleName().c_str(), NAPI_AUTO_LENGTH, &value);
    napi_set_named_property(env, result, "creatorBundleName", value);

    // readonly creatorUid?: number
    napi_create_int32(env, request->GetCreatorUid(), &value);
    napi_set_named_property(env, result, "creatorUid", value);

    // readonly creatorPid?: number
    napi_create_int32(env, request->GetCreatorPid(), &value);
    napi_set_named_property(env, result, "creatorPid", value);

    return NapiGetboolean(env, true);
}

napi_value Common::SetNotificationSortingMap(
    const napi_env &env, const std::shared_ptr<NotificationSortingMap> &sortingMap, napi_value &result)
{
    ANS_LOGI("SetNotificationSortingMap start");
    if (sortingMap == nullptr) {
        ANS_LOGE("sortingMap is null");
        return NapiGetboolean(env, false);
    }
    if (sortingMap->GetKey().size() == 0) {
        ANS_LOGE("sortingMap GetKey().size is empty");
        return NapiGetboolean(env, false);
    }

    size_t count = 0;
    napi_value arrSortedHashCode = nullptr;
    napi_create_array(env, &arrSortedHashCode);
    napi_value sortingsResult = nullptr;
    napi_create_object(env, &sortingsResult);
    for (auto key : sortingMap->GetKey()) {
        NotificationSorting sorting;
        if (sortingMap->GetNotificationSorting(key, sorting)) {
            // sortedHashCode: Array<string>
            napi_value keyValue = nullptr;
            ANS_LOGI("sortingMap key = %{public}s", key.c_str());
            napi_create_string_utf8(env, key.c_str(), NAPI_AUTO_LENGTH, &keyValue);
            napi_set_element(env, arrSortedHashCode, count, keyValue);

            // sortings:{[key : string] : NotificationSorting}
            napi_value sortingResult = nullptr;
            napi_create_object(env, &sortingResult);
            if (!SetNotificationSorting(env, sorting, sortingResult)) {
                ANS_LOGE("SetNotificationSorting call failed");
                return NapiGetboolean(env, false);
            }
            napi_set_named_property(env, sortingsResult, key.c_str(), sortingResult);
            count++;
        } else {
            ANS_LOGW("sortingMap Key: %{public}s match value is empty", key.c_str());
        }
    }
    napi_set_named_property(env, result, "sortedHashCode", arrSortedHashCode);
    napi_set_named_property(env, result, "sortings", sortingsResult);

    return NapiGetboolean(env, true);
}

napi_value Common::SetNotificationSorting(const napi_env &env, const NotificationSorting &sorting, napi_value &result)
{
    ANS_LOGI("SetNotificationSorting start");

    // slot: NotificationSlot
    napi_value slotResult = nullptr;
    napi_value value = nullptr;
    napi_create_object(env, &slotResult);
    if (!SetNotificationSlot(env, sorting.GetSlot(), slotResult)) {
        ANS_LOGE("SetNotificationSlot call failed");
        return NapiGetboolean(env, false);
    }
    napi_set_named_property(env, result, "slot", slotResult);

    // hashCode?: string
    napi_create_string_utf8(env, sorting.GetKey().c_str(), NAPI_AUTO_LENGTH, &value);
    napi_set_named_property(env, result, "hashCode", value);

    // ranking?: number
    napi_create_int32(env, sorting.GetRanking(), &value);
    napi_set_named_property(env, result, "ranking", value);

    // isDisplayBadge?: boolean
    napi_get_boolean(env, sorting.IsDisplayBadge(), &value);
    napi_set_named_property(env, result, "isDisplayBadge", value);

    // isHiddenNotification?: boolean
    napi_get_boolean(env, sorting.IsHiddenNotification(), &value);
    napi_set_named_property(env, result, "isHiddenNotification", value);

    // importance?: number
    napi_create_int32(env, sorting.GetImportance(), &value);
    napi_set_named_property(env, result, "importance", value);

    // groupKeyOverride?: string
    napi_create_string_utf8(env, sorting.GetGroupKeyOverride().c_str(), NAPI_AUTO_LENGTH, &value);
    napi_set_named_property(env, result, "groupKeyOverride", value);

    // visiblenessOverride?: number
    napi_create_int32(env, sorting.GetVisiblenessOverride(), &value);
    napi_set_named_property(env, result, "visiblenessOverride", value);

    return NapiGetboolean(env, true);
}

napi_value Common::SetNotificationSlot(const napi_env &env, const NotificationSlot &slot, napi_value &result)
{
    ANS_LOGI("enter");

    napi_value value = nullptr;
    // type: SlotType
    enum SlotType outType = SlotType::UNKNOWN_TYPE;
    if (!SlotTypeCToJS(slot.GetType(), outType)) {
        return NapiGetboolean(env, false);
    }
    napi_create_int32(env, outType, &value);
    napi_set_named_property(env, result, "type", value);

    // level?: number
    enum SlotLevel outLevel { SlotLevel::LEVEL_NONE };
    if (!SlotLevelCToJS(slot.GetLevel(), outLevel)) {
        return NapiGetboolean(env, false);
    }
    napi_create_int32(env, outLevel, &value);
    napi_set_named_property(env, result, "level", value);

    // desc?: string
    napi_create_string_utf8(env, slot.GetDescription().c_str(), NAPI_AUTO_LENGTH, &value);
    napi_set_named_property(env, result, "desc", value);

    // badgeFlag?: boolean
    napi_get_boolean(env, slot.IsShowBadge(), &value);
    napi_set_named_property(env, result, "badgeFlag", value);

    // bypassDnd?: boolean
    napi_get_boolean(env, slot.IsEnableBypassDnd(), &value);
    napi_set_named_property(env, result, "bypassDnd", value);

    // lockscreenVisibility?: number
    int32_t lockScreenVisibleness = (int32_t)slot.GetLockScreenVisibleness();
    napi_create_int32(env, lockScreenVisibleness, &value);
    napi_set_named_property(env, result, "lockscreenVisibility", value);

    // vibrationEnabled?: boolean
    napi_get_boolean(env, slot.CanVibrate(), &value);
    napi_set_named_property(env, result, "vibrationEnabled", value);

    // sound?: string
    napi_create_string_utf8(env, slot.GetSound().ToString().c_str(), NAPI_AUTO_LENGTH, &value);
    napi_set_named_property(env, result, "sound", value);

    // lightEnabled?: boolean
    napi_get_boolean(env, slot.CanEnableLight(), &value);
    napi_set_named_property(env, result, "lightEnabled", value);

    // lightColor?: number
    napi_create_int32(env, slot.GetLedLightColor(), &value);
    napi_set_named_property(env, result, "lightColor", value);

    // vibrationValues?: Array<number>
    napi_value arr = nullptr;
    napi_create_array(env, &arr);
    size_t count = 0;
    for (auto vec : slot.GetVibrationStyle()) {
        napi_create_int64(env, vec, &value);
        napi_set_element(env, arr, count, value);
        count++;
    }
    napi_set_named_property(env, result, "vibrationValues", arr);

    return NapiGetboolean(env, true);
}

napi_value Common::SetNotificationContent(
    const napi_env &env, const std::shared_ptr<NotificationContent> &content, napi_value &result)
{
    ANS_LOGI("SetNotificationContent start");
    if (content == nullptr) {
        ANS_LOGE("content is null");
        return NapiGetboolean(env, false);
    }

    napi_value value = nullptr;
    // contentType: ContentType
    NotificationContent::Type type = content->GetContentType();
    ContentType outType = ContentType::NOTIFICATION_CONTENT_BASIC_TEXT;
    if (!ContentTypeCToJS(type, outType)) {
        return NapiGetboolean(env, false);
    }

    napi_create_int32(env, (int32_t)outType, &value);
    napi_set_named_property(env, result, "contentType", value);

    std::shared_ptr<NotificationBasicContent> basicContent = content->GetNotificationContent();
    if (basicContent == nullptr) {
        ANS_LOGI("content is null");
        return NapiGetboolean(env, false);
    }
    if (outType == ContentType::NOTIFICATION_CONTENT_BASIC_TEXT) {
        // normal?: NotificationBasicContent
        napi_value basicContentResult = nullptr;
        napi_create_object(env, &basicContentResult);
        if (SetNotificationBasicContent(env, basicContent.get(), basicContentResult)) {
            napi_set_named_property(env, result, "normal", basicContentResult);
        } else {
            return NapiGetboolean(env, false);
        }

    } else if (outType == ContentType::NOTIFICATION_CONTENT_LONG_TEXT) {
        // longText?: NotificationLongTextContent
        OHOS::Notification::NotificationLongTextContent *longTextContent =
            static_cast<OHOS::Notification::NotificationLongTextContent *>(basicContent.get());
        napi_value longTextContentResult = nullptr;
        napi_create_object(env, &longTextContentResult);
        if (SetNotificationLongTextContent(env, longTextContent, longTextContentResult)) {
            napi_set_named_property(env, result, "longText", longTextContentResult);
        } else {
            return NapiGetboolean(env, false);
        }

    } else if (outType == ContentType::NOTIFICATION_CONTENT_PICTURE) {
        // picture?: NotificationPictureContent
        OHOS::Notification::NotificationPictureContent *pictureContent =
            static_cast<OHOS::Notification::NotificationPictureContent *>(basicContent.get());
        napi_value pictureContentResult = nullptr;
        napi_create_object(env, &pictureContentResult);
        if (SetNotificationPictureContent(env, pictureContent, pictureContentResult)) {
            napi_set_named_property(env, result, "picture", pictureContentResult);
        } else {
            return NapiGetboolean(env, false);
        }
    } else if (outType == ContentType::NOTIFICATION_CONTENT_CONVERSATION) {
        // conversational?: NotificationConversationalContent
        OHOS::Notification::NotificationConversationalContent *conversationalContent =
            static_cast<OHOS::Notification::NotificationConversationalContent *>(basicContent.get());
        napi_value conversationalContentResult = nullptr;
        napi_create_object(env, &conversationalContentResult);
        if (SetNotificationConversationalContent(env, conversationalContent, conversationalContentResult)) {
            napi_set_named_property(env, result, "conversational", conversationalContentResult);
        } else {
            return NapiGetboolean(env, false);
        }
    } else if (outType == ContentType::NOTIFICATION_CONTENT_MULTILINE) {
        // multiLine?: NotificationMultiLineContent
        OHOS::Notification::NotificationMultiLineContent *multiLineContent =
            static_cast<OHOS::Notification::NotificationMultiLineContent *>(basicContent.get());
        napi_value multiLineContentResult = nullptr;
        napi_create_object(env, &multiLineContentResult);
        if (SetNotificationMultiLineContent(env, multiLineContent, multiLineContentResult)) {
            napi_set_named_property(env, result, "multiLine", multiLineContentResult);
        } else {
            return NapiGetboolean(env, false);
        }
    } else {
        ANS_LOGE("ContentType is does not exist");
        return NapiGetboolean(env, false);
    }
    return NapiGetboolean(env, true);
}

napi_value Common::SetNotificationBasicContent(
    const napi_env &env, NotificationBasicContent *basicContent, napi_value &result)
{
    ANS_LOGI("SetNotificationBasicContent start");
    if (basicContent == nullptr) {
        ANS_LOGE("basicContent is null");
        return NapiGetboolean(env, false);
    }

    napi_value value = nullptr;

    // title: string
    napi_create_string_utf8(env, basicContent->GetTitle().c_str(), NAPI_AUTO_LENGTH, &value);
    napi_set_named_property(env, result, "title", value);

    // text: string
    napi_create_string_utf8(env, basicContent->GetText().c_str(), NAPI_AUTO_LENGTH, &value);
    napi_set_named_property(env, result, "text", value);

    // additionalText?: string
    napi_create_string_utf8(env, basicContent->GetAdditionalText().c_str(), NAPI_AUTO_LENGTH, &value);
    napi_set_named_property(env, result, "additionalText", value);

    return NapiGetboolean(env, true);
}

napi_value Common::SetNotificationLongTextContent(
    const napi_env &env, OHOS::Notification::NotificationLongTextContent *longTextContent, napi_value &result)
{
    ANS_LOGI("SetNotificationLongTextContent start");
    if (longTextContent == nullptr) {
        ANS_LOGE("longTextContent is null");
        return NapiGetboolean(env, false);
    }

    if (!SetNotificationBasicContent(env, longTextContent, result)) {
        ANS_LOGE("SetNotificationBasicContent call failed");
        return NapiGetboolean(env, false);
    }

    napi_value value = nullptr;

    // longText: string
    napi_create_string_utf8(env, longTextContent->GetLongText().c_str(), NAPI_AUTO_LENGTH, &value);
    napi_set_named_property(env, result, "longText", value);

    // briefText: string
    napi_create_string_utf8(env, longTextContent->GetBriefText().c_str(), NAPI_AUTO_LENGTH, &value);
    napi_set_named_property(env, result, "briefText", value);

    // expandedTitle: string
    napi_create_string_utf8(env, longTextContent->GetExpandedTitle().c_str(), NAPI_AUTO_LENGTH, &value);
    napi_set_named_property(env, result, "expandedTitle", value);

    return NapiGetboolean(env, true);
}

napi_value Common::SetNotificationPictureContent(
    const napi_env &env, OHOS::Notification::NotificationPictureContent *pictureContent, napi_value &result)
{
    ANS_LOGI("SetNotificationPictureContent start");
    if (pictureContent == nullptr) {
        ANS_LOGE("pictureContent is null");
        return NapiGetboolean(env, false);
    }

    if (!SetNotificationBasicContent(env, pictureContent, result)) {
        ANS_LOGE("SetNotificationBasicContent call failed");
        return NapiGetboolean(env, false);
    }

    napi_value value = nullptr;

    // briefText: string
    napi_create_string_utf8(env, pictureContent->GetBriefText().c_str(), NAPI_AUTO_LENGTH, &value);
    napi_set_named_property(env, result, "briefText", value);

    // expandedTitle: string
    napi_create_string_utf8(env, pictureContent->GetExpandedTitle().c_str(), NAPI_AUTO_LENGTH, &value);
    napi_set_named_property(env, result, "expandedTitle", value);

    // picture: image.PixelMap
    std::shared_ptr<Media::PixelMap> picture = pictureContent->GetBigPicture();
    if (picture) {
        napi_value pictureResult = nullptr;
        napi_valuetype valuetype = napi_undefined;
        pictureResult = Media::PixelMapNapi::CreatePixelMap(env, picture);
        NAPI_CALL(env, napi_typeof(env, pictureResult, &valuetype));
        if (valuetype == napi_undefined) {
            ANS_LOGI("pictureResult is undefined");
            napi_set_named_property(env, result, "picture", NapiGetNull(env));
        } else {
            napi_set_named_property(env, result, "picture", pictureResult);
        }
    }
    return NapiGetboolean(env, true);
}

napi_value Common::SetNotificationConversationalContent(const napi_env &env,
    OHOS::Notification::NotificationConversationalContent *conversationalContent, napi_value &result)
{
    ANS_LOGI("SetNotificationConversationalContent start");
    if (conversationalContent == nullptr) {
        ANS_LOGE("conversationalContent is null");
        return NapiGetboolean(env, false);
    }

    if (!SetNotificationBasicContent(env, conversationalContent, result)) {
        ANS_LOGE("SetNotificationBasicContent call failed");
        return NapiGetboolean(env, false);
    }

    napi_value value = nullptr;
    // conversationTitle: string
    napi_create_string_utf8(env, conversationalContent->GetConversationTitle().c_str(), NAPI_AUTO_LENGTH, &value);
    napi_set_named_property(env, result, "conversationTitle", value);

    // conversationGroup: boolean
    napi_get_boolean(env, conversationalContent->IsConversationGroup(), &value);
    napi_set_named_property(env, result, "conversationGroup", value);

    // messages: Array<ConversationalMessage>
    napi_value arr = nullptr;
    int count = 0;
    napi_create_array(env, &arr);
    std::vector<std::shared_ptr<NotificationConversationalMessage>> messages =
        conversationalContent->GetAllConversationalMessages();
    for (auto vec : messages) {
        if (!vec) {
            continue;
        }
        napi_value conversationalMessageResult = nullptr;
        napi_create_object(env, &conversationalMessageResult);
        if (SetConversationalMessage(env, vec, conversationalMessageResult)) {
            napi_set_element(env, arr, count, conversationalMessageResult);
            count++;
        }
    }
    napi_set_named_property(env, result, "messages", arr);

    // user: MessageUser
    napi_value messageUserResult = nullptr;
    napi_create_object(env, &messageUserResult);
    if (!SetMessageUser(env, conversationalContent->GetMessageUser(), messageUserResult)) {
        messageUserResult = NapiGetNull(env);
    }
    napi_set_named_property(env, result, "user", messageUserResult);

    return NapiGetboolean(env, true);
}

napi_value Common::SetNotificationMultiLineContent(
    const napi_env &env, OHOS::Notification::NotificationMultiLineContent *multiLineContent, napi_value &result)
{
    ANS_LOGI("SetNotificationMultiLineContent start");
    if (multiLineContent == nullptr) {
        ANS_LOGE("multiLineContent is null");
        return NapiGetboolean(env, false);
    }

    if (!SetNotificationBasicContent(env, multiLineContent, result)) {
        ANS_LOGE("SetNotificationBasicContent call failed");
        return NapiGetboolean(env, false);
    }

    napi_value value = nullptr;
    // briefText: string
    napi_create_string_utf8(env, multiLineContent->GetBriefText().c_str(), NAPI_AUTO_LENGTH, &value);
    napi_set_named_property(env, result, "briefText", value);

    // longTitle: string
    napi_create_string_utf8(env, multiLineContent->GetExpandedTitle().c_str(), NAPI_AUTO_LENGTH, &value);
    napi_set_named_property(env, result, "longTitle", value);

    // lines: Array<String>
    napi_value arr = nullptr;
    int count = 0;
    napi_create_array(env, &arr);
    for (auto vec : multiLineContent->GetAllLines()) {
        napi_create_string_utf8(env, vec.c_str(), NAPI_AUTO_LENGTH, &value);
        napi_set_element(env, arr, count, value);
        count++;
    }
    napi_set_named_property(env, result, "lines", arr);

    return NapiGetboolean(env, true);
}

napi_value Common::SetMessageUser(const napi_env &env, const MessageUser &messageUser, napi_value &result)
{
    ANS_LOGI("SetMessageUser start");

    napi_value value = nullptr;
    // name: string
    napi_create_string_utf8(env, messageUser.GetName().c_str(), NAPI_AUTO_LENGTH, &value);
    napi_set_named_property(env, result, "name", value);

    // uri: string
    napi_create_string_utf8(env, messageUser.GetUri().ToString().c_str(), NAPI_AUTO_LENGTH, &value);
    napi_set_named_property(env, result, "uri", value);

    // important: boolean
    napi_get_boolean(env, messageUser.IsUserImportant(), &value);
    napi_set_named_property(env, result, "important", value);

    // machine: boolean
    napi_get_boolean(env, messageUser.IsMachine(), &value);
    napi_set_named_property(env, result, "machine", value);

    // key: string
    napi_create_string_utf8(env, messageUser.GetKey().c_str(), NAPI_AUTO_LENGTH, &value);
    napi_set_named_property(env, result, "key", value);

    return NapiGetboolean(env, true);
}

napi_value Common::SetConversationalMessage(const napi_env &env,
    const std::shared_ptr<NotificationConversationalMessage> &conversationalMessage, napi_value &result)
{
    ANS_LOGI("SetConversationalMessage start");
    if (conversationalMessage == nullptr) {
        ANS_LOGE("SetConversationalMessage is null");
        return NapiGetboolean(env, false);
    }

    napi_value value = nullptr;
    // text: string
    napi_create_string_utf8(env, conversationalMessage->GetText().c_str(), NAPI_AUTO_LENGTH, &value);
    napi_set_named_property(env, result, "text", value);

    // timestamp: number
    napi_create_int64(env, conversationalMessage->GetArrivedTime(), &value);
    napi_set_named_property(env, result, "timestamp", value);

    // mimeType: string
    napi_create_string_utf8(env, conversationalMessage->GetMimeType().c_str(), NAPI_AUTO_LENGTH, &value);
    napi_set_named_property(env, result, "mimeType", value);

    // uri: string
    napi_create_string_utf8(env, conversationalMessage->GetUri()->ToString().c_str(), NAPI_AUTO_LENGTH, &value);
    napi_set_named_property(env, result, "uri", value);

    // sender: MessageUser
    napi_value messageUserResult = nullptr;
    napi_create_object(env, &messageUserResult);
    if (SetMessageUser(env, conversationalMessage->GetSender(), messageUserResult)) {
        messageUserResult = NapiGetNull(env);
    }
    napi_set_named_property(env, result, "sender", messageUserResult);

    return NapiGetboolean(env, true);
}

napi_value Common::SetNotificationActionButton(
    const napi_env &env, const std::shared_ptr<NotificationActionButton> &actionButton, napi_value &result)
{
    ANS_LOGI("SetNotificationActionButton start");
    if (actionButton == nullptr) {
        ANS_LOGE("actionButton is null");
        return NapiGetboolean(env, false);
    }

    napi_value value = nullptr;

    // title: string
    napi_create_string_utf8(env, actionButton->GetTitle().c_str(), NAPI_AUTO_LENGTH, &value);
    napi_set_named_property(env, result, "title", value);

    // wantAgent: WantAgent
    std::shared_ptr<WantAgent::WantAgent> agent = actionButton->GetWantAgent();
    if (agent == nullptr) {
        ANS_LOGI("wantAgent is null");
        napi_set_named_property(env, result, "wantAgent", NapiGetNull(env));
        return NapiGetboolean(env, false);
    }
    napi_value wantAgent = nullptr;
    wantAgent = CreateWantAgentByJS(env, agent);
    napi_set_named_property(env, result, "wantAgent", wantAgent);

    // icon?: image.PixelMap
    std::shared_ptr<Media::PixelMap> icon = actionButton->GetIcon();
    if (icon) {
        napi_value iconResult = nullptr;
        napi_valuetype valuetype = napi_undefined;
        iconResult = Media::PixelMapNapi::CreatePixelMap(env, icon);
        NAPI_CALL(env, napi_typeof(env, iconResult, &valuetype));
        if (valuetype == napi_undefined) {
            ANS_LOGI("iconResult is undefined");
            napi_set_named_property(env, result, "icon", NapiGetNull(env));
        } else {
            napi_set_named_property(env, result, "icon", iconResult);
        }
    }
    return NapiGetboolean(env, true);
}

napi_value Common::GetNotificationSubscriberInfo(
    const napi_env &env, const napi_value &value, NotificationSubscribeInfo &subscriberInfo)
{
    uint32_t length = 0;
    size_t strLen = 0;
    bool hasProperty = false;
    bool isArray = false;
    napi_valuetype valuetype = napi_undefined;

    // bundleNames?: Array<string>
    NAPI_CALL(env, napi_has_named_property(env, value, "bundleNames", &hasProperty));
    if (hasProperty) {
        napi_value nBundleNames = nullptr;
        napi_get_named_property(env, value, "bundleNames", &nBundleNames);
        napi_is_array(env, nBundleNames, &isArray);
        NAPI_ASSERT(env, isArray, "Property bundleNames is expected to be an array.");
        napi_get_array_length(env, nBundleNames, &length);
        NAPI_ASSERT(env, length > 0, "The array is empty.");
        for (uint32_t i = 0; i < length; ++i) {
            napi_value nBundleName = nullptr;
            char str[STR_MAX_SIZE] = {0};
            napi_get_element(env, nBundleNames, i, &nBundleName);
            NAPI_CALL(env, napi_typeof(env, nBundleName, &valuetype));
            NAPI_ASSERT(env, valuetype == napi_string, "Wrong argument type. String expected.");
            NAPI_CALL(env, napi_get_value_string_utf8(env, nBundleName, str, STR_MAX_SIZE - 1, &strLen));
            subscriberInfo.bundleNames.emplace_back(str);
            subscriberInfo.hasSubscribeInfo = true;
        }
    }

    // userId?: number
    NAPI_CALL(env, napi_has_named_property(env, value, "userId", &hasProperty));
    if (hasProperty) {
        napi_value nUserId = nullptr;
        napi_get_named_property(env, value, "userId", &nUserId);
        NAPI_CALL(env, napi_typeof(env, nUserId, &valuetype));
        NAPI_ASSERT(env, valuetype == napi_number, "Wrong argument type. Number expected.");
        NAPI_CALL(env, napi_get_value_int32(env, nUserId, &subscriberInfo.userId));
        subscriberInfo.hasSubscribeInfo = true;
    }

    return NapiGetNull(env);
}

napi_value Common::GetNotificationRequest(const napi_env &env, const napi_value &value, NotificationRequest &request)
{
    ANS_LOGI("enter");

    // content: NotificationContent
    if (GetNotificationContent(env, value, request) == nullptr) {
        return nullptr;
    }

    // id?: number
    if (GetIdByPublish(env, value, request) == nullptr) {
        return nullptr;
    }

    // slotType?: notification.SlotType
    if (GetSlotType(env, value, request) == nullptr) {
        return nullptr;
    }

    // isOngoing?: boolean
    if (GetNotificationIsOngoing(env, value, request) == nullptr) {
        return nullptr;
    }

    // isUnremovable?: boolean
    if (GetNotificationIsUnremovable(env, value, request) == nullptr) {
        return nullptr;
    }

    // deliveryTime?: number
    if (GetNotificationDeliveryTime(env, value, request) == nullptr) {
        return nullptr;
    }

    // tapDismissed?: boolean
    if (GetNotificationtapDismissed(env, value, request) == nullptr) {
        return nullptr;
    }

    // wantAgent?: WantAgent
    if (GetNotificationWantAgent(env, value, request) == nullptr) {
        return nullptr;
    }

    // extraInfo?: {[key: string]: any}
    if (GetNotificationExtraInfo(env, value, request) == nullptr) {
        return nullptr;
    }

    // removalWantAgent?: WantAgent
    if (GetNotificationRemovalWantAgent(env, value, request) == nullptr) {
        return nullptr;
    }

    // maxScreenWantAgent?: WantAgent
    if (GetNotificationMaxScreenWantAgent(env, value, request) == nullptr) {
        return nullptr;
    }

    // autoDeletedTime?: number
    if (GetNotificationAutoDeletedTime(env, value, request) == nullptr) {
        return nullptr;
    }

    // classification?: string
    if (GetNotificationClassification(env, value, request) == nullptr) {
        return nullptr;
    }

    // color?: number
    if (GetNotificationColor(env, value, request) == nullptr) {
        return nullptr;
    }

    // colorEnabled?: boolean
    if (GetNotificationColorEnabled(env, value, request) == nullptr) {
        return nullptr;
    }

    // isAlertOnce?: boolean
    if (GetNotificationIsAlertOnce(env, value, request) == nullptr) {
        return nullptr;
    }

    // isStopwatch?: boolean
    if (GetNotificationIsStopwatch(env, value, request) == nullptr) {
        return nullptr;
    }

    // isCountDown?: boolean
    if (GetNotificationIsCountDown(env, value, request) == nullptr) {
        return nullptr;
    }

    // statusBarText?: string
    if (GetNotificationStatusBarText(env, value, request) == nullptr) {
        return nullptr;
    }

    // label?: string
    if (GetNotificationLabel(env, value, request) == nullptr) {
        return nullptr;
    }

    // badgeIconStyle?: number
    if (GetNotificationBadgeIconStyle(env, value, request) == nullptr) {
        return nullptr;
    }

    // showDeliveryTime?: boolean
    if (GetNotificationShowDeliveryTime(env, value, request) == nullptr) {
        return nullptr;
    }

    // actionButtons?: Array<NotificationActionButton>
    if (GetNotificationNotificationActionButtons(env, value, request) == nullptr) {
        return nullptr;
    }

    // smallIcon?: image.PixelMap
    if (GetNotificationSmallIcon(env, value, request) == nullptr) {
        return nullptr;
    }

    // largeIcon?: image.PixelMap
    if (GetNotificationLargeIcon(env, value, request) == nullptr) {
        return nullptr;
    }

    return Common::NapiGetNull(env);
}

napi_value GetIdByPublish(const napi_env &env, const napi_value &value, NotificationRequest &request)
{
    ANS_LOGI("enter");

    napi_valuetype valuetype = napi_undefined;
    napi_value result = nullptr;
    bool hasProperty = false;
    int32_t notificationId = 0;

    NAPI_CALL(env, napi_has_named_property(env, value, "id", &hasProperty));
    if (hasProperty) {
        napi_get_named_property(env, value, "id", &result);
        NAPI_CALL(env, napi_typeof(env, result, &valuetype));
        NAPI_ASSERT(env, valuetype == napi_number, "Wrong argument type. Number expected.");
        napi_get_value_int32(env, result, &notificationId);
        request.SetNotificationId(notificationId);
        ANS_LOGI("============GetIdByPublish notificationId = %{public}d", notificationId);
    } else {
        request.SetNotificationId(0);
    }

    ANS_LOGI("end");
    return Common::NapiGetNull(env);
}

napi_value GetSlotType(const napi_env &env, const napi_value &value, NotificationRequest &request)
{
    ANS_LOGI("enter");

    napi_valuetype valuetype = napi_undefined;
    napi_value result = nullptr;
    bool hasProperty = false;
    int32_t slotType = 0;

    NAPI_CALL(env, napi_has_named_property(env, value, "slotType", &hasProperty));
    if (hasProperty) {
        napi_get_named_property(env, value, "slotType", &result);
        NAPI_CALL(env, napi_typeof(env, result, &valuetype));
        NAPI_ASSERT(env, valuetype == napi_number, "Wrong argument type. Number expected.");
        napi_get_value_int32(env, result, &slotType);

        enum NotificationConstant::SlotType outType = NotificationConstant::SlotType::OTHER;
        if (!Common::SlotTypeJSToC(SlotType(slotType), outType)) {
            return nullptr;
        }
        request.SetSlotType(outType);
        ANS_LOGI("============GetSlotType slotType = %{public}d", slotType);
    } else {
        request.SetSlotType(NotificationConstant::OTHER);
    }
    ANS_LOGI("end");
    return Common::NapiGetNull(env);
}

napi_value Common::GetNotificationContent(const napi_env &env, const napi_value &value, NotificationRequest &request)
{
    ANS_LOGI("enter");

    napi_valuetype valuetype = napi_undefined;
    napi_value result = nullptr;
    bool hasProperty = false;
    int32_t type = 0;

    NAPI_CALL(env, napi_has_named_property(env, value, "content", &hasProperty));
    if (!hasProperty) {
        ANS_LOGE("Property content expected.");
        return nullptr;
    }

    napi_get_named_property(env, value, "content", &result);
    NAPI_CALL(env, napi_typeof(env, result, &valuetype));
    NAPI_ASSERT(env, valuetype == napi_object, "Wrong argument type. Object expected.");
    if (GetNotificationContentType(env, result, type) == nullptr) {
        return nullptr;
    }
    NotificationContent::Type outType = NotificationContent::Type::NONE;
    if (!ContentTypeJSToC(ContentType(type), outType)) {
        return nullptr;
    }
    switch (outType) {
        case NotificationContent::Type::BASIC_TEXT:
            if (GetNotificationBasicContent(env, result, request) == nullptr) {
                return nullptr;
            }
            break;
        case NotificationContent::Type::LONG_TEXT:
            if (GetNotificationLongTextContent(env, result, request) == nullptr) {
                return nullptr;
            }
            break;
        case NotificationContent::Type::PICTURE:
            if (GetNotificationPictureContent(env, result, request) == nullptr) {
                return nullptr;
            }
            break;
        case NotificationContent::Type::CONVERSATION:
            if (GetNotificationConversationalContent(env, result, request) == nullptr) {
                return nullptr;
            }
            break;
        case NotificationContent::Type::MULTILINE:
            if (GetNotificationMultiLineContent(env, result, request) == nullptr) {
                return nullptr;
            }
            break;
        case NotificationContent::Type::MEDIA:
            break;
        default:
            return nullptr;
            break;
    }

    ANS_LOGI("end");
    return Common::NapiGetNull(env);
}

napi_value GetNotificationIsOngoing(const napi_env &env, const napi_value &value, NotificationRequest &request)
{
    ANS_LOGI("enter");

    napi_valuetype valuetype = napi_undefined;
    napi_value result = nullptr;
    bool hasProperty = false;
    bool isOngoing = false;

    NAPI_CALL(env, napi_has_named_property(env, value, "isOngoing", &hasProperty));
    if (hasProperty) {
        napi_get_named_property(env, value, "isOngoing", &result);
        NAPI_CALL(env, napi_typeof(env, result, &valuetype));
        NAPI_ASSERT(env, valuetype == napi_boolean, "Wrong argument type. Bool expected.");
        napi_get_value_bool(env, result, &isOngoing);
        request.SetInProgress(isOngoing);
    }

    return Common::NapiGetNull(env);
}

napi_value GetNotificationIsUnremovable(const napi_env &env, const napi_value &value, NotificationRequest &request)
{
    ANS_LOGI("enter");

    napi_valuetype valuetype = napi_undefined;
    napi_value result = nullptr;
    bool hasProperty = false;
    bool isUnremovable = false;

    NAPI_CALL(env, napi_has_named_property(env, value, "isUnremovable", &hasProperty));
    if (hasProperty) {
        napi_get_named_property(env, value, "isUnremovable", &result);
        NAPI_CALL(env, napi_typeof(env, result, &valuetype));
        NAPI_ASSERT(env, valuetype == napi_boolean, "Wrong argument type. Bool expected.");
        napi_get_value_bool(env, result, &isUnremovable);
        request.SetUnremovable(isUnremovable);
    }

    return Common::NapiGetNull(env);
}

napi_value GetNotificationDeliveryTime(const napi_env &env, const napi_value &value, NotificationRequest &request)
{
    ANS_LOGI("enter");

    napi_valuetype valuetype = napi_undefined;
    napi_value result = nullptr;
    bool hasProperty = false;
    int64_t deliveryTime = 0;

    NAPI_CALL(env, napi_has_named_property(env, value, "deliveryTime", &hasProperty));
    if (hasProperty) {
        napi_get_named_property(env, value, "deliveryTime", &result);
        NAPI_CALL(env, napi_typeof(env, result, &valuetype));
        NAPI_ASSERT(env, valuetype == napi_number, "Wrong argument type. Number expected.");
        napi_get_value_int64(env, result, &deliveryTime);
        request.SetDeliveryTime(deliveryTime);
    }

    return Common::NapiGetNull(env);
}

napi_value GetNotificationtapDismissed(const napi_env &env, const napi_value &value, NotificationRequest &request)
{
    ANS_LOGI("enter");

    napi_valuetype valuetype = napi_undefined;
    napi_value result = nullptr;
    bool hasProperty = false;
    bool tapDismissed = false;

    NAPI_CALL(env, napi_has_named_property(env, value, "tapDismissed", &hasProperty));
    if (hasProperty) {
        napi_get_named_property(env, value, "tapDismissed", &result);
        NAPI_CALL(env, napi_typeof(env, result, &valuetype));
        NAPI_ASSERT(env, valuetype == napi_boolean, "Wrong argument type. Bool expected.");
        napi_get_value_bool(env, result, &tapDismissed);
        request.SetTapDismissed(tapDismissed);
    }

    return Common::NapiGetNull(env);
}

napi_value Common::GetNotificationWantAgent(const napi_env &env, const napi_value &value, NotificationRequest &request)
{
    ANS_LOGI("enter");

    bool hasProperty = false;
    WantAgent::WantAgent *wantAgent = nullptr;
    napi_value result = nullptr;
    napi_valuetype valuetype = napi_undefined;

    NAPI_CALL(env, napi_has_named_property(env, value, "wantAgent", &hasProperty));
    if (hasProperty) {
        napi_get_named_property(env, value, "wantAgent", &result);
        NAPI_CALL(env, napi_typeof(env, result, &valuetype));
        NAPI_ASSERT(env, valuetype == napi_object, "Wrong argument type. Object expected.");
        napi_unwrap(env, result, (void **)&wantAgent);
        if (wantAgent == nullptr) {
            ANS_LOGE("Invalid object wantAgent");
            return nullptr;
        }
        std::shared_ptr<WantAgent::WantAgent> sWantAgent = std::make_shared<WantAgent::WantAgent>(*wantAgent);
        request.SetWantAgent(sWantAgent);
    }

    return Common::NapiGetNull(env);
}

napi_value GetNotificationExtraInfo(const napi_env &env, const napi_value &value, NotificationRequest &request)
{
    ANS_LOGI("enter");

    napi_valuetype valuetype = napi_undefined;
    napi_value result = nullptr;
    bool hasProperty = false;

    NAPI_CALL(env, napi_has_named_property(env, value, "extraInfo", &hasProperty));
    if (hasProperty) {
        napi_get_named_property(env, value, "extraInfo", &result);
        NAPI_CALL(env, napi_typeof(env, result, &valuetype));
        NAPI_ASSERT(env, valuetype == napi_object, "Wrong argument type. Object expected.");
        AAFwk::WantParams wantParams;
        if (!OHOS::AppExecFwk::UnwrapWantParams(env, result, wantParams)) {
            return nullptr;
        }

        std::shared_ptr<AAFwk::WantParams> extras = std::make_shared<AAFwk::WantParams>(wantParams);
        request.SetAdditionalData(extras);
    }

    return Common::NapiGetNull(env);
}

napi_value GetNotificationRemovalWantAgent(const napi_env &env, const napi_value &value, NotificationRequest &request)
{
    ANS_LOGI("enter");

    bool hasProperty = false;
    WantAgent::WantAgent *wantAgent = nullptr;
    napi_value result = nullptr;
    napi_valuetype valuetype = napi_undefined;

    NAPI_CALL(env, napi_has_named_property(env, value, "removalWantAgent", &hasProperty));
    if (hasProperty) {
        napi_get_named_property(env, value, "removalWantAgent", &result);
        NAPI_CALL(env, napi_typeof(env, result, &valuetype));
        NAPI_ASSERT(env, valuetype == napi_object, "Wrong argument type. Object expected.");
        napi_unwrap(env, result, (void **)&wantAgent);
        if (wantAgent == nullptr) {
            ANS_LOGE("Invalid object removalWantAgent");
            return nullptr;
        }
        std::shared_ptr<WantAgent::WantAgent> removeWantAgent = std::make_shared<WantAgent::WantAgent>(*wantAgent);
        request.SetRemovalWantAgent(removeWantAgent);
    }

    return Common::NapiGetNull(env);
}

napi_value GetNotificationMaxScreenWantAgent(const napi_env &env, const napi_value &value, NotificationRequest &request)
{
    ANS_LOGI("enter");

    bool hasProperty = false;
    WantAgent::WantAgent *wantAgent = nullptr;
    napi_value result = nullptr;
    napi_valuetype valuetype = napi_undefined;

    NAPI_CALL(env, napi_has_named_property(env, value, "maxScreenWantAgent", &hasProperty));
    if (hasProperty) {
        napi_get_named_property(env, value, "maxScreenWantAgent", &result);
        NAPI_CALL(env, napi_typeof(env, result, &valuetype));
        NAPI_ASSERT(env, valuetype == napi_object, "Wrong argument type. Object expected.");
        napi_unwrap(env, result, (void **)&wantAgent);
        if (wantAgent == nullptr) {
            ANS_LOGE("Invalid object maxScreenWantAgent");
            return nullptr;
        }
        std::shared_ptr<WantAgent::WantAgent> maxScreenWantAgent = std::make_shared<WantAgent::WantAgent>(*wantAgent);
        request.SetMaxScreenWantAgent(maxScreenWantAgent);
    }

    return Common::NapiGetNull(env);
}

napi_value GetNotificationAutoDeletedTime(const napi_env &env, const napi_value &value, NotificationRequest &request)
{
    ANS_LOGI("enter");

    napi_valuetype valuetype = napi_undefined;
    napi_value result = nullptr;
    bool hasProperty = false;
    int64_t autoDeletedTime = 0;

    NAPI_CALL(env, napi_has_named_property(env, value, "autoDeletedTime", &hasProperty));
    if (hasProperty) {
        napi_get_named_property(env, value, "autoDeletedTime", &result);
        NAPI_CALL(env, napi_typeof(env, result, &valuetype));
        NAPI_ASSERT(env, valuetype == napi_number, "Wrong argument type. Number expected.");
        napi_get_value_int64(env, result, &autoDeletedTime);
        request.SetAutoDeletedTime(autoDeletedTime);
    }

    return Common::NapiGetNull(env);
}

napi_value GetNotificationClassification(const napi_env &env, const napi_value &value, NotificationRequest &request)
{
    ANS_LOGI("enter");

    napi_valuetype valuetype = napi_undefined;
    napi_value result = nullptr;
    bool hasProperty = false;
    char str[STR_MAX_SIZE] = {0};
    size_t strLen = 0;

    NAPI_CALL(env, napi_has_named_property(env, value, "classification", &hasProperty));
    if (hasProperty) {
        napi_get_named_property(env, value, "classification", &result);
        NAPI_CALL(env, napi_typeof(env, result, &valuetype));
        NAPI_ASSERT(env, valuetype == napi_string, "Wrong argument type. String expected.");
        NAPI_CALL(env, napi_get_value_string_utf8(env, result, str, STR_MAX_SIZE - 1, &strLen));
        request.SetClassification(str);
    }

    return Common::NapiGetNull(env);
}

napi_value GetNotificationColor(const napi_env &env, const napi_value &value, NotificationRequest &request)
{
    ANS_LOGI("enter");

    napi_valuetype valuetype = napi_undefined;
    napi_value result = nullptr;
    bool hasProperty = false;
    int32_t color = 0;

    NAPI_CALL(env, napi_has_named_property(env, value, "color", &hasProperty));
    if (hasProperty) {
        napi_get_named_property(env, value, "color", &result);
        NAPI_CALL(env, napi_typeof(env, result, &valuetype));
        NAPI_ASSERT(env, valuetype == napi_number, "Wrong argument type. Number expected.");
        napi_get_value_int32(env, result, &color);
        NAPI_ASSERT(env, color >= 0, "Wrong argument type. Natural number expected.");
        request.SetColor(color);
    }

    return Common::NapiGetNull(env);
}

napi_value GetNotificationColorEnabled(const napi_env &env, const napi_value &value, NotificationRequest &request)
{
    ANS_LOGI("enter");

    napi_valuetype valuetype = napi_undefined;
    napi_value result = nullptr;
    bool hasProperty = false;
    bool colorEnabled = false;

    NAPI_CALL(env, napi_has_named_property(env, value, "colorEnabled", &hasProperty));
    if (hasProperty) {
        napi_get_named_property(env, value, "colorEnabled", &result);
        NAPI_CALL(env, napi_typeof(env, result, &valuetype));
        NAPI_ASSERT(env, valuetype == napi_boolean, "Wrong argument type. Bool expected.");
        napi_get_value_bool(env, result, &colorEnabled);
        request.SetColorEnabled(colorEnabled);
    }

    return Common::NapiGetNull(env);
}

napi_value GetNotificationIsAlertOnce(const napi_env &env, const napi_value &value, NotificationRequest &request)
{
    ANS_LOGI("enter");

    napi_valuetype valuetype = napi_undefined;
    napi_value result = nullptr;
    bool hasProperty = false;
    bool isAlertOnce = false;

    NAPI_CALL(env, napi_has_named_property(env, value, "isAlertOnce", &hasProperty));
    if (hasProperty) {
        napi_get_named_property(env, value, "isAlertOnce", &result);
        NAPI_CALL(env, napi_typeof(env, result, &valuetype));
        NAPI_ASSERT(env, valuetype == napi_boolean, "Wrong argument type. Bool expected.");
        napi_get_value_bool(env, result, &isAlertOnce);
        request.SetAlertOneTime(isAlertOnce);
    }

    return Common::NapiGetNull(env);
}

napi_value GetNotificationIsStopwatch(const napi_env &env, const napi_value &value, NotificationRequest &request)
{
    ANS_LOGI("enter");

    napi_valuetype valuetype = napi_undefined;
    napi_value result = nullptr;
    bool hasProperty = false;
    bool isStopwatch = false;

    NAPI_CALL(env, napi_has_named_property(env, value, "isStopwatch", &hasProperty));
    if (hasProperty) {
        napi_get_named_property(env, value, "isStopwatch", &result);
        NAPI_CALL(env, napi_typeof(env, result, &valuetype));
        NAPI_ASSERT(env, valuetype == napi_boolean, "Wrong argument type. Bool expected.");
        napi_get_value_bool(env, result, &isStopwatch);
        request.SetShowStopwatch(isStopwatch);
    }

    return Common::NapiGetNull(env);
}

napi_value GetNotificationIsCountDown(const napi_env &env, const napi_value &value, NotificationRequest &request)
{
    ANS_LOGI("enter");

    napi_valuetype valuetype = napi_undefined;
    napi_value result = nullptr;
    bool hasProperty = false;
    bool isCountDown = false;

    NAPI_CALL(env, napi_has_named_property(env, value, "isCountDown", &hasProperty));
    if (hasProperty) {
        napi_get_named_property(env, value, "isCountDown", &result);
        NAPI_CALL(env, napi_typeof(env, result, &valuetype));
        NAPI_ASSERT(env, valuetype == napi_boolean, "Wrong argument type. Bool expected.");
        napi_get_value_bool(env, result, &isCountDown);
        request.SetCountdownTimer(isCountDown);
    }

    return Common::NapiGetNull(env);
}

napi_value GetNotificationStatusBarText(const napi_env &env, const napi_value &value, NotificationRequest &request)
{
    ANS_LOGI("enter");

    napi_valuetype valuetype = napi_undefined;
    napi_value result = nullptr;
    bool hasProperty = false;
    char str[STR_MAX_SIZE] = {0};
    size_t strLen = 0;

    NAPI_CALL(env, napi_has_named_property(env, value, "statusBarText", &hasProperty));
    if (hasProperty) {
        napi_get_named_property(env, value, "statusBarText", &result);
        NAPI_CALL(env, napi_typeof(env, result, &valuetype));
        NAPI_ASSERT(env, valuetype == napi_string, "Wrong argument type. String expected.");
        NAPI_CALL(env, napi_get_value_string_utf8(env, result, str, STR_MAX_SIZE - 1, &strLen));
        request.SetStatusBarText(str);
    }

    return Common::NapiGetNull(env);
}

napi_value GetNotificationLabel(const napi_env &env, const napi_value &value, NotificationRequest &request)
{
    ANS_LOGI("enter");

    napi_valuetype valuetype = napi_undefined;
    napi_value result = nullptr;
    bool hasProperty = false;
    char str[STR_MAX_SIZE] = {0};
    size_t strLen = 0;

    NAPI_CALL(env, napi_has_named_property(env, value, "label", &hasProperty));
    if (hasProperty) {
        napi_get_named_property(env, value, "label", &result);
        NAPI_CALL(env, napi_typeof(env, result, &valuetype));
        NAPI_ASSERT(env, valuetype == napi_string, "Wrong argument type. String expected.");
        NAPI_CALL(env, napi_get_value_string_utf8(env, result, str, STR_MAX_SIZE - 1, &strLen));
        request.SetLabel(str);
    }

    return Common::NapiGetNull(env);
}

napi_value GetNotificationBadgeIconStyle(const napi_env &env, const napi_value &value, NotificationRequest &request)
{
    ANS_LOGI("enter");

    napi_valuetype valuetype = napi_undefined;
    napi_value result = nullptr;
    bool hasProperty = false;
    int32_t badgeIconStyle = 0;

    NAPI_CALL(env, napi_has_named_property(env, value, "badgeIconStyle", &hasProperty));
    if (hasProperty) {
        napi_get_named_property(env, value, "badgeIconStyle", &result);
        NAPI_CALL(env, napi_typeof(env, result, &valuetype));
        NAPI_ASSERT(env, valuetype == napi_number, "Wrong argument type. Number expected.");
        napi_get_value_int32(env, result, &badgeIconStyle);
        request.SetBadgeIconStyle(static_cast<NotificationRequest::BadgeStyle>(badgeIconStyle));
    }

    return Common::NapiGetNull(env);
}

napi_value GetNotificationShowDeliveryTime(const napi_env &env, const napi_value &value, NotificationRequest &request)
{
    ANS_LOGI("enter");

    napi_valuetype valuetype = napi_undefined;
    napi_value result = nullptr;
    bool hasProperty = false;
    bool showDeliveryTime = false;

    NAPI_CALL(env, napi_has_named_property(env, value, "showDeliveryTime", &hasProperty));
    if (hasProperty) {
        napi_get_named_property(env, value, "showDeliveryTime", &result);
        NAPI_CALL(env, napi_typeof(env, result, &valuetype));
        NAPI_ASSERT(env, valuetype == napi_boolean, "Wrong argument type. Bool expected.");
        napi_get_value_bool(env, result, &showDeliveryTime);
        request.SetShowDeliveryTime(showDeliveryTime);
    }

    return Common::NapiGetNull(env);
}

napi_value GetNotificationNotificationActionButtons(
    const napi_env &env, const napi_value &value, NotificationRequest &request)
{
    ANS_LOGI("enter");

    bool isArray = false;
    napi_valuetype valuetype = napi_undefined;
    napi_value actionButtons = nullptr;
    char str[STR_MAX_SIZE] = {0};
    size_t strLen = 0;
    uint32_t length = 0;
    bool hasProperty = false;
    napi_value titelActionButton = nullptr;
    napi_value wantAgentActionButton = nullptr;
    napi_value iconActionButton = nullptr;
    std::string title;
    WantAgent::WantAgent *wantAgentPtr = nullptr;
    std::shared_ptr<Media::PixelMap> pixelMap = nullptr;
    std::shared_ptr<WantAgent::WantAgent> wantAgent;

    napi_has_named_property(env, value, "actionButtons", &hasProperty);
    if (!hasProperty) {
        return Common::NapiGetNull(env);
    }

    napi_get_named_property(env, value, "actionButtons", &actionButtons);
    napi_is_array(env, actionButtons, &isArray);
    NAPI_ASSERT(env, isArray, "Property actionButtons is expected to be an array.");
    napi_get_array_length(env, actionButtons, &length);
    NAPI_ASSERT(env, length > 0, "The array is empty.");
    for (size_t i = 0; i < length; i++) {
        napi_value actionButton = nullptr;
        napi_get_element(env, actionButtons, i, &actionButton);
        NAPI_CALL(env, napi_typeof(env, actionButton, &valuetype));
        NAPI_ASSERT(env, valuetype == napi_object, "Wrong argument type. Object expected.");

        // title: string
        NAPI_CALL(env, napi_has_named_property(env, actionButton, "title", &hasProperty));
        if (hasProperty) {
            napi_get_named_property(env, actionButton, "title", &titelActionButton);
            NAPI_CALL(env, napi_typeof(env, titelActionButton, &valuetype));
            NAPI_ASSERT(env, valuetype == napi_string, "Wrong argument type. String expected.");
            NAPI_CALL(env, napi_get_value_string_utf8(env, titelActionButton, str, STR_MAX_SIZE - 1, &strLen));
            title = str;
        } else {
            ANS_LOGE("Property title expected.");
            return nullptr;
        }

        // wantAgent: WantAgent
        NAPI_CALL(env, napi_has_named_property(env, actionButton, "wantAgent", &hasProperty));
        if (hasProperty) {
            napi_get_named_property(env, actionButton, "wantAgent", &wantAgentActionButton);
            NAPI_CALL(env, napi_typeof(env, wantAgentActionButton, &valuetype));
            NAPI_ASSERT(env, valuetype == napi_object, "Wrong argument type. Object expected.");
            napi_unwrap(env, wantAgentActionButton, (void **)&wantAgentPtr);
            if (wantAgentPtr == nullptr) {
                ANS_LOGE("Invalid object wantAgent");
                return nullptr;
            }
            wantAgent = std::make_shared<WantAgent::WantAgent>(*wantAgentPtr);
        } else {
            ANS_LOGE("Property wantAgent expected.");
            return nullptr;
        }

        // icon?: image.PixelMap
        NAPI_CALL(env, napi_has_named_property(env, actionButton, "icon", &hasProperty));
        if (hasProperty) {
            napi_get_named_property(env, actionButton, "icon", &iconActionButton);
            NAPI_CALL(env, napi_typeof(env, iconActionButton, &valuetype));
            NAPI_ASSERT(env, valuetype == napi_object, "Wrong argument type. Object expected.");
            pixelMap = Media::PixelMapNapi::GetPixelMap(env, iconActionButton);
            if (pixelMap == nullptr) {
                ANS_LOGE("Invalid object pixelMap");
                return nullptr;
            }
        }

        auto pActionButton = NotificationActionButton::Create(pixelMap, title, wantAgent);
        request.AddActionButton(pActionButton);
    }

    return Common::NapiGetNull(env);
}

napi_value GetNotificationSmallIcon(const napi_env &env, const napi_value &value, NotificationRequest &request)
{
    ANS_LOGI("enter");

    napi_valuetype valuetype = napi_undefined;
    napi_value result = nullptr;
    bool hasProperty = false;

    NAPI_CALL(env, napi_has_named_property(env, value, "smallIcon", &hasProperty));
    if (hasProperty) {
        napi_get_named_property(env, value, "smallIcon", &result);
        NAPI_CALL(env, napi_typeof(env, result, &valuetype));
        NAPI_ASSERT(env, valuetype == napi_object, "Wrong argument type. Object expected.");
        std::shared_ptr<Media::PixelMap> pixelMap = nullptr;
        pixelMap = Media::PixelMapNapi::GetPixelMap(env, result);
        if (pixelMap == nullptr) {
            ANS_LOGE("Invalid object pixelMap");
            return nullptr;
        }
        request.SetLittleIcon(pixelMap);
    }

    return Common::NapiGetNull(env);
}

napi_value GetNotificationLargeIcon(const napi_env &env, const napi_value &value, NotificationRequest &request)
{
    ANS_LOGI("enter");

    napi_valuetype valuetype = napi_undefined;
    napi_value result = nullptr;
    bool hasProperty = false;

    NAPI_CALL(env, napi_has_named_property(env, value, "largeIcon", &hasProperty));
    if (hasProperty) {
        napi_get_named_property(env, value, "largeIcon", &result);
        NAPI_CALL(env, napi_typeof(env, result, &valuetype));
        NAPI_ASSERT(env, valuetype == napi_object, "Wrong argument type. Object expected.");
        std::shared_ptr<Media::PixelMap> pixelMap = nullptr;
        pixelMap = Media::PixelMapNapi::GetPixelMap(env, result);
        if (pixelMap == nullptr) {
            ANS_LOGE("Invalid object pixelMap");
            return nullptr;
        }
        request.SetBigIcon(pixelMap);
    }

    return Common::NapiGetNull(env);
}

napi_value GetNotificationContentType(const napi_env &env, const napi_value &result, int32_t &type)
{
    ANS_LOGI("enter");

    napi_value contentResult = nullptr;
    napi_valuetype valuetype = napi_undefined;
    bool hasProperty = false;

    NAPI_CALL(env, napi_has_named_property(env, result, "contentType", &hasProperty));
    if (!hasProperty) {
        ANS_LOGE("Property contentType expected.");
        return nullptr;
    }

    napi_get_named_property(env, result, "contentType", &contentResult);
    NAPI_CALL(env, napi_typeof(env, contentResult, &valuetype));
    NAPI_ASSERT(env, valuetype == napi_number, "Wrong argument type. Number expected.");
    napi_get_value_int32(env, contentResult, &type);

    return Common::NapiGetNull(env);
}

napi_value GetNotificationBasicContent(const napi_env &env, const napi_value &result, NotificationRequest &request)
{
    ANS_LOGI("enter");

    napi_valuetype valuetype = napi_undefined;
    napi_value contentResult = nullptr;
    napi_value basicContentResult = nullptr;
    bool hasProperty = false;
    char str[STR_MAX_SIZE] = {0};
    size_t strLen = 0;

    NAPI_CALL(env, napi_has_named_property(env, result, "normal", &hasProperty));
    NAPI_ASSERT(env, hasProperty, "Property normal expected.");
    napi_get_named_property(env, result, "normal", &contentResult);
    NAPI_CALL(env, napi_typeof(env, contentResult, &valuetype));
    NAPI_ASSERT(env, valuetype == napi_object, "Wrong argument type. Object expected.");

    std::shared_ptr<NotificationNormalContent> normalContent = std::make_shared<NotificationNormalContent>();
    if (normalContent == nullptr) {
        ANS_LOGE("normalContent is null");
        return nullptr;
    }

    // title: string
    NAPI_CALL(env, napi_has_named_property(env, contentResult, "title", &hasProperty));
    if (hasProperty) {
        napi_get_named_property(env, contentResult, "title", &basicContentResult);
        NAPI_CALL(env, napi_typeof(env, basicContentResult, &valuetype));
        NAPI_ASSERT(env, valuetype == napi_string, "Wrong argument type. String expected.");
        NAPI_CALL(env, napi_get_value_string_utf8(env, basicContentResult, str, STR_MAX_SIZE - 1, &strLen));
        normalContent->SetTitle(str);
        ANS_LOGI("normal::title = %{public}s", str);
    } else {
        ANS_LOGE("Property title expected.");
        return nullptr;
    }

    // text: string
    NAPI_CALL(env, napi_has_named_property(env, contentResult, "text", &hasProperty));
    if (hasProperty) {
        napi_get_named_property(env, contentResult, "text", &basicContentResult);
        NAPI_CALL(env, napi_typeof(env, basicContentResult, &valuetype));
        NAPI_ASSERT(env, valuetype == napi_string, "Wrong argument type. String expected.");
        NAPI_CALL(env, napi_get_value_string_utf8(env, basicContentResult, str, STR_MAX_SIZE - 1, &strLen));
        normalContent->SetText(str);
        ANS_LOGI("normal::text = %{public}s", str);
    } else {
        ANS_LOGE("Property text expected.");
        return nullptr;
    }

    // additionalText?: string
    NAPI_CALL(env, napi_has_named_property(env, contentResult, "additionalText", &hasProperty));
    if (hasProperty) {
        napi_get_named_property(env, contentResult, "additionalText", &basicContentResult);
        NAPI_CALL(env, napi_typeof(env, basicContentResult, &valuetype));
        NAPI_ASSERT(env, valuetype == napi_string, "Wrong argument type. String expected.");
        NAPI_CALL(env, napi_get_value_string_utf8(env, basicContentResult, str, STR_MAX_SIZE - 1, &strLen));
        normalContent->SetAdditionalText(str);
        ANS_LOGI("normal::additionalText = %{public}s", str);
    }

    request.SetContent(std::make_shared<NotificationContent>(normalContent));

    return Common::NapiGetNull(env);
}

napi_value GetNotificationLongTextContent(const napi_env &env, const napi_value &result, NotificationRequest &request)
{
    ANS_LOGI("enter");

    napi_valuetype valuetype = napi_undefined;
    napi_value contentResult = nullptr;
    napi_value longContentResult = nullptr;
    napi_value basicContentResult = nullptr;
    bool hasProperty = false;
    char str[STR_MAX_SIZE] = {0};
    char long_str[LONG_STR_MAX_SIZE + 1] = {0};
    size_t strLen = 0;

    NAPI_CALL(env, napi_has_named_property(env, result, "longText", &hasProperty));
    NAPI_ASSERT(env, hasProperty, "Property longText expected.");

    napi_get_named_property(env, result, "longText", &contentResult);
    NAPI_CALL(env, napi_typeof(env, contentResult, &valuetype));
    NAPI_ASSERT(env, valuetype == napi_object, "Wrong argument type. Object expected.");

    std::shared_ptr<OHOS::Notification::NotificationLongTextContent> longContent =
        std::make_shared<OHOS::Notification::NotificationLongTextContent>();
    if (longContent == nullptr) {
        ANS_LOGE("longContent is null");
        return nullptr;
    }

    // title: string
    NAPI_CALL(env, napi_has_named_property(env, contentResult, "title", &hasProperty));
    if (hasProperty) {
        napi_get_named_property(env, contentResult, "title", &basicContentResult);
        NAPI_CALL(env, napi_typeof(env, basicContentResult, &valuetype));
        NAPI_ASSERT(env, valuetype == napi_string, "Wrong argument type. String expected.");
        NAPI_CALL(env, napi_get_value_string_utf8(env, basicContentResult, str, STR_MAX_SIZE - 1, &strLen));
        longContent->SetTitle(str);
        ANS_LOGI("longText::title = %{public}s", str);
    } else {
        ANS_LOGE("Property title expected.");
        return nullptr;
    }

    // text: string
    NAPI_CALL(env, napi_has_named_property(env, contentResult, "text", &hasProperty));
    if (hasProperty) {
        napi_get_named_property(env, contentResult, "text", &basicContentResult);
        NAPI_CALL(env, napi_typeof(env, basicContentResult, &valuetype));
        NAPI_ASSERT(env, valuetype == napi_string, "Wrong argument type. String expected.");
        NAPI_CALL(env, napi_get_value_string_utf8(env, basicContentResult, str, STR_MAX_SIZE - 1, &strLen));
        longContent->SetText(str);
        ANS_LOGI("longText::text = %{public}s", str);
    } else {
        ANS_LOGE("Property text expected.");
        return nullptr;
    }

    // additionalText?: string
    NAPI_CALL(env, napi_has_named_property(env, contentResult, "additionalText", &hasProperty));
    if (hasProperty) {
        napi_get_named_property(env, contentResult, "additionalText", &basicContentResult);
        NAPI_CALL(env, napi_typeof(env, basicContentResult, &valuetype));
        NAPI_ASSERT(env, valuetype == napi_string, "Wrong argument type. String expected.");
        NAPI_CALL(env, napi_get_value_string_utf8(env, basicContentResult, str, STR_MAX_SIZE - 1, &strLen));
        longContent->SetAdditionalText(str);
        ANS_LOGI("longText::additionalText = %{public}s", str);
    }

    // longText: string
    NAPI_CALL(env, napi_has_named_property(env, contentResult, "longText", &hasProperty));
    if (hasProperty) {
        napi_get_named_property(env, contentResult, "longText", &longContentResult);
        NAPI_CALL(env, napi_typeof(env, longContentResult, &valuetype));
        NAPI_ASSERT(env, valuetype == napi_string, "Wrong argument type. String expected.");
        NAPI_CALL(env, napi_get_value_string_utf8(env, longContentResult, long_str, LONG_STR_MAX_SIZE, &strLen));
        longContent->SetLongText(long_str);
        ANS_LOGI("longText::longText = %{public}s", long_str);
    } else {
        ANS_LOGE("Property longText expected.");
        return nullptr;
    }

    // briefText: string
    NAPI_CALL(env, napi_has_named_property(env, contentResult, "briefText", &hasProperty));
    if (hasProperty) {
        napi_get_named_property(env, contentResult, "briefText", &longContentResult);
        NAPI_CALL(env, napi_typeof(env, longContentResult, &valuetype));
        NAPI_ASSERT(env, valuetype == napi_string, "Wrong argument type. String expected.");
        NAPI_CALL(env, napi_get_value_string_utf8(env, longContentResult, str, STR_MAX_SIZE - 1, &strLen));
        longContent->SetBriefText(str);
        ANS_LOGI("longText::briefText = %{public}s", str);
    } else {
        ANS_LOGE("Property briefText expected.");
        return nullptr;
    }

    // expandedTitle: string
    NAPI_CALL(env, napi_has_named_property(env, contentResult, "expandedTitle", &hasProperty));
    if (hasProperty) {
        napi_get_named_property(env, contentResult, "expandedTitle", &longContentResult);
        NAPI_CALL(env, napi_typeof(env, longContentResult, &valuetype));
        NAPI_ASSERT(env, valuetype == napi_string, "Wrong argument type. String expected.");
        NAPI_CALL(env, napi_get_value_string_utf8(env, longContentResult, str, STR_MAX_SIZE - 1, &strLen));
        longContent->SetExpandedTitle(str);
        ANS_LOGI("longText::expandedTitle = %{public}s", str);
    } else {
        ANS_LOGE("Property expandedTitle expected.");
        return nullptr;
    }

    request.SetContent(std::make_shared<NotificationContent>(longContent));

    return Common::NapiGetNull(env);
}

napi_value GetNotificationPictureContent(const napi_env &env, const napi_value &result, NotificationRequest &request)
{
    ANS_LOGI("enter");

    napi_valuetype valuetype = napi_undefined;
    napi_value contentResult = nullptr;
    napi_value pictureContentResult = nullptr;
    napi_value basicContentResult = nullptr;
    bool hasProperty = false;
    char str[STR_MAX_SIZE] = {0};
    size_t strLen = 0;

    NAPI_CALL(env, napi_has_named_property(env, result, "picture", &hasProperty));
    NAPI_ASSERT(env, hasProperty, "Property picture expected.");
    napi_get_named_property(env, result, "picture", &contentResult);
    NAPI_CALL(env, napi_typeof(env, contentResult, &valuetype));
    NAPI_ASSERT(env, valuetype == napi_object, "Wrong argument type. Object expected.");

    std::shared_ptr<OHOS::Notification::NotificationPictureContent> pictureContent =
        std::make_shared<OHOS::Notification::NotificationPictureContent>();
    if (pictureContent == nullptr) {
        ANS_LOGE("pictureContent is null");
        return nullptr;
    }

    // title: string
    NAPI_CALL(env, napi_has_named_property(env, contentResult, "title", &hasProperty));
    if (hasProperty) {
        napi_get_named_property(env, contentResult, "title", &basicContentResult);
        NAPI_CALL(env, napi_typeof(env, basicContentResult, &valuetype));
        NAPI_ASSERT(env, valuetype == napi_string, "Wrong argument type. String expected.");
        NAPI_CALL(env, napi_get_value_string_utf8(env, basicContentResult, str, STR_MAX_SIZE - 1, &strLen));
        pictureContent->SetTitle(str);
        ANS_LOGI("pictureText::title = %{public}s", str);
    } else {
        ANS_LOGE("Property title expected.");
        return nullptr;
    }

    // text: string
    NAPI_CALL(env, napi_has_named_property(env, contentResult, "text", &hasProperty));
    if (hasProperty) {
        napi_get_named_property(env, contentResult, "text", &basicContentResult);
        NAPI_CALL(env, napi_typeof(env, basicContentResult, &valuetype));
        NAPI_ASSERT(env, valuetype == napi_string, "Wrong argument type. String expected.");
        NAPI_CALL(env, napi_get_value_string_utf8(env, basicContentResult, str, STR_MAX_SIZE - 1, &strLen));
        pictureContent->SetText(str);
        ANS_LOGI("pictureText::text = %{public}s", str);
    } else {
        ANS_LOGE("Property text expected.");
        return nullptr;
    }

    // additionalText?: string
    NAPI_CALL(env, napi_has_named_property(env, contentResult, "additionalText", &hasProperty));
    if (hasProperty) {
        napi_get_named_property(env, contentResult, "additionalText", &basicContentResult);
        NAPI_CALL(env, napi_typeof(env, basicContentResult, &valuetype));
        NAPI_ASSERT(env, valuetype == napi_string, "Wrong argument type. String expected.");
        NAPI_CALL(env, napi_get_value_string_utf8(env, basicContentResult, str, STR_MAX_SIZE - 1, &strLen));
        pictureContent->SetAdditionalText(str);
        ANS_LOGI("pictureText::additionalText = %{public}s", str);
    }

    // briefText: string
    NAPI_CALL(env, napi_has_named_property(env, contentResult, "briefText", &hasProperty));
    if (hasProperty) {
        napi_get_named_property(env, contentResult, "briefText", &pictureContentResult);
        NAPI_CALL(env, napi_typeof(env, pictureContentResult, &valuetype));
        NAPI_ASSERT(env, valuetype == napi_string, "Wrong argument type. String expected.");
        NAPI_CALL(env, napi_get_value_string_utf8(env, pictureContentResult, str, STR_MAX_SIZE - 1, &strLen));
        pictureContent->SetBriefText(str);
        ANS_LOGI("pictureText::briefText = %{public}s", str);
    } else {
        ANS_LOGE("Property briefText expected.");
        return nullptr;
    }

    // expandedTitle: string
    NAPI_CALL(env, napi_has_named_property(env, contentResult, "expandedTitle", &hasProperty));
    if (hasProperty) {
        napi_get_named_property(env, contentResult, "expandedTitle", &pictureContentResult);
        NAPI_CALL(env, napi_typeof(env, pictureContentResult, &valuetype));
        NAPI_ASSERT(env, valuetype == napi_string, "Wrong argument type. String expected.");
        NAPI_CALL(env, napi_get_value_string_utf8(env, pictureContentResult, str, STR_MAX_SIZE - 1, &strLen));
        pictureContent->SetExpandedTitle(str);
        ANS_LOGI("pictureText::expandedTitle = %{public}s", str);
    } else {
        ANS_LOGE("Property expandedTitle expected.");
        return nullptr;
    }

    // picture: image.PixelMap
    NAPI_CALL(env, napi_has_named_property(env, contentResult, "picture", &hasProperty));
    if (hasProperty) {
        napi_get_named_property(env, contentResult, "picture", &pictureContentResult);
        NAPI_CALL(env, napi_typeof(env, pictureContentResult, &valuetype));
        NAPI_ASSERT(env, valuetype == napi_object, "Wrong argument type. Object expected.");
        std::shared_ptr<Media::PixelMap> pixelMap = nullptr;
        pixelMap = Media::PixelMapNapi::GetPixelMap(env, pictureContentResult);
        if (pixelMap == nullptr) {
            ANS_LOGE("Invalid object pixelMap");
            return nullptr;
        }
        pictureContent->SetBigPicture(pixelMap);
    } else {
        ANS_LOGE("Property picture expected.");
        return nullptr;
    }
    request.SetContent(std::make_shared<NotificationContent>(pictureContent));

    return Common::NapiGetNull(env);
}

napi_value GetMessageUser(const napi_env &env, const napi_value &result, MessageUser &messageUser)
{
    ANS_LOGI("enter");
    napi_valuetype valuetype = napi_undefined;
    bool hasProperty = false;
    char str[STR_MAX_SIZE] = {0};
    size_t strLen = 0;

    // name: string
    NAPI_CALL(env, napi_has_named_property(env, result, "name", &hasProperty));
    if (hasProperty) {
        napi_value nameResult = nullptr;
        napi_get_named_property(env, result, "name", &nameResult);
        NAPI_CALL(env, napi_typeof(env, nameResult, &valuetype));
        NAPI_ASSERT(env, valuetype == napi_string, "Wrong argument type. String expected.");
        NAPI_CALL(env, napi_get_value_string_utf8(env, nameResult, str, STR_MAX_SIZE - 1, &strLen));
        messageUser.SetName(str);
        ANS_LOGI("MessageUser::name = %{public}s", str);
    } else {
        ANS_LOGE("Property name expected.");
        return nullptr;
    }

    // uri: string
    NAPI_CALL(env, napi_has_named_property(env, result, "uri", &hasProperty));
    if (hasProperty) {
        napi_value uriResult = nullptr;
        napi_get_named_property(env, result, "uri", &uriResult);
        NAPI_CALL(env, napi_typeof(env, uriResult, &valuetype));
        NAPI_ASSERT(env, valuetype == napi_string, "Wrong argument type. String expected.");
        NAPI_CALL(env, napi_get_value_string_utf8(env, uriResult, str, STR_MAX_SIZE - 1, &strLen));
        Uri uri(str);
        messageUser.SetUri(uri);
        ANS_LOGI("MessageUser::uri = %{public}s", str);
    } else {
        ANS_LOGE("Property uri expected.");
        return nullptr;
    }

    // important: boolean
    NAPI_CALL(env, napi_has_named_property(env, result, "important", &hasProperty));
    if (hasProperty) {
        napi_value importantResult = nullptr;
        napi_get_named_property(env, result, "important", &importantResult);
        NAPI_CALL(env, napi_typeof(env, importantResult, &valuetype));
        NAPI_ASSERT(env, valuetype == napi_boolean, "Wrong argument type. Bool expected.");
        bool important = false;
        napi_get_value_bool(env, importantResult, &important);
        messageUser.SetUserAsImportant(important);
        ANS_LOGI("MessageUser::important = %{public}d", important);
    } else {
        ANS_LOGE("Property important expected.");
        return nullptr;
    }

    // machine: boolean
    NAPI_CALL(env, napi_has_named_property(env, result, "machine", &hasProperty));
    if (hasProperty) {
        napi_value machineResult = nullptr;
        napi_get_named_property(env, result, "machine", &machineResult);
        NAPI_CALL(env, napi_typeof(env, machineResult, &valuetype));
        NAPI_ASSERT(env, valuetype == napi_boolean, "Wrong argument type. Bool expected.");
        bool machine = false;
        napi_get_value_bool(env, machineResult, &machine);
        messageUser.SetMachine(machine);
        ANS_LOGI("MessageUser::machine = %{public}d", machine);
    } else {
        ANS_LOGE("Property machine expected.");
        return nullptr;
    }

    // key: string
    NAPI_CALL(env, napi_has_named_property(env, result, "key", &hasProperty));
    if (hasProperty) {
        napi_value keyResult = nullptr;
        napi_get_named_property(env, result, "key", &keyResult);
        NAPI_CALL(env, napi_typeof(env, keyResult, &valuetype));
        NAPI_ASSERT(env, valuetype == napi_string, "Wrong argument type. String expected.");
        NAPI_CALL(env, napi_get_value_string_utf8(env, keyResult, str, STR_MAX_SIZE - 1, &strLen));
        messageUser.SetKey(str);
        ANS_LOGI("MessageUser::key = %{public}s", str);
    } else {
        ANS_LOGE("Property key expected.");
        return nullptr;
    }

    return Common::NapiGetNull(env);
}

napi_value GetConversationalMessage(const napi_env &env, const napi_value &conversationalMessage,
    std::shared_ptr<NotificationConversationalMessage> &message)
{
    ANS_LOGI("enter");
    napi_valuetype valuetype = napi_undefined;
    bool hasProperty = false;
    char str[STR_MAX_SIZE] = {0};
    size_t strLen = 0;
    std::string text;
    int64_t timestamp = 0;
    std::string mimeType;
    std::string uri;
    MessageUser sender;

    // text: string
    NAPI_CALL(env, napi_has_named_property(env, conversationalMessage, "text", &hasProperty));
    if (hasProperty) {
        napi_value textResult = nullptr;
        napi_get_named_property(env, conversationalMessage, "text", &textResult);
        NAPI_CALL(env, napi_typeof(env, textResult, &valuetype));
        NAPI_ASSERT(env, valuetype == napi_string, "Wrong argument type. String expected.");
        NAPI_CALL(env, napi_get_value_string_utf8(env, textResult, str, STR_MAX_SIZE - 1, &strLen));
        text = str;
        ANS_LOGI("conversationalMessage::text = %{public}s", str);
    } else {
        ANS_LOGE("Property text expected.");
    }

    // timestamp: number
    NAPI_CALL(env, napi_has_named_property(env, conversationalMessage, "timestamp", &hasProperty));
    if (hasProperty) {
        napi_value timestampResult = nullptr;
        napi_get_named_property(env, conversationalMessage, "timestamp", &timestampResult);
        NAPI_CALL(env, napi_typeof(env, timestampResult, &valuetype));
        NAPI_ASSERT(env, valuetype == napi_number, "Wrong argument type. Number expected.");
        napi_get_value_int64(env, timestampResult, &timestamp);
        ANS_LOGI("conversationalMessage::timestamp = %{public}" PRId64, timestamp);
    } else {
        ANS_LOGE("Property timestamp expected.");
    }

    // mimeType: string
    NAPI_CALL(env, napi_has_named_property(env, conversationalMessage, "mimeType", &hasProperty));
    if (hasProperty) {
        napi_value mimeTypeResult = nullptr;
        napi_get_named_property(env, conversationalMessage, "mimeType", &mimeTypeResult);
        NAPI_CALL(env, napi_typeof(env, mimeTypeResult, &valuetype));
        NAPI_ASSERT(env, valuetype == napi_string, "Wrong argument type. String expected.");
        NAPI_CALL(env, napi_get_value_string_utf8(env, mimeTypeResult, str, STR_MAX_SIZE - 1, &strLen));
        mimeType = str;
        ANS_LOGI("conversationalMessage::mimeType = %{public}s", str);
    } else {
        ANS_LOGE("Property mimeType expected.");
    }

    // uri: string
    NAPI_CALL(env, napi_has_named_property(env, conversationalMessage, "uri", &hasProperty));
    if (hasProperty) {
        napi_value uriResult = nullptr;
        napi_get_named_property(env, conversationalMessage, "uri", &uriResult);
        NAPI_CALL(env, napi_typeof(env, uriResult, &valuetype));
        NAPI_ASSERT(env, valuetype == napi_string, "Wrong argument type. String expected.");
        NAPI_CALL(env, napi_get_value_string_utf8(env, uriResult, str, STR_MAX_SIZE - 1, &strLen));
        uri = str;
        ANS_LOGI("conversationalMessage::uri = %{public}s", str);
    } else {
        ANS_LOGE("Property uri expected.");
    }

    // sender: MessageUser
    NAPI_CALL(env, napi_has_named_property(env, conversationalMessage, "sender", &hasProperty));
    if (hasProperty) {
        napi_value senderResult = nullptr;
        napi_get_named_property(env, conversationalMessage, "sender", &senderResult);
        NAPI_CALL(env, napi_typeof(env, senderResult, &valuetype));
        NAPI_ASSERT(env, valuetype == napi_object, "Wrong argument type. Object expected.");
        if (!GetMessageUser(env, senderResult, sender)) {
            return nullptr;
        }
    } else {
        ANS_LOGE("Property sender expected.");
    }

    message = std::make_shared<NotificationConversationalMessage>(text, timestamp, sender);
    if (message == nullptr) {
        ANS_LOGE("Failed to create NotificationConversationalMessage object.");
        return nullptr;
    }
    std::shared_ptr<Uri> uriPtr = std::make_shared<Uri>(uri);
    message->SetData(mimeType, uriPtr);

    return Common::NapiGetNull(env);
}

napi_value GetNotificationConversationalContent(
    const napi_env &env, const napi_value &result, NotificationRequest &request)
{
    ANS_LOGI("enter");

    napi_valuetype valuetype = napi_undefined;
    napi_value contentResult = nullptr;
    napi_value conversationalContentResult = nullptr;
    napi_value basicContentResult = nullptr;
    bool hasProperty = false;
    char str[STR_MAX_SIZE] = {0};
    size_t strLen = 0;
    MessageUser user;

    NAPI_CALL(env, napi_has_named_property(env, result, "conversational", &hasProperty));
    NAPI_ASSERT(env, hasProperty, "Property conversational expected.");
    napi_get_named_property(env, result, "conversational", &contentResult);
    NAPI_CALL(env, napi_typeof(env, contentResult, &valuetype));
    NAPI_ASSERT(env, valuetype == napi_object, "Wrong argument type. Object expected.");

    // user: MessageUser
    NAPI_CALL(env, napi_has_named_property(env, contentResult, "user", &hasProperty));
    if (hasProperty) {
        napi_value userResult = nullptr;
        napi_get_named_property(env, contentResult, "user", &userResult);
        NAPI_CALL(env, napi_typeof(env, userResult, &valuetype));
        NAPI_ASSERT(env, valuetype == napi_object, "Wrong argument type. Object expected.");
        if (!GetMessageUser(env, userResult, user)) {
            return nullptr;
        }
    } else {
        ANS_LOGE("Property user expected.");
        return nullptr;
    }

    std::shared_ptr<OHOS::Notification::NotificationConversationalContent> conversationalContent =
        std::make_shared<OHOS::Notification::NotificationConversationalContent>(user);
    if (conversationalContent == nullptr) {
        ANS_LOGE("conversationalContent is null");
        return nullptr;
    }

    // title: string
    NAPI_CALL(env, napi_has_named_property(env, contentResult, "title", &hasProperty));
    if (hasProperty) {
        napi_get_named_property(env, contentResult, "title", &basicContentResult);
        NAPI_CALL(env, napi_typeof(env, basicContentResult, &valuetype));
        NAPI_ASSERT(env, valuetype == napi_string, "Wrong argument type. String expected.");
        NAPI_CALL(env, napi_get_value_string_utf8(env, basicContentResult, str, STR_MAX_SIZE - 1, &strLen));
        conversationalContent->SetTitle(str);
        ANS_LOGI("conversationalText::title = %{public}s", str);
    } else {
        ANS_LOGE("Property title expected.");
        return nullptr;
    }

    // text: string
    NAPI_CALL(env, napi_has_named_property(env, contentResult, "text", &hasProperty));
    if (hasProperty) {
        napi_get_named_property(env, contentResult, "text", &basicContentResult);
        NAPI_CALL(env, napi_typeof(env, basicContentResult, &valuetype));
        NAPI_ASSERT(env, valuetype == napi_string, "Wrong argument type. String expected.");
        NAPI_CALL(env, napi_get_value_string_utf8(env, basicContentResult, str, STR_MAX_SIZE - 1, &strLen));
        conversationalContent->SetText(str);
        ANS_LOGI("conversationalText::text = %{public}s", str);
    } else {
        ANS_LOGE("Property text expected.");
        return nullptr;
    }

    // additionalText?: string
    NAPI_CALL(env, napi_has_named_property(env, contentResult, "additionalText", &hasProperty));
    if (hasProperty) {
        napi_get_named_property(env, contentResult, "additionalText", &basicContentResult);
        NAPI_CALL(env, napi_typeof(env, basicContentResult, &valuetype));
        NAPI_ASSERT(env, valuetype == napi_string, "Wrong argument type. String expected.");
        NAPI_CALL(env, napi_get_value_string_utf8(env, basicContentResult, str, STR_MAX_SIZE - 1, &strLen));
        conversationalContent->SetAdditionalText(str);
        ANS_LOGI("conversationalText::additionalText = %{public}s", str);
    }

    // conversationTitle: string
    NAPI_CALL(env, napi_has_named_property(env, contentResult, "conversationTitle", &hasProperty));
    if (hasProperty) {
        napi_get_named_property(env, contentResult, "conversationTitle", &conversationalContentResult);
        NAPI_CALL(env, napi_typeof(env, conversationalContentResult, &valuetype));
        NAPI_ASSERT(env, valuetype == napi_string, "Wrong argument type. String expected.");
        NAPI_CALL(env, napi_get_value_string_utf8(env, conversationalContentResult, str, STR_MAX_SIZE - 1, &strLen));
        conversationalContent->SetConversationTitle(str);
        ANS_LOGI("conversationalText::conversationTitle = %{public}s", str);
    } else {
        ANS_LOGE("Property conversationTitle expected.");
        return nullptr;
    }

    // conversationGroup: boolean
    NAPI_CALL(env, napi_has_named_property(env, contentResult, "conversationGroup", &hasProperty));
    if (hasProperty) {
        napi_get_named_property(env, contentResult, "conversationGroup", &conversationalContentResult);
        NAPI_CALL(env, napi_typeof(env, conversationalContentResult, &valuetype));
        NAPI_ASSERT(env, valuetype == napi_boolean, "Wrong argument type. Bool expected.");
        bool conversationGroup = false;
        napi_get_value_bool(env, conversationalContentResult, &conversationGroup);
        conversationalContent->SetConversationGroup(conversationGroup);
        ANS_LOGI("conversationalText::conversationGroup = %{public}d", conversationGroup);
    } else {
        ANS_LOGE("Property conversationGroup expected.");
        return nullptr;
    }

    // messages: Array<ConversationalMessage>
    NAPI_CALL(env, napi_has_named_property(env, contentResult, "messages", &hasProperty));
    if (hasProperty) {
        napi_get_named_property(env, contentResult, "messages", &conversationalContentResult);
        bool isArray = false;
        napi_is_array(env, conversationalContentResult, &isArray);
        NAPI_ASSERT(env, isArray, "Property messages is expected to be an array.");
        uint32_t length = 0;
        napi_get_array_length(env, conversationalContentResult, &length);
        NAPI_ASSERT(env, length > 0, "The array is empty.");
        for (size_t i = 0; i < length; i++) {
            napi_value conversationalMessage = nullptr;
            napi_get_element(env, conversationalContentResult, i, &conversationalMessage);
            NAPI_CALL(env, napi_typeof(env, conversationalMessage, &valuetype));
            NAPI_ASSERT(env, valuetype == napi_object, "Wrong argument type. Object expected.");
            std::shared_ptr<NotificationConversationalMessage> message = nullptr;
            if (!GetConversationalMessage(env, conversationalMessage, message)) {
                return nullptr;
            }
            conversationalContent->AddConversationalMessage(message);
        }
    } else {
        ANS_LOGE("Property messages expected.");
        return nullptr;
    }

    request.SetContent(std::make_shared<NotificationContent>(conversationalContent));

    return Common::NapiGetNull(env);
}

napi_value GetNotificationMultiLineContent(const napi_env &env, const napi_value &result, NotificationRequest &request)
{
    ANS_LOGI("enter");

    napi_valuetype valuetype = napi_undefined;
    napi_value contentResult = nullptr;
    napi_value multiLineContentResult = nullptr;
    napi_value basicContentResult = nullptr;
    bool hasProperty = false;
    char str[STR_MAX_SIZE] = {0};
    size_t strLen = 0;

    NAPI_CALL(env, napi_has_named_property(env, result, "multiLine", &hasProperty));
    NAPI_ASSERT(env, hasProperty, "Property multiLine expected.");
    napi_get_named_property(env, result, "multiLine", &contentResult);
    NAPI_CALL(env, napi_typeof(env, contentResult, &valuetype));
    NAPI_ASSERT(env, valuetype == napi_object, "Wrong argument type. Object expected.");

    std::shared_ptr<OHOS::Notification::NotificationMultiLineContent> multiLineContent =
        std::make_shared<OHOS::Notification::NotificationMultiLineContent>();
    if (multiLineContent == nullptr) {
        ANS_LOGE("multiLineContent is null");
        return nullptr;
    }

    // title: string
    NAPI_CALL(env, napi_has_named_property(env, contentResult, "title", &hasProperty));
    if (hasProperty) {
        napi_get_named_property(env, contentResult, "title", &basicContentResult);
        NAPI_CALL(env, napi_typeof(env, basicContentResult, &valuetype));
        NAPI_ASSERT(env, valuetype == napi_string, "Wrong argument type. String expected.");
        NAPI_CALL(env, napi_get_value_string_utf8(env, basicContentResult, str, STR_MAX_SIZE - 1, &strLen));
        multiLineContent->SetTitle(str);
        ANS_LOGI("multiLine::title = %{public}s", str);
    } else {
        ANS_LOGE("Property title expected.");
        return nullptr;
    }

    // text: string
    NAPI_CALL(env, napi_has_named_property(env, contentResult, "text", &hasProperty));
    if (hasProperty) {
        napi_get_named_property(env, contentResult, "text", &basicContentResult);
        NAPI_CALL(env, napi_typeof(env, basicContentResult, &valuetype));
        NAPI_ASSERT(env, valuetype == napi_string, "Wrong argument type. String expected.");
        NAPI_CALL(env, napi_get_value_string_utf8(env, basicContentResult, str, STR_MAX_SIZE - 1, &strLen));
        multiLineContent->SetText(str);
        ANS_LOGI("multiLine::text = %{public}s", str);
    } else {
        ANS_LOGE("Property text expected.");
        return nullptr;
    }

    // additionalText?: string
    NAPI_CALL(env, napi_has_named_property(env, contentResult, "additionalText", &hasProperty));
    if (hasProperty) {
        napi_get_named_property(env, contentResult, "additionalText", &basicContentResult);
        NAPI_CALL(env, napi_typeof(env, basicContentResult, &valuetype));
        NAPI_ASSERT(env, valuetype == napi_string, "Wrong argument type. String expected.");
        NAPI_CALL(env, napi_get_value_string_utf8(env, basicContentResult, str, STR_MAX_SIZE - 1, &strLen));
        multiLineContent->SetAdditionalText(str);
        ANS_LOGI("multiLine::additionalText = %{public}s", str);
    }

    // briefText: string
    NAPI_CALL(env, napi_has_named_property(env, contentResult, "briefText", &hasProperty));
    if (hasProperty) {
        napi_get_named_property(env, contentResult, "briefText", &multiLineContentResult);
        NAPI_CALL(env, napi_typeof(env, multiLineContentResult, &valuetype));
        NAPI_ASSERT(env, valuetype == napi_string, "Wrong argument type. String expected.");
        NAPI_CALL(env, napi_get_value_string_utf8(env, multiLineContentResult, str, STR_MAX_SIZE - 1, &strLen));
        multiLineContent->SetBriefText(str);
        ANS_LOGI("multiLine: briefText = %{public}s", str);
    } else {
        ANS_LOGE("Property briefText expected.");
        return nullptr;
    }

    // longTitle: string
    NAPI_CALL(env, napi_has_named_property(env, contentResult, "longTitle", &hasProperty));
    if (hasProperty) {
        napi_get_named_property(env, contentResult, "longTitle", &multiLineContentResult);
        NAPI_CALL(env, napi_typeof(env, multiLineContentResult, &valuetype));
        NAPI_ASSERT(env, valuetype == napi_string, "Wrong argument type. String expected.");
        NAPI_CALL(env, napi_get_value_string_utf8(env, multiLineContentResult, str, STR_MAX_SIZE - 1, &strLen));
        multiLineContent->SetExpandedTitle(str);
        ANS_LOGI("multiLine: longTitle = %{public}s", str);
    } else {
        ANS_LOGE("Property longTitle expected.");
        return nullptr;
    }

    // lines: Array<String>
    NAPI_CALL(env, napi_has_named_property(env, contentResult, "lines", &hasProperty));
    if (hasProperty) {
        if (GetNotificationMultiLineContentLines(env, contentResult, multiLineContent) == nullptr) {
            return nullptr;
        }
    } else {
        ANS_LOGE("Property lines expected.");
        return nullptr;
    }

    request.SetContent(std::make_shared<NotificationContent>(multiLineContent));

    return Common::NapiGetNull(env);
}

napi_value GetNotificationMultiLineContentLines(const napi_env &env, const napi_value &result,
    std::shared_ptr<OHOS::Notification::NotificationMultiLineContent> &multiLineContent)
{
    ANS_LOGI("enter");

    bool isArray = false;
    napi_valuetype valuetype = napi_undefined;
    napi_value multilines = nullptr;
    char str[STR_MAX_SIZE] = {0};
    size_t strLen = 0;
    uint32_t length = 0;

    napi_get_named_property(env, result, "lines", &multilines);
    napi_is_array(env, multilines, &isArray);
    NAPI_ASSERT(env, isArray, "Property lines is expected to be an array.");

    napi_get_array_length(env, multilines, &length);
    NAPI_ASSERT(env, length > 0, "The array is empty.");
    for (size_t i = 0; i < length; i++) {
        napi_value line = nullptr;
        napi_get_element(env, multilines, i, &line);
        NAPI_CALL(env, napi_typeof(env, line, &valuetype));
        NAPI_ASSERT(env, valuetype == napi_string, "Wrong argument type. String expected.");
        NAPI_CALL(env, napi_get_value_string_utf8(env, line, str, STR_MAX_SIZE - 1, &strLen));
        multiLineContent->AddSingleLine(str);
        ANS_LOGI("multiLine: lines : addSingleLine = %{public}s", str);
    }

    return Common::NapiGetNull(env);
}

napi_value Common::GetNotificationSlot(const napi_env &env, const napi_value &value, NotificationSlot &slot)
{
    ANS_LOGI("enter");
    napi_value nobj = nullptr;
    napi_valuetype valuetype = napi_undefined;
    bool hasProperty = false;
    size_t strLen = 0;
    uint32_t length = 0;

    // type: notification.SlotType
    int slotType = 0;
    NAPI_CALL(env, napi_has_named_property(env, value, "type", &hasProperty));
    NAPI_ASSERT(env, hasProperty, "Property type expected.");
    napi_get_named_property(env, value, "type", &nobj);
    NAPI_CALL(env, napi_typeof(env, nobj, &valuetype));
    NAPI_ASSERT(env, valuetype == napi_number, "Wrong argument type. Number expected.");
    napi_get_value_int32(env, nobj, &slotType);
    enum NotificationConstant::SlotType outType = NotificationConstant::SlotType::OTHER;
    if (!SlotTypeJSToC(SlotType(slotType), outType)) {
        return nullptr;
    }
    slot.SetType(outType);

    // level?: number
    NAPI_CALL(env, napi_has_named_property(env, value, "level", &hasProperty));
    if (hasProperty) {
        int inLevel = 0;
        napi_get_named_property(env, value, "level", &nobj);
        NAPI_CALL(env, napi_typeof(env, nobj, &valuetype));
        NAPI_ASSERT(env, valuetype == napi_number, "Wrong argument type. Number expected.");
        napi_get_value_int32(env, nobj, &inLevel);
        ANS_LOGI("level is: %{public}d", inLevel);

        NotificationSlot::NotificationLevel outLevel {NotificationSlot::NotificationLevel::LEVEL_NONE};
        if (!SlotLevelJSToC(SlotLevel(inLevel), outLevel)) {
            return nullptr;
        }
        slot.SetLevel(outLevel);
    }

    // desc?: string
    NAPI_CALL(env, napi_has_named_property(env, value, "desc", &hasProperty));
    if (hasProperty) {
        std::string desc;
        char str[STR_MAX_SIZE] = {0};
        napi_get_named_property(env, value, "desc", &nobj);
        NAPI_CALL(env, napi_typeof(env, nobj, &valuetype));
        NAPI_ASSERT(env, valuetype == napi_string, "Wrong argument type. String expected.");
        NAPI_CALL(env, napi_get_value_string_utf8(env, nobj, str, STR_MAX_SIZE - 1, &strLen));
        desc = str;
        ANS_LOGI("desc is: %{public}s", desc.c_str());
        slot.SetDescription(desc);
    }

    // badgeFlag?: boolean
    NAPI_CALL(env, napi_has_named_property(env, value, "badgeFlag", &hasProperty));
    if (hasProperty) {
        bool badgeFlag = false;
        napi_get_named_property(env, value, "badgeFlag", &nobj);
        NAPI_CALL(env, napi_typeof(env, nobj, &valuetype));
        NAPI_ASSERT(env, valuetype == napi_boolean, "Wrong argument type. Bool expected.");
        napi_get_value_bool(env, nobj, &badgeFlag);
        ANS_LOGI("badgeFlag is: %{public}d", badgeFlag);
        slot.EnableBadge(badgeFlag);
    }

    // bypassDnd?: boolean
    NAPI_CALL(env, napi_has_named_property(env, value, "bypassDnd", &hasProperty));
    if (hasProperty) {
        bool bypassDnd = false;
        napi_get_named_property(env, value, "bypassDnd", &nobj);
        NAPI_CALL(env, napi_typeof(env, nobj, &valuetype));
        NAPI_ASSERT(env, valuetype == napi_boolean, "Wrong argument type. Bool expected.");
        napi_get_value_bool(env, nobj, &bypassDnd);
        ANS_LOGI("bypassDnd is: %{public}d", bypassDnd);
        slot.EnableBypassDnd(bypassDnd);
    }

    // lockscreenVisibility?: number
    NAPI_CALL(env, napi_has_named_property(env, value, "lockscreenVisibility", &hasProperty));
    if (hasProperty) {
        int lockscreenVisibility = 0;
        napi_get_named_property(env, value, "lockscreenVisibility", &nobj);
        NAPI_CALL(env, napi_typeof(env, nobj, &valuetype));
        NAPI_ASSERT(env, valuetype == napi_number, "Wrong argument type. Number expected.");
        napi_get_value_int32(env, nobj, &lockscreenVisibility);
        ANS_LOGI("lockscreenVisibility is: %{public}d", lockscreenVisibility);
        slot.SetLockscreenVisibleness(NotificationConstant::VisiblenessType(lockscreenVisibility));
    }

    // vibrationEnabled?: boolean
    NAPI_CALL(env, napi_has_named_property(env, value, "vibrationEnabled", &hasProperty));
    if (hasProperty) {
        bool vibrationEnabled = false;
        napi_get_named_property(env, value, "vibrationEnabled", &nobj);
        NAPI_CALL(env, napi_typeof(env, nobj, &valuetype));
        NAPI_ASSERT(env, valuetype == napi_boolean, "Wrong argument type. Bool expected.");
        napi_get_value_bool(env, nobj, &vibrationEnabled);
        ANS_LOGI("vibrationEnabled is: %{public}d", vibrationEnabled);
        slot.SetEnableVibration(vibrationEnabled);
    }

    // sound?: string
    NAPI_CALL(env, napi_has_named_property(env, value, "sound", &hasProperty));
    if (hasProperty) {
        std::string sound;
        char str[STR_MAX_SIZE] = {0};
        napi_get_named_property(env, value, "sound", &nobj);
        NAPI_CALL(env, napi_typeof(env, nobj, &valuetype));
        NAPI_ASSERT(env, valuetype == napi_string, "Wrong argument type. String expected.");
        NAPI_CALL(env, napi_get_value_string_utf8(env, nobj, str, STR_MAX_SIZE - 1, &strLen));
        sound = str;
        ANS_LOGI("sound is: %{public}s", sound.c_str());
        slot.SetSound(Uri(sound));
    }

    // lightEnabled?: boolean
    NAPI_CALL(env, napi_has_named_property(env, value, "lightEnabled", &hasProperty));
    if (hasProperty) {
        bool lightEnabled = false;
        napi_get_named_property(env, value, "lightEnabled", &nobj);
        NAPI_CALL(env, napi_typeof(env, nobj, &valuetype));
        NAPI_ASSERT(env, valuetype == napi_boolean, "Wrong argument type. Bool expected.");
        napi_get_value_bool(env, nobj, &lightEnabled);
        ANS_LOGI("lightEnabled is: %{public}d", lightEnabled);
        slot.SetEnableLight(lightEnabled);
    }

    // lightColor?: number
    NAPI_CALL(env, napi_has_named_property(env, value, "lightColor", &hasProperty));
    if (hasProperty) {
        int lightColor = 0;
        napi_get_named_property(env, value, "lightColor", &nobj);
        NAPI_CALL(env, napi_typeof(env, nobj, &valuetype));
        NAPI_ASSERT(env, valuetype == napi_number, "Wrong argument type. Number expected.");
        napi_get_value_int32(env, nobj, &lightColor);
        ANS_LOGI("lightColor is: %{public}d", lightColor);
        slot.SetLedLightColor(lightColor);
    }

    // vibrationValues?: Array<number>
    NAPI_CALL(env, napi_has_named_property(env, value, "vibrationValues", &hasProperty));
    if (hasProperty) {
        bool isArray = false;
        napi_get_named_property(env, value, "vibrationValues", &nobj);
        napi_is_array(env, nobj, &isArray);
        NAPI_ASSERT(env, isArray, "Property vibrationValues is expected to be an array.");
        napi_get_array_length(env, nobj, &length);
        NAPI_ASSERT(env, length > 0, "The array is empty.");
        std::vector<int64_t> vibrationValues;
        for (size_t i = 0; i < length; i++) {
            napi_value nVibrationValue = nullptr;
            int64_t vibrationValue = 0;
            napi_get_element(env, nobj, i, &nVibrationValue);
            NAPI_CALL(env, napi_typeof(env, nVibrationValue, &valuetype));
            NAPI_ASSERT(env, valuetype == napi_number, "Wrong argument type. Number expected.");
            napi_get_value_int64(env, nVibrationValue, &vibrationValue);
            ANS_LOGI("vibrationValue is: %{public}" PRId64, vibrationValue);
            vibrationValues.emplace_back(vibrationValue);
        }
        slot.SetVibrationStyle(vibrationValues);
    }
    return NapiGetNull(env);
}

napi_value Common::GetBundleOption(const napi_env &env, const napi_value &value, BundleOption &option)
{
    ANS_LOGI("enter");

    bool hasProperty {false};
    napi_valuetype valuetype = napi_undefined;
    napi_value result = nullptr;

    char str[STR_MAX_SIZE] = {0};
    size_t strLen = 0;
    // bundle: string
    NAPI_CALL(env, napi_has_named_property(env, value, "bundle", &hasProperty));
    NAPI_ASSERT(env, hasProperty, "Property bundle expected.");
    napi_get_named_property(env, value, "bundle", &result);
    NAPI_CALL(env, napi_typeof(env, result, &valuetype));
    NAPI_ASSERT(env, valuetype == napi_string, "Wrong argument type. String expected.");
    NAPI_CALL(env, napi_get_value_string_utf8(env, result, str, STR_MAX_SIZE - 1, &strLen));
    option.bundle = str;

    // uid?: number
    NAPI_CALL(env, napi_has_named_property(env, value, "uid", &hasProperty));
    if (hasProperty) {
        napi_get_named_property(env, value, "uid", &result);
        NAPI_CALL(env, napi_typeof(env, result, &valuetype));
        NAPI_ASSERT(env, valuetype == napi_number, "Wrong argument type. Number expected.");
        napi_get_value_int32(env, result, &option.uid);
    }

    return NapiGetNull(env);
}

napi_value Common::GetNotificationKey(const napi_env &env, const napi_value &value, NotificationKey &key)
{
    ANS_LOGI("enter");

    bool hasProperty {false};
    napi_valuetype valuetype = napi_undefined;
    napi_value result = nullptr;

    // id: number
    NAPI_CALL(env, napi_has_named_property(env, value, "id", &hasProperty));
    NAPI_ASSERT(env, hasProperty, "Property id expected.");
    napi_get_named_property(env, value, "id", &result);
    NAPI_CALL(env, napi_typeof(env, result, &valuetype));
    NAPI_ASSERT(env, valuetype == napi_number, "Wrong argument type. Number expected.");
    napi_get_value_int32(env, result, &key.id);

    // label?: string
    NAPI_CALL(env, napi_has_named_property(env, value, "label", &hasProperty));
    if (hasProperty) {
        char str[STR_MAX_SIZE] = {0};
        size_t strLen = 0;
        napi_get_named_property(env, value, "label", &result);
        NAPI_CALL(env, napi_typeof(env, result, &valuetype));
        NAPI_ASSERT(env, valuetype == napi_string, "Wrong argument type. String expected.");
        NAPI_CALL(env, napi_get_value_string_utf8(env, result, str, STR_MAX_SIZE - 1, &strLen));
        key.label = str;
    }

    return NapiGetNull(env);
}

bool Common::ContentTypeJSToC(const enum ContentType &inType, enum NotificationContent::Type &outType)
{
    switch (inType) {
        case ContentType::NOTIFICATION_CONTENT_BASIC_TEXT:
            outType = NotificationContent::Type::BASIC_TEXT;
            break;
        case ContentType::NOTIFICATION_CONTENT_LONG_TEXT:
            outType = NotificationContent::Type::LONG_TEXT;
            break;
        case ContentType::NOTIFICATION_CONTENT_MULTILINE:
            outType = NotificationContent::Type::MULTILINE;
            break;
        case ContentType::NOTIFICATION_CONTENT_PICTURE:
            outType = NotificationContent::Type::PICTURE;
            break;
        default:
            ANS_LOGE("ContentType %{public}d is an invalid value", inType);
            return false;
    }
    return true;
}

bool Common::ContentTypeCToJS(const enum NotificationContent::Type &inType, enum ContentType &outType)
{
    switch (inType) {
        case NotificationContent::Type::BASIC_TEXT:
            outType = ContentType::NOTIFICATION_CONTENT_BASIC_TEXT;
            break;
        case NotificationContent::Type::LONG_TEXT:
            outType = ContentType::NOTIFICATION_CONTENT_LONG_TEXT;
            break;
        case NotificationContent::Type::MULTILINE:
            outType = ContentType::NOTIFICATION_CONTENT_MULTILINE;
            break;
        case NotificationContent::Type::PICTURE:
            outType = ContentType::NOTIFICATION_CONTENT_PICTURE;
            break;
        default:
            ANS_LOGE("ContentType %{public}d is an invalid value", inType);
            return false;
    }
    return true;
}

bool Common::SlotTypeJSToC(const enum SlotType &inType, enum NotificationConstant::SlotType &outType)
{
    switch (inType) {
        case SlotType::SOCIAL_COMMUNICATION:
            outType = NotificationConstant::SlotType::SOCIAL_COMMUNICATION;
            break;
        case SlotType::SERVICE_INFORMATION:
            outType = NotificationConstant::SlotType::SERVICE_REMINDER;
            break;
        case SlotType::CONTENT_INFORMATION:
            outType = NotificationConstant::SlotType::CONTENT_INFORMATION;
            break;
        case SlotType::UNKNOWN_TYPE:
        case SlotType::OTHER_TYPES:
            outType = NotificationConstant::SlotType::OTHER;
            break;
        default:
            ANS_LOGE("SlotType %{public}d is an invalid value", inType);
            return false;
    }
    return true;
}

bool Common::SlotTypeCToJS(const enum NotificationConstant::SlotType &inType, enum SlotType &outType)
{
    switch (inType) {
        case NotificationConstant::SlotType::CUSTOM:
            outType = SlotType::UNKNOWN_TYPE;
            break;
        case NotificationConstant::SlotType::SOCIAL_COMMUNICATION:
            outType = SlotType::SOCIAL_COMMUNICATION;
            break;
        case NotificationConstant::SlotType::SERVICE_REMINDER:
            outType = SlotType::SERVICE_INFORMATION;
            break;
        case NotificationConstant::SlotType::CONTENT_INFORMATION:
            outType = SlotType::CONTENT_INFORMATION;
            break;
        case NotificationConstant::SlotType::OTHER:
            outType = SlotType::OTHER_TYPES;
            break;
        default:
            ANS_LOGE("SlotType %{public}d is an invalid value", inType);
            return false;
    }
    return true;
}

bool Common::SlotLevelJSToC(const SlotLevel &inLevel, NotificationSlot::NotificationLevel &outLevel)
{
    switch (inLevel) {
        case SlotLevel::LEVEL_NONE:
            outLevel = NotificationSlot::NotificationLevel::LEVEL_NONE;
            break;
        case SlotLevel::LEVEL_MIN:
            outLevel = NotificationSlot::NotificationLevel::LEVEL_MIN;
            break;
        case SlotLevel::LEVEL_LOW:
            outLevel = NotificationSlot::NotificationLevel::LEVEL_LOW;
            break;
        case SlotLevel::LEVEL_DEFAULT:
            outLevel = NotificationSlot::NotificationLevel::LEVEL_DEFAULT;
            break;
        case SlotLevel::LEVEL_HIGH:
            outLevel = NotificationSlot::NotificationLevel::LEVEL_HIGH;
            break;
        default:
            ANS_LOGE("SlotLevel %{public}d is an invalid value", inLevel);
            return false;
    }
    return true;
}

bool Common::SlotLevelCToJS(const NotificationSlot::NotificationLevel &inLevel, SlotLevel &outLevel)
{
    switch (inLevel) {
        case NotificationSlot::NotificationLevel::LEVEL_NONE:
        case NotificationSlot::NotificationLevel::LEVEL_UNDEFINED:
            outLevel = SlotLevel::LEVEL_NONE;
            break;
        case NotificationSlot::NotificationLevel::LEVEL_MIN:
            outLevel = SlotLevel::LEVEL_MIN;
            break;
        case NotificationSlot::NotificationLevel::LEVEL_LOW:
            outLevel = SlotLevel::LEVEL_LOW;
            break;
        case NotificationSlot::NotificationLevel::LEVEL_DEFAULT:
            outLevel = SlotLevel::LEVEL_DEFAULT;
            break;
        case NotificationSlot::NotificationLevel::LEVEL_HIGH:
            outLevel = SlotLevel::LEVEL_HIGH;
            break;
        default:
            ANS_LOGE("SlotLevel %{public}d is an invalid value", inLevel);
            return false;
    }
    return true;
}

bool Common::ReasonCToJS(const int &inType, int &outType)
{
    switch (inType) {
        case NotificationConstant::CLICK_REASON_DELETE:
            outType = NotificationReason::CLICK_REASON_DELETE;
            break;
        case NotificationConstant::CANCEL_REASON_DELETE:
            outType = NotificationReason::CANCEL_REASON_DELETE;
            break;
        case NotificationConstant::CANCEL_ALL_REASON_DELETE:
            outType = NotificationReason::CANCEL_ALL_REASON_DELETE;
            break;
        case NotificationConstant::ERROR_REASON_DELETE:
            outType = NotificationReason::ERROR_REASON_DELETE;
            break;
        case NotificationConstant::PACKAGE_CHANGED_REASON_DELETE:
            outType = NotificationReason::PACKAGE_CHANGED_REASON_DELETE;
            break;
        case NotificationConstant::USER_STOPPED_REASON_DELETE:
            outType = NotificationReason::USER_STOPPED_REASON_DELETE;
            break;
        case NotificationConstant::APP_CANCEL_REASON_DELETE:
            outType = NotificationReason::APP_CANCEL_REASON_DELETE;
            break;
        case NotificationConstant::APP_CANCEL_ALL_REASON_DELETE:
            outType = NotificationReason::APP_CANCEL_ALL_REASON_DELETE;
            break;
        case NotificationConstant::APP_CANCEL_REASON_OTHER:
            outType = NotificationReason::APP_CANCEL_REASON_OTHER;
            break;
        default:
            ANS_LOGE("Reason %{public}d is an invalid value", inType);
            return false;
    }
    return true;
}

bool Common::DisturbModeJSToC(const enum DisturbMode &inType, enum NotificationConstant::DisturbMode &outType)
{
    switch (inType) {
        case DisturbMode::ALLOW_UNKNOWN:
            outType = NotificationConstant::DisturbMode::ALLOW_UNKNOWN;
            break;
        case DisturbMode::ALLOW_ALL:
            outType = NotificationConstant::DisturbMode::ALLOW_ALL;
            break;
        case DisturbMode::ALLOW_PRIORITY:
            outType = NotificationConstant::DisturbMode::ALLOW_PRIORITY;
            break;
        case DisturbMode::ALLOW_NONE:
            outType = NotificationConstant::DisturbMode::ALLOW_NONE;
            break;
        case DisturbMode::ALLOW_ALARMS:
            outType = NotificationConstant::DisturbMode::ALLOW_ALARMS;
            break;
        default:
            ANS_LOGE("DisturbMode %{public}d is an invalid value", inType);
            return false;
    }
    return true;
}

napi_value Common::CreateWantAgentByJS(const napi_env &env, const std::shared_ptr<WantAgent::WantAgent> &agent)
{
    if (agent == nullptr) {
        ANS_LOGI("agent is nullptr");
        return nullptr;
    }

    wantAgent_.insert(agent);
    napi_value wantAgent = nullptr;
    napi_value wantAgentClass = nullptr;
    napi_define_class(env,
        "wantAgentClass",
        NAPI_AUTO_LENGTH,
        [](napi_env env, napi_callback_info info) -> napi_value {
            napi_value thisVar = nullptr;
            napi_get_cb_info(env, info, nullptr, nullptr, &thisVar, nullptr);
            return thisVar;
        },
        nullptr,
        0,
        nullptr,
        &wantAgentClass);
    napi_new_instance(env, wantAgentClass, 0, nullptr, &wantAgent);
    napi_wrap(env,
        wantAgent,
        (void *)agent.get(),
        [](napi_env env, void *data, void *hint) {
            WantAgent::WantAgent *objectInfo = (WantAgent::WantAgent *)data;
            ANS_LOGI("CreateWantAgentByJS this = %{public}p", objectInfo);

            if (objectInfo) {
                for (auto it = wantAgent_.begin(); it != wantAgent_.end(); ++it) {
                    if ((*it).get() == objectInfo) {
                        wantAgent_.erase(it);
                        break;
                    }
                }
            }
        },
        nullptr,
        nullptr);

    return wantAgent;
}
}  // namespace NotificationNapi
}  // namespace OHOS
