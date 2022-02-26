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

import { describe, beforeAll, beforeEach, afterEach, afterAll, it, expect } from 'deccjsunit/index'
import demo from '@ohos.bundle'
import featureAbility from '@ohos.ability.featureability'
import abilityManager from '@ohos.app.abilityManager'
import commonEvent from '@ohos.commonevent'

const PATH = "/data/"
const ERROR = "error.hap"
const BMSJSTEST1 = "bmsJstest1.hap"
const BMSJSTEST3 = "bmsJstest3.hap"
const BMSJSTEST4 = "bmsJstest4.hap"
const BMSJSTEST5 = "bmsJstest5.hap"
const BMSJSTEST6 = "bmsJstest6.hap"
const BMSJSTEST8 = "bmsJstest8.hap"
const NAME1 = "com.example.myapplication1"
const NAME2 = "com.example.myapplication2"
const NAME3 = "com.example.myapplication4"
const NAME4 = "com.example.myapplication5"
const NAME5 = "com.example.myapplication6"
const THIRD1 = "com.example.third1"
const LAUNCHER = "com.ohos.launcher"
const OBJECT = "object"
const SUCCESS = "SUCCESS"
const TIMEOUT = 1000
const TIMEOUTPROCESS = 9000;
const START_ABILITY_TIMEOUT = 5000;
var subscriberInfoEvent_0100 = {
    events: ['Third1_Publish_CommonEvent'],
};

describe('ActsBundleManagerTest', function () {
    /**
     * @tc.number uninstall_0100
     * @tc.name BUNDLE::uninstall
     * @tc.desc Test uninstall interfaces.
     */
    it('uninstall_0100', 0, async function (done) {
        await install([PATH + BMSJSTEST1]);
        var datainfo1 = await demo.getBundleInfo(NAME1, 1);
        expect(datainfo1.name).assertEqual(NAME1);
        await uninstall(NAME1)
        var datainfo2 = await demo.getBundleInfo(NAME1, 1);
        expect(datainfo2.name).assertEqual('');
        done();
        setTimeout(function () {
            console.info('====> uninstall_0100 =====>')
        }, TIMEOUT)
    })

    /**
     * @tc.number uninstall_0200
     * @tc.name BUNDLE::uninstall
     * @tc.desc Test uninstall interfaces.
     */
    it('uninstall_0200', 0, async function (done) {
        await install([PATH + BMSJSTEST1]);
        var datainfo1 = await demo.getBundleInfo(NAME1, 1);
        expect(datainfo1.name).assertEqual(NAME1);
        await install([PATH + BMSJSTEST3]);
        var datainfo1 = await demo.getBundleInfo(NAME2, 1);
        expect(datainfo1.name).assertEqual(NAME2);
        await uninstall(NAME1);
        var datainfo3 = await demo.getBundleInfo(NAME1, 1);
        expect(datainfo3.name).assertEqual('');
        await uninstall(NAME2);
        var datainfo4 = await demo.getBundleInfo(NAME2, 1);
        expect(datainfo4.name).assertEqual('');
        done();
        setTimeout(function () {
            console.info('====> uninstall_0200 =====>')
        }, TIMEOUT)
    })

    /**
     * @tc.number uninstall_0300
     * @tc.name BUNDLE::uninstall
     * @tc.desc Test uninstall interfaces.
     */
    it('uninstall_0300', 0, async function (done) {
        await install([PATH + BMSJSTEST4, PATH + BMSJSTEST5, PATH + BMSJSTEST6]);
        var datainfo1 = await demo.getBundleInfo(NAME3, 1);
        expect(datainfo1.name).assertEqual(NAME3);
        var datainfo2 = await demo.getBundleInfo(NAME4, 1);
        expect(datainfo2.name).assertEqual(NAME4);
        var datainfo3 = await demo.getBundleInfo(NAME5, 1);
        expect(datainfo3.name).assertEqual(NAME5);
        await uninstall(NAME3);
        var datainfo4 = await demo.getBundleInfo(NAME3, 1);
        expect(datainfo4.name).assertEqual('');
        await uninstall(NAME4);
        var datainfo5 = await demo.getBundleInfo(NAME4, 1);
        expect(datainfo5.name).assertEqual('');
        await uninstall(NAME5);
        var datainfo6 = await demo.getBundleInfo(NAME5, 1);
        expect(datainfo6.name).assertEqual('');
        done();
        setTimeout(function () {
            console.info('====> uninstall_0300 =====>')
        }, TIMEOUT)
    });

    /**
     * @tc.number uninstall_0400
     * @tc.name BUNDLE::uninstall
     * @tc.desc Test uninstall interfaces.
     */
    it('uninstall_0400', 0, async function (done) {
        demo.getBundleInstaller().then((data) => {
            data.uninstall(ERROR, {
                param: {
                    userId: 0,
                    isKeepData: false
                }
            }, OnReceiveinstallEvent);

            function OnReceiveinstallEvent(err, data) {
                expect(typeof data).assertEqual(OBJECT);
                expect(data.statusMessage).assertEqual("STATUS_UNINSTALL_FAILURE");
            }
            done();
        });

        setTimeout(function () {
            console.info('====> uninstall_0400 =====>')
        }, TIMEOUT)
    })

    /**
     * @tc.number uninstall_0500
     * @tc.name BUNDLE::uninstall
     * @tc.desc Test uninstall interfaces.
     */
    it('uninstall_0500', 0, async function (done) {
        demo.getBundleInstaller().then((data) => {
            data.uninstall('', {
                param: {
                    userId: 0,
                    isKeepData: false
                }
            }, OnReceiveinstallEvent);

            function OnReceiveinstallEvent(err, data) {
                expect(typeof data).assertEqual(OBJECT);
                console.info('======data.statusMessage=====' + JSON.stringify(data.statusMessage));
                expect(data.statusMessage).assertEqual("STATUS_UNINSTALL_FAILURE");
                done();
            }
        });
        setTimeout(function () {
            console.info('====> uninstall_0500 =====>')
        }, TIMEOUT)
    })

    /**
     * @tc.number uninstall_0600
     * @tc.name BUNDLE::uninstall
     * @tc.desc Test uninstall interfaces.
     */
    it('uninstall_0600', 0, async function (done) {
        demo.getBundleInstaller().then((data) => {
            data.uninstall(LAUNCHER, {
                param: {
                    userId: 0,
                    isKeepData: false
                }
            }, OnReceiveinstallEvent);

            function OnReceiveinstallEvent(err, data) {
                expect(typeof data).assertEqual(OBJECT);
                console.info('======data.statusMessage=====' + JSON.stringify(data.statusMessage));
                expect(data.statusMessage).assertEqual("STATUS_UNINSTALL_FAILURE");
                done();
            }
        });
        setTimeout(function () {
            console.info('====> uninstall_0600 =====>')
        }, TIMEOUT)
    })

    /**
     * @tc.number uninstall_0700
     * @tc.name BUNDLE::uninstall
     * @tc.desc Test uninstall interfaces.
     */
    it('uninstall_0700', 0, async function (done) {
        let result = await demo.getBundleInstaller();
        await result.install([PATH + BMSJSTEST1], {
            param: {
                userId: 0,
                installFlag: 1,
                isKeepData: true
            }
        }, OnReceiveinstallEvent);
        async function OnReceiveinstallEvent(err, data) {
            expect(typeof data).assertEqual(OBJECT);
            expect(data.statusMessage).assertEqual(SUCCESS);
            var datainfo2 = await demo.getBundleInfo(NAME1, 1);
            expect(datainfo2.name).assertEqual(NAME1);
            await uninstall(NAME1);
            done();
        }
        setTimeout(function () {
            console.info('====> uninstall_0700 =====>')
        }, TIMEOUT)
    })

    /**
     * @tc.number uninstall_0800
     * @tc.name BUNDLE::uninstall
     * @tc.desc Test uninstall interfaces.
     */
    it('uninstall_0800', 0, async function (done) {
        await install([PATH + BMSJSTEST8]);
        var Subscriber;
        let id;
        commonEvent.createSubscriber(subscriberInfoEvent_0100).then((data) => {
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
            console.debug('uninstall_0800=====timeout======');
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
        async function SubscribeCallBack(err, data) {
            clearTimeout(id);
            expect(data.event).assertEqual('Third1_Publish_CommonEvent');
            console.debug('====>Subscribe CallBack data:====>' + JSON.stringify(data));
            let processInfos1 = await abilityManager.getActiveProcessInfos();
            expect(typeof processInfos1).assertEqual('object');
            let processMap1 = new Map();
            let processMap2 = new Map();
            for (var i = 0, len = processInfos1.length; i < len; i++) {
                console.debug('=======Active Process uid=====' + processInfos1[i].uid);
                processMap1.set(processInfos1[i].uid, 0);
            }
            let bundleInfo = await demo.getBundleInfo('com.example.third1', 1);
            let uid = bundleInfo.uid;
            expect(processMap1.has(uid)).assertTrue();
            await uninstall(THIRD1);
            let processInfos2 = await abilityManager.getActiveProcessInfos();
            for (var i = 0, len = processInfos2.length; i < len; i++) {
                console.debug('=======Active Process uid=====' + processInfos1[i].uid);
                processMap2.set(processInfos2[i].uid, 0);
            }
            expect(processMap2.has(uid)).assertFalse();
            commonEvent.unsubscribe(Subscriber, UnSubscribeCallback)
            done();
        }
        setTimeout(function () {
            console.info('====> install_0900 =====>')
        }, TIMEOUTPROCESS)
    })
    async function install(bundlePath) {
        let result = await demo.getBundleInstaller();
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
        let result = await demo.getBundleInstaller();
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
})

