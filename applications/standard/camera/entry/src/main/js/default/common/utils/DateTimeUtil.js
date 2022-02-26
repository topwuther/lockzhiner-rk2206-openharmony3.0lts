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

import LogUtil from '../utils/LogUtil.js';

let mLogUtil = new LogUtil();

export default class DateTimeUtil {
    getTime() {
        mLogUtil.cameraInfo('getTime begin.');
        const DATETIME = new Date();
        const HOURS = DATETIME.getHours();
        const MINUTES = DATETIME.getMinutes();
        const SECONDS = DATETIME.getSeconds();
        mLogUtil.cameraInfo('getTime end.');
        return this.concatTime(HOURS, MINUTES, SECONDS);
    }

    getDate() {
        mLogUtil.cameraInfo('getDate begin.');
        const DATETIME = new Date();
        const YEAR = DATETIME.getFullYear();
        const MONTH = DATETIME.getMonth() + 1;
        const DAY = DATETIME.getDate();
        mLogUtil.cameraInfo('getDate end.');
        return this.concatDate(YEAR, MONTH, DAY);
    }

    fill(value) {
        mLogUtil.cameraInfo('fill begin.');
        return (value > 9 ? '' : '0') + value;
    }

    concatDate(year, month, date) {
        mLogUtil.cameraInfo('concatDate begin.');
        return `${year}-${month}-${date}`;
    }

    concatTime(hours, minutes, seconds) {
        mLogUtil.cameraInfo('concatTime begin.');
        return `${this.fill(hours)}:${this.fill(minutes)}:${this.fill(seconds)}`;
    }
}
