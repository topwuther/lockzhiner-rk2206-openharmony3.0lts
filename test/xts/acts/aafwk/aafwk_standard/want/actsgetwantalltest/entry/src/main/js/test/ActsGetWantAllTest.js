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
import featureAbility from '@ohos.ability.featureability'
import bundle from '@ohos.bundle'
import wantConstant from '@ohos.ability.wantConstant'
import {describe, beforeAll, beforeEach, afterEach, afterAll, it, expect} from 'deccjsunit/index'

const installPath = "/data/ActsGetWantAllTestHap.hap"
describe('ActsGetWantTest', function () {
    beforeAll(async (done) => {
        console.debug('=======before all install========');
        bundle.getBundleInstaller().then(data => {
            data.install([
                    installPath], {
                param: {
                    userId: 0,
                    isKeepData: false
                }
            }, onReceiveinstallEvent);
        })

        function onReceiveinstallEvent(err, data) {
            console.info('========install finish========' + JSON.stringify(err));
            console.info('========install finish========' + JSON.stringify(data));
            console.info('========install finish========' + data.status);
            console.info('========install finish========' + data.statusMessage);
            done()
        }
    })
    afterAll((done) => {
        console.debug('=======after all uninstall========');
        bundle.getBundleInstaller().then(data => {
            data.uninstall("com.example.actsgetwantalltesthap", {
                param: {
                    userId: 0,
                    isKeepData: false
                }
            }, onReceiveinstallEvent);

        })

        function onReceiveinstallEvent(err, data) {
            console.info('========uninstall finish========' + JSON.stringify(err));
            console.info('========uninstall finish========' + JSON.stringify(data));
            console.info('========uninstall finish========' + data.status);
            console.info('========uninstall finish========' + data.statusMessage);
            done();
        }
    })
    //  @tc.number: ACTS_GetWant_0100
    //  @tc.name: getWant : get want in current ability
    //  @tc.desc:Start the ability through startabilityforresult,
    //           and then use terminateselfwithresult to return the data
    it('ACTS_GetWant_0100', 0, async function (done) {
        featureAbility.startAbilityForResult({
            want: {
                deviceId: "",
                bundleName: "com.example.actsgetwantalltesthap",
                abilityName: "com.example.actsgetwantalltesthap.MainAbility",
                action: "action1",
                parameters:
                {
                    mykey0: 1,
                    mykey1: [1, 2, 3],
                    mykey2: "[1, 2, 3]",
                    mykey3: "str",
                    mykey4: [false, true, false],
                    mykey5: ["str", "STR", "helloopenharmony"],
                },
            }
        },
        (err, data) => {
            console.info('====> ACTS_StartAbilityForResult_0100 start ability=====>' + JSON.stringify(data))
            expect(data.want.deviceId).assertEqual("");
            expect(data.want.bundleName).assertEqual("com.example.actsgetwantalltesthap");
            expect(data.want.abilityName).assertEqual("com.example.actsgetwantalltesthap.MainAbility");
            expect(data.want.action).assertEqual("action1");
            expect(data.want.parameters.mykey0).assertEqual(1);
            expect(data.want.parameters.mykey1[0]).assertEqual(1);
            expect(data.want.parameters.mykey1[1]).assertEqual(2);
            expect(data.want.parameters.mykey1[2]).assertEqual(3);
            expect(data.want.parameters.mykey2).assertEqual("[1, 2, 3]");
            expect(data.want.parameters.mykey3).assertEqual("str");
            expect(data.want.parameters.mykey4[0]).assertEqual(false);
            expect(data.want.parameters.mykey4[1]).assertEqual(true);
            expect(data.want.parameters.mykey4[2]).assertEqual(false);
            expect(data.want.parameters.mykey5[0]).assertEqual("str");
            expect(data.want.parameters.mykey5[1]).assertEqual("STR");
            expect(data.want.parameters.mykey5[2]).assertEqual("helloopenharmony");

            console.info('====> before done=====>')
            done();
            setTimeout(function () {
                console.info('====> ACTS_StartAbilityForResult_0100 =====>')
            }, 5000)
            console.info('====> after done=====>')
        })
    })


    //  @tc.number: ACTS_GetWant_0200
    //  @tc.name: getWant : get want in current ability
    //  @tc.desc:Start the ability through startabilityforresult,
    //           and then use terminateselfwithresult to return the data
    it('ACTS_GetWant_0200', 0, async function (done) {
        featureAbility.startAbilityForResult({
            want: {
                deviceId: "",
                bundleName: "com.example.actsgetwantalltesthap",
                abilityName: "com.example.actsgetwantalltesthap.MainAbility",
                action: "action2",
            }
        },
            (err, data) => {
                console.info('====> ACTS_StartAbilityForResult_0200 start ability=====>' + JSON.stringify(data))
                expect(data.want.deviceId).assertEqual("");
                expect(data.want.bundleName).assertEqual("com.example.actsgetwantalltesthap");
                expect(data.want.abilityName).assertEqual("com.example.actsgetwantalltesthap.MainAbility");
                expect(data.want.action).assertEqual("action2");
                console.info('====> before done=====>')
                done();
                setTimeout(function () {
                    console.info('====> ACTS_StartAbilityForResult_0200 =====>')
                }, 5000)
                console.info('====> after done=====>')
            })
    })

    //  @tc.number: ACTS_GetWant_0200
    //  @tc.name: getWant : get want in current ability
    //  @tc.desc:Start the ability through startabilityforresult,
    //           and then use terminateselfwithresult to return the data
    it('ACTS_GetWant_0300', 0, async function (done) {
        featureAbility.startAbilityForResult({
                want:
                {
                    deviceId: "",
                    bundleName: "com.example.actsgetwantalltesthap",
                    abilityName: "com.example.actsgetwantalltesthap.MainAbility",
                    action: "action1",
                    entities: ["entity1"],
                    type: "MIMETYPE",
                    uri: "key={true,true,false}",
                },
        },
            (err, data) => {
                console.info('====> ACTS_StartAbilityForResult_0300 start ability=====>' + JSON.stringify(data))
                expect(data.want.deviceId).assertEqual("");
                expect(data.want.bundleName).assertEqual("com.example.actsgetwantalltesthap");
                expect(data.want.abilityName).assertEqual("com.example.actsgetwantalltesthap.MainAbility");
                expect(data.want.action).assertEqual("action1");
                expect(data.want.entities[0]).assertEqual("entity1");
                expect(data.want.type).assertEqual("MIMETYPE");
                expect(data.want.uri).assertEqual("key={true,true,false}");
                console.info('====> before done=====>')
                done();
                setTimeout(function () {
                    console.info('====> ACTS_StartAbilityForResult_0300 =====>')
                }, 5000)
                console.info('====> after done=====>')
            })
    })
    //  @tc.number: ACTS_GetWant_0400
    //  @tc.name: getWant : get want in current ability
    //  @tc.desc:Start the ability through startabilityforresult,
    //           and then use terminateselfwithresult to return the data
    it('ACTS_GetWant_0400', 0, async function (done) {
        featureAbility.startAbilityForResult({
                want:
                {
                    deviceId: "",
                    bundleName: "com.example.actsgetwantalltesthap",
                    abilityName: "com.example.actsgetwantalltesthap.MainAbility",
                    action: "action2",
                    entities: ["entity1","entity2"],
                    type: "MIMETYPE",
                    uri: "key={true,true,false}",
                    flags:wantConstant.Flags.FLAG_ABILITY_FORWARD_RESULT,
                    parameters:
                    {
                        mykey0: 0.1,
                        mykey1: [0.1, 0.2, 0.3],
                        mykey2: "[1, 2, 3]",
                        mykey3: "str",
                        mykey4: [false, true, false],
                        mykey5: ["str", "!@#$%", "helloopenharmony"],
                    },
                },
        },
            (err, data) => {
                console.info('====> ACTS_StartAbilityForResult_0400 start ability=====>' + JSON.stringify(data))
                expect(data.want.deviceId).assertEqual("");
                expect(data.want.bundleName).assertEqual("com.example.actsgetwantalltesthap");
                expect(data.want.abilityName).assertEqual("com.example.actsgetwantalltesthap.MainAbility");
                expect(data.want.action).assertEqual("action2");
                expect(data.want.entities[0]).assertEqual("entity1");
                expect(data.want.entities[1]).assertEqual("entity2");
                expect(data.want.type).assertEqual("MIMETYPE");
                expect(data.want.uri).assertEqual("key={true,true,false}");
                expect(data.want.flags).assertEqual(wantConstant.Flags.FLAG_ABILITY_FORWARD_RESULT);
                expect(data.want.parameters.mykey0).assertEqual(0.1);
                expect(data.want.parameters.mykey1[0]).assertEqual(0.1);
                expect(data.want.parameters.mykey1[1]).assertEqual(0.2);
                expect(data.want.parameters.mykey1[2]).assertEqual(0.3);
                expect(data.want.parameters.mykey2).assertEqual("[1, 2, 3]");
                expect(data.want.parameters.mykey3).assertEqual("str");
                expect(data.want.parameters.mykey4[0]).assertEqual(false);
                expect(data.want.parameters.mykey4[1]).assertEqual(true);
                expect(data.want.parameters.mykey4[2]).assertEqual(false);
                expect(data.want.parameters.mykey5[0]).assertEqual("str");
                expect(data.want.parameters.mykey5[1]).assertEqual("!@#$%");
                expect(data.want.parameters.mykey5[2]).assertEqual("helloopenharmony");
                console.info('====> before done=====>')
                done();
                setTimeout(function () {
                    console.info('====> ACTS_StartAbilityForResult_0400 =====>')
                }, 5000)
                console.info('====> after done=====>')
            })
    })

    //  @tc.number: ACTS_GetWant_0500
    //  @tc.name: getWant : get want in current ability
    //  @tc.desc:Start the ability through startabilityforresult,
    //           and then use terminateselfwithresult to return the data
    it('ACTS_GetWant_0500', 0, async function (done) {
        featureAbility.startAbilityForResult({
                want:
                {
                    deviceId: "",
                    bundleName: "com.example.actsgetwantalltesthap",
                    abilityName: "com.example.actsgetwantalltesthap.MainAbility",
                    action: "action1",
                    entities: ["entity1"],
                    type: "MIMETYPE",
                    uri: "key={true,true,false}",
                    flags:wantConstant.Flags.FLAG_ABILITY_FORM_ENABLED,
                    parameters:
                    {
                        mykey0: 0.1,
                        mykey1: [0.1, 0.2, 0.0000000003],
                        mykey2: "[a, b, c]",
                        mykey3: "str",
                        mykey4: [false, true, false],
                        mykey5: ["str", "STR", "helloopenharmonyhelloopenharmonyhelloopenharmony"],
                    },
                },
        },
            (err, data) => {
                console.info('====> ACTS_StartAbilityForResult_0500 start ability=====>' + JSON.stringify(data))
                expect(data.want.deviceId).assertEqual("");
                expect(data.want.bundleName).assertEqual("com.example.actsgetwantalltesthap");
                expect(data.want.abilityName).assertEqual("com.example.actsgetwantalltesthap.MainAbility");
                expect(data.want.action).assertEqual("action1");
                expect(data.want.entities[0]).assertEqual("entity1");
                expect(data.want.type).assertEqual("MIMETYPE");
                expect(data.want.uri).assertEqual("key={true,true,false}");
                expect(data.want.flags).assertEqual(wantConstant.Flags.FLAG_ABILITY_FORM_ENABLED);
                expect(data.want.parameters.mykey0).assertEqual(0.1);
                expect(data.want.parameters.mykey1[0]).assertEqual(0.1);
                expect(data.want.parameters.mykey1[1]).assertEqual(0.2);
                expect(data.want.parameters.mykey1[2]).assertEqual(0.0000000003);
                expect(data.want.parameters.mykey2).assertEqual("[a, b, c]");
                expect(data.want.parameters.mykey3).assertEqual("str");
                expect(data.want.parameters.mykey4[0]).assertEqual(false);
                expect(data.want.parameters.mykey4[1]).assertEqual(true);
                expect(data.want.parameters.mykey4[2]).assertEqual(false);
                expect(data.want.parameters.mykey5[0]).assertEqual("str");
                expect(data.want.parameters.mykey5[1]).assertEqual("STR");
                expect(data.want.parameters.mykey5[2]).assertEqual("helloopenharmonyhelloopenharmonyhelloopenharmony");
                console.info('====> before done=====>')
                done();
                setTimeout(function () {
                    console.info('====> ACTS_StartAbilityForResult_0500 =====>')
                }, 5000)
                console.info('====> after done=====>')
            })
    })

})
