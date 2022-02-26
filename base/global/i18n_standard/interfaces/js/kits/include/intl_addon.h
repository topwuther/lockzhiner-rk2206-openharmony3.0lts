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

#ifndef INTL_ADDON_H
#define INTL_ADDON_H

#include <string>

#include "napi/native_api.h"
#include "napi/native_node_api.h"
#include "locale_info.h"
#include "date_time_format.h"
#include "number_format.h"

namespace OHOS {
namespace Global {
namespace I18n {
static void GetLocaleTags(napi_env env, napi_value rawLocaleTag, std::vector<std::string> &localeTags);
static void GetOptionValue(napi_env env, napi_value options, const std::string &optionName,
    std::map<std::string, std::string> &map);
static void GetBoolOptionValue(napi_env env, napi_value options, const std::string &optionName,
    std::map<std::string, std::string> &map);
static void GetIntegerOptionValue(napi_env env, napi_value options, const std::string &optionName,
    std::map<std::string, std::string> &map);
static void GetDateOptionValues(napi_env env, napi_value options, std::map<std::string, std::string> &map);
static void GetNumberOptionValues(napi_env env, napi_value options, std::map<std::string, std::string> &map);
static void SetOptionProperties(napi_env env, napi_value &result, std::map<std::string, std::string> &options,
    const std::string &option);
static void SetIntegerOptionProperties(napi_env env, napi_value &result,
    std::map<std::string, std::string> &options, const std::string &option);
static void SetBooleanOptionProperties(napi_env env, napi_value &result,
    std::map<std::string, std::string> &options, const std::string &option);

class IntlAddon {
public:
    static napi_value InitLocale(napi_env env, napi_value exports);
    static napi_value InitDateTimeFormat(napi_env env, napi_value exports);
    static napi_value InitNumberFormat(napi_env env, napi_value exports);
    static void Destructor(napi_env env, void *nativeObject, void *finalize_hint);

    IntlAddon();
    virtual ~IntlAddon();

private:
    static napi_value DateTimeFormatConstructor(napi_env env, napi_callback_info info);
    static napi_value NumberFormatConstructor(napi_env env, napi_callback_info info);
    static napi_value LocaleConstructor(napi_env env, napi_callback_info info);
    static napi_value GetLanguage(napi_env env, napi_callback_info info);
    static napi_value GetScript(napi_env env, napi_callback_info info);
    static napi_value GetRegion(napi_env env, napi_callback_info info);
    static napi_value GetBaseName(napi_env env, napi_callback_info info);
    static napi_value GetCalendar(napi_env env, napi_callback_info info);
    static napi_value GetCollation(napi_env env, napi_callback_info info);
    static napi_value GetHourCycle(napi_env env, napi_callback_info info);
    static napi_value GetNumberingSystem(napi_env env, napi_callback_info info);
    static napi_value GetNumeric(napi_env env, napi_callback_info info);
    static napi_value GetCaseFirst(napi_env env, napi_callback_info info);
    static napi_value ToString(napi_env env, napi_callback_info info);
    static napi_value Maximize(napi_env env, napi_callback_info info);
    static napi_value Minimize(napi_env env, napi_callback_info info);

    static napi_value FormatDateTime(napi_env env, napi_callback_info info);
    static napi_value FormatDateTimeRange(napi_env env, napi_callback_info info);
    static napi_value GetDateTimeResolvedOptions(napi_env env, napi_callback_info info);

    static napi_value GetNumberResolvedOptions(napi_env env, napi_callback_info info);
    static napi_value FormatNumber(napi_env env, napi_callback_info info);

    static int64_t GetYear(napi_env env, napi_value *argv, int index);
    static int64_t GetMonth(napi_env env, napi_value *argv, int index);
    static int64_t GetDay(napi_env env, napi_value *argv, int index);
    static int64_t GetHour(napi_env env, napi_value *argv, int index);
    static int64_t GetMinute(napi_env env, napi_value *argv, int index);
    static int64_t GetSecond(napi_env env, napi_value *argv, int index);
    bool InitLocaleContext(napi_env env, napi_callback_info info, const std::string localeTag,
        std::map<std::string, std::string> &map);
    bool InitDateTimeFormatContext(napi_env env, napi_callback_info info, std::vector<std::string> localeTags,
        std::map<std::string, std::string> &map);
    bool InitNumberFormatContext(napi_env env, napi_callback_info info, std::vector<std::string> localeTags,
        std::map<std::string, std::string> &map);

    napi_env env_;
    napi_ref wrapper_;
    std::unique_ptr<LocaleInfo> locale_;
    std::unique_ptr<DateTimeFormat> datefmt_;
    std::unique_ptr<NumberFormat> numberfmt_;
};
} // namespace I18n
} // namespace Global
} // namespace OHOS
#endif