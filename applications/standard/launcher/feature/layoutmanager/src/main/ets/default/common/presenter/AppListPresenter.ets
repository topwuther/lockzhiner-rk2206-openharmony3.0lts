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

import Prompt from '@ohos.prompt';
import BaseAppPresenter from '../../../../../../../../common/src/main/ets/default/base/BaseAppPresenter.ets';
import PinyinSort from '../../../../../../../../common/src/main/ets/default/utils/PinyinSort.ets'

const PROHIBITED = '禁止卸载'
const UNINSTALL_SUCCEEDED = '卸载成功'
const UNINSTALL_DAILED = '卸载失败'
const KEY_APP_LIST = "appListInfo";
const UNINSTALL_SUCCESS = "UNINSTALL_SUCCESS";
const UNINSTALL_FAILED = "UNINSTALL_FAILED";
const UNINSTALL_PROHIBITED = "UNINSTALL_PROHIBITED";
const KEY_NAME = "name";

export default class AppListPresenter extends BaseAppPresenter {
  private static appListPresenter: AppListPresenter = new AppListPresenter();
  private mPinyinSort: PinyinSort;

  private constructor() {
    super()
    this.mPinyinSort = new PinyinSort();
  }

  public static getInstance(): AppListPresenter {
    return this.appListPresenter;
  }

  public getAppList() {
    this.mAppModel.getAppList(this.getListCallback.bind(this));
  }

  public getListCallback(list) {
    let cached = true;
    for (let item of list) {
      let appName = this.mResourceManager.getAppResourceCache(item.bundleName, KEY_NAME);
      console.info("Launcher AppListPresenter regroupDataAfterInstall + appName = " + appName);
      if (appName != null) {
        cached = false;
        item.AppName = appName;
      }
    }
    if (cached) {
      list = this.mAppListInfoCacheManager.getCache(KEY_APP_LIST);
    } else {
      list.sort(this.mPinyinSort.sortByAppName.bind(this.mPinyinSort));
      this.mAppListInfoCacheManager.setCache(KEY_APP_LIST, list);
    }
    AppStorage.SetOrCreate('listInfo', list);
  }

  public regroupDataAfterInstall(callbackList) {
    for (let item of callbackList) {
      let appName = this.mResourceManager.getAppResourceCache(item.bundleName, KEY_NAME);
      console.info("Launcher AppListPresenter regroupDataAfterInstall + appName = " + appName);
      if (appName != null) {
        item.AppName = appName;
      }
    }
    callbackList.sort(this.mPinyinSort.sortByAppName.bind(this.mPinyinSort));
    this.mAppListInfoCacheManager.setCache(KEY_APP_LIST, callbackList);
    AppStorage.SetOrCreate('listInfo', callbackList);
  }

  public regroupDataAfterUnInstall(result) {
    console.info("Launcher AppListView getUninstallApp uninstallationResult:" + result);
    if (result == UNINSTALL_PROHIBITED) {
      Prompt.showToast({
        message: PROHIBITED
      })
    } else if (result == UNINSTALL_SUCCESS) {
      Prompt.showToast({
        message: UNINSTALL_SUCCEEDED
      });
    } else if (result == UNINSTALL_FAILED) {
      Prompt.showToast({
        message: UNINSTALL_DAILED
      });
    }
  }

  public intoSetting() {
    console.info("Launcher AppListView intoSetting");
    this.jumpToSetting();
  }

  /**
   * Open application function.
   *
   * @param {string} abilityName - ability name of the application to be jump to.
   * @param {string} bundleName - bundle name of the application to be jump to.
   */
  public openApplication(abilityName, bundleName) {
    console.info("Launcher AppListView openApplication abilityName:" + abilityName);
    this.jumpTo(abilityName, bundleName);
  }
}