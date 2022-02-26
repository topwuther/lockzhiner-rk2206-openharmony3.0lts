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
#include "napi_hiappevent_config.h"

#include <string>

#include "ability.h"
#include "hiappevent_base.h"
#include "hiappevent_config.h"
#include "hilog/log.h"

namespace OHOS {
namespace HiviewDFX {
namespace {
const HiLogLabel LABEL = { LOG_CORE, HIAPPEVENT_DOMAIN, "HiAppEvent_NAPI_Config" };
constexpr int NAPI_VALUE_STRING_LEN = 100;

std::string GetStringFromNapiValue(const napi_env env, const napi_value value)
{
    napi_valuetype type;
    napi_status status = napi_typeof(env, value, &type);
    if (status != napi_ok) {
        HiLog::Error(LABEL, "failed to get the type of the config value.");
        return "";
    }

    if (type == napi_valuetype::napi_boolean) {
        bool boolValue = true;
        napi_get_value_bool(env, value, &boolValue);
        return boolValue ? "true" : "false";
    } else if (type == napi_valuetype::napi_number) {
        double doubleValue = 0;
        napi_get_value_double(env, value, &doubleValue);
        return std::to_string(doubleValue);
    } else if (type == napi_valuetype::napi_string) {
        char strValue[NAPI_VALUE_STRING_LEN] = {0};
        size_t len = 0;
        napi_get_value_string_utf8(env, value, strValue, NAPI_VALUE_STRING_LEN - 1, &len);
        return strValue;
    } else {
        HiLog::Warn(LABEL, "the type of the config value is invalid.");
        return "";
    }
}

bool ConfigureFromNapiInner(const napi_env env, const napi_value configObj)
{
    napi_value keyArr = nullptr;
    napi_status status = napi_get_property_names(env, configObj, &keyArr);
    if (status != napi_ok) {
        HiLog::Error(LABEL, "failed to get the property names of configObj.");
        return false;
    }

    uint32_t len = 0;
    status = napi_get_array_length(env, keyArr, &len);
    if (status != napi_ok) {
        HiLog::Error(LABEL, "failed to get the length of the key array of the configObj.");
        return false;
    }

    bool result = true;
    for (uint32_t i = 0; i < len; i++) {
        napi_value key = nullptr;
        napi_get_element(env, keyArr, i, &key);

        napi_valuetype valueType;
        napi_typeof(env, key, &valueType);
        if (valueType != napi_valuetype::napi_string) {
            HiLog::Warn(LABEL, "the key type of the configObj must be String.");
            result = false;
            continue;
        }

        // get key string
        char keyStr[NAPI_VALUE_STRING_LEN] = {0};
        size_t cValueLength = 0;
        napi_get_value_string_utf8(env, key, keyStr, NAPI_VALUE_STRING_LEN - 1, &cValueLength);

        // get value string
        napi_value value = nullptr;
        napi_get_named_property(env, configObj, keyStr, &value);
        std::string valueStr = GetStringFromNapiValue(env, value);
        if (valueStr.empty()) {
            result = false;
            continue;
        }

        // start to configure HiAppEvent Manager
        if (!HiAppEventConfig::GetInstance().SetConfigurationItem(keyStr, valueStr)) {
            result = false;
        }
    }

    return result;
}
}

bool ConfigureFromNapi(const napi_env env, const napi_value configObj)
{
    napi_valuetype valueType;
    napi_typeof(env, configObj, &valueType);
    if (valueType != napi_valuetype::napi_object) {
        HiLog::Error(LABEL, "failed to check configuration param because param type must be object.");
        return false;
    }
    return ConfigureFromNapiInner(env, configObj);
}

void SetStorageDirFromNapi(const napi_env env, const napi_callback_info info)
{
    static bool isSetDir = false;
    if (isSetDir) {
        return;
    }
    HiLog::Debug(LABEL, "start to init storage path.");
    napi_value global = nullptr;
    napi_get_global(env, &global);
    napi_value abilityObj = nullptr;
    napi_get_named_property(env, global, "ability", &abilityObj);
    AppExecFwk::Ability* ability = nullptr;
    napi_get_value_external(env, abilityObj, (void**)&ability);
    if (ability == nullptr) {
        HiLog::Error(LABEL, "ability is null, stop setting the storage dir.");
        return;
    }
    std::string dir = ability->GetFilesDir();
    HiAppEventConfig::GetInstance().SetStorageDir(dir);
    isSetDir = true;
}
}
}