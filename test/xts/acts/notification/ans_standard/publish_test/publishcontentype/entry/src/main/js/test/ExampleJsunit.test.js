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
import notification from '@ohos.notification'
import {describe, beforeAll, beforeEach, afterEach, afterAll, it, expect} from 'deccjsunit/index'
var comsumData
var time = 1000
describe('ActsAnsNotificationTest', function () {
    function publishMULTILINEContentCallback001(error){
        console.log('ActsNotificationTest ACTS_PublishMULTILINEContent_0100 asyncCallback'+JSON.stringify(error.code))
    }
    function publishMULTILINEContentCallback002(error){
        console.log('ActsNotificationTest ACTS_PublishMULTILINEContent_0200 asyncCallback'+JSON.stringify(error.code))
    }
    function publishMULTILINEContentCallback003(error){
        console.log('ActsNotificationTest ACTS_PublishMULTILINEContent_0300 asyncCallback'+JSON.stringify(error.code))
    }
    function publishLONGContentCallback001(error){
        console.log('ActsNotificationTest ACTS_PublishLONGContent_0100 asyncCallback'+JSON.stringify(error.code))
    }
    function publishLONGContentCallback002(error){
        console.log('ActsNotificationTest ACTS_PublishLONGContent_0200 asyncCallback'+JSON.stringify(error.code))
    }
    function publishLONGContentCallback003(error){
        console.log('ActsNotificationTest ACTS_PublishLONGContent_0300 asyncCallback'+JSON.stringify(error.code))
    }
    function publishSlotTypeContentCallback001(error){
        console.log('ActsNotificationTest ACTS_Publish_SlotTypeContent_0100 asyncCallback'+JSON.stringify(error.code))
    }
    function publishSlotTypeContentCallback002(error){
        console.log('ActsNotificationTest ACTS_Publish_SlotTypeContent_0200 asyncCallback'+JSON.stringify(error.code))
    }
    function publishSlotTypeContentCallback003(error){
        console.log('ActsNotificationTest ACTS_Publish_SlotTypeContent_0300 asyncCallback'+JSON.stringify(error.code))
    }
    function publishSlotTypeContentCallback004(error){
        console.log('ActsNotificationTest ACTS_Publish_SlotTypeContent_0400 asyncCallback'+JSON.stringify(error.code))
    }
    function publishSlotOtherCallback001(error){
        console.log('ActsNotificationTest ACTS_Publish_SlotTypeOther_0100 asyncCallback'+JSON.stringify(error.code))
    }
    function publishSlotOtherCallback002(error){
        console.log('ActsNotificationTest ACTS_Publish_SlotTypeOther_0200 asyncCallback'+JSON.stringify(error.code))
    }
    function publishSlotOtherCallback003(error){
        console.log('ActsNotificationTest ACTS_Publish_SlotTypeOther_0300 asyncCallback'+JSON.stringify(error.code))
    }
    function publishSlotOtherCallback004(error){
        console.log('ActsNotificationTest ACTS_Publish_SlotTypeOther_0400 asyncCallback'+JSON.stringify(error.code))
    }
    function publishSlotServiceCallback001(error){
        console.log('ActsNotificationTest ACTS_Publish_SlotTypeService_0100 asyncCallback'+JSON.stringify(error.code))
    }
    function publishSlotServiceCallback002(error){
        console.log('ActsNotificationTest ACTS_Publish_SlotTypeService_0200 asyncCallback'+JSON.stringify(error.code))
    }
    function publishSlotServiceCallback003(error){
        console.log('ActsNotificationTest ACTS_Publish_SlotTypeService_0300 asyncCallback'+JSON.stringify(error.code))
    }
    function publishSlotServiceCallback004(error){
        console.log('ActsNotificationTest ACTS_Publish_SlotTypeService_0400 asyncCallback'+JSON.stringify(error.code))
    }
    function publishSlotSocialCallback001(error){
        console.log('ActsNotificationTest ACTS_Publish_SlotTypeService_0400 asyncCallback'+JSON.stringify(error.code))
    }
    function publishSlotSocialCallback002(error){
        console.log('ActsNotificationTest ACTS_Publish_SlotTypeService_0400 asyncCallback'+JSON.stringify(error.code))
    }
    function publishSlotSocialCallback003(error){
        console.log('ActsNotificationTest ACTS_Publish_SlotTypeService_0400 asyncCallback'+JSON.stringify(error.code))
    }
    function publishSlotSocialCallback004(error){
        console.log('ActsNotificationTest ACTS_Publish_SlotTypeService_0400 asyncCallback'+JSON.stringify(error.code))
    }
    //consume
    function consumeCallback(err,data) {
        console.debug("==========================>consumeDoNotCallback1 data : =======================>" + err + JSON.stringify(data));
        comsumData = data
    }
    //subscribeOn
    function subscribeOnCallback(err) {
        console.debug("==========================>subscribeDoNotOnCallback=======================>");
    }
    //subscribe
    function subscribeCallback(err) {
        console.debug("==========================>subscribeDoNotCallback=======================>");
    }
    /*
     * @tc.number: ActsDoNotSubscriber_test_0100
     * @tc.name: subscribe()
     * @tc.desc: verify the function of subscribe
     */
    it('ActsSubscriber_test_0100', 0, async function (done) {
        console.debug("===============ActsDoNotSubscriber_test_0100======begin====================>");

        var subInfo ={
            onConsume:consumeCallback,
            onConnect:subscribeOnCallback,
        }
        try {
            await notification.subscribe(subInfo, subscribeCallback);
        }
        catch(err) {
            console.error('ActsSubscriber_test_0100  订阅 subscribeDoNotCallback err:'+err);
        }
        console.debug("===============ActsSubscriber_test_0100=======end3===================>");
        done();
        setTimeout(function(){
            console.debug("====>time out ActsSubscriber_test_0100====>");
        }, time);
    })

    /*
     * @tc.number: ACTS_PublishMULTILINEContent_0100
     * @tc.name: publish()
     * @tc.desc: verify the function of publish(contentType:NOTIFICATION_CONTENT_MULTILINE)
     */
    it('ACTS_PublishMULTILINEContent_0100', 0,async function (done) {
       try {
           await notification.publish({
               id: 1,
               content: {
                   contentType: notification.ContentType.NOTIFICATION_CONTENT_MULTILINE,
                   multiLine: {
                       title: "test_title",
                       text: "test_text",
                       additionalText: "test_additionalText",
                       briefText: "briefText",
                       longTitle: "longTitle",
                       lines: ["123", "234", "345", "456"]
                   },
               }
           }, publishMULTILINEContentCallback001);
       }catch(error){
        console.log('ActsNotificationTest ACTS_PublishMULTILINEContent_0100 error'+JSON.stringify(error.code))}
//        console.log("============ACTS_PublishMULTILINEContent_0100 finished============"+comsumData.request.id)
        done();
       setTimeout(function(){
           console.debug("====>time out ACTS_PublishMULTILINEContent_0100====>");
       }, time);
       expect(comsumData.request.id).assertEqual(1)

    })

    /*
     * @tc.number: ACTS_PublishMULTILINEContent_0200
     * @tc.name: publish()
     * @tc.desc: verify the function of publish()
     */
    it('ACTS_PublishMULTILINEContent_0200', 0,async function (done) {
       try {
           var notificationInfo = {
               id: 2,
               content: {
               //                contentType: notification.ContentType.NOTIFICATION_CONTENT_MULTILINE,
                   multiLine: {
                       title: "test_title",
                       text: "test_text",
                       additionalText: "test_additionalText",
                       briefText: "briefText",
                       longTitle: "longTitle",
                       lines: ["123", "234", "345", "456"]
                   },
               }
           }
     }catch(err){
          console.log('ActsNotificationTest ACTS_PublishMULTILINEContent11111111'+err)
        }
      try{
           console.log('ActsNotificationTest ACTS_PublishMULTILINEContent222222222222')
           await notification.publish(notificationInfo,publishMULTILINEContentCallback002);
       }catch(error){
           console.log('ActsNotificationTest ACTS_PublishMULTILINEContent_0200 asyncCallback'+error)
       }
//        console.log("============ACTS_PublishMULTILINEContent_0200 finished============"+comsumData.request.id)
        done();
       setTimeout(function(){
           console.debug("====>time out ACTS_PublishMULTILINEContent_0200====>");
       }, time);
       expect(comsumData.request.id).assertEqual(2)
    })

    /*
     * @tc.number: ACTS_PublishMULTILINEContent_0300
     * @tc.name: publish()
     * @tc.desc: verify the function of publish(contentType:NOTIFICATION_CONTENT_PICTURE)
     */
    it('ACTS_PublishMULTILINEContent_0300', 0,async function (done) {
       try {
           await notification.publish({
               id: 3,
               content: {
                   contentType: notification.ContentType.NOTIFICATION_CONTENT_PICTURE,
                   multiLine: {
                       title: "test_title",
                       text: "test_text",
                       additionalText: "test_additionalText",
                       briefText: "briefText",
                       longTitle: "longTitle",
                       lines: ["123", "234", "345", "456"]
                   },
               }
           }, publishMULTILINEContentCallback003);
       }catch(error){
           console.log('ActsNotificationTest ACTS_PublishMULTILINEContent_0300 asyncCallback'+JSON.stringify(error.code))
       }
       done();
       setTimeout(function(){
           console.debug("====>time out ACTS_PublishMULTILINEContent_0300====>");
       }, time);
       expect(comsumData.request.id).assertEqual(3)
    })

    /*
     * @tc.number: ACTS_PublishMULTILINEContent_0400
     * @tc.name: publish()
     * @tc.desc: verify the function of publish(contentType:NOTIFICATION_CONTENT_MULTILINE) promise
     */
    it('ACTS_PublishMULTILINEContent_0400', 0,async function (done) {
         notification.publish({
            id: 4,
            content: {
                contentType: notification.ContentType.NOTIFICATION_CONTENT_MULTILINE,
                multiLine: {
                    title: "test_title",
                    text: "test_text",
                    additionalText: "test_additionalText",
                    briefText:"briefText",
                    longTitle:"longTitle",
                    lines:["123","234","345","456"]
                },
            }
        }).then(console.log("============ACTS_PublishMULTILINEContent_0400 finished============"))
        done();
        setTimeout(function(){
            console.debug("====>time out ACTS_PublishMULTILINEContent_0400====>");
        }, time);
        expect(comsumData.request.id).assertEqual(4)
    })
    /*
     * @tc.number: ACTS_PublishMULTILINEContent_0500
     * @tc.name: publish()
     * @tc.desc: verify the function of publish() promise
     */
    it('ACTS_PublishMULTILINEContent_0500', 0,async function (done) {
         notification.publish({
            id: 5,
            content: {
            //                contentType: notification.ContentType.NOTIFICATION_CONTENT_MULTILINE,
                multiLine: {
                    title: "test_title",
                    text: "test_text",
                    additionalText: "test_additionalText",
                    briefText:"briefText",
                    longTitle:"longTitle",
                    lines:["123","234","345","456"]
                },
            }
        }).then(console.log("============ACTS_PublishMULTILINEContent_0500 finished============"))
        done();
        setTimeout(function(){
            console.debug("====>time out ACTS_PublishMULTILINEContent_0500====>");
        }, time);
        expect(comsumData.request.id).assertEqual(5)
    })

    /*
     * @tc.number: ACTS_PublishMULTILINEContent_0600
     * @tc.name: publish()
     * @tc.desc: verify the function of publish(contentType:NOTIFICATION_CONTENT_PICTURE) promise
     */
    it('ACTS_PublishMULTILINEContent_0600', 0,async function (done) {
         notification.publish({
            id: 6,
            content: {
                contentType: notification.ContentType.NOTIFICATION_CONTENT_PICTURE,
                multiLine: {
                    title: "test_title",
                    text: "test_text",
                    additionalText: "test_additionalText",
                    briefText:"briefText",
                    longTitle:"longTitle",
                    lines:["123","234","345","456"]
                },
            }
        }).then(console.log("============ACTS_PublishMULTILINEContent_0300 finished============"))
        done();
        setTimeout(function(){
            console.debug("====>time out ACTS_PublishMULTILINEContent_0600====>");
        }, time);
        expect(comsumData.request.id).assertEqual(6)
    })

    /*
     * @tc.number: ACTS_PublishLONGContent_0100
     * @tc.name: publish()
     * @tc.desc: verify the function of publish(contentType:NOTIFICATION_CONTENT_LONG_TEXT)
     */
    it('ACTS_PublishLONGContent_0100', 0,async function (done) {
        await notification.publish({
            id: 7,
            content: {
                contentType: notification.ContentType.NOTIFICATION_CONTENT_LONG_TEXT,
                longText: {
                    title: "test_title",
                    text: "test_text",
                    additionalText: "test_additionalText",
                    longText:"longText",
                    briefText:"briefText",
                    expandedTitle:"expandedTitle"
                }}
        },publishLONGContentCallback001);
        console.log("============ACTS_PublishLONGContent_0100 finished============")
        done();
        setTimeout(function(){
            console.debug("====>time out ACTS_PublishLONGContent_0100====>");
        }, time);
        expect(comsumData.request.id).assertEqual(7)
    })

    /*
     * @tc.number: ACTS_PublishLONGContent_0200
     * @tc.name: publish()
     * @tc.desc: verify the function of publish()
     */
    it('ACTS_PublishLONGContent_0200', 0,async function (done) {
        await notification.publish({
            id: 8,
            content: {
            //                contentType: notification.ContentType.NOTIFICATION_CONTENT_LONG_TEXT,
                longText: {
                    title: "test_title",
                    text: "test_text",
                    additionalText: "test_additionalText",
                    longText:"longText",
                    briefText:"briefText",
                    expandedTitle:"expandedTitle"
                }}
        },publishLONGContentCallback002);
        console.log("============ACTS_PublishLONGContent_0200 finished============")
        done();
        setTimeout(function(){
            console.debug("====>time out ACTS_PublishLONGContent_0200====>");
        }, time);
        expect(comsumData.request.id).assertEqual(8)
    })

    /*
     * @tc.number: ACTS_PublishLONGContent_0300
     * @tc.name: publish()
     * @tc.desc: verify the function of publish(contentType:NOTIFICATION_CONTENT_MEDIA)
     */
    it('ACTS_PublishLONGContent_0300', 0,async function (done) {
        await notification.publish({
            id: 9,
            content: {
                contentType: notification.ContentType.NOTIFICATION_CONTENT_MEDIA,
                longText: {
                    title: "test_title",
                    text: "test_text",
                    additionalText: "test_additionalText",
                    longText:"longText",
                    briefText:"briefText",
                    expandedTitle:"expandedTitle"
                }}
        },publishLONGContentCallback003);
        console.log("============ACTS_PublishLONGContent_0300 finished============")
        done();
        setTimeout(function(){
            console.debug("====>time out ACTS_PublishLONGContent_0300====>");
        }, time);
        expect(comsumData.request.id).assertEqual(9)
    })

    /*
     * @tc.number: ACTS_PublishLONGContent_0400
     * @tc.name: publish()
     * @tc.desc: verify the function of publish(contentType:NOTIFICATION_CONTENT_LONG_TEXT) promise
     */
    it('ACTS_PublishLONGContent_0400', 0,async function (done) {
         notification.publish({
            id: 10,
            content: {
                contentType: notification.ContentType.NOTIFICATION_CONTENT_LONG_TEXT,
                longText: {
                    title: "test_title",
                    text: "test_text",
                    additionalText: "test_additionalText",
                    longText:"longText",
                    briefText:"briefText",
                    expandedTitle:"expandedTitle"
                }}
        }).then(console.log("============ACTS_PublishLONGContent_0400 finished============"))
        done();
        setTimeout(function(){
            console.debug("====>time out ACTS_PublishLONGContent_0400====>");
        }, time);
        expect(comsumData.request.id).assertEqual(10)
    })

    /*
     * @tc.number: ACTS_PublishLONGContent_0500
     * @tc.name: publish()
     * @tc.desc: verify the function of publish() promise
     */
    it('ACTS_PublishLONGContent_0500', 0,async function (done) {
        notification.publish({
            id: 11,
            content: {
            //                contentType: notification.ContentType.NOTIFICATION_CONTENT_LONG_TEXT,
                longText: {
                    title: "test_title",
                    text: "test_text",
                    additionalText: "test_additionalText",
                    longText:"longText",
                    briefText:"briefText",
                    expandedTitle:"expandedTitle"
                }}
        }).then(console.log("============ACTS_PublishLONGContent_0500 finished============"))
        done();
        setTimeout(function(){
            console.debug("====>time out ACTS_PublishLONGContent_0500====>");
        }, time);
        expect(comsumData.request.id).assertEqual(11)
    })

    /*
     * @tc.number: ACTS_PublishLONGContent_0600
     * @tc.name: publish()
     * @tc.desc: verify the function of publish(contentType:NOTIFICATION_CONTENT_MEDIA) promise
     */
    it('ACTS_PublishLONGContent_0600', 0,async function (done) {
        notification.publish({
            id: 12,
            content: {
                contentType: notification.ContentType.NOTIFICATION_CONTENT_MEDIA,
                longText: {
                    title: "test_title",
                    text: "test_text",
                    additionalText: "test_additionalText",
                    longText:"longText",
                    briefText:"briefText",
                    expandedTitle:"expandedTitle"
                }}
        }).then(console.log("============ACTS_PublishLONGContent_0600 finished============"))
        done();
        setTimeout(function(){
            console.debug("====>time out ACTS_PublishLONGContent_0600====>");
        }, time);
        expect(comsumData.request.id).assertEqual(12)
    })

    /*
     * @tc.number: ACTS_Publish_SlotTypeContent_0100
     * @tc.name: publish()
     * @tc.desc: verify the function of publish(contentType:NOTIFICATION_CONTENT_BASIC_TEXT,slotType:CONTENT_INFORMATION)
     */
    it('ACTS_Publish_SlotTypeContent_0100', 0,async function (done) {
        await notification.publish({
            id: 13,
            content: {
                contentType: notification.ContentType.NOTIFICATION_CONTENT_BASIC_TEXT,
                normal: {
                    title: "test_title",
                    text: "test_text",
                    additionalText: "test_additionalText"
                }
            },
            slotType:notification.SlotType.CONTENT_INFORMATION
        },publishSlotTypeContentCallback001);
        console.log("============ACTS_Publish_SlotTypeContent_0100 finished============")
        done();
        setTimeout(function(){
            console.debug("====>time out ACTS_Publish_SlotTypeContent_0100====>");
        }, time);
        expect(comsumData.request.id).assertEqual(13)
    })

    /*
     * @tc.number: ACTS_Publish_SlotTypeContent_0200
     * @tc.name: publish()
     * @tc.desc: verify the function of publish(slotType:CONTENT_INFORMATION)
     */
    it('ACTS_Publish_SlotTypeContent_0200', 0,async function (done) {
        await notification.publish({
            id: 14,
            content: {
            //                contentType: notification.ContentType.NOTIFICATION_CONTENT_BASIC_TEXT,
                normal: {
                    title: "test_title",
                    text: "test_text",
                    additionalText: "test_additionalText"
                }
            },
            slotType:notification.SlotType.CONTENT_INFORMATION
        },publishSlotTypeContentCallback002);
        console.log("============ACTS_Publish_SlotTypeContent_0200 finished============")
        done();
        setTimeout(function(){
            console.debug("====>time out ACTS_Publish_SlotTypeContent_0200====>");
        }, time);
        expect(comsumData.request.id).assertEqual(14)
    })

    /*
     * @tc.number: ACTS_Publish_SlotTypeContent_0300
     * @tc.name: publish()
     * @tc.desc: verify the function of publish(contentType:NOTIFICATION_CONTENT_BASIC_TEXT)
     */
    it('ACTS_Publish_SlotTypeContent_0300', 0,async function (done) {
        await notification.publish({
            id: 15,
            content: {
                contentType: notification.ContentType.NOTIFICATION_CONTENT_BASIC_TEXT,
                normal: {
                    title: "test_title",
                    text: "test_text",
                    additionalText: "test_additionalText"
                }
            },
        //            slotType:notification.SlotType.CONTENT_INFORMATION
        },publishSlotTypeContentCallback003);
        console.log("============ACTS_Publish_SlotTypeContent_0300 finished============")
        done();
        setTimeout(function(){
            console.debug("====>time out ACTS_Publish_SlotTypeContent_0300====>");
        }, time);
        expect(comsumData.request.id).assertEqual(15)
    })

    /*
     * @tc.number: ACTS_Publish_SlotTypeContent_0400
     * @tc.name: publish()
     * @tc.desc: verify the function of publish(contentType:NOTIFICATION_CONTENT_BASIC_TEXT,slotType:SERVICE_INFORMATION)
     */
    it('ACTS_Publish_SlotTypeContent_0400', 0,async function (done) {
        await notification.publish({
            id: 16,
            content: {
                contentType: notification.ContentType.NOTIFICATION_CONTENT_BASIC_TEXT,
                normal: {
                    title: "test_title",
                    text: "test_text",
                    additionalText: "test_additionalText"
                }
            },
            slotType:notification.SlotType.SERVICE_INFORMATION
        },publishSlotTypeContentCallback004);
        console.log("============ACTS_Publish_SlotTypeContent_0400 finished============")
        done();
        setTimeout(function(){
            console.debug("====>time out ACTS_Publish_SlotTypeContent_0400====>");
        }, time);
        expect(comsumData.request.id).assertEqual(16)
    })

    /*
     * @tc.number: ACTS_Publish_SlotTypeContent_0500
     * @tc.name: publish()
     * @tc.desc: verify the function of publish(contentType:NOTIFICATION_CONTENT_BASIC_TEXT,slotType:CONTENT_INFORMATION)  promise
     */
    it('ACTS_Publish_SlotTypeContent_0500', 0,async function (done) {
        notification.publish({
            id: 17,
            content: {
                contentType: notification.ContentType.NOTIFICATION_CONTENT_BASIC_TEXT,
                normal: {
                    title: "test_title",
                    text: "test_text",
                    additionalText: "test_additionalText"
                }
            },
            slotType:notification.SlotType.CONTENT_INFORMATION
        }).then( console.log("============ACTS_Publish_SlotTypeContent_0500 finished============"))
        done();
        setTimeout(function(){
            console.debug("====>time out ACTS_Publish_SlotTypeContent_0500====>");
        }, time);
        expect(comsumData.request.id).assertEqual(17)
    })

    /*
     * @tc.number: ACTS_Publish_SlotTypeContent_0600
     * @tc.name: publish()
     * @tc.desc: verify the function of publish(slotType:CONTENT_INFORMATION)  promise
     */
    it('ACTS_Publish_SlotTypeContent_0600', 0,async function (done) {
        notification.publish({
            id: 18,
            content: {
            //                contentType: notification.ContentType.NOTIFICATION_CONTENT_BASIC_TEXT,
                normal: {
                    title: "test_title",
                    text: "test_text",
                    additionalText: "test_additionalText"
                }
            },
            slotType:notification.SlotType.CONTENT_INFORMATION
        }).then(console.log("============ACTS_Publish_SlotTypeContent_0600 finished============"))
        done();
        setTimeout(function(){
            console.debug("====>time out ACTS_Publish_SlotTypeContent_0600====>");
        }, time);
        expect(comsumData.request.id).assertEqual(18)
    })

    /*
     * @tc.number: ACTS_Publish_SlotTypeContent_0700
     * @tc.name: publish()
     * @tc.desc: verify the function of publish(contentType:NOTIFICATION_CONTENT_BASIC_TEXT)  promise
     */
    it('ACTS_Publish_SlotTypeContent_0700', 0,async function (done) {
        notification.publish({
            id: 19,
            content: {
                contentType: notification.ContentType.NOTIFICATION_CONTENT_BASIC_TEXT,
                normal: {
                    title: "test_title",
                    text: "test_text",
                    additionalText: "test_additionalText"
                }
            },
        //            slotType:notification.SlotType.CONTENT_INFORMATION
        }).then(console.log("============ACTS_Publish_SlotTypeContent_0700 finished============"))
        done();
        setTimeout(function(){
            console.debug("====>time out ACTS_Publish_SlotTypeContent_0700====>");
        }, time);
        expect(comsumData.request.id).assertEqual(19)
    })

    /*
     * @tc.number: ACTS_Publish_SlotTypeContent_0800
     * @tc.name: publish()
     * @tc.desc: verify the function of publish(contentType:NOTIFICATION_CONTENT_BASIC_TEXT,slotType:SERVICE_INFORMATION) promise
     */
    it('ACTS_Publish_SlotTypeContent_0800', 0,async function (done) {
        notification.publish({
            id: 20,
            content: {
                contentType: notification.ContentType.NOTIFICATION_CONTENT_BASIC_TEXT,
                normal: {
                    title: "test_title",
                    text: "test_text",
                    additionalText: "test_additionalText"
                }
            },
            slotType:notification.SlotType.SERVICE_INFORMATION
        }).then(console.log("============ACTS_Publish_SlotTypeContent_0800 finished============"))
        done();
        setTimeout(function(){
            console.debug("====>time out ACTS_Publish_SlotTypeContent_0800====>");
        }, time);
        expect(comsumData.request.id).assertEqual(20)
    })

    /*
     * @tc.number: ACTS_PublishSlotTypeOther_0100
     * @tc.name: publish()
     * @tc.desc: verify the function of publish(contentType:NOTIFICATION_CONTENT_BASIC_TEXT,slotType:OTHER_TYPES)
     */
    it('ACTS_PublishSlotTypeOther_0100', 0,async function (done) {
        await notification.publish({
            id:21,
            content: {
                contentType: notification.ContentType.NOTIFICATION_CONTENT_BASIC_TEXT,
                normal: {
                    title: "test_title",
                    text: "test_text",
                    additionalText: "test_additionalText"
                },
                slotType:notification.SlotType.OTHER_TYPES
            }
        },publishSlotOtherCallback001);
        console.log("============ACTS_PublishSlotTypeOther_0100 finished============")
        done();
        setTimeout(function(){
            console.debug("====>time out ACTS_PublishSlotTypeOther_0100====>");
        }, time);
        expect(comsumData.request.id).assertEqual(21)
    })

    /*
     * @tc.number: ACTS_PublishSlotTypeOther_0200
     * @tc.name: publish()
     * @tc.desc: verify the function of publish(slotType:OTHER_TYPES)
     */
    it('ACTS_PublishSlotTypeOther_0200', 0,async function (done) {
        await notification.publish({
            id:22,
            content: {
            //                contentType: notification.ContentType.NOTIFICATION_CONTENT_BASIC_TEXT,
                normal: {
                    title: "test_title",
                    text: "test_text",
                    additionalText: "test_additionalText"
                },
                slotType:notification.SlotType.OTHER_TYPES
            }
        },publishSlotOtherCallback002);
        console.log("============ACTS_PublishSlotTypeOther_0200 finished============")
        done();
        setTimeout(function(){
            console.debug("====>time out ACTS_PublishSlotTypeOther_0200====>");
        }, time);
        expect(comsumData.request.id).assertEqual(22)
    })

    /*
     * @tc.number: ACTS_PublishSlotTypeOther_0300
     * @tc.name: publish()
     * @tc.desc: verify the function of publish(contentType:NOTIFICATION_CONTENT_BASIC_TEXT)
     */
    it('ACTS_PublishSlotTypeOther_0300', 0,async function (done) {
        await notification.publish({
            id:23,
            content: {
                contentType: notification.ContentType.NOTIFICATION_CONTENT_BASIC_TEXT,
                normal: {
                    title: "test_title",
                    text: "test_text",
                    additionalText: "test_additionalText"
                },
            //                slotType:notification.SlotType.OTHER_TYPES
            }
        },publishSlotOtherCallback003);
        console.log("============ACTS_PublishSlotTypeOther_0300 finished============")
        done();
        setTimeout(function(){
            console.debug("====>time out ACTS_PublishSlotTypeOther_0300====>");
        }, time);
        expect(comsumData.request.id).assertEqual(23)
    })

    /*
     * @tc.number: ACTS_PublishSlotTypeOther_0400
     * @tc.name: publish()
     * @tc.desc: verify the function of publish(contentType:NOTIFICATION_CONTENT_BASIC_TEXT,slotType:SOCIAL_COMMUNICATION)
     */
    it('ACTS_PublishSlotTypeOther_0400', 0,async function (done) {
        await notification.publish({
            id:24,
            content: {
                contentType: notification.ContentType.NOTIFICATION_CONTENT_BASIC_TEXT,
                normal: {
                    title: "test_title",
                    text: "test_text",
                    additionalText: "test_additionalText"
                },
                slotType:notification.SlotType.SOCIAL_COMMUNICATION
            }
        },publishSlotOtherCallback004);
        console.log("============ACTS_PublishSlotTypeOther_0400 finished============")
        done();
        setTimeout(function(){
            console.debug("====>time out ACTS_PublishSlotTypeOther_0400====>");
        }, time);
        expect(comsumData.request.id).assertEqual(24)
    })

    /*
     * @tc.number: ACTS_PublishSlotTypeOther_0500
     * @tc.name: publish()
     * @tc.desc: verify the function of publish(contentType:NOTIFICATION_CONTENT_BASIC_TEXT,slotType:OTHER_TYPES)  promise
     */
    it('ACTS_PublishSlotTypeOther_0500', 0,async function (done) {
        notification.publish({
            id:25,
            content: {
                contentType: notification.ContentType.NOTIFICATION_CONTENT_BASIC_TEXT,
                normal: {
                    title: "test_title",
                    text: "test_text",
                    additionalText: "test_additionalText"
                },
                slotType:notification.SlotType.OTHER_TYPES
            }
        }).then(console.log("============ACTS_PublishSlotTypeOther_0500 finished============"))
        done();
        setTimeout(function(){
            console.debug("====>time out ACTS_PublishSlotTypeOther_0500====>");
        }, time);
        expect(comsumData.request.id).assertEqual(25)
    })

    /*
     * @tc.number: ACTS_PublishSlotTypeOther_0600
     * @tc.name: publish()
     * @tc.desc: verify the function of publish(slotType:OTHER_TYPES)  promise
     */
    it('ACTS_PublishSlotTypeOther_0600', 0,async function (done) {
        notification.publish({
            id:26,
            content: {
            //                contentType: notification.ContentType.NOTIFICATION_CONTENT_BASIC_TEXT,
                normal: {
                    title: "test_title",
                    text: "test_text",
                    additionalText: "test_additionalText"
                },
                slotType:notification.SlotType.OTHER_TYPES
            }
        }).then(console.log("============ACTS_PublishSlotTypeOther_0600 finished============"))
        done();
        setTimeout(function(){
            console.debug("====>time out ACTS_PublishSlotTypeOther_0600====>");
        }, time);
        expect(comsumData.request.id).assertEqual(26)
    })

    /*
     * @tc.number: ACTS_PublishSlotTypeOther_0700
     * @tc.name: publish()
     * @tc.desc: verify the function of publish(contentType:NOTIFICATION_CONTENT_BASIC_TEXT)  promise
     */
    it('ACTS_PublishSlotTypeOther_0700', 0,async function (done) {
        notification.publish({
            id:27,
            content: {
                contentType: notification.ContentType.NOTIFICATION_CONTENT_BASIC_TEXT,
                normal: {
                    title: "test_title",
                    text: "test_text",
                    additionalText: "test_additionalText"
                },
            //                slotType:notification.SlotType.OTHER_TYPES
            }
        }).then(console.log("============ACTS_PublishSlotTypeOther_0700 finished============"))
        done();
        setTimeout(function(){
            console.debug("====>time out ACTS_PublishSlotTypeOther_0700====>");
        }, time);
        expect(comsumData.request.id).assertEqual(27)
    })

    /*
     * @tc.number: ACTS_PublishSlotTypeOther_0500
     * @tc.name: publish()
     * @tc.desc: verify the function of publish(contentType:NOTIFICATION_CONTENT_BASIC_TEXT,slotType:SOCIAL_COMMUNICATION)  promise
     */
    it('ACTS_PublishSlotTypeOther_0800', 0,async function (done) {
        notification.publish({
            id:28,
            content: {
                contentType: notification.ContentType.NOTIFICATION_CONTENT_BASIC_TEXT,
                normal: {
                    title: "test_title",
                    text: "test_text",
                    additionalText: "test_additionalText"
                },
                slotType:notification.SlotType.SOCIAL_COMMUNICATION
            }
        }).then(console.log("============ACTS_PublishSlotTypeOther_0800 finished============"))
        done();
        setTimeout(function(){
            console.debug("====>time out ACTS_PublishSlotTypeOther_0800====>");
        }, time);
        expect(comsumData.request.id).assertEqual(28)
    })

    /*
     * @tc.number: ACTS_PublishSlotTypeService_0100
     * @tc.name: publish()
     * @tc.desc: verify the function of publish(contentType:NOTIFICATION_CONTENT_BASIC_TEXT,slotType:SERVICE_INFORMATION)
     */
    it('ACTS_PublishSlotTypeService_0100', 0,async function (done) {
        await notification.publish({
            id:29,
            content: {
                contentType: notification.ContentType.NOTIFICATION_CONTENT_BASIC_TEXT,
                normal: {
                    title: "test_title",
                    text: "test_text",
                    additionalText: "test_additionalText"
                },
            },
            slotType:notification.SlotType.SERVICE_INFORMATION
        },publishSlotServiceCallback001);
        console.log("============ACTS_PublishSlotTypeService_0100 finished============")
        done();
        setTimeout(function(){
            console.debug("====>time out ACTS_PublishSlotTypeService_0100====>");
        }, time);
        expect(comsumData.request.id).assertEqual(29)
    })

    /*
     * @tc.number: ACTS_PublishSlotTypeService_0200
     * @tc.name: publish()
     * @tc.desc: verify the function of publish(slotType:SERVICE_INFORMATION)
     */
    it('ACTS_PublishSlotTypeService_0200', 0,async function (done) {
        await notification.publish({
            id: 30,
            content: {
            //                contentType: notification.ContentType.NOTIFICATION_CONTENT_BASIC_TEXT,
                normal: {
                    title: "test_title",
                    text: "test_text",
                    additionalText: "test_additionalText"
                },
            },
            slotType:notification.SlotType.SERVICE_INFORMATION
        },publishSlotServiceCallback002);
        console.log("============ACTS_PublishSlotTypeService_0200 finished============")
        done();
        setTimeout(function(){
            console.debug("====>time out ACTS_PublishSlotTypeService_0200====>");
        }, time);
        expect(comsumData.request.id).assertEqual(30)
    })

    /*
     * @tc.number: ACTS_PublishSlotTypeService_0300
     * @tc.name: publish()
     * @tc.desc: verify the function of publish(contentType:NOTIFICATION_CONTENT_BASIC_TEXT)
     */
    it('ACTS_PublishSlotTypeService_0300', 0,async function (done) {
        await notification.publish({
            id: 31,
            content: {
                contentType: notification.ContentType.NOTIFICATION_CONTENT_BASIC_TEXT,
                normal: {
                    title: "test_title",
                    text: "test_text",
                    additionalText: "test_additionalText"
                },
            },
        //            slotType:notification.SlotType.SERVICE_INFORMATION
        },publishSlotServiceCallback003);
        console.log("============ACTS_PublishSlotTypeService_0300 finished============")
        done();
        setTimeout(function(){
            console.debug("====>time out ACTS_PublishSlotTypeService_0300====>");
        }, time);
        expect(comsumData.request.id).assertEqual(31)
    })

    /*
     * @tc.number: ACTS_PublishSlotTypeService_0400
     * @tc.name: publish()
     * @tc.desc: verify the function of publish(contentType:NOTIFICATION_CONTENT_BASIC_TEXT,slotType:UNKNOWN_TYPE)
     */
    it('ACTS_PublishSlotTypeService_0400', 0,async function (done) {
        await notification.publish({
            id: 32,
            content: {
                contentType: notification.ContentType.NOTIFICATION_CONTENT_BASIC_TEXT,
                normal: {
                    title: "test_title",
                    text: "test_text",
                    additionalText: "test_additionalText"
                },
            },
            slotType:notification.SlotType.UNKNOWN_TYPE
        },publishSlotServiceCallback004);
        console.log("============ACTS_PublishSlotTypeService_0400 finished============")
        done();
        setTimeout(function(){
            console.debug("====>time out ACTS_PublishSlotTypeService_0400====>");
        }, time);
        expect(comsumData.request.id).assertEqual(32)
    })

    /*
     * @tc.number: ACTS_PublishSlotTypeService_0500
     * @tc.name: publish()
     * @tc.desc: verify the function of publish(contentType:NOTIFICATION_CONTENT_BASIC_TEXT,slotType:SERVICE_INFORMATION) promise
     */
    it('ACTS_PublishSlotTypeService_0500', 0,async function (done) {
        notification.publish({
            id: 33,
            content: {
                contentType: notification.ContentType.NOTIFICATION_CONTENT_BASIC_TEXT,
                normal: {
                    title: "test_title",
                    text: "test_text",
                    additionalText: "test_additionalText"
                },
            },
            slotType:notification.SlotType.SERVICE_INFORMATION
        }).then(console.log("============ACTS_PublishSlotTypeService_0500 finished============"))
        done();
        setTimeout(function(){
            console.debug("====>time out ACTS_PublishSlotTypeService_0500====>");
        }, time);
        expect(comsumData.request.id).assertEqual(33)
    })

    /*
     * @tc.number: ACTS_PublishSlotTypeService_0600
     * @tc.name: publish()
     * @tc.desc: verify the function of publish(slotType:SERVICE_INFORMATION) promise
     */
    it('ACTS_PublishSlotTypeService_0600', 0,async function (done) {
        notification.publish({
            id: 34,
            content: {
            //                contentType: notification.ContentType.NOTIFICATION_CONTENT_BASIC_TEXT,
                normal: {
                    title: "test_title",
                    text: "test_text",
                    additionalText: "test_additionalText"
                },
            },
            slotType:notification.SlotType.SERVICE_INFORMATION
        }).then(console.log("============ACTS_PublishSlotTypeService_0600 finished============"))
        done();
        setTimeout(function(){
            console.debug("====>time out ACTS_PublishSlotTypeService_0600====>");
        }, time);
        expect(comsumData.request.id).assertEqual(34)
    })

    /*
     * @tc.number: ACTS_PublishSlotTypeService_0700
     * @tc.name: publish()
     * @tc.desc: verify the function of publish(contentType:NOTIFICATION_CONTENT_BASIC_TEXT) promise
     */
    it('ACTS_PublishSlotTypeService_0700', 0,async function (done) {
        notification.publish({
            id: 35,
            content: {
                contentType: notification.ContentType.NOTIFICATION_CONTENT_BASIC_TEXT,
                normal: {
                    title: "test_title",
                    text: "test_text",
                    additionalText: "test_additionalText"
                },
            },
        //            slotType:notification.SlotType.SERVICE_INFORMATION
        }).then(console.log("============ACTS_PublishSlotTypeService_0700 finished============"))
        done();
        setTimeout(function(){
            console.debug("====>time out ACTS_PublishSlotTypeService_0700====>");
        }, time);
        expect(comsumData.request.id).assertEqual(35)
    })

    /*
     * @tc.number: ACTS_PublishSlotTypeService_0800
     * @tc.name: publish()
     * @tc.desc: verify the function of publish(contentType:NOTIFICATION_CONTENT_BASIC_TEXT,slotType:UNKNOWN_TYPE) promise
     */
    it('ACTS_PublishSlotTypeService_0800', 0,async function (done) {
        notification.publish({
            id: 36,
            content: {
                contentType: notification.ContentType.NOTIFICATION_CONTENT_BASIC_TEXT,
                normal: {
                    title: "test_title",
                    text: "test_text",
                    additionalText: "test_additionalText"
                },
            },
            slotType:notification.SlotType.UNKNOWN_TYPE
        }).then(console.log("============ACTS_PublishSlotTypeService_0800 finished============"))
        done();
        setTimeout(function(){
            console.debug("====>time out ACTS_PublishSlotTypeService_0800====>");
        }, time);
        expect(comsumData.request.id).assertEqual(36)
    })

    /*
     * @tc.number: ACTS_PublishSlotTypeSocial_0100
     * @tc.name: publish()
     * @tc.desc: verify the function of publish(contentType:NOTIFICATION_CONTENT_BASIC_TEXT,slotType:SOCIAL_COMMUNICATION)
     */
    it('ACTS_PublishSlotTypeSocial_0100', 0,async function (done) {
        await notification.publish({
            id: 37,
            content: {
                contentType: notification.ContentType.NOTIFICATION_CONTENT_BASIC_TEXT,
                normal: {
                    title: "test_title",
                    text: "test_text",
                    additionalText: "test_additionalText"
                },
                slotType:notification.SlotType.SOCIAL_COMMUNICATION
            }
        },publishSlotSocialCallback001);
        console.log("============ACTS_PublishSlotTypeSocial_0600 finished============")
        done();
        setTimeout(function(){
            console.debug("====>time out ACTS_PublishSlotTypeSocial_0100====>");
        }, time);
        expect(comsumData.request.id).assertEqual(37)
    })

    /*
     * @tc.number: ACTS_PublishSlotTypeSocial_0200
     * @tc.name: publish()
     * @tc.desc: verify the function of publish(slotType:SOCIAL_COMMUNICATION)
     */
    it('ACTS_PublishSlotTypeSocial_0200', 0,async function (done) {
        await notification.publish({
            id: 38,
            content: {
            //                contentType: notification.ContentType.NOTIFICATION_CONTENT_BASIC_TEXT,
                normal: {
                    title: "test_title",
                    text: "test_text",
                    additionalText: "test_additionalText"
                },
                slotType:notification.SlotType.SOCIAL_COMMUNICATION
            }
        },publishSlotSocialCallback002);
        console.log("============ACTS_PublishSlotTypeSocial_0600 finished============")
        done();
        setTimeout(function(){
            console.debug("====>time out ACTS_PublishSlotTypeSocial_0200====>");
        }, time);
        expect(comsumData.request.id).assertEqual(38)
    })

    /*
     * @tc.number: ACTS_PublishSlotTypeSocial_0300
     * @tc.name: publish()
     * @tc.desc: verify the function of publish(contentType:NOTIFICATION_CONTENT_BASIC_TEXT)
     */
    it('ACTS_PublishSlotTypeSocial_0300', 0,async function (done) {
        await notification.publish({
            id: 39,
            content: {
                contentType: notification.ContentType.NOTIFICATION_CONTENT_BASIC_TEXT,
                normal: {
                    title: "test_title",
                    text: "test_text",
                    additionalText: "test_additionalText"
                },
            //                slotType:notification.SlotType.SOCIAL_COMMUNICATION
            }
        },publishSlotSocialCallback003);
        console.log("============ACTS_PublishSlotTypeSocial_0300 finished============")
        done();
        setTimeout(function(){
            console.debug("====>time out ACTS_PublishSlotTypeSocial_0300====>");
        }, time);
        expect(comsumData.request.id).assertEqual(39)
    })

    /*
     * @tc.number: ACTS_PublishSlotTypeSocial_0400
     * @tc.name: publish()
     * @tc.desc: verify the function of publish(contentType:NOTIFICATION_CONTENT_BASIC_TEXT,slotType:CONTENT_INFORMATION)
     */
    it('ACTS_PublishSlotTypeSocial_0400', 0,async function (done) {
        await notification.publish({
            id: 40,
            content: {
                contentType: notification.ContentType.NOTIFICATION_CONTENT_BASIC_TEXT,
                normal: {
                    title: "test_title",
                    text: "test_text",
                    additionalText: "test_additionalText"
                },
                slotType:notification.SlotType.CONTENT_INFORMATION
            }
        },publishSlotSocialCallback004);
        console.log("============ACTS_PublishSlotTypeSocial_0400 finished============")
        done();
        setTimeout(function(){
            console.debug("====>time out ACTS_PublishSlotTypeSocial_0400====>");
        }, time);
        expect(comsumData.request.id).assertEqual(40)
    })

    /*
     * @tc.number: ACTS_PublishSlotTypeSocial_0500
     * @tc.name: publish()
     * @tc.desc: verify the function of publish(contentType:NOTIFICATION_CONTENT_BASIC_TEXT,slotType:SOCIAL_COMMUNICATION) promise
     */
    it('ACTS_PublishSlotTypeSocial_0500', 0,async function (done) {
        notification.publish({
            id: 41,
            content: {
                contentType: notification.ContentType.NOTIFICATION_CONTENT_BASIC_TEXT,
                normal: {
                    title: "test_title",
                    text: "test_text",
                    additionalText: "test_additionalText"
                },
                slotType:notification.SlotType.SOCIAL_COMMUNICATION
            }
        }).then(console.log("============ACTS_PublishSlotTypeSocial_0500 finished============"))
        done();
        setTimeout(function(){
            console.debug("====>time out ACTS_PublishSlotTypeSocial_0500====>");
        }, time);
        expect(comsumData.request.id).assertEqual(41)
    })

    /*
     * @tc.number: ACTS_PublishSlotTypeSocial_0600
     * @tc.name: publish()
     * @tc.desc: verify the function of publish(slotType:SOCIAL_COMMUNICATION) promise
     */
    it('ACTS_PublishSlotTypeSocial_0600', 0,async function (done) {
        notification.publish({
            id: 42,
            content: {
            //                contentType: notification.ContentType.NOTIFICATION_CONTENT_BASIC_TEXT,
                normal: {
                    title: "test_title",
                    text: "test_text",
                    additionalText: "test_additionalText"
                },
                slotType:notification.SlotType.SOCIAL_COMMUNICATION
            }
        }).then(console.log("============ACTS_PublishSlotTypeSocial_0600 finished============"))
        done();
        setTimeout(function(){
            console.debug("====>time out ACTS_PublishSlotTypeSocial_0600====>");
        }, time);
        expect(comsumData.request.id).assertEqual(42)
    })

    /*
     * @tc.number: ACTS_PublishSlotTypeSocial_0700
     * @tc.name: publish()
     * @tc.desc: verify the function of publish(contentType:NOTIFICATION_CONTENT_BASIC_TEXT) promise
     */
    it('ACTS_PublishSlotTypeSocial_0700', 0,async function (done) {
        notification.publish({
            id: 43,
            content: {
                contentType: notification.ContentType.NOTIFICATION_CONTENT_BASIC_TEXT,
                normal: {
                    title: "test_title",
                    text: "test_text",
                    additionalText: "test_additionalText"
                },
            //                slotType:notification.SlotType.SOCIAL_COMMUNICATION
            }
        }).then(console.log("============ACTS_PublishSlotTypeSocial_0700 finished============"),
        )
        done()
        setTimeout(function(){
            console.debug("====>time out ACTS_PublishSlotTypeSocial_0700====>");
        }, time);
        expect(comsumData.request.id).assertEqual(43)
    })

    /*
     * @tc.number: ACTS_PublishSlotTypeSocial_0800
     * @tc.name: publish()
     * @tc.desc: verify the function of publish(contentType:NOTIFICATION_CONTENT_BASIC_TEXT,slotType:CONTENT_INFORMATION) promise
     */
    it('ACTS_PublishSlotTypeSocial_0800', 0,async function (done) {
        notification.publish({
            id: 44,
            content: {
                contentType: notification.ContentType.NOTIFICATION_CONTENT_BASIC_TEXT,
                normal: {
                    title: "test_title",
                    text: "test_text",
                    additionalText: "test_additionalText"
                },
                slotType:notification.SlotType.CONTENT_INFORMATION
            }
        }).then(console.log("============ACTS_PublishSlotTypeSocial_0800 finished============"))
        done();
        setTimeout(function(){
            console.debug("====>time out ACTS_PublishSlotTypeSocial_0800====>");
        }, time);
        expect(comsumData.request.id).assertEqual(44)
    })
})