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

#include "intl_addon.h"

#include <vector>

#include "hilog/log.h"
#include "node_api.h"

namespace OHOS {
namespace Global {
namespace I18n {
#define GET_PARAMS(env, info, num)      \
    size_t argc = num;                  \
    napi_value argv[num];               \
    napi_value thisVar = nullptr;       \
    void *data = nullptr;               \
    napi_get_cb_info(env, info, &argc, argv, &thisVar, &data)

static constexpr OHOS::HiviewDFX::HiLogLabel LABEL = { LOG_CORE, 0xD001E00, "IntlJs" };
using namespace OHOS::HiviewDFX;
static napi_ref *g_constructor = nullptr;

IntlAddon::IntlAddon() : env_(nullptr), wrapper_(nullptr) {}

IntlAddon::~IntlAddon()
{
    napi_delete_reference(env_, wrapper_);
}

void IntlAddon::Destructor(napi_env env, void *nativeObject, void *hint)
{
    if (nativeObject == nullptr) {
        return;
    }
    reinterpret_cast<IntlAddon *>(nativeObject)->~IntlAddon();
}

napi_value IntlAddon::InitLocale(napi_env env, napi_value exports)
{
    napi_status status;
    napi_property_descriptor properties[] = {
        DECLARE_NAPI_GETTER("language", GetLanguage),
        DECLARE_NAPI_GETTER("baseName", GetBaseName),
        DECLARE_NAPI_GETTER("region", GetRegion),
        DECLARE_NAPI_GETTER("script", GetScript),
        DECLARE_NAPI_GETTER("calendar", GetCalendar),
        DECLARE_NAPI_GETTER("collation", GetCollation),
        DECLARE_NAPI_GETTER("hourCycle", GetHourCycle),
        DECLARE_NAPI_GETTER("numberingSystem", GetNumberingSystem),
        DECLARE_NAPI_GETTER("numeric", GetNumeric),
        DECLARE_NAPI_GETTER("caseFirst", GetCaseFirst),
        DECLARE_NAPI_FUNCTION("toString", ToString),
        DECLARE_NAPI_FUNCTION("minimize", Minimize),
        DECLARE_NAPI_FUNCTION("maximize", Maximize),
    };

    napi_value constructor;
    status = napi_define_class(env, "Locale", NAPI_AUTO_LENGTH, LocaleConstructor, nullptr,
        sizeof(properties) / sizeof(napi_property_descriptor), properties, &constructor);
    if (status != napi_ok) {
        HiLog::Error(LABEL, "Define class failed when InitLocale");
        return nullptr;
    }

    status = napi_set_named_property(env, exports, "Locale", constructor);
    if (status != napi_ok) {
        HiLog::Error(LABEL, "Set property failed when InitLocale");
        return nullptr;
    }
    g_constructor = new (std::nothrow) napi_ref;
    if (g_constructor == nullptr) {
        HiLog::Error(LABEL, "Failed to create ref at init");
        return nullptr;
    }
    status = napi_create_reference(env, constructor, 1, g_constructor);
    if (status != napi_ok) {
        HiLog::Error(LABEL, "Failed to create reference at init");
        return nullptr;
    }
    return exports;
}

napi_value IntlAddon::InitDateTimeFormat(napi_env env, napi_value exports)
{
    napi_status status;
    napi_property_descriptor properties[] = {
        DECLARE_NAPI_FUNCTION("format", FormatDateTime),
        DECLARE_NAPI_FUNCTION("formatRange", FormatDateTimeRange),
        DECLARE_NAPI_FUNCTION("resolvedOptions", GetDateTimeResolvedOptions)
    };

    napi_value constructor;
    status = napi_define_class(env, "DateTimeFormat", NAPI_AUTO_LENGTH, DateTimeFormatConstructor, nullptr,
        sizeof(properties) / sizeof(napi_property_descriptor), properties, &constructor);
    if (status != napi_ok) {
        HiLog::Error(LABEL, "Define class failed when InitDateTimeFormat");
        return nullptr;
    }

    status = napi_set_named_property(env, exports, "DateTimeFormat", constructor);
    if (status != napi_ok) {
        HiLog::Error(LABEL, "Set property failed when InitDateTimeFormat");
        return nullptr;
    }
    return exports;
}

napi_value IntlAddon::InitNumberFormat(napi_env env, napi_value exports)
{
    napi_status status;
    napi_property_descriptor properties[] = {
        DECLARE_NAPI_FUNCTION("format", FormatNumber),
        DECLARE_NAPI_FUNCTION("resolvedOptions", GetNumberResolvedOptions)
    };

    napi_value constructor;
    status = napi_define_class(env, "NumberFormat", NAPI_AUTO_LENGTH, NumberFormatConstructor, nullptr,
        sizeof(properties) / sizeof(napi_property_descriptor), properties, &constructor);
    if (status != napi_ok) {
        HiLog::Error(LABEL, "Define class failed when InitNumberFormat");
        return nullptr;
    }

    status = napi_set_named_property(env, exports, "NumberFormat", constructor);
    if (status != napi_ok) {
        HiLog::Error(LABEL, "Set property failed when InitNumberFormat");
        return nullptr;
    }
    return exports;
}

void GetOptionValue(napi_env env, napi_value options, const std::string &optionName,
    std::map<std::string, std::string> &map)
{
    napi_value optionValue = nullptr;
    napi_valuetype type = napi_undefined;
    napi_status status = napi_typeof(env, options, &type);
    if (status != napi_ok && type != napi_object) {
        HiLog::Error(LABEL, "Get option failed, option is not an object");
        return;
    }
    bool hasProperty = false;
    napi_status propStatus = napi_has_named_property(env, options, optionName.c_str(), &hasProperty);
    if (propStatus == napi_ok && hasProperty) {
        status = napi_get_named_property(env, options, optionName.c_str(), &optionValue);
        if (status == napi_ok) {
            size_t len;
            napi_get_value_string_utf8(env, optionValue, nullptr, 0, &len);
            std::vector<char> optionBuf(len + 1);
            status = napi_get_value_string_utf8(env, optionValue, optionBuf.data(), len + 1, &len);
            map.insert(make_pair(optionName, optionBuf.data()));
        }
    }
}

void GetIntegerOptionValue(napi_env env, napi_value options, const std::string &optionName,
    std::map<std::string, std::string> &map)
{
    napi_value optionValue = nullptr;
    napi_valuetype type = napi_undefined;
    napi_status status = napi_typeof(env, options, &type);
    if (status != napi_ok && type != napi_object) {
        HiLog::Error(LABEL, "Set option failed, option is not an object");
        return;
    }
    bool hasProperty = false;
    napi_status propStatus = napi_has_named_property(env, options, optionName.c_str(), &hasProperty);
    if (propStatus == napi_ok && hasProperty) {
        status = napi_get_named_property(env, options, optionName.c_str(), &optionValue);
        if (status == napi_ok) {
            int64_t integerValue = -1;
            napi_get_value_int64(env, optionValue, &integerValue);
            if (integerValue != -1) {
                map.insert(make_pair(optionName, std::to_string(integerValue)));
            }
        }
    }
}

void GetBoolOptionValue(napi_env env, napi_value options, const std::string &optionName,
    std::map<std::string, std::string> &map)
{
    napi_value optionValue = nullptr;
    napi_valuetype type = napi_undefined;
    napi_status status = napi_typeof(env, options, &type);
    if (status != napi_ok && type != napi_object) {
        HiLog::Error(LABEL, "Set option failed, option is not an object");
        return;
    }
    bool hasProperty = false;
    napi_status propStatus = napi_has_named_property(env, options, optionName.c_str(), &hasProperty);
    if (propStatus == napi_ok && hasProperty) {
        status = napi_get_named_property(env, options, optionName.c_str(), &optionValue);
        if (status == napi_ok) {
            bool boolValue = false;
            napi_get_value_bool(env, optionValue, &boolValue);
            std::string value = boolValue ? "true" : "false";
            map.insert(make_pair(optionName, value));
        }
    }
}

void GetDateOptionValues(napi_env env, napi_value options, std::map<std::string, std::string> &map)
{
    GetOptionValue(env, options, "calendar", map);
    GetOptionValue(env, options, "dateStyle", map);
    GetOptionValue(env, options, "timeStyle", map);
    GetOptionValue(env, options, "hourCycle", map);
    GetOptionValue(env, options, "timeZone", map);
    GetOptionValue(env, options, "timeZoneName", map);
    GetOptionValue(env, options, "numberingSystem", map);
    GetBoolOptionValue(env, options, "hour12", map);
    GetOptionValue(env, options, "weekday", map);
    GetOptionValue(env, options, "era", map);
    GetOptionValue(env, options, "year", map);
    GetOptionValue(env, options, "month", map);
    GetOptionValue(env, options, "day", map);
    GetOptionValue(env, options, "hour", map);
    GetOptionValue(env, options, "minute", map);
    GetOptionValue(env, options, "second", map);
    GetOptionValue(env, options, "localeMatcher", map);
    GetOptionValue(env, options, "formatMatcher", map);
    GetOptionValue(env, options, "dayPeriod", map);
}

napi_value IntlAddon::LocaleConstructor(napi_env env, napi_callback_info info)
{
    // Need to get one parameter of a locale in string format to create Locale object.
    size_t argc = 2;
    napi_value argv[2] = { 0 };
    napi_value thisVar = nullptr;
    void *data = nullptr;
    napi_status status = napi_get_cb_info(env, info, &argc, argv, &thisVar, &data);
    napi_valuetype valueType = napi_valuetype::napi_undefined;
    napi_typeof(env, argv[0], &valueType);
    if (valueType != napi_valuetype::napi_string) {
        napi_throw_type_error(env, nullptr, "Parameter type does not match");
        return nullptr;
    }
    size_t len;
    status = napi_get_value_string_utf8(env, argv[0], nullptr, 0, &len);
    if (status != napi_ok) {
        HiLog::Error(LABEL, "Get locale tag length failed");
        return nullptr;
    }
    std::vector<char> buf(len + 1);
    status = napi_get_value_string_utf8(env, argv[0], buf.data(), len + 1, &len);
    if (status != napi_ok) {
        HiLog::Error(LABEL, "Get locale tag failed");
        return nullptr;
    }
    std::map<std::string, std::string> map = {};
    if (argv[1] != nullptr) {
        GetOptionValue(env, argv[1], "calendar", map);
        GetOptionValue(env, argv[1], "collation", map);
        GetOptionValue(env, argv[1], "hourCycle", map);
        GetOptionValue(env, argv[1], "numberingSystem", map);
        GetBoolOptionValue(env, argv[1], "numeric", map);
        GetOptionValue(env, argv[1], "caseFirst", map);
    }

    std::unique_ptr<IntlAddon> obj = std::make_unique<IntlAddon>();
    if (obj == nullptr) {
        HiLog::Error(LABEL, "Create IntlAddon failed");
        return nullptr;
    }

    status =
        napi_wrap(env, thisVar, reinterpret_cast<void *>(obj.get()), IntlAddon::Destructor, nullptr, &obj->wrapper_);
    if (status != napi_ok) {
        HiLog::Error(LABEL, "Wrap IntlAddon failed");
        return nullptr;
    }

    std::string localeTag = buf.data();
    if (!obj->InitLocaleContext(env, info, localeTag, map)) {
        return nullptr;
    }

    obj.release();

    return thisVar;
}

bool IntlAddon::InitLocaleContext(napi_env env, napi_callback_info info, const std::string localeTag,
    std::map<std::string, std::string> &map)
{
    napi_value global;
    napi_status status = napi_get_global(env, &global);
    if (status != napi_ok) {
        HiLog::Error(LABEL, "Get global failed");
        return false;
    }
    env_ = env;
    locale_ = std::make_unique<LocaleInfo>(localeTag, map);

    return locale_ != nullptr;
}

void GetLocaleTags(napi_env env, napi_value rawLocaleTag, std::vector<std::string> &localeTags)
{
    size_t len;
    napi_status status = napi_get_value_string_utf8(env, rawLocaleTag, nullptr, 0, &len);
    if (status != napi_ok) {
        HiLog::Error(LABEL, "Get locale tag length failed");
        return;
    }
    std::vector<char> buf(len + 1);
    status = napi_get_value_string_utf8(env, rawLocaleTag, buf.data(), len + 1, &len);
    if (status != napi_ok) {
        HiLog::Error(LABEL, "Get locale tag failed");
        return;
    }
    localeTags.push_back(buf.data());
}

napi_value IntlAddon::DateTimeFormatConstructor(napi_env env, napi_callback_info info)
{
    // Need to get one parameter of a locale in string format to create DateTimeFormat object.
    size_t argc = 2;
    napi_value argv[2] = { 0 };
    napi_value thisVar = nullptr;
    void *data = nullptr;
    napi_status status = napi_get_cb_info(env, info, &argc, argv, &thisVar, &data);
    napi_valuetype valueType = napi_valuetype::napi_undefined;
    napi_typeof(env, argv[0], &valueType);
    std::vector<std::string> localeTags;
    bool isArray = false;
    napi_is_array(env, argv[0], &isArray);
    if (valueType == napi_valuetype::napi_string) {
        GetLocaleTags(env, argv[0], localeTags);
    } else if (isArray) {
        uint32_t arrayLength = 0;
        napi_get_array_length(env, argv[0], &arrayLength);
        napi_value element;
        for (uint32_t i = 0; i < arrayLength; i++) {
            napi_get_element(env, argv[0], i, &element);
            GetLocaleTags(env, element, localeTags);
        }
    } else {
        return nullptr;
    }

    std::map<std::string, std::string> map = {};
    if (argv[1] != nullptr) {
        GetDateOptionValues(env, argv[1], map);
    }

    std::unique_ptr<IntlAddon> obj = std::make_unique<IntlAddon>();
    if (obj == nullptr) {
        HiLog::Error(LABEL, "Create IntlAddon failed");
        return nullptr;
    }

    status =
        napi_wrap(env, thisVar, reinterpret_cast<void *>(obj.get()), IntlAddon::Destructor, nullptr, &obj->wrapper_);
    if (status != napi_ok) {
        HiLog::Error(LABEL, "Wrap IntlAddon failed");
        return nullptr;
    }

    if (!obj->InitDateTimeFormatContext(env, info, localeTags, map)) {
        HiLog::Error(LABEL, "Init DateTimeFormat failed");
        return nullptr;
    }

    obj.release();
    return thisVar;
}

bool IntlAddon::InitDateTimeFormatContext(napi_env env, napi_callback_info info, std::vector<std::string> localeTags,
    std::map<std::string, std::string> &map)
{
    napi_value global;
    napi_status status = napi_get_global(env, &global);
    if (status != napi_ok) {
        HiLog::Error(LABEL, "Get global failed");
        return false;
    }
    env_ = env;
    datefmt_ = std::make_unique<DateTimeFormat>(localeTags, map);

    return datefmt_ != nullptr;
}

napi_value IntlAddon::FormatDateTime(napi_env env, napi_callback_info info)
{
    GET_PARAMS(env, info, 1); // Need to get one parameter of a date object to format.
    int64_t year = GetYear(env, argv, 0);
    int64_t month = GetMonth(env, argv, 0);
    int64_t day = GetDay(env, argv, 0);
    int64_t hour = GetHour(env, argv, 0);
    int64_t minute = GetMinute(env, argv, 0);
    int64_t second = GetSecond(env, argv, 0);
    if (year == -1 || month == -1 || day == -1 || hour == -1 || minute == -1 || second == -1) {
        return nullptr;
    }
    IntlAddon *obj = nullptr;
    napi_status status = napi_unwrap(env, thisVar, reinterpret_cast<void **>(&obj));
    if (status != napi_ok || obj == nullptr || obj->datefmt_ == nullptr) {
        HiLog::Error(LABEL, "Get DateTimeFormat object failed");
        return nullptr;
    }
    int64_t date[] = { year, month, day, hour, minute, second };
    std::string value = obj->datefmt_->Format(date, std::end(date) - std::begin(date));
    napi_value result;
    status = napi_create_string_utf8(env, value.c_str(), NAPI_AUTO_LENGTH, &result);
    if (status != napi_ok) {
        HiLog::Error(LABEL, "Create format string failed");
        return nullptr;
    }
    return result;
}

napi_value IntlAddon::FormatDateTimeRange(napi_env env, napi_callback_info info)
{
    GET_PARAMS(env, info, 2); // Need to get two parameter of date objects to format.
    int64_t firstYear = GetYear(env, argv, 0);
    int64_t firstMonth = GetMonth(env, argv, 0);
    int64_t firstDay = GetDay(env, argv, 0);
    int64_t firstHour = GetHour(env, argv, 0);
    int64_t firstMinute = GetMinute(env, argv, 0);
    int64_t firstSecond = GetSecond(env, argv, 0);
    int64_t firstDate[] = { firstYear, firstMonth, firstDay, firstHour, firstMinute, firstSecond };
    int64_t secondYear = GetYear(env, argv, 1);
    int64_t secondMonth = GetMonth(env, argv, 1);
    int64_t secondDay = GetDay(env, argv, 1);
    int64_t secondHour = GetHour(env, argv, 1);
    int64_t secondMinute = GetMinute(env, argv, 1);
    int64_t secondSecond = GetSecond(env, argv, 1);
    int64_t secondDate[] = { secondYear, secondMonth, secondDay, secondHour, secondMinute, secondSecond };
    if (firstYear == -1 || firstMonth == -1 || firstDay == -1 || firstHour == -1 || firstMinute == -1 ||
        firstSecond == -1) {
        return nullptr;
    }
    IntlAddon *obj = nullptr;
    napi_status status = napi_unwrap(env, thisVar, reinterpret_cast<void **>(&obj));
    if (status != napi_ok || obj == nullptr || obj->datefmt_ == nullptr) {
        HiLog::Error(LABEL, "Get DateTimeFormat object failed");
        return nullptr;
    }
    std::string value = obj->datefmt_->FormatRange(firstDate, std::end(firstDate) - std::begin(firstDate),
        secondDate, std::end(secondDate) - std::begin(secondDate));
    napi_value result;
    status = napi_create_string_utf8(env, value.c_str(), NAPI_AUTO_LENGTH, &result);
    if (status != napi_ok) {
        HiLog::Error(LABEL, "Create format string failed");
        return nullptr;
    }
    return result;
}

void GetNumberOptionValues(napi_env env, napi_value options, std::map<std::string, std::string> &map)
{
    GetOptionValue(env, options, "currency", map);
    GetOptionValue(env, options, "currencySign", map);
    GetOptionValue(env, options, "currencyDisplay", map);
    GetOptionValue(env, options, "unit", map);
    GetOptionValue(env, options, "unitDisplay", map);
    GetOptionValue(env, options, "compactDisplay", map);
    GetOptionValue(env, options, "signDisplay", map);
    GetOptionValue(env, options, "localeMatcher", map);
    GetOptionValue(env, options, "style", map);
    GetOptionValue(env, options, "numberingSystem", map);
    GetOptionValue(env, options, "notation", map);
    GetBoolOptionValue(env, options, "useGrouping", map);
    GetIntegerOptionValue(env, options, "minimumIntegerDigits", map);
    GetIntegerOptionValue(env, options, "minimumFractionDigits", map);
    GetIntegerOptionValue(env, options, "maximumFractionDigits", map);
    GetIntegerOptionValue(env, options, "minimumSignificantDigits", map);
    GetIntegerOptionValue(env, options, "maximumSignificantDigits", map);
}

napi_value IntlAddon::NumberFormatConstructor(napi_env env, napi_callback_info info)
{
    // Need to get one parameter of a locale in string format to create DateTimeFormat object.
    size_t argc = 2;
    napi_value argv[2] = { 0 };
    napi_value thisVar = nullptr;
    void *data = nullptr;
    napi_status status = napi_get_cb_info(env, info, &argc, argv, &thisVar, &data);
    napi_valuetype valueType = napi_valuetype::napi_undefined;
    napi_typeof(env, argv[0], &valueType);
    std::vector<std::string> localeTags;
    bool isArray = false;
    napi_is_array(env, argv[0], &isArray);
    if (valueType == napi_valuetype::napi_string) {
        GetLocaleTags(env, argv[0], localeTags);
    } else if (isArray) {
        uint32_t arrayLength = 0;
        napi_get_array_length(env, argv[0], &arrayLength);
        napi_value element;
        for (uint32_t i = 0; i < arrayLength; i++) {
            napi_get_element(env, argv[0], i, &element);
            GetLocaleTags(env, element, localeTags);
        }
    } else {
        return nullptr;
    }

    std::map<std::string, std::string> map = {};
    if (argv[1] != nullptr) {
        GetNumberOptionValues(env, argv[1], map);
    }

    std::unique_ptr<IntlAddon> obj = std::make_unique<IntlAddon>();
    if (obj == nullptr) {
        HiLog::Error(LABEL, "Create IntlAddon failed");
        return nullptr;
    }

    status =
        napi_wrap(env, thisVar, reinterpret_cast<void *>(obj.get()), IntlAddon::Destructor, nullptr, &obj->wrapper_);
    if (status != napi_ok) {
        HiLog::Error(LABEL, "Wrap IntlAddon failed");
        return nullptr;
    }

    if (!obj->InitNumberFormatContext(env, info, localeTags, map)) {
        HiLog::Error(LABEL, "Init NumberFormat failed");
        return nullptr;
    }

    obj.release();
    return thisVar;
}

bool IntlAddon::InitNumberFormatContext(napi_env env, napi_callback_info info, std::vector<std::string> localeTags,
    std::map<std::string, std::string> &map)
{
    napi_value global;
    napi_status status = napi_get_global(env, &global);
    if (status != napi_ok) {
        HiLog::Error(LABEL, "Get global failed");
        return false;
    }
    env_ = env;
    numberfmt_ = std::make_unique<NumberFormat>(localeTags, map);

    return numberfmt_ != nullptr;
}

int64_t IntlAddon::GetYear(napi_env env, napi_value *argv, int index)
{
    napi_value funcGetDateInfo;
    napi_status status = napi_get_named_property(env, argv[index], "getFullYear", &funcGetDateInfo);
    if (status != napi_ok) {
        HiLog::Error(LABEL, "Get year property failed");
        return -1;
    }
    napi_value ret_value;
    status = napi_call_function(env, argv[index], funcGetDateInfo, 0, nullptr, &ret_value);
    if (status != napi_ok) {
        HiLog::Error(LABEL, "Get year function failed");
        return -1;
    }
    int64_t year;
    status = napi_get_value_int64(env, ret_value, &year);
    if (status != napi_ok) {
        HiLog::Error(LABEL, "Get year failed");
        return -1;
    }
    return year;
}

int64_t IntlAddon::GetMonth(napi_env env, napi_value *argv, int index)
{
    napi_value funcGetDateInfo;
    napi_status status = napi_get_named_property(env, argv[index], "getMonth", &funcGetDateInfo);
    if (status != napi_ok) {
        HiLog::Error(LABEL, "Get month property failed");
        return -1;
    }
    napi_value ret_value;
    status = napi_call_function(env, argv[index], funcGetDateInfo, 0, nullptr, &ret_value);
    if (status != napi_ok) {
        HiLog::Error(LABEL, "Get month function failed");
        return -1;
    }
    int64_t month;
    status = napi_get_value_int64(env, ret_value, &month);
    if (status != napi_ok) {
        HiLog::Error(LABEL, "Get month failed");
        return -1;
    }
    return month;
}

int64_t IntlAddon::GetDay(napi_env env, napi_value *argv, int index)
{
    napi_value funcGetDateInfo;
    napi_status status = napi_get_named_property(env, argv[index], "getDate", &funcGetDateInfo);
    if (status != napi_ok) {
        HiLog::Error(LABEL, "Get day property failed");
        return -1;
    }
    napi_value ret_value;
    status = napi_call_function(env, argv[index], funcGetDateInfo, 0, nullptr, &ret_value);
    if (status != napi_ok) {
        HiLog::Error(LABEL, "Get day function failed");
        return -1;
    }
    int64_t day;
    status = napi_get_value_int64(env, ret_value, &day);
    if (status != napi_ok) {
        HiLog::Error(LABEL, "Get day failed");
        return -1;
    }
    return day;
}

int64_t IntlAddon::GetHour(napi_env env, napi_value *argv, int index)
{
    napi_value funcGetDateInfo;
    napi_status status = napi_get_named_property(env, argv[index], "getHours", &funcGetDateInfo);
    if (status != napi_ok) {
        HiLog::Error(LABEL, "Get hour property failed");
        return -1;
    }
    napi_value ret_value;
    status = napi_call_function(env, argv[index], funcGetDateInfo, 0, nullptr, &ret_value);
    if (status != napi_ok) {
        HiLog::Error(LABEL, "Get hour function failed");
        return -1;
    }
    int64_t hour;
    status = napi_get_value_int64(env, ret_value, &hour);
    if (status != napi_ok) {
        HiLog::Error(LABEL, "Get hour failed");
        return -1;
    }
    return hour;
}

int64_t IntlAddon::GetMinute(napi_env env, napi_value *argv, int index)
{
    napi_value funcGetDateInfo;
    napi_status status = napi_get_named_property(env, argv[index], "getMinutes", &funcGetDateInfo);
    if (status != napi_ok) {
        HiLog::Error(LABEL, "Get minute property failed");
        return -1;
    }
    napi_value ret_value;
    status = napi_call_function(env, argv[index], funcGetDateInfo, 0, nullptr, &ret_value);
    if (status != napi_ok) {
        HiLog::Error(LABEL, "Get minute function failed");
        return -1;
    }
    int64_t minute;
    status = napi_get_value_int64(env, ret_value, &minute);
    if (status != napi_ok) {
        HiLog::Error(LABEL, "Get minute failed");
        return -1;
    }
    return minute;
}

int64_t IntlAddon::GetSecond(napi_env env, napi_value *argv, int index)
{
    napi_value funcGetDateInfo;
    napi_status status = napi_get_named_property(env, argv[index], "getSeconds", &funcGetDateInfo);
    if (status != napi_ok) {
        HiLog::Error(LABEL, "Get second property failed");
        return -1;
    }
    napi_value ret_value;
    status = napi_call_function(env, argv[index], funcGetDateInfo, 0, nullptr, &ret_value);
    if (status != napi_ok) {
        HiLog::Error(LABEL, "Get second function failed");
        return -1;
    }
    int64_t second;
    status = napi_get_value_int64(env, ret_value, &second);
    if (status != napi_ok) {
        HiLog::Error(LABEL, "Get second failed");
        return -1;
    }
    return second;
}

napi_value IntlAddon::GetLanguage(napi_env env, napi_callback_info info)
{
    // No parameters are needed to get the language.
    GET_PARAMS(env, info, 0);

    IntlAddon *obj = nullptr;
    napi_status status = napi_unwrap(env, thisVar, reinterpret_cast<void **>(&obj));
    if (status != napi_ok || obj == nullptr || obj->locale_ == nullptr) {
        HiLog::Error(LABEL, "Get Locale object failed");
        return nullptr;
    }
    std::string value = obj->locale_->GetLanguage();

    napi_value result;
    status = napi_create_string_utf8(env, value.c_str(), NAPI_AUTO_LENGTH, &result);
    if (status != napi_ok) {
        HiLog::Error(LABEL, "Create language string failed");
        return nullptr;
    }
    return result;
}

napi_value IntlAddon::GetScript(napi_env env, napi_callback_info info)
{
    // No parameters are needed to get the script.
    GET_PARAMS(env, info, 0);

    IntlAddon *obj = nullptr;
    napi_status status = napi_unwrap(env, thisVar, reinterpret_cast<void **>(&obj));
    if (status != napi_ok || obj == nullptr || obj->locale_ == nullptr) {
        HiLog::Error(LABEL, "Get Locale object failed");
        return nullptr;
    }
    std::string value = obj->locale_->GetScript();

    napi_value result;
    status = napi_create_string_utf8(env, value.c_str(), NAPI_AUTO_LENGTH, &result);
    if (status != napi_ok) {
        HiLog::Error(LABEL, "Create script string failed");
        return nullptr;
    }
    return result;
}

napi_value IntlAddon::GetRegion(napi_env env, napi_callback_info info)
{
    // No parameters are needed to get the region.
    GET_PARAMS(env, info, 0);

    IntlAddon *obj = nullptr;
    napi_status status = napi_unwrap(env, thisVar, reinterpret_cast<void **>(&obj));
    if (status != napi_ok || obj == nullptr || obj->locale_ == nullptr) {
        HiLog::Error(LABEL, "Get Locale object failed");
        return nullptr;
    }
    std::string value = obj->locale_->GetRegion();

    napi_value result;
    status = napi_create_string_utf8(env, value.c_str(), NAPI_AUTO_LENGTH, &result);
    if (status != napi_ok) {
        HiLog::Error(LABEL, "Create region string failed");
        return nullptr;
    }
    return result;
}

napi_value IntlAddon::GetBaseName(napi_env env, napi_callback_info info)
{
    // No parameters are needed to get the baseName.
    GET_PARAMS(env, info, 0);

    IntlAddon *obj = nullptr;
    napi_status status = napi_unwrap(env, thisVar, reinterpret_cast<void **>(&obj));
    if (status != napi_ok || obj == nullptr || obj->locale_ == nullptr) {
        HiLog::Error(LABEL, "Get Locale object failed");
        return nullptr;
    }
    std::string value = obj->locale_->GetBaseName();

    napi_value result;
    status = napi_create_string_utf8(env, value.c_str(), NAPI_AUTO_LENGTH, &result);
    if (status != napi_ok) {
        HiLog::Error(LABEL, "Create base name string failed");
        return nullptr;
    }
    return result;
}

napi_value IntlAddon::GetCalendar(napi_env env, napi_callback_info info)
{
    // No parameters are needed to get the baseName.
    GET_PARAMS(env, info, 0);

    IntlAddon *obj = nullptr;
    napi_status status = napi_unwrap(env, thisVar, reinterpret_cast<void **>(&obj));
    if (status != napi_ok || obj == nullptr || obj->locale_ == nullptr) {
        HiLog::Error(LABEL, "Get Locale object failed");
        return nullptr;
    }
    std::string value = obj->locale_->GetCalendar();

    napi_value result;
    status = napi_create_string_utf8(env, value.c_str(), NAPI_AUTO_LENGTH, &result);
    if (status != napi_ok) {
        HiLog::Error(LABEL, "Create base name string failed");
        return nullptr;
    }
    return result;
}

napi_value IntlAddon::GetCollation(napi_env env, napi_callback_info info)
{
    // No parameters are needed to get the baseName.
    GET_PARAMS(env, info, 0);

    IntlAddon *obj = nullptr;
    napi_status status = napi_unwrap(env, thisVar, reinterpret_cast<void **>(&obj));
    if (status != napi_ok || obj == nullptr || obj->locale_ == nullptr) {
        HiLog::Error(LABEL, "Get Locale object failed");
        return nullptr;
    }
    std::string value = obj->locale_->GetCollation();

    napi_value result;
    status = napi_create_string_utf8(env, value.c_str(), NAPI_AUTO_LENGTH, &result);
    if (status != napi_ok) {
        HiLog::Error(LABEL, "Create base name string failed");
        return nullptr;
    }
    return result;
}

napi_value IntlAddon::GetHourCycle(napi_env env, napi_callback_info info)
{
    // No parameters are needed to get the baseName.
    GET_PARAMS(env, info, 0);

    IntlAddon *obj = nullptr;
    napi_status status = napi_unwrap(env, thisVar, reinterpret_cast<void **>(&obj));
    if (status != napi_ok || obj == nullptr || obj->locale_ == nullptr) {
        HiLog::Error(LABEL, "Get Locale object failed");
        return nullptr;
    }
    std::string value = obj->locale_->GetHourCycle();

    napi_value result;
    status = napi_create_string_utf8(env, value.c_str(), NAPI_AUTO_LENGTH, &result);
    if (status != napi_ok) {
        HiLog::Error(LABEL, "Create base name string failed");
        return nullptr;
    }
    return result;
}

napi_value IntlAddon::GetNumberingSystem(napi_env env, napi_callback_info info)
{
    // No parameters are needed to get the baseName.
    GET_PARAMS(env, info, 0);

    IntlAddon *obj = nullptr;
    napi_status status = napi_unwrap(env, thisVar, reinterpret_cast<void **>(&obj));
    if (status != napi_ok || obj == nullptr || obj->locale_ == nullptr) {
        HiLog::Error(LABEL, "Get Locale object failed");
        return nullptr;
    }
    std::string value = obj->locale_->GetNumberingSystem();

    napi_value result;
    status = napi_create_string_utf8(env, value.c_str(), NAPI_AUTO_LENGTH, &result);
    if (status != napi_ok) {
        HiLog::Error(LABEL, "Create base name string failed");
        return nullptr;
    }
    return result;
}

napi_value IntlAddon::GetNumeric(napi_env env, napi_callback_info info)
{
    // No parameters are needed to get the baseName.
    GET_PARAMS(env, info, 0);

    IntlAddon *obj = nullptr;
    napi_status status = napi_unwrap(env, thisVar, reinterpret_cast<void **>(&obj));
    if (status != napi_ok || obj == nullptr || obj->locale_ == nullptr) {
        HiLog::Error(LABEL, "Get Locale object failed");
        return nullptr;
    }
    std::string value = obj->locale_->GetNumeric();
    bool optionBoolValue = (value == "true");
    napi_value result;
    status = napi_get_boolean(env, optionBoolValue, &result);
    if (status != napi_ok) {
        HiLog::Error(LABEL, "Create numeric boolean value failed");
        return nullptr;
    }
    return result;
}

napi_value IntlAddon::GetCaseFirst(napi_env env, napi_callback_info info)
{
    // No parameters are needed to get the baseName.
    GET_PARAMS(env, info, 0);

    IntlAddon *obj = nullptr;
    napi_status status = napi_unwrap(env, thisVar, reinterpret_cast<void **>(&obj));
    if (status != napi_ok || obj == nullptr || obj->locale_ == nullptr) {
        HiLog::Error(LABEL, "Get Locale object failed");
        return nullptr;
    }
    std::string value = obj->locale_->GetCaseFirst();
    napi_value result;
    status = napi_create_string_utf8(env, value.c_str(), NAPI_AUTO_LENGTH, &result);
    if (status != napi_ok) {
        HiLog::Error(LABEL, "Create caseFirst string failed");
        return nullptr;
    }
    return result;
}

napi_value IntlAddon::ToString(napi_env env, napi_callback_info info)
{
    // No parameters are needed to get the language.
    GET_PARAMS(env, info, 0);

    IntlAddon *obj = nullptr;
    napi_status status = napi_unwrap(env, thisVar, reinterpret_cast<void **>(&obj));
    if (status != napi_ok || obj == nullptr || obj->locale_ == nullptr) {
        HiLog::Error(LABEL, "Get Locale object failed");
        return nullptr;
    }
    std::string value = obj->locale_->ToString();

    napi_value result;
    status = napi_create_string_utf8(env, value.c_str(), NAPI_AUTO_LENGTH, &result);
    if (status != napi_ok) {
        HiLog::Error(LABEL, "Create language string failed");
        return nullptr;
    }
    return result;
}

napi_value IntlAddon::Maximize(napi_env env, napi_callback_info info)
{
    // No parameters are needed to get the language.
    GET_PARAMS(env, info, 0);

    IntlAddon *obj = nullptr;
    napi_status status = napi_unwrap(env, thisVar, reinterpret_cast<void **>(&obj));
    if (status != napi_ok || obj == nullptr || obj->locale_ == nullptr) {
        HiLog::Error(LABEL, "Get Locale object failed");
        return nullptr;
    }
    std::string localeTag = obj->locale_->Maximize();

    napi_value constructor;
    status = napi_get_reference_value(env, *g_constructor, &constructor);
    if (status != napi_ok) {
        HiLog::Error(LABEL, "Get locale constructor reference failed");
        return nullptr;
    }
    napi_value result = nullptr;
    napi_value arg = nullptr;
    status = napi_create_string_utf8(env, localeTag.c_str(), NAPI_AUTO_LENGTH, &arg);
    if (status != napi_ok) {
        HiLog::Error(LABEL, "Create localeTag string failed");
        return nullptr;
    }
    status = napi_new_instance(env, constructor, 1, &arg, &result);
    if (status != napi_ok) {
        HiLog::Error(LABEL, "Create new locale instance failed");
        return nullptr;
    }
    return result;
}

napi_value IntlAddon::Minimize(napi_env env, napi_callback_info info)
{
    // No parameters are needed to get the language.
    GET_PARAMS(env, info, 0);

    IntlAddon *obj = nullptr;
    napi_status status = napi_unwrap(env, thisVar, reinterpret_cast<void **>(&obj));
    if (status != napi_ok || obj == nullptr || obj->locale_ == nullptr) {
        HiLog::Error(LABEL, "Get Locale object failed");
        return nullptr;
    }
    std::string localeTag = obj->locale_->Minimize();

    napi_value constructor;
    status = napi_get_reference_value(env, *g_constructor, &constructor);
    if (status != napi_ok) {
        HiLog::Error(LABEL, "Get locale constructor reference failed");
        return nullptr;
    }
    napi_value result = nullptr;
    napi_value arg = nullptr;
    status = napi_create_string_utf8(env, localeTag.c_str(), NAPI_AUTO_LENGTH, &arg);
    if (status != napi_ok) {
        HiLog::Error(LABEL, "Create localeTag string failed");
        return nullptr;
    }
    status = napi_new_instance(env, constructor, 1, &arg, &result);
    if (status != napi_ok) {
        HiLog::Error(LABEL, "Create new locale instance failed");
        return nullptr;
    }
    return result;
}

void SetOptionProperties(napi_env env, napi_value &result, std::map<std::string, std::string> &options,
    const std::string &option)
{
    if (options.count(option) > 0) {
        std::string optionValue = options[option];
        napi_value optionJsValue;
        napi_create_string_utf8(env, optionValue.c_str(), NAPI_AUTO_LENGTH, &optionJsValue);
        napi_set_named_property(env, result, option.c_str(), optionJsValue);
    } else {
        napi_value undefined;
        napi_get_undefined(env, &undefined);
        napi_set_named_property(env, result, option.c_str(), undefined);
    }
}

void SetIntegerOptionProperties(napi_env env, napi_value &result, std::map<std::string, std::string> &options,
    const std::string &option)
{
    if (options.count(option) > 0) {
        std::string optionValue = options[option];
        napi_value optionJsValue;
        int64_t integerValue = std::stoi(optionValue);
        napi_create_int64(env, integerValue, &optionJsValue);
        napi_set_named_property(env, result, option.c_str(), optionJsValue);
    } else {
        napi_value undefined;
        napi_get_undefined(env, &undefined);
        napi_set_named_property(env, result, option.c_str(), undefined);
    }
}

void SetBooleanOptionProperties(napi_env env, napi_value &result, std::map<std::string, std::string> &options,
    const std::string &option)
{
    if (options.count(option) > 0) {
        std::string optionValue = options[option];
        bool optionBoolValue = (optionValue == "true");
        napi_value optionJsValue;
        napi_get_boolean(env, optionBoolValue, &optionJsValue);
        napi_set_named_property(env, result, option.c_str(), optionJsValue);
    } else {
        napi_value undefined;
        napi_get_undefined(env, &undefined);
        napi_set_named_property(env, result, option.c_str(), undefined);
    }
}

napi_value IntlAddon::GetDateTimeResolvedOptions(napi_env env, napi_callback_info info)
{
    // No parameters are needed to get the baseName.
    GET_PARAMS(env, info, 0);

    IntlAddon *obj = nullptr;
    napi_status status = napi_unwrap(env, thisVar, reinterpret_cast<void **>(&obj));
    if (status != napi_ok || obj == nullptr || obj->datefmt_ == nullptr) {
        HiLog::Error(LABEL, "Get DateTimeFormat object failed");
        return nullptr;
    }
    napi_value result;
    napi_create_object(env, &result);
    std::map<std::string, std::string> options = {};
    obj->datefmt_->GetResolvedOptions(options);
    SetOptionProperties(env, result, options, "locale");
    SetOptionProperties(env, result, options, "calendar");
    SetOptionProperties(env, result, options, "dateStyle");
    SetOptionProperties(env, result, options, "timeStyle");
    SetOptionProperties(env, result, options, "hourCycle");
    SetOptionProperties(env, result, options, "timeZone");
    SetOptionProperties(env, result, options, "timeZoneName");
    SetOptionProperties(env, result, options, "numberingSystem");
    SetBooleanOptionProperties(env, result, options, "hour12");
    SetOptionProperties(env, result, options, "weekday");
    SetOptionProperties(env, result, options, "era");
    SetOptionProperties(env, result, options, "year");
    SetOptionProperties(env, result, options, "month");
    SetOptionProperties(env, result, options, "day");
    SetOptionProperties(env, result, options, "hour");
    SetOptionProperties(env, result, options, "minute");
    SetOptionProperties(env, result, options, "second");
    SetOptionProperties(env, result, options, "dayPeriod");
    SetOptionProperties(env, result, options, "localeMatcher");
    SetOptionProperties(env, result, options, "formatMatcher");
    return result;
}

napi_value IntlAddon::GetNumberResolvedOptions(napi_env env, napi_callback_info info)
{
    // No parameters are needed to get the baseName.
    GET_PARAMS(env, info, 0);

    IntlAddon *obj = nullptr;
    napi_status status = napi_unwrap(env, thisVar, reinterpret_cast<void **>(&obj));
    if (status != napi_ok || obj == nullptr || obj->numberfmt_ == nullptr) {
        HiLog::Error(LABEL, "Get NumberFormat object failed");
        return nullptr;
    }
    napi_value result;
    napi_create_object(env, &result);
    std::map<std::string, std::string> options = {};
    obj->numberfmt_->GetResolvedOptions(options);
    SetOptionProperties(env, result, options, "locale");
    SetOptionProperties(env, result, options, "currency");
    SetOptionProperties(env, result, options, "currencySign");
    SetOptionProperties(env, result, options, "currencyDisplay");
    SetOptionProperties(env, result, options, "unit");
    SetOptionProperties(env, result, options, "unitDisplay");
    SetOptionProperties(env, result, options, "signDisplay");
    SetOptionProperties(env, result, options, "compactDisplay");
    SetOptionProperties(env, result, options, "notation");
    SetOptionProperties(env, result, options, "style");
    SetOptionProperties(env, result, options, "numberingSystem");
    SetBooleanOptionProperties(env, result, options, "useGrouping");
    SetIntegerOptionProperties(env, result, options, "minimumIntegerDigits");
    SetIntegerOptionProperties(env, result, options, "minimumFractionDigits");
    SetIntegerOptionProperties(env, result, options, "maximumFractionDigits");
    SetIntegerOptionProperties(env, result, options, "minimumSignificantDigits");
    SetIntegerOptionProperties(env, result, options, "maximumSignificantDigits");
    SetOptionProperties(env, result, options, "localeMatcher");
    return result;
}

napi_value IntlAddon::FormatNumber(napi_env env, napi_callback_info info)
{
    GET_PARAMS(env, info, 1); // Need to get one parameter of a date object to format.
    double number;
    napi_get_value_double(env, argv[0], &number);
    IntlAddon *obj = nullptr;
    napi_status status = napi_unwrap(env, thisVar, reinterpret_cast<void **>(&obj));
    if (status != napi_ok || obj == nullptr || obj->numberfmt_ == nullptr) {
        HiLog::Error(LABEL, "Get NumberFormat object failed");
        return nullptr;
    }
    std::string value = obj->numberfmt_->Format(number);
    napi_value result;
    status = napi_create_string_utf8(env, value.c_str(), NAPI_AUTO_LENGTH, &result);
    if (status != napi_ok) {
        HiLog::Error(LABEL, "Create format string failed");
        return nullptr;
    }
    return result;
}

napi_value Init(napi_env env, napi_value exports)
{
    napi_value val = IntlAddon::InitLocale(env, exports);
    val = IntlAddon::InitDateTimeFormat(env, val);
    return IntlAddon::InitNumberFormat(env, val);
}

static napi_module g_intlModule = {
    .nm_version = 1,
    .nm_flags = 0,
    .nm_filename = nullptr,
    .nm_register_func = Init,
    .nm_modname = "intl",
    .nm_priv = ((void *)0),
    .reserved = { 0 }
};

extern "C" __attribute__((constructor)) void AbilityRegister()
{
    napi_module_register(&g_intlModule);
}
} // namespace I18n
} // namespace Global
} // namespace OHOS