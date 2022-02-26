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

export default class Constants {

  static TOUCHTYPE_DOWN = 0;
  static TOUCHTYPE_UP = 1;
  static TOUCHTYPE_MOVE = 2;

  static WEEKDAY_LIST = [
    $r('app.string.monday'),
    $r('app.string.tuesday'),
    $r('app.string.wednesday'),
    $r('app.string.thursday'),
    $r('app.string.friday'),
    $r('app.string.saturday'),
    $r('app.string.sunday')
  ]

  static DIGITS = 10;
  static DEFAULT_MIDDLE_PART_WIDTH = 70;

  static MARK_MONTH = 0;
  static MARK_DAY = 1;

  static WIFI  = 'wifi'
  static SIGNAL = 'signal'
  static CLOCK = 'clock'
  static BATTERY = 'battery'
  static NOTIFICATION = 'notification'
}