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

import timeModel from '../timeModel.ets'
import Log from '../../../../../../../common/src/main/ets/default/Log.ets'
const TAG = 'ClockComponent-clockIcon';

@Component
export default struct ClockIcon {

  @StorageLink('time') mTime: string = '16:01'

  aboutToAppear() {
    Log.showInfo(TAG, 'aboutToAppear');
    timeModel.initTimeModel();
  }

  aboutToDisappear() {
    Log.showInfo(TAG, 'aboutToDisappear');
    timeModel.unInitTimeModel();
  }

  build() {
    Flex({alignItems: ItemAlign.Center, justifyContent: FlexAlign.Center}) {
      Text(this.mTime)
        .fontColor($r('app.color.clock_text_color'))
        .fontSize($r('app.float.clock_fontsize'))
    }
    .width($r('app.float.clock_maxwidth'))
    .height('50%')
  }
}