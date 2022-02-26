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
import {describe, beforeAll, beforeEach, afterEach, afterAll, it, expect} from 'deccjsunit/index'

describe('ActsAmsCallBackSixth Scene', function () {
    console.info('----ActsAmsCallBackSixth----');
    beforeAll(function () {
        featureAbility.startAbility(
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
    });
    /*
    * @tc.number    : Acts_Ams_test_12200
    * @tc.name      : clearMissions : Clear Missions
    * @tc.desc      : Clear Missions(by CallBack)
     */
    it('Acts_Ams_test_12200', 0, async function (done) {
        setTimeout(function (){
            abilitymanager.clearMissions(
                (error,info) => {
                    console.info('Acts_Ams_test_12200 clearMissions error.code ' + error.code + ',data  [' + info + ']' );
                    expect(typeof(info)).assertEqual("number");
                    expect(info).assertEqual(0);
                });
            done();
        },5000)
    })
})
