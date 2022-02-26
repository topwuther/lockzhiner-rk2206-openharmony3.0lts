/*
 * Copyright (C) 2021 Huawei Device Co., Ltd.
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
import Intl from '@ohos.intl'
import {describe, beforeAll, beforeEach, afterEach, afterAll, it, expect} from 'deccjsunit/index'

describe('intlTest', function () {
    console.log('*************start IntlTest*************');

    /* *
    * @tc.number SUB_GLOBAL_INTL_JS_LOCALE_0100
    * @tc.name test the language in en-Latn-GB locale
    * @tc.desc check the language
    */
    it('locale_test_0100', 0, function () {
        let locale = new Intl.Locale('en-Latn-GB');
        console.log('locale_test_0100 ' + locale.language);
        expect(locale.language).assertEqual('en');
    })

    /* *
    * @tc.number SUB_GLOBAL_INTL_JS_LOCALE_0200
    * @tc.name test the language in ja-Jpan-JP-u-ca-japanese-hc-h12-co-emoji locale
    * @tc.desc check the language
    */
    it('locale_test_0200', 0, function () {
        let locale = new Intl.Locale('ja-Jpan-JP-u-ca-japanese-hc-h12-co-emoji');
        console.log('locale_test_0200 ' + locale.language);
        expect(locale.language).assertEqual('ja');
    })

    /* *
    * @tc.number SUB_GLOBAL_INTL_JS_LOCALE_0300
    * @tc.name test the script in locale
    * @tc.desc check the script
    */
    it('locale_test_0300', 0, function () {
        let locale = new Intl.Locale('en-Latn-GB');
        console.log('locale_test_0300 ' + locale.script);
        expect(locale.script).assertEqual('Latn');
    })

    /* *
    * @tc.number SUB_GLOBAL_INTL_JS_LOCALE_0400
    * @tc.name test the region in locale
    * @tc.desc check the region
    */
    it('locale_test_0400', 0, function () {
        let locale = new Intl.Locale('en-Latn-GB');
        console.log('locale_test_0400 ' + locale.region);
        expect(locale.region).assertEqual('GB');
    })

    /* *
    * @tc.number SUB_GLOBAL_INTL_JS_LOCALE_0500
    * @tc.name test the basename in locale
    * @tc.desc check the basename
    */
    it('locale_test_0500', 0, function () {
        let locale = new Intl.Locale('en-Latn-GB');
        console.log('locale_test_0500 ' + locale.baseName);
        expect(locale.baseName).assertEqual('en-Latn-GB');
    })

    /* *
    * @tc.number SUB_GLOBAL_INTL_JS_LOCALE_0600
    * @tc.name test the basename in ja-Jpan-JP-u-ca-japanese-hc-h12
    * @tc.desc check the ja-Jpan-JP-u-ca-japanese-hc-h12 basename
    */
    it('locale_test_0600', 0, function () {
        let locale = new Intl.Locale('ja-Jpan-JP-u-ca-japanese-hc-h12');
        console.log('locale_test_0600 ' + locale.baseName);
        expect(locale.baseName).assertEqual('ja-Jpan-JP');
    })

    /* *
    * @tc.number SUB_GLOBAL_INTL_JS_LOCALE_0700
    * @tc.name test the hourCycle
    * @tc.desc check the hourCycle
    */
    it('locale_test_0700', 0, function () {
        let locale = new Intl.Locale('zh-CN', {hourCycle: 'h24', calendar: 'gregory'});
        console.log('locale_test_0700 ' + locale.hourCycle);
        expect(locale.hourCycle).assertEqual('h24');
    })

    /* *
    * @tc.number SUB_GLOBAL_INTL_JS_LOCALE_0800
    * @tc.name test the calendar
    * @tc.desc check the calendar
    */
    it('locale_test_0800', 0, function () {
        let locale = new Intl.Locale('zh-CN', {hourCycle: '24', calendar: 'gregory'});
        console.log('locale_test_0800 ' + locale.calendar);
        expect(locale.calendar).assertEqual('gregory');
    })

    /* *
    * @tc.number SUB_GLOBAL_INTL_JS_LOCALE_0900
    * @tc.name test the japanese calendar
    * @tc.desc check the japanese calendar
    */
    it('locale_test_0900', 0, function () {
        let locale = new Intl.Locale('ja-Jpan-JP-u-ca-japanese-hc-h12');
        console.log('locale_test_0900 ' + locale.calendar);
        expect(locale.calendar).assertEqual('japanese');
    })

    /* *
    * @tc.number SUB_GLOBAL_INTL_JS_LOCALE_1000
    * @tc.name test the h12 hourCycle
    * @tc.desc check the h12 hourCycle
    */
    it('locale_test_1000', 0, function () {
        let locale = new Intl.Locale('ja-Jpan-JP-u-ca-japanese-hc-h12');
        console.log('locale_test_1000 ' + locale.hourCycle);
        expect(locale.hourCycle).assertEqual('h12');
    })

    /* *
    * @tc.number SUB_GLOBAL_INTL_JS_LOCALE_1100
    * @tc.name test the caseFirst with upper
    * @tc.desc check the caseFirst
    */
    it('locale_test_1100', 0, function () {
        let locale = new Intl.Locale('ja-Jpan-JP', {caseFirst: 'upper'});
        console.log('locale_test_1100 ' + locale.caseFirst);
        expect(locale.caseFirst).assertEqual('upper');
    })

    /* *
    * @tc.number SUB_GLOBAL_INTL_JS_LOCALE_1120
    * @tc.name test the caseFirst with lower
    * @tc.desc check the caseFirst
    */
    it('locale_test_1120', 0, function () {
        let locale = new Intl.Locale('ja-Jpan-JP', {caseFirst: 'lower'});
        console.log('locale_test_1120 ' + locale.caseFirst);
        expect(locale.caseFirst).assertEqual('lower');
    })

    /* *
    * @tc.number SUB_GLOBAL_INTL_JS_LOCALE_1140
    * @tc.name test the caseFirst with false
    * @tc.desc check the caseFirst
    */
    it('locale_test_1140', 0, function () {
        let locale = new Intl.Locale('ja-Jpan-JP', {caseFirst: 'false'});
        console.log('locale_test_1140 ' + locale.caseFirst);
        expect(locale.caseFirst).assertEqual('false');
    })

    /* *
    * @tc.number SUB_GLOBAL_INTL_JS_LOCALE_1200
    * @tc.name test the collation
    * @tc.desc check the collation
    */
    it('locale_test_1200', 0, function () {
        let locale = new Intl.Locale('ja-Jpan-JP', {collation: 'big5han'});
        console.log('locale_test_1200 ' + locale.collation);
        expect(locale.collation).assertEqual('big5han');
    })

    /* *
    * @tc.number SUB_GLOBAL_INTL_JS_LOCALE_1300
    * @tc.name test the numeric with true value
    * @tc.desc check the numeric
    */
    it('locale_test_1300', 0, function () {
        let locale = new Intl.Locale('ja-Jpan-JP', {numeric: true});
        console.log('locale_test_1300 ' + locale.numeric);
        expect(locale.numeric).assertTrue();
    })

    /* *
    * @tc.number SUB_GLOBAL_INTL_JS_LOCALE_1350
    * @tc.name test the numeric with false value
    * @tc.desc check the numeric
    */
    it('locale_test_1350', 0, function () {
        let locale = new Intl.Locale('ja-Jpan-JP', {numeric: false});
        console.log('locale_test_1350 ' + locale.numeric);
        expect(locale.numeric).assertFalse();
    })

    /* *
    * @tc.number SUB_GLOBAL_INTL_JS_LOCALE_1400
    * @tc.name test the numberingSystem
    * @tc.desc check the numberingSystem
    */
    it('locale_test_1400', 0, function () {
        let locale = new Intl.Locale('ja-Jpan-JP', {numberingSystem: 'arab'});
        console.log('locale_test_1400 ' + locale.numberingSystem);
        expect(locale.numberingSystem).assertEqual('arab');
    })

    /* *
    * @tc.number SUB_GLOBAL_INTL_JS_LOCALE_1500
    * @tc.name test the toString interface with zh
    * @tc.desc check the toString method with zh
    */
    it('locale_test_1500', 0, function () {
        let locale = new Intl.Locale('zh');
        console.log('locale_test_1500 ' + locale.toString());
        expect(locale.toString()).assertEqual('zh');
    })

    /* *
    * @tc.number SUB_GLOBAL_INTL_JS_LOCALE_1600
    * @tc.name test the toString interface with zh-CN
    * @tc.desc check the toString method with zh-CN
    */
    it('locale_test_1600', 0, function () {
        let locale = new Intl.Locale('zh-CN');
        console.log('locale_test_1600 ' + locale.toString());
        expect(locale.toString()).assertEqual('zh-CN');
    })

    /* *
    * @tc.number SUB_GLOBAL_INTL_JS_LOCALE_1700
    * @tc.name test the toString interface with zh-Hans-CN
    * @tc.desc check the toString method with zh-Hans-CN
    */
    it('locale_test_1700', 0, function () {
        let locale = new Intl.Locale('zh-Hans-CN');
        console.log('locale_test_1700 ' + locale.toString());
        expect(locale.toString()).assertEqual('zh-Hans-CN');
    })

    /* *
    * @tc.number SUB_GLOBAL_INTL_JS_LOCALE_1800
    * @tc.name test the toString interface with zh-Hans-CN-u-hc-h12-ca-japanese
    * @tc.desc check the toString method with zh-Hans-CN-u-hc-h12-ca-japanese
    */
    it('locale_test_1800', 0, function () {
        let locale = new Intl.Locale('zh-Hans-CN-u-ca-japanese-hc-h12');
        console.log('locale_test_1800 ' + locale.toString());
        expect(locale.toString()).assertEqual('zh-Hans-CN-u-hc-h12-ca-japanese');
    })

    /* *
    * @tc.number SUB_GLOBAL_INTL_JS_LOCALE_1900
    * @tc.name test the maximize interface with zh-u-ca-gregory-co-compact param
    * @tc.desc check the maximize method with zh-u-ca-gregory-co-compact param
    */
    it('locale_test_1900', 0, function () {
        let locale = new Intl.Locale('zh', {calendar: 'gregory', collation: 'compact'});
        console.log('locale_test_1900 ' + locale.toString());
        expect(locale.toString()).assertEqual('zh-u-ca-gregory-co-compact');
    })

    /* *
    * @tc.number SUB_GLOBAL_INTL_JS_LOCALE_2000
    * @tc.name test the toString interface with zh-CN-u-ca-gregory-co-compact param
    * @tc.desc check the toString method with zh-CN-u-ca-gregory-co-compact param
    */
    it('locale_test_2000', 0, function () {
        let locale = new Intl.Locale('zh-CN', {calendar: 'gregory', collation: 'compact'});
        console.log('locale_test_2000 ' + locale.toString());
        expect(locale.toString()).assertEqual('zh-CN-u-ca-gregory-co-compact');
    })

    /* *
    * @tc.number SUB_GLOBAL_INTL_JS_LOCALE_2100
    * @tc.name test the toString interface with zh-hans-CN-u-ca-gregory-co-compact param
    * @tc.desc check the toString method with zh-hans-CN-u-ca-gregory-co-compact param
    */
    it('locale_test_2100', 0, function () {
        let locale = new Intl.Locale('zh-hans-CN', {calendar: 'gregory', collation: 'compact'});
        console.log('locale_test_2100 ' + locale.toString());
        expect(locale.toString()).assertEqual('zh-hans-CN-u-ca-gregory-co-compact');
    })

    /* *
    * @tc.number SUB_GLOBAL_INTL_JS_LOCALE_2200
    * @tc.name test the minimize interface with one param zh
    * @tc.desc check the minimize method with one param zh
    */
    it('locale_test_2200', 0, function () {
        let locale = new Intl.Locale('zh');
        console.log('locale_test_2200 ' + locale.minimize().toString());
        expect(locale.minimize().toString()).assertEqual('zh');
    })

    /* *
    * @tc.number SUB_GLOBAL_INTL_JS_LOCALE_2201
    * @tc.name test the minimize interface with one param zh-CN
    * @tc.desc check the minimize method with one param zh-CN
    */
    it('locale_test_2201', 0, function () {
        let locale = new Intl.Locale('zh-CN');
        console.log('locale_test_2201 ' + locale.minimize().toString());
        expect(locale.minimize().toString()).assertEqual('zh');
    })

    /* *
    * @tc.number SUB_GLOBAL_INTL_JS_LOCALE_2202
    * @tc.name test the minimize interface with one param zh-Hans-CN
    * @tc.desc check the minimize method with one param zh-Hans-CN
    */
    it('locale_test_2202', 0, function () {
        let locale = new Intl.Locale('zh-Hans-CN');
        console.log('locale_test_2202 ' + locale.minimize().toString());
        expect(locale.minimize().toString()).assertEqual('zh');
    })

    /* *
    * @tc.number SUB_GLOBAL_INTL_JS_LOCALE_2203
    * @tc.name test the minimize interface with one param zh-Hans-CN-u-ca-gregory-co-compact
    * @tc.desc check the minimize method with one param zh-Hans-CN-u-ca-gregory-co-compact
    */
    it('locale_test_2203', 0, function () {
        let locale = new Intl.Locale('zh-Hans-CN-u-ca-gregory-co-compact');
        console.log('locale_test_2203 ' + locale.minimize().toString());
        expect(locale.minimize().toString()).assertEqual('zh-u-ca-gregory-co-compact');
    })

    /* *
    * @tc.number SUB_GLOBAL_INTL_JS_LOCALE_2204
    * @tc.name test the minimize interface with zh lcoale
    * @tc.desc check the minimize method with zh lcoale
    */
    it('locale_test_2204', 0, function () {
        let locale = new Intl.Locale('zh', {calendar: 'gregory', collation: 'compact'});
        console.log('locale_test_2204 ' + locale.minimize().toString());
        expect(locale.minimize().toString()).assertEqual('zh-u-ca-gregory-co-compact');
    })

    /* *
    * @tc.number SUB_GLOBAL_INTL_JS_LOCALE_2205
    * @tc.name test the minimize interface with zh-CN lcoale
    * @tc.desc check the minimize method with zh-CN lcoale
    */
    it('locale_test_2205', 0, function () {
        let locale = new Intl.Locale('zh-CN', {calendar: 'gregory', collation: 'compact'});
        console.log('locale_test_2205 ' + locale.minimize().toString());
        expect(locale.minimize().toString()).assertEqual('zh-u-ca-gregory-co-compact');
    })

    /* *
    * @tc.number SUB_GLOBAL_INTL_JS_LOCALE_2206
    * @tc.name test the minimize interface with zh-Hans-CN lcoale
    * @tc.desc check the minimize method with zh-Hans-CN lcoale
    */
    it('locale_test_2206', 0, function () {
        let locale = new Intl.Locale('zh-hans-CN', {calendar: 'gregory', collation: 'compact'});
        console.log('locale_test_2206 ' + locale.minimize().toString());
        expect(locale.minimize().toString()).assertEqual('zh-u-ca-gregory-co-compact');
    })

    /* *
    * @tc.number SUB_GLOBAL_INTL_JS_LOCALE_2300
    * @tc.name test the maximize interface with one param zh
    * @tc.desc check the maximize method with one param zh
    */
    it('locale_test_2300', 0, function () {
        let locale = new Intl.Locale('zh');
        console.log('locale_test_2300 ' + locale.maximize().toString());
        expect(locale.maximize().toString()).assertEqual('zh-Hans-CN');
    })

    /* *
    * @tc.number SUB_GLOBAL_INTL_JS_LOCALE_2301
    * @tc.name test the maximize interface with one param zh-CN
    * @tc.desc check the maximize method with one param zh-CN
    */
    it('locale_test_2301', 0, function () {
        let locale = new Intl.Locale('zh-CN');
        console.log('locale_test_2301 ' + locale.maximize().toString());
        expect(locale.maximize().toString()).assertEqual('zh-Hans-CN');
    })

    /* *
    * @tc.number SUB_GLOBAL_INTL_JS_LOCALE_2302
    * @tc.name test the maximize interface with one param zh-Hans-CN-u-ca-gregory-co-compact
    * @tc.desc check the maximize method with one param zh-Hans-CN-u-ca-gregory-co-compact
    */
    it('locale_test_2302', 0, function () {
        let locale = new Intl.Locale('zh-Hans-CN');
        console.log('locale_test_2302 ' + locale.maximize().toString());
        expect(locale.maximize().toString()).assertEqual('zh-Hans-CN');
    })

    /* *
    * @tc.number SUB_GLOBAL_INTL_JS_LOCALE_2303
    * @tc.name test the maximize interface with zh-Hans-CN-u-ca-gregory-co-compact locale
    * @tc.desc check the maximize method with zh-Hans-CN-u-ca-gregory-co-compact locale
    */
    it('locale_test_2303', 0, function () {
        let locale = new Intl.Locale('zh-Hans-CN-u-ca-gregory-co-compact');
        console.log('locale_test_2303 ' + locale.maximize().toString());
        expect(locale.maximize().toString()).assertEqual('zh-Hans-CN-u-ca-gregory-co-compact');
    })

    /* *
    * @tc.number SUB_GLOBAL_INTL_JS_LOCALE_2304
    * @tc.name test the maximize interface with zh locale
    * @tc.desc check the maximize method with zh locale
    */
    it('locale_test_2304', 0, function () {
        let locale = new Intl.Locale('zh', {calendar: 'gregory', collation: 'compact'});
        console.log('locale_test_2304 ' + locale.maximize().toString());
        expect(locale.maximize().toString()).assertEqual('zh-Hans-CN-u-ca-gregory-co-compact');
    })

    /* *
    * @tc.number SUB_GLOBAL_INTL_JS_LOCALE_2305
    * @tc.name test the maximize interface with zh-CN locale
    * @tc.desc check the maximize method with zh-CN locale
    */
    it('locale_test_2305', 0, function () {
        let locale = new Intl.Locale('zh-CN', {calendar: 'gregory', collation: 'compact'});
        console.log('locale_test_2305 ' + locale.maximize().toString());
        expect(locale.maximize().toString()).assertEqual('zh-Hans-CN-u-ca-gregory-co-compact');
    })

    /* *
    * @tc.number SUB_GLOBAL_INTL_JS_LOCALE_2306
    * @tc.name test the maximize interface with zh-Hans-CN locale
    * @tc.desc check the maximize method with zh-Hans-CN locale
    */
    it('locale_test_2306', 0, function () {
        let locale = new Intl.Locale('zh-Hans-CN', {calendar: 'gregory', collation: 'compact'});
        console.log('locale_test_2306 ' + locale.maximize().toString());
        expect(locale.maximize().toString()).assertEqual('zh-Hans-CN-u-ca-gregory-co-compact');
    })

    /* *
    * @tc.number SUB_GLOBAL_INTL_JS_DATETIME_0100
    * @tc.name format the datetime with en-GB locale
    * @tc.desc check the datetime is not null
    */
    it('dateTimeFormat_test_0100', 0, function () {
        let datefmt = new Intl.DateTimeFormat('en-GB');
        expect(datefmt !== null).assertTrue();
    })

    /* *
    * @tc.number SUB_GLOBAL_INTL_JS_DATETIME_0200
    * @tc.name format the date with zh locale
    * @tc.desc check the date
    */
    it('dateTimeFormat_test_0200', 0, function () {
        let date = new Date(2021, 11, 17, 3, 24, 0);
        let datefmt = new Intl.DateTimeFormat('zh');
        console.log('dateTimeFormat_test_0200 ' + datefmt.format(date));
        expect(datefmt.format(date)).assertEqual('2021/12/17');
    })

    /* *
    * @tc.number SUB_GLOBAL_INTL_JS_DATETIME_0300
    * @tc.name format the date with en locale
    * @tc.desc check the date
    */
    it('dateTimeFormat_test_0300', 0, function () {
        let date = new Date(2021, 11, 17, 3, 24, 0);
        let datefmt = new Intl.DateTimeFormat('en');
        console.log('dateTimeFormat_test_0300 ' + datefmt.format(date));
        expect(datefmt.format(date)).assertEqual('12/17/21');
    })

    /* *
    * @tc.number SUB_GLOBAL_INTL_JS_DATETIME_0310
    * @tc.name format the date with en-US locale
    * @tc.desc check the date
    */
    it('dateTimeFormat_test_0310', 0, function () {
        let date = new Date(2021, 11, 17, 3, 24, 0);
        let datefmt = new Intl.DateTimeFormat('en-US');
        console.log('dateTimeFormat_test_0310 ' + datefmt.format(date));
        expect(datefmt.format(date)).assertEqual('12/17/21');
    })

    /* *
    * @tc.number SUB_GLOBAL_INTL_JS_DATETIME_0320
    * @tc.name format the date with en-GB locale
    * @tc.desc check the date
    */
    it('dateTimeFormat_test_0320', 0, function () {
        let date = new Date(2021, 11, 17, 3, 24, 0);
        let datefmt = new Intl.DateTimeFormat('en-GB');
        console.log('dateTimeFormat_test_0320 ' + datefmt.format(date));
        expect(datefmt.format(date)).assertEqual('17/12/2021');
    })

    /* *
    * @tc.number SUB_GLOBAL_INTL_JS_DATETIME_0400
    * @tc.name format the date with aa locale
    * @tc.desc check the date
    */
    it('dateTimeFormat_test_0400', 0, function () {
        let date = new Date(2021, 11, 17, 3, 24, 0);
        let datefmt = new Intl.DateTimeFormat('aa');
        let value = datefmt.format(date);
        console.log('dateTimeFormat_test_0400 ' + value);
        expect(value).assertInstanceOf('String');
    })

    /* *
    * @tc.number SUB_GLOBAL_INTL_JS_DATETIME_0500
    * @tc.name format the hour with ban and zh locale
    * @tc.desc check the hour with ban and zh locale
    */
    it('dateTimeFormat_test_0500', 0, function () {
        let date = new Date(2020, 11, 20, 14, 23, 16);
        let option = { dateStyle: 'full' };
        let datefmt = new Intl.DateTimeFormat(['ban', 'zh'], option);
        console.log('dateTimeFormat_test_0500 ' + datefmt.format(date));
        expect(datefmt.format(date)).assertEqual('2020年12月20日星期日');
    })

    /* *
    * @tc.number SUB_GLOBAL_INTL_JS_DATETIME_0600
    * @tc.name format the hour with en and zh locale
    * @tc.desc check the hour with en and zh locale
    */
    it('dateTimeFormat_test_0600', 0, function () {
        let date = new Date(2020, 11, 20, 14, 23, 16);
        let option = { dateStyle: 'full' };
        let datefmt = new Intl.DateTimeFormat(['en', 'zh'], option);
        console.log('dateTimeFormat_test_0600 ' + datefmt.format(date));
        expect(datefmt.format(date)).assertEqual('Sunday, December 20, 2020');
    })

    /* *
    * @tc.number SUB_GLOBAL_INTL_JS_DATETIME_0700
    * @tc.name format the hour with en and ban locale
    * @tc.desc check the hour with en and ban locale
    */
    it('dateTimeFormat_test_0700', 0, function () {
        let date = new Date(2020, 11, 20, 14, 23, 16);
        let option = { dateStyle: 'full' };
        let datefmt = new Intl.DateTimeFormat(['en', 'ban'], option);
        console.log('dateTimeFormat_test_0700 ' + datefmt.format(date));
        expect(datefmt.format(date)).assertEqual('Sunday, December 20, 2020');
    })

    /* *
    * @tc.number SUB_GLOBAL_INTL_JS_DATETIME_0800
    * @tc.name format the hour without correct locale
    * @tc.desc check the hour without correct locale
    */
    it('dateTimeFormat_test_0800', 0, function () {
        let date = new Date(2020, 11, 20, 14, 23, 16);
        let option = { dateStyle: 'full' };
        let datefmt = new Intl.DateTimeFormat(['abc', 'ban'], option);
        console.log('dateTimeFormat_test_0800 ' + datefmt.format(date));
        expect(datefmt.format(date)).assertEqual('12/20/20, 2:23 PM');
    })

    /* *
    * @tc.number SUB_GLOBAL_INTL_JS_DATETIME_0900
    * @tc.name format the date with full datestyle and mediu timestyle
    * @tc.desc check the date with full datestyle and mediu timestyle
    */
    it('dateTimeFormat_test_0900', 0, function () {
        let date = new Date(2020, 11, 20, 14, 23, 16);
        let option = { dateStyle: 'full', timeStyle: 'full' };
        let datefmt = new Intl.DateTimeFormat('zh-Hans-CN', option);
        console.log('dateTimeFormat_test_0900 ' + datefmt.format(date));
        expect(datefmt.format(date)).assertEqual('2020年12月20日星期日 协调世界时 下午2:23:16');
    })

    /* *
    * @tc.number SUB_GLOBAL_INTL_JS_DATETIME_1000
    * @tc.name format the date dateStyle with long style
    * @tc.desc check the dateStyle with long style
    */
    it('dateTimeFormat_test_1000', 0, function () {
        let date = new Date(2020, 11, 20, 14, 23, 16);
        let option = { dateStyle: 'long', timeStyle: 'long' };
        let datefmt = new Intl.DateTimeFormat('zh-CN', option);
        console.log('dateTimeFormat_test_1000 ' + datefmt.resolvedOptions().dateStyle);
        expect(datefmt.resolvedOptions().dateStyle).assertEqual('long');
        expect(datefmt.format(date)).assertEqual('2020年12月20日 UTC 下午2:23:16');
    })

    /* *
    * @tc.number SUB_GLOBAL_INTL_JS_DATETIME_1100
    * @tc.name format the date dateStyle with medium style
    * @tc.desc check the dateStyle with medium style
    */
    it('dateTimeFormat_test_1100', 0, function () {
        let date = new Date(2020, 11, 20, 14, 23, 16);
        let option = { dateStyle: 'medium', timeStyle: 'medium' };
        let datefmt = new Intl.DateTimeFormat('zh-CN', option);
        console.log('dateTimeFormat_test_1100 ' + datefmt.resolvedOptions().dateStyle);
        expect(datefmt.resolvedOptions().dateStyle).assertEqual('medium');
        expect(datefmt.format(date)).assertEqual('2020年12月20日 下午2:23:16');
    })

    /* *
    * @tc.number SUB_GLOBAL_INTL_JS_DATETIME_1200
    * @tc.name format the date dateStyle with short style
    * @tc.desc check the dateStyle with short style
    */
    it('dateTimeFormat_test_1200', 0, function () {
        let date = new Date(2020, 11, 20, 14, 23, 16);
        let option = { dateStyle: 'short', timeStyle: 'short' };
        let datefmt = new Intl.DateTimeFormat('zh-CN', option);
        console.log('dateTimeFormat_test_1200 ' + datefmt.resolvedOptions().dateStyle);
        expect(datefmt.resolvedOptions().dateStyle).assertEqual('short');
        expect(datefmt.format(date)).assertEqual('2020/12/20 下午2:23');
    })

    /* *
    * @tc.number SUB_GLOBAL_INTL_JS_DATETIME_1300
    * @tc.name format the date with daterange
    * @tc.desc check the daterange
    */
    it('dateTimeFormat_test_1300', 0, function () {
        let startdate = new Date(2020, 11, 20, 14, 23, 16);
        let enddate = new Date(2020, 11, 21, 14, 23, 16);
        let datefmt = new Intl.DateTimeFormat('en-GB');
        console.log('dateTimeFormat_test_1300 ' + datefmt.formatRange(startdate, enddate));
        expect(datefmt.formatRange(startdate, enddate)).assertEqual('20/12/2020 – 21/12/2020');
    })

    /* *
    * @tc.number SUB_GLOBAL_INTL_JS_DATETIME_1400
    * @tc.name format the date with daterange when the start and end date is equal
    * @tc.desc check the daterange when the start and end date is equal
    */
    it('dateTimeFormat_test_1400', 0, function () {
        let startdate = new Date(2020, 11, 20, 14, 23, 16);
        let enddate = new Date(2020, 11, 20, 14, 23, 16);
        let datefmt = new Intl.DateTimeFormat('en-GB');
        console.log('dateTimeFormat_test_1400 ' + datefmt.formatRange(startdate, enddate));
        expect(datefmt.formatRange(startdate, enddate)).assertEqual('20/12/2020');
    })

    /* *
    * @tc.number SUB_GLOBAL_INTL_JS_DATETIME_1500
    * @tc.name format the year、month、day、weekday
    * @tc.desc check the year、month、day、weekday
    */
    it('dateTimeFormat_test_1500', 0, function () {
        let date = new Date(2020, 11, 20, 14, 23, 16);
        let option = { year: 'numeric', month: 'long', day: 'numeric', weekday: 'long' };
        let datefmt = new Intl.DateTimeFormat('ja', option);
        console.log('dateTimeFormat_test_1500 ' + datefmt.format(date));
        expect(datefmt.format(date)).assertEqual('2020年12月20日日曜日');
    })

    /* *
    * @tc.number SUB_GLOBAL_INTL_JS_DATETIME_1600
    * @tc.name format the timerange with year、month、day、weekday
    * @tc.desc check the timerange with year、month、day、weekday
    */
    it('dateTimeFormat_test_1600', 0, function () {
        let startdate = new Date(2020, 11, 20, 14, 23, 16);
        let enddate = new Date(2021, 4, 5, 10, 5, 3);
        let option = { year: 'numeric', month: 'long', day: 'numeric', weekday: 'long' };
        let datefmt = new Intl.DateTimeFormat('en', option);
        console.log('dateTimeFormat_test_1600 ' + datefmt.formatRange(startdate, enddate));
        expect(datefmt.formatRange(startdate, enddate)).assertEqual('Sunday, December 20, 2020 – Wednesday, May 5, 2021');
    })

    /* *
    * @tc.number SUB_GLOBAL_INTL_JS_DATETIME_1700
    * @tc.name format the hour&minute&second
    * @tc.desc check the hour&minute&second
    */
    it('dateTimeFormat_test_1700', 0, function () {
        let date = new Date(2020, 11, 20, 14, 23, 16);
        let option = { year: 'numeric', month: 'long', day: 'numeric',
            hour: 'numeric', minute: '2-digit', second: 'numeric', weekday: 'long', era: 'short' };
        let datefmt = new Intl.DateTimeFormat('zh-CN-u-hc-h12', option);
        console.log('dateTimeFormat_test_1700 ' + datefmt.format(date));
        expect(datefmt.format(date)).assertEqual('公元2020年12月20日星期日 下午2:23:16');
    })

    /* *
    * @tc.number SUB_GLOBAL_INTL_JS_DATETIME_1800
    * @tc.name format the timeZone
    * @tc.desc check the timeZone
    */
    it('dateTimeFormat_test_1800', 0, function () {
        let date = new Date(2020, 3, 14, 15, 5, 3);
        let option = { timeZone: 'America/Los_Angeles', timeZoneName: 'long' };
        let datefmt = new Intl.DateTimeFormat('zh-CN-u-hc-h12', option);
        console.log('dateTimeFormat_test_1800 ' + datefmt.format(date));
        expect(datefmt.format(date).indexOf('北美太平洋夏令时间') !== -1).assertTrue();
    })

    /* *
    * @tc.number SUB_GLOBAL_INTL_JS_DATETIME_1900
    * @tc.name format the America/Los_Angeles timeZone
    * @tc.desc check the America/Los_Angeles timeZone
    */
    it('dateTimeFormat_test_1900', 0, function () {
        let option = { timeZone: 'America/Los_Angeles', timeZoneName: 'long' };
        let datefmt = new Intl.DateTimeFormat('zh-CN-u-hc-h12', option);
        console.log('dateTimeFormat_test_1900 ' + datefmt.resolvedOptions().timeZone);
        expect(datefmt.resolvedOptions().timeZone).assertEqual('America/Los_Angeles');
    })

    /* *
    * @tc.number SUB_GLOBAL_INTL_JS_DATETIME_2000
    * @tc.name format the America/Los_Angeles timeZoneName
    * @tc.desc check the America/Los_Angeles timeZoneName
    */
    it('dateTimeFormat_test_2000', 0, function () {
        let option = { timeZone: 'America/Los_Angeles', timeZoneName: 'long' };
        let datefmt = new Intl.DateTimeFormat('zh-CN-u-hc-h12', option);
        console.log('dateTimeFormat_test_2000 ' + datefmt.resolvedOptions().timeZoneName);
        expect(datefmt.resolvedOptions().timeZoneName).assertEqual('long');
    })

    /* *
    * @tc.number SUB_GLOBAL_INTL_JS_DATETIME_2100
    * @tc.name format the year
    * @tc.desc check the year
    */
    it('dateTimeFormat_test_2100', 0, function () {
        let option = { year: 'numeric', month: 'long', day: 'numeric',
            hour: 'numeric', minute: '2-digit', second: 'numeric', weekday: 'long', era: 'short' };
        let datefmt = new Intl.DateTimeFormat('zh-CN-u-hc-h12', option);
        console.log('dateTimeFormat_test_2100 ' + datefmt.resolvedOptions().year);
        expect(datefmt.resolvedOptions().year).assertEqual('numeric');
    })

    /* *
    * @tc.number SUB_GLOBAL_INTL_JS_DATETIME_2200
    * @tc.name format the weekday
    * @tc.desc check the weekday
    */
    it('dateTimeFormat_test_2200', 0, function () {
        let option = { year: 'numeric', month: 'long', day: 'numeric',
            hour: 'numeric', minute: '2-digit', second: 'numeric', weekday: 'long', era: 'short' };
        let datefmt = new Intl.DateTimeFormat('zh-CN-u-hc-h12', option);
        console.log('dateTimeFormat_test_2200 ' + datefmt.resolvedOptions().weekday);
        expect(datefmt.resolvedOptions().weekday).assertEqual('long');
    })

    /* *
    * @tc.number SUB_GLOBAL_INTL_JS_DATETIME_2300
    * @tc.name format the hour in zh-CN-u-hc-h12
    * @tc.desc check the hour in zh-CN-u-hc-h12
    */
    it('dateTimeFormat_test_2300', 0, function () {
        let option = { year: 'numeric', month: 'long', day: 'numeric',
            hour: 'numeric', minute: '2-digit', second: 'numeric', weekday: 'long', era: 'short' };
        let datefmt = new Intl.DateTimeFormat('zh-CN-u-hc-h12', option);
        console.log('dateTimeFormat_test_2300 ' + datefmt.resolvedOptions().hour);
        expect(datefmt.resolvedOptions().hour).assertEqual('numeric');
    })

    /* *
    * @tc.number SUB_GLOBAL_INTL_JS_DATETIME_2400
    * @tc.name format the hour in fr-Latn-FR-u-nu-mong
    * @tc.desc check the hour in fr-Latn-FR-u-nu-mong
    */
    it('dateTimeFormat_test_2400', 0, function () {
        let date = new Date(2020, 11, 20, 14, 23, 16);
        let option = { dateStyle: 'full' };
        let datefmt = new Intl.DateTimeFormat('fr-Latn-FR-u-nu-mong', option);
        console.log('dateTimeFormat_test_2400 ' + datefmt.format(date));
        expect(datefmt.format(date)).assertEqual('dimanche ᠒᠐ décembre ᠒᠐᠒᠐');
    })

    /* *
    * @tc.number SUB_GLOBAL_INTL_JS_DATETIME_2500
    * @tc.name format the hour in zh-CN-u-hc-h12 with matcher
    * @tc.desc check the hour in zh-CN-u-hc-h12 with matcher
    */
    it('dateTimeFormat_test_2500', 0, function () {
        let option = { year: 'numeric', month: 'long', day: 'numeric',
            hour: 'numeric', minute: '2-digit', second: 'numeric', weekday: 'long', era: 'short',
            hourCycle: 'h24',numberingSystem: 'arab', hour12: true, dayPeriod: 'short',
            formatMatcher: 'basic', localeMatcher: 'lookup' };
        let datefmt = new Intl.DateTimeFormat('zh-CN-u-hc-h12', option);
        expect(datefmt.resolvedOptions().locale).assertEqual('zh-CN');
        expect(datefmt.resolvedOptions().hourCycle).assertEqual('h24');
        expect(datefmt.resolvedOptions().numberingSystem).assertEqual('arab');
        expect(datefmt.resolvedOptions().hour12).assertTrue();
        expect(datefmt.resolvedOptions().era).assertEqual('short');
        expect(datefmt.resolvedOptions().month).assertEqual('long');
        expect(datefmt.resolvedOptions().day).assertEqual('numeric');
        expect(datefmt.resolvedOptions().minute).assertEqual('2-digit');
        expect(datefmt.resolvedOptions().second).assertEqual('numeric');
        expect(datefmt.resolvedOptions().dayPeriod).assertEqual('short');
        expect(datefmt.resolvedOptions().formatMatcher).assertEqual('basic');
        expect(datefmt.resolvedOptions().localeMatcher).assertEqual('lookup');
    })

    /* *
    * @tc.number SUB_GLOBAL_INTL_JS_NUMBER_0100
    * @tc.name format the number in zh
    * @tc.desc check the number in zh
    */
    it('formatNumber_test_0100', 0, function () {
        let numfmt = new Intl.NumberFormat('zh');
        console.log('formatNumber_test_0100 ' + numfmt.format(123456.789));
        expect(numfmt.format(123456.789)).assertEqual('123,456.789');
    })

    /* *
    * @tc.number SUB_GLOBAL_INTL_JS_NUMBER_0200
    * @tc.name format the number in zh-u-nu-latn
    * @tc.desc check the number in zh-u-nu-latn
    */
    it('formatNumber_test_0200', 0, function () {
        let numfmt = new Intl.NumberFormat('zh-u-nu-latn');
        console.log('formatNumber_test_0200 ' + numfmt.format(123456.789));
        expect(numfmt.format(123456.789)).assertEqual('123,456.789');
    })

    /* *
    * @tc.number SUB_GLOBAL_INTL_JS_NUMBER_0300
    * @tc.name format the number in zh-u-nu-arab
    * @tc.desc check the number in zh-u-nu-arab
    */
    it('formatNumber_test_0300', 0, function () {
        let numfmt = new Intl.NumberFormat('zh-u-nu-arab');
        console.log('formatNumber_test_0300 ' + numfmt.format(123456.789));
        expect(numfmt.format(123456.789)).assertEqual('١٢٣٬٤٥٦٫٧٨٩');
    })

    /* *
    * @tc.number SUB_GLOBAL_INTL_JS_NUMBER_0400
    * @tc.name format the number in en
    * @tc.desc check the number in en
    */
    it('formatNumber_test_0400', 0, function () {
        let numfmt = new Intl.NumberFormat('en');
        console.log('formatNumber_test_0400 ' + numfmt.format(123456.789));
        expect(numfmt.format(123456.789)).assertEqual('123,456.789');
    })

    /* *
    * @tc.number SUB_GLOBAL_INTL_JS_NUMBER_0500
    * @tc.name format the number in en-u-nu-thai
    * @tc.desc check the number in en-u-nu-thai
    */
    it('formatNumber_test_0500', 0, function () {
        let numfmt = new Intl.NumberFormat('en-u-nu-thai');
        console.log('formatNumber_test_0500 ' + numfmt.format(123456.789));
        expect(numfmt.format(123456.789)).assertEqual('๑๒๓,๔๕๖.๗๘๙');
    })

    /* *
    * @tc.number SUB_GLOBAL_INTL_JS_NUMBER_0600
    * @tc.name format the number in en-GB
    * @tc.desc check the number in en-GB
    */
    it('formatNumber_test_0600', 0, function () {
        let numfmt = new Intl.NumberFormat('en-GB');
        console.log('formatNumber_test_0600 ' + numfmt.format(123456.789));
        expect(numfmt.format(123456.789)).assertEqual('123,456.789');
    })

    /* *
    * @tc.number SUB_GLOBAL_INTL_JS_NUMBER_0700
    * @tc.name format the number in en-US
    * @tc.desc check the number in en-US
    */
    it('formatNumber_test_0700', 0, function () {
        let numfmt = new Intl.NumberFormat('en-US');
        console.log('formatNumber_test_0700 ' + numfmt.format(123456.789));
        expect(numfmt.format(123456.789)).assertEqual('123,456.789');
    })

    /* *
    * @tc.number SUB_GLOBAL_INTL_JS_NUMBER_0800
    * @tc.name format the number in aa
    * @tc.desc check the number in aa
    */
    it('formatNumber_test_0800', 0, function () {
        let numfmt = new Intl.NumberFormat('aa');
        console.log('formatNumber_test_0800 ' + numfmt.format(123456.789));
        expect(numfmt.format(123456.789)).assertEqual('123456.789');
    })

    /* *
    * @tc.number SUB_GLOBAL_INTL_JS_NUMBER_0900
    * @tc.name format the number in zh and en list
    * @tc.desc check the number in zh and en list
    */
    it('formatNumber_test_0900', 0, function () {
        let numfmt = new Intl.NumberFormat(['zh','en']);
        console.log('formatNumber_test_0900 ' + numfmt.format(123456.789));
        expect(numfmt.format(123456.789)).assertEqual('123,456.789');
    })

    /* *
    * @tc.number SUB_GLOBAL_INTL_JS_NUMBER_1000
    * @tc.name format the number in en and zh list
    * @tc.desc check the number in en and zh list
    */
    it('formatNumber_test_1000', 0, function () {
        let numfmt = new Intl.NumberFormat(['en','zh']);
        console.log('formatNumber_test_1000 ' + numfmt.format(123456.789));
        expect(numfmt.format(123456.789)).assertEqual('123,456.789');
    })

    /* *
    * @tc.number SUB_GLOBAL_INTL_JS_NUMBER_1100
    * @tc.name format the number in en and aa list
    * @tc.desc check the number in en and aa list
    */
    it('formatNumber_test_1100', 0, function () {
        let numfmt = new Intl.NumberFormat(['en','aa']);
        console.log('formatNumber_test_1100 ' + numfmt.format(123456.789));
        expect(numfmt.format(123456.789)).assertEqual('123,456.789');
    })

    /* *
    * @tc.number SUB_GLOBAL_INTL_JS_NUMBER_1200
    * @tc.name format the number in aa and zh list
    * @tc.desc check the number in aa and zh list
    */
    it('formatNumber_test_1200', 0, function () {
        let numfmt = new Intl.NumberFormat(['aa','zh']);
        console.log('formatNumber_test_1200 ' + numfmt.format(123456.789));
        expect(numfmt.format(123456.789)).assertEqual('123,456.789');
    })

    /* *
    * @tc.number SUB_GLOBAL_INTL_JS_NUMBER_1300
    * @tc.name format the number in aa and bb list
    * @tc.desc check the number in aa and bb list
    */
    it('formatNumber_test_1300', 0, function () {
        let numfmt = new Intl.NumberFormat(['aa','bb']);
        console.log('formatNumber_test_1300 ' + numfmt.format(123456.789));
        expect(numfmt.format(123456.789)).assertEqual('123456.789');
    })

    /* *
    * @tc.number SUB_GLOBAL_INTL_JS_NUMBER_1400
    * @tc.name format the number in aa bb and es list
    * @tc.desc check the number in aa  bb and es list
    */
    it('formatNumber_test_1400', 0, function () {
        let numfmt = new Intl.NumberFormat(['aa','bb','es']);
        console.log('formatNumber_test_1400 ' + numfmt.format(123456.789));
        expect(numfmt.format(123456.789)).assertEqual('123.456,789');
    })

    /* *
    * @tc.number SUB_GLOBAL_INTL_JS_NUMBER_1500
    * @tc.name format the number in aa it and bb list
    * @tc.desc check the number in aa it and bb list
    */
    it('formatNumber_test_1500', 0, function () {
        let numfmt = new Intl.NumberFormat(['aa','it','bb']);
        console.log('formatNumber_test_1500 ' + numfmt.format(123456.789));
        expect(numfmt.format(123456.789)).assertEqual('123.456,789');
    })

    /* *
    * @tc.number SUB_GLOBAL_INTL_JS_NUMBER_1600
    * @tc.name format the number with scientific
    * @tc.desc check the number with scientific
    */
    it('formatNumber_test_1600', 0, function () {
        let numfmt = new Intl.NumberFormat('en-GB', { style: 'decimal', notation: 'scientific' });
        console.log('formatNumber_test_1600 ' + numfmt.format(123456.789));
        expect(numfmt.format(123456.789)).assertEqual('1.234568E5');
        expect(numfmt.resolvedOptions().style).assertEqual('decimal');
        expect(numfmt.resolvedOptions().notation).assertEqual('scientific');
    })

    /* *
    * @tc.number SUB_GLOBAL_INTL_JS_NUMBER_1700
    * @tc.name format the number with currency EUR
    * @tc.desc check the number with currency EUR
    */
    it('formatNumber_test_1700', 0, function () {
        let numfmt = new Intl.NumberFormat('en-GB', { style: 'currency', currency: 'EUR',
            currencyDisplay: 'name', currencySign: 'accounting', signDisplay: 'always' });
        console.log('formatNumber_test_1700 ' + numfmt.format(123456.789));
        expect(numfmt.format(123456.789)).assertEqual('+123,456.79 euros');
        expect(numfmt.resolvedOptions().locale).assertEqual('en-GB');
        expect(numfmt.resolvedOptions().style).assertEqual('currency');
        expect(numfmt.resolvedOptions().currency).assertEqual('EUR');
        expect(numfmt.resolvedOptions().currencyDisplay).assertEqual('name');
        expect(numfmt.resolvedOptions().currencySign).assertEqual('accounting');
        expect(numfmt.resolvedOptions().signDisplay).assertEqual('always');
    })

    /* *
    * @tc.number SUB_GLOBAL_INTL_JS_NUMBER_1800
    * @tc.name format the number with currency USD
    * @tc.desc check the number with currency USD
    */
    it('formatNumber_test_1800', 0, function () {
        let numfmt = new Intl.NumberFormat('en-US', { style: 'currency', currency: 'USD',
            currencyDisplay: 'code', currencySign: 'accounting', signDisplay: 'always' });
        console.log('formatNumber_test_1800 ' + numfmt.format(123456.789));
        expect(numfmt.format(123456.789)).assertContain('+USD');
        expect(numfmt.format(123456.789)).assertContain('123,456.79');
    })

    /* *
    * @tc.number SUB_GLOBAL_INTL_JS_NUMBER_1900
    * @tc.name format the number with currency CNY
    * @tc.desc check the number with currency CNY
    */
    it('formatNumber_test_1900', 0, function () {
        let numfmt = new Intl.NumberFormat('zh-CN', { style: 'currency', currency: 'CNY',
            currencyDisplay: 'symbol', currencySign: 'accounting', signDisplay: 'always' });
        console.log('formatNumber_test_1900 ' + numfmt.format(123456.789));
        expect(numfmt.format(123456.789)).assertEqual('+¥123,456.79');
    })

    /* *
    * @tc.number SUB_GLOBAL_INTL_JS_NUMBER_2000
    * @tc.name format the number with currency JPY
    * @tc.desc check the number with currency JPY
    */
    it('formatNumber_test_2000', 0, function () {
        let numfmt = new Intl.NumberFormat('ja-JP', { style: 'currency', currency: 'JPY',
            currencyDisplay: 'narrowSymbol', currencySign: 'accounting', signDisplay: 'always' });
        console.log('formatNumber_test_2000 ' + numfmt.format(123456.789));
        expect(numfmt.format(123456.789)).assertEqual('+￥123,457');
    })

    /* *
    * @tc.number SUB_GLOBAL_INTL_JS_NUMBER_2100
    * @tc.name format the number with signDisplay
    * @tc.desc check the number with signDisplay
    */
    it('formatNumber_test_2100', 0, function () {
        let numfmt = new Intl.NumberFormat('en-GB', { style: 'currency', currency: 'EUR',
            currencyDisplay: 'name', currencySign: 'accounting', signDisplay: 'always',
            notation: 'compact', compactDisplay: 'long', localeMatcher: 'lookup' });
        console.log('formatNumber_test_2100 ' + numfmt.format(123456.789));
        expect(numfmt.format(123456.789)).assertEqual('+123 thousand euros');
        expect(numfmt.resolvedOptions().locale).assertEqual('en-GB');
        expect(numfmt.resolvedOptions().style).assertEqual('currency');
        expect(numfmt.resolvedOptions().currency).assertEqual('EUR');
        expect(numfmt.resolvedOptions().currencyDisplay).assertEqual('name');
        expect(numfmt.resolvedOptions().currencySign).assertEqual('accounting');
        expect(numfmt.resolvedOptions().compactDisplay).assertEqual('long');
        expect(numfmt.resolvedOptions().localeMatcher).assertEqual('lookup');
        expect(numfmt.resolvedOptions().signDisplay).assertEqual('always');
    })

    /* *
    * @tc.number SUB_GLOBAL_INTL_JS_NUMBER_2200
    * @tc.name format the number with unit
    * @tc.desc check the number with unit
    */
    it('formatNumber_test_2200', 0, function () {
        let numfmt = new Intl.NumberFormat('zh-CN', { style: 'unit', unit: 'meter',
            unitDisplay: 'long', maximumFractionDigits: 2, minimumIntegerDigits: 7 });
        console.log('formatNumber_test_2200 ' + numfmt.format(123456.789));
        expect(numfmt.format(123456.789)).assertEqual('0,123,456.79米');
        expect(numfmt.resolvedOptions().style).assertEqual('unit');
        expect(numfmt.resolvedOptions().unit).assertEqual('meter');
        expect(numfmt.resolvedOptions().unitDisplay).assertEqual('long');
    })

    /* *
    * @tc.number SUB_GLOBAL_INTL_JS_NUMBER_2300
    * @tc.name format the number with unit
    * @tc.desc check the number with unit
    */
    it('formatNumber_test_2300', 0, function () {
        let numfmt = new Intl.NumberFormat('en-CN', { style: 'percent', maximumFractionDigits: 2,
            minimumIntegerDigits: 7 });
        console.log('formatNumber_test_2300 ' + numfmt.format(123456.789));
        expect(numfmt.format(123456.789)).assertEqual('0,123,456.79%');
    })

    /* *
    * @tc.number SUB_GLOBAL_INTL_JS_NUMBER_2400
    * @tc.name format the number with numberingSystem
    * @tc.desc check the number with numberingSystem
    */
    it('formatNumber_test_2400', 0, function () {
        let numfmt = new Intl.NumberFormat('ar-EG', { numberingSystem: 'arab' });
        console.log('formatNumber_test_2400 ' + numfmt.format(123456.789));
        expect(numfmt.format(123456.789)).assertEqual('١٢٣٬٤٥٦٫٧٨٩');
        expect(numfmt.resolvedOptions().numberingSystem).assertEqual('arab');
    })

    /* *
    * @tc.number SUB_GLOBAL_INTL_JS_NUMBER_2500
    * @tc.name format the number with style
    * @tc.desc check the number with style
    */
    it('formatNumber_test_2500', 0, function () {
        let numfmt = new Intl.NumberFormat('ar', { style: 'percent', minimumFractionDigits: 1, maximumFractionDigits: 2,
            minimumIntegerDigits: 7, minimumSignificantDigits: 5, maximumSignificantDigits: 10, useGrouping: true  });
        console.log('formatNumber_test_2500 ' + numfmt.resolvedOptions().style);
        expect(numfmt.resolvedOptions().style).assertEqual('percent');
        expect(numfmt.resolvedOptions().minimumFractionDigits).assertEqual(1);
        expect(numfmt.resolvedOptions().maximumFractionDigits).assertEqual(2);
        expect(numfmt.resolvedOptions().minimumIntegerDigits).assertEqual(7);
        expect(numfmt.resolvedOptions().minimumSignificantDigits).assertEqual(5);
        expect(numfmt.resolvedOptions().maximumSignificantDigits).assertEqual(10);
        expect(numfmt.resolvedOptions().useGrouping).assertTrue();
    })
    console.log('*************end IntlTest*************');
})