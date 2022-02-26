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

import Constants from './common/constants.ets'
import Log from '../../../../../../../common/src/main/ets/default/Log.ets'
import BatteryIcon from '../../../../../../../features/batterycomponent/src/main/ets/default/pages/batteryIcon.ets'
import ClockIcon from '../../../../../../../features/clockcomponent/src/main/ets/default/pages/clockIcon.ets'
import WifiIcon from '../../../../../../../features/wificomponent/src/main/ets/default/pages/wifiIcon.ets'
import SignalIcon from '../../../../../../../features/signalcomponent/src/main/ets/default/pages/signalIcon.ets'
import ConfigReader from '../common/util/configReader.ets'

const TAG = 'StatusBar-TwoGroupLayout'

let mConfigReader;

@Component
export default
struct Index {
  @StorageLink('notificationList') notificationList: any = []
  @StorageLink('layoutConfig') layoutConfig: any = {}
  @StorageLink('wifiStatus') wifiStatus: boolean = false

  aboutToAppear() {
    if (mConfigReader == null || mConfigReader == undefined) {
      mConfigReader = new ConfigReader()
      mConfigReader.readLayoutConfig()
    }
    Log.showInfo(TAG, `aboutToAppear Start`)
  }

  aboutToDisappear() {
    Log.showInfo(TAG, `aboutToDisAppear`)
  }

  build() {
    Row() {
      ForEach(this.layoutConfig.Groups, (item: any) => {
        Row() {
          ForEach(item.Components, (item: string) => {
            if (item == Constants.WIFI) {
              WifiIcon()
            }
            if (item == Constants.SIGNAL) {
              SignalIcon()
            }
            if (item == Constants.CLOCK) {
              ClockIcon()
            }
            if (item == Constants.BATTERY) {
              BatteryIcon()
            }
            if (item == Constants.NOTIFICATION) {
              Row() {
                if (this.notificationList.length > 3) {
                  ForEach(this.notificationList.slice(0, 3), (item: any) => {
                    Image(item.icon)
                      .objectFit(ImageFit.ScaleDown)
                      .height('50%')
                      .width(30)
                      .margin({right:2})
                  })
                  Row() {
                    Text('...')
                      .fontSize(20)
                  }
                } else {
                  ForEach(this.notificationList, (item: any) => {
                    Image(item.icon)
                      .objectFit(ImageFit.ScaleDown)
                      .height('50%')
                      .width(30)
                      .margin({right:2})
                  })
                }
              }
              .width(this.notificationList.length > 3 ? 34 * 4 : 34 * this.notificationList.length)
            }
          })
        }.constraintSize({ maxWidth: 200 })

        if (item.id < this.layoutConfig.Groups.length) {
          Text('')
            .layoutWeight(1)
        }
      })
    }
    .width('100%')
    .height('100%')
    .backgroundColor($r("app.color.twogroup_layout_background"))
  }
}