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
 
#ifndef JSON_UTILS_H
#define JSON_UTILS_H

#include <nlohmann/json.hpp>
#include <string>
#include "log_print.h"
#include "visibility.h"

namespace OHOS {
namespace DistributedKv {
using json = nlohmann::json;

class JsonUtils {
public:
template<typename T>
static bool ParseJsonField(const std::string &jsonStr, const std::string &key, uint8_t valType, T &fieldVal)
{
    json jsonObj = json::parse(jsonStr, nullptr, false);
    if (jsonObj.is_discarded()) {
        ZLOGE("jsonObj is discarded, parse json[%s] failed.", key.c_str());
        return false;
    }
    if (jsonObj.find(key) == jsonObj.end()) {
        return false;
    }
    if (static_cast<uint8_t>(jsonObj[key].type()) != valType) {
        ZLOGE("not match in jsonObj, parse json[%s] failed.", key.c_str());
        return false;
    }
    jsonObj[key].get_to(fieldVal);
    return true;
}

template<typename T>
static bool ParseJsonField(const json &jsonObj, const std::string &key, uint8_t valType, T &fieldVal)
{
    if (jsonObj.is_discarded()) {
        ZLOGE("jsonObj is discarded, parse json[%s] failed.", key.c_str());
        return false;
    }
    if (jsonObj.find(key) == jsonObj.end()) {
        return false;
    }
    if (static_cast<uint8_t>(jsonObj[key].type()) != valType) {
        ZLOGE("not match in jsonObj, parse json[%s] failed.", key.c_str());
        return false;
    }
    jsonObj[key].get_to(fieldVal);
    return true;
}
};
} // namespace DistributedKv
} // namespace OHOS
#endif // JSON_UTILS_H