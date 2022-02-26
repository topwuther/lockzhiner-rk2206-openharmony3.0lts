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

import Subscriber from '@ohos.commonevent'
import wantAgent from '@ohos.wantAgent';
import { OperationType, Flags } from '@ohos.wantagent';
import {describe, beforeAll, beforeEach, afterEach, afterAll, it, expect} from 'deccjsunit/index'
var commonEventSubscribeInfo;
var commonEventSubscriber;
var WantAgent;
var time = 1000
describe('ActsAnsCommonSubscriberTest', function () {
    console.info("===========ActsSubscriberTest start====================>");

    //consume
   async function getSubscribeInfoCallBack(err, data) {
       console.log("============data=========="+JSON.stringify(data))
//        expect(data.events[0]).assertEqual("action1");
        await Subscriber.subscribe(commonEventSubscriber, subscriberCallBack001);
    }
    function subscriberCallBack001(err, data) {
        console.info("==========================>subscriberCallBack001");
        expect(data.event).assertEqual("action1");
        expect(data.bundleName).assertEqual("");
        expect(data.code).assertEqual(0);
        expect(data.data).assertEqual("");
    }
    //subscribe
    function subscribeCallback(err) {
        console.debug("==========================>subscribeCallback=======================>");
    }

    /*
     * @tc.number    : ActsSubscriberTestUnorder_0100
     * @tc.name      : verify subscribe and publish : Check subscribe and publish common event data
     * @tc.desc      : Check the subscriber can receive event "publish_event0100" type of the interface (by Promise)
     */
    it('ActsSubscriberTestUnorder_0100', 0, async function (done) {
        console.info("===============ActsSubscriberTestUnorder_0100=============================>");
        commonEventSubscribeInfo = {
            events: ["action1"],
        };

        await Subscriber.createSubscriber(
            commonEventSubscribeInfo
        ).then(async (data) =>{
            console.info("===============ActsSubscriberTestUnorder_0100=========createSubscriber promise"+JSON.stringify(data));
            commonEventSubscriber = data;
            await data.getSubscribeInfo().then(async (data)=>{
                console.info("===============ActsSubscriberTestUnorder_0100=========getSubscribeInfo promise");
                await getSubscribeInfoCallBack(data);
            });
        })
        console.info("===============ActsSubscriberTestUnorder_0100==============================>");
        done();
        setTimeout(function(){
            console.debug("====>time out ActsSubscriberTestUnorder_0100====>");
        }, time)
    })
})

