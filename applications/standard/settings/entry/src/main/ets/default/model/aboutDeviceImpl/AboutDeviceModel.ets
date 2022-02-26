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
import BaseModel from '../BaseModel';
import  BaseParseConfModel from '../baseParseConfImpl/BaseParseConfModel.ets';
import ConfigData from '../../common/baseUtil/ConfigData.ets';
import LogUtil from '../../common/baseUtil/LogUtil.ets';
import deviceInfo from '@ohos.deviceInfo';

/**
 * about device service class
 */
export class AboutDeviceModel extends BaseModel {
  setOnAboutDeviceListener(): any {
    LogUtil.info('settings setOnAboutDeviceListener in');
    LogUtil.info('settings setOnAboutDeviceListener deviceInfo ' + JSON.stringify(deviceInfo));
    return deviceInfo;
  }

  public getAboutDeviceInfoListener(): any[] {
    LogUtil.info('settings getAboutDeviceInfoListener come in');
    return BaseParseConfModel.getJsonData(ConfigData.FILE_URI.concat('aboutDevice.json'));
  }
}

let aboutDeviceModel = new AboutDeviceModel();
export default aboutDeviceModel as AboutDeviceModel
;