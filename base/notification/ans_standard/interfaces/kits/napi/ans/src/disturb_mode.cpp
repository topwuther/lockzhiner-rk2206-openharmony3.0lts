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

#include "disturb_mode.h"

namespace OHOS {
namespace NotificationNapi {
const int DISTURB_MODE_MAX_PARA = 2;
const int DISTURB_MODE_MIN_PARA = 1;

struct DisturbModeParams {
    NotificationConstant::DisturbMode mode = NotificationConstant::DisturbMode::ALLOW_UNKNOWN;
    napi_ref callback = nullptr;
};

struct AsyncCallbackInfoDisturbMode {
    napi_env env = nullptr;
    napi_async_work asyncWork = nullptr;
    DisturbModeParams params;
    CallbackPromiseInfo info;
};

napi_value ParseParameters(const napi_env &env, const napi_callback_info &info, DisturbModeParams &params)
{
    ANS_LOGI("enter");

    size_t argc = DISTURB_MODE_MAX_PARA;
    napi_value argv[DISTURB_MODE_MAX_PARA] = {nullptr};
    napi_value thisVar = nullptr;
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, &thisVar, NULL));
    NAPI_ASSERT(env, argc >= DISTURB_MODE_MIN_PARA, "Wrong number of arguments");

    // argv[0]: mode
    napi_valuetype valuetype = napi_undefined;
    NAPI_CALL(env, napi_typeof(env, argv[0], &valuetype));
    NAPI_ASSERT(env, valuetype == napi_number, "Wrong argument type. Number expected.");
    int mode = 0;
    napi_get_value_int32(env, argv[0], &mode);
    ANS_LOGI("mode is: %{public}d", mode);

    if (!Common::DisturbModeJSToC(DisturbMode(mode), params.mode)) {
        return nullptr;
    }

    // argv[1]:callback
    if (argc >= DISTURB_MODE_MAX_PARA) {
        NAPI_CALL(env, napi_typeof(env, argv[1], &valuetype));
        NAPI_ASSERT(env, valuetype == napi_function, "Wrong argument type. Function expected.");
        napi_create_reference(env, argv[1], 1, &params.callback);
    }

    return Common::NapiGetNull(env);
}

napi_value SetDoNotDisturbMode(napi_env env, napi_callback_info info)
{
    ANS_LOGI("enter");

    DisturbModeParams params {};
    if (ParseParameters(env, info, params) == nullptr) {
        return Common::JSParaError(env, params.callback);
    }

    AsyncCallbackInfoDisturbMode *asynccallbackinfo =
        new (std::nothrow) AsyncCallbackInfoDisturbMode {.env = env, .asyncWork = nullptr, .params = params};
    if (!asynccallbackinfo) {
        return Common::JSParaError(env, params.callback);
    }
    napi_value promise = nullptr;
    Common::PaddingCallbackPromiseInfo(env, params.callback, asynccallbackinfo->info, promise);

    napi_value resourceName = nullptr;
    napi_create_string_latin1(env, "setDoNotDisturbMode", NAPI_AUTO_LENGTH, &resourceName);
    // Asynchronous function call
    napi_create_async_work(env,
        nullptr,
        resourceName,
        [](napi_env env, void *data) {
            ANS_LOGI("SetDoNotDisturbMode napi_create_async_work start");
            AsyncCallbackInfoDisturbMode *asynccallbackinfo = (AsyncCallbackInfoDisturbMode *)data;
            ANS_LOGI("asynccallbackinfo->params.mode = %{public}d", asynccallbackinfo->params.mode);
            asynccallbackinfo->info.errorCode = NotificationHelper::SetDisturbMode(asynccallbackinfo->params.mode);
            ANS_LOGI("asynccallbackinfo->info.errorCode = %{public}d", asynccallbackinfo->info.errorCode);
        },
        [](napi_env env, napi_status status, void *data) {
            ANS_LOGI("SetDoNotDisturbMode napi_create_async_work end");
            AsyncCallbackInfoDisturbMode *asynccallbackinfo = (AsyncCallbackInfoDisturbMode *)data;

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