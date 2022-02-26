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
#ifndef OHOS_JS_UTIL_H
#define OHOS_JS_UTIL_H

#include <cstdint>
#include <change_notification.h>
#include <map>
#include "napi/native_common.h"
#include "napi/native_api.h"
#include "napi/native_node_api.h"
#include "types.h"

#define DECLARE_NAPI_METHOD(name, func) { name, 0, func, 0, 0, 0, napi_default, 0 }
namespace OHOS::DistributedData {
class JSUtil {
public:
    static constexpr int32_t MAX_ARGC = 6;
    static constexpr int32_t MAX_NUMBER_BYTES = 8;
    static constexpr int32_t MAX_LEN = 4096;

    static std::string Convert2String(napi_env env, napi_value jsString);
    static napi_value Convert2JSString(napi_env env, const std::string &cString);
    static napi_value Convert2JSString(napi_env env, const std::vector<uint8_t> &key);
    static napi_value Convert2JSValue(napi_env env, const std::vector<uint8_t> &data);
    static napi_value Convert2JSNumber(napi_env env, const std::vector<uint8_t> &data);
    static napi_value Convert2JSNotification(napi_env env, const DistributedKv::ChangeNotification &notification);
    static napi_value Convert2JSTupleArray(napi_env env, std::map<std::string, int32_t> &data);
    static DistributedKv::Options Convert2Options(napi_env env, napi_value jsOptions);
    static std::vector<uint8_t> Convert2Vector(napi_env env, napi_value jsValue);
    static std::vector<uint8_t> ConvertUint8Array2Vector(napi_env env, napi_value jsValue);
    static std::vector<uint8_t> ConvertString2Vector(napi_env env, napi_value jsValue);
    static std::vector<uint8_t> ConvertNumber2Vector(napi_env env, napi_value jsValue);
    static std::vector<uint8_t> ConvertBool2Vector(napi_env env, napi_value jsValue);
    static std::vector<std::string> Convert2StringArray(napi_env env, napi_value jsValue);
private:
    static napi_value GetJSEntries(napi_env env, const std::list<DistributedKv::Entry> &entries);
    enum ValueType : uint8_t {
        /** Indicates that the value type is string. */
        STRING = 0,
        /** Indicates that the value type is int. */
        INTEGER = 1,
        /** Indicates that the value type is float. */
        FLOAT = 2,
        /** Indicates that the value type is byte array. */
        BYTE_ARRAY = 3,
        /** Indicates that the value type is boolean. */
        BOOLEAN = 4,
        /** Indicates that the value type is double. */
        DOUBLE = 5,

        INVALID = 255,
    };
    static constexpr int32_t TYPE_POS = 0;
    static constexpr int32_t DATA_POS = TYPE_POS + 1;
    static constexpr int32_t TUPLE_SIZE = 2;
};
}
#endif // OHOS_JS_UTIL_H
