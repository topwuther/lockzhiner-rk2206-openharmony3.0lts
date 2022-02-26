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

import NotificationManager from '../../../../../../../../common/src/main/ets/default/abilitymanager/notificationManager.ets';
import BundleManager from '../../../../../../../../common/src/main/ets/default/abilitymanager/bundleManager.ets';
import Log from '../../../../../../../../common/src/main/ets/default/Log.ets';

const TAG = 'NotificationService'

let mNotificationList
let mSubscriber;
let mDate;
let mAppName;
let mEmptyArray;

/**
 * Responsible for notification binding and interaction between View and Model.
 */
export class NotificationService {

  constructor() {
    let tempLink = AppStorage.SetAndLink('notificationList', [])
    mNotificationList = tempLink.get()
    Log.showInfo(TAG, `NotificationService constructor ${JSON.stringify(mNotificationList)}`)
  }

  initNotificationService() {
    Log.showInfo(TAG, 'initNotificationCenter')
    mDate = new Date();
    mAppName = new Map();
    mEmptyArray = [];
    if (mSubscriber == undefined || mSubscriber == null) {
      this.createNotificationCallback();
      Log.showInfo(TAG, `initNotificationCenter, if: ${JSON.stringify(mSubscriber)}`);
    }
  }

  updateNotification() {
    Log.showInfo(TAG, `updateNotification  list: ${JSON.stringify(mNotificationList)}`);
    Log.showInfo(TAG, `updateNotification  len: ${mNotificationList.length}`);
    let listLink = AppStorage.Link('notificationList');
    listLink.set(mNotificationList);
    Log.showInfo(TAG, `updateNotification  list: ${JSON.stringify(listLink.get())}`);
  }

  /**
   * Call the method to receive notification events.
   */
  createNotificationCallback() {
    Log.showInfo(TAG, 'createNotificationCallback enter ====================');
    mSubscriber = {
      onConsume: this.consumeCallback.bind(this),
      onCancel: this.OnCancelCallback.bind(this)
    }
    Log.showInfo(TAG, `createNotificationCallback, ${JSON.stringify(mSubscriber)}`);
    Log.showInfo(TAG, `createNotificationCallback, ${JSON.stringify(mSubscriber.onConsume)}`);
    Log.showInfo(TAG, `createNotificationCallback, ${JSON.stringify(mSubscriber.onCancel)}`);
    NotificationManager.subscribeNotification(TAG, mSubscriber, this.subscribeCallback.bind(this));
    Log.showInfo(TAG, 'createNotificationCallback end ====================');
  }

  /**
     * Call the method before current process finished.
     */
  async unRegisterNotificationSubscriber() {
    if (mSubscriber != undefined && mSubscriber != null) {
      NotificationManager.unsubscribeNotification(TAG, mSubscriber);
    }
  }

  /**
   * The callback method that will be called after receiving a notification
   *
   * @param {Object} err - Error may occur.
   * @param {Object} data - Callback data.
   */
  consumeCallback(err, data) {
    Log.showInfo(TAG, 'consumeCallback started ====================');
    Log.showInfo(TAG, JSON.stringify(data.request?.wantAgent));
    Log.showInfo(TAG, JSON.stringify(data));

    let contentType = data?.request?.content?.contentType;
    if (contentType == null || contentType == undefined) {
      Log.showInfo(TAG, 'consumeCallback contentType is empty');
      return;
    }
    Log.showInfo(TAG, `contentType is : ${contentType}`);
    let request = data.request;
    let want = data.request.wantAgent;
    let standardTime = this.getStandardTime(request.deliveryTime);
    let bundleName = request.creatorBundleName
    this.getAppName(bundleName, (AppName) => {
      Log.showInfo(TAG, `notificationItem = ${want}`);
      Log.showInfo(TAG, `notificationItem = ${AppName}`);
      let appName = AppName.appName;
      let icon = AppName.icon;
      Log.showInfo(TAG, `appName = ${appName}`);
      Log.showInfo(TAG, `icon = ${icon}`);
      let notificationItem = {
        'id': request.id,
        'hashcode': request.hashCode,
        'type': contentType + '',
        'timestamp': request.deliveryTime,
        'time': standardTime,
        'name': appName,
        'want': want,
        'icon': icon,
        'bundleName': bundleName
      };
      Log.showInfo(TAG, `notificationItem = ${JSON.stringify(notificationItem)}`);
      switch (contentType) {
        case NotificationManager.TYPE_BASIC:
          Log.showInfo(TAG, `contentType NOTIFICATION_CONTENT_BASIC_TEXT ====================`);
          notificationItem['title'] = request.content.normal?.title ?? '';
          notificationItem['content'] = request.content.normal?.text ?? '';
          notificationItem['additionalText'] = request.content.normal?.additionalText ?? '';
          break;
        case NotificationManager.TYPE_LONG:
          Log.showInfo(TAG, `contentType NOTIFICATION_CONTENT_LONG_TEXT ====================`);
          notificationItem['title'] = request.content.longText?.expandedTitle ?? '';
          notificationItem['content'] = request.content.longText?.longText ?? '';
          notificationItem['additionalText'] = request.content.longText?.briefText ?? '';
          break;
        case NotificationManager.TYPE_MULTI:
          Log.showInfo(TAG, `contentType NOTIFICATION_CONTENT_MULTILINE ====================`);
          notificationItem['title'] = request.content.multiLine?.longTitle ?? '';
          notificationItem['content'] = request.content.multiLine?.lines ?? mEmptyArray;
          notificationItem['additionalText'] = request.content.multiLine?.briefText ?? '';
          break;
        default:
          Log.showInfo(TAG, 'no contentType match ====================');
          break;
      }
      Log.showInfo(TAG, `notificationItem = ${JSON.stringify(notificationItem)}`);
      this.removeNotificationItem(notificationItem['hashcode'],false);
      mNotificationList.unshift(notificationItem);
//      mNotificationList.sort(this.compare('timestamp'));
      this.updateNotification();
      Log.showInfo(TAG, 'consumeCallback finished ====================');
    });
  }

  /**
   * Get app name with bundleName.
   *
   * @param {string} bundleName - BundleName of the target app.
   */
  getAppName(bundleName, callback) {
    Log.showInfo(TAG, 'getAppName start ====================');
    if (mAppName.has(bundleName)) {
      callback(mAppName.get(bundleName));
      Log.showInfo(TAG, 'HAS BUNDLENAME');
    } else {
      BundleManager.getBundleInfo(TAG, bundleName, 0, (data) => {
        Log.showInfo(TAG, `getBundleInfo` + JSON.stringify(data));
        let labelId = data.appInfo.labelId;
        let iconId = data.appInfo.iconId;
        let appName = '';
        BundleManager.getResourceManager(TAG, bundleName, (item) => {
          Log.showInfo(TAG, `BundleManager.getResourceManager ${BundleManager.getResourceManager}`);
          item.getString(labelId, (error, value) => {
            Log.showInfo(TAG, `getString` + JSON.stringify(value));
            if (value != null && value != undefined) {
              appName = value
            }
            item.getMediaBase64(iconId, (error, MediaValue) => {
              Log.showInfo(TAG, `getMediaBase64` + JSON.stringify(MediaValue));
              if (MediaValue != null) {
                let appMessage = {
                  'appName': appName,
                  'icon': MediaValue
                }
                if (appName != null && appName != undefined && appName != '') {
                  mAppName.set(bundleName, appMessage);
                }
                callback(appMessage);
              }
            });
          })
        })
      });
      Log.showInfo(TAG, 'getAppName end ====================');
    }
  }

  /**
   * notification CancelCallback
   *
   * @param {Object} err - return err Message.
   * @param {Object} data - return data Message.
   */
  OnCancelCallback(err, data) {
    console.info("================>OnCancelCallback err : =======================> " + JSON.stringify(err));
    console.info("================>OnCancelCallback data : =======================> " + JSON.stringify(data));
    this.removeNotificationItem(data.request.hashCode,false);
  }

  onUpdateCallback(err, data) {
    Log.showInfo(TAG, `onUpdateCallback = ${JSON.stringify(data.request)}`);
  }

  /**
   * The callback will be called after registered callback.
   *
   * @param {Object} err - Error may occur.
   * @param {Object} data - Callback data.
   */
  subscribeCallback(err, data) {
        Log.showInfo(TAG, 'subscribeCallback finished ====================');
        Log.showInfo(TAG, `subscribeCallback err: ${JSON.stringify(err)}`)
        Log.showInfo(TAG, `subscribeCallback data: ${JSON.stringify(data)}`)
  }

  /**
   * Reformat the timestamp to hh:mm.
   *
   * @param {number} timestamp - Target timestamp.
   */
  getStandardTime(timestamp) {
    Log.showInfo(TAG, 'getStandardTime start ====================');
    if (timestamp == 0 || timestamp == undefined || timestamp == null) {
      return '';
    }
    mDate.setTime(timestamp);
    let hoursNumber = mDate.getHours();
    let minutesNumber = mDate.getMinutes();
    let hours = hoursNumber < 10 ? '0' + hoursNumber : hoursNumber;
    let minutes = minutesNumber < 10 ? '0' + minutesNumber : minutesNumber;
    return hours + ':' + minutes;
  }

  /**
   * Compare method will be used when sorting a two-dimensional array.
   *
   * @param {string} property - Property in the object will be sorted.
   */
  compare(params) {
    return function (item1, item2) {
      var itemTime1 = item1[params];
      var itemTime2 = item2[params];
      return itemTime2 - itemTime1;
    }
  }

  /**
     * Remove all notifications.
     */
  removeAllNotifications() {
    mNotificationList = []
    this.updateNotification()
    NotificationManager.removeAll(TAG, (err, data) => {
      Log.showInfo(TAG, `removeAll , err: ${JSON.stringify(err)}`);
      Log.showInfo(TAG, `removeAll , data: ${JSON.stringify(data)}`);
    });
  }

  removeNotificationItem(hashCode,isDelSysConent) {
    Log.showInfo(TAG, `removeNotificationItem start, hashCode: ${hashCode}`);
    for (let i = 0, len = mNotificationList.length; i < len; i++) {
      if (mNotificationList[i].hashcode == hashCode) {
        Log.showInfo(TAG, `removeNotificationItem i = ${i}`);
        mNotificationList.splice(i, 1);
//        mNotificationList.pop(i);
        break;
      }
    }
    this.updateNotification()
    if(isDelSysConent){
      this.removeSysNotificationItem(hashCode);
    }
  }
  removeSysNotificationItem(hashCode){
    NotificationManager.remove(TAG, hashCode, (err, data) => {
      Log.showInfo(TAG, `removeNotificationItem, err: ${JSON.stringify(err)}`);
      Log.showInfo(TAG, `removeNotificationItem, data: ${JSON.stringify(data)}`);
    })
    Log.showInfo(TAG, `removeNotificationItem end, length: ${mNotificationList.length}`);
  }
}

let notificationService = new NotificationService();
export default notificationService as NotificationService;