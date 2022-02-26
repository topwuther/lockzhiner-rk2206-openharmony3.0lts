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
import app from '@system.app';
import {describe, beforeAll, beforeEach, afterEach, afterAll, it, expect} from 'deccjsunit/index'
import deviceManager from '@ohos.distributedHardware.deviceManager'

describe('deviceManagerTest', function() {

    /*
     * @tc.number  deviceManagerTest_createDeviceManager_0100
     * @tc.name    correct Packagesname for createDeviceManager
     * @tc.desc    Function test
     * @tc.level   0
     */
    it('deviceManagerTest_createDeviceManager_0100', 0, async function (done) {
        console.log("----------start running deviceManagerTest_createDeviceManager_0100----------");
        deviceManager.createDeviceManager('com.ohos.devicemangagerdemo', (err, data) => {
            if (err) {
                expect().assertFail();
                return ;
            }
            expect(data).assertUndefined();
        });
        console.log("----------stop running deviceManagerTest_createDeviceManager_0100----------");
        done();
    })

    /*
     * @tc.number  deviceManagerTest_createDeviceManager_0200
     * @tc.name    null Packagesname  for createDeviceManager
     * @tc.desc    Function test
     * @tc.level   3
     */
    it('deviceManagerTest_createDeviceManager_0200', 0, async function (done) {
        console.log("----------start running deviceManagerTest_createDeviceManager_0200----------");
        deviceManager.createDeviceManager('', (err, data) => {
            if (err) {
                expect().assertFail();
                return ;
            }
            expect(data).assertNull();
        });
        console.log("----------stop running deviceManagerTest_createDeviceManager_0200----------");
        done();
    })

    /*
     * @tc.number  deviceManagerTest_createDeviceManager_0300
     * @tc.name    incorrect Packagesname for createDeviceManager
     * @tc.desc    Function test
     * @tc.level   3
     */
    it('deviceManagerTest_createDeviceManager_0300', 0, async function (done) {
        console.log("----------start running deviceManagerTest_createDeviceManager_0300----------");
        deviceManager.createDeviceManager('comohosdevicemangagerdemo', (err, data) => {
            if (err) {
                expect().assertFail();
                return ;
            }
            expect(data).assertNull();
        });
        console.log("----------stop running deviceManagerTest_createDeviceManager_0300----------");
        done();
    })

    /*
     * @tc.number  deviceManagerTest_createDeviceManager_0400
     * @tc.name    incorrect Packagesname case2 for createDeviceManager
     * @tc.desc    Function test
     * @tc.level   3
     */
    it('deviceManagerTest_createDeviceManager_0400', 0, async function (done) {
        console.log("----------start running deviceManagerTest_createDeviceManager_0400----------");
        deviceManager.createDeviceManager('com.ohos.123', (err, data) => {
            if (err) {
                expect().assertFail();
                return ;
            }
            expect(data).assertNull();
        });
        console.log("----------stop running deviceManagerTest_createDeviceManager_0400----------");
        done();
    })

    /*
     * @tc.number  deviceManagerTest_createDeviceManager_0500
     * @tc.name    special Packagesname for createDeviceManager
     * @tc.desc    Function test
     * @tc.level   3
     */
    it('deviceManagerTest_createDeviceManager_0500', 0, async function (done) {
        console.log("----------start running deviceManagerTest_createDeviceManager_0500----------");
        deviceManager.createDeviceManager('com#ohos%devicemangagerdemo&*', (err, data) => {
            if (err) {
                expect().assertFail();
                return ;
            }
            expect(data).assertNull();
        });
        console.log("----------stop running deviceManagerTest_createDeviceManager_0500----------");
        done();
    })

    /*
     * @tc.number  deviceManagerTest_startDeviceDiscovery_0100
     * @tc.name    correct deviceinfo for startDeviceDiscovery
     * @tc.desc    Function test
     * @tc.level   3
     */
    it('deviceManagerTest_startDeviceDiscovery_0100', 0, async function (done) {
        console.log("----------start running deviceManagerTest_startDeviceDiscovery_0100----------");
        deviceManager.createDeviceManager('com.ohos.devicemangagerdemo', (err, data) => {
            if (err) {
                expect().assertFail();
                return ;
            }

            var info = {
                "subscribeId": 2115,
                "mode": 0xAA,
                "medium": 0,
                "freq": 2,
                "isSameAccount": false,
                "isWakeRemote": true,
                "capability": 0
            };
            var start = data.startDeviceDiscovery(info);
            expect(start).assertUndefined();
        });
        console.log("----------stop running deviceManagerTest_startDeviceDiscovery_0100----------");
        done();
    })

    /*
     * @tc.number  deviceManagerTest_release_0100
     * @tc.name    testcase for release
     * @tc.desc    Function test
     * @tc.level   1
     */
    it('deviceManagerTest_release_0100', 0, async function (done) {
        console.log("----------start running deviceManagerTest_release_0100----------");
        deviceManager.createDeviceManager('com.ohos.devicemangagerdemo', (err, data) => {
            if (err) {
                expect().assertFail();
                return ;
            }
            var re = data.release();
            expect(re).assertUndefined();
        });
        console.log("----------stop running deviceManagerTest_release_0100----------");
        done();
    })

    /*
     * @tc.number  deviceManagerTest_getTrustedDeviceListSync_0100
     * @tc.name    testcase for getTrustedDeviceListSync
     * @tc.desc    Function test
     * @tc.level   2
     */
    it('deviceManagerTest_getTrustedDeviceListSync_0100', 0, async function (done) {
        console.log("----------start running deviceManagerTest_getTrustedDeviceListSync_0100----------");
        deviceManager.createDeviceManager('com.ohos.devicemangagerdemo', (err, data) => {
            if (err) {
                expect().assertFail();
                return ;
            }
            var array = data.getTrustedDeviceListSync();
            expect(array).assertUndefined();
        });
        console.log("----------stop running deviceManagerTest_getTrustedDeviceListSync_0100----------");
        done();
    })

    /*
     * @tc.number  deviceManagerTest_stopDeviceDiscovery_0100
     * @tc.name    testcase for stopDeviceDiscovery
     * @tc.desc    Function test
     * @tc.level   2
     */
    it('deviceManagerTest_stopDeviceDiscovery_0100', 0, async function (done) {
        console.log("----------start running deviceManagerTest_stopDeviceDiscovery_0100----------");
        deviceManager.createDeviceManager('com.ohos.devicemangagerdemo', (err, data) => {
            if (err) {
                expect().assertFail();
                return ;
            }
            var subscribeId = 2115;
            var start = data.stopDeviceDiscovery(subscribeId);
            expect(start).assertUndefined();
        });
        console.log("----------stop running deviceManagerTest_stopDeviceDiscovery_0100----------");
        done();
    })

    /*
     * @tc.number  deviceManagerTest_authenticateDevice_0100
     * @tc.name    testcase for authenticateDevice in correct para
     * @tc.desc    Function test
     * @tc.level   2
     */
    it('deviceManagerTest_authenticateDevice_0100', 0, async function (done) {
        console.log("----------start running deviceManagerTest_authenticateDevice_0100----------");
        deviceManager.createDeviceManager('com.ohos.devicemangagerdemo', (err, data) => {
            if (err) {
                expect().assertFail();
                return ;
            }
            let deviceInfo = {
            deviceId: "",
            deviceName: "",
            deviceType: 0
            }
            let extraInfo = {
                "targetPkgName": 'com.ohos.devicemangagerdemo',
                "appName": "myapplication",
                "appDescription": "apply auth",
                "business": '0',
                "displayOwner": 0
            }
            let authParam = {
                "authType": 1,
                "appIcon": new Uint8Array([]),
                "appThumbnail": new Uint8Array([]),
                "extraInfo": extraInfo
            }
            data.authenticateDevice(deviceInfo, authParam, (err, data) => {
                if (err) {
                    expect().assertFail();
                    return;
                }
                var token = data.pinToken;
                expect(token).assertNotNull();
            });
        });
        console.log("----------stop running deviceManagerTest_authenticateDevice_0100----------");
        done();
    })

    /*
     * @tc.number  deviceManagerTest_authenticateDevice_0100
     * @tc.name    testcase two for authenticateDevice in correct para 
     * @tc.desc    Function test
     * @tc.level   2
     */
    it('deviceManagerTest_authenticateDevice_0100', 0, async function (done) {
        console.log("----------start running deviceManagerTest_authenticateDevice_0100----------");
        deviceManager.createDeviceManager('com.ohos.devicemangagerdemo', (err, data) => {
            if (err) {
                expect().assertFail();
                return ;
            }
            let authInfo = {
                "authType": 1,
                "token": 12,
                "extraInfo": {
                    "pinCode": 123456,
                }
            }
            data.verifyAuthInfo(authInfo, (err, data) => {
                if (err) {
                    expect().assertFail();
                    return;
                }
                expect(data).assertUndefined();
            });
        });
        console.log("----------stop running deviceManagerTest_authenticateDevice_0100----------");
        done();
    })

    /*
     * @tc.number  deviceManagerTest_getAuthenticationParam_0100
     * @tc.name    testcase for getAuthenticationParam
     * @tc.desc    Function test
     * @tc.level   2
     */
    it('deviceManagerTest_getAuthenticationParam_0100', 0, async function (done) {
        console.log("----------start running deviceManagerTest_getAuthenticationParam_0100----------");
        deviceManager.createDeviceManager('com.ohos.devicemangagerdemo', (err, data) => {
            if (err) {
                expect().assertFail();
                return ;
            }
            var authparam = data.getAuthenticationParam();
            expect(authparam).assertNotNull();
        });
        console.log("----------stop running deviceManagerTest_getAuthenticationParam_0100----------");
        done();
    })

    /*
     * @tc.number  deviceManagerTest_on-deviceStateChange_0100
     * @tc.name    testcase for deviceStateChange on
     * @tc.desc    Function test
     * @tc.level   2
     */
    it('deviceManagerTest_on-deviceStateChange_0100', 0, async function (done) {
        console.log("----------start running deviceManagerTest_on-deviceStateChange_0100----------");
        deviceManager.createDeviceManager('com.ohos.devicemangagerdemo', (err, data) => {
            if (err) {
                expect().assertFail();
                return ;
            }
            var fa = data.on('deviceStateChange', () => {});
            expect(fa).assertUndefined();
        });
        console.log("----------stop running deviceManagerTest_on-deviceStateChange_0100----------");
        done();
    })

    /*
     * @tc.number  deviceManagerTest_off-deviceStateChange_0100
     * @tc.name    testcase for deviceStateChange off
     * @tc.desc    Function test
     * @tc.level   2
     */
    it('deviceManagerTest_off-deviceStateChange_0100', 0, async function (done) {
        console.log("----------start running deviceManagerTest_off-deviceStateChange_0100----------");
        deviceManager.createDeviceManager('com.ohos.devicemangagerdemo', (err, data) => {
            if (err) {
                expect().assertFail();
                return ;
            }
            var fa = data.off('deviceStateChange', () => {});
            expect(fa).assertUndefined();
        });
        console.log("----------stop running deviceManagerTest_off-deviceStateChange_0100----------");
        done();
    })

    /*
     * @tc.number  deviceManagerTest_on-deviceFound_0100
     * @tc.name    testcase for deviceFound on
     * @tc.desc    Function test
     * @tc.level   2
     */
    it('deviceManagerTest_on-deviceFound_0100', 0, async function (done) {
        console.log("----------start running deviceManagerTest_on-deviceFound_0100----------");
        deviceManager.createDeviceManager('com.ohos.devicemangagerdemo', (err, data) => {
            if (err) {
                expect().assertFail();
                return ;
            }
            var fa = data.on('deviceFound', () => {});
            expect(fa).assertUndefined();
        });
        console.log("----------stop running deviceManagerTest_on-deviceFound_0100----------");
        done();
    })

    /*
     * @tc.number  deviceManagerTest_off-deviceFound_0100
     * @tc.name    testcase for deviceFound off
     * @tc.desc    Function test
     * @tc.level   2
     */
    it('deviceManagerTest_off-deviceFound_0100', 0, async function (done) {
        console.log("----------start running deviceManagerTest_off-deviceFound_0100----------");
        deviceManager.createDeviceManager('com.ohos.devicemangagerdemo', (err, data) => {
            if (err) {
                expect().assertFail();
                return ;
            }
            var fa = data.off('deviceFound', () => {});
            expect(fa).assertUndefined();
        });
        console.log("----------stop running deviceManagerTest_off-deviceFound_0100----------");
        done();
    })

    /*
     * @tc.number  deviceManagerTest_on-discoverFail_0100
     * @tc.name    testcase for discoverFail on
     * @tc.desc    Function test
     * @tc.level   2
     */
    it('deviceManagerTest_on-discoverFail_0100', 0, async function (done) {
        console.log("----------start running deviceManagerTest_on-discoverFail_0100----------");
        deviceManager.createDeviceManager('com.ohos.devicemangagerdemo', (err, data) => {
            if (err) {
                expect().assertFail();
                return ;
            }
            var fa = data.on('discoverFail', () => {});
            expect(fa).assertUndefined();
        });
        console.log("----------stop running deviceManagerTest_on-discoverFail_0100----------");
        done();
    })

    /*
     * @tc.number  deviceManagerTest_off-discoverFail_0100
     * @tc.name    testcase for discoverFail off
     * @tc.desc    Function test
     * @tc.level   2
     */
    it('deviceManagerTest_off-discoverFail_0100', 0, async function (done) {
        console.log("----------start running deviceManagerTest_off-discoverFail_0100----------");
        deviceManager.createDeviceManager('com.ohos.devicemangagerdemo', (err, data) => {
            if (err) {
                expect().assertFail();
                return ;
            }
            var fa = data.off('discoverFail', () => {});
            expect(fa).assertUndefined();
        });
        console.log("----------stop running deviceManagerTest_off-discoverFail_0100----------");
        done();
    })

    /*
     * @tc.number  deviceManagerTest_on-serviceDie_0100
     * @tc.name    testcase for serviceDie on
     * @tc.desc    Function test
     * @tc.level   2
     */
    it('deviceManagerTest_on-serviceDie_0100', 0, async function (done) {
        console.log("----------start running deviceManagerTest_on-serviceDie_0100----------");
        deviceManager.createDeviceManager('com.ohos.devicemangagerdemo', (err, data) => {
            if (err) {
                expect().assertFail();
                return ;
            }
            var fa = data.on('serviceDie', () => {});
            expect(fa).assertUndefined();
        });
        console.log("----------start running deviceManagerTest_on-serviceDie_0100----------");
        done();
    })

    /*
     * @tc.number  deviceManagerTest_off-serviceDie_0100
     * @tc.name    testcase for serviceDie off
     * @tc.desc    Function test
     * @tc.level   2
     */
    it('deviceManagerTest_off-serviceDie_0100', 0, async function (done) {
        console.log("----------start running deviceManagerTest_off-serviceDie_0100----------");
        deviceManager.createDeviceManager('com.ohos.devicemangagerdemo', (err, data) => {
            if (err) {
                expect().assertFail();
                return ;
            }
            var fa = data.off('serviceDie', () => {});
            expect(fa).assertUndefined();
        });
        console.log("----------stop running deviceManagerTest_off-serviceDie_0100----------");
        done();
    })
})