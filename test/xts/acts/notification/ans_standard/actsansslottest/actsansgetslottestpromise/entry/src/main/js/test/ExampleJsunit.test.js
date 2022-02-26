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

describe('ActsAnsGetSlotTestPromise', function () {

    /*
     * @tc.number    : ActsAnsGetSlotTestPromise_0100
     * @tc.name      : Verify getSlot SOCIAL_COMMUNICATION
     * @tc.desc      : Get the SOCIAL_COMMUNICATION type slot after adding
     */
    it('ActsAnsGetSlotTestPromise_0100', 0, async function (done) {
        console.debug("====>ActsAnsGetSlotTestPromise_0100 start====>");
        notification.addSlot(notification.SlotType.SOCIAL_COMMUNICATION).then(() => {
            console.debug("====>addSlotActsAnsGetSlotTestPromise_0100 enter====>");
        })
        console.debug("====>getSlot SlotType.SOCIAL_COMMUNICATION: ====>");
        notification.getSlot(notification.SlotType.SOCIAL_COMMUNICATION).then((data) => {
            console.debug("====>getSlotActsAnsGetSlotTestPromise_0100 enter====>");
            console.debug("====>getSlotActsAnsGetSlotTestPromise_0100 data====>" + JSON.stringify(data));
            expect(data.type).assertEqual(notification.SlotType.SOCIAL_COMMUNICATION);
            expect(data.level).assertEqual(3);
            expect(data.desc).assertEqual("");
            expect(data.badgeFlag).assertEqual(true);
            expect(data.bypassDnd).assertEqual(false);
            expect(data.lockscreenVisibility).assertEqual(2);
            expect(data.vibrationEnabled).assertEqual(true);
            expect(data.sound).assertEqual("");
            expect(data.lightEnabled).assertEqual(false);
            expect(data.lightColor).assertEqual(0);
            console.debug("====>getSlotActsAnsGetSlotTestPromise_0100 finish====>");
            done();
        })
        console.debug("====>ActsAnsGetSlotTestPromise_0100 end====>");
        setTimeout(function(){
            console.debug("====>time out ActsAnsGetSlotTestPromise_0100====>");
        }, 1000);
    })

    /*
     * @tc.number    : ActsAnsGetSlotTestPromise_0200
     * @tc.name      : Verify getSlot SERVICE_INFORMATION
     * @tc.desc      : Get the SERVICE_INFORMATION type slot after adding
     */
    it('ActsAnsGetSlotTestPromise_0200', 0, async function (done) {
        console.debug("====>ActsAnsGetSlotTestPromise_0200 start====>");
        notification.addSlot(notification.SlotType.SERVICE_INFORMATION).then(() => {
            console.debug("====>addSlotByTypePromise SERVICE_INFORMATION ActsAnsGetSlotTestPromise_0200 enter");
        })
        console.debug("====>getSlot SlotType.SERVICE_INFORMATION: ====>");
        notification.getSlot(notification.SlotType.SERVICE_INFORMATION).then((data) => {
            console.debug("====>getSlotPromise SERVICE_INFORMATION ActsAnsGetSlotTestPromise_0200 enter====>");
            console.debug("====>getSlotPromise ActsAnsGetSlotTestPromise_0200 data====>" + JSON.stringify(data));
            expect(data.type).assertEqual(notification.SlotType.SERVICE_INFORMATION);
            expect(data.level).assertEqual(3);
            expect(data.desc).assertEqual("");
            expect(data.badgeFlag).assertEqual(true);
            expect(data.bypassDnd).assertEqual(false);
            expect(data.lockscreenVisibility).assertEqual(2);
            expect(data.vibrationEnabled).assertEqual(true);
            expect(data.sound).assertEqual("");
            expect(data.lightEnabled).assertEqual(false);
            expect(data.lightColor).assertEqual(0);
            console.debug("====>getSlotPromise SERVICE_INFORMATION ActsAnsGetSlotTestPromise_0200 finish====>");
            done();
        })
        console.debug("====>ActsAnsGetSlotTestPromise_0200 end====>");
        setTimeout(function(){
            console.debug("====>time out ActsAnsGetSlotTestPromise_0200====>");
        }, 1000);
    })

    /*
     * @tc.number    : ActsAnsGetSlotTestPromise_0300
     * @tc.name      : Verify getSlot CONTENT_INFORMATION
     * @tc.desc      : Get the CONTENT_INFORMATION type slot after adding
     */
    it('ActsAnsGetSlotTestPromise_0300', 0, async function (done) {
        console.debug("====>ActsAnsGetSlotTestPromise_0300 Promise start====>");
        console.debug("====>addSlot SlotType.CONTENT_INFORMATION: ====>");
        notification.addSlot(notification.SlotType.CONTENT_INFORMATION).then(() => {
            console.debug("====>addSlotByTypePromise CONTENT_INFORMATION ActsAnsGetSlotTestPromise_0300 enter");
        })
        console.debug("====>getSlot SlotType.CONTENT_INFORMATION: ====>");
        notification.getSlot(notification.SlotType.CONTENT_INFORMATION).then((data) => {
            console.debug("====>getSlotPromise CONTENT_INFORMATION ActsAnsGetSlotTestPromise_0300 enter====>");
            console.debug("====>getSlotPromise ActsAnsGetSlotTestPromise_0300 data====>" + JSON.stringify(data));
            expect(data.type).assertEqual(notification.SlotType.CONTENT_INFORMATION);
            expect(data.level).assertEqual(3);
            expect(data.desc).assertEqual("");
            expect(data.badgeFlag).assertEqual(true);
            expect(data.bypassDnd).assertEqual(false);
            expect(data.lockscreenVisibility).assertEqual(3);
            expect(data.vibrationEnabled).assertEqual(false);
            expect(data.sound).assertEqual("");
            expect(data.lightEnabled).assertEqual(false);
            expect(data.lightColor).assertEqual(0);
            console.debug("====>getSlotPromise CONTENT_INFORMATION ActsAnsGetSlotTestPromise_0300 finish====>");
            done();
        })
        console.info("====>ActsAnsGetSlotTestPromise_0300 end====>");
        setTimeout(function(){
            console.debug("====>time out ActsAnsGetSlotTestPromise_0300====>");
        }, 1000);
    })

    /*
     * @tc.number    : ActsAnsGetSlotTestPromise_0400
     * @tc.name      : Verify getSlot OTHER_TYPES
     * @tc.desc      : Get the OTHER_TYPES type slot after adding
     */
    it('ActsAnsGetSlotTestPromise_0400', 0, async function (done) {
        console.debug("====>ActsAnsGetSlotTestPromise_0400 start====>");
        console.debug("====>addSlot SlotType.OTHER_TYPES: ====>");
        notification.addSlot(notification.SlotType.OTHER_TYPES).then(() => {
            console.debug("====>addSlotByTypePromise AnsGetSlotByTypeAfterAddPromiseTest_0400 enter====>");
        })
        console.debug("====>getSlot SlotType.OTHER_TYPES: ====>");
        notification.getSlot(notification.SlotType.OTHER_TYPES).then((data) => {
            console.debug("====>getSlotPromise OTHER_TYPES ActsAnsGetSlotTestPromise_0400 enter====>");
            console.debug("====>getSlotPromise ActsAnsGetSlotTestPromise_0400 data====>" + JSON.stringify(data));
            expect(data.type).assertEqual(notification.SlotType.OTHER_TYPES);
            expect(data.level).assertEqual(3);
            expect(data.desc).assertEqual("");
            expect(data.badgeFlag).assertEqual(true);
            expect(data.bypassDnd).assertEqual(false);
            expect(data.lockscreenVisibility).assertEqual(3);
            expect(data.vibrationEnabled).assertEqual(false);
            expect(data.sound).assertEqual("");
            expect(data.lightEnabled).assertEqual(false);
            expect(data.lightColor).assertEqual(0);
            console.debug("====>getSlotPromise OTHER_TYPES ActsAnsGetSlotTestPromise_0400 finish====>");
            done();
        })
        console.debug("====>ActsAnsGetSlotTestPromise_0400 end====>");
        setTimeout(function(){
            console.debug("====>time out ActsAnsGetSlotTestPromise_0400====>");
        }, 1000);
    })

    /*
     * @tc.number    : ActsAnsGetSlotTestPromise_0500
     * @tc.name      : Verify getSlot UNKNOWN_TYPE
     * @tc.desc      : Get the UNKNOWN_TYPE type slot after adding
     */
    it('ActsAnsGetSlotTestPromise_0500', 0, async function (done) {
        console.debug("====>ActsAnsGetSlotTestPromise_0500 start====>");
        console.debug("====>addSlot SlotType.UNKNOWN_TYPE: ====>");
        notification.addSlot(notification.SlotType.UNKNOWN_TYPE).then(() => {
            console.debug("====>addSlotByTypePromise UNKNOWN_TYPE ActsAnsGetSlotTestPromise_0500 enter====>");
        })
        console.debug("====>getSlot SlotType.UNKNOWN_TYPE: ====>");
        notification.getSlot(notification.SlotType.UNKNOWN_TYPE).then((data) => {
            console.debug("====>getSlotPromise UNKNOWN_TYPE ActsAnsGetSlotTestPromise_0500 enter====>");
            console.debug("====>getSlotPromise  ActsAnsGetSlotTestPromise_0500 data====>" + JSON.stringify(data));
            expect(data.type).assertEqual(notification.SlotType.OTHER_TYPES);
            expect(data.level).assertEqual(3);
            expect(data.desc).assertEqual("");
            expect(data.badgeFlag).assertEqual(true);
            expect(data.bypassDnd).assertEqual(false);
            expect(data.lockscreenVisibility).assertEqual(3);
            expect(data.vibrationEnabled).assertEqual(false);
            expect(data.sound).assertEqual("");
            expect(data.lightEnabled).assertEqual(false);
            expect(data.lightColor).assertEqual(0);
            console.debug("====>getSlotPromise UNKNOWN_TYPE ActsAnsGetSlotTestPromise_0500 finish====>");
            done();
        })
        console.debug("====>ActsAnsGetSlotTestPromise_0500 end====>");
        setTimeout(function(){
            console.debug("====>time out ActsAnsGetSlotTestPromise_0500====>");
        }, 1000);
    })
}) 