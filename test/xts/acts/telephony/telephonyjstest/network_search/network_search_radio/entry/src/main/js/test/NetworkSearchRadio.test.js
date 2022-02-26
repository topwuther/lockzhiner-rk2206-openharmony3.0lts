/*
 * Copyright (C) 2021 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the 'License')
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http:www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an 'AS IS' BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

import radio from '@ohos.telephony.radio';
import { describe, beforeAll, beforeEach, afterEach, afterAll, it, expect } from 'deccjsunit/index';

describe('NetworkSearchTest', function () {
    const SLOT_0 = 0;
    const SLOT_ID_ERR = -3;
    const ISO_COUNTRY_CODE = 'cn';
    const PLMN_SIZE = 5;

    /**
     * @tc.number  Telephony_NetworkSearch_turnOnRadio_Async_0100
     * @tc.name    Radio is off, test The Radio module is turned on and the Radio status is changed to 'true'
     * @tc.desc    Function test
     */
    it('Telephony_NetworkSearch_turnOnRadio_Async_0100', 0, async function (done) {
        try {
            await radio.turnOnRadio();
            console.log('Telephony_NetworkSearch_recoverRadioState success');
        } catch (err) {
            console.log(`Telephony_NetworkSearch_recoverRadioState fail ${err}`);
            expect().assertFail();
            done();
            return;
        }
        radio.turnOffRadio((err) => {
            if (err) {
                console.log(`Telephony_NetworkSearch_turnOnRadio_Async_0100 turnOff fail: ${err}`);
                expect().assertFail();
                done();
                return;
            }
            console.log('Telephony_NetworkSearch_turnOnRadio_Async_0100 turnOff finish');
            radio.turnOnRadio((err) => {
                if (err) {
                    console.log(`Telephony_NetworkSearch_turnOnRadio_Async_0100 turnOn fail: ${err}`);
                    expect().assertFail();
                    done();
                    return;
                }
                console.log('Telephony_NetworkSearch_turnOnRadio_Async_0100 turnOn finish');
                radio.isRadioOn((err, data) => {
                    if (err) {
                        console.log(`Telephony_NetworkSearch_turnOnRadio_Async_0100 isOn fail: ${err}`);
                        expect().assertFail();
                        done();
                        return;
                    }
                    console.log(`Telephony_NetworkSearch_turnOnRadio_Async_0100 finish data: ${data}`);
                    expect(data).assertTrue();
                    done();
                });
            });
        });
    });

    /**
     * @tc.number  Telephony_NetworkSearch_turnOnRadio_Async_0200
     * @tc.name    Radio is on, test The Radio module is turned on and the Radio status is changed to 'true'
     * @tc.desc    Function test
     */
    it('Telephony_NetworkSearch_turnOnRadio_Async_0200', 0, async function (done) {
        try {
            await radio.turnOnRadio();
            console.log('Telephony_NetworkSearch_recoverRadioState success');
        } catch (err) {
            console.log(`Telephony_NetworkSearch_recoverRadioState fail ${err}`);
            expect().assertFail();
            done();
            return;
        }
        radio.turnOnRadio((err) => {
            if (err) {
                console.log(`Telephony_NetworkSearch_turnOnRadio_Async_0200 turnOn1 fail: ${err}`);
                expect().assertFail();
                done();
                return;
            }
            console.log('Telephony_NetworkSearch_turnOnRadio_Async_0200 turnOn1 finish');
            radio.turnOnRadio((err) => {
                if (err) {
                    console.log(`Telephony_NetworkSearch_turnOnRadio_Async_0200 turnOn2 fail: ${err}`);
                    expect().assertFail();
                    done();
                    return;
                }
                console.log('Telephony_NetworkSearch_turnOnRadio_Async_0200 turnOn2 finish');
                radio.isRadioOn((err, data) => {
                    if (err) {
                        console.log(`Telephony_NetworkSearch_turnOnRadio_Async_0200 isOn fail: ${err}`);
                        expect().assertFail();
                        done();
                        return;
                    }
                    expect(data).assertTrue();
                    console.log(`Telephony_NetworkSearch_turnOnRadio_Async_0200 finish data: ${data}`);
                    done();
                });
            });
        });
    });

    /**
     * @tc.number  Telephony_NetworkSearch_turnOffRadio_Async_0100
     * @tc.name    Radio is on, test The Radio module is turned off and the Radio status is changed to 'false'
     * @tc.desc    Function test
     */
    it('Telephony_NetworkSearch_turnOffRadio_Async_0100', 0, async function (done) {
        try {
            await radio.turnOnRadio();
            console.log('Telephony_NetworkSearch_recoverRadioState success');
        } catch (err) {
            console.log(`Telephony_NetworkSearch_recoverRadioState fail ${err}`);
            expect().assertFail();
            done();
            return;
        }
        radio.turnOnRadio((err) => {
            if (err) {
                console.log(`Telephony_NetworkSearch_turnOffRadio_Async_0100 turnOn fail: ${err}`);
                expect().assertFail();
                done();
                return;
            }
            console.log('Telephony_NetworkSearch_turnOffRadio_Async_0100 turnOn1 finish');
            radio.turnOffRadio((err) => {
                if (err) {
                    console.log(`Telephony_NetworkSearch_turnOffRadio_Async_0100 turnOff fail: ${err}`);
                    expect().assertFail();
                    done();
                    return;
                }
                console.log('Telephony_NetworkSearch_turnOffRadio_Async_0100 turnOff finish');
                radio.isRadioOn((err, data) => {
                    if (err) {
                        console.log(`Telephony_NetworkSearch_turnOffRadio_Async_0100 isOn fail: ${err}`);
                        expect().assertFail();
                        done();
                        return;
                    }
                    console.log(`Telephony_NetworkSearch_turnOffRadio_Async_0100 finish data: ${data}`)
                    expect(data).assertFalse();
                    done();
                });
            });
        });
    });

    /**
     * @tc.number  Telephony_NetworkSearch_turnOffRadio_Async_0200
     * @tc.name    Radio is off, test The Radio module is turned off and the Radio status is changed to 'false'
     * @tc.desc    Function test
     */
    it('Telephony_NetworkSearch_turnOffRadio_Async_0200', 0, async function (done) {
        try {
            await radio.turnOnRadio();
            console.log('Telephony_NetworkSearch_recoverRadioState success');
        } catch (err) {
            console.log(`Telephony_NetworkSearch_recoverRadioState fail ${err}`);
            expect().assertFail();
            done();
            return;
        }
        radio.turnOffRadio((err) => {
            if (err) {
                console.log(`Telephony_NetworkSearch_turnOffRadio_Async_0200 turnOff1 fail: ${err}`);
                expect().assertFail();
                done();
                return;
            }
            console.log('Telephony_NetworkSearch_turnOffRadio_Async_0200 turnOff1 finish');
            radio.turnOffRadio((err) => {
                if (err) {
                    console.log(`Telephony_NetworkSearch_turnOffRadio_Async_0200 turnOff2 fail: ${err}`);
                    expect().assertFail();
                    done();
                    return;
                }
                console.log('Telephony_NetworkSearch_turnOffRadio_Async_0200 turnOff2 finish');
                radio.isRadioOn((err, data) => {
                    if (err) {
                        console.log(`Telephony_NetworkSearch_turnOffRadio_Async_0200 isOn fail: ${err}`);
                        expect().assertFail();
                        done();
                        return;
                    }
                    console.log(`Telephony_NetworkSearch_turnOffRadio_Async_0200 finish data: ${data}`)
                    expect(data).assertFalse();
                    done();
                });
            });
        });
    });

    /**
     * @tc.number  Telephony_NetworkSearch_turnOffRadio_Async_0500
     * @tc.name    Radio is off, test the getNetworkState() longOperatorName is null and shortOperatorName is null and
     *             plmnNumeric is null and regStatus is REG_STATE_EMERGENCY_CALL_ONLY and nsaState is
     *             NSA_STATE_NOT_SUPPORT and
     *             isRoaming is false and isCaActive is false and isEmergency is false
     * @tc.desc    Function test
     */
    it('Telephony_NetworkSearch_turnOffRadio_Async_0500', 0, async function (done) {

        try {
            await radio.turnOnRadio();
            console.log('Telephony_NetworkSearch_recoverRadioState success');
        } catch (err) {
            console.log(`Telephony_NetworkSearch_recoverRadioState fail ${err}`);
            expect().assertFail();
            done();
            return;
        }
        radio.turnOffRadio((err) => {
            if (err) {
                console.log(`Telephony_NetworkSearch_turnOffRadio_Async_0500 turnOff fail err: ${err}`);
                expect().assertFail();
                done();
                return;
            }
            console.log('Telephony_NetworkSearch_turnOffRadio_Async_0500 turnOff finish');
            radio.getNetworkState((err, data) => {
                if (err) {
                    console.log(`Telephony_NetworkSearch_turnOffRadio_Async_0500 get fail err: ${err}`);
                    expect().assertFail();
                    done();
                    return;
                }
                console.log(`Telephony_NetworkSearch_turnOffRadio_Async_0500 finish data: ${JSON.stringify(data)}`);
                expect(
                    data.longOperatorName === undefined || data.longOperatorName === '' ||
                     data.longOperatorName === null
                ).assertTrue();
                expect(
                    data.shortOperatorName === undefined || data.shortOperatorName === '' ||
                     data.shortOperatorName === null
                ).assertTrue();
                expect(data.plmnNumeric === undefined || data.plmnNumeric === '' ||
                data.plmnNumeric === null).assertTrue();
                expect(data.regStatus === radio.REG_STATE_POWER_OFF).assertTrue();
                expect(data.nsaState === radio.NSA_STATE_NOT_SUPPORT).assertTrue();
                expect(data.isRoaming === false).assertTrue();
                expect(data.isCaActive === false).assertTrue();
                expect(data.isEmergency === false).assertTrue();
                done();
            });
        });
    });

    /**
     * @tc.number  Telephony_NetworkSearch_turnOffRadio_Async_0600
     * @tc.name    Radio is off, test the getSignalInformation() return the data length is 0
     * @tc.desc    Function test
     */
    it('Telephony_NetworkSearch_turnOffRadio_Async_0600', 0, async function (done) {
        try {
            await radio.turnOnRadio();
            console.log('Telephony_NetworkSearch_recoverRadioState success');
        } catch (err) {
            console.log(`Telephony_NetworkSearch_recoverRadioState fail ${err}`);
            expect().assertFail();
            done();
            return;
        }
        radio.turnOffRadio((err) => {
            if (err) {
                console.log(`Telephony_NetworkSearch_turnOffRadio_Async_0600 turnOff fail err: ${err}`);
                expect().assertFail();
                done();
                return;
            }
            console.log('Telephony_NetworkSearch_turnOffRadio_Async_0600 turnOff finish');
            radio.getSignalInformation(SLOT_0, (err, data) => {
                if (err) {
                    console.log(`Telephony_NetworkSearch_turnOffRadio_Async_0600 get fail err: ${err}`);
                    expect().assertFail();
                    done();
                    return;
                }
                console.log(`Telephony_NetworkSearch_turnOffRadio_Async_0600 finish data: ${JSON.stringify(data)}`);
                expect(data != null && data != undefined).assertTrue();
                expect(data.length === 0).assertTrue();
                done();
            });
        });
    });

    /**
     * @tc.number  Telephony_NetworkSearch_turnOffRadio_Async_0700
     * @tc.name    Radio is off, test the getRadioTech() return the psRadioTech is RADIO_TECHNOLOGY_UNKNOWN and
     *             csRadioTech is RADIO_TECHNOLOGY_UNKNOWN
     * @tc.desc    Function test
     */
    it('Telephony_NetworkSearch_turnOffRadio_Async_0700', 0, async function (done) {
        try {
            await radio.turnOnRadio();
            console.log('Telephony_NetworkSearch_recoverRadioState success');
        } catch (err) {
            console.log(`Telephony_NetworkSearch_recoverRadioState fail ${err}`);
            expect().assertFail();
            done();
            return;
        }
        radio.turnOffRadio((err) => {
            if (err) {
                console.log(`Telephony_NetworkSearch_turnOffRadio_Async_0700 turnOff fail err: ${err}`);
                expect().assertFail();
                done();
                return;
            }
            console.log('Telephony_NetworkSearch_turnOffRadio_Async_0700 turnOff finish');
            radio.getRadioTech(SLOT_0, (err, { psRadioTech,
                csRadioTech }) => {
                if (err) {
                    console.log(`Telephony_NetworkSearch_turnOffRadio_Async_0700 get fail err: ${err}`);
                    expect().assertFail();
                    done();
                    return;
                }
                console.log(`Telephony_NetworkSearch_turnOffRadio_Async_0700 finish psRadioTech: ${psRadioTech}
                 csRadioTech: ${csRadioTech}`);
                expect(psRadioTech === radio.RADIO_TECHNOLOGY_UNKNOWN).assertTrue();
                expect(csRadioTech === radio.RADIO_TECHNOLOGY_UNKNOWN).assertTrue();
                done();
            });
        });
    });

    /**
     * @tc.number  Telephony_NetworkSearch_isRadioOn_Async_0100
     * @tc.name    Turn off Radio, input slotId parameter as 1, test isRadioOn() returns 'false'
     * @tc.desc    Function test
     */
    it('Telephony_NetworkSearch_isRadioOn_Async_0100', 0, async function (done) {
        try {
            await radio.turnOnRadio();
            console.log('Telephony_NetworkSearch_recoverRadioState success');
        } catch (err) {
            console.log(`Telephony_NetworkSearch_recoverRadioState fail ${err}`);
            expect().assertFail();
            done();
            return;
        }
        radio.turnOffRadio((err) => {
            if (err) {
                console.log(`Telephony_NetworkSearch_isRadioOn_Async_0100 turnOff fail err: ${err}`);
                expect().assertFail();
                done();
                return;
            }
            console.log('Telephony_NetworkSearch_isRadioOn_Async_0100 turnOff finish');
            radio.isRadioOn((err, data) => {
                if (err) {
                    console.log(`Telephony_NetworkSearch_isRadioOn_Async_0100 isOn fail err: ${err}`);
                    expect().assertFail();
                    done();
                    return;
                }
                console.log(`Telephony_NetworkSearch_isRadioOn_Async_0100 finish data: ${data}`);
                expect(data === false).assertTrue();
                done();
            });

        });
    });

    /**
     * @tc.number  Telephony_NetworkSearch_isRadioOn_Async_0200
     * @tc.name    Turn on Radio, input slotId parameter as 1, test isRadioOn() returns 'true'
     * @tc.desc    Function test
     */
    it('Telephony_NetworkSearch_isRadioOn_Async_0200', 0, async function (done) {
        try {
            await radio.turnOnRadio();
            console.log('Telephony_NetworkSearch_recoverRadioState success');
        } catch (err) {
            console.log(`Telephony_NetworkSearch_recoverRadioState fail ${err}`);
            expect().assertFail();
            done();
            return;
        }
        radio.turnOnRadio((err) => {
            if (err) {
                console.log(`Telephony_NetworkSearch_isRadioOn_Async_0200 turnOn fail err:${err}`);
                expect().assertFail();
                done();
                return;
            }
            console.log('Telephony_NetworkSearch_isRadioOn_Async_0200 turnOn finish');
            radio.isRadioOn((err, data) => {
                if (err) {
                    console.log(`Telephony_NetworkSearch_isRadioOn_Async_0200 isOn fail err:${err}`);
                    expect().assertFail();
                    done();
                    return;
                }
                console.log(`Telephony_NetworkSearch_isRadioOn_Async_0200 finish data: ${data}`);
                expect(data).assertTrue();
                done();
            });
        });
    });

    /**
     * @tc.number  Telephony_NetworkSearch_turnOnRadio_Promise_0100
     * @tc.name    Radio is off, test The Radio module is turned on and the Radio status is changed to 'true'
     * @tc.desc    Function test
     */
    it('Telephony_NetworkSearch_turnOnRadio_Promise_0100', 0, async function (done) {
        try {
            await radio.turnOnRadio();
            console.log('Telephony_NetworkSearch_recoverRadioState success');
        } catch (err) {
            console.log(`Telephony_NetworkSearch_recoverRadioState fail ${err}`);
            expect().assertFail();
            done();
            return;
        }
        try {
            await radio.turnOffRadio();
            console.log('Telephony_NetworkSearch_turnOnRadio_Promise_0100 turnOff finish');
        } catch (err) {
            console.log(`Telephony_NetworkSearch_turnOnRadio_Promise_0100 turnOff fail err: ${err}`);
            expect().assertFail();
            done();
            return;
        }
        try {
            await radio.turnOnRadio();
            console.log('Telephony_NetworkSearch_turnOnRadio_Promise_0100 turnOn finish');
        } catch (err) {
            console.log(`Telephony_NetworkSearch_turnOnRadio_Promise_0100 turnOn fail err: ${err}`);
            expect().assertFail();
            done();
            return;
        }
        try {
            let data = await radio.isRadioOn();
            console.log(`Telephony_NetworkSearch_turnOnRadio_Promise_0100 finish data: ${data}`);
            expect(data).assertTrue();
        } catch (err) {
            console.log(`Telephony_NetworkSearch_turnOnRadio_Promise_0100  fail err: ${err}`);
            expect().assertFail();
            done();
            return;
        }

        done();
    });

    /**
     * @tc.number  Telephony_NetworkSearch_turnOnRadio_Promise_0200
     * @tc.name    Radio is on, test The Radio module is turned on and the Radio status is changed to 'true'
     * @tc.desc    Function test
     */
    it('Telephony_NetworkSearch_turnOnRadio_Promise_0200', 0, async function (done) {
        try {
            await radio.turnOnRadio();
            console.log('Telephony_NetworkSearch_recoverRadioState success');
        } catch (err) {
            console.log(`Telephony_NetworkSearch_recoverRadioState fail ${err}`);
            expect().assertFail();
            done();
            return;
        }
        try {
            await radio.turnOnRadio();
            console.log('Telephony_NetworkSearch_turnOnRadio_Promise_0200 turnOn finish');
        } catch (err) {
            console.log(`Telephony_NetworkSearch_turnOnRadio_Promise_0200 turnOn fail err ${err}`);
            expect().assertFail();
            done();
        }
        try {
            let data = await radio.isRadioOn();
            console.log(`Telephony_NetworkSearch_turnOnRadio_Promise_0200 finish data: ${data}`);
            expect(data).assertTrue();
        } catch (err) {
            console.log(`Telephony_NetworkSearch_turnOnRadio_Promise_0200 fail err ${err}`);
            expect().assertFail();
            done();
            return;
        }
        done();
    });

    /**
     * @tc.number  Telephony_NetworkSearch_turnOffRadio_Promise_0100
     * @tc.name    Radio is on, test The Radio module is turned off and the Radio status is changed to 'false'
     * @tc.desc    Function test
     */
    it('Telephony_NetworkSearch_turnOffRadio_Promise_0100', 0, async function (done) {
        try {
            await radio.turnOnRadio();
            console.log('Telephony_NetworkSearch_recoverRadioState success');
        } catch (err) {
            console.log(`Telephony_NetworkSearch_recoverRadioState fail ${err}`);
            expect().assertFail();
            done();
            return;
        }
        try {
            await radio.turnOffRadio();
            console.log('Telephony_NetworkSearch_turnOffRadio_Promise_0100 turnOff finish');
        } catch (err) {
            console.log(`Telephony_NetworkSearch_turnOffRadio_Promise_0100 turnOff fail: ${err}`);
            expect().assertFail();
            done();
            return;
        }
        try {
            let data = await radio.isRadioOn();
            console.log(`Telephony_NetworkSearch_turnOffRadio_Promise_0100 finish data: ${data}`);
            expect(data).assertFalse()
        } catch (err) {
            console.log(`Telephony_NetworkSearch_turnOffRadio_Promise_0100 fail: ${err}`);
            expect().assertFail();
            done();
            return;
        }
        done();
    });

    /**
     * @tc.number  Telephony_NetworkSearch_turnOffRadio_Promise_0200
     * @tc.name    Radio is off, test The Radio module is turned off and the Radio status is changed to 'false'
     * @tc.desc    Function test
     */
    it('Telephony_NetworkSearch_turnOffRadio_Promise_0200', 0, async function (done) {
        try {
            await radio.turnOnRadio();
            console.log('Telephony_NetworkSearch_recoverRadioState success');
        } catch (err) {
            console.log(`Telephony_NetworkSearch_recoverRadioState fail ${err}`);
            expect().assertFail();
            done();
            return;
        }
        try {
            await radio.turnOffRadio();
            console.log('Telephony_NetworkSearch_turnOffRadio_Promise_0200 turnOff1 finish');
        } catch (err) {
            console.log(`Telephony_NetworkSearch_turnOffRadio_Promise_0200 turnOff1 fail: ${err}`);
            expect().assertFail();
            done();
            return;
        }
        try {
            await radio.turnOffRadio();
            console.log('Telephony_NetworkSearch_turnOffRadio_Promise_0200 turnOff2 finish');
            done();
            return;
        } catch (err) {
            console.log(`Telephony_NetworkSearch_turnOffRadio_Promise_0200 turnOff2 fail: ${err}`);
            expect().assertFail();
            done();
        }
        try {
            let data = await radio.isRadioOn();
            expect(data).assertFalse()
            console.log('Telephony_NetworkSearch_turnOffRadio_Promise_0200 finish');
        } catch (err) {
            console.log(`Telephony_NetworkSearch_turnOffRadio_Promise_0200 fail: ${err}`);
            expect().assertFail();
            done();
            return;
        }
        done();
    });

    /**
     * @tc.number  Telephony_NetworkSearch_turnOffRadio_Promise_0500
     * @tc.name    Radio is off, test the getNetworkState() longOperatorName is null and shortOperatorName is null and
     *             plmnNumeric is null and regStatus is REG_STATE_EMERGENCY_CALL_ONLY and nsaState is
     *             NSA_STATE_NOT_SUPPORT and
     *             isRoaming is false and isCaActive is false and isEmergency is false
     * @tc.desc    Function test
     */
    it('Telephony_NetworkSearch_turnOffRadio_Promise_0500', 0, async function (done) {
        try {
            await radio.turnOnRadio();
            console.log('Telephony_NetworkSearch_recoverRadioState success');
        } catch (err) {
            console.log(`Telephony_NetworkSearch_recoverRadioState fail ${err}`);
            expect().assertFail();
            done();
            return;
        }
        try {
            await radio.turnOffRadio();
            console.log('Telephony_NetworkSearch_turnOffRadio_Promise_0500 turnOff finish');
        } catch (err) {
            console.log(`Telephony_NetworkSearch_turnOffRadio_Promise_0500 turnOff fail err: ${err}`);
            expect().assertFail();
            done();
            return;
        }
        try {
            let data = await radio.getNetworkState();
            console.log(`Telephony_NetworkSearch_turnOffRadio_Promise_0500 finish data: ${JSON.stringify(data)}`);
            expect(data != null && data != undefined).assertTrue();
            expect(
                data.longOperatorName === undefined || data.longOperatorName === '' || data.longOperatorName === null
            ).assertTrue();
            expect(
                data.shortOperatorName === undefined || data.shortOperatorName === '' || data.shortOperatorName === null
            ).assertTrue();
            expect(data.plmnNumeric === undefined || data.plmnNumeric === '' || data.plmnNumeric === null).assertTrue();
            expect(data.regStatus === radio.REG_STATE_POWER_OFF).assertTrue();
            expect(data.nsaState === radio.NSA_STATE_NOT_SUPPORT).assertTrue();
            expect(data.isRoaming === false).assertTrue();
            expect(data.isCaActive === false).assertTrue();
            expect(data.isEmergency === false).assertTrue();
            
        } catch (err) {
            console.log(`Telephony_NetworkSearch_turnOffRadio_Promise_0500 fail err: ${err}`);
            expect().assertFail();
            done();
            return;
        }
        done();
    });

    /**
     * @tc.number  Telephony_NetworkSearch_turnOffRadio_Promise_0600
     * @tc.name    Radio is off, test the getSignalInformation() return the data length is 0
     * @tc.desc    Function test
     */
    it('Telephony_NetworkSearch_turnOffRadio_Promise_0600', 0, async function (done) {
        try {
            await radio.turnOnRadio();
            console.log('Telephony_NetworkSearch_recoverRadioState success');
        } catch (err) {
            console.log(`Telephony_NetworkSearch_recoverRadioState fail ${err}`);
            expect().assertFail();
            done();
            return;
        }
        try {
            await radio.turnOffRadio();
            console.log('Telephony_NetworkSearch_turnOffRadio_Promise_0600 turnOff finish');
        } catch (err) {
            console.log(`Telephony_NetworkSearch_turnOffRadio_Promise_0600 turnOff fail: ${err}`);
            expect().assertFail();
            done();
            return;
        }
        try {
            let data = await radio.getSignalInformation(SLOT_0);
            console.log(`Telephony_NetworkSearch_turnOffRadio_Promise_0600 finish data: ${data}`);
            expect(data != null && data != undefined).assertTrue();
            expect(data.length === 0).assertTrue();
        } catch (err) {
            console.log(`Telephony_NetworkSearch_turnOffRadio_Promise_0600 fail: ${err}`);
            expect().assertFail();
            done();
            return;
        }
        done();
    });

    /**
     * @tc.number  Telephony_NetworkSearch_turnOffRadio_Promise_0700
     * @tc.name    Radio is off, test the getRadioTech() return the psRadioTech is RADIO_TECHNOLOGY_UNKNOWN and
     *             csRadioTech is RADIO_TECHNOLOGY_UNKNOWN
     * @tc.desc    Function test
     */
    it('Telephony_NetworkSearch_turnOffRadio_Promise_0700', 0, async function (done) {
        try {
            await radio.turnOnRadio();
            console.log('Telephony_NetworkSearch_recoverRadioState success');
        } catch (err) {
            console.log(`Telephony_NetworkSearch_recoverRadioState fail ${err}`);
            expect().assertFail();
            done();
            return;
        }
        try {
            await radio.turnOffRadio();
            console.log('Telephony_NetworkSearch_turnOffRadio_Promise_0700 turnOff finish');
        } catch (err) {
            console.log(`Telephony_NetworkSearch_turnOffRadio_Promise_0700 turnOff fail: ${err}`);
            expect().assertFail();
            done();
            return;
        }
        try {
            let data = await radio.getRadioTech(SLOT_0);
            console.log(`Telephony_NetworkSearch_turnOffRadio_Promise_0700 finish data: ${JSON.stringify(data)}`);
            expect(data != null && data != undefined).assertTrue();
            expect(data.psRadioTech === radio.RADIO_TECHNOLOGY_UNKNOWN).assertTrue();
            expect(data.csRadioTech === radio.RADIO_TECHNOLOGY_UNKNOWN).assertTrue();
        } catch (err) {
            console.log(`Telephony_NetworkSearch_turnOffRadio_Promise_0700 fail err: ${err}`);
            expect().assertFail();
            done();
            return;
        }
        done();
    });

    /**
     * @tc.number  Telephony_NetworkSearch_isRadioOn_Promise_0100
     * @tc.name    Turn off Radio, input slotId parameter as 1, test isRadioOn() returns 'false'
     * @tc.desc    Function test
     */
    it('Telephony_NetworkSearch_isRadioOn_Promise_0100', 0, async function (done) {
        try {
            await radio.turnOnRadio();
            console.log('Telephony_NetworkSearch_recoverRadioState success');
        } catch (err) {
            console.log(`Telephony_NetworkSearch_recoverRadioState fail ${err}`);
            expect().assertFail();
            done();
            return;
        }
        try {
            await radio.turnOffRadio();
            console.log('Telephony_NetworkSearch_isRadioOn_Promise_0100 turnOff finish');
        } catch (err) {
            console.log(`Telephony_NetworkSearch_isRadioOn_Promise_0100 turnOff fail: ${err}`);
            expect().assertFail();
            done();
            return;
        }
        try {
            let data = await radio.isRadioOn();
            expect(data).assertFalse()
            console.log(`Telephony_NetworkSearch_isRadioOn_Promise_0100 finish data: ${data}`);
        } catch (err) {
            console.log(`Telephony_NetworkSearch_isRadioOn_Promise_0100 fail: ${err}`);
            expect().assertFail();
            done();
            return;
        }
        done();
    });

    /**
     * @tc.number  Telephony_NetworkSearch_isRadioOn_Promise_0200
     * @tc.name    Turn on Radio, input slotId parameter as 1, test isRadioOn() returns 'true'
     * @tc.desc    Function test
     */
    it('Telephony_NetworkSearch_isRadioOn_Promise_0200', 0, async function (done) {
        try {
            await radio.turnOnRadio();
            console.log('Telephony_NetworkSearch_recoverRadioState success');
        } catch (err) {
            console.log(`Telephony_NetworkSearch_recoverRadioState fail ${err}`);
            expect().assertFail();
            done();
            return;
        }
        try {
            let data = await radio.isRadioOn();
            expect(data).assertTrue();
            console.log(`Telephony_NetworkSearch_isRadioOn_Promise_0200 finish data: ${data}`);
        } catch (err) {
            console.log(`Telephony_NetworkSearch_isRadioOn_Promise_0200 fail err: ${err}`);
            expect().assertFail();
            done();
            return;
        }
        done();
    });

    /**
     * @tc.number  Telephony_NetworkSearch_getISOCountryCodeForNetwork_Async_0500
     * @tc.name    Radio is off, test the getISOCountryCodeForNetwork() return the data is 'cn'
     * @tc.desc    Function test
     */
    it('Telephony_NetworkSearch_getISOCountryCodeForNetwork_Async_0500', 0, async function (done) {
        try {
            await radio.turnOnRadio();
            console.log('Telephony_NetworkSearch_recoverRadioState success');
        } catch (err) {
            console.log(`Telephony_NetworkSearch_recoverRadioState fail ${err}`);
            expect().assertFail();
            done();
            return;
        }
        radio.turnOffRadio((err) => {
            if (err) {
                console.log(`Telephony_NetworkSearch_getISOCountryCodeForNetwork_Async_0500 turnOff fail: ${err}`);
                expect().assertFail();
                done();
                return;
            }
            console.log('Telephony_NetworkSearch_getISOCountryCodeForNetwork_Async_0500 turnOff finish');
            radio.getISOCountryCodeForNetwork(SLOT_0, (err, data) => {
                if (err) {
                    console.log(`Telephony_NetworkSearch_getISOCountryCodeForNetwork_Async_0500 get fail: ${err}`);
                    expect().assertFail();
                    done();
                    return;
                }
                console.log(`Telephony_NetworkSearch_getISOCountryCodeForNetwork_Async_0500 finish
                 data: ${JSON.stringify(data)}`);
                expect(data === "" || data === undefined || data === null).assertTrue();
                done();
            });
        });
    });

    /**
     * @tc.number  Telephony_NetworkSearch_getISOCountryCodeForNetwork_Promise_0500
     * @tc.name    Radio is off, test the getISOCountryCodeForNetwork() return the data is 'cn'
     * @tc.desc    Function test
     */
    it('Telephony_NetworkSearch_getISOCountryCodeForNetwork_Promise_0500', 0, async function (done) {
        try {
            await radio.turnOnRadio();
            console.log('Telephony_NetworkSearch_recoverRadioState success');
        } catch (err) {
            console.log(`Telephony_NetworkSearch_recoverRadioState fail ${err}`);
            expect().assertFail();
            done();
            return;
        }
        try {
            await radio.turnOffRadio();
            console.log('Telephony_NetworkSearch_getISOCountryCodeForNetwork_Promise_0500 turnOff finish');
        } catch (err) {
            console.log(`Telephony_NetworkSearch_getISOCountryCodeForNetwork_Promise_0500 turnOff fail: ${err}`);
            expect().assertFail();
            done();
            return;
        }
        try {
            let data = await radio.getISOCountryCodeForNetwork(SLOT_0);
            console.log(`Telephony_NetworkSearch_getISOCountryCodeForNetwork_Promise_0500 finish: ${data}`);
            expect(data === "" || data === undefined || data === null).assertTrue();
        } catch (err) {
            console.log(`Telephony_NetworkSearch_getISOCountryCodeForNetwork_Promise_0500 fail: ${err}`);
            expect().assertFail();
            done();
            return;
        }
        try {
            await radio.turnOnRadio();
            console.log('Telephony_NetworkSearch_getISOCountryCodeForNetwork_Promise_0500 turnOn success');
        } catch (err) {
            console.log(`Telephony_NetworkSearch_getISOCountryCodeForNetwork_Promise_0500 turnOff fail ${err}`);
            expect().assertFail();
            done();
            return;
        }
        done();
    });
});
