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

import notify from '@ohos.notification'
import {describe, beforeAll, beforeEach, afterEach, afterAll, it, expect} from 'deccjsunit/index'
var time = 1000
describe('ActsAnsActiveSubscriberTest', function () {
    console.info("===ActsSubscriberTest start===>");

    //consume
    function consumeCallback(err, data) {
        console.debug("===>consumeCallback data : ===>" + JSON.stringify(data));
    }
    //subscribeOn
    function subscribeOnCallback(err) {
        console.debug("===>subscribeOnCallback===>");
    }
    //subscribe
    function subscribeCallback(err) {
        console.debug("===>subscribeCallback===>");
    }

    /*
    * @tc.number: ActsWantAgent_test_0100
    * @tc.name: createSubscriber(),subscribe()
    * @tc.desc: verify the function of createSubscriber,subscribe
    */
    it('ActsActiveSubscriber_test_0100', 0, async function (done) {
        console.debug("==ActsWantAgent_test_0100==begin==>");

        var subInfo ={
            onConsume:consumeCallback,
            onConnect:subscribeOnCallback,
        }
        try{
            await notify.subscribe(subInfo,subscribeCallback);
        }catch(err) {
            console.error('=ActsActiveSubscriber_test_0100  订阅 subscribeCallback err:'+err);
        }
        console.debug("==ActsActiveSubscriber_test_0100==end==>");
        done();
        setTimeout(function(){
            console.debug("====>time out ActsActiveSubscriber_test_0100====>");
        }, time);
    })
})

