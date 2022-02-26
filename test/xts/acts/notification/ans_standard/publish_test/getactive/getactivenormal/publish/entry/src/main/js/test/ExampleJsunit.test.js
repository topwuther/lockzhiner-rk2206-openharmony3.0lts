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
var time = 1000
describe('ActsAnsActivePublishTest', function () {
    function publishCallback001(){
        console.log('ActsNotificationTest ACTS_Publish_0100 asyncCallback')
    }

    /*
    * @tc.number: ACTS_GetActiveNumTest_0100
    * @tc.name: getActiveNotifications(),getActiveNotificationNums()
    * @tc.desc: verify the function of getActiveNotifications,getActiveNotificationNums
    */
    it('ACTS_GetActiveNumTest_0100', 0,async function (done) {
        await notification.publish({
            id: 1,
            content: {
                contentType: notification.ContentType.NOTIFICATION_CONTENT_BASIC_TEXT,
                normal: {
                    title: "test1_title",
                    text: "test1_text",
                    additionalText: "test1_additionalText"
                },
            },
            slotType:notification.SlotType.SOCIAL_COMMUNICATION,
            classification:"classification1",
            sortingKey:"sortingKey1",
        },publishCallback001);
        await notification.publish({
            id: 2,
            content: {
                contentType: notification.ContentType.NOTIFICATION_CONTENT_BASIC_TEXT,
                normal: {
                    title: "test2_title",
                    text: "test2_text",
                    additionalText: "test2_additionalText"
                },
            },
            slotType:notification.SlotType.SOCIAL_COMMUNICATION,
            classification:"classification2",
            sortingKey:"sortingKey2",
        },publishCallback001);

        await notification.getActiveNotifications(
        (error,data) => {
            console.log("==PublishgetActiveNotificationsTest1=="+data)
            expect(typeof(data)).assertEqual('object')
            for (let i = 0; i < data.length; i++) {
                console.log("===>PublishgetActiveNotificationsTest1===>" + data[i].hashCode)
                console.log("===>PublishgetActiveNotificationsTest1===>" + data[i].id)
                console.log("===>PublishgetActiveNotificationsTest1===>" + data[i].classification)
                console.log("===>PublishgetActiveNotificationsTest1===>" + data[i].sortingKey)
                console.log("===>PublishgetActiveNotificationsTest1===>" + data[i].slotType)
                console.log("===>PublishgetActiveNotificationsTest1===>" + data[i].content.contentType)
                console.log("===>PublishgetActiveNotificationsTest1===>" + data[i].content.normal.title)
                console.log("===>PublishgetActiveNotificationsTest1===>" + data[i].content.normal.text)
                console.log("===>PublishgetActiveNotificationsTest1===>" + data[i].content.normal.additionalText)
            }
        })

        var promise = await notification.getActiveNotifications();
        expect(typeof(promise)).assertEqual('object')
        for (let i = 0; i < promise.length; i++) {
            console.log("==>publishgetAllActiveNotificationsPromisehashCode==>" + promise[i].hashCode)
            console.log("==>publishgetAllActiveNotificationsPromisehashCode==>" + promise[i].id)
            console.log("==>publishgetAllActiveNotificationsPromisehashCode==>" + promise[i].slotType)
            console.log("==>publishgetAllActiveNotificationsPromisehashCode==>" + promise[i].classification)
            console.log("==>publishgetAllActiveNotificationsPromisehashCode==>" + promise[i].sortingKey)
            console.log("==>publishgetAllActiveNotificationsPromisehashCode==>" + promise[i].content.contentType)
            console.log("==>publishgetAllActiveNotificationsPromisehashCode==>" + promise[i].content.normal.title)
            console.log("==>publishgetAllActiveNotificationsPromisehashCode==>" + promise[i].content.normal.text)
            console.log("==>publishgetAllActiveNotificationsPromisehashCode==>" + promise[i].content.normal.additionalText)
        }

        await notification.getActiveNotificationCount((error,data) => {
            console.log("==getActiveNotificationNumsTest1=="+JSON.stringify(data))
            expect(typeof(data)).assertEqual('number')
        })

        var promise = await notification.getActiveNotificationCount();
        console.log("==getActiveNotificationNumsTest1Promise=="+JSON.stringify(promise))
        console.log("==ACTS_PublishTest1_0100 finished==")
        expect(typeof(promise)).assertEqual('number')
        done();
       	setTimeout(function(){
            console.debug("====>time out ACTS_GetActiveNumTest_0100====>");
        }, time);
    })
}) 
