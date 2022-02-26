/*
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

import Storage from '@ohos.data.storage';
import ILayoutConfig from './ILayoutConfig.ets'
import DefaultLayoutConfig from '../configs/DefaultLayoutConfig.ets'

const QEUIPMENT_TYPE = 'EquipmentType';
const APP_PAGE_START_CONFIG = 'AppStartPageType';
const GRID_CONFIG = "GridConfig";
const RECENT_PROCESS_LIMIT = "RecentProcessLimit";
const GRID_LAYOUT_INFO = "GridLayoutInfo";
const PREFERENCES_PATH = '/data/accounts/account_0/appdata/com.ohos.launcher/sharedPreference/LauncherPreference';

let mPreferences = Storage.getStorageSync(PREFERENCES_PATH);
let mGridLayoutInfo = {
  layoutDescription: {
    pageCount: 1,
    row: 5,
    column: 4
  },
  layoutInfo: [],
  bottomBarInfo: []
}

export default class StorageLayoutConfig extends ILayoutConfig {
  constructor() {
    super()
  }

  /**
   * Should overridden by sub-classes , load the launcher layout view type.
   */
  public loadAppPageStartConfig() {
    console.info('Launcher mPreferences get APP_PAGE_START_CONFIG');
    let data = mPreferences.getSync(APP_PAGE_START_CONFIG, DefaultLayoutConfig.DefaultAppPageStartConfig);
    console.info('Launcher mPreferences get' + data);
    return data;
  }

  /**
   * Should overridden by sub-classes , save the launcher layout view type.
   */
  public saveAppPageStartConfig(type) {
    console.info('Launcher mPreferences put type' + type);
    mPreferences.putSync(APP_PAGE_START_CONFIG, type);
    mPreferences.flushSync();
    console.info('Launcher mPreferences put type flush');
  }

  /**
   * Should overridden by sub-classes , load the launcher grid view layout config id.
   */
  public loadGridConfig() {
    console.info('Launcher mPreferences get GRID_CONFIG');
    let data = mPreferences.getSync(GRID_CONFIG, DefaultLayoutConfig.DefaultGridConfig);
    console.info('Launcher mPreferences get' + data);
    return data;
  }

  /**
   * Should overridden by sub-classes , save the launcher grid view layout config id.
   */
  public saveGridConfig(id) {
    console.info('Launcher mPreferences put id' + id);
    mPreferences.putSync(GRID_CONFIG, id);
    mPreferences.flushSync();
    console.info('Launcher mPreferences put id flush');
  }

  /**
   * Should overridden by sub-classes , load the recent process max limit.
   */
  public loadRecentProcessLimit() {
    console.info('Launcher mPreferences get');
    let data = mPreferences.getSync(RECENT_PROCESS_LIMIT, DefaultLayoutConfig.DefaultRecentProcessLimit);
    console.info('Launcher mPreferences get' + data);
    return data;
  }

  /**
   * Should overridden by sub-classes , save the recent process max limit.
   */
  public saveRecentProcessLimit(num) {
    console.info('Launcher mPreferences put num' + num);
    mPreferences.putSync(RECENT_PROCESS_LIMIT, num);
    mPreferences.flushSync();
    console.info('Launcher mPreferences put num flush');
  }

  /**
   * Should overridden by sub-classes , load the layout information of grid view.
   */
  public loadGridLayoutInfo() {
    console.info('Launcher StorageLayoutConfig loadGridLayoutInfo start');
    let data: any = mPreferences.getSync(GRID_LAYOUT_INFO, '');
    console.info('Launcher StorageLayoutConfig loadGridLayoutInfo ' + data);
    if (data == '') {
      return mGridLayoutInfo;
    } else {
      return JSON.parse(data);
    }
  }

  /**
   * Should overridden by sub-classes , save the layout information of grid view.
   */
  public saveGridLayoutInfo(layoutInfo) {
    console.info('Launcher StorageLayoutConfig saveGridLayoutInfo start');
    mPreferences.putSync(GRID_LAYOUT_INFO, JSON.stringify(layoutInfo));
    mPreferences.flushSync();
    console.info('Launcher StorageLayoutConfig saveGridLayoutInfo end');
  }

  /**
   * Should overridden by sub-classes , remove layout information of grid view.
   */
  public removeGridLayoutInfo() {
    console.info('Launcher StorageLayoutConfig removeGridLayoutInfo start');
    mPreferences.deleteSync(GRID_LAYOUT_INFO);
    console.info('Launcher StorageLayoutConfig removeGridLayoutInfo start');
  }

  saveEquipmentType(Etype) {
    console.info('Launcher StorageLayoutConfig saveEquipmentType start');
    mPreferences.putSync(QEUIPMENT_TYPE, Etype);
    mPreferences.flushSync();
    console.info('Launcher StorageLayoutConfig saveEquipmentType end');
  }

  loadEquipmentType(): string {
    console.info('Launcher settings StorageLayoutConfig loadEquipmentType  mPreferences get');
    let data = mPreferences.getSync(QEUIPMENT_TYPE, 'phone');
    console.info('Launcher settings StorageLayoutConfig loadEquipmentType mPreferences get' + data);
    return data;
  }
}