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

#ifndef WIFI_NAPI_UTILS_H_
#define WIFI_NAPI_UTILS_H_

#include <string>
#include "napi/native_api.h"
#include "napi/native_node_api.h"

namespace OHOS {
namespace Wifi {
napi_value UndefinedNapiValue(const napi_env& env);
napi_value JsObjectToString(const napi_env& env, const napi_value& object,
    const char* fieldStr, const int bufLen, std::string& fieldRef);
napi_value JsObjectToInt(const napi_env& env, const napi_value& object, const char* fieldStr, int& fieldRef);
napi_value JsObjectToBool(const napi_env& env, const napi_value& object, const char* fieldStr, bool& fieldRef);
void SetValueUtf8String(const napi_env& env, const char* fieldStr, const char* str, napi_value& result);
void SetValueInt32(const napi_env& env, const char* fieldStr, const int intValue, napi_value& result);
void SetValueInt64(const napi_env& env, const char* fieldStr, const int64_t intValue, napi_value& result);

struct AsyncCallbackInfo {
    napi_env env;
    napi_async_work asyncWork;
    napi_deferred deferred;
    napi_ref callback[2] = { 0 };
    void *obj;
    napi_value result;
    bool isSuccess;
};

}  // namespace Wifi
}  // namespace OHOS

#endif
