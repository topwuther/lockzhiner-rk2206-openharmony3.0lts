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

import audio from '@ohos.multimedia.audio';
import VolumeModel from '../common/volume/VolumeModel.ets';
import Log from '../../../../../../../../../common/src/main/ets/default/Log.ets';

const TAG = 'Control-volumeComponent'
const volumeType = audio.AudioVolumeType.MEDIA;

let volumeModel = new VolumeModel();
let volume = {
  value: 7,
  minValue: 0,
  maxValue: 10,
};

@Component
export
struct MyVol {
  private audioManager = audio.getAudioManager();

  aboutToDisappear () {
    Log.showInfo(TAG,'aboutToDisappear')
  }

  initVolume() {
    volumeModel.getMaxVolume(volume, volumeType);
    volumeModel.getMinVolume(volume, volumeType);
    volumeModel.getVolume(volume, volumeType);
    Log.showInfo(TAG, `initVolume ${volume.value} ${volume.minValue} ${volume.maxValue} `);
  }


  setVolume(volumeValue, volumeType) {
    Log.showInfo(TAG, `setVolume = ${volume.value} Type = ${volumeType}`)
    audio.getAudioManager().setVolume(volumeType, volume.value).then(() => {
      Log.showInfo(TAG,'Promise returned to indicate a successful volume setting')
    })
  }

  aboutToAppear() {
    Log.showInfo(TAG,'Start init Volume!')
    this.initVolume()
    Log.showInfo(TAG,'Finished init Volume!')
  }

  build() {
    Flex({ direction: FlexDirection.Row, justifyContent: FlexAlign.Start, alignItems: ItemAlign.Center }) {
      Column({useAlign:HorizontalAlign.Center}) {
        Text($r('app.string.volume_control'))
          .fontSize($r('app.float.control_common_font_size')).alignSelf(ItemAlign.Center)

      }
      .width('15%')
      .padding({ left: $r('app.float.slider_text_padding_left') })

      Flex({ direction: FlexDirection.Row, justifyContent: FlexAlign.Center, alignItems: ItemAlign.Center }) {
        Slider({
          value: volume.value,
          min: volume.minValue,
          max: volume.maxValue,
          step: 1,
          style: SliderStyle.INSET
        })
          .size({ width: 300 })
          .width('100%')
          .blockColor(Color.Blue)
          .trackColor(Color.Grey)
          .selectedColor(Color.Blue)
          .onChange((value: number) => {
          volume.value = value
          this.setVolume(volume, volumeType)
        })
      }
      .width('85%')
    }
    .height('100%')
    .width('100%')
    .backgroundColor($r('app.color.volume_bg_color'))
    .borderRadius($r('app.float.volume_border_radius'))
  }
}