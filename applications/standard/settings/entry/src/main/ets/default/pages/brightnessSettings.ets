/**
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
import BrightnessSettingModel from '../model/brightnessImpl/BrightnessSettingModel.ets';
import HeadComponent from './component/headComponent.ets';
import ConfigData from '../common/baseUtil/ConfigData.ets';
import LogUtil from '../common/baseUtil/LogUtil.ets';
import SubscriberUtils from '../common/baseUtil/SubscriberUtil.ets';
import Router from '@system.router';

const BRIGHTNESS_MIN_VALUE = 0;
const BRIGHTNESS_MAX_VALUE = 255;
const BRIGHTNESS_STEP_VALUE = 10;
const SENT_EVENT_IN_BRIGHTNESS_PAGES = '-1';

@Entry
@Component
struct BrightnessSettings {
  @StorageLink('BrightnessSetValue') bright: number= 150;

  build() {
    Column() {
      HeadComponent({ headName: $r('app.string.brightnessTab'), isActive: true });
      Row() {
        Image($r('app.media.ic_settings_brightness_minimum'))
          .width($r('app.float.wh_value_40'))
          .height($r('app.float.wh_value_40'))
          .objectFit(ImageFit.Contain);
        Slider({
          value: this.bright,
          min: BRIGHTNESS_MIN_VALUE,
          max: BRIGHTNESS_MAX_VALUE,
          step: BRIGHTNESS_STEP_VALUE
        })
          .selectedColor(Color.Blue)
          .blockColor(Color.Blue)
          .width(ConfigData.WH_83_100)
          .onChange((number, SliderChangeMode) => {
          this.bright = Math.round(number);
          BrightnessSettingModel.setBrightnessListener(this.bright);
          BrightnessSettingModel.setStorageValue(ConfigData.BRIGHTNESS_SAVE_VALUE_KEY,this.bright);
          SubscriberUtils.sendEvent(this.bright + '', ConfigData.SENT_EVENT_BROADCAST_BRIGHTNESS_VALUE);
        });
        Image($r('app.media.ic_settings_brightness_maximum'))
          .width($r('app.float.wh_value_40'))
          .height($r('app.float.wh_value_40'))
          .objectFit(ImageFit.Contain);
      }.padding({ left: $r('app.float.wh_value_15'), right: $r('app.float.wh_value_15') });
    }
    .align(Alignment.Center)
    .width(ConfigData.WH_100_100)
    .height(ConfigData.WH_100_100);
  }

  private aboutToAppear(): void{
    LogUtil.info('settings BrightnessSettings aboutToDisappear in');
    this.bright = BrightnessSettingModel.getStorageValue(ConfigData.BRIGHTNESS_SAVE_VALUE_KEY,
      ConfigData.BRIGHTNESS_DEFAULT_VALUE);
    SubscriberUtils.sendEvent(SENT_EVENT_IN_BRIGHTNESS_PAGES, ConfigData.SENT_EVENT_BROADCAST_BRIGHTNESS_VALUE);
    SubscriberUtils.registerStatusListener();
    LogUtil.info('settings BrightnessSettings aboutToDisappear out');
  }

  onBackPress() {
    LogUtil.info('settings BrightnessSettings onBackPress in');
    Router.back();
    LogUtil.info('settings BrightnessSettings onBackPress out');
  }

  aboutToDisappear(): void{
    LogUtil.info('settings BrightnessSettings aboutToDisappear in');
    SubscriberUtils.unSubscriberListener();
    LogUtil.info('settings BrightnessSettings aboutToDisappear out');
  }
}