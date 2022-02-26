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

import bundle from '@ohos.bundle'
import { describe, beforeAll, beforeEach, afterEach, afterAll, it, expect } from 'deccjsunit/index'

const TIMEOUT = 1000;

describe('ActsBmsMetaDataTest', function () {

    /*
    * @tc.number: bms_getMetaData_0100
    * @tc.name: test to get meta data for an application.
    * @tc.desc: get an application's meta data which contains two HAP packages.
    */
    it('bms_getMetaData_0100', 0, async function (done) {
        console.info('=====================bms_getMetaData_0100==================');
        await install(['/data/test/bmsThirdBundleTest1.hap'])
        await install(['/data/test/bmsThirdBundleTest3.hap'])
        let abilityName1 = 'com.example.third1.MainAbility';
        let abilityName2 = 'com.example.third3.MainAbility';
        let dataMap1 = new Map();
        let dataMap2 = new Map();
        var dataInfos = await bundle.queryAbilityByWant({
            want: {
                action: 'action.system.home',
                entities: ['entity.system.home'],
                elementName: {
                    deviceId: '0',
                    bundleName: 'com.example.third1',
                    abilityName: abilityName1,
                },
            }
        }, 0, 0)
        for (let i = 0, len = dataInfos.length; i < len; i++) {
            dataMap1.set(dataInfos[i].name, dataInfos[i].metaData)
        }
        expect(dataMap1.has(abilityName1)).assertTrue();
        expect(dataMap1.has(abilityName2)).assertTrue();
        if (dataMap1.has(abilityName1) && dataMap1.has(abilityName2)) {
            checkMetaData(dataMap1.get(abilityName1), 'Data1');
            checkMetaData(dataMap1.get(abilityName2), 'Data3');
        }
        dataInfos = await bundle.queryAbilityByWant({
            want: {
                action: 'action.system.home',
                entities: ['entity.system.home'],
                elementName: {
                    deviceId: '0',
                    bundleName: 'com.example.third1',
                    abilityName: abilityName2,
                },
            }
        }, 0, 0)
        for (let i = 0, len = dataInfos.length; i < len; i++) {
            dataMap2.set(dataInfos[i].name, dataInfos[i].metaData)
        }
        expect(dataMap2.has(abilityName1)).assertTrue();
        expect(dataMap2.has(abilityName2)).assertTrue();
        if (dataMap2.has(abilityName1) && dataMap2.has(abilityName2)) {
            checkMetaData(dataMap2.get(abilityName1), 'Data1');
            checkMetaData(dataMap2.get(abilityName2), 'Data3');
        }
        done();
        setTimeout(function () {
            console.debug('============bms_getMetaData_0100===========')
        }, TIMEOUT);
    })

    /*
    * @tc.number: bms_getMetaData_0200
    * @tc.name: test to get meta data for an update application.
    * @tc.desc: get an application's meta data that is updated.
    */
    it('bms_getMetaData_0200', 0, async function (done) {
        console.info('=====================bms_getMetaData_0200==================');
        await install(['/data/test/bmsThirdBundleTestA1.hap'])
        let dataMap = new Map();
        let abilityName1 = 'com.example.third1.AMainAbility';
        let abilityName2 = 'com.example.third3.MainAbility';
        var dataInfos = await bundle.queryAbilityByWant({
            want: {
                action: 'action.system.home',
                entities: ['entity.system.home'],
                elementName: {
                    deviceId: '0',
                    bundleName: 'com.example.third1',
                    abilityName: 'com.example.third1.AMainAbility',
                },
            }
        }, 0, 0)
        for (let i = 0, len = dataInfos.length; i < len; i++) {
            dataMap.set(dataInfos[i].name, dataInfos[i].metaData)
        }
        expect(dataMap.has(abilityName1)).assertTrue();
        expect(dataMap.has(abilityName2)).assertTrue();
        if (dataMap.has(abilityName1) && dataMap.has(abilityName2)) {
            checkMetaData(dataMap.get(abilityName1), 'DataA1');
            checkMetaData(dataMap.get(abilityName2), 'Data3');
        }
        done();
        setTimeout(function () {
            console.debug('============bms_getMetaData_0200===========')
        }, TIMEOUT);
    })

    /*
    * @tc.number: bms_getMetaData_0300
    * @tc.name: test to get meta data for an uninstalled application.
    * @tc.desc: get an application's meta data that is uninstalled.
    */
    it('bms_getMetaData_0300', 0, async function (done) {
        console.info('=====================bms_getMetaData_0300==================');
        await uninstall('com.example.third1');
        var dataInfos = await bundle.queryAbilityByWant({
            want: {
                action: 'action.system.home',
                entities: ['entity.system.home'],
                elementName: {
                    deviceId: '0',
                    bundleName: 'com.example.third1',
                    abilityName: 'com.example.third1.MainAbility',
                },
            }
        }, 0, 0)
        console.info('==========abilityInfo is ==========' + dataInfos);
        console.info('==========abilityInfo is ==========' + JSON.stringify(dataInfos));
        expect(dataInfos.length).assertEqual(0);
        done();
        setTimeout(function () {
            console.debug('============bms_getMetaData_0300===========')
        }, TIMEOUT);
    })

    /*
    * @tc.number: bms_getMetaData_0400
    * @tc.name: test to get meta data for an application.
    * @tc.desc: get an application's meta data which has two abilities.
    */
    it('bms_getMetaData_0400', 0, async function (done) {
        console.info('=====================bms_getMetaData_0400==================');
        await install(['/data/test/bmsThirdBundleTest5.hap']);
        let dataMap = new Map();
        let abilityName1 = 'com.example.third5.AMainAbility';
        let abilityName2 = 'com.example.third5.BMainAbility';
        var dataInfos = await bundle.queryAbilityByWant({
            want: {
                action: 'action.system.home',
                entities: ['entity.system.home'],
                elementName: {
                    deviceId: '0',
                    bundleName: 'com.example.third5',
                    abilityName: 'com.example.third5.AMainAbility',
                },
            }
        }, 0, 0)
        for (let i = 0, len = dataInfos.length; i < len; i++) {
            dataMap.set(dataInfos[i].name, dataInfos[i].metaData)
        }
        expect(dataMap.has(abilityName1)).assertTrue();
        expect(dataMap.has(abilityName2)).assertTrue();
        if (dataMap.has(abilityName1) && dataMap.has(abilityName2)) {
            checkMetaData(dataMap.get(abilityName1), 'Data5A');
            checkMetaData(dataMap.get(abilityName2), 'Data5B');
        }
        await uninstall('com.example.third5');
        done();
        setTimeout(function () {
            console.debug('============bms_getMetaData_0400===========')
        }, TIMEOUT);
    })

    /*
    * @tc.number: bms_getMetaData_0500
    * @tc.name: test to get meta data for an application that does not exist.
    * @tc.desc: get an application's meta data which does not exist.
    */
    it('bms_getMetaData_0500', 0, async function (done) {
        console.info('=====================bms_getMetaData_0500==================');
        var dataInfos = await bundle.queryAbilityByWant({
            want: {
                action: 'action.system.home',
                entities: ['entity.system.home'],
                elementName: {
                    deviceId: '0',
                    bundleName: 'com.example.noexist',
                    abilityName: 'com.example.noexist.MainAbility',
                },
            }
        }, 0, 0)
        console.info('==========abilityInfo is ==========' + dataInfos);
        console.info('==========abilityInfo is ==========' + JSON.stringify(dataInfos));
        expect(dataInfos.length).assertEqual(0);
        done();
        setTimeout(function () {
            console.debug('============bms_getMetaData_0500===========')
        }, TIMEOUT);
    })

    /*
    * @tc.number: bms_getMetaData_0600
    * @tc.name: test to get meta data for a system application.
    * @tc.desc: get a system application's meta data.
    */
    it('bms_getMetaData_0600', 0, async function (done) {
        console.info('=====================bms_getMetaData_0600==================');
        let dataMap = new Map();
        let abilityName1 = 'com.example.system1.MainAbility';
        var dataInfos = await bundle.queryAbilityByWant({
            want: {
                action: 'action.system.home',
                entities: ['entity.system.home'],
                elementName: {
                    deviceId: '0',
                    bundleName: 'com.example.system1',
                    abilityName: 'com.example.system1.MainAbility',
                },
            }
        }, 0, 0)
        for (let i = 0, len = dataInfos.length; i < len; i++) {
            dataMap.set(dataInfos[i].name, dataInfos[i].metaData)
        }
        expect(dataMap.has(abilityName1)).assertTrue();
        if (dataMap.has(abilityName1)) {
            checkMetaData(dataMap.get(abilityName1), 'Data1S');
        }
        done();
        setTimeout(function () {
            console.debug('============bms_getMetaData_0600===========')
        }, TIMEOUT);
    })

    /*
    * @tc.number: bms_getMetaData_0700
    * @tc.name: test to get meta data for a vendor application.
    * @tc.desc: get a vendor application's meta data.
    */
    it('bms_getMetaData_0700', 0, async function (done) {
        console.info('=====================bms_getMetaData_0700==================');
        let dataMap = new Map();
        let abilityName1 = 'com.example.vendor1.MainAbility';
        var dataInfos = await bundle.queryAbilityByWant({
            want: {
                action: 'action.system.home',
                entities: ['entity.system.home'],
                elementName: {
                    deviceId: '0',
                    bundleName: 'com.example.vendor1',
                    abilityName: 'com.example.vendor1.MainAbility',
                },
            }
        }, 0, 0)
        for (let i = 0, len = dataInfos.length; i < len; i++) {
            dataMap.set(dataInfos[i].name, dataInfos[i].metaData)
        }
        expect(dataMap.has(abilityName1)).assertTrue();
        if (dataMap.has(abilityName1)) {
            checkMetaData(dataMap.get(abilityName1), 'Data1V');
        }
        done();
        setTimeout(function () {
            console.debug('============bms_getMetaData_0700===========')
        }, TIMEOUT);
    })

    function checkMetaData(data, name) {
        console.info('==========MetaData is==========' + JSON.stringify(data));
        var parameters = data.parameters;
        var results = data.results;
        var customizeDatas = data.customizeDatas;
        expect(typeof parameters).assertEqual('object');
        expect(typeof results).assertEqual('object');
        expect(typeof customizeDatas).assertEqual('object');
        console.info('==========parameters.length is==========' + parameters.length);
        console.info('==========results.length is==========' + results.length);
        console.info('==========customizeDatas.length is==========' + customizeDatas.length);
        expect(parameters.length).assertLarger(0);
        expect(results.length).assertLarger(0);
        expect(customizeDatas.length).assertLarger(0);
        for (let i = 0; i < parameters.length; i++) {
            console.info('==========Parameter description is==========' + parameters[i].description);
            expect(typeof parameters[i].description).assertEqual('string');
            console.info('==========Parameter name is==========' + parameters[i].name);
            expect(typeof parameters[i].name).assertEqual('string');
            expect(parameters[i].name).assertEqual(name);
            console.info('==========Parameter type is==========' + parameters[i].type);
            expect(typeof parameters[i].type).assertEqual('string');
        }
        for (let i = 0; i < results.length; i++) {
            console.info('==========Results description is==========' + results[i].description);
            expect(typeof results[i].description).assertEqual('string');
            console.info('==========Results name is==========' + results[i].name);
            expect(typeof results[i].name).assertEqual('string');
            expect(results[i].name).assertEqual(name);
            console.info('==========Results type is==========' + results[i].type);
            expect(typeof results[i].type).assertEqual('string');
        }
        for (let i = 0; i < customizeDatas.length; i++) {
            console.info('==========CustomizeData name is==========' + customizeDatas[i].name);
            expect(typeof customizeDatas[i].name).assertEqual('string');
            console.info('==========CustomizeData value is==========' + customizeDatas[i].value);
            expect(typeof customizeDatas[i].value).assertEqual('string');
            expect(customizeDatas[i].name).assertEqual(name);
            console.info('==========CustomizeData extra is==========' + customizeDatas[i].extra);
            expect(typeof customizeDatas[i].extra).assertEqual('string');
        }
    }
    async function install(bundlePath) {
        var installer = await bundle.getBundleInstaller();
        installer.install(bundlePath, {
            param: {
                userId: 0,
                installFlag: 1,
                isKeepData: false
            }
        }, onReceiveinstallEvent);
        function onReceiveinstallEvent(err, data) {
            console.info('========install Finish========');
            expect(typeof err).assertEqual('object');
            expect(err.code).assertEqual(0);
            expect(typeof data).assertEqual('object');
            expect(data.status).assertEqual(0);
            expect(data.statusMessage).assertEqual('SUCCESS');
        }
    }

    async function uninstall(bundleName) {
        var installer = await bundle.getBundleInstaller();
        installer.uninstall(bundleName, {
            param: {
                userId: 0,
                installFlag: 1,
                isKeepData: false
            }
        }, onReceiveinstallEvent);
        function onReceiveinstallEvent(err, data) {
            console.info('========uninstall Finish========');
            expect(typeof err).assertEqual('object');
            expect(err.code).assertEqual(0);
            expect(typeof data).assertEqual('object');
            expect(data.status).assertEqual(0);
            expect(data.statusMessage).assertEqual('SUCCESS');
        }
    }
})