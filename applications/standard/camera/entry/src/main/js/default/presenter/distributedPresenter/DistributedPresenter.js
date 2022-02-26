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
import RouterUtil from '../../common/utils/RouterUtil.js';
import PageData from '../../common/constants/PageData.js';

const PREVIEW_VIEW = PageData.PREVIEW_PAGE;
let mLogUtil = new LogUtil();
let mPreviewModel
let mKvStoreModel;
let mRemoteDeviceModel;

export default class DistributedPresenter {
    constructor(previewModel, kvStoreModel, remoteDeviceModel) {
        mLogUtil.cameraInfo('DistributedPresenter constructor begin.');
        mPreviewModel = previewModel;
        mKvStoreModel = kvStoreModel;
        mRemoteDeviceModel = remoteDeviceModel;
        mKvStoreModel.setOnMessageReceivedListener(mKvStoreModel.messageData().msgFromResponderBack, () => {
            mKvStoreModel.setOffMessageReceivedListener();
            this.setCurrentDeviceId('localhost');
            RouterUtil.replace(PREVIEW_VIEW);
        });
        mLogUtil.cameraInfo('DistributedPresenter constructor end.');
    }

    stopRemoteCamera() {
        mLogUtil.cameraInfo('stopRemoteCamera begin.');
        mKvStoreModel.broadcastMessage(mKvStoreModel.messageData().msgFromDistributedBack);
        mKvStoreModel.setOffMessageReceivedListener();
        mLogUtil.cameraInfo('stopRemoteCamera end.');
    }

    remoteTakePhoto() {
        mLogUtil.cameraInfo('remoteTakePhoto begin.');
        mKvStoreModel.broadcastMessage(mKvStoreModel.messageData().msgFromDistributedTakePhoto);
        mLogUtil.cameraInfo('remoteTakePhoto end.');
    }

    registerDeviceStateChangeCallback(callback) {
        mLogUtil.cameraInfo('registerDeviceStateChangeCallback begin.');
        mRemoteDeviceModel.registerDeviceStateChangeCallback(callback);
        mLogUtil.cameraInfo('registerDeviceStateChangeCallback end.');
    }

    setCurrentDeviceId(deviceId) {
        mLogUtil.cameraInfo('DistributedPresenter setCurrentDeviceId begin.');
        mRemoteDeviceModel.setCurrentDeviceId(deviceId);
    }

    getCurrentDeviceId() {
        mLogUtil.cameraInfo('getCurrentDeviceId begin.');
        return mRemoteDeviceModel.getCurrentDeviceId();
    }

    getPreviewStyle(callback) {
        mLogUtil.cameraInfo('DistributedPresenter getPreviewStyle begin.');
        mPreviewModel.getPreviewStyle(callback);
        mLogUtil.cameraInfo('DistributedPresenter getPreviewStyle end');
    }
}