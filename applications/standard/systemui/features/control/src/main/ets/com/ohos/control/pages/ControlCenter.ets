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

import {UpTitle} from './UpTitle.ets'
import {AirPlaneComponent} from './airPlaneComponent.ets'
import {ElseWifiComponent} from './wifiComponent.ets'
import {MyBrightness} from './brightnessComponent.ets'
import {MyVol} from './volumeComponent.ets'

import Log from '../../../../../../../../../common/src/main/ets/default/Log.ets';
const TAG='Control-ControlCenter'

@Component
export default
struct ControlCenterComponent {
  @Link showStatusBar: boolean

  aboutToAppear() {
    Log.showInfo(TAG,'aboutToAppear')
  }

  aboutToDisappear () {
    Log.showInfo(TAG,'aboutToDisappear ')
  }

  build() {
    Column() {
      Column() {
        UpTitle({ showStatusBar: $showStatusBar })
      }
      .width('100%')
      .height('10%')
      .margin({ top: $r('app.float.center_item_margin_top'),
        left: $r('app.float.center_item_margin_left'),
        right: $r('app.float.center_item_margin_right') })

      Flex({ direction: FlexDirection.Row, justifyContent: FlexAlign.SpaceBetween, alignItems: ItemAlign.Center }) {
        Flex() {
          ElseWifiComponent()
        }
        .width('48%')
        .height('100%')

        Flex() {
          AirPlaneComponent()
        }
        .width('48%')
        .height('100%')
      }
      .width('100%')
      .height($r('app.float.wifi_and_air_height'))
      .margin({ top: $r('app.float.center_item_margin_top'),
        left: $r('app.float.center_item_margin_left'),
        right: $r('app.float.center_item_margin_right') })

      Column() {
        MyBrightness()
      }
      .width('100%')
      .height('8%')
      .margin({ top: $r('app.float.center_item_margin_top'),
        left: $r('app.float.center_item_margin_left'),
        right: $r('app.float.center_item_margin_right') })

      Flex() {
        MyVol()
      }
      .width('100%')
      .height('8%')
      .margin({ top: $r('app.float.center_item_margin_top'),
        left: $r('app.float.center_item_margin_left'),
        right: $r('app.float.center_item_margin_right') })
    }
    .backgroundColor($r('app.color.control_center_bg_color'))
    .width('100%')
    .height('100%')
  }
}
