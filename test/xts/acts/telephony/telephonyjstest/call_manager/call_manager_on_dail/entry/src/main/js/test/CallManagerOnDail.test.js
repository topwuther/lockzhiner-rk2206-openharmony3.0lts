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
import { describe, beforeAll, beforeEach, afterEach, afterAll, it, expect } from 'deccjsunit/index';
describe('CallManagerOnDial', function () {
    const TEST_PHONY_NUMBER = '10086';
    var callId = 0;
    var callState = -1;
    var timeOut = 0;
    var endTime = 0;
    var onTime = 25000;
    var newCallId = 0
    var waitTime = 200;
    const INVALID_NUMBER = '13800000000'
    const GETMAIN_CALLID_ERRO = -1;

    const sleep = (time) => {
        return new Promise((resolve, reject) => {
            setTimeout(() => {
                resolve()
            }, time )
        })
    }

    beforeAll(async function () {
        call.on('callDetailsChange', (err, callStateInfo) => {
            if (err) {
                console.log('Telephony_CallManager_on_Async fail err = ' + err);
                return;
            }
            console.log('Telephony_CallManager_callDetailsChange: ' + JSON.stringify(callStateInfo));
            callId = callStateInfo.callId;
            callState = callStateInfo.callState;
        })
    })

    afterAll(async function () {
        call.off('callDetailsChange', (err) => {
            if (err) {
                console.log('Telephony_CallManager_off_Async fail err = ' + err);
            }
        })
    })

    afterEach(async function () {
        const startTime = new Date().getTime();
        while(true){
            if (callId > newCallId) {
                if (callState === call.CALL_STATUS_DISCONNECTED) {
                    return;
                }
                newCallId = callId
                console.log('Telephony_CallManager_hangup_Async callId = ' + callId);
                call.hangup(callId, async (err) => {
                    if (err) {
                        console.log('Telephony_CallManager_hangup_Async fail err  = ' + err);
                        return;
                    }
                    console.log('Telephony_CallManager_hangup_Async finish');
                    return;
                })
            }
            await sleep(waitTime);
            endTime = new Date().getTime();
            timeOut = endTime - startTime;
            if (timeOut > onTime) {
                console.log("Telephony_CallManager_hangup is out of time");
                break;

            }
        }
    })

    /*
     * @tc.number  Telephony_CallManager_dial_Async_0100
     * @tc.name    Set the mandatory phone number to a normal number (10086) and call dial() as a callback,
     *             returning :true
     * @tc.desc    Function test
     */
    it('Telephony_CallManager_dial_Async_0100', 0, async function (done) {
        call.dial(TEST_PHONY_NUMBER, (err, data) => {
            if (err) {
                console.log('Telephony_CallManager_dial_Async_0100 fail' + err);
                expect().assertFail();
                done();
                return;
            }
            expect(data).assertTrue();
            console.log('Telephony_CallManager_dial_Async_0100 dial finish data = ' + data);
        })
        const startTime = new Date().getTime();
        while (true) {
            if (callState === call.CALL_STATUS_DIALING || callState === call.CALL_STATUS_ACTIVE) {
                done();
                return;
            }
            await sleep(waitTime);
            endTime = new Date().getTime();
            timeOut = endTime - startTime;
            if (timeOut > onTime) {
                done();
                break;
            }
        }
    })

    /*
     * @tc.number  Telephony_CallManager_dial_Async_0400
     * @tc.name    Set the mandatory phone number to a phone number with a ',' callback that calls dial(),
     *             returning :true
     * @tc.desc    Function test
     */
    it('Telephony_CallManager_dial_Async_0400', 0, async function (done) {
        call.dial('10086,1234567890123456789123', { accountId: 0 }, (err, data) => {
            if (err) {
                console.log('Telephony_CallManager_dial_Async_0400 fail err = ' + err);
                done();
                return;
            }
            expect(data).assertTrue();
            console.log('Telephony_CallManager_dial_Async_0400 finish data = ' + data);
            done();
        })
        const startTime = new Date().getTime();
        while (true) {
            if (callState === call.CALL_STATUS_DIALING || callState === call.CALL_STATUS_ACTIVE) {
                done();
                return;
            }
            await sleep(waitTime);
            endTime = new Date().getTime();
            timeOut = endTime - startTime;
            if (timeOut > onTime) {
                done();
                break;
            }
        }
    })

    /*
     * @tc.number  Telephony_CallManager_dial_Async_0500
     * @tc.name    Set the mandatory phone number to an invalid number (abcDE123456),
     *             call dial() in callback mode, and return :true
     * @tc.desc    Function test
     */
    it('Telephony_CallManager_dial_Async_0500', 0, async function (done) {
        call.dial('abcde123456', (err, data) => {
            if (err) {
                console.log('Telephony_CallManager_dial_Async_0500 fail err = ' + err);
                expect().assertFail();
                done();
                return;
            }
            expect(data).assertTrue();
            console.log('Telephony_CallManager_dial_Async_0500 finish data = ' + data);
        })
        const startTime = new Date().getTime();
        while (true) {
            if (callState === call.CALL_STATUS_DIALING || callState === call.CALL_STATUS_ACTIVE) {
                done();
                return;
            }
            await sleep(waitTime);
            endTime = new Date().getTime();
            timeOut = endTime - startTime;
            if (timeOut > onTime) {
                done();
                break;
            }
        }
    })

    /*
     * @tc.number  Telephony_CallManager_dial_Async_0800
     * @tc.name    Set the mandatory phone number to a normal number (10086) and the optional parameters options
     *             to {accountId: 0, videoState: 0, dialScene: 0, dialType: 0}. Dial () in callback mode and return
     *             :true
     * @tc.desc    Function test
     */
    it('Telephony_CallManager_dial_Async_0800', 0, async function (done) {
        let obj = { accountId: 0, videoState: 0, dialScene: 0, dialType: 0 };
        call.dial('abcedf123456', obj, (err, data) => {
            if (err) {
                expect().assertFail();
                console.log('Telephony_CallManager_dial_Async_0800 fail err = ' + err);
                done();
                return;
            }
            expect(data).assertTrue();
            console.log('Telephony_CallManager_dial_Async_0800 finish data = ' + data);
        })
        const startTime = new Date().getTime();
        while (true) {
            if (callState === call.CALL_STATUS_DIALING || callState === call.CALL_STATUS_ACTIVE) {
                done();
                return;
            }
            await sleep(waitTime);
            endTime = new Date().getTime();
            timeOut = endTime - startTime;
            if (timeOut > onTime) {
                done();
                break;
            }
        }
    })

    /*
     * @tc.number  Telephony_CallManager_dial_Async_1200
     * @tc.name    Call dial() by way of callback, phoneNumber is a normal number, options is ''
     * @tc.desc    Function test
     */
    it('Telephony_CallManager_dial_Async_1200', 0, async function (done) {
        call.dial(INVALID_NUMBER, (err, data) => {
            if (err) {
                console.log('Telephony_CallManager_dial_Async_1200 fail err = ' + err);
                expect().assertFail();
                done();
                return;
            }
            expect(data).assertTrue();
            console.log('Telephony_CallManager_dial_Async_1200 dial finish data = ' + data);
        })
        const startTime = new Date().getTime();
        while (true) {
            if (callState === call.CALL_STATUS_DIALING || callState === call.CALL_STATUS_ACTIVE) {
                done();
                return;
            }
            await sleep(waitTime);
            endTime = new Date().getTime();
            timeOut = endTime - startTime;
            if (timeOut > onTime) {
                done();
                break;
            }
        }
    })

    /*
     * @tc.number  Telephony_CallManager_dial_Async_1300
     * @tc.name    Set the mandatory phone number to a normal number (112) and the optional parameters options to
     *             {accountId: 0, videoState: 0, dialScene: 0, dialType: 0}. Dial () in callback mode and return :true
     * @tc.desc    Function test
     */
    it('Telephony_CallManager_dial_Async_1300', 0, async function (done) {
        let obj = { accountId: 0, videoState: 0, dialScene: 0, dialType: 0 };
        call.dial('112', obj, (err, data) => {
            if (err) {
                console.log('Telephony_CallManager_dial_Async_1300 fail err = ' + err);
                expect().assertFail();
                done();
                return;
            }
            expect(data).assertTrue();
            console.log('Telephony_CallManager_dial_Async_1300 dial finish data = ' + data);
        })
        const startTime = new Date().getTime();
        while (true) {
            if (callState === call.CALL_STATUS_DIALING || callState === call.CALL_STATUS_ACTIVE) {
                done();
                return;
            }
            await sleep(waitTime);
            endTime = new Date().getTime();
            timeOut = endTime - startTime;
            if (timeOut > onTime) {
                done();
                break;
            }
        }
    })

    /*
     * @tc.number  Telephony_CallManager_dial_Promise_0100
     * @tc.name    Set the mandatory phone number to a normal number (10086) and the optional parameters options to
     *             {accountId: 0, videoState: 0, dialScene: 0, dialType: 0}. Call dial() and return :true
     * @tc.desc    Function test
     */
    it('Telephony_CallManager_dial_Promise_0100', 0, async function (done) {
        try {
            var data = await call.dial('123456789123456789', { accountId: 0 })
            expect(data).assertTrue();
            console.log('Telephony_CallManager_dial_Promise_0100 finish data = ' + data);
            const startTime = new Date().getTime();
            while (true) {
                if (callState === call.CALL_STATUS_DIALING || callState === call.CALL_STATUS_ACTIVE) {
                    done();
                    return;
                }
                await sleep(waitTime);
                endTime = new Date().getTime();
                timeOut = endTime - startTime;
                if (timeOut > onTime) {
                    done();
                    break;
                }
            }
        } catch (err) {
            expect().assertFail();
            console.log('Telephony_CallManager_dial_Promise_0100 fail err = ' + err);
            done();
        }
    })

    /*
     * @tc.number  Telephony_CallManager_dial_Promise_0300
     * @tc.name    Set the mandatory phone number to abcDE123456 and optional parameters options to
     *             {accountId: 0, videoState: 0, dialScene: 0, dialType:  0},call dial() to dial, return :true
     * @tc.desc    Function test
     */
    it('Telephony_CallManager_dial_Promise_0300', 0, async function (done) {
        try {
            var data = await call.dial('abcde123456', { accountId: 0 })
            expect(data).assertTrue();
            console.log('Telephony_CallManager_dial_Promise_0300 finish data = ' + data);
        } catch (err) {
            console.log('Telephony_CallManager_dial_Promise_0300 fail err = ' + err);
            expect().assertFail();
            done();
            return;
        }
        const startTime = new Date().getTime();
        while (true) {
            if (callState === call.CALL_STATUS_DIALING || callState === call.CALL_STATUS_ACTIVE) {
                done();
                return;
            }
            await sleep(waitTime);
            endTime = new Date().getTime();
            timeOut = endTime - startTime;
            if (timeOut > onTime) {
                done();
                break;
            }
        }
    })

    /*
     * @tc.number  Telephony_CallManager_dial_Promise_0600
     * @tc.name    Set the required phone number to a normal number (10086) with no optional arguments and call
     *             dial() to dial, returning :true
     * @tc.desc    Function test
     */
    it('Telephony_CallManager_dial_Promise_0600', 0, async function (done) {
        try {
            var data = await call.dial(TEST_PHONY_NUMBER, { accountId: 0 })
            console.log('Telephony_CallManager_dial_Promise_0600 finish data = ' + data);
            expect(data).assertTrue();
        } catch (err) {
            console.log('Telephony_CallManager_dial_Promise_0600 fail err = ' + err);
            expect().assertFail();
            done();
            return;
        }
        const startTime = new Date().getTime();
        while (true) {
            if (callState === call.CALL_STATUS_DIALING || callState === call.CALL_STATUS_ACTIVE) {
                done();
                return;
            }
            await sleep(waitTime);
            endTime = new Date().getTime();
            timeOut = endTime - startTime;
            if (timeOut > onTime) {
                done();
                break;
            }
        }
    })

    /*
     * @tc.number  Telephony_CallManager_dial_Promise_0700
     * @tc.name    Example Set the required phone number to a normal number INVALID_NUMBER. Options are
     *             {accountId: 0, videoState: 0, dialScene: 0, dialType:  0}, call dial() to dial, returning :true.
     * @tc.desc    Function test
     */
    it('Telephony_CallManager_dial_Promise_0700', 0, async function (done) {
        try {
            var data = await call.dial(INVALID_NUMBER, { accountId: 0 })
            console.log('Telephony_CallManager_dial_Promise_0700 finish data = ' + data);
            expect(data).assertTrue();
        } catch (err) {
            console.log('Telephony_CallManager_dial_Promise_0700 fail err = ' + err);
            expect().assertFail();
            done();
            return;
        }
        const startTime = new Date().getTime();
        while (true) {
            if (callState === call.CALL_STATUS_DIALING || callState === call.CALL_STATUS_ACTIVE) {
                done();
                return;
            }
            await sleep(waitTime);
            endTime = new Date().getTime();
            timeOut = endTime - startTime;
            if (timeOut > onTime) {
                done();
                break;
            }
        }
    })

    /*
     * @tc.number  Telephony_CallManager_dial_Promise_0800
     * @tc.name    Set the required phone number to a normal number (112) without the optional parameters,
     *             and call dial() to dial, returning :true
     * @tc.desc    Function test
     */
    it('Telephony_CallManager_dial_Promise_0800', 0, async function (done) {
        try {
            var data = await call.dial('112');
            expect(data).assertTrue();
            console.log('Telephony_CallManager_dial_Promise_0800 finish data = ' + data);
        } catch (err) {
            console.log('Telephony_CallManager_dial_Promise_0800 fail err = ' + err);
            expect().assertFail();
            done();
            return;
        }
        const startTime = new Date().getTime();
        while (true) {
            if (callState === call.CALL_STATUS_DIALING || callState === call.CALL_STATUS_ACTIVE) {
                done();
                return;
            }
            await sleep(waitTime);
            endTime = new Date().getTime();
            timeOut = endTime - startTime;
            if (timeOut > onTime) {
                done();
                break;
            }
        }
    })

    /*
     * @tc.number  Telephony_CallManager_dial_Promise_1000
     * @tc.name    Will choose parameter number add ', '(10086123567901345789123), with no optional parameters,
     *             call dial () to dial, return: true
     * @tc.desc    Function test
     */
    it('Telephony_CallManager_dial_Promise_1000', 0, async function (done) {
        try {
            var data = await call.dial('10086,1234567890123456789123');
            expect(data).assertTrue();
            console.log('Telephony_CallManager_dial_Promise_1000 finish data = ' + data);
        } catch (err) {
            console.log('Telephony_CallManager_dial_Promise_1000 fail err = ' + err);
            expect().assertFail();
            done();
            return;
        }
        const startTime = new Date().getTime();
        while (true) {
            if (callState === call.CALL_STATUS_DIALING || callState === call.CALL_STATUS_ACTIVE) {
                done();
                return;
            }
            await sleep(waitTime);
            endTime = new Date().getTime();
            timeOut = endTime - startTime;
            if (timeOut > onTime) {
                done();
                break;
            }
        }
    })

    /*
     * @tc.number  Telephony_CallManager_dial_Promise_1100
     * @tc.name    This parameter is mandatory. The phone number is 00000000.
     *             Dial () with no optional parameters returns true
     * @tc.desc    Function test
     */
    it('Telephony_CallManager_dial_Promise_1100', 0, async function (done) {
        try {
            var data = await call.dial('00000000');
            expect(data).assertTrue();
            console.log('Telephony_CallManager_dial_Promise_1100 finish data = ' + data);
        } catch (err) {
            console.log('Telephony_CallManager_dial_Promise_1100 fail err = ' + err);
            expect().assertFail();
            done();
            return;
        }
        const startTime = new Date().getTime();
        while (true) {
            if (callState === call.CALL_STATUS_DIALING || callState === call.CALL_STATUS_ACTIVE) {
                done();
                return;
            }
            await sleep(waitTime);
            endTime = new Date().getTime();
            timeOut = endTime - startTime;
            if (timeOut > onTime) {
                done();
                break;
            }
        }
    })

    /*
     * @tc.number  Telephony_CallManager_dial_Promise_1200
     * @tc.name    The phone number is 000000000000000000. Dial () without any optional parameters.
     *             True is returned
     * @tc.desc    Function test
     */
    it('Telephony_CallManager_dial_Promise_1200', 0, async function (done) {
        try {
            var data = await call.dial('000000000000000000');
            expect(data).assertTrue();
            console.log('Telephony_CallManager_dial_Promise_1200 finish data = ' + data);
        } catch (err) {
            console.log('Telephony_CallManager_dial_Promise_1200 fail err = ' + err);
            expect().assertFail();
            done();
            return;
        }
        const startTime = new Date().getTime();
        while (true) {
            if (callState === call.CALL_STATUS_DIALING || callState === call.CALL_STATUS_ACTIVE) {
                done();
                return;
            }
            await sleep(waitTime);
            endTime = new Date().getTime();
            timeOut = endTime - startTime;
            if (timeOut > onTime) {
                done();
                break;
            }
        }
    })

    /*
     * @tc.number  Telephony_CallManager_dial_Promise_1300
     * @tc.name    The phone number is the area code (0898) and the number is 0000 0000. Dial () without optional
     *             parameters returns true
     * @tc.desc    Function test
     */
    it('Telephony_CallManager_dial_Promise_1300', 0, async function (done) {
        try {
            var data = await call.dial('0898 0000 0000');
            expect(data).assertTrue();
            console.log('Telephony_CallManager_dial_Promise_1300 finish data = ' + data);
        } catch (err) {
            console.log('Telephony_CallManager_dial_Promise_1300 fail err = ' + err);
            expect().assertFail();
            done();
            return;
        }
        const startTime = new Date().getTime();
        while (true) {
            if (callState === call.CALL_STATUS_DIALING || callState === call.CALL_STATUS_ACTIVE) {
                done();
                return;
            }
            await sleep(waitTime);
            endTime = new Date().getTime();
            timeOut = endTime - startTime;
            if (timeOut > onTime) {
                done();
                break;
            }
        }
    })

    /*
     * @tc.number  Telephony_CallManager_switchCall_Async_0400
     * @tc.name    The call is connected, passing in the current call ID, calling switchCall() to enable the call,
     *             or calling switchCall() to enable the call, with no return value
     * @tc.desc    Function test
     */
    it('Telephony_CallManager_switchCall_Async_0400', 0, async function (done) {
        call.dial(TEST_PHONY_NUMBER, async (err, data) => {
            if (err) {
                expect().assertFail();
                console.log('Telephony_CallManager_switchCall_Async_0400 dial fail err = ' + err);
                done();
                return;
            }
            expect(data).assertTrue();
            console.log('Telephony_CallManager_switchCall_Async_0400 dial finish data = ' + data);
            const startTime = new Date().getTime();
            while (true) {
                if (callState === call.CALL_STATUS_DIALING) {
                    call.switchCall(callId, (err) => {
                        if (err) {
                            console.log(
                                'Telephony_CallManager_switchCall_Async_0400 switchCall finish err = ' + err.message);
                            done();
                            return;
                        }
                        expect().assertFail();
                        console.log('Telephony_CallManager_switchCall_Async_0400 switchCall fail');
                    })
                    done();
                    return;
                }
                await sleep(waitTime);
                endTime = new Date().getTime();
                timeOut = endTime - startTime;
                if (timeOut > onTime) {
                    done();
                    break;
                }
            }
        })
    })

    /*
     * @tc.number  Telephony_CallManager_switchCall_Promise_0400
     * @tc.name    The call is connected, pass in the current call ID, call switchCall() to enable the call switchCall,
     *             call switchCall() to enable the call switchCall, no return value
     * @tc.desc    Function test
     */
    it('Telephony_CallManager_switchCall_Promise_0400', 0, async function (done) {
        try {
            var data = await call.dial(TEST_PHONY_NUMBER);
            expect(data).assertTrue();
            console.log('Telephony_CallManager_switchCall_Promise_0400 dial finish data = ' + data);
        } catch (err) {
            console.log('Telephony_CallManager_switchCall_Promise_0400 dial fail err = ' + err);
            expect().assertFail();
            done();
            return;
        }
        const startTime = new Date().getTime();
        while (true) {
            if (callState === call.CALL_STATUS_DIALING) {
                try {
                    await call.switchCall(callId);
                    expect().assertFail();
                    console.log('Telephony_CallManager_switchCall_Promise_0400 switchCall fail');
                } catch (err) {
                    console.log('Telephony_CallManager_switchCall_Promise_0400 switchCall finish err = ' + err.message);
                    done();
                    return;
                }
                done();
                return;
            }
            await sleep(waitTime);
            endTime = new Date().getTime();
            timeOut = endTime - startTime;
            if (timeOut > onTime) {
                done();
                break;
            }
        }
    })

    /*
     * @tc.number  Telephony_CallManager_holdCall_Async_0400
     * @tc.name    The call is connected, passing in the current call ID, calling holdCall() to enable the call,
     *             or calling holdCall() to enable the call, with no return value
     * @tc.desc    Function test
     */
    it('Telephony_CallManager_holdCall_Async_0400', 0, async function (done) {
        call.dial(TEST_PHONY_NUMBER, async (err, data) => {
            if (err) {
                expect().assertFail();
                console.log('Telephony_CallManager_holdCall_Async_0400 dial fail err = ' + err);
                done();
                return;
            }
            expect(data).assertTrue();
            console.log('Telephony_CallManager_holdCall_Async_0400 dial finish data = ' + data);
            const startTime = new Date().getTime();
            while (true) {
                if (callState === call.CALL_STATUS_ACTIVE) {
                    call.holdCall(callId, (err) => {
                        if (err) {
                            expect().assertFail();
                            console.log('Telephony_CallManager_holdCall_Async_0400 fail err = ' + err.message);
                            done();
                            return;
                        }
                        console.log('Telephony_CallManager_holdCall_Async_0400 finish');
                    })
                    done();
                    return;
                }
                await sleep(waitTime);
                endTime = new Date().getTime();
                timeOut = endTime - startTime;
                if (timeOut > onTime) {
                    done();
                    break;
                }
            }
        })
    })

    /*
     * @tc.number  Telephony_CallManager_holdCall_Promise_0400
     * @tc.name    The call is connected, pass in the current call ID, call holdCall() to enable the call hold,
     *             call holdCall() to enable the call hold, no return value
     * @tc.desc    Function test
     */
    it('Telephony_CallManager_holdCall_Promise_0400', 0, async function (done) {
        try {
            var data = await call.dial(TEST_PHONY_NUMBER);
            expect(data).assertTrue();
            console.log('Telephony_CallManager_holdCall_Promise_0400 dial finish data = ' + data);
        } catch (err) {
            console.log('Telephony_CallManager_holdCall_Promise_0400 dial fail err = ' + err);
            expect().assertFail();
            done();
            return;
        }
        const startTime = new Date().getTime();
        while (true) {
            if (callState === call.CALL_STATUS_ACTIVE) {
                try {
                    await call.holdCall(callId);
                    console.log('Telephony_CallManager_holdCall_Promise_0400 holdCall finish');
                } catch (err) {
                    console.log('Telephony_CallManager_holdCall_Promise_0400 holdCall fail err = ' + err.message);
                    expect().assertFail();
                    done();
                    return;
                }
                done();
                return;
            }
            await sleep(waitTime);
            endTime = new Date().getTime();
            timeOut = endTime - startTime;
            if (timeOut > onTime) {
                done();
                break;
            }
        }
    })

     /*
     * @tc.number  Telephony_CallManager_unholdCall_Async_0400
     * @tc.name    The call is connected, passing in the current call ID, calling unHoldCall() to deactivate the call,
     *             or calling unHoldCall() to enable the call unHoldCall, with no return value
     * @tc.desc    Function test
     */
     it('Telephony_CallManager_unholdCall_Async_0400', 0, async function (done) {
        call.dial(TEST_PHONY_NUMBER, async (err, data) => {
            if (err) {
                expect().assertFail();
                console.log('Telephony_CallManager_unholdCall_Async_0400 dial fail err = ' + err);
                done();
                return;
            }
            expect(data).assertTrue();
            console.log('Telephony_CallManager_unholdCall_Async_0400  dial finish data = ' + data);
            const startTime = new Date().getTime();
            let holdCallflig = true;
            let unHoldCallflig = true;
            while (true) {
                if (callState === call.CALL_STATUS_ACTIVE && holdCallflig === true) {
                    holdCallflig = false;
                    call.holdCall(callId, (err) => {
                        if (err) {
                            expect().assertFail();
                            console.log(
                                'Telephony_CallManager_unholdCall_Async_0400 holdCall fail err = ' + err.message
                            );
                            done();
                            return;
                        }
                        console.log('Telephony_CallManager_unholdCall_Async_0400 holdCall finish');
                    })
                }
                await sleep(waitTime);
                if (callState === call.CALL_STATUS_HOLDING && unHoldCallflig === true) {
                    unHoldCallflig = false;
                    call.unHoldCall(callId, (err) => {
                        if (err) {
                            expect().assertFail();
                            console.log(
                                'Telephony_CallManager_unholdCall_Async_0400 unHoldCall fail err = ' +
                                err.message
                            );
                            done();
                            return;
                        }
                        console.log('Telephony_CallManager_unholdCall_Async_0400 unHoldCall finish');
                        done();
                    })
                    return;
                }
                await sleep(waitTime);
                endTime = new Date().getTime();
                timeOut = endTime - startTime;
                if (timeOut > onTime) {
                    done();
                    break;
                }
            }
        })
    })

    /*
     * @tc.number  Telephony_CallManager_unholdCall_Promise_0400
     * @tc.name    The call is connected, pass in the current call ID, call unholdCall() to deactivate the call,
     *             call unholdCall() to deactivate the call, no return value
     * @tc.desc    Function test
     */
    it('Telephony_CallManager_unholdCall_Promise_0400', 0, async function (done) {
        try {
            var data = await call.dial(TEST_PHONY_NUMBER);
            expect(data).assertTrue();
            console.log('Telephony_CallManager_unholdCall_Promise_0400 dial finish data = ' + data);
        } catch (err) {
            console.log('Telephony_CallManager_unholdCall_Promise_0400 dial fail err = ' + err);
            expect().assertFail();
            done();
            return;
        }
        let holdCallflig = true;
        let unHoldCallflig = true;
        const startTime = new Date().getTime();
        while (true) {
            if (callState === call.CALL_STATUS_ACTIVE && holdCallflig === true) {
                holdCallflig = false;
                try {
                    await call.holdCall(callId);
                    console.log('Telephony_CallManager_unholdCall_Promise_0400 holdCall finish');
                } catch (err) {
                    console.log('Telephony_CallManager_unholdCall_Promise_0400 holdCall fail err = ' + err.message);
                    expect().assertFail();
                    done();
                }
            }
            await sleep(waitTime);
            if (callState === call.CALL_STATUS_HOLDING && unHoldCallflig === true) {
                unHoldCallflig = false;
                try {
                    await call.unHoldCall(callId);
                    console.log('Telephony_CallManager_unholdCall_Promise_0400 unHoldCall finish');
                    done();
                    return;
                } catch (err) {
                    console.log(
                        'Telephony_CallManager_unholdCall_Promise_0400 unHoldCall fail err = ' + err.message
                    );
                    expect().assertFail();
                    done();
                    return;
                }
            }
        }
        await sleep(waitTime);
        endTime = new Date().getTime();
        timeOut = endTime - startTime;
        if (timeOut > onTime) {
            done();
            return;
        }
    })

    /*
     * @tc.number  Telephony_CallManager_hasCall_Async_0500
     * @tc.name    Call 10086, hasCall() to confirm that there is a call, returns true
     * @tc.desc    Function test
     */
    it('Telephony_CallManager_hasCall_Async_0500', 0, async function (done) {
        call.dial(TEST_PHONY_NUMBER, async (err, data) => {
            if (err) {
                expect().assertFail();
                console.log('Telephony_CallManager_hasCall_Async_0500 dial fail err = ' + err);
                done();
                return;
            }
            expect(data).assertTrue();
            console.log('Telephony_CallManager_hasCall_Async_0500  dial finish data = ' + data);
            const startTime = new Date().getTime();
            while (true) {
                if (callState === call.CALL_STATUS_DIALING || callState === call.CALL_STATUS_ACTIVE) {
                    call.hasCall((err, data) => {
                        if (err) {
                            expect().assertFail();
                            console.log('Telephony_CallManager_hasCall_Async_0500 fail err = ' + err);
                            done();
                            return;
                        }
                        expect(data).assertTrue();
                        console.log('Telephony_CallManager_hasCall_Async_0500 finish data = ' + data);
                    })
                    done();
                    return;
                }
                await sleep(waitTime);
                endTime = new Date().getTime();
                timeOut = endTime - startTime;
                if (timeOut > onTime) {
                    done();
                    break;
                }
            }
        })
    })

    /*
     * @tc.number  Telephony_CallManager_hasCall_Promise_0500
     * @tc.name    Call 10086, hasCall() to confirm that there is a call, returns true
     * @tc.desc    Function test
     */
    it('Telephony_CallManager_hasCall_Promise_0500', 0, async function (done) {
        try {
            var data = await call.dial(TEST_PHONY_NUMBER);
            expect(data).assertTrue();
            console.log('Telephony_CallManager_hasCall_Promise_0500 dial finish data = ' + data);
        } catch (err) {
            console.log('Telephony_CallManager_hasCall_Promise_0500 dial fail err = ' + err);
            expect().assertFail();
            done();
            return;
        }
        const startTime = new Date().getTime();
        while (true) {
            if (callState === call.CALL_STATUS_DIALING || callState === call.CALL_STATUS_ACTIVE) {
                try {
                    data = await call.hasCall();
                    expect(data).assertTrue();
                    console.log('Telephony_CallManager_hasCall_Promise_0500 getCallState finish data = ' + data);
                } catch (err) {
                    console.log('Telephony_CallManager_hasCall_Promise_0500 getCallState fail err = ' + err);
                    expect().assertFail();
                    done();
                    return;
                }
                done();
                return;
            }
            await sleep(waitTime);
            endTime = new Date().getTime();
            timeOut = endTime - startTime;
            if (timeOut > onTime) {
                done();
                break;
            }
        }
    })

    /*
     * @tc.number  Telephony_CallManager_getCallState_Async_0200
     * @tc.name    GetCallState () is used as a callback to get the current call status. call.CALL_STATE_OFFHOOK
     *             is returned
     * @tc.desc    Function test
     */
    it('Telephony_CallManager_getCallState_Async_0200', 0, async function (done) {
        call.dial(TEST_PHONY_NUMBER, async (err, data) => {
            if (err) {
                expect().assertFail();
                console.log('Telephony_CallManager_getCallState_Async_0200 dial fail err = ' + err);
                done();
                return;
            }
            expect(data).assertTrue();
            console.log('Telephony_CallManager_getCallState_Async_0200  dial finish data = ' + data);
            const startTime = new Date().getTime();
            while (true) {
                if (callState === call.CALL_STATUS_DIALING || callState === call.CALL_STATUS_ACTIVE) {
                    call.getCallState((err, data) => {
                        if (err) {
                            expect().assertFail();
                            console.log('Telephony_CallManager_getCallState_Async_0200 fail err = ' + err);
                            done();
                            return;
                        }
                        expect(data === call.CALL_STATE_OFFHOOK).assertTrue();
                        console.log('Telephony_CallManager_getCallState_Async_0200 finish data = ' + data);
                    })
                    done();
                    return;
                }
                await sleep(waitTime);
                endTime = new Date().getTime();
                timeOut = endTime - startTime;
                if (timeOut > onTime) {
                    done();
                    break;
                }
            }
        })
    })

    /*
     * @tc.number  Telephony_CallManager_getCallState_Promise_0200
     * @tc.name    To get the call status during dialing, call getCallState() to get the current call status.
     *             call.CALL_STATE_OFFHOOK is returned
     * @tc.desc    Function test
     */
    it('Telephony_CallManager_getCallState_Promise_0200', 0, async function (done) {
        try {
            var data = await call.dial(TEST_PHONY_NUMBER);
            expect(data).assertTrue();
            console.log('Telephony_CallManager_getCallState_Promise_0200 dial finish data = ' + data);
        } catch (err) {
            console.log('Telephony_CallManager_getCallState_Promise_0200 dial fail err = ' + err);
            expect().assertFail();
            done();
            return;
        }
        const startTime = new Date().getTime();
        while (true) {
            if (callState === call.CALL_STATUS_DIALING || callState === call.CALL_STATUS_ACTIVE) {
                try {
                    data = await call.getCallState();
                    expect(data === call.CALL_STATE_OFFHOOK).assertTrue();
                    console.log('Telephony_CallManager_getCallState_Promise_0200 getCallState finish data = ' + data);
                } catch (err) {
                    console.log('Telephony_CallManager_getCallState_Promise_0200 getCallState fail err = ' + err);
                    expect().assertFail();
                    done();
                    return;
                }
                done();
                return;
            }
            await sleep(waitTime);
            endTime = new Date().getTime();
            timeOut = endTime - startTime;
            if (timeOut > onTime) {
                done();
                break;
            }
        }
    })

    /*
     * @tc.number  Telephony_CallManager_startDTMF_Async_0100
     * @tc.name    CallId is the current callId, character is 0, call startDTMF() to startDTMF, true
     * @tc.desc    Function test
     */
    it('Telephony_CallManager_startDTMF_Async_0100', 0, async function (done) {
        call.dial(TEST_PHONY_NUMBER, async (err, data) => {
            if (err) {
                console.log('Telephony_CallManager_startDTMF_Async_0100 dial fail err = ' + err);
                expect().assertFail();
                done();
                return;
            }
            expect(data).assertTrue();
            console.log('Telephony_CallManager_startDTMF_Async_0100 dial finish data = ' + data);
            const startTime = new Date().getTime();
            while (true) {
                if (callState === call.CALL_STATUS_DIALING || callState === call.CALL_STATUS_ACTIVE) {
                    call.startDTMF(callId, '0', (err) => {
                        if (err) {
                            expect().assertFail();
                            console.log('Telephony_CallManager_startDTMF_Async_0100 startDTMF fail err = ' + err);
                            return;
                        }
                        console.log('Telephony_CallManager_startDTMF_Async_0100 startDTMF finish data = ' + data);
                    })
                    call.stopDTMF(callId, (err) => {
                        if (err) {
                            expect().assertFail();
                            console.log('Telephony_CallManager_startDTMF_Async_0100 stopDTMF fail err = ' + err);
                            return;
                        }
                        console.log('Telephony_CallManager_startDTMF_Async_0100 stopDTMF finish data = ' + data);
                    })
                    done();
                    return;
                }
                await sleep(waitTime);
                endTime = new Date().getTime();
                timeOut = endTime - startTime;
                if (timeOut > onTime) {
                    done();
                    break;
                }
            }
        })
    })

    /*
     * @tc.number  Telephony_CallManager_startDTMF_Async_0200
     * @tc.name    CallId is the current callId, character is 5, call startDTMF() to startDTMF, true
     * @tc.desc    Function test
     */
    it('Telephony_CallManager_startDTMF_Async_0200', 0, async function (done) {
        call.dial(TEST_PHONY_NUMBER, async (err, data) => {
            if (err) {
                console.log('Telephony_CallManager_startDTMF_Async_0200 dial fail err = ' + err);
                expect().assertFail();
                done();
                return;
            }
            expect(data).assertTrue();
            console.log('Telephony_CallManager_startDTMF_Async_0200 dial finish data = ' + data);
            const startTime = new Date().getTime();
            while (true) {
                if (callState === call.CALL_STATUS_DIALING || callState === call.CALL_STATUS_ACTIVE) {
                    call.startDTMF(callId, '5', (err) => {
                        if (err) {
                            expect().assertFail();
                            console.log('Telephony_CallManager_startDTMF_Async_0200 startDTMF fail err = ' + err);
                            return;
                        }
                        console.log('Telephony_CallManager_startDTMF_Async_0200 startDTMF finish data = ' + data);
                    })
                    call.stopDTMF(callId, (err) => {
                        if (err) {
                            expect().assertFail();
                            console.log('Telephony_CallManager_startDTMF_Async_0200 stopDTMF fail err = ' + err);
                            return;
                        }
                        console.log('Telephony_CallManager_startDTMF_Async_0200 stopDTMF finish data = ' + data);
                    })
                    done();
                    return;
                }
                await sleep(waitTime);
                endTime = new Date().getTime();
                timeOut = endTime - startTime;
                if (timeOut > onTime) {
                    done();
                    break;
                }
            }
        })
    })

    /*
     * @tc.number  Telephony_CallManager_startDTMF_Async_0300
     * @tc.name    CallId is the current callId, character is 9, call startDTMF() to startDTMF, true
     * @tc.desc    Function test
     */
    it('Telephony_CallManager_startDTMF_Async_0300', 0, async function (done) {
        call.dial(TEST_PHONY_NUMBER, async (err, data) => {
            if (err) {
                console.log('Telephony_CallManager_startDTMF_Async_0300 dial fail err = ' + err);
                expect().assertFail();
                done();
                return;
            }
            expect(data).assertTrue();
            console.log('Telephony_CallManager_startDTMF_Async_0300 dial finish data = ' + data);
            const startTime = new Date().getTime();
            while (true) {
                if (callState === call.CALL_STATUS_DIALING || callState === call.CALL_STATUS_ACTIVE) {
                    call.startDTMF(callId, '9', (err) => {
                        if (err) {
                            expect().assertFail();
                            console.log('Telephony_CallManager_startDTMF_Async_0300 startDTMF fail err = ' + err);
                            return;
                        }
                        console.log('Telephony_CallManager_startDTMF_Async_0300 startDTMF finish data = ' + data);
                    })
                    call.stopDTMF(callId, (err) => {
                        if (err) {
                            expect().assertFail();
                            console.log('Telephony_CallManager_startDTMF_Async_0300 stopDTMF fail err = ' + err);
                            return;
                        }
                        console.log('Telephony_CallManager_startDTMF_Async_0300 stopDTMF finish data = ' + data);
                    })
                    done();
                    return;
                }
                await sleep(waitTime);
                endTime = new Date().getTime();
                timeOut = endTime - startTime;
                if (timeOut > onTime) {
                    done();
                    break;
                }
            }
        })
    })

    /*
     * @tc.number  Telephony_CallManager_startDTMF_Async_0400
     * @tc.name    CallId is the current callId, character is A, call startDTMF() to startDTMF, true
     * @tc.desc    Function test
     */
    it('Telephony_CallManager_startDTMF_Async_0400', 0, async function (done) {
        call.dial(TEST_PHONY_NUMBER, async (err, data) => {
            if (err) {
                console.log('Telephony_CallManager_startDTMF_Async_0400 dial fail err = ' + err);
                expect().assertFail();
                done();
                return;
            }
            expect(data).assertTrue();
            console.log('Telephony_CallManager_startDTMF_Async_0400 dial finish : data = ' + data);
            const startTime = new Date().getTime();
            while (true) {
                if (callState === call.CALL_STATUS_DIALING || callState === call.CALL_STATUS_ACTIVE) {
                    call.startDTMF(callId, 'A', (err) => {
                        if (err) {
                            expect().assertFail();
                            console.log('Telephony_CallManager_startDTMF_Async_0400 startDTMF fail : err = ' + err);
                            return;
                        }
                        console.log('Telephony_CallManager_startDTMF_Async_0400 startDTMF finish : data = ' + data);
                    })
                    call.stopDTMF(callId, (err) => {
                        if (err) {
                            expect().assertFail();
                            console.log('Telephony_CallManager_startDTMF_Async_0400 stopDTMF fail : err = ' + err);
                            return;
                        }
                        console.log('Telephony_CallManager_startDTMF_Async_0400 stopDTMF finish : data = ' + data);
                    })
                    done();
                    return;
                }
                await sleep(waitTime);
                endTime = new Date().getTime();
                timeOut = endTime - startTime;
                if (timeOut > onTime) {
                    done();
                    break;
                }
            }
        })
    })

    /*
     * @tc.number  Telephony_CallManager_startDTMF_Async_0500
     * @tc.name    CallId is the current callId, character is C, call startDTMF() to startDTMF, true
     * @tc.desc    Function test
     */
    it('Telephony_CallManager_startDTMF_Async_0500', 0, async function (done) {
        call.dial(TEST_PHONY_NUMBER,async (err, data) => {
            if (err) {
                console.log('Telephony_CallManager_startDTMF_Async_0500 dial fail : err = ' + err);
                expect().assertFail();
                done();
                return;
            }
            expect(data).assertTrue();
            console.log('Telephony_CallManager_startDTMF_Async_0500 dial finish : data = ' + data);
            const startTime = new Date().getTime();
            while (true) {
                if (callState === call.CALL_STATUS_DIALING || callState === call.CALL_STATUS_ACTIVE) {
                    call.startDTMF(callId, 'C', (err) => {
                        if (err) {
                            expect().assertFail();
                            console.log('Telephony_CallManager_startDTMF_Async_0500 startDTMF fail : err = ' + err);
                            return;
                        }
                        console.log('Telephony_CallManager_startDTMF_Async_0500 startDTMF finish : data = ' + data);
                    })
                    call.stopDTMF(callId, (err) => {
                        if (err) {
                            expect().assertFail();
                            console.log('Telephony_CallManager_startDTMF_Async_0500 stopDTMF fail : err = ' + err);
                            return;
                        }
                        console.log('Telephony_CallManager_startDTMF_Async_0500 stopDTMF finish : data = ' + data);
                    })
                    done();
                    return;
                }
                await sleep(waitTime);
                endTime = new Date().getTime();
                timeOut = endTime - startTime;
                if (timeOut > onTime) {
                    done();
                    break;
                }
            }
        })
    })

    /*
     * @tc.number  Telephony_CallManager_startDTMF_Async_0600
     * @tc.name    CallId is the current callId, character is D, call startDTMF() to startDTMF, true
     * @tc.desc    Function test
     */
    it('Telephony_CallManager_startDTMF_Async_0600', 0, async function (done) {
        call.dial(TEST_PHONY_NUMBER, async (err, data) => {
            if (err) {
                console.log('Telephony_CallManager_startDTMF_Async_0600 dial fail : err = ' + err);
                expect().assertFail();
                done();
                return;
            }
            expect(data).assertTrue();
            console.log('Telephony_CallManager_startDTMF_Async_0600 dial finish : data = ' + data);
            const startTime = new Date().getTime();
            while (true) {
                if (callState === call.CALL_STATUS_DIALING || callState === call.CALL_STATUS_ACTIVE) {
                    call.startDTMF(callId, 'D', (err) => {
                        if (err) {
                            expect().assertFail();
                            console.log('Telephony_CallManager_startDTMF_Async_0600 startDTMF fail : err = ' + err);
                            return;
                        }
                        console.log('Telephony_CallManager_startDTMF_Async_0600 startDTMF finish : data = ' + data);
                    })
                    call.stopDTMF(callId, (err) => {
                        if (err) {
                            expect().assertFail();
                            console.log('Telephony_CallManager_startDTMF_Async_0600 stopDTMF fail : err = ' + err);
                            return;
                        }
                        console.log('Telephony_CallManager_startDTMF_Async_0600 stopDTMF finish : data = ' + data);
                    })
                    done();
                    return;
                }
                await sleep(waitTime);
                endTime = new Date().getTime();
                timeOut = endTime - startTime;
                if (timeOut > onTime) {
                    done();
                    break;
                }
            }
        })
    })

    /*
     * @tc.number  Telephony_CallManager_startDTMF_Async_0700
     * @tc.name    CallId is the current callId, character is d, call startDTMF() to startDTMF, false
     * @tc.desc    Function test
     */
    it('Telephony_CallManager_startDTMF_Async_0700', 0, async function (done) {
        call.dial(TEST_PHONY_NUMBER, async (err, data) => {
            if (err) {
                console.log('Telephony_CallManager_startDTMF_Async_0700 dial fail : err = ' + err);
                expect().assertFail();
                done();
                return;
            }
            expect(data).assertTrue();
            console.log('Telephony_CallManager_startDTMF_Async_0700 dial finish : data = ' + data);
            const startTime = new Date().getTime();
            while (true) {
                if (callState === call.CALL_STATUS_DIALING || callState === call.CALL_STATUS_ACTIVE) {
                    call.startDTMF(callId, 'd', (err) => {
                        if (err) {
                            console.log('Telephony_CallManager_startDTMF_Async_0700 startDTMF finish : err = ' + err);
                            return;
                        }
                        console.log('Telephony_CallManager_startDTMF_Async_0700 startDTMF fail');
                        expect().assertFail();
                    })
                    done();
                    return;
                }
                await sleep(waitTime);
                endTime = new Date().getTime();
                timeOut = endTime - startTime;
                if (timeOut > onTime) {
                    done();
                    break;
                }
            }
        })
    })

    /*
     * @tc.number  Telephony_CallManager_startDTMF_Async_0800
     * @tc.name    CallId is the current callId, character is *, call startDTMF() to startDTMF, true
     * @tc.desc    Function test
     */
    it('Telephony_CallManager_startDTMF_Async_0800', 0, async function (done) {
        call.dial(TEST_PHONY_NUMBER, async (err, data) => {
            if (err) {
                console.log('Telephony_CallManager_startDTMF_Async_0800 dial fail : err = ' + err);
                expect().assertFail();
                done();
                return;
            }
            expect(data).assertTrue();
            console.log('Telephony_CallManager_startDTMF_Async_0800 dial finish : data = ' + data);
            const startTime = new Date().getTime();
            while (true) {
                if (callState === call.CALL_STATUS_DIALING || callState === call.CALL_STATUS_ACTIVE) {
                    call.startDTMF(callId, '*', (err) => {
                        if (err) {
                            expect().assertFail();
                            console.log('Telephony_CallManager_startDTMF_Async_0800 startDTMF fail : err = ' + err);
                            return;
                        }
                        console.log('Telephony_CallManager_startDTMF_Async_0800 startDTMF finish : data = ' + data);
                    })
                    call.stopDTMF(callId, (err) => {
                        if (err) {
                            expect().assertFail();
                            console.log('Telephony_CallManager_startDTMF_Async_0800 stopDTMF fail : err = ' + err);
                            return;
                        }
                        console.log('Telephony_CallManager_startDTMF_Async_0800 stopDTMF finish : data = ' + data);
                    })
                    done();
                    return;
                }
                await sleep(waitTime);
                endTime = new Date().getTime();
                timeOut = endTime - startTime;
                if (timeOut > onTime) {
                    done();
                    break;
                }
            }
        })
    })

    /*
     * @tc.number  Telephony_CallManager_startDTMF_A  sync_0900
     * @tc.name    CallId is the current callId, character is #, call startDTMF() to startDTMF, true
     * @tc.desc    Function test
     */
    it('Telephony_CallManager_startDTMF_Async_0900', 0, async function (done) {
        call.dial(TEST_PHONY_NUMBER, async (err, data) => {
            if (err) {
                console.log('Telephony_CallManager_startDTMF_Async_0900 dial fail : err = ' + err);
                expect().assertFail();
                done();
                return;
            }
            expect(data).assertTrue();
            console.log('Telephony_CallManager_startDTMF_Async_0900 dial finish : data = ' + data);
            const startTime = new Date().getTime();
            while (true) {
                if (callState === call.CALL_STATUS_DIALING || callState === call.CALL_STATUS_ACTIVE) {
                    call.startDTMF(callId, '#', (err) => {
                        if (err) {
                            expect().assertFail();
                            console.log('Telephony_CallManager_startDTMF_Async_0900 startDTMF fail : err = ' + err);
                            return;
                        }
                        console.log('Telephony_CallManager_startDTMF_Async_0900 startDTMF finish : data = ' + data);
                    })
                    call.stopDTMF(callId, (err) => {
                        if (err) {
                            expect().assertFail();
                            console.log('Telephony_CallManager_startDTMF_Async_0900 stopDTMF fail : err = ' + err);
                            return;
                        }
                        console.log('Telephony_CallManager_startDTMF_Async_0900 stopDTMF finish : data = ' + data);
                    })
                    done();
                    return;
                }
                await sleep(waitTime);
                endTime = new Date().getTime();
                timeOut = endTime - startTime;
                if (timeOut > onTime) {
                    done();
                    break;
                }
            }
        })
    })

    /*
     * @tc.number  Telephony_CallManager_startDTMF_Promise_0100
     * @tc.name    CallId is the current callId, character is 1, call startDTMF() to startDTMF, true
     * @tc.desc    Function test
     */
    it('Telephony_CallManager_startDTMF_Promise_0100', 0, async function (done) {
        try {
            var data = await call.dial(TEST_PHONY_NUMBER);
            expect(data).assertTrue();
            console.log('Telephony_CallManager_startDTMF_Promise_0100 dial finish : data = ' + data);
            const startTime = new Date().getTime();
            while (true) {
                if (callState === call.CALL_STATUS_DIALING || callState === call.CALL_STATUS_ACTIVE) {
                    try {
                        await call.startDTMF(callId, '1');
                        console.log('Telephony_CallManager_startDTMF_Promise_0100 startDTMF finish : data = ' + data);
                    } catch (err) {
                        expect().assertFail();
                        console.log('Telephony_CallManager_startDTMF_Promise_0100 startDTMF fail : err = ' + err);
                        return;
                    }
                    try {
                        await call.stopDTMF(callId);
                        console.log('Telephony_CallManager_startDTMF_Promise_0100 stopDTMF finish : data = ' + data);
                    } catch (err) {
                        expect().assertFail();
                        console.log('Telephony_CallManager_startDTMF_Promise_0100 stopDTMF fail : err = ' + err);
                        return;
                    }
                    done();
                    return;
                }
                await sleep(waitTime);
                endTime = new Date().getTime();
                timeOut = endTime - startTime;
                if (timeOut > onTime) {
                    done();
                    break;
                }
            }
        } catch (err) {
            expect().assertFail();
            console.log('Telephony_CallManager_startDTMF_Promise_0100 dial fail : err = ' + err);
            done();
        }
    })

    /*
     * @tc.number  Telephony_CallManager_startDTMF_Promise_0200
     * @tc.name    CallId is the current callId, character is 4, call startDTMF() to startDTMF, true
     * @tc.desc    Function test
     */
    it('Telephony_CallManager_startDTMF_Promise_0200', 0, async function (done) {
        try {
            var data = await call.dial(TEST_PHONY_NUMBER);
            expect(data).assertTrue();
            console.log('Telephony_CallManager_startDTMF_Promise_0200 dial finish : data = ' + data);
            const startTime = new Date().getTime();
            while (true) {
                if (callState === call.CALL_STATUS_DIALING || callState === call.CALL_STATUS_ACTIVE) {
                    try {
                        await call.startDTMF(callId, '4');
                        console.log('Telephony_CallManager_startDTMF_Promise_0200 startDTMF finish : data = ' + data);
                    } catch (err) {
                        expect().assertFail();
                        console.log('Telephony_CallManager_startDTMF_Promise_0200 startDTMF fail : err = ' + err);
                        return;
                    }
                    try {
                        await call.stopDTMF(callId);
                        console.log('Telephony_CallManager_startDTMF_Promise_0200 stopDTMF finish : data = ' + data);
                    } catch (err) {
                        expect().assertFail();
                        console.log('Telephony_CallManager_startDTMF_Promise_0200 stopDTMF fail : err = ' + err);
                        return;
                    }
                    done();
                    return;
                }
                await sleep(waitTime);
                endTime = new Date().getTime();
                timeOut = endTime - startTime;
                if (timeOut > onTime) {
                    done();
                    break;
                }
            }
        } catch (err) {
            expect().assertFail();
            console.log('Telephony_CallManager_startDTMF_Promise_0100 dial fail : err = ' + err);
            done();
        }
    })

    /*
     * @tc.number  Telephony_CallManager_startDTMF_Promise_0300
     * @tc.name    CallId is the current callId, character is 8, call startDTMF() to startDTMF, true
     * @tc.desc    Function test
     */
    it('Telephony_CallManager_startDTMF_Promise_0300', 0, async function (done) {
        try {
            var data = await call.dial(TEST_PHONY_NUMBER);
            expect(data).assertTrue();
            console.log('Telephony_CallManager_startDTMF_Promise_0300 dial finish : data = ' + data);
            const startTime = new Date().getTime();
            while (true) {
                if (callState === call.CALL_STATUS_DIALING || callState === call.CALL_STATUS_ACTIVE) {
                    try {
                        await call.startDTMF(callId, '8');
                        console.log('Telephony_CallManager_startDTMF_Promise_0300 startDTMF finish : data = ' + data);
                    } catch (err) {
                        expect().assertFail();
                        console.log('Telephony_CallManager_startDTMF_Promise_0300 startDTMF fail : err = ' + err);
                        return;
                    }
                    try {
                        await call.stopDTMF(callId);
                        console.log('Telephony_CallManager_startDTMF_Promise_0300 stopDTMF finish : data = ' + data);
                    } catch (err) {
                        expect().assertFail();
                        console.log('Telephony_CallManager_startDTMF_Promise_0300 stopDTMF fail : err = ' + err);
                        return;
                    }
                    done();
                    return;
                }
                await sleep(waitTime);
                endTime = new Date().getTime();
                timeOut = endTime - startTime;
                if (timeOut > onTime) {
                    done();
                    break;
                }
            }
        } catch (err) {
            expect().assertFail();
            console.log('Telephony_CallManager_startDTMF_Promise_0300 dial fail : err = ' + err);
            done();
        }
    })

    /*
     * @tc.number  Telephony_CallManager_startDTMF_Promise_0400
     * @tc.name    CallId is the current callId, character is B, call startDTMF() to startDTMF, true
     * @tc.desc    Function test
     */
    it('Telephony_CallManager_startDTMF_Promise_0400', 0, async function (done) {
        try {
            var data = await call.dial(TEST_PHONY_NUMBER);
            expect(data).assertTrue();
            console.log('Telephony_CallManager_startDTMF_Promise_0400 dial finish : data = ' + data);
            const startTime = new Date().getTime();
            while (true) {
                if (callState === call.CALL_STATUS_DIALING || callState === call.CALL_STATUS_ACTIVE) {
                    try {
                        await call.startDTMF(callId, 'B');
                        console.log('Telephony_CallManager_startDTMF_Promise_0400 startDTMF finish : data = ' + data);
                    } catch (err) {
                        expect().assertFail();
                        console.log('Telephony_CallManager_startDTMF_Promise_0400 startDTMF fail : err = ' + err);
                        return;
                    }
                    try {
                        await call.stopDTMF(callId);
                        console.log('Telephony_CallManager_startDTMF_Promise_0400 stopDTMF finish : data = ' + data);
                    } catch (err) {
                        expect().assertFail();
                        console.log('Telephony_CallManager_startDTMF_Promise_0400 stopDTMF fail : err = ' + err);
                        return;
                    }
                    done();
                    return;
                }
                await sleep(waitTime);
                endTime = new Date().getTime();
                timeOut = endTime - startTime;
                if (timeOut > onTime) {
                    done();
                    break;
                }
            }
        } catch (err) {
            expect().assertFail();
            console.log('Telephony_CallManager_startDTMF_Promise_0100 dial fail : err = ' + err);
            done();
        }
    })

    /*
     * @tc.number  Telephony_CallManager_startDTMF_Promise_0500
     * @tc.name    CallId is the current callId, character is 7, call startDTMF() to startDTMF, true
     * @tc.desc    Function test
     */
    it('Telephony_CallManager_startDTMF_Promise_0500', 0, async function (done) {
        try {
            var data = await call.dial(TEST_PHONY_NUMBER);
            expect(data).assertTrue();
            console.log('Telephony_CallManager_startDTMF_Promise_0500 dial finish : data = ' + data);
            const startTime = new Date().getTime();
            while (true) {
                if (callState === call.CALL_STATUS_DIALING || callState === call.CALL_STATUS_ACTIVE) {
                    try {
                        await call.startDTMF(callId, '7');
                        console.log('Telephony_CallManager_startDTMF_Promise_0500 startDTMF finish : data = ' + data);
                    } catch (err) {
                        expect().assertFail();
                        console.log('Telephony_CallManager_startDTMF_Promise_0500 startDTMF fail : err = ' + err);
                        return;
                    }
                    try {
                        await call.stopDTMF(callId);
                        console.log('Telephony_CallManager_startDTMF_Promise_0500 stopDTMF finish : data = ' + data);
                    } catch (err) {
                        expect().assertFail();
                        console.log('Telephony_CallManager_startDTMF_Promise_0500 stopDTMF fail : err = ' + err);
                        return;
                    }
                    done();
                    return;
                }
                await sleep(waitTime);
                endTime = new Date().getTime();
                timeOut = endTime - startTime;
                if (timeOut > onTime) {
                    done();
                    break;
                }
            }
        } catch (err) {
            expect().assertFail();
            console.log('Telephony_CallManager_startDTMF_Promise_0500 dial fail : err = ' + err);
            done();
        }
    })

    /*
     * @tc.number  Telephony_CallManager_startDTMF_Promise_0600
     * @tc.name    CallId is the current callId, character is Z, call startDTMF() to startDTMF, false
     * @tc.desc    Function test
     */
    it('Telephony_CallManager_startDTMF_Promise_0600', 0, async function (done) {
        try {
            var data = await call.dial(TEST_PHONY_NUMBER);
            expect(data).assertTrue();
            console.log('Telephony_CallManager_startDTMF_Promise_0600 dial finish : data = ' + data);
            const startTime = new Date().getTime();
            while (true) {
                if (callState === call.CALL_STATUS_DIALING || callState === call.CALL_STATUS_ACTIVE) {
                    try {
                        await call.startDTMF(callId, 'Z');
                        console.log('Telephony_CallManager_startDTMF_Promise_0600 startDTMF fail');
                    } catch (err) {
                        console.log('Telephony_CallManager_startDTMF_Promise_0600 startDTMF finish : err = ' + err);
                    }
                    try {
                        await call.stopDTMF(callId);
                        console.log('Telephony_CallManager_startDTMF_Promise_0600 stopDTMF finish : data = ' + data);
                    } catch (err) {
                        expect().assertFail();
                        console.log('Telephony_CallManager_startDTMF_Promise_0600 stopDTMF fail : err = ' + err);
                    }
                    done();
                    return;
                }
                await sleep(waitTime);
                endTime = new Date().getTime();
                timeOut = endTime - startTime;
                if (timeOut > onTime) {
                    done();
                    break;
                }
            }
        } catch (err) {
            expect().assertFail();
            console.log('Telephony_CallManager_startDTMF_Promise_0600 dial fail : err = ' + err);
            done();
        }
    })

    /*
     * @tc.number  Telephony_CallManager_startDTMF_Promise_0700
     * @tc.name    CallId is the current callId, character is a, call startDTMF() to startDTMF, false
     * @tc.desc    Function test
     */
    it('Telephony_CallManager_startDTMF_Promise_0700', 0, async function (done) {
        try {
            var data = await call.dial(TEST_PHONY_NUMBER);
            expect(data).assertTrue();
            console.log('Telephony_CallManager_startDTMF_Promise_0700 dial finish : data = ' + data);
            const startTime = new Date().getTime();
            while (true) {
                if (callState === call.CALL_STATUS_DIALING || callState === call.CALL_STATUS_ACTIVE) {
                    try {
                        await call.startDTMF(callId, 'a');
                        expect().assertFail();
                        console.log('Telephony_CallManager_startDTMF_Promise_0700 startDTMF fail');
                    } catch (err) {
                        console.log('Telephony_CallManager_startDTMF_Promise_0700 startDTMF finish : err = ' + err);
                    }
                    try {
                        await call.stopDTMF(callId);
                        console.log('Telephony_CallManager_startDTMF_Promise_0700 stopDTMF finish : data = ' + data);
                    } catch (err) {
                        expect().assertFail();
                        console.log('Telephony_CallManager_startDTMF_Promise_0700 stopDTMF fail : err = ' + err);
                        return;
                    }
                    done();
                    return;
                }
                await sleep(waitTime);
                endTime = new Date().getTime();
                timeOut = endTime - startTime;
                if (timeOut > onTime) {
                    done();
                    break;
                }
            }
        } catch (err) {
            expect().assertFail();
            console.log('Telephony_CallManager_startDTMF_Promise_0700 dial fail : err = ' + err);
            done();
        }
    })

    /*
     * @tc.number  Telephony_CallManager_startDTMF_Promise_0800
     * @tc.name    CallId is the current callId, character is *, call startDTMF() to startDTMF, true
     * @tc.desc    Function test
     */
    it('Telephony_CallManager_startDTMF_Promise_0800', 0, async function (done) {
        try {
            var data = await call.dial(TEST_PHONY_NUMBER);
            expect(data).assertTrue();
            console.log('Telephony_CallManager_startDTMF_Promise_0800 dial finish : data = ' + data);
            const startTime = new Date().getTime();
            while (true) {
                if (callState === call.CALL_STATUS_DIALING || callState === call.CALL_STATUS_ACTIVE) {
                    try {
                        await call.startDTMF(callId, '*');
                        console.log('Telephony_CallManager_startDTMF_Promise_0800 startDTMF finish : data = ' + data);
                    } catch (err) {
                        expect().assertFail();
                        console.log('Telephony_CallManager_startDTMF_Promise_0800 startDTMF fail : err = ' + err);
                        return;
                    }
                    try {
                        await call.stopDTMF(callId);
                        console.log('Telephony_CallManager_startDTMF_Promise_0800 stopDTMF finish : data = ' + data);
                    } catch (err) {
                        expect().assertFail();
                        console.log('Telephony_CallManager_startDTMF_Promise_0800 stopDTMF fail : err = ' + err);
                    }
                    done();
                    return;
                }
                await sleep(waitTime);
                endTime = new Date().getTime();
                timeOut = endTime - startTime;
                if (timeOut > onTime) {
                    done();
                    break;
                }
            }
        } catch (err) {
            expect().assertFail();
            console.log('Telephony_CallManager_startDTMF_Promise_0800 dial fail : err = ' + err);
            done();
        }
    })

    /*
     * @tc.number  Telephony_CallManager_startDTMF_Promise_0900
     * @tc.name    CallId is the current callId, character is #, call startDTMF() to startDTMF, true
     * @tc.desc    Function test
     */
    it('Telephony_CallManager_startDTMF_Promise_0900', 0, async function (done) {
        try {
            var data = await call.dial(TEST_PHONY_NUMBER);
            expect(data).assertTrue();
            console.log('Telephony_CallManager_startDTMF_Promise_0900 dial finish : data = ' + data);
            const startTime = new Date().getTime();
            while (true) {
                if (callState === call.CALL_STATUS_DIALING || callState === call.CALL_STATUS_ACTIVE) {
                    try {
                        await call.startDTMF(callId, '#');
                        console.log('Telephony_CallManager_startDTMF_Promise_0900 startDTMF finish : data = ' + data);
                    } catch (err) {
                        expect().assertFail();
                        console.log('Telephony_CallManager_startDTMF_Promise_0900 startDTMF fail : err = ' + err);
                        return;
                    }
                    try {
                        await call.stopDTMF(callId);
                        console.log('Telephony_CallManager_startDTMF_Promise_0900 stopDTMF finish : data = ' + data);
                    } catch (err) {
                        expect().assertFail();
                        console.log('Telephony_CallManager_startDTMF_Promise_0900 stopDTMF fail : err = ' + err);
                        return;
                    }
                    done();
                    return;
                }
                await sleep(waitTime);
                endTime = new Date().getTime();
                timeOut = endTime - startTime;
                if (timeOut > onTime) {
                    done();
                    break;
                }
            }
        } catch (err) {
            expect().assertFail();
            console.log('Telephony_CallManager_startDTMF_Promise_0900 dial fail : err = ' + err);
            done();
        }
    })

    /*
     * @tc.number  Telephony_CallManager_stopDTMF_Async_0200
     * @tc.name    CallId is the current callId, character is , call startDTMF() to startDTMF, true
     * @tc.desc    Function test
     */
    it('Telephony_CallManager_stopDTMF_Async_0200', 0, async function (done) {
        call.dial(TEST_PHONY_NUMBER, async (err, data) => {
            if (err) {
                expect().assertFail();
                console.log('Telephony_CallManager_stopDTMF_Async_0200 dial fail : err = ' + err);
                done();
                return;
            }
            expect(data).assertTrue();
            console.log('Telephony_CallManager_stopDTMF_Async_0200  dial finish : data = ' + data);
            const startTime = new Date().getTime();
            while (true) {
                if (callState === call.CALL_STATUS_DIALING || callState === call.CALL_STATUS_ACTIVE) {
                    call.stopDTMF(callId, (err) => {
                        if (err) {
                            expect().assertFail();
                            console.log('Telephony_CallManager_stopDTMF_Async_0200 stopDTMF fail : err = ' + err);
                            return;
                        }
                        console.log('Telephony_CallManager_stopDTMF_Async_0200 stopDTMF finish : data = ' + data);
                    })
                    done();
                    return;
                }
                await sleep(waitTime);
                endTime = new Date().getTime();
                timeOut = endTime - startTime;
                if (timeOut > onTime) {
                    done();
                    break;
                }
            }
        })
    })

    /*
     * @tc.number  Telephony_CallManager_stopDTMF_Promise_0200
     * @tc.name    CallId is the current callId, call stopDTMF() to start DTMF, true
     * @tc.desc    Function test
     */
    it('Telephony_CallManager_stopDTMF_Promise_0200', 0, async function (done) {
        try {
            var data = await call.dial(TEST_PHONY_NUMBER);
            expect(data).assertTrue();
            console.log('Telephony_CallManager_stopDTMF_Promise_0200 dial finish : data = ' + data);
            const startTime = new Date().getTime();
            while (true) {
                if (callState === call.CALL_STATUS_DIALING || callState === call.CALL_STATUS_ACTIVE) {
                    try {
                        await call.stopDTMF(callId);
                        console.log('Telephony_CallManager_stopDTMF_Promise_0200 stopDTMF finish : data = ' + data);
                    } catch (err) {
                        console.log('Telephony_CallManager_stopDTMF_Promise_0200 stopDTMF fail : err = ' + err);
                        expect().assertFail();
                        return;
                    }
                    done();
                    return;
                }
                await sleep(waitTime);
                endTime = new Date().getTime();
                timeOut = endTime - startTime;
                if (timeOut > onTime) {
                    done();
                    break;
                }
            }
        } catch (err) {
            expect().assertFail();
            console.log('Telephony_CallManager_stopDTMF_Promise_0200 dial fail : err = ' + err);
            done();
        }
    })

    /*
     * @tc.number  Telephony_CallManager_combineConference_Async_0200
     * @tc.name    After making a call, call combineConference() to merge the conference call and capture err
     * @tc.desc    Function test
     */
    it('Telephony_CallManager_combineConference_Async_0200', 0, async function (done) {
        call.dial(TEST_PHONY_NUMBER, async (err, data) => {
            if (err) {
                expect().assertFail();
                console.log('Telephony_CallManager_combineConference_Async_0200 dial fail : err = ' + err);
                done();
                return;
            }
            expect(data).assertTrue();
            console.log('Telephony_CallManager_combineConference_Async_0200  dial finish : data = ' + data);
            const startTime = new Date().getTime();
            while (true) {
                if (callState === call.CALL_STATUS_DIALING || callState === call.CALL_STATUS_ACTIVE) {
                    call.combineConference(callId, (err) => {
                        if (err) {
                            console.log('Telephony_CallManager_combineConference_Async_0200 finish : err = ' + err);
                            done();
                            return;
                        }
                        console.log('Telephony_CallManager_combineConference_Async_0200 fail');
                        expect().assertFail();
                        console.log('Telephony_CallManager_combineConference_Async_0200 finish : err = ' + err);
                    })
                    done();
                    return;
                }
                await sleep(waitTime);
                endTime = new Date().getTime();
                timeOut = endTime - startTime;
                if (timeOut > onTime) {
                    done();
                    break;
                }
            }
        })
    })

    /*
     * @tc.number  Telephony_CallManager_combineConference_Promise_0200
     * @tc.name    Make a call, current callId, call combineConference() to merge the conference call, and capture err
     * @tc.desc    Function test
     */
    it('Telephony_CallManager_combineConference_Promise_0200', 0, async function (done) {
        try {
            var data = await call.dial(TEST_PHONY_NUMBER);
            expect(data).assertTrue();
            console.log('Telephony_CallManager_combineConference_Promise_0200 dial finish : data = ' + data);
            const startTime = new Date().getTime();
            while (true) {
                if (callState === call.CALL_STATUS_DIALING || callState === call.CALL_STATUS_ACTIVE) {
                    try {
                        await call.combineConference(callId);
                        expect().assertFail();
                        console.log('Telephony_CallManager_combineConference_Promise_0200 fail');
                    } catch (err) {
                        console.log('Telephony_CallManager_combineConference_Promise_0200 finish : err = ' + err);
                        done();
                    }
                    done();
                    return;
                }
                await sleep(waitTime);
                endTime = new Date().getTime();
                timeOut = endTime - startTime;
                if (timeOut > onTime) {
                    done();
                    break;
                }
            }
        } catch (err) {
            expect().assertFail();
            console.log('Telephony_CallManager_combineConference_Promise_0200 dial fail : err = ' + err);
            done();
        }
    })

    /*
     * @tc.number  Telephony_CallManager_getMainCallId_Async_0200
     * @tc.name    Call getMainCallId() to get the call of the main call, and return -1
     * @tc.desc    Function test
     */
    it('Telephony_CallManager_getMainCallId_Async_0200', 0, async function (done) {
        call.dial(TEST_PHONY_NUMBER,async (err, data) => {
            if (err) {
                expect().assertFail();
                console.log('Telephony_CallManager_getMainCallId_Async_0200 dial fail : err = ' + err);
                done();
                return;
            }
            expect(data).assertTrue();
            console.log('Telephony_CallManager_getMainCallId_Async_0200 dial finish : data = ' + data);
            const startTime = new Date().getTime();
            while (true) {
                if (callState === call.CALL_STATUS_DIALING || callState === call.CALL_STATUS_ACTIVE) {
                    call.getMainCallId(callId, (err, data) => {
                        if (err) {
                            expect().assertFail();
                            console.log('Telephony_CallManager_getMainCallId_Async_0200 fail : err = ' + err);
                        }
                        expect(data === GETMAIN_CALLID_ERRO).assertTrue();
                        console.log('Telephony_CallManager_getMainCallId_Async_0200 finish : data = ' + data);
                    })
                    done();
                    return;
                }
                await sleep(waitTime);
                endTime = new Date().getTime();
                timeOut = endTime - startTime;
                if (timeOut > onTime) {
                    done();
                    break;
                }
            }
        })
    })

    /*
     * @tc.number  Telephony_CallManager_getMainCallId_Promise_0200
     * @tc.name    Call getMainCallId() to get the call of the main call, and return -1
     * @tc.desc    Function test
     */
    it('Telephony_CallManager_getMainCallId_Promise_0200', 0, async function (done) {
        try {
            var data = await call.dial(TEST_PHONY_NUMBER);
            expect(data).assertTrue();
            console.log('Telephony_CallManager_getMainCallId_Promise_0200 dial finish : data = ' + data);
            const startTime = new Date().getTime();
            while (true) {
                if (callState === call.CALL_STATUS_DIALING || callState === call.CALL_STATUS_ACTIVE) {
                    try {
                        var data = await call.getMainCallId(callId);
                        expect(data === GETMAIN_CALLID_ERRO).assertTrue();
                        console.log('Telephony_CallManager_getMainCallId_Promise_0200 finish : data = ' + data);
                    } catch (err) {
                        expect().assertFail();
                        console.log('Telephony_CallManager_getMainCallId_Promise_0200 fail : err = ' + err);
                        return;
                    }
                    done();
                    return;
                }
                await sleep(waitTime);
                endTime = new Date().getTime();
                timeOut = endTime - startTime;
                if (timeOut > onTime) {
                    done();
                    break;
                }
            }
        } catch (err) {
            expect().assertFail();
            console.log('Telephony_CallManager_getMainCallId_Promise_0200 dial fail : err = ' + err);
            done();
        }
    })

    /*
     * @tc.number  Telephony_CallManager_getSubCallIdList_Async_0200
     * @tc.name    If callId is 999, call getSubCallIdList() to get the call list of subcalls and return the empty list
     * @tc.desc    Function test
     */
    it('Telephony_CallManager_getSubCallIdList_Async_0200', 0, async function (done) {
        call.dial(TEST_PHONY_NUMBER, async (err, data) => {
            if (err) {
                expect().assertFail();
                console.log('Telephony_CallManager_getSubCallIdList_Async_0200 dial fail : err = ' + err);
                done();
                return;
            }
            expect(data).assertTrue();
            console.log('Telephony_CallManager_getSubCallIdList_Async_0200  dial finish : data = ' + data);
            const startTime = new Date().getTime();
            while (true) {
                if (callState === call.CALL_STATUS_DIALING || callState === call.CALL_STATUS_ACTIVE) {
                    call.getSubCallIdList(callId, (err, data) => {
                        if (err) {
                            expect().assertFail();
                            console.log('Telephony_CallManager_getSubCallIdList_Async_0200 fail : err = ' + err);
                            return;
                        }
                        expect(data.length === 0).assertTrue();
                        console.log('Telephony_CallManager_getSubCallIdList_Async_0200 finish : data = ' + data);
                    })
                    done();
                    return;
                }
                await sleep(waitTime);
                endTime = new Date().getTime();
                timeOut = endTime - startTime;
                if (timeOut > onTime) {
                    done();
                    break;
                }
            }
        })
    })

    /*
     * @tc.number  Telephony_CallManager_getSubCallIdList_Promise_0200
     * @tc.name    Call getSubCallIdList() to get the call list of child calls. The empty list is returned
     * @tc.desc    Function test
     */
    it('Telephony_CallManager_getSubCallIdList_Promise_0200', 0, async function (done) {
        try {
            var data = await call.dial(TEST_PHONY_NUMBER);
            expect(data).assertTrue();
            console.log('Telephony_CallManager_getSubCallIdList_Promise_0200 dial finish : data = ' + data);
            const startTime = new Date().getTime();
            while (true) {
                if (callState === call.CALL_STATUS_DIALING || callState === call.CALL_STATUS_ACTIVE) {
                    try {
                        var data = await call.getSubCallIdList(callId);
                        expect(data.length === 0).assertTrue();
                        console.log('Telephony_CallManager_getSubCallIdList_Promise_0200 finish : data = ' + data);
                    } catch (err) {
                        console.log('Telephony_CallManager_getSubCallIdList_Promise_0200 fail : err = ' + err);
                        expect().assertFail();
                        return;
                    }
                    done();
                    return;
                }
                await sleep(waitTime);
                endTime = new Date().getTime();
                timeOut = endTime - startTime;
                if (timeOut > onTime) {
                    done();
                    break;
                }
            }
        } catch (err) {
            expect().assertFail();
            console.log('Telephony_CallManager_getSubCallIdList_Promise_0200 dial fail : err = ' + err);
            done();
        }
    })

    /*
     * @tc.number  Telephony_CallManager_getCallIdListForConference_Async_0200
     * @tc.name    Make a phone call, the current callId, calls the callback way getCallIdListForConference () to
     *             obtain a list all the phone call in the conference call, returns an empty list
     * @tc.desc    Function test
     */
    it('Telephony_CallManager_getCallIdListForConference_Async_0200', 0, async function (done) {
        call.dial(TEST_PHONY_NUMBER, async (err, data) => {
            if (err) {
                expect().assertFail();
                console.log('Telephony_CallManager_getCallIdListForConference_Async_0200 dial fail : err = ' + err);
                done();
                return;
            }
            expect(data).assertTrue();
            console.log('Telephony_CallManager_getCallIdListForConference_Async_0200  dial finish : data = ' + data);
            const startTime = new Date().getTime();
            while (true) {
                if (callState === call.CALL_STATUS_DIALING || callState === call.CALL_STATUS_ACTIVE) {
                    call.getCallIdListForConference(callId, (err, data) => {
                        if (err) {
                            expect().assertFail();
                            console.log(
                                'Telephony_CallManager_getCallIdListForConference_Async_0200 fail : err = ' + err
                            );
                            return;
                        }
                        expect(data.length === 0).assertTrue();
                        console.log(
                            'Telephony_CallManager_getCallIdListForConference_Async_0200 finish : data = ' + data
                        );
                    })
                    done();
                    return;
                }
                await sleep(waitTime);
                endTime = new Date().getTime();
                timeOut = endTime - startTime;
                if (timeOut > onTime) {
                    done();
                    break;
                }
            }
        })
    })

    /*
     * @tc.number  Telephony_CallManager_getCallIdListForConference_Promise_0200
     * @tc.name    Make a phone call, the current callId, call getCallIdListForConference () to obtain a list all
     *             the phone call in the conference call, returns an empty list
     * @tc.desc    Function test
     */
    it('Telephony_CallManager_getCallIdListForConference_Promise_0200', 0, async function (done) {
        try {
            var data = await call.dial(TEST_PHONY_NUMBER);
            expect(data).assertTrue();
            console.log('Telephony_CallManager_getCallIdListForConference_Promise_0200 dial finish : data = ' + data);
            const startTime = new Date().getTime();
            while (true) {
                if (callState === call.CALL_STATUS_DIALING || callState === call.CALL_STATUS_ACTIVE) {
                    try {
                        var data = await call.getCallIdListForConference(callId);
                        expect(data.length === 0).assertTrue();
                        console.log(
                            'Telephony_CallManager_getCallIdListForConference_Promise_0200 finish : data = ' + data
                        );
                    } catch (err) {
                        expect().assertFail();
                        console.log(
                            'Telephony_CallManager_getCallIdListForConference_Promise_0200 fail : err = ' + err
                        );
                        return;
                    }
                    done();
                    return;
                }
                await sleep(waitTime);
                endTime = new Date().getTime();
                timeOut = endTime - startTime;
                if (timeOut > onTime) {
                    done();
                    break;
                }
            }
        } catch (err) {
            expect().assertFail();
            console.log('Telephony_CallManager_getCallIdListForConference_Promise_0200 dial fail : err = ' + err);
            done();
        }
    })

    /*
     * @tc.number  Telephony_CallManager_isInEmergencyCall_Async_0100
     * @tc.name    Make a call to a normal number (10086) and call isInEmergencyCall() to
     *             verify an emergency call, returning false
     * @tc.desc    Function test
     */
    it('Telephony_CallManager_isInEmergencyCall_Async_0100', 0, async function (done) {
        call.dial(TEST_PHONY_NUMBER, async (err, data) => {
            if (err) {
                expect().assertFail();
                console.log('Telephony_CallManager_isInEmergencyCall_Async_0100 dial fail : err = ' + err);
                done();
                return;
            }
            expect(data).assertTrue();
            console.log('Telephony_CallManager_isInEmergencyCall_Async_0100  dial finish : data = ' + data);
            const startTime = new Date().getTime();
            while (true) {
                if (callState === call.CALL_STATUS_DIALING || callState === call.CALL_STATUS_ACTIVE) {
                    call.isInEmergencyCall((err, data) => {
                        if (err) {
                            console.log(
                                'Telephony_CallManager_isInEmergencyCall_Async_0100 isInEmrgencyCall fail : err = ' +
                                err
                            );
                            expect().assertFail();
                        }
                        expect(data === false).assertTrue();
                        console.log(
                            'Telephony_CallManager_isInEmergencyCall_Async_0100 isInEmrgencyCall finish : data = ' +
                            data
                        );
                    })
                    done();
                    return;
                }
                await sleep(waitTime);
                endTime = new Date().getTime();
                timeOut = endTime - startTime;
                if (timeOut > onTime) {
                    done();
                    break;
                }
            }
        })
    })

    /*
     * @tc.number  Telephony_CallManager_isInEmergencyCall_Async_0200
     * @tc.name    Dial an emergency number (112) and call isInEmergencyCall() to determine whether an emergency
     *             call is made, returning true
     * @tc.desc    Function test
     */
    it('Telephony_CallManager_isInEmergencyCall_Async_0200', 0, async function (done) {
        call.dial('112', async (err, data) => {
            if (err) {
                expect().assertFail();
                console.log('Telephony_CallManager_isInEmergencyCall_Async_0200 dial fail : err = ' + err);
                done();
                return;
            }
            expect(data).assertTrue();
            console.log('Telephony_CallManager_isInEmergencyCall_Async_0200  dial finish : data = ' + data);
            const startTime = new Date().getTime();
            while (true) {
                if (callState === call.CALL_STATUS_DIALING || callState === call.CALL_STATUS_ACTIVE) {
                    call.isInEmergencyCall((err, data) => {
                        if (err) {
                            console.log(
                                'Telephony_CallManager_isInEmergencyCall_Async_0200 isInEmrgencyCall fail : err = ' +
                                err
                            );
                            expect().assertFail();
                            return;
                        }
                        expect(data).assertTrue();
                        console.log(
                            'Telephony_CallManager_isInEmergencyCall_Async_0200 isInEmrgencyCall finish : data = ' +
                            data
                        );
                    })
                    done();
                    return;
                }
                await sleep(waitTime);
                endTime = new Date().getTime();
                timeOut = endTime - startTime;
                if (timeOut > onTime) {
                    done();
                    break;
                }
            }
        })
    })

    /*
     * @tc.number  Telephony_CallManager_isInEmergencyCall_Promise_0100
     * @tc.name    False is returned when an emergency call is made to a common number 'INVALID_NUMBER'
     *             (emergencyCall ())
     * @tc.desc    Function test
     */
    it('Telephony_CallManager_isInEmergencyCall_Promise_0100', 0, async function (done) {
        try {
            var data = await call.dial(INVALID_NUMBER);
            expect(data).assertTrue();
            console.log('Telephony_CallManager_isInEmergencyCall_Promise_0100 dial finish : data = ' + data);
            const startTime = new Date().getTime();
            while (true) {
                if (callState === call.CALL_STATUS_DIALING || callState === call.CALL_STATUS_ACTIVE) {
                    try {
                        var data = await call.isInEmergencyCall();
                        expect(data === false).assertTrue();
                        console.log('Telephony_CallManager_isInEmergencyCall_Promise_0100 finish : data = ' + data);
                    } catch (err) {
                        expect().assertFail();
                        console.log('Telephony_CallManager_isInEmergencyCall_Promise_0100 fail : err = ' + err);
                    }
                    done();
                    return;
                }
                await sleep(waitTime);
                endTime = new Date().getTime();
                timeOut = endTime - startTime;
                if (timeOut > onTime) {
                    done();
                    break;
                }
            }
        } catch (err) {
            expect().assertFail();
            console.log('Telephony_CallManager_isInEmergencyCall_Promise_0100 dial fail : err = ' + err);
            done();
        }
    })

    /*
     * @tc.number  Telephony_CallManager_isInEmergencyCall_Promise_0200
     * @tc.name    Dial an emergency number (112) and call isInEmergencyCall() to determine whether
     *              an emergency call is made. The return value is true
     * @tc.desc    Function test
     */
    it('Telephony_CallManager_isInEmergencyCall_Promise_0200', 0, async function (done) {
        try {
            var data = await call.dial('112');
            expect(data).assertTrue();
            console.log('Telephony_CallManager_isInEmergencyCall_Promise_0200 dial finish : data = ' + data);
            const startTime = new Date().getTime();
            while (true) {
                if (callState === call.CALL_STATUS_DIALING || callState === call.CALL_STATUS_ACTIVE) {
                    try {
                        var data = await call.isInEmergencyCall();
                        expect(data).assertTrue();
                        console.log('Telephony_CallManager_isInEmergencyCall_Promise_0200 finish : data = ' + data);
                    } catch (err) {
                        expect().assertFail();
                        console.log('Telephony_CallManager_isInEmergencyCall_Promise_0200 fail : err = ' + err);
                    }
                    done();
                    return;
                }
                await sleep(waitTime);
                endTime = new Date().getTime();
                timeOut = endTime - startTime;
                if (timeOut > onTime) {
                    done();
                    break;
                }
            }
        } catch (err) {
            expect().assertFail();
            console.log('Telephony_CallManager_isInEmergencyCall_Promise_0200 dial fail : err = ' + err);
            done();
        }
    })
})