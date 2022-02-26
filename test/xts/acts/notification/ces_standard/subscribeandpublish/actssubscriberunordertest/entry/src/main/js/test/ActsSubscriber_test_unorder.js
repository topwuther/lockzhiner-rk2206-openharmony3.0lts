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

describe('ActsSubscriberTestUnorder', async function (done) {
    console.info("===========ActsSubscriberTestUnorder start====================>");
    var commonEventSubscriber001;
    var commonEventSubscriber002;
    var commonEventSubscriber003;
    var commonEventSubscriber004;
    var commonEventSubscriber005_1;
    var commonEventSubscriber005_2;
    var commonEventSubscriber006;
    var commonEventSubscriber007;
    var commonEventSubscriber008;
    var commonEventSubscriber009;
    var commonEventSubscriber010_1;
    var commonEventSubscriber010_2;
    var commonEventSubscriber011;
    var commonEventSubscriber012;
//    var commonEventSubscriber013;

    function publishCallback(err) {
        console.info("==========================>publishCallback");
    }

    function publishCallback1000_1(err) {
        console.info("==========================>publishCallback");
    }

    function publishCallback1000_2(err) {
        console.info("==========================>publishCallback");
    }

    function unsubscriberCallBack(err) {
        console.info("==========================>unsubscriberCallBack");
    }

    /*
     * @tc.number    : ActsSubscriberTestUnorder_0100
     * @tc.name      : verify subscribe and publish : Check subscribe and publish common event data
     * @tc.desc      : Check the subscriber can receive event "publish_event0100" type of the interface (by Promise)
     */
    it('ActsSubscriberTestUnorder_0100', 0, async function (done) {
        console.info("===============ActsSubscriberTestUnorder_0100=============================>");

        function subscriberCallBack001(err, data) {
            console.info("==========================>subscriberCallBack001");
            expect(data.event).assertEqual("publish_event0100");
            expect(data.bundleName).assertEqual("");
            expect(data.code).assertEqual(0);
            expect(data.data).assertEqual("");
            done();
        }

        var commonEventSubscribeInfo = {
            events: ["publish_event0100"],
        };

        Subscriber.createSubscriber(
            commonEventSubscribeInfo
        ).then(function (data) {
            console.info("===============ActsSubscriberTestUnorder_0100=========createSubscriber promise");
            commonEventSubscriber001 = data;
            data.getSubscribeInfo().then(function (data){
                console.info("===============ActsSubscriberTestUnorder_0100=========getSubscribeInfo promise");
                Subscriber.subscribe(commonEventSubscriber001, subscriberCallBack001);
                Subscriber.publish("publish_event0100", publishCallback);
            });
        })

        setTimeout(function (){
            console.debug("===================ActsSubscriberTestUnorder_0100 end==================");
        }, 30000);
    })

    /*
     * @tc.number    : ActsSubscriberTestUnorder_0200
     * @tc.name      : verify subscribe and publish : Check subscribe and publish common event data
     *                 of containing special characters
     * @tc.desc      : Check the subscriber can receive event "@#￥#3243adsafdf_" type of the interface (by Promise)
     */
    it('ActsSubscriberTestUnorder_0200', 0, async function (done) {
        console.info("===============ActsSubscriberTestUnorder_0200==========================>");

        function subscriberCallBack002(err, data) {
            console.info("==========================>subscriberCallBack002");
            expect(data.event).assertEqual("@#￥#3243adsafdf_");
            expect(data.bundleName).assertEqual("");
            expect(data.code).assertEqual(0);
            expect(data.data).assertEqual("");
            done();
        }

        var commonEventSubscribeInfo = {
            events: ["@#￥#3243adsafdf_"],
        };

        Subscriber.createSubscriber(
            commonEventSubscribeInfo
        ).then(function (data) {
            console.info("===============ActsSubscriberTestUnorder_0200=========createSubscriber promise");
            commonEventSubscriber002 = data;
            data.getSubscribeInfo().then(function (data){
                console.info("===============ActsSubscriberTestUnorder_0200=========getSubscribeInfo promise");
                Subscriber.subscribe(commonEventSubscriber002, subscriberCallBack002);
                Subscriber.publish("@#￥#3243adsafdf_", publishCallback);
            });
        })

        setTimeout(function (){
            console.debug("===================ActsSubscriberTestUnorder_0200 end==================");
        }, 30000);
    })

    /*
     * @tc.number    : ActsSubscriberTestUnorder_0300
     * @tc.name      : verify subscribe and publish : Check subscribe and publish common event data
     *                 with publishInfo data
     * @tc.desc      : Check the subscriber can receive event "publish_event0300" type of the interface (by Promise)
     */
    it('ActsSubscriberTestUnorder_0300', 0, async function (done) {
        console.info("===============ActsSubscriberTestUnorder_0300==========================>");
        var commonEventSubscribeInfo = {
            events: ["publish_event0300"],
            publisherDeviceId: "PublishDeviceId0300",
            priority: 10,
        };

        var commonEventPublishData = {
            isOrdered: false,
            bundleName: "PublishBundleName0300",
            code: 55,
            data: "PublishData0300",
        }

        function isOrderedCommonEventCallback003(err, data) {
            console.info("==========================>isOrderedCommonEventCallback003");
            expect(data).assertEqual(false);
            done();
        }

        function subscriberCallBack003(err, data) {
            console.info("==========================>subscriberCallBack003");
            expect(data.event).assertEqual("publish_event0300");
            expect(data.bundleName).assertEqual("PublishBundleName0300");
            expect(data.code).assertEqual(55);
            expect(data.data).assertEqual("PublishData0300");
            commonEventSubscriber003.isOrderedCommonEvent(isOrderedCommonEventCallback003);
        }

        Subscriber.createSubscriber(
            commonEventSubscribeInfo
        ).then(function (data) {
            console.info("===============ActsSubscriberTestUnorder_0300=========createSubscriber promise");
            commonEventSubscriber003 = data;
            data.getSubscribeInfo().then(function (data) {
                console.info("===============ActsSubscriberTestUnorder_0300=========getSubscribeInfo promise");
                Subscriber.subscribe(commonEventSubscriber003, subscriberCallBack003);
                Subscriber.publish("publish_event0300", commonEventPublishData, publishCallback);
            });
        })
        setTimeout(function (){
            console.debug("===================ActsSubscriberTestUnorder_0300 end==================");
        }, 30000);
    })

    /*
     * @tc.number    : ActsSubscriberTestUnorder_0400
     * @tc.name      : verify subscribe and publish : Check subscribe and publish sticky event
     * @tc.desc      : Check the subscriber can receive event "publish_event0400" type of the interface (by Promise)
     */
    it('ActsSubscriberTestUnorder_0400', 0, async function (done) {
        console.info("===============ActsSubscriberTestUnorder_0400==========================>");
        var commonEventSubscribeInfo = {
            events: ["publish_event0400"]
        };

        var commonEventPublishData = {
            isOrdered: false,
            isSticky: true,
        }

        function isStickyCommonEventCallback004(err, data) {
            console.info("==========================>isStickyCommonEventCallback004 isSticky:"+ data);
            expect(data).assertEqual(true);
            done();
        }

        function subscriberCallBack004(err, data) {
            console.info("==========================>subscriberCallBack0400");
            expect(data.event).assertEqual("publish_event0400");
            expect(data.bundleName).assertEqual("");
            expect(data.code).assertEqual(0);
            expect(data.data).assertEqual("");
            commonEventSubscriber004.isStickyCommonEvent(isStickyCommonEventCallback004);
        }

        Subscriber.createSubscriber(
            commonEventSubscribeInfo
        ).then(function (data) {
            console.info("===============ActsSubscriberTestUnorder_0400=========createSubscriber promise");
            commonEventSubscriber004 = data;
            data.getSubscribeInfo().then(function (data){
                console.info("===============ActsSubscriberTestUnorder_0400=========getSubscribeInfo promise");
                Subscriber.subscribe(commonEventSubscriber004, subscriberCallBack004);
                Subscriber.publish("publish_event0400", commonEventPublishData, publishCallback);
            });
        })

        setTimeout(function (){
            console.debug("===================ActsSubscriberTestUnorder_0400 end==================");
        }, 30000);
    })

    /*
     * @tc.number    : ActsSubscriberTestUnorder_0500
     * @tc.name      : verify subscribe and publish : Check the two different subscribe and one publish,
     *                 and check unsubscribe event
     * @tc.desc      : Check the subscriber can receive event "publish_event0500" type of the interface (by Promise)
     */
    it('ActsSubscriberTestUnorder_0500', 0, async function (done) {
        console.info("===============ActsSubscriberTestUnorder_0500==========================>");
        var commonEventSubscribeInfo = {
            events: ["publish_event0500"]
        };

        var commonEventPublishData = {
            isOrdered: false,
            isSticky: false,
        }

        function subscriberCallBack005(err, data) {
            console.info("==========================>subscriberCallBack0500");
            expect(data.event).assertEqual("publish_event0500");
            expect(data.bundleName).assertEqual("");
            expect(data.code).assertEqual(0);
            expect(data.data).assertEqual("");
            done();
        }

        Subscriber.createSubscriber(
            commonEventSubscribeInfo
        ).then(function (data) {
            console.info("===============ActsSubscriberTestUnorder_0500_1=========createSubscriber promise");
            commonEventSubscriber005_1 = data;
            data.getSubscribeInfo().then(function (data) {
                console.info("===============ActsSubscriberTestUnorder_0500_1=========getSubscribeInfo promise");
                Subscriber.subscribe(commonEventSubscriber005_1, subscriberCallBack005);
            });
        })

        Subscriber.createSubscriber(
            commonEventSubscribeInfo
        ).then(function (data) {
            console.info("===============ActsSubscriberTestUnorder_0500_2=========createSubscriber promise");
            commonEventSubscriber005_2 = data;
            data.getSubscribeInfo().then(function (data) {
                console.info("===============ActsSubscriberTestUnorder_0500_2=========getSubscribeInfo promise");
                Subscriber.subscribe(commonEventSubscriber005_2, subscriberCallBack005);
                Subscriber.unsubscribe(commonEventSubscriber005_1, unsubscriberCallBack);
                Subscriber.publish("publish_event0500", commonEventPublishData, publishCallback);
            });
        })

        setTimeout(function (){
            console.debug("===================ActsSubscriberTestUnorder_0500 end==================");
        }, 30000);
    })

    /*
     * @tc.number    : ActsSubscriberTestUnorder_0600
     * @tc.name      : verify subscribe and publish : Check one subscribe and twice publish common events
     * @tc.desc      : Check the subscriber can receive event "publish_event0600" type of the interface (by Promise)
     */
    it('ActsSubscriberTestUnorder_0600', 0, async function (done) {
        console.info("===============ActsSubscriberTestUnorder_0600==========================>");
        var num = 0;
        var commonEventSubscribeInfo = {
            events: ["publish_event0600"]
        };

        var commonEventPublishData = {
            isOrdered: false,
            isSticky: false,
        }

        function subscriberCallBack006(err, data) {
            console.info("==========================>subscriberCallBack0600");
            expect(data.event).assertEqual("publish_event0600");
            expect(data.bundleName).assertEqual("");
            expect(data.code).assertEqual(0);
            expect(data.data).assertEqual("");
            console.info("==========================>subscriberCallBack0600 num = " + num);
            if (num == 0) {
                num++;
            } else if(num == 1) {
                num = 0;
                done();
            }
        }
        console.info("===============ActsSubscriberTestUnorder_0600 11111111111111==========================>");
        Subscriber.createSubscriber(
            commonEventSubscribeInfo
        ).then(function (data) {
            console.info("===============ActsSubscriberTestUnorder_0600=========createSubscriber promise");
            commonEventSubscriber006 = data;
            console.info("===============ActsSubscriberTestUnorder_0600 22222222222222==========================>");
            data.getSubscribeInfo().then(function (data) {
                console.info("===============ActsSubscriberTestUnorder_0700=========getSubscribeInfo promise");
                Subscriber.subscribe(commonEventSubscriber006, subscriberCallBack006);
                Subscriber.publish("publish_event0600", commonEventPublishData, publishCallback);
                Subscriber.publish("publish_event0600", commonEventPublishData, publishCallback);
            });
        })

        setTimeout(function (){
            console.debug("===================ActsSubscriberTestUnorder_0600 end==================");
        }, 30000);
    })

    /*
     * @tc.number    : ActsSubscriberTestUnorder_0700
     * @tc.name      : verify subscribe and publish : Check subscribe and publish common event with permission
     * @tc.desc      : Check the subscriber can receive event "publish_event0700" type of the interface (by Promise)
     */
    it('ActsSubscriberTestUnorder_0700', 0, async function (done) {
        console.info("===============ActsSubscriberTestUnorder_0700==========================>");
        var commonEventSubscribeInfo = {
            events: ["publish_event0700"],
            publisherPermission:"publish_event0700_permission"
        };

        var commonEventPublishData = {
            subscriberPermissions:["publish_event0700_permission"],
            isOrdered: false,
            isSticky: false,
        }

        function subscriberCallBack007(err, data) {
            console.info("==========================>subscriberCallBack0700");
            expect(data.event).assertEqual("publish_event0700");
            expect(data.bundleName).assertEqual("");
            expect(data.code).assertEqual(0);
            expect(data.data).assertEqual("");
            done();
        }

        Subscriber.createSubscriber(
            commonEventSubscribeInfo
        ).then(function (data)  {
            console.info("=================ActsSubscriberTestUnorder_0700=========createSubscriberPromise0700");
            commonEventSubscriber007 = data;
            data.getSubscribeInfo().then(function (data) {
                console.info("===============ActsSubscriberTestUnorder_0700=========getSubscribeInfo promise");
                Subscriber.subscribe(commonEventSubscriber007, subscriberCallBack007);
                Subscriber.publish("publish_event0700", commonEventPublishData, publishCallback);
            });
        });

        setTimeout(function (){
            console.debug("===================ActsSubscriberTestUnorder_0700 end==================");
        }, 30000);
    })

    /*
     * @tc.number    : ActsSubscriberTestUnorder_0800
     * @tc.name      : verify subscribe and publish : Check subscribe and publish common event
     *                 with permission and check IsOrderedCommonEvent
     * @tc.desc      : Check the subscriber can receive event "publish_event0800" type of the interface (by Promise)
     */
    it('ActsSubscriberTestUnorder_0800', 0, async function (done) {
        console.info("===============ActsSubscriberTestUnorder_0800==========================>");
        var commonEventSubscribeInfo = {
            events: ["publish_event0800"],
            publisherPermission:"publish_event0800_publisherPermission"
        };

        var commonEventPublishData = {
            subscriberPermissions:["publish_event0800_subscriberPermissions"],
            isOrdered: false,
            isSticky: false
        }

        function isOrderedCommonEventCallback009(err, data) {
            console.info("==========================>isOrderedCommonEventCallback008");
            expect(data).assertEqual(false);
            done();
        }

        function sleep(delay) {
            var start = (new Date()).getTime();
            while((new Date()).getTime() - start < delay) {
                continue;
            }
            done();
        }

        function subscriberCallBack008(err, data) {
            console.info("==========================>subscriberCallBack0800");
            expect(data.event).assertEqual("publish_event0800");
            expect(data.bundleName).assertEqual("");
            expect(data.code).assertEqual(0);
            expect(data.data).assertEqual("");
            expect().assertFail();
            commonEventSubscriber008.isOrderedCommonEvent(isOrderedCommonEventCallback009);
        }

        Subscriber.createSubscriber(
            commonEventSubscribeInfo
        ).then(function (data)  {
            console.info("==========================>createSubscriberPromise0800=======================>");
            commonEventSubscriber008 = data;
            data.getSubscribeInfo().then(function (data) {
                console.info("===============ActsSubscriberTestUnorder_0800=========getSubscribeInfo promise");
                Subscriber.subscribe(commonEventSubscriber008, subscriberCallBack008);
                Subscriber.publish("publish_event0800", commonEventPublishData, publishCallback);
            });
        });
        sleep(5);
    })

    /*
     * @tc.number    : ActsSubscriberTestUnorder_0900
     * @tc.name      : verify subscribe and publish : Check subscribe and publish sticky event
     *                 with publishpermission and check IsStickyCommonEvent
     * @tc.desc      : Check the subscriber can receive event "publish_event0900" type of the interface (by Promise)
     */
    it('ActsSubscriberTestUnorder_0900', 0, async function (done) {
        console.info("===============ActsSubscriberTestUnorder_0900==========================>");
        var commonEventSubscribeInfo = {
            events: ["publish_event0900"]
        };

        var commonEventPublishData = {
            subscriberPermissions:["publish_event0900_permission"],
            isOrdered: false,
            isSticky: true
        }

        function isStickyCommonEventCallback009(err, data) {
            console.info("==========================>isStickyCommonEventCallback009");
            expect(data).assertEqual(true);
            done();
        }

        function subscriberCallBack009(err, data) {
            console.info("==========================>subscriberCallBack0900");
            expect(data.event).assertEqual("publish_event0900");
            expect(data.bundleName).assertEqual("");
            expect(data.code).assertEqual(0);
            expect(data.data).assertEqual("");
            commonEventSubscriber009.isStickyCommonEvent(isStickyCommonEventCallback009);
        }

        Subscriber.createSubscriber(
            commonEventSubscribeInfo
        ).then(function (data)  {
            console.info("==========================>createSubscriberPromise0900=======================>");
            commonEventSubscriber009 = data;
            data.getSubscribeInfo().then(function (data) {
                console.info("===============ActsSubscriberTestUnorder_0900=========getSubscribeInfo promise=");
                Subscriber.subscribe(commonEventSubscriber009, subscriberCallBack009);
                Subscriber.publish("publish_event0900", commonEventPublishData, publishCallback);
            });
        });

        setTimeout(function (){
            console.debug("===================ActsSubscriberTestUnorder_0900 end==================");
        }, 30000);

    })

    /*
     * @tc.number    : ActsSubscriberTestUnorder_1000
     * @tc.name      : verify subscribe and publish : Check different subscribes and publish different common event
     * @tc.desc      : Check the subscriber can receive event "publish_event1000" type of the interface
     */
    it('ActsSubscriberTestUnorder_1000', 0, async function (done) {
        console.info("===============ActsSubscriberTestUnorder_1000==========================>");

        var num = 0;

        var commonEventSubscribeInfo_1 = {
            events: ["publish_event1000_1"],
            priority: 5
        };

        var commonEventSubscribeInfo_2 = {
            events: ["publish_event1000_2"],
            priority: 10
        };

        var commonEventPublishData_1 = {
            bundleName: "publish_event1000_bundleName1",
            code: 3,
            data: "publish_event1000_data",
            isOrdered: false,
            isSticky: false,
        }

        var commonEventPublishData_2 = {
            bundleName: "publish_event1000_bundleName2",
            code: 5,
            data: "publish_event1000_data",
            isOrdered: false,
            isSticky: false,
        }

        async function subscriberCallBack010_1(err, data) {
            console.info("==========================>subscriberCallBack010_1");
            console.info("==========================>subscriberCallBack010_1 event:"+data.event);
            console.info("==========================>subscriberCallBack010_1 bundleName:"+data.bundleName);
            expect(data.event).assertEqual("publish_event1000_1");
            expect(data.bundleName).assertEqual("publish_event1000_bundleName1");
            expect(data.code).assertEqual(3);
            expect(data.data).assertEqual("publish_event1000_data");
            if (num == 0) {
                num++;
            }else if (num == 1) {
                done();
            }
        }

        function subscriberCallBack010_2(err, data) {
            console.info("==========================>subscriberCallBack010_2");
            console.info("==========================>subscriberCallBack010_2 event:"+data.event);
            console.info("==========================>subscriberCallBack010_2 bundleName:"+data.bundleName);
            expect(data.event).assertEqual("publish_event1000_2");
            expect(data.bundleName).assertEqual("publish_event1000_bundleName2");
            expect(data.code).assertEqual(5);
            expect(data.data).assertEqual("publish_event1000_data");
            if (num == 0) {
                num++;
            }else if (num == 1) {
                done();
            }
        }

        Subscriber.createSubscriber(
            commonEventSubscribeInfo_1
        ).then(function (data)  {
            console.info("=================ActsSubscriberTestUnorder_1000=========>createSubscriber Promise1000_1");
            commonEventSubscriber010_1 = data;
            data.getSubscribeInfo().then(function (data) {
                console.info("===============ActsSubscriberTestUnorder_1000=========getSubscribeInfo promise1000_1");
                Subscriber.subscribe(commonEventSubscriber010_1, subscriberCallBack010_1);
            });
        });

        Subscriber.createSubscriber(
            commonEventSubscribeInfo_2
        ).then(function (data)  {
            console.info("=================ActsSubscriberTestUnorder_1000=========>createSubscriber Promise1000_2");
            commonEventSubscriber010_2 = data;
            data.getSubscribeInfo().then(function (data) {
                console.info("===============ActsSubscriberTestUnorder_1000=========getSubscribeInfo promise1000_2");
                Subscriber.subscribe(commonEventSubscriber010_2, subscriberCallBack010_2);
                Subscriber.publish("publish_event1000_1", commonEventPublishData_1, publishCallback1000_1);
                Subscriber.publish("publish_event1000_2", commonEventPublishData_2, publishCallback1000_2);
            });
        });

        setTimeout(function (){
            console.debug("===================ActsSubscriberTestUnorder_1000 end==================");
        }, 30000);
    })

    /*
     * @tc.number    : ActsSubscriberTestUnorder_1100
     * @tc.name      : verify subscribe and publish : Check one subscribes and publish one common event
     * @tc.desc      : Check the subscriber can receive event "publish_event1100" type of the callback interface
     */
    it('ActsSubscriberTestUnorder_1100', 0, async function (done) {
        console.info("===============ActsSubscriberTestUnorder_1100==========================>");
        function subscriberCallBack011(err, data) {
            console.info("==========================>subscriberCallBack011");
            expect(data.event).assertEqual("publish_event1100");
            expect(data.bundleName).assertEqual("");
            expect(data.code).assertEqual(0);
            expect(data.data).assertEqual("");
            done();
        }

        var commonEventSubscribeInfo = {
            events: ["publish_event1100"],
        };

        function getSubscribeInfo1100CallBack(err, data) {
            console.info("==========================>getSubscribeInfo1100CallBack=======================>");
            expect(data.events[0]).assertEqual("publish_event1100");
            Subscriber.subscribe(commonEventSubscriber011, subscriberCallBack011);
            Subscriber.publish("publish_event1100", publishCallback);
        }

        function createSubscriber1100CallBack(err, data) {
            console.info("==========================>createSubscriber1100CallBack=======================>");
            commonEventSubscriber011 = data;
            data.getSubscribeInfo(getSubscribeInfo1100CallBack);
        }

        Subscriber.createSubscriber(commonEventSubscribeInfo, createSubscriber1100CallBack);

        setTimeout(function (){
            console.debug("===================ActsSubscriberTestUnorder_1100 end==================");
        }, 30000);
    })

    /*
     * @tc.number    : ActsSubscriberTestUnorder_1200
     * @tc.name      : verify subscribe and publish : Check one subscribes and publish one common event with parameters
     * @tc.desc      : Check the subscriber can receive event "publish_event1200" type of the callback interface
     */
   it('ActsSubscriberTestUnorder_1200', 0, async function (done) {
       console.info("===============ActsSubscriberTestUnorder_1200==========================>");
       function subscriberCallBack012(err, data) {
           console.info("==========================>subscriberCallBack012");
           console.info("==========================>subscriberCallBack012 " + JSON.stringify(data));
           expect(data.event).assertEqual("publish_event1200");
           expect(data.bundleName).assertEqual("");
           expect(data.code).assertEqual(0);
           expect(data.data).assertEqual("");
           console.info("==========================>subscriberCallBack012:key1-value = "+ data.parameters["key1"]);
           console.info("==========================>subscriberCallBack012:key2-value = "+ data.parameters["key2"][0]);
           console.info("==========================>subscriberCallBack012:key2-value = "+ data.parameters["key2"][1]);
           console.info("==========================>subscriberCallBack012:key3-value = "+ data.parameters["key3"]);
           done();
       }

       var commonEventSubscribeInfo = {
           events: ["publish_event1200"],
       };

       var commonEventPublishData = {
           isOrdered: false,
           isSticky: false,
           parameters: {
               key1: "ActsSubscriberTestUnorder_1200_key",
               key2: [100,200],
               key3: 44
           }
       }

       function getSubscribeInfo1200CallBack(err, data) {
           console.info("==========================>getSubscribeInfo1200CallBack=======================>");
           expect(data.events[0]).assertEqual("publish_event1200");
           Subscriber.subscribe(commonEventSubscriber012, subscriberCallBack012);
           Subscriber.publish("publish_event1200", commonEventPublishData, publishCallback);
       }

       function createSubscriber1200CallBack(err, data) {
           console.info("==========================>createSubscriber1200CallBack=======================>");
           commonEventSubscriber012 = data;
           data.getSubscribeInfo(getSubscribeInfo1200CallBack);
       }

       Subscriber.createSubscriber(commonEventSubscribeInfo, createSubscriber1200CallBack);

       setTimeout(function (){
           console.debug("===================ActsSubscriberTestUnorder_1200 end==================");
       }, 30000);
   })
})

