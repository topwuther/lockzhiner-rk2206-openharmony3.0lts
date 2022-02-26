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

#ifndef OHOS_SA_PROFILE_H_
#define OHOS_SA_PROFILE_H_

#include <string>
#include <vector>

namespace OHOS {
using DlHandle = void*;

struct SaProfile {
    int32_t saId = 0;
    std::u16string libPath;
    std::vector<std::u16string> dependSa;
    int32_t dependTimeout = 0;
    bool runOnCreate = false;
    bool distributed = false;
    int32_t dumpLevel = 0;
    std::u16string capability;
    std::u16string permission;
    std::u16string bootPhase;
    DlHandle handle = nullptr;
};
}
#endif
