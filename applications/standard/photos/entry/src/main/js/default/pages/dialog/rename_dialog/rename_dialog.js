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
 * @file: 重命名图片、视频和相册弹框
 */

export default {
    data: {
        inputName: 'modifyAlbumName'
    },

    /**
    * 弹框显示
    */
    show() {
        this.$element('renameDialog').show();
    },

    /**
    * 弹框隐藏
    */
    close() {
        this.$element('renameDialog').close();
    },

    /**
    * 设置输入框数据
    *
    * @param {string} inputName - 图片、相册和视频命名
    */
    setInputName(inputName) {
        this.inputName = inputName;
    },

    /**
    * 获取弹窗输入框数据
    *
    * @return {string} 输入框名称
    */
    getInputName() {
        return this.inputName;
    },

    /**
    * 重命名确定
    */
    query() {
        this.$emit('renameDialogAlbum', this.inputName);
    },

    /**
    * 重命名取消
    */
    cancel() {
        this.close();
    }
};
