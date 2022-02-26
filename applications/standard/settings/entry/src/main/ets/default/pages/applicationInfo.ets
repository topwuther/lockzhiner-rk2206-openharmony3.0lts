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
import EntryComponent from './component/entryComponent.ets';
import HeadComponent from './component/headComponent.ets';
import ConfigData from '../common/baseUtil/ConfigData.ets';
import Router from '@system.router'

let appInfo;

@Entry
@Component
struct ApplicationInfo {
  build() {
    Column() {
      HeadComponent({ headName: $r('app.string.applicationInfo'), isActive: true });
      Row() {
        EntryComponent({
          settingIcon: appInfo.settingIcon,
          settingTitle: appInfo.settingTitle,
          settingSummary: appInfo.settingSummary,
          settingValue: '',
          settingArrow: '',
          settingArrowStyle: '',
          settingUri: appInfo.settingUri
        });
      }
    }
  }

  aboutToAppear(): void{
    LogUtil.info('settings ApplicationInfo aboutToAppear in');
    appInfo = Router.getParams();
    LogUtil.info('settings ApplicationInfo aboutToAppear this.item:' + JSON.stringify(appInfo));
  }

  onBackPress() {
    LogUtil.info('settings ApplicationInfo onBackPress in');
    Router.back();
    LogUtil.info('settings ApplicationInfo onBackPress out');
  }
}