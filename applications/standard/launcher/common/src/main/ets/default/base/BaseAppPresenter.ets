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

import AppModel from '../common/model/AppModel.ets'
import SettingsModel from '../common/model/SettingsModel.ets'
import AppListInfoCacheManager from '../cache/AppListInfoCacheManager.ets'
import ResourceManager from '../manager/ResourceManager.ets'

const KEY_APP_LIST = "appListInfo";
const UNINSTALL_SUCCESS = true;
const KEY_NAME = "name";
const UNINSTALL_PROHIBITED = "UNINSTALL_PROHIBITED";
const SETTING_BUNDLE = 'com.ohos.launcher';
const SETTING_ABILITY = 'com.ohos.launcher.settings.MainAbility';

export default class BaseAppPresenter {
  protected mAppModel: AppModel;
  protected mSettingsModel: SettingsModel;
  protected mAppListInfoCacheManager: AppListInfoCacheManager;
  protected mResourceManager: ResourceManager;
  protected mScreenHeight: number = 0;
  protected mScreenWidth: number = 0;
  private mAppListInstallListener;
  private mAppListUninstallListener;
  private mAppListChangeListener;

  protected constructor() {
    this.mAppModel = AppModel.getInstance();
    this.mSettingsModel = new SettingsModel();
    this.mAppListInfoCacheManager = new AppListInfoCacheManager();
    this.mResourceManager = ResourceManager.getInstance();
    this.mAppListInstallListener = this.appListInstallListener.bind(this);
    this.mAppListUninstallListener = this.appListUninstallListener.bind(this);
    this.mAppListChangeListener = this.appListChangeListener.bind(this);
  }

  public setScreenHeight(height: number): void{
    this.mScreenHeight = height;
  }

  public setScreenWidth(width: number): void{
    this.mScreenWidth = width;
  }

  jumpTo(abilityName, bundleName) {
    this.mAppModel.startApplication(abilityName, bundleName);
  }

  jumpToSetting() {
    this.mAppModel.startApplication(SETTING_ABILITY, SETTING_BUNDLE);
  }

  uninstallApp(uninstallBundleName, ifSystem) {
    if (ifSystem == UNINSTALL_SUCCESS) {
      this.regroupDataAfterUnInstall(UNINSTALL_PROHIBITED);
    } else {
      this.mAppModel.uninstallApp(uninstallBundleName, this.uninstallAppCallback.bind(this));
    }
  }

  uninstallAppCallback(callback) {
    this.regroupDataAfterUnInstall(callback);
  }

  registerAppListChangeCallback() {
    this.mAppModel.registerAppListInstallListener(this.mAppListInstallListener);
    this.mAppModel.registerAppListUninstallListener(this.mAppListUninstallListener);
    this.mAppModel.registerAppListChangeListener(this.mAppListChangeListener);
  }

  unregisterAppListChangeCallback() {
    console.info("Launcher appPresenter unregisterAppListChangeCallback");
    this.mAppModel.unregisterAppListInstallListener(this.mAppListInstallListener);
    this.mAppModel.unregisterAppListUninstallListener(this.mAppListUninstallListener);
    this.mAppModel.unregisterAppListChangeListener(this.mAppListChangeListener);
  }

  public appListInstallListener(bundleInfo) {
    let nameCallback = (appName) => {
      bundleInfo.AppName = appName;
      this.modifyItemList(this.appendItem, bundleInfo);
    }
    this.mResourceManager.getAppName(bundleInfo.labelId, bundleInfo.bundleName, bundleInfo.AppName, nameCallback);
  }

  private appendItem(list, item) {
    list.push(item);
    return list;
  }

  public appListUninstallListener(bundleInfo) {
    this.modifyItemList(this.removeItem, bundleInfo);
  }

  private removeItem(list, item) {
    for (let listItem of list) {
      if (listItem.bundleName == item.bundleName) {
        let index = list.indexOf(listItem);
        list.splice(index, 1);
      }
    }
    return list;
  }

  public appListChangeListener(bundleInfo) {
    this.modifyItemList(this.replaceItem, bundleInfo);
  }

  private replaceItem(list, item) {
    for (let listItem of list) {
      console.info("Launcher replaceCache + " + listItem.bundleName + " + " + item.bundleName);
      if (listItem.bundleName == item.bundleName) {
        let index = list.indexOf(listItem);
        list.splice(index, 1, item);
      }
    }
    return list;
  }

  private modifyItemList(method, item) {
    console.info("Launcher AppListPresenter appListChangeListener + " + JSON.stringify(item));
    let currentCacheList = this.mAppListInfoCacheManager.getCache(KEY_APP_LIST);
    method(currentCacheList, item);
    this.regroupDataAfterInstall(currentCacheList);
  }

  public regroupDataAfterInstall(callbackList) {
  }

  public regroupDataAfterUnInstall(result) {
  }

  getAppName(keyName) {
    return this.mResourceManager.getAppResourceCache(keyName, KEY_NAME);
  }
}
