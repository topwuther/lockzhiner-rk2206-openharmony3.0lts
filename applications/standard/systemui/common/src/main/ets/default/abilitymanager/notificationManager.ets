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

//import { NotificationSubscriber } from './notification/notificationSubscriber';
import Notification from '@ohos.notification';
import Log from '../Log.ets';

const TAG = 'NotificationManager';


export default class NotificationManager {

  static TYPE_BASIC: number = Notification.ContentType.NOTIFICATION_CONTENT_BASIC_TEXT;
  static TYPE_LONG: number = Notification.ContentType.NOTIFICATION_CONTENT_LONG_TEXT;
  static TYPE_MULTI: number = Notification.ContentType.NOTIFICATION_CONTENT_MULTILINE;

  static subscribeNotification(tag, subscriber, asyncCallback) {
    Log.showInfo(TAG, `subscribeNotification from: ${tag}`));
    Notification.subscribe(subscriber, asyncCallback);
  }

  static unsubscribeNotification(tag, subscriber) {
    Log.showInfo(TAG, `subscribeNotification from: ${tag}`));
    Notification.unsubscribe(subscriber);
  }

  static removeAll(tag, callback) {
    Log.showInfo(TAG, `removeAll from: ${tag}`));
    Notification.removeAll(callback);
  }

  static remove(tag, hashCode, callback) {
    Log.showInfo(TAG, `remove from: ${tag}`));
    Notification.remove(hashCode, callback)
  }


}