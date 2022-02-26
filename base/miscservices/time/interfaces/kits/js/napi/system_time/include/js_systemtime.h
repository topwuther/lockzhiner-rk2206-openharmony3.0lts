/*
 * Copyright (C) 2021 Huawei Device Co., Ltd.
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
#ifndef N_JS_SYSTEMTIME_H
#define N_JS_SYSTEMTIME_H

#include <string>
#include "napi/native_api.h"
#include "napi/native_node_api.h"
#include "js_native_api.h"

namespace OHOS {
namespace MiscServicesNapi {
namespace {
const int NONE_PARAMETER = 0;
const int ONE_PARAMETER = 1;
const int TWO_PARAMETERS = 2;
const int THREE_PARAMETERS = 3;
const int SET_TIME_MAX_PARA = 2;
const int SET_TIMEZONE_MAX_PARA = 2;
const int MAX_TIME_ZONE_ID = 1024;
const int NO_ERROR = 0;
const int ERROR = -1;
const int PARAM0 = 0;
const int PARAM1 = 1;
const int ARGS_TWO = 2;
}

typedef struct AsyncContext {
    napi_env env = nullptr;
    napi_async_work work = nullptr;
    int64_t time;
    std::string timeZone;
    napi_deferred deferred = nullptr;
    napi_ref callbackRef = nullptr;
    bool isCallback = false;
    bool isOK = false;
    int errorCode = NO_ERROR;
} AsyncContext;

struct TimeCallbackPromiseInfo {
    napi_ref callback = nullptr;
    napi_deferred deferred = nullptr;
    bool isCallback = false;
    int errorCode = NO_ERROR;
};
} // MiscServicesNapi
} // OHOS
#endif