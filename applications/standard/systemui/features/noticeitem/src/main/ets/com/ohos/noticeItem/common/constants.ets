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
  static NOTIFICATION_TYPE_NORMAL: string = '0'
  static NOTIFICATION_TYPE_LONG: string = '1'
  static NOTIFICATION_TYPE_IMAGE: string = '2'
  static NOTIFICATION_TYPE_MULTILINE: string = '4'
  static NOTIFICATION_TYPE_SOCIAL: string = '3'
  static NOTIFICATION_TYPE_MEDIA: string = '5'
  static TEXT_MAX_LENGTH: number = 21
  static TEXT_EXPANDED_MAX_LENGTH: number = 13
  static TOUCH_TYPE_DOWN: number = 0;
  static TOUCH_TYPE_UP: number = 1;
  static TOUCH_TYPE_MOVE: number = 2;
  static HIDDEN_TRANSLATE_X: number = 150;
  static DISPLAY_TRANSLATE_X: number = -150;
  static REMOVE_TRANSLATE_X: number = 150;
  static DEFAULT_MAX_LINES: number= 1;
  static CONTENT_LINE_HEIGHT: number = 20;
}