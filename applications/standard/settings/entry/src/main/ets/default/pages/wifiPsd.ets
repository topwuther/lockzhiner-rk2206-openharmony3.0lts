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
import LogUtil from '../common/baseUtil/LogUtil.ets';
import HeadComponent from './component/headComponent.ets';
import ConfigData from '../common/baseUtil/ConfigData.ets';
import Router from '@system.router'

const WIFI_DEFAULT_PSD = '12345678';

let name;
let title;

@Entry
@Component
struct WifiPsd {
  @State text: string= '';

  build() {
    Column() {
      HeadComponent({
        icBackIsVisibility: true,
        headName: null === title ? $r('app.string.wifiPassword') : title,
        isActive: true
      });

      TextInput({ placeholder: '密码', text: this.text })
        .placeholderColor(Color.Blue)
        .placeholderFont({ size: ConfigData.font_20, weight: 2, family: "sans-serif", style: FontStyle.Normal })
        .caretColor(Color.Blue)
        .height($r('app.float.wh_value_70'))
        .backgroundColor(Color.White)
        .type(InputType.Password)
        .width(ConfigData.WH_100_100)
        .margin({ left: $r('app.float.wh_value_15'), right: $r('app.float.wh_value_15') })
        .onChange((value: string) => {
        this.text = value;
      });
      Text()
        .backgroundColor(Color.Black)
        .width(ConfigData.WH_100_100)
        .height($r('app.float.wh_value_1'))
        .margin({
        left: $r('app.float.wh_value_15'),
        right: $r('app.float.wh_value_15')
      });

      Row() {
        Button() {
          Text($r('app.string.cancel')).fontSize($r('app.float.font_20'))
            .fontColor(Color.Blue)
        }
        .backgroundColor($r('app.color.color_E3E3E3_grey'))
        .width($r('app.float.wh_value_160'))
        .height($r('app.float.wh_value_40'))
        .align(Alignment.Center)
        .onClick(() => {
          Router.back();
        });
        Blank().width($r('app.float.wh_value_50'));
        Button() {
          Text($r('app.string.confirm')).fontSize($r('app.float.font_20'))
            .fontColor(Color.Blue)
        }
        .backgroundColor($r('app.color.color_E3E3E3_grey'))
        .width($r('app.float.wh_value_160'))
        .height($r('app.float.wh_value_40'))
        .align(Alignment.Center)
        .onClick(() => {
          AppStorage.SetOrCreate('wifiPsd', '' === this.text ? WIFI_DEFAULT_PSD : this.text);
          Router.back();
        });
      }
      .margin({
        left: $r('app.float.wh_value_15'),
        top: $r('app.float.wh_value_20'),
        right: $r('app.float.wh_value_15')
      })
      .width(ConfigData.WH_100_100)
      .align(Alignment.Bottom);
    }
    .backgroundColor(Color.White)
    .width(ConfigData.WH_100_100)
    .height(ConfigData.WH_100_100);
  }

  aboutToAppear(): void{
    LogUtil.info('settings wifiPsd aboutToAppear start');
    name = Router.getParams();
    LogUtil.info('settings wifiPsd aboutToAppear name:' + JSON.stringify(name));
    title = name.connectName;
    LogUtil.info('settings wifiPsd aboutToAppear title:' + title);
    AppStorage.SetOrCreate('wifiPsd', '');
  }

  onBackPress() {
    LogUtil.info('settings wifiPsd onBackPress start');
    Router.back();
    LogUtil.info('settings wifiPsd onBackPress start');
  }
}