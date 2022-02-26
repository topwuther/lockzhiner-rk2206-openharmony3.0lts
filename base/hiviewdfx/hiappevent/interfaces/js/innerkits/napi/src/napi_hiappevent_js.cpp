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
#include "hiappevent_base.h"
#include "hilog/log.h"
#include "napi/native_api.h"
#include "napi/native_node_api.h"
#include "napi_hiappevent_build.h"
#include "napi_hiappevent_config.h"
#include "napi_hiappevent_init.h"
#include "napi_hiappevent_write.h"

using namespace OHOS::HiviewDFX;

namespace {
constexpr HiLogLabel LABEL = { LOG_CORE, HIAPPEVENT_DOMAIN, "HiAppEvent_NAPI" };
constexpr int CONFIGURE_FUNC_MAX_PARAM_NUM = 1;
constexpr int WRITE_FUNC_MAX_PARAM_NUM = 4;
}

static napi_value Write(napi_env env, napi_callback_info info)
{
    size_t paramNum = WRITE_FUNC_MAX_PARAM_NUM;
    napi_value params[WRITE_FUNC_MAX_PARAM_NUM] = {0};
    napi_value thisArg = nullptr;
    void *data = nullptr;
    NAPI_CALL(env, napi_get_cb_info(env, info, &paramNum, params, &thisArg, &data));

    HiAppEventAsyncContext* asyncContext = new HiAppEventAsyncContext {
        .env = env,
        .asyncWork = nullptr,
        .deferred = nullptr,
    };

    // set event file dirtory
    SetStorageDirFromNapi(env, info);

    // build AppEventPack object and check event
    int32_t result = 0;
    asyncContext->appEventPack = BuildAppEventPackFromNapi(env, params, paramNum, result);
    asyncContext->result = result;

    // set callback function if it exists
    if (paramNum == WRITE_FUNC_MAX_PARAM_NUM) {
        napi_valuetype lastParamType;
        napi_typeof(env, params[paramNum - 1], &lastParamType);
        if (lastParamType == napi_valuetype::napi_function) {
            napi_create_reference(env, params[paramNum - 1], 1, &asyncContext->callback);
        }
    } else if (paramNum > WRITE_FUNC_MAX_PARAM_NUM) {
        HiLog::Error(LABEL, "invalid number=%{public}d of params.", (int)paramNum);
        asyncContext->result = ErrorCode::ERROR_INVALID_PARAM_NUM_JS;
    }

    // set promise object if callback function is null
    napi_value promise = nullptr;
    napi_get_undefined(env, &promise);
    if (asyncContext->callback == nullptr) {
        napi_create_promise(env, &asyncContext->deferred, &promise);
    }

    WriteEventFromNapi(env, asyncContext);
    return promise;
}

static napi_value Configure(napi_env env, napi_callback_info info)
{
    size_t paramNum = CONFIGURE_FUNC_MAX_PARAM_NUM;
    napi_value params[CONFIGURE_FUNC_MAX_PARAM_NUM] = {0};
    napi_value thisArg = nullptr;
    void *data = nullptr;
    NAPI_CALL(env, napi_get_cb_info(env, info, &paramNum, params, &thisArg, &data));

    napi_value result = nullptr;
    napi_get_boolean(env, false, &result);
    if (paramNum < CONFIGURE_FUNC_MAX_PARAM_NUM) {
        HiLog::Error(LABEL, "failed to configure because the number of params must be at least 1.");
        return result;
    }

    napi_get_boolean(env, ConfigureFromNapi(env, params[0]), &result);
    return result;
}

EXTERN_C_START
static napi_value Init(napi_env env, napi_value exports)
{
    napi_property_descriptor desc[] = {
        DECLARE_NAPI_FUNCTION("write", Write),
        DECLARE_NAPI_FUNCTION("configure", Configure)
    };
    NAPI_CALL(env, napi_define_properties(env, exports, sizeof(desc) / sizeof(napi_property_descriptor), desc));

    // init EventType class, Event class and Param class
    InitNapiClass(env, exports);

    return exports;
}
EXTERN_C_END

static napi_module _module = {
    .nm_version = 1,
    .nm_flags = 0,
    .nm_filename = nullptr,
    .nm_register_func = Init,
    .nm_modname = "hiappevent",
    .nm_priv = ((void *)0),
    .reserved = {0}
};

extern "C" __attribute__((constructor)) void RegisterModule(void)
{
    napi_module_register(&_module);
}