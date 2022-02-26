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
import {describe, beforeAll, beforeEach, afterEach, afterAll, it, expect} from 'deccjsunit/index'

describe('ActsSubscriberTestUnorderSystem', async function (done) {
    console.info("===========ActsSubscriberTestUnorderSystem start====================>");
    var commonEventSubscriber;

    function publishCallback(err) {
        console.info("==========================>publishCallback");
    }

    /*
     * @tc.number    : ActsSubscriberTestUnorderSystem_0100
     * @tc.name      : verify subscribe and publish : Check subscribe and publish common event data
     *                 with publishInfo data
     * @tc.desc      : Check the subscriber can receive event "publish_event0100" type of the interface (by Promise)
     */
    it('ActsSubscriberTestUnorderSystem_0100', 0, async function (done) {
        console.info("===============ActsSubscriberTestUnorderSystem_0100==========================>");
        var commonEventSubscribeInfo = {
            events: ["publish_event0100"],
            publisherDeviceId: "PublishDeviceId0100",
            priority: 10,
        };

        var commonEventPublishData = {
            isOrdered: false,
            bundleName: "PublishBundleName0100",
            code: 55,
            data: "PublishData0100",
        }

        function isOrderedCommonEventCallback001(err, data) {
            console.info("==========================>isOrderedCommonEventCallback001");
            expect(data).assertEqual(false);
            done();
        }

        function subscriberCallBack001(err, data) {
            console.info("==========================>subscriberCallBack001");
            expect(data.event).assertEqual("publish_event0100");
            expect(data.bundleName).assertEqual("PublishBundleName0100");
            expect(data.code).assertEqual(55);
            expect(data.data).assertEqual("PublishData0100");
            commonEventSubscriber.isOrderedCommonEvent(isOrderedCommonEventCallback001);
        }

        Subscriber.createSubscriber(
            commonEventSubscribeInfo
        ).then(function (data) {
            console.info("===============ActsSubscriberTestUnorderSystem_0100=========createSubscriber promise");
            commonEventSubscriber = data;
            data.getSubscribeInfo().then(function (data) {
                console.info("===============ActsSubscriberTestUnorderSystem_0100=========getSubscribeInfo promise");
                Subscriber.subscribe(commonEventSubscriber, subscriberCallBack001);
                Subscriber.publish("publish_event0100", commonEventPublishData, publishCallback);
            });
        })
        setTimeout(function (){
            console.debug("===================ActsSubscriberTestUnorderSystem_0100 end==================");
        }, 30000);
    })
})

