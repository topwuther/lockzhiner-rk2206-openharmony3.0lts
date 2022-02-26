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

import mBatteryModel from '../batteryModel.ets';
import BatteryPic from './batteryPic.ets'
import BatterySoc from './batterySoc'
import Constants from '../common/constants.ets'
import Log from '../../../../../../../common/src/main/ets/default/Log.ets'

const TAG = 'BatteryComponent-batteryIcon'
@Component
export default
struct BatteryIcon {
  @StorageLink('batterySoc') batterySoc: number = 100

  aboutToAppear() {
    mBatteryModel.initBatteryModel();
    Log.showInfo(TAG,'aboutToAppear');
  }

  aboutToDisappear() {
    mBatteryModel.uninitBatteryModel();
    Log.showInfo(TAG,'aboutToDisappear');
  }

  build() {
    Flex({ direction: FlexDirection.Row, alignItems: ItemAlign.Center, justifyContent: FlexAlign.Center }) {
      Flex({ direction: FlexDirection.Row, alignItems: ItemAlign.Center, justifyContent: FlexAlign.Center }) {
        BatterySoc()
      }
      Flex({ direction: FlexDirection.Row, alignItems: ItemAlign.Center, justifyContent: FlexAlign.Center }) {
        BatteryPic()
      }
      .width('70%')
      .height('85%')
    }
    .width($r('app.float.whole_container_width'))
    .height('50%')
    .margin($r('app.float.whole_container_margin'))
  }
}
