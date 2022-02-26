/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

import notification from '../@ohos.notification';
import image from './@ohos.multimedia.image';

/**
 * Constructs basic notifications.
 *
 * @name NotificationBasicContent
 * @since 7
 * @sysCap SystemCapability.Notification.ANS
 * @devices phone, tablet, tv, wearable, car
 * @permission N/A
 */
export interface NotificationBasicContent {
  title: string;
  text: string;
  additionalText?: string;
}

/**
 * Constructs notifications that include long text. The long text contains a maximum of 1024 characters.
 *
 * @name NotificationLongTextContent
 * @since 7
 * @sysCap SystemCapability.Notification.ANS
 * @devices phone, tablet, tv, wearable, car
 * @permission N/A
 */
export interface NotificationLongTextContent extends NotificationBasicContent {
  longText: string;
  briefText: string;
  expandedTitle: string;
}

/**
 * Constructs a notification that includes multiple lines of text.
 *
 * @name NotificationMultiLineContent
 * @since 7
 * @sysCap SystemCapability.Notification.ANS
 * @devices phone, tablet, tv, wearable, car
 * @permission N/A
 */
export interface NotificationMultiLineContent extends NotificationBasicContent {
  briefText: string;
  longTitle: string;
  lines: Array<string>;
}

/**
 * Constructs a notification that includes a picture.
 *
 * @name NotificationPictureContent
 * @since 7
 * @sysCap SystemCapability.Notification.ANS
 * @devices phone, tablet, tv, wearable, car
 * @permission N/A
 */
export interface NotificationPictureContent extends NotificationBasicContent {
  briefText: string;
  expandedTitle: string;
  picture: image.PixelMap;
}

/**
 * Constructs a notification.
 *
 * @name NotificationContent
 * @since 7
 * @sysCap SystemCapability.Notification.ANS
 * @devices phone, tablet, tv, wearable, car
 * @permission N/A
 */
export interface NotificationContent {
  contentType: notification.ContentType;
  normal?: NotificationBasicContent;
  longText?: NotificationLongTextContent;
  multiLine?: NotificationMultiLineContent;
  picture?: NotificationPictureContent;
}
