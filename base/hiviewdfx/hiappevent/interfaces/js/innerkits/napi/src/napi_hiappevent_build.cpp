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
#include "napi_hiappevent_build.h"

#include "hiappevent_base.h"
#include "hiappevent_pack.h"
#include "hiappevent_verify.h"
#include "hilog/log.h"

using namespace OHOS::HiviewDFX;
using namespace OHOS::HiviewDFX::ErrorCode;

namespace OHOS {
namespace HiviewDFX {
namespace {
const HiLogLabel LABEL = { LOG_CORE, HIAPPEVENT_DOMAIN, "HiAppEvent_NAPI_Build" };
constexpr int NAPI_VALUE_STRING_LEN = 10240;
constexpr int EVENT_NAME_INDEX = 0;
constexpr int EVENT_TYPE_INDEX = 1;
constexpr int JSON_OBJECT_INDEX = 2;
constexpr int SUCCESS_FLAG = 0;
const std::string INVALID_KEY_TYPE_ARR[] = {
    "[object Object]",
    "null",
    "()",
    ","
};

int CheckWriteParamsType(const napi_env env, const napi_value params[], int paramNum)
{
    if (paramNum < 3) { // the min number of params for the write function is 3
        HiLog::Error(LABEL, "invalid number=%{public}d of params.", paramNum);
        return ERROR_INVALID_PARAM_NUM_JS;
    }

    napi_valuetype valueType;
    napi_typeof(env, params[EVENT_NAME_INDEX], &valueType);
    if (valueType != napi_valuetype::napi_string) {
        HiLog::Error(LABEL, "the first param must be of type string.");
        return ERROR_INVALID_PARAM_TYPE_JS;
    }

    napi_typeof(env, params[EVENT_TYPE_INDEX], &valueType);
    if (valueType != napi_valuetype::napi_number) {
        HiLog::Error(LABEL, "the second param must be of type number.");
        return ERROR_INVALID_PARAM_TYPE_JS;
    }

    napi_typeof(env, params[JSON_OBJECT_INDEX], &valueType);
    if (valueType != napi_valuetype::napi_object) {
        HiLog::Error(LABEL, "the third param must be of type object.");
        return ERROR_INVALID_PARAM_TYPE_JS;
    }

    return SUCCESS_FLAG;
}

bool CheckKeyTypeString(const std::string str)
{
    bool result = true;
    for (auto invalidType : INVALID_KEY_TYPE_ARR) {
        if (str.find(invalidType) != std::string::npos) {
            result = false;
            break;
        }
    }
    return result;
}

std::shared_ptr<AppEventPack> CreateEventPackFromNapi(napi_env env, napi_value nameValue, napi_value typeValue)
{
    char eventName[NAPI_VALUE_STRING_LEN] = {0};
    size_t cValueLength = 0;
    napi_get_value_string_utf8(env, nameValue, eventName, NAPI_VALUE_STRING_LEN - 1, &cValueLength);

    int32_t eventType = 0;
    napi_get_value_int32(env, typeValue, &eventType);

    return std::make_shared<AppEventPack>(eventName, eventType);
}

void AddBoolParam2EventPack(napi_env env, const std::string &key, const napi_value param,
    std::shared_ptr<AppEventPack>& appEventPack)
{
    bool value = true;
    napi_status status = napi_get_value_bool(env, param, &value);
    if (status != napi_ok) {
        HiLog::Error(LABEL, "failed to get the value of bool param.");
        return;
    }

    AddEventParam(appEventPack, key, value);
}

void AddNumberParam2EventPack(napi_env env, const std::string &key, const napi_value param,
    std::shared_ptr<AppEventPack>& appEventPack)
{
    double value = 0;
    napi_status status = napi_get_value_double(env, param, &value);
    if (status != napi_ok) {
        HiLog::Error(LABEL, "failed to get the value of number param.");
        return;
    }

    AddEventParam(appEventPack, key, value);
}

void AddStringParam2EventPack(napi_env env, const std::string &key, const napi_value param,
    std::shared_ptr<AppEventPack>& appEventPack)
{
    char value[NAPI_VALUE_STRING_LEN] = {0};
    size_t len = 0;
    napi_status status = napi_get_value_string_utf8(env, param, value, NAPI_VALUE_STRING_LEN - 1, &len);
    if (status != napi_ok) {
        HiLog::Error(LABEL, "failed to get the value of string param.");
        return;
    }

    std::string valueStr = value;
    AddEventParam(appEventPack, key, valueStr);
}

int AddBoolArrayParam2EventPack(napi_env env, const std::string &key, const napi_value arrParam,
    std::shared_ptr<AppEventPack>& appEventPack)
{
    uint32_t len = 0;
    napi_status status = napi_get_array_length(env, arrParam, &len);
    if (status != napi_ok) {
        HiLog::Error(LABEL, "failed to get the length of bool array.");
        return ERROR_UNKNOWN;
    }

    std::vector<bool> bools;
    napi_value element;
    napi_valuetype type;
    for (uint32_t i = 0; i < len; i++) {
        status = napi_get_element(env, arrParam, i, &element);
        if (status != napi_ok) {
            HiLog::Error(LABEL, "failed to get the element of bool array.");
            return ERROR_UNKNOWN;
        }

        status = napi_typeof(env, element, &type);
        if (status != napi_ok) {
            HiLog::Error(LABEL, "failed to get the type of bool array element.");
            return ERROR_UNKNOWN;
        }

        if (type != napi_valuetype::napi_boolean) {
            HiLog::Warn(LABEL, "param=%{public}s is discarded because the array elements are not all boolean types.",
                key.c_str());
            return ERROR_INVALID_LIST_PARAM_TYPE;
        }

        bool value = true;
        status = napi_get_value_bool(env, element, &value);
        if (status != napi_ok) {
            HiLog::Error(LABEL, "failed to get the bool value of bool array element.");
            return ERROR_UNKNOWN;
        }
        bools.push_back(value);
    }
    AddEventParam(appEventPack, key, bools);
    return SUCCESS_FLAG;
}

int AddNumberArrayParam2EventPack(napi_env env, const std::string &key, const napi_value arrParam,
    std::shared_ptr<AppEventPack>& appEventPack)
{
    uint32_t len = 0;
    napi_status status = napi_get_array_length(env, arrParam, &len);
    if (status != napi_ok) {
        HiLog::Error(LABEL, "failed to get the length of number array.");
        return ERROR_UNKNOWN;
    }

    std::vector<double> doubles;
    napi_value element;
    napi_valuetype type;
    for (uint32_t i = 0; i < len; i++) {
        status = napi_get_element(env, arrParam, i, &element);
        if (status != napi_ok) {
            HiLog::Error(LABEL, "failed to get the element of number array.");
            return ERROR_UNKNOWN;
        }

        status = napi_typeof(env, element, &type);
        if (status != napi_ok) {
            HiLog::Error(LABEL, "failed to get the type of number array element.");
            return ERROR_UNKNOWN;
        }

        if (type != napi_valuetype::napi_number) {
            HiLog::Warn(LABEL, "param=%{public}s is discarded because the array elements are not all number types.",
                key.c_str());
            return ERROR_INVALID_LIST_PARAM_TYPE;
        }

        double value = 0;
        status = napi_get_value_double(env, element, &value);
        if (status != napi_ok) {
            HiLog::Error(LABEL, "failed to get the number value of number array element.");
            return ERROR_UNKNOWN;
        }
        doubles.push_back(value);
    }
    AddEventParam(appEventPack, key, doubles);
    return SUCCESS_FLAG;
}

int AddStringArrayParam2EventPack(napi_env env, const std::string &key, const napi_value arrParam,
    std::shared_ptr<AppEventPack>& appEventPack)
{
    uint32_t len = 0;
    napi_status status = napi_get_array_length(env, arrParam, &len);
    if (status != napi_ok) {
        HiLog::Error(LABEL, "failed to get the length of string array.");
        return ERROR_UNKNOWN;
    }

    std::vector<const std::string> strs;
    napi_value element;
    napi_valuetype type;
    for (uint32_t i = 0; i < len; i++) {
        status = napi_get_element(env, arrParam, i, &element);
        if (status != napi_ok) {
            HiLog::Error(LABEL, "failed to get the element of string array.");
            return ERROR_UNKNOWN;
        }

        status = napi_typeof(env, element, &type);
        if (status != napi_ok) {
            HiLog::Error(LABEL, "failed to get the type of string array element.");
            return ERROR_UNKNOWN;
        }

        if (type != napi_valuetype::napi_string) {
            HiLog::Warn(LABEL, "param=%{public}s is discarded because the array elements are not all string types.",
                key.c_str());
            return ERROR_INVALID_LIST_PARAM_TYPE;
        }

        char value[NAPI_VALUE_STRING_LEN] = {0};
        size_t valueLen = 0;
        status = napi_get_value_string_utf8(env, element, value, NAPI_VALUE_STRING_LEN - 1, &valueLen);
        if (status != napi_ok) {
            HiLog::Error(LABEL, "failed to get the string value of string array element.");
            return ERROR_UNKNOWN;
        }
        strs.push_back(value);
    }
    AddEventParam(appEventPack, key, strs);
    return SUCCESS_FLAG;
}

int AddArrayParam2EventPack(napi_env env, const std::string &key, const napi_value arrParam,
    std::shared_ptr<AppEventPack>& appEventPack)
{
    uint32_t len = 0;
    napi_status status = napi_get_array_length(env, arrParam, &len);
    if (status != napi_ok) {
        HiLog::Warn(LABEL, "param=%{public}s is discarded because the value is not an array.", key.c_str());
        return ERROR_INVALID_PARAM_VALUE_TYPE;
    }

    if (len == 0) {
        HiLog::Warn(LABEL, "param=%{public}s array value is empty.", key.c_str());
        AddEventParam(appEventPack, key);
        return SUCCESS_FLAG;
    }

    napi_value value;
    status = napi_get_element(env, arrParam, 0, &value);
    if (status != napi_ok) {
        HiLog::Error(LABEL, "failed to get the element of array.");
        return ERROR_UNKNOWN;
    }

    napi_valuetype type;
    status = napi_typeof(env, value, &type);
    if (status != napi_ok) {
        HiLog::Error(LABEL, "failed to get the type of array element.");
        return ERROR_UNKNOWN;
    }

    int res = SUCCESS_FLAG;
    if (type == napi_valuetype::napi_boolean) {
        res = AddBoolArrayParam2EventPack(env, key, arrParam, appEventPack);
    } else if (type == napi_valuetype::napi_number) {
        res = AddNumberArrayParam2EventPack(env, key, arrParam, appEventPack);
    } else if (type == napi_valuetype::napi_string) {
        res = AddStringArrayParam2EventPack(env, key, arrParam, appEventPack);
    } else {
        HiLog::Warn(LABEL, "param=%{public}s is discarded because the list value type is invalid.", key.c_str());
        return ERROR_INVALID_LIST_PARAM_TYPE;
    }

    return res;
}

int AddParam2EventPack(napi_env env, const std::string &key, const napi_value param,
    std::shared_ptr<AppEventPack>& appEventPack)
{
    napi_valuetype type;
    napi_status status = napi_typeof(env, param, &type);
    if (status != napi_ok) {
        HiLog::Error(LABEL, "failed to get the type of param value.");
        return ERROR_UNKNOWN;
    }

    if (type == napi_valuetype::napi_boolean) {
        AddBoolParam2EventPack(env, key, param, appEventPack);
    } else if (type == napi_valuetype::napi_number) {
        AddNumberParam2EventPack(env, key, param, appEventPack);
    } else if (type == napi_valuetype::napi_string) {
        AddStringParam2EventPack(env, key, param, appEventPack);
    } else if (type == napi_valuetype::napi_object) {
        return AddArrayParam2EventPack(env, key, param, appEventPack);
    } else {
        HiLog::Warn(LABEL, "param=%{public}s is discarded because the value type is invalid.", key.c_str());
        return ERROR_INVALID_PARAM_VALUE_TYPE;
    }

    return SUCCESS_FLAG;
}

int BuildAppEventPackFromNapiInner(napi_env env, const napi_value object, std::shared_ptr<AppEventPack>& appEventPack)
{
    napi_value keyArr = nullptr;
    napi_status status = napi_get_property_names(env, object, &keyArr);
    if (status != napi_ok) {
        HiLog::Error(LABEL, "napi_get_property_names failed.");
        return ERROR_UNKNOWN;
    }

    uint32_t len = 0;
    status = napi_get_array_length(env, keyArr, &len);
    if (status != napi_ok) {
        HiLog::Error(LABEL, "napi_get_array_length failed.");
        return ERROR_UNKNOWN;
    }

    int buildRes = SUCCESS_FLAG;
    for (uint32_t i = 0; i < len; i++) {
        napi_value keyNapiValue = nullptr;
        napi_get_element(env, keyArr, i, &keyNapiValue);

        napi_valuetype valueType;
        napi_typeof(env, keyNapiValue, &valueType);
        if (valueType != napi_valuetype::napi_string) {
            HiLog::Warn(LABEL, "param is discarded because the key type of the event params must be String.");
            buildRes = ERROR_INVALID_PARAM_KEY_TYPE;
            continue;
        }

        char key[NAPI_VALUE_STRING_LEN] = {0};
        size_t cValueLength = 0;
        napi_get_value_string_utf8(env, keyNapiValue, key, NAPI_VALUE_STRING_LEN - 1, &cValueLength);
        if (!CheckKeyTypeString(key)) {
            HiLog::Warn(LABEL, "param is discarded because the key type may be invalid.");
            buildRes = ERROR_INVALID_PARAM_KEY_TYPE;
            continue;
        }

        napi_value value = nullptr;
        napi_get_named_property(env, object, key, &value);
        int addParamRes = AddParam2EventPack(env, key, value, appEventPack);
        buildRes = (addParamRes == SUCCESS_FLAG) ? buildRes : addParamRes;
    }

    return buildRes;
}
}

std::shared_ptr<AppEventPack> BuildAppEventPackFromNapi(const napi_env env, const napi_value params[],
    int paramNum, int32_t& result)
{
    std::shared_ptr<AppEventPack> appEventPack = nullptr;

    // check the number and type of parameters
    result = CheckWriteParamsType(env, params, paramNum);
    // if the check is successful, start building the object
    if (result == SUCCESS_FLAG) {
        appEventPack = CreateEventPackFromNapi(env, params[EVENT_NAME_INDEX], params[EVENT_TYPE_INDEX]);
        int buildRes = BuildAppEventPackFromNapiInner(env, params[JSON_OBJECT_INDEX], appEventPack);
        result = buildRes == SUCCESS_FLAG ? result : buildRes;

        // perform event verification and delete illegal parameters
        int verifyResult = VerifyAppEvent(appEventPack);
        if (verifyResult != SUCCESS_FLAG) {
            HiLog::Warn(LABEL, "event verify failed, delete illegal parameters.");
            result = verifyResult;
        }
    }

    return appEventPack;
}
}
}