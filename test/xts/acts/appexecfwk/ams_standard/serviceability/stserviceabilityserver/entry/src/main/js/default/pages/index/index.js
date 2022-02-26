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

let mMyStub;
let mMyProxy;

export default {
    data: {
        title: ""
    },
    onInit() {
        this.title = this.$t('strings.world');
    },
    onStart(want) {
        console.log('SerivceAbilityServer onStart');
        class MyStub extends rpc.RemoteObject{
            constructor(des) {
                if (typeof des === 'string') {
                    super(des, des.length);
                }
                return null;
            }
            onRemoteRequest(code, message, reply, option) {
                console.log("RPCTestServer onRemoteRequest code:" + code);
                if (code === 1) {
                    console.log("RPCTestServer: 调用到方法");
                    let getContextObject = rpc.IPCSkeleton.getContextObject();
                    console.log("RPCTestServer: getContextObject 方法调用的结果为: " + getContextObject);
                    let getCallingPid = rpc.IPCSkeleton.getCallingPid();
                    console.log("RPCTestServer: getCallingPid 方法调用的结果为: " + getCallingPid);
                    let getCallingUid = rpc.IPCSkeleton.getCallingUid();
                    console.log("RPCTestServer: getCallingUid 方法调用的结果为: " + getCallingUid);
                    let getCallingDeviceID = rpc.IPCSkeleton.getCallingDeviceID();
                    console.log("RPCTestServer: getCallingDeviceID 方法调用的结果为: " + getCallingDeviceID);
                    let getLocalDeviceID = rpc.IPCSkeleton.getLocalDeviceID();
                    console.log("RPCTestServer: getLocalDeviceID 方法调用的结果为: " + getLocalDeviceID);
                    let isLocalCalling = rpc.IPCSkeleton.isLocalCalling();
                    console.log("RPCTestServer: isLocalCalling 方法调用的结果为: " + isLocalCalling);
                    let remoteObject = new rpc.RemoteObject("aaa",3);
                    let flushCommands = rpc.IPCSkeleton.flushCommands(remoteObject);
                    console.log("RPCTestServer: flushCommands 方法调用的结果为: " + flushCommands);
                    let resetCallingIdentity = rpc.IPCSkeleton.resetCallingIdentity();
                    console.log("RPCTestServer: resetCallingIdentity 方法调用的结果为: " + resetCallingIdentity);
                    let setCallingIdentity = rpc.IPCSkeleton.setCallingIdentity("aaa", 3);
                    console.log("RPCTestServer: setCallingIdentity 方法调用的结果为: " + setCallingIdentity);
                    let num = message.readInt();
                    let msg = message.readString();
                    console.log("num is " + num + "msg is " + msg);
                    reply.writeString("Success");
                    return true;
                }
            }
        }

        console.log("RPCTestServer: 判断其是否可以创建对象")
        mMyStub = new MyStub("ServiceAbility-test");
        console.log("xxx RPCTestServer: 创建的对象为:" + mMyStub)
        // console.log("RPCTestServer: 向saMgr中注册stub");
        // let result = rpc.IPCSkeleton.addLocalAbility(mMyStub);
        // console.log("RPCTestServer: 注册的结果为：" + result);

        // mMyProxy = rpc.IPCSkeleton.getLocalAbility();
        // console.log("RPCTestProxy: 判断当前获取的对象是不是proxy:" + (mMyProxy instanceof rpc.RemoteProxy));
    },
    onReady() {
        console.info('SerivceAbilityServer onReady');
    },

    onStop() {
        console.info('SerivceAbilityServer onStop');
    },
    onConnect(want) {
        console.info('stub SerivceAbilityServer OnConnect');
        return mMyStub;
    },
    onReconnect(want) {
        console.info('SerivceAbilityServer onReconnect');
    },
    onDisconnect() {
        console.info('SerivceAbilityServer OnDisConnect');
    },
    onCommand(want, restart, startId) {
        console.info('SerivceAbilityServer onCommand');
    },
}