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
import ImageAnimatorComponent from './component/imageAnimatorComponent.ets';
import ConfigData from '../common/baseUtil/ConfigData.ets';
import AppManagementModel from '../model/appManagementImpl/AppManagementModel.ets';
import EntryComponent from './component/entryComponent.ets';
import HeadComponent from './component/headComponent.ets';
import LogUtil from '../common/baseUtil/LogUtil.ets';
import Router from '@system.router';

@Entry
@Component
struct AppManagement {
  @StorageLink('appManagementList') appList: any[]= [];
  @State animatorIsShow: boolean= true;
  @State whtl: number= 80;

  build() {
    Column() {
      HeadComponent({ headName: $r('app.string.appManagement'), isActive: true });
      Column() {
        ImageAnimatorComponent({ imageWidth: $r('app.float.wh_value_80'),
          imageHeight: $r('app.float.wh_value_80'), whtl: $r('app.float.wh_value_80') });
      }
      .zIndex(1)
      .visibility(this.animatorIsShow ? Visibility.Visible : Visibility.None)
      .align(Alignment.Center)
      .padding({ top: ConfigData.WH_35_100 })
      .width(ConfigData.WH_100_100)
      .height(ConfigData.WH_100_100);

      List() {
        ForEach(this.appList, (item) => {
          ListItem() {
            EntryComponent({
              settingIcon: item.settingIcon,
              settingTitle: item.settingTitle + '',
              settingSummary: item.settingSummary,
              settingValue: item.settingValue,
              settingArrow: item.settingArrow,
              settingArrowStyle: '',
              settingUri: item.settingUri,
            });
          }
          .onClick(() => {
            Router.push({ uri: item.settingUri, params: item });
          })
          .height($r('app.float.wh_value_70'));
        }, item => JSON.stringify(item));
      }
      .divider({
        strokeWidth: 1,
        color: $r('app.color.color_E3E3E3_grey'),
        startMargin: $r('app.float.wh_value_65'),
        endMargin: $r('app.float.wh_value_15')
      })
      .width(ConfigData.WH_100_100)
      .height(ConfigData.WH_100_100)
      .margin({ bottom: $r('app.float.wh_value_70') })
      .visibility(this.animatorIsShow ? Visibility.None : Visibility.Visible)
      .zIndex(0);
    }
    .align(Alignment.Center)
    .width(ConfigData.WH_100_100)
    .height(ConfigData.WH_100_100);
  }

  private aboutToAppear(): void{
    LogUtil.info('settings AppManagement aboutToAppear in');
    AppManagementModel.setAppManagementListener();
    LogUtil.info('settings AppManagement aboutToAppear  appList:' + JSON.stringify(this.appList));
  }

  onPageShow(): void{
    LogUtil.info('settings AppManagement onPageShow in');
    this.animatorIsShow = [] === this.appList ? true : false;
    LogUtil.info('settings AppManagement onPageShow out');
  }

  onBackPress() {
    LogUtil.info('settings AppManagement onBackPress in');
    AppStorage.SetOrCreate('appManagementList', []);
    Router.back();
    LogUtil.info('settings AppManagement onBackPress out');
  }

  private aboutToDisappear(): void{
    LogUtil.info('settings AppManagement aboutToDisappear in');
    AppStorage.SetOrCreate('appManagementList', []);
    LogUtil.info('settings AppManagement aboutToDisappear out');
  }
}