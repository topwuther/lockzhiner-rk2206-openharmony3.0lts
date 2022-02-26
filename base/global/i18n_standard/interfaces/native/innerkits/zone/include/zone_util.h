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

#ifndef OHOS_GLOBAL_I18N_ZONE_UTIL_H
#define OHOS_GLOBAL_I18N_ZONE_UTIL_H

#include <string>
#include <unordered_map>
#include <vector>
#include "phonenumbers/phonenumberutil.h"
#include "strenum.h"

namespace OHOS {
namespace Global {
namespace I18n {
class ZoneUtil {
public:
    /**
     * default constructor
     */
    ZoneUtil() : phone_util(*i18n::phonenumbers::PhoneNumberUtil::GetInstance()) {}

    /**
     * deconstructor
     */
    ~ZoneUtil() {}

    /**
     * @brief Get the default timezone for the given country
     *
     * @param country Indicating the country code
     * @return Returns the default timezone if the country code is valid, otherwise
     * returns an empty string.
     */
    std::string GetDefaultZone(const std::string &country);

    /**
     * @brief Get the default timezone for the given region code
     *
     * @param number Indicating the region code, for example 86 can
     * be used to retrieve the default timezone of China.
     * @return Returns the default timezone name if the region code is valid, otherwise
     * returns an empty string.
     */
    std::string GetDefaultZone(const int32_t number);

    /**
     * @brief Get the default timezone name for the given country code
     *
     * @param country Indicating the country code
     * @param offset Indicating the offset from GMT(in milliseconds)
     * @return Returns the default timezone name if the country code is valid, otherwise
     * returns an empty string.
     */
    std::string GetDefaultZone(const std::string country, const int32_t offset);

    /**
     * @brief Get the default timezone name for the given region code
     *
     * @param number Indicating the region code, for example 86 can
     * be used to retrieve the default timezone of China.
     * @param offset Indicating the offset from GMT(in milliseconds).
     * @return Returns the default timezone name if the country code is valid, otherwise
     * returns an empty string.
     */
    std::string GetDefaultZone(const int32_t number, const int32_t offset);

    /**
     * @brief Get the timezone list for the given country code
     *
     * @param country Indicating the country code
     * @param retVec used to store the returned timezones
     */
    void GetZoneList(const std::string country, std::vector<std::string> &retVec);

    /**
     * @brief Get the timezone list for the given country code
     *
     * @param country Indicating the country code
     * @param offset Indicating the offset from GMT(in milliseconds)
     * @param retVec used to store the returned timezones
     */
    void GetZoneList(const std::string country, const int32_t offset, std::vector<std::string> &retVec);
private:
    const i18n::phonenumbers::PhoneNumberUtil &phone_util;
    static std::unordered_map<std::string, std::string> defaultMap;
    static bool icuInitialized;
    static void GetList(icu::StringEnumeration *strEnum, std::vector<std::string> &ret);
    static void GetString(icu::StringEnumeration *strEnum, std::string &ret);
    static bool Init();
};
} // I18n
} // Global
} // OHOS
#endif
