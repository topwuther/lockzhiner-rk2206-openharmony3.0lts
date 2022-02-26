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
describe('ActsAnsGetActiveTest', function () {
    console.info("===ActsSubscriberTest start===>");

    /*
    * @tc.number: ActsActive_test_0100
    * @tc.name: getAllActiveNotifications()
    * @tc.desc: verify the function of getAllActiveNotifications
    */
    it('ActsActive_test_0100', 0, async function (done) {
        await notify.getAllActiveNotifications((error, data) => {
            console.log("===>getAllActiveNotifications1===>" + data)
            expect(typeof(data)).assertEqual('object')
            for (let i = 0; i < data.length; i++) {
                console.log("===>getAllActiveNotificationshashCode1===>" + data[i].hashCode)
                console.log("===>getAllActiveNotificationsid1===>" + data[i].id)
                console.log("===>getAllActiveNotificationslotType1===>" + data[i].slotType)
                console.log("===>getAllActiveNotificationclassification1===>" + data[i].classification)
                console.log("===>getAllActiveNotificationsortingKey1===>" + data[i].sortingKey)
                console.log("===>getAllActiveNotificationscontentType1===>" + data[i].content.contentType)
                console.log("===>getAllActiveNotificationstitle1===>" + data[i].content.normal.title)
                console.log("===>getAllActiveNotificationstext1===>" + data[i].content.normal.text)
                console.log("===>getAllActiveNotificationsadditionalText1===>" + data[i].content.normal.additionalText)
            }
        })

        var promise = await notify.getAllActiveNotifications()
        expect(typeof(promise)).assertEqual('object')
        for (let i = 0; i < promise.length; i++) {
            console.log("===>getAllActiveNotificationsPromisehashCode1===>" + promise[i].hashCode)
            console.log("===>getAllActiveNotificationsPromiseid1===>" + promise[i].id)
            console.log("===>getAllActiveNotificationsPromiseslotType1===>" + promise[i].slotType)
            console.log("===>getAllActiveNotificationsPromiseclassification1===>" + promise[i].classification)
            console.log("===>getAllActiveNotificationsPromisesortingKey1===>" + promise[i].sortingKey)
            console.log("===>getAllActiveNotificationsPromisecontentType1===>" + promise[i].content.contentType)
            console.log("===>getAllActiveNotificationsPromisetitle1===>" + promise[i].content.normal.title)
            console.log("===>getAllActiveNotificationsPromisetext1===>" + promise[i].content.normal.text)
            console.log("===>getAllActiveNotificationsPromiseadditionalText1===>" + promise[i].content.normal.additionalText)
        }
        done();
	setTimeout(function(){
            console.debug("====>time out ActsActive_test_0100====>");
        }, time);
    })	
})	

