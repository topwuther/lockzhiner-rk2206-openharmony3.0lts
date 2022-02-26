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
import abilitymanager from '@ohos.app.abilitymanager'
import { describe, beforeAll, beforeEach, afterEach, afterAll, it, expect } from 'deccjsunit/index'

var WeightReasonCode = {
    REASON_UNKNOWN: 0,
    WEIGHT_FOREGROUND: 100,
    WEIGHT_FOREGROUND_SERVICE: 125,
    WEIGHT_VISIBLE: 200,
    WEIGHT_PERCEPTIBLE: 230,
    WEIGHT_SERVICE: 300,
    WEIGHT_TOP_SLEEPING: 325,
    WEIGHT_CANT_SAVE_STATE: 350,
    WEIGHT_CACHED: 400,
    WEIGHT_GONE: 1000
}

var abilityNameList = [
    "com.ohos.launcher.MainAbility",
    "com.example.SimulateFeatureAbilityFir",
    "com.example.VerifyActThirdAbility",
    "com.example.VerifyIoThirdAbility",
    "com.example.actsamstestfourthscene.MainAbility",
    "com.example.SimulateEntryAbilityFir"
]

var bundleNameList = [
    "com.ohos.launcher",
    "com.ohos.systemui",
    "com.ix.simulate.feature",
    "com.ix.verify.io",
    "com.ix.simulate.entry",
    "com.example.actsamstestfourthscene",
    "com.ix.verify.act"
]
describe('ActsAmsTestFourthScene', function () {
    console.info('----ActsAmsTestFourthScene----');
    beforeAll(async function (done) {
        await featureAbility.startAbility(
            {
                want:
                {
                    deviceId: "",
                    bundleName: "com.ohos.launcher",
                    abilityName: "com.ohos.launcher.MainAbility",
                    action: "action1",
                    entities: ["entity1"],
                    type: "MIMETYPE",
                    uri: "key={true,true,false}",
                    options:
                        {},
                    parameters:
                        {},
                },
            },
        );
        var maxnum = 10, flag = 1;
        var data = await abilitymanager.queryRecentAbilityMissionInfos(maxnum, flag);
        console.log('queryRecentAbilityMissionInfos data  ' + JSON.stringify(data));
        for (var i = 0; i < data.length; i++) {
            if (data[i].baseAbility.bundleName != 'com.example.actsamstestfourthscene' &&
                data[i].topAbility.bundleName != 'com.example.actsamstestfourthscene') {
                var info = abilitymanager.removeMission(data[i].id);
                console.log(' removeMission data  [' + info + ']');
            }
        };
        await featureAbility.startAbility(
            {
                want:
                {
                    deviceId: "",
                    bundleName: "com.ix.verify.io",
                    abilityName: "com.example.VerifyIoThirdAbility",
                    action: "action1",
                    entities: ["entity1"],
                    type: "MIMETYPE",
                    uri: "key={true,true,false}",
                    options:
                        {},
                    parameters:
                        {},
                },
            },
        );
        await featureAbility.startAbility(
            {
                want:
                {
                    deviceId: "",
                    bundleName: "com.ix.simulate.feature",
                    abilityName: "com.example.SimulateFeatureAbilityFir",
                    action: "action1",
                    entities: ["entity1"],
                    type: "MIMETYPE",
                    uri: "key={true,true,false}",
                    options:
                        {},
                    parameters:
                        {},
                },
            },
        );
        await featureAbility.startAbility(
            {
                want:
                {
                    deviceId: "",
                    bundleName: "com.ix.verify.act",
                    abilityName: "com.example.VerifyActThirdAbility",
                    action: "action1",
                    entities: ["entity1"],
                    type: "MIMETYPE",
                    uri: "key={true,true,false}",
                    options:
                        {},
                    parameters:
                        {},
                },
            },
        );
        await featureAbility.startAbility(
            {
                want:
                {
                    deviceId: "",
                    bundleName: "com.ix.simulate.entry",
                    abilityName: "com.example.SimulateEntryAbilityFir",
                    action: "action1",
                    entities: ["entity1"],
                    type: "MIMETYPE",
                    uri: "key={true,true,false}",
                    options:
                        {},
                    parameters:
                        {},
                },
            },
        );
        setTimeout(done(), 5000);
    });

    function timeout(done) {
        expect().assertFail();
        console.debug('Acts_Ams_test=========timeout========');
        done();
    }

    function sleep(delay) {
        var start = (new Date()).getTime();
        while((new Date()).getTime() - start < delay) {
            continue;
        }
    }

    /*
     * @tc.number    : Acts_Ams_test_4900
     * @tc.name      : getAllRunningProcesses : Get All Running Processes Info
     * @tc.desc      : Get All Running Processes Info(by Promise)
     */
    it('Acts_Ams_test_4900', 0, async function (done) {
        console.info("sleep begin");
        sleep(5000);
        console.info("sleep end");
        var info = await abilitymanager.getAllRunningProcesses();
        console.info('getAllRunningProcesses data length [' + info.length + ']');
        console.info('Acts_Ams_test_4900 getAllRunningProcesses JSON String: ' + JSON.stringify(info));
        expect(Array.isArray(info)).assertEqual(true);
        expect(info.length).assertEqual(6);
        for (var i = 0; i < info.length; i++) {
            expect(typeof (info[i].pid)).assertEqual("number");
            expect(info[i].pid).assertLarger(0);

            expect(typeof (info[i].processName)).assertEqual("string");
            expect(info[i].processName.length).assertLarger(0);
            expect(bundleNameList.indexOf(info[i].processName)).assertLarger(-1);

            expect(Array.isArray(info[i].pkgList)).assertEqual(true);
            expect(info[i].pkgList.length).assertEqual(0);

            expect(typeof (info[i].uid)).assertEqual("number");
            expect(info[i].uid).assertLarger(0);

            expect(typeof (info[i].lastMemoryLevel)).assertEqual("number");
            expect(info[i].lastMemoryLevel).assertEqual(-1);

            expect(typeof (info[i].weight)).assertEqual("number");
            expect(info[i].weight).assertEqual(-1);

            expect(typeof (info[i].weightReasonCode)).assertEqual("number");
            expect(info[i].weightReasonCode).assertEqual(WeightReasonCode.REASON_UNKNOWN);
        }
        done();
        setTimeout(timeout, 5000);
    })

    /*
     * @tc.number    : Acts_Ams_test_5300
     * @tc.name      : queryRecentAbilityMissionInfos : Query Recent Ability Mission Infos
     * @tc.desc      : Query Recent Ability Mission Infos(by Promise)
     */
    it('Acts_Ams_test_5300', 0, async function (done) {
        var maxnum = 100, flag = 1;
        var data = await abilitymanager.queryRecentAbilityMissionInfos(maxnum, flag);
        console.info(' queryRecentAbilityMissionInfos data length [' + data.length + ']');
        expect(Array.isArray(data)).assertEqual(true);
        expect(data.length).assertEqual(4);
        for (var i = 0; i < data.length; i++) {
            console.info('Acts_Ams_test_5300 queryRecentAbilityMissionInfos  id: \
                ' + data[i].id + ' baseAbility.deviceId: \
                ' + data[i].baseAbility.deviceId + ' baseAbility.bundleName: \
                ' + data[i].baseAbility.bundleName + ' baseAbility.abilityName: \
                ' + data[i].baseAbility.abilityName + ' topAbility.deviceId: \
                ' + data[i].topAbility.deviceId + ' topAbility.bundleName: \
                ' + data[i].topAbility.bundleName + ' topAbility.abilityName: \
                ' + data[i].topAbility.abilityName + ' missionDescription.label: \
                ' + data[i].missionDescription.label + ' missionDescription.iconPath: \
                ' + data[i].missionDescription.iconPath);

            expect(typeof (data[i].id)).assertEqual("number");
            expect(data[i].id).assertLarger(0);

            expect(typeof (data[i].baseAbility)).assertEqual("object");
            expect(typeof (data[i].baseAbility.deviceId)).assertEqual("string");
            expect(data[i].baseAbility.deviceId.length).assertEqual(0);
            expect(typeof (data[i].baseAbility.bundleName)).assertEqual("string");
            expect(data[i].baseAbility.bundleName.length).assertLarger(0);
            expect(bundleNameList.indexOf(data[i].baseAbility.bundleName)).assertLarger(-1);
            expect(typeof (data[i].baseAbility.abilityName)).assertEqual("string");
            expect(data[i].baseAbility.abilityName.length).assertLarger(0);
            expect(abilityNameList.indexOf(data[i].baseAbility.abilityName)).assertLarger(-1);

            expect(typeof (data[i].topAbility)).assertEqual("object");
            expect(typeof (data[i].topAbility.deviceId)).assertEqual("string");
            expect(typeof (data[i].topAbility.bundleName)).assertEqual("string");
            expect(data[i].topAbility.bundleName.length).assertLarger(0);
            expect(bundleNameList.indexOf(data[i].topAbility.bundleName)).assertLarger(-1);
            expect(typeof (data[i].topAbility.abilityName)).assertEqual("string");
            expect(data[i].topAbility.abilityName.length).assertLarger(0);
            expect(abilityNameList.indexOf(data[i].topAbility.abilityName)).assertLarger(-1);

            expect(typeof (data[i].missionDescription)).assertEqual("object");
            expect(typeof (data[i].missionDescription.label)).assertEqual("string");
            expect(typeof (data[i].missionDescription.iconPath)).assertEqual("string");
        }
        done();
        setTimeout(timeout, 5000);
    })

    /*
     * @tc.number    : Acts_Ams_test_5100
     * @tc.name      : queryRunningAbilityMissionInfos : Query Running Ability Mission Infos
     * @tc.desc      : Query Running Ability Mission Infos(by Promise)
     */
    it('Acts_Ams_test_5100', 0, async function (done) {
        var maxnum = 30;
        var data = await abilitymanager.queryRunningAbilityMissionInfos(maxnum);
        console.info('queryRunningAbilityMissionInfos data length [' + data.length + ']');
        expect(Array.isArray(data)).assertEqual(true);
        expect(data.length).assertEqual(4);
        for (var i = 0; i < data.length; i++) {
            console.info('Acts_Ams_test_5100 queryRunningAbilityMissionInfos  id: \
                ' + data[i].id + ' baseAbility.deviceId: \
                ' + data[i].baseAbility.deviceId + ' baseAbility.bundleName: \
                ' + data[i].baseAbility.bundleName + ' baseAbility.abilityName: \
                ' + data[i].baseAbility.abilityName + ' topAbility.deviceId: \
                ' + data[i].topAbility.deviceId + ' topAbility.bundleName: \
                ' + data[i].topAbility.bundleName + ' topAbility.abilityName: \
                ' + data[i].topAbility.abilityName + ' missionDescription.label: \
                ' + data[i].missionDescription.label + ' missionDescription.iconPath: \
                ' + data[i].missionDescription.iconPath);

            expect(typeof (data[i].id)).assertEqual("number");
            expect(data[i].id).assertLarger(0);

            expect(typeof (data[i].baseAbility)).assertEqual("object");
            expect(typeof (data[i].baseAbility.deviceId)).assertEqual("string");
            expect(data[i].baseAbility.deviceId.length).assertEqual(0);
            expect(typeof (data[i].baseAbility.bundleName)).assertEqual("string");
            expect(data[i].baseAbility.bundleName.length).assertLarger(0);
            expect(bundleNameList.indexOf(data[i].baseAbility.bundleName)).assertLarger(-1);
            expect(typeof (data[i].baseAbility.abilityName)).assertEqual("string");
            expect(data[i].baseAbility.abilityName.length).assertLarger(0);
            expect(abilityNameList.indexOf(data[i].baseAbility.abilityName)).assertLarger(-1);

            expect(typeof (data[i].topAbility)).assertEqual("object");
            expect(typeof (data[i].topAbility.deviceId)).assertEqual("string");
            expect(typeof (data[i].topAbility.bundleName)).assertEqual("string");
            expect(data[i].topAbility.bundleName.length).assertLarger(0);
            expect(bundleNameList.indexOf(data[i].topAbility.bundleName)).assertLarger(-1);
            expect(typeof (data[i].topAbility.abilityName)).assertEqual("string");
            expect(data[i].topAbility.abilityName.length).assertLarger(0);
            expect(abilityNameList.indexOf(data[i].topAbility.abilityName)).assertLarger(-1);

            expect(typeof (data[i].missionDescription)).assertEqual("object");
            expect(typeof (data[i].missionDescription.label)).assertEqual("string");
            expect(typeof (data[i].missionDescription.iconPath)).assertEqual("string");
        }
        done();
        setTimeout(timeout, 5000);
    })

    /*
     * @tc.number    : Acts_Ams_test_5500
     * @tc.name      : removeMission : Remove Mission
     * @tc.desc      : Remove Mission(by Promise)
     */
    it('Acts_Ams_test_5500', 0, async function (done) {
        var maxnum = 30;
        var result = await abilitymanager.queryRunningAbilityMissionInfos(maxnum);
        var info = await abilitymanager.removeMission(result[0].id);
        console.info('Acts_Ams_test_5500 removeMission data  [' + info + ']');
        expect(typeof (info)).assertEqual("number");
        expect(info).assertEqual(0);
        done();
        setTimeout(timeout, 5000);
    })

    /*
     * @tc.number    : Acts_Ams_test_5900
     * @tc.name      : moveMissionToTop : Move Mission To Top
     * @tc.desc      : Move Mission To Top(by Promise)
     */
    it('Acts_Ams_test_5900', 0, async function (done) {
        var maxnum = 30;
        var result = await abilitymanager.queryRunningAbilityMissionInfos(maxnum);
        var info = await abilitymanager.moveMissionToTop(result[0].id);
        console.info('Acts_Ams_test_5900 moveMissionToTop data  [' + info + ']');
        expect(typeof (info)).assertEqual("number");
        expect(info).assertEqual(0);
        done();
        setTimeout(timeout, 5000);
    })

    /*
     * @tc.number    : Acts_Ams_test_6100
     * @tc.name      : removeMissions: Remove Missions
     * @tc.desc      : Remove Missions(by Promise)
     */
    it('Acts_Ams_test_6100', 0, async function (done) {
        var maxnum = 30;
        var result = await abilitymanager.queryRunningAbilityMissionInfos(maxnum);
        var info = await abilitymanager.removeMissions([result[0].id, result[1].id]);
        console.info('Acts_Ams_test_6100 removeMissions data  [' + info + ']');
        expect(typeof (info)).assertEqual("number");
        expect(info).assertEqual(0);
        done();
        setTimeout(timeout, 5000);
    })

    /*
     * @tc.number    : Acts_Ams_test_6300
     * @tc.name      : killProcessesByBundleName : Kill Processes By BundleName
     * @tc.desc      : Kill Processes By BundleName(by Promise)
     */
    it('Acts_Ams_test_6300', 0, async function (done) {
        var info = await abilitymanager.killProcessesByBundleName('xxxxxxxxxxxx');
        console.info('Acts_Ams_test_6300 killProcessesByBundleName data  [' + info + ']');
        expect(typeof (info)).assertEqual("number");
        expect(info).assertEqual(0);
        done();
        setTimeout(timeout, 5000);
    })

    /*
     * @tc.number    : Acts_Ams_test_8400
     * @tc.name      : getActiveProcessInfos : Get All Active Processes Info
     * @tc.desc      : Get All Active Processes Info(by Promise)
     */
    it('Acts_Ams_test_8400', 0, async function (done) {
        var info = await abilitymanager.getActiveProcessInfos();
        console.info('Acts_Ams_test_8400 getActiveProcessInfos JSON String: ' + JSON.stringify(info));
        expect(Array.isArray(info)).assertEqual(true);
        expect(info.length).assertEqual(3);
        for (var i = 0; i < info.length; i++) {
            expect(typeof (info[i].pid)).assertEqual("number");
            expect(info[i].pid).assertLarger(0);

            expect(typeof (info[i].processName)).assertEqual("string");
            expect(info[i].processName.length).assertLarger(0);
            expect(bundleNameList.indexOf(info[i].processName)).assertLarger(-1);

            expect(Array.isArray(info[i].bundleNames)).assertEqual(true);
            expect(info[i].bundleNames.length).assertEqual(0);

            expect(typeof (info[i].uid)).assertEqual("number");
            expect(info[i].uid).assertLarger(0);
        }
        done();
        setTimeout(timeout, 5000);
    })

    /*
     * @tc.number    : Acts_Ams_test_9400
     * @tc.name      : getActiveAbilityMissionInfos : Get Active Ability Mission Infos
     * @tc.desc      : Get Active Ability Mission Infos(by Promise)
     */
    it('Acts_Ams_test_9400', 0, async function (done) {
        var upperLimit = 20;
        var data = await abilitymanager.getActiveAbilityMissionInfos(upperLimit);
        console.info('Acts_Ams_test_9400 getActiveAbilityMissionInfos data ' + JSON.stringify(data));
        expect(Array.isArray(data)).assertEqual(true);
        expect(data.length).assertEqual(1);
        for (var i = 0; i < data.length; i++) {
            expect(typeof (data[i].missionId)).assertEqual("number");
            expect(data[i].missionId).assertLarger(0);

            expect(typeof (data[i].bottomAbility)).assertEqual("object");
            expect(typeof (data[i].bottomAbility.deviceId)).assertEqual("string");
            expect(data[i].bottomAbility.deviceId.length).assertEqual(0);
            expect(typeof (data[i].bottomAbility.bundleName)).assertEqual("string");
            expect(data[i].bottomAbility.bundleName.length).assertLarger(0);
            expect(bundleNameList.indexOf(data[i].bottomAbility.bundleName)).assertLarger(-1);
            expect(typeof (data[i].bottomAbility.abilityName)).assertEqual("string");
            expect(data[i].bottomAbility.abilityName.length).assertLarger(0);
            expect(abilityNameList.indexOf(data[i].bottomAbility.abilityName)).assertLarger(-1);
            expect(typeof (data[i].bottomAbility.uri)).assertEqual("string");
            expect(data[i].bottomAbility.uri.length).assertEqual(0);
            expect(typeof (data[i].bottomAbility.shortName)).assertEqual("string");
            expect(data[i].bottomAbility.shortName.length).assertEqual(0);

            expect(typeof (data[i].topAbility)).assertEqual("object");
            expect(typeof (data[i].topAbility.deviceId)).assertEqual("string");
            expect(typeof (data[i].topAbility.bundleName)).assertEqual("string");
            expect(data[i].topAbility.bundleName.length).assertLarger(0);
            expect(bundleNameList.indexOf(data[i].topAbility.bundleName)).assertLarger(-1);
            expect(typeof (data[i].topAbility.abilityName)).assertEqual("string");
            expect(data[i].topAbility.abilityName.length).assertLarger(0);
            expect(abilityNameList.indexOf(data[i].topAbility.abilityName)).assertLarger(-1);
            expect(typeof (data[i].topAbility.uri)).assertEqual("string");
            expect(data[i].topAbility.uri.length).assertEqual(0);
            expect(typeof (data[i].topAbility.shortName)).assertEqual("string");
            expect(data[i].topAbility.shortName.length).assertEqual(0);

            expect(typeof (data[i].windowMode)).assertEqual("number");
            expect(data[i].windowMode).assertEqual(0);
        }
        done();
        setTimeout(timeout, 5000);
    })

    /*
     * @tc.number    : Acts_Ams_test_10400
     * @tc.name      : getPreviousAbilityMissionInfos : Get Previous Ability Mission Infos
     * @tc.desc      : Get Previous Ability Mission Infos(by Promise)
     */
    it('Acts_Ams_test_10400', 0, async function (done) {
        var upperLimit = 20;
        var data = await abilitymanager.getPreviousAbilityMissionInfos(upperLimit);
        console.info('Acts_Ams_test_10400 getPreviousAbilityMissionInfos data ' + JSON.stringify(data));
        expect(Array.isArray(data)).assertEqual(true);
        expect(data.length).assertEqual(1);
        for (var i = 0; i < data.length; i++) {
            expect(typeof (data[i].missionId)).assertEqual("number");
            expect(data[i].missionId).assertLarger(0);

            expect(typeof (data[i].bottomAbility)).assertEqual("object");
            expect(typeof (data[i].bottomAbility.deviceId)).assertEqual("string");
            expect(data[i].bottomAbility.deviceId.length).assertEqual(0);
            expect(typeof (data[i].bottomAbility.bundleName)).assertEqual("string");
            expect(data[i].bottomAbility.bundleName.length).assertLarger(0);
            expect(bundleNameList.indexOf(data[i].bottomAbility.bundleName)).assertLarger(-1);
            expect(typeof (data[i].bottomAbility.abilityName)).assertEqual("string");
            expect(data[i].bottomAbility.abilityName.length).assertLarger(0);
            expect(abilityNameList.indexOf(data[i].bottomAbility.abilityName)).assertLarger(-1);
            expect(typeof (data[i].bottomAbility.uri)).assertEqual("string");
            expect(data[i].bottomAbility.uri.length).assertEqual(0);
            expect(typeof (data[i].bottomAbility.shortName)).assertEqual("string");
            expect(data[i].bottomAbility.shortName.length).assertEqual(0);

            expect(typeof (data[i].topAbility)).assertEqual("object");
            expect(typeof (data[i].topAbility.deviceId)).assertEqual("string");
            expect(typeof (data[i].topAbility.bundleName)).assertEqual("string");
            expect(data[i].topAbility.bundleName.length).assertLarger(0);
            expect(bundleNameList.indexOf(data[i].topAbility.bundleName)).assertLarger(-1);
            expect(typeof (data[i].topAbility.abilityName)).assertEqual("string");
            expect(data[i].topAbility.abilityName.length).assertLarger(0);
            expect(abilityNameList.indexOf(data[i].topAbility.abilityName)).assertLarger(-1);
            expect(typeof (data[i].topAbility.uri)).assertEqual("string");
            expect(data[i].topAbility.uri.length).assertEqual(0);
            expect(typeof (data[i].topAbility.shortName)).assertEqual("string");
            expect(data[i].topAbility.shortName.length).assertEqual(0);

            expect(typeof (data[i].windowMode)).assertEqual("number");
            expect(data[i].windowMode).assertEqual(0);
        }
        done();
        setTimeout(timeout, 5000);
    })

})