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

import brightness from '@ohos.brightness';
import mBrightnessManager from '../common/brightness/brightnessManager.ets';
import Log from '../../../../../../../../../common/src/main/ets/default/Log.ets';

const TAG = 'Control-brightnessComponent'


@Component
export
struct MyBrightness {
  @StorageLink('BrightnessValue') brightnessValue: number = 0;
  @State brightnessItem: any = {
    min: 0,
    max: 255,
    value: 0
  };

  aboutToAppear() {
    Log.showInfo(TAG, `Brightness  aboutToAppear ${JSON.stringify(this.brightnessItem)}`);
  }

  aboutToDisappear (){
    Log.showInfo(TAG,'aboutToDisappear')
  }

  setBrightness(value) {
    Log.showInfo(TAG, `setBrightness = ${value}`);
    mBrightnessManager.setValue(value);
  }

  build() {
    Flex({ direction: FlexDirection.Row, justifyContent: FlexAlign.Start, alignItems: ItemAlign.Center }) {
      Column() {
        Text($r('app.string.brightness_text'))
          .fontSize($r('app.float.control_common_font_size'))
      }
      .width('15%')
      .padding({ left: $r('app.float.slider_text_padding_left') })

      Flex({ direction: FlexDirection.Row, justifyContent: FlexAlign.Center, alignItems: ItemAlign.Center }) {
        Slider({
          value: this.brightnessValue,
          min: this.brightnessItem.min,
          max: this.brightnessItem.max,
          step: 1,
          style: SliderStyle.INSET
        })
          .size({ width: $r('app.float.slider_size_width') })
          .blockColor(Color.Blue)
          .trackColor(Color.Grey)
          .selectedColor(Color.Blue)
          .width('100%')
          .onChange((value: number) => {
          this.brightnessItem.value = value;
          this.setBrightness(this.brightnessItem);
        })
      }
      .width('85%')
    }
    .height('100%')
    .width('100%')
    .backgroundColor($r('app.color.brightness_bg_color'))
    .borderRadius($r('app.float.brightness_border_radius'))
  }
}