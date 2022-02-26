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
const int ENABLE_NOTIFICATION_MAX_PARA = 3;
const int ENABLE_NOTIFICATION_MIN_PARA = 2;
const int IS_NOTIFICATION_ENABLE_MAX_PARA = 2;

struct EnableParams {
    BundleOption option;
    bool enable = false;
    napi_ref callback = nullptr;
};

struct AsyncCallbackInfoEnable {
    napi_env env = nullptr;
    napi_async_work asyncWork = nullptr;
    EnableParams params;
    CallbackPromiseInfo info;
};

struct IsEnableParams {
    BundleOption option;
    napi_ref callback = nullptr;
    bool hasBundleOption = false;
};

struct AsyncCallbackInfoIsEnable {
    napi_env env = nullptr;
    napi_async_work asyncWork = nullptr;
    IsEnableParams params;
    CallbackPromiseInfo info;
    bool allowed = false;
};

napi_value ParseParameters(const napi_env &env, const napi_callback_info &info, EnableParams &params)
{
    ANS_LOGI("enter");

    size_t argc = ENABLE_NOTIFICATION_MAX_PARA;
    napi_value argv[ENABLE_NOTIFICATION_MAX_PARA] = {nullptr};
    napi_value thisVar = nullptr;
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, &thisVar, NULL));
    NAPI_ASSERT(env, argc >= ENABLE_NOTIFICATION_MIN_PARA, "Wrong number of arguments");

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
    if (argc >= ENABLE_NOTIFICATION_MAX_PARA) {
        NAPI_CALL(env, napi_typeof(env, argv[ENABLE_NOTIFICATION_MAX_PARA - 1], &valuetype));
        NAPI_ASSERT(env, valuetype == napi_function, "Wrong argument type. Function expected.");
        napi_create_reference(env, argv[ENABLE_NOTIFICATION_MAX_PARA - 1], 1, &params.callback);
    }

    return Common::NapiGetNull(env);
}

napi_value ParseParameters(const napi_env &env, const napi_callback_info &info, IsEnableParams &params)
{
    ANS_LOGI("enter");

    size_t argc = IS_NOTIFICATION_ENABLE_MAX_PARA;
    napi_value argv[IS_NOTIFICATION_ENABLE_MAX_PARA] = {nullptr};
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
    if (argc >= IS_NOTIFICATION_ENABLE_MAX_PARA) {
        NAPI_CALL(env, napi_typeof(env, argv[1], &valuetype));
        NAPI_ASSERT(env, valuetype == napi_function, "Wrong argument type. Function expected.");
        napi_create_reference(env, argv[1], 1, &params.callback);
    }

    return Common::NapiGetNull(env);
}

napi_value EnableNotification(napi_env env, napi_callback_info info)
{
    ANS_LOGI("enter");

    EnableParams params {};
    if (ParseParameters(env, info, params) == nullptr) {
        return Common::JSParaError(env, params.callback);
    }

    AsyncCallbackInfoEnable *asynccallbackinfo =
        new (std::nothrow) AsyncCallbackInfoEnable {.env = env, .asyncWork = nullptr, .params = params};
    if (!asynccallbackinfo) {
        return Common::JSParaError(env, params.callback);
    }
    napi_value promise = nullptr;
    Common::PaddingCallbackPromiseInfo(env, params.callback, asynccallbackinfo->info, promise);

    napi_value resourceName = nullptr;
    napi_create_string_latin1(env, "enableNotification", NAPI_AUTO_LENGTH, &resourceName);
    // Asynchronous function call
    napi_create_async_work(env,
        nullptr,
        resourceName,
        [](napi_env env, void *data) {
            ANS_LOGI("EnableNotification napi_create_async_work start");
            AsyncCallbackInfoEnable *asynccallbackinfo = (AsyncCallbackInfoEnable *)data;
            NotificationBundleOption bundleOption;
            bundleOption.SetBundleName(asynccallbackinfo->params.option.bundle);
            bundleOption.SetUid(asynccallbackinfo->params.option.uid);
            std::string deviceId;
            asynccallbackinfo->info.errorCode = NotificationHelper::SetNotificationsEnabledForSpecifiedBundle(
                bundleOption, deviceId, asynccallbackinfo->params.enable);
            ANS_LOGI("asynccallbackinfo->info.errorCode = %{public}d", asynccallbackinfo->info.errorCode);
        },
        [](napi_env env, napi_status status, void *data) {
            ANS_LOGI("EnableNotification napi_create_async_work end");
            AsyncCallbackInfoEnable *asynccallbackinfo = (AsyncCallbackInfoEnable *)data;

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

napi_value IsNotificationEnabled(napi_env env, napi_callback_info info)
{
    ANS_LOGI("enter");

    IsEnableParams params {};
    if (ParseParameters(env, info, params) == nullptr) {
        return Common::JSParaError(env, params.callback);
    }

    AsyncCallbackInfoIsEnable *asynccallbackinfo =
        new (std::nothrow) AsyncCallbackInfoIsEnable {.env = env, .asyncWork = nullptr, .params = params};
    if (!asynccallbackinfo) {
        return Common::JSParaError(env, params.callback);
    }
    napi_value promise = nullptr;
    Common::PaddingCallbackPromiseInfo(env, params.callback, asynccallbackinfo->info, promise);

    napi_value resourceName = nullptr;
    napi_create_string_latin1(env, "isNotificationEnabled", NAPI_AUTO_LENGTH, &resourceName);
    // Asynchronous function call
    napi_create_async_work(env,
        nullptr,
        resourceName,
        [](napi_env env, void *data) {
            ANS_LOGI("IsNotificationEnabled napi_create_async_work start");
            AsyncCallbackInfoIsEnable *asynccallbackinfo = (AsyncCallbackInfoIsEnable *)data;

            if (asynccallbackinfo->params.hasBundleOption) {
                NotificationBundleOption bundleOption;
                bundleOption.SetBundleName(asynccallbackinfo->params.option.bundle);
                bundleOption.SetUid(asynccallbackinfo->params.option.uid);
                ANS_LOGI("asynccallbackinfo->params.option.bundle = %{public}s",
                    asynccallbackinfo->params.option.bundle.c_str());
                ANS_LOGI("asynccallbackinfo->params.option.uid = %{public}d", asynccallbackinfo->params.option.uid);
                ANS_LOGI("asynccallbackinfo->allowed = %{public}d", asynccallbackinfo->allowed);
                asynccallbackinfo->info.errorCode =
                    NotificationHelper::IsAllowedNotify(bundleOption, asynccallbackinfo->allowed);
            } else {
                ANS_LOGI("asynccallbackinfo->allowed = %{public}d", asynccallbackinfo->allowed);
                asynccallbackinfo->info.errorCode = NotificationHelper::IsAllowedNotify(asynccallbackinfo->allowed);
            }
            ANS_LOGI("asynccallbackinfo->info.errorCode = %{public}d", asynccallbackinfo->info.errorCode);
        },
        [](napi_env env, napi_status status, void *data) {
            ANS_LOGI("IsNotificationEnabled napi_create_async_work end");
            AsyncCallbackInfoIsEnable *asynccallbackinfo = (AsyncCallbackInfoIsEnable *)data;

            napi_value result = nullptr;
            napi_get_boolean(env, asynccallbackinfo->allowed, &result);
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