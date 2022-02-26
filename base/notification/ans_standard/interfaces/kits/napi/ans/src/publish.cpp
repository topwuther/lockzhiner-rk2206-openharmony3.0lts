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
#include "publish.h"
#include "notification_basic_content.h"
#include "notification_constant.h"
#include "notification_content.h"
#include "notification_normal_content.h"

namespace OHOS {
namespace NotificationNapi {
static const int32_t PUBLISH_NOTIFICATION_MAX = 2;

struct AsyncCallbackInfoPublish {
    napi_env env = nullptr;
    napi_async_work asyncWork = nullptr;
    NotificationRequest request;
    std::string lable;
    CallbackPromiseInfo info;
};

struct ParametersInfoPublish {
    std::string lable;
    NotificationRequest request;
    napi_ref callback = nullptr;
};

napi_value GetCallback(const napi_env &env, const napi_value &value, ParametersInfoPublish &params)
{
    ANS_LOGI("enter");

    napi_valuetype valuetype = napi_undefined;
    NAPI_CALL(env, napi_typeof(env, value, &valuetype));
    NAPI_ASSERT(env, valuetype == napi_function, "Wrong argument type. Function expected.");
    napi_create_reference(env, value, 1, &params.callback);
    ANS_LOGI("end");
    return Common::NapiGetNull(env);
}

napi_value ParseParameters(const napi_env &env, const napi_callback_info &info, ParametersInfoPublish &params)
{
    ANS_LOGI("enter");

    size_t argc = PUBLISH_NOTIFICATION_MAX;
    napi_value argv[PUBLISH_NOTIFICATION_MAX] = {nullptr};
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, NULL, NULL));
    NAPI_ASSERT(env, argc >= 1, "Wrong number of arguments");

    napi_valuetype valuetype = napi_undefined;
    NAPI_CALL(env, napi_typeof(env, argv[0], &valuetype));
    NAPI_ASSERT(env, valuetype == napi_object, "Wrong argument type. Object expected.");

    // argv[0] : NotificationRequest
    if (Common::GetNotificationRequest(env, argv[0], params.request) == nullptr) {
        return nullptr;
    }

    // argv[1] : callback
    if (argc >= PUBLISH_NOTIFICATION_MAX) {
        if (GetCallback(env, argv[1], params) == nullptr) {
            return nullptr;
        }
    }

    ANS_LOGI("end");
    return Common::NapiGetNull(env);
}

napi_value Publish(napi_env env, napi_callback_info info)
{
    ANS_LOGI("enter");

    ParametersInfoPublish params;
    if (ParseParameters(env, info, params) == nullptr) {
        return Common::JSParaError(env, params.callback);
    }

    napi_value promise = nullptr;
    AsyncCallbackInfoPublish *asynccallbackinfo =
        new (std::nothrow) AsyncCallbackInfoPublish {.env = env, .asyncWork = nullptr};
    if (!asynccallbackinfo) {
        return Common::JSParaError(env, params.callback);
    }
    asynccallbackinfo->lable = params.lable;
    asynccallbackinfo->request = params.request;
    Common::PaddingCallbackPromiseInfo(env, params.callback, asynccallbackinfo->info, promise);

    napi_value resourceName = nullptr;
    napi_create_string_latin1(env, "publish", NAPI_AUTO_LENGTH, &resourceName);

    napi_create_async_work(env,
        nullptr,
        resourceName,
        [](napi_env env, void *data) {
            ANS_LOGI("Publish napi_create_async_work start");
            AsyncCallbackInfoPublish *asynccallbackinfo = (AsyncCallbackInfoPublish *)data;
            ANS_LOGI("Publish napi_create_async_work start notificationId = %{public}d, contentType = "
                     "%{public}d",
                asynccallbackinfo->request.GetNotificationId(),
                asynccallbackinfo->request.GetContent()->GetContentType());

            asynccallbackinfo->info.errorCode =
                NotificationHelper::PublishNotification(asynccallbackinfo->lable, asynccallbackinfo->request);
        },
        [](napi_env env, napi_status status, void *data) {
            ANS_LOGI("Publish napi_create_async_work complete start");
            AsyncCallbackInfoPublish *asynccallbackinfo = (AsyncCallbackInfoPublish *)data;

            Common::ReturnCallbackPromise(env, asynccallbackinfo->info, Common::NapiGetNull(env));

            if (asynccallbackinfo->info.callback != nullptr) {
                napi_delete_reference(env, asynccallbackinfo->info.callback);
            }

            napi_delete_async_work(env, asynccallbackinfo->asyncWork);
            if (asynccallbackinfo) {
                delete asynccallbackinfo;
                asynccallbackinfo = nullptr;
            }
            ANS_LOGI("Publish napi_create_async_work complete end");
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