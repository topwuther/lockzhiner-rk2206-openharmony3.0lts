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
import Log from '../../../../../../../../../common/src/main/ets/default/Log.ets'
const TAG = 'NoticeItem-TitleItem'

@Component
export default
struct titleItem {
  @Prop notificationIcon: string
  @Prop notificationName: string
  @Prop notificationTime: string
  @Link srcIconDisplay: any
  @Link isExpand: boolean
  @Prop needExpand: boolean
  @State rowSpace: any = $r('app.float.titleitem_row_space')
  aboutToAppear() {
    Log.showInfo(TAG, `aboutToDisAppear Start`)
  }

  aboutToDisappear(){
    Log.showInfo(TAG, `aboutToDisAppear`)
  }

  build() {
    Row() {
      Row({ space: this.rowSpace }) {
        Image(this.notificationIcon)
          .objectFit(ImageFit.Contain)
          .width($r('app.float.title_image_width'))
          .height($r('app.float.title_image_height'))
        Text(this.notificationName)
          .fontSize($r('app.float.title_name_fontsize'))
        Text(this.notificationTime)
          .fontSize($r('app.float.title_time_fontsize'))
      }
      .width('80%')

      Flex({ direction: FlexDirection.Row, justifyContent: FlexAlign.End, alignItems: ItemAlign.Center }) {
        if (this.needExpand) {
          Image(this.srcIconDisplay)
            .objectFit(ImageFit.Contain)
            .width($r('app.float.displayicon_width'))
            .height($r('app.float.displayicon_height'))
        }
      }
      .width('20%')
    }
    .height($r('app.float.titleitem_height'))
    .width('100%')
    .onClick(() => {
      if (this.isExpand) {
        this.isExpand = false
        this.srcIconDisplay = $r('app.media.ic_notification_down')
      } else {
        this.isExpand = true
        this.srcIconDisplay = $r('app.media.ic_notification_up')
      }
    })
  }
}
