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

import LogUtil from '../common/utils/LogUtil.js';
import MediaLibrary from '@ohos.multimedia.medialibrary';
import display from '@ohos.display';

let mLogUtil = new LogUtil();
let mMediaImage = MediaLibrary.getMediaLibraryHelper();
const ALBUMNAME = 'camera';

const SYSTEM_UI_HEIGHT = 134;
const DESIGN_WIDTH = 720.0;
const PHOTO_ASPECT_RATIO_3_TO_4 = 0;
const PHOTO_ASPECT_RATIO_9_TO_16 = 1;
const PHOTO_ASPECT_RATIO_1_TO_1 = 2;
const PHOTO_ASPECT_RATIO_15_TO_26 = 3;
const MOVIE_ASPECT_RATIO_3_TO_4 = 4;
const MOVIE_ASPECT_RATIO_9_TO_16 = 5;
const MOVIE_ASPECT_RATIO_15_TO_26 = 6;
const PREVIEW_ASPECT_RATIO_LIST = [0.75, 0.5625, 1, 0.576923, 0.75, 0.5625, 0.576923];

export default class PreviewModel {
    photoQuality = '';
    previewAspectRatio = PHOTO_ASPECT_RATIO_3_TO_4;
    proportion = 0;
    screenHeight = 0;
    screenWidth = 0;
    dialogWidth = 0;
    viewAreaStyle = {};
    dialogStyle = {};

    constructor() {

    }

    getDialogStyle(callback) {
        mLogUtil.cameraInfo('getDialogStyle begin.');
        if (this.dialogWidth !== 0) {
            this.dialogStyle.dialogWidth = this.dialogWidth;
        } else {
            this.dialogStyle.dialogWidth = DESIGN_WIDTH * 0.694;
        }
        mLogUtil.cameraInfo(`getDialogStyle this.dialogWidth= ${this.dialogWidth}`);
        this.dialogStyle.dialogTitleTextHeight = this.dialogStyle.dialogWidth * 0.16;
        this.dialogStyle.dialogTitleTextFontSize = this.dialogStyle.dialogWidth * 0.064;
        this.dialogStyle.innerBtnHeight = this.dialogStyle.dialogWidth * 0.24;
        this.dialogStyle.dialogCancelButtonFontSize = this.dialogStyle.dialogWidth * 0.064;
        this.dialogStyle.dialogDeviceListMaxHeight = this.dialogStyle.dialogWidth * 0.3;
        this.dialogStyle.deviceListItemHeight = this.dialogStyle.dialogWidth * 0.16;
        this.dialogStyle.deviceItemTitleFontSize = this.dialogStyle.dialogWidth * 0.06;
        callback(this.dialogStyle);
        mLogUtil.cameraInfo('getDialogStyle end');
    }

    getPreviewStyle(callback) {
        mLogUtil.cameraInfo('getPreviewStyle begin.');
        let self = this;
        if (this.screenHeight === 0) {
            display.getDefaultDisplay().then((dis) => {
                mLogUtil.cameraInfo('getPreviewStyle getDefaultDisplay begin.');
                self.proportion = DESIGN_WIDTH / dis.width;
                self.screenHeight = (dis.height - SYSTEM_UI_HEIGHT) * self.proportion;
                self.screenWidth = DESIGN_WIDTH;
                self.getPreviewStyle_();
                callback(self.viewAreaStyle);
                mLogUtil.cameraInfo(`getPreviewStyle screenHeight=0'${JSON.stringify(self.viewAreaStyle)}`);
            });
        } else {
            this.getPreviewStyle_();
            callback(this.viewAreaStyle);
            mLogUtil.cameraInfo(`getPreviewStyle screenHeight!=0'${JSON.stringify(self.viewAreaStyle)}`);
        }
        mLogUtil.cameraInfo('getPreviewStyle end');
    }

    getPreviewStyle_() {
        mLogUtil.cameraInfo('getPreviewStyle_ begin.');
        if (this.screenWidth / this.screenHeight <= PREVIEW_ASPECT_RATIO_LIST[this.previewAspectRatio]) {
            mLogUtil.cameraInfo('getPreviewStyle_  without margin');
            this.viewAreaStyle.previewAreaWidth = this.screenWidth;
            this.viewAreaStyle.previewAreaHeight
                = this.viewAreaStyle.previewAreaWidth / PREVIEW_ASPECT_RATIO_LIST[this.previewAspectRatio];
            this.viewAreaStyle.footerWrapMargin = 0;
        } else {
            mLogUtil.cameraInfo('getPreviewStyle_ left and right margin');
            this.viewAreaStyle.previewAreaHeight = this.screenHeight;
            this.viewAreaStyle.previewAreaWidth
                = this.viewAreaStyle.previewAreaHeight * PREVIEW_ASPECT_RATIO_LIST[this.previewAspectRatio];
            this.viewAreaStyle.footerWrapMargin = (this.screenWidth - this.viewAreaStyle.previewAreaWidth) / 2;
        }
        this.dialogWidth = this.viewAreaStyle.previewAreaWidth * 0.694;
        this.viewAreaStyle.cameraViewImageWidth = this.viewAreaStyle.previewAreaWidth * 0.1333;
        this.viewAreaStyle.cameraViewImageHeight = this.viewAreaStyle.previewAreaWidth * 0.1111;
        this.viewAreaStyle.remoteTitleTextFontSize = this.viewAreaStyle.previewAreaWidth * 0.0333;
        this.viewAreaStyle.modeSwitchHeight = this.viewAreaStyle.previewAreaWidth * 0.1417;
        this.viewAreaStyle.listItemMargin = this.viewAreaStyle.previewAreaWidth * 0.4;
        this.viewAreaStyle.modeText = this.viewAreaStyle.previewAreaWidth * 0.0333;
        this.viewAreaStyle.footBarHeight = this.viewAreaStyle.previewAreaWidth * 0.2344;
        this.viewAreaStyle.imageStyleWidth = this.viewAreaStyle.previewAreaWidth * 0.1;
        this.viewAreaStyle.imageStyleHeight = this.viewAreaStyle.imageStyleWidth;
        this.viewAreaStyle.imageStyleBorderRadius = this.viewAreaStyle.previewAreaWidth * 0.05;
        this.viewAreaStyle.shootWidth = this.viewAreaStyle.previewAreaWidth * 0.1889;
        this.viewAreaStyle.shootHeight = this.viewAreaStyle.shootWidth;
        this.viewAreaStyle.shootBorderRadius = this.viewAreaStyle.previewAreaWidth * 0.0944;
        this.viewAreaStyle.smallSwitchCameraWidth = this.viewAreaStyle.previewAreaWidth * 0.0667;
        this.viewAreaStyle.smallSwitchCameraHeight = this.viewAreaStyle.smallSwitchCameraWidth;
        this.viewAreaStyle.smallSwitchCameraBorderRadius = this.viewAreaStyle.smallSwitchCameraWidth / 2;
        this.viewAreaStyle.switchCameraCircleWidth = this.viewAreaStyle.previewAreaWidth * 0.1;
        this.viewAreaStyle.switchCameraCircleHeight = this.viewAreaStyle.switchCameraCircleWidth;
        this.viewAreaStyle.switchCameraCircleBorderRadius = this.viewAreaStyle.previewAreaWidth * 0.05;
        this.viewAreaStyle.borderStyleBorder = this.viewAreaStyle.previewAreaWidth * 0.0056;
        mLogUtil.cameraInfo('getPreviewStyle_ end');
    }

    setCurrentPreviewAspectRatio(aspectRatio) {
        mLogUtil.cameraInfo('setCurrentPreviewAspectRatio begin.');
        this.previewAspectRatio = aspectRatio;
        mLogUtil.cameraInfo(`setCurrentPreviewAspectRatio end.${this.previewAspectRatio}`);
    }

    getCurrentPreviewAspectRatio() {
        mLogUtil.cameraInfo(`setCurrentPreviewAspectRatio begin.${this.previewAspectRatio}`);
        return this.previewAspectRatio;
    }

    getPhotoUri() {
        return new Promise((resolve, reject) => {
            mLogUtil.cameraInfo('PreviewModel getPhotoUri begin.');
            const IMAGEARGS = {
                selections: ALBUMNAME,
                selectionArgs: ['imagealbum'],
            };
            let photoUri;
            mMediaImage.getImageAssets(IMAGEARGS, (error, value) => {
                if (error) {
                    mLogUtil.cameraError(`MediaLibrary: getImageAssets returned an error ${error.message}`);
                }
                if (value === undefined) {
                    mLogUtil.cameraError(`MediaLibrary: There are no images in ${IMAGEARGS.selections} folder`);
                } else {
                    mLogUtil.cameraInfo(`MediaLibrary: There are images in ${IMAGEARGS.selections} folder`);
                    photoUri = `file://${value[0].URI}`;
                    mLogUtil.cameraInfo(`MediaLibrary: initialize photoUri ${photoUri}`);
                }
                resolve(photoUri);
            });
        });
    }

    getPhotoQuality() {
        return this.photoQuality;
    }
}