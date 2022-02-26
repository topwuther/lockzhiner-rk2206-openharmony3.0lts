/*
* Copyright (c) 2021 Huawei Device Co., Ltd.
* Licensed under the Apache License, Version 2.0 (the 'License');
* you may not use this file except in compliance with the License.
* You may obtain a copy of the License at
*
*     http://www.apache.org/licenses/LICENSE-2.0
*
* Unless required by applicable law or agreed to in writing, software
* distributed under the License is distributed on an 'AS IS' BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
* See the License for the specific language governing permissions and
* limitations under the License.
*/
import bundle from '@ohos.bundle'
import { describe, beforeAll, beforeEach, afterEach, afterAll, it, expect } from 'deccjsunit/index'
import featureAbility from '@ohos.ability.featureability'
import commonEvent from '@ohos.commonevent'

const START_ABILITY_TIMEOUT = 5000;
const NUM_TWO = 2;
const NUM_TEN = 10;
const INVALID_NUM = -1;
const TIMEOUT = 1000;
const EVENTTIMEOUT = 2000;

var subscriberInfoGetModuleUsageRecordTest_0100 = {
    events: ['ACTS_Third1_Publish_CommonEvent'],
};
describe('ActsBmsModuleUsageRecordTest', function () {
    beforeAll(async (done) => {
        console.debug('=======before all install========');
        await install(['/data/test/bmsThirdBundleTest1.hap'])
        done();
        setTimeout(function () {
            console.debug('=======before all install finish========');
        }, TIMEOUT)
    })

    /*
    * @tc.number: bms_getModuleUsageRecordTest_0100
    * @tc.name: getModuleUsageRecord(maxNum)
    * @tc.desc: Use the getModuleUsageRecord interface to query the activation times and last
    *           activation time of ability
    */
    it('bms_getModuleUsageRecordTest_0100', 0, async function (done) {
        console.debug('=====================bms_getModuleUsageRecordTest_0100==================');
        var bundleName = 'com.example.third1';
        var Subscriber;
        let id;
        async function SubscribeCallBack(err, data) {
            clearTimeout(id);
            expect(data.event).assertEqual('ACTS_Third1_Publish_CommonEvent');
            console.debug('====>Subscribe CallBack data:====>' + JSON.stringify(data));
            let records = await bundle.getModuleUsageRecords(NUM_TWO);
            checkModuleUsageRecord(records, 'bms_getModuleUsageRecordTest_0100');
            var result = checkIsExist(records, bundleName);
            expect(result).assertEqual(true);
            commonEvent.unsubscribe(Subscriber, UnSubscribeCallback);
            done();
        }
        commonEvent.createSubscriber(subscriberInfoGetModuleUsageRecordTest_0100).then((data) => {
            console.debug('====>Create Subscriber====>');
            Subscriber = data;
            commonEvent.subscribe(Subscriber, SubscribeCallBack);
        })
        function UnSubscribeCallback() {
            console.debug('====>UnSubscribe CallBack====>');
            done();
        }
        function timeout() {
            expect().assertFail();
            console.debug('bms_getModuleUsageRecordTest_0100=====timeout======');
            commonEvent.unsubscribe(Subscriber, UnSubscribeCallback)
            done();
        }
        id = setTimeout(timeout, START_ABILITY_TIMEOUT);
        console.debug('=======start ability========')
        let result = await featureAbility.startAbility(
            {
                want:
                {
                    bundleName: 'com.example.third1',
                    abilityName: 'com.example.third1.MainAbility'
                }
            }
        )
        expect(result).assertEqual(0);
        setTimeout(function () {
            console.debug('=====================bms_getModuleUsageRecordTest_0100==================end');
        }, EVENTTIMEOUT)
    })

    /*
    * @tc.number: bms_getModuleUsageRecordTest_0200
    * @tc.name: getModuleUsageRecord(maxNum)
    * @tc.desc: When the number of starts of ability is less than maxNum, call interface getModuleUsageRecord
    */
    it('bms_getModuleUsageRecordTest_0200', 0, async function (done) {
        console.debug('=====================bms_getModuleUsageRecordTest_0200==================');
        var bundleName = 'com.example.third1';
        bundle.getModuleUsageRecords(NUM_TWO, (err, data) => {
            expect(err.code).assertEqual(0);
            checkModuleUsageRecord(data, 'bms_getModuleUsageRecordTest_0200');
            var result = checkIsExist(data, bundleName);
            expect(result).assertEqual(true);
            done();
        });
        setTimeout(function () {
            console.debug('=====================bms_getModuleUsageRecordTest_0200==================end');
        }, TIMEOUT)
    })

    /*
    * @tc.number: bms_getModuleUsageRecordTest_0300
    * @tc.name: getModuleUsageRecord(maxNum)
    * @tc.desc: When the number of starts of ability is less than maxNum, call interface getModuleUsageRecord
    */
    it('bms_getModuleUsageRecordTest_0300', 0, async function (done) {
        console.debug('=====================bms_getModuleUsageRecordTest_0300==================');
        var bundleName = 'com.example.third1';
        bundle.getModuleUsageRecords(NUM_TEN, (err, data) => {
            expect(err.code).assertEqual(0);
            checkModuleUsageRecord(data, 'bms_getModuleUsageRecordTest_0300');
            var result = checkIsExist(data, bundleName);
            expect(result).assertEqual(true);
            done();
        });
        setTimeout(function () {
            console.debug('=====================bms_getModuleUsageRecordTest_0300==================end');
        }, TIMEOUT)
    })

    /*
    * @tc.number: bms_getModuleUsageRecordTest_0400
    * @tc.name: getModuleUsageRecord(maxNum)
    * @tc.desc: When the number of starts of ability is less than maxNum, call interface getModuleUsageRecord
    */
    it('bms_getModuleUsageRecordTest_0400', 0, async function (done) {
        console.debug('=====================bms_getModuleUsageRecordTest_0400==================');
        var bundleName = 'com.example.third1';
        var records = await bundle.getModuleUsageRecords(NUM_TEN);
        checkModuleUsageRecord(records, 'bms_getModuleUsageRecordTest_0400');
        var result = checkIsExist(records, bundleName);
        expect(result).assertEqual(true);
        done();
        setTimeout(function () {
            console.debug('=====================bms_getModuleUsageRecordTest_0400==================end');
        }, TIMEOUT)
    })

    /*
    * @tc.number: bms_getModuleUsageRecordTest_0500
    * @tc.name: getModuleUsageRecord(maxNum) by promise
    * @tc.desc: Uninstall a third-party application, and then use the getModuleUsageRecord interface
    *           to query the activation times and last activation time of ability
    */
    it('bms_getModuleUsageRecordTest_0500', 0, async function (done) {
        console.debug('=====================bms_getModuleUsageRecordTest_0500==================');
        var bundleName = 'com.example.third1'
        await uninstall(bundleName);
        console.debug('===================uninstall third1====================');
        var records = await bundle.getModuleUsageRecords(NUM_TEN)
        checkModuleUsageRecord(records, 'bms_getModuleUsageRecordTest_0500');
        var result = checkIsExist(records, bundleName);
        expect(result).assertEqual(false);
        done();
        setTimeout(function () {
            console.debug('=====================bms_getModuleUsageRecordTest_0500==================end');
        }, TIMEOUT)
    })

    /*
    * @tc.number: bms_getModuleUsageRecordTest_0600
    * @tc.name: getModuleUsageRecord(maxNum,callback: AsyncCallback<Array<ModuleUsageRecord>>)
    * @tc.desc: Uninstall a third-party application, and then use the getModuleUsageRecord interface
    *           to query the activation times and last activation time of ability
    */
    it('bms_getModuleUsageRecordTest_0600', 0, async function (done) {
        console.debug('=====================bms_getModuleUsageRecordTest_0600==================');
        var bundleName = 'com.example.third1'
        await bundle.getModuleUsageRecords(NUM_TEN, (err, data) => {
            expect(err.code).assertEqual(0);
            checkModuleUsageRecord(data, 'bms_getModuleUsageRecordTest_0600');
            var result = checkIsExist(data, bundleName);
            expect(result).assertEqual(false);
            done();
        });
        setTimeout(function () {
            console.debug('=====================bms_getModuleUsageRecordTest_0600==================end');
        }, TIMEOUT)
    })

    /*
    * @tc.number: bms_getModuleUsageRecordTest_0700
    * @tc.name: getModuleUsageRecord(maxNum, callback: AsyncCallback<Array<ModuleUsageRecord>>)
    * @tc.desc: test getModuleUsageRecord with the invalid parameter
    */
    it('bms_getModuleUsageRecordTest_0700', 0, async function (done) {
        console.debug('=====================bms_getModuleUsageRecordTest_0700==================');
        await bundle.getModuleUsageRecords(INVALID_NUM, (err, data) => {
            console.debug('============err.code==========' + err.code);
            expect(err.code).assertEqual(0);
            expect(data.length).assertEqual(0);
            done();
        });
        setTimeout(function () {
            console.debug('=====================bms_getModuleUsageRecordTest_0700==================end');
        }, TIMEOUT)
    })

    /*
    * @tc.number: bms_getModuleUsageRecordTest_0800
    * @tc.name: getModuleUsageRecord(maxNum) by promise
    * @tc.desc: test getModuleUsageRecord with the invalid parameter
    */
    it('bms_getModuleUsageRecordTest_0800', 0, async function (done) {
        console.debug('=====================bms_getModuleUsageRecordTest_0800==================');
        var data = await bundle.getModuleUsageRecords(INVALID_NUM);
        expect(data.length).assertEqual(0);
        done();
        setTimeout(function () {
            console.debug('=====================bms_getModuleUsageRecordTest_0800==================end');
        }, TIMEOUT)
    })

    /*
    * @tc.number: bms_getModuleUsageRecordTest_0900
    * @tc.name: getModuleUsageRecord(maxNum, callback: AsyncCallback<Array<ModuleUsageRecord>>)
    * @tc.desc: test getModuleUsageRecord with the critical value
    */
    it('bms_getModuleUsageRecordTest_0900', 0, async function (done) {
        console.debug('=====================bms_getModuleUsageRecordTest_0900==================');
        await bundle.getModuleUsageRecords(0, (err, data) => {
            console.debug('============err.code==========' + err.code);
            expect(err.code).assertEqual(0);
            expect(data.length).assertEqual(0);
            done();
        });
        setTimeout(function () {
            console.debug('=====================bms_getModuleUsageRecordTest_0900==================end');
        }, TIMEOUT)
    })

    /*
    * @tc.number: bms_getModuleUsageRecordTest_1000
    * @tc.name: getModuleUsageRecord(maxNum, callback: AsyncCallback<Array<ModuleUsageRecord>>)
    * @tc.desc: test getModuleUsageRecord with the critical value
    */
    it('bms_getModuleUsageRecordTest_1000', 0, async function (done) {
        console.debug('=====================bms_getModuleUsageRecordTest_1000==================');
        var data = await bundle.getModuleUsageRecords(0);
        expect(data.length).assertEqual(0);
        done();
        setTimeout(function () {
            console.debug('=====================bms_getModuleUsageRecordTest_1000==================end');
        }, TIMEOUT)
    })

    /*
    * @tc.number: bms_getModuleUsageRecordTest_1100
    * @tc.name: getModuleUsageRecord(maxNum) by promise
    * @tc.desc: Install a third-party which is uninstalled, and then use the getModuleUsageRecord interface
    *           to query the activation times and last activation time of ability
    */
    it('bms_getModuleUsageRecordTest_1100', 0, async function (done) {
        console.debug('=====================bms_getModuleUsageRecordTest_1100==================');
        var bundleName = 'com.example.third1'
        var bundlePath = ['/data/test/bmsThirdBundleTest1.hap']
        await install(bundlePath);
        console.debug('===================install third1====================');
        var records = await bundle.getModuleUsageRecords(NUM_TEN)
        checkModuleUsageRecord(records, 'bms_getModuleUsageRecordTest_1100');
        var result = checkIsExist(records, bundleName);
        expect(result).assertEqual(true);
        done();
        setTimeout(function () {
            console.debug('=====================bms_getModuleUsageRecordTest_1100==================end');
        }, TIMEOUT)
    })

    /*
    * @tc.number: bms_getModuleUsageRecordTest_1200
    * @tc.name: getModuleUsageRecord(maxNum, callback: AsyncCallback<Array<ModuleUsageRecord>>)
    * @tc.desc: Install a third-party which is uninstalled, and then use the getModuleUsageRecord interface
    *           to query the activation times and last activation time of ability
    */
    it('bms_getModuleUsageRecordTest_1200', 0, async function (done) {
        console.debug('=====================bms_getModuleUsageRecordTest_1200==================');
        var bundleName = 'com.example.third1'
        await bundle.getModuleUsageRecords(NUM_TEN, (err, data) => {
            expect(err.code).assertEqual(0);
            checkModuleUsageRecord(data, 'bms_getModuleUsageRecordTest_1200');
            var result = checkIsExist(data, bundleName);
            expect(result).assertEqual(true);
            done();
        })
        setTimeout(function () {
            console.debug('=====================bms_getModuleUsageRecordTest_1200==================end');
        }, TIMEOUT)
    })

    function checkModuleUsageRecord(data, caseName) {
        console.debug('======================check ModuleUsageRecord begin==========================');
        console.debug(caseName + ' ==========record length is ========== ' + data.length);
        expect(data.length).assertLarger(0);
        for (let i = 0, length = data.length; i < length; i++) {
            console.debug('=======All Info========' + JSON.stringify(data[i]));
            console.debug('=============bundleName is=========' + JSON.stringify(data[i].bundleName));
            expect(data[i].bundleName.length).assertLarger(0);
            console.debug('=============appLabelId==============' + JSON.stringify(data[i].appLabelId));
            expect(data[i].appLabelId).assertLarger(0);
            console.debug('=============name==============' + JSON.stringify(data[i].name));
            expect(data[i].name.length).assertLarger(0);
            console.debug('=============labelId==============' + JSON.stringify(data[i].labelId));
            expect(data[i].labelId >= 0).assertTrue();
            console.debug('=============descriptionId==============' + JSON.stringify(data[i].descriptionId));
            expect(data[i].descriptionId).assertEqual(0);
            console.debug('=============abilityName==============' + JSON.stringify(data[i].abilityName));
            expect(data[i].abilityName.length).assertLarger(0);
            console.debug('=============abilityLabelId==============' + JSON.stringify(data[i].abilityLabelId));
            expect(data[i].abilityLabelId).assertLarger(0);
            console.debug('===========abilityDescriptionId===========' + JSON.stringify(data[i].abilityDescriptionId));
            expect(data[i].abilityDescriptionId).assertLarger(0);
            console.debug('=============abilityIconId==============' + JSON.stringify(data[i].abilityIconId));
            expect(data[i].abilityIconId).assertLarger(0);
            console.debug('=============launchedCount==============' + JSON.stringify(data[i].launchedCount));
            expect(data[i].launchedCount).assertLarger(0);
            console.debug('=============lastLaunchTime==============' + JSON.stringify(data[i].lastLaunchTime));
            expect(data[i].lastLaunchTime).assertLarger(0);
            console.debug('=============isRemoved==============' + JSON.stringify(data[i].isRemoved));
            expect(data[i].isRemoved).assertEqual(false);
            expect(data[i].installationFreeSupported).assertEqual(false);
        }
    }
    function checkIsExist(data, bundleName) {
        let bundles = new Map();
        for (let i = 0, length = data.length; i < length; i++) {
            console.debug('=============bundleName is=========' + JSON.stringify(data[i].bundleName));
            bundles.set(data[i].bundleName, data[i]);
        }
        if (bundles.has(bundleName)) {
            console.debug(bundleName + ' is exist');
            return true;
        }
        else {
            console.debug(bundleName + ' is not exist');
            return false;
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
            console.debug('========install Finish========');
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
            console.debug('========uninstall Finish========');
            expect(err.code).assertEqual(0);
            expect(typeof data).assertEqual('object');
            expect(data.status).assertEqual(0);
            expect(data.statusMessage).assertEqual('SUCCESS');
            console.debug('========data.statusMessage========' + data.statusMessage);
        }
    }
    afterAll(async (done) => {
        console.debug('=======after all install========');
        let bundleName = 'com.example.third1'
        uninstall(bundleName);
        done();
    })
})