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

#include "get_active.h"

namespace OHOS {
namespace NotificationNapi {
const int ALL_ACTIVE_MAX_PARA = 1;
const int ACTIVE_OR_NUMS_MAX_PARA = 1;

struct AsyncCallbackInfoActive {
    napi_env env = nullptr;
    napi_async_work asyncWork = nullptr;
    napi_value result = nullptr;
    CallbackPromiseInfo info;
};

napi_value ParseParametersByAllActive(const napi_env &env, const napi_callback_info &info, napi_ref &callback)
{
    ANS_LOGI("enter");

    size_t argc = ALL_ACTIVE_MAX_PARA;
    napi_value argv[ALL_ACTIVE_MAX_PARA] = {nullptr};
    napi_value thisVar = nullptr;
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, &thisVar, NULL));

    if (argc >= ALL_ACTIVE_MAX_PARA) {
        napi_valuetype valuetype = napi_undefined;
        // argv[0]:callback
        NAPI_CALL(env, napi_typeof(env, argv[0], &valuetype));
        NAPI_ASSERT(env, valuetype == napi_function, "Wrong argument type. Function expected.");
        napi_create_reference(env, argv[0], 1, &callback);
    }

    return Common::NapiGetNull(env);
}

napi_value ParseParametersByGetActive(const napi_env &env, const napi_callback_info &info, napi_ref &callback)
{
    ANS_LOGI("enter");

    size_t argc = ACTIVE_OR_NUMS_MAX_PARA;
    napi_value argv[ACTIVE_OR_NUMS_MAX_PARA] = {nullptr};
    napi_value thisVar = nullptr;
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, &thisVar, NULL));

    if (argc >= ACTIVE_OR_NUMS_MAX_PARA) {
        napi_valuetype valuetype = napi_undefined;
        // argv[0]:callback
        NAPI_CALL(env, napi_typeof(env, argv[0], &valuetype));
        NAPI_ASSERT(env, valuetype == napi_function, "Wrong argument type. Function expected.");
        napi_create_reference(env, argv[0], 1, &callback);
    }

    return Common::NapiGetNull(env);
}

napi_value GetAllActiveNotifications(napi_env env, napi_callback_info info)
{
    ANS_LOGI("enter");

    napi_ref callback = nullptr;
    if (ParseParametersByAllActive(env, info, callback) == nullptr) {
        return Common::JSParaError(env, callback);
    }

    AsyncCallbackInfoActive *asynccallbackinfo =
        new (std::nothrow) AsyncCallbackInfoActive {.env = env, .asyncWork = nullptr};
    if (!asynccallbackinfo) {
        return Common::JSParaError(env, callback);
    }
    napi_value promise = nullptr;
    Common::PaddingCallbackPromiseInfo(env, callback, asynccallbackinfo->info, promise);

    napi_value resourceName = nullptr;
    napi_create_string_latin1(env, "getAllActiveNotifications", NAPI_AUTO_LENGTH, &resourceName);
    // Asynchronous function call
    napi_create_async_work(
        env,
        nullptr,
        resourceName,
        [](napi_env env, void *data) {
            ANS_LOGI("GetAllActiveNotifications napi_create_async_work start");
            AsyncCallbackInfoActive *asynccallbackinfo = (AsyncCallbackInfoActive *)data;

            std::vector<sptr<OHOS::Notification::Notification>> notifications;
            asynccallbackinfo->info.errorCode = NotificationHelper::GetAllActiveNotifications(notifications);
            if (asynccallbackinfo->info.errorCode != ERR_OK) {
                asynccallbackinfo->result = Common::NapiGetNull(env);
                return;
            }
            napi_value arr = nullptr;
            int count = 0;
            napi_create_array(env, &arr);
            for (auto vec : notifications) {
                if (!vec) {
                    ANS_LOGW("Invalid Notification object ptr");
                    continue;
                }
                napi_value result = nullptr;
                napi_create_object(env, &result);
                if (!Common::SetNotification(env, vec.GetRefPtr(), result)) {
                    ANS_LOGW("Set Notification object failed");
                    continue;
                }
                napi_set_element(env, arr, count, result);
                count++;
            }
            ANS_LOGI("GetAllActiveNotifications count = %{public}d", count);
            asynccallbackinfo->result = arr;
            if ((count == 0) && (notifications.size() > 0)) {
                asynccallbackinfo->info.errorCode = ERROR;
                asynccallbackinfo->result = Common::NapiGetNull(env);
            }
        },
        [](napi_env env, napi_status status, void *data) {
            ANS_LOGI("GetAllActiveNotifications napi_create_async_work end");
            AsyncCallbackInfoActive *asynccallbackinfo = (AsyncCallbackInfoActive *)data;

            Common::ReturnCallbackPromise(env, asynccallbackinfo->info, asynccallbackinfo->result);

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

napi_value GetActiveNotifications(napi_env env, napi_callback_info info)
{
    ANS_LOGI("enter");

    napi_ref callback = nullptr;
    if (ParseParametersByGetActive(env, info, callback) == nullptr) {
        return Common::JSParaError(env, callback);
    }

    AsyncCallbackInfoActive *asynccallbackinfo =
        new (std::nothrow) AsyncCallbackInfoActive {.env = env, .asyncWork = nullptr};
    if (!asynccallbackinfo) {
        return Common::JSParaError(env, callback);
    }
    napi_value promise = nullptr;
    Common::PaddingCallbackPromiseInfo(env, callback, asynccallbackinfo->info, promise);

    napi_value resourceName = nullptr;
    napi_create_string_latin1(env, "getActiveNotifications", NAPI_AUTO_LENGTH, &resourceName);
    // Asynchronous function call
    napi_create_async_work(
        env,
        nullptr,
        resourceName,
        [](napi_env env, void *data) {
            ANS_LOGI("GetActiveNotifications napi_create_async_work start");
            AsyncCallbackInfoActive *asynccallbackinfo = (AsyncCallbackInfoActive *)data;

            std::vector<sptr<OHOS::Notification::NotificationRequest>> requests;
            asynccallbackinfo->info.errorCode = NotificationHelper::GetActiveNotifications(requests);
            if (asynccallbackinfo->info.errorCode != ERR_OK) {
                asynccallbackinfo->result = Common::NapiGetNull(env);
                return;
            }
            napi_value arr = nullptr;
            int count = 0;
            napi_create_array(env, &arr);
            for (auto vec : requests) {
                if (!vec) {
                    ANS_LOGW("Invalid NotificationRequest object ptr");
                    continue;
                }
                napi_value result = nullptr;
                napi_create_object(env, &result);
                if (!Common::SetNotificationRequest(env, vec.GetRefPtr(), result)) {
                    ANS_LOGW("Set NotificationRequest object failed");
                    continue;
                }
                napi_set_element(env, arr, count, result);
                count++;
            }
            ANS_LOGI("GetActiveNotifications count = %{public}d", count);
            asynccallbackinfo->result = arr;
            if ((count == 0) && (requests.size() > 0)) {
                asynccallbackinfo->info.errorCode = ERROR;
                asynccallbackinfo->result = Common::NapiGetNull(env);
            }
        },
        [](napi_env env, napi_status status, void *data) {
            ANS_LOGI("GetActiveNotifications napi_create_async_work end");
            AsyncCallbackInfoActive *asynccallbackinfo = (AsyncCallbackInfoActive *)data;

            Common::ReturnCallbackPromise(env, asynccallbackinfo->info, asynccallbackinfo->result);

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

napi_value GetActiveNotificationCount(napi_env env, napi_callback_info info)
{
    ANS_LOGI("enter");

    napi_ref callback = nullptr;
    if (ParseParametersByGetActive(env, info, callback) == nullptr) {
        return Common::JSParaError(env, callback);
    }

    AsyncCallbackInfoActive *asynccallbackinfo =
        new (std::nothrow) AsyncCallbackInfoActive {.env = env, .asyncWork = nullptr};
    if (!asynccallbackinfo) {
        return Common::JSParaError(env, callback);
    }
    napi_value promise = nullptr;
    Common::PaddingCallbackPromiseInfo(env, callback, asynccallbackinfo->info, promise);

    napi_value resourceName = nullptr;
    napi_create_string_latin1(env, "getActiveNotificationCount", NAPI_AUTO_LENGTH, &resourceName);
    // Asynchronous function call
    napi_create_async_work(
        env,
        nullptr,
        resourceName,
        [](napi_env env, void *data) {
            ANS_LOGI("GetActiveNotificationCount napi_create_async_work start");
            AsyncCallbackInfoActive *asynccallbackinfo = (AsyncCallbackInfoActive *)data;

            int32_t num = 0;
            asynccallbackinfo->info.errorCode = NotificationHelper::GetActiveNotificationNums(num);
            ANS_LOGI("GetActiveNotificationCount count = %{public}d", num);
            if (asynccallbackinfo->info.errorCode != ERR_OK) {
                asynccallbackinfo->result = Common::NapiGetNull(env);
                return;
            }
            napi_create_int32(env, num, &asynccallbackinfo->result);
        },
        [](napi_env env, napi_status status, void *data) {
            ANS_LOGI("GetActiveNotificationCount napi_create_async_work end");
            AsyncCallbackInfoActive *asynccallbackinfo = (AsyncCallbackInfoActive *)data;

            Common::ReturnCallbackPromise(env, asynccallbackinfo->info, asynccallbackinfo->result);

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