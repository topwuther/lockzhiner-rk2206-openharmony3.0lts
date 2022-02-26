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
 * @file: 底部操作栏
 */

// 弹框高度
const POP_HEIGHT = 100;

// 删除id
const DELETE_ID = -10;

// 弹框显示与隐藏延时
const POP_VISIBLE = 50;

// 触发弹框点击事件延时
const POP_ITEM_CLICK = 200;

export default {
    data: {
        isShow: true,
        dataSource: [],
        popupList: [],
        popVisible: false
    },
    computed: {
        popHeight() {
            let length = 0;
            this.popList = [];
            for (let i = 0; i < this.popupList.length; i++) {
                const item = this.popupList[i];
                if (item.visible) {
                    length++;
                }
            }
            return (length * POP_HEIGHT) + 'px';
        }
    },

    /**
    * 底部菜单点击事件
    *
    * @param {Object} item - 底部点击项
    */
    tabClick(item) {
        let self = this;
        if (!item.disabled) {
            if (item.id === DELETE_ID) {
                setTimeout(() => {
                    self.popVisible = !self.popVisible;
                    self.$emit('changePopVisible', self.popVisible);
                }, POP_VISIBLE);
            } else {
                setTimeout(() => {
                    self.popVisible = false;
                }, POP_VISIBLE);
                self.$emit('bottomTabClick', item);
            }
        }
    },

    /**
    * 弹出层点击事件
    *
    * @param {Object} item - 弹框点击项
    */
    popupItemClick(item) {
        let self = this;
        setTimeout(() => {
            self.popVisible = false;
        }, POP_VISIBLE);

        setTimeout(() => {
            if (!item.disabled) {
                self.$emit('popupItemClick', item);
            }
        }, POP_ITEM_CLICK);
    },

    /**
    * 弹出层显示隐藏
    */
    popupVisible() {
        this.popVisible = !this.popVisible;
    },

    /**
    * 获取弹出层显示隐藏
    *
    * @return {boolean} - 弹框显示与隐藏
    */
    getPopVisible() {
        return this.popVisible;
    }
};
