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

#include "unsubscribe.h"
#include "subscribe.h"

namespace OHOS {
namespace NotificationNapi {
const int UNSUBSCRIBE_MAX_PARA = 2;

struct AsyncCallbackInfoUnsubscribe {
    napi_env env = nullptr;
    napi_async_work asyncWork = nullptr;
    SubscriberInstance *objectInfo = nullptr;
    CallbackPromiseInfo info;
};

struct ParametersInfoUnsubscribe {
    SubscriberInstance *objectInfo = nullptr;
    napi_ref callback = nullptr;
};

napi_value ParseParameters(const napi_env &env, const napi_callback_info &info, ParametersInfoUnsubscribe &paras)
{
    ANS_LOGI("enter");

    size_t argc = UNSUBSCRIBE_MAX_PARA;
    napi_value argv[UNSUBSCRIBE_MAX_PARA] = {nullptr};
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, NULL, NULL));
    NAPI_ASSERT(env, argc >= 1, "Wrong number of arguments");

    napi_valuetype valuetype = napi_undefined;
    // argv[0]:subscriber
    NAPI_CALL(env, napi_typeof(env, argv[0], &valuetype));
    NAPI_ASSERT(env, valuetype == napi_object, "Wrong argument type for arg0. Subscribe object expected.");

    SubscriberInstancesInfo subscriberInstancesInfo;
    if (!HasNotificationSubscriber(env, argv[0], subscriberInstancesInfo)) {
        return nullptr;
    }
    paras.objectInfo = subscriberInstancesInfo.subscriber;
    ANS_LOGI("ObjectInfo = %{public}p start", paras.objectInfo);

    // argv[1]:callback
    if (argc >= UNSUBSCRIBE_MAX_PARA) {
        NAPI_CALL(env, napi_typeof(env, argv[1], &valuetype));
        NAPI_ASSERT(env, valuetype == napi_function, "Wrong argument type. Function expected.");
        napi_create_reference(env, argv[1], 1, &paras.callback);
    }

    return Common::NapiGetNull(env);
}

napi_value Unsubscribe(napi_env env, napi_callback_info info)
{
    ANS_LOGI("Unsubscribe start");

    ParametersInfoUnsubscribe paras;
    if (ParseParameters(env, info, paras) == nullptr) {
        return Common::JSParaError(env, paras.callback);
    }

    AsyncCallbackInfoUnsubscribe *asynccallbackinfo = new (std::nothrow)
        AsyncCallbackInfoUnsubscribe {.env = env, .asyncWork = nullptr, .objectInfo = paras.objectInfo};
    if (!asynccallbackinfo) {
        return Common::JSParaError(env, paras.callback);
    }
    napi_value promise = nullptr;
    Common::PaddingCallbackPromiseInfo(env, paras.callback, asynccallbackinfo->info, promise);

    napi_value resourceName = nullptr;
    napi_create_string_latin1(env, "Unsubscribe", NAPI_AUTO_LENGTH, &resourceName);

    // Asynchronous function call
    napi_create_async_work(env,
        nullptr,
        resourceName,
        [](napi_env env, void *data) {
            ANS_LOGI("Unsubscribe napi_create_async_work start");
            AsyncCallbackInfoUnsubscribe *asynccallbackinfo = (AsyncCallbackInfoUnsubscribe *)data;

            asynccallbackinfo->info.errorCode =
                NotificationHelper::UnSubscribeNotification(*(asynccallbackinfo->objectInfo));
        },
        [](napi_env env, napi_status status, void *data) {
            ANS_LOGI("Unsubscribe napi_create_async_work end");
            AsyncCallbackInfoUnsubscribe *asynccallbackinfo = (AsyncCallbackInfoUnsubscribe *)data;

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