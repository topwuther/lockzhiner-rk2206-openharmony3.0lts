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
import commonEvent from "@ohos.commonevent"
import rpc from "@ohos.rpc"
import featureAbility from '@ohos.ability.featureAbility'
import {describe, beforeAll, beforeEach, afterEach, afterAll, it, expect} from 'deccjsunit/index'

const TIMEOUT = 9000;
const START_ABILITY_TIMEOUT = 5000;

var subscriberInfoStartAbility_0100 = {
    events: ["ACTS_Particle_StartAbility_0100_CommonEvent"],
};
var subscriberInfoStartAbility_0200 = {
    events: ["ACTS_Particle_StartAbility_0200_CommonEvent"],
};

describe('ActsStServiceAbilityTest', function () {
    let bundleName = "com.example.particletestserver";
    let abilityName = "com.example.particletestserver.MainAbility";
    var mRemote;
    function onConnectCallback(element, remote) {
        console.log('ACTS_featureAbilityTest ConnectAbility onConnect element.deviceId : ' + element.deviceId)
        console.log('ACTS_featureAbilityTest ConnectAbility onConnect element.bundleName : ' + element.bundleName)
        console.log('ACTS_featureAbilityTest ConnectAbility onConnect element.abilityName : ' + element.abilityName)
        console.log('ACTS_featureAbilityTest ConnectAbility onConnect element.uri : ' + element.uri)
        console.log('ACTS_featureAbilityTest ConnectAbility onConnect element.shortName : ' + element.shortName)
        console.log('ACTS_featureAbilityTest ConnectAbility onConnect remote : ' + remote);
        mRemote = remote;
        console.log('ACTS_featureAbilityTest ConnectAbility onConnect remote 是否为proxy:' + (remote instanceof rpc.RemoteProxy));
    }

    function onDisconnectCallback(element) {
        console.log('ACTS_featureAbilityTest ConnectAbility onDisconnect element.deviceId : ' + element.deviceId)
        console.log('ACTS_featureAbilityTest ConnectAbility onDisconnect element.bundleName : ' + element.bundleName)
        console.log('ACTS_featureAbilityTest ConnectAbility onDisconnect element.abilityName : ' + element.abilityName)
        console.log('ACTS_featureAbilityTest ConnectAbility onDisconnect element.uri : ' + element.uri)
        console.log('ACTS_featureAbilityTest ConnectAbility onDisconnect element.shortName : ' + element.shortName)
    }

    function onFailedCallback(code) {
        console.log('ACTS_featureAbilityTest ConnectAbility onFailed errCode : ' + code)
    }

    /*
* @tc.number: ACTS_ParticleAbility_startAbility_0100
* @tc.name: ConnectAbility : Connects an ability to a Service ability.
* @tc.desc: Check the return value of the interface (by Promise)
*/
    it('ACTS_ParticleAbility_startAbility_0100', 0, async function (done) {
        console.log('ACTS_ParticleAbility_startAbility_0100====<begin');
        console.log('========StartConnect called');
        var subscriber;
        let id;
        let connId;

        function subscribeCallBack(err, data) {
            clearTimeout(id);
            expect(data.event).assertEqual("ACTS_Particle_StartAbility_0100_CommonEvent");
            console.debug("====>Subscribe CallBack data:====>" + JSON.stringify(data));
            var result = featureAbility.disconnectAbility(
                connId,
                (error, data) => {
                    console.log('featureAbilityTest DisconnectAbility result errCode : ' + error.code + " data: " + data)
                },
            );
            console.log('DisconnectNative ConnectAbility result : ' + result);
            commonEvent.unsubscribe(subscriber, unSubscribeCallback)
            done();
        }

        commonEvent.createSubscriber(subscriberInfoStartAbility_0100).then(async (data) => {
            console.debug("====>Create Subscriber====>");
            subscriber = data;
            await commonEvent.subscribe(subscriber, subscribeCallBack);
        })

        function unSubscribeCallback() {
            console.debug("====>UnSubscribe CallBack====>");
            done();
        }

        function timeout() {
            expect().assertFail();
            console.debug('ACTS_ParticleAbility_startAbility_0100 timeout');
            commonEvent.unsubscribe(subscriber, unSubscribeCallback)
            done();
        }

        id = setTimeout(timeout, START_ABILITY_TIMEOUT);
        connId = await featureAbility.connectAbility(
            {
                bundleName: bundleName,
                abilityName: abilityName,
                action: "StartAbilityPromise"
            },
            {
                onConnect: onConnectCallback,
                onDisconnect: onDisconnectCallback,
                onFailed: onFailedCallback,
            },
        );
        console.log('StartConnectNative ConnectAbility connId : ' + connId);
        setTimeout(function () {
            console.log('StartConnectNative ConnectAbility timeout')
        }, TIMEOUT);
    })

        /*
    * @tc.number: ACTS_ParticleAbility_startAbility_0200
    * @tc.name: ConnectAbility : Connects an ability to a Service ability.
    * @tc.desc: Check the return value of the interface (by Promise)
    */
        it('ACTS_ParticleAbility_startAbility_0200', 0, async function (done) {
            console.log('ACTS_ParticleAbility_startAbility_0200====<begin');
            console.log('========StartConnect called');
            var subscriber;
            let id;
            let connId;

            function subscribeCallBack(err, data) {
                clearTimeout(id);
                expect(data.event).assertEqual("ACTS_Particle_StartAbility_0200_CommonEvent");
                console.debug("====>Subscribe CallBack data:====>" + JSON.stringify(data));
                var result = featureAbility.disconnectAbility(
                connId,
                (error,data) => {
                    console.log('featureAbilityTest DisconnectAbility result errCode : ' + error.code + " data: " + data)
                },
            );
            console.log('DisconnectNative ConnectAbility result : ' + result);
                commonEvent.unsubscribe(subscriber, unSubscribeCallback)
                done();
            }

            commonEvent.createSubscriber(subscriberInfoStartAbility_0200).then(async (data) => {
                console.debug("====>Create Subscriber====>");
                subscriber = data;
                await commonEvent.subscribe(subscriber, subscribeCallBack);
            })

            function unSubscribeCallback() {
                console.debug("====>UnSubscribe CallBack====>");
                done();
            }

            function timeout() {
                expect().assertFail();
                console.debug('ACTS_ParticleAbility_startAbility_0200 timeout');
                commonEvent.unsubscribe(subscriber, unSubscribeCallback)
                done();
            }

            id = setTimeout(timeout, START_ABILITY_TIMEOUT);
            connId = await featureAbility.connectAbility(
                {
                    bundleName: bundleName,
                    abilityName: abilityName,
                    action: "StartAbilityCallback"
                },
                {
                    onConnect: onConnectCallback,
                    onDisconnect: onDisconnectCallback,
                    onFailed: onFailedCallback,
                },
            );
            console.log('StartConnectNative ConnectAbility connId : ' + connId);
            setTimeout(function () {
                console.log('StartConnectNative ConnectAbility timeout')
            }, TIMEOUT);
        })

})