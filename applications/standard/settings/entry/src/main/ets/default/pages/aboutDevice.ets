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
import ConfigData from '../common/baseUtil/ConfigData.ets';
import HeadComponent from './component/headComponent.ets';
import AboutDeviceModel from '../model/aboutDeviceImpl/AboutDeviceModel.ets'

@Entry
@Component
struct Index {
  private systemName: any  = 'OpenHarmony'
  private deviceInfo: any = ''
  private aboutDeviceList: any[] = []

  build() {
    Column() {
      HeadComponent({ headName: $r('app.string.aboutTab'), isActive: true })
      List() {
        ForEach(this.aboutDeviceList, (item) => {
          ListItem() {
            Flex({ justifyContent: FlexAlign.SpaceBetween }) {
              Text(item.settingTitle)
                .fontSize($r('app.float.font_22'))
                .fontColor($r('app.color.color_333333_grey'))
                .textAlign(TextAlign.Start)
                .align(Alignment.Center)
                .flexShrink(0)
                .alignSelf(ItemAlign.Center);
              Text(item.settingValue)
                .fontSize($r('app.float.font_20'))
                .fontColor($r('app.color.color_666666_grey'))
                .align(Alignment.BottomEnd)
                .textAlign(TextAlign.End);
            }
            .width(ConfigData.WH_100_100)
          }
          .height($r('app.float.wh_value_70'))
        })
      }
      .divider({ strokeWidth: 1, color: $r('app.color.color_E3E3E3_grey') })
      .width(ConfigData.WH_100_100)
      .height(ConfigData.WH_100_100)
      .margin({ left: $r('app.float.wh_value_15'), right: $r('app.float.wh_value_15') })
      .align(Alignment.TopStart);
    }
  }

  private getDeviceInfo(): void {
    for (let key in this.aboutDeviceList) {
      let value = this.aboutDeviceList[key].settingAlias;
      if ('systemName' === value) {
        this.aboutDeviceList[key].settingTitle = $r('app.string.systemName');
      }
      if ('model' === value) {
        this.aboutDeviceList[key].settingValue = this.deviceInfo.productModel;
        this.aboutDeviceList[key].settingTitle = $r('app.string.model');
      }
      if ('companyInfo' === value) {
        this.aboutDeviceList[key].settingValue = this.deviceInfo.manufacture;
        this.aboutDeviceList[key].settingTitle = $r('app.string.companyInfo');
      }
      if ('deviceId' === value) {
        this.aboutDeviceList[key].settingValue = this.deviceInfo.serial;
        this.aboutDeviceList[key].settingTitle = $r('app.string.deviceId');
      }
      if ('softwareVersion' === value) {
        this.aboutDeviceList[key].settingValue = this.deviceInfo.displayVersion;
        this.aboutDeviceList[key].settingTitle = $r('app.string.softwareVersion');
      }
    }
  }

  private aboutToAppear(): void {
    console.log('settings get device info come in');
    this.deviceInfo = AboutDeviceModel.setOnAboutDeviceListener();
    this.aboutDeviceList = AboutDeviceModel.getAboutDeviceInfoListener();
    this.getDeviceInfo();
    console.log('settings get device info' + JSON.stringify(AboutDeviceModel.setOnAboutDeviceListener()));
    console.log('settings get device info end in');
  }

  onBackPress() {
    console.log('settings get device onBackPress in');
    Navigator().active(true);
    console.log('settings get device onBackPress out');
  }
}
