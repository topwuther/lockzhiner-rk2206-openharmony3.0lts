/*
 * Copyright (C) 2021 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the 'License')
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

import radio from '@ohos.telephony.radio';
import { describe, beforeAll, beforeEach, afterEach, afterAll, it, expect } from 'deccjsunit/index';

describe('NetworkSearchTest', function () {
    //Resident status
    let garrRegState = [
        radio.REG_STATE_NO_SERVICE,
        radio.REG_STATE_IN_SERVICE,
        radio.REG_STATE_EMERGENCY_CALL_ONLY,
        radio.REG_STATE_POWER_OFF,
    ];
    //NSA network registration status
    let garrNsaState = [
        radio.NSA_STATE_NOT_SUPPORT,
        radio.NSA_STATE_NO_DETECT,
        radio.NSA_STATE_CONNECTED_DETECT,
        radio.NSA_STATE_IDLE_DETECT,
        radio.NSA_STATE_DUAL_CONNECTED,
        radio.NSA_STATE_SA_ATTACHED,
    ];
    //Available network formats
    let garrRadioTech = [
        radio.RADIO_TECHNOLOGY_UNKNOWN,
        radio.RADIO_TECHNOLOGY_GSM,
        radio.RADIO_TECHNOLOGY_1XRTT,
        radio.RADIO_TECHNOLOGY_WCDMA,
        radio.RADIO_TECHNOLOGY_HSPA,
        radio.RADIO_TECHNOLOGY_HSPAP,
        radio.RADIO_TECHNOLOGY_TD_SCDMA,
        radio.RADIO_TECHNOLOGY_EVDO,
        radio.RADIO_TECHNOLOGY_EHRPD,
        radio.RADIO_TECHNOLOGY_LTE,
        radio.RADIO_TECHNOLOGY_LTE_CA,
        radio.RADIO_TECHNOLOGY_IWLAN,
        radio.RADIO_TECHNOLOGY_NR,
    ];
    //Network type corresponding to signal
    let garrSignalType = [
        radio.NETWORK_TYPE_UNKNOWN,
        radio.NETWORK_TYPE_GSM,
        radio.NETWORK_TYPE_CDMA,
        radio.NETWORK_TYPE_WCDMA,
        radio.NETWORK_TYPE_TDSCDMA,
        radio.NETWORK_TYPE_LTE,
        radio.NETWORK_TYPE_NR,
    ];
    //Corresponding level of signal
    let garrSignalLevel = [0, 1, 2, 3, 4, 5];
    const SLOT_0 = 0;
    const ISO_COUNTRY_CODE = 'cn';
    const PLMN_SIZE = 5;


    /**
     * @tc.number  Telephony_NetworkSearch_getNetworkState_Async_0100
     * @tc.name    Test the getNetworkState() query function and return the default card 0 network registration status
     * @tc.desc    Function test
     */
    it('Telephony_NetworkSearch_getNetworkState_Async_0100', 0, async function (done) {
        radio.getNetworkState((err, data) => {
            if (err) {
                console.log(`Telephony_NetworkSearch_getNetworkState_Async_0100 fail err: ${err}`);
                expect().assertFail();
                done();
                return;
            }
            console.log(`Telephony_NetworkSearch_getNetworkState_Async_0100 finish data: ${JSON.stringify(data)}`);
            expect(data != null && data != undefined).assertTrue();
            expect(
                data.longOperatorName != undefined &&
                data.longOperatorName != '' &&
                data.longOperatorName != null
            ).assertTrue();
            expect(
                data.shortOperatorName != undefined &&
                data.shortOperatorName != '' &&
                data.shortOperatorName != null
            ).assertTrue();
            expect(
                data.plmnNumeric != undefined &&
                data.plmnNumeric != '' &&
                data.plmnNumeric != null
            ).assertTrue();
            expect(data.plmnNumeric.length == 5 && data.plmnNumeric.substr(0, 3) === '460').assertTrue();
            expect(garrRegState).assertContain(data.regStatus);
            expect(data.nsaState === 1).assertTrue();
            expect(data.isRoaming === false).assertTrue();
            expect(data.isCaActive === false).assertTrue();
            expect(data.isEmergency === false).assertTrue();
            done();
        });
    });

    /**
     * @tc.number  Telephony_NetworkSearch_getNetworkState_Async_0200
     * @tc.name    The slotId parameter input is 0, test the getNetworkState() query function,
     *             and return the network registration status of card 0
     * @tc.desc    Function test
     */
    it('Telephony_NetworkSearch_getNetworkState_Async_0200', 0, async function (done) {
        radio.getNetworkState(SLOT_0, (err, data) => {
            if (err) {
                console.log(`Telephony_NetworkSearch_getNetworkState_Async_0200 fail err: ${err}`);
                expect().assertFail();
                done();
                return;
            }
            console.log(`Telephony_NetworkSearch_getNetworkState_Async_0200 finish data: ${JSON.stringify(data)}`);
            expect(data != null && data != undefined).assertTrue();
            expect(
                data.longOperatorName != undefined && data.longOperatorName != '' && data.longOperatorName != null
            ).assertTrue();
            expect(
                data.shortOperatorName != undefined && data.shortOperatorName != '' && data.shortOperatorName != null
            ).assertTrue();
            expect(data.plmnNumeric.length === PLMN_SIZE && data.plmnNumeric.substr(0, 3) === '460').assertTrue();
            expect(garrRegState).assertContain(data.regStatus);
            expect(data.nsaState === 1).assertTrue();
            expect(data.isRoaming === false).assertTrue();
            expect(data.isCaActive === false).assertTrue();
            expect(data.isEmergency === false).assertTrue();

            done();
        });
    });

    /**
     * @tc.number  Telephony_NetworkSearch_getNetworkState_Async_0700
     * @tc.name    SlotId parameter input is 3, test getNetworkState() query function go to the error
     * @tc.desc    Function test
     */
    it('Telephony_NetworkSearch_getNetworkState_Async_0700', 0, async function (done) {
        let slotId = 3;
        radio.getNetworkState(slotId, (err, data) => {
            if (err) {
                console.log(`Telephony_NetworkSearch_getNetworkState_Async_0700 finish err: ${err}`);
                done();
                return;
            }
            console.log('Telephony_NetworkSearch_getNetworkState_Async_0700 fail not go to err');
            expect().assertFail();
            done();
        });
    });

    /**
     * @tc.number  Telephony_NetworkSearch_getRadioTech_Async_0100
     * @tc.name    SlotId parameter input is 0, test getRadioTech() query function, return PS, CS network mode
     * @tc.desc    Function test
     */
    it('Telephony_NetworkSearch_getRadioTech_Async_0100', 0, async function (done) {
        radio.getRadioTech(SLOT_0, (err, {
            psRadioTech,
            csRadioTech
        }) => {
            if (err) {
                console.log(`Telephony_NetworkSearch_getRadioTech_Async_0100 fail err: ${err}`);
                expect().assertFail();
                done();
                return;
            }
            console.log(`Telephony_NetworkSearch_getRadioTech_Async_0100 finish psRadioTech: ${psRadioTech}
             csRadioTech: ${csRadioTech}`);
            expect(garrRadioTech).assertContain(psRadioTech);
            expect(garrRadioTech).assertContain(csRadioTech);
            done();
        });
    });

    /**
     * @tc.number  Telephony_NetworkSearch_getRadioTech_Async_0400
     * @tc.name    SlotId parameter input is 7, test getRadioTech() query function go to the error
     * @tc.desc    Function test
     */
    it('Telephony_NetworkSearch_getRadioTech_Async_0400', 0, async function (done) {
        let slotId = 7;
        radio.getRadioTech(slotId, (err, {
            psRadioTech,
            csRadioTech
        }) => {
            if (err) {
                console.log(`Telephony_NetworkSearch_getRadioTech_Async_0400 finish err: ${err}`);
                done();
                return;
            }
            console.log('Telephony_NetworkSearch_getRadioTech_Async_0400 fail not go to err');
            expect().assertFail();
            done();
        });
    });

    /**
     * @tc.number  Telephony_NetworkSearch_getSignalInformation_Async_0100
     * @tc.name    SlotId parameter input is 0, test getSignalInformation() query function,
     *             return signal strength list information
     * @tc.desc    Function test
     */
    it('Telephony_NetworkSearch_getSignalInformation_Async_0100', 0, async function (done) {
        radio.getSignalInformation(SLOT_0, (err, data) => {
            if (err) {
                console.log(`Telephony_NetworkSearch_getSignalInformation_Async_0100 fail err: ${err}`);
                expect().assertFail();
                done();
                return;
            }
            console.log(
                `Telephony_NetworkSearch_getSignalInformation_Async_0100 finish data: ${JSON.stringify(data)}`
                );
            expect(data != null && data != undefined).assertTrue();
            if (data.length <= 0) {
                console.log(`Telephony_NetworkSearch_getSignalInformation_Async_0100 fail data.length: ${data.length}`);
                expect().assertFail();
                done();
                return;
            }
            expect(garrSignalType).assertContain(data[0].signalType);
            expect(garrSignalLevel).assertContain(data[0].signalLevel);
            done();
        });
    });

    /**
     * @tc.number  Telephony_NetworkSearch_getSignalInformation_Async_0400
     * @tc.name    SlotId parameter input is 77, test getSignalInformation() query function go to the error
     * @tc.desc    Function test
     */
    it('Telephony_NetworkSearch_getSignalInformation_Async_0400', 0, async function (done) {
        let slotId = 77;
        radio.getSignalInformation(slotId, (err, data) => {
            if (err) {
                console.log(`Telephony_NetworkSearch_getSignalInformation_Async_0400 fail err: ${err}`);
                expect().assertFail();
                done();
                return;
            }
            console.log(`Telephony_NetworkSearch_getSignalInformation_Async_0400 finish data: ${JSON.stringify(data)}`);
            expect(data === undefined || data.length === 0).assertTrue();
            done();
        });
    });

    /**
     * @tc.number  Telephony_NetworkSearch_getNetworkState_Promise_0100
     * @tc.name    Test the getNetworkState() query function and return the default card 0 network registration status
     * @tc.desc    Function test
     */
    it('Telephony_NetworkSearch_getNetworkState_Promise_0100', 0, async function (done) {
        try {
            let data = await radio.getNetworkState();
            console.log(`Telephony_NetworkSearch_getNetworkState_Promise_0100 finish data: ${JSON.stringify(data)}`);
            expect(data != null && data != undefined).assertTrue();
            expect(
                data.longOperatorName != undefined && data.longOperatorName != '' && data.longOperatorName != null
            ).assertTrue();
            expect(
                data.shortOperatorName != undefined && data.shortOperatorName != '' && data.shortOperatorName != null
            ).assertTrue();
            expect(data.plmnNumeric.length === PLMN_SIZE && data.plmnNumeric.substr(0, 3) === '460').assertTrue();
            expect(garrRegState).assertContain(data.regStatus);
            expect(data.nsaState === 1).assertTrue();
            expect(data.isRoaming === false).assertTrue();
            expect(data.isCaActive === false).assertTrue();
            expect(data.isEmergency === false).assertTrue();

        } catch (err) {
            console.log(`Telephony_NetworkSearch_getNetworkState_Promise_0100 fail err: ${err}`);
            expect().assertFail();
            done();
            return;
        }
        done();
    });

    /**
     * @tc.number  Telephony_NetworkSearch_getNetworkState_Promise_0200
     * @tc.name    The slotId parameter input is 0, test the getNetworkState() query function,
     *             and return the network registration status of card 0
     * @tc.desc    Function test
     */
    it('Telephony_NetworkSearch_getNetworkState_Promise_0200', 0, async function (done) {
        try {
            let data = await radio.getNetworkState(SLOT_0);
            console.log(`Telephony_NetworkSearch_getNetworkState_Promise_0200 finish data: ${JSON.stringify(data)}`);
            expect(data != null && data != undefined).assertTrue();
            expect(
                data.longOperatorName != undefined && data.longOperatorName != '' && data.longOperatorName != null
            ).assertTrue();
            expect(
                data.shortOperatorName != undefined && data.shortOperatorName != '' && data.shortOperatorName != null
            ).assertTrue();
            expect(data.plmnNumeric.length === PLMN_SIZE && data.plmnNumeric.substr(0, 3) === '460').assertTrue();
            expect(garrRegState).assertContain(data.regStatus);
            expect(data.nsaState === 1).assertTrue();
            expect(data.isRoaming === false).assertTrue();
            expect(data.isCaActive === false).assertTrue();
            expect(data.isEmergency === false).assertTrue();

        } catch (err) {
            console.log(`Telephony_NetworkSearch_getNetworkState_Promise_0200 fail err: ${err}`);
            expect().assertFail();
            done();
            return;
        }
        done();
    });

    /**
     * @tc.number  Telephony_NetworkSearch_getNetworkState_Promise_0700
     * @tc.name    SlotId parameter input is -2, test getNetworkState() query function go to the error
     * @tc.desc    Function test
     */
    it('Telephony_NetworkSearch_getNetworkState_Promise_0700', 0, async function (done) {
        try {
            let slotId = -2;
            let data = await radio.getNetworkState(slotId);
            console.log('Telephony_NetworkSearch_getNetworkState_Promise_0700 fail not go to err');
            expect().assertFail();
        } catch (err) {
            console.log(`Telephony_NetworkSearch_getNetworkState_Promise_0700 finish err: ${err}`);
            done();
            return;
        }
        done();
    });

    /**
     * @tc.number  Telephony_NetworkSearch_getRadioTech_Promise_0100
     * @tc.name    SlotId parameter input is 0, test getRadioTech() query function, return PS, CS network mode
     * @tc.desc    Function test
     */
    it('Telephony_NetworkSearch_getRadioTech_Promise_0100', 0, async function (done) {
        try {
            let data = await radio.getRadioTech(SLOT_0);
            console.log(`Telephony_NetworkSearch_getRadioTech_Promise_0100 finish data: ${JSON.stringify(data)}`);
            expect(data != null && data != undefined).assertTrue();
            expect(garrRadioTech).assertContain(data.psRadioTech);
            expect(garrRadioTech).assertContain(data.csRadioTech);
        } catch (err) {
            console.log(`Telephony_NetworkSearch_getRadioTech_Promise_0100 fail err: ${err}`);
            expect().assertFail();
            done();
            return;
        }
        done();
    });

    /**
     * @tc.number  Telephony_NetworkSearch_getRadioTech_Promise_0400
     * @tc.name    SlotId parameter input is 8, test getRadioTech() query function go to the error
     * @tc.desc    Function test
     */
    it('Telephony_NetworkSearch_getRadioTech_Promise_0400', 0, async function (done) {
        try {
            let slotId = 8;
            let data = await radio.getRadioTech(slotId);
            console.log('Telephony_NetworkSearch_getRadioTech_Promise_0400 fail not go to err');
            expect().assertFail();
        } catch (err) {
            console.log(`Telephony_NetworkSearch_getRadioTech_Promise_0400 finish err: ${err}`);
            done();
            return;
        }
        done();
    });

    /**
     * @tc.number  Telephony_NetworkSearch_getSignalInformation_Promise_0100
     * @tc.name    SlotId parameter input is 0, test getSignalInformation() query function,
     *             return signal strength list information
     * @tc.desc    Function test
     */
    it('Telephony_NetworkSearch_getSignalInformation_Promise_0100', 0, async function (done) {
        try {
            let data = await radio.getSignalInformation(SLOT_0);
            console.log(
                `Telephony_NetworkSearch_getSignalInformation_Promise_0100 finish data: ${JSON.stringify(data)}`);
            expect(data != null && data != undefined).assertTrue();
            if (data.length <= 0) {
                console.log(
                    `Telephony_NetworkSearch_getSignalInformation_Promise_0100 fail data.length: ${data.length}`);
                expect().assertFail();
                done();
                return;
            }
            expect(garrSignalType).assertContain(data[0].signalType);
            expect(garrSignalLevel).assertContain(data[0].signalLevel);
        } catch (err) {
            console.log(`Telephony_NetworkSearch_getSignalInformation_Promise_0100 fail err: ${err}`);
            expect().assertFail();
            done();
            return;
        }
        done();
    });

    /**
     * @tc.number  Telephony_NetworkSearch_getSignalInformation_Promise_0400
     * @tc.name    SlotId parameter input is 8, test getSignalInformation() query function return the datalength is 0
     * @tc.desc    Function test
     */
    it('Telephony_NetworkSearch_getSignalInformation_Promise_0400', 0, async function (done) {
        try {
            let slotId = 8;
            let data = await radio.getSignalInformation(slotId);
            console.log(
                `Telephony_NetworkSearch_getSignalInformation_Promise_0400 finish data: ${JSON.stringify(data)}`);
            expect(data === undefined || data.length === 0).assertTrue();
        } catch (err) {
            console.log(`Telephony_NetworkSearch_getSignalInformation_Promise_0400 fail err: ${err}`);
            expect().assertFail();
            done();
            return;
        }
        done();
    });

    /**
     * @tc.number  Telephony_NetworkSearch_getISOCountryCodeForNetwork_Async_0100
     * @tc.name    SlotId parameter input is 0, test getISOCountryCodeForNetwork() returns country code is 'cn'
     * @tc.desc    Function test
     */
    it('Telephony_NetworkSearch_getISOCountryCodeForNetwork_Async_0100', 0, async function (done) {
        radio.getISOCountryCodeForNetwork(SLOT_0, (err, data) => {
            if (err) {
                console.log(`Telephony_NetworkSearch_getISOCountryCodeForNetwork_Async_0100 fail err: ${err}`);
                expect().assertFail();
                done();
                return;
            }
            console.log(
            `Telephony_NetworkSearch_getISOCountryCodeForNetwork_Async_0100 finish data: ${JSON.stringify(data)}`);
            expect(data === ISO_COUNTRY_CODE).assertTrue();
            done();
        });
    });

    /**
     * @tc.number  Telephony_NetworkSearch_getISOCountryCodeForNetwork_Promise_0100
     * @tc.name    SlotId parameter input is 0, test getISOCountryCodeForNetwork() returns country code is 'cn'
     * @tc.desc    Function test
     */
    it('Telephony_NetworkSearch_getISOCountryCodeForNetwork_Promise_0100', 0, async function (done) {
        try {
            let data = await radio.getISOCountryCodeForNetwork(SLOT_0);
            expect(data === ISO_COUNTRY_CODE).assertTrue();
            console.log(
                `Telephony_NetworkSearch_getISOCountryCodeForNetwork_Promise_0100 finish data:${JSON.stringify(data)}`);
        } catch (err) {
            console.log(`Telephony_NetworkSearch_getISOCountryCodeForNetwork_Promise_0100 fail err: ${err}`);
            expect().assertFail();
            done();
            return;
        }
        done();
    });

    /**
     * @tc.number  Telephony_NetworkSearch_getISOCountryCodeForNetwork_Async_0400
     * @tc.name    SlotId parameter input is 6, test getISOCountryCodeForNetwork() query function and datalengtch = 0
     * @tc.desc    Function test
     */
    it('Telephony_NetworkSearch_getISOCountryCodeForNetwork_Async_0400', 0, async function (done) {
        let slotId = 6;
        radio.getISOCountryCodeForNetwork(slotId, (err, data) => {
            if (err) {
                console.log(`Telephony_NetworkSearch_getISOCountryCodeForNetwork_Async_0400 fail err: ${err}`);
                expect().assertFail();
                done();
                return;
            }
            console.log(
                `Telephony_NetworkSearch_getISOCountryCodeForNetwork_Async_0400 finish data: ${JSON.stringify(data)}`);
            expect(data === undefined || data.length === 0).assertTrue();
            done();
        });
    });

    /**
     * @tc.number  Telephony_NetworkSearch_getISOCountryCodeForNetwork_Promise_0400
     * @tc.name    SlotId parameter input is 55, test getISOCountryCodeForNetwork() query function go to the error
     * @tc.desc    Function test
     */
    it('Telephony_NetworkSearch_getISOCountryCodeForNetwork_Promise_0400', 0, async function (done) {
        try {
            let slotId = 55;
            let data = await radio.getISOCountryCodeForNetwork(slotId);
            console.log(
                `Telephony_NetworkSearch_getISOCountryCodeForNetwork_Promise_0400 finish data:${JSON.stringify(data)}`);
            expect(data === undefined || data.length === 0).assertTrue();
        } catch (err) {
            console.log(`Telephony_NetworkSearch_getISOCountryCodeForNetwork_Promise_0400 fail err: ${err}`);
            expect().assertFail();
            done();
            return;
        }
        done();
    });
});
