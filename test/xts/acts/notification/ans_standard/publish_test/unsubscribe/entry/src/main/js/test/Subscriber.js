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
describe('ActsAnsUnSubscriberTest', function () {
    console.info("==ActsDoNotSubscriberTest start==>");
    //subscribeOn
    function subscribeOnCallback(err) {
        console.debug("===>subscribeDoNotOnCallback===>");
    }
    //subscribe
    function subscribeCallback(err) {
        console.debug("===>subscribeDoNotCallback===>");

    }
    function unSubscribeCallbackOne(err) {
        console.debug("===>unSubscribeCallbackOne===>");
    }
    function unSubscribeCallbackTwo(err) {
        console.debug("===>unSubscribeCallbackTwo===>");
    }
    function unSubscribeCallbackThree(err) {
        console.debug("===>unSubscribeCallbackThree===>");
    }
    function unSubscribeCallbackFour(err) {
        console.debug("===>unSubscribeCallbackFour===>");
    }
    function unSubscribeCallbackFive(err) {
        console.debug("===>unSubscribeCallbackFive===>");
    }
    function unSubscribeCallbackSix(err) {
        console.debug("===>unSubscribeCallbackSix===>");
    }
    function unSubscribeCallbackSeven(err) {
        console.debug("===>unSubscribeCallbackSeven===>");
    }
    function unSubscribeCallbackEight(err) {
        console.debug("===>unSubscribeCallbackEight===>");
    }

    function publishCallback001(){
        console.log('ActsAnsSubscriberTest ACTS_Publish_0100 asyncCallback')
    }

    /*
     * @tc.number: ActsDoNotSubscriber_test_0100
     * @tc.name: subscribe()
     * @tc.desc: verify the function of subscribe
     */
    it('ActsUnSubscriber_test_0100', 0, async function (done) {
        console.debug("===ActsDoNotSubscriber_test_0100===begin===>");

        var subInfoOne ={
            onConnecte:subscribeOnCallback,
            onDestroy:unSubscribeCallbackOne
        }
      try {
          await notify.subscribe(subInfoOne, subscribeCallback);
      }
        catch(err) {
            console.error('=ActsDoNotSubscriber_test_0100  订阅 subscribeDoNotCallback err:'+err);
        }
        try{
            await notify.unsubscribe(subInfoOne, unSubscribeCallbackOne);
        }
        catch(err) {
            console.error('=ActsSubscriber_test_0100  取消订阅 unSubscribeCallbackOne err:'+err);
        }
        console.debug("===ActsSubscriber_test_0100===end3===>");
        done();
        setTimeout(function(){
            console.debug("====>time out ActsSubscriber_test_0100====>");
        }, time);
    })

    /*
     * @tc.number: ActsDoNotSubscriber_test_0200
     * @tc.name: subscribe()
     * @tc.desc: verify the function of subscribe
     */
    it('ActsUnSubscriber_test_0200', 0, async function (done) {
        console.debug("===ActsSubscriber_test_0200===begin===>");

        var subInfoOne ={
            onConnecte:subscribeOnCallback,
            onDestroy:unSubscribeCallbackOne
        }
        try {
            await notify.subscribe(subInfoOne, subscribeCallback);
        }
        catch(err) {
            console.error('=ActsSubscriber_test_0200  订阅 subscribeDoNotCallback err:'+err);
        }
        try{
            notify.unsubscribe(subInfoOne).then(()=>
            console.log("====promise====")
            )
        }
        catch(err) {
            console.error('=ActsSubscriber_test_0200  取消订阅 unSubscribeCallbackOne err:'+err);
        }
        console.debug("===ActsSubscriber_test_0200===end3===>");
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
    it('ActsUnSubscriber_test_0300', 0, async function (done) {
        console.debug("===ActsSubscriber_test_0300===begin===>");
        var subInfoTwo ={
        //            onConsumed:consumeCallbackTwo,
            onConnecte:subscribeOnCallback,
            onDestroy:unSubscribeCallbackTwo
        }
        try{
            await notify.subscribe(subInfoTwo,subscribeCallback);
            await notify.subscribe(subInfoTwo,subscribeCallback);
        }catch(err) {
            console.error('=ActsSubscriber_test_0300  订阅 subscribeDoNotCallback err:'+err);
        }
        try{
            await notify.unsubscribe(subInfoTwo, unSubscribeCallbackTwo);
            await notify.unsubscribe(subInfoTwo, unSubscribeCallbackTwo);
        }
        catch(err) {
            console.error('=ActsSubscriber_test_0300  取消订阅 unSubscribeCallbackOne err:'+err);
        }
        console.debug("===ActsSubscriber_test_0300===end3===>");
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
    it('ActsUnSubscriber_test_0400', 0, async function (done) {
        console.debug("====ActsSubscriber_test_0400===begin====>");
        var subInfoTwo ={
        //            onConsumed:consumeCallbackTwo,
            onConnecte:subscribeOnCallback,
            onDestroy:unSubscribeCallbackTwo
        }
        try{
            await notify.subscribe(subInfoTwo,subscribeCallback);
            await notify.subscribe(subInfoTwo,subscribeCallback);
        }catch(err) {
            console.error('=ActsSubscriber_test_0400  订阅 subscribeDoNotCallback err:'+err);
        }
        try{
             notify.unsubscribe(subInfoTwo).then(
                console.log("===unsubscribeTwo Promise===")
            )
             notify.unsubscribe(subInfoTwo).then(
                console.log("===unsubscribeTwo Promise===")
            )
        }
        catch(err) {
            console.error('=ActsSubscriber_test_0400  取消订阅 unSubscribeCallbackOne err:'+err);
        }
        console.debug("===ActsSubscriber_test_0400===end3===>");
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
    it('ActsUnSubscriber_test_0500', 0, async function (done) {
        console.debug("===============ActsSubscriber_test_0500======begin====================>");

        var subInfo1 ={
            onConnecte:subscribeOnCallback,
            onDestroy:unSubscribeCallbackThree,
        }
        var subInfo2 ={
            onConnecte:subscribeOnCallback,
            onDestroy:unSubscribeCallbackFour,
        }
        try{
            await notify.subscribe(subInfo1,subscribeCallback);
            await notify.subscribe(subInfo2,subscribeCallback);
        }catch(err) {
            console.error('=ActsSubscriber_test_0500  订阅 subscribeDoNotCallback err:'+err);
        }
        try{
            await notify.unsubscribe(subInfo1, unSubscribeCallbackThree);
            await notify.unsubscribe(subInfo2, unSubscribeCallbackFour);
        }
        catch(err) {
            console.error('=ActsSubscriber_test_0500  取消订阅 unSubscribeCallback err:'+err);
        }
        console.debug("===ActsSubscriber_test_0500===end3===>");
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
    it('ActsUnSubscriber_test_0600', 0, async function (done) {
        console.debug("===ActsSubscriber_test_0600===begin====>");

        var subInfo1 ={
            onConnecte:subscribeOnCallback,
            onDestroy:unSubscribeCallbackThree,
        }
        var subInfo2 ={
            onConnecte:subscribeOnCallback,
            onDestroy:unSubscribeCallbackFour,
        }
        try{
            await notify.subscribe(subInfo1,subscribeCallback);
            await notify.subscribe(subInfo2,subscribeCallback);
        }catch(err) {
            console.error('=ActsSubscriber_test_0600  订阅 subscribeDoNotCallback err:'+err);
        }
        try{
                notify.unsubscribe(subInfo1).then(
                    console.log("==unsubscribeTree Promise==")
                )
                notify.unsubscribe(subInfo2).then(
                    console.log("===unsubscribeTree Promise===")
                )
        }
        catch(err) {
            console.error('=ActsSubscriber_test_0600  取消订阅 unSubscribeCallback err:'+err);
        }
        console.debug("===ActsSubscriber_test_0600===end3===>");
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
    it('ActsUnSubscriber_test_0700', 0, async function (done) {
        console.debug("===ActsSubscriber_test_0700===begin===>");
        try{
            await notify.subscribe({
                onConnecte:subscribeOnCallback,
                onDestroy:unSubscribeCallbackFive
            },subscribeCallback);
        }catch(err) {
            console.error('=ActsSubscriber_test_0700  订阅 subscribeDoNotCallback err:'+err);
        }
        try{
            await notify.unsubscribe({
                onConnecte:subscribeOnCallback,
                onDestroy:unSubscribeCallbackFive
            },unSubscribeCallbackFive);
        }catch(err) {
            console.error('=ActsSubscriber_test_0700  订阅 subscribeDoNotCallback err:'+err);
        }
        console.debug("===ActsSubscriber_test_0700===end3===>");
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
    it('ActsUnSubscriber_test_0800', 0, async function (done) {
        console.debug("===ActsSubscriber_test_0800===begin===>");
        try{
            await notify.subscribe({
                onConnecte:subscribeOnCallback,
                onDestroy:unSubscribeCallbackFive
            },subscribeCallback);
        }catch(err) {
            console.error('=ActsSubscriber_test_0800  订阅 subscribeDoNotCallback err:'+err);
        }
        try{
            notify.unsubscribe({
                    onConnecte:subscribeOnCallback,
                    onDestroy:unSubscribeCallbackFive
                }).then(
                    console.log("====unsubscribeFour Promise====")
                )
        }catch(err) {
            console.error('=ActsSubscriber_test_0800  订阅 subscribeDoNotCallback err:'+err);
        }
        console.debug("===ActsSubscriber_test_0800===end3===>");
        done();
        setTimeout(function(){
            console.debug("====>time out ActsSubscriber_test_0800====>");
        }, time);
    })
})

