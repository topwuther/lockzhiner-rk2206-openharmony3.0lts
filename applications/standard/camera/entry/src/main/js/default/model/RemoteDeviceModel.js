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

import deviceManager from '@ohos.distributedHardware.deviceManager';
import LogUtil from '../common/utils/LogUtil.js';

var SUBSCRIBE_ID = 100;
let mLogUtil = new LogUtil();
let cameraDeviceId = 'localhost';

export default class RemoteDeviceModel {
    deviceList = [];
    deviceTrustedInfo = [];
    callbackForList;
    callbackForStateChange;
    authCallback = null;
    #deviceManager;

    constructor() {

    }

    registerDeviceManagerOn() {
        mLogUtil.cameraInfo('registerDeviceManagerOn begin.');
        let self = this;
        this.#deviceManager.on('deviceStateChange', (data) => {
            mLogUtil.cameraInfo(`Camera[RemoteDeviceModel] deviceStateChange data= ${JSON.stringify(data)}`);
            let j = 0;
            let deviceItemExist = false;
            switch (data.action) {
                case 0:
                    self.deviceStateChangeOnLine(self, j, deviceItemExist, data);
                    break;
                case 2:
                    self.deviceStateChangeReady(self, data);
                    break;
                case 1:
                    self.deviceStateChangeOffLine(self, data);
                    break;
                default:
                    break;
            }
            mLogUtil.cameraInfo('deviceStateChange end.');
        });
        this.#deviceManager.on('deviceFound', (data) => {
            mLogUtil.cameraInfo(`Camera[RemoteDeviceModel] deviceFound data= ${JSON.stringify(data)}`);
            mLogUtil.cameraInfo(`Camera[RemoteDeviceModel] deviceFound self.deviceList= ${self.deviceList}`);
            for (var i = 0; i < self.deviceList.length; i++) {
                if (self.deviceList[i].deviceId === data.device.deviceId) {
                    mLogUtil.cameraInfo('Camera[RemoteDeviceModel] device founded, ignored');
                    return;
                }
            }
            mLogUtil.cameraInfo(`Camera[RemoteDeviceModel] deviceFound data.device= ${JSON.stringify(data.device)}`);
            self.deviceList[self.deviceList.length] = data.device;
            mLogUtil.cameraInfo(`deviceList information ${JSON.stringify(self.deviceList)} ${self.deviceList.length}`);
            self.deviceTrustedInfo[self.deviceTrustedInfo.length] = false;
            self.callbackForList();
            mLogUtil.cameraInfo('deviceFound end.');
        });
        this.#deviceManager.on('discoverFail', (data) => {
            mLogUtil.cameraInfo(`Camera[RemoteDeviceModel] discoverFail data= ${JSON.stringify(data)}`);
        });
        this.#deviceManager.on('serviceDie', () => {
            mLogUtil.cameraInfo('serviceDie begin.');
            self.callbackForStateChange('SERVICEDIE', 0);
            mLogUtil.cameraError('Camera[RemoteDeviceModel] serviceDie');
        });
        this.startDeviceDiscovery();
        mLogUtil.cameraInfo('registerDeviceManagerOn end.');
    }

    deviceStateChangeOnLine(self, j, deviceItemExist, data) {
        mLogUtil.cameraInfo('deviceStateChangeOnLine begin.');
        for (let i = 0; i < self.deviceList.length; i++) {
            if (self.deviceList[i].deviceName === data.device.deviceName) {
                mLogUtil.cameraInfo('online device exists in deviceList');
                j = i;
                deviceItemExist = true;
                break;
            }
        }
        if (!deviceItemExist) {
            mLogUtil.cameraInfo('online device do not exist in deviceList');
            self.deviceList[self.deviceList.length] = data.device;
            self.deviceTrustedInfo[self.deviceTrustedInfo.length] = true;
            self.callbackForList();
        } else if (!self.deviceTrustedInfo[j]) {
            mLogUtil.cameraInfo('online device exists in deviceList and not auth');
            self.deviceTrustedInfo[j] = true;
            self.deviceList[j] = data.device;
            if (self.authCallback != null) {
                mLogUtil.cameraInfo('authCallback is define');
                self.authCallback();
                self.authCallback = null;
            } else {
                mLogUtil.cameraInfo('authCallback is unDefine');
            }
        }
        mLogUtil.cameraInfo(`Camera[RemoteDeviceModel] online, list= ${JSON.stringify(self.deviceList)}`);
    }

    deviceStateChangeReady(self, data) {
        mLogUtil.cameraInfo('deviceStateChangeReady begin.');
        if (self.deviceList.length > 0) {
            for (var i = 0; i < self.deviceList.length; i++) {
                if (self.deviceList[i].deviceId === data.device.deviceId) {
                    self.deviceList[i] = data.device;
                    break;
                }
            }
        }
        mLogUtil.cameraInfo(`Camera[RemoteDeviceModel] change, list= ${JSON.stringify(self.deviceList)}`);
        self.callbackForList();
        mLogUtil.cameraInfo('deviceStateChangeReady end');
    }

    deviceStateChangeOffLine(self, data) {
        mLogUtil.cameraInfo('deviceStateChangeOffLine begin');
        if (self.deviceList.length > 0) {
            for (var i = 0; i < self.deviceList.length; i++) {
                if (self.deviceList[i].deviceId === data.device.deviceId) {
                    mLogUtil.cameraInfo(`RemoteDeviceModel offline deviceId: ${data.device.deviceId}`);
                    self.deviceList.splice(i, 1);
                    self.deviceTrustedInfo.splice(i, 1);
                    self.callbackForStateChange('OFFLINE', data.device.deviceId);
                    self.callbackForList();
                    break;
                }
            }
        }
        mLogUtil.cameraInfo(`Camera[RemoteDeviceModel] offline, list= ${JSON.stringify(data.device)}`);
    }

    startDeviceDiscovery() {
        mLogUtil.cameraInfo('startDeviceDiscovery begin.');
        SUBSCRIBE_ID = Math.floor(65536 * Math.random());
        var info = {
            subscribeId: SUBSCRIBE_ID,
            mode: 0xAA,
            medium: 2,
            freq: 2,
            isSameAccount: false,
            isWakeRemote: true,
            capability: 0
        };
        mLogUtil.cameraInfo(`Camera[RemoteDeviceModel] startDeviceDiscovery ${SUBSCRIBE_ID}`);
        this.#deviceManager.startDeviceDiscovery(info);
        mLogUtil.cameraInfo('startDeviceDiscovery end');
    }

    authDevice(deviceId, callback) {
        mLogUtil.cameraInfo(`Camera[RemoteDeviceModel] authDevice ${deviceId}`);
        for (var i = 0; i < this.deviceList.length; i++) {
            if (this.deviceList[i].deviceId === deviceId) {
                if (this.deviceTrustedInfo[i]) {
                    return;
                }
                mLogUtil.cameraInfo(JSON.stringify(this.deviceList[i]));
                let self = this;
                let j = i;
                setTimeout(() => {
                    let extraInfo = {
                        "targetPkgName": 'com.ohos.camera',
                        "appName": 'Camera',
                        "appDescription": 'Camera player application',
                        "business": '0'
                    };
                    let authParam = {
                        "authType": 1,
                        "appIcon": '',
                        "appThumbnail": '',
                        "extraInfo": extraInfo
                    };
                    mLogUtil.cameraInfo(`Camera authenticateDevice= ${JSON.stringify(self.deviceList[j])}`);
                    self.#deviceManager.authenticateDevice(self.deviceList[j], authParam, (err, data) => {
                        mLogUtil.cameraInfo('authResult begin.');
                        mLogUtil.cameraInfo(`Camera[RemoteDeviceModel] authResult data= ${JSON.stringify(data)}`);
                        if (err) {
                            mLogUtil.cameraError(`authenticateDevice err: ${JSON.stringify(err)}`);
                            self.authCallback = null;
                        } else {
                            mLogUtil.cameraInfo(`authenticateDevice succeed, data= ${JSON.stringify(data)}`);
                            self.authCallback = callback;
                        }
                        mLogUtil.cameraInfo('authResult end.');
                    });
                }, 0);
            }
        }
    }

    registerDeviceManagerOff() {
        mLogUtil.cameraInfo('registerDeviceManagerOff begin.');
        this.#deviceManager.stopDeviceDiscovery(SUBSCRIBE_ID);
        this.#deviceManager.off('deviceStateChange');
        this.#deviceManager.off('deviceFound');
        this.#deviceManager.off('discoverFail');
        this.#deviceManager.off('serviceDie');
        mLogUtil.cameraInfo('registerDeviceManagerOff end.');
    }

    createDeviceManager(callback) {
        mLogUtil.cameraInfo('createDeviceManager begin.');
        if (typeof (this.#deviceManager) === 'undefined') {
            let self = this;
            deviceManager.createDeviceManager('com.ohos.camera', (error, value) => {
                if (error) {
                    mLogUtil.cameraError('createDeviceManager failed.');
                    return;
                }
                self.#deviceManager = value;
                callback();
                mLogUtil.cameraError('Camera[RemoteDeviceModel] createDeviceManager callback returned');
                mLogUtil.cameraError(`error= ${error} value= ${JSON.stringify(value)}`);
            });
        } else {
            callback();
        }
        mLogUtil.cameraInfo('createDeviceManager end.');
    }

    registerDeviceListCallback(callback) {
        mLogUtil.cameraInfo('Camera[RemoteDeviceModel] registerDeviceListCallback');
        let self = this;
        this.createDeviceManager(() => {
            self.callbackForList = callback;
            if (self.#deviceManager === undefined) {
                mLogUtil.cameraError('Camera[RemoteDeviceModel] deviceManager has not initialized');
                self.callbackForList();
                return;
            }
            var list = self.#deviceManager.getTrustedDeviceListSync();
            mLogUtil.cameraInfo(`getTrustedDeviceListSync end, deviceList= ${JSON.stringify(list)}`);
            if (typeof (list) !== 'undefined' && typeof (list.length) !== 'undefined') {
                self.deviceList = list;
                for (let i = 0; i < self.deviceList.length; i++) {
                    self.deviceTrustedInfo[i] = true;
                }
                mLogUtil.cameraInfo(`self.deviceTrustedInfo ${JSON.stringify(self.deviceTrustedInfo)}`);
            }
            self.callbackForList();
            self.registerDeviceManagerOff();
            self.registerDeviceManagerOn();
        });
        mLogUtil.cameraInfo('registerDeviceListCallback end.');
    }

    unregisterDeviceListCallback() {
        mLogUtil.cameraInfo('unregisterDeviceListCallback begin.');
        this.callbackForList = null;
        this.registerDeviceManagerOff();
        if (this.callbackForStateChange !== null) {
            this.registerDeviceManagerOn();
        }
        this.deviceList = [];
        mLogUtil.cameraInfo('unregisterDeviceListCallback end.');
    }

    registerDeviceStateChangeCallback(callback) {
        mLogUtil.cameraInfo('registerDeviceStateChangeCallback begin.');
        let self = this;
        this.createDeviceManager(() => {
            self.callbackForStateChange = callback;
            if (self.#deviceManager === undefined) {
                mLogUtil.cameraError('Camera[RemoteDeviceModel] deviceManager has not initialized');
                return;
            }
            self.registerDeviceManagerOff();
            self.registerDeviceManagerOn();
        });
        mLogUtil.cameraInfo('registerDeviceStateChangeCallback end.');
    }

    unregisterDeviceStateChangeCallback() {
        mLogUtil.cameraInfo('unregisterDeviceStateChangeCallback begin.');
        this.callbackForStateChange = null;
        this.registerDeviceManagerOff();
        if (this.callbackForList !== null) {
            this.registerDeviceManagerOn();
        }
        mLogUtil.cameraInfo('unregisterDeviceStateChangeCallback end.');
    }

    setCurrentDeviceId(deviceId) {
        mLogUtil.cameraInfo('setCurrentDeviceId begin.');
        cameraDeviceId = deviceId;
        mLogUtil.cameraInfo(`setCurrentDeviceId end.${cameraDeviceId}`);
    }

    getCurrentDeviceId() {
        mLogUtil.cameraInfo(`getCurrentDeviceId begin.${cameraDeviceId}`);
        return cameraDeviceId;
    }
}