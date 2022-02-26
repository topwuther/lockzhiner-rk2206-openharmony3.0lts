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
#include "locale_info.h"
#include <algorithm>
#include "ohos/init_data.h"

namespace OHOS {
namespace Global {
namespace I18n {
using namespace icu;

LocaleInfo::LocaleInfo(std::string localeTag)
{
    UErrorCode status = U_ZERO_ERROR;
    configs = {};
    auto builder = std::make_unique<LocaleBuilder>();
    Locale locale = builder->setLanguageTag(StringPiece(localeTag)).build(status);
    if (status != U_ZERO_ERROR) {
        locale = Locale::getDefault();
    }
    language = locale.getLanguage();
    script = locale.getScript();
    region = locale.getCountry();
    baseName = language;
    if (script.length() == SCRIPT_LEN) {
        baseName += "-" + script;
    }
    if (region.length() == REGION_LEN) {
        baseName += "-" + region;
    }
}

LocaleInfo::LocaleInfo(const std::string &localeTag, std::map<std::string, std::string> &configMap)
{
    UErrorCode status = U_ZERO_ERROR;
    configs = configMap;
    ComputeFinalLocaleTag(localeTag);
    auto builder = std::make_unique<LocaleBuilder>();
    locale = builder->setLanguageTag(StringPiece(finalLocaleTag)).build(status);
    if (status != U_ZERO_ERROR) {
        locale = Locale::getDefault();
    }
    language = locale.getLanguage();
    script = locale.getScript();
    region = locale.getCountry();
    baseName = locale.getBaseName();
    std::replace(baseName.begin(), baseName.end(), '_', '-');
}

LocaleInfo::~LocaleInfo() {}

void LocaleInfo::ComputeFinalLocaleTag(const std::string &localeTag)
{
    if (localeTag.find("-u-") == std::string::npos) {
        finalLocaleTag = localeTag;
        ParseConfigs();
    } else {
        finalLocaleTag = localeTag.substr(0, localeTag.find("-u-"));
        ParseLocaleTag(localeTag);
        ParseConfigs();
    }
    if (!script.empty()) {
        finalLocaleTag += "-" + script;
    }
    if (!region.empty()) {
        finalLocaleTag += "-" + region;
    }
    if (!hourCycle.empty() || !numberingSystem.empty() || !calendar.empty() || !collation.empty() ||
        !caseFirst.empty() || !numeric.empty()) {
        finalLocaleTag += "-u";
    }
    if (!hourCycle.empty()) {
        finalLocaleTag += hourCycleTag + hourCycle;
    }
    if (!numberingSystem.empty()) {
        finalLocaleTag += numberingSystemTag + numberingSystem;
    }
    if (!calendar.empty()) {
        finalLocaleTag += calendarTag + calendar;
    }
    if (!collation.empty()) {
        finalLocaleTag += collationTag + collation;
    }
    if (!caseFirst.empty()) {
        finalLocaleTag += caseFirstTag + caseFirst;
    }
    if (!numeric.empty()) {
        finalLocaleTag += numericTag + numeric;
    }
}

void LocaleInfo::ParseLocaleTag(const std::string &localeTag)
{
    std::string flag = "-";
    if (localeTag.find(hourCycleTag) != std::string::npos) {
        hourCycle = localeTag.substr(localeTag.find(hourCycleTag) + CONFIG_TAG_LEN);
        hourCycle = hourCycle.substr(0, hourCycle.find(flag));
    }
    if (localeTag.find(numberingSystemTag) != std::string::npos) {
        numberingSystem = localeTag.substr(localeTag.find(numberingSystemTag) + CONFIG_TAG_LEN);
        numberingSystem = numberingSystem.substr(0, numberingSystem.find(flag));
    }
    if (localeTag.find(calendarTag) != std::string::npos) {
        calendar = localeTag.substr(localeTag.find(calendarTag) + CONFIG_TAG_LEN);
        calendar = calendar.substr(0, calendar.find(flag));
    }
    if (localeTag.find(collationTag) != std::string::npos) {
        collation = localeTag.substr(localeTag.find(collationTag) + CONFIG_TAG_LEN);
        collation = collation.substr(0, collation.find(flag));
    }
    if (localeTag.find(caseFirstTag) != std::string::npos) {
        caseFirst = localeTag.substr(localeTag.find(caseFirstTag) + CONFIG_TAG_LEN);
        caseFirst = hourCycle.substr(0, caseFirst.find(flag));
    }
    if (localeTag.find(numericTag) != std::string::npos) {
        numeric = localeTag.substr(localeTag.find(numericTag) + CONFIG_TAG_LEN);
        numeric = numeric.substr(0, numeric.find(flag));
    }
}

void LocaleInfo::ParseConfigs()
{
    if (configs.count("script") > 0) {
        script = configs["script"];
    }
    if (configs.count("region") > 0) {
        region = configs["region"];
    }
    if (configs.count("hourCycle") > 0) {
        hourCycle = configs["hourCycle"];
    }
    if (configs.count("numberingSystem") > 0) {
        numberingSystem = configs["numberingSystem"];
    }
    if (configs.count("calendar") > 0) {
        calendar = configs["calendar"];
    }
    if (configs.count("collation") > 0) {
        collation = configs["collation"];
    }
    if (configs.count("caseFirst") > 0) {
        caseFirst = configs["caseFirst"];
    }
    if (configs.count("numeric") > 0) {
        numeric = configs["numeric"];
    }
}

bool LocaleInfo::icuInitialized = LocaleInfo::Init();

std::string LocaleInfo::GetLanguage() const
{
    return language;
}

std::string LocaleInfo::GetScript() const
{
    return script;
}

std::string LocaleInfo::GetRegion() const
{
    return region;
}

std::string LocaleInfo::GetBaseName() const
{
    return baseName;
}

std::string LocaleInfo::GetCalendar() const
{
    return calendar;
}

std::string LocaleInfo::GetCollation() const
{
    return collation;
}

std::string LocaleInfo::GetHourCycle() const
{
    return hourCycle;
}

std::string LocaleInfo::GetNumberingSystem() const
{
    return numberingSystem;
}

std::string LocaleInfo::GetNumeric() const
{
    return numeric;
}

std::string LocaleInfo::GetCaseFirst() const
{
    return caseFirst;
}

std::string LocaleInfo::ToString() const
{
    return finalLocaleTag;
}

Locale LocaleInfo::GetLocale() const
{
    return locale;
}

std::string LocaleInfo::Maximize()
{
    UErrorCode status = U_ZERO_ERROR;
    Locale curLocale = locale;
    curLocale.addLikelySubtags(status);
    if (status == U_ZERO_ERROR) {
        std::string restConfigs = "";
        if (finalLocaleTag.find("-u-") != std::string::npos) {
            restConfigs = finalLocaleTag.substr(finalLocaleTag.find("-u-"));
        }
        std::string curBaseName = (curLocale.getBaseName() == nullptr) ? "" : curLocale.getBaseName();
        std::replace(curBaseName.begin(), curBaseName.end(), '_', '-');
        std::string localeTag = curBaseName + restConfigs;
        return localeTag;
    }
    return finalLocaleTag;
}

std::string LocaleInfo::Minimize()
{
    UErrorCode status = U_ZERO_ERROR;
    Locale curLocale = locale;
    curLocale.minimizeSubtags(status);
    if (status == U_ZERO_ERROR) {
        std::string restConfigs = "";
        if (finalLocaleTag.find("-u-") != std::string::npos) {
            restConfigs = finalLocaleTag.substr(finalLocaleTag.find("-u-"));
        }
        std::string curBaseName = (curLocale.getBaseName() == nullptr) ? "" : curLocale.getBaseName();
        std::replace(curBaseName.begin(), curBaseName.end(), '_', '-');
        std::string localeTag = curBaseName + restConfigs;
        return localeTag;
    }
    return finalLocaleTag;
}

bool LocaleInfo::Init()
{
    SetHwIcuDirectory();
    return true;
}
} // namespace I18n
} // namespace Global
} // namespace OHOS
