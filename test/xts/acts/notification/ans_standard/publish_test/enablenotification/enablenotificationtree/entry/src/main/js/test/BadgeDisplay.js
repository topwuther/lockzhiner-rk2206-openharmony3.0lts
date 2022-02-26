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
describe('ActsAnsEnableNotificationTreeTest', function () {
    console.info("===ActsEnableNotificationTest3 start===>");

    /*
     * @tc.number: ActsEnableNotification_test_0500
     * @tc.name: enableNotification()
     * @tc.desc: verify the function of enableNotification
     */
    it('ActsEnableNotification_test_0500', 0, async function (done) {
        await notify.enableNotification({
            bundle:"com.example.actsanslocalnotificationtest",
        },false,(err) => {
            console.log("====>ActsEnableNotification_test_0500 success====>"+err)
        })
        done();
        setTimeout(function(){
            console.debug("====>time out ActsEnableNotification_test_0500====>");
        }, time);
    })

    /*
     * @tc.number: ActsEnableNotification_test_0600
     * @tc.name: enableNotification()
     * @tc.desc: verify the function of enableNotification
     */
    it('ActsEnableNotification_test_0600', 0, async function (done) {
        await notify.enableNotification(
            {
                bundle:"com.example.actsanslocalnotificationtest",
            },true).then(console.log("====>ActsEnableNotification_test_0600 success====>"))
        done();
        setTimeout(function(){
            console.debug("====>time out ActsEnableNotification_test_0600====>");
        }, time);
    })

    /*
     * @tc.number: ActsSystemNotification_test_0500
     * @tc.name: isNotificationEnabled(bundle)
     * @tc.desc: verify the function of isNotificationEnabled
     */
    it('ActsSystemNotification_test_0500', 0, async function (done) {
        await notify.isNotificationEnabled({
            bundle:"com.example.actsanslocalnotificationtest",
        },(err,data) => {
            console.log("====>ActsSystemNotification_test_0500 success====>"+err+data)
            expect(typeof(data)).assertEqual('boolean')
        })
        done();
        setTimeout(function(){
            console.debug("====>time out ActsSystemNotification_test_0500====>");
        }, time);
    })

    /*
     * @tc.number: ActsSystemNotification_test_0600
     * @tc.name: isNotificationEnabled(bundle)
     * @tc.desc: verify the function of isNotificationEnabled
     */
    it('ActsSystemNotification_test_0600', 0, async function (done) {
        var promise = await notify.isNotificationEnabled(
            {
                bundle:"com.example.actsanslocalnotificationtest",
            })
        expect(typeof(promise)).assertEqual('boolean')
        console.log("====>ActsSystemNotification_test_0600 success====>"+promise)
        done();
        setTimeout(function(){
            console.debug("====>time out ActsSystemNotification_test_0600====>");
        }, time);
    })

})

