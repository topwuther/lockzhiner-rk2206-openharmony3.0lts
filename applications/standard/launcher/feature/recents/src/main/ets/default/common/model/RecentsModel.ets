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

import NapiAbilityManager from '@ohos.app.abilityManager';
import Storage from '@ohos.data.storage';
import BundleMgr from '@ohos.bundle';
import FeatureAbility from '@ohos.ability.featureAbility';

const PREFERENCES_PATH: string = '/data/accounts/account_0/appdata/com.ohos.launcher/sharedPreference/LauncherPreference';
const MAX_NUM: number = 20;
const PERMISSION_NUM: number = 8;
const NON: number = 0;
const RECENT_PROCESS_LIMIT_KEY: string = 'RecentProcessLimit';
const DEFAULT_RECENT_PROCESS_LIMIT: number = 3;

/**
 * Class RecentsModel.
 */
export default class RecentsModel {
  private static mRecentsModel = new RecentsModel();
  private mRecentList = [];
  private mIconResultCount: number = 0;
  private mPreferences: any = Storage.getStorageSync(PREFERENCES_PATH);

  constructor() {
  }

  /**
   * Return an instance of RecentsModel.
   */
  public static getInstance() {
    return this.mRecentsModel;
  }

  /**
   * Get recent process list.
   *
   * @param {object} callback - The callback from presenter.
   */
  getRecentProcessList(callback) {
    console.info("Launcher recents  RecentsModel getRecentProcessList start");
    this.mIconResultCount = 0;
    console.info("Launcher recents  RecentsModel NapiAbilityManager.queryRecentAbilityMissionInfos start");
    NapiAbilityManager.queryRunningAbilityMissionInfos(MAX_NUM)
      .then((data) => {
      this.mRecentList = [];
      console.info("Launcher recents  RecentsModel NapiAbilityManager.queryRecentAbilityMissionInfos() callback");
      console.info('Launcher recents  queryRecentAbilityMissionInfos data length [' + data.length + ']');
      console.info('Launcher recents  queryRecentAbilityMissionInfos data = ' + JSON.stringify(data));
      this.mRecentList = [];
      if (data.length === 0) {
        console.info('Launcher recents data empty');
        callback(this.mRecentList);
        return;
      }
      for (let i = 0; i < data.length; i++) {
        let recentTaskInfo =
          {
            AppName: data[i].missionDescription.label,
            AppId: data[i].topAbility.bundleName,
            bundleName: data[i].topAbility.bundleName,
            abilityName: data[i].topAbility.abilityName,
            iconId: '',
            labelId: '',
            missionId: data[i].id
          }
        BundleMgr.getApplicationInfo(data[i].topAbility.bundleName, PERMISSION_NUM, NON)
          .then(data1 => {
          recentTaskInfo.iconId = data1.iconId;
          recentTaskInfo.labelId = data1.labelId;
          this.mRecentList.push(recentTaskInfo);
          this.mIconResultCount++;
          if (this.mIconResultCount === this.mRecentList.length) {
            callback(this.mRecentList);
          }
        })
          .catch(error =>
        console.error("Launcher recents RecentsModel getRecentProcessList promise::catch : " + JSON.stringify(error))
        );
      }
    })
      .catch(error =>
    console.error("Launcher recents RecentsModel getRecentProcessList promise::catch : " + JSON.stringify(error))
    );
    console.info("Launcher recents  RecentsModel getRecentProcessList end");
  }

  /**
   * Clear recent process list.
   */
  clearRecentProcess() {
    console.info("Launcher recents  RecentsModel clearRecentProcess start");
    while (this.mRecentList.length > 0) {
      this.mRecentList.pop();
    }
    NapiAbilityManager.clearMissions((data) => {
    });
    AppStorage.SetOrCreate('recentProcessList', this.mRecentList);
    setTimeout(() => {
      console.info("Launcher recents  RecentsModel feature_ability.terminateAbility start");
      FeatureAbility.terminateAbility()
        .then(data => console.info("Launcher recents  terminateAbility promise::then : " + data))
        .catch(error => console.info("Launcher recents  terminateAbility promise::catch : " + error))
    }, 1000);
    console.info("Launcher recents  RecentsModel clearRecentProcess end");
  }

  /**
   * Remove recent process list.
   *
   * @param {string} missionId - The missionId of recent process.
   */
  removeRecentProcess(missionId) {
    console.info("Launcher recents  RecentsModel removeRecentProcess start");
    console.info('Launcher recents RecentsModel removeRecentProcess missionId  ' + missionId);
    for (let idx = 0; idx < this.mRecentList.length; idx++) {
      if (this.mRecentList[idx].missionId === missionId) {
        this.mRecentList.splice(idx, 1);
        break;
      }
    }
    NapiAbilityManager.removeMission(missionId, (err, data) => {
      console.info('removeMission data [' + data + ']');
    });
    AppStorage.SetOrCreate('recentProcessList', this.mRecentList);
    if (this.mRecentList.length == 0) {
      setTimeout(() => {
        console.info("Launcher recents  RecentsModel feature_ability.terminateAbility start");
        FeatureAbility.terminateAbility()
          .then(data => console.info("Launcher recents  terminateAbility promise::then : " + data))
          .catch(error => console.info("Launcher recents  terminateAbility promise::catch : " + error))
      }, 1000);
    }
    console.info("Launcher recents  RecentsModel removeRecentProcess end");
  }

  /**
   * Get recent process list.
   *
   * @return {number} - The number of recent process.
   */
  getRecentProcessLimit() {
    console.info("Launcher recents RecentsModel getRecentProcessLimit start");
    let limit: number = DEFAULT_RECENT_PROCESS_LIMIT;
    if (this.mPreferences != null && this.mPreferences != undefined) {
      limit = this.mPreferences.getSync(RECENT_PROCESS_LIMIT_KEY, DEFAULT_RECENT_PROCESS_LIMIT);
    }
    console.info("Launcher recents RecentsModel getRecentProcessLimit end limit = " + limit);
    return limit;
  }

  /**
   * Start ability.
   *
   * @param { abilityName, bundleName  } - The app abilityName, bundleName.
   */
  startAbility(abilityName, bundleName) {
    console.info('Launcher startApplication abilityname');
    let result = FeatureAbility.startAbility({
      want: {
        bundleName: bundleName,
        abilityName: abilityName
      }
    })
      .then(data =>
    console.info("Launcher promise::then : " + JSON.stringify(data))
    )
      .catch(error =>
    console.info("Launcher promise::catch : " + JSON.stringify(error))
    );
    console.info("Launcher AceApplication : startAbility : " + result);
  }
}