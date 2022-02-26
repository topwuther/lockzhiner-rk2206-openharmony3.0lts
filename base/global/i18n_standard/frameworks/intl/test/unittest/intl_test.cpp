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

#include "intl_test.h"
#include <gtest/gtest.h>
#include <map>
#include <vector>
#include "date_time_format.h"
#include "locale_info.h"
#include "number_format.h"

using namespace OHOS::Global::I18n;
using testing::ext::TestSize;
using namespace std;

namespace {
class IntlTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
};

void IntlTest::SetUpTestCase(void)
{}

void IntlTest::TearDownTestCase(void)
{}

void IntlTest::SetUp(void)
{}

void IntlTest::TearDown(void)
{}

/**
 * @tc.name: IntlFuncTest001
 * @tc.desc: Test Intl DateTimeFormat.format
 * @tc.type: FUNC
 */
HWTEST_F(IntlTest, IntlFuncTest001, TestSize.Level1)
{
    string locale = "zh-CN-u-hc-h12";
    string expects = "公元2021年4月14日星期三 下午3:05:03";
    vector<string> locales;
    locales.push_back("jessie");
    locales.push_back(locale);
    map<string, string> options = { { "year", "numeric" },
                                    { "month", "long" },
                                    { "day", "numeric" },
                                    { "hour", "numeric" },
                                    { "minute", "2-digit" },
                                    { "second", "numeric" },
                                    { "weekday", "long" },
                                    { "era", "short"} };
    DateTimeFormat *dateFormat = new (std::nothrow) DateTimeFormat(locales, options);
    if (dateFormat == nullptr) {
        EXPECT_TRUE(false);
        return;
    }
    int64_t date[] = { 2021, 3, 14, 15, 5, 3 };
    string out = dateFormat->Format(date, 6);
    EXPECT_EQ(out, expects);
    EXPECT_EQ(dateFormat->GetYear(), "numeric");
    EXPECT_EQ(dateFormat->GetMonth(), "long");
    EXPECT_EQ(dateFormat->GetDay(), "numeric");
    EXPECT_EQ(dateFormat->GetHour(), "numeric");
    EXPECT_EQ(dateFormat->GetMinute(), "2-digit");
    EXPECT_EQ(dateFormat->GetSecond(), "numeric");
    EXPECT_EQ(dateFormat->GetWeekday(), "long");
    EXPECT_EQ(dateFormat->GetEra(), "short");
    EXPECT_EQ(dateFormat->GetHourCycle(), "h12");
    delete dateFormat;
}

/**
 * @tc.name: IntlFuncTest002
 * @tc.desc: Test Intl LocaleInfo
 * @tc.type: FUNC
 */
HWTEST_F(IntlTest, IntlFuncTest002, TestSize.Level0)
{
    string locale = "ja-Jpan-JP-u-ca-japanese-hc-h12-co-emoji";
    map<string, string> options = { { "hourCycle", "h11" },
                                    { "numeric", "true" },
                                    { "numberingSystem", "jpan" } };
    LocaleInfo *loc = new (std::nothrow) LocaleInfo(locale, options);
    if (loc == nullptr) {
        EXPECT_TRUE(false);
        return;
    }
    EXPECT_EQ(loc->GetLanguage(), "ja");
    EXPECT_EQ(loc->GetScript(), "Jpan");
    EXPECT_EQ(loc->GetRegion(), "JP");
    EXPECT_EQ(loc->GetBaseName(), "ja-Jpan-JP");
    EXPECT_EQ(loc->GetCalendar(), "japanese");
    EXPECT_EQ(loc->GetHourCycle(), "h11");
    EXPECT_EQ(loc->GetNumberingSystem(), "jpan");
    EXPECT_EQ(loc->Minimize(), "ja-u-hc-h11-nu-jpan-ca-japanese-co-emoji-kn-true");
    EXPECT_EQ(loc->Maximize(), "ja-Jpan-JP-u-hc-h11-nu-jpan-ca-japanese-co-emoji-kn-true");
    EXPECT_EQ(loc->GetNumeric(), "true");
    EXPECT_EQ(loc->GetCaseFirst(), "");
    delete loc;
}

/**
 * @tc.name: IntlFuncTest003
 * @tc.desc: Test Intl LocaleInfo
 * @tc.type: FUNC
 */
HWTEST_F(IntlTest, IntlFuncTest003, TestSize.Level1)
{
    string locale = "en-GB";
    LocaleInfo *loc = new (std::nothrow) LocaleInfo(locale);
    if (loc == nullptr) {
        EXPECT_TRUE(false);
        return;
    }
    string language = "en";
    string script = "";
    string region = "GB";
    string baseName = "en-GB";
    EXPECT_EQ(loc->GetLanguage(), language);
    EXPECT_EQ(loc->GetScript(), script);
    EXPECT_EQ(loc->GetRegion(), region);
    EXPECT_EQ(loc->GetBaseName(), baseName);
    delete loc;
}

/**
 * @tc.name: IntlFuncTest001
 * @tc.desc: Test Intl DateTimeFormat.format
 * @tc.type: FUNC
 */
HWTEST_F(IntlTest, IntlFuncTest004, TestSize.Level1)
{
    string locale = "en-GB";
    string expects = "14 April 2021, 15:05 – 5 May 2021, 10:05";
    vector<string> locales;
    locales.push_back(locale);
    string dateStyle = "long";
    string timeStyle = "short";
    map<string, string> options = { { "dateStyle", dateStyle },
                                    { "timeStyle", timeStyle } };
    DateTimeFormat *dateFormat = new (std::nothrow) DateTimeFormat(locales, options);
    if (dateFormat == nullptr) {
        EXPECT_TRUE(false);
        return;
    }
    int64_t date1[] = {2021, 3, 14, 15, 5, 3};
    int64_t date2[] = {2021, 4, 5, 10, 5, 3};
    string out = dateFormat->FormatRange(date1, 6, date2, 6);
    EXPECT_EQ(out, expects);
    EXPECT_EQ(dateFormat->GetDateStyle(), dateStyle);
    EXPECT_EQ(dateFormat->GetTimeStyle(), timeStyle);
    delete dateFormat;
}

/**
 * @tc.name: IntlFuncTest001
 * @tc.desc: Test Intl DateTimeFormat.format
 * @tc.type: FUNC
 */
HWTEST_F(IntlTest, IntlFuncTest006, TestSize.Level1)
{
    string locale = "ja";
    string expects = "2021年4月14日水曜日";
    vector<string> locales;
    locales.push_back(locale);
    map<string, string> options = { { "year", "numeric" },
                                    { "month", "long" },
                                    { "day", "numeric" },
                                    { "weekday", "long"} };
    DateTimeFormat *dateFormat = new (std::nothrow) DateTimeFormat(locales, options);
    if (dateFormat == nullptr) {
        EXPECT_TRUE(false);
        return;
    }
    int64_t date[] = { 2021, 3, 14, 15, 5, 3 };
    string out = dateFormat->Format(date, 6);
    EXPECT_EQ(out, expects);
    int64_t date1[] = {2021, 3, 14, 15, 5, 3};
    int64_t date2[] = {2021, 4, 5, 10, 5, 3};
    expects = "2021/04/14水曜日～2021/05/05水曜日";
    out = dateFormat->FormatRange(date1, 6, date2, 6);
    EXPECT_EQ(out, expects);
    delete dateFormat;
}

/**
 * @tc.name: IntlFuncTest005
 * @tc.desc: Test Intl DateTimeFormat.format
 * @tc.type: FUNC
 */
HWTEST_F(IntlTest, IntlFuncTest005, TestSize.Level1)
{
    string locale = "en-IN";
    string expects = "+1,23,456.79 euros";
    vector<string> locales;
    locales.push_back(locale);
    string useGrouping = "true";
    string minimumIntegerDigits = "7";
    string maximumFractionDigits = "2";
    string style = "currency";
    string currency = "EUR";
    map<string, string> options = { { "useGrouping", useGrouping },
                                    { "style", style },
                                    { "currency", currency },
                                    { "currencyDisplay", "name" },
                                    { "currencySign", "accounting" },
                                    { "signDisplay", "always" } };
    NumberFormat *numFmt = new (std::nothrow) NumberFormat(locales, options);
    if (numFmt == nullptr) {
        EXPECT_TRUE(false);
        return;
    }
    string out = numFmt->Format(123456.789);
    EXPECT_EQ(out, expects);
    EXPECT_EQ(numFmt->GetUseGrouping(), useGrouping);
    EXPECT_EQ(numFmt->GetStyle(), style);
    EXPECT_EQ(numFmt->GetCurrency(), currency);
    delete numFmt;
}

/**
 * @tc.name: IntlFuncTest005
 * @tc.desc: Test Intl DateTimeFormat.format
 * @tc.type: FUNC
 */
HWTEST_F(IntlTest, IntlFuncTest007, TestSize.Level1)
{
    string locale = "zh-CN";
    string expects = "0,123,456.79米";
    vector<string> locales;
    locales.push_back("ban");
    locales.push_back(locale);
    string minimumIntegerDigits = "7";
    string maximumFractionDigits = "2";
    string style = "unit";
    map<string, string> options = { { "style", style },
                                    { "minimumIntegerDigits", minimumIntegerDigits },
                                    { "maximumFractionDigits", maximumFractionDigits },
                                    { "unit", "meter" },
                                    { "unitDisplay", "long"} };
    NumberFormat *numFmt = new (std::nothrow) NumberFormat(locales, options);
    if (numFmt == nullptr) {
        EXPECT_TRUE(false);
        return;
    }
    string out = numFmt->Format(123456.789);
    EXPECT_EQ(out, expects);
    EXPECT_EQ(numFmt->GetStyle(), style);
    delete numFmt;
}

/**
 * @tc.name: IntlFuncTest005
 * @tc.desc: Test Intl DateTimeFormat.format
 * @tc.type: FUNC
 */
HWTEST_F(IntlTest, IntlFuncTest008, TestSize.Level1)
{
    string locale = "en-CN";
    string expects = "0,123,456.79%";
    vector<string> locales;
    locales.push_back(locale);
    string minimumIntegerDigits = "7";
    string maximumFractionDigits = "2";
    string style = "percent";
    map<string, string> options = { { "style", style },
                                    { "minimumIntegerDigits", minimumIntegerDigits },
                                    { "maximumFractionDigits", maximumFractionDigits } };
    NumberFormat *numFmt = new (std::nothrow) NumberFormat(locales, options);
    if (numFmt == nullptr) {
        EXPECT_TRUE(false);
        return;
    }
    string out = numFmt->Format(123456.789);
    EXPECT_EQ(out, expects);
    EXPECT_EQ(numFmt->GetStyle(), style);
    delete numFmt;
}

/**
 * @tc.name: IntlFuncTest005
 * @tc.desc: Test Intl DateTimeFormat.format
 * @tc.type: FUNC
 */
HWTEST_F(IntlTest, IntlFuncTest009, TestSize.Level1)
{
    string locale = "en-CN";
    string expects = "0,123,456.79";
    vector<string> locales;
    locales.push_back(locale);
    string minimumIntegerDigits = "7";
    string maximumFractionDigits = "2";
    string style = "decimal";
    map<string, string> options = { { "style", style },
                                    { "minimumIntegerDigits", minimumIntegerDigits },
                                    { "maximumFractionDigits", maximumFractionDigits } };
    NumberFormat *numFmt = new (std::nothrow) NumberFormat(locales, options);
    if (numFmt == nullptr) {
        EXPECT_TRUE(false);
        return;
    }
    string out = numFmt->Format(123456.789);
    EXPECT_EQ(out, expects);
    EXPECT_EQ(numFmt->GetStyle(), style);
    delete numFmt;
}

/**
 * @tc.name: IntlFuncTest005
 * @tc.desc: Test Intl DateTimeFormat.format
 * @tc.type: FUNC
 */
HWTEST_F(IntlTest, IntlFuncTest010, TestSize.Level1)
{
    string locale = "en-CN";
    string expects = "1.234568E5";
    vector<string> locales;
    locales.push_back(locale);
    string style = "decimal";
    map<string, string> options = { { "style", style },
                                    { "notation", "scientific" } };
    NumberFormat *numFmt = new (std::nothrow) NumberFormat(locales, options);
    if (numFmt == nullptr) {
        EXPECT_TRUE(false);
        return;
    }
    string out = numFmt->Format(123456.789);
    EXPECT_EQ(out, expects);
    EXPECT_EQ(numFmt->GetStyle(), style);
    delete numFmt;
}

/**
 * @tc.name: IntlFuncTest005
 * @tc.desc: Test Intl DateTimeFormat.format
 * @tc.type: FUNC
 */
HWTEST_F(IntlTest, IntlFuncTest011, TestSize.Level1)
{
    string locale = "en-CN";
    string expects = "123 thousand";
    vector<string> locales;
    locales.push_back(locale);
    string style = "decimal";
    map<string, string> options = { { "style", style },
                                    { "notation", "compact" },
                                    { "compactDisplay", "long" } };
    NumberFormat *numFmt = new (std::nothrow) NumberFormat(locales, options);
    if (numFmt == nullptr) {
        EXPECT_TRUE(false);
        return;
    }
    string out = numFmt->Format(123456.789);
    EXPECT_EQ(out, expects);
    EXPECT_EQ(numFmt->GetStyle(), style);
    delete numFmt;
}

/**
 * @tc.name: IntlFuncTest002
 * @tc.desc: Test Intl LocaleInfo
 * @tc.type: FUNC
 */
HWTEST_F(IntlTest, IntlFuncTest0012, TestSize.Level1)
{
    string locale = "en";
    map<string, string> options = {};
    vector<string> locales;
    locales.push_back(locale);
    std::string expects = "4/14/21";
    DateTimeFormat *dateFormat = new (std::nothrow) DateTimeFormat(locales, options);
    if (dateFormat == nullptr) {
        EXPECT_TRUE(false);
        return;
    }
    int64_t date[] = {2021, 3, 14, 15, 5, 3};
    string out = dateFormat->Format(date, 6);
    EXPECT_EQ(out, expects);
    delete dateFormat;
}

/**
 * @tc.name: IntlFuncTest002
 * @tc.desc: Test Intl LocaleInfo
 * @tc.type: FUNC
 */
HWTEST_F(IntlTest, IntlFuncTest0013, TestSize.Level1)
{
    string locale = "en-CN";
    vector<string> locales;
    locales.push_back(locale);
    map<string, string> options = {};
    std::string expects = "123,456.789";
    NumberFormat *numFmt = new (std::nothrow) NumberFormat(locales, options);
    if (numFmt == nullptr) {
        EXPECT_TRUE(false);
        return;
    }
    string out = numFmt->Format(123456.789);
    EXPECT_EQ(out, expects);
    delete numFmt;
}

/**
 * @tc.name: IntlFuncTest002
 * @tc.desc: Test Intl LocaleInfo
 * @tc.type: FUNC
 */
HWTEST_F(IntlTest, IntlFuncTest0014, TestSize.Level1)
{
    string locale = "zh-CN-u-hc-h12";
    string expects = "北美太平洋夏令时间";
    vector<string> locales;
    locales.push_back("jessie");
    locales.push_back(locale);
    map<string, string> options = { { "timeZone", "America/Los_Angeles"  }, { "timeZoneName", "long" } };
    DateTimeFormat *dateFormat = new (std::nothrow) DateTimeFormat(locales, options);
    if (dateFormat == nullptr) {
        EXPECT_TRUE(false);
        return;
    }
    int64_t date[] = { 2021, 3, 14, 15, 5, 3 };
    string out = dateFormat->Format(date, 6);
    EXPECT_TRUE(out.find(expects) != out.npos);
    delete dateFormat;
}

/**
 * @tc.name: IntlFuncTest002
 * @tc.desc: Test Intl LocaleInfo
 * @tc.type: FUNC
 */
HWTEST_F(IntlTest, IntlFuncTest0015, TestSize.Level1)
{
    string locale = "zh-CN-u-hc-h12";
    vector<string> locales;
    locales.push_back("jessie");
    locales.push_back(locale);
    map<string, string> options = { { "timeZone", "America/Los_Angeles"  }, { "timeZoneName", "long" } };
    DateTimeFormat *dateFormat = new (std::nothrow) DateTimeFormat(locales, options);
    if (dateFormat == nullptr) {
        EXPECT_TRUE(false);
        return;
    }
    EXPECT_EQ(dateFormat->GetTimeZone(), "America/Los_Angeles");
    EXPECT_EQ(dateFormat->GetTimeZoneName(), "long");
    delete dateFormat;
}
}
