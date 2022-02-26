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
import I18n from '@ohos.i18n'
import {describe, beforeAll, beforeEach, afterEach, afterAll, it, expect} from 'deccjsunit/index'

describe('I18nTest', function () {
    console.log('*************start I18NTest*************');

    /* *
    * @tc.number SUB_GLOBAL_I18N_JS_0100
    * @tc.name getSystemLanguage
    * @tc.desc check the getSystemLanguage
    */
    it('i18n_test_0100', 0, function () {
        let lang = I18n.getSystemLanguage();
        console.log('i18n_test_0100 ' + lang);
        expect(lang).assertInstanceOf('String');
    })

    /* *
    * @tc.number SUB_GLOBAL_I18N_JS_0200
    * @tc.name getSystemRegion
    * @tc.desc check the getSystemRegion
    */
    it('i18n_test_0200', 0, function () {
        let region = I18n.getSystemRegion();
        console.log('i18n_test_0200 ' + region);
        expect(region).assertInstanceOf('String');
    })

    /* *
    * @tc.number SUB_GLOBAL_I18N_JS_0300
    * @tc.name getSystemLocale
    * @tc.desc check the getSystemLocale
    */
    it('i18n_test_0300', 0, function () {
        let locale = I18n.getSystemLocale();
        console.log('i18n_test_0300 ' + locale);
        expect(locale).assertInstanceOf('String');
    })

    /* *
    * @tc.number SUB_GLOBAL_I18N_JS_0400
    * @tc.name setSystemLanguage with en param
    * @tc.desc check the language
    */
    it('i18n_test_0400', 0, function () {
        let value = I18n.setSystemLanguage('en');
        console.log('i18n_test_0400 ' + value);
        expect(value).assertTrue();
        let lang = I18n.getSystemLanguage();
        console.log('i18n_test_0400 ' + lang);
        expect(lang).assertEqual('en');
    })

    /* *
    * @tc.number SUB_GLOBAL_I18N_JS_0500
    * @tc.name setSystemLanguage with ko-KR param
    * @tc.desc check the language
    */
    it('i18n_test_0500', 0, function () {
        let value = I18n.setSystemLanguage('ko-KR');
        console.log('i18n_test_0500 ' + value);
        expect(value).assertTrue();
        let lang = I18n.getSystemLanguage();
        console.log('i18n_test_0500 ' + lang);
        expect(lang).assertEqual('ko-KR');
    })

    /* *
    * @tc.number SUB_GLOBAL_I18N_JS_0600
    * @tc.name setSystemLanguage with it-Ital-IT param
    * @tc.desc check the language
    */
    it('i18n_test_0600', 0, function () {
        let value = I18n.setSystemLanguage('it-Ital-IT');
        console.log('i18n_test_0600 ' + value);
        expect(value).assertTrue();
        let lang = I18n.getSystemLanguage();
        console.log('i18n_test_0600 ' + lang);
        expect(lang).assertEqual('it-Ital-IT');
    })

    /* *
    * @tc.number SUB_GLOBAL_I18N_JS_0700
    * @tc.name setSystemLanguage with null param
    * @tc.desc check the language
    */
    it('i18n_test_0700', 0, function () {
        let value = I18n.setSystemLanguage('');
        console.log('i18n_test_0700 ' + value);
        expect(value).assertFalse();
        let lang = I18n.getSystemLanguage();
        console.log('i18n_test_0700 ' + lang);
        expect(lang).assertEqual('it-Ital-IT');
    })

    /* *
    * @tc.number SUB_GLOBAL_I18N_JS_0800
    * @tc.name setSystemLanguage with not exist param aa
    * @tc.desc check the language
    */
    it('i18n_test_0800', 0, function () {
        let value = I18n.setSystemLanguage('aa');
        console.log('i18n_test_0800 ' + value);
        expect(value).assertTrue();
        let lang = I18n.getSystemLanguage();
        console.log('i18n_test_0800 ' + lang);
        expect(lang).assertEqual('aa');
    })

    /* *
    * @tc.number SUB_GLOBAL_I18N_JS_0900
    * @tc.name setSystemLanguage with not exist param aabb
    * @tc.desc check the language
    */
    it('i18n_test_0900', 0, function () {
        let value = I18n.setSystemLanguage('aabb');
        console.log('i18n_test_0900 ' + value);
        expect(value).assertFalse();
        let lang = I18n.getSystemLanguage();
        console.log('i18n_test_0900 ' + lang);
        expect(lang).assertEqual('aa');
    })

    /* *
    * @tc.number SUB_GLOBAL_I18N_JS_1000
    * @tc.name setSystemLanguage with es-aaaa param
    * @tc.desc check the language
    */
    it('i18n_test_1000', 0, function () {
        let value = I18n.setSystemLanguage('es-aaaa');
        console.log('i18n_test_1000 ' + value);
        expect(value).assertTrue();
        let lang = I18n.getSystemLanguage();
        console.log('i18n_test_1000 ' + lang);
        expect(lang).assertEqual('es-aaaa');
    })

    /* *
    * @tc.number SUB_GLOBAL_I18N_JS_1100
    * @tc.name setSystemLanguage with it-Ital-AA param
    * @tc.desc check the language
    */
    it('i18n_test_1100', 0, function () {
        let value = I18n.setSystemLanguage('it-Ital-AA');
        console.log('i18n_test_1100 ' + value);
        expect(value).assertTrue();
        let lang = I18n.getSystemLanguage();
        console.log('i18n_test_1100 ' + lang);
        expect(lang).assertEqual('it-Ital-AA');
    })

    /* *
    * @tc.number SUB_GLOBAL_I18N_JS_1200
    * @tc.name setSystemRegion with US param
    * @tc.desc check the region
    */
    it('i18n_test_1200', 0, function () {
        let value = I18n.setSystemRegion('US');
        console.log('i18n_test_1200 ' + value);
        expect(value).assertTrue();
        let region = I18n.getSystemRegion();
        console.log('i18n_test_1200 ' + region);
        expect(region).assertEqual('US');
    })

    /* *
    * @tc.number SUB_GLOBAL_I18N_JS_1300
    * @tc.name setSystemRegion with null param
    * @tc.desc check the region
    */
    it('i18n_test_1300', 0, function () {
        let value = I18n.setSystemRegion('');
        console.log('i18n_test_1300 ' + value);
        expect(value).assertFalse();
        let region = I18n.getSystemRegion();
        console.log('i18n_test_1300 ' + region);
        expect(region).assertEqual('US');
    })

    /* *
    * @tc.number SUB_GLOBAL_I18N_JS_1400
    * @tc.name setSystemRegion with AA param
    * @tc.desc check the region
    */
    it('i18n_test_1400', 0, function () {
        let value = I18n.setSystemRegion('AA');
        console.log('i18n_test_1400 ' + value);
        expect(value).assertTrue();
    })

    /* *
    * @tc.number SUB_GLOBAL_I18N_JS_1500
    * @tc.name setSystemRegion with zh-CN param
    * @tc.desc check the reigon
    */
    it('i18n_test_1500', 0, function () {
        let value = I18n.setSystemRegion('zh-CN');
        console.log('i18n_test_1500 ' + value);
        expect(value).assertFalse();
    })

    /* *
    * @tc.number SUB_GLOBAL_I18N_JS_1600
    * @tc.name setSystemLocale with en-Latn-US param
    * @tc.desc check the locale
    */
    it('i18n_test_1600', 0, function () {
        let value = I18n.setSystemLocale('en-Latn-US');
        console.log('i18n_test_1600 ' + value);
        expect(value).assertTrue();
        let locale = I18n.getSystemLocale();
        console.log('i18n_test_1600 ' + locale);
        expect(locale).assertEqual('en-Latn-US');
    })

    /* *
    * @tc.number SUB_GLOBAL_I18N_JS_1700
    * @tc.name setSystemLocale with null param
    * @tc.desc check the locale
    */
    it('i18n_test_1700', 0, function () {
        let value = I18n.setSystemLocale('');
        console.log('i18n_test_1700 ' + value);
        expect(value).assertFalse();
    })

    /* *
    * @tc.number SUB_GLOBAL_I18N_JS_1800
    * @tc.name setSystemLocale with aa param
    * @tc.desc check the locale
    */
    it('i18n_test_1800', 0, function () {
        let value = I18n.setSystemLocale('aa');
        console.log('i18n_test_1800 ' + value);
        expect(value).assertTrue();
    })

    /* *
    * @tc.number SUB_GLOBAL_I18N_JS_1900
    * @tc.name setSystemLocale with zh-AAAA param
    * @tc.desc check the locale
    */
    it('i18n_test_1900', 0, function () {
        let value = I18n.setSystemLocale('zh-AAAA');
        console.log('i18n_test_1900 ' + value);
        expect(value).assertTrue();
    })

    /* *
    * @tc.number SUB_GLOBAL_I18N_JS_2000
    * @tc.name setSystemLocale with zh-Hans-AA param
    * @tc.desc check the locale
    */
    it('i18n_test_2000', 0, function () {
        let value = I18n.setSystemLocale('zh-Hans-AA');
        console.log('i18n_test_2000 ' + value);
        expect(value).assertTrue();
    })

    /* *
    * @tc.number SUB_GLOBAL_I18N_JS_2100
    * @tc.name getSystemLanguages
    * @tc.desc check the language
    */
    it('i18n_test_2100', 0, function () {
        let lang = I18n.getSystemLanguages();
        let len = lang.length;
        console.log('i18n_test_2100 ' + len);
        expect(len).assertLarger(0);
    })

    /* *
    * @tc.number SUB_GLOBAL_I18N_JS_2200
    * @tc.name getSystemCountries with en param
    * @tc.desc check the country
    */
    it('i18n_test_2200', 0, function () {
        let count = I18n.getSystemCountries('en');
        let len = count.length;
        console.log('i18n_test_2200 ' + len);
        expect(len).assertLarger(0);
    })

    /* *
    * @tc.number SUB_GLOBAL_I18N_JS_2220
    * @tc.name getSystemCountries with zh-CN param
    * @tc.desc check the country
    */
    it('i18n_test_2220', 0, function () {
        let count = I18n.getSystemCountries('zh-CN');
        let len = count.length;
        console.log('i18n_test_2220 ' + len);
        expect(len).assertLarger(0);
    })

    /* *
    * @tc.number SUB_GLOBAL_I18N_JS_2240
    * @tc.name getSystemCountries with zh-Hans-CN param
    * @tc.desc check the country
    */
    it('i18n_test_2240', 0, function () {
        let count = I18n.getSystemCountries('zh-Hans-CN');
        let len = count.length;
        console.log('i18n_test_2240 ' + len);
        expect(len).assertLarger(0);
    })

    /* *
    * @tc.number SUB_GLOBAL_I18N_JS_2260
    * @tc.name getSystemCountries with null param
    * @tc.desc check the country
    */
    it('i18n_test_2260', 0, function () {
        let count = I18n.getSystemCountries('');
        let len = count.length;
        console.log('i18n_test_2260 ' + len);
        expect(len).assertLarger(0);
    })

    /* *
    * @tc.number SUB_GLOBAL_I18N_JS_2280
    * @tc.name getSystemCountries with aaa param
    * @tc.desc check the country
    */
    it('i18n_test_2280', 0, function () {
        let count = I18n.getSystemCountries('aaa');
        let len = count.length;
        console.log('i18n_test_2280 ' + len);
        expect(len).assertLarger(0);
    })

    /* *
    * @tc.number SUB_GLOBAL_I18N_JS_2300
    * @tc.name isSuggested with en param
    * @tc.desc check the language
    */
    it('i18n_test_2300', 0, function () {
        let region = I18n.setSystemRegion('US');
        console.log('i18n_test_2300 ' + region);
        expect(region).assertTrue();
        let value = I18n.isSuggested('en');
        console.log('i18n_test_2300 ' + value);
        expect(value).assertTrue();
    })

    /* *
    * @tc.number SUB_GLOBAL_I18N_JS_2400
    * @tc.name isSuggested with zh-Hans param
    * @tc.desc check the language
    */
    it('i18n_test_2400', 0, function () {
        let region = I18n.setSystemRegion('CN');
        console.log('i18n_test_2400 ' + region);
        expect(region).assertTrue();
        let value = I18n.isSuggested('zh-Hans');
        console.log('i18n_test_2400 ' + value);
        expect(value).assertTrue();
    })

    /* *
    * @tc.number SUB_GLOBAL_I18N_JS_2500
    * @tc.name isSuggested with zh-Hans-CN param
    * @tc.desc check the language
    */
    it('i18n_test_2500', 0, function () {
        let region = I18n.setSystemRegion('CN');
        console.log('i18n_test_2500 ' + region);
        expect(region).assertTrue();
        let value = I18n.isSuggested('zh-Hans-CN');
        console.log('i18n_test_2500 ' + value);
        expect(value).assertTrue();
    })

    /* *
    * @tc.number SUB_GLOBAL_I18N_JS_2600
    * @tc.name isSuggested with fr param
    * @tc.desc check the language
    */
    it('i18n_test_2600', 0, function () {
        let value = I18n.isSuggested('fr');
        console.log('i18n_test_2600 ' + value);
        expect(value).assertFalse();
    })

    /* *
    * @tc.number SUB_GLOBAL_I18N_JS_2700
    * @tc.name isSuggested with fr-FR param
    * @tc.desc check the language
    */
    it('i18n_test_2700', 0, function () {
        let value = I18n.isSuggested('fr-FR');
        console.log('i18n_test_2700 ' + value);
        expect(value).assertFalse();
    })

    /* *
    * @tc.number SUB_GLOBAL_I18N_JS_2800
    * @tc.name isSuggested with az-Latn-AZ param
    * @tc.desc check the language
    */
    it('i18n_test_2800', 0, function () {
        let value = I18n.isSuggested('az-Latn-AZ');
        console.log('i18n_test_2800 ' + value);
        expect(value).assertFalse();
    })

    /* *
    * @tc.number SUB_GLOBAL_I18N_JS_2900
    * @tc.name isSuggested with en-ZH param
    * @tc.desc check the language
    */
    it('i18n_test_2900', 0, function () {
        let value = I18n.isSuggested('en-ZH');
        console.log('i18n_test_2900 ' + value);
        expect(value).assertFalse();
    })

    /* *
    * @tc.number SUB_GLOBAL_I18N_JS_3000
    * @tc.name isSuggested with aa param
    * @tc.desc check the language
    */
    it('i18n_test_3000', 0, function () {
        let value = I18n.isSuggested('aa');
        console.log('i18n_test_3000 ' + value);
        expect(value).assertFalse();
    })

    /* *
    * @tc.number SUB_GLOBAL_I18N_JS_3100
    * @tc.name isSuggested with en and GB param
    * @tc.desc check the language
    */
    it('i18n_test_3100', 0, function () {
        let region = I18n.setSystemRegion('GB');
        console.log('i18n_test_3100 ' + region);
        expect(region).assertTrue();
        let value = I18n.isSuggested('en', 'GB');
        console.log('i18n_test_3100 ' + value);
        expect(value).assertFalse();
    })

    /* *
    * @tc.number SUB_GLOBAL_I18N_JS_3150
    * @tc.name isSuggested with en-SG and GB param
    * @tc.desc check the language
    */
    it('i18n_test_3150', 0, function () {
        let region = I18n.setSystemRegion('GB');
        console.log('i18n_test_3150 ' + region);
        expect(region).assertTrue();
        let value = I18n.isSuggested('en-SG', 'GB');
        console.log('i18n_test_3150 ' + value);
        expect(value).assertTrue();
    })

    /* *
    * @tc.number SUB_GLOBAL_I18N_JS_3200
    * @tc.name isSuggested with en and ZH param
    * @tc.desc check the language
    */
    it('i18n_test_3200', 0, function () {
        let value = I18n.isSuggested('en', 'ZH');
        console.log('i18n_test_3200 ' + value);
        expect(value).assertFalse();
    })

    /* *
    * @tc.number SUB_GLOBAL_I18N_JS_3300
    * @tc.name isSuggested with en-GB and ZH param
    * @tc.desc check the language
    */
    it('i18n_test_3300', 0, function () {
        let value = I18n.isSuggested('en-GB', 'ZH');
        console.log('i18n_test_3300 ' + value);
        expect(value).assertFalse();
    })

    /* *
    * @tc.number SUB_GLOBAL_I18N_JS_3400
    * @tc.name isSuggested with en-ZH and GB param
    * @tc.desc check the language
    */
    it('i18n_test_3400', 0, function () {
        let value = I18n.isSuggested('en-ZH', 'GB');
        console.log('i18n_test_3400 ' + value);
        expect(value).assertTrue();
    })

    /* *
    * @tc.number SUB_GLOBAL_I18N_JS_3500
    * @tc.name isSuggested with en-JP and GB param
    * @tc.desc check the language
    */
    it('i18n_test_3500', 0, function () {
        let value = I18n.isSuggested('en-JP', 'GB');
        console.log('i18n_test_3500 ' + value);
        expect(value).assertTrue();
    })

    /* *
    * @tc.number SUB_GLOBAL_I18N_JS_3600
    * @tc.name isSuggested with zh-Hans-TW and CN param
    * @tc.desc check the language
    */
    it('i18n_test_3600', 0, function () {
        let value = I18n.isSuggested('zh-Hans-TW', 'CN');
        console.log('i18n_test_3600 ' + value);
        expect(value).assertTrue();
    })

    /* *
    * @tc.number SUB_GLOBAL_I18N_JS_3700
    * @tc.name isSuggested with zh-Hans-CN and HK param
    * @tc.desc check the language
    */
    it('i18n_test_3700', 0, function () {
        let value = I18n.isSuggested('zh-Hans-CN', 'HK');
        console.log('i18n_test_3700 ' + value);
        expect(value).assertTrue();
    })

    /* *
    * @tc.number SUB_GLOBAL_I18N_JS_3800
    * @tc.name getDisplayCountry with zh-Hans-CN and en-US and true param
    * @tc.desc check the display country
    */
    it('i18n_test_3800', 0, function () {
        let value = I18n.getDisplayCountry('zh-Hans-CN', 'en-US', true);
        console.log('i18n_test_3800 ' + value);
        expect(value).assertEqual('China');
    })

    /* *
    * @tc.number SUB_GLOBAL_I18N_JS_3900
    * @tc.name getDisplayCountry with zh and en-US and true param
    * @tc.desc check the display country
    */
    it('i18n_test_3900', 0, function () {
        let value = I18n.getDisplayCountry('zh', 'en-US', true);
        console.log('i18n_test_3900 ' + value);
        expect(value).assertEqual('China');
    })

    /* *
    * @tc.number SUB_GLOBAL_I18N_JS_4000
    * @tc.name getDisplayCountry with zh-CN and en-US and true param
    * @tc.desc check the display country
    */
    it('i18n_test_4000', 0, function () {
        let value = I18n.getDisplayCountry('zh-CN', 'en-US', true);
        console.log('i18n_test_4000 ' + value);
        expect(value).assertEqual('China');
    })

    /* *
    * @tc.number SUB_GLOBAL_I18N_JS_4100
    * @tc.name getDisplayCountry with zh-Hans and en-US and true param
    * @tc.desc check the display country
    */
    it('i18n_test_4100', 0, function () {
        let value = I18n.getDisplayCountry('zh-Hans', 'en-US', true);
        console.log('i18n_test_4100 ' + value);
        expect(value).assertEqual('China');
    })

    /* *
    * @tc.number SUB_GLOBAL_I18N_JS_4200
    * @tc.name getDisplayCountry with zh-Hans-CN and en-US and false param
    * @tc.desc check the language
    */
    it('i18n_test_4200', 0, function () {
        let value = I18n.getDisplayCountry('zh-Hans-CN', 'en-US', false);
        console.log('i18n_test_4200 ' + value);
        expect(value).assertEqual('China');
    })

    /* *
    * @tc.number SUB_GLOBAL_I18N_JS_4300
    * @tc.name getDisplayCountry with zh-Hans-CN and en-US param
    * @tc.desc check the language
    */
    it('i18n_test_4300', 0, function () {
        let value = I18n.getDisplayCountry('zh-Hans-CN', 'en-US');
        console.log('i18n_test_4300 ' + value);
        expect(value).assertEqual('China');
    })

    /* *
    * @tc.number SUB_GLOBAL_I18N_JS_4400
    * @tc.name getDisplayLanguage with zh-Hans-CN and en-US and true param
    * @tc.desc check the language
    */
    it('i18n_test_4400', 0, function () {
        let value = I18n.getDisplayLanguage('zh-Hans-CN', 'en-US', true);
        console.log('i18n_test_4400 ' + value);
        expect(value).assertEqual('Chinese');
    })

    /* *
    * @tc.number SUB_GLOBAL_I18N_JS_4500
    * @tc.name getDisplayLanguage with zh-Hans-CN and en-GB and true param
    * @tc.desc check the language
    */
    it('i18n_test_4500', 0, function () {
        let value = I18n.getDisplayLanguage('zh-Hans-CN', 'en-GB', true);
        console.log('i18n_test_4500 ' + value);
        expect(value).assertEqual('Chinese');
    })

    /* *
    * @tc.number SUB_GLOBAL_I18N_JS_4600
    * @tc.name getDisplayLanguage with zh and en-US and true param
    * @tc.desc check the language
    */
    it('i18n_test_4600', 0, function () {
        let value = I18n.getDisplayLanguage('zh', 'en-US', true);
        console.log('i18n_test_4600 ' + value);
        expect(value).assertEqual('Chinese');
    })

    /* *
    * @tc.number SUB_GLOBAL_I18N_JS_4700
    * @tc.name getDisplayLanguage with zh-CN and en-US and true param
    * @tc.desc check the language
    */
    it('i18n_test_4700', 0, function () {
        let value = I18n.getDisplayLanguage('zh-CN', 'en-US', true);
        console.log('i18n_test_4700 ' + value);
        expect(value).assertEqual('Chinese');
    })

    /* *
    * @tc.number SUB_GLOBAL_I18N_JS_4800
    * @tc.name getDisplayLanguage with zh-Hans and en-US and true param
    * @tc.desc check the language
    */
    it('i18n_test_4800', 0, function () {
        let value = I18n.getDisplayLanguage('zh-Hans', 'en-US', true);
        console.log('i18n_test_4800 ' + value);
        expect(value).assertEqual('Chinese');
    })

    /* *
    * @tc.number SUB_GLOBAL_I18N_JS_4900
    * @tc.name getDisplayLanguage with zh-Hans-CN and en-US and false param
    * @tc.desc check the language
    */
    it('i18n_test_4900', 0, function () {
        let value = I18n.getDisplayLanguage('zh-Hans-CN', 'en-US', false);
        console.log('i18n_test_4900 ' + value);
        expect(value).assertEqual('Chinese');
    })

    /* *
    * @tc.number SUB_GLOBAL_I18N_JS_5000
    * @tc.name getDisplayLanguage with zh-Hans-CN and en-US param
    * @tc.desc check the language
    */
    it('i18n_test_5000', 0, function () {
        let value = I18n.getDisplayLanguage('zh-Hans-CN', 'en-US');
        console.log('i18n_test_5000 ' + value);
        expect(value).assertEqual('Chinese');
    })

    console.log('*************end I18NTest*************');
})