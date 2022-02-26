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
import DistributedPresenter from '../../presenter/distributedPresenter/DistributedPresenter.js';
import Prompt from '@system.prompt';
import RouterUtil from '../../common/utils/RouterUtil.js';
import PageData from '../../common/constants/PageData.js';

const PREVIEW_VIEW = PageData.PREVIEW_PAGE;
let mLogUtil = new LogUtil();
let mDistributedPresenter;

export default {
    data: {
        isTouchPhoto: false,
        isPromptDialogShow: false,
        isDeviceListDialogOpen: false,
        previewAreaWidth: 0,
        previewAreaHeight: 0,
        cameraViewImageWidth: 0,
        cameraViewImageHeight: 0,
        footerWrapMargin: 0,
        modeSwitchHeight: 0,
        remoteTitleTextFontSize: 0,
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
        mLogUtil.cameraInfo('DistributedView onInit begin.');
        mDistributedPresenter = new DistributedPresenter(
            this.$app.$def.data.previewModel,
            this.$app.$def.data.kvStoreModel,
            this.$app.$def.data.remoteDeviceModel);
        mDistributedPresenter.registerDeviceStateChangeCallback((action, deviceId) => {
            mLogUtil.cameraInfo(`DistributedView on device state changed ${deviceId} , action ${action}`);
            switch (action) {
                case 'OFFLINE':
                    if (mDistributedPresenter.getCurrentDeviceId() === deviceId) {
                        this.promptShowDialog();
                        setTimeout(() => {
                            this.startPreviewView();
                        }, 3000);
                    }
                    break;
                default:
                    break;
            }
        });
        mDistributedPresenter.getPreviewStyle((data) => {
            mLogUtil.cameraInfo(`PreviewView onInit begin getPreviewStyle= ${JSON.stringify(data)}`);
            this.previewAreaHeight = data.previewAreaHeight;
            this.previewAreaWidth = data.previewAreaWidth;
            this.cameraViewImageWidth = data.cameraViewImageWidth;
            this.cameraViewImageHeight = data.cameraViewImageHeight;
            this.footerWrapMargin = data.footerWrapMargin;
            this.modeSwitchHeight = data.modeSwitchHeight;
            this.remoteTitleTextFontSize = data.remoteTitleTextFontSize;
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
        mLogUtil.cameraInfo('DistributedView onInit end.');
    },
    onDestroy() {
        mLogUtil.cameraInfo('DistributedView onDestroy begin.');
        mDistributedPresenter.stopRemoteCamera();
        mLogUtil.cameraInfo('DistributedView onDestroy end.');
    },
    onBackPress() {
        mLogUtil.cameraInfo('DistributedView onBackPress begin.');
        if (this.isPromptDialogShow) {
            mLogUtil.cameraInfo('DistributedView onBackPress PromptDialogExist');
            return true;
        } else if (this.isDeviceListDialogOpen) {
            mLogUtil.cameraInfo('DistributedView onBackPress DialogComponentExist');
            this.isDeviceListDialogOpen = false;
            return true;
        } else {
            mLogUtil.cameraInfo('DistributedView Router begin.');
            this.startPreviewView();
            mLogUtil.cameraInfo('DistributedView Router end.');
            return true;
        }
        mLogUtil.cameraInfo('DistributedView onBackPress end.');
    },
    deviceListDialogCancel() {
        mLogUtil.cameraInfo('DistributedView deviceListDialogCancel begin.');
        this.isDeviceListDialogOpen = false;
        mLogUtil.cameraInfo('DistributedView deviceListDialogCancel end.');
    },
    switchCamera() {
        mLogUtil.cameraInfo('DistributedView switchCamera begin.');
        this.isDeviceListDialogOpen = true;
        mLogUtil.cameraInfo('DistributedView switchCamera end.');
    },
    onTouchStart() {
        mLogUtil.cameraInfo('onTouchStart begin.');
        mDistributedPresenter.remoteTakePhoto();
        this.isTouchPhoto = true;
        mLogUtil.cameraInfo('onTouchStart end.');
    },
    onTouchEnd() {
        mLogUtil.cameraInfo('onTouchEnd begin.');
        this.isTouchPhoto = false;
        mLogUtil.cameraInfo('onTouchEnd end.');
    },
    deviceListRadioChange(e) {
        mLogUtil.cameraInfo('deviceListRadioChange begin.');
        var inputValue = e.detail.inputValue;
        var event = e.detail.event;
        mLogUtil.cameraInfo(`DistributedView Camera inputValue ${JSON.stringify(inputValue)}`);
        mLogUtil.cameraInfo(`DistributedView Camera event ${JSON.stringify(event)}`);
        if (inputValue === event.value) {
            mLogUtil.cameraInfo('DistributedView equal');
            if (event.value === 'localhost') {
                this.deviceListDialogCancel();
                this.startPreviewView();
            }
        }
        mLogUtil.cameraInfo('deviceListRadioChange end.');
    },
    promptShowDialog() {
        mLogUtil.cameraInfo('promptShowDialog begin.');
        let self = this;
        this.isPromptDialogShow = true;
        Prompt.showDialog({
            message: self.$t('strings.network_interruption'),
            buttons:
            [{
                text: self.$t('strings.restore_defaults_dialog_confirm'),
                color: '#666666',
            }],
            success: function (data) {
                self.isPromptDialogShow = false;
                mLogUtil.cameraInfo(`dialog success callback，click button : ${data.index}`);
            },
            cancel: function () {
                self.isPromptDialogShow = false;
                mLogUtil.cameraInfo('dialog cancel callback');
            },
        });
        mLogUtil.cameraInfo('promptShowDialog end.');
    },
    startPreviewView() {
        mLogUtil.cameraInfo('DistributedView startPreviewView begin.');
        RouterUtil.replace(PREVIEW_VIEW);
        mDistributedPresenter.setCurrentDeviceId('localhost');
        mLogUtil.cameraInfo('DistributedView startPreviewView end.');
    }
};