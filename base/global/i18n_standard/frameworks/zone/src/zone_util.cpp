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

#include "zone_util.h"
#include <algorithm>
#include <cctype>
#include "ohos/init_data.h"
#include "strenum.h"
#include "timezone.h"

using namespace OHOS::Global::I18n;
using namespace icu;
using namespace std;


unordered_map<string, string> ZoneUtil::defaultMap = {
    {"AQ", "Antarctica/McMurdo"},
    {"AR", "America/Argentina/Buenos_Aires"},
    {"AU", "Australia/Sydney"},
    {"BR", "America/Noronha"},
    {"CA", "America/St_Johns"},
    {"CD", "Africa/Kinshasa"},
    {"CL", "America/Santiago"},
    {"CN", "Asia/Shanghai"},
    {"CY", "Asia/Nicosia"},
    {"DE", "Europe/Berlin"},
    {"EC", "America/Guayaquil"},
    {"ES", "Europe/Madrid"},
    {"FM", "Pacific/Pohnpei"},
    {"GL", "America/Godthab"},
    {"ID", "Asia/Jakarta"},
    {"KI", "Pacific/Tarawa"},
    {"KZ", "Asia/Almaty"},
    {"MH", "Pacific/Majuro"},
    {"MN", "Asia/Ulaanbaatar"},
    {"MX", "America/Mexico_City"},
    {"MY", "Asia/Kuala_Lumpur"},
    {"NZ", "Pacific/Auckland"},
    {"PF", "Pacific/Tahiti"},
    {"PG", "Pacific/Port_Moresby"},
    {"PS", "Asia/Gaza"},
    {"PT", "Europe/Lisbon"},
    {"RU", "Europe/Moscow"},
    {"UA", "Europe/Kiev"},
    {"UM", "Pacific/Wake"},
    {"US", "America/New_York"},
    {"UZ", "Asia/Tashkent"},
};

bool ZoneUtil::icuInitialized = ZoneUtil::Init();

string ZoneUtil::GetDefaultZone(const string &country)
{
    string temp(country);
    for (size_t i = 0; i < temp.size(); i++) {
        temp[i] = toupper(temp[i]);
    }
    if (defaultMap.find(temp) != defaultMap.end()) {
        return defaultMap[temp];
    }
    string ret;
    StringEnumeration *strEnum = TimeZone::createEnumeration(temp.c_str());
    GetString(strEnum, ret);
    if (strEnum != nullptr) {
        delete strEnum;
    }
    return ret;
}

string ZoneUtil::GetDefaultZone(const int32_t number)
{
    string *region_code = new(nothrow) string();
    if (region_code == nullptr) {
        return "";
    }
    phone_util.GetRegionCodeForCountryCode(number, region_code);
    if (region_code == nullptr) {
        return "";
    }
    string ret = GetDefaultZone(*region_code);
    if (region_code != nullptr) {
        delete region_code;
    }
    return ret;
}

string ZoneUtil::GetDefaultZone(const string country, const int32_t offset)
{
    UErrorCode status = U_ZERO_ERROR;
    StringEnumeration *strEnum =
        TimeZone::createTimeZoneIDEnumeration(UCAL_ZONE_TYPE_ANY, country.c_str(), &offset, status);
    if (status != U_ZERO_ERROR) {
        return "";
    }
    string ret;
    GetString(strEnum, ret);
    if (strEnum != nullptr) {
        delete strEnum;
        strEnum = nullptr;
    }
    return ret;
}

string ZoneUtil::GetDefaultZone(const int32_t number, const int32_t offset)
{
    string *region_code = new(nothrow) string();
    if (region_code == nullptr) {
        return "";
    }
    phone_util.GetRegionCodeForCountryCode(number, region_code);
    if (region_code == nullptr) {
        return "";
    }
    string ret = GetDefaultZone(*region_code, offset);
    if (region_code != nullptr) {
        delete region_code;
        region_code = nullptr;
    }
    return ret;
}

void ZoneUtil::GetZoneList(const string country, vector<string> &retVec)
{
    StringEnumeration *strEnum = TimeZone::createEnumeration(country.c_str());
    GetList(strEnum, retVec);
    if (strEnum != nullptr) {
        delete strEnum;
        strEnum = nullptr;
    }
}

void ZoneUtil::GetZoneList(const string country, const int32_t offset, vector<string> &retVec)
{
    UErrorCode status = U_ZERO_ERROR;
    StringEnumeration *strEnum =
        TimeZone::createTimeZoneIDEnumeration(UCAL_ZONE_TYPE_ANY, country.c_str(), &offset, status);
    if (status != U_ZERO_ERROR) {
        delete strEnum;
        strEnum = nullptr;
        return;
    }
    GetList(strEnum, retVec);
    if (strEnum != nullptr) {
        delete strEnum;
        strEnum = nullptr;
    }
}

void ZoneUtil::GetString(StringEnumeration *strEnum, string& ret)
{
    UErrorCode status = U_ZERO_ERROR;
    UnicodeString uniString;
    if (strEnum == nullptr) {
        return;
    }
    int32_t count = strEnum->count(status);
    if ((status != U_ZERO_ERROR) || count <= 0) {
        return;
    }
    const UnicodeString *uniStr = strEnum->snext(status);
    if ((status != U_ZERO_ERROR) || (uniStr == nullptr)) {
        return;
    }
    UnicodeString canonicalUnistring;
    TimeZone::getCanonicalID(*uniStr, canonicalUnistring, status);
    if (status != U_ZERO_ERROR) {
        return;
    }
    canonicalUnistring.toUTF8String(ret);
    return;
}

void ZoneUtil::GetList(StringEnumeration *strEnum, vector<string> &retVec)
{
    if (strEnum == nullptr) {
        return;
    }
    UErrorCode status = U_ZERO_ERROR;
    int32_t count = strEnum->count(status);
    if (count <= 0 || status != U_ZERO_ERROR) {
        return;
    }
    while (count > 0) {
        const UnicodeString *uniStr = strEnum->snext(status);
        if ((uniStr == nullptr) || (status != U_ZERO_ERROR)) {
            retVec.clear();
            break;
        }
        UnicodeString canonicalUnistring;
        TimeZone::getCanonicalID(*uniStr, canonicalUnistring, status);
        if (status != U_ZERO_ERROR) {
            retVec.clear();
            break;
        }
        string canonicalString = "";
        canonicalUnistring.toUTF8String(canonicalString);
        if ((canonicalString != "") && (find(retVec.begin(), retVec.end(), canonicalString) == retVec.end())) {
            retVec.push_back(canonicalString);
        }
        --count;
    }
    return;
}

bool ZoneUtil::Init()
{
    SetHwIcuDirectory();
    return true;
}