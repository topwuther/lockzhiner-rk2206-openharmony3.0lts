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
import Log from '../../../../../../common/src/main/ets/default/Log.ets'

const SHORT_LENGTH = 1;
const LOOP_TIME = 3000;
const TAG = 'ClockComponent-timeModel';

var timeInterval = null;

var mTimeLink;
var mDayLink;
var mWeekDayLink;
var mMonthLink;


export class TimeModel {

  /**
   * Init Time Model
   *
   */
  initTimeModel() {
    Log.showInfo(TAG, 'initTimeModel');
    Log.showInfo(TAG, `timeInterval : ${timeInterval} type: ${typeof timeInterval}`);
    if (timeInterval === null || timeInterval === undefined) {
      Log.showInfo(TAG, 'first initTimeModel');
      mTimeLink = AppStorage.SetAndLink('time', '');
      mDayLink = AppStorage.SetAndLink('day', '');
      mWeekDayLink = AppStorage.SetAndLink('weekDay', '');
      mMonthLink = AppStorage.SetAndLink('month', '');
      this.getCurrentDate();
    }
  }

  unInitTimeModel() {
    Log.showInfo(TAG, 'unInitTimeModel');
  }

  /**
   * Get current date and time.
   *
   */
  private getCurrentDate() {
    Log.showInfo(TAG, 'getCurrentDate');
    this.getDate();
    timeInterval = setInterval(() => {
      this.getDate();
    }, LOOP_TIME);
  }

  private getDate(){
    Log.showInfo(TAG, 'getDate');
    let date = new Date();
    Log.showInfo(TAG, `getCurrentDate ${date.toTimeString()}`);
    this.updateTime(date);
    this.updateDay(date);
    this.updateWeekDay(date);
    this.updateMonth(date);
    Log.showInfo(TAG, `updateTime time ${mTimeLink.get()}
    updateDay day: ${mDayLink.get()}
    updateWeekDay weekDay: ${mWeekDayLink.get()}
    updateMonth month: ${mMonthLink.get()}`);
  }

  /**
   * Update Time.
   *
   * @param {Object} date - Object of Date.
   */
  private updateTime(date) {
    let time = date.toTimeString().substring(0, 5);
    mTimeLink.set(time);
  }

  /**
   * Update Day.
   *
   * @param {Object} date - Object of Date.
   */
  private updateDay(date) {
    let day = date.getDate();
    mDayLink.set(day);
  }

  /**
   * Update WeekDay.
   *
   * @param {Object} date - Object of Date.
   */
  private updateWeekDay(date) {
    let weekDay = date.getDay();
    mWeekDayLink.set(weekDay);
  }

  /**
   * Update Month.
   *
   * @param {Object} date - Object of Date.
   */
  private updateMonth(date) {
    let month = (date.getMonth() + 1);
    mMonthLink.set(month);
  }
}

let timeModel = new TimeModel();
export default timeModel as TimeModel;