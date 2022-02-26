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
describe('ActsAnsSubscriberTest', function () {
    console.info("==ActsDoNotSubscriberTest start==>");

    //consume
    function consumeCallbackOne(err,data) {
        console.debug("==>consumeDoNotCallback1 data : ==>" +err+ JSON.stringify(data));
        expect(typeof(data)).assertEqual("object")
    }
    function consumeCallbackTwo(err,data) {
        console.debug("==>consumeDoNotCallback2 data : ==>" +err+ JSON.stringify(data));
        expect(typeof(data)).assertEqual("object")
    }
    function consumeCallbackTree(err,data) {
        console.debug("==>consumeDoNotCallback3 data : ==>" +err+ JSON.stringify(data));
        expect(typeof(data)).assertEqual("object")
    }
    function consumeCallbackFour(err,data) {
        console.debug("==>consumeDoNotCallback4 data : ==>" +err+ JSON.stringify(data));
        expect(typeof(data)).assertEqual("object")
    }
    function consumeCallbackFive(err,data) {
        console.debug("==>consumeDoNotCallback5 data : ==>" +err+ JSON.stringify(data));
        expect(typeof(data)).assertEqual("object")
    }
    function consumeCallbackSix(err,data) {
        console.debug("==>consumeDoNotCallback6 data : ==>" +err+ JSON.stringify(data));
        expect(typeof(data)).assertEqual("object")
    }
    function consumeCallbackSeven(err,data) {
        console.debug("==>consumeDoNotCallback7 data : ==>" +err+ JSON.stringify(data));
        expect(typeof(data)).assertEqual("object")
    }
    function consumeCallbackEight(err,data) {
        console.debug("==>consumeDoNotCallback8 data : ==>" +err+ JSON.stringify(data));
        expect(typeof(data)).assertEqual("object")
    }
    function consumeCallbacka(err,data) {
        console.debug("==>consumeCallbacka data : ==>" +err+ JSON.stringify(data));
        expect(typeof(data)).assertEqual("object")
    }
    function consumeCallbackb(err,data) {
        console.debug("==>consumeCallbackb data : ==>" +err+ JSON.stringify(data));
        expect(typeof(data)).assertEqual("object")
    }
    function consumeCallbackc(err,data) {
        console.debug("==>consumeCallbackc data : ==>" +err+ JSON.stringify(data));
        expect(typeof(data)).assertEqual("object")
    }
    function consumeCallbackd(err,data) {
        console.debug("==>consumeCallbackd data : ==>" +err+ JSON.stringify(data));
        expect(typeof(data)).assertEqual("object")
    }
    function consumeCallbacke(err,data) {
        console.debug("==>consumeCallbacke data : ==>" +err+ JSON.stringify(data));
        expect(typeof(data)).assertEqual("object")
    }
    function consumeCallbackf(err,data) {
        console.debug("==>consumeCallbackf data : ==>" +err+ JSON.stringify(data));
        expect(typeof(data)).assertEqual("object")
    }
    //subscribeOn
    function subscribeOnCallback(err) {
        console.debug("==>subscribeDoNotOnCallback==>");
    }
    //subscribe
    function subscribeCallback(err) {
        console.debug("==>subscribeDoNotCallback==>");

    }
    function publishCallback001(){
        console.log('ActsAnsSubscriberTest ACTS_Publish_0100 asyncCallback')
    }

    /*
     * @tc.number: ActsDoNotSubscriber_test_0100
     * @tc.name: subscribe()
     * @tc.desc: verify the function of subscribe
     */
    it('ActsSubscriber_test_0100', 0, async function (done) {
        console.debug("==ActsDoNotSubscriber_test_0100==begin==>");

        var subInfo ={
            onConsumed:consumeCallbackOne,
            onConnected:subscribeOnCallback,
        }
      try {
          await notify.subscribe(subInfo, subscribeCallback);
      }
        catch(err) {
            console.error('=ActsDoNotSubscriber_test_0100  订阅 subscribeDoNotCallback err:'+err);
        }
        console.debug("==ActsSubscriber_test_0100==end3==>");
        done();
 	setTimeout(function(){
            console.debug("====>time out ActsSubscriber_test_0100====>");
        }, time);
    })

    /*
     * @tc.number: ActsSubscriber_test_0200
     * @tc.name: subscribe()
     * @tc.desc: verify the function of subscribe
     */
    it('ActsSubscriber_test_0200', 0, async function (done) {
        console.debug("==ActsSubscriber_test_0200==begin==>");
        var subInfo ={
            onConsumed:consumeCallbackTwo,
            onConnected:subscribeOnCallback,
        }
        try{
            await notify.subscribe(subInfo,subscribeCallback);
            await notify.subscribe(subInfo,subscribeCallback);
        }catch(err) {
            console.error('=ActsDoNotSubscriber_test_0200  订阅 subscribeDoNotCallback err:'+err);
        }
        console.debug("==ActsSubscriber_test_0200==end3==>");
        done();
 	setTimeout(function(){
            console.debug("====>time out ActsSubscriber_test_0200====>");
        }, time);
    })

    /*
     * @tc.number: ActsSubscriber_test_0300
     * @tc.name: subscribe()
     * @tc.desc: verify the function of subscribe
     */
    it('ActsSubscriber_test_0300', 0, async function (done) {
        console.debug("==ActsSubscriber_test_0300==begin==>");

        var subInfo ={
            onConsumed:consumeCallbackTree,
            onConnected:subscribeOnCallback,
        }
        var subInfo2 ={
            onConsumed:consumeCallbackFour,
            onConnected:subscribeOnCallback,
        }
        try{
            await notify.subscribe(subInfo,subscribeCallback);
            await notify.subscribe(subInfo2,subscribeCallback);
        }catch(err) {
            console.error('=ActsSubscriber_test_0300  订阅 subscribeDoNotCallback err:'+err);
        }
        console.debug("==ActsSubscriber_test_0300==end3==>");
        done();
 	setTimeout(function(){
            console.debug("====>time out ActsSubscriber_test_0300====>");
        }, time);
    })

    /*
     * @tc.number: ActsSubscriber_test_0400
     * @tc.name: subscribe()
     * @tc.desc: verify the function of subscribe
     */
    it('ActsSubscriber_test_0400', 0, async function (done) {
        console.debug("==ActsSubscriber_test_0400==begin==>");
        try{
            await notify.subscribe({
                onConsumed:consumeCallbackFive,
                onConnected:subscribeOnCallback,
            },subscribeCallback);
        }catch(err) {
            console.error('=ActsSubscriber_test_0400  订阅 subscribeDoNotCallback err:'+err);
        }
        console.debug("==ActsSubscriber_test_0400==end3==>");
        done();
 	setTimeout(function(){
            console.debug("====>time out ActsSubscriber_test_0400====>");
        }, time);
    })

    /*
     * @tc.number: ActsSubscriber_test_0500
     * @tc.name: subscribe()
     * @tc.desc: verify the function of subscribe
     */
    it('ActsSubscriber_test_0500', 0, async function (done) {
        console.debug("==ActsSubscriber_test_0500==begin==>");

        var subInfo ={
            onConsumed:consumeCallbackSix,
            onConnected:subscribeOnCallback,
        }
        var subInfo2 ={
            onConsumed:consumeCallbackSeven,
            onConnected:subscribeOnCallback,
        }
        try{
            await notify.subscribe(subInfo,subscribeCallback);
            await notify.subscribe(subInfo,subscribeCallback);
            await notify.subscribe(subInfo2,subscribeCallback);
            await notify.subscribe({
                onConsumed:consumeCallbackEight,
                onConnected:subscribeOnCallback,
            },subscribeCallback);
        }catch(err) {
            console.error('=ActsSubscriber_test_0500  订阅 subscribeDoNotCallback err:'+err);
        }
        console.debug("==ActsSubscriber_test_0500==end3==>");
        done();
 	setTimeout(function(){
            console.debug("====>time out ActsSubscriber_test_0500====>");
        }, time);
    })

    /*
     * @tc.number: ActsSubscriber_test_0600
     * @tc.name: subscribe()
     * @tc.desc: verify the function of subscribe
     */
    it('ActsSubscriber_test_0600', 0, async function (done) {
        console.debug("==ActsSubscriber_test_0600==begin==>");

        var subInfo ={
            onConsumed:consumeCallbacka,
            onConnected:subscribeOnCallback,
        }
        try {
            await notify.subscribe(subInfo, {bundleNames:["com.neu.actsanspublishonetest"]},subscribeCallback);
        }
        catch(err) {
            console.error('=ActsSubscriber_test_0600  订阅 subscribeDoNotCallback err:'+err);
        }
        console.debug("==ActsSubscriber_test_0600==end3==>");
        done();
 	setTimeout(function(){
            console.debug("====>time out ActsSubscriber_test_0600====>");
        }, time);
    })

    /*
     * @tc.number: ActsSubscriber_test_0700
     * @tc.name: subscribe()
     * @tc.desc: verify the function of subscribe
     */
    it('ActsSubscriber_test_0700', 0, async function (done) {
        console.debug("==ActsSubscriber_test_0700==begin==>");

        var subInfo ={
            onConsumed:consumeCallbackb,
            onConnected:subscribeOnCallback,
        }
        try {
            await notify.subscribe(subInfo, {bundleNames:["com.neu.actsanspublishonetest"]}).then(
                    console.log("ActsSubscriber_test_0700=======promise")
            );
        }
        catch(err) {
            console.error('=ActsSubscriber_test_0700  订阅 promise err:'+err);
        }
        console.debug("==ActsSubscriber_test_0700==end3==>");
        done();
 	setTimeout(function(){
            console.debug("====>time out ActsSubscriber_test_0700====>");
        }, time);
    })

    /*
     * @tc.number: ActsSubscriber_test_0800
     * @tc.name: subscribe()
     * @tc.desc: verify the function of subscribe
     */
    it('ActsSubscriber_test_0800', 0, async function (done) {
        console.debug("==ActsDoNotSubscriber_test_0100==begin==>");

        var subInfo ={
            onConsumed:consumeCallbackc,
            onConnected:subscribeOnCallback,
        }
        try {
            await notify.subscribe(subInfo, {bundleNames:["com.neu.actsanspublishonetest","com.neu.actsanspublishtwotest"]},subscribeCallback);
        }
        catch(err) {
            console.error('=ActsSubscriber_test_0800  订阅 subscribeCallback err:'+err);
        }
        console.debug("==ActsSubscriber_test_0800==end3==>");
        done();
 	setTimeout(function(){
            console.debug("====>time out ActsSubscriber_test_0800====>");
        }, time);
    })

    /*
     * @tc.number: ActsSubscriber_test_0900
     * @tc.name: subscribe()
     * @tc.desc: verify the function of subscribe
     */
    it('ActsSubscriber_test_0900', 0, async function (done) {
        console.debug("==ActsSubscriber_test_0800==begin==>");

        var subInfo ={
            onConsumed:consumeCallbackd,
            onConnected:subscribeOnCallback,
        }
        try {
            await notify.subscribe(subInfo, {bundleNames:["com.neu.actsanspublishonetest","com.neu.actsanspublishtwotest"],}).then(
                console.log("ActsSubscriber_test_0900=======promise")
            );
        }
        catch(err) {
            console.error('=ActsSubscriber_test_0900  订阅 promise err:'+err);
        }
        console.debug("==ActsSubscriber_test_0900==end3==>");
        done();
 	setTimeout(function(){
            console.debug("====>time out ActsSubscriber_test_0900====>");
        }, time);
    })

    /*
     * @tc.number: ActsSubscriber_test_1000
     * @tc.name: subscribe()
     * @tc.desc: verify the function of subscribe
     */
    it('ActsSubscriber_test_1000', 0, async function (done) {
        console.debug("==ActsSubscriber_test_1000==begin==>");

        var subInfo ={
            onConsumed:consumeCallbacke,
            onConnected:subscribeOnCallback,
        }
        try {
            await notify.subscribe(subInfo, {bundleNames:["com.neu.actsanspublishonetest","com.neu.actsanspublishonetest"]},subscribeCallback);
        }
        catch(err) {
            console.error('=ActsSubscriber_test_1000  订阅 subscribeCallback err:'+err);
        }
        console.debug("==ActsSubscriber_test_1000==end3==>");
        done();
 	setTimeout(function(){
            console.debug("====>time out ActsSubscriber_test_1000====>");
        }, time);
    })

    /*
     * @tc.number: ActsSubscriber_test_1100
     * @tc.name: subscribe()
     * @tc.desc: verify the function of subscribe
     */
    it('ActsSubscriber_test_1100', 0, async function (done) {
        console.debug("==ActsSubscriber_test_1100==begin==>");

        var subInfo ={
            onConsumed:consumeCallbackf,
            onConnected:subscribeOnCallback,
        }
        try {
            await notify.subscribe(subInfo, {bundleNames:["com.neu.actsanspublishonetest","com.neu.actsanspublishonetest"]}).then(
                    console.log("ActsSubscriber_test_1100=======promise")
            );
        }
        catch(err) {
            console.error('=ActsSubscriber_test_1100  订阅 subscribeDoNotCallback err:'+err);
        }
        console.debug("==ActsSubscriber_test_1100==end3==>");
        done();
 	setTimeout(function(){
            console.debug("====>time out ActsSubscriber_test_1100====>");
        }, time);
    })

    /*
     * @tc.number: ACTS_publishTest_0100
     * @tc.name: publish()
     * @tc.desc: verify the function of publish
     */
    it('ACTS_publishTest_0100', 0,async function (done) {
        await notify.publish({
            id: 1,
            content: {
                contentType: notify.ContentType.NOTIFICATION_CONTENT_BASIC_TEXT,
                normal: {
                    title: "test1_title",
                    text: "test1_text",
                    additionalText: "test1_additionalText"
                },
            },
            slotType:notify.SlotType.SOCIAL_COMMUNICATION,
            classification:"classification1",
            sortingKey:"sortingKey1",
        },publishCallback001);
	done();
 	setTimeout(function(){
            console.debug("====>time out ACTS_publishTest_0100====>");
        }, time);
    })
})

