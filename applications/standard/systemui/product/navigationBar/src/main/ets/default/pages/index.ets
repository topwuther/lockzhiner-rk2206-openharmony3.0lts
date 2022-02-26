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

import Three from './threeLayout.ets'
import Log from '../../../../../../../common/src/main/ets/default/Log.ets';
import Constants from '../common/constants.ets'

const STORAGE_NAVIGATION_TYPE = 'navigationType'

let mConfigReader
const TAG = 'NavigationBar-Index';
@Entry
@Component
struct Index {
  @StorageLink('navigationLayoutConfig') mConfig: any = {}

  aboutToAppear() {
    Log.showInfo(TAG, `aboutToAppear Start`)
  }
  aboutToDisappear(){
    Log.showInfo(TAG, `aboutToDisAppear`)
  }

  build() {
    Flex({ direction: FlexDirection.Column, alignItems: ItemAlign.Center, justifyContent: FlexAlign.Center }) {
      Three();
    }
    .width('100%')
    .height('100%')
    .backgroundColor($r('app.color.index_background'))
    .padding({ left: $r('app.float.index_padding_left'), right: $r('app.float.index_padding_right') })
  }
}
