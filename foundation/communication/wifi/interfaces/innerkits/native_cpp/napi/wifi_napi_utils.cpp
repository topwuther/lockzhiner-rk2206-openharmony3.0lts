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

#include "wifi_napi_utils.h"
#include "securec.h"
#include "wifi_logger.h"

namespace OHOS {
namespace Wifi {
DEFINE_WIFILOG_LABEL("WifiNAPIUtils");

napi_value UndefinedNapiValue(const napi_env& env)
{
    napi_value result;
    napi_get_undefined(env, &result);
    return result;
}

napi_value JsObjectToString(const napi_env& env, const napi_value& object,
    const char* fieldStr, const int bufLen, std::string& fieldRef)
{
    bool hasProperty = false;
    NAPI_CALL(env, napi_has_named_property(env, object, fieldStr, &hasProperty));
    if (hasProperty) {
        napi_value field;
        napi_valuetype valueType;

        napi_get_named_property(env, object, fieldStr, &field);
        NAPI_CALL(env, napi_typeof(env, field, &valueType));
        NAPI_ASSERT(env, valueType == napi_string, "Wrong argument type. String expected.");
        if (bufLen <= 0) {
            goto error;
        }
        char *buf = (char *)malloc(bufLen);
        if (buf == nullptr) {
            WIFI_LOGE("[Wifi Js] js object to str malloc failed");
            goto error;
        }
        (void)memset_s(buf, bufLen, 0, bufLen);
        size_t result = 0;
        NAPI_CALL(env, napi_get_value_string_utf8(env, field, buf, bufLen, &result));
        fieldRef = buf;
        free(buf);
        buf = nullptr;
    } else {
        WIFI_LOGW("[Wifi Js] wifi napi js to str no property: %{public}s", fieldStr);
    }

error:
    return UndefinedNapiValue(env);
}

napi_value JsObjectToInt(const napi_env& env, const napi_value& object, const char* fieldStr, int& fieldRef)
{
    bool hasProperty = false;
    NAPI_CALL(env, napi_has_named_property(env, object, fieldStr, &hasProperty));
    if (hasProperty) {
        napi_value field;
        napi_valuetype valueType;

        napi_get_named_property(env, object, fieldStr, &field);
        NAPI_CALL(env, napi_typeof(env, field, &valueType));
        NAPI_ASSERT(env, valueType == napi_number, "Wrong argument type. Number expected.");
        napi_get_value_int32(env, field, &fieldRef);
    } else {
        WIFI_LOGW("[Wifi Js] wifi napi js to int no property: %{public}s", fieldStr);
    }
    return UndefinedNapiValue(env);
}

napi_value JsObjectToBool(const napi_env& env, const napi_value& object, const char* fieldStr, bool& fieldRef)
{
    bool hasProperty = false;
    NAPI_CALL(env, napi_has_named_property(env, object, fieldStr, &hasProperty));
    if (hasProperty) {
        napi_value field;
        napi_valuetype valueType;

        napi_get_named_property(env, object, fieldStr, &field);
        NAPI_CALL(env, napi_typeof(env, field, &valueType));
        NAPI_ASSERT(env, valueType == napi_boolean, "Wrong argument type. Bool expected.");
        napi_get_value_bool(env, field, &fieldRef);
    } else {
        WIFI_LOGW("[Wifi Js] wifi napi js to bool no property: %{public}s", fieldStr);
    }
    return UndefinedNapiValue(env);
}

void SetValueUtf8String(const napi_env& env, const char* fieldStr, const char* str, napi_value& result)
{
    napi_value value;
    napi_create_string_utf8(env, str, NAPI_AUTO_LENGTH, &value);
    napi_set_named_property(env, result, fieldStr, value);
}

void SetValueInt32(const napi_env& env, const char* fieldStr, const int intValue, napi_value& result)
{
    napi_value value;
    napi_create_int32(env, intValue, &value);
    napi_set_named_property(env, result, fieldStr, value);
}

void SetValueInt64(const napi_env& env, const char* fieldStr, const int64_t intValue, napi_value& result)
{
    napi_value value;
    napi_create_int64(env, intValue, &value);
    napi_set_named_property(env, result, fieldStr, value);
}

}  // namespace Wifi
}  // namespace OHOS
