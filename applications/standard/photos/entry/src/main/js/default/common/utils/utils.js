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
 * @file: 工具类方法
 */

// 倍数
const DOUBLE = 2;

// 月份
const MOUTH = 10;

let iconCache = {};

let isStartLog = true;

export default {
    isStartLog,

    /**
    * 获取图标
    *
    * @param {string} type - 图标
    * @return {string} - 返回图标路径
    */
    getIcon(type) {
        return iconCache[type];
    },

    /**
    * 初始化图标
    */
    initIcon() {
        iconCache['camera'] = '/common/image/svg/camera.svg';
        iconCache['selected'] = '/common/image/svg/checkbox_filled.svg';
        iconCache['unselected'] = '/common/image/svg/checkbox_white.svg';
        iconCache['video'] = '/common/image/svg/video_white_filled.svg';
        iconCache['rename'] = '/common/image/svg/rename.svg';
        iconCache['delete'] = '/common/image/svg/delete.svg';
        iconCache['more'] = '/common/image/svg/more.svg';
        iconCache['select_all'] = '/common/image/svg/select_all.svg';
        iconCache['move'] = '/common/image/svg/move.svg';
        iconCache['copy'] = '/common/image/svg/copy.svg';
        iconCache['back'] = '/common/image/svg/back.svg';
        iconCache['close'] = '/common/image/svg/close.svg';
        iconCache['info'] = '/common/image/svg/info.svg';
        iconCache['unselected_black'] = '/common/image/svg/unchecked_black.svg';
        iconCache['scaleadd'] = '/common/image/svg/scaleadd.svg';
        iconCache['scaleminus'] = '/common/image/svg/scaleminus.svg';
        iconCache['add'] = '/common/image/svg/add.svg';
        iconCache['select'] = '/common/image/icon/selected.png';
    },

    /**
    * px转dp
    *
    * @param {number} value - px值
    * @param {number} density - 屏幕密度
    * @return {number} - dp单位大小
    */
    pxTodp(value, density) {
        return value / density * DOUBLE;
    },

    /**
    * 以下函数返回 min（包含）～ max（包含）之间的数字
    *
    * @param {number} min - 最小值
    * @param {number} max - 最大值
    * @return {number} - 随机数
    */
    getRandomInt(min, max) {
        return Math.floor(Math.random() * (max - min + 1)) + min;
    },

    /**
    * logInfo
    *
    * @param {string} str - 日志
    */
    logInfo(str) {
        if (isStartLog) {
            console.info('Photos info : ' + str);
        }
    },

    /**
    * logError
    *
    * @param {string} str - 日志
    */
    logError(str) {
        if (isStartLog) {
            console.error('Photos error : ' + str);
        }
    },

    /**
    * logDebug
    *
    * @param {string} str - 日志
    */
    logDebug(str) {
        if (isStartLog) {
            console.debug('Photos debug : ' + str);
        }
    },

    /**
    * log
    *
    * @param {string} str - 日志
    */
    log(str) {
        if (isStartLog) {
            console.log('Photos log : ' + str);
        }
    },

    /**
    * logWarn
    *
    * @param {string} str - 日志
    */
    logWarn(str) {
        if (isStartLog) {
            console.warn('Photos warn : ' + str);
        }
    },

    /**
    * 设置是否开启日志
    *
    * @param {boolean} flag - 是否开启日志
    */
    setStartLog(flag) {
        isStartLog = flag;
    },

    /**
    * 获取时间拼接
    *
    * @return {string} - 时间
    */
    getTime() {
        let date = new Date();
        let Y = date.getFullYear() + '-';
        let M = (date.getMonth() + 1 < MOUTH ? '0' + (date.getMonth() + 1) : date.getMonth() + 1) + '-';
        let D = date.getDate() + '-';
        let h = date.getHours() + ':';
        let m = date.getMinutes() + ':';
        let s = date.getSeconds() + ':';
        let mill = date.getMilliseconds();
        return Y + M + D + h + m + s + mill;
    },

    /**
    * 深度拷贝
    *
    * @param {Object} obj - 被拷贝对象
    * @return {Object} copy - 目标对象
    */
    deepCopy(object) {
        let copyObj;
        if (null == object || "object" != typeof object) {
            return object;
        }
        if (object instanceof Date) {
            copyObj = new Date();
            copyObj.setTime(object.getTime());
            return copyObj;
        }

        if (object instanceof Array) {
            copyObj = [];
            for (let i = 0, len = object.length; i < len; i++) {
                copyObj[i] = this.deepCopy(object[i]);
            }
            return copyObj;
        }

        if (object instanceof Object) {
            copyObj = {};
            for (let attr in object) {
                copyObj[attr] = this.deepCopy(object[attr]);
            }
            return copyObj;
        }
    }
};