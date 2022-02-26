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

import {IconWidth, IconHeight} from './Uniform.ets';
import mWifiManger from '../../../../../../../../wificomponent/src/main/ets/default/WifiModel.ets';

import Log from '../../../../../../../../../common/src/main/ets/default/Log.ets';

const TAG='Control-IconComponent'
@Component
export
struct IconComponent {
  @State iconOnBG: any = $r('app.color.icon_on_bg')
  private iconOn: string = ''
  @State iconOffBG: any = $r('app.color.icon_off_bg')
  private iconOff: string = ''
  @Link OnOrOff: boolean
  @State OnOrOffShow: boolean = false
  @StorageLink('flyModelStatus') flyModelStatus: boolean = false
  @StorageLink('wifiOpenStatus') wifiOpenStatus: boolean = false


  aboutToAppear() {
    Log.showInfo(TAG,'aboutToAppear')
    mWifiManger.initWifiModel();
  }
  aboutToDisappear () {
    Log.showInfo(TAG,'aboutToDisappear')
  }

  build() {
    Column() {
      Stack() {
        Stack() {
          Flex()
            .backgroundColor(this.iconOffBG)
            .clip(new Circle({ width: IconWidth, height: IconHeight }))
            .width($r('app.float.uniform_icon_bg_width'))
            .height($r('app.float.uniform_icon_bg_height'))
          Image(this.iconOff)
            .size({ width: $r('app.float.uniform_icon_width'), height: $r('app.float.uniform_icon_height') })
            .objectFit(ImageFit.Contain)
        }
        .margin({ left: $r('app.float.icon_stack_common_margin') })

        if (this.wifiOpenStatus == true) {
          Stack() {
            Flex()
              .backgroundColor(this.iconOnBG)
              .clip(new Circle({ width: IconWidth, height: IconHeight }))
              .width($r('app.float.uniform_icon_bg_width'))
              .height($r('app.float.uniform_icon_bg_height'))
            Image(this.iconOn)
              .size({ width: $r('app.float.uniform_icon_width'), height: $r('app.float.uniform_icon_height') })
              .objectFit(ImageFit.Contain)
          }
          .margin({ left: $r('app.float.icon_stack_common_margin') })
        }
      }
      .onClick(() => {
        console.log(`IconComponent onClick ${this.flyModelStatus}`)
        if (this.wifiOpenStatus == false) {
          mWifiManger.enableWifi();
        } else {
          mWifiManger.disableWifi();
        }
      })
    }
  }
}

@Component
export
struct IconFlyComponent {
  @State iconOnBG: any = $r('app.color.icon_on_bg')
  private iconOn: string = ''
  @State iconOffBG: any = $r('app.color.icon_off_bg')
  private iconOff: string = ''
  @StorageLink('flyModelStatus') flyModelStatus: boolean = false
  @StorageLink('wifiOpenStatus') wifiOpenStatus: boolean = false

  aboutToAppear() {
    Log.showInfo(TAG,'aboutToAppear')
  }

  aboutToDisappear () {
    Log.showInfo(TAG,'aboutToDisappear')
  }

  build() {
    Column() {
      Stack() {
        Stack() {
          Flex()
            .backgroundColor(this.iconOffBG)
            .clip(new Circle({ width: IconWidth, height: IconHeight }))
            .width($r('app.float.uniform_icon_bg_width'))
            .height($r('app.float.uniform_icon_bg_height'))
          Image(this.iconOff)
            .size({ width: $r('app.float.uniform_icon_width'), height: $r('app.float.uniform_icon_height') })
            .objectFit(ImageFit.Contain)
        }
        .margin({ left: $r('app.float.icon_stack_common_margin') })

        if (this.flyModelStatus == true) {
          Stack() {
            Flex()
              .backgroundColor(this.iconOnBG)
              .clip(new Circle({ width: IconWidth, height: IconHeight }))
              .width($r('app.float.uniform_icon_bg_width'))
              .height($r('app.float.uniform_icon_bg_height'))
            Image(this.iconOn)
              .size({ width: $r('app.float.uniform_icon_width'), height: $r('app.float.uniform_icon_height') })
              .objectFit(ImageFit.Contain)
          }
          .margin({ left: $r('app.float.icon_stack_common_margin') })
        }
      }
      .onClick(() => {
        console.log(`IconFlyComponent onClick ${this.flyModelStatus}  `)
        this.flyModelStatus =!this.flyModelStatus
        console.log(`IconFlyComponent onClick ${this.flyModelStatus}  `)
        if (this.flyModelStatus == true) {
          mWifiManger.disableWifi();
        }else{
          mWifiManger.enableWifi();
        }
      })
    }
  }
}