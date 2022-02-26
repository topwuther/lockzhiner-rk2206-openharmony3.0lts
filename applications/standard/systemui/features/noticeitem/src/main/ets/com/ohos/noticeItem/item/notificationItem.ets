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

import Constants from '../common/constants.ets'
import longItem from './longItem.ets'
import multiItem from './multiItem.ets'
import Log from '../../../../../../../../../common/src/main/ets/default/Log.ets'
const TAG = 'NoticeItem-NotificationItem'

@Component
export default
struct notificationItem {
  @Link showStatusBar: boolean
  private itemObject: any
  private normal: boolean = false;
  private long: boolean = false;
  private multi: boolean = false;

  aboutToAppear() {
    Log.showInfo(TAG, `aboutToDisAppear Start`)
    switch (this.itemObject.type) {
      case Constants.NOTIFICATION_TYPE_NORMAL:
        this.normal = true;
        break;
      case Constants.NOTIFICATION_TYPE_LONG:
        this.long = true;
        break;
      case Constants.NOTIFICATION_TYPE_MULTILINE:
        this.multi = true;
        break;
      default:
        break;
    }
  }

  aboutToDisappear(){
    Log.showInfo(TAG, `aboutToDisAppear`)
  }

  build() {
    Flex({ direction: FlexDirection.Column, justifyContent: FlexAlign.Start, alignItems: ItemAlign.Center }) {
      if (this.normal) {
        longItem({
          notificationId: this.itemObject.id,
          notificationIcon: this.itemObject.icon,
          notificationHashCode: this.itemObject.hashcode,
          notificationContent: this.itemObject.content,
          notificationName: this.itemObject.name,
          notificationTime: this.itemObject.time,
          notificationTitle: this.itemObject.title,
          notificationIconDisplay: this.itemObject.iconDisplay,
          notificationWant: this.itemObject.want,
          showStatusBar: $showStatusBar
        });
      }
      if (this.long) {
        longItem({
          notificationId: this.itemObject.id,
          notificationIcon: this.itemObject.icon,
          notificationHashCode: this.itemObject.hashcode,
          notificationContent: this.itemObject.content,
          notificationName: this.itemObject.name,
          notificationTime: this.itemObject.time,
          notificationTitle: this.itemObject.title,
          notificationIconDisplay: this.itemObject.iconDisplay,
          notificationWant: this.itemObject.want,
          showStatusBar: $showStatusBar
        });
      }
      if (this.multi) {
        multiItem({
          notificationId: this.itemObject.id,
          notificationIcon: this.itemObject.icon,
          notificationHashCode: this.itemObject.hashcode,
          notificationContent: this.itemObject.content,
          notificationName: this.itemObject.name,
          notificationTime: this.itemObject.time,
          notificationTitle: this.itemObject.title,
          notificationIconDisplay: this.itemObject.iconDisplay,
          notificationWant: this.itemObject.want,
          showStatusBar: $showStatusBar
        });
      }
    }
  }
}
