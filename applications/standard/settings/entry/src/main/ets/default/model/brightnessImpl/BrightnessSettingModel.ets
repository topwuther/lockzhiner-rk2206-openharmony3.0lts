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
import BaseModel from '../BaseModel.ets';
import LogUtil from '../../common/baseUtil/LogUtil.ets';
import ConfigData from '../../common/baseUtil/ConfigData.ets';
import SubscriberUtils from '../../common/baseUtil/SubscriberUtil.ets';
import Brightness from '@ohos.brightness';
import Storage from '@ohos.data.storage';

let mBrightnessPreferences = Storage.getStorageSync(ConfigData.PREFERENCES_PATH);

export class BrightnessSettingModel extends BaseModel {
  setBrightnessListener(brightnessValue) {
    LogUtil.info('settings settingList BrightnessSettingModel Brightness:' + brightnessValue);
    Brightness.setValue(brightnessValue);
    LogUtil.info('settings settingList BrightnessSettingModel end');
  }

  setStorageValue(key,brightnessValue) {
    LogUtil.info('settings BrightnessSettingModel setStorageValue Storage:' + brightnessValue);
    ConfigData.BRIGHTNESS_SAVE_VALUE_KEY
    mBrightnessPreferences.putSync(key, brightnessValue);
    mBrightnessPreferences.flushSync();
  }

  getStorageValue(key,defaultValue): any {
    LogUtil.info('settings BrightnessSettingModel getStorageValue in' +
    mBrightnessPreferences.getSync(key, defaultValue));
    return mBrightnessPreferences.getSync(key, defaultValue);
  }
}

let brightnessSettingModel = new BrightnessSettingModel();
export default brightnessSettingModel as BrightnessSettingModel;