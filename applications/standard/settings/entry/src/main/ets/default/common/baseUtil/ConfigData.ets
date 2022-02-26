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

export class ConfigData {
  FILE_URI = '/data/accounts/account_0/applications/com.ohos.settings'
  + '/com.ohos.settings/assets/entry/resources/rawfile/';
  PREFERENCES_PATH = '/data/accounts/account_0/appdata/com.ohos.settings/sharedPreference/SettingPreferences';
  BRIGHTNESS_SAVE_VALUE_KEY = 'BrightnessSaveValue';
  SENT_EVENT_BROADCAST_BRIGHTNESS_VALUE = 'BRIGHTNESS_VALUE';
  SENT_EVENT_WIFI_CONNECT_NAME = 'WIFI_CONNECT_NAME';
  BRIGHTNESS_DEFAULT_VALUE = 50;
  DEFAULT_BUNDLE_NAME = 'com.ohos.settings';
  DATE_AND_TIME_YEAR='DATE_AND_TIME_YEAR'
  DATE_AND_TIME_MONTH='DATE_AND_TIME_MONTH'
  DATE_AND_TIME_DAY='DATE_AND_TIME_DAY'
  DateAndTime

  WH_100_100 = '100%';
  WH_30_100 = '30%';
  WH_35_100 = '35%';
  WH_40_100 = '40%';
  WH_45_100 = '45%';
  WH_50_100 = '50%';
  WH_55_100 = '55%';
  WH_83_100 = '83%';
  WH_90_100 = '90%';
  value_1 = 1;
  value_5 = 5;
  value_10 = 10;
  value_15 = 15;
  value_20 = 20;
  value_40 = 40;
  value_50 = 50;
  value_60 = 60;
  value_65 = 65;
  value_70 = 70;
  value_80 = 80;
  value_130 = 130;
  value_160 = 160;
  font_18 = 18;
  font_20 = 20;
  font_22 = 22;
  font_24 = 24;

  ONE_HUNDRED_PERCENT = '100%';
  THIRTY_PERCENT = '30%';
  FORTY_PERCENT = '40%';
  FIFTY_PERCENT = '50%';
  FORTY_FIVE_PERCENT  = '45%';
  FIFTY_FIVE_PERCENT = '55%';

  height_value_1 = 1;
  height_value_5 = 5;
  height_value_10 = 10;
  height_value_15 = 15;
  height_value_20 = 20;
  height_value_40 = 40;
  height_value_60 = 60;
  height_value_65 = 65;
  height_value_70 = 70;
  height_value_80 = 80;
  height_value_160 = 160;

  width_value_1 = 1;
  width_value_5 = 5;
  width_value_10 = 10;
  width_value_15 = 15;
  width_value_20 = 20;
  width_value_40 = 40;
  width_value_60 = 60;
  width_value_65 = 65;
  width_value_70 = 70;
  width_value_80 = 80;
  width_value_160 = 160;

  font_size_18 = 18;
  font_size_20 = 20;
  font_size_22 = 22;
  font_size_24 = 24;

  color_333333_grey = '#333333';
  color_666666_grey = '#666666';
  color_999999_grey = '#999999';
  color_E3E3E3_grey = '#E3E3E3';
  color_00000000_white = '#00000000';

  color_333333 = '#333333';
  color_666666 = '#666666';
  color_999999 = '#999999';
  color_E3E3E3 = '#E3E3E3';
  color_00000000 = '#00000000';
  MAX_LINES_1 = 1;
  MAX_LINES_2 = 2;
  MAX_LINES_3 = 3;
  DURATION_TIME = 200;
}

let configData = new ConfigData();
export default configData as ConfigData
;