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

#ifndef OHOS_SA_PROFILE_PARSER_H_
#define OHOS_SA_PROFILE_PARSER_H_

#include <list>
#include <string>
#include "sa_profiles.h"
#include "libxml/parser.h"
#include "libxml/xpath.h"

namespace OHOS {
class SaProfileParser {
public:
    ~SaProfileParser();
    bool ParseSaProfiles(const std::string& profilePath);

    const std::list<SaProfile>& GetAllSaProfiles() const;
    void ClearResource();
    void OpenSo();
    void CloseSo(int32_t systemAbilityId);
    bool LoadSaLib(int32_t systemAbilityId);
private:
    void CloseSo();
    void OpenSo(SaProfile& saProfile);
    void CloseHandle(SaProfile& saProfile);
    bool ParseSystemAbility(const xmlNode& rootNode);
    void ParseSAProp(const std::string& nodeName, const std::string& nodeContent, SaProfile& saProfile);

    std::list<SaProfile> saProfiles_;
};
} // namespace OHOS

#endif // OHOS_SA_PROFILE_PARSER_H_
