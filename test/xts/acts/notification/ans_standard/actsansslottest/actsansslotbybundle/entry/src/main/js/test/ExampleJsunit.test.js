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

describe('ActsAnsSlotByBundle', function () {
    /*
     * @tc.number    : ActsAnsSlotByBundle_0100
     * @tc.name      : Verify getSlot SOCIAL_COMMUNICATION
     * @tc.desc      : After setSlot, call the getSlotsByBundle interface to
     *                 check whether the information is consistent.(promise)
     */
    it('ActsAnsSlotByBundle_0100', 0, async function (done) {
        console.debug("====>ActsAnsSlotByBundle_0100 start====>");
        var ret = false
        var bundleoption = {
            bundle: "com.example.actsansslotbybundle"
        }
        var notificationslot = {
            type: notification.SlotType.SERVICE_INFORMATION,
            level: 4
        }
        notification.addSlot(notification.SlotType.SERVICE_INFORMATION).then(() => {
            console.debug("====>addSlotByTypePromise SERVICE_INFORMATION enter====>");
        })
        console.debug("====>getSlotsByBundle1 start====>");
        notification.getSlotsByBundle(bundleoption).then((data) => {
            console.debug("====>getSlotsByBundle1====>" + JSON.stringify(data));
            expect(data[0].type).assertEqual(2)
            expect(data[0].level).assertEqual(3)
            expect(data[0].badgeFlag).assertEqual(true)
            expect(data[0].bypassDnd).assertEqual(false)
            expect(data[0].lockscreenVisibility).assertEqual(2)
            expect(data[0].vibrationEnabled).assertEqual(true)
            expect(data[0].lightEnabled).assertEqual(false)
            expect(data[0].lightColor).assertEqual(0)
            console.debug("====>getSlotsByBundle1 finish====>");
            console.debug("====>setSlotByBundle start====>");
        })
        notification.setSlotByBundle(bundleoption, notificationslot).then(() => {
            console.debug("====>setSlotsByBundle====>")
        })
        console.debug("====>getSlotsByBundle1.1 start====>");
        notification.getSlotsByBundle(bundleoption).then((data) => {
            console.debug("====>getSlotsByBundle1.1====>" + JSON.stringify(data));
            expect(data[0].type).assertEqual(2)
            expect(data[0].level).assertEqual(4)
            expect(data[0].badgeFlag).assertEqual(true)
            expect(data[0].bypassDnd).assertEqual(false)
            expect(data[0].lockscreenVisibility).assertEqual(2)
            expect(data[0].vibrationEnabled).assertEqual(true)
            expect(data[0].lightEnabled).assertEqual(false)
            expect(data[0].lightColor).assertEqual(0)
            console.debug("====>getSlotsByBundle1.1 finish====>");
            notification.getSlotNumByBundle(bundleoption).then((data) => {
                console.debug("====>getSlotNumAsBundle1====>" + JSON.stringify(data));
                expect(data).assertEqual(1)
                ret = true

            })
        })
        done();
        setTimeout(function () {
            expect(ret).assertEqual(true)
        }, 1000)
    })

    /*
     * @tc.number    : ActsAnsSlotByBundle_0100
     * @tc.name      : Verify getSlot SOCIAL_COMMUNICATION
     * @tc.desc      : After setSlot, call the getSlotsByBundle interface to
     *                 check whether the information is consistent.(callback)
     */
    it('ActsAnsSetSlotByBundle_0200', 0, async function (done) {
        var ret = false
        console.debug("====>ActsAnsSlotByBundle_0200 start====>");
        var bundleoption = {
            bundle: "com.example.actsansslotbybundle"
        }
        var notificationslot = {
            type: notification.SlotType.SERVICE_INFORMATION,
            level: 4
        }
        notification.addSlot(notification.SlotType.SERVICE_INFORMATION).then(() => {
            console.debug("====>addSlotByTypePromise SERVICE_INFORMATION enter====>");
        })
        console.debug("====>getSlotsByBundle2 start====>");
        notification.getSlotsByBundle(bundleoption, (err, data) => {
            console.debug("====>getSlotsByBundle2====>" + JSON.stringify(data));
            expect(data[0].type).assertEqual(2)
            expect(data[0].level).assertEqual(3)
            expect(data[0].badgeFlag).assertEqual(true)
            expect(data[0].bypassDnd).assertEqual(false)
            expect(data[0].lockscreenVisibility).assertEqual(2)
            expect(data[0].vibrationEnabled).assertEqual(true)
            expect(data[0].lightEnabled).assertEqual(false)
            expect(data[0].lightColor).assertEqual(0)
            console.debug("====>getSlotsByBundle2 finish====>");
        })
        console.debug("====>setSlotByBundle start====>");
        notification.setSlotByBundle(bundleoption, notificationslot, () => {
            console.debug("====>setSlotsByBundle====>")
        })
        console.debug("====>getSlotsByBundle2.1 start====>");
        notification.getSlotsByBundle(bundleoption, (err, data) => {
            console.debug("====>getSlotsByBundle2.1====>" + JSON.stringify(data));
            expect(data[0].type).assertEqual(2)
            expect(data[0].level).assertEqual(4)
            expect(data[0].badgeFlag).assertEqual(true)
            expect(data[0].bypassDnd).assertEqual(false)
            expect(data[0].lockscreenVisibility).assertEqual(2)
            expect(data[0].vibrationEnabled).assertEqual(true)
            expect(data[0].lightEnabled).assertEqual(false)
            expect(data[0].lightColor).assertEqual(0)
            console.debug("====>getSlotsByBundle2.1 finish====>");
        })
        notification.getSlotNumByBundle(bundleoption, (err, data) => {
            console.debug("====>getSlotNumAsBundle2====>" + JSON.stringify(data));
            expect(data).assertEqual(1)
            ret = true
        })
        done();
        setTimeout(function () {
            expect(ret).assertEqual(true)
        }, 1000)
    })
})
