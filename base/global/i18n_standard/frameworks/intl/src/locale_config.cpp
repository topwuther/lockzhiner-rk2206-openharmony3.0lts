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
#include <algorithm>
#include <memory.h>
#include <unordered_set>
#include "locale_config.h"
#include "libxml/parser.h"
#include "locale_info.h"
#include "localebuilder.h"
#include "locid.h"
#include "ohos/init_data.h"
#include "parameter.h"
#include "sim_card_manager.h"
#include "string_ex.h"
#include "ucase.h"
#include "unistr.h"

namespace OHOS {
namespace Global {
namespace I18n {
using namespace std;

const char *LocaleConfig::LANGUAGE_KEY = "hm.sys.language";
const char *LocaleConfig::LOCALE_KEY = "hm.sys.locale";
const char *LocaleConfig::DEFAULT_LOCALE = "zh-Hans-CN";
const char *LocaleConfig::DEFAULT_LANGUAGE = "zh-Hans";
const char *LocaleConfig::DEFAULT_REGION = "CN";
const char *LocaleConfig::SUPPORTED_LOCALES_NAME = "supported_locales";
const char *LocaleConfig::SUPPORTED_REGIONS_NAME = "supported_regions";
const char *LocaleConfig::WHITE_LANGUAGES_NAME = "white_languages";
const char *LocaleConfig::FORBIDDEN_LANGUAGES_NAME = "forbidden_languages";
const char *LocaleConfig::FORBIDDEN_REGIONS_NAME = "forbidden_regions";
const char *LocaleConfig::FORBIDDEN_LANGUAGES_PATH = "/system/usr/ohos_locale_config/forbidden_languages.xml";
const char *LocaleConfig::FORBIDDEN_REGIONS_PATH = "/system/usr/ohos_locale_config/forbidden_regions.xml";
const char *LocaleConfig::SUPPORTED_LOCALES_PATH = "/system/usr/ohos_locale_config/supported_locales.xml";
const char *LocaleConfig::SUPPORTED_REGIONS_PATH = "/system/usr/ohos_locale_config/supported_regions.xml";
const char *LocaleConfig::WHITE_LANGUAGES_PATH = "/system/usr/ohos_locale_config/white_languages.xml";
unordered_set<string> LocaleConfig::supportedLocales;
unordered_set<string> LocaleConfig::supportedRegions;
unordered_set<string> LocaleConfig::whiteLanguages;
unordered_map<string, string> LocaleConfig::dialectMap {
    { "es-Latn-419", "es-Latn-419" },
    { "es-Latn-BO", "es-Latn-419" },
    { "es-Latn-BR", "es-Latn-419" },
    { "es-Latn-BZ", "es-Latn-419" },
    { "es-Latn-CL", "es-Latn-419" },
    { "es-Latn-CO", "es-Latn-419" },
    { "es-Latn-CR", "es-Latn-419" },
    { "es-Latn-CU", "es-Latn-419" },
    { "es-Latn-DO", "es-Latn-419" },
    { "es-Latn-EC", "es-Latn-419" },
    { "es-Latn-GT", "es-Latn-419" },
    { "es-Latn-HN", "es-Latn-419" },
    { "es-Latn-MX", "es-Latn-419" },
    { "es-Latn-NI", "es-Latn-419" },
    { "es-Latn-PA", "es-Latn-419" },
    { "es-Latn-PE", "es-Latn-419" },
    { "es-Latn-PR", "es-Latn-419" },
    { "es-Latn-PY", "es-Latn-419" },
    { "es-Latn-SV", "es-Latn-419" },
    { "es-Latn-US", "es-Latn-419" },
    { "es-Latn-UY", "es-Latn-419" },
    { "es-Latn-VE", "es-Latn-419" },
    { "pt-Latn-PT", "pt-Latn-PT" },
    { "en-Latn-US", "en-Latn-US" }
};

bool LocaleConfig::listsInitialized = LocaleConfig::InitializeLists();

string LocaleConfig::GetSystemLanguage()
{
    char value[CONFIG_LEN];
    int code = GetParameter(LANGUAGE_KEY, "", value, CONFIG_LEN);
    if (code > 0) {
        return value;
    }
    return DEFAULT_LANGUAGE;
}

string LocaleConfig::GetSystemRegion()
{
    string locale = GetSystemLocale();
    char value[CONFIG_LEN];
    int code = GetParameter(LOCALE_KEY, "", value, CONFIG_LEN);
    if (code > 0) {
        string tag(value, code);
        UErrorCode status = U_ZERO_ERROR;
        icu::Locale origin = icu::Locale::forLanguageTag(tag, status);
        if (status == U_ZERO_ERROR) {
            return origin.getCountry();
        }
    }
    return DEFAULT_REGION;
}

string LocaleConfig::GetSystemLocale()
{
    char value[CONFIG_LEN];
    int code = GetParameter(LOCALE_KEY, "", value, CONFIG_LEN);
    if (code > 0) {
        return value;
    }
    return DEFAULT_LOCALE;
}

bool LocaleConfig::SetSystemLanguage(const string &language)
{
    if (!IsValidTag(language)) {
        return false;
    }
    return SetParameter(LANGUAGE_KEY, language.data()) == 0;
}

bool LocaleConfig::SetSystemRegion(const string &region)
{
    if (!IsValidRegion(region)) {
        return false;
    }
    char value[CONFIG_LEN];
    int code = GetParameter(LOCALE_KEY, "", value, CONFIG_LEN);
    string newLocale;
    if (code > 0) {
        string tag(value, code);
        newLocale = GetRegionChangeLocale(tag, region);
        if (newLocale == "") {
            return false;
        }
    } else {
        icu::Locale temp("", region.c_str());
        UErrorCode status = U_ZERO_ERROR;
        temp.addLikelySubtags(status);
        if (status != U_ZERO_ERROR) {
            return false;
        }
        newLocale = temp.toLanguageTag<string>(status);
        if (status != U_ZERO_ERROR) {
            return false;
        }
    }
    return SetParameter(LOCALE_KEY, newLocale.data()) == 0;
}

bool LocaleConfig::SetSystemLocale(const string &locale)
{
    if (!IsValidTag(locale)) {
        return false;
    }
    return SetParameter(LOCALE_KEY, locale.data()) == 0;
}

bool LocaleConfig::IsValidLanguage(const string &language)
{
    string::size_type size = language.size();
    if ((size != LANGUAGE_LEN) && (size != LANGUAGE_LEN + 1)) {
        return false;
    }
    for (size_t i = 0; i < size; ++i) {
        if ((language[i] > 'z') || (language[i] < 'a')) {
            return false;
        }
    }
    return true;
}

bool LocaleConfig::IsValidScript(const string &script)
{
    string::size_type size = script.size();
    if (size != LocaleInfo::SCRIPT_LEN) {
        return false;
    }
    char first = script[0];
    if ((first < 'A') || (first > 'Z')) {
        return false;
    }
    for (string::size_type i = 1; i < LocaleInfo::SCRIPT_LEN; ++i) {
        if ((script[i] > 'z') || (script[i] < 'a')) {
            return false;
        }
    }
    return true;
}

bool LocaleConfig::IsValidRegion(const string &region)
{
    string::size_type size = region.size();
    if (size != LocaleInfo::REGION_LEN) {
        return false;
    }
    for (size_t i = 0; i < LocaleInfo::REGION_LEN; ++i) {
        if ((region[i] > 'Z') || (region[i] < 'A')) {
            return false;
        }
    }
    return true;
}

bool LocaleConfig::IsValidTag(const string &tag)
{
    if (tag.size() == 0) {
        return false;
    }
    vector<string> splits;
    Split(tag, "-", splits);
    if (!IsValidLanguage(splits[0])) {
        return false;
    }
    return true;
}

void LocaleConfig::Split(const string &src, const string &sep, vector<string> &dest)
{
    string::size_type begin = 0;
    string::size_type end = src.find(sep);
    while (end != string::npos) {
        dest.push_back(src.substr(begin, end - begin));
        begin = end + sep.size();
        end = src.find(sep, begin);
    }
    if (begin != src.size()) {
        dest.push_back(src.substr(begin));
    }
}

// language in white languages should have script.
void LocaleConfig::GetSystemLanguages(vector<string> &ret)
{
    for (auto item : whiteLanguages) {
        ret.push_back(item);
    }
}

const unordered_set<string>& LocaleConfig::GetSupportedLocales()
{
    return supportedLocales;
}

const unordered_set<string>& LocaleConfig::GetSupportedRegions()
{
    return supportedRegions;
}

void LocaleConfig::GetSystemCountries(vector<string> &ret)
{
    for (auto item : supportedRegions) {
        ret.push_back(item);
    }
}

bool LocaleConfig::IsSuggested(const string &language)
{
    unordered_set<string> relatedLocales;
    vector<string> simCountries;
    GetCountriesFromSim(simCountries);
    GetRelatedLocales(relatedLocales, simCountries);
    for (auto iter = relatedLocales.begin(); iter != relatedLocales.end();) {
        if (whiteLanguages.find(*iter) == whiteLanguages.end()) {
            iter = relatedLocales.erase(iter);
        } else {
            ++iter;
        }
    }
    string mainLanguage = GetMainLanguage(language);
    return relatedLocales.find(mainLanguage) != relatedLocales.end();
}

bool LocaleConfig::IsSuggested(const std::string &language, const std::string &region)
{
    unordered_set<string> relatedLocales;
    vector<string> countries { region };
    GetRelatedLocales(relatedLocales, countries);
    for (auto iter = relatedLocales.begin(); iter != relatedLocales.end();) {
        if (whiteLanguages.find(*iter) == whiteLanguages.end()) {
            iter = relatedLocales.erase(iter);
        } else {
            ++iter;
        }
    }
    string mainLanguage = GetMainLanguage(language);
    return relatedLocales.find(mainLanguage) != relatedLocales.end();
}

void LocaleConfig::GetRelatedLocales(unordered_set<string> &relatedLocales, vector<string> countries)
{
    // remove unsupported countries
    const unordered_set<string> &regions = GetSupportedRegions();
    for (auto iter = countries.begin(); iter != countries.end();) {
        if (regions.find(*iter) == regions.end()) {
            iter = countries.erase(iter);
        } else {
            ++iter;
        }
    }
    const unordered_set<string> &locales = GetSupportedLocales();
    for (string locale : locales) {
        bool find = false;
        for (string country : countries) {
            if (locale.find(country) != string::npos) {
                find = true;
                break;
            }
        }
        if (!find) {
            continue;
        }
        string mainLanguage = GetMainLanguage(locale);
        if (mainLanguage != "") {
            relatedLocales.insert(mainLanguage);
        }
    }
}

void LocaleConfig::GetCountriesFromSim(vector<string> &simCountries)
{
    simCountries.push_back(GetSystemRegion());
    Telephony::SimCardManager simCardManager;
    simCardManager.ConnectService();
    simCountries.push_back(Str16ToStr8(simCardManager.GetIsoCountryCodeForSim(0)));
}

void LocaleConfig::GetListFromFile(const char *path, const char *resourceName, unordered_set<string> &ret)
{
    xmlKeepBlanksDefault(0);
    if (path == nullptr) {
        return;
    }
    xmlDocPtr doc = xmlParseFile(path);
    if (doc == nullptr) {
        return;
    }
    xmlNodePtr cur = xmlDocGetRootElement(doc);
    if (cur == nullptr || (xmlStrcmp(cur->name, reinterpret_cast<const xmlChar *>(resourceName))) != 0) {
        xmlFreeDoc(doc);
        return;
    }
    cur = cur->xmlChildrenNode;
    xmlChar *content = nullptr;
    while (cur != nullptr) {
        content = xmlNodeGetContent(cur);
        if (content != nullptr) {
            ret.insert(reinterpret_cast<const char*>(content));
            xmlFree(content);
            cur = cur->next;
        } else {
            break;
        }
    }
    xmlFreeDoc(doc);
}

void LocaleConfig::Expunge(unordered_set<string> &src, const unordered_set<string> &another)
{
    for (auto iter = src.begin(); iter != src.end();) {
        if (another.find(*iter) != another.end()) {
            iter = src.erase(iter);
        } else {
            ++iter;
        }
    }
}

bool LocaleConfig::InitializeLists()
{
    SetHwIcuDirectory();
    GetListFromFile(SUPPORTED_REGIONS_PATH, SUPPORTED_REGIONS_NAME, supportedRegions);
    unordered_set<string> forbiddenRegions;
    GetListFromFile(FORBIDDEN_REGIONS_PATH, FORBIDDEN_REGIONS_NAME, forbiddenRegions);
    Expunge(supportedRegions, forbiddenRegions);
    GetListFromFile(WHITE_LANGUAGES_PATH, WHITE_LANGUAGES_NAME, whiteLanguages);
    unordered_set<string> forbiddenLanguages;
    GetListFromFile(FORBIDDEN_LANGUAGES_PATH, FORBIDDEN_LANGUAGES_NAME, forbiddenLanguages);
    Expunge(whiteLanguages, forbiddenLanguages);
    GetListFromFile(SUPPORTED_LOCALES_PATH, SUPPORTED_LOCALES_NAME, supportedLocales);
    return true;
}

string LocaleConfig::GetRegionChangeLocale(const string &languageTag, const string &region)
{
    UErrorCode status = U_ZERO_ERROR;
    const icu::Locale origin = icu::Locale::forLanguageTag(languageTag, status);
    if (status != U_ZERO_ERROR) {
        return "";
    }
    icu::LocaleBuilder builder = icu::LocaleBuilder().setLanguage(origin.getLanguage()).
        setScript(origin.getScript()).setRegion(region);
    icu::Locale temp = builder.setExtension('u', "").build(status);
    if (status != U_ZERO_ERROR) {
        return "";
    }
    string ret = temp.toLanguageTag<string>(status);
    return (status != U_ZERO_ERROR) ? "" : ret;
}

string LocaleConfig::GetMainLanguage(const string &language)
{
    UErrorCode status = U_ZERO_ERROR;
    icu::Locale origin = icu::Locale::forLanguageTag(language, status);
    if (status != U_ZERO_ERROR) {
        return "";
    }
    origin.addLikelySubtags(status);
    if (status != U_ZERO_ERROR) {
        return "";
    }
    icu::LocaleBuilder builder = icu::LocaleBuilder().setLanguage(origin.getLanguage()).
        setScript(origin.getScript()).setRegion(origin.getCountry());
    icu::Locale temp = builder.setExtension('u', "").build(status);
    string fullLanguage = temp.toLanguageTag<string>(status);
    if (status != U_ZERO_ERROR) {
        return "";
    }
    if (dialectMap.find(fullLanguage) != dialectMap.end()) {
        return dialectMap[fullLanguage];
    }
    builder.setRegion("");
    temp = builder.build(status);
    fullLanguage = temp.toLanguageTag<string>(status);
    if (status != U_ZERO_ERROR) {
        return "";
    }
    return fullLanguage;
}

string LocaleConfig::GetDisplayLanguage(const string &language, const string &displayLocale, bool sentenceCase)
{
    UErrorCode status = U_ZERO_ERROR;
    icu::Locale originLocale = icu::Locale::forLanguageTag(language, status);
    originLocale.addLikelySubtags(status);
    if (status != U_ZERO_ERROR) {
        return "";
    }
    const icu::Locale locale = icu::Locale::forLanguageTag(displayLocale, status);
    if (status != U_ZERO_ERROR) {
        return "";
    }
    icu::UnicodeString displayLang;
    originLocale.getDisplayLanguage(locale, displayLang);
    if (sentenceCase) {
        UChar ch = ucase_toupper(displayLang.char32At(0));
        displayLang.replace(0, 1, ch);
    }
    string temp;
    displayLang.toUTF8String(temp);
    return temp;
}

string LocaleConfig::GetDisplayRegion(const string &region, const string &displayLocale, bool sentenceCase)
{
    UErrorCode status = U_ZERO_ERROR;
    icu::Locale originLocale;
    if (IsValidRegion(region)) {
        icu::LocaleBuilder builder = icu::LocaleBuilder().setRegion(region);
        originLocale = builder.build(status);
    } else {
        originLocale = icu::Locale::forLanguageTag(region, status);
        originLocale.addLikelySubtags(status);
    }
    if (status != U_ZERO_ERROR) {
        return "";
    }
    icu::Locale locale = icu::Locale::forLanguageTag(displayLocale, status);
    if (status != U_ZERO_ERROR) {
        return "";
    }
    icu::UnicodeString displayRegion;
    originLocale.getDisplayCountry(locale, displayRegion);
    if (sentenceCase) {
        UChar ch = ucase_toupper(displayRegion.char32At(0));
        displayRegion.replace(0, 1, ch);
    }
    string temp;
    displayRegion.toUTF8String(temp);
    return temp;
}
} // I18n
} // Global
} // OHOS
