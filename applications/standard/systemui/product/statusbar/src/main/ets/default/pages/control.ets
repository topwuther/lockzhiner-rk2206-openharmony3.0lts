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
import ControlCenterComponent from '../../../../../../../features/control/src/main/ets/com/ohos/control/pages/ControlCenter.ets'
import WindowManager from '../../../../../../../common/src/main/ets/default/WindowManager.ets'
import BatteryIcon from '../../../../../../../features/batterycomponent/src/main/ets/default/pages/batteryIcon.ets'
import ClockIcon from '../../../../../../../features/clockcomponent/src/main/ets/default/pages/clockIcon.ets'
import WifiIcon from '../../../../../../../features/wificomponent/src/main/ets/default/pages/wifiIcon.ets'
import SignalIcon from '../../../../../../../features/signalcomponent/src/main/ets/default/pages/signalIcon.ets'
import Log from '../../../../../../../common/src/main/ets/default/Log.ets'
import Constants from './common/constants.ets'

const TAG = 'StatusBar-Control';

let mWindowManager;

@Component
export default struct Control {
  @Link showStatusBar: boolean
  private minHeight: number
  startX: number = 0
  startY: number = 0
  @State moveX: number = 0
  @State moveY: number = 0
  aboutToAppear() {
    Log.showInfo(TAG, `aboutToAppear Start`)
    mWindowManager = new WindowManager();
  }
  aboutToDisappear(){
    Log.showInfo(TAG, `aboutToDisAppear`)
  }
  build() {
    Flex({ direction: FlexDirection.Column, alignItems: ItemAlign.Start, justifyContent: FlexAlign.Start }) {
      statusBar({minHeight: this.minHeight})
      ControlCenterComponent({showStatusBar: $showStatusBar});
    }
    .width('100%')
    .height('100%')
    .backgroundColor($r('app.color.default_background'))
    .onTouch(this.controlTouch.bind(this))
  }

  controlTouch(event: TouchEvent) {
    Log.showInfo(TAG, `controlTouch================ event: ${ event }`)
    if (event.type == Constants.TOUCHTYPE_DOWN) {
      this.startX = event.touches[0].screenX
      this.startY = event.touches[0].screenY
      Log.showInfo(TAG, `controlTouch=======startX: ${this.startX}, startY: ${this.startY}`)
    } else if (event.type == Constants.TOUCHTYPE_MOVE) {
      this.moveX = event.touches[0].screenX - this.startX
      this.moveY = event.touches[0].screenY - this.startY
    } else if (event.type == Constants.TOUCHTYPE_UP) {
      Log.showInfo(TAG, `controlTouch, moveX: ${this.moveX}, moveY: ${this.moveY}`)
      if (this.moveY < -30) {
        this.showStatusBar = true;
        mWindowManager.setWindowMin((result)=>{});
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
      .margin({ left: 10 })

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
