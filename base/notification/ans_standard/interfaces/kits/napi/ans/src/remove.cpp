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

#include "remove.h"
#include <optional>
#include "common.h"
#include "notification_bundle_option.h"

namespace OHOS {
namespace NotificationNapi {
const int REMOVE_MIN_PARA = 1;
const int REMOVE_OR_BUNDLE_MAX_PARA = 2;

const int REMOVE_ALL_MAX_PARA = 2;
const int REMOVE_ALL_WHEN_HAS_PARA_MIN_PARA = 1;

const int REMOVE_BY_BUNDLE_AND_KEY_MIN_PARA = 2;
const int REMOVE_BY_BUNDLE_AND_KEY_MAX_PARA = 3;

struct BundleAndKeyInfo {
    BundleOption option;
    NotificationKey key;
};

struct RemoveParams {
    std::optional<std::string> hashcode {};
    std::optional<BundleAndKeyInfo> bundleAndKeyInfo {};
    napi_ref callback = nullptr;
};

struct AsyncCallbackInfoRemove {
    napi_env env = nullptr;
    napi_async_work asyncWork = nullptr;
    RemoveParams params {};
    CallbackPromiseInfo info;
};

napi_value ParseParameters(const napi_env &env, const napi_callback_info &info, RemoveParams &params)
{
    ANS_LOGI("enter");

    size_t argc = REMOVE_BY_BUNDLE_AND_KEY_MAX_PARA;
    napi_value argv[REMOVE_BY_BUNDLE_AND_KEY_MAX_PARA] = {nullptr};
    napi_value thisVar = nullptr;
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, &thisVar, NULL));
    NAPI_ASSERT(env, argc >= REMOVE_MIN_PARA, "Wrong number of arguments");

    napi_valuetype valuetype = napi_undefined;
    NAPI_CALL(env, napi_typeof(env, argv[0], &valuetype));
    NAPI_ASSERT(env,
        (valuetype == napi_string) || (valuetype == napi_object),
        "Wrong argument type. String or object expected.");
    if (valuetype == napi_string) {
        // argv[0]: hashCode
        size_t strLen = 0;
        char str[STR_MAX_SIZE] = {0};
        NAPI_CALL(env, napi_get_value_string_utf8(env, argv[0], str, STR_MAX_SIZE - 1, &strLen));
        params.hashcode = str;

        // argv[1]:callback
        if (argc >= REMOVE_OR_BUNDLE_MAX_PARA) {
            NAPI_CALL(env, napi_typeof(env, argv[1], &valuetype));
            NAPI_ASSERT(env, valuetype == napi_function, "Wrong argument type. Function expected.");
            napi_create_reference(env, argv[1], 1, &params.callback);
        }
    } else {
        NAPI_ASSERT(env, argc >= REMOVE_BY_BUNDLE_AND_KEY_MIN_PARA, "Wrong number of arguments");

        BundleAndKeyInfo info {};
        // argv[0]: BundleOption
        auto retValue = Common::GetBundleOption(env, argv[0], info.option);
        if (retValue == nullptr) {
            ANS_LOGE("GetBundleOption failed.");
            return nullptr;
        }

        // argv[1]: NotificationKey
        retValue = Common::GetNotificationKey(env, argv[1], info.key);
        if (retValue == nullptr) {
            ANS_LOGE("GetNotificationKey failed.");
            return nullptr;
        }

        params.bundleAndKeyInfo = info;

        // argv[2]:callback
        if (argc >= REMOVE_BY_BUNDLE_AND_KEY_MAX_PARA) {
            NAPI_CALL(env, napi_typeof(env, argv[REMOVE_BY_BUNDLE_AND_KEY_MAX_PARA - 1], &valuetype));
            NAPI_ASSERT(env, valuetype == napi_function, "Wrong argument type. Function expected.");
            napi_create_reference(env, argv[REMOVE_BY_BUNDLE_AND_KEY_MAX_PARA - 1], 1, &params.callback);
        }
    }

    return Common::NapiGetNull(env);
}

napi_value ParseParametersByRemoveAll(const napi_env &env, const napi_callback_info &info, RemoveParams &params)
{
    ANS_LOGI("enter");

    size_t argc = REMOVE_ALL_MAX_PARA;
    napi_value argv[REMOVE_ALL_MAX_PARA] = {nullptr};
    napi_value thisVar = nullptr;
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, &thisVar, NULL));

    if (argc < REMOVE_ALL_WHEN_HAS_PARA_MIN_PARA) {
        return Common::NapiGetNull(env);
    }

    napi_valuetype valuetype = napi_undefined;
    NAPI_CALL(env, napi_typeof(env, argv[0], &valuetype));
    NAPI_ASSERT(env,
        (valuetype == napi_function) || (valuetype == napi_object),
        "Wrong argument type. Function or object expected.");
    if (valuetype == napi_function) {
        // argv[0]: callback
        napi_create_reference(env, argv[0], 1, &params.callback);
    } else {
        BundleAndKeyInfo info {};
        // argv[0]: BundleOption
        auto retValue = Common::GetBundleOption(env, argv[0], info.option);
        NAPI_ASSERT(env, retValue != nullptr, "GetBundleOption failed.");
        params.bundleAndKeyInfo = info;

        // argv[1]:callback
        if (argc >= REMOVE_ALL_MAX_PARA) {
            NAPI_CALL(env, napi_typeof(env, argv[1], &valuetype));
            NAPI_ASSERT(env, valuetype == napi_function, "Wrong argument type. Function expected.");
            napi_create_reference(env, argv[1], 1, &params.callback);
        }
    }

    return Common::NapiGetNull(env);
}

napi_value Remove(napi_env env, napi_callback_info info)
{
    ANS_LOGI("enter");

    RemoveParams params {};
    if (ParseParameters(env, info, params) == nullptr) {
        return Common::JSParaError(env, params.callback);
    }

    AsyncCallbackInfoRemove *asynccallbackinfo =
        new (std::nothrow) AsyncCallbackInfoRemove {.env = env, .asyncWork = nullptr, .params = params};
    if (!asynccallbackinfo) {
        return Common::JSParaError(env, params.callback);
    }
    napi_value promise = nullptr;
    Common::PaddingCallbackPromiseInfo(env, params.callback, asynccallbackinfo->info, promise);

    napi_value resourceName = nullptr;
    napi_create_string_latin1(env, "remove", NAPI_AUTO_LENGTH, &resourceName);
    // Asynchronous function call
    napi_create_async_work(env,
        nullptr,
        resourceName,
        [](napi_env env, void *data) {
            ANS_LOGI("Remove napi_create_async_work start");
            AsyncCallbackInfoRemove *asynccallbackinfo = (AsyncCallbackInfoRemove *)data;

            if (asynccallbackinfo->params.hashcode.has_value()) {
                asynccallbackinfo->info.errorCode =
                    NotificationHelper::RemoveNotification(asynccallbackinfo->params.hashcode.value());
            } else if (asynccallbackinfo->params.bundleAndKeyInfo.has_value()) {
                auto &infos = asynccallbackinfo->params.bundleAndKeyInfo.value();

                NotificationBundleOption bundleOption(infos.option.bundle, infos.option.uid);
                asynccallbackinfo->info.errorCode =
                    NotificationHelper::RemoveNotification(bundleOption, infos.key.id, infos.key.label);
            }
        },
        [](napi_env env, napi_status status, void *data) {
            ANS_LOGI("Remove napi_create_async_work end");
            AsyncCallbackInfoRemove *asynccallbackinfo = (AsyncCallbackInfoRemove *)data;

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

napi_value RemoveAll(napi_env env, napi_callback_info info)
{
    ANS_LOGI("enter");

    RemoveParams params {};
    if (ParseParametersByRemoveAll(env, info, params) == nullptr) {
        return Common::JSParaError(env, params.callback);
    }

    AsyncCallbackInfoRemove *asynccallbackinfo =
        new (std::nothrow) AsyncCallbackInfoRemove {.env = env, .asyncWork = nullptr, .params = params};
    if (!asynccallbackinfo) {
        return Common::JSParaError(env, params.callback);
    }
    napi_value promise = nullptr;
    Common::PaddingCallbackPromiseInfo(env, params.callback, asynccallbackinfo->info, promise);

    napi_value resourceName = nullptr;
    napi_create_string_latin1(env, "removeAll", NAPI_AUTO_LENGTH, &resourceName);
    // Asynchronous function call
    napi_create_async_work(env,
        nullptr,
        resourceName,
        [](napi_env env, void *data) {
            ANS_LOGI("RemoveAll napi_create_async_work start");
            AsyncCallbackInfoRemove *asynccallbackinfo = (AsyncCallbackInfoRemove *)data;

            if (asynccallbackinfo->params.bundleAndKeyInfo.has_value()) {
                auto &infos = asynccallbackinfo->params.bundleAndKeyInfo.value();

                NotificationBundleOption bundleOption(infos.option.bundle, infos.option.uid);
                asynccallbackinfo->info.errorCode = NotificationHelper::RemoveAllNotifications(bundleOption);
            } else {
                asynccallbackinfo->info.errorCode = NotificationHelper::RemoveNotifications();
            }
        },
        [](napi_env env, napi_status status, void *data) {
            ANS_LOGI("RemoveAll napi_create_async_work end");
            AsyncCallbackInfoRemove *asynccallbackinfo = (AsyncCallbackInfoRemove *)data;

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

}  // namespace NotificationNapi
}  // namespace OHOS