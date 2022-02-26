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

#ifndef DATA_TRANSLATOR_H
#define DATA_TRANSLATOR_H

#include <string>

#include "permission_def.h"
#include "permission_req.h"
#include "generic_values.h"

namespace OHOS {
namespace Security {
namespace Permission {
class DataTranslator final {
public:
    static int TranslationIntoPermissionDef(const GenericValues& inGenericValues, PermissionDef& outPermissionDef);
    static int TranslationIntoPermissionReq(const GenericValues& inGenericValues, PermissionReq& outPermissionReq);
    static int TranslationIntoGenericValues(const PermissionDef& inPermissionDef, GenericValues& outGenericValues);
    static int TranslationIntoGenericValues(const PermissionReq& inPermissionReq, GenericValues& outGenericValues);
    static std::string ToString(const PermissionDef& inPermissionDef);
};
} // namespace Permission
} // namespace Security
} // namespace OHOS
#endif // DATA_TRANSLATOR_H
