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

import call from '@ohos.telephony.call';
import {describe, beforeAll, beforeEach, afterEach, afterAll, it, expect} from 'deccjsunit/index';

describe('CallManagerMMI', function () {

    /*
     * @tc.number  Telephony_CallManager_MMI_Async_0100
     * @tc.name    PhoneNumber: **21*12599*10# Call dial() to enable unconditional call transfer. The transfer
     *             number is 12599. The callback result is true
     * @tc.desc    Function test
     */
    it('Telephony_CallManager_MMI_Async_0100', 0, async function (done) {
        call.dial('**21*12599*10#', (err, data) => {
            if (err) {
                console.log('Telephony_CallManager_MMI_Async_0100 fail');
                done();
                return;
            }
            expect(data).assertTrue();
            console.log('Telephony_CallManager_MMI_Async_0100 finish data = ' + data);
            done();
        })
        call.dial('##002#', (err, data) => {
            if (err) {
                console.log('Telephony_CallManager_MMI_Async_0100 fail');
                done();
                return;
            }
            expect(data).assertTrue();
            console.log('Telephony_CallManager_MMI_Async_0100 finish data = ' + data);
            done();
        })
    })

    /*
     * @tc.number  Telephony_CallManager_MMI_Async_0200
     * @tc.name    SphoneNumber为**33*0000#Dial () is called to enable limiting all calls. The result of
     *             the callback is true
     * @tc.desc    Function test
     */
    it('Telephony_CallManager_MMI_Async_0200', 0, async function (done) {
        call.dial('**33*0000#', (err, data) => {
            if (err) {
                console.log('Telephony_CallManager_MMI_Async_0200 fail');
                done();
                return;
            }
            expect(data).assertTrue();
            console.log('Telephony_CallManager_MMI_Async_0200 finish data = ' + data);
        })
        call.dial('#33*0000#', (err, data) => {
            if (err) {
                console.log('Telephony_CallManager_MMI_Async_0200 fail');
                done();
                return;
            }
            expect(data).assertTrue();
            console.log('Telephony_CallManager_MMI_Async_0200 finish data = ' + data);
            done();
        })
    })

    /*
     * @tc.number  Telephony_CallManager_MMI_Async_0300
     * @tc.name    SphoneNumber为*331*0000# Dial () is called to enable the restriction of all international
     *             outgoing calls. The result of the callback is true
     * @tc.desc    Function test
     */
    it('Telephony_CallManager_MMI_Async_0300', 0, async function (done) {
        call.dial('*331*0000#', (err, data) => {
            if (err) {
                console.log('Telephony_CallManager_MMI_Async_0300 fail');
                done();
                return;
            }
            expect(data).assertTrue();
            console.log('Telephony_CallManager_MMI_Async_0300 finish data = ' + data);
        })
        call.dial('#331#0000#', (err, data) => {
            if (err) {
                console.log('Telephony_CallManager_MMI_Async_0300 fail');
                done();
                return;
            }
            expect(data).assertTrue();
            console.log('Telephony_CallManager_MMI_Async_0300 finish data = ' + data);
            done();
        })
    })

    /*
     * @tc.number  Telephony_CallManager_MMI_Async_0400
     * @tc.name    If phoneNumber is #35*0000#, dial() is called to turn off the restriction of all incoming calls.
     *             The result of the callback is true
     * @tc.desc    Function test
     */
    it('Telephony_CallManager_MMI_Async_0400', 0, async function (done) {
        call.dial('#35*0000#', (err, data) => {
            if (err) {
                console.log('Telephony_CallManager_MMI_Async_0400 fail');
                done();
                return;
            }
            expect(data).assertTrue();
            console.log('Telephony_CallManager_MMI_Async_0400 finish data = ' + data);
            done();
        })
    })

    /*
     * @tc.number  Telephony_CallManager_MMI_Promise_0100
     * @tc.name   PhoneNumber is ##330* password #, call dial() to close all dials, the callback result is :true
     * @tc.desc    Function test
     */
    it('Telephony_CallManager_MMI_Promise_0100', 0, async function (done) {
        try {
            var data = await call.dial('##330*0000#');
            expect(data).assertTrue();
            console.log('Telephony_CallManager_MMI_Promise_0100 finish data = ' + data);
        } catch (err) {
            console.log('Telephony_CallManager_MMI_Promise_0100 fail');
            expect().assertFail();
            done();
        }
        done();
    })

    /*
     * @tc.number  Telephony_CallManager_MMI_Promise_0200
     * @tc.name    If phoneNumber is *#330#0000# password #, dial() is used to query all incoming calls. The callback
     *             result is true
     * @tc.desc    Function test
     */
    it('Telephony_CallManager_MMI_Promise_0200', 0, async function (done) {
        try {
            var data = await call.dial('*#330#0000#');
            expect(data).assertTrue();
            console.log('Telephony_CallManager_MMI_Promise_0200 finish data = ' + data);
        } catch (err) {
            expect().assertFail();
            console.log('Telephony_CallManager_MMI_Promise_0200 fail');
            done();
        }
        done();
    })

    /*
     * @tc.number  Telephony_CallManager_MMI_Promise_0300
     * @tc.name    If phoneNumber is *#31#, dial() is called to query the caller ID. The callback result is true
     * @tc.desc    Function test
     */
    it('Telephony_CallManager_MMI_Promise_0300', 0, async function (done) {
        try {
            var data = await call.dial('*#31#');
            expect(data).assertTrue();
            console.log('Telephony_CallManager_MMI_Promise_0300 finish data = ' + data);
        } catch (err) {
            expect().assertFail();
            console.log('Telephony_CallManager_MMI_Promise_0300 fail');
            done();
        }
        done();
    })

    /*
     * @tc.number  Telephony_CallManager_MMI_Promise_0400
     * @tc.name    If phoneNumber is #31#, call dial() to hide the caller ID. The callback result is true
     * @tc.desc    Function test
     */
    it('Telephony_CallManager_MMI_Promise_0400', 0, async function (done) {
        try {
            var data = await call.dial('#31#');
            expect(data).assertTrue();
            console.log('Telephony_CallManager_MMI_Promise_0400 finish data = ' + data);
        } catch (err) {
            expect().assertFail();
            console.log('Telephony_CallManager_MMI_Promise_0400 fail');
            done();
        }
        try {
            var data = await call.dial('*31#');
            expect(data).assertTrue();
            console.log('Telephony_CallManager_MMI_Promise_0400 finish data = ' + data);
        } catch (err) {
            expect().assertFail();
            console.log('Telephony_CallManager_MMI_Promise_0400 fail');
            done();
        }
        done();
    })
})
