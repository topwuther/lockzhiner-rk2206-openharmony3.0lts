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


#ifndef UTILS_NATIVE_INCLUDE_TOOLS_H_
#define UTILS_NATIVE_INCLUDE_TOOLS_H_

#include <string>

namespace OHOS {
const uint32_t MAX_SERVICES = 1000;

// template<typename T>
std::u16string DeleteAllMark(const std::u16string& str, const std::u16string& mark);

// template<typename T>
std::u16string DeleteBlank(const std::u16string& str);

std::string AnonymizeDeviceId(const std::string& deviceId);
}

#endif // !defined(UTILS_NATIVE_INCLUDE_TOOLS_H_)
