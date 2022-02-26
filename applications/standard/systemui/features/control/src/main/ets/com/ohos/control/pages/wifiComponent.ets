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

import {IconComponent} from './IconComponent.ets'
import Log from '../../../../../../../../../common/src/main/ets/default/Log.ets';

const TAG= 'Control-wifiComponent'
@Component
export
struct ElseWifiComponent {
  @StorageLink('flyModelStatus') flyModelStatus: boolean = false
  @StorageLink('wifiName') title: string = 'WLAN'
  @StorageLink('wifiOpenStatus') wifiOpenStatus: boolean = false
  aboutToAppear() {
    Log.showInfo(TAG,'aboutToAppear')
  }

  aboutToDisappear () {
    Log.showInfo(TAG,'aboutToDisappear')
  }
  
  private IconInfo: any[] = [
    'common/png/ic_notification_switch_off_bg.svg',
    $r('app.media.wifi_d'),
    '',
    $r('app.media.wifi'),
  ]

  build() {
    Flex({ direction: FlexDirection.Row, alignItems: ItemAlign.Center, justifyContent: FlexAlign.Start }) {
      Flex({ direction: FlexDirection.Column, alignItems: ItemAlign.Center, justifyContent: FlexAlign.Center }) {
        IconComponent({
          iconOff: this.IconInfo[1],
          iconOn: this.IconInfo[3],
          OnOrOff: $wifiOpenStatus
        })
      }
      .height('100%')

      Flex({ direction: FlexDirection.Column, alignItems: ItemAlign.Start, justifyContent: FlexAlign.Center }) {
        Text(this.title)
          .fontSize($r('app.float.control_common_font_size'))
          .textOverflow({ overflow: TextOverflow.Ellipsis })
          .margin({ left: $r('app.float.wifi_text_margin') })
      }
      .height('100%')
    }
    .borderRadius($r('app.float.wifi_border_radius'))
    .backgroundColor($r('app.color.wifi_bg_color'))
    .height('100%')
    .width('100%')
  }
}