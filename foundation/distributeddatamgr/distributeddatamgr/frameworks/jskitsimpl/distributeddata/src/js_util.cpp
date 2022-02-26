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
#define LOG_TAG "JSUtil"

#include "js_util.h"
#include <securec.h>
#include "log_print.h"

using namespace OHOS::DistributedKv;
namespace OHOS::DistributedData {
DistributedKv::Options JSUtil::Convert2Options(napi_env env, napi_value jsOptions)
{
    DistributedKv::Options options;
    napi_value value = nullptr;
    napi_get_named_property(env, jsOptions, "createIfMissing", &value);
    if (value != nullptr) {
        napi_get_value_bool(env, value, &options.createIfMissing);
    }
    value = nullptr;
    napi_get_named_property(env, jsOptions, "encrypt", &value);
    if (value != nullptr) {
        napi_get_value_bool(env, value, &options.encrypt);
    }
    value = nullptr;
    napi_get_named_property(env, jsOptions, "backup", &value);
    if (value != nullptr) {
        napi_get_value_bool(env, value, &options.backup);
    }
    value = nullptr;
    options.autoSync = false;
    napi_get_named_property(env, jsOptions, "autoSync", &value);
    if (value != nullptr) {
        napi_get_value_bool(env, value, &options.autoSync);
    }
    value = nullptr;
    napi_get_named_property(env, jsOptions, "kvStoreType", &value);
    if (value != nullptr) {
        int32_t storeType = DistributedKv::DEVICE_COLLABORATION;
        napi_get_value_int32(env, value, &storeType);
        options.kvStoreType = static_cast<DistributedKv::KvStoreType>(storeType);
    }
    value = nullptr;
    napi_get_named_property(env, jsOptions, "securityLevel", &value);
    if (value != nullptr) {
        napi_get_value_int32(env, value, &options.securityLevel);
    }
    return options;
}

std::string JSUtil::Convert2String(napi_env env, napi_value jsString)
{
    size_t maxLen = JSUtil::MAX_LEN;
    napi_status status = napi_get_value_string_utf8(env, jsString, NULL, 0, &maxLen);
    if (status != napi_ok) {
        GET_AND_THROW_LAST_ERROR((env));
        maxLen = JSUtil::MAX_LEN;
    }
    if (maxLen <= 0) {
        return std::string();
    }
    char *buf = new char[maxLen + 1];
    if (buf == nullptr) {
        return std::string();
    }
    size_t len = 0;
    status = napi_get_value_string_utf8(env, jsString, buf, maxLen + 1, &len);
    if (status != napi_ok) {
        GET_AND_THROW_LAST_ERROR((env));
    }
    buf[len] = 0;
    std::string value(buf);
    delete[] buf;
    return value;
}

napi_value JSUtil::Convert2JSNotification(napi_env env, const DistributedKv::ChangeNotification &notification)
{
    napi_value result = nullptr;
    napi_create_object(env, &result);
    napi_set_named_property(env, result, "deviceId", Convert2JSString(env, notification.GetDeviceId()));
    napi_set_named_property(env, result, "insertEntries", GetJSEntries(env, notification.GetInsertEntries()));
    napi_set_named_property(env, result, "updateEntries", GetJSEntries(env, notification.GetUpdateEntries()));
    napi_set_named_property(env, result, "deleteEntries", GetJSEntries(env, notification.GetDeleteEntries()));
    return result;
}

napi_value JSUtil::Convert2JSTupleArray(napi_env env, std::map<std::string, int32_t> &data)
{
    napi_value result = nullptr;
    napi_create_array_with_length(env, data.size(), &result);
    int index = 0;
    for (const auto &[key, value] : data) {
        napi_value element = nullptr;
        napi_create_array_with_length(env, TUPLE_SIZE, &element);
        napi_value jsKey = nullptr;
        napi_create_string_utf8(env, key.c_str(), key.size(), &jsKey);
        napi_set_element(env, element, 0, jsKey);
        napi_value jsValue = nullptr;
        napi_create_int32(env, value, &jsValue);
        napi_set_element(env, element, 1, jsValue);
        napi_set_element(env, result, index++, element);
    }
    return result;
}

std::vector<std::string> JSUtil::Convert2StringArray(napi_env env, napi_value jsValue)
{
    bool isArray = false;
    napi_is_array(env, jsValue, &isArray);
    NAPI_ASSERT_BASE(env, isArray, "not array", { });
    uint32_t length = 0;
    napi_get_array_length(env, jsValue, &length);
    std::vector<std::string> devices;
    for (uint32_t i = 0; i < length; ++i) {
        napi_value deviceId = nullptr;
        napi_get_element(env, jsValue, i, &deviceId);
        if (deviceId == nullptr) {
            continue;
        }
        devices.push_back(Convert2String(env, deviceId));
    }
    return devices;
}

napi_value JSUtil::Convert2JSString(napi_env env, const std::vector<uint8_t> &key)
{
    std::string realkey(key.begin(), key.end());
    napi_value jsKey = nullptr;
    napi_create_string_utf8(env, realkey.c_str(), realkey.size(), &jsKey);
    return jsKey;
}

napi_value JSUtil::Convert2JSValue(napi_env env, const std::vector<uint8_t> &data)
{
    napi_value result = nullptr;
    napi_get_undefined(env, &result);
    NAPI_ASSERT_BASE(env, data.size() > 1, "invalid data", result);
    napi_create_object(env, &result);
    napi_value jsType = nullptr;
    napi_create_int32(env, data[0], &jsType);
    napi_set_named_property(env, result, "type", jsType);
    napi_value jsValue = nullptr;
    switch (data[0]) {
        case STRING:
            napi_create_string_utf8(env, reinterpret_cast<const char *>(data.data() + DATA_POS), data.size() - DATA_POS,
                                    &jsValue);
            break;
        case BOOLEAN:
            napi_get_boolean(env, *(reinterpret_cast<const int32_t *>(data.data() + DATA_POS)), &jsValue);
            break;
        case BYTE_ARRAY: {
            uint8_t *native = nullptr;
            napi_value buffer = nullptr;
            napi_create_arraybuffer(env, data.size() - DATA_POS, reinterpret_cast<void **>(&native), &buffer);
            memcpy_s(native, data.size() - DATA_POS, data.data() + DATA_POS, data.size() - DATA_POS);
            napi_create_typedarray(env, napi_uint8_array, data.size() - DATA_POS, buffer, 0, &jsValue);
            break;
        }
        default:
            jsValue = Convert2JSNumber(env, data);
            break;
    }
    napi_set_named_property(env, result, "value", jsValue);
    return result;
}

std::vector<uint8_t> JSUtil::Convert2Vector(napi_env env, napi_value jsValue)
{
    napi_valuetype valueType;
    napi_typeof(env, jsValue, &valueType);
    switch (valueType) {
        case napi_boolean:
            return ConvertBool2Vector(env, jsValue);
        case napi_number:
            return ConvertNumber2Vector(env, jsValue);
        case napi_string:
            return ConvertString2Vector(env, jsValue);
        case napi_object:
            return ConvertUint8Array2Vector(env, jsValue);
        default:
            break;
    }
    return {INVALID};
}

std::vector<uint8_t> JSUtil::ConvertUint8Array2Vector(napi_env env, napi_value jsValue)
{
    bool isTypedArray = false;
    if (napi_is_typedarray(env, jsValue, &isTypedArray) != napi_ok || !isTypedArray) {
        return {INVALID};
    }

    napi_typedarray_type type;
    size_t length = 0;
    napi_value buffer = nullptr;
    size_t offset = 0;
    uint8_t *data = nullptr;
    NAPI_CALL_BASE(env, napi_get_typedarray_info(env, jsValue, &type,
        &length, reinterpret_cast<void **>(&data), &buffer, &offset), { INVALID });
    if (type != napi_uint8_array || length == 0 || data == nullptr) {
        return {INVALID};
    }
    std::vector<uint8_t> result(sizeof(uint8_t) + length);
    result[TYPE_POS] = BYTE_ARRAY;
    memcpy_s(result.data() + DATA_POS, result.size() - DATA_POS, data, length);
    return result;
}

std::vector<uint8_t> JSUtil::ConvertString2Vector(napi_env env, napi_value jsValue)
{
    std::string value = Convert2String(env, jsValue);
    std::vector<uint8_t> result(sizeof(uint8_t) + value.size());
    result[TYPE_POS] = STRING;
    if (memcpy_s(result.data() + DATA_POS, result.size() - DATA_POS, value.data(), value.size()) != EOK) {
        result[TYPE_POS] = INVALID;
    }
    return result;
}

std::vector<uint8_t> JSUtil::ConvertNumber2Vector(napi_env env, napi_value jsValue)
{
    // the JavaScript number is 64 bits double.
    double value = 0;
    auto status = napi_get_value_double(env, jsValue, &value);
    if (status == napi_ok) {
        std::vector<uint8_t> result(sizeof(uint8_t) + sizeof(double));
        result[TYPE_POS] = DOUBLE;
        uint8_t byteValue[MAX_NUMBER_BYTES];
        errno_t err = memcpy_s(byteValue, MAX_NUMBER_BYTES, &value, sizeof(double));
        if (err != EOK) {
            return {INVALID};
        }
        err = memcpy_s(result.data() + DATA_POS, sizeof(double), byteValue, sizeof(byteValue));
        if (err != EOK) {
            return {INVALID};
        }
        return result;
    }

    return {INVALID};
}

std::vector<uint8_t> JSUtil::ConvertBool2Vector(napi_env env, napi_value jsValue)
{
    std::vector<uint8_t> result;
    result.resize(sizeof(uint8_t) + sizeof(int32_t));
    result[TYPE_POS] = BOOLEAN;
    bool value = false;
    napi_get_value_bool(env, jsValue, &value);
    *reinterpret_cast<int32_t *>(result.data() + DATA_POS) = value ? 1 : 0;
    return result;
}

napi_value JSUtil::GetJSEntries(napi_env env, const std::list<DistributedKv::Entry> &entries)
{
    napi_value jsValue = nullptr;
    napi_create_array_with_length(env, entries.size(), &jsValue);
    int index = 0;
    for (const auto &data : entries) {
        napi_value entry = nullptr;
        napi_create_object(env, &entry);
        napi_set_named_property(env, entry, "key", Convert2JSString(env, data.key.Data()));
        napi_set_named_property(env, entry, "value", Convert2JSValue(env, data.value.Data()));
        napi_set_element(env, jsValue, index++, entry);
    }
    return jsValue;
}

napi_value JSUtil::Convert2JSString(napi_env env, const std::string &cString)
{
    napi_value jsValue = nullptr;
    napi_create_string_utf8(env, cString.c_str(), cString.size(), &jsValue);
    return jsValue;
}

napi_value JSUtil::Convert2JSNumber(napi_env env, const std::vector<uint8_t> &data)
{
    napi_value jsValue = nullptr;
    uint8_t byteValue[MAX_NUMBER_BYTES];
    double value;
    switch (data[0]) {
        case INTEGER:
            memcpy_s(byteValue, sizeof(byteValue), data.data() + DATA_POS, sizeof(int32_t));
            value = *(reinterpret_cast<const int32_t *>(byteValue));
            break;
        case FLOAT:
            memcpy_s(byteValue, sizeof(byteValue), data.data() + DATA_POS, sizeof(float));
            value = *(reinterpret_cast<const float *>(byteValue));
            break;
        case DOUBLE:
            memcpy_s(byteValue, sizeof(byteValue), data.data() + DATA_POS, sizeof(double));
            value = *(reinterpret_cast<const double *>(byteValue));
            break;
        default:
            napi_get_undefined(env, &jsValue);
            return jsValue;
    }
    napi_create_double(env, value, &jsValue);
    return jsValue;
}
}