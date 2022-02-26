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

#ifndef SERVICES_SAMGR_NATIVE_INCLUDE_UTILS_H_
#define SERVICES_SAMGR_NATIVE_INCLUDE_UTILS_H_

#include <limits>
#include <map>
#include <set>
#include <string>
#include "nocopyable.h"

#include "libxml/xpath.h"

namespace OHOS {
class Utils {
public:
    static const uint64_t MAX_U64_VAL = (std::numeric_limits<uint64_t>::max)();
    static uint64_t StrToUint64(const std::string& param);
    static bool ParseCoreSaList(const std::string& saPath, std::set<std::int32_t>& saList);
    static bool ParseSysCapMap(const std::string& sysCapPath, std::map<std::string, bool>& sysCapMap);
private:
    Utils() = delete;
    ~Utils() = delete;
    DISALLOW_COPY_AND_MOVE(Utils);
    static bool ParseSystemAbility(const xmlNode& rootNode, std::set<std::int32_t>& saList);
};
} // namespace OHOS

#endif // SERVICES_SAMGR_NATIVE_INCLUDE_UTILS_H_
