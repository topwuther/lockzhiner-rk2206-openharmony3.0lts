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

const BUNDLE_NAME = 'com.example.actsbmscheckpermissiontest';
const PERMISSION_ONE = 'com.permission.CAMERA';
const PERMISSION_TWO = 'com.permission.PERMISSION_A';
const TIMEOUT = 1000;
const NAMECOUNT = 10000;

describe('ActsBmsCheckPermissionTest', function () {

    /*
    * @tc.number: bms_checkPermission_0100
    * @tc.name: check whether the permission is granted
    * @tc.desc: verify the requested permission whose availableScope is system_grant whether is granted. (by promise)
    */
    it('bms_checkPermission_0100', 0, async function (done) {
        console.info('=====================bms_checkPermission_0100==================');
        var data = await bundle.checkPermission(BUNDLE_NAME, PERMISSION_TWO)
        expect(data).assertEqual(0);
        done();
        setTimeout(function () {
            console.info('=====================bms_checkPermission_0100==================end');
        }, TIMEOUT)
    })

    /*
    * @tc.number: bms_checkPermission_0200
    * @tc.name: check whether the permission is granted
    * @tc.desc: verify the requested permission whose availableScope is system_grant whether is granted. (by callback)
    */
    it('bms_checkPermission_0200', 0, async function (done) {
        console.info('=====================bms_checkPermission_0200==================');
        await bundle.checkPermission(BUNDLE_NAME, PERMISSION_TWO, (err, data) => {
            expect(err.code).assertEqual(0);
            expect(data).assertEqual(0);
            done();
        })
        setTimeout(function () {
            console.info('=====================bms_checkPermission_0200==================end');
        }, TIMEOUT)
    })

    /*
    * @tc.number: bms_checkPermission_0300
    * @tc.name: check whether the permission is granted
    * @tc.desc: verify the requested permission whether is granted, which by calling the checkpermission
    *           function and the bundleName is invalid. (by promise)
    */
    it('bms_checkPermission_0300', 0, async function (done) {
        console.info('=====================bms_checkPermission_0300==================');
        var data = await bundle.checkPermission('', PERMISSION_ONE)
        expect(data).assertEqual(-1);
        done();
        setTimeout(function () {
            console.info('=====================bms_checkPermission_0300==================end');
        }, TIMEOUT)
    })

    /*
    * @tc.number: bms_checkPermission_0400
    * @tc.name: check whether the permission is granted
    * @tc.desc: verify the requested permission whether is granted, which by calling the checkpermission
    *           function and the bundleName is invalid. (by callback)
    */
    it('bms_checkPermission_0400', 0, async function (done) {
        console.info('=====================bms_checkPermission_0400==================');
        await bundle.checkPermission('', PERMISSION_ONE, (err, data) => {
            expect(err.code).assertEqual(-1);
            expect(data).assertEqual(-1);
            done();
        })
        setTimeout(function () {
            console.info('=====================bms_checkPermission_0400==================end');
        }, TIMEOUT)
    })

    /*
    * @tc.number: bms_checkPermission_0500
    * @tc.name: check whether the permission is granted
    * @tc.desc: verify the requested permission whether is granted, which by calling the checkpermission
    *           function and the permissionName is invalid. (by promise)
    */
    it('bms_checkPermission_0500', 0, async function (done) {
        console.info('=====================bms_checkPermission_0500==================');
        var data = await bundle.checkPermission(BUNDLE_NAME, '')
        expect(data).assertEqual(-1);
        done();
        setTimeout(function () {
            console.info('=====================bms_checkPermission_0500==================end');
        }, TIMEOUT)
    })

    /*
    * @tc.number: bms_checkPermission_0600
    * @tc.name: check whether the permission is granted
    * @tc.desc: verify the requested permission whether is granted, which by calling the checkpermission
    *           function and the permissionName is invalid. (by callback)
    */
    it('bms_checkPermission_0600', 0, async function (done) {
        console.info('=====================bms_checkPermission_0600==================');
        await bundle.checkPermission(BUNDLE_NAME, '', (err, data) => {
            expect(err.code).assertEqual(-1);
            expect(data).assertEqual(-1);
            done();
        })
        setTimeout(function () {
            console.info('=====================bms_checkPermission_0600==================end');
        }, TIMEOUT)
    })

    /*
       * @tc.number: bms_checkPermission_0700
       * @tc.name: check whether the permission is granted
       * @tc.desc: verify the requested permission whether is granted, which by calling the checkpermission
       *           function and the permissionName is invalid. (by promise)
       */
    it('bms_checkPermission_0700', 0, async function (done) {
        console.info('=====================bms_checkPermission_0700==================');
        var data = await bundle.checkPermission(BUNDLE_NAME, '@\n,. 。测试@abc1^%12345')
        expect(data).assertEqual(-1);
        done();
        setTimeout(function () {
            console.info('=====================bms_checkPermission_0700==================end');
        }, TIMEOUT)
    })

    /*
    * @tc.number: bms_checkPermission_0800
    * @tc.name: check whether the permission is granted
    * @tc.desc: verify the requested permission whether is granted, which by calling the checkpermission
    *           function and the permissionName is invalid. (by callback)
    */
    it('bms_checkPermission_0800', 0, async function (done) {
        console.info('=====================bms_checkPermission_0800==================');
        await bundle.checkPermission(BUNDLE_NAME, '@\n,. 。测试@abc1^%12345', (err, data) => {
            expect(err.code).assertEqual(-1);
            expect(data).assertEqual(-1);
            done();
        })
        setTimeout(function () {
            console.info('=====================bms_checkPermission_0800==================end');
        }, TIMEOUT)
    })

    /*
    * @tc.number: bms_checkPermission_0900
    * @tc.name: check whether the permission is granted
    * @tc.desc: verify the requested permission whether is granted, which by calling the checkpermission
    *           function and the bundleName is invalid. (by callback)
    */
    it('bms_checkPermission_0900', 0, async function (done) {
        console.info('=====================bms_checkPermission_0900==================');
        var data = await bundle.checkPermission('@\n,. 。测试@abc1^%12345', PERMISSION_ONE)
        expect(data).assertEqual(-1);
        done();
        setTimeout(function () {
            console.info('=====================bms_checkPermission_0900==================end');
        }, TIMEOUT)
    })

    /*
    * @tc.number: bms_checkPermission_1000
    * @tc.name: check whether the permission is granted
    * @tc.desc: verify the requested permission whether is granted, which by calling the checkpermission
    *           function and the bundleName is invalid. (by callback)
    */
    it('bms_checkPermission_1000', 0, async function (done) {
        console.info('=====================bms_checkPermission_1000==================');
        await bundle.checkPermission('@\n,. 。测试@abc1^%12345', PERMISSION_ONE, (err, data) => {
            expect(err.code).assertEqual(-1);
            expect(data).assertEqual(-1);
            done();
        })
        setTimeout(function () {
            console.info('=====================bms_checkPermission_1000==================end');
        }, TIMEOUT)
    })

    /*
    * @tc.number: bms_checkPermission_1100
    * @tc.name: check whether the permission is granted
    * @tc.desc: verify the requested permission whether is granted, which by calling the checkpermission
    *           function and the permissionName is invalid. (by promise)
    */
    it('bms_checkPermission_1100', 0, async function (done) {
        console.info('=====================bms_checkPermission_1100==================');
        var permissionName = 'test';
        for (var i = 0; i < NAMECOUNT; i++) {
            permissionName += 'test';
        }
        var data = await bundle.checkPermission(BUNDLE_NAME, permissionName)
        expect(data).assertEqual(-1);
        done();
        setTimeout(function () {
            console.info('=====================bms_checkPermission_1100==================end');
        }, TIMEOUT)
    })

    /*
    * @tc.number: bms_checkPermission_1200
    * @tc.name: check whether the permission is granted
    * @tc.desc: verify the requested permission whether is granted, which by calling the checkpermission
    *           function and the permissionName is invalid. (by callback)
    */
    it('bms_checkPermission_1200', 0, async function (done) {
        console.info('=====================bms_checkPermission_1200==================');
        var permissionName = 'test';
        for (var i = 0; i < NAMECOUNT; i++) {
            permissionName += 'test';
        }
        await bundle.checkPermission(BUNDLE_NAME, permissionName, (err, data) => {
            expect(err.code).assertEqual(-1);
            expect(data).assertEqual(-1);
            done();
        })
        setTimeout(function () {
            console.info('=====================bms_checkPermission_1200==================end');
        }, TIMEOUT)
    })

    /*
    * @tc.number: bms_checkPermission_1300
    * @tc.name: check whether the permission is granted
    * @tc.desc: verify the requested permission whether is granted, which by calling the checkpermission
    *           function and the bundleName is invalid. (by promise)
    */
    it('bms_checkPermission_1300', 0, async function (done) {
        console.info('=====================bms_checkPermission_1300==================');
        var bundleName = 'test';
        for (var i = 0; i < NAMECOUNT; i++) {
            bundleName += 'test';
        }
        var data = await bundle.checkPermission(bundleName, PERMISSION_ONE)
        expect(data).assertEqual(-1);
        done();
        setTimeout(function () {
            console.info('=====================bms_checkPermission_1300==================end');
        }, TIMEOUT)
    })

    /*
    * @tc.number: bms_checkPermission_1400
    * @tc.name: check whether the permission is granted
    * @tc.desc: verify the requested permission whether is granted, which by calling the checkpermission
    *           function and the bundleName is invalid. (by callback)
    */
    it('bms_checkPermission_1400', 0, async function (done) {
        console.info('=====================bms_checkPermission_1400==================');
        var bundleName = 'test';
        for (var i = 0; i < NAMECOUNT; i++) {
            bundleName += 'test';
        }
        await bundle.checkPermission(bundleName, PERMISSION_ONE, (err, data) => {
            expect(err.code).assertEqual(-1);
            expect(data).assertEqual(-1);
            done();
        })
        setTimeout(function () {
            console.info('=====================bms_checkPermission_1400==================end');
        }, TIMEOUT)
    })
})