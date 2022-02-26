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
import ConfigurableLayout from './ConfigurableLayout.ets'
import Notification from './notification.ets'
import WindowManager from '../../../../../../../common/src/main/ets/default/WindowManager.ets'
import ConfigReader from '../common/util/configReader.ets'
import mNotificationService from '../../../../../../../features/notificationService/src/main/ets/com/ohos/notificationservice/NotificationService.ets'
import ControlCenterComponent from './control.ets'
import Constants from './common/constants.ets'
import mBrightnessManager from '../../../../../../../features/control/src/main/ets/com/ohos/control/common/brightness/brightnessManager.ets';

const TAG = 'StatusBar-Index'

let mWindowManager;

@Entry
@Component
struct Index {
  startX: number = 0
  startY: number = 0
  @State moveX: number = 0
  @State moveY: number = 0
  @State showStatusBar: boolean = true
  @StorageLink('minHeight') minHeight:number = 0
  @StorageLink('maxWidth') maxWidth:number = 0
  aboutToAppear() {
    Log.showInfo(TAG, `aboutToAppear, showStatusBar: ${this.showStatusBar}`)

    mWindowManager = new WindowManager();
    mWindowManager.initWindowManager();
    mNotificationService.initNotificationService();
    mBrightnessManager.registerBrightnessListener();
  }

  aboutToDisappear() {
    Log.showInfo(TAG, `aboutToDisappear, showStatusBar: ${this.showStatusBar}`);
    mBrightnessManager.unRegisterBrightnessListener();
  }

  build() {
    Flex({ direction: FlexDirection.Column, alignItems: ItemAlign.Center, justifyContent: FlexAlign.Center }) {
      if (this.showStatusBar) {
        Flex({ direction: FlexDirection.Row, alignItems: ItemAlign.Center, justifyContent: FlexAlign.Center }) {
          ConfigurableLayout()
        }
        .width('100%')
        .height('100%')
        .onTouch(this.touchEvent.bind(this))
      } else {
        if( this.startX <= parseInt((this.maxWidth/2).toString()) ) {
          Notification({ showStatusBar: $showStatusBar,minHeight:this.minHeight })
        } else {
          ControlCenterComponent({ showStatusBar: $showStatusBar,minHeight:this.minHeight })
        }
      }
    }
    .width('100%')
    .height('100%')
  }

  touchEvent(event: TouchEvent) {
    Log.showInfo(TAG, 'touchEvent================' + event.touches)
    Log.showInfo(TAG, 'touchEvent================ stringfy' + JSON.stringify(event.touches))
    if (event.type == Constants.TOUCHTYPE_DOWN) { //down
      this.startX = event.touches[0].screenX
      this.startY = event.touches[0].screenY
      Log.showInfo(TAG, `touchStart=======startX: ${this.startX}, startY: ${this.startY}`)
    } else if (event.type == Constants.TOUCHTYPE_MOVE) { //move
      this.moveX = event.touches[0].screenX - this.startX
      this.moveY = event.touches[0].screenY - this.startY
    } else if (event.type == Constants.TOUCHTYPE_UP) { //up
      Log.showInfo(TAG, `touchEnd, moveX: ${this.moveX}, moveY: ${this.moveY}`)
      if (this.moveY > 10) {
        this.showStatusBar = false;
        mWindowManager.setWindowMax((result)=>{});
      }
    }
  }
}
