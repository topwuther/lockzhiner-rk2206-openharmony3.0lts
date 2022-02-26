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

import Log from '../../../../../../../common/src/main/ets/default/Log.ets'
import Constants from './common/constants.ets'
import FeatureAbilityManager from '../../../../../../../common/src/main/ets/default/abilitymanager/featureAbilityManager.ets'
import BatteryIcon from '../../../../../../../features/batterycomponent/src/main/ets/default/pages/batteryIcon.ets'
import ClockIcon from '../../../../../../../features/clockcomponent/src/main/ets/default/pages/clockIcon.ets'
import WifiIcon from '../../../../../../../features/wificomponent/src/main/ets/default/pages/wifiIcon.ets'
import SignalIcon from '../../../../../../../features/signalcomponent/src/main/ets/default/pages/signalIcon.ets'
import notificationItem from '../../../../../../../features/noticeitem/src/main/ets/com/ohos/noticeItem/item/notificationItem.ets'
import WindowManager from '../../../../../../../common/src/main/ets/default/WindowManager.ets'
import mNotificationService from '../../../../../../../features/notificationService/src/main/ets/com/ohos/notificationservice/NotificationService.ets'

const TAG = 'StatusBar-notification'

let mWindowManager;
let mFeatureAbilityManager;

@Component
export default
struct Index {
  @Link showStatusBar: boolean
  private minHeight: number
  startX: number = 0
  startY: number = 0
  @State moveX: number = 0
  @State moveY: number = 0

  aboutToAppear() {
    mWindowManager = new WindowManager();
    mFeatureAbilityManager = new FeatureAbilityManager();
    Log.showInfo(TAG, `aboutToAppear Start`)
  }

  build() {
    Flex({ direction: FlexDirection.Column, alignItems: ItemAlign.Start, justifyContent: FlexAlign.Start }) {
      statusBar({minHeight:this.minHeight})
      quicklySetting({ showStatusBar: $showStatusBar})
      notificationItems({ showStatusBar: $showStatusBar})
    }
    .width('100%')
    .height('100%')
    .backgroundColor($r('app.color.default_background'))
    .onTouch(this.touchEvent.bind(this))
  }

  touchEvent(event: TouchEvent) {
    if (event.type == Constants.TOUCHTYPE_DOWN) {
      this.startX = event.touches[0].screenX
      this.startY = event.touches[0].screenY
      Log.showInfo(TAG, `touchStart2=======startX: ${this.startX}, startY: ${this.startY}`)
    } else if (event.type == Constants.TOUCHTYPE_MOVE) {
      this.moveX = event.touches[0].screenX - this.startX
      this.moveY = event.touches[0].screenY - this.startY
    } else if (event.type == Constants.TOUCHTYPE_UP) {
      Log.showInfo(TAG, `touchEnd2, moveX: ${this.moveX}, moveY: ${this.moveY}`)
      if (this.moveY < -30) {
        this.showStatusBar = true;
        mWindowManager.setWindowMin((result)=>{

        })
      }
    }
  }
}

@Component
struct statusBar {
  private minHeight: number

  build() {
    Flex({ direction: FlexDirection.Row, alignItems: ItemAlign.Center, justifyContent: FlexAlign.Center }) {
      Flex({ direction: FlexDirection.Row, alignItems: ItemAlign.Center, justifyContent: FlexAlign.Start }) {
        SignalIcon()
        WifiIcon()
      }

      Flex({ direction: FlexDirection.Row, alignItems: ItemAlign.Center, justifyContent: FlexAlign.End }) {
        BatteryIcon()
        ClockIcon()
      }
    }
    .width('100%')
    .height(this.minHeight)
    .backgroundColor($r('app.color.statusbar_background'))
  }
}

@Component
struct quicklySetting {
  @Link showStatusBar: boolean
  @StorageLink('time') mTime: string = '01:01'
  @StorageLink('month') mMonth: number = 1
  @StorageLink('day') mDay: number = 1
  @StorageLink('weekDay') mWeekDay: number = 1
  @State showDay: string = 'day'
  @State showMonth: string = 'month'
  @State showWeekDay: any = 'weekday'
  @StorageLink('quicklySettingH') quicklySettingH:number = 0

  aboutToAppear() {
    Log.showInfo(TAG, `quicklySetting, aboutToAppear`)
    //show month
    this.showMonth = this.alignmentNumber(this.mMonth);
    //show day
    this.showDay = this.alignmentNumber(this.mDay);
    //show weekDay
    this.showWeekDay = Constants.WEEKDAY_LIST[this.mWeekDay-1];
  }

  aboutToDisappear() {
    Log.showInfo(TAG, `aboutToDisappear, showStatusBar: ${this.showStatusBar}`)
  }

  build() {
    Flex({ direction: FlexDirection.Row, justifyContent: FlexAlign.SpaceBetween, alignItems: ItemAlign.Center }) {
      Flex({ direction: FlexDirection.Column, justifyContent: FlexAlign.Center, alignItems: ItemAlign.Start }) {
        Text(this.mTime)
          .fontColor($r('app.color.quicklysetting_time_fontcolor'))
          .fontSize($r('app.float.quicklysetting_time_fontsize'))
        Flex({direction: FlexDirection.Row, justifyContent: FlexAlign.Start, alignItems: ItemAlign.Center}){
          dateItem({number: this.showMonth, unit: Constants.MARK_MONTH})
          dateItem({number: this.showDay, unit: Constants.MARK_DAY})
          Text(this.showWeekDay)
            .fontColor($r('app.color.quicklysetting_date_fontcolor'))
            .fontSize($r('app.float.quicklysetting_date_fontsize'))
        }
      }
      .width('50%')
      .height('100%')
      .margin({ left: $r('app.float.quicklysetting_textflex_marginleft') })

      Flex({ direction: FlexDirection.Row, justifyContent: FlexAlign.End }) {
        Image($r('app.media.ic_settings'))
          .objectFit(ImageFit.Contain)
          .onClick(this.settingClick.bind(this))
      }
      .width($r('app.float.quicklysetting_image_width'))
      .height($r('app.float.quicklysetting_image_height'))
      .margin({ right: $r('app.float.quicklysetting_image_marginright') })

    }
    .width('100%')
    .height(this.quicklySettingH)
    .borderRadius($r('app.float.quicklysetting_borderradius'))
    .margin({ left: $r('app.float.quicklysetting_marginleft'), right: $r('app.float.quicklysetting_marginright') })
  }

  settingClick() {
    Log.showInfo(TAG,` settingClick `)
    this.showStatusBar = true;
    mWindowManager.setWindowMin((result)=>{
      mFeatureAbilityManager.openAbility(TAG, {
        want: {
          bundleName: 'com.ohos.settings',
          abilityName: 'com.ohos.settings.MainAbility'
        }
      });
    });
  }
  alignmentNumber(timeOrDate){
    if(timeOrDate < Constants.DIGITS){
      return '0' + timeOrDate
    }else{
      return '' + timeOrDate
    }
  }
}

@Component
struct dateItem {

  @Prop number: string
  @Prop unit: number

  build() {
    Row() {
      Text(this.number)
        .fontColor($r('app.color.quicklysetting_date_fontcolor'))
        .fontSize($r('app.float.quicklysetting_date_fontsize'))
      if (this.unit == Constants.MARK_MONTH) {
        Text($r('app.string.month'))
          .fontColor($r('app.color.quicklysetting_date_fontcolor'))
          .fontSize($r('app.float.quicklysetting_date_fontsize'))
      }
      if (this.unit == Constants.MARK_DAY) {
        Text($r('app.string.day'))
          .fontColor($r('app.color.quicklysetting_date_fontcolor'))
          .fontSize($r('app.float.quicklysetting_date_fontsize'))
      }
    }
  }
}

@Component
struct notificationItems {
  @StorageLink('notificationList') notificationList: any[] = []
  @StorageLink('notificationH') notificationH:number = 0
  @Link showStatusBar: boolean

  build() {
    Flex({ direction: FlexDirection.Column, justifyContent: FlexAlign.Start, alignItems: ItemAlign.Center }) {
      if (this.notificationList.length == 0) {
        //text: no notification
        Flex({ direction: FlexDirection.Column, justifyContent: FlexAlign.Center, alignItems: ItemAlign.Center }) {
          Text($r('app.string.nonotification_text'))
            .fontColor($r('app.color.nonotification_text_color'))
        }
      } else {
        Stack({ alignContent: Alignment.TopStart }) {
          List() {
            ForEach(this.notificationList, (item: any) => { //notificationItems
              ListItem() {
                notificationItem({ itemObject: item, showStatusBar: $showStatusBar })
              }

            }, (item: any) => item.hashcode.toString())
          }
          .onTouch((event: TouchEvent) => {
            Log.showInfo(TAG, `Touch Event ${event.type} at Point ${event.touches[0].x}, ${event.touches[0].y}`)
            event.stopPropagation();
          })

          Image($r('app.media.delete_all')) //icon: remove all notifications
          .objectFit(ImageFit.Contain)
            .width($r('app.float.deleteall_image_width'))
            .height($r('app.float.deleteall_image_height'))
            .borderRadius($r('app.float.deleteall_image_borderradius'))
            .opacity($r('app.float.deleteall_image_opacity'))
            .onClick(this.removeAllNotification.bind(this))
            .position({ x: '44.5%', y: '85%' })

          Flex() { //bottom Line
          }.width('100%').height('7%')
          .opacity($r('app.float.deleteall_image_opacity'))
          .position({ x: 0, y: '93%' })
        }
        .width('100%')
        .height('100%')
      }
    }
    .width('100%')
    .height(this.notificationH)
  }

  removeAllNotification() {
    Log.showInfo(TAG,` removeAllNotification `)
    mNotificationService.removeAllNotifications();
  }
}
