/*
 * Copyright (C) 2021 Huawei Device Co., Ltd.
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

import { AsyncCallback, ErrorCallback } from './basic';

/**
 * System time and timezone.
 *
 * @since 6
 */
declare namespace systemTime {
    /**
     * Sets the system time.
     * @permission ohos.permission.SET_TIME
     * @since 6
     */
    function setTime(time : number, callback : AsyncCallback<void>) : void;
    function setTime(time : number) : Promise<void>;

    /**
     * Sets the system time.
     * @permission ohos.permission.SET_TIME
     * @since 7
     */
    function setDate(date: Date, callback: AsyncCallback<void>): void;
    function setDate(date: Date): Promise<void>;

    /**
     * Sets the system time zone.
     * @permission ohos.permission.SET_TIME_ZONE
     * @since 7
     */
    function setTimezone(timezone: string, callback: AsyncCallback<void>): void;
    function setTimezone(timezone: string): Promise<void>;
}

export default systemTime;
