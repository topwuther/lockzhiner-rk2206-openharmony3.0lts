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

describe('ActsBmsHapModuleTest', function () {

    /*
     * @tc.number: bms_getHapModuleInfo_0100
     * @tc.name: get hapModuleInfo from one app by getBundleInfo
     * @tc.desc: get the module information of the hap with type of entry
     */
    it('bms_getHapModuleInfo_0100', 0, async function (done) {
        console.debug('===========begin bms_getHapModuleInfo_0100===========')
        await install(['/data/test/bmsMainAbilityFirstScene.hap']);
        let bundleName = 'com.example.bmsmainabilityfirstscene';
        let ret = false
        bundle.getBundleInfo(bundleName, 1, callback);
        function callback(err, data) {
            console.debug('=======get bundle========' + JSON.stringify(data));
            let hapModuleInfo = data.hapModuleInfo[0];
            console.debug('=======get hapModule========' + JSON.stringify(hapModuleInfo))
            console.debug('=======get hapModule mainAbilityName========' + hapModuleInfo.mainAbilityName)
            expect(hapModuleInfo.moduleName).assertEqual('entry');
            expect(hapModuleInfo.mainAbilityName).assertEqual('com.example.bmsmainabilityfirstscene.MainAbility');
            checkHapModuleInfo(hapModuleInfo);
            ret = true;
            done();
        }
        setTimeout(function () {
            expect(ret).assertTrue();
        }, TIMEOUT);
    });

    /*
     * @tc.number: bms_getHapModuleInfo_0200
     * @tc.name: get hapModuleInfo from two modules by getBundleInfo
     * @tc.desc: get the module information of the hap with type of feature
     */
    it('bms_getHapModuleInfo_0200', 0, async function (done) {
        console.debug('===========begin bms_getHapModuleInfo_0200===========')
        await install(['/data/test/bmsMainAbilitySecondScene.hap']);
        let ret = false
        let bundleName = 'com.example.bmsmainabilityfirstscene';
        let firstMainAbility = 'com.example.bmsmainabilityfirstscene.MainAbility';
        let secondMainAbility = 'com.example.bmsmainabilitysecondscene.MainAbility';
        let result = new Map();
        bundle.getBundleInfo(bundleName, 1, async (err, data) => {
            console.debug('=======hapModule length========' + data.hapModuleInfo.length);
            expect(data.hapModuleInfo.length).assertEqual(2);
            for (let i = 0, len = data.hapModuleInfo.length; i < len; i++) {
                console.debug('=======get hapModule========' + JSON.stringify(data.hapModuleInfo[i]))
                console.debug('=======get hapModule mainAbilityName========' + data.hapModuleInfo[i].mainAbilityName);
                checkHapModuleInfo(data.hapModuleInfo[i]);
                result.set(data.hapModuleInfo[i].mainAbilityName, data.hapModuleInfo[i]);
            }
            expect(result.has(firstMainAbility)).assertTrue();
            expect(result.has(secondMainAbility)).assertTrue();
            expect(result.get(firstMainAbility).moduleName).assertEqual('entry');
            expect(result.get(secondMainAbility).moduleName).assertEqual('bmsmainabilitysecondscene');
            await uninstall(bundleName);
            ret = true;
            done();
        })
        setTimeout(function () {
            expect(ret).assertTrue();
        }, TIMEOUT);
    })

    /*
     * @tc.number: bms_getHapModuleInfo_0300
     * @tc.name: get hapModuleInfo one app without mainAbility by getBundleInfo
     * @tc.desc: get the module information of the hap without mainAbility
     */
    it('bms_getHapModuleInfo_0300', 0, async function (done) {
        console.debug('===========begin bms_getHapModuleInfo_0300===========')
        await install(['/data/test/bmsThirdBundleTest2.hap']);
        let bundleName = 'com.example.third2';
        let ret = false;
        bundle.getBundleInfo(bundleName, 1).then(async (data) => {
            console.debug('=======get hapModule========' + JSON.stringify(data))
            expect(data.hapModuleInfo.length).assertEqual(1);
            console.debug('=======get hapModule mainAbilityName========' + data.hapModuleInfo[0].mainAbilityName)
            expect(data.hapModuleInfo[0].mainAbilityName).assertEqual('');
            expect(data.hapModuleInfo[0].moduleName).assertEqual('entry');
            checkHapModuleInfo(data.hapModuleInfo[0]);
            await uninstall(bundleName);
            ret = true;
            done();
        })
        setTimeout(function () {
            expect(ret).assertTrue();
        }, TIMEOUT);
    })

    function checkHapModuleInfo(dataInfo) {
        console.debug('========begin check hapModuleInfo========')
        expect(typeof dataInfo.name).assertEqual('string');
        expect(typeof dataInfo.description).assertEqual('string');
        expect(typeof dataInfo.descriptionId).assertEqual('number');
        expect(typeof dataInfo.icon).assertEqual('string');
        expect(typeof dataInfo.label).assertEqual('string');
        expect(typeof dataInfo.labelId).assertEqual('number');
        expect(typeof dataInfo.iconId).assertEqual('number');
        expect(typeof dataInfo.backgroundImg).assertEqual('string');
        expect(typeof dataInfo.supportedModes).assertEqual('number');
        expect(typeof dataInfo.reqCapabilities).assertEqual('object');
        expect(typeof dataInfo.deviceTypes).assertEqual('object');
        expect(typeof dataInfo.abilityInfo).assertEqual('object');
        expect(typeof dataInfo.moduleName).assertEqual('string');
        expect(typeof dataInfo.mainAbilityName).assertEqual('string');
        expect(typeof dataInfo.installationFree).assertEqual('boolean');
    }

    async function install(bundlePath) {
        var installer = await bundle.getBundleInstaller();
        console.log('========install========' + typeof installer);
        installer.install(bundlePath, {
            param: {
                userId: 0,
                installFlag: 1,
                isKeepData: false
            }
        }, onReceiveInstallEvent);
        function onReceiveInstallEvent(err, data) {
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
        }, onReceiveUninstallEvent);

        function onReceiveUninstallEvent(err, data) {
            console.info('========uninstall Finish========');
            expect(typeof err).assertEqual('object');
            expect(err.code).assertEqual(0);
            expect(typeof data).assertEqual('object');
            expect(data.status).assertEqual(0);
            expect(data.statusMessage).assertEqual('SUCCESS');
        }
    }
})