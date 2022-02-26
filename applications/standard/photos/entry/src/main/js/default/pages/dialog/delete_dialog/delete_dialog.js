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
 * @file: 删除图片、视频和相册弹框
 */

export default {
    data: {
        title: ''
    },

    /**
    * 设置标题
    *
    * @param {string} text - 标题
    */
    setTitle(text) {
        this.title = text;
    },

    /**
    * 弹框显示
    */
    show() {
        this.$element('delete_dialog').show();
    },

    /**
    * 弹框隐藏
    */
    close() {
        this.$element('delete_dialog').close();
    },

    /**
    * 确定
    */
    query() {
        this.$emit('deleteQuery');
        this.close();
    },

    /**
    * 取消
    */
    cancel() {
        this.close();
    }
};
