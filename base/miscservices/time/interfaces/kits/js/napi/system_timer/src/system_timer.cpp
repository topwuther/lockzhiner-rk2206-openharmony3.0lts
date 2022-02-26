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

#include <uv.h>
#include <memory>
#include <string>
#include <vector>
#include "timer_type.h"
#include "want_agent.h"
#include "securec.h"
#include "system_timer.h"

namespace OHOS {
namespace MiscServicesNapi {
namespace {
const int NO_ERROR = 0;
const int ERROR = -1;
const int CREATE_MAX_PARA = 2;
const int START_MAX_PARA = 3;
const int STOP_MAX_PARA = 2;
const int DESTROY_MAX_PARA = 2;
const int ARGS_TWO = 2;
const int PARAM0 = 0;
const int PARAM1 = 1;
}

struct CallbackPromiseInfo {
    napi_ref callback = nullptr;
    napi_deferred deferred = nullptr;
    bool isCallback = false;
    int errorCode = 0;
};

struct ReceiveDataWorker {
    napi_env env = nullptr;
    napi_ref ref = 0;
};

struct AsyncCallbackInfoCreate {
    napi_env env = nullptr;
    napi_async_work asyncWork = nullptr;
    napi_ref callback = nullptr;
    napi_deferred deferred = nullptr;
    std::shared_ptr<ITimerInfoInstance> iTimerInfoInstance = nullptr;
    uint64_t timerId = 0;
    bool isCallback = false;
    int errorCode = NO_ERROR;
};

struct AsyncCallbackInfoStart {
    napi_env env = nullptr;
    napi_async_work asyncWork = nullptr;
    napi_ref callback = nullptr;
    napi_deferred deferred = nullptr;
    uint64_t timerId = 0;
    uint64_t triggerTime = 0;
    bool isOK = false;
    bool isCallback = false;
    int errorCode = NO_ERROR;
};

struct AsyncCallbackInfoStop {
    napi_env env = nullptr;
    napi_async_work asyncWork = nullptr;
    napi_ref callback = nullptr;
    napi_deferred deferred = nullptr;
    uint64_t timerId = 0;
    bool isOK = false;
    bool isCallback = false;
    int errorCode = NO_ERROR;
};

struct AsyncCallbackInfoDestroy {
    napi_env env = nullptr;
    napi_async_work asyncWork = nullptr;
    napi_ref callback = nullptr;
    napi_deferred deferred = nullptr;
    uint64_t timerId = 0;
    bool isOK = false;
    bool isCallback = false;
    int errorCode = NO_ERROR;
};

static std::vector<AsyncCallbackInfoCreate *> asyncCallbackInfoCreateInfo;

napi_value NapiGetNull(napi_env env)
{
    napi_value result = nullptr;
    napi_get_null(env, &result);
    return result;
}

napi_value GetCallbackErrorValue(napi_env env, int errCode)
{
    napi_value result = nullptr;
    napi_value eCode = nullptr;
    NAPI_CALL(env, napi_create_int32(env, errCode, &eCode));
    NAPI_CALL(env, napi_create_object(env, &result));
    NAPI_CALL(env, napi_set_named_property(env, result, "code", eCode));
    return result;
}

void SetPromise(const napi_env &env, const napi_deferred &deferred, const napi_value &result)
{
    napi_resolve_deferred(env, deferred, result);
}

void SetCallback(const napi_env &env, const napi_ref &callbackIn, const int &errorCode, const napi_value &result)
{
    napi_value undefined = nullptr;
    napi_get_undefined(env, &undefined);

    napi_value callback = nullptr;
    napi_value resultout = nullptr;
    napi_get_reference_value(env, callbackIn, &callback);
    napi_value results[ARGS_TWO] = {0};
    results[PARAM0] = GetCallbackErrorValue(env, errorCode);
    results[PARAM1] = result;
    NAPI_CALL_RETURN_VOID(env, napi_call_function(env, undefined, callback, ARGS_TWO, &results[PARAM0], &resultout));
}

napi_value JSParaError(const napi_env &env, const napi_ref &callback)
{
    if (callback) {
        return NapiGetNull(env);
    } else {
        napi_value promise = nullptr;
        napi_deferred deferred = nullptr;
        napi_create_promise(env, &deferred, &promise);
        SetPromise(env, deferred, NapiGetNull(env));
        return promise;
    }
}

void ReturnCallbackPromise(const napi_env &env, const CallbackPromiseInfo &info, const napi_value &result)
{
    if (info.isCallback) {
        SetCallback(env, info.callback, info.errorCode, result);
    } else {
        SetPromise(env, info.deferred, result);
    }
}

ITimerInfoInstance::ITimerInfoInstance()
    : callbackInfo_ {}
{}

ITimerInfoInstance::~ITimerInfoInstance()
{}

void ITimerInfoInstance::OnTrigger()
{
    if (callbackInfo_.ref == nullptr) {
        return;
    }

    uv_loop_s *loop = nullptr;
#if NAPI_VERSION >= 2
    napi_get_uv_event_loop(callbackInfo_.env, &loop);
#endif  // NAPI_VERSION >= 2

    ReceiveDataWorker *dataWorker = new (std::nothrow) ReceiveDataWorker();
    if (!dataWorker) {
        return;
    }
    dataWorker->env = callbackInfo_.env;
    dataWorker->ref = callbackInfo_.ref;

    uv_work_t *work = new (std::nothrow) uv_work_t;
    if (!work) {
        delete dataWorker;
        return;
    }
    if (!loop) {
        delete dataWorker;
        delete work;
        return;
    }
    work->data = (void *)dataWorker;
    uv_queue_work(loop,
        work,
        [](uv_work_t *work) {},
        [](uv_work_t *work, int status) {
            ReceiveDataWorker *dataWorkerData = (ReceiveDataWorker *)work->data;
            if (dataWorkerData == nullptr) {
                return;
            }

            SetCallback(dataWorkerData->env,
                        dataWorkerData->ref,
                        NO_ERROR,
                        NapiGetNull(dataWorkerData->env));
            delete dataWorkerData;
            dataWorkerData = nullptr;
            delete work;
            work = nullptr;
        });
}

void ITimerInfoInstance::SetCallbackInfo(const napi_env &env, const napi_ref &ref)
{
    callbackInfo_.env = env;
    callbackInfo_.ref = ref;
}

void ITimerInfoInstance::SetType(const int &_type)
{
    type = _type;
}

void ITimerInfoInstance::SetRepeat(bool _repeat)
{
    repeat = _repeat;
}
void ITimerInfoInstance::SetInterval(const uint64_t &_interval)
{
    interval = _interval;
}
void ITimerInfoInstance::SetWantAgent(std::shared_ptr<OHOS::Notification::WantAgent::WantAgent> _wantAgent)
{
    wantAgent = _wantAgent;
}

napi_value GetTimerOptions(const napi_env &env, const napi_value &value,
    std::shared_ptr<ITimerInfoInstance> &iTimerInfoInstance)
{
    napi_valuetype valuetype = napi_undefined;
    napi_value result = nullptr;
    OHOS::Notification::WantAgent::WantAgent *wantAgent = nullptr;
    bool hasProperty = false;

    // type: number
    int type = 0;
    NAPI_CALL(env, napi_has_named_property(env, value, "type", &hasProperty));
    NAPI_ASSERT(env, hasProperty, "type expected.");
    napi_get_named_property(env, value, "type", &result);
    NAPI_CALL(env, napi_typeof(env, result, &valuetype));
    NAPI_ASSERT(env, valuetype == napi_number, "Wrong argument type. Number expected.");
    napi_get_value_int32(env, result, &type);
    iTimerInfoInstance->SetType(type);

    // repeat: boolean
    bool repeat = false;
    NAPI_CALL(env, napi_has_named_property(env, value, "repeat", &hasProperty));
    NAPI_ASSERT(env, hasProperty, "repeat expected.");
    napi_get_named_property(env, value, "repeat", &result);
    NAPI_CALL(env, napi_typeof(env, result, &valuetype));
    NAPI_ASSERT(env, valuetype == napi_boolean, "Wrong argument type. Bool expected.");
    napi_get_value_bool(env, result, &repeat);
    iTimerInfoInstance->SetRepeat(repeat);

    // interval?: number
    int64_t interval = 0;
    NAPI_CALL(env, napi_has_named_property(env, value, "interval", &hasProperty));
    if (hasProperty) {
        napi_get_named_property(env, value, "interval", &result);
        NAPI_CALL(env, napi_typeof(env, result, &valuetype));
        NAPI_ASSERT(env, valuetype == napi_number, "Wrong argument type. Number expected.");
        napi_get_value_int64(env, result, &interval);
        NAPI_ASSERT(env, interval >= 0, "Wrong argument number. Positive number expected.");
        iTimerInfoInstance->SetInterval((uint64_t)interval);
    }

    // wantAgent?: WantAgent
    NAPI_CALL(env, napi_has_named_property(env, value, "wantAgent", &hasProperty));
    if (hasProperty) {
        napi_get_named_property(env, value, "wantAgent", &result);
        NAPI_CALL(env, napi_typeof(env, result, &valuetype));
        NAPI_ASSERT(env, valuetype == napi_object, "Wrong argument type. Object expected.");
        napi_unwrap(env, result, (void **)&wantAgent);
        if (wantAgent == nullptr) {
            return nullptr;
        }
        std::shared_ptr<OHOS::Notification::WantAgent::WantAgent> sWantAgent = 
            std::make_shared<OHOS::Notification::WantAgent::WantAgent>(*wantAgent);
        iTimerInfoInstance->SetWantAgent(sWantAgent);
    }

    // callback?: () => void
    NAPI_CALL(env, napi_has_named_property(env, value, "callback", &hasProperty));
    if (hasProperty) {
        napi_get_named_property(env, value, "callback", &result);
        NAPI_CALL(env, napi_typeof(env, result, &valuetype));
        NAPI_ASSERT(env, valuetype == napi_function, "Wrong argument type. Function expected.");
        napi_ref onTriggerCallback;
        napi_create_reference(env, result, 1, &onTriggerCallback);
        iTimerInfoInstance->SetCallbackInfo(env, onTriggerCallback);
    }
    return NapiGetNull(env);
}

napi_value ParseParametersByCreateTimer(const napi_env &env, const napi_value (&argv)[CREATE_MAX_PARA],
    const size_t &argc, std::shared_ptr<ITimerInfoInstance> &iTimerInfoInstance, napi_ref &callback)
{
    NAPI_ASSERT(env, argc >= CREATE_MAX_PARA - 1, "Wrong number of arguments");
    napi_valuetype valuetype = napi_undefined;

    // argv[0]: TimerOptions
    NAPI_CALL(env, napi_typeof(env, argv[0], &valuetype));
    NAPI_ASSERT(env, valuetype == napi_object, "Wrong argument type. Object expected.");
    if (GetTimerOptions(env, argv[0], iTimerInfoInstance) == nullptr) {
        return nullptr;
    }

    // argv[1]:callback
    if (argc >= CREATE_MAX_PARA) {
        NAPI_CALL(env, napi_typeof(env, argv[1], &valuetype));
        NAPI_ASSERT(env, valuetype == napi_function, "Wrong argument type. Function expected.");
        napi_create_reference(env, argv[1], 1, &callback);
    }

    return NapiGetNull(env);
}

void PaddingAsyncCallbackInfoIsByCreateTimer(
    const napi_env &env, AsyncCallbackInfoCreate *&asynccallbackinfo, const napi_ref &callback, napi_value &promise)
{
    if (callback) {
        asynccallbackinfo->callback = callback;
        asynccallbackinfo->isCallback = true;
    } else {
        napi_deferred deferred = nullptr;
        NAPI_CALL_RETURN_VOID(env, napi_create_promise(env, &deferred, &promise));
        asynccallbackinfo->deferred = deferred;
        asynccallbackinfo->isCallback = false;
    }
}

napi_value CreateTimer(napi_env env, napi_callback_info info)
{
    size_t argc = CREATE_MAX_PARA;
    napi_value argv[CREATE_MAX_PARA] = {0};
    napi_value thisVar = nullptr;
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, &thisVar, NULL));
    std::shared_ptr<ITimerInfoInstance> iTimerInfoInstance = std::make_shared<ITimerInfoInstance>();
    napi_ref callback = nullptr;
    if (ParseParametersByCreateTimer(env, argv, argc, iTimerInfoInstance, callback) == nullptr) {
        return JSParaError(env, callback);
    }
    AsyncCallbackInfoCreate *asynccallbackinfo = new (std::nothrow) AsyncCallbackInfoCreate {
        .env = env,
        .asyncWork = nullptr,
        .iTimerInfoInstance = iTimerInfoInstance
    };
    if (!asynccallbackinfo) {
        return JSParaError(env, callback);
    }
    napi_value promise = nullptr;
    PaddingAsyncCallbackInfoIsByCreateTimer(env, asynccallbackinfo, callback, promise);
    napi_value resourceName = nullptr;
    napi_create_string_latin1(env, "createTimer", NAPI_AUTO_LENGTH, &resourceName);
    napi_create_async_work(env,
        nullptr,
        resourceName,
        [](napi_env env, void *data) {
            AsyncCallbackInfoCreate *asynccallbackinfo = (AsyncCallbackInfoCreate *)data;
            asynccallbackinfo->timerId =
                TimeServiceClient::GetInstance()->CreateTimer(asynccallbackinfo->iTimerInfoInstance);
            if (asynccallbackinfo->timerId > 0) {
                asyncCallbackInfoCreateInfo.emplace_back(asynccallbackinfo);
            }
        },
        [](napi_env env, napi_status status, void *data) {
            AsyncCallbackInfoCreate *asynccallbackinfo = (AsyncCallbackInfoCreate *)data;
            CallbackPromiseInfo info;
            info.isCallback = asynccallbackinfo->isCallback;
            info.callback = asynccallbackinfo->callback;
            info.deferred = asynccallbackinfo->deferred;
            info.errorCode = asynccallbackinfo->errorCode;
            napi_value result = nullptr;
            napi_create_int64(env, asynccallbackinfo->timerId, &result);
            ReturnCallbackPromise(env, info, result);
            napi_delete_async_work(env, asynccallbackinfo->asyncWork);
        },
        (void *)asynccallbackinfo,
        &asynccallbackinfo->asyncWork);
    NAPI_CALL(env, napi_queue_async_work(env, asynccallbackinfo->asyncWork));
    if (asynccallbackinfo->isCallback) {
        return NapiGetNull(env);
    } else {
        return promise;
    }
}

napi_value ParseParametersByStartTimer(const napi_env &env, const napi_value (&argv)[START_MAX_PARA],
    const size_t &argc, uint64_t &uintTimerId, uint64_t &uintTriggerTime, napi_ref &callback)
{
    NAPI_ASSERT(env, argc >= START_MAX_PARA - 1, "Wrong number of arguments");
    napi_valuetype valuetype = napi_undefined;

    // argv[0]: timer
    NAPI_CALL(env, napi_typeof(env, argv[0], &valuetype));
    NAPI_ASSERT(env, valuetype == napi_number, "Wrong argument type. Number expected.");
    int64_t timerId = 0;
    napi_get_value_int64(env, argv[0], &timerId);
    NAPI_ASSERT(env, timerId >= 0, "Wrong argument timer. Positive number expected.");
    uintTimerId = (uint64_t)timerId;

    // argv[1]: triggerTime
    NAPI_CALL(env, napi_typeof(env, argv[1], &valuetype));
    NAPI_ASSERT(env, valuetype == napi_number, "Wrong argument type. Number expected.");
    int64_t triggerTime = 0;
    napi_get_value_int64(env, argv[1], &triggerTime);
    NAPI_ASSERT(env, triggerTime >= 0, "Wrong argument triggerTime. Positive number expected.");
    uintTriggerTime = static_cast<uint64_t>(triggerTime);

    // argv[2]:callback
    if (argc >= START_MAX_PARA) {
        NAPI_CALL(env, napi_typeof(env, argv[TWO_PARAMETERS], &valuetype));
        NAPI_ASSERT(env, valuetype == napi_function, "Wrong argument type. Function expected.");
        napi_create_reference(env, argv[TWO_PARAMETERS], 1, &callback);
    }

    return NapiGetNull(env);
}

void PaddingAsyncCallbackInfoIsByStartTimer(const napi_env &env,
    AsyncCallbackInfoStart *&asynccallbackinfo,
    const napi_ref &callback,
    napi_value &promise)
{
    if (callback) {
        asynccallbackinfo->callback = callback;
        asynccallbackinfo->isCallback = true;
    } else {
        napi_deferred deferred = nullptr;
        NAPI_CALL_RETURN_VOID(env, napi_create_promise(env, &deferred, &promise));
        asynccallbackinfo->deferred = deferred;
        asynccallbackinfo->isCallback = false;
    }
}

napi_value StartTimer(napi_env env, napi_callback_info info)
{
    size_t argc = START_MAX_PARA;
    napi_value argv[START_MAX_PARA] = {0};
    napi_value thisVar = nullptr;
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, &thisVar, NULL));

    uint64_t timerId;
    uint64_t triggerTime;
    napi_ref callback = nullptr;
    if (ParseParametersByStartTimer(env, argv, argc, timerId, triggerTime, callback) == nullptr) {
        return JSParaError(env, callback);
    }

    AsyncCallbackInfoStart *asynccallbackinfo = new (std::nothrow)AsyncCallbackInfoStart{.env = env,
        .asyncWork = nullptr,
        .timerId = timerId,
        .triggerTime = triggerTime
        };
    if (!asynccallbackinfo) {
        return JSParaError(env, callback);
    }

    napi_value promise = nullptr;
    PaddingAsyncCallbackInfoIsByStartTimer(env, asynccallbackinfo, callback, promise);

    napi_value resourceName = nullptr;
    napi_create_string_latin1(env, "startTimer", NAPI_AUTO_LENGTH, &resourceName);
    // Asynchronous function call
    napi_create_async_work(env,
        nullptr,
        resourceName,
        [](napi_env env, void *data) {
            AsyncCallbackInfoStart *asynccallbackinfo = (AsyncCallbackInfoStart *)data;

            asynccallbackinfo->isOK = TimeServiceClient::GetInstance()->StartTimer(
                asynccallbackinfo->timerId, asynccallbackinfo->triggerTime);
        },
        [](napi_env env, napi_status status, void *data) {
            AsyncCallbackInfoStart *asynccallbackinfo = (AsyncCallbackInfoStart *)data;

            if (!asynccallbackinfo->isOK) {
                asynccallbackinfo->errorCode = ERROR;
            }
            CallbackPromiseInfo info;
            info.isCallback = asynccallbackinfo->isCallback;
            info.callback = asynccallbackinfo->callback;
            info.deferred = asynccallbackinfo->deferred;
            info.errorCode = asynccallbackinfo->errorCode;

            // result: void
            napi_value result = 0;
            napi_get_null(env, &result);
            ReturnCallbackPromise(env, info, result);

            napi_delete_async_work(env, asynccallbackinfo->asyncWork);
            if (asynccallbackinfo) {
                delete asynccallbackinfo;
                asynccallbackinfo = nullptr;
            }
        },
        (void *)asynccallbackinfo,
        &asynccallbackinfo->asyncWork);

    NAPI_CALL(env, napi_queue_async_work(env, asynccallbackinfo->asyncWork));

    if (asynccallbackinfo->isCallback) {
        return NapiGetNull(env);
    } else {
        return promise;
    }
}

napi_value ParseParametersByStopTimer(const napi_env &env, const napi_value (&argv)[STOP_MAX_PARA], const size_t &argc,
    uint64_t &uintTimerId, napi_ref &callback)
{
    NAPI_ASSERT(env, argc >= STOP_MAX_PARA - 1, "Wrong number of arguments");
    napi_valuetype valuetype = napi_undefined;

    // argv[0]: timer
    NAPI_CALL(env, napi_typeof(env, argv[0], &valuetype));
    NAPI_ASSERT(env, valuetype == napi_number, "Wrong argument type. Number expected.");
    int64_t timerId = 0;
    napi_get_value_int64(env, argv[0], &timerId);
    NAPI_ASSERT(env, timerId >= 0, "Wrong argument timer. Positive number expected.");
    uintTimerId = (uint64_t)timerId;

    // argv[1]:callback
    if (argc >= STOP_MAX_PARA) {
        NAPI_CALL(env, napi_typeof(env, argv[1], &valuetype));
        NAPI_ASSERT(env, valuetype == napi_function, "Wrong argument type. Function expected.");
        napi_create_reference(env, argv[1], 1, &callback);
    }

    return NapiGetNull(env);
}

void PaddingAsyncCallbackInfoIsByStopTimer(const napi_env &env,
    AsyncCallbackInfoStop *&asynccallbackinfo,
    const napi_ref &callback,
    napi_value &promise)
{
    if (callback) {
        asynccallbackinfo->callback = callback;
        asynccallbackinfo->isCallback = true;
    } else {
        napi_deferred deferred = nullptr;
        NAPI_CALL_RETURN_VOID(env, napi_create_promise(env, &deferred, &promise));
        asynccallbackinfo->deferred = deferred;
        asynccallbackinfo->isCallback = false;
    }
}

napi_value StopTimer(napi_env env, napi_callback_info info)
{
    size_t argc = STOP_MAX_PARA;
    napi_value argv[STOP_MAX_PARA] = {0};
    napi_value thisVar = nullptr;
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, &thisVar, NULL));

    uint64_t timerId;
    napi_ref callback = nullptr;
    if (ParseParametersByStopTimer(env, argv, argc, timerId, callback) == nullptr) {
        return JSParaError(env, callback);
    }

    AsyncCallbackInfoStop *asynccallbackinfo = new (std::nothrow) AsyncCallbackInfoStop{.env = env,
            .asyncWork = nullptr,
            .timerId = timerId};
    if (!asynccallbackinfo) {
        return JSParaError(env, callback);
    }

    napi_value promise = nullptr;
    PaddingAsyncCallbackInfoIsByStopTimer(env, asynccallbackinfo, callback, promise);

    napi_value resourceName = nullptr;
    napi_create_string_latin1(env, "stopTimer", NAPI_AUTO_LENGTH, &resourceName);
    // Asynchronous function call
    napi_create_async_work(env,
        nullptr,
        resourceName,
        [](napi_env env, void *data) {
            AsyncCallbackInfoStop *asynccallbackinfo = (AsyncCallbackInfoStop *)data;
            asynccallbackinfo->isOK = TimeServiceClient::GetInstance()->StopTimer(asynccallbackinfo->timerId);
        },
        [](napi_env env, napi_status status, void *data) {
            AsyncCallbackInfoStop *asynccallbackinfo = (AsyncCallbackInfoStop *)data;

            if (!asynccallbackinfo->isOK) {
                asynccallbackinfo->errorCode = ERROR;
            }
            CallbackPromiseInfo info;
            info.isCallback = asynccallbackinfo->isCallback;
            info.callback = asynccallbackinfo->callback;
            info.deferred = asynccallbackinfo->deferred;
            info.errorCode = asynccallbackinfo->errorCode;

            // result: void
            napi_value result = 0;
            napi_get_null(env, &result);
            ReturnCallbackPromise(env, info, result);

            napi_delete_async_work(env, asynccallbackinfo->asyncWork);
            if (asynccallbackinfo) {
                delete asynccallbackinfo;
                asynccallbackinfo = nullptr;
            }
        },
        (void *)asynccallbackinfo,
        &asynccallbackinfo->asyncWork);

    NAPI_CALL(env, napi_queue_async_work(env, asynccallbackinfo->asyncWork));

    if (asynccallbackinfo->isCallback) {
        return NapiGetNull(env);
    } else {
        return promise;
    }
}

napi_value ParseParametersByDestroyTimer(const napi_env &env, const napi_value (&argv)[DESTROY_MAX_PARA],
    const size_t &argc, uint64_t &uintTimerId, napi_ref &callback)
{
    NAPI_ASSERT(env, argc >= DESTROY_MAX_PARA - 1, "Wrong number of arguments");
    napi_valuetype valuetype = napi_undefined;

    // argv[0]: timer
    NAPI_CALL(env, napi_typeof(env, argv[0], &valuetype));
    NAPI_ASSERT(env, valuetype == napi_number, "Wrong argument type. Number expected.");
    int64_t timerId = 0;
    napi_get_value_int64(env, argv[0], &timerId);
    NAPI_ASSERT(env, timerId >= 0, "Wrong argument timer. Positive number expected.");
    uintTimerId = (uint64_t)timerId;

    // argv[1]:callback
    if (argc >= DESTROY_MAX_PARA) {
        NAPI_CALL(env, napi_typeof(env, argv[1], &valuetype));
        NAPI_ASSERT(env, valuetype == napi_function, "Wrong argument type. Function expected.");
        napi_create_reference(env, argv[1], 1, &callback);
    }

    return NapiGetNull(env);
}

void PaddingAsyncCallbackInfoIsByDestroyTimer(
    const napi_env &env, AsyncCallbackInfoDestroy *&asynccallbackinfo, const napi_ref &callback, napi_value &promise)
{
    if (callback) {
        asynccallbackinfo->callback = callback;
        asynccallbackinfo->isCallback = true;
    } else {
        napi_deferred deferred = nullptr;
        NAPI_CALL_RETURN_VOID(env, napi_create_promise(env, &deferred, &promise));
        asynccallbackinfo->deferred = deferred;
        asynccallbackinfo->isCallback = false;
    }
}

napi_value DestroyTimer(napi_env env, napi_callback_info info)
{
    size_t argc = DESTROY_MAX_PARA;
    napi_value argv[DESTROY_MAX_PARA] = {0};
    napi_value thisVar = nullptr;
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, &thisVar, NULL));

    uint64_t timerId;
    napi_ref callback = nullptr;
    if (ParseParametersByDestroyTimer(env, argv, argc, timerId, callback) == nullptr) {
        return JSParaError(env, callback);
    }

    AsyncCallbackInfoDestroy *asynccallbackinfo =
        new (std::nothrow) AsyncCallbackInfoDestroy{.env = env, .asyncWork = nullptr, .timerId = timerId};
    if (!asynccallbackinfo) {
        return JSParaError(env, callback);
    }

    napi_value promise = nullptr;
    PaddingAsyncCallbackInfoIsByDestroyTimer(env, asynccallbackinfo, callback, promise);

    napi_value resourceName = nullptr;
    napi_create_string_latin1(env, "destroyTimer", NAPI_AUTO_LENGTH, &resourceName);
    // Asynchronous function call
    napi_create_async_work(env,
        nullptr,
        resourceName,
        [](napi_env env, void *data) {
            AsyncCallbackInfoDestroy *asynccallbackinfo = (AsyncCallbackInfoDestroy *)data;
            asynccallbackinfo->isOK = TimeServiceClient::GetInstance()->DestroyTimer(asynccallbackinfo->timerId);
        },
        [](napi_env env, napi_status status, void *data) {
            AsyncCallbackInfoDestroy *asynccallbackinfo = (AsyncCallbackInfoDestroy *)data;

            if (asynccallbackinfo->isOK) {
                for (auto it = asyncCallbackInfoCreateInfo.begin(); it != asyncCallbackInfoCreateInfo.end(); it++) {
                    if ((*it)->timerId == asynccallbackinfo->timerId) {
                        it = asyncCallbackInfoCreateInfo.erase(it);
                        delete (*it);
                        *it = nullptr;
                    }
                }
            } else {
                asynccallbackinfo->errorCode = ERROR;
            }
            CallbackPromiseInfo info;
            info.isCallback = asynccallbackinfo->isCallback;
            info.callback = asynccallbackinfo->callback;
            info.deferred = asynccallbackinfo->deferred;
            info.errorCode = asynccallbackinfo->errorCode;

            // result: void
            napi_value result = 0;
            napi_get_null(env, &result);
            ReturnCallbackPromise(env, info, result);

            napi_delete_async_work(env, asynccallbackinfo->asyncWork);
            if (asynccallbackinfo) {
                delete asynccallbackinfo;
                asynccallbackinfo = nullptr;
            }
        },
        (void *)asynccallbackinfo,
        &asynccallbackinfo->asyncWork);

    NAPI_CALL(env, napi_queue_async_work(env, asynccallbackinfo->asyncWork));

    if (asynccallbackinfo->isCallback) {
        return NapiGetNull(env);
    } else {
        return promise;
    }
}

napi_value SystemtimerInit(napi_env env, napi_value exports)
{
    napi_property_descriptor desc[] = {
        DECLARE_NAPI_FUNCTION("createTimer", CreateTimer),
        DECLARE_NAPI_FUNCTION("startTimer", StartTimer),
        DECLARE_NAPI_FUNCTION("stopTimer", StopTimer),
        DECLARE_NAPI_FUNCTION("destroyTimer", DestroyTimer),
    };

    NAPI_CALL(env, napi_define_properties(env, exports, sizeof(desc) / sizeof(desc[0]), desc));

    OHOS::MiscServicesNapi::TimerTypeInit(env, exports);
    return exports;
}
}  // namespace MiscServicesNapi
}  // namespace OHOS