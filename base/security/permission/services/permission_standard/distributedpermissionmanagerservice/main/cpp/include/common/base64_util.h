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

#ifndef BASE64_UTIL_H
#define BASE64_UTIL_H

#include <cstddef>
#include <cstring>
#include <string>

namespace OHOS {
namespace Security {
namespace Permission {
class Base64Util {
public:
    static void Encode(const unsigned char *input, unsigned long inLen, std::string &output);

    static void Decode(const std::string &input, unsigned char *output, unsigned long &outLen);
};
}  // namespace Permission
}  // namespace Security
}  // namespace OHOS
#endif
