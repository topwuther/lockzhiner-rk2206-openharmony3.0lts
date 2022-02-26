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
const NAMECOUNT = 10000;

describe('ActsBmsAllShortcutInfoTest', function () {

    /*
     * @tc.number: bms_getAllShortcutInfo_0100
     * @tc.name: test getAllShortcutInfo with promise
     * @tc.desc: get the shortcut information of the hap with type of entry
     */
    it('bms_getAllShortcutInfo_0100', 0, async function (done) {
        console.info('=====================bms_getAllShortcutInfo_0100==================');
        var bundlePath = ['/data/test/bmsThirdBundleTest1.hap'];
        await install(bundlePath);
        var bundleName = 'com.example.third1';
        bundle.getAllShortcutInfo(bundleName).then((data) => {
            expect(typeof data).assertEqual('object');
            expect(data.length).assertEqual(1);
            checkShortcutIsExist(data, 'id.third1', 'third1');
            done();
        });
        setTimeout(function () {
            console.info('=====================bms_getAllShortcutInfo_0100==================end');
        }, TIMEOUT)
    });

    /*
     * @tc.number: bms_getAllShortcutInfo_0200
     * @tc.name: test getAllShortcutInfo with callback
     * @tc.desc: get the shortcut information of the hap with type of entry
     */
    it('bms_getAllShortcutInfo_0200', 0, async function (done) {
        console.info('=====================bms_getAllShortcutInfo_0200==================');
        var bundleName = 'com.example.third1';
        bundle.getAllShortcutInfo(bundleName, (result, data) => {
            expect(result.code).assertEqual(0);
            expect(data.length).assertEqual(1);
            checkShortcutIsExist(data, 'id.third1', 'third1');
            done();
        });
        setTimeout(function () {
            console.info('=====================bms_getAllShortcutInfo_0200==================end');
        }, TIMEOUT)
    })

    /*
     * @tc.number: bms_getAllShortcutInfo_0300
     * @tc.name: test getAllShortcutInfo with promise
     * @tc.desc: 1.install a hap with type of feature
     *           1.get the shortcut informations of two types of hap
     */
    it('bms_getAllShortcutInfo_0300', 0, async function (done) {
        console.info('=====================bms_getAllShortcutInfo_0300==================');
        var bundleName = 'com.example.third1';
        var bundlePath = ['/data/test/bmsThirdBundleTest3.hap'];
        await install(bundlePath);
        var data = await bundle.getAllShortcutInfo(bundleName);
        expect(typeof data).assertEqual('object');
        expect(data.length).assertEqual(2);
        checkShortcutIsExist(data, 'id.third1', 'third1');
        checkShortcutIsExist(data, 'id.third3', 'third3');
        done();
        setTimeout(function () {
            console.info('=====================bms_getAllShortcutInfo_0300==================end');
        }, TIMEOUT)
    })

    /*
     * @tc.number: bms_getAllShortcutInfo_0400
     * @tc.name: test getAllShortcutInfo with callback
     * @tc.desc: 1.get the shortcut informations of two types of hap
     */
    it('bms_getAllShortcutInfo_0400', 0, async function (done) {
        console.info('=====================bms_getAllShortcutInfo_0400==================');
        var bundleName = 'com.example.third1';
        bundle.getAllShortcutInfo(bundleName, async (result, data) => {
            expect(result.code).assertEqual(0);
            expect(data.length).assertEqual(2);
            checkShortcutIsExist(data, 'id.third1', 'third1');
            checkShortcutIsExist(data, 'id.third3', 'third3');
            await uninstall(bundleName);
            done();
        });
        setTimeout(function () {
            console.info('=====================bms_getAllShortcutInfo_0400==================end');
        }, TIMEOUT)
    })

    /*
     * @tc.number: bms_getAllShortcutInfo_0500
     * @tc.name: test getAllShortcutInfo with promise
     * @tc.desc: get the shortcut informations of an invalid hap
     */
    it('bms_getAllShortcutInfo_0500', 0, async function (done) {
        console.info('=====================bms_getAllShortcutInfo_0500==================');
        var data = await bundle.getAllShortcutInfo('');
        expect(typeof data).assertEqual('object');
        expect(data.length).assertEqual(0);
        done();
        setTimeout(function () {
            console.info('=====================bms_getAllShortcutInfo_0500==================end');
        }, TIMEOUT)
    })

    /*
     * @tc.number: bms_getAllShortcutInfo_0600
     * @tc.name: test getAllShortcutInfo with callback
     * @tc.desc: get the shortcut informations of an invalid hap
     */
    it('bms_getAllShortcutInfo_0600', 0, async function (done) {
        console.info('=====================bms_getAllShortcutInfo_0600==================');
        var bundleName = '';
        bundle.getAllShortcutInfo(bundleName, (result, data) => {
            expect(result.code).assertEqual(-1);
            expect(data.length).assertEqual(0);
            done();
        });
        setTimeout(function () {
            console.info('=====================bms_getAllShortcutInfo_0600==================end');
        }, TIMEOUT)
    })

    /*
     * @tc.number: bms_getAllShortcutInfo_0700
     * @tc.name: test getAllShortcutInfo with promise
     * @tc.desc: 1.install a hap with low version
     *           2.install a hap with high version
     *           3.get the shortcut information of a hap with high version by bundlename
     */
    it('bms_getAllShortcutInfo_0700', 0, async function (done) {
        console.info('=====================bms_getAllShortcutInfo_0700==================');
        var bundlePath1 = ['/data/test/bmsThirdBundleTest1.hap'];
        await install(bundlePath1);
        var bundleName = 'com.example.third1';
        var bundlePath2 = ['/data/test/bmsThirdBundleTestA1.hap'];
        await install(bundlePath2);
        var data = await bundle.getAllShortcutInfo(bundleName);
        expect(typeof data).assertEqual('object');
        expect(data.length).assertEqual(1);
        for (var i = 0; i < data.length; i++) {
            expect(typeof data[i]).assertEqual('object');
            expect(typeof data[i].id).assertEqual('string');
            expect(data[i].id).assertEqual('id.thirdA1');
            expect(typeof data[i].disableMessage).assertEqual('string');
            expect(typeof data[i].isStatic).assertEqual('boolean');
            expect(typeof data[i].isHomeShortcut).assertEqual('boolean');
            expect(typeof data[i].isEnabled).assertEqual('boolean');
            expect(typeof data[i].hostAbility).assertEqual('string');
            expect(typeof data[i].wants).assertEqual('object')
            if (typeof data[i].wants != 'undefined' && Object.keys(data[i].wants).length != 0) {
                for (var j = 0; j < data[i].wants.length; j++) {
                    expect(data[i].wants[j].targetClass).assertEqual('com.example.third1.AMainAbility');
                    expect(data[i].wants[j].targetBundle).assertEqual('com.example.third1');
                }
            }
        }
        done();
        setTimeout(function () {
            console.info('=====================bms_getAllShortcutInfo_0700==================end');
        }, TIMEOUT)
    })

    /*
     * @tc.number: bms_getAllShortcutInfo_0800
     * @tc.name: test getAllShortcutInfo with callback
     * @tc.desc: get the shortcut information of a hap with high version by bundlename
     */
    it('bms_getAllShortcutInfo_0800', 0, async function (done) {
        console.info('=====================bms_getAllShortcutInfo_0800==================');
        var bundleName = 'com.example.third1';
        bundle.getAllShortcutInfo(bundleName, async (result, data) => {
            expect(result.code).assertEqual(0);
            expect(data.length).assertEqual(1);
            for (var i = 0; i < data.length; i++) {
                expect(typeof data[i]).assertEqual('object');
                expect(typeof data[i].id).assertEqual('string');
                expect(data[i].id).assertEqual('id.thirdA1');
                expect(typeof data[i].disableMessage).assertEqual('string');
                expect(typeof data[i].isStatic).assertEqual('boolean');
                expect(typeof data[i].isHomeShortcut).assertEqual('boolean');
                expect(typeof data[i].isEnabled).assertEqual('boolean');
                expect(typeof data[i].hostAbility).assertEqual('string');
                expect(typeof data[i].wants).assertEqual('object')
                if (typeof data[i].wants != 'undefined' && Object.keys(data[i].wants).length != 0) {
                    for (var j = 0; j < data[i].wants.length; j++) {
                        expect(data[i].wants[j].targetClass).assertEqual('com.example.third1.AMainAbility');
                        expect(data[i].wants[j].targetBundle).assertEqual('com.example.third1');
                    }
                }
            }
            await uninstall(bundleName);
            done();
        });
        setTimeout(function () {
            console.info('=====================bms_getAllShortcutInfo_0800==================end');
        }, TIMEOUT)
    })

    /*
     * @tc.number: bms_getAllShortcutInfo_0900
     * @tc.name: test getAllShortcutInfo with promise
     * @tc.desc: get the shortcut information of a hap without shortcut in config.json
     */
    it('bms_getAllShortcutInfo_0900', 0, async function (done) {
        console.info('=====================bms_getAllShortcutInfo_0900==================');
        var bundleName = 'com.example.third2';
        var bundlePath = ['/data/test/bmsThirdBundleTest2.hap'];
        await install(bundlePath);
        var data = await bundle.getAllShortcutInfo(bundleName);
        expect(typeof data).assertEqual('object');
        expect(data.length).assertEqual(0);
        done();
        setTimeout(function () {
            console.info('=====================bms_getAllShortcutInfo_0900==================end');
        }, TIMEOUT)
    })

    /*
     * @tc.number: bms_getAllShortcutInfo_1000
     * @tc.name: test getAllShortcutInfo with callback
     * @tc.desc: get the shortcut information of a hap without shortcut in config.json
     */
    it('bms_getAllShortcutInfo_1000', 0, async function (done) {
        console.info('=====================bms_getAllShortcutInfo_1000==================');
        var bundleName = 'com.example.third2';
        bundle.getAllShortcutInfo(bundleName, async (result, data) => {
            expect(result.code).assertEqual(0);
            expect(data.length).assertEqual(0);
            await uninstall(bundleName);
            done();
        });
        setTimeout(function () {
            console.info('=====================bms_getAllShortcutInfo_1000==================end');
        }, TIMEOUT)
    })

    /*
     * @tc.number: bms_getAllShortcutInfo_1100
     * @tc.name: test getAllShortcutInfo with promise
     * @tc.desc: get the shortcut information of a system hap
     */
    it('bms_getAllShortcutInfo_1100', 0, async function (done) {
        console.info('=====================bms_getAllShortcutInfo_1100==================');
        var bundleName = 'com.example.system1';
        var data = await bundle.getAllShortcutInfo(bundleName);
        expect(typeof data).assertEqual('object');
        expect(data.length).assertEqual(1);
        checkShortcutIsExist(data, 'id.system1', 'system1');
        done();
        setTimeout(function () {
            console.info('=====================bms_getAllShortcutInfo_1100==================end');
        }, TIMEOUT)
    })

    /*
     * @tc.number: bms_getAllShortcutInfo_1200
     * @tc.name: test getAllShortcutInfo with callback
     * @tc.desc: get the shortcut information of a system hap
     */
    it('bms_getAllShortcutInfo_1200', 0, async function (done) {
        console.info('=====================bms_getAllShortcutInfo_1200==================');
        var bundleName = 'com.example.system1';
        bundle.getAllShortcutInfo(bundleName, (result, data) => {
            expect(result.code).assertEqual(0);
            expect(data.length).assertEqual(1);
            checkShortcutIsExist(data, 'id.system1', 'system1');
            done();
        });
        setTimeout(function () {
            console.info('=====================bms_getAllShortcutInfo_1200==================end');
        }, TIMEOUT)
    })

    /*
     * @tc.number: bms_getAllShortcutInfo_1300
     * @tc.name: test getAllShortcutInfo with promise
     * @tc.desc: get the shortcut information of a vendor hap
     */
    it('bms_getAllShortcutInfo_1300', 0, async function (done) {
        console.info('=====================bms_getAllShortcutInfo_1300==================');
        var bundleName = 'com.example.vendor1';
        var data = await bundle.getAllShortcutInfo(bundleName);
        expect(typeof data).assertEqual('object');
        expect(data.length).assertEqual(1);
        checkShortcutIsExist(data, 'id.vendor1', 'vendor1');
        done();
        setTimeout(function () {
            console.info('=====================bms_getAllShortcutInfo_1300==================end');
        }, TIMEOUT)
    })

    /*
     * @tc.number: bms_getAllShortcutInfo_1400
     * @tc.name: test getAllShortcutInfo with callback
     * @tc.desc: get the shortcut information of a vendor hap
     */
    it('bms_getAllShortcutInfo_1400', 0, async function (done) {
        console.info('=====================bms_getAllShortcutInfo_1400==================');
        var bundleName = 'com.example.vendor1';
        bundle.getAllShortcutInfo(bundleName, (result, data) => {
            expect(result.code).assertEqual(0);
            expect(data.length).assertEqual(1);
            checkShortcutIsExist(data, 'id.vendor1', 'vendor1');
            done();
        });
        setTimeout(function () {
            console.info('=====================bms_getAllShortcutInfo_1400==================end');
        }, TIMEOUT)
    })

    /*
     * @tc.number: bms_getAllShortcutInfo_1500
     * @tc.name: test getAllShortcutInfo with promise
     * @tc.desc: 1.install a normal hap
     *           2.get the shortcut information of this hap by bundlename
     *           3.get the shortcut information of this hap after uninstalling the hap
     */
    it('bms_getAllShortcutInfo_1500', 0, async function (done) {
        console.info('=====================bms_getAllShortcutInfo_1500==================');
        var bundlePath = ['/data/test/bmsThirdBundleTest1.hap'];
        await install(bundlePath);
        var bundleName = 'com.example.third1';
        var data = await bundle.getAllShortcutInfo(bundleName);
        expect(typeof data).assertEqual('object');
        expect(data.length).assertEqual(1);
        checkShortcutIsExist(data, 'id.third1', 'third1');
        await uninstall(bundleName);
        var info = await bundle.getAllShortcutInfo(bundleName);
        expect(info.length).assertEqual(0);
        done();
        setTimeout(function () {
            console.info('=====================bms_getAllShortcutInfo_1500==================end');
        }, TIMEOUT)
    })

    /*
     * @tc.number: bms_getAllShortcutInfo_1600
     * @tc.name: test getAllShortcutInfo with callback
     * @tc.desc: get the shortcut information of this hap after uninstalling the hap
     */
    it('bms_getAllShortcutInfo_1600', 0, async function (done) {
        console.info('=====================bms_getAllShortcutInfo_1600==================');;
        var bundleName = 'com.example.third1';
        bundle.getAllShortcutInfo(bundleName, (result, data) => {
            expect(result.code).assertEqual(-1);
            expect(data.length).assertEqual(0);
            done();
        });
        setTimeout(function () {
            console.info('=====================bms_getAllShortcutInfo_1600==================end');
        }, TIMEOUT)
    })

    /*
     * @tc.number: bms_getAllShortcutInfo_1700
     * @tc.name: test getAllShortcutInfo with promise
     * @tc.desc: get the shortcut informations of this hap with two shortcuts in config.json by bundlename
     */
    it('bms_getAllShortcutInfo_1700', 0, async function (done) {
        console.info('=====================bms_getAllShortcutInfo_1700==================');
        var bundleName = 'com.example.third4';
        var bundlePath = ['/data/test/bmsThirdBundleTest4.hap'];
        await install(bundlePath);
        var data = await bundle.getAllShortcutInfo(bundleName);
        expect(typeof data).assertEqual('object');
        expect(data.length).assertEqual(2);
        checkShortcutIsExist(data, 'id.third4A', 'third4A');
        checkShortcutIsExist(data, 'id.third4B', 'third4B');
        done();
        setTimeout(function () {
            console.info('=====================bms_getAllShortcutInfo_1700==================end');
        }, TIMEOUT)
    })

    /*
     * @tc.number: bms_getAllShortcutInfo_1800
     * @tc.name: test getAllShortcutInfo with callback
     * @tc.desc: get the shortcut informations of this hap with two shortcuts in config.json by bundlename
     */
    it('bms_getAllShortcutInfo_1800', 0, async function (done) {
        console.info('=====================bms_getAllShortcutInfo_1800==================');
        var bundleName = 'com.example.third4';
        bundle.getAllShortcutInfo(bundleName, async (result, data) => {
            expect(result.code).assertEqual(0);
            expect(data.length).assertEqual(2);
            checkShortcutIsExist(data, 'id.third4A', 'third4A');
            checkShortcutIsExist(data, 'id.third4B', 'third4B');
            await uninstall(bundleName);
            done();
        });
        setTimeout(function () {
            console.info('=====================bms_getAllShortcutInfo_1800==================end');
        }, TIMEOUT)
    })

    /*
     * @tc.number: bms_getAllShortcutInfo_1900
     * @tc.name: test getAllShortcutInfo with promise
     * @tc.desc: get the shortcut informations of an invalid bundleName
     */
    it('bms_getAllShortcutInfo_1900', 0, async function (done) {
        console.info('=====================bms_getAllShortcutInfo_1900==================');
        var bundleName = '!@#$%^&ERTYUhusdf7254_=-';
        bundle.getAllShortcutInfo(bundleName).then((data) => {
            expect(data.length).assertEqual(0);
            done();
        });
        setTimeout(function () {
            console.info('=====================bms_getAllShortcutInfo_1900==================end');
        }, TIMEOUT)
    })

    /*
     * @tc.number: bms_getAllShortcutInfo_2000
     * @tc.name: test getAllShortcutInfo with promise
     * @tc.desc: get the shortcut informations of an invalid bundleName
     */
    it('bms_getAllShortcutInfo_2000', 0, async function (done) {
        console.info('=====================bms_getAllShortcutInfo_2000==================');
        var bundleName = '!@#$%^&ERTYUhusdf7254_=-';
        bundle.getAllShortcutInfo(bundleName, async (result, data) => {
            expect(result.code).assertEqual(-1);
            expect(data.length).assertEqual(0);
            done();
        });
        setTimeout(function () {
            console.info('=====================bms_getAllShortcutInfo_2000==================end');
        }, TIMEOUT)
    })

    /*
     * @tc.number: bms_getAllShortcutInfo_2100
     * @tc.name: test getAllShortcutInfo with promise
     * @tc.desc: get the shortcut informations of an invalid long bundleName
     */
    it('bms_getAllShortcutInfo_2100', 0, async function (done) {
        console.info('=====================bms_getAllShortcutInfo_2100==================');
        var bundleName = 'test';
        for (var i = 0; i < NAMECOUNT; i++) {
            bundleName += 'test';
        }
        var data = await bundle.getAllShortcutInfo(bundleName);
        expect(typeof data).assertEqual('object');
        expect(data.length).assertEqual(0);
        done();
        setTimeout(function () {
            console.info('=====================bms_getAllShortcutInfo_2100==================end');
        }, TIMEOUT)
    })

    /*
     * @tc.number: bms_getAllShortcutInfo_2200
     * @tc.name: test getAllShortcutInfo with promise
     * @tc.desc: get the shortcut informations of an invalid long bundleName
     */
    it('bms_getAllShortcutInfo_2200', 0, async function (done) {
        console.info('=====================bms_getAllShortcutInfo_2200==================');
        var bundleName = 'test';
        for (var i = 0; i < NAMECOUNT; i++) {
            bundleName += 'test';
        }
        bundle.getAllShortcutInfo(bundleName, async (result, data) => {
            expect(result.code).assertEqual(-1);
            expect(data.length).assertEqual(0);
            done();
        });
        setTimeout(function () {
            console.info('=====================bms_getAllShortcutInfo_2200==================end');
        }, TIMEOUT)
    })

    function checkShortcutInfo(dataInfo, name) {
        console.info('=======Shortcut Info========' + JSON.stringify(dataInfo))
        console.info('=============Shortcutid=========' + dataInfo.id);
        expect(typeof dataInfo.id).assertEqual('string');
        expect(dataInfo.id).assertEqual('id.' + name);
        console.info('=============icon==============' + JSON.stringify(dataInfo.icon));
        expect(typeof dataInfo.disableMessage).assertEqual('string');
        expect(typeof dataInfo.isStatic).assertEqual('boolean');
        expect(typeof dataInfo.isHomeShortcut).assertEqual('boolean');
        expect(typeof dataInfo.isEnabled).assertEqual('boolean');
        expect(typeof dataInfo.hostAbility).assertEqual('string');
        console.info('=============label==============' + JSON.stringify(dataInfo.label));
        expect(typeof dataInfo.wants).assertEqual('object');
        if (typeof dataInfo.wants != 'undefined' && Object.keys(dataInfo.wants).length != 0) {
            expect(dataInfo.wants.length).assertLarger(0);
            for (var j = 0; j < dataInfo.wants.length; j++) {
                console.info('========targetClass=========' + JSON.stringify(dataInfo.wants[j].targetClass));
                expect(dataInfo.wants[j].targetClass).assertEqual('com.example.' + name + '.MainAbility');
                console.info('========targetBundle=========' + JSON.stringify(dataInfo.wants[j].targetBundle));
                expect(dataInfo.wants[j].targetBundle).assertEqual('com.example.' + name);
            }
        }
    }

    async function install(bundlePath) {
        let result = await bundle.getBundleInstaller();
        result.install(bundlePath, {
            param: {
                userId: 0,
                installFlag: 1,
                isKeepData: false
            }
        }, OnReceiveInstallEvent);

        function OnReceiveInstallEvent(err, data) {
            expect(data.status).assertEqual(0);
            expect(data.statusMessage).assertEqual("SUCCESS");
        };
    }
    async function uninstall(bundleName) {
        let result = await bundle.getBundleInstaller();
        result.uninstall(bundleName, {
            param: {
                userId: 0,
                installFlag: 1,
                isKeepData: false
            }
        }, OnReceiveUninstallEvent);

        function OnReceiveUninstallEvent(err, data) {
            expect(data.status).assertEqual(0);
            expect(data.statusMessage).assertEqual("SUCCESS");
        };
    }

    function checkShortcutIsExist(dataInfo, shortcutId, testName) {
        let info = new Map();
        for (var i = 0, len = dataInfo.length; i < len; i++) {
            expect(typeof dataInfo[i]).assertEqual('object');
            info.set(dataInfo[i].id, dataInfo[i]);
        }
        expect(info.has(shortcutId)).assertTrue();
        if (info.has(shortcutId)) {
            checkShortcutInfo(info.get(shortcutId), testName);
        }
    }
})