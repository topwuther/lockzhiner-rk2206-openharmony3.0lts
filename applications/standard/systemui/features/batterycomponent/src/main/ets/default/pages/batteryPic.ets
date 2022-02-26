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
import Log from '../../../../../../../common/src/main/ets/default/Log.ets'
const TAG = 'BatteryComponent-batteryPic'

@Component
export default
struct BatteryPic {
  @StorageLink('batterySoc') batterySoc: number = 100
  @StorageLink('batteryCharging') batteryCharging : boolean = false
  aboutToAppear(){
    Log.showInfo(TAG,'aboutToAppear Start');
  }

  aboutToDisappear(){
    Log.showInfo(TAG,'aboutToDisappear');
  }

  build() {

    Flex({ direction: FlexDirection.Row, alignItems: ItemAlign.Center, justifyContent: FlexAlign.Center }) {
      Flex({ direction: FlexDirection.Row, alignItems: ItemAlign.Center, justifyContent: FlexAlign.Start }) {
        Flex({ direction: FlexDirection.Row, alignItems: ItemAlign.Center, justifyContent: FlexAlign.Center }) {

        }
        .height('100%')
        .width(this.batterySoc + '%')
        .backgroundColor(this.getBatteryColor(this.batterySoc, this.batteryCharging))
      }
      .width('88%')
      .height('80%')
      .backgroundColor($r('app.color.battery_background'))
      .border({ width: $r('app.float.battery_border_width'),
        color: $r('app.color.battery_border'),
        radius: $r('app.float.battery_border_radius'),
        style: BorderStyle.Solid })
      .padding($r('app.float.battery_content_padding'))

      Flex({ direction: FlexDirection.Row, alignItems: ItemAlign.Center, justifyContent: FlexAlign.Start }) {

      }
      .width('10%')
      .height('50%')
      .backgroundColor($r('app.color.battery_border'))
      .borderRadius($r('app.float.battery_border_radius'))
      .margin({ left: $r('app.float.battery_decorator_margin') })
    }
    .width('100%')
    .height('100%')
  }

  private getBatteryColor(val, charging) {
    Log.showInfo(TAG, `getBatteryColor, val: ${ val }  charging: ${ charging } `);
    if (charging) {
      return $r('app.color.battery_charging_color');
    } else if (val <= Constants.BATTERY_LEVEL_LOW) {
      return $r('app.color.battery_low_color');
    } else {
      return $r('app.color.battery_default_color');
    }
  }
}
