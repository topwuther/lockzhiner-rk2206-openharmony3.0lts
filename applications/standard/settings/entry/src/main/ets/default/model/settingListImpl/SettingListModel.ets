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
import ConfigData from '../../common/baseUtil/ConfigData.ets';
import LogUtil from '../../common/baseUtil/LogUtil.ets';
import  BaseParseConfModel from '../baseParseConfImpl/BaseParseConfModel.ets';
import BaseModel from '../BaseModel.ets';
import FeatureAbility from '@ohos.ability.featureAbility';

/**
 * app setting homepage service class
 */
export class SettingListModel extends BaseModel {
  setSettingListener(): any[]{
    LogUtil.info('settings SettingListModel setSettingListener in');
    return BaseParseConfModel.getJsonData(ConfigData.FILE_URI.concat('settinglist.json'));
  }

  startPage(callback): void{
    LogUtil.info('settings SettingList startPage start');
    FeatureAbility.getWant().then((Want) => {
      LogUtil.info(`settings SettingList startWifiPage successful. Data: ${ JSON.stringify(Want) }`);
      LogUtil.info(`settings SettingList startWifiPage successful. Want.uri: ${ Want.uri }`);
      callback( Want.uri);
    })
      .catch((error) => {
      LogUtil.error('settings SettingList startWifiPage failed. Cause: ' + JSON.stringify(error));
    })
  }
}

let settingListModel = new SettingListModel();
export default settingListModel as SettingListModel
;