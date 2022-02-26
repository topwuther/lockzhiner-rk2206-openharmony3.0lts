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
    let gslot = 0;
    let gradioTech = '2';
    let goperatorName = '46000';
    let gnetworkSMode = {
        slotId: gslot,
        selectMode: radio.NETWORK_SELECTION_AUTOMATIC,
        networkInformation: {
            operatorName: '46000',
            operatorNumeric: '46011',
            state: radio.NETWORK_AVAILABLE,
            radioTech: gradioTech,
        },
        resumeSelection: false,
    };
    //Network status
    let garrNetworkState = [
        radio.NETWORK_UNKNOWN,
        radio.NETWORK_AVAILABLE,
        radio.NETWORK_CURRENT,
        radio.NETWORK_FORBIDDEN,
    ];
    //Network system
    let garrNetworkRadioTech = [
            'UNKNOWN','GSM','1XRTT','WCDMA',
            'HSPA','HSPAP', 'TD_SCDMA','EVDO',
            'EHRPD','LTE','LTE_CA','IWLAN','NR'];

    const SLOT_0 = 0;
    const OPERATOR_NUMERIC_SSSSS = 'SSSSS';
    const ERR_VALUE_5 = 5;



    async function recoverNetworkSelectionMode() {
        try {
            await radio.setNetworkSelectionMode(gnetworkSMode);
            console.log('Telephony_NetworkSearch_recoverNetworkSelectionMode success');
        } catch (err) {
            console.log('Telephony_NetworkSearch_recoverNetworkSelectionMode fail');
        }
    }

    afterAll(async function () {
        //Initialize network selection mode after all test cases
        recoverNetworkSelectionMode();
        console.log('Telephony_NetworkSearch_SelectionModeTest End!!!');
    });

    /**
     * @tc.number  Telephony_NetworkSearch_getNetworkSelectionMode_Async_0100
     * @tc.name    Test the getNetworkSelectionMode() query function and set the slotId parameter input to 0,
     *             test the return value as NETWORK_SELECTION_AUTOMATIC
     * @tc.desc    Function test
     */
    it('Telephony_NetworkSearch_getNetworkSelectionMode_Async_0100', 0, async function (done) {
        try {
            await radio.turnOnRadio();
            console.log('Telephony_NetworkSearch_recoverRadioState success');
        } catch (err) {
            console.log(`Telephony_NetworkSearch_recoverRadioState fail ${err}`);
            expect().assertFail();
            done();
            return;
        }
        try{
            let data = await radio.getNetworkState(SLOT_0);
            console.log(`Telephony_NetworkSearch_getNetworkState data: ${JSON.stringify(data)}`)
            if(data.plmnNumeric != '' && data.plmnNumeric != undefined){
                gnetworkSMode.networkInformation.operatorNumeric = data.plmnNumeric;
            }else{
                console.log(`Telephony_NetworkSearch_getNetworkState fail`)
                expect().assertFail();
                done();
                return;
            }
        }catch(err){
            console.log(`Telephony_NetworkSearch_getNetworkSelectionMode_Async_0100 init err: ${err}`);
            expect().assertFail();
            done();
            return;
        }
        let networkSMode = {
            slotId: SLOT_0,
            selectMode: radio.NETWORK_SELECTION_AUTOMATIC,
            networkInformation: {
                operatorName: goperatorName,
                operatorNumeric: gnetworkSMode.networkInformation.operatorNumeric,
                state: radio.NETWORK_AVAILABLE,
                radioTech: gradioTech,
            },
            resumeSelection: false,
        };
        console.log(
            `Telephony_NetworkSearch_getNetworkSelectionMode_Async_0100 networkSMode:${JSON.stringify(networkSMode)}`)
        radio.setNetworkSelectionMode(networkSMode, (err) => {
            if (err) {
                console.log(`Telephony_NetworkSearch_getNetworkSelectionMode_Async_0100 set err: ${err}`);
                expect().assertFail();
                done();
                return;
            }
            console.log('Telephony_NetworkSearch_getNetworkSelectionMode_Async_0100 set finish');
            radio.getNetworkSelectionMode(SLOT_0, (err, data) => {
                if (err) {
                    console.log(`Telephony_NetworkSearch_getNetworkSelectionMode_Async_0100 get err: ${err}`);
                    expect().assertFail();
                    done();
                    return;
                }
                console.log(`Telephony_NetworkSearch_getNetworkSelectionMode_Async_0100 get finish data: ${data}`);
                expect(data === radio.NETWORK_SELECTION_AUTOMATIC).assertTrue();
                done();
            });
        });
    });

    /**
     * @tc.number  Telephony_NetworkSearch_getNetworkSelectionMode_Async_0200
     * @tc.name    Test the getNetworkSelectionMode() query function and set the slotId parameter input to 0,
     *             and test the return value as NETWORK_SELECTION_MANUAL
     * @tc.desc    Function test
     */
    it('Telephony_NetworkSearch_getNetworkSelectionMode_Async_0200', 0, async function (done) {
        try{
            let data = await radio.getNetworkState(SLOT_0);
            console.log(`Telephony_NetworkSearch_getNetworkState data: ${JSON.stringify(data)}`)
            if(data.plmnNumeric != '' && data.plmnNumeric != undefined){
                gnetworkSMode.networkInformation.operatorNumeric = data.plmnNumeric;
            }else{
                console.log(`Telephony_NetworkSearch_getNetworkState fail`)
                expect().assertFail();
                done();
                return;
            }
        }catch(err){
            expect().assertFail();
            done();
            return;
        }
        let networkSMode = {
            slotId: SLOT_0,
            selectMode: radio.NETWORK_SELECTION_MANUAL,
            networkInformation: {
                operatorName: goperatorName,
                operatorNumeric: gnetworkSMode.networkInformation.operatorNumeric,
                state: radio.NETWORK_AVAILABLE,
                radioTech: gradioTech,
            },
            resumeSelection: false,
        };
        console.log(
            `Telephony_NetworkSearch_getNetworkSelectionMode_Async_0200 networkSMode:${JSON.stringify(networkSMode)}`)
        radio.setNetworkSelectionMode(networkSMode, (err) => {
            if (err) {
                console.log(`Telephony_NetworkSearch_getNetworkSelectionMode_Async_0200 set err: ${err}`);
                expect().assertFail();
                done();
                return;
            }
            console.log('Telephony_NetworkSearch_getNetworkSelectionMode_Async_0200 set finish');
            radio.getNetworkSelectionMode(SLOT_0, (err, data) => {
                if (err) {
                    console.log(`Telephony_NetworkSearch_getNetworkSelectionMode_Async_0200 get err: ${err}`);
                    expect().assertFail();
                    done();
                    return;
                }
                console.log(`Telephony_NetworkSearch_getNetworkSelectionMode_Async_0200 get finish data: ${data}`);
                expect(data === radio.NETWORK_SELECTION_MANUAL).assertTrue();
                done();
            });
        });
    });

    /**
     * @tc.number  Telephony_NetworkSearch_getNetworkSelectionMode_Async_0500
     * @tc.name    SlotId parameter input is 3, test getNetworkSelectionMode() query function go to the error
     * @tc.desc    Function test
     */
    it('Telephony_NetworkSearch_getNetworkSelectionMode_Async_0500', 0, async function (done) {
        let slotId = 3;
        radio.getNetworkSelectionMode(slotId, (err, res) => {
            if (err) {
                console.log(`Telephony_NetworkSearch_getNetworkSelectionMode_Async_0500 err: ${err}`);
                done();
            } else {
                console.log('Telephony_NetworkSearch_getNetworkSelectionMode_Async_0500 fail not go to err');
                expect().assertFail();
                done();
            }

        });
    });

    /**
     * @tc.number  Telephony_NetworkSearch_getNetworkSelectionMode_Promise_0100
     * @tc.name    Test the getNetworkSelectionMode() query function and set the slotId parameter input to 0,
     *             and test the return value as NETWORK_SELECTION_AUTOMATIC
     * @tc.desc    Function test
     */
    it('Telephony_NetworkSearch_getNetworkSelectionMode_Promise_0100', 0, async function (done) {
        try{
            let data = await radio.getNetworkState(SLOT_0);
            console.log(`Telephony_NetworkSearch_getNetworkState data: ${JSON.stringify(data)}`)
            if(data.plmnNumeric != '' && data.plmnNumeric != undefined){
                gnetworkSMode.networkInformation.operatorNumeric = data.plmnNumeric;
            }else{
                console.log(`Telephony_NetworkSearch_getNetworkState fail`)
                expect().assertFail();
                done();
                return;
            }
        }catch(err){
            expect().assertFail();
            done();
            return;
        }
        let networkSMode = {
            slotId: SLOT_0,
            selectMode: radio.NETWORK_SELECTION_AUTOMATIC,
            networkInformation: {
                operatorName: goperatorName,
                operatorNumeric: gnetworkSMode.networkInformation.operatorNumeric,
                state: radio.NETWORK_AVAILABLE,
                radioTech: gradioTech,
            },
            resumeSelection: false,
        };
        console.log(
            `Telephony_NetworkSearch_getNetworkSelectionMode_Promise_0100 networkSMode:${JSON.stringify(networkSMode)}`)
        try {
            await radio.setNetworkSelectionMode(networkSMode);
            console.log('Telephony_NetworkSearch_getNetworkSelectionMode_Promise_0100 set finish');
        } catch (err) {
            console.log(`Telephony_NetworkSearch_getNetworkSelectionMode_Promise_0100 set err: ${err}`);
            expect().assertFail();
            done();
            return;
        }
        try {
            let data = await radio.getNetworkSelectionMode(SLOT_0);
            console.log(`Telephony_NetworkSearch_getNetworkSelectionMode_Promise_0100 get finish data: ${data}`);
            expect(data === radio.NETWORK_SELECTION_AUTOMATIC).assertTrue();
        } catch (err) {
            console.log(`Telephony_NetworkSearch_getNetworkSelectionMode_Promise_0100 get err: ${err}`);
            expect().assertFail();
            done();
            return;
        }
        done();
    });

    /**
     * @tc.number  Telephony_NetworkSearch_getNetworkSelectionMode_Promise_0200
     * @tc.name    Test the getNetworkSelectionMode() query function and set the slotId parameter input to 0,
     *             and test the return value as '1'
     * @tc.desc    Function test
     */
    it('Telephony_NetworkSearch_getNetworkSelectionMode_Promise_0200', 0, async function (done) {
        try{
            let data = await radio.getNetworkState(SLOT_0);
            console.log(`Telephony_NetworkSearch_getNetworkState data: ${JSON.stringify(data)}`)
            if(data.plmnNumeric != '' && data.plmnNumeric != undefined){
                gnetworkSMode.networkInformation.operatorNumeric = data.plmnNumeric;
            }else{
                console.log(`Telephony_NetworkSearch_getNetworkState fail`)
                expect().assertFail();
                done();
                return;
            }
        }catch(err){
            expect().assertFail();
            done();
            return;
        }
        let networkSMode = {
            slotId: SLOT_0,
            selectMode: radio.NETWORK_SELECTION_MANUAL,
            networkInformation: {
                operatorName: goperatorName,
                operatorNumeric: gnetworkSMode.networkInformation.operatorNumeric,
                state: radio.NETWORK_AVAILABLE,
                radioTech: gradioTech,
            },
            resumeSelection: false,
        };
        console.log(
            `Telephony_NetworkSearch_getNetworkSelectionMode_Promise_0200 networkSMode:${JSON.stringify(networkSMode)}`)
        try {
            await radio.setNetworkSelectionMode(networkSMode);
            console.log('Telephony_NetworkSearch_getNetworkSelectionMode_Promise_0200 set finish');
        } catch (err) {
            console.log(`Telephony_NetworkSearch_getNetworkSelectionMode_Promise_0200 set err: ${err}`);
            expect().assertFail();
            done();
            return;
        }
        try {
            let data = await radio.getNetworkSelectionMode(SLOT_0);
            console.log(`Telephony_NetworkSearch_getNetworkSelectionMode_Promise_0200 get finish data: ${data}`);
            expect(data === radio.NETWORK_SELECTION_MANUAL).assertTrue();
        } catch (err) {
            console.log(`Telephony_NetworkSearch_getNetworkSelectionMode_Promise_0200 get err: ${err}`);
            expect().assertFail();
            done();
            return;
        }
        done();
    });

    /**
     * @tc.number  Telephony_NetworkSearch_getNetworkSelectionMode_Promise_0500
     * @tc.name    SlotId parameter input is 33, test getNetworkSelectionMode() query function go to the error
     * @tc.desc    Function test
     */
    it('Telephony_NetworkSearch_getNetworkSelectionMode_Promise_0500', 0, async function (done) {
        try {
            let slotId = 33;
            let res = await radio.getNetworkSelectionMode(slotId);
            console.log('Telephony_NetworkSearch_getNetworkSelectionMode_Promise_0500 fail not go to err');
            expect().assertFail();
        } catch (err) {
            console.log('Telephony_NetworkSearch_getNetworkSelectionMode_Promise_0500 finish');
            done();
            return;
        }
        done();
    });

    /**
     * @tc.number  Telephony_NetworkSearch_setNetworkSelectionMode_Async_0100
     * @tc.name    Test the setNetworkSelectionMode() query function and set the selectmode parameter input to
     *             'NETWORK_SELECTION_UNKNOWN' and test the return value is 'False' and the network selection mode
     *             update failed
     * @tc.desc    Function test
     */
    it('Telephony_NetworkSearch_setNetworkSelectionMode_Async_0100', 0, async function (done) {
        try{
            let data = await radio.getNetworkState(SLOT_0);
            console.log(`Telephony_NetworkSearch_getNetworkState data: ${JSON.stringify(data)}`)
            if(data.plmnNumeric != '' && data.plmnNumeric != undefined){
                gnetworkSMode.networkInformation.operatorNumeric = data.plmnNumeric;
            }else{
                console.log(`Telephony_NetworkSearch_getNetworkState fail`)
                expect().assertFail();
                done();
                return;
            }

        }catch(err){
            expect().assertFail();
            done();
            return;
        }
        recoverNetworkSelectionMode();
        let networkSMode = {
            slotId: SLOT_0,
            selectMode: radio.NETWORK_SELECTION_UNKNOWN,
            networkInformation: {
                operatorName: goperatorName,
                operatorNumeric: gnetworkSMode.networkInformation.operatorNumeric,
                state: radio.NETWORK_AVAILABLE,
                radioTech: gradioTech,
            },
            resumeSelection: false,
        };
        console.log(
            `Telephony_NetworkSearch_setNetworkSelectionMode_Async_0100 networkSMode:${JSON.stringify(networkSMode)}`)
        radio.setNetworkSelectionMode(networkSMode, (err) => {
            if (err) {
                radio.getNetworkSelectionMode(SLOT_0, (err, data) => {
                    if (err) {
                        console.log(`Telephony_NetworkSearch_setNetworkSelectionMode_Async_0100 get err: ${err}`);
                        expect().assertFail();
                        done();
                        return;
                    }
                    console.log(`Telephony_NetworkSearch_setNetworkSelectionMode_Async_0100 get finish ${data}`);
                    expect(data === radio.NETWORK_SELECTION_AUTOMATIC).assertTrue();
                    done();
                });

            } else {
                console.log(`Telephony_NetworkSearch_setNetworkSelectionMode_Async_0100 set not go to err`);
                expect().assertFail();
                done();
            }
        });
    });

    /**
     * @tc.number  Telephony_NetworkSearch_setNetworkSelectionMode_Async_0200
     * @tc.name    Test the setNetworkSelectionMode() query function and set the selectmode parameter input to '5'
     *             and test the return value is 'False' and the network selection mode update failed
     * @tc.desc    Function test
     */
    it('Telephony_NetworkSearch_setNetworkSelectionMode_Async_0200', 0, async function (done) {
        try{
            let data = await radio.getNetworkState(SLOT_0);
            console.log(`Telephony_NetworkSearch_getNetworkState data: ${JSON.stringify(data)}`)
            if(data.plmnNumeric != '' && data.plmnNumeric != undefined){
                gnetworkSMode.networkInformation.operatorNumeric = data.plmnNumeric;
            }else{
                console.log(`Telephony_NetworkSearch_getNetworkState fail`)
                expect().assertFail();
                done();
                return;
            }
        }catch(err){
            expect().assertFail();
            done();
            return;
        }
        recoverNetworkSelectionMode();
        let networkSMode = {
            slotId: SLOT_0,
            selectMode: ERR_VALUE_5,
            networkInformation: {
                operatorName: goperatorName,
                operatorNumeric: gnetworkSMode.networkInformation.operatorNumeric,
                state: radio.NETWORK_AVAILABLE,
                radioTech: gradioTech,
            },
            resumeSelection: false,
        };
        console.log(
            `Telephony_NetworkSearch_setNetworkSelectionMode_Async_0200 networkSMode:${JSON.stringify(networkSMode)}`)
        radio.setNetworkSelectionMode(networkSMode, (err) => {
            if (err) {
                console.log('Telephony_NetworkSearch_setNetworkSelectionMode_Async_0200 set finish');
                radio.getNetworkSelectionMode(SLOT_0, (err, data) => {
                    if (err) {
                        console.log(`Telephony_NetworkSearch_setNetworkSelectionMode_Async_0200 get err: ${err}`);
                        expect().assertFail();
                        done();
                        return;
                    }
                    console.log(`Telephony_NetworkSearch_setNetworkSelectionMode_Async_0200 get finish data: ${data}`);
                    expect(data === radio.NETWORK_SELECTION_AUTOMATIC).assertTrue();
                    done();
                });
            } else {
                console.log('Telephony_NetworkSearch_setNetworkSelectionMode_Async_0200 set not go to err');
                expect().assertFail();
                done();
            }

        });
    });

    /**
     * @tc.number  Telephony_NetworkSearch_setNetworkSelectionMode_Async_0300
     * @tc.name    Test the setNetworkSelectionMode() and set the operatorNumeric parameter input to '46001'
     *             and test value is 'False' and the network selection mode update fails
     * @tc.desc    Function test
     */
    it('Telephony_NetworkSearch_setNetworkSelectionMode_Async_0300', 0, async function (done) {
        try{
            let data = await radio.getNetworkState(SLOT_0);
            console.log(`Telephony_NetworkSearch_getNetworkState data: ${JSON.stringify(data)}`)
            if(data.plmnNumeric != '' && data.plmnNumeric != undefined){
                gnetworkSMode.networkInformation.operatorNumeric = data.plmnNumeric;
            }else{
                console.log(`Telephony_NetworkSearch_getNetworkState fail`)
                expect().assertFail();
                done();
                return;
            }
        }catch(err){
            expect().assertFail();
            done();
            return;
        }
        recoverNetworkSelectionMode();
        let networkSMode = {
            slotId: SLOT_0,
            selectMode: radio.NETWORK_SELECTION_MANUAL,
            networkInformation: {
                operatorName: goperatorName,
                operatorNumeric: OPERATOR_NUMERIC_SSSSS,
                state: radio.NETWORK_AVAILABLE,
                radioTech: gradioTech,
            },
            resumeSelection: false,
        };
        console.log(
            `Telephony_NetworkSearch_setNetworkSelectionMode_Async_0300 networkSMode:${JSON.stringify(networkSMode)}`)
        radio.setNetworkSelectionMode(networkSMode, (err) => {
            if (err) {
                console.log('Telephony_NetworkSearch_setNetworkSelectionMode_Async_0300 set finish');
                radio.getNetworkSelectionMode(SLOT_0, (err, data) => {
                    if (err) {
                        console.log(`Telephony_NetworkSearch_setNetworkSelectionMode_Async_0300 get err: ${err}`);
                        expect().assertFail();
                        done();
                        return;
                    }
                    console.log(`Telephony_NetworkSearch_setNetworkSelectionMode_Async_0300 get finish data: ${data}`);
                    expect(data === radio.NETWORK_SELECTION_AUTOMATIC).assertTrue();
                    done();
                });
            } else {
                console.log('Telephony_NetworkSearch_setNetworkSelectionMode_Async_0300 set not go to err');
                expect().assertFail();
                done();
            }

        });
    });

    /**
     * @tc.number  Telephony_NetworkSearch_setNetworkSelectionMode_Async_0400
     * @tc.name    Test the setNetworkSelectionMode() and set the resumeSelection parameter input to 'true'
     *             and test value is 'False' and the network selection mode update fails
     * @tc.desc    Function test
     */
    it('Telephony_NetworkSearch_setNetworkSelectionMode_Async_0400', 0, async function (done) {
        try{
            let data = await radio.getNetworkState(SLOT_0);
            console.log(`Telephony_NetworkSearch_getNetworkState data: ${JSON.stringify(data)}`)
            if(data.plmnNumeric != '' && data.plmnNumeric != undefined){
                gnetworkSMode.networkInformation.operatorNumeric = data.plmnNumeric;
            }else{
                console.log(`Telephony_NetworkSearch_getNetworkState fail`)
                expect().assertFail();
                done();
                return;
            }
        }catch(err){
            expect().assertFail();
            done();
            return;
        }
        recoverNetworkSelectionMode();
        let networkSMode = {
            slotId: SLOT_0,
            selectMode: radio.NETWORK_SELECTION_MANUAL,
            networkInformation: {
                operatorName: goperatorName,
                operatorNumeric: gnetworkSMode.networkInformation.operatorNumeric,
                state: radio.NETWORK_AVAILABLE,
                radioTech: gradioTech,
            },
            resumeSelection: true,
        };
        console.log(
            `Telephony_NetworkSearch_setNetworkSelectionMode_Async_0400 networkSMode:${JSON.stringify(networkSMode)}`)
        radio.setNetworkSelectionMode(networkSMode, (err) => {
            if (err) {
                console.log(`Telephony_NetworkSearch_setNetworkSelectionMode_Async_0400 set fail err: ${err}`);
                expect().assertFail();
                done();
                return;
            }
            console.log('Telephony_NetworkSearch_setNetworkSelectionMode_Async_0400 set finish');
            radio.getNetworkSelectionMode(SLOT_0, (err, data) => {
                if (err) {
                    console.log(`Telephony_NetworkSearch_setNetworkSelectionMode_Async_0400 get fail err: ${err}`);
                    expect().assertFail();
                    done();
                    return;
                }
                console.log(`Telephony_NetworkSearch_setNetworkSelectionMode_Async_0400 get finish data: ${data}`);
                expect(data === radio.NETWORK_SELECTION_MANUAL).assertTrue();
                done();
            });
        });
    });

    /**
     * @tc.number  Telephony_NetworkSearch_setNetworkSelectionMode_Async_0700
     * @tc.name    SlotId parameter input is 55, test setNetworkSelectionMode() query function go to the error
     * @tc.desc    Function test
     */
    it('Telephony_NetworkSearch_setNetworkSelectionMode_Async_0700', 0, async function (done) {
        try{
            let data = await radio.getNetworkState(SLOT_0);
            console.log(`Telephony_NetworkSearch_getNetworkState data: ${JSON.stringify(data)}`)
            if(data.plmnNumeric != '' && data.plmnNumeric != undefined){
                gnetworkSMode.networkInformation.operatorNumeric = data.plmnNumeric;
            }else{
                console.log(`Telephony_NetworkSearch_getNetworkState fail`)
                expect().assertFail();
                done();
                return;
            }
        }catch(err){
            expect().assertFail();
            done();
            return;
        }
        recoverNetworkSelectionMode();
        let networkSMode = {
            slotId: 55, //set the error slot id is 55
            selectMode: radio.NETWORK_SELECTION_AUTOMATIC,
            networkInformation: {
                operatorName: goperatorName,
                operatorNumeric: gnetworkSMode.networkInformation.operatorNumeric,
                state: radio.NETWORK_AVAILABLE,
                radioTech: gradioTech,
            },
            resumeSelection: false,
        };
        console.log(
            `Telephony_NetworkSearch_setNetworkSelectionMode_Async_0700 networkSMode:${JSON.stringify(networkSMode)}`)
        radio.setNetworkSelectionMode(networkSMode, (err) => {
            if (err) {
                console.log('Telephony_NetworkSearch_setNetworkSelectionMode_Async_0700 set finish');
                done();
            } else {
                console.log('Telephony_NetworkSearch_setNetworkSelectionMode_Async_0700 set not go to err');
                expect().assertFail();
                done();
            }

        });
    });

    /**
     * @tc.number  Telephony_NetworkSearch_setNetworkSelectionMode_Promise_0100
     * @tc.name    Test the setNetworkSelectionMode() and set the selectmode parameter input to
     *             'NETWORK_SELECTION_UNKNOWN' and test value is 'False' and the network selection mode update fails
     * @tc.desc    Function test
     */
    it('Telephony_NetworkSearch_setNetworkSelectionMode_Promise_0100', 0, async function (done) {
        try{
            let data = await radio.getNetworkState(SLOT_0);
            console.log(`Telephony_NetworkSearch_getNetworkState data: ${JSON.stringify(data)}`)
            if(data.plmnNumeric != '' && data.plmnNumeric != undefined){
                gnetworkSMode.networkInformation.operatorNumeric = data.plmnNumeric;
            }else{
                console.log(`Telephony_NetworkSearch_getNetworkState fail`)
                expect().assertFail();
                done();
                return;
            }
        }catch(err){
            expect().assertFail();
            done();
            return;
        }
       try {
           await radio.setNetworkSelectionMode(gnetworkSMode);
           console.log('Telephony_NetworkSearch_recoverNetworkSelectionMode success');
       } catch (err) {
           console.log('Telephony_NetworkSearch_recoverNetworkSelectionMode fail');
           expect().assertFail();
           done();
           return;
       }
        let networkSMode = {
            slotId: SLOT_0,
            selectMode: radio.NETWORK_SELECTION_UNKNOWN,
            networkInformation: {
                operatorName: goperatorName,
                operatorNumeric: gnetworkSMode.networkInformation.operatorNumeric,
                state: radio.NETWORK_AVAILABLE,
                radioTech: gradioTech,
            },
            resumeSelection: false,
        };
        console.log(
            `Telephony_NetworkSearch_setNetworkSelectionMode_Promise_0100 networkSMode:${JSON.stringify(networkSMode)}`)
        try {
            console.log(`Telephony_NetworkSearch_setNetworkSelectionMode_Promise_0100 ${JSON.stringify(networkSMode)}`)
            await radio.setNetworkSelectionMode(networkSMode);
            console.log(`Telephony_NetworkSearch_setNetworkSelectionMode_Promise_0100 set fail`);
            expect().assertFail();
            done();
            return;
        } catch (err) {
            console.log('Telephony_NetworkSearch_setNetworkSelectionMode_Promise_0100 set finish');
            try {
                let data = await radio.getNetworkSelectionMode(SLOT_0);
                console.log(
                    `Telephony_NetworkSearch_setNetworkSelectionMode_Promise_0100 get finish: ${JSON.stringify(data)}`);
                expect(data === radio.NETWORK_SELECTION_AUTOMATIC).assertTrue();
            } catch (err) {
                console.log(`Telephony_NetworkSearch_setNetworkSelectionMode_Promise_0100 get err: ${err}`);
                expect().assertFail();
                done();
                return;
            }
        }

        done();
    });

    /**
     * @tc.number  Telephony_NetworkSearch_setNetworkSelectionMode_Promise_0200
     * @tc.name    Test the setNetworkSelectionMode() and enter an outlier selectMode: 5 return value is 'False' and
     *             the network selection mode update fails
     * @tc.desc    Function test
     */
    it('Telephony_NetworkSearch_setNetworkSelectionMode_Promise_0200', 0, async function (done) {
        try{
            let data = await radio.getNetworkState(SLOT_0);
            console.log(`Telephony_NetworkSearch_getNetworkState data: ${JSON.stringify(data)}`)
            if(data.plmnNumeric != '' && data.plmnNumeric != undefined){
                gnetworkSMode.networkInformation.operatorNumeric = data.plmnNumeric;
            }else{
                console.log(`Telephony_NetworkSearch_getNetworkState fail`)
                expect().assertFail();
                done();
                return;
            }
        }catch(err){
            expect().assertFail();
            done();
            return;
        }
        try {
            await radio.setNetworkSelectionMode(gnetworkSMode);
            console.log('Telephony_NetworkSearch_recoverNetworkSelectionMode success');
        } catch (err) {
            console.log('Telephony_NetworkSearch_recoverNetworkSelectionMode fail');
            expect().assertFail();
            done();
            return;
        }
        let networkSMode = {
            slotId: SLOT_0,
            selectMode: ERR_VALUE_5,
            networkInformation: {
                operatorName: goperatorName,
                operatorNumeric: gnetworkSMode.networkInformation.operatorNumeric,
                state: radio.NETWORK_AVAILABLE,
                radioTech: gradioTech,
            },
            resumeSelection: false,
        };
        console.log(
            `Telephony_NetworkSearch_setNetworkSelectionMode_Promise_0200 networkSMode:${JSON.stringify(networkSMode)}`)
        try {
            await radio.setNetworkSelectionMode(networkSMode);
            console.log('Telephony_NetworkSearch_setNetworkSelectionMode_Promise_0200 set fail');
            expect().assertFail();
            done();
            return;
        } catch (err) {
            console.log('Telephony_NetworkSearch_setNetworkSelectionMode_Promise_0200 set finish');
            done();
        }
        try {
            let data = await radio.getNetworkSelectionMode(SLOT_0);
            console.log(`Telephony_NetworkSearch_setNetworkSelectionMode_Promise_0200 get finish data: ${data}`);
            expect(data === radio.NETWORK_SELECTION_AUTOMATIC).assertTrue();
        } catch (err) {
            console.log(`Telephony_NetworkSearch_setNetworkSelectionMode_Promise_0200 get err: ${err}`);
            expect().assertFail();
            done();
            return;
        }
        done();
    });

    /**
     * @tc.number  Telephony_NetworkSearch_setNetworkSelectionMode_Promise_0300
     * @tc.name    Test the setNetworkSelectionMode() and enter an outlier operatorNumeric: '46001'
     *             return value is 'False' and the network selection mode update fails
     * @tc.desc    Function test
     */
    it('Telephony_NetworkSearch_setNetworkSelectionMode_Promise_0300', 0, async function (done) {
        try{
            let data = await radio.getNetworkState(SLOT_0);
            console.log(`Telephony_NetworkSearch_getNetworkState data: ${JSON.stringify(data)}`)
            if(data.plmnNumeric != '' && data.plmnNumeric != undefined){
                gnetworkSMode.networkInformation.operatorNumeric = data.plmnNumeric;
            }else{
                console.log(`Telephony_NetworkSearch_getNetworkState fail`)
                expect().assertFail();
                done();
                return;
            }
        }catch(err){
            expect().assertFail();
            done();
            return;
        }
        try {
            await radio.setNetworkSelectionMode(gnetworkSMode);
            console.log('Telephony_NetworkSearch_recoverNetworkSelectionMode success');
        } catch (err) {
            console.log('Telephony_NetworkSearch_recoverNetworkSelectionMode fail');
            expect().assertFail();
            done();
            return;
        }
        let networkSMode = {
            slotId: SLOT_0,
            selectMode: radio.NETWORK_SELECTION_MANUAL,
            networkInformation: {
                operatorName: goperatorName,
                operatorNumeric: OPERATOR_NUMERIC_SSSSS,
                state: radio.NETWORK_AVAILABLE,
                radioTech: gradioTech,
            },
            resumeSelection: false,
        };
        console.log(
            `Telephony_NetworkSearch_setNetworkSelectionMode_Promise_0300 networkSMode:${JSON.stringify(networkSMode)}`)
        try {
            await radio.setNetworkSelectionMode(networkSMode);
            console.log('Telephony_NetworkSearch_setNetworkSelectionMode_Promise_0300 set fail');
            expect().assertFail();
            done();
            return;
        } catch (err) {
            console.log('Telephony_NetworkSearch_setNetworkSelectionMode_Promise_0300 set finish');
            done();
        }
        try {
            let data = await radio.getNetworkSelectionMode(SLOT_0);
            console.log(`Telephony_NetworkSearch_setNetworkSelectionMode_Promise_0300 get finish data: ${data}`);
            expect(data === radio.NETWORK_SELECTION_AUTOMATIC).assertTrue();
        } catch (err) {
            console.log(`Telephony_NetworkSearch_setNetworkSelectionMode_Promise_0300 get err: ${err}`);
            expect().assertFail();
            done();
            return;
        }
        done();
    });

    /**
     * @tc.number  Telephony_NetworkSearch_setNetworkSelectionMode_Promise_0400
     * @tc.name    Test the setNetworkSelectionMode() query function and set the resumeSelection parameter input to
     *             'true' and test the return value is 'True' and the network selection mode is updated to automatic
     *             network search
     * @tc.desc    Function test
     */
    it('Telephony_NetworkSearch_setNetworkSelectionMode_Promise_0400', 0, async function (done) {
        try{
            let data = await radio.getNetworkState(SLOT_0);
            console.log(`Telephony_NetworkSearch_getNetworkState data: ${JSON.stringify(data)}`)
            if(data.plmnNumeric != '' && data.plmnNumeric != undefined){
                gnetworkSMode.networkInformation.operatorNumeric = data.plmnNumeric;
            }else{
                console.log(`Telephony_NetworkSearch_getNetworkState fail`)
                expect().assertFail();
                done();
                return;
            }
        }catch(err){
            expect().assertFail();
            done();
            return;
        }
        try {
            await radio.setNetworkSelectionMode(gnetworkSMode);
            console.log('Telephony_NetworkSearch_recoverNetworkSelectionMode success');
        } catch (err) {
            console.log('Telephony_NetworkSearch_recoverNetworkSelectionMode fail');
            expect().assertFail();
            done();
            return;
        }
        let networkSMode = {
            slotId: SLOT_0,
            selectMode: radio.NETWORK_SELECTION_MANUAL,
            networkInformation: {
                operatorName: goperatorName,
                operatorNumeric: gnetworkSMode.networkInformation.operatorNumeric,
                state: radio.NETWORK_AVAILABLE,
                radioTech: gradioTech,
            },
            resumeSelection: true,
        };
        console.log(
            `Telephony_NetworkSearch_setNetworkSelectionMode_Promise_0400 networkSMode:${JSON.stringify(networkSMode)}`)
        try {
            await radio.setNetworkSelectionMode(networkSMode);
            console.log('Telephony_NetworkSearch_setNetworkSelectionMode_Promise_0400 set finish');
        } catch (err) {
            console.log(`Telephony_NetworkSearch_setNetworkSelectionMode_Promise_0400 set fail err: ${err}`);
            expect().assertFail();
            done();
        }
        try {
            let data = await radio.getNetworkSelectionMode(SLOT_0);
            console.log(`Telephony_NetworkSearch_setNetworkSelectionMode_Promise_0400 get finish: data: ${data}`);
            expect(data === radio.NETWORK_SELECTION_MANUAL).assertTrue();
        } catch (err) {
            console.log(`Telephony_NetworkSearch_setNetworkSelectionMode_Promise_0400 get err: ${err}`);
            expect().assertFail();
            done();
            return;
        }
        done();
    });

    /**
     * @tc.number  Telephony_NetworkSearch_setNetworkSelectionMode_Promise_0700
     * @tc.name    SlotId parameter input is 6, test setNetworkSelectionMode() query function go to the error
     * @tc.desc    Function test
     */
    it('Telephony_NetworkSearch_setNetworkSelectionMode_Promise_0700', 0, async function (done) {
        try{
            let data = await radio.getNetworkState(SLOT_0);
            console.log(`Telephony_NetworkSearch_getNetworkState data: ${JSON.stringify(data)}`)
            if(data.plmnNumeric != '' && data.plmnNumeric != undefined){
                gnetworkSMode.networkInformation.operatorNumeric = data.plmnNumeric;
            }else{
                console.log(`Telephony_NetworkSearch_getNetworkState fail`)
                expect().assertFail();
                done();
                return;
            }
        }catch(err){
            expect().assertFail();
            done();
            return;
        }
        try {
            await radio.setNetworkSelectionMode(gnetworkSMode);
            console.log('Telephony_NetworkSearch_recoverNetworkSelectionMode success');
        } catch (err) {
            console.log('Telephony_NetworkSearch_recoverNetworkSelectionMode fail');
            expect().assertFail();
            done();
            return;
        }
        let networkSMode = {
            slotId: 6, //set the error slot id is 6
            selectMode: radio.NETWORK_SELECTION_AUTOMATIC,
            networkInformation: {
                operatorName: goperatorName,
                operatorNumeric: gnetworkSMode.networkInformation.operatorNumeric,
                state: radio.NETWORK_AVAILABLE,
                radioTech: gradioTech,
            },
            resumeSelection: false,
        };
        console.log(
            `Telephony_NetworkSearch_setNetworkSelectionMode_Promise_0700 networkSMode:${JSON.stringify(networkSMode)}`)
        try {
            await radio.setNetworkSelectionMode(networkSMode);
            console.log('Telephony_NetworkSearch_setNetworkSelectionMode_Promise_0700 fail');
            expect().assertFail();
            done();
            return;
        } catch (err) {
            console.log('Telephony_NetworkSearch_setNetworkSelectionMode_Promise_0700 finish');
            done();
        }
    });

    /**
     * @tc.number  Telephony_NetworkSearch_getNetworkSearchInformation_Async_0100
     * @tc.name    Test the getNetworkSearchInformation() query function and set the slotId parameter input to 0,
     *             test the return value isNetworkSearchSuccess is true and specific operator information
     * @tc.desc    Function test
     */
    it('Telephony_NetworkSearch_getNetworkSearchInformation_Async_0100', 0, async function (done) {
        try{
            let data = await radio.getNetworkState(SLOT_0);
            console.log(`Telephony_NetworkSearch_getNetworkState data: ${JSON.stringify(data)}`)
            if(data.plmnNumeric != '' && data.plmnNumeric != undefined){
                gnetworkSMode.networkInformation.operatorNumeric = data.plmnNumeric;
            }else{
                console.log(`Telephony_NetworkSearch_getNetworkState fail`)
                expect().assertFail();
                done();
                return;
            }
        }catch(err){
            expect().assertFail();
            done();
            return;
        }
        try {
            await radio.setNetworkSelectionMode(gnetworkSMode);
            console.log('Telephony_NetworkSearch_recoverNetworkSelectionMode success');
        } catch (err) {
            console.log('Telephony_NetworkSearch_recoverNetworkSelectionMode fail');
            expect().assertFail();
            done();
            return;
        }
        console.log('Telephony_NetworkSearch_getNetworkSearchInformation_Async_0100 start search')
        radio.getNetworkSearchInformation(SLOT_0, (err, data) => {
            if (err) {
                console.log(`Telephony_NetworkSearch_getNetworkSearchInformation_Async_0100 fail err: ${err}`);
                expect().assertFail();
                done();
                return;
            }
            expect(data != null && data != undefined).assertTrue();
            if (data.networkSearchResult.length === 0) {
                console.log('Telephony_NetworkSearch_getNetworkSearchInformation_Async_0100 fail');
                expect().assertFail();
                done();
                return;
            }
            console.log(
                `Telephony_NetworkSearch_getNetworkSearchInformation_Async_0100 finish data: ${JSON.stringify(data)}`);
            expect(data.isNetworkSearchSuccess).assertTrue();
            expect(
                data.networkSearchResult[0].operatorName != undefined &&
                data.networkSearchResult[0].operatorName != '' &&
                data.networkSearchResult[0].operatorName != null
            ).assertTrue();
            expect(garrNetworkState).assertContain(data.networkSearchResult[0].state);
            expect(garrNetworkRadioTech).assertContain(data.networkSearchResult[0].radioTech);
            expect(data.networkSearchResult[0].operatorNumeric.substr(0, 3) === '460').assertTrue();
            done();
        });
    });

    /**
     * @tc.number  Telephony_NetworkSearch_getNetworkSearchInformation_Promise_0100
     * @tc.name    Test the getNetworkSearchInformation() query function and set the slotId parameter input to 0,
     *             test the return value isNetworkSearchSuccess is true and specific operator information
     * @tc.desc    Function test
     */
    it('Telephony_NetworkSearch_getNetworkSearchInformation_Promise_0100', 0, async function (done) {
        try{
            let data = await radio.getNetworkState(SLOT_0);
            console.log(`Telephony_NetworkSearch_getNetworkState data: ${JSON.stringify(data)}`)
            if(data.plmnNumeric != '' && data.plmnNumeric != undefined){
                gnetworkSMode.networkInformation.operatorNumeric = data.plmnNumeric;
            }else{
                console.log(`Telephony_NetworkSearch_getNetworkState fail`)
                expect().assertFail();
                done();
                return;
            }
        }catch(err){
            expect().assertFail();
            done();
            return;
        }
        try {
            await radio.setNetworkSelectionMode(gnetworkSMode);
            console.log('Telephony_NetworkSearch_recoverNetworkSelectionMode success');
        } catch (err) {
            console.log('Telephony_NetworkSearch_recoverNetworkSelectionMode fail');
            expect().assertFail();
            done();
            return;
        }
        console.log('Telephony_NetworkSearch_getNetworkSearchInformation_Promise_0100 start search');
        try {
            let data = await radio.getNetworkSearchInformation(SLOT_0);
            expect(data != null && data != undefined).assertTrue();
            if (data.networkSearchResult.length === 0) {
                console.log('Telephony_NetworkSearch_getNetworkSearchInformation_Promise_0100 fail');
                expect().assertFail();
                done();
                return;
            }
            console.log(
                `Telephony_NetworkSearch_getNetworkSearchInformation_Promise_0100 finish data:${JSON.stringify(data)}`);
            expect(data.isNetworkSearchSuccess).assertTrue();
            expect(
                data.networkSearchResult[0].operatorName != undefined &&
                data.networkSearchResult[0].operatorName != '' &&
                data.networkSearchResult[0].operatorName != null
            ).assertTrue();
            expect(data.networkSearchResult[0].operatorNumeric.substr(0, 3) === '460').assertTrue();
            expect(garrNetworkState).assertContain(data.networkSearchResult[0].state);
            expect(garrNetworkRadioTech).assertContain(data.networkSearchResult[0].radioTech);
        } catch (err) {
            console.log(`Telephony_NetworkSearch_getNetworkSearchInformation_Promise_0100 fail err: ${err}`);
            expect().assertFail();
            done();
            return;
        }
        done();
    });

    /**
     * @tc.number  Telephony_NetworkSearch_getNetworkSearchInformation_Async_0400
     * @tc.name    SlotId parameter input is 33, test getNetworkSearchInformation() query function go to the error
     * @tc.desc    Function test
     */
    it('Telephony_NetworkSearch_getNetworkSearchInformation_Async_0400', 0, async function (done) {
        try{
            let data = await radio.getNetworkState(SLOT_0);
            console.log(`Telephony_NetworkSearch_getNetworkState data: ${JSON.stringify(data)}`)
            if(data.plmnNumeric != '' && data.plmnNumeric != undefined){
                gnetworkSMode.networkInformation.operatorNumeric = data.plmnNumeric;
            }else{
                console.log(`Telephony_NetworkSearch_getNetworkState fail`)
                expect().assertFail();
                done();
                return;
            }
        }catch(err){
            expect().assertFail();
            done();
            return;
        }
        let slotId = 33;
        radio.getNetworkSearchInformation(slotId, (err, data) => {
            if (err) {
                console.log('Telephony_NetworkSearch_getNetworkSearchInformation_Async_0400 finish');
                done();
            } else {
                console.log('Telephony_NetworkSearch_getNetworkSearchInformation_Async_0400 fail not go to err');
                expect().assertFail();
            }
        });
        done();
    });

    /**
     * @tc.number  Telephony_NetworkSearch_getNetworkSearchInformation_Promise_0400
     * @tc.name    SlotId parameter input is 77, test getNetworkSearchInformation() query function go to the error
     * @tc.desc    Function test
     */
    it('Telephony_NetworkSearch_getNetworkSearchInformation_Promise_0400', 0, async function (done) {
        try {
            let slotId = 77;
            await radio.getNetworkSearchInformation(slotId);
            console.log('Telephony_NetworkSearch_getNetworkSearchInformation_Promise_0400 fail not go to err');
            expect().assertFail();
            done();
            return;
        } catch (err) {
            console.log(`Telephony_NetworkSearch_getNetworkSearchInformation_Promise_0400 finish err: ${err}`);
        }
        done();
    });
});
