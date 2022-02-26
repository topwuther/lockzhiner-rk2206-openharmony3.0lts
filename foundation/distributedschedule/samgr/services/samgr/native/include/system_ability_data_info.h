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

#include <string>
#include "app_types.h"
#include "utils.h"
#include "string_ex.h"

#ifndef OHOS_SERVICES_SAMGR_NATIVE_INCLUDE_SYSTEM_ABILITY_DATA_INFO_H_
#define OHOS_SERVICES_SAMGR_NATIVE_INCLUDE_SYSTEM_ABILITY_DATA_INFO_H_

namespace OHOS {
namespace {
const std::string SEPARATOR = "_";
const std::string EMPTY_STRING = "nullstr";

constexpr size_t REMOTE_OBJ_INDEX = 0;
constexpr size_t CAPABILITY_INDEX = 1;
constexpr size_t DEF_PERM_INDEX = 2;
constexpr size_t SPLIT_LENGTH = 3;
}
struct SADataValue {
    SADataValue() = default;
    void ConvertToValue(AppDistributedKv::Value& value)
    {
        value = std::to_string(iRemoteObjAddr)
            .append(SEPARATOR).append(capability.empty() ? EMPTY_STRING : capability)
            .append(SEPARATOR).append(permission.empty() ? EMPTY_STRING : permission);
    }

    bool UnConvert(const AppDistributedKv::Value& value)
    {
        if (value.Empty()) {
            return false;
        }
        std::string addValue = value.ToString();
        if (addValue.empty()) {
            return false;
        }
        std::vector<std::string> strVector;
        SplitStr(addValue, SEPARATOR, strVector);
        if (strVector.size() < SPLIT_LENGTH) {
            return false;
        }
        iRemoteObjAddr = Utils::StrToUint64(strVector[REMOTE_OBJ_INDEX]);
        if (iRemoteObjAddr == Utils::MAX_U64_VAL) {
            return false;
        }
        if (strVector[CAPABILITY_INDEX] != EMPTY_STRING) {
            capability = strVector[CAPABILITY_INDEX];
        }
        if (strVector[DEF_PERM_INDEX] != EMPTY_STRING) {
            permission = strVector[DEF_PERM_INDEX];
        }
        return true;
    }

    SADataValue(uint64_t remoteObjAddr, const std::string& capability, const std::string& permission)
    {
        iRemoteObjAddr = remoteObjAddr;
        this->capability = capability;
        this->permission = permission;
    }

    uint64_t iRemoteObjAddr = Utils::MAX_U64_VAL;
    std::string capability;
    std::string permission;
};
} // namespace OHOS

#endif // OHOS_SERVICES_SAMGR_NATIVE_INCLUDE_SYSTEM_ABILITY_DATA_INFO_H_
