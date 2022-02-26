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

describe('ActsSubscriberTestOrder', function () {
    console.info("===========ActsSubscriberTestOrder start====================>");
    var num = 0, num2 = 0;
    var order = false;
    var commonEventSubscriber0100;
    var commonEventSubscriber0101;
    var commonEventSubscriber0200_1;
    var commonEventSubscriber0200_2;
    var commonEventSubscriber0300_1;
    var commonEventSubscriber0300_2;
    var commonEventSubscriber0400_1;
    var commonEventSubscriber0400_2;
    var commonEventSubscriber0500_1;
    var commonEventSubscriber0500_2;

    function sleep(delay) {
        var start = (new Date()).getTime();
        while((new Date()).getTime() - start < delay) {
            continue;   
        }
    }

    function publishCallback(err) {
        console.info("===============>publishCallback");
    }

    function unsubscribeCallback(err) {
        console.info("===============>unsubscribeCallback");
    }

    async function subscriberCallBack0100(err, data) {
        console.info("===============>subscriberCallBack0100========event: " + data.event);
        console.info("===============>subscriberCallBack0100========bundleName: " + data.bundleName);
        console.info("===============>subscriberCallBack0100=========code: " + data.code);
        console.info("===============>subscriberCallBack0100=========data: " + data.data);
        expect(data.event).assertEqual("publish_event0100");
        expect(data.code).assertEqual(1);
        expect(data.data).assertEqual("publish_event1001_init");
        expect(data.bundleName).assertEqual("publish_event0100_bundleName");

        commonEventSubscriber0100.setCode(2).then(()=>{
            console.info("===============>subscriberCallBack0100 setCodeCallBack promise");
        })

        commonEventSubscriber0100.setData("publish_event1001_change").then(()=>{
            console.info("===============>subscriberCallBack0100 setDataCallBack promise");
        })

        commonEventSubscriber0100.setCodeAndData(2, "publish_event1001_change").then(()=>{
            console.info("===============>subscriberCallBack0100 setCodeAndDataCallback promise");
        })

        commonEventSubscriber0100.getAbortCommonEvent().then(function(data) {
            console.info("===============>subscriberCallBack0100 getAbortCommonEventCallback1 promise abort: " + data);
        })

        commonEventSubscriber0100.clearAbortCommonEvent().then(()=>{
            console.info("===============>subscriberCallBack0100 clearAbortCommonEventCallback promise");
        })

        commonEventSubscriber0100.getAbortCommonEvent().then(function(data) {
            console.info("===============>subscriberCallBack0100 getAbortCommonEventCallback2 promise abort: " + data);
        })

        commonEventSubscriber0100.finishCommonEvent().then(()=>{
            console.info("===============>subscriberCallBack0100 finishCommonEvent promise");
        })
    }

    async function subscriberCallBack0101(err, data) {
        console.info("===============>subscriberCallBack0101========code: " + err.code);
        console.info("===============>subscriberCallBack0101========event: " + data.event);
        console.info("===============>subscriberCallBack0101========bundleName: " + data.bundleName);
        console.info("===============>subscriberCallBack0101=========code: " + data.code);
        console.info("===============>subscriberCallBack0101=========data: " + data.data);
        expect(data.event).assertEqual("publish_event0100");
        console.info("===============>subscriberCallBack0101=========1 code "+ data.code);
        expect(data.code).assertEqual(2);
        commonEventSubscriber0101.getCode().then(function(data) {
            console.info("===============>subscriberCallBack0101 getCodeCallBack promise code: "+ data);
        })
        expect(data.data).assertEqual("publish_event1001_change");
        commonEventSubscriber0100.getData().then(function(data) {
            console.info("===============>subscriberCallBack0100 getDataCallBack promise data: "+ data);
        })
        expect(data.bundleName).assertEqual("publish_event0100_bundleName");
        commonEventSubscriber0101.finishCommonEvent().then(()=>{
            console.info("===============>subscriberCallBack0101 finishCommonEvent promise");
        })
    }

    async function subscriberCallBack0200_1(err, data) {
        console.info("===============>subscriberCallBack0200_1========event: " + data.event);
        console.info("===============>subscriberCallBack0200_1========bundleName: " + data.bundleName);
        console.info("===============>subscriberCallBack0200_1=========code: " + data.code);
        console.info("===============>subscriberCallBack0200_1=========data: " + data.data);
        console.info("===============>subscriberCallBack0200_1=========num2: " + num2);
        if (num2 == 0) {
            num2++;
            expect(data.event).assertEqual("publish_event0200");
            console.info("===============>subscriberCallBack0200_1=====num2:0====code: " + data.code);
            expect(data.code).assertEqual(1);
            expect(data.data).assertEqual("publish_event0200_init");
            expect(data.bundleName).assertEqual("publish_event0200_bundleName");
        }else {
            num2 = 0;
            expect(data.event).assertEqual("publish_event0201");
            console.info("===============>subscriberCallBack0200_1=====num2:1====code: " + data.code);
            expect(data.code).assertEqual(1);
            expect(data.data).assertEqual("publish_event0201_init");
            expect(data.bundleName).assertEqual("publish_event0201_bundleName");
            commonEventSubscriber0200_1.finishCommonEvent().then(()=>{
                console.info("===============>subscriberCallBack0200_1:num:1:finishCommonEventCallBack");
            })
        }
    }

    async function subscriberCallBack0200_2(err, data) {
        console.info("===============>subscriberCallBack0200_2========event: " + data.event);
        console.info("===============>subscriberCallBack0200_2========bundleName: " + data.bundleName);
        console.info("===============>subscriberCallBack0200_2=========code=: " + data.code);
        console.info("===============>subscriberCallBack0200_2=========data: " + data.data);

        expect(data.event).assertEqual("publish_event0201");
        console.info("===============>subscriberCallBack0200_2=========code: " + data.code);
        expect(data.code).assertEqual(1);
        expect(data.data).assertEqual("publish_event0201_init");
        expect(data.bundleName).assertEqual("publish_event0201_bundleName");
        commonEventSubscriber0200_2.finishCommonEvent().then(()=>{
            console.info("===============>subscriberCallBack0200_2:finishCommonEventCallBack");
        })
    }

    async function subscriberCallBack0300_1(err, data) {
        console.info("===============>subscriberCallBack0300_1========event: " + data.event);
        console.info("===============>subscriberCallBack0300_1========bundleName: " + data.bundleName);
        console.info("===============>subscriberCallBack0300_1=========code: " + data.code);
        console.info("===============>subscriberCallBack0300_1=========dat: " + data.data);
        console.info("===============>subscriberCallBack0300_1=========order: " + order);

        expect(data.event).assertEqual("publish_event0301");
        expect(data.code).assertEqual(1);
        expect(data.data).assertEqual("publish_event0301_init");
        expect(data.bundleName).assertEqual("publish_event0301_bundleName");
        expect(order).assertEqual(1);

        if (order == true) {
            order = false;
        }

        commonEventSubscriber0300_1.isOrderedCommonEvent().then(function(data) {
            console.info("========>subscriberCallBack0300_1 publish_event0301 isOrderedCommonEvent promise " + data);
            expect(data).assertEqual(1);
        })
        commonEventSubscriber0300_1.finishCommonEvent().then(()=>{
            console.info("===============>subscriberCallBack0300_1 publish_event0301 finishCommonEventCallBack");
        })
    }

    async function subscriberCallBack0300_2(err, data) {
        console.info("===============>subscriberCallBack0300_2========event: " + data.event);
        console.info("===============>subscriberCallBack0300_2========bundleName: " + data.bundleName);
        console.info("===============>subscriberCallBack0300_2=========code: " + data.code);
        console.info("===============>subscriberCallBack0300_2=========data: " + data.data);
        console.info("===============>subscriberCallBack0300_2=========order: " + order);

        expect(data.event).assertEqual("publish_event0301");
        expect(data.code).assertEqual(1);
        expect(data.data).assertEqual("publish_event0301_init");
        expect(data.bundleName).assertEqual("publish_event0301_bundleName");

        if (order == false) {
            order = true;
        }

        commonEventSubscriber0300_2.isOrderedCommonEvent().then(function(data) {
            console.info("===============>subscriberCallBack0300_2 isOrderedCommonEvent promise " + data);
            expect(data).assertEqual(1);
        })
        commonEventSubscriber0300_2.finishCommonEvent().then(()=>{
            console.info("===============>subscriberCallBack0300_2 publish_event0301 finishCommonEventCallBack");
        })
    }

    async function subscriberCallBack0400_1(err, data) {
        console.info("===============>SubscriberPromise0400_1========event: " + data.event);
        console.info("===============>SubscriberPromise0400_1========bundleName: " + data.bundleName);
        console.info("===============>SubscriberPromise0400_1=========code: " + data.code);
        console.info("===============>SubscriberPromise0400_1=========data: " + data.data);
        expect(data.event).assertEqual("publish_event0400");
        expect(data.code).assertEqual(1);
        expect(data.data).assertEqual("publish_event0400_init");
        expect(data.bundleName).assertEqual("publish_event0400_bundleName");
        commonEventSubscriber0400_1.getAbortCommonEvent().then(function(data) {
            console.info("====>SubscriberPromise0400_1======>getAbortCommonEventPromise0400_1:1 abort2: " + data);
        });
        commonEventSubscriber0400_1.abortCommonEvent().then(() => {
            console.info("===============>SubscriberPromise0400_1=========>AbortCommonEventPromise0400_1");
        });
        commonEventSubscriber0400_1.getAbortCommonEvent().then(function(data) {
            console.info("===>SubscriberPromise0400_1===========>getAbortCommonEventPromise0400_1:2 abort2: " + data);
        });
        commonEventSubscriber0400_1.finishCommonEvent().then(()=> {
            console.info("===============>SubscriberPromise0400_1==========>finishCommonEventPromise0400_1");
        });
    }

    async function subscriberCallBack0400_2(err, data) {
        console.info("===============>SubscriberPromise0400_2========event: " + data.event);
        expect().assertFail();
    }

    async function subscriberCallBack0500_1(err, data) {
        console.info("===============>SubscriberPromise0500_1========event: " + data.event);
        console.info("===============>SubscriberPromise0500_1========bundleName: " + data.bundleName);
        console.info("===============>SubscriberPromise0500_1=========code: " + data.code);
        console.info("===============>SubscriberPromise0500_1=========data: " + data.data);
        expect(data.event).assertEqual("publish_event0500");
        expect(data.code).assertEqual(10);
        expect(data.data).assertEqual("publish_event0500_init");
        expect(data.bundleName).assertEqual("publish_event0500_bundleName");
        commonEventSubscriber0500_1.finishCommonEvent().then(() => {
            console.info("===============>SubscriberPromise0500_1==========>finishCommonEventPromise0500_1");
        });
    }

    /*
     * @tc.number    : ActsSubscriberTestOrder_0100
     * @tc.name      : verify subscribe and publish : Check subscribe same event and publish common ordered event
     * @tc.desc      : Check the subscriber can receive event "publish_event0100" type of the interface (by Promise)
     */
    it('ActsSubscriberTestOrder_0100', 0, async function (done) {
        console.info("===============ActsSubscriberTestOrder_0100===============>");

        var commonEventSubscribeInfo_1 = {
            events: ["publish_event0100"],
            priority: 10
        };

        var commonEventSubscribeInfo_2 = {
            events: ["publish_event0100"],
            priority: 9
        };

        var commonEventPublishData = {
            bundleName: "publish_event0100_bundleName",
            code: 1,
            data: "publish_event1001_init",
            isOrdered: true,
            isSticky: false,
        }

        Subscriber.createSubscriber(
            commonEventSubscribeInfo_1
        ).then(function (data) {
            console.info("===============ActsSubscriberTestOrder_0100==========createSubscriber promise1");
            commonEventSubscriber0100 = data;
            data.getSubscribeInfo().then(function (data) {
                console.info("===============ActsSubscriberTestOrder_0100=========getSubscribeInfo promise1");
                Subscriber.subscribe(commonEventSubscriber0100, subscriberCallBack0100);
            });
        })

        Subscriber.createSubscriber(
            commonEventSubscribeInfo_2
        ).then(function (data) {
            console.info("===============ActsSubscriberTestOrder_0100==========createSubscriber promise2");
            commonEventSubscriber0101 = data;
            data.getSubscribeInfo().then(function (data) {
                console.info("===============ActsSubscriberTestOrder_0100=========getSubscribeInfo promise2");
                Subscriber.subscribe(commonEventSubscriber0101, subscriberCallBack0101);
                Subscriber.publish("publish_event0100", commonEventPublishData, publishCallback);
            });
        })
        sleep(5);
        done();
    })

    /*
     * @tc.number    : ActsSubscriberTestOrder_0200
     * @tc.name      : verify subscribe and publish : Check subscribe different event
     *                 and twice publish common ordered event and check unsubscribe event
     * @tc.desc      : Check the subscriber can receive event "publish_event0200" type of the interface (by Promise)
     */
    it ('ActsSubscriberTestOrder_0200', 0, async function (done) {
        console.info("===============ActsSubscriberTestOrder_0200===============>");

        var commonEventSubscribeInfo_1 = {
            events: ["publish_event0200",
                "publish_event0201"],
            priority: 10
        };

        var commonEventSubscribeInfo_2 = {
            events: ["publish_event0201"],
            priority: 9
        };

        var commonEventPublishData1 = {
            bundleName: "publish_event0200_bundleName",
            code: 1,
            data: "publish_event0200_init",
            isOrdered: false,
            isSticky: false,
        }

        var commonEventPublishData2 = {
            bundleName: "publish_event0201_bundleName",
            code: 1,
            data: "publish_event0201_init",
            isOrdered: true,
            isSticky: false,
        }

        Subscriber.createSubscriber(
            commonEventSubscribeInfo_1,
        ).then(function (data) {
            console.info("===============ActsSubscriberTestOrder_0200==========createSubscriber promise1");
            commonEventSubscriber0200_1 = data;
            data.getSubscribeInfo().then(function (data) {
                console.info("===============ActsSubscriberTestOrder_0200=========getSubscribeInfo promise1");
                Subscriber.subscribe(commonEventSubscriber0200_1, subscriberCallBack0200_1);
            });
        })

        Subscriber.createSubscriber(
            commonEventSubscribeInfo_2,
        ).then(function (data) {
            console.info("===============ActsSubscriberTestOrder_0200==========createSubscriber promise2");
            commonEventSubscriber0200_2 = data;
            data.getSubscribeInfo().then(function (data) {
                console.info("===============ActsSubscriberTestOrder_0200=========getSubscribeInfo promise2");
                Subscriber.subscribe(commonEventSubscriber0200_2, subscriberCallBack0200_2);
                Subscriber.unsubscribe(commonEventSubscriber0200_1,unsubscribeCallback);
                Subscriber.publish("publish_event0200", commonEventPublishData1, publishCallback);
                Subscriber.publish("publish_event0201", commonEventPublishData2, publishCallback);

            });
        })
        sleep(5);
        done();
    })

    /*
     * @tc.number    : ActsSubscriberTestOrder_0300
     * @tc.name      : verify subscribe and publish : Check subscribe different events
     *                 and some publish common ordered events
     * @tc.desc      : Check the subscriber can receive event "publish_event0301" type of the interface (by Promise)
     */
    it ('ActsSubscriberTestOrder_0300', 0, async function (done) {
        console.info("===============ActsSubscriberTestOrder_0300===============>");

        var commonEventSubscribeInfo_1 = {
            events: ["publish_event0301"],
            priority: 9
        };

        var commonEventSubscribeInfo_2 = {
            events: ["publish_event0301"],
            priority: 10
        };

        var commonEventPublishData2 = {
            bundleName: "publish_event0301_bundleName",
            code: 1,
            data: "publish_event0301_init",
            isOrdered: true,
            isSticky: false,
        }

        Subscriber.createSubscriber(
            commonEventSubscribeInfo_1,
        ).then(function (data) {
            console.info("===============ActsSubscriberTestOrder_0300==========createSubscriber promise1");
            commonEventSubscriber0300_1 = data;
            data.getSubscribeInfo().then(function (data) {
                console.info("===============ActsSubscriberTestOrder_0300=========getSubscribeInfo promise1");
                Subscriber.subscribe(commonEventSubscriber0300_1, subscriberCallBack0300_1);
            });
        })

        Subscriber.createSubscriber(
            commonEventSubscribeInfo_2,
        ).then(function (data) {
            console.info("===============ActsSubscriberTestOrder_0300==========createSubscriber promise2");
            commonEventSubscriber0300_2 = data;
            data.getSubscribeInfo().then(function (data) {
                console.info("===============ActsSubscriberTestOrder_0300=========getSubscribeInfo promise2");
                Subscriber.subscribe(commonEventSubscriber0300_2, subscriberCallBack0300_2);
                var numindex = 0;
                for (; numindex < 3; ++numindex) {
                    Subscriber.publish("publish_event0301", commonEventPublishData2, publishCallback);
                }
            });
        })

        sleep(5);
        done();
    })

    /*
     * @tc.number    : ActsSubscriberTestOrder_0400
     * @tc.name      : verify subscribe and publish : Check subscribe same events
     *                 and publish common ordered events and check abort event
     * @tc.desc      : Check the subscriber can receive event "publish_event0400" type of the interface (by promise)
     */
    it ('ActsSubscriberTestOrder_0400', 0, async function (done) {
        console.info("===============ActsSubscriberTestOrder_0400===============>");

        var commonEventSubscribeInfo_1 = {
            events: ["publish_event0400"],
            priority: 10
        };

        var commonEventSubscribeInfo_2 = {
            events: ["publish_event0400"],
            priority: 9
        };

        var commonEventPublishData = {
            bundleName: "publish_event0400_bundleName",
            code: 1,
            data: "publish_event0400_init",
            isOrdered: true,
            isSticky: false,
        }

        Subscriber.createSubscriber(
            commonEventSubscribeInfo_1
        ).then(function (data) {
            console.info("===============createSubscriberPromise0400_1:1");
            commonEventSubscriber0400_1 = data;
            data.getSubscribeInfo().then(function (data) {
                console.info("=============GetSubscribeInfoPromisek0400_1 data.events[0]: " + data.events[0]);
                expect(data.events[0]).assertEqual("publish_event0400");
                Subscriber.subscribe(commonEventSubscriber0400_1, subscriberCallBack0400_1);
            })
        })

        Subscriber.createSubscriber(
            commonEventSubscribeInfo_2
        ).then(function (data) {
            console.info("==================createSubscriberPromise0400_1:2");
            commonEventSubscriber0400_2 = data;
            data.getSubscribeInfo().then(function (data) {
                console.info("==============GetSubscribeInfoPromisek0400_1 data.events[0] : " + data.events[0]);
                expect(data.events[0]).assertEqual("publish_event0400");
                Subscriber.subscribe(commonEventSubscriber0400_2, subscriberCallBack0400_2);
                Subscriber.publish("publish_event0400", commonEventPublishData, publishCallback);
            })
        })

        sleep(5);
        done();
    })

    /*
     * @tc.number    : ActsSubscriberTestOrder_0500
     * @tc.name      : verify subscribe and publish : Check subscriber same events
     *                 and publish common ordered events and check subscriberInfo priority
     * @tc.desc      : Check the subscriber can receive event "publish_event0500" type of the interface (by promise)
     */
    it ('ActsSubscriberTestOrder_0500', 0, async function (done) {
        console.info("===============ActsSubscriberTestOrder_0500===============>");

        var commonEventSubscribeInfo_1 = {
            events: ["publish_event0500"],
            priority: 1001
        };

        var commonEventSubscribeInfo_2 = {
            events: ["publish_event0500"],
            priority: -101
        };

        var commonEventPublishData = {
            bundleName: "publish_event0500_bundleName",
            code: 10,
            data: "publish_event0500_init",
            isOrdered: true,
            isSticky: false,
        }

        async function subscriberCallBack0500_2(err, data) {
            console.info("===============>SubscriberPromise0500_2========event: " + data.event);
            console.info("===============>SubscriberPromise0500_2========bundleName: " + data.bundleName);
            console.info("===============>SubscriberPromise0500_2=========code: " + data.code);
            console.info("===============>SubscriberPromise0500_2========data: " + data.data);
            expect(data.event).assertEqual("publish_event0500");
            expect(data.code).assertEqual(10);
            expect(data.data).assertEqual("publish_event0500_init");
            expect(data.bundleName).assertEqual("publish_event0500_bundleName");
            done();
        }

        Subscriber.createSubscriber(
            commonEventSubscribeInfo_1
        ).then(function (data) {
            console.info("===============createSubscriberPromise0500_1:1");
            commonEventSubscriber0500_1 = data;
            data.getSubscribeInfo().then(function (data) {
                console.info("=============GetSubscribeInfoPromisek0500_1:1 data.events[0]: " + data.events[0]);
                expect(data.events[0]).assertEqual("publish_event0500");
                Subscriber.subscribe(commonEventSubscriber0500_1, subscriberCallBack0500_1);
            })
        })

        Subscriber.createSubscriber(
            commonEventSubscribeInfo_2
        ).then(function (data) {
            console.info("==================createSubscriberPromise0500_1:2");
            commonEventSubscriber0500_2 = data;
            data.getSubscribeInfo().then(function (data) {
                console.info("==============GetSubscribeInfoPromisek0500_1:2 data.events[0] : " + data.events[0]);
                expect(data.events[0]).assertEqual("publish_event0500");
                Subscriber.subscribe(commonEventSubscriber0500_2, subscriberCallBack0500_2);
                Subscriber.publish("publish_event0500", commonEventPublishData, publishCallback);
            })
        })

        setTimeout(function (){
            console.debug("===================ActsSubscriberTestUnorder_1000 end==================");
        }, 30000);
    })
})
