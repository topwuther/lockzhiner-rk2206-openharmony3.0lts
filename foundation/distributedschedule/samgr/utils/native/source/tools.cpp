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

#include "tools.h"
#include <codecvt>
#include <locale>
#include <map>
#include <string>

namespace OHOS {
const std::string EMPTY_DEVICEID = "";
constexpr int NON_ANONYMIZE_LENGTH = 6;
// template<typename T>
std::u16string DeleteAllMark(const std::u16string& str, const std::u16string& mark)
{
    std::u16string s = str;
    size_t nSize = mark.size();
    while (true) {
        size_t pos = s.find(mark);
        if (pos == std::string::npos) {
            return s;
        }
        s.erase(pos, nSize);
    }
    return s;
}

// template<typename T>
std::u16string DeleteBlank(const std::u16string& str)
{
    std::u16string res = DeleteAllMark(str, u" ");
    res = DeleteAllMark(res, u"/t");
    res = DeleteAllMark(res, u"/n");
    return DeleteAllMark(res, u"/r");
}

std::string AnonymizeDeviceId(const std::string& deviceId)
{
    if (deviceId.length() < NON_ANONYMIZE_LENGTH) {
        return EMPTY_DEVICEID;
    }
    std::string anonymizeDeviceId = deviceId.substr(0, NON_ANONYMIZE_LENGTH);
    anonymizeDeviceId.append("******");
    return anonymizeDeviceId;
}
}