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

import {IconFlyComponent} from './IconComponent.ets'
import Log from '../../../../../../../../../common/src/main/ets/default/Log.ets';

const TAG = 'Control-airPlaneComponent'
@Component
export
struct AirPlaneComponent {
  @State flyModel: boolean = false
  @StorageLink('flyModelStatus') flyModelStatus: boolean = this.flyModel

  aboutToAppear(){
    Log.showInfo(TAG,'aboutToAppear')
  }

  aboutToDisappear (){
    Log.showInfo(TAG,'aboutToDisappear ')
  }

  private IconInfo: any[] = [
    '',
    $r('app.media.airplane_d'),
    '',
    $r('app.media.airplane'),
  ]

  build() {
    Flex({ direction: FlexDirection.Row, alignItems: ItemAlign.Center, justifyContent: FlexAlign.Start }) {
      Flex({ direction: FlexDirection.Column, alignItems: ItemAlign.Center, justifyContent: FlexAlign.Center }) {
        IconFlyComponent({
          iconOff: this.IconInfo[1],
          iconOn: this.IconInfo[3]
        })
      }
      .height('100%')
      Flex({ direction: FlexDirection.Column, alignItems: ItemAlign.Start, justifyContent: FlexAlign.Center }) {
        Text($r('app.string.flight_mode'))
          .fontSize($r('app.float.control_common_font_size'))
          .textOverflow({ overflow: TextOverflow.Ellipsis })
          .margin({ left: $r('app.float.plane_margin_left') })
      }
      .height('100%')
    }
    .borderRadius($r('app.float.plane_border_radius'))
    .backgroundColor($r('app.color.plane_bg_color'))
    .height('100%')
    .width('100%')
  }
}