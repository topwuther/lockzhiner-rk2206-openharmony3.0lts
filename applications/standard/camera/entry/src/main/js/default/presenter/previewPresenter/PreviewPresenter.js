/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *    http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

import LogUtil from '../../common/utils/LogUtil.js';
import featureAbility from '@ohos.ability.featureAbility';
import media from '@ohos.multimedia.media';
import mediaLibrary from '@ohos.multimedia.medialibrary';

let mMedia = mediaLibrary.getMediaLibraryHelper();
let mLogUtil = new LogUtil();
let mPreviewModel;
let mKvStoreModel;
let mRemoteDeviceModel;
let mAudioPlayer;
let mTimeOUtFlag;

export default class PreviewPresenter {
    constructor(previewModel, kvStoreModel, remoteDeviceModel) {
        mPreviewModel = previewModel;
        mKvStoreModel = kvStoreModel;
        mRemoteDeviceModel = remoteDeviceModel;
        mAudioPlayer = undefined;
    }

    takePhoto(element, state) {
        mLogUtil.cameraInfo('takePhoto begin.');
        if (state) {
            this.playSound('file://system/etc/capture.ogg');
        }
        let photoUri = '';
        return new Promise((resolve, reject) => {
            element.takePhoto({
                quality: mPreviewModel.getPhotoQuality(),
                success: (res) => {
                    mLogUtil.cameraInfo(`takePhoto success: ${JSON.stringify(res)}`);
                    photoUri = res.uri;
                    if (state) {
                        resolve({
                            result: 'success',
                            photoUri: photoUri
                        });
                    }
                },
                fail: (res) => {
                    reject(new Error({
                        result: 'fail'
                    }));
                    mLogUtil.cameraError(`takePhoto fail: ${res.errormsg} ${res.errorcode}`);
                },
                complete: (res) => {
                    if (!state) {
                        resolve({
                            result: 'complete',
                            photoUri: photoUri
                        });
                    }
                    mLogUtil.cameraInfo(`takePhoto complete: ${res}`);
                },
            });
        });
        mLogUtil.cameraInfo('takePhoto end.');
    }

    async getPhotoUri() {
        mLogUtil.cameraInfo('getPhotoUri begin.');
        let photoUri = await mPreviewModel.getPhotoUri().then((data) => {
            mLogUtil.cameraInfo(`getPhotoUri: ${JSON.stringify(data)}`);
            return data;
        });
        mLogUtil.cameraInfo(`getPhotoUri: photoUri ${JSON.stringify(photoUri)}`);
        return JSON.stringify(photoUri);
    }

    jumpToAlbum() {
        mLogUtil.cameraInfo('jumpToAlbum begin.');
        let actionData = {
            uri: 'photodetail'
        };
        let paramBundleName = 'com.ohos.photos';
        let paramAbilityName = 'com.ohos.photos.MainAbility';
        let result = featureAbility.startAbility({
            want: {
                parameters: actionData,
                bundleName: paramBundleName,
                abilityName: paramAbilityName
            },
        }).then((data) => {
            mLogUtil.cameraInfo(`startAbility : success : ${JSON.stringify(data)}`);
        }).catch((error) => {
            mLogUtil.cameraError(`startAbility : fail : ${JSON.stringify(error)}`);
        });
        mLogUtil.cameraInfo(`jumpToAlbum end: ${result}`);
    }

    previewStartedSuccess(element, callback) {
        mLogUtil.cameraInfo('previewStartedSuccess begin.');
        mKvStoreModel.broadcastMessage(mKvStoreModel.messageData().msgFromResponderReady);
        mKvStoreModel.setOnMessageReceivedListener(mKvStoreModel.messageData().msgFromDistributedBack, () => {
            mLogUtil.cameraInfo('OnMessageReceived previewBack');
            featureAbility.terminateSelf((error) => {
                mLogUtil.cameraError(`previewStartedSuccess terminateSelf finished, error= ${error}`);
            });
        });
        mKvStoreModel.setOnMessageReceivedListener(mKvStoreModel.messageData().msgFromDistributedTakePhoto, () => {
            mLogUtil.cameraInfo('OnMessageReceived takePhoto');
            callback();
        });
        mLogUtil.cameraInfo('previewStartedSuccess end.');
    }

    registerDeviceStateChangeCallback(callback) {
        mLogUtil.cameraInfo('registerDeviceStateChangeCallback begin.');
        mRemoteDeviceModel.registerDeviceStateChangeCallback(callback);
        mLogUtil.cameraInfo('registerDeviceStateChangeCallback end.');
    }

    remoteReturnBack() {
        mLogUtil.cameraInfo('remoteReturnBack begin.');
        mKvStoreModel.broadcastMessage(mKvStoreModel.messageData().msgFromResponderBack);
        mLogUtil.cameraInfo('remoteReturnBack end.');
    }

    startAbilityContinuation(device, callback) {
        mLogUtil.cameraInfo(`startAbilityContinuation deviceId= ${device.deviceId} deviceName= ${device.deviceName}`);
        mTimeOUtFlag = false;
        let startedFailTimer = setTimeout(() => {
            mLogUtil.cameraInfo('remoteCameraStartedFail');
            callback('remoteCameraStartedFail');
            mTimeOUtFlag = true;
            mRemoteDeviceModel.setCurrentDeviceId('localhost');
        }, 7000);
        mKvStoreModel.setOnMessageReceivedListener(mKvStoreModel.messageData().msgFromResponderReady, () => {
            mLogUtil.cameraInfo('OnMessageReceived, remoteAbilityStarted');
            clearTimeout(startedFailTimer);
            if (!mTimeOUtFlag) {
                callback('remoteCameraStartedSuccess');
            }
        });
        featureAbility.startAbility({
            want: {
                bundleName: 'com.ohos.camera',
                abilityName: 'com.ohos.camera.MainAbility',
                deviceId: device.deviceId,
                parameters: {
                    request: 'startPhotoBack'
                }
            }
        }).then((data) => {
            mLogUtil.cameraInfo(`featureAbility.startAbility finished, ${JSON.stringify(data)}`);
        });
    }

    startRemoteCamera(inputValue, event, deviceList, callback) {
        mLogUtil.cameraInfo(`startRemoteCamera ${inputValue}, ${event.value}`);
        mRemoteDeviceModel.setCurrentDeviceId(event.value);
        if (inputValue !== event.value) {
            return;
        }
        if (event.value === 'localhost') {
            mLogUtil.cameraInfo('startRemoteCamera radioChange to localhost.');
            callback('backToLocalhost');
            return;
        }
        if (deviceList.filter(item => item.id === event.value).length === 0) {
            return;
        }
        let self = this;
        let deviceItemExist = false;
        let deviceName = '';
        let j = 0;
        mLogUtil.cameraInfo(`deviceList.length: ${mRemoteDeviceModel.deviceList.length}`);
        for (var i = 0; i < mRemoteDeviceModel.deviceList.length; i++) {
            if (mRemoteDeviceModel.deviceList[i].deviceId === event.value) {
                j = i;
                deviceItemExist = true;
                break;
            }
        }
        if (!deviceItemExist) {
            mLogUtil.cameraError('can not find radioChange device from deviceList');
            return;
        }
        if (mRemoteDeviceModel.deviceTrustedInfo[j]) {
            mLogUtil.cameraInfo('radioChange device has been authorized');
            self.startAbilityContinuation(mRemoteDeviceModel.deviceList[j], callback);
        } else {
            deviceName = mRemoteDeviceModel.deviceList[j].deviceName;
            mRemoteDeviceModel.authDevice(event.value, () => {
                mLogUtil.cameraInfo('radioChange device authorization success');
                for (var i = 0; i < mRemoteDeviceModel.deviceList.length; i++) {
                    if (mRemoteDeviceModel.deviceList[i].deviceName === deviceName) {
                        if (mRemoteDeviceModel.deviceTrustedInfo[i]) {
                            mLogUtil.cameraInfo('authorization success and startAbilityContinuation');
                            self.startAbilityContinuation(mRemoteDeviceModel.deviceList[i], callback);
                            break;
                        }
                    }
                }
            });
        }
        mLogUtil.cameraInfo('startRemoteCamera end.');
    }

    returnDeviceList() {
        mLogUtil.cameraInfo('returnDeviceList begin.');
        return mRemoteDeviceModel.deviceList;
    }

    getDeviceList(callback) {
        mLogUtil.cameraInfo('getDeviceList begin.');
        mRemoteDeviceModel.registerDeviceListCallback(callback);
        mLogUtil.cameraInfo('getDeviceList end.');
    }

    unregisterDeviceListCallback() {
        mLogUtil.cameraInfo('unregisterDeviceListCallback begin.');
        mRemoteDeviceModel.unregisterDeviceListCallback();
        mLogUtil.cameraInfo('unregisterDeviceListCallback end.');
    }

    getCurrentDeviceId() {
        mLogUtil.cameraInfo('getCurrentDeviceId begin.');
        return mRemoteDeviceModel.getCurrentDeviceId();
        mLogUtil.cameraInfo('getCurrentDeviceId end.');
    }

    playSound(soundUri) {
        mLogUtil.cameraInfo('playSound begin.');
        if (typeof (mAudioPlayer) !== 'undefined' && mAudioPlayer.src === soundUri && mAudioPlayer.state !== 'idle') {
            mAudioPlayer.play();
        } else {
            mLogUtil.cameraInfo('playSound createAudioPlayer');
            if (typeof (mAudioPlayer) !== 'undefined') {
                mAudioPlayer.release();
                mAudioPlayer = undefined;
            }
            mAudioPlayer = media.createAudioPlayer();
            mAudioPlayer.on('dataLoad', () => {
                mLogUtil.cameraInfo('playSound dataLoad callback');
                mAudioPlayer.play();
            });
            mAudioPlayer.src = soundUri;
        }
        mLogUtil.cameraInfo('playSound end.');
    }

    startRecorder(element) {
        mLogUtil.cameraInfo('startRecorder begin.');
        this.playSound('file://system/etc/record_start.ogg');
        element.startRecorder();
        mLogUtil.cameraInfo('startRecorder end.');
    }

    closeRecorder(element) {
        mLogUtil.cameraInfo('closeRecorder begin.');
        this.playSound('file://system/etc/record_stop.ogg');
        mLogUtil.cameraInfo('closeRecorder after playSound');
        return new Promise((resolve, reject) => {
            mLogUtil.cameraInfo('closeRecorder Promise begin.');
            element.closeRecorder({
                success: (res) => {
                    mLogUtil.cameraInfo(`closeRecorder success: ${JSON.stringify(res)}`);
                    resolve({
                        result: 'success',
                        photoUri: res.uri
                    });
                    mLogUtil.cameraInfo('closeRecorder Promise success end.');
                },
                fail: (res) => {
                    mLogUtil.cameraError('closeRecorder Promise fail begin.');
                    reject(new Error({
                        result: 'fail'
                    }));
                    mLogUtil.cameraError(`closeRecorder fail: ${res.errormsg} ${res.errorcode}`);
                },
                complete: (res) => {
                    mLogUtil.cameraInfo(`closeRecorder complete: ${res}`);
                },
            });
            mLogUtil.cameraInfo('closeRecorder Promise end.');
        });
    }

    deleteAlbumAsset() {
        mLogUtil.cameraInfo('deleteAlbumAsset begin.');
        let args = {
            selections: 'camera',
            selectionArgs: ['imagealbum'],
        };
        mMedia.getImageAssets(args, (error, images) => {
            mLogUtil.cameraInfo(`deleteAlbumAsset images: ${images[0].URI}`);
            images[0].commitDelete((error, commitFlag) => {
                if (commitFlag) {
                    mLogUtil.cameraInfo('deleteAlbumAsset success');
                }
            });
        });
        mLogUtil.cameraInfo('deleteAlbumAsset end.');
    }

    getPreviewStyle(callback) {
        mLogUtil.cameraInfo('previewPresenter getPreviewStyle begin.');
        mPreviewModel.getPreviewStyle(callback);
        mLogUtil.cameraInfo('previewPresenter getPreviewStyle end');
    }
}