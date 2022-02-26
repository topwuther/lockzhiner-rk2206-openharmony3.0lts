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

/**
 * @file: 新建相册弹框
 */

import prompt from '@system.prompt';

// 新建相册命名随机数
const GET_RANDOM_NAME = 5;

export default {
    data: {
        inputName: ''
    },

    /**
    * 新建相册名
    */
    onReady() {
        let str = 'abcdefghijklmnopqrstuvwxyz';
        for (let i = 0; i < GET_RANDOM_NAME; i++) {
            this.inputName += str[this.getRandomInt(0, GET_RANDOM_NAME)];
        }
    },

    /**
    * 随机获取新建相册名
    *
    * @param {number} min - 随机最小数
    * @param {number} max - 随机最大数
    * @return {number} - 随机数
    */
    getRandomInt(min, max) {
        return Math.floor(Math.random() * (max - min + 1)) + min;
    },

    /**
    * 弹框显示
    */
    show() {
        this.$element('createDialog').show();
    },

    /**
    * 弹框隐藏
    */
    close() {
        this.$element('createDialog').close();
    },

    /**
    * 输入框改变事件
    *
    * @param {string} value - 相册名
    */
    valueChange(value) {
        this.inputName = value.value;
    },

    /**
    * 设置输入框数据
    *
    * @param {string} inputName - 相册名
    */
    setInputName(inputName) {
        this.inputName = inputName;
    },

    /**
    * 获取弹窗输入框数据
    *
    * @return {string} - 相册名
    */
    getInputName() {
        return this.inputName;
    },

    /**
    * 弹窗确定
    *
    * @return {boolean} - 中断代码
    */
    query() {
        if (!this.inputName) {
            prompt.showToast({
                message: this.$t('strings.inputPlaceholder')
            });
            return false;
        }
        this.$emit('createDialogAlbum', this.inputName);
    },

    /**
    * 重命名取消
    */
    cancel() {
        this.close();
    }
};
