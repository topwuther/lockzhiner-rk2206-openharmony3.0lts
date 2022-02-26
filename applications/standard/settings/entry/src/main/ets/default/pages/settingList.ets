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
import ConfigData from '../common/baseUtil/ConfigData.ets';
import EntryComponent from './component/entryComponent.ets';
import HeadComponent from './component/headComponent.ets';
import SettingListModel from '../model/settingListImpl/SettingListModel.ets';
import WifiModel from '../model/wifiImpl/WifiModel.ets';
import SubscriberUtils from '../common/baseUtil/SubscriberUtil.ets';
import GlobalResourceManager from '../common/baseUtil/GlobalResourceManager.ets';
import BrightnessSettingModel from '../model/brightnessImpl/BrightnessSettingModel.ets';
import Router from '@system.router';

const SETTING_LIST_INDEX = 0;

@Entry
@Component
struct SettingList {
  @State settingsList: any[]= [];
  @State wifiStatus: boolean= false;
  @State status: any= '';

  build() {
    Column() {
      HeadComponent({ icBackIsVisibility: false, headName: $r('app.string.settings'), isActive: false });
      List() {
        ForEach(this.settingsList, (item) => {
          ListItem() {
            Navigator({ target: item.settingUri }) {
              EntryComponent({
                settingIcon: item.settingIcon,
                settingTitle: item.settingTitle,
                settingSummary: item.settingSummary,
                settingValue: item.settingValue,
                settingArrow: item.settingArrow,
                settingArrowStyle: '',
                settingUri: item.settingUri
              });
            }
          }
          .onTouch(() => {
            Router.push({
              uri: item.settingUri,
            });
          })
          .height($r('app.float.wh_value_70'));
        });
      }
      .divider({
        strokeWidth: 1,
        color: $r('app.color.color_E3E3E3_grey'),
        startMargin: $r('app.float.wh_value_65'),
        endMargin: $r('app.float.wh_value_15')
      })
      .width(ConfigData.WH_100_100)
      .height(ConfigData.WH_100_100)
      .alignSelf(ItemAlign.Start);
    }
    .width(ConfigData.WH_100_100)
    .height(ConfigData.WH_100_100);
  }

  aboutToAppear(): void{
    LogUtil.info('settings SettingList aboutToAppear enter');
    this.settingsList = SettingListModel.setSettingListener();
    SubscriberUtils.registerStatusListener();
    LogUtil.info('settings SettingList aboutToAppear startPage');
    SettingListModel.startPage(this.pageValue);
    this.setTitleValue();
    let nYear = GlobalResourceManager.getStringByResource($r('app.string.year'));
    nYear.then(resp=> BrightnessSettingModel.setStorageValue(ConfigData.DATE_AND_TIME_YEAR,resp) );
    let nMonth = GlobalResourceManager.getStringByResource($r('app.string.month'));
    nMonth.then(resp=> BrightnessSettingModel.setStorageValue(ConfigData.DATE_AND_TIME_MONTH,resp) );
    let nDay = GlobalResourceManager.getStringByResource($r('app.string.day'));
    nDay.then(resp=> {
      BrightnessSettingModel.setStorageValue(ConfigData.DATE_AND_TIME_DAY,resp);
      LogUtil.info('SettingList day:'+ BrightnessSettingModel.getStorageValue(ConfigData.DATE_AND_TIME_DAY,''));
    }
    )
    LogUtil.info('settings SettingList aboutToAppear end');
  }

  pageValue(pagePath): void{
    LogUtil.info('settings settingList pageValue pagePath:' + pagePath);
    if ('pages/wifi' === pagePath) {
      Router.push({
        uri: 'pages/wifi',
      });
    }
    LogUtil.info('settings settingList pageValue end');
  }

  setTitleValue() {
    LogUtil.info('SettingList setTitleValue this.settingsList:' + JSON.stringify(this.settingsList));
    for (let key in this.settingsList) {
      let settingAlias = this.settingsList[key].settingAlias;
      let value;
      this.wifiStatus = WifiModel.getWifiStatus();
      if ('wifiTab' === settingAlias) {
        value = $r('app.string.wifiTab');
        if (this.wifiStatus) {
          this.settingsList[key].settingValue = $r('app.string.enabled');
        } else {
          this.settingsList[key].settingValue = $r('app.string.disabled');
        }
      }
      else if ('brightnessTab' === settingAlias) {
        value = $r('app.string.brightnessTab');
      } else if ('applyTab' === settingAlias) {
        value = $r('app.string.applyTab');
      } else if ('dateAndTimeTab' === settingAlias) {
        value = $r('app.string.dateAndTimeTab');
      } else if ('aboutTab' === settingAlias) {
        value = $r('app.string.aboutTab');
      }
      this.settingsList[key].settingTitle = value;
    }
    LogUtil.info('SettingList setTitleValue this.settingsList:' + JSON.stringify(this.settingsList));
  }

  onPageShow(): void{
    LogUtil.info('SettingList onPageShow in:' + JSON.stringify(this.settingsList));
    this.wifiStatus = WifiModel.getWifiStatus();
    LogUtil.info('SettingList onPageShow this.wifiStatus:' + this.wifiStatus);
    for (let key in this.settingsList) {
      let settingAlias = this.settingsList[key].settingAlias;
      if ('wifiTab' === settingAlias) {
        if (this.wifiStatus) {
          this.status = $r('app.string.enabled');
        } else {
          this.status = $r('app.string.disabled');
        }
        LogUtil.info('SettingList onPageShow settingValue:' + this.settingsList[key].settingValue);
      }
    }
    this.settingsList.unshift({
      settingSummary: '',
      settingTitle: this.settingsList[SETTING_LIST_INDEX].settingTitle,
      settingValue: this.status,
      settingAlias: this.settingsList[SETTING_LIST_INDEX].settingAlias,
      settingIcon: this.settingsList[SETTING_LIST_INDEX].settingIcon,
      settingArrow: this.settingsList[SETTING_LIST_INDEX].settingArrow,
      settingUri: this.settingsList[SETTING_LIST_INDEX].settingUri,
    });
    this.settingsList.splice(1, 1);
    LogUtil.info('SettingList onPageShow out:' + +JSON.stringify(this.settingsList));
  }

  aboutToDisappear(): void{
    LogUtil.info('settingList aboutToDisappear in');
    SubscriberUtils.unSubscriberListener();
    LogUtil.info('settingList aboutToDisappear out');
  }
}