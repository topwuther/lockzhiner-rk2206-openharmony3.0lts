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

#include "enable_notification.h"

namespace OHOS {
namespace NotificationNapi {
const int ENABLE_BADGE_DISPLAYED_MAX_PARA = 3;
const int ENABLE_BADGE_DISPLAYED_MIN_PARA = 2;
const int IS_DISPLAY_BADGE_MAX_PARA = 2;

struct EnableBadgeParams {
    BundleOption option;
    bool enable = false;
    napi_ref callback = nullptr;
};

struct AsyncCallbackInfoEnableBadge {
    napi_env env = nullptr;
    napi_async_work asyncWork = nullptr;
    EnableBadgeParams params;
    CallbackPromiseInfo info;
};

struct IsDisplayBadgeParams {
    BundleOption option;
    napi_ref callback = nullptr;
    bool hasBundleOption = false;
};

struct AsyncCallbackInfoIsDisplayBadge {
    napi_env env = nullptr;
    napi_async_work asyncWork = nullptr;
    IsDisplayBadgeParams params;
    CallbackPromiseInfo info;
    bool enabled = false;
};

napi_value ParseParameters(const napi_env &env, const napi_callback_info &info, EnableBadgeParams &params)
{
    ANS_LOGI("enter");

    size_t argc = ENABLE_BADGE_DISPLAYED_MAX_PARA;
    napi_value argv[ENABLE_BADGE_DISPLAYED_MAX_PARA] = {nullptr};
    napi_value thisVar = nullptr;
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, &thisVar, NULL));
    NAPI_ASSERT(env, argc >= ENABLE_BADGE_DISPLAYED_MIN_PARA, "Wrong number of arguments");

    // argv[0]: bundle
    napi_valuetype valuetype = napi_undefined;
    NAPI_CALL(env, napi_typeof(env, argv[0], &valuetype));
    NAPI_ASSERT(env, valuetype == napi_object, "Wrong argument type.Object expected.");
    auto retValue = Common::GetBundleOption(env, argv[0], params.option);
    if (retValue == nullptr) {
        ANS_LOGE("GetBundleOption failed.");
        return nullptr;
    }

    // argv[1]: enable
    NAPI_CALL(env, napi_typeof(env, argv[1], &valuetype));
    NAPI_ASSERT(env, valuetype == napi_boolean, "Wrong argument type. Bool expected.");
    napi_get_value_bool(env, argv[1], &params.enable);

    // argv[2]:callback
    if (argc >= ENABLE_BADGE_DISPLAYED_MAX_PARA) {
        NAPI_CALL(env, napi_typeof(env, argv[ENABLE_BADGE_DISPLAYED_MAX_PARA - 1], &valuetype));
        NAPI_ASSERT(env, valuetype == napi_function, "Wrong argument type. Function expected.");
        napi_create_reference(env, argv[ENABLE_BADGE_DISPLAYED_MAX_PARA - 1], 1, &params.callback);
    }

    return Common::NapiGetNull(env);
}

napi_value ParseParameters(const napi_env &env, const napi_callback_info &info, IsDisplayBadgeParams &params)
{
    ANS_LOGI("enter");

    size_t argc = IS_DISPLAY_BADGE_MAX_PARA;
    napi_value argv[IS_DISPLAY_BADGE_MAX_PARA] = {nullptr};
    napi_value thisVar = nullptr;
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, &thisVar, NULL));

    if (argc == 0) {
        return Common::NapiGetNull(env);
    }

    // argv[0]: bundle / callback
    napi_valuetype valuetype = napi_undefined;
    NAPI_CALL(env, napi_typeof(env, argv[0], &valuetype));
    NAPI_ASSERT(env,
        (valuetype == napi_function) || (valuetype == napi_object),
        "Wrong argument type. Function or object expected.");
    if (valuetype == napi_object) {
        auto retValue = Common::GetBundleOption(env, argv[0], params.option);
        if (retValue == nullptr) {
            ANS_LOGE("GetBundleOption failed.");
            return nullptr;
        }
        params.hasBundleOption = true;
    } else {
        napi_create_reference(env, argv[0], 1, &params.callback);
    }

    // argv[1]:callback
    if (argc >= IS_DISPLAY_BADGE_MAX_PARA) {
        NAPI_CALL(env, napi_typeof(env, argv[1], &valuetype));
        NAPI_ASSERT(env, valuetype == napi_function, "Wrong argument type. Function expected.");
        napi_create_reference(env, argv[1], 1, &params.callback);
    }

    return Common::NapiGetNull(env);
}

napi_value DisplayBadge(napi_env env, napi_callback_info info)
{
    ANS_LOGI("enter");

    EnableBadgeParams params {};
    if (ParseParameters(env, info, params) == nullptr) {
        return Common::JSParaError(env, params.callback);
    }

    AsyncCallbackInfoEnableBadge *asynccallbackinfo =
        new (std::nothrow) AsyncCallbackInfoEnableBadge {.env = env, .asyncWork = nullptr, .params = params};
    if (!asynccallbackinfo) {
        return Common::JSParaError(env, params.callback);
    }
    napi_value promise = nullptr;
    Common::PaddingCallbackPromiseInfo(env, params.callback, asynccallbackinfo->info, promise);

    napi_value resourceName = nullptr;
    napi_create_string_latin1(env, "DisplayBadge", NAPI_AUTO_LENGTH, &resourceName);
    // Asynchronous function call
    napi_create_async_work(env,
        nullptr,
        resourceName,
        [](napi_env env, void *data) {
            ANS_LOGI("DisplayBadge napi_create_async_work start");
            AsyncCallbackInfoEnableBadge *asynccallbackinfo = (AsyncCallbackInfoEnableBadge *)data;
            NotificationBundleOption bundleOption;
            bundleOption.SetBundleName(asynccallbackinfo->params.option.bundle);
            bundleOption.SetUid(asynccallbackinfo->params.option.uid);
            ANS_LOGI("asynccallbackinfo->params.option.bundle = %{public}s",
                asynccallbackinfo->params.option.bundle.c_str());
            ANS_LOGI("asynccallbackinfo->params.option.uid = %{public}d", asynccallbackinfo->params.option.uid);
            ANS_LOGI("asynccallbackinfo->params.enable = %{public}d", asynccallbackinfo->params.enable);
            asynccallbackinfo->info.errorCode =
                NotificationHelper::SetShowBadgeEnabledForBundle(bundleOption, asynccallbackinfo->params.enable);
            ANS_LOGI("asynccallbackinfo->info.errorCode = %{public}d", asynccallbackinfo->info.errorCode);
        },
        [](napi_env env, napi_status status, void *data) {
            ANS_LOGI("DisplayBadge napi_create_async_work end");
            AsyncCallbackInfoEnableBadge *asynccallbackinfo = (AsyncCallbackInfoEnableBadge *)data;

            Common::ReturnCallbackPromise(env, asynccallbackinfo->info, Common::NapiGetNull(env));

            if (asynccallbackinfo->info.callback != nullptr) {
                napi_delete_reference(env, asynccallbackinfo->info.callback);
            }

            napi_delete_async_work(env, asynccallbackinfo->asyncWork);
            if (asynccallbackinfo) {
                delete asynccallbackinfo;
                asynccallbackinfo = nullptr;
            }
        },
        (void *)asynccallbackinfo,
        &asynccallbackinfo->asyncWork);

    NAPI_CALL(env, napi_queue_async_work(env, asynccallbackinfo->asyncWork));

    if (asynccallbackinfo->info.isCallback) {
        return Common::NapiGetNull(env);
    } else {
        return promise;
    }
}

napi_value IsBadgeDisplayed(napi_env env, napi_callback_info info)
{
    ANS_LOGI("enter");

    IsDisplayBadgeParams params {};
    if (ParseParameters(env, info, params) == nullptr) {
        return Common::JSParaError(env, params.callback);
    }

    AsyncCallbackInfoIsDisplayBadge *asynccallbackinfo =
        new (std::nothrow) AsyncCallbackInfoIsDisplayBadge {.env = env, .asyncWork = nullptr, .params = params};
    if (!asynccallbackinfo) {
        return Common::JSParaError(env, params.callback);
    }
    napi_value promise = nullptr;
    Common::PaddingCallbackPromiseInfo(env, params.callback, asynccallbackinfo->info, promise);

    napi_value resourceName = nullptr;
    napi_create_string_latin1(env, "IsBadgeDisplayed", NAPI_AUTO_LENGTH, &resourceName);
    // Asynchronous function call
    napi_create_async_work(env,
        nullptr,
        resourceName,
        [](napi_env env, void *data) {
            ANS_LOGI("IsBadgeDisplayed napi_create_async_work start");
            AsyncCallbackInfoIsDisplayBadge *asynccallbackinfo = (AsyncCallbackInfoIsDisplayBadge *)data;

            NotificationBundleOption bundleOption;
            bundleOption.SetBundleName(asynccallbackinfo->params.option.bundle);
            bundleOption.SetUid(asynccallbackinfo->params.option.uid);
            if (asynccallbackinfo->params.hasBundleOption) {
                ANS_LOGI("asynccallbackinfo->params.option.bundle = %{public}s",
                    asynccallbackinfo->params.option.bundle.c_str());
                ANS_LOGI("asynccallbackinfo->params.option.uid = %{public}d", asynccallbackinfo->params.option.uid);
                ANS_LOGI("asynccallbackinfo->enabled = %{public}d", asynccallbackinfo->enabled);
                asynccallbackinfo->info.errorCode =
                    NotificationHelper::GetShowBadgeEnabledForBundle(bundleOption, asynccallbackinfo->enabled);
            } else {
                ANS_LOGI("asynccallbackinfo->enabled = %{public}d", asynccallbackinfo->enabled);
                asynccallbackinfo->info.errorCode = NotificationHelper::GetShowBadgeEnabled(asynccallbackinfo->enabled);
            }
            ANS_LOGI("asynccallbackinfo->info.errorCode = %{public}d", asynccallbackinfo->info.errorCode);
        },
        [](napi_env env, napi_status status, void *data) {
            ANS_LOGI("IsBadgeDisplayed napi_create_async_work end");
            AsyncCallbackInfoIsDisplayBadge *asynccallbackinfo = (AsyncCallbackInfoIsDisplayBadge *)data;

            napi_value result = nullptr;
            napi_get_boolean(env, asynccallbackinfo->enabled, &result);
            Common::ReturnCallbackPromise(env, asynccallbackinfo->info, result);

            if (asynccallbackinfo->info.callback != nullptr) {
                napi_delete_reference(env, asynccallbackinfo->info.callback);
            }

            napi_delete_async_work(env, asynccallbackinfo->asyncWork);
            if (asynccallbackinfo) {
                delete asynccallbackinfo;
                asynccallbackinfo = nullptr;
            }
        },
        (void *)asynccallbackinfo,
        &asynccallbackinfo->asyncWork);

    NAPI_CALL(env, napi_queue_async_work(env, asynccallbackinfo->asyncWork));

    if (asynccallbackinfo->info.isCallback) {
        return Common::NapiGetNull(env);
    } else {
        return promise;
    }
}

}  // namespace NotificationNapi
}  // namespace OHOS