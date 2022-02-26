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
import resmgr from '@ohos.resourceManager'
import {describe, beforeAll, beforeEach, afterEach, afterAll, it, expect} from 'deccjsunit/index'

describe('resMgrTest', function () {
    console.log('*************start ResmgrTest*************');

    /* *
    * @tc.number SUB_GLOBAL_RESMGR_JS_0100
    * @tc.name test getResourceManager method in callback mode
    * @tc.desc get the resource manager in callback mode
    */
    it('getResourceManager_test_001', 0, async function (done) {
        resmgr.getResourceManager((error, mgr) => {
            expect(mgr !== null).assertTrue();
        })
        done();
    })

    /* *
    * @tc.number SUB_GLOBAL_RESMGR_JS_0200
    * @tc.name test getResourceManager method in callback mode with bundlename
    * @tc.desc get the resource manager in callback mode with bundlename
    */
    it('getResourceManager_test_002', 0, async function (done) {
        resmgr.getResourceManager('com.ohos.settings', (error, mgr) => {
            expect(mgr !== null).assertTrue();
        })
        done();
    })

    /* *
    * @tc.number SUB_GLOBAL_RESMGR_JS_0300
    * @tc.name test getResourceManager method in promise mode
    * @tc.desc get the resource manager in promise mode
    */
    it('getResourceManager_test_003', 0, async function (done) {
        resmgr.getResourceManager().then(mgr => {
            expect(mgr !== null).assertTrue();
        })
        done();
    })

    /* *
    * @tc.number SUB_GLOBAL_RESMGR_JS_0400
    * @tc.name test getResourceManager method in promise mode with bundlename
    * @tc.desc get the resource manager in promise mode with bundlename
    */
    it('getResourceManager_test_004', 0, async function (done) {
        resmgr.getResourceManager('com.ohos.settings').then(mgr => {
            expect(mgr !== null).assertTrue();
        })
        done();
    })

    /* *
    * @tc.number SUB_GLOBAL_RESMGR_JS_0500
    * @tc.name test getString method in callback mode
    * @tc.desc get the string in callback mode
    */
    it('getString_test_001', 0, async function (done) {
        resmgr.getResourceManager((error, mgr) => {
            mgr.getString(0x1000000, (err, value) => {
                expect(value !== null).assertTrue();
            })
        })
        done();
    })

    /* *
    * @tc.number SUB_GLOBAL_RESMGR_JS_0600
    * @tc.name test getString method in promise mode
    * @tc.desc get the string in promise mode
    */
    it('getString_test_002', 0, async function (done) {
        resmgr.getResourceManager((error, mgr) => {
            mgr.getString(0x1000000).then(value => {
                expect(value !== null).assertTrue();
            })
        })
        done();
    })

    /* *
    * @tc.number SUB_GLOBAL_RESMGR_JS_0700
    * @tc.name test getStringArray method in callback mode
    * @tc.desc get the string array in callback mode
    */
    it('getStringArray_test_001', 0, async function (done) {
        resmgr.getResourceManager((error, mgr) => {
            mgr.getStringArray(0x1000002, (err, value) => {
                expect(value !== null).assertTrue();
            })
        })
        done();
    })

    /* *
    * @tc.number SUB_GLOBAL_RESMGR_JS_0800
    * @tc.name test getStringArray method in promise mode
    * @tc.desc get the string array in promise mode
    */
    it('getStringArray_test_002', 0, async function (done) {
        resmgr.getResourceManager((error, mgr) => {
            mgr.getStringArray(0x1000002).then(value => {
                expect(value !== null).assertTrue();
            })
        })
        done();
    })

    /* *
    * @tc.number SUB_GLOBAL_RESMGR_JS_0900
    * @tc.name test getMedia method in callback mode
    * @tc.desc get the media in callback mode
    */
    it('getMedia_test_001', 0, async function (done) {
        resmgr.getResourceManager((error, mgr) => {
            mgr.getMedia(0x1000004, (err, value) => {
                expect(value.length > 0).assertTrue();
            })
        })
        done();
    })

    /* *
    * @tc.number SUB_GLOBAL_RESMGR_JS_1000
    * @tc.name test getMedia method in promise mode
    * @tc.desc get the media in promise mode
    */
    it('getMedia_test_002', 0, async function (done) {
        resmgr.getResourceManager((error, mgr) => {
            mgr.getMedia(0x1000004).then(value => {
                expect(value.length > 0).assertTrue();
            })
        })
        done();
    })

    /* *
    * @tc.number SUB_GLOBAL_RESMGR_JS_1100
    * @tc.name test getMediaBase64 method in callback mode
    * @tc.desc get the media base64 in callback mode
    */
    it('getMediaBase64_test_001', 0, async function (done) {
        resmgr.getResourceManager((error, mgr) => {
            mgr.getMediaBase64(0x1000004, (err, value) => {
                expect(value.length > 0).assertTrue();
            })
        })
        done();
    })

    /* *
    * @tc.number SUB_GLOBAL_RESMGR_JS_1200
    * @tc.name test getMediaBase64 method in promise mode
    * @tc.desc get the media base64 in promise mode
    */
    it('getMediaBase64_test_002', 0, async function (done) {
        resmgr.getResourceManager((error, mgr) => {
            mgr.getMediaBase64(0x1000004).then(value => {
                expect(value.length > 0).assertTrue();
            })
        })
        done();
    })

    /* *
    * @tc.number SUB_GLOBAL_RESMGR_JS_1300
    * @tc.name test getConfiguration method in callback mode
    * @tc.desc get the configuration in callback mode
    */
    it('getConfiguration_test_001', 0, async function (done) {
        resmgr.getResourceManager((error, mgr) => {
            mgr.getConfiguration((error, cfg) => {
                expect(cfg !== null).assertTrue();
            })
        })
        done();
    })

    /* *
    * @tc.number SUB_GLOBAL_RESMGR_JS_1400
    * @tc.name test getConfiguration method in promise mode
    * @tc.desc get the configuration in promise mode
    */
    it('getConfiguration_test_002', 0, async function (done) {
        resmgr.getResourceManager((error, mgr) => {
            mgr.getConfiguration().then(cfg => {
                expect(cfg !== null).assertTrue();
            })
        })
        done();
    })

    /* *
    * @tc.number SUB_GLOBAL_RESMGR_JS_1500
    * @tc.name test getDeviceCapability method in callback mode
    * @tc.desc get the device capability in callback mode
    */
    it('getDeviceCapability_test_001', 0, async function (done) {
        resmgr.getResourceManager((error, mgr) => {
            mgr.getDeviceCapability((error, dc) => {
                expect(dc !== null).assertTrue();
            })
        })
        done();
    })

    /* *
    * @tc.number SUB_GLOBAL_RESMGR_JS_1600
    * @tc.name test getDeviceCapability method in promise mode
    * @tc.desc get the device capability in promise mode
    */
    it('getDeviceCapability_test_002', 0, async function (done) {
        resmgr.getResourceManager((error, mgr) => {
            mgr.getDeviceCapability().then(dc => {
                expect(dc !== null).assertTrue();
            })
        })
        done();
    })

    /* *
    * @tc.number SUB_GLOBAL_RESMGR_JS_1700
    * @tc.name test getPluralString method in callback mode
    * @tc.desc get the plural string in callback mode
    */
    it('getPluralString_test_001', 0, async function (done) {
        resmgr.getResourceManager((error, mgr) => {
            mgr.getPluralString(0x1000003, 1, (error, value) => {
                expect(value !== null).assertTrue();
            })
        })
        done();
    })

    /* *
    * @tc.number SUB_GLOBAL_RESMGR_JS_1800
    * @tc.name test getPluralString method in promise mode
    * @tc.desc get the plural string in promise mode
    */
    it('getPluralString_test_002', 0, async function (done) {
        resmgr.getResourceManager((error, mgr) => {
            mgr.getPluralString(0x1000003, 1).then(value => {
                expect(value !== null).assertTrue();
            })
        })
        done();
    })
    console.log('*************end ResmgrTest*************');
}) 