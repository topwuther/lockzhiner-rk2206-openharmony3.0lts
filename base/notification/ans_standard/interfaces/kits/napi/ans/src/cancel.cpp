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

#include "cancel.h"

namespace OHOS {
namespace NotificationNapi {

const int CANCEL_MAX_PARA = 3;
const int CANCEL_ALL_MAX_PARA = 1;

struct ParametersInfoCancel {
    int id = 0;
    std::string label = "";
    napi_ref callback = nullptr;
};

struct AsyncCallbackInfoCancel {
    napi_env env = nullptr;
    napi_async_work asyncWork = nullptr;
    int id = 0;
    std::string label;
    CallbackPromiseInfo info;
};

napi_value ParseParameters(const napi_env &env, const napi_callback_info &info, ParametersInfoCancel &paras)
{
    ANS_LOGI("enter");

    size_t argc = CANCEL_MAX_PARA;
    napi_value argv[CANCEL_MAX_PARA] = {nullptr};
    napi_value thisVar = nullptr;
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, &thisVar, NULL));
    NAPI_ASSERT(env, argc >= 1, "Wrong number of arguments");

    napi_valuetype valuetype = napi_undefined;
    // argv[0]: id: number
    NAPI_CALL(env, napi_typeof(env, argv[0], &valuetype));
    NAPI_ASSERT(env, valuetype == napi_number, "Wrong argument type. Number expected.");
    NAPI_CALL(env, napi_get_value_int32(env, argv[0], &paras.id));

    // argv[1]: label: string / callback
    if (argc >= CANCEL_MAX_PARA - 1) {
        NAPI_CALL(env, napi_typeof(env, argv[1], &valuetype));
        NAPI_ASSERT(env,
            (valuetype == napi_string || valuetype == napi_function),
            "Wrong argument type. String or function expected.");
        if (valuetype == napi_string) {
            char str[STR_MAX_SIZE] = {0};
            size_t strLen = 0;
            NAPI_CALL(env, napi_get_value_string_utf8(env, argv[1], str, STR_MAX_SIZE - 1, &strLen));
            paras.label = str;
        } else {
            napi_create_reference(env, argv[1], 1, &paras.callback);
        }
    }

    // argv[2]: callback
    if (argc >= CANCEL_MAX_PARA) {
        NAPI_CALL(env, napi_typeof(env, argv[CANCEL_MAX_PARA - 1], &valuetype));
        NAPI_ASSERT(env, valuetype == napi_function, "Wrong argument type. Function expected.");
        napi_create_reference(env, argv[CANCEL_MAX_PARA - 1], 1, &paras.callback);
    }

    return Common::NapiGetNull(env);
}

napi_value ParseParametersByCancelAll(const napi_env &env, const napi_callback_info &info, napi_ref &callback)
{
    ANS_LOGI("enter");

    size_t argc = CANCEL_ALL_MAX_PARA;
    napi_value argv[CANCEL_ALL_MAX_PARA] = {nullptr};
    napi_value thisVar = nullptr;
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, &thisVar, NULL));

    if (argc >= CANCEL_ALL_MAX_PARA) {
        napi_valuetype valuetype = napi_undefined;
        // argv[0]:callback
        NAPI_CALL(env, napi_typeof(env, argv[0], &valuetype));
        NAPI_ASSERT(env, valuetype == napi_function, "Wrong argument type. Function expected.");
        napi_create_reference(env, argv[0], 1, &callback);
    }
    return Common::NapiGetNull(env);
}

napi_value Cancel(napi_env env, napi_callback_info info)
{
    ANS_LOGI("enter");

    ParametersInfoCancel paras;
    if (ParseParameters(env, info, paras) == nullptr) {
        return Common::JSParaError(env, paras.callback);
    }

    AsyncCallbackInfoCancel *asynccallbackinfo = new (std::nothrow)
        AsyncCallbackInfoCancel {.env = env, .asyncWork = nullptr, .id = paras.id, .label = paras.label};
    if (!asynccallbackinfo) {
        return Common::JSParaError(env, paras.callback);
    }
    napi_value promise = nullptr;
    Common::PaddingCallbackPromiseInfo(env, paras.callback, asynccallbackinfo->info, promise);

    napi_value resourceName = nullptr;
    napi_create_string_latin1(env, "cancel", NAPI_AUTO_LENGTH, &resourceName);
    // Asynchronous function call
    napi_create_async_work(env,
        nullptr,
        resourceName,
        [](napi_env env, void *data) {
            ANS_LOGI("Cancel napi_create_async_work start");
            AsyncCallbackInfoCancel *asynccallbackinfo = (AsyncCallbackInfoCancel *)data;

            asynccallbackinfo->info.errorCode =
                NotificationHelper::CancelNotification(asynccallbackinfo->label, asynccallbackinfo->id);
        },
        [](napi_env env, napi_status status, void *data) {
            ANS_LOGI("Cancel napi_create_async_work end");
            AsyncCallbackInfoCancel *asynccallbackinfo = (AsyncCallbackInfoCancel *)data;

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

napi_value CancelAll(napi_env env, napi_callback_info info)
{
    ANS_LOGI("enter");

    napi_ref callback = nullptr;
    if (ParseParametersByCancelAll(env, info, callback) == nullptr) {
        return Common::JSParaError(env, callback);
    }

    AsyncCallbackInfoCancel *asynccallbackinfo =
        new (std::nothrow) AsyncCallbackInfoCancel {.env = env, .asyncWork = nullptr};
    if (!asynccallbackinfo) {
        return Common::JSParaError(env, callback);
    }
    napi_value promise = nullptr;
    Common::PaddingCallbackPromiseInfo(env, callback, asynccallbackinfo->info, promise);

    napi_value resourceName = nullptr;
    napi_create_string_latin1(env, "cancelAll", NAPI_AUTO_LENGTH, &resourceName);
    // Asynchronous function call
    napi_create_async_work(env,
        nullptr,
        resourceName,
        [](napi_env env, void *data) {
            ANS_LOGI("CancelAll napi_create_async_work start");
            AsyncCallbackInfoCancel *asynccallbackinfo = (AsyncCallbackInfoCancel *)data;
            asynccallbackinfo->info.errorCode = NotificationHelper::CancelAllNotifications();
        },
        [](napi_env env, napi_status status, void *data) {
            ANS_LOGI("CancelAll napi_create_async_work end");
            AsyncCallbackInfoCancel *asynccallbackinfo = (AsyncCallbackInfoCancel *)data;

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