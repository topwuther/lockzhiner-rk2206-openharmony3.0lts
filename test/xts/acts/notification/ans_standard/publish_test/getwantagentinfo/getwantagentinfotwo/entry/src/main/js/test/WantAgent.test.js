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

import wantAgent from '@ohos.wantAgent';
import { OperationType, Flags } from '@ohos.wantagent';
import {describe, beforeAll, beforeEach, afterEach, afterAll, it, expect} from 'deccjsunit/index';

var WantAgent1;
var WantAgent2;
var time = 1000
describe('ActsAnsGetWantAgentInfoTwoTest', function () {
    console.info('----ActsGetWantAgentInfoTest----');

    function CancelWantCallback(){
        console.log("=========CancelWantCallback===========")
    }

    /*
    * @tc.number: ACTS_SetWantInfo_0500
    * @tc.name: getWantAgent(),cancel()
    * @tc.desc: verify the function of getWantAgent(),cancel()
    */
    it('ACTS_SetWantInfo_0500', 0, async function (done) {
        console.info('----scene_test_callback_1----');
        var agentInfo1 = {
            wants: [
                    {
                        deviceId: "deviceId",
                        bundleName: "com.neu.WantAgentTest1",
                        abilityName: "com.example.test.MainAbility",
                        action: "action1",
                        entities: ["entity1"],
                        type: "MIMETYPE",
                        uri: "key={true,true,false}",
                        parameters:
                        {
                            mykey0: 2222,
                            mykey1: [1, 2, 3],
                            mykey2: "[1, 2, 3]",
                            mykey3: "ssssssssssssssssssssssssss",
                            mykey4: [false, true, false],
                            mykey5: ["qqqqq", "wwwwww", "aaaaaaaaaaaaaaaaa"],
                            mykey6: true,
                        }
                    },
            ],
            operationType: OperationType.START_ABILITY,
            requestCode: 0,
            wantAgentFlags:[Flags.UPDATE_PRESENT_FLAG]
        }


        console.info('----getWantAgent1 before----');
        wantAgent.getWantAgent(agentInfo1,
            (err, data) => {
                if (err.code == 0) {
                    WantAgent1 = data;
                    console.info('----getWantAgent1 success!----');
                    console.info(data);
                    expect(typeof(data)).assertEqual("object");
                    wantAgent.cancel(data,CancelWantCallback);
                } else {
                    console.info('----getWantAgent failed!----');
                    console.info(data);
                    expect(typeof(data)).assertEqual("object");
                }
                done();
            }),
 		setTimeout(function(){
                    console.debug("====>time out ACTS_SetWantInfo_0500====>");
                }, time)
            console.info('----getWantAgent1 after----');
    })

    /*
    * @tc.number: ACTS_SetWantInfo_0600
    * @tc.name: getWantAgent(),equal()
    * @tc.desc: verify the function of getWantAgent(),equal()
    */
    it('ACTS_SetWantInfo_0600', 0, async function (done) {
        var agentInfo2 = {
            wants: [
                    {
                        deviceId: "deviceId",
                        bundleName: "com.neu.WantAgentTest1",
                        abilityName: "com.example.test.MainAbility",
                        action: "action1",
                        entities: ["entity1"],
                        type: "MIMETYPE",
                        uri: "key={true,true,false}",
                        parameters:
                        {
                            mykey0: 2222,
                            mykey1: [1, 2, 3],
                            mykey2: "[1, 2, 3]",
                            mykey3: "ssssssssssssssssssssssssss",
                            mykey4: [false, true, false],
                            mykey5: ["qqqqq", "wwwwww", "aaaaaaaaaaaaaaaaa"],
                            mykey6: true,
                        }
                    },
            ],
            operationType: OperationType.START_ABILITY,
            requestCode: 0,
            wantAgentFlags:[Flags.UPDATE_PRESENT_FLAG]
        }
        console.info('----getWantAgent2 before----');
        wantAgent.getWantAgent(agentInfo2,
            (err, data) => {
                if (err.code == 0) {
                    WantAgent2 = data;
                    console.info('----getWantAgent2 success!----');
                    console.log("==WantAgent1=="+JSON.stringify(WantAgent1))
                    console.log("==WantAgent2=="+JSON.stringify(WantAgent2))
                    console.info(data);
                    expect(typeof(data)).assertEqual("object");
                    wantAgent.equal(WantAgent1,WantAgent2,
                        (error,data) => {
                            if(error.code == 0) {
                                console.info('----equal1 success!----')
                                console.info(data);
                            }
                            else{
                                console.info('----equal1 failed!----')
                            }
                        }
                    )
                    wantAgent.equal(WantAgent1,WantAgent1,
                        (error,data) => {
                            if(error.code == 0) {
                                console.info('----equal2 success!----')
                                console.info(data);
                            }
                            else{
                                console.info('----equal2 failed!----')
                            }
                        }
                    )
                }
                else{
                    console.info('----getWantAgent failed!----');
                    console.info(data);
                    expect(typeof(data)).assertEqual("object");
                }
                done(); 
            });
	       setTimeout(function(){
                    console.debug("====>time out ACTS_SetWantInfo_0600====>");
                }, time)
            console.info('----getWantAgent2 after----');
      })

})

