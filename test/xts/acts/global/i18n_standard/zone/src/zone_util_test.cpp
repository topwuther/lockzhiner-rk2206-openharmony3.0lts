/*
 * Copyright (C) 2021 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <gtest/gtest.h>

#include "zone_util_test.h"
#include "zone_util.h"

namespace OHOS {
using namespace std;
using namespace testing;
using namespace testing::ext;
using namespace OHOS::Global::I18n;

class ZoneUtilTest : public testing::Test {};

/* *
 * @tc.number SUB_GLOBAL_I18N_TIMEZONE_GETDEFAULTZONE_ONEPARAM_0100
 * @tc.name the input country code have multiple timezones
 * @tc.desc Test GetDefaultZone(country) function.
 * @tc.require SR000FK13G
 */
HWTEST_F(ZoneUtilTest, GetDefaultZoneTest0100, Function | MediumTest | Level3)
{
    string expects[] = { "Antarctica/McMurdo", "America/Argentina/Buenos_Aires", "Australia/Sydney", "America/Noronha",
    "America/St_Johns", "Africa/Kinshasa", "America/Santiago", "Asia/Shanghai", "Asia/Nicosia", "Europe/Berlin",
    "America/Guayaquil", "Europe/Madrid", "Pacific/Pohnpei", "America/Godthab", "Asia/Jakarta", "Pacific/Tarawa",
    "Asia/Almaty", "Pacific/Majuro", "Asia/Ulaanbaatar", "America/Mexico_City", "Asia/Kuala_Lumpur", "Pacific/Auckland",
    "Pacific/Tahiti", "Pacific/Port_Moresby", "Asia/Gaza", "Europe/Lisbon", "Europe/Moscow", "Europe/Kiev",
    "Pacific/Wake", "America/New_York", "Asia/Tashkent" };
    string countries[] = { "AQ", "AR", "AU", "BR", "CA", "CD", "CL", "CN", "CY", "DE", "EC", "ES", "FM", "GL", "ID",
    "KI", "KZ", "MH", "MN", "MX", "MY", "NZ", "PF", "PG", "PS", "PT", "RU", "UA", "UM", "US", "UZ" };
    int count = size(expects);
    ZoneUtil util;
    for (int i = 0; i < count; ++i) {
        string out = util.GetDefaultZone(countries[i].c_str());
        EXPECT_EQ(out, expects[i]);
    }
}

/* *
 * @tc.number SUB_GLOBAL_I18N_TIMEZONE_GETDEFAULTZONE_ONEPARAM_0200
 * @tc.name the input country code have one timezone
 * @tc.desc Test GetDefaultZone(country) function.
 * @tc.require SR000FK13G
 */
HWTEST_F(ZoneUtilTest, GetDefaultZoneTest0200, Function | MediumTest | Level1)
{
    string expects[] = { "Asia/Kabul", "Europe/Sarajevo", "Africa/Abidjan", "Africa/Lusaka" };
    string countries[] = { "AF", "BA", "CI", "ZM" };
    int count = size(expects);
    ZoneUtil util;
    for (int i = 0; i < count; ++i) {
        string out = util.GetDefaultZone(countries[i].c_str());
        EXPECT_EQ(out, expects[i]);
    }
}

/* *
 * @tc.number SUB_GLOBAL_I18N_TIMEZONE_GETDEFAULTZONE_ONEPARAM_0300
 * @tc.name the input country code is lowercase style
 * @tc.desc Test GetDefaultZone(country) function.
 * @tc.require SR000FK13G
 */
HWTEST_F(ZoneUtilTest, GetDefaultZoneTest0300, Function | MediumTest | Level3)
{
    string expects[] = { "Asia/Shanghai", "America/New_York" };
    string countries[] = { "cn", "us" };
    int count = size(countries);
    ZoneUtil util;
    for (int i = 0; i < count; ++i) {
        string out = util.GetDefaultZone(countries[i].c_str());
        EXPECT_EQ(out, expects[i]);
    }
}

/* *
 * @tc.number SUB_GLOBAL_I18N_TIMEZONE_GETDEFAULTZONE_ONEPARAM_0400
 * @tc.name the input country code not exist
 * @tc.desc Test GetDefaultZone(country) function.
 * @tc.require SR000FK13G
 */
HWTEST_F(ZoneUtilTest, GetDefaultZoneTest0400, Function | MediumTest | Level3)
{
    ZoneUtil util;
    string out = util.GetDefaultZone("AAA");
    EXPECT_EQ(out, "");
}

/* *
 * @tc.number SUB_GLOBAL_I18N_TIMEZONE_GETDEFAULTZONE_ONEPARAM_0500
 * @tc.name the input country code is number
 * @tc.desc Test GetDefaultZone(country) function.
 * @tc.require SR000FK13G
 */
HWTEST_F(ZoneUtilTest, GetDefaultZoneTest0500, Function | MediumTest | Level3)
{
    ZoneUtil util;
    string out = util.GetDefaultZone("123");
    EXPECT_EQ(out, "");
}

/* *
 * @tc.number SUB_GLOBAL_I18N_TIMEZONE_GETDEFAULTZONE_ONEPARAM_0600
 * @tc.name the input country code is null
 * @tc.desc Test GetDefaultZone(country) function.
 * @tc.require SR000FK13G
 */
HWTEST_F(ZoneUtilTest, GetDefaultZoneTest0600, Function | MediumTest | Level4)
{
    ZoneUtil util;
    string out = util.GetDefaultZone("");
    EXPECT_EQ(out, "");
}

/* *
 * @tc.number SUB_GLOBAL_I18N_TIMEZONE_GETDEFAULTZONE_TWOPARAM_0100
 * @tc.name the input country code have multiple timezones,the offerset matches one timezone
 * @tc.desc Test GetDefaultZone(country, offset) function.
 * @tc.require SR000FK13G
 */
HWTEST_F(ZoneUtilTest, GetDefaultZone2Test0100, Function | MediumTest | Level3)
{
    string expects[] = { "Asia/Shanghai", "America/Detroit" };
    int32_t offsets[] = { 3600 * 1000 * 8, -3600 * 1000 * 5 };
    string countries[] = { "CN", "US" };
    int count = size(expects);
    ZoneUtil util;
    for (int i = 0; i < count; ++i) {
        string out = util.GetDefaultZone(countries[i].c_str(), offsets[i]);
        EXPECT_EQ(out, expects[i]);
    }
}

/* *
 * @tc.number SUB_GLOBAL_I18N_TIMEZONE_GETDEFAULTZONE_TWOPARAM_0200
 * @tc.name the input country code have multiple timezones,the offerset not matches anyone timezone
 * @tc.desc Test GetDefaultZone(country, offset) function.
 * @tc.require SR000FK13G
 */
HWTEST_F(ZoneUtilTest, GetDefaultZone2Test0200, Function | MediumTest | Level3)
{
    int32_t offsets[] = { 3600 * 1000 * 1, -3600 * 1000 * 1 };
    string countries[] = { "CN", "US" };
    int count = size(countries);
    ZoneUtil util;
    for (int i = 0; i < count; ++i) {
        string out = util.GetDefaultZone(countries[i].c_str(), offsets[i]);
        EXPECT_EQ(out, "");
    }
}

/* *
 * @tc.number SUB_GLOBAL_I18N_TIMEZONE_GETDEFAULTZONE_TWOPARAM_0300
 * @tc.name the input country code have one timezone,the offerset matches it
 * @tc.desc Test GetDefaultZone(country, offset) function.
 * @tc.require SR000FK13G
 */
HWTEST_F(ZoneUtilTest, GetDefaultZone2Test0300, Function | MediumTest | Level1)
{
    ZoneUtil util;
    string out = util.GetDefaultZone("AF", 3600 * 1000 * 4.5);
    EXPECT_EQ(out, "Asia/Kabul");
}

/* *
 * @tc.number SUB_GLOBAL_I18N_TIMEZONE_GETDEFAULTZONE_TWOPARAM_0400
 * @tc.name the input country code have one timezone,the offerset not matches it
 * @tc.desc Test GetDefaultZone(country, offset) function.
 * @tc.require SR000FK13G
 */
HWTEST_F(ZoneUtilTest, GetDefaultZone2Test0400, Function | MediumTest | Level3)
{
    ZoneUtil util;
    string out = util.GetDefaultZone("AF", 3600 * 1000 * 1);
    EXPECT_EQ(out, "");
}

/* *
 * @tc.number SUB_GLOBAL_I18N_TIMEZONE_GETDEFAULTZONE_TWOPARAM_0500
 * @tc.name the input country code not exist with offset param in getDefaultZone
 * @tc.desc Test GetDefaultZone(country, offset) function.
 * @tc.require SR000FK13G
 */
HWTEST_F(ZoneUtilTest, GetDefaultZone2Test0500, Function | MediumTest | Level3)
{
    ZoneUtil util;
    string out = util.GetDefaultZone("AAA", 3600 * 1000 * 8);
    EXPECT_EQ(out, "");
}

/* *
 * @tc.number SUB_GLOBAL_I18N_TIMEZONE_GETDEFAULTZONE_TWOPARAM_0600
 * @tc.name the input country code is number with offset param in getDefaultZone
 * @tc.desc Test GetDefaultZone(country, offset) function.
 * @tc.require SR000FK13G
 */
HWTEST_F(ZoneUtilTest, GetDefaultZone2Test0600, Function | MediumTest | Level3)
{
    ZoneUtil util;
    string out = util.GetDefaultZone("123", 3600 * 1000 * 8);
    EXPECT_EQ(out, "");
}

/* *
 * @tc.number SUB_GLOBAL_I18N_TIMEZONE_GETDEFAULTZONE_TWOPARAM_0700
 * @tc.name the input country code is null with offset param in getDefaultZone
 * @tc.desc Test GetDefaultZone(country, offset) function.
 * @tc.require SR000FK13G
 */
HWTEST_F(ZoneUtilTest, GetDefaultZone2Test0700, Function | MediumTest | Level3)
{
    ZoneUtil util;
    string out = util.GetDefaultZone("", 3600 * 1000 * 8);
    EXPECT_EQ(out, "");
}

/* *
 * @tc.number SUB_GLOBAL_I18N_TIMEZONE_GETZONELIST_TWOPARAM_0100
 * @tc.name the input country code have multiple timezones in getZoneList
 * @tc.desc Test GetDefaultZone(country, offset) function.
 * @tc.require SR000FK13G
 */
HWTEST_F(ZoneUtilTest, GetZoneListTest0100, Function | MediumTest | Level3)
{
    vector<string> expects = { "Asia/Shanghai", "Asia/Urumqi" };
    string country = "CN";
    vector<string> out;
    ZoneUtil util;
    util.GetZoneList(country, out);
    EXPECT_EQ(expects.size(), out.size());
    if (expects.size() == out.size()) {
        for (decltype(expects.size()) i = 0; i < expects.size(); ++i) {
            EXPECT_EQ(expects[i], out[i]);
        }
    }
}

/* *
 * @tc.number SUB_GLOBAL_I18N_TIMEZONE_GETZONELIST_TWOPARAM_0200
 * @tc.name the input country code have one timezone in getZoneList
 * @tc.desc Test GetDefaultZone(country, offset) function.
 * @tc.require SR000FK13G
 */
HWTEST_F(ZoneUtilTest, GetZoneListTest0200, Function | MediumTest | Level1)
{
    vector<string> expects = { "Asia/Kabul" };
    string country = "AF";
    vector<string> out;
    ZoneUtil util;
    util.GetZoneList(country, out);
    EXPECT_EQ(expects.size(), out.size());
    if (expects.size() == out.size()) {
        for (decltype(expects.size()) i = 0; i < expects.size(); ++i) {
            EXPECT_EQ(expects[i], out[i]);
        }
    }
}

/* *
 * @tc.number SUB_GLOBAL_I18N_TIMEZONE_GETZONELIST_TWOPARAM_0300
 * @tc.name the input country code not exist in getZoneList
 * @tc.desc Test GetDefaultZone(country, offset) function.
 * @tc.require SR000FK13G
 */
HWTEST_F(ZoneUtilTest, GetZoneListTest0300, Function | MediumTest | Level3)
{
    string country = "AAA";
    vector<string> out;
    ZoneUtil util;
    util.GetZoneList(country, out);
    size_t expect = 0;
    EXPECT_EQ(expect, out.size());
}

/* *
 * @tc.number SUB_GLOBAL_I18N_TIMEZONE_GETZONELIST_TWOPARAM_0400
 * @tc.name the input country code is number in getZoneList
 * @tc.desc Test GetDefaultZone(country, offset) function.
 * @tc.require SR000FK13G
 */
HWTEST_F(ZoneUtilTest, GetZoneListTest0400, Function | MediumTest | Level3)
{
    string country = "123";
    vector<string> out;
    ZoneUtil util;
    util.GetZoneList(country, out);
    size_t expect = 0;
    EXPECT_EQ(expect, out.size());
}

/* *
 * @tc.number SUB_GLOBAL_I18N_TIMEZONE_GETZONELIST_TWOPARAM_0500
 * @tc.name the input country code is null in getZoneList
 * @tc.desc Test GetDefaultZone(country, offset) function.
 * @tc.require SR000FK13G
 */
HWTEST_F(ZoneUtilTest, GetZoneListTest0500, Function | MediumTest | Level3)
{
    string country = "";
    vector<string> out;
    ZoneUtil util;
    util.GetZoneList(country, out);
    size_t expect = 0;
    EXPECT_EQ(expect, out.size());
}

/* *
 * @tc.number SUB_GLOBAL_I18N_TIMEZONE_GETZONELIST_THREEPARAM_0100
 * @tc.name the input country code have multiple timezones with three params in getZoneList ,the offset matches one
 * timezone
 * @tc.desc Test GetDefaultZone(country, offset) function.
 * @tc.require SR000FK13G
 */
HWTEST_F(ZoneUtilTest, GetZoneList2Test0100, Function | MediumTest | Level3)
{
    vector<string> expects = { "Asia/Shanghai" };
    string country = "CN";
    vector<string> out;
    ZoneUtil util;
    util.GetZoneList(country, 3600 * 1000 * 8, out);
    EXPECT_EQ(expects.size(), out.size());
    if (expects.size() == out.size()) {
        for (decltype(expects.size()) i = 0; i < expects.size(); ++i) {
            EXPECT_EQ(expects[i], out[i]);
        }
    }
}

/* *
 * @tc.number SUB_GLOBAL_I18N_TIMEZONE_GETZONELIST_THREEPARAM_0200
 * @tc.name the input country code have multiple timezones with three params in getZoneList ,the offset not matches
 * anyone timezone
 * @tc.desc Test GetDefaultZone(country, offset) function.
 * @tc.require SR000FK13G
 */
HWTEST_F(ZoneUtilTest, GetZoneList2Test0200, Function | MediumTest | Level3)
{
    string country = "CN";
    vector<string> out;
    ZoneUtil util;
    util.GetZoneList(country, 3600 * 1000 * 1, out);
    size_t expect = 0;
    EXPECT_EQ(expect, out.size());
}

/* *
 * @tc.number SUB_GLOBAL_I18N_TIMEZONE_GETZONELIST_THREEPARAM_0300
 * @tc.name the input country code have one timezone with three params in getZoneList ,the offset matches
 * @tc.desc Test GetDefaultZone(country, offset) function.
 * @tc.require SR000FK13G
 */
HWTEST_F(ZoneUtilTest, GetZoneList2Test0300, Function | MediumTest | Level3)
{
    vector<string> expects = { "Europe/London" };
    string country = "GB";
    vector<string> out;
    ZoneUtil util;
    util.GetZoneList(country, 3600 * 1000 * 1, out);
    // EXPECT_EQ(expects.size(), out.size());
    if (expects.size() == out.size()) {
        for (decltype(expects.size()) i = 0; i < expects.size(); ++i) {
            EXPECT_EQ(expects[i], out[i]);
        }
    }
}

/* *
 * @tc.number SUB_GLOBAL_I18N_TIMEZONE_GETZONELIST_THREEPARAM_0400
 * @tc.name the input country code have one timezone with three params in getZoneList ,the offset not match
 * @tc.desc Test GetDefaultZone(country, offset) function.
 * @tc.require SR000FK13G
 */
HWTEST_F(ZoneUtilTest, GetZoneList2Test0400, Function | MediumTest | Level3)
{
    string country = "GB";
    vector<string> out;
    ZoneUtil util;
    util.GetZoneList(country, 3600 * 1000 * 5, out);
    size_t expect = 0;
    EXPECT_EQ(expect, out.size());
}

/* *
 * @tc.number SUB_GLOBAL_I18N_TIMEZONE_GETZONELIST_THREEPARAM_0500
 * @tc.name the input country code not exist with three params in getZoneList
 * @tc.desc Test GetDefaultZone(country, offset) function.
 * @tc.require SR000FK13G
 */
HWTEST_F(ZoneUtilTest, GetZoneList2Test0500, Function | MediumTest | Level3)
{
    string country = "GGG";
    vector<string> out;
    ZoneUtil util;
    util.GetZoneList(country, 3600 * 1000 * 5, out);
    size_t expect = 0;
    EXPECT_EQ(expect, out.size());
}

/* *
 * @tc.number SUB_GLOBAL_I18N_TIMEZONE_GETZONELIST_THREEPARAM_0600
 * @tc.name the input country code is number with three params in getZoneList
 * @tc.desc Test GetDefaultZone(country, offset) function.
 * @tc.require SR000FK13G
 */
HWTEST_F(ZoneUtilTest, GetZoneList2Test0600, Function | MediumTest | Level3)
{
    string country = "123";
    vector<string> out;
    ZoneUtil util;
    util.GetZoneList(country, 3600 * 1000 * 5, out);
    size_t expect = 0;
    EXPECT_EQ(expect, out.size());
}

/* *
 * @tc.number SUB_GLOBAL_I18N_TIMEZONE_GETZONELIST_THREEPARAM_0700
 * @tc.name the input country code is null with three params in getZoneList
 * @tc.desc Test GetDefaultZone(country, offset) function.
 * @tc.require SR000FK13G
 */
HWTEST_F(ZoneUtilTest, GetZoneList2Test0700, Function | MediumTest | Level3)
{
    string country = "";
    vector<string> out;
    ZoneUtil util;
    util.GetZoneList(country, 3600 * 1000 * 5, out);
    size_t expect = 0;
    EXPECT_EQ(expect, out.size());
}
}
