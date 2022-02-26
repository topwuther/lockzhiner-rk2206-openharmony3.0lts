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
describe('ActsAnsGetCancelActiveNumTest', function () {
    console.info("===========ActsGetCancelActiveNumTest start====================>");

    //consume
    function consumeCallback(err, data) {
        console.debug("==========================>consumeCallback data : =======================>" + JSON.stringify(data));
        console.debug("==========================>consumeCallback===notificationId:====================>" + data.request.id);
        console.debug("==========================>consumeCallback===sortingMap.sortedHashCode.length:====================>" + data.sortingMap.sortedHashCode.length);
    }

    function cancelAllCallBack(){
        console.info("==========================>cancelAllCallBack start=======================>");
    }

    //subscribeOn
    function subscribeOnCallback(err) {
        console.debug("==========================>subscribeOnCallback=======================>");
    }
    //subscribe
    function subscribeCallback(err) {
        console.debug("==========================>subscribeCallback=======================>");

    }
    //publish
    function publishCallback001(){
        console.log('ActsAnsGetCancelActiveNumTest ActsGetCancelActiveNum_test_0200  publish asyncCallback')
    }

    /*
    * @tc.number: ActsGetCancelActiveNum_test_0100
    * @tc.name: createSubscriber(),subscriber()
    * @tc.desc: verify the function of createSubscriber,subscriber
    */
    it('ActsGetCancelActiveNum_test_0100', 0, async function (done) {
        console.debug("===============ActsGetCancelActiveNum_test_0100======begin====================>");
        var subInfo ={
            onConsume:consumeCallback,
            onConnecte:subscribeOnCallback,
        }
        try{
            await notify.subscribe(subInfo,subscribeCallback);
        }catch(err) {
            console.error('=ActsWantAgent_test_0100  订阅 subscribeCallback err:'+err);
        }
        console.debug("===============ActsGetCancelActiveNum_test_0100=======end===================>");
        done();
        setTimeout(function(){
            console.debug("====>time out ActsGetCancelActiveNum_test_0100====>");
        }, time);
    })

    /*
    * @tc.number: ActsGetCancelActiveNum_test_0200
    * @tc.name: getActiveNotifications(),getActiveNotificationNums()
    * @tc.desc: verify the function of getActiveNotifications,getActiveNotificationNums
    */
    it('ActsGetCancelActiveNum_test_0200', 0,async function (done) {
        await notify.publish({
            id: 1,
            content: {
                contentType: notify.ContentType.NOTIFICATION_CONTENT_BASIC_TEXT,
                normal: {
                    title: "test1_title",
                    text: "test1_text",
                    additionalText: "test1_additionalText"
                },
            }
        },publishCallback001);
        await notify.publish({
            id: 2,
            content: {
                contentType: notify.ContentType.NOTIFICATION_CONTENT_BASIC_TEXT,
                normal: {
                    title: "test2_title",
                    text: "test2_text",
                    additionalText: "test2_additionalText"
                },
            }
        },publishCallback001);
        await notify.publish({
            id: 3,
            content: {
                contentType: notify.ContentType.NOTIFICATION_CONTENT_BASIC_TEXT,
                normal: {
                    title: "test3_title",
                    text: "test3_text",
                    additionalText: "test3_additionalText"
                },
            }
        },publishCallback001);

        await notify.getActiveNotifications(
            (error,data) => {
                console.log("============PublishGetActiveNotificationsTest1============"+data)
                expect(typeof(data)).assertEqual('object')
                for (let i = 0; i < data.length; i++) {
                    console.log("==========================>PublishGetActiveNotificationsTest1=======================>" + data[i].hashCode)
                    console.log("==========================>PublishGetActiveNotificationsTest1=======================>" + data[i].Id)
                    console.log("==========================>PublishGetActiveNotificationsTest1=======================>" + data[i].slotType)
                    console.log("==========================>PublishGetActiveNotificationsTest1=======================>" + data[i].content.contentType)
                    console.log("==========================>PublishGetActiveNotificationsTest1=======================>" + data[i].content.normal.title)
                    console.log("==========================>PublishGetActiveNotificationsTest1=======================>" + data[i].content.normal.text)
                    console.log("==========================>PublishGetActiveNotificationsTest1=======================>" + data[i].content.normal.additionalText)
                }
            })

        var promise = await notify.getActiveNotifications();
        expect(typeof(promise)).assertEqual('object')
        for (let i = 0; i < promise.length; i++) {
            console.log("==========================>publishGetAllActiveNotificationsPromisehashCode=======================>" + promise[i].hashCode)
            console.log("==========================>publishGetAllActiveNotificationsPromisehashCode=======================>" + promise[i].Id)
            console.log("==========================>publishGetAllActiveNotificationsPromisehashCode=======================>" + promise[i].slotType)
            console.log("==========================>publishGetAllActiveNotificationsPromisehashCode=======================>" + promise[i].content.contentType)
            console.log("==========================>publishGetAllActiveNotificationsPromisehashCode=======================>" + promise[i].content.normal.title)
            console.log("==========================>publishGetAllActiveNotificationsPromisehashCode=======================>" + promise[i].content.normal.text)
            console.log("==========================>publishGetAllActiveNotificationsPromisehashCode=======================>" + promise[i].content.normal.additionalText)
        }

        await notify.getActiveNotificationCount((error,data) => {
            console.log("============getActiveNotificationNumsTest1============"+JSON.stringify(data))
            expect(typeof(data)).assertEqual('number')
        })

        var promise = await notify.getActiveNotificationCount();
        expect(typeof(promise)).assertEqual('number')
        console.log("============getActiveNotificationNumsTest1Promise============"+JSON.stringify(promise))
        console.log("============ActsGetCancelActiveNum_test_0200 finished============")
        done();
        setTimeout(function(){
            console.debug("====>time out ActsGetCancelActiveNum_test_0200====>");
        }, time);
    })

    /*
    * @tc.number: ActsGetCancelActiveNum_test_0300
    * @tc.name: cancel(),getActiveNotificationNums()
    * @tc.desc: verify the function of cancel,getActiveNotificationNums
    */
    it('ActsGetCancelActiveNum_test_0300', 0,async function (done) {

        await notify.cancel(1,cancelAllCallBack);

        await notify.getActiveNotificationCount((error,data) => {
            expect(typeof(data)).assertEqual('number')
            console.log("============getActiveNotificationCancelNums============"+data)
        })

        var promise = await notify.getActiveNotificationCount();
        expect(typeof(promise)).assertEqual('number')
        console.log("============getActiveNotificationCancelNumsPromise============"+JSON.stringify(promise))
        console.log("============ActsGetCancelActiveNum_test_0300 finished============")
        done();
        setTimeout(function(){
            console.debug("====>time out ActsGetCancelActiveNum_test_0300====>");
        }, time);
    })
})

