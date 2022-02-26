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
#ifndef OHOS_GLOBAL_I18N_LOCALE_INFO_H
#define OHOS_GLOBAL_I18N_LOCALE_INFO_H

#include "unicode/locid.h"
#include "unicode/localebuilder.h"
#include "unicode/stringpiece.h"
#include <map>

namespace OHOS {
namespace Global {
namespace I18n {
class LocaleInfo {
public:
    LocaleInfo(std::string locale);
    LocaleInfo(const std::string &localeTag, std::map<std::string, std::string> &configs);
    virtual ~LocaleInfo();
    std::string GetLanguage() const;
    std::string GetScript() const;
    std::string GetRegion() const;
    std::string GetBaseName() const;
    std::string GetCalendar() const;
    std::string GetCollation() const;
    std::string GetHourCycle() const;
    std::string GetNumberingSystem() const;
    std::string Maximize();
    std::string Minimize();
    std::string GetNumeric() const;
    std::string GetCaseFirst() const;
    std::string ToString() const;
    icu::Locale GetLocale() const;
    icu::Locale GetLocaleTag() const;
    static const uint32_t SCRIPT_LEN = 4;
    static const uint32_t REGION_LEN = 2;
private:
    std::string language;
    std::string region;
    std::string script;
    std::string baseName;
    std::string calendar;
    std::string collation;
    std::string hourCycle;
    std::string numberingSystem;
    std::string numeric;
    std::string caseFirst;
    std::string finalLocaleTag;
    std::string localeTag;
    icu::Locale locale;
    std::string calendarTag = "-ca-";
    std::string collationTag = "-co-";
    std::string hourCycleTag = "-hc-";
    std::string numberingSystemTag = "-nu-";
    std::string numericTag = "-kn-";
    std::string caseFirstTag = "-kf-";
    static bool icuInitialized;
    static bool Init();
    static const uint32_t CONFIG_TAG_LEN = 4;
    std::map<std::string, std::string> configs;
    void ComputeFinalLocaleTag(const std::string &localeTag);
    void ParseConfigs();
    void ParseLocaleTag(const std::string &localeTag);
};
} // namespace I18n
} // namespace Global
} // namespace OHOS
#endif