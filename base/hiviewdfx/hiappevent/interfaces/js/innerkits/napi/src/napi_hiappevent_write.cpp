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
#include "napi_hiappevent_write.h"

#include "hiappevent_write.h"

using namespace OHOS::HiviewDFX;

namespace OHOS {
namespace HiviewDFX {
namespace {
constexpr int CALLBACK_FUNC_PARAM_NUM = 2;
constexpr int SUCCESS_FLAG = 0;
}

void WriteEventFromNapi(const napi_env env, HiAppEventAsyncContext* asyncContext)
{
    napi_value resource = nullptr;
    napi_create_string_utf8(env, "JSHiAppEventWrite", NAPI_AUTO_LENGTH, &resource);

    napi_create_async_work(
        env, nullptr, resource,
        [](napi_env env, void* data) {
            HiAppEventAsyncContext* asyncContext = (HiAppEventAsyncContext*)data;
            if (asyncContext->appEventPack != nullptr && asyncContext->result >= SUCCESS_FLAG) {
                WriterEvent(asyncContext->appEventPack);
            }
        },
        [](napi_env env, napi_status status, void* data) {
            HiAppEventAsyncContext* asyncContext = (HiAppEventAsyncContext*)data;
            napi_value result[CALLBACK_FUNC_PARAM_NUM] = {0};
            if (asyncContext->result == SUCCESS_FLAG) {
                napi_get_undefined(env, &result[0]);
                napi_create_int32(env, asyncContext->result, &result[1]);
            } else {
                napi_create_object(env, &result[0]);
                napi_value errCode = nullptr;
                napi_create_int32(env, asyncContext->result, &errCode);
                napi_set_named_property(env, result[0], "code", errCode);
                napi_get_undefined(env, &result[1]);
            }

            if (asyncContext->deferred) {
                if (asyncContext->result == SUCCESS_FLAG) {
                    napi_resolve_deferred(env, asyncContext->deferred, result[1]);
                } else {
                    napi_reject_deferred(env, asyncContext->deferred, result[0]);
                }
            } else {
                napi_value callback = nullptr;
                napi_get_reference_value(env, asyncContext->callback, &callback);
                napi_value retValue = nullptr;
                napi_call_function(env, nullptr, callback, CALLBACK_FUNC_PARAM_NUM, result, &retValue);
                napi_delete_reference(env, asyncContext->callback);
            }

            napi_delete_async_work(env, asyncContext->asyncWork);
            delete asyncContext;
        },
        (void*)asyncContext, &asyncContext->asyncWork);
    napi_queue_async_work(env, asyncContext->asyncWork);
}
}
}