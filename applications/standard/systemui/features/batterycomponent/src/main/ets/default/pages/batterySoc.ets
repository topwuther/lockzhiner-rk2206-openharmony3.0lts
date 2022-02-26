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

import Log from '../../../../../../../common/src/main/ets/default/Log.ets'
const TAG = 'BatteryComponent-batterySoc'
@Component
export default
struct BatterySoc {
  @StorageLink('batterySoc') batterySoc: number = 100

  aboutToAppear(){
    Log.showInfo(TAG,'aboutToAppear Start');
  }

  aboutToDisappear(){
    Log.showInfo(TAG,'aboutToDisappear');
  }

  build() {
    Flex({ direction: FlexDirection.Row, alignItems: ItemAlign.Center, justifyContent: FlexAlign.Center }) {
      Text(this.batterySoc + '%')
        .height('100%')
        .fontColor($r('app.color.battery_default_color'))
        .fontSize($r('app.float.fontSize_network_state'))
    }
  }
}
