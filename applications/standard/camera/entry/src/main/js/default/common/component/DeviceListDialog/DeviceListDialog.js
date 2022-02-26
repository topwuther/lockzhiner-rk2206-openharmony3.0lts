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

import LogUtil from '../../../common/utils/LogUtil.js';
import DateTimeUtil from '../../utils/DateTimeUtil.js';

let mLogUtil = new LogUtil();
let mDateTimeUtil = new DateTimeUtil();

export default {
    props: {
        openDeviceListDialog: {
            default: false,
        }
    },
    data: {
        presentDeviceId: 'localhost',
        isDeviceListDialogOpen: false,
        deviceList: [],
        dialogWidth: 0,
        dialogTitleTextHeight: 0,
        dialogTitleTextFontSize: 0,
        innerBtnHeight: 0,
        dialogCancelButtonFontSize: 0,
        dialogDeviceListMaxHeight: 0,
        deviceListItemHeight: 0,
        deviceItemTitleFontSize: 0
    },
    onInit() {
        mLogUtil.cameraInfo('openDeviceListDialog onInit begin.');
        this.$watch('openDeviceListDialog', (newV, oldV) => {
            mLogUtil.cameraInfo(`openDeviceListDialog newV: ${newV} oldV: ${oldV}`);
            if (newV && !this.isDeviceListDialogOpen) {
                this.openDialog();
            }
            if (!newV && this.isDeviceListDialogOpen) {
                this.dismissDialog();
            }
        });
        this.$app.$def.data.previewModel.getDialogStyle((data) => {
            mLogUtil.cameraInfo(`openDeviceListDialog onInit dialogStyle data= ${JSON.stringify(data)}`);
            this.dialogWidth = data.dialogWidth;
            this.dialogTitleTextHeight = data.dialogTitleTextHeight;
            this.dialogTitleTextFontSize = data.dialogTitleTextFontSize;
            this.innerBtnHeight = data.innerBtnHeight;
            this.dialogCancelButtonFontSize = data.dialogCancelButtonFontSize;
            this.dialogDeviceListMaxHeight = data.dialogDeviceListMaxHeight;
            this.deviceListItemHeight = data.deviceListItemHeight;
            this.deviceItemTitleFontSize = data.deviceItemTitleFontSize;
            mLogUtil.cameraInfo('openDeviceListDialog onInit dialogStyle end');
        });
        mLogUtil.cameraInfo('openDeviceListDialog onInit end.');
    },
    onDestroy() {
        mLogUtil.cameraInfo('openDeviceListDialog onDestroy begin.');
        this.dismissDialog();
        mLogUtil.cameraInfo('openDeviceListDialog onDestroy end.');
    },
    onRadioChange(value, e) {
        mLogUtil.cameraInfo('radioChange begin.');
        if (value === e.value) {
            mLogUtil.cameraInfo('onRadioChange');
            this.$emit('deviceListRadioChange', {
                inputValue: value,
                event: e,
                deviceList: this.deviceList
            });
        }
        mLogUtil.cameraInfo('radioChange end.');
    },
    openDialog() {
        mLogUtil.cameraInfo('openDialog begin.');
        let self = this;
        let curRandom = Math.random();
        let curTime = mDateTimeUtil.getTime();
        let splitTime = curTime.split(':').join('');
        let curDay = mDateTimeUtil.getDate();
        let splitDay = curDay.split('-').join('');
        let curListName = `${splitDay}` + `${splitTime}` + `${curRandom}`;
        this.deviceList = [{
            name: this.$t('strings.localhost_front'),
            id: 'localhost',
            listName: curListName
        }];
        this.$app.$def.data.remoteDeviceModel.registerDeviceListCallback(() => {
            mLogUtil.cameraInfo('CameraDeviceList on remote device updated');
            mLogUtil.cameraInfo(`count= ${this.$app.$def.data.remoteDeviceModel.deviceList.length}`);
            var list = [];
            list[0] = self.deviceList[0];
            for (let [item, index] of new Map(
                                          this.$app.$def.data.remoteDeviceModel.deviceList.map(
                                              (item, i) => [item, i]))) {
                mLogUtil.cameraInfo(`CameraDevice ${index} / ${item.deviceId}
                deviceName= ${item.deviceName} deviceType= ${item.deviceType}`);
                list[index + 1] = {
                    name: item.deviceName,
                    id: item.deviceId,
                    listName: curListName
                };
            }
            self.deviceList = list;
        });
        this.presentDeviceId = this.$app.$def.data.remoteDeviceModel.getCurrentDeviceId();
        mLogUtil.cameraInfo(`switchCamera presentDeviceId: ${this.presentDeviceId}`);
        this.isDeviceListDialogOpen = true;
        this.$element('ContinueAbilityDialog').show();
        mLogUtil.cameraInfo('openDialog end.');
    },
    cancelDialog() {
        mLogUtil.cameraInfo('cancelDialog begin.');
        this.isDeviceListDialogOpen = false;
        this.$emit('deviceListDialogCancel');
        this.$app.$def.data.remoteDeviceModel.unregisterDeviceListCallback();
        mLogUtil.cameraInfo('cancelDialog end.');
    },
    onDismissDialogClicked() {
        mLogUtil.cameraInfo('onDismissDialogClicked begin.');
        this.dismissDialog();
        mLogUtil.cameraInfo('onDismissDialogClicked end.');
    },
    dismissDialog() {
        mLogUtil.cameraInfo('dismissDialog begin.');
        this.$emit('deviceListDialogCancel');
        this.isDeviceListDialogOpen = false;
        this.$element('ContinueAbilityDialog').close();
        this.$app.$def.data.remoteDeviceModel.unregisterDeviceListCallback();
        mLogUtil.cameraInfo('dismissDialog end.');
    }
};
