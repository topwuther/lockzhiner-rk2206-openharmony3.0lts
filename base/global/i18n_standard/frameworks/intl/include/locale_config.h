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
#ifndef OHOS_GLOBAL_I18N_LOCALE_CONFIG_H
#define OHOS_GLOBAL_I18N_LOCALE_CONFIG_H

#include <vector>
#include <string>
#include <unordered_map>
#include <unordered_set>

namespace OHOS {
namespace Global {
namespace I18n {
class LocaleConfig {
public:
    LocaleConfig() = default;
    virtual ~LocaleConfig() = default;
    static bool SetSystemLanguage(const std::string &language);
    static bool SetSystemRegion(const std::string &region);
    static bool SetSystemLocale(const std::string &locale);
    static std::string GetSystemLanguage();
    static std::string GetSystemRegion();
    static std::string GetSystemLocale();
    static void GetSystemLanguages(std::vector<std::string> &ret);
    static void GetSystemCountries(std::vector<std::string> &ret);
    static bool IsSuggested(const std::string &language);
    static bool IsSuggested(const std::string &language, const std::string &region);
    static std::string GetDisplayLanguage(const std::string &language, const std::string &displayLocale,
        bool sentenceCase);
    static std::string GetDisplayRegion(const std::string &region, const std::string &displayLocale,
        bool sentenceCase);
private:
    static bool IsValidLanguage(const std::string &language);
    static bool IsValidScript(const std::string &script);
    static bool IsValidRegion(const std::string &region);
    static bool IsValidTag(const std::string &tag);
    static void Split(const std::string &src, const std::string &sep, std::vector<std::string> &dest);
    static constexpr uint32_t LANGUAGE_LEN = 2;
    static constexpr uint32_t LOCALE_ITEM_COUNT = 3;
    static constexpr uint32_t SCRIPT_OFFSET = 2;
    static const char *LANGUAGE_KEY;
    static const char *LOCALE_KEY;
    static const char *DEFAULT_LOCALE;
    static const char *DEFAULT_LANGUAGE;
    static const char *DEFAULT_REGION;
    static const char *SUPPORTED_LOCALES_PATH;
    static const char *SUPPORTED_REGIONS_PATH;
    static constexpr int CONFIG_LEN = 128;
    static const char *SUPPORTED_LOCALES_NAME;
    static const char *SUPPORTED_REGIONS_NAME;
    static const char *WHITE_LANGUAGES_NAME;
    static const char *WHITE_LANGUAGES_PATH;
    static const char *FORBIDDEN_REGIONS_PATH;
    static const char *FORBIDDEN_REGIONS_NAME;
    static const char *FORBIDDEN_LANGUAGES_PATH;
    static const char *FORBIDDEN_LANGUAGES_NAME;
    static const std::unordered_set<std::string>& GetSupportedLocales();
    static const std::unordered_set<std::string>& GetForbiddenRegions();
    static const std::unordered_set<std::string>& GetSupportedRegions();
    static void GetCountriesFromSim(std::vector<std::string> &simCountries);
    static void GetRelatedLocales(std::unordered_set<std::string> &relatedLocales,
        const std::vector<std::string> countries);
    static std::string GetRegionChangeLocale(const std::string &languageTag, const std::string &region);
    static void GetListFromFile(const char *path, const char *resourceName, std::unordered_set<std::string> &ret);
    static void Expunge(std::unordered_set<std::string> &src, const std::unordered_set<std::string> &another);
    static std::string GetMainLanguage(const std::string &language);
    static std::unordered_set<std::string> supportedLocales;
    static std::unordered_set<std::string> supportedRegions;
    static std::unordered_set<std::string> whiteLanguages;
    static std::unordered_map<std::string, std::string> dialectMap;
    static bool listsInitialized;
    static bool InitializeLists();
};
} // I18n
} // Global
} // OHOS
#endif
