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

import LogUtil from '../../common/utils/LogUtil.js';
import PreviewPresenter from '../../presenter/previewPresenter/PreviewPresenter.js';
import featureAbility from '@ohos.ability.featureAbility';
import Prompt from '@system.prompt';
import RouterUtil from '../../common/utils/RouterUtil.js';
import PageData from '../../common/constants/PageData.js';

const DISTRIBUTED_VIEW = PageData.DISTRIBUTED_PAGE;
let mLogUtil = new LogUtil();
let mPreviewPresenter;

export default {
    data: {
        isTouchPhoto: false,
        photoUri: '/common/media/ic_camera_thumbnail_default_white.svg',
        newestPicUri: '/common/media/ic_camera_thumbnail_default_white.svg',
        photoFontWeight: 600,
        videoFontWeight: 400,
        animationClassName: '',
        isShowFlashingState: false,
        isCreateCamera: true,
        previewHeight: 960,
        whichPage: 'DistributedPreview',
        cameraStatus: 'DistributedPreview',
        modeIndex: 0,
        mode: 'photo',
        scrollValue: 0,
        isPhotoShootButton: true,
        dialogMessage: '',
        isPromptDialogShow: false,
        isDeviceListDialogOpen: false,
        isVideoShoot: true,
        isVideoStopButton: false,
        isVideoShootButton: false,
        hazyPictureUri: '',
        isShowHazyPicture: false,
        isInSwitchingPreviewSize: false,
        isVideoStop: false,
        previewAreaWidth: 0,
        previewAreaHeight: 0,
        footerWrapMargin: 0,
        modeSwitchHeight: 0,
        listItemMargin: 0,
        modeText: 0,
        footBarHeight: 0,
        imageStyleWidth: 0,
        imageStyleHeight: 0,
        imageStyleBorderRadius: 0,
        shootWidth: 0,
        shootHeight: 0,
        shootBorderRadius: 0,
        smallSwitchCameraWidth: 0,
        smallSwitchCameraHeight: 0,
        smallSwitchCameraBorderRadius: 0,
        switchCameraCircleWidth: 0,
        switchCameraCircleHeight: 0,
        switchCameraCircleBorderRadius: 0,
        borderStyleBorder: 0
    },
    onInit() {
        mLogUtil.cameraInfo('PreviewView onInit begin.');
        mPreviewPresenter = new PreviewPresenter(
            this.$app.$def.data.previewModel,
            this.$app.$def.data.kvStoreModel,
            this.$app.$def.data.remoteDeviceModel);
        featureAbility.getWant().then((want) => {
            mLogUtil.cameraInfo(`onInit Camera featureAbility.getWant = ${JSON.stringify(want.parameters)}`);
            switch (want.parameters.request) {
                case 'startPhotoBack':
                    this.whichPage = 'ResponderPreview';
                    this.cameraStatus = 'ResponderPreview';
                    mLogUtil.cameraInfo('Camera featureAbility.getWant success');
                    break;
                default:
                    this.cameraStatus = 'DistributedPreview';
                    break;
            }
            mLogUtil.cameraInfo('onInit getWant success');
            this.responderPreviewStartedSuccess();
        }).catch((error) => {
            mLogUtil.cameraError(`Camera featureAbility.getWant fail ${error}`);
        });
        mPreviewPresenter.getPreviewStyle((data) => {
            mLogUtil.cameraInfo(`PreviewView onInit begin getPreviewStyle= ${JSON.stringify(data)}`);
            this.previewAreaHeight = data.previewAreaHeight;
            this.previewAreaWidth = data.previewAreaWidth;
            this.footerWrapMargin = data.footerWrapMargin;
            this.modeSwitchHeight = data.modeSwitchHeight;
            this.listItemMargin = data.listItemMargin;
            this.modeText = data.modeText;
            this.footBarHeight = data.footBarHeight;
            this.imageStyleWidth = data.imageStyleWidth;
            this.imageStyleHeight = data.imageStyleHeight;
            this.imageStyleBorderRadius = data.imageStyleBorderRadius;
            this.shootWidth = data.shootWidth;
            this.shootHeight = data.shootHeight;
            this.shootBorderRadius = data.shootBorderRadius;
            this.smallSwitchCameraWidth = data.smallSwitchCameraWidth;
            this.smallSwitchCameraHeight = data.smallSwitchCameraHeight;
            this.smallSwitchCameraBorderRadius = data.smallSwitchCameraBorderRadius;
            this.switchCameraCircleWidth = data.switchCameraCircleWidth;
            this.switchCameraCircleHeight = data.switchCameraCircleHeight;
            this.switchCameraCircleBorderRadius = data.switchCameraCircleBorderRadius;
            this.borderStyleBorder = data.borderStyleBorder;
            mLogUtil.cameraInfo(`PreviewView onInit end getPreviewStyle= ${JSON.stringify(data)}`);
        });
        mLogUtil.cameraInfo('PreviewView onInit end.');
    },
    onReady() {

    },
    onShow() {
        mLogUtil.cameraInfo('PreviewView onShow begin.');
        this.animationClassName = '';
        let self = this;
        this.photoUri = '/common/media/ic_camera_thumbnail_default_white.svg';
        mPreviewPresenter.getPhotoUri().then((data) => {
            if (data !== '') {
                self.photoUri = data.replace('"', '').replace('"', '');
                self.newestPicUri = self.photoUri;
            }
            mLogUtil.cameraInfo(`PreviewView onShow photoUri: ${this.photoUri}`);
        });
        mLogUtil.cameraInfo('PreviewView onShow end.');
    },
    onDestroy() {
        mLogUtil.cameraInfo('PreviewView onDestroy begin.');
        if (this.cameraStatus === 'ResponderPreview') {
            mPreviewPresenter.remoteReturnBack();
        }
        mLogUtil.cameraInfo('PreviewView onDestroy end.');
    },
    onBackPress() {
        mLogUtil.cameraInfo('PreviewView onBackPress begin.');
        if (this.isPromptDialogShow) {
            mLogUtil.cameraInfo('PreviewView isPromptDialogShow exist');
            return true;
        } else if (this.isDeviceListDialogOpen) {
            mLogUtil.cameraInfo('PreviewView closeDialogComponent');
            this.isDeviceListDialogOpen = false;
            return true;
        } else if (this.mode === 'video' && !this.isVideoShoot) {
            this.onTouchEndVideoStop();
            mLogUtil.cameraInfo('PreviewView onBackPress stopVideoShoot');
            return true;
        } else {
            mLogUtil.cameraInfo('PreviewView onBackPress withoutDialog');
            return false;
        }
        mLogUtil.cameraInfo('PreviewView onBackPress end.');
    },
    onNewRequest() {
        mLogUtil.cameraInfo('PreviewView onNewRequest begin.');
        featureAbility.getWant().then((want) => {
            mLogUtil.cameraInfo(`onNewRequest Camera featureAbility.getWant = ${JSON.stringify(want.parameters)}`);
            switch (want.parameters.request) {
                case 'startPhotoBack':
                    this.whichPage = 'ResponderPreview';
                    this.cameraStatus = 'ResponderPreview';
                    mLogUtil.cameraInfo('Camera featureAbility.getWant success');
                    break;
                default:
                    this.cameraStatus = 'DistributedPreview';
                    break;
            }
            mLogUtil.cameraInfo('onNewRequest getWant success');
            this.responderPreviewStartedSuccess();
        }).catch((error) => {
            mLogUtil.cameraError(`Camera featureAbility.getWant fail ${error}`);
        });
        mLogUtil.cameraInfo('PreviewView onNewRequest end.');
    },
    responderPreviewStartedSuccess() {
        let self = this;
        mLogUtil.cameraInfo('PreviewView responderPreviewStartedSuccess begin.');
        if (this.cameraStatus === 'ResponderPreview') {
            mPreviewPresenter.previewStartedSuccess(this.$element('CameraId'), () => {
                mLogUtil.cameraInfo('responderPreviewStartedSuccess onTouchStartPhoto.');
                self.onTouchStartPhoto();
                setTimeout(() => {
                    mLogUtil.cameraInfo('responderPreviewStartedSuccess onTouchEndPhoto.');
                    self.onTouchEndPhoto();
                }, 200);
            });
            mPreviewPresenter.registerDeviceStateChangeCallback((action, deviceId) => {
                mLogUtil.cameraInfo('PreviewView registerDeviceStateChangeCallback begin.');
                switch (action) {
                    case 'OFFLINE':
                        this.promptShowDialog(self.$t('strings.network_interruption'));
                        setTimeout(() => {
                            featureAbility.terminateSelf((error) => {
                                mLogUtil.cameraError(`PreviewView terminateSelf finished, error= ${error}`);
                            });
                        }, 3000);
                        break;
                    default:
                        break;
                }
            });
        }
        mLogUtil.cameraInfo('PreviewView responderPreviewStartedSuccess end.');
    },
    videoStopDefault() {
        mLogUtil.cameraInfo('videoStopDefault begin.');
        this.isPhotoShootButton = false;
        this.isVideoShootButton = true;
        this.isVideoStopButton = false;
        this.isVideoShoot = true;
        this.isVideoStop = true;
        mLogUtil.cameraInfo('videoStopDefault end.');
    },
    onTouchEndVideoStop() {
        mLogUtil.cameraInfo('onTouchEndVideoStop begin.');
        if (this.isInSwitchingPreviewSize) {
            mLogUtil.cameraInfo('onTouchEndVideoStop finished, in switching preview size');
            return;
        }
        mPreviewPresenter.closeRecorder(this.$element('CameraId')).then((object) => {
            if (object.result === 'success') {
                this.animationClassName = '';
                this.animationClassName = 'AnimationStyle';
                this.animationClassName = '';
                this.videoStopDefault();
            }
        });
        mLogUtil.cameraInfo('onTouchEndVideoStop end.');
    },
    onTouchEndVideoStart() {
        mLogUtil.cameraInfo('onTouchEndVideoStart begin.');
        if (this.isInSwitchingPreviewSize) {
            mLogUtil.cameraInfo('onTouchEndVideoStart finished, in switching preview size');
            return;
        }
        mPreviewPresenter.startRecorder(this.$element('CameraId'));
        this.isPhotoShootButton = false;
        this.isVideoShootButton = false;
        this.isVideoStopButton = true;
        this.isVideoShoot = false;
        mLogUtil.cameraInfo('onTouchEndVideoStart end.');
    },
    jumpToPhoto() {
        mLogUtil.cameraInfo('jumpToPhoto begin.');
        let self = this;
        if (self.mode === 'video') {
            self.mode = 'photo';
            self.isPhotoShootButton = true;
            self.isVideoShootButton = false;
            self.isVideoStopButton = false;
            self.photoFontWeight = 600;
            self.videoFontWeight = 400;
            self.modeIndex = 0;
            self.isVideoStop = false;
            self.$element('List').scrollTo({
                index: self.modeIndex
            });
        }
        mLogUtil.cameraInfo('jumpToPhoto end.');
    },
    jumpToVideo() {
        mLogUtil.cameraInfo('jumpToVideo begin.');
        let self = this;
        if (self.mode === 'photo') {
            self.mode = 'video';
            self.isPhotoShootButton = false;
            self.isVideoShootButton = true;
            self.isVideoStopButton = false;
            self.photoFontWeight = 400;
            self.videoFontWeight = 600;
            self.modeIndex = 1;
            self.isVideoStop = false;
            self.$element('List').scrollTo({
                index: self.modeIndex
            });
        }
        mLogUtil.cameraInfo('jumpToVideo end.');
    },
    scroll({scrollX: scrollXValue, scrollState: stateValue}) {
        mLogUtil.cameraInfo('scroll begin.');
        let self = this;
        self.scrollValue += scrollXValue;
        mLogUtil.cameraInfo(`scroll data: scrollValue ${self.scrollValue} stateValue ${stateValue}`);
        if (self.scrollValue < -50 && stateValue === 1 && self.mode === 'video') {
            self.jumpToPhoto();
        }
        if (self.scrollValue > 50 && stateValue === 1 && self.mode === 'photo') {
            self.jumpToVideo();
        }
        if (stateValue === 0) {
            self.scrollValue = 0;
            self.$element('List').scrollTo({
                index: self.modeIndex
            });
        }
        mLogUtil.cameraInfo('scroll end.');
    },
    switchCamera() {
        mLogUtil.cameraInfo('PreviewView switchCamera begin.');
        if (this.isInSwitchingPreviewSize) {
            mLogUtil.cameraInfo('switchCamera finished, in switching preview size');
            return;
        }
        this.isDeviceListDialogOpen = true;
        mLogUtil.cameraInfo('PreviewView switchCamera end.');
    },
    setPreviewSize(height, thumbnailUri) {
        mLogUtil.cameraInfo(`PreviewView setPreviewSize ${height}`);
        this.stashedPreviewHeight = height;
        if (this.isInSwitchingPreviewSize) {
            mLogUtil.cameraInfo('PreviewView setPreviewSize finished, in switching preview size');
            return;
        }
        this.isInSwitchingPreviewSize = true;
        mPreviewPresenter.takePhoto(this.$element('CameraId')).then((object) => {
            let self = this;
            if (object.result === 'complete') {
                self.hazyPictureUri = object.photoUri;
                self.isShowHazyPicture = true;
                self.isCreateCamera = false;
                setTimeout(() => {
                    self.previewHeight = self.stashedPreviewHeight;
                    self.isCreateCamera = true;
                    self.isShowHazyPicture = false;
                    if (thumbnailUri) {
                        self.photoUri = thumbnailUri;
                    }
                    mPreviewPresenter.deleteAlbumAsset();
                    self.isInSwitchingPreviewSize = false;
                    mLogUtil.cameraInfo(`PreviewView openCamera previewHeight= ${self.previewHeight}`);
                }, 1500);
            } else {
                self.isInSwitchingPreviewSize = false;
                mLogUtil.cameraError('PreviewView setPreviewSize failed, generate preview frame failed');
            }
        });
    },
    jumpToAlbum() {
        mLogUtil.cameraInfo('jumpToAlbum begin.');
        if (this.isInSwitchingPreviewSize) {
            mLogUtil.cameraInfo('jumpToAlbum finished, in switching preview size');
            return;
        }
        mPreviewPresenter.jumpToAlbum();
        mLogUtil.cameraInfo('jumpToAlbum end.');
    },
    onTouchStartPhoto() {
        mLogUtil.cameraInfo('onTouchStartPhoto begin.');
        if (this.isInSwitchingPreviewSize) {
            mLogUtil.cameraInfo('onTouchStartPhoto finished, in switching preview size');
            return;
        }
        this.isShowFlashingState = true;
        setTimeout(() => {
            this.isShowFlashingState = false;
        }, 100);
        mPreviewPresenter.takePhoto(this.$element('CameraId'), true).then((object) => {
            if (object.result === 'success') {
                this.photoUri = object.photoUri;
                this.newestPicUri = this.photoUri;
                this.animationClassName = '';
                this.animationClassName = 'AnimationStyle';
                this.animationClassName = '';
            }
        });
        this.isTouchPhoto = true;
        mLogUtil.cameraInfo('onTouchStartPhoto end.');
    },
    onTouchEndPhoto() {
        mLogUtil.cameraInfo('onTouchEndPhoto begin.');
        if (this.isInSwitchingPreviewSize) {
            mLogUtil.cameraInfo('onTouchEndPhoto finished, in switching preview size');
            return;
        }
        this.isTouchPhoto = false;
        mLogUtil.cameraInfo('onTouchEndPhoto end.');
    },
    deviceListDialogCancel() {
        mLogUtil.cameraInfo('PreviewView deviceListDialogCancel begin.');
        this.isDeviceListDialogOpen = false;
        mLogUtil.cameraInfo('PreviewView deviceListDialogCancel end.');
    },
    deviceListRadioChange(e) {
        mLogUtil.cameraInfo('deviceListRadioChange begin.');
        var inputValue = e.detail.inputValue;
        var event = e.detail.event;
        var deviceList = e.detail.deviceList;
        mLogUtil.cameraInfo(`Camera[IndexPage] JSON.stringify inputValue ${JSON.stringify(inputValue)}`);
        mLogUtil.cameraInfo(`Camera[IndexPage] JSON.stringify event ${JSON.stringify(event)}`);
        mLogUtil.cameraInfo(`Camera[IndexPage] deviceList ${JSON.stringify(deviceList)}`);
        let self = this;
        mPreviewPresenter.startRemoteCamera(inputValue, event, deviceList, (data) => {
            switch (data) {
                case 'backToLocalhost':
                    break;
                case 'remoteCameraStartedFail':
                    mLogUtil.cameraInfo('remoteCameraStartedFail begin.');
                    self.isDeviceListDialogOpen = false;
                    setTimeout(() => {
                        self.promptShowDialog(self.$t('strings.remote_camera_started_fail'));
                    }, 500);
                    mLogUtil.cameraInfo('remoteCameraStartedFail end.');
                    break;
                case 'remoteCameraStartedSuccess':
                    mLogUtil.cameraInfo('remoteCameraStartedSuccess begin.');
                    self.startDistributedView();
                    mLogUtil.cameraInfo('remoteCameraStartedSuccess end.');
                    break;
                default:
                    break;
            }
        });
        mLogUtil.cameraInfo('deviceListRadioChange end.');
    },
    listTouchEnd() {
        mLogUtil.cameraInfo('listTouchEnd begin.');
        this.scrollValue = 0;
        mLogUtil.cameraInfo('listTouchEnd end.');
    },
    promptShowDialog(message) {
        mLogUtil.cameraInfo('promptShowDialog begin.');
        this.isPromptDialogShow = true;
        let self = this;
        Prompt.showDialog({
            message: message,
            buttons:
            [{
                text: self.$t('strings.restore_defaults_dialog_confirm'),
                color: '#666666',
            }],
            success: function (data) {
                self.isPromptDialogShow = false;
                mLogUtil.cameraInfo(`dialog success callback ${JSON.stringify(data)}`);
            },
            cancel: function () {
                self.isPromptDialogShow = false;
                mLogUtil.cameraInfo('dialog cancel callback');
            },
        });
        mLogUtil.cameraInfo('promptShowDialog end.');
    },
    startDistributedView() {
        RouterUtil.replace(DISTRIBUTED_VIEW);
    }
};