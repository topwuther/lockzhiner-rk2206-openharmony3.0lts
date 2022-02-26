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
import rpc from "@ohos.rpc";
import featureAbility from '@ohos.ability.featureAbility'
import {describe, beforeAll, beforeEach, afterEach, afterAll, it, expect} from 'deccjsunit/index'

describe('ActsStServiceAbilityTest', function () {
    let bundleName = "com.amsst.stserviceabilityserver";
    let abilityName = "com.amsst.stserviceabilityserver.MainAbility";
    let g_setTimeout = 10
    var mConnIdJsPromise;
    var mConnIdJsCallback;
    var mRemote;

    function onConnectCallback(element, remote){
        console.log('ACTS_featureAbilityTest ConnectAbility onConnect element.deviceId : ' + element.deviceId)
        console.log('ACTS_featureAbilityTest ConnectAbility onConnect element.bundleName : ' + element.bundleName)
        console.log('ACTS_featureAbilityTest ConnectAbility onConnect element.abilityName : ' + element.abilityName)
        console.log('ACTS_featureAbilityTest ConnectAbility onConnect element.uri : ' + element.uri)
        console.log('ACTS_featureAbilityTest ConnectAbility onConnect element.shortName : ' + element.shortName)
        console.log('ACTS_featureAbilityTest ConnectAbility onConnect remote : ' + remote);
        mRemote = remote;
        console.log('ACTS_featureAbilityTest ConnectAbility onConnect remote 是否为proxy:' + (remote instanceof rpc.RemoteProxy));
    }

    function onDisconnectCallback(element){
        console.log('ACTS_featureAbilityTest ConnectAbility onDisconnect element.deviceId : ' + element.deviceId)
        console.log('ACTS_featureAbilityTest ConnectAbility onDisconnect element.bundleName : ' + element.bundleName)
        console.log('ACTS_featureAbilityTest ConnectAbility onDisconnect element.abilityName : ' + element.abilityName)
        console.log('ACTS_featureAbilityTest ConnectAbility onDisconnect element.uri : ' + element.uri)
        console.log('ACTS_featureAbilityTest ConnectAbility onDisconnect element.shortName : ' + element.shortName)
    }

    function onFailedCallback(code){
        console.log('ACTS_featureAbilityTest ConnectAbility onFailed errCode : ' + code)
    }

/*
* @tc.number: ACTS_ConnectAbility_0100
* @tc.name: ConnectAbility : Connects an ability to a Service ability.
* @tc.desc: Check the return value of the interface (by Promise)
*/
    it('ACTS_ConnectAbility_0100',0, async function (done) {
        console.log('ACTS_ConnectAbility_0100====<begin');
        let ret = false;
        try{
            mConnIdJsPromise = featureAbility.connectAbility(
                {
                    bundleName: bundleName,
                    abilityName: abilityName,
                },
                {
                    onConnect: onConnectCallback,
                    onDisconnect: onDisconnectCallback,
                    onFailed: onFailedCallback,
                },
            )
//            ).then(function (data) {
//                console.debug("=ACTS_ConnectAbility_0100 then data====>"
//                + (" json data 【") + JSON.stringify(data)+ (" 】; ====>")+data);
//                expect(typeof(data)).assertEqual("object");
//                console.log('=ACTS_ConnectAbility_0100 promise mConnIdJsPromise)====>:' + JSON.stringify(mConnIdJsPromise)+","+mConnIdJsPromise);
//                ret = true;
//                done()
//            }).catch(function (err){
//                console.debug("=ACTS_ConnectAbility_0100 catch err====>"
//                + ("json err 【") + JSON.stringify(err) + (" 】 ====>")+err);
//                console.log('=ACTS_ConnectAbility_0100 mConnIdJsPromise====>:' + JSON.stringify(mConnIdJsPromise))
//                ret = false;
//                done();
//            });
            ret = true;
            done()
        }catch(err) {
            console.error('=ACTS_ConnectAbility_0100 catch(err)====>:'
            + ("json err 【") + JSON.stringify(err) + (" 】 ,")+err);
            ret = false;
            done();
        }

        setTimeout(function(){
            expect(ret).assertTrue();
        }, g_setTimeout);
        console.log('ACTS_ConnectAbility_0100====<end')
    })

    /*
* @tc.number: ACTS_DisconnectAbility_0100
* @tc.name: DisconnectAbility : Disconnects an ability from a Service ability..
* @tc.desc: Check the return value of the interface (by Promise)
*/
    it('ACTS_DisconnectAbility_0100',0, async function (done) {
        console.log('ACTS_DisconnectAbility_0100====<begin');
        let ret = false;
        try{
            console.log('=ACTS_DisconnectAbility_0100====>' + mConnIdJsPromise);
            var result = featureAbility.disconnectAbility(
                mConnIdJsPromise,
            ).then(function (data) {
                console.debug("=ACTS_DisconnectAbility_0100 then data====>"
                + (" json data 【") + JSON.stringify(data)+ (" 】; ====>")+data);
                expect(typeof(data)).assertEqual("object");
                console.log('=ACTS_DisconnectAbility_0100 promise result)====>:' + JSON.stringify(result)+","+result);
                ret = true;
                done()
            }).catch(function (err){
                console.debug("=ACTS_DisconnectAbility_0100 catch err====>"
                + ("json err 【") + JSON.stringify(err) + (" 】 ====>")+err);
                console.log('=ACTS_DisconnectAbility_0100 result====>:' + JSON.stringify(result))
                ret = false;
                done();
            });

            console.log('=ACTS_DisconnectAbility_0100 result ====>: '
            + ("json result 【") + JSON.stringify(result) + (" 】 ,")+result);
        }catch(err) {
            console.error('=ACTS_DisconnectAbility_0100 catch(err)====>:'
            + ("json err 【") + JSON.stringify(err) + (" 】 ,")+err);
            ret = false;
            done();
        }

        setTimeout(function(){
            expect(ret).assertTrue();
        }, g_setTimeout);
        console.log('ACTS_DisconnectAbility_0100====<end')
    })


    /*
* @tc.number: ACTS_ConnectAbility_0200
* @tc.name: ConnectAbility : Connects an ability to a Service ability.
* @tc.desc: Check the return value of the interface (by AsyncCallback)
*/
    it('ACTS_ConnectAbility_0200',0, async function (done) {
        console.log('ACTS_ConnectAbility_0200====<begin');
        let ret = false;
        try{
            mConnIdJsCallback = featureAbility.connectAbility(
                {
                    bundleName: bundleName,
                    abilityName: abilityName,
                },
                {
                    onConnect: onConnectCallback,
                    onDisconnect: onDisconnectCallback,
                    onFailed: onFailedCallback,
                },
            //                (err,data) => {
            //                    console.debug("=ACTS_ConnectAbility_0200 connectAbility err,data====>"
            //                    + ("json err【") + JSON.stringify(err) + (" 】json data【") + JSON.stringify(data)+ (" 】;"));
            //                    ret = true;
            //                    done()
            //                },
            )
            ret = true;
            done()
        }catch(err) {
            console.error('=ACTS_ConnectAbility_0200 catch(err)====>:'
            + ("json err 【") + JSON.stringify(err) + (" 】 ,")+err);
            ret = false;
            done();
        }

        setTimeout(function(){
            expect(ret).assertTrue();
        }, g_setTimeout);
        console.log('ACTS_ConnectAbility_0200====<end')
    })

    /*
* @tc.number: ACTS_DisconnectAbility_0200
* @tc.name: DisconnectAbility : Disconnects an ability from a Service ability..
* @tc.desc: Check the return value of the interface (by AsyncCallback)
*/
    it('ACTS_DisconnectAbility_0200',0, async function (done) {
        console.log('ACTS_DisconnectAbility_0200====<begin');
        let ret = false;
        try{
            console.log('=ACTS_DisconnectAbility_0200====>' + mConnIdJsCallback);
            var result = featureAbility.disconnectAbility(
                mConnIdJsCallback,
//                {
//                    onConnect: onConnectCallback,
//                    onDisconnect: onDisconnectCallback,
//                    onFailed: onFailedCallback,
//                },
                (err,data) => {
                    console.debug("=ACTS_DisconnectAbility_0200 disconnectAbility err,data====>"
                    + ("json err【") + JSON.stringify(err) + (" 】json data【") + JSON.stringify(data)+ (" 】;"));
                    ret = true;
                    done()
                },
            );
            console.log('=ACTS_DisconnectAbility_0200 result ====>: '
            + ("json result 【") + JSON.stringify(result) + (" 】 ,")+result);
        }catch(err) {
            console.error('=ACTS_DisconnectAbility_0200 catch(err)====>:'
            + ("json err 【") + JSON.stringify(err) + (" 】 ,")+err);
            ret = false;
            done();
        }

        setTimeout(function(){
            expect(ret).assertTrue();
        }, g_setTimeout);
        console.log('ACTS_DisconnectAbility_0200====<end')
    })
})