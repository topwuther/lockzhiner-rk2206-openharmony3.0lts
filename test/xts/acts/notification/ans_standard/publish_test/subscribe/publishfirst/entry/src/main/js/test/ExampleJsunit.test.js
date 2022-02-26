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

describe('ActsAnsPublishOneTest', function () {
    function publishCallback001(){
        console.log('ActsAnsPublishTest ACTS_PublishTest_0100 asyncCallback')
    }

    /*
    * @tc.number: ACTS_PublishTest_0100
    * @tc.name: publish()
    * @tc.desc: verify the function of publish
    */
    it('ACTS_PublishTest_0100', 0,async function (done) {
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
        done();
    })
}) 